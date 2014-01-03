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

#include "arch_uart.h"
#include "vme.h"
#include "Fwl_Serial.h"
#include "eng_debug.h"
#include "windows.h"

T_BOOL initFlag = AK_FALSE;
static T_U8 uart_id = uiUART1;
HANDLE m_TransmitThread = NULL;
T_fUART_CB gUartReadCallFunc = AK_NULL;
BYTE UartBuf[4096];
T_U32 uart_read_byte(T_eUART_ID id, T_U8 *buf, T_U32 size);
T_BOOL uart_reset(T_eUART_ID id, T_eUART_BAUDRATE baud_rate);
CRITICAL_SECTION cs;
DWORD __stdcall SerialSendHandle(PVOID pParam) 
{
	DWORD size;
	
	Sleep(1000);
	while(1)
	{
		Sleep(1);
		size = uart_read_byte(uart_id, UartBuf, 4096);
		if(gUartReadCallFunc && size)
		{
			EnterCriticalSection(&cs);
			gUartReadCallFunc(UartBuf, size);
			LeaveCriticalSection(&cs);
		}
	}
}


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
//	gUartReadCallFunc = AK_NULL;
    InitializeCriticalSection(&cs);
	initFlag = AK_TRUE;
	m_TransmitThread = CreateThread(NULL,0,SerialSendHandle,NULL,0,NULL);
	if(m_TransmitThread == NULL)
	{
		return 0;
	}
    return uart_init(uart_id, baudrate);
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
    //TODO
    gUartReadCallFunc = AK_NULL;
    CloseHandle(m_TransmitThread);
	uart_close(uart_id);
    DeleteCriticalSection(&cs);
	initFlag = AK_FALSE;
	return AK_TRUE;
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
    return AK_TRUE;
}

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
	if(initFlag != AK_TRUE)
	{
    	InitializeCriticalSection(&cs);
	}

	if(status)
	{	
		LeaveCriticalSection(&cs);
	}
	else
	{
		EnterCriticalSection(&cs);
	}
    return AK_TRUE;
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
    //TODO    
	return uart_reset(uart_id, baudrate);
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
    uart_write_dat(uart_id, data, len);
	return AK_TRUE;
}

T_VOID Fwl_UartSetCallback(T_fUART_CB callback_func)
{
	gUartReadCallFunc = callback_func;
}


T_VOID Fwl_UartSetStateRTS(T_BOOL bState)
{
	uart_setstateRTS(uart_id,bState);
}
