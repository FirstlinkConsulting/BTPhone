/*******************************************************************************
 * @file    Fwl_PMU.c
 * @brief   This file provides pmu APIs: initialization
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#include "Fwl_PMU.h"
#include "arch_pmu.h"


#pragma arm section zidata = "_bootbss_"
static T_BOOL PMU_bLDOMode;
static CORE12_VOLTAGE PMU_VDDCORE;
static LDO33_VOLTAGE PMU_VDDIO;
#pragma arm section zidata


/*******************************************************************************
  Function:     Fwl_PMU_Init
  Description:  PMU initial
  Input:        bLDOmode: whether use LDO mode
  Return:       T_VOID
  Author:       liangxiong
  Data:         2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_Init (T_BOOL bLDOmode)
{
    CORE12_TYPE mode;

    PMU_bLDOMode = bLDOmode;
    PMU_VDDCORE  = CORE12_12V;
    PMU_VDDIO    = LDO33_33V;

    mode = PMU_bLDOMode ? CORE12_LDO : CORE12_DCDC;
    pmu_core12_sel(mode, PMU_VDDCORE);
    pmu_ldo33_sel(PMU_VDDIO);
}

/*******************************************************************************
*  Function:     Fwl_PMU_USBPushIn
*  Description:  change to highest voltage or charge mode when enter USB
*  Input:        T_VOID
*  Return:       T_VOID
*  Author:       liangxiong
*  Data:         2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_USBPushIn(T_VOID)
{
}

/*******************************************************************************
*  Function:     FWL_PMU_USBPullOut
*  Description:  resume voltage or charge mode when exit USB
*  Input:        T_VOID
*  Return:       T_VOID
*  Author:       xuping
*  Data:         2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_USBPullOut(T_VOID)
{
    CORE12_TYPE mode;

    mode = PMU_bLDOMode ? CORE12_LDO : CORE12_DCDC;
    pmu_core12_sel(mode, PMU_VDDCORE);
    pmu_ldo33_sel(PMU_VDDIO);
}


