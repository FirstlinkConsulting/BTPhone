/**
 * @file
 * @brief ANYKA software
 * this file will constraint the access to the bottom layer serial function,
 * avoid resource competition. Also, this file os for porting to 
 * different OS
 *
 * @author ZouMai
 * @date 2001-4-20
 * @version 1.0
 */
#include "boot.h"
#include "arch_uart.h"
#include "vme.h"
#include "Fwl_Serial.h"
#include "gpio_define.h"

//#pragma arm section code = "_sysinit_"

/*********************************************************************
  Function:         Fwl_UartInit
  Description:      initialize uart.
  Input:            T_U32 baudrate.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartInit(T_U32 baudrate)
{
    return uart_init(SERIAL_UART, baudrate);
}

/*********************************************************************
  Function:         Fwl_UartFree
  Description:      free uart.
  Input:            T_VOID.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartFree(T_VOID)
{
    return uart_close(SERIAL_UART);
}

/*********************************************************************
  Function:         Fwl_UartSetBaudrate
  Description:      change uart baudrate.
  Input:            T_U32 baudrate.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartSetBaudrate(T_U32 baudrate)
{
#if 1
    return uart_reset(SERIAL_UART, baudrate);
#else
    return AK_TRUE;
#endif
}


/*********************************************************************
  Function:         Fwl_UartGetTxStatus
  Description:      get status of uart.
  Input:            T_VOID
  Return:           T_bool AK_FALSE:NOT SEND END; AK_TRUE: SEND OVER.
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartGetTxStatus(T_VOID)
{
    return uart_get_write_status(SERIAL_UART);
}
#pragma arm section code = "_SYS_BLUE_CODE_"
/*********************************************************************
  Function:         Fwl_UartSetIntStatus
  Description:      Fwl_UartSetIntStatus.
  Input:            T_VOID
  Return:           T_bool AK_FALSE:DISABLE fiq; AK_TRUE: enable fiq.
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartSetIntStatus(T_BOOL status)
{
    uart_set_fiq_int(SERIAL_UART, status);
	return AK_TRUE;
}

/*********************************************************************
  Function:         Fwl_UartWriteData
  Description:      write data to uart.
  Input:            T_U8 *data: The string which will be written to UART
  Return:           T_U32 written number.
  Author:           liangxiong
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_UartWriteData(T_U8 *data, T_U32 len)
{
    return uart_write_dat(SERIAL_UART, data, len);
}
#pragma arm section code

T_VOID Fwl_UartSetCallback(T_fUART_CB callback_func)
{
    #if 1
    uart_set_callback(SERIAL_UART, callback_func);
    #endif
}



