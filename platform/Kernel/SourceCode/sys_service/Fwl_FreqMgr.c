/*******************************************************************************
 * @file    Fwl_FreqMgr.c
 * @brief   freq manager file
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  li_shengkai
 * @date    2012-11-29
 * @version 1.0
*******************************************************************************/
#include "Fwl_FreqMgr.h"
#include "hal_clk.h"
#include "eng_debug.h"


const T_U8 appClkTbl[FREQ_APP_NUM] =
{
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_DEFAULT,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_MAX,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_VIDEO,
    LEVEL8_CLOCK_DIV8,      //FREQ_APP_AUDIO_L1,
    LEVEL4_CLOCK_DIV4,      //FREQ_APP_AUDIO_L2,
    LEVEL2_CLOCK_DIV2,      //FREQ_APP_AUDIO_L3,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_AUDIO_L4,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_AUDIO_L5,
    LEVEL18_CLOCK_DIV32,    //FREQ_APP_RADIO,
    LEVEL18_CLOCK_DIV32,    //FREQ_APP_STANDBY,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_USB,
    LEVEL4_CLOCK_DIV4,      //FREQ_APP_AUDIO_REC,
    LEVEL8_CLOCK_DIV8,      //FREQ_APP_AUDIO_REC_STOP,
    LEVEL1_CLOCK_DIV1,      //FREQ_APP_FS_SEARCH
    LEVEL2_CLOCK_DIV2,       //FREQ_APP_USB_MUSIC,
    LEVEL1_CLOCK_MAX       //FREQ_APP_BLUE,
};

static const T_U32 m_freqs[]=
{
    FREQ_PLL_MAX,
    FREQ_PLL_MAX/2,
    FREQ_PLL_MAX/4,
    FREQ_PLL_MAX/6,
    FREQ_PLL_MAX/8,
    FREQ_PLL_MAX/10,
    FREQ_PLL_MAX/12,
    FREQ_PLL_MAX/14,
    FREQ_PLL_MAX/16,
    FREQ_PLL_MAX/18,
    FREQ_PLL_MAX/20,
    FREQ_PLL_MAX/24,
    FREQ_PLL_MAX/28,
    FREQ_PLL_MAX/32,
    FREQ_PLL_MAX/36,
    FREQ_PLL_MAX/40,
    FREQ_PLL_MAX/48,
    FREQ_PLL_MAX/56,
    FREQ_PLL_MAX/64
};

typedef struct
{
    T_FREQ_APP          freqStackArr[FREQ_STACK_DEPTH]; /* stack array          */  
    T_U32               freqSetted[FREQ_STACK_DEPTH];
    T_SYS_CLOCK_LEVEL   curClock;                       /* current system clock */
    T_U32               curFreqSetted;
    T_U32               freq_add;
    T_U8                stackPC;                        /* offset pc        */
}T_FREQ_INFO;


static T_FREQ_INFO freq_info;


/*******************************************************************************
 * @brief   initialize freq manage stack 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FreqMgrInit(T_VOID)
{
    T_U8 i;

    freq_info.stackPC = 0;

    for (i=0; i<FREQ_STACK_DEPTH; i++)
    {
        freq_info.freqStackArr[i] = FREQ_APP_DEFAULT; //SYSTEM_CLOCK_DEFAULT
        freq_info.freqSetted[i]   = FREQ_APP_DEFAULT;
    }

    freq_info.curClock = LEVEL1_CLOCK_DIV1;
    freq_info.freq_add = 0;
    freq_info.curFreqSetted = 0;
}


/*******************************************************************************
 * @brief   push freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]app: 
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_FreqPush(T_FREQ_APP app)
{
    T_BOOL ret = AK_FALSE;
    T_U32 freq;
	
  akerror("-------------push:", app, 0);
  akerror(" pc:", freq_info.stackPC+1, 1);

    if(freq_info.stackPC < (FREQ_STACK_DEPTH-1))
    {
        freq_info.stackPC++;
        freq_info.freqStackArr[freq_info.stackPC] = app;
    }
    else
    {
        akerror("error: frequency stack overflow", freq_info.stackPC, 1);
        return AK_FALSE;
    }

    if (freq_info.stackPC == 1)
    {
        freq = m_freqs[appClkTbl[app]] + freq_info.freq_add;
        freq = (freq<FREQ_PLL_MAX) ? freq : FREQ_PLL_MAX;
        freq_info.curFreqSetted = clk_set_cpu(freq);
        freq_info.freqSetted[freq_info.stackPC] = freq_info.curFreqSetted;
        freq_info.curClock  = appClkTbl[app];
        ret = AK_TRUE;
    }
    else
    {   
        freq = m_freqs[appClkTbl[app]] + freq_info.freq_add;        
        freq = (freq<FREQ_PLL_MAX) ? freq : FREQ_PLL_MAX;

        if (freq_info.curFreqSetted < freq)
        {
            freq_info.curFreqSetted = clk_set_cpu(freq);
            freq_info.curClock = appClkTbl[app];
            ret = AK_TRUE;
        }
        freq_info.freqSetted[freq_info.stackPC] = freq_info.curFreqSetted;
    }

    return ret;
}


/*******************************************************************************
 * @brief   pop freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_FreqPop(T_VOID)
{
    T_FREQ_APP app;
    T_U32 freq, i;
	
  akerror("-------------pop:", freq_info.freqStackArr[freq_info.stackPC], 0);
  akerror(" pc:", freq_info.stackPC-1, 1);

    if(freq_info.stackPC > 0)
    {
        freq_info.stackPC--;
        app = freq_info.freqStackArr[freq_info.stackPC];
    }
    else
    {
        akerror("error: frequency stack no freq", freq_info.stackPC, 1);
        return AK_FALSE;
    }

    freq = m_freqs[appClkTbl[app]];

    for (i=1; i<freq_info.stackPC; i++)
    {
        app = freq_info.freqStackArr[i];
        freq = (freq > m_freqs[appClkTbl[app]])?freq:m_freqs[appClkTbl[app]];
    }

    freq += freq_info.freq_add;
    freq = (freq<FREQ_PLL_MAX) ? freq : FREQ_PLL_MAX;

    freq_info.curFreqSetted = clk_set_cpu(freq);

    freq_info.curClock = appClkTbl[app];

    if (freq_info.freqSetted[freq_info.stackPC] != freq_info.curFreqSetted)
    {
        akerror("freq setted not fit! cur:", freq_info.curFreqSetted, 0);
        akerror("  pre:", freq_info.freqSetted[freq_info.stackPC], 1);
        freq_info.freqSetted[freq_info.stackPC] = freq_info.curFreqSetted;
    }

    return AK_TRUE;
}

#pragma arm section code = "_frequentcode_"
/*******************************************************************************
 * @brief   get freq 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the freq
*******************************************************************************/
T_U32 Fwl_FreqGet(T_VOID)
{
    return freq_info.curFreqSetted;
}
#pragma arm section code

T_BOOL Fwl_Freq_Add2Calc(T_U32 val)
{
    freq_info.freq_add = val;

    Fwl_FreqPush(freq_info.freqStackArr[freq_info.stackPC]);

    return AK_TRUE;
}

T_BOOL Fwl_Freq_Clr_Add(T_VOID)
{
    freq_info.freq_add = 0;
    Fwl_FreqPop();

    return AK_TRUE;
}



