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
 *
 *****************************************************************************
*/

#ifndef _COMPORT_H
#define _COMPORT_H

//#include "arch_uart.h"

/**
* error values
*/
enum {
    COMPORT_OK  =   0,
    ERR_NO_PORT_RESOURCE,
    ERR_PORT_CANNOT_BE_OPENED,
    ERR_CANNOT_CREATE_EVENTTHREAD,
    ERR_READ_FROM_PORT,
    ERR_FLUSH
};

/**
 * ----------------------------------------------------------------------------
 * com ports
 * ----------------------------------------------------------------------------
*/
void comport_Init(T_VOID);
DWORD comport_Open(DWORD baud_rate);
void comport_Close(T_VOID);
DWORD comport_Read(BYTE *pReadBuffer, DWORD dwNumberOfBytesToRead);
DWORD comport_Write(BYTE *pWriteBuffer, DWORD dwNumberOfBytesToWrite);
DWORD comport_SetBaudRate(DWORD dwBaudRate);
DWORD comport_GetBaudRate(T_VOID);
DWORD comport_Flush(T_VOID);
BOOL comport_GetStateCTS(T_VOID);
BOOL comport_GetStateDSR(T_VOID);
BOOL comport_GetStateRING(T_VOID);
DWORD comport_SetStateRTS(BOOL bState);
DWORD comport_SetStateDTR(BOOL bState);
BOOL comport_GetStateRTS(T_VOID);
BOOL comport_GetStateDTR(T_VOID);
void comport_ClearRXBuffer(T_VOID);
DWORD comport_PeekRXBuffer(T_VOID);
DWORD comport_SetRTSControlHandshake(T_VOID);

#endif // _COMPORT_H
