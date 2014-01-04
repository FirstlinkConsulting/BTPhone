/**
 * @file    arch_sd.c
 * @brief   Implement arch level operations of how to control sd&sdio.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_common_sd.h"
#include "l2.h"
#include "share_pin.h"
#include "clk.h"
#include "drv_api.h"
#include "arch_init.h"


extern T_U32               g_clk_map;
static T_U32               s_SdReg_Base  = MCI1_MODULE_BASE_ADDR; // sd register base addr
static DEVICE_SELECT       s_L2Select    = ADDR_MCI1;
static T_eSHARE_PIN_CFG    s_PinSelect   = ePIN_AS_MCI1;

static T_SD_DEVICE s_sd_device[2] = 
{
    {INTERFACE_NOT_SD, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}},
    {INTERFACE_NOT_SD, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}}
};

static T_VOID set_data_reg(T_U32 len,T_U32 blk_size,T_U8 bus_mode,T_U8 dir);
static T_VOID sd_cfg_buf(T_U8 buf_mode, T_U32 buff_addr, T_U32 buf_len);

/**
 * @brief Get the sd device.
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return T_pVOID
 */
T_pVOID get_sd_device(T_eCARD_INTERFACE cif)
{
    return &s_sd_device[cif];
}

static T_VOID sd_cfg_buf(T_U8 buf_mode, T_U32 buff_addr, T_U32 buf_len)
{
    T_U32 reg_value = 0;

    if (L2_DMA_MODE == buf_mode)
    {
       reg_value = BUF_EN | DMA_EN | (0 << START_ADDR_OFFSET)
                   | (buf_len << BUF_SIZE_OFFSET);
    }
    else
    {
       reg_value = BUF_EN | (0 << START_ADDR_OFFSET) 
                   | (buf_len << BUF_SIZE_OFFSET);
    }

    REG32(s_SdReg_Base + oSD_DMA_MODE) = reg_value;
}
/**
 * @brief set sd controller data register.
 *
 * Set timeout value,transfer size,transfer direction,bus mode,data block len
 * @author Huang Xin
 * @date 2010-07-14
 * @param len[in] Transfer size
 * @param blk_size[in] Block length
 * @param dir[in] transfer direction
 * @return T_VOID
 */
static T_VOID set_data_reg(T_U32 len, T_U32 blk_size, T_U8 bus_mode, T_U8 dir)
{
    T_U32 reg_value;

    REG32(s_SdReg_Base + oSD_DATA_TMR) = SD_DAT_MAX_TIMER_V*2;
    REG32(s_SdReg_Base + oSD_DATA_LEN) = len;

    reg_value = SD_DATA_CTL_ENABLE | ( dir << SD_DATA_CTL_DIRECTION_OFFSET ) \
                | (bus_mode << SD_DATA_CTL_BUS_MODE_OFFSET) \
                | (blk_size << SD_DATA_CTL_BLOCK_LEN_OFFSET );

    REG32(s_SdReg_Base + oSD_DATA_CTRL) = reg_value;
}

#pragma arm section code = "_sd_common_"

/**
 * @brief Set the sd interface.
 *
 * Select the sd interface(INTERFACE_MMC1 or INTERFACE_SD2)and select the relevant registers,L2 ,pin.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_MMC1 or INTERFACE_SD2.
 * @return T_VOID
 */
T_VOID set_interface(T_eCARD_INTERFACE cif, T_U8 bus_mode)
{
    if(INTERFACE_NOT_SD != cif)
    {
    #if (DRV_SUPPORT_SPI_BOOT == 0)  
        g_clk_map |= CLK_MCI_EN;
    #endif    
        store_int(INT_EN_L2);

        //select pin share
    #if (CHIP_SEL_10C > 0)
        if(INTERFACE_SD2 == cif)
        {
            if(USE_ONE_BUS == bus_mode){
                s_PinSelect = ePIN_AS_MCI2_1LINE;
            }
            else{
                s_PinSelect = ePIN_AS_MCI2_4LINE;
            }
        }
        else
        {
            s_PinSelect = ePIN_AS_MCI1;
        }
    #else
        if(INTERFACE_SD2 == cif)
        {
            s_PinSelect = ePIN_AS_MCI2;
        }
        else
        {
            if (USE_EIGHT_BUS == bus_mode){
                s_PinSelect = ePIN_AS_MCI1_8LINE;
            }
            else{
                s_PinSelect = ePIN_AS_MCI1;
            }
        }
    #endif
    
        if(INTERFACE_SD2 == cif)
        {
            s_SdReg_Base    = MCI2_MODULE_BASE_ADDR;
            s_L2Select      = ADDR_MCI2;
            sys_module_enable(eVME_MCI2_CLK, AK_TRUE);
        }
        else
        {
            s_SdReg_Base    = MCI1_MODULE_BASE_ADDR;
            s_L2Select      = ADDR_MCI1;
            sys_module_enable(eVME_MCI1_CLK, AK_TRUE);
        }
        
        sys_share_pin_lock(s_PinSelect);
    }
    else
    {
        sys_module_enable(eVME_MCI1_CLK, AK_FALSE);
        sys_module_enable(eVME_MCI2_CLK, AK_FALSE);
        sys_share_pin_unlock(s_PinSelect);
        restore_int();
        #if (DRV_SUPPORT_SPI_BOOT == 0)  
        g_clk_map &= ~CLK_MCI_EN;
        #endif
    }
}

