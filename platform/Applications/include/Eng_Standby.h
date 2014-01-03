/**
 * @file Eng_ScreenSave.h
 * @brief This header file is for global data
 * 
 */

#ifndef __ENG_STANDBY_H__
#define __ENG_STANDBY_H__

#include "anyka_types.h"

#define STANDBY_DISABLE_ID  0xFFFF
#define STANDBY_ON_ID       0xFFFE
#define STANDBY_POWER_OFF   0xFDFD
#define STANDBY_TIME        10
#define PRINT_TIMES_SEC     2

T_VOID  UserCountDownReset(T_VOID);

T_VOID CallStandbyEvent(T_U32 StdbyParam);
T_VOID StandbyCountSet(T_U32 count);
T_BOOL StandbyCountDecrease(T_U32 millisecond);
T_VOID StandbyDisable(T_VOID);
T_VOID StandbyEnable(T_VOID);
T_BOOL StandbyIsOn(T_VOID);

#endif /* __ENG_STANDBY_H__ */
