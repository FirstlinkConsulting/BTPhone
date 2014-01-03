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
#ifndef __ARCH_UART_H__
#define __ARCH_UART_H__


#include "anyka_types.h"


/**
 * @brief UART ID define
 *  define uart id
 */
typedef enum
{
    uiUART1 = 0,
    uiUART2,
    MAX_UART_NUM        /* UART number */
} T_eUART_ID;

/**
 * @brief UART baudrate define
 *  define port baudrate
 */
typedef enum
{
    UART_BAUD_9600   = 9600,
    UART_BAUD_19200  = 19200,
    UART_BAUD_38400  = 38400,
    UART_BAUD_57600  = 57600,
    UART_BAUD_115200 = 115200,
    UART_BAUD_921600 = 921600
} T_eUART_BAUDRATE;

typedef T_VOID (*T_fUART_CALLBACK)(T_U8 *add, T_U32 cnt);



/* callback function setting for GPS */
T_VOID uart_set_callback(T_eUART_ID id, T_fUART_CALLBACK callback_func);


/*******************************************************************************
 * @brief   init the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]baud_rate
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE; 
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
T_BOOL uart_init(T_eUART_ID id, T_eUART_BAUDRATE baud_rate);


/*******************************************************************************
 * @brief   init the uart
 * @author  liuhuadong
 * @date    2012.12.19
 * @param   [in]gpio_num
 * @return  T_VOID
*******************************************************************************/
T_VOID Simu_UART_Init(T_U8 gpio_num);


/*******************************************************************************
 * @brief   close the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE; 
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
T_BOOL uart_close(T_eUART_ID id);


/*******************************************************************************
 * @brief   send one char to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]chr
 * @return  T_BOOL
 * @retval  0, fail, or else succeed.
*******************************************************************************/
T_BOOL uart_write_chr(T_eUART_ID id, T_U8 chr);


/*******************************************************************************
 * @brief   send one char to uart
 * @author  liuhuadong
 * @date    2012.12.19
 * @param   [in]pStr
 * @param   [in]len
 * @return  T_VOID
*******************************************************************************/
T_VOID Simu_UART_SendDat(T_U8 *pStr, T_U32 len);


/*******************************************************************************
 * @brief   send one char to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]str
 * @return  T_U32
 * @retval  the number of the bytes sended to uart, if 0, mean the 
 *          ADDR_UART_TX buffer is no init.
*******************************************************************************/
T_U32 uart_write_str(T_eUART_ID id, T_U8 *str);


/*******************************************************************************
 * @brief   send one char to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id
 * @param   [in]str
 * @return  T_U32
 * @retval  the number of the bytes sended to uart, if 0, mean the 
 *          ADDR_UART_TX buffer is no init.
*******************************************************************************/
T_U32 uart_write_dat(T_eUART_ID id, T_U8 *str, T_U32 len);

/*******************************************************************************
 * @brief   get uart tx status
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id
 * @return  T_BOOL
*******************************************************************************/
T_BOOL uart_get_write_status(T_eUART_ID id);

/*******************************************************************************
 * @brief   send str to uart
 * @author  liuhuadong
 * @date    2012.12.19
 * @param   [in]pStr
 * @retval  the number of the bytes sended to uart, if 0, mean the 
 *          ADDR_UART_TX buffer is no init.
*******************************************************************************/
T_VOID Simu_UART_SendStr(T_U8 *pStr);


/*******************************************************************************
 * @brief   receive one char from the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id
 * @return  T_U8
 * @retval  the char received from the uart.
*******************************************************************************/
T_U8 uart_read_chr(T_eUART_ID id);


/*******************************************************************************
 * @brief   send one char to uart
 * @author  liuhuadong
 * @date    2012.12.19
 * @param   [in]chr
 * @return  T_VOID
*******************************************************************************/
T_VOID Simu_UART_SendByte(T_U8 chr);

/*******************************************************************************
 * @brief   uart_set_fiq_int
 * @author  liuhuadong
 * @date    2012.12.19
 * @param   [in]id
 * @param   [in]status
 * @return  T_VOID
*******************************************************************************/
T_VOID uart_set_fiq_int(T_eUART_ID id, T_BOOL status);

/* reset uart */
T_BOOL uart_reset(T_eUART_ID id, T_eUART_BAUDRATE baud_rate);


#endif    //__ARCH_UART_H__

