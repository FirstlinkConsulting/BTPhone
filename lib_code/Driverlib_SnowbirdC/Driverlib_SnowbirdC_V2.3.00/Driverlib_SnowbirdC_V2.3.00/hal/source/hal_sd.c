/**
 * @file    hal_sd.c
 * @brief   Implement sd operations of how to control sd.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "share_pin.h"
#include "l2.h"
#include "clk.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_common_sd.h"
#include "drv_api.h"
#include "drv_module.h"
#include "arch_init.h"

//The interface shall be INTERFACE_MMC1,INTERFACE_SD2
#define SD_DRV_PROTECT(interface, mode) \
        do{ \
            set_interface(interface, mode);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define SD_DRV_UNPROTECT(interface,mode) \
        do{ \
            set_interface(interface, mode);\
        }while(0)

#define SD_USE_L2      1 ///1 is use L2,0 is use cpu   

static T_BOOL send_acmd(T_U32 rca, T_U8 cmd_index, T_U8 resp, T_U32 arg);
static T_BOOL sd_get_cid(T_U32 *cid);
static T_BOOL sd_get_csd(T_U32 rca, T_U32 *csd);
static T_U8   sd_nego_volt(T_pSD_DEVICE drv_card, T_U32 volt);
static T_U8   sd_get_ocr(T_U32 rca, T_U32 *ocr );
static T_BOOL sd_set_block_len(T_U32 block_len);
static T_BOOL sd_get_card_status(T_U32 rca, T_U32 *status_buf );
static T_U8   mmc_nego_volt(T_pSD_DEVICE drv_card, T_U32 volt);
static T_BOOL sd_rw_block(T_pSD_DEVICE drv_card, T_U32 block_src, T_U8 *databuf,
                              T_U32 size, T_U32 mode,T_U8 dir);
static T_BOOL mmc_get_extcsd(T_pSD_DEVICE card_handle);
static T_BOOL sd_get_capacity(T_pSD_DEVICE drv_card);
static T_BOOL sd_get_scr(T_pSD_DEVICE drv_card, T_U8 *scr);
static T_U8   sd_get_spec_vers(T_pSD_DEVICE drv_card);

static T_BOOL sd_change_mode(T_pSD_DEVICE drv_card, T_BOOL speed_mode);

static T_S32 stuff_bits (T_U16 * resp, T_S32 start, T_S32 size)
{
    T_U16 __size = size;
    T_U16 __mask = (__size < 16 ? 1 << __size : 0) - 1;
    T_S32 __off = ((start) / 16);
    T_S32 __shft = (start) & 15;
    T_U16 __res = 0;

    __res = resp [__off] >> __shft;
    if (__size + __shft > 16)
    {
            __res = resp [__off] >> __shft;
            __res |= resp[__off+1] << ((16 - __shft) % 16);
    }

    return (__res & __mask);
}

/**
 * @brief Wait read or write complete
 *
 * Called when read or write sd card finish
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_BOOL
 */
static T_BOOL wait_rw_complete(T_pSD_DEVICE drv_card)
{
    T_U32 sd_stat, tmp = 0;

    #define RETRY_TIMES1   (100000)

    while(RETRY_TIMES1 > tmp++)
    {
        if (sd_get_card_status(drv_card->ulRCA, &sd_stat))
        {
            if ( (sd_stat & (1<<8)) 
                && (((sd_stat & SD_CURRENT_STATE_MASK) >> SD_CURRENT_STATE_OFFSET) == SD_CURRENT_STATE_TRAN))
            {
                break;
            }
        }
        else
        {
            drv_print("The get SD card status is error22.", 0, AK_TRUE);
            return AK_FALSE;
        }
		delay_us(50);
    }
    if (tmp >= RETRY_TIMES1)
    {
        drv_print("get sd card status time out, status=0x", sd_stat, AK_TRUE);
        return AK_FALSE;
    }

    return AK_TRUE;
}


/**
 * @brief get sd mmc card  capacity
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_VOID
 */
