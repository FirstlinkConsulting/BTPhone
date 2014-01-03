/*******************************************************************************
 * @file    Fwl_PMU.h
 * @brief   pmu function header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_PMU_H__
#define __FWL_PMU_H__


#include "anyka_types.h"


/*******************************************************************************
  Function:         Fwl_PMU_Init
  Description:      PMU initial
  Input:            bLDOmode: whether use LDO mode
  Return:           T_VOID
  Author:           liangxiong
  Data:             2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_Init (T_BOOL bLDOmode);

/*******************************************************************************
*  Function:        Fwl_PMU_USBPushIn
*  Description:     change to highest voltage or charge mode when enter USB
*  Input:           T_VOID
*  Return:          T_VOID
*  Author:          liangxiong
*  Data:            2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_USBPushIn(T_VOID);

/*******************************************************************************
*  Function:        Fwl_PMU_USBPullOut
*  Description:     resume voltage or charge mode when exit USB
*  Input:           T_VOID
*  Return:          T_VOID
*  Author:          xuping
*  Data:            2012-11-20
*******************************************************************************/
T_VOID Fwl_PMU_USBPullOut(T_VOID);

#endif

