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


/*******************************************************************************
 * @brief   initialize freq manage stack 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FreqMgrInit(T_VOID)
{

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
    return AK_TRUE;
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
    return AK_TRUE;
}


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
    return 0;
}

T_BOOL Fwl_Freq_Add2Calc(T_U32 val)
{
    return AK_TRUE;
}

T_BOOL Fwl_Freq_Clr_Add(T_VOID)
{
    return AK_TRUE;
}

T_U32 Fwl_Get_AddFreq(T_VOID)
{
    return 0;
}

T_U32 Fwl_Get_Calc_Freq(T_SYS_CLOCK_LEVEL clock)
{
    return 0;
}