static T_BOOL sd_get_capacity(T_pSD_DEVICE drv_card)
{
    T_U32 mult=0,c_size=0,blocknr=0;

    //capacity part 
    if ((CARD_SD == drv_card->enmCardType) && drv_card->bHighCapacity)
    {
    	c_size = ((drv_card->ulCSD[2]<<16) &0x3f0000)+(drv_card->ulCSD[1] >> 16);
		drv_card->ulCapacity = (c_size + 1) << 10;
    }
    else if((CARD_MMC == drv_card->enmCardType) && drv_card->bHighCapacity)
    {
        // > mmc 4.0
        if (4 <= drv_card->ucSpecVersion)
        {
            if (!mmc_get_extcsd(drv_card))
            {
                return AK_FALSE;
            }
        }
        else
        {
            drv_card->ulCapacity = MMC_C_SIZE(drv_card->ulCSD) << 10;
        }
    }
    else
    {
        mult = 1 << (((drv_card->ulCSD[1]>>15)&0x7) + 2);
        c_size = ((drv_card->ulCSD[1]>>30)&0x3) + ((drv_card->ulCSD[2]&0x3ff)<< 2);
        blocknr = ( c_size + 1 ) * mult;
        //capacity is the num of 512bytes
        drv_card->ulCapacity = blocknr * (drv_card->ulMaxReadBlockLen >> 9);
    }
    
    return AK_TRUE;
}

/**
 * @brief Set sd card clock.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_VOID
 */
static T_VOID set_trans_clk(T_VOID)
{
    set_clock(SD_TRANSFER_MODE_CLK, clk_get_asic(), SD_POWER_SAVE_ENABLE); //20Mhz
}


/**
* @brief Read block from mmc or sd card with L2 buffer + DMA mode
* @author Huang Xin
* @date 2010-07-14
* @param block_src[in] the address of block to be selected to read 
* @param databuf[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will read from card
* @param mode[in] single block or multi block
* @return T_BOOL
* @retval  AK_TRUE: read  successfully
* @retval  AK_FALSE: read failed
*/
static T_BOOL sd_rw_block(T_pSD_DEVICE drv_card, T_U32 blk_num, T_U8 *databuf, T_U32 size, T_U32 mode,T_U8 dir)
{
    T_BOOL ret = AK_FALSE;
    T_U32 cmd;

    //Data address is in byte units in a standard capacity sd card and in block(512byte) units in a high capacity sd card
    if (!drv_card->bHighCapacity)        //not HC card
    {
        blk_num <<= 9;
    }

    if(SD_DATA_MODE_SINGLE == mode)
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = SD_CMD(17);
        else
            cmd = SD_CMD(24);
    }
    else
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = SD_CMD(18);
        else
            cmd = SD_CMD(25);
    }
    
    //step1: check bus busy
    if (sd_trans_busy())
    {
        return AK_FALSE;
    }
    
    //step2: send card command
    if ((cmd == SD_CMD(25)) && (CARD_SD == drv_card->enmCardType))
    {
        if (!send_acmd(drv_card->ulRCA, 23, SD_SHORT_RESPONSE, size / drv_card->ulDataBlockLen ))
        {
            ret = AK_FALSE;
            goto EXIT;
        }
    }
    
    if (!send_cmd(cmd, SD_SHORT_RESPONSE, blk_num ))
    {
        drv_print("block rw command is failed! cmd:", cmd, AK_TRUE);
        ret = AK_FALSE;
        goto EXIT;
    }

    //step3: transfer data
    if (SD_USE_L2 && (0 == ((T_U32)databuf & 3)))
    {
        ret = sd_trans_data_dma(drv_card, (T_U32)databuf, size, dir);
    }
    else
    {
        if (SD_DATA_CTL_TO_HOST == dir)
        {
            ret = sd_rx_data_cpu(drv_card, databuf, size);
        }
        else
        {
            ret = sd_tx_data_cpu(drv_card, databuf, size);
        }
    }
	
	delay_us(20);

    //step4: send cmd12 if multi-block operation
    if ((SD_DATA_MODE_SINGLE != mode)
		|| (AK_FALSE == ret))
    {
        if (!send_cmd( SD_CMD(12), SD_SHORT_RESPONSE, 0 ))
        {
        
            ret = AK_FALSE;
        }
    }

    //step5: wait card status to idle
    if (!wait_rw_complete(drv_card))
    {       
		ret = AK_FALSE;	 
    }

EXIT:
    return ret;
}

/**
 * @brief Negotiation of the mmc card  voltage
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSD_STATUS
 */
