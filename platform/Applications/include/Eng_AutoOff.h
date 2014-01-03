/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME Eng_AutoOff.h
 * @BRIEF auto power off and auto background light off
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-30
 * @Version£º
**************************************************************************/

#ifndef __ENG_AUTO_POWER_OFF_H__
#define __ENG_AUTO_POWER_OFF_H__

#include "m_event.h"
//#include "Fwl_System.h"
//#include "log_aud_control.h"

extern T_U16 gb_AutoOffCount;
extern T_U8  gb_BgLightOffCount;
extern T_U8  gb_TimeOutCoutn;
extern T_U16 gb_AutoPOCountSleep;

#define BATTERY_VALUE_CHARGE        4100

#ifndef IC_PACKAGE_BUCK
#define BATTERY_VALUE_SDOWN         1000
#define BATTERY_VALUE_WARN          1050
#define BATTERY_VALUE_MIN           950
#define BATTERY_VALUE_MAX           1400
#define BATTERY_VALUE_TEST          1200
#define BATTERY_VALUE_AVDD          900
#define BATTERY_VALUE_NOR_L1        1100
#define BATTERY_VALUE_NOR_L2        1200
#define BATTERY_VALUE_NOR_L3        1300
#define BATTERY_VALUE_BASE_VAL      250
#else
#define BATTERY_VALUE_SDOWN         3450
#define BATTERY_VALUE_WARN          3550
#define BATTERY_VALUE_MIN           3450
#define BATTERY_VALUE_MAX           3920
#define BATTERY_VALUE_TEST          3700
#define BATTERY_VALUE_AVDD          3300
#define BATTERY_VALUE_NOR_L1        3650
#define BATTERY_VALUE_NOR_L2        3700
#define BATTERY_VALUE_NOR_L3        3800
#define BATTERY_VALUE_BASE_VAL      250
#endif
#define BATTERY_STAT_NULL           0
#define BATTERY_STAT_NOR_L0         1
#define BATTERY_STAT_NOR_L1         2
#define BATTERY_STAT_NOR_L2         3
#define BATTERY_STAT_NOR_L3         4           
#define BATTERY_STAT_NOR_FULL       5
#define BATTERY_STAT_LOW_WARN       6
#define BATTERY_STAT_LOW_SHUTDOWN   7
#define BATTERY_STAT_EXCEEDVOLT     8

#define MASK_BIT_15 (1<<15)
#define MASK_BIT_7  (1<<7)

T_BOOL bglight_state_off(T_VOID);

//void bglight_off(void);

void bglight_on(T_BOOL bDelayEnable);

/************************************************************************
 * @BRIEF enable to auto poweroff
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-30
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
#define AutoPowerOffEnable() {  gb_AutoOffCount |= MASK_BIT_15;\
                                AutoPowerOffCountSet(gb.PoffTime);\
                             }

/************************************************************************
 * @BRIEF disable to auto poweroff
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-30
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
#define AutoPowerOffDisable() {gb_AutoOffCount = 0;}

/************************************************************************
 * @BRIEF set auto power off time
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-30
 * @PARAM T_U16 count :(s)
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
T_VOID AutoPOffCountSetSleep(T_U16 count,T_BOOL bSleep);


#define AutoPowerOffCountSet(count) AutoPOffCountSetSleep(count,0)


#define AutoBgLightOffEnable() {gb_BgLightOffCount |= MASK_BIT_7;}
#define AutoBgLightOffDisable() {gb_BgLightOffCount &= ~MASK_BIT_7;}


#define isBtEnableSendBat(CurBat,PreBat) (((CurBat)>(PreBat)&&(CurBat)>=(PreBat+2))||((CurBat)<(PreBat)&&(CurBat)<=(PreBat-2)))

/************************************************************************
 * @BRIEF set auto background light off time
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-05-03
 * @PARAM T_U8 count :(s)
 * note: the bit 7 is used for a flag , so count must be less than 128
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
T_VOID AutoBgLightOffSet(T_U8 count);


/************************************************************************
 * @BRIEF decrease power off time per second
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-30
 * @PARAM T_VOID
 * @RETURN T_BOOL 
 * @RETVAL If send power off event, return true;
 **************************************************************************/
T_VOID AutoPowerOffCountDecrease(T_VOID);

/************************************************************************
 * @BRIEF decrease background light off time per second
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-05-03
 * @PARAM T_VOID
 * @RETURN T_BOOL 
 * @RETVAL If send background light off event, return true;
 **************************************************************************/
T_BOOL AutoBgLightOffCountDecrease(T_VOID);

T_U8 Eng_VolDetect(T_U8 *Grade);
T_VOID Eng_SetPowerDownVol(T_U32 vol);

/************************************************************************
 * @BRIEF get current volt_val
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL current volt
 **************************************************************************/
T_U32 get_cur_volt_val(T_VOID);

#endif /* __ENG_AUTO_POWER_OFF_H__ */

