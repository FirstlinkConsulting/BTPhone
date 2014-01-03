/**
 * @file uart.c
 * @brief UART driver, define UART1(serial port 1) and UART2(serial port 2) APIs.
 * This file provides UART APIs: UART initialization, write data to UART, read data from
 * UART, register callback function to handle data from UART, and interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-16
 * @version 1.0
 * @ref AK3210 technical manual.
 */

#include <assert.h>
#include <windows.h>
#include <Windowsx.h>

#include "arch_uart.h"
#include "anyka_types.h"
#include "w_comport.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


T_U32 uart_write(T_eUART_ID id, const T_U8 *data, T_U32 data_len);

/**
 * @brief Initialize UART
 * Initialize UART base on UART ID, baudrate and system clock. If user want to change
 * baudrate or system clock is changed, user should call this function to initialize
 * UART again.
 * Function uart_init() must be called before call any other UART functions
 * @author ZouMai
 * @date 2004-09-16
 * @param T_UART_ID uart_id: UART ID, must be uiUART1 or uiUART2
 * @param T_U32 baud_rate: Baud rate, use UART_BAUD_9600, UART_BAUD_19200 ...
 * @param T_U32 sys_clk: system clock
 * @return T_BOOL: Init UART OK or not
 * @retval AK_TRUE: Successfully initialized UART.
 * @retval AK_FALSE: Initializing UART failed.
 */
T_BOOL uart_init(T_eUART_ID id, T_eUART_BAUDRATE baud_rate)
{
    if (COMPORT_OK != comport_Open(baud_rate))
    {
        return FALSE;
    }

    return AK_TRUE;
}

T_BOOL uart_reset(T_eUART_ID id, T_eUART_BAUDRATE baud_rate)
{
    if(comport_SetBaudRate(baud_rate))
        return AK_TRUE;
    else
        return AK_FALSE;
}
T_BOOL uart_close(T_eUART_ID id)
{
    comport_Close();
    return AK_TRUE;
}

/**
 * @brief Write one character to UART base on UART ID
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID, must be uiUART1 or uiUART2
 * @param T_U8 chr: The character which will be written to UART
 * @return T_BOOL: Write character OK or not
 * @retval AK_TRUE: Successfully written character to UART.
 * @retval AK_FALSE: Writing character to UART failed.
 */
T_BOOL uart_write_chr(T_eUART_ID id, T_U8 chr)
{
    if (comport_Write(&chr, 1) == 1)
        return AK_TRUE;
    else
        return AK_FALSE;
}

/**
 * @brief Write string to UART base on UART ID
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID, must be uiUART1 or uiUART2
 * @param T_U8 *str: The string which will be written to UART
 * @return T_U32: Length of the data which have been written to UART
 * @retval
 */
T_U32 uart_write_str(T_eUART_ID id, T_U8 *str)
{
    T_U32   written_num = 0;

    if (str == AK_NULL)
    {
        return 0;
    }

    return comport_Write(str, strlen(str));

}
T_VOID uart_setstateRTS(T_eUART_ID id,BOOL bState)
{
	comport_SetStateRTS(bState);
}
T_U32 uart_write_dat(T_eUART_ID id, T_U8 *d, T_U32 len)
{
    T_U32   written_num = 0;

    if (d == AK_NULL)
    {
        return 0;
    }

    return comport_Write(d, len);

}

/**
 * @brief Read a character from UART
 * This function will not return until get a character from UART
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param T_UART_ID uart_id: UART ID, must be uiUART1 or uiUART2
 * @param T_U8 *chr: character for return
 * @return T_BOOL: Got character or not
 * @retval
 */
T_U8 uart_read_chr(T_eUART_ID id)
{
	T_U8 chr;

    while (comport_Read(&chr, 1) == 1);

    return chr;
}

T_U32 uart_read_byte(T_eUART_ID id, T_U8 *buf, T_U32 size)
{
    return comport_Read(buf, size);
}

/**
 * @brief UART interrupt handler
 * If chip detect that UART received data, this function will be called.
 * This function will get UART data from UART Receive Data Hold Register, and call
 * UART callback function to process the data if it is available.
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-16
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID uart_interrupt_handler_WIN32(T_VOID)
{
    T_U8    cur_chr;

    while (1)
    {
        if (comport_Read(&cur_chr, 1) == 0)
        {
            break;
        }
    }

    return;
}

/**
 * @brief Print data to CONSOLE UART
 * Function uart_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-17
 * @param const T_U8 *s: Data format
 * @param ...: data
 * @return AK_VOID
 * @retval
 */
/*
T_VOID uart_printf(const T_U8 *s, ...)
{
    T_S8 printf_buf[1024] = {0};
    T_U32 i;
    va_list ap;

    va_start(ap, s);
    vsprintf(printf_buf, s, ap);
    va_end(ap);

    for(i=0; i<strlen(printf_buf); i++)
        uart_write_chr(CONSOLE_UART, printf_buf[i]);

    return;
}
*/
/* end of file */
