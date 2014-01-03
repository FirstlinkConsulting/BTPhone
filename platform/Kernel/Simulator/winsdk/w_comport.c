/*****************************************************************************
 * Copyright (C) Anyka 2005
 *****************************************************************************
 *   Project:
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *    first step: only one com port, without multiplexer
 *
 *****************************************************************************
*/

#include <stdio.h>
#include <assert.h>
#include "anyka_types.h"
#include "Windows.h"
#include "w_comport.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/
#define MAX_COM_PORTS           5
#define MAX_PORTMODE_STRING     500
#define PORTNAME_PREFIX         "COM5"
#define COM_EVENT_MASK_ALL_EVENTS      (EV_CTS    | \
                                        EV_DSR    | \
                                        EV_RING   | \
                                        EV_RXCHAR)

typedef void (*comportT_ComEventCallback)     (void *vPort, DWORD dwEvent);

typedef struct {
    HANDLE                      hPort;
    HANDLE                      hThread;
    DWORD                       dwPortNum;
    DCB                         Dcb;
    comportT_ComEventCallback   EventCallback;
    BOOL                        bStateRTS;
    BOOL                        bStateDTR;
} comportT_Port;

/**
array for available com ports
*/
static comportT_Port   PortTab[MAX_COM_PORTS];

extern T_VOID uart_interrupt_handler_WIN32(T_VOID);

/*****************************************************************************
 * functions
 *****************************************************************************
*/

static BOOL IsPortPointerValid(comportT_Port *pPort)
{
    int i;

    assert (pPort != NULL);

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        if (pPort == &PortTab[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL IsPortOpened(comportT_Port *pPort)
{
    assert (pPort != NULL);

    if (INVALID_HANDLE_VALUE != pPort->hPort)
    {
        return TRUE;
    }
    return FALSE;
}

static void FreePort(comportT_Port *pPort)
{
    pPort->hPort = INVALID_HANDLE_VALUE;
}

/**
* thread for port events
*/
static DWORD WINAPI EventReader(PVOID pData)
{
    comportT_Port   *pPort;
    DWORD           dwEventMask;
    OVERLAPPED      ovl = {0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };

    pPort = (comportT_Port *)pData;

    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        return -1;
    }

    while (1)
    {
        if (!WaitCommEvent (pPort->hPort, &dwEventMask, &ovl))
        {
            if ( ERROR_IO_PENDING == GetLastError() )
            {
                DWORD dwBytesTransferred=0;
                if ( !GetOverlappedResult( pPort->hPort, &ovl, &dwBytesTransferred, TRUE ) )
                {
                    CloseHandle ( ovl.hEvent );
                    return -2;
                }
            }
            else
            {
                CloseHandle ( ovl.hEvent );
                return -3;
            }
        }

        if ( dwEventMask & EV_ERR )    { pPort->EventCallback( pPort, EV_ERR );    }
        if ( dwEventMask & EV_BREAK )  { pPort->EventCallback( pPort, EV_BREAK );  }
        if ( dwEventMask & EV_CTS )    { pPort->EventCallback( pPort, EV_CTS );    }
        if ( dwEventMask & EV_DSR )    { pPort->EventCallback( pPort, EV_DSR );    }
        if ( dwEventMask & EV_RING )   { pPort->EventCallback( pPort, EV_RING );   }
        if ( dwEventMask & EV_RXCHAR )
        {
            pPort->EventCallback( pPort, EV_RXCHAR );
        }

        if ( dwEventMask & EV_RLSD )   { pPort->EventCallback( pPort, EV_RLSD );   }
        if ( dwEventMask & EV_RXFLAG ) { pPort->EventCallback( pPort, EV_RXFLAG ); }
        if ( dwEventMask & EV_TXEMPTY ){ pPort->EventCallback( pPort, EV_TXEMPTY );}

    }

    CloseHandle (ovl.hEvent);
    return 0;
}

static comportT_Port *UartID_2_Port(void)
{
    int i;
    DWORD   port_id = 0;

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        if (port_id == PortTab[i].dwPortNum)
        {
            return &PortTab[i];
        }
    }

    return NULL;
}

static comportT_Port *GetNewPort(void)
{
    int i;

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        if (INVALID_HANDLE_VALUE == PortTab[i].hPort)
        {
            return &PortTab[i];
        }
    }
    return NULL;
}

