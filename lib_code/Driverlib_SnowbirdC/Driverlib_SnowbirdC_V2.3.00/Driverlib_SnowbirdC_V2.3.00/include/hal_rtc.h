/*******************************************************************************
 * @file    hal_rtc.h
 * @brief   rtc driver API
 * This file provides all the APIs RTC Function
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @version 1.0
 * @ref     AK10X6X technical manual.
*******************************************************************************/
#ifndef __HAL_RTC_H__
#define __HAL_RTC_H__


#include "anyka_types.h"


typedef T_VOID (*T_fRTC_CALLBACK)(T_VOID);

typedef enum
{
    RTC_ALARM1 = 0,
    RTC_ALARM2,
    RTC_ALARM_NUM
} RTC_ALARM_TYPE;

typedef struct 
{
    T_U16   year;           /* 2 byte: 1-9999 */
    T_U8    month;          /* 1-12 */
    T_U8    day;            /* 1-31 */
    T_U8    hour;           /* 0-23 */
    T_U8    minute;         /* 0-59 */
    T_U8    second;         /* 0-59 */
    T_U8    week;           /* 0-6, 0: sunday, 1: monday */
} T_SYSTIME, *T_pSYSTIME;   /* system time structure */ 


/*******************************************************************************
 * @brief   rtc device initial
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]pFuntion
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_Init(T_fRTC_CALLBACK pFuntion);


/*******************************************************************************
 * @brief   set date from rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_SetTime(T_SYSTIME *date);


/*******************************************************************************
 * @brief   get date from rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_GetTime(T_SYSTIME *date);


/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]en: enable or disable
 * @param   [in]type: rtc alarm type
 * @param   [in]date: the buffer of a structural which include 
 *                        second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL RTC_Set_Alarm_Time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date);


#endif //__HAL_RTC_H__