static T_U8 mmc_nego_volt(T_pSD_DEVICE drv_card, T_U32 volt)
{
    T_U32 response = 0;
    T_U32 i=0;

    #define RETRY_TIMES2   (10000)
    do
    {
        if (send_cmd(SD_CMD(1), SD_SHORT_RESPONSE, volt)) 
        {
            get_short_resp(&response);
        }
        else
        {
            return SD_NEGO_FAIL;
        }
    }while((!(response & SD_STATUS_POWERUP))&& (RETRY_TIMES2 > i++));

    if(RETRY_TIMES2 <= i)
    {
        drv_print("mmc nego time out!", 0, AK_TRUE);
        return SD_NEGO_TIMEOUT;
    }
    
    //mmc4.2, a definition for implementation of media higher than 2GB was introduced
    if (response&SD_CCS)
    {
        drv_card->bHighCapacity = 1;
    }
    else
    {
        drv_card->bHighCapacity = 0;
    }
    
    return SD_NEGO_SUCCESS;
}

/**
 * @brief Get the sd card  cid register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cid[out] The buffer to save card cid.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_BOOL sd_get_cid(T_U32 *cid)
{
    if (send_cmd(SD_CMD(2), SD_LONG_RESPONSE, SD_NO_ARGUMENT ))
    {
        if (AK_NULL != cid)
            get_long_resp(cid);
        return AK_TRUE;
    }
    return AK_FALSE;
}

/**
 * @brief Get the sd card  csd register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param csd[out] The buffer to save card csd.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_BOOL sd_get_csd(T_U32 rca, T_U32 *csd)
{
    if (send_cmd(SD_CMD(9), SD_LONG_RESPONSE, (rca << 16)))
    {
        get_long_resp(csd);
        return AK_TRUE;
    }
    return AK_FALSE;
}

/**
 * @brief Get the mmc4.0 card  ext csd register
 *
 * Called when init mmc4.0 card
 * @author Huang Xin
 * @date 2010-07-14
 * @param ext_csd[out] The buffer to save mmc4.0 ext csd.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_BOOL mmc_get_extcsd(T_pSD_DEVICE card_handle)
{
    T_BOOL ret = AK_FALSE;
#ifndef UNSUPPORT_REMAP
    T_U32 *ulExtCSD = (T_U32 *)remap_alloc(4096);
#else
    T_U32 ulExtCSD[128];
#endif
    //step1: check bus busy
    if (sd_trans_busy())
    {
        goto exit;
    }

    //step2: send card command
    if (!send_cmd(SD_CMD(8), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))
    {
        goto exit;
    }
    
    //step3: transfer data //hesdmmc,这里如果不接收512会有问题吗?
    ret = sd_trans_data_dma(card_handle, (T_U32)ulExtCSD,512,SD_DATA_CTL_TO_HOST);

    //step4: wait card status to idle
    if (!wait_rw_complete(card_handle))
    {
         ret = AK_FALSE;
    }

    card_handle->ulCapacity = MMC4_SECTOR_CNT(ulExtCSD);

exit:
    
#ifndef UNSUPPORT_REMAP
    if (ulExtCSD)
        remap_free((T_pVOID)ulExtCSD);
#endif
    return ret;
}

/**
 * @brief send sd acmd.
 *
 * ALL the ACMDS shall be preceded with APP_CMD command cmd55
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_BOOL
 * @retval  AK_TRUE: ACMD sent successfully
 * @retval  AK_FALSE: ACMD sent failed
 */
static T_BOOL send_acmd(T_U32 rca, T_U8 cmd_index, T_U8 resp, T_U32 arg)
{
    send_cmd(SD_CMD(55), SD_SHORT_RESPONSE, rca << 16);
    return (send_cmd(cmd_index, resp, arg));
}

/**
 * @brief Set sd card block length.
 *
 * Usually set the block length  512 bytes .
 * @author Huang Xin
 * @date 2010-07-14
 * @param block_len[in] The block length.
 * @return T_BOOL
 * @retval  AK_TRUE: set successfully
 * @retval  AK_FALSE: set failed
 */
static T_BOOL sd_set_block_len(T_U32 block_len)
{
    if (send_cmd(SD_CMD(16), SD_SHORT_RESPONSE, block_len))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}


/**
 * @brief Set sd high speed mode to change card clk.
 *
 * Usually set the speed_mode  high speed mode or default mode.
 * @author Huang Xueying
 * @date 2010-07-14
 * @param speed_mode[in]:high speed mode:1, default mode:0.
 * @return T_BOOL
 * @retval  AK_TRUE: set successfully
 * @retval  AK_FALSE: set failed
 */