/**
* com port event handler
*/
static void comport_EventHandler(void *vPort, DWORD dwComEvent)
{
    comportT_Port *pPort = (comportT_Port *)vPort;

    uart_interrupt_handler_WIN32();

    return;
}

/*****************************************************************************
 * interface
 *****************************************************************************
*/

void comport_Init(void)
{
    int i;

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        // reset PortTab
        PortTab[i].hPort            =   INVALID_HANDLE_VALUE;
        PortTab[i].hThread          =   INVALID_HANDLE_VALUE;
        PortTab[i].dwPortNum        =   0;
        PortTab[i].EventCallback    =   NULL;
    }

    return;
}

DWORD comport_Open(DWORD baud_rate)
{
    DWORD           dwPortNum;
    comportT_Port   *pPort;
    char            strPortName[_MAX_PATH];
    //DWORD           dwThreadId;
    char            strPortMode[MAX_PORTMODE_STRING];
    COMMTIMEOUTS    ComTimeOut;

    dwPortNum = 0;

    /**
    * get structure for port parameters
    */
    pPort = GetNewPort ();
    if (NULL == pPort)
    {
        return ERR_NO_PORT_RESOURCE;
    }

    pPort->dwPortNum        = dwPortNum;
    pPort->EventCallback    = comport_EventHandler;

    /**
    * build filename for requested port
    */
    sprintf (strPortName, "%s", PORTNAME_PREFIX);

    /**
    * open port
    */
    pPort->hPort    =   CreateFile (
                            strPortName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            NULL
                        );

    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        FreePort (pPort);
        return ERR_PORT_CANNOT_BE_OPENED;
    }

    /**
    * set port configuration
    */
    sprintf (strPortMode, "baud=%d parity=N data=8 stop=1", baud_rate);
    BuildCommDCB (strPortMode, &pPort->Dcb);
    SetCommState (pPort->hPort, &pPort->Dcb);
    pPort->bStateDTR    =   TRUE;
    pPort->bStateRTS    =   TRUE;

    /**
    * set eventmask
    */
    PurgeComm (pPort->hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    SetCommMask (pPort->hPort, COM_EVENT_MASK_ALL_EVENTS);

    /**
    * set timouts for operations with com port
    */
    ComTimeOut.ReadIntervalTimeout          =   10;
    ComTimeOut.ReadTotalTimeoutConstant     =   20;
    ComTimeOut.ReadTotalTimeoutMultiplier   =   10;
    ComTimeOut.WriteTotalTimeoutConstant    =   10;
    ComTimeOut.WriteTotalTimeoutMultiplier  =   10;


    SetCommTimeouts (pPort->hPort, &ComTimeOut);

    /**
    * create thread for port events
    */
#if 0
    pPort->hThread  =   CreateThread (
                            NULL,           // pointer to thread security attributes
                            0,              // stack size
                            EventReader,    // thread service routine
                            pPort,          // argument for new thread -> pointer port description
                            0,              // creation flags
                            &dwThreadId     // pointer to returned thread identifier
                        );
#endif
    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        FreePort (pPort);
        return ERR_CANNOT_CREATE_EVENTTHREAD;
    }

    return COMPORT_OK;
}

void comport_Close(void)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return;

    FlushFileBuffers (pPort->hPort);
    PurgeComm (pPort->hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    /**
    * stop thread for port events
    */
    if (INVALID_HANDLE_VALUE != pPort->hThread)
    {
        TerminateThread(pPort->hThread, 0);
        CloseHandle (pPort->hThread  );
    }

    /**
    * close port
    */
    if (INVALID_HANDLE_VALUE != pPort->hPort)
    {
        CloseHandle (pPort->hPort);
    }

    FreePort (pPort);

    return;
}

