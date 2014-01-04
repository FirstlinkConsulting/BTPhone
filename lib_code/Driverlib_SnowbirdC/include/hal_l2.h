/*******************************************************************************
 * @file    hal_l2.h
 * @brief   the interface for the register callback function when DMA finished.
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.22
 * @version 1.0
*******************************************************************************/
#ifndef __HAL_L2_H__
#define __HAL_L2_H__


#include "anyka_types.h"


typedef enum
{
    GB_DAC,
    GB_ADC,
    GB_UART1_TX,
    GB_UART1_RX,
    GB_UART2_TX,
    GB_UART2_RX,
    GB_CAMERA,
    GB_MAX_TYPE
}T_eGB_TYPE;

typedef enum
{
    L2_TRANS_CPU,
    L2_TRANS_DMA,
    L2_MODE_MAX
}T_eMode_TYPE;


typedef T_VOID(*T_FUNC_GET_BUF)(T_U8 **ppBuf, T_U32 *pLen);


/*******************************************************************************
 * @brief   set the callback function which will be called when DAM finish.
 * @author  wangguotian
 * @date    2012.11.21
 * @param   [in]pget_buffer
 *              a pointer to a call back function.
 * @param   [in]type
 * @return  T_BOOL
*******************************************************************************/
T_BOOL l2_get_buffer_register(T_FUNC_GET_BUF pget_buffer, T_eGB_TYPE type);

/*******************************************************************************
 * @brief   start the l2 transter.
 * @author  wangguotian
 * @date    2012.11.21
 * @param   [in]type
 * @param   [in]tran_mode
 * @param   [in]interval
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
T_BOOL l2_transfer_start(T_eGB_TYPE type, T_eMode_TYPE tran_mode, T_U8 interval);

/*******************************************************************************
 * @brief   stop the l2 transter.
 * @author  zhanggaoxin
 * @date    2013.02.25
 * @param   [in]type
 * @return  T_VOID
*******************************************************************************/
T_VOID l2_transfer_stop(T_eGB_TYPE type);


#endif  //__HAL_L2_H__

