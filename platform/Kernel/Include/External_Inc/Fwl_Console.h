/*******************************************************************************
 * @file    Fwl_console.h
 * @brief   This header file is for console function prototype
 * @author  liuhuadong
 * @date    2001-4-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_CONSOLE_H__
#define __FWL_CONSOLE_H__


#include "akdefine.h"


/*******************************************************************************
 * @brief   initialize uart
 * @author  liuhuadong
 * @date    2012-11-20
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE = initialization sucess; 
 * @retval  AK_FALSE = initialization error.
*******************************************************************************/
T_BOOL Fwl_ConsoleInit(T_VOID);


/*********************************************************************
  Function:         Fwl_ConsoleWriteChr
  Description:      write character to uart.
  Input:            chr: The character which will be written to UART.
  Author:           
  Data:             2012-11-20
**********************************************************************/
T_VOID Fwl_ConsoleWriteChr(T_U8 chr);


/*********************************************************************
  Function:         Fwl_ConsoleWriteStr
  Description:      write string to uart.
  Input:            T_U8 *str: The string which will be written to UART
  Return:           T_U32 written number of string.
  Author:           
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_ConsoleWriteStr(T_U8 *str);


#endif
