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

#ifndef _WINVME_H
#define _WINVME_H

#include "vme.h"

/**
* application
*/
void winvme_ScheduleVMEEngine(void);
void winvme_CloesAppl(void);

/**
* timer
*/
unsigned int winvme_StartTimer(unsigned int uiTimeOut, unsigned int uiTimerId);
void winvme_StopTimer(unsigned int uiTimerId);

/**
* display
*/
void winvme_DisplayOn(void);
void winvme_DisplayOff(void);
void winvme_DisplayUpdate(void);

#endif // _WINVME_H
