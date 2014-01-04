/*******************************************************************************
 * @file pmu.h
 * @brief pmu bits define header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author zhanggaoxin
 * @date 2012-12-25
 * @version 1.0
 * @ref AK1080L technical manual.
*******************************************************************************/
#ifndef __PMU_H__
#define __PMU_H__


#include "anyka_types.h"


/*******************************************************************************
 * @brief:  pmu initial
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_VOID
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_init(T_VOID);


/*******************************************************************************
 * @brief:  charger interrupt handler
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]int_flag
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_interrupt_handler(T_U32 int_flag);


#endif

