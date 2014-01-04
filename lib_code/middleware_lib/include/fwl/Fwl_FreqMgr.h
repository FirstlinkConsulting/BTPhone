/*******************************************************************************
 * @file    Fwl_FreqMgr.h
 * @brief   freq manager file
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  li_shengkai
 * @date    2012-11-29
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_FREQMGR_H__
#define __FWL_FREQMGR_H__


#include "anyka_types.h"

#ifdef SUPPORT_BLUETOOTH

#define FREQ_PLL_MAX            228000000
#define FREQ_STACK_DEPTH        10    //max frequency manage depth
#define FREQ_VAL_1              57000000
#else
#define FREQ_PLL_MAX            232000000
#define FREQ_STACK_DEPTH        10    //max frequency manage depth
#define FREQ_VAL_1              58000000
#endif


typedef enum {
    LEVEL1_CLOCK_MAX = 0,       // 232MHZ
    LEVEL1_CLOCK_DIV1,          // 116MHz
    LEVEL2_CLOCK_DIV2,          // 58MHz
    LEVEL3_CLOCK_DIV3,          // 38.6MHz
    LEVEL4_CLOCK_DIV4,          // 29MHz
    LEVEL5_CLOCK_DIV5,          // 23.2MHz
    LEVEL6_CLOCK_DIV6,          // 19.3MHz
    LEVEL7_CLOCK_DIV7,          // 16.5MHz  
    LEVEL8_CLOCK_DIV8,          // 14.5MHz
    LEVEL9_CLOCK_DIV9,          // 12.8MHz
    LEVEL10_CLOCK_DIV10,        // 11.6MHz
    LEVEL11_CLOCK_DIV12,        // 9.6MHz
    LEVEL12_CLOCK_DIV14,        // 8.2MHz
    LEVEL13_CLOCK_DIV16,        // 7.2MHz
    LEVEL14_CLOCK_DIV18,        // 6.4MHz
    LEVEL15_CLOCK_DIV20,        // 5.8MHz
    LEVEL16_CLOCK_DIV24,        // 4.8MHz
    LEVEL17_CLOCK_DIV28,        // 4.1MHz
    LEVEL18_CLOCK_DIV32,        // 3.6MHz
} T_SYS_CLOCK_LEVEL;

typedef enum
{
    FREQ_APP_DEFAULT = 0,   //please do not use this app as you frequency,it has special function
    FREQ_APP_MAX,
    FREQ_APP_VIDEO,
    FREQ_APP_AUDIO_L1,
    FREQ_APP_AUDIO_L2,
    FREQ_APP_AUDIO_L3,
    FREQ_APP_AUDIO_L4,
    FREQ_APP_AUDIO_L5,
    FREQ_APP_RADIO,
    FREQ_APP_STANDBY,
    FREQ_APP_USB,
    FREQ_APP_AUDIO_REC,
    FREQ_APP_AUDIO_REC_STOP,
    FREQ_APP_FS_SEARCH,
    FREQ_APP_USB_MUSIC,
#ifdef SUPPORT_BLUETOOTH
	FREQ_APP_BLUE,
#endif
    FREQ_APP_NUM 
} T_FREQ_APP;



/*******************************************************************************
 * @brief   initialize freq manage stack 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FreqMgrInit(T_VOID);


/*******************************************************************************
 * @brief   push freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]app: 
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_FreqPush(T_FREQ_APP app);


/*******************************************************************************
 * @brief   pop freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_FreqPop(T_VOID);


/*******************************************************************************
 * @brief   get freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the freq
*******************************************************************************/
T_U32 Fwl_FreqGet(T_VOID);


/*******************************************************************************
FUNC NAME: Fwl_Freq_Add2Calc
DESCRIPTION: set the addtion freq
INPUT:      T_U32 val   the addtion freq
CREATE DATE:
MODIFY LOG:
*******************************************************************************/
T_BOOL Fwl_Freq_Add2Calc(T_U32 val);


/*******************************************************************************
FUNC NAME: Fwl_Freq_Clr_Add
DESCRIPTION: clear the addtion freq
INPUT:      T_VOID
CREATE DATE:
MODIFY LOG:
*******************************************************************************/
T_BOOL Fwl_Freq_Clr_Add(T_VOID);


#endif //__FWL_FREQMGR_H__

