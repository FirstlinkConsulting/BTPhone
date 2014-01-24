/************************************************************************/
/* pstn_phone gpio data ly201401                                        */
/************************************************************************/

#include "arch_gpio.h"
#include "vme.h"
#include "eng_debug.h"
#include "Fwl_Gpio.h"
#include "windows.h"

 
T_BOOL initFlag = AK_FALSE;
static T_U8 uart_id /*= uiUART1*/;	//temp
HANDLE m_TransmitThread = NULL;
T_fGPIO_CB gGpioReadCallFunc = AK_NULL;
BYTE UartBuf[4096];
//T_U32 uart_read_byte(T_eUART_ID id, T_U8 *buf, T_U32 size);	//temp
//T_BOOL uart_reset(T_eUART_ID id, T_eUART_BAUDRATE baud_rate);	//temp
CRITICAL_SECTION cs;

DWORD __stdcall GpioSendHandle(PVOID pParam) 
{
	DWORD size;
	
	Sleep(1000);
	while(1)
	{
		Sleep(1);
		//size = uart_read_byte(uart_id, UartBuf, 4096);	//temp
		if(gGpioReadCallFunc && size)
		{
			EnterCriticalSection(&cs);
			gGpioReadCallFunc(UartBuf, size);
			LeaveCriticalSection(&cs);
		}
	}
}

T_BOOL Fwl_GpioInit(T_U32 baudrate)
{
    InitializeCriticalSection(&cs);
	initFlag = AK_TRUE;
	m_TransmitThread = CreateThread(NULL,0,GpioSendHandle,NULL,0,NULL);
	if(m_TransmitThread == NULL)
	{
		return 0;
	}
   // return uart_init(uart_id, baudrate);	//temp
	return AK_TRUE;
}

T_BOOL Fwl_GpioFree(T_VOID)
{
    //TODO
    gGpioReadCallFunc = AK_NULL;
    CloseHandle(m_TransmitThread);
	//uart_close(uart_id);	//temp
    DeleteCriticalSection(&cs);
	initFlag = AK_FALSE;
	return AK_TRUE;
}

T_BOOL Fwl_GpioGetTxStatus(T_VOID)
{
    return AK_TRUE;
}

T_BOOL Fwl_GpioSetIntStatus(T_BOOL status)
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

T_BOOL Fwl_GpioSetBaudrate(T_U32 baudrate)
{
    //TODO    
	return AK_TRUE/*uart_reset(uart_id, baudrate)*/;	//temp
}

T_BOOL Fwl_GpioWriteData(T_U8 *data, T_U32 len)
{
    //uart_write_dat(uart_id, data, len);	//temp
	return AK_TRUE;
}

T_VOID Fwl_GpioSetCallback(T_fGPIO_CB callback_func)
{
	gGpioReadCallFunc = callback_func;
}


T_VOID Fwl_GpioSetStateRTS(T_BOOL bState)
{
	//uart_setstateRTS(uart_id,bState);
}