static T_BOOL sd_change_mode(T_pSD_DEVICE drv_card, T_BOOL speed_mode)
{
    T_U32 arg_value = 0;
    T_BOOL ret = AK_FALSE;

	if (0x4 <= drv_card->ucSpecVersion)
    {
        //mmc4.x set power class,ignored
        //mmc4.x set high speed mode
		if((DEFAULT_SPEED_MODE == speed_mode)&&(HIGH_SPEED_MODE == drv_card->bSpeedmode))
		{
			set_clock(350000, clk_get_asic(), SD_POWER_SAVE_ENABLE); //from high speed mode to low,the clock should be changed to low first
		}

		arg_value = (0x3<<24)|(185<<16)|(speed_mode<<8);
		ret = send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value);

		if (ret)
        {
            if (!wait_rw_complete(drv_card))
            {
                return AK_FALSE;
            }
            drv_card->bSpeedmode = speed_mode;
			ret = AK_TRUE;
        }
        else
        {
            ret = AK_FALSE;
        }
    }
    else
    {
        drv_card->bSpeedmode = DEFAULT_SPEED_MODE;
		ret = AK_FALSE;
    }

	return ret;

}

/**
 * @brief Set sd card bus width.
 *
 * Usually set the bus width  1 bit or 4 bit.
 * @author Huang Xin
 * @date 2010-07-14
 * @param wide_bus[in] The bus width.
 * @return T_BOOL
 * @retval  AK_TRUE: set successfully
 * @retval  AK_FALSE: set failed
 */
static T_BOOL sd_set_bus_width(T_pSD_DEVICE drv_card, T_U8 wide_bus)
{
    T_U32 arg_value = 0;
    T_BOOL ret = AK_TRUE;

    switch (drv_card->enmCardType)
    {
        case CARD_MMC:
            //mmc 4.x
            if (0x4 <= drv_card->ucSpecVersion)
            {
                //mmc4.x set power class,ignored
                //mmc4.x set bus width
                arg_value = (0x3<<24)|(183<<16)|(wide_bus<<8);
                if (send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value))
                {
                    if (!wait_rw_complete(drv_card))
                    {
                        ret = AK_FALSE;
                    }
                    drv_card->enmBusMode = wide_bus;
                }
                else
                {
                    ret = AK_FALSE;
                }
            }
            else
            {
                drv_card->enmBusMode = USE_ONE_BUS;
            }
            break;
        case CARD_SD:
        case CARD_COMBO:
            if (USE_ONE_BUS == wide_bus)
                arg_value = SD_BUS_WIDTH_1BIT;
            else if (USE_FOUR_BUS == wide_bus)
                arg_value = SD_BUS_WIDTH_4BIT;
            else //sd is not allowed to use 8bit bus
            {
                drv_print("sd is not allowed to use 8bit bus", 0, AK_TRUE);
                return AK_FALSE;
            }
            if (send_acmd(drv_card->ulRCA, SD_CMD(6), SD_SHORT_RESPONSE, arg_value))
            {
                drv_card->enmBusMode = wide_bus;
            }
            else
            {
                ret = AK_FALSE;
            }
            break;
        default:
            ret = AK_FALSE;
            break;
    }
    return ret;
}


/**
 * @brief Get the sd card status register
 *
 * Called when wait read or write complete
 * @author Huang Xin
 * @date 2010-07-14
 * @param status_buf[out] The buffer to save card status .
 * @return T_BOOL
 * @retval AK_TRUE: get successfully
 * @retval AK_FALSE: get failed
 */
static T_BOOL sd_get_card_status(T_U32 rca, T_U32 *status_buf)
{
    if (send_cmd(SD_CMD(13), SD_SHORT_RESPONSE, (rca << 16)))
    {
        get_short_resp(status_buf);
        return  AK_TRUE;
    }
    else
        return AK_FALSE;
}

/**
 * @brief Get the sd card  ocr register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param ocr[out] The buffer to save card ocr.
 * @return T_U8
 * @retval T_eSD_STATUS
 */
static T_U8 sd_get_ocr(T_U32 rca, T_U32 *ocr)
{
    T_U32  response = 0;
    T_U8   fun_num = 0;

    if (send_acmd(rca, SD_CMD(41), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))
    {
        get_short_resp(&response);
        if(0 == (response & SD_DEFAULT_VOLTAGE))
            return SD_GET_OCR_INVALID;//b_process
        *ocr = response & SD_OCR_MASK;
        return SD_GET_OCR_VALID;
    }
    else
        return SD_GET_OCR_FAIL;
}