DWORD comport_Read(BYTE *pReadBuffer, DWORD dwNumberOfBytesToRead)
{
    comportT_Port   *pPort;
    OVERLAPPED      ovl = { 0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };
    DWORD           dwRet = COMPORT_OK;
    DWORD           dwBytesRead = 0;

    assert (pReadBuffer != NULL);

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    // attempt an asynchronous read operation
    if (!ReadFile(pPort->hPort, pReadBuffer, dwNumberOfBytesToRead, &dwBytesRead, &ovl))
    {
        DWORD dwErrorCode = GetLastError();

        // deal with the error code
        if (dwErrorCode == ERROR_IO_PENDING)
        {
            // if there was still a problem ...
            if (!GetOverlappedResult(pPort->hPort, &ovl, &dwBytesRead, TRUE))
            {
                dwRet   = ERR_READ_FROM_PORT;
            }
        }
    }

    CloseHandle ( ovl.hEvent );

    return dwBytesRead;
}

DWORD comport_Write(BYTE *pWriteBuffer, DWORD dwNumberOfBytesToWrite)
{
    DWORD           dwBytesWritten = 0;
    comportT_Port   *pPort;
    OVERLAPPED      ovl = { 0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };

    assert (pWriteBuffer != NULL);

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    // attempt an asynchronous write operation
    if (!WriteFile(pPort->hPort, pWriteBuffer, dwNumberOfBytesToWrite, &dwBytesWritten, &ovl))
    {
        DWORD dwErrorCode = GetLastError();

        // deal with the error code
        if (dwErrorCode == ERROR_IO_PENDING)
        {
            // if there was still a problem ...
            if (!GetOverlappedResult(pPort->hPort, &ovl, &dwBytesWritten, TRUE))
            {
                assert (FALSE);
            }
        }
    }

    FlushFileBuffers (pPort->hPort);
    CloseHandle ( ovl.hEvent );

    return dwBytesWritten;
}

DWORD comport_SetBaudRate(DWORD baud_rate)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    pPort->Dcb.BaudRate = baud_rate;

    if (!SetCommState (pPort->hPort, &pPort->Dcb))
    {
        assert (FALSE);
    }

    return COMPORT_OK;
}

DWORD comport_GetBaudRate(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    return pPort->Dcb.BaudRate;
}

DWORD comport_Flush(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    if (!FlushFileBuffers (pPort->hPort))
    {
        return ERR_FLUSH;
    }

    return COMPORT_OK;
}

BOOL comport_GetStateDSR(T_VOID)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }

    return (dwModeStat & MS_DSR_ON) ? TRUE : FALSE;
}

BOOL comport_GetStateCTS(T_VOID)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }

    return (dwModeStat & MS_CTS_ON) ? TRUE : FALSE;
}

BOOL comport_GetStateRING(T_VOID)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }

    return (dwModeStat & MS_RING_ON) ? TRUE : FALSE;
}

DWORD comport_SetStateRTS(BOOL bState)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    if (!EscapeCommFunction(pPort->hPort, bState ? SETRTS : CLRRTS))
    {
        assert (FALSE);
    }

    pPort->bStateRTS = bState;

    return COMPORT_OK;
}

DWORD comport_SetStateDTR(BOOL bState)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    if (!EscapeCommFunction(pPort->hPort, bState ? SETDTR : CLRDTR))
    {
        assert (FALSE);
    }

    pPort->bStateDTR = bState;

    return COMPORT_OK;
}

BOOL comport_GetStateRTS(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return FALSE;

    return pPort->bStateRTS;
}

BOOL comport_GetStateDTR(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return FALSE;

    return pPort->bStateDTR;
}

void comport_ClearRXBuffer(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return;

    if (!PurgeComm (pPort->hPort, PURGE_RXCLEAR))
    {
        assert (FALSE);
    }

    return;
}

DWORD comport_PeekRXBuffer(T_VOID)
{
    comportT_Port   *pPort;
    COMSTAT         ComStat;
    DWORD           dwComError;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    if (!ClearCommError (pPort->hPort, &dwComError, &ComStat))
    {
        assert (FALSE);
    }

    return ComStat.cbInQue;
}

DWORD comport_SetRTSControlHandshake(T_VOID)
{
    comportT_Port   *pPort;

    if ((pPort = UartID_2_Port()) == NULL ||
        !IsPortPointerValid(pPort) ||
        !IsPortOpened(pPort))
        return ERR_NO_PORT_RESOURCE;

    pPort->Dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;

    if (!SetCommState (pPort->hPort, &pPort->Dcb))
    {
        assert (FALSE);
    }

    return COMPORT_OK;
}