#pragma arm section code


/**
 * @brief Set sd card clock.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param sd_clk[in] The main clock for sd card.
 * @param asic_freq[in] The asic clock (unit,hz)
 * @param pwr_save[in] Set this parameter true to enable power save
 * @return T_VOID
 */
T_VOID set_clock(T_U32 sd_clk, T_U32 asic_freq, T_BOOL pwr_save)
{
    T_U8 clk_div_l, clk_div_h;
    T_U32 reg_value, tmp;
    vT_ModuleList module;

    if(0 == sd_clk)
    {
        reg_value = 0;
    }
    else
    {

        if (asic_freq < sd_clk * 2)
        {
            clk_div_l = clk_div_h = 0;
        }
        else
        {
            // clk = asic / ((clk_div_h+1) + (clk_div_l+1))
            //NOTE:clk_div_h and clk_div_l present high and low level cycle time
            tmp = asic_freq / sd_clk;
            if (asic_freq % sd_clk)
                tmp += 1;
            tmp -= 2;
            clk_div_h = tmp / 2;
            clk_div_l = tmp - clk_div_h;
        }
        reg_value = (clk_div_l<<CLK_DIV_L_OFFSET) | (clk_div_h<<CLK_DIV_H_OFFSET)
                    | SD_CLK_ENABLE | FALLING_TRIGGER | SD_INTERFACE_ENABLE;
        
        if (pwr_save)
            reg_value |= PWR_SAVE_ENABLE;
    }
    #if (DRV_SUPPORT_SPI_BOOT == 0)  
    if (0 == (g_clk_map & CLK_MCI_EN))
    {
        if (MCI1_MODULE_BASE_ADDR == s_SdReg_Base)
            module = eVME_MCI1_CLK;
        else
            module = eVME_MCI2_CLK;
            
        sys_module_enable(module, AK_TRUE);
    }
    #endif
    REG32(s_SdReg_Base + oSD_CLK_CTRL) = reg_value;
    #if (DRV_SUPPORT_SPI_BOOT == 0)  
    if (0 == (g_clk_map & CLK_MCI_EN))    
    {
        sys_module_enable(module, AK_FALSE);
    }
    #endif
}

/**
 * @brief send sd command.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_BOOL
 * @retval AK_TRUE: CMD sent successfully
 * @retval AK_FALSE: CMD sent failed

 */
T_BOOL send_cmd(T_U8 cmd_index, T_U8 resp, T_U32 arg)
{
    T_U32 cmd_value = 0;
    T_U32 status;

    if (cmd_index == SD_CMD(1) || cmd_index == SD_CMD(41) || cmd_index == SD_CMD(5))      //R3 is no crc
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET) 
                                | ( cmd_index << CMD_INDEX_OFFSET) | RSP_CRC_NO_CHK;
    }
    else
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET ) 
                                | ( cmd_index << CMD_INDEX_OFFSET) ;
    }

    REG32(s_SdReg_Base + oSD_ARG) = arg;
    REG32(s_SdReg_Base + oSD_CMD) = cmd_value;

    if (SD_NO_RESPONSE == resp)
    {
        while(1)
        {
            status = REG32(s_SdReg_Base + oSD_INT_STA);
            if (status & CMD_SENT)
            {
                return AK_TRUE;
            }
        }
    }
    else if ((SD_SHORT_RESPONSE == resp) ||(SD_LONG_RESPONSE == resp))
    {
        while(1)
        {
            status = REG32(s_SdReg_Base + oSD_INT_STA);
            if ((status & CMD_TIME_OUT)||(status & CMD_CRC_FAIL))
            {
                drv_print("ce, ", (cmd_index << 16) | status, AK_TRUE);
                return AK_FALSE;
            }
            else if (status & CMD_RESP_END)
            {
                return AK_TRUE;
            }
        }
    }
    else
    {
        return AK_FALSE;
    }
}


/**
 * @brief Get sd card  short response.
 *
 * Only get register response0 .
 * @author Huang Xin
 * @date 2010-07-14
 * @return The value of register response0
 */
T_VOID get_short_resp(T_U32 resp[])
{   
    resp[0] = REG32(s_SdReg_Base + oSD_RESP0);
}


/**
 * @brief Get sd card  long response.
 *
 *  Get register response0,1,2,3.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The buffer address to save long response
 * @return T_VOID
 */
T_VOID get_long_resp(T_U32 resp[])
{
    resp[3] = REG32(s_SdReg_Base + oSD_RESP0);
    resp[2] = REG32(s_SdReg_Base + oSD_RESP1);
    resp[1] = REG32(s_SdReg_Base + oSD_RESP2);
    resp[0] = REG32(s_SdReg_Base + oSD_RESP3);
}