/**
 * @brief Get the sd card  scr register
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param scr[out] The buffer to save card scr.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_BOOL sd_get_scr(T_pSD_DEVICE drv_card, T_U8 *scr)
{
    T_U8   i = 0;
    T_U8   scr_tmp[8] = {0};
    T_BOOL ret = AK_TRUE;

    drv_card->ulDataBlockLen = 8;
    
    //step1: check bus busy
    if(sd_trans_busy())
    {
        ret = AK_FALSE;
        goto exit;
    }
    
    //step2: send card command
    if(!send_acmd(drv_card->ulRCA, SD_CMD(51), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))
    {
        ret = AK_FALSE;
        goto exit;
    }
    
    //step3: transfer data
    ret = sd_rx_data_cpu(drv_card, scr_tmp, 8);

    //step4: wait card status to idle
    if (!wait_rw_complete(drv_card))
    {
        ret = AK_FALSE;
    }
    
    //get scr use bit mode,so exchange MSB with LSB
    for(i = 8; i > 0; i--)
    {
        scr[i-1] = scr_tmp[8-i];
    }
    
exit:
    //resume the default block len
    drv_card->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;
    return ret;
}


/**
 * @brief Get the sd mmc  card  spec vers
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param scr[out] The buffer to save card scr.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_U8 sd_get_spec_vers(T_pSD_DEVICE drv_card)
{
    T_U8 scr[8] = {0};
    T_U8 spec_vers = SD_MMC_INVALID_SPEC_VERSION;

    switch (drv_card->enmCardType)
    {
        case CARD_MMC:
            spec_vers = MMC_SPEC_VERSION(drv_card->ulCSD);
            break;
        case CARD_SD:
        case CARD_COMBO:
            if(sd_get_scr(drv_card, scr))
            {
                spec_vers = scr[7]&0x0f;
            }
            break;
        default:
            break;
    }
    return spec_vers;
}

/**
 * @brief Negotiation of the sd card  voltage
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSD_STATUS
 */
static T_U8 sd_nego_volt(T_pSD_DEVICE drv_card, T_U32 volt)
{
    T_U32 response = 0;
    T_U32 i=0;

    #define RETRY_TIMES3     (10000)

    do
    {
        if (send_acmd(drv_card->ulRCA, SD_CMD(41), SD_SHORT_RESPONSE, volt))
        {
            get_short_resp(&response);
        }
        else
        {
            return SD_NEGO_FAIL;
        }
    }while((!(response & SD_STATUS_POWERUP))&& (RETRY_TIMES3 > i++));

    if(RETRY_TIMES3 <= i)
    {
        drv_print("sd nego time out!", 0, AK_TRUE);
        return SD_NEGO_TIMEOUT;
    }
    
    if(response&SD_CCS)
    {
        drv_card->bHighCapacity = 1;
    }
    else
    {
        drv_card->bHighCapacity = 0;
    }
    return SD_NEGO_SUCCESS;
}


