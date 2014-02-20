/*******************************************************************************
 * @file    rtc_device.h
 * @brief   the driver head of rtc module
 * Copyright (C) 2013 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @version 1.0
*******************************************************************************/
#ifndef __RTC_DEVICE_H__
#define __RTC_DEVICE_H__


#include "anyka_types.h"
#include "hal_rtc.h"


/*******************************************************************************
 * @brief   rtc device initial
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_init(T_VOID);


/*******************************************************************************
 * @brief   get date from rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_get_time(T_SYSTIME *date);


/*******************************************************************************
 * @brief   set date to rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_set_time(T_SYSTIME *date);


/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]en: enable or disable
 * @param   [in]type: type of alarm
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL rtc_driver_set_alarm_time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date);


#endif

