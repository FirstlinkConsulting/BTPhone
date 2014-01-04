/**
 * @file eng_lunarconvert.h
 * @brief This header file is for debug & trace function prototype
 * @author ANYKA
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 
 * @version 1.0
 */

#ifndef __ENG_LUNARCALENDAR_H__
#define __ENG_LUNARCALENDAR_H__

#include "Gbl_Global.h"

#define START_YEAR  1901
#define END_YEAR    2100

 
#ifdef SUPPORT_CALENDAR

typedef struct 
{
    T_U16   cYear;          
    T_U16   cMonth;     
    T_U16   cDay;   
    T_U16   weekDay;
    T_BOOL  isLeap; 
}T_LUNARCALENDAR,*T_pLUNARCALENDAR;


/**
 * @brief convert the system time to the lunar calendar
 * @author 
 * @date 2009-04-29
 * @param [in]const T_SYSTIME systime
 * @param [out]T_PLUNARCALENDAR lunar
 * @return AK_TRUE if success,  AK_FALSE if failed
 * @remark the year is between 1901 and 2100
 */
T_BOOL Eng_LunarCalendarCovert(const T_SYSTIME systime, T_pLUNARCALENDAR lunar);

#endif

/**
 * @brief check iYear is leap year.
 * @author 
 * @date 2009-04-29
 * @param [in]T_U16 iYear
 * @param [out]T_BOOL
 * @return AK_TRUE--leap year. AK_FALSE --not leep year.
 * @remark the year is between 1901 and 2100
 */
T_BOOL IsLeapYear(T_U16 iYear);

/**
 * @brief get week day.
 * @author 
 * @date 2009-04-29
 * @param [in]T_U16 iYear
 * @param [in]T_U16 iMonth
 * @param [in]T_U16 iDay
 * @param [out]T_U16
 * @return 0~6
 * @remark the year is between 1901 and 2100
 */
T_U16 WeekDay(T_U16 iYear, T_U16 iMonth, T_U16 iDay);

/**
 * @brief get month days.
 * @author 
 * @date 2009-04-29
 * @param [in]T_U16 iYear
 * @param [in]T_U16 iMonth
 * @param [out]T_U16
 * @return idays
 * @remark the year is between 1901 and 2100
 */
T_U16 MonthDays(T_U16 iYear, T_U16 iMonth);
#endif