/**
* @brief initial mmc sd or comob card
* @author Huang Xin
* @date 2010-06-17
* @param cif[in] card interface selected
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sd_initial(T_eCARD_INTERFACE cif, T_eBUS_MODE bus_mode)
{
    T_pSD_DEVICE pSdCard;

    //check param
    
#if (CHIP_SEL_10C > 0)

    if(INTERFACE_MMC1 == cif)
    {
        bus_mode = USE_ONE_BUS;
    }

#endif

    if((INTERFACE_SD2 == cif) && (bus_mode > USE_FOUR_BUS))
    {
        bus_mode = USE_FOUR_BUS;
    }

    pSdCard = get_sd_device(cif);
    memset(pSdCard, 0, sizeof(T_SD_DEVICE));
    
    pSdCard->enmInterface = cif;

    SD_DRV_PROTECT(cif, bus_mode);

    if(cif == INTERFACE_MMC1)
    {
        sys_module_reset(eVME_MCI1_CLK);
    }
    else
    {
        sys_module_reset(eVME_MCI2_CLK);
    }
    //set indentification working clock which must be less than 400K
    set_clock(SD_IDENTIFICATION_MODE_CLK, clk_get_asic(), SD_POWER_SAVE_ENABLE);
    pSdCard->enmCardType = init_card(pSdCard);
    if (CARD_UNUSABLE == pSdCard->enmCardType)
    {
        drv_print("get type fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }

    if (!sd_get_csd(pSdCard->ulRCA, pSdCard->ulCSD))
    {
        drv_print("get csd fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }

//hxy:some SD CARD such as SANDISK 4GB, need to set SD-CLK to 20M before get the version
//while some EMMC such as SAMSUNG VER4.41, they need to set SD-CLK to high speed after get the version.
	if( CARD_MMC !=pSdCard->enmCardType)
    	set_trans_clk();
	
    if (!select_card(pSdCard->ulRCA))
    {
        drv_print("select card fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }
    
    if(sd_set_block_len(SD_DEFAULT_BLOCK_LEN))
    {
        pSdCard->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;
        
    }
    else
    {
        drv_print("set blk len fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }

    pSdCard->ucSpecVersion = sd_get_spec_vers(pSdCard);
    if(SD_MMC_INVALID_SPEC_VERSION == pSdCard->ucSpecVersion)
    {
        drv_print("get vers fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }
    
	//hxy: some EMMC change mode(CMD 6 must sent after CMD7) and change clk should after get the card version.	
	//set_trans_clk();
	if( CARD_MMC ==pSdCard->enmCardType)	
		sd_change_clock((T_pCARD_HANDLE)pSdCard,SD_TRANSFER_MODE_CLK);
	
    if(!sd_set_bus_width(pSdCard, bus_mode))
    {
        drv_print("set bus width fail", 0, AK_TRUE);
        goto ERR_EXIT;
    }

    pSdCard->ulMaxReadBlockLen = 1 << ((pSdCard->ulCSD[2]>>16)&0xf);

    if(!sd_get_capacity(pSdCard))
        goto ERR_EXIT;

#if 1
    drv_print("init finish, the card info:", 0, AK_TRUE);
    drv_print((pSdCard->enmInterface==0)? "infterface MMC1" : "infterface SD2", 0, AK_TRUE);
    drv_print("type(0-F;1-MMC;2-Sd;3-SDIO;4-COMBO):", pSdCard->enmCardType, AK_TRUE);
    drv_print((pSdCard->bHighCapacity==1)? "is High Capacity Card":"No High Capacity Card", 0, AK_TRUE);
    drv_print("Capacity(block number):", pSdCard->ulCapacity, AK_TRUE);
    drv_print("data block len:", pSdCard->ulDataBlockLen, AK_TRUE);
    drv_print("addr(RCA): ", pSdCard->ulRCA, AK_TRUE);
    drv_print("Spec Version: ", pSdCard->ucSpecVersion, AK_TRUE);

#endif

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD,pSdCard->enmBusMode);

    return (T_pCARD_HANDLE)pSdCard;

ERR_EXIT:
    drv_print("sd init fail", 0, AK_TRUE);
    SD_DRV_UNPROTECT(INTERFACE_NOT_SD,pSdCard->enmBusMode);
    return AK_NULL;
}

/**
 * @brief read data from sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_src[in] source block to read
 * @param databuf[out] data buffer to read
 * @param block_count[in] size of blocks to be readed
 * @return T_BOOL
 * @retval AK_TRUE: read successfully
 * @retval AK_FALSE: read failed
 */
T_BOOL sd_read_block(T_pCARD_HANDLE handle, T_U32 block_src,
                         T_U8 *databuf, T_U32 block_count)
{
    T_BOOL ret = 0;
    T_U8  retry =0;
    T_U32 read_mode;
    T_U32 read_size;
    T_U32 block_left;
    T_U32 block_to_read;

        
    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface,
        ((T_pSD_DEVICE)handle)->enmBusMode);
	
	block_left = block_count;

	
    while (0 < block_left)
    {
        //In order to improve sd read or write speed, trans 64k one time
        //use single block mode when the transfer one block
        if (SD_DMA_BLOCK_64K <= block_left)
        {
            block_to_read = SD_DMA_BLOCK_64K;
        }
        else
        {
            block_to_read = block_left;
        }
        read_mode = (block_to_read > 1) ? SD_DATA_MODE_MULTI : SD_DATA_MODE_SINGLE;
        read_size =  block_to_read * ((T_pSD_DEVICE)handle)->ulDataBlockLen;
        
        for (retry = 0; retry < 3; retry++)
        {
            ret = sd_rw_block((T_pSD_DEVICE)handle, block_src, (T_U8 *)databuf, read_size, 
                              read_mode, SD_DATA_CTL_TO_HOST);
            if (ret)
            {
                break;
            }
            else
            {
            
                //reset sd controller
                if (INTERFACE_SD2 == ((T_pSD_DEVICE)handle)->enmInterface)
                {
                    sys_module_reset(eVME_MCI2_CLK);
                }
                else
                {
                    sys_module_reset(eVME_MCI1_CLK);
                }

				if( CARD_MMC !=((T_pSD_DEVICE)handle)->enmCardType)	
					set_trans_clk();
                else
					sd_change_clock(handle,SD_TRANSFER_MODE_CLK);
            }

        }
        
        if (3 <= retry)
        {
            drv_print("read error, src = 0x", block_src, AK_FALSE);
            drv_print("block_cnt = 0x", block_to_read, AK_TRUE);
            SD_DRV_UNPROTECT(INTERFACE_NOT_SD,((T_pSD_DEVICE)handle)->enmBusMode);
            return ret;
        }
        
        block_left -= block_to_read;
        block_src += block_to_read;
        databuf += read_size;
    };

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD,((T_pSD_DEVICE)handle)->enmBusMode);
    return ret;
}

