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

#ifndef _TIMER_H
#define _TIMER_H

#include "anyka_types.h"

T_VOID vtimer_init(T_VOID);
T_TIMER vtimer_start(T_U16 tick, T_BOOL loop, T_fVTIMER_CALLBACK callback_func);
T_TIMER vtimer_stop(T_TIMER timer_id);


#endif // _TIMER_H
