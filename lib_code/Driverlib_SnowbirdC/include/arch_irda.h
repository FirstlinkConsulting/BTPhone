/*******************************************************************************
 * @file    arch_uart.h
 * @brief   UART driver header file
 * This file provides UART APIs: UART initialization, write data to UART.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  ChenWeiwen
 * @date    2008-06-07
 * @version 1.0
 * @ref     AK1036X technical manual.
*******************************************************************************/
#ifndef __ARCH_IRDA_H__
#define __ARCH_IRDA_H__


#include "anyka_types.h"


typedef T_VOID (*T_irDA_CALLBACK)(T_U8 *data, T_U32 cnt);

/*******************************************************************************
 * @brief   init the irDA
 * @author  Liuhuadong
 * @date    2012.12.19
 * @param   [in]irDA_cb  
 * @return  T_VOID
*******************************************************************************/
T_VOID IrDA_init(T_BOOL signal_reverse,T_BOOL chkbyte_reverse);
/*******************************************************************************
 * @brief   Set the irDA Data in handle
 * @author  Zhs
 * @date    2012.12.19
 * @param   [in]irDA_cb  :when irDA_cb is AK_NULL,irDA receive shoule use poll mode,otherwise irDA
 *                                  receive will use interrupt mode
 * @return  T_VOID
*******************************************************************************/
T_VOID IrDA_SetCallBak(T_irDA_CALLBACK irDA_cb);
/*******************************************************************************
 * @brief   Poll IrDA data manual
 * @author  Zhs
 * @date    2012.12.19
 * @param   [in]irDA_cb :when in irDA in Poll mode,programer should use this api to get irDA data
 * @return  T_VOID
*******************************************************************************/
T_VOID IrDA_Poll(T_irDA_CALLBACK irDA_cb);

#endif    //__ARCH_UART_H__