/**
 * @brief write data to sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_dest[in] destation block to write
 * @param databuf[in] data buffer to write
 * @param block_count[in] size of blocks to be written
 * @return T_BOOL
 * @retval  AK_TRUE:write successfully
 * @retval  AK_FALSE: write failed
 */
T_BOOL sd_write_block(T_pCARD_HANDLE handle,T_U32 block_dest, 
                         const T_U8 *databuf, T_U32 block_count)
{
    T_BOOL ret = 0;
    T_U8 retry = 0;
    T_U32 write_mode;
    T_U32 write_size;
    T_U32 block_left;
    T_U32 block_to_write;

    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface,
                   ((T_pSD_DEVICE)handle)->enmBusMode);

    block_left = block_count;
    while (0 < block_left)
    {
        //In order to improve sd read or write speed,trans 64k one time
        //use  single  block mode when the transfer one block
        if (SD_DMA_BLOCK_64K <= block_left)
        {
            block_to_write = SD_DMA_BLOCK_64K;
        }
        else
        {
            block_to_write = block_left;
        }
        write_mode = (block_to_write > 1) ? SD_DATA_MODE_MULTI : SD_DATA_MODE_SINGLE;
        write_size =  block_to_write * ((T_pSD_DEVICE)handle)->ulDataBlockLen;

        for (retry = 0; retry < 3; retry++)
        {
            ret = sd_rw_block((T_pSD_DEVICE)handle, block_dest, (T_U8*)databuf, write_size, 
                              write_mode,SD_DATA_CTL_TO_CARD);
            if (ret)
            {
                break;
            }
            else
            {
                //reset sd controller
                if (INTERFACE_SD2 == ((T_pSD_DEVICE)handle)->enmInterface)
                {
                    sys_module_reset(eVME_MCI2_CLK);
                }
                else
                {
                    sys_module_reset(eVME_MCI1_CLK);
                }

                set_trans_clk();
            }
        }
        
        if (3 <= retry)
        {
            drv_print("write error,dest = 0x", block_dest, AK_FALSE);
            drv_print("block_cnt = 0x", block_to_write, AK_TRUE);
            SD_DRV_UNPROTECT(INTERFACE_NOT_SD,((T_pSD_DEVICE)handle)->enmBusMode);
            return ret;
        }
        
        block_left -= block_to_write;
        block_dest += block_to_write;
        databuf += write_size;
    };

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD,((T_pSD_DEVICE)handle)->enmBusMode);
    return ret;
}





/**
 * @brief : change the sd clk freely 
 * @author Huang Xueying
 * @date 2013-08-27
 * @param handle[in] : card handle

 * @param clock[in]: suggest value of HIGH SPEED MODE : >=20MHz(20~23MHz,29~57MHz ), 
 * @                         <20MHz(350KHz, 500KHz, 1.5MHz, 2MHz, 4MHz, 5MHz, 8MHz,9MHz)
 * @suggest value of DEFAULT SPEED MODE : 
 * @                         <20MHz(350KHz, 500KHz, 1.5MHz, 2MHz, 4MHz, 5MHz, 8MHz,9MHz,13MHz)
 * @
 * @ return T_BOOL
 * @retval  AK_TRUE:set successfully
 * @retval  AK_FALSE: set failed
 */