/**
 * @brief SD read or write data use l2 dma
 *
 * start l2 dma
 * @author Huang Xin
 * @date 2010-07-14 
 * @param drv_card[in] the handle of sd card
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_trans_data_dma(T_pSD_DEVICE drv_card, T_U32 ram_addr, T_U32 size, T_U8 dir)
{
    T_U32 tmp;
    T_U32 pBuff_addr;
    T_U32 pBuff_len;
    T_BOOL ret;

    //alloc L2 buffer
    ret = l2_init_device_buf(s_L2Select);
    if(AK_FALSE == ret)
    {
        return AK_FALSE;
    }

    ret = get_device_buf_info(s_L2Select, &pBuff_addr, &pBuff_len, &tmp);
    if(AK_FALSE == ret)
    {
        goto EXIT;
    }
        
    sd_cfg_buf(L2_CPU_MODE, pBuff_addr, pBuff_len);
    //set data reg
    set_data_reg(size, drv_card->ulDataBlockLen, drv_card->enmBusMode, dir);

    //set l2 trans dir
    if (SD_DATA_CTL_TO_HOST == dir)
    {
        ret = l2_trans_data_cpu(ram_addr, s_L2Select, size, 0);
    }
    else
    {
        ret = l2_trans_data_cpu(s_L2Select, ram_addr, size, 0);
    }
    
    
    if(AK_FALSE == ret)
    {
        drv_print("sd dma fail", dir, AK_TRUE);        
        goto EXIT;
    }

    while (1)
    {
        tmp = REG32(s_SdReg_Base + oSD_INT_STA);
        if ((tmp & DATA_TIME_OUT) || (tmp & DATA_CRC_FAIL))
        {
            drv_print("sd data error", tmp, AK_TRUE);
            ret = AK_FALSE;
            break;
        }
        
        if (SD_DATA_CTL_TO_HOST == dir && !(tmp & RX_ACTIVE))
        {
            break;
        }
        
        else if (SD_DATA_CTL_TO_CARD == dir && !(tmp & TX_ACTIVE))
        {
            break;
        }
    }

EXIT:
    l2_release_device_buf(s_L2Select);
    return ret;
}

T_BOOL sd_rx_data_cpu(T_pSD_DEVICE drv_card, T_U8 buf[], T_U32 len)
{
    T_U32 status;
    T_U32 buf_tmp;
    T_U32 i;
    T_U32 offset, size;

    REG32(s_SdReg_Base + oSD_DMA_MODE) = 0;
    set_data_reg(len, drv_card->ulDataBlockLen, drv_card->enmBusMode, 1);

    offset = 0;
    size = len;
    while(1)
    {
        status = REG32(s_SdReg_Base + oSD_INT_STA);
        if ((status & DATA_BUF_FULL))
        {
            buf_tmp  = REG32(s_SdReg_Base + oSD_CPU_MODE);
            for (i = 0; i < 4; i++)
            {
                buf[offset + i] = (T_U8)((buf_tmp >> (i * 8)) & 0xff);
            }
            offset += 4;
            size -= 4;
        }
        if ((size > 0) && (size < 4) && (status & DATA_END))
        {
            buf_tmp = REG32(s_SdReg_Base + oSD_CPU_MODE);
            for ( i = 0; i < size; i++)
            {
                buf[offset + i] = (T_U8)((buf_tmp >> (i * 8)) & 0xff);
            }
            size = 0;
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            drv_print("crc or timeout, status:", status, 1);
            return AK_FALSE;
        }
        if (!(status & RX_ACTIVE))
        {
            break;
        }
    }

    return AK_TRUE;
}

T_BOOL sd_tx_data_cpu(T_pSD_DEVICE drv_card, T_U8 buf[], T_U32 len)
{
    T_U32 status;
    T_U32 offset = 0;

    REG32(s_SdReg_Base + oSD_DMA_MODE) = 0;
    set_data_reg(len, drv_card->ulDataBlockLen, drv_card->enmBusMode, 0);

    while(1)
    {
        status = REG32(s_SdReg_Base + oSD_INT_STA);
        if ((offset < len) && (status & DATA_BUF_EMPTY))
        {
            REG32(s_SdReg_Base + oSD_CPU_MODE) = (buf[offset])|(buf[offset+1]<<8)
                                                 |(buf[offset+2]<<16)|(buf[offset+3]<<24);
            offset += 4;
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            drv_print("crc or timeout, status:", status, 1);
            return AK_FALSE;
        }
        if (!(status & TX_ACTIVE))
        {
            break;
        }
    }

    return AK_TRUE;
}

/**
* @brief Check if sd controller is transferring now
* @author Huang Xin
* @date 2010-07-14
* @return T_BOOL
* @retval  AK_TRUE: sd controller is transferring
* @retval  AK_FALSE: sd controller is not transferring
*/
T_BOOL sd_trans_busy(T_VOID)
{
    T_U32 status;
    status = REG32((s_SdReg_Base + oSD_INT_STA));
    if ((status & TX_ACTIVE) || (status & RX_ACTIVE))
    {
        return  AK_TRUE;
    }
    else 
    {
        return AK_FALSE;
    }
}

