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


/*******************************************************************************
 * @brief   print char
 * @author  liuhuadong
 * @date    2012-11-20
 * @param   [in]chr:
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_ConsoleWriteChr(T_U8 chr);


/*******************************************************************************
 * @brief   print string
 * @author  liuhuadong
 * @date    2012-11-20
 * @param   [in]str:
 * @return  T_BOOL
 * @retval  AK_TRUE
 * @retval  AK_FALSE
*******************************************************************************/
T_BOOL Fwl_ConsoleWriteStr(T_U8 *str);


#endif
