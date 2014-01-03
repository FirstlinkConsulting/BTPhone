/*******************************************************************************
 * @FILENAME: hal_clk.h
 * @BRIEF hal_clk driver head file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR zhanggaoxin
 * @DATE 2012-11-20
 * @VERSION 1.0
 * @REF
*******************************************************************************/
#ifndef __HAL_CLK_H__
#define __HAL_CLK_H__


#include "anyka_types.h"


/*******************************************************************************
 * @brief   set cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]freq the cpu frequency to set
 * @return  T_U32 the cpu frequency(Hz) running
*******************************************************************************/
T_U32 clk_set_cpu(T_U32 freq);


/*******************************************************************************
 * @brief   get cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @return  T_U32 the cpu frequency(Hz)
*******************************************************************************/
T_U32 clk_get_cpu(T_VOID);


#endif    //__HAL_CLK_H__

