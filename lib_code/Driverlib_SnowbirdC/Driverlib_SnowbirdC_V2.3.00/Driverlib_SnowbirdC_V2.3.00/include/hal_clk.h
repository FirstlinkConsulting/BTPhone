/*******************************************************************************
 * @file    hal_clk.h
 * @brief   hal_clk driver head file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#ifndef __HAL_CLK_H__
#define __HAL_CLK_H__


#include "anyka_types.h"


#define MAX_ASIC_NUM                5
#define INVALID_ASIC_INDEX          0xff


/*******************************************************************************
 * @brief   set cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]freq: the cpu frequency to set
 * @return  T_U32
 * @retval  the cpu frequency(Hz) running
*******************************************************************************/
T_U32 clk_set_cpu(T_U32 freq);


/*******************************************************************************
 * @brief   get cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @return  T_U32
 * @retval  the cpu frequency(Hz)
*******************************************************************************/
T_U32 clk_get_cpu(T_VOID);


/*******************************************************************************
 * @brief   request minimal asic frequency.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @param   [in]asic_clk: the asic frequency requested
 * @return  T_U8
 * @retval  the index of the asic frequency in array.
*******************************************************************************/
T_U8 clk_request_min_asic(T_U32 asic_clk);


/*******************************************************************************
 * @brief   free minimal asic frequency.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @param   [in]index:
 * @return  T_BOOL
*******************************************************************************/
T_BOOL clk_cancel_min_asic(T_U8 asic_index);


#endif  //__HAL_CLK_H__