T_BOOL sd_change_clock(T_pCARD_HANDLE handle, T_U32 clock)
{

	T_U32 asic_freq = 0;


	if(clock>58*1000*1000)
	{		
        drv_print("clock is over range,sd_change_clock failed!", 0, AK_TRUE);
		return AK_FALSE;
	}

	else if(SD_INIT_VALID == ((T_pSD_DEVICE)handle)->bInitMemSuccess)
    {
		drv_print("MMC is not initialed, change_clock must after init!!", 0, AK_TRUE);
        return AK_FALSE;
    }
	
	else if(SD_MMC_INVALID_SPEC_VERSION == ((T_pSD_DEVICE)handle)->ucSpecVersion)	
	{
        drv_print("MMC ver is not valid, change_clock must after sd version is got !", 0, AK_TRUE);
        return AK_FALSE;
		
	}
	else
	{
		asic_freq = clk_get_asic();

		if((DEFAULT_SPEED_MODE == ((T_pSD_DEVICE)handle)->bSpeedmode)&&(clock>=20*1000*1000))
		{				
			sd_change_mode((T_pSD_DEVICE)handle, HIGH_SPEED_MODE);
		}
		else if((HIGH_SPEED_MODE == ((T_pSD_DEVICE)handle)->bSpeedmode)&&(clock<20*1000*1000))
		{				
			sd_change_mode((T_pSD_DEVICE)handle, DEFAULT_SPEED_MODE);
		}
		
		set_clock(clock, asic_freq, SD_POWER_SAVE_ENABLE); 	
        drv_print("sd_change_clock success!!!!!", 0, AK_TRUE);
		
		return AK_TRUE;	
	}
}


/**
 * @brief power off sd controller
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @return T_VOID
 */
T_VOID sd_free(T_pCARD_HANDLE handle)
{   
    if (AK_NULL == handle)
    {
        drv_print("The SD card handle is null", 0, AK_TRUE);
        return;
    }
}

/**
 * @brief get sd card information
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param total_block[out] current sd's total block number
 * @param block_size[out] current sd's block size
 * @a block = 512 bytes
 * @return T_VOID
 */
T_VOID sd_get_info(T_pCARD_HANDLE handle, T_U32 *total_block, T_U32 *block_size)
{
    *total_block = ((T_pSD_DEVICE)handle)->ulCapacity;
    *block_size = ((T_pSD_DEVICE)handle)->ulDataBlockLen;
}



T_VOID sd_on_change(T_U32 asic)
{
    set_clock(SD_TRANSFER_MODE_CLK, asic, SD_POWER_SAVE_ENABLE); //20Mhz
}

/**
 * @brief Init the mem partion 
 *
 * Called when init card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
T_U8 init_mem(T_pSD_DEVICE drv_card)
{
    T_U32 status,resp,ocr;

    status = sd_get_ocr(drv_card->ulRCA, &ocr);
    if (SD_GET_OCR_FAIL == status) //init mmc card
    {
        status = mmc_nego_volt(drv_card, SD_HCS|SD_DEFAULT_VOLTAGE);
        if (SD_NEGO_FAIL== status)
        {
            return COMMON_SD_INIT_FAIL;
        }
        else if (SD_NEGO_TIMEOUT == status)
        {
            return COMMON_SD_INIT_MEM_FAIL;
        }
        else if(SD_NEGO_SUCCESS== status)
        {
            if(sd_get_cid(AK_NULL))
            {
            	drv_card->bInitMemSuccess = SD_INIT_MMC;
                return COMMON_SD_INIT_MEM_SUCCESS;
            }
            else
            {
                return COMMON_SD_INIT_FAIL;
            }
        }
    }
    else if (SD_GET_OCR_VALID == status) //init sd card
    {
        status = sd_nego_volt(drv_card, SD_HCS | (ocr & SD_DEFAULT_VOLTAGE));
        if (SD_NEGO_FAIL == status)
        {
            return COMMON_SD_INIT_FAIL;
        }
        if (SD_NEGO_TIMEOUT == status)
        {
            return COMMON_SD_INIT_MEM_FAIL;
        }
        if(SD_NEGO_SUCCESS == status)
        {
            drv_card->bInitMemSuccess = SD_INIT_SD;
            if(sd_get_cid(AK_NULL))
            {
                return COMMON_SD_INIT_MEM_SUCCESS;
            }
            else
            {
                return COMMON_SD_INIT_FAIL;
            }
        }
    }
    else //init fail SD_GET_OCR_INVALID
    {
        return COMMON_SD_INIT_MEM_FAIL;
    }

    return COMMON_SD_INIT_MEM_ERROR;
}


