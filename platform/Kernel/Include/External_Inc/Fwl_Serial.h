/*******************************************************************************
 * @file    Fwl_Serial.h
 * @brief   This header file is for serial function prototype 
 * @author  ZouMai
 * @date    2001-4-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_SERIAL_H__
#define __FWL_SERIAL_H__


#include "akdefine.h"


typedef T_VOID (*T_fUART_CB)(T_U8 *add, T_U32 cnt);


/*********************************************************************
  Function:         Fwl_UartInit
  Description:      initialize uart.
  Input:            T_U32 baudrate.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartInit(T_U32 baudrate);

/*********************************************************************
  Function:         Fwl_UartFree
  Description:      free uart.
  Input:            T_VOID.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartFree(T_VOID);

/*********************************************************************
  Function:         Fwl_UartSetBaudrate
  Description:      change uart baudrate.
  Input:            T_U32 baudrate.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartSetBaudrate(T_U32 baudrate);

/*********************************************************************
  Function:         Fwl_UartWriteData
  Description:      write data to uart.
  Input:            T_U8 *data: The string which will be written to UART
  Return:           T_U32 written number.
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartWriteData(T_U8 *data, T_U32 len);

/*******************************************************************************
 * @brief   get uart tx status
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]void
 * @return  T_BOOL
*******************************************************************************/
T_BOOL Fwl_UartGetTxStatus(T_VOID);

/*********************************************************************
  Function:         Fwl_UartSetIntStatus
  Description:      Fwl_UartSetIntStatus.
  Input:            T_VOID
  Return:           T_bool AK_FALSE:DISABLE fiq; AK_TRUE: enable fiq.
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartSetIntStatus(T_BOOL status);

/*********************************************************************
  Function:         Fwl_UartSetCallback
  Description:      set receive data callback.
  Input:            T_fUART_CB callback_func
  Return:           T_VOID
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_VOID Fwl_UartSetCallback(T_fUART_CB callback_func);

#endif

