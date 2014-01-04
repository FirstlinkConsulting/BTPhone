/**@file hal_common_sd.c
 * @brief Implement sd&sdio commonl operations of how to control sd&sdio.
 *
 * This file implement sd&sdio common bus driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "l2.h"
#include "share_pin.h"
#include "clk.h"
#include "sd.h"
#include "hal_common_sd.h"
#include "drv_api.h"

static T_BOOL init_start(T_VOID);
static T_eCARD_TYPE get_card_type(T_pSD_DEVICE drv_card);
static T_U32 get_rca(T_VOID);
static T_U32 set_rca(T_U16 rca);

/**
 * @brief Get card type
 *
 * Called at the end of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
static T_eCARD_TYPE get_card_type(T_pSD_DEVICE drv_card)
{
    T_eCARD_TYPE type = CARD_UNUSABLE;

    if (SD_INIT_SD == drv_card->bInitMemSuccess)
    {
        drv_card->ulRCA = get_rca();
        type = CARD_SD;
    }
    else
    {
        //card addr is fixed 0x2,but the addr must be different  for supporting multi card 
        drv_card->ulRCA = set_rca(0x2);
        type = CARD_MMC;
    }
    
    if(ERROR_INVALID_RCA != drv_card->ulRCA)
    {
        return type;
    }
    else
    {
		drv_print("get rca fail", 0, AK_TRUE);
		return CARD_UNUSABLE;
    }
}

static T_VOID card_delay_us(T_U32 us)
{
	T_U32 tmp;
	T_U32 i, sum;
	sum = (us * 10) / 6;
	for (i=0; i <= sum; i++)
	{
		tmp = *(volatile T_U32 *)(0x00400000);
	}
}

/**
 * @brief Init card start
 *
 * Called at the beginning of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_BOOL
 * @retval AK_TRUE Start successful
 * @retval AK_FALSE  Start failed
 */
static T_BOOL init_start(T_VOID)
{
    T_U8 i=0;
    T_U32 response=0;
    
    //only init mem,or init all
    if (!send_cmd(SD_CMD(0), SD_NO_RESPONSE, SD_NO_ARGUMENT))
    {
        return AK_FALSE;
    }
    
    //NOTE:this delay is necessary ,otherwise re-init will failed while sd is power on for some cards
    card_delay_us(3000);
    
    for (i = 0; i<3; i++)
    {
        if (send_cmd(SD_CMD(8), SD_SHORT_RESPONSE, 0x1aa))
        {
            get_short_resp(&response);
            if (response != 0x1aa)
            {
                return AK_FALSE;
            }
            break;
        }
    }
    
    return AK_TRUE;
}

/**
 * @brief get sd relative address.
 *
 * Send CMD3 to get the sd relative address.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_U32
 * @retval The RCA
 * @retval ERROR_INVALID_RCA
 */
static T_U32 get_rca(T_VOID)
{
    T_U32 rca=0;
    if (send_cmd(SD_CMD(3), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))
    {
        get_short_resp(&rca);
        rca = rca >> 16;
        return rca;
    }
    else
    {
        return ERROR_INVALID_RCA;
    }
}

static T_U32 set_rca(T_U16 rca)
{ 
    if (send_cmd(SD_CMD(3), SD_SHORT_RESPONSE, rca<<16))
    {
        return rca;
    }
    else
    {
        return ERROR_INVALID_RCA;
    }
}

/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE init_card(T_pSD_DEVICE drv_card)
{
    if (!init_start())
    {
		drv_print("init start fail", 0, AK_TRUE);
		return CARD_UNUSABLE;
    }

    if (COMMON_SD_INIT_MEM_SUCCESS == init_mem(drv_card))
    {   
        return get_card_type(drv_card);
    }
	
	drv_print("init mem fail", 0, AK_TRUE);
	
    return CARD_UNUSABLE;
}

/**
 * @brief Slect or reject a mmc or sd card.
 *
 * Send CMD7 to select a sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param rca[in] The selected card relative address 
 * @return T_BOOL
 * @retval AK_TRUE Select successful
 * @retval AK_FALSE Select failed
 */
T_BOOL select_card(T_U32 rca)
{
    //deselect card
    if (rca == 0)
    {
        if (send_cmd(SD_CMD(7), SD_NO_RESPONSE, (rca << 16)))
            return AK_TRUE;
    }
    //select card
    else
    {
        if (send_cmd(SD_CMD(7), SD_SHORT_RESPONSE, (rca << 16)))
            return AK_TRUE;
    }
    return AK_FALSE;
}

