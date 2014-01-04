#include "Eng_Time.h"
#include "Eng_String.h"


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
extern T_U16 WeekDay(T_U16 iYear, T_U16 iMonth, T_U16 iDay);


#if 0
T_VOID	Utl_Convert_DateToString(T_SYSTIME *SysTime, T_pSTR string)
{
    T_S8 *ptr = string;

	Utl_Itoa(SysTime->year, ptr);
	ptr += Utl_StrLen(ptr);
	Utl_StrCpy(ptr, "/");
	ptr += 1;

	Utl_Itoa(SysTime->month, ptr);
	ptr += Utl_StrLen(ptr);
	Utl_StrCpy(ptr, "/");
	ptr += 1;

	Utl_Itoa(SysTime->day, ptr);
	ptr += Utl_StrLen(ptr);

	Utl_StrCpy(ptr, " ");
	ptr += 1;

	Utl_Itoa(SysTime->hour, ptr);
	ptr += Utl_StrLen(ptr);
	Utl_StrCpy(ptr, ":");
	ptr += 1;	

	Utl_Itoa(SysTime->minute, ptr);
	ptr += Utl_StrLen(ptr);
	Utl_StrCpy(ptr, ":");
	ptr += 1;	

	Utl_Itoa(SysTime->second, ptr);
	
    return;
}
#endif

#define BASE_YEAR    1980

const static T_U8 month_std_day[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const static T_U8 month_leap_day[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

T_VOID Utl_Convert_SecondToDate(T_U32 seconds, T_SYSTIME *date)
{
    T_U32 TmpVal;
    T_U32 year, mouth;

    if (AK_NULL == date)
    {
        return;
    }

    TmpVal       = seconds % 60;
    date->second = (T_U8)TmpVal;
    TmpVal       = seconds / 60;
    date->minute = (T_U8)(TmpVal % 60);
    TmpVal       = TmpVal / 60;
    date->hour   = (T_U8)(TmpVal % 24);

    /* day is from the number: 1, so we should add one after the devision. */
    TmpVal  = (seconds / (24 * 3600)) + 1;
    year    = BASE_YEAR;
    while (TmpVal > 365)
    {
        if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
        {
            TmpVal -= 366;
            /* Added to avoid the case of "day==0" */
            if (0 == TmpVal)
            {
                /* resume the value and stop the loop */
                TmpVal = 366;
                break;
            }
        }
        else
        {
            TmpVal -= 365;
        }
        year++;
    }
    
    mouth = 1;
    if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
    {
        while (TmpVal > month_leap_day[mouth])
        {
            TmpVal -= month_leap_day[mouth];
            mouth++;
        }
    }
    else
    {
        /* Dec23,06 - Modified from month_leap_day[], since here is not leap year */
        while (TmpVal > month_std_day[mouth])
        {
            TmpVal -= month_std_day[mouth];
            mouth++;
        }
    }

    date->year  = (T_U16)year;
    date->month = (T_U8)mouth;
    date->day   = (T_U8)TmpVal;
    date->week  = (T_U8)WeekDay((T_U16)year,(T_U8)mouth,(T_U8)TmpVal);
}

T_U32 Utl_Convert_DateToSecond(T_SYSTIME *date)
{
    T_U32 current_time = 0;
    T_U16 MonthToDays  = 0;
    T_U16 std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 TempYear;
    T_U16 month;
    T_U16 i;

    if (AK_NULL == date)
    {
        return 0;
    }
    month = date->month;

    if(date->year < BASE_YEAR) 
        date->year= BASE_YEAR;
    for (TempYear = date->year - 1; TempYear >= BASE_YEAR; TempYear--)
    {
        /* the case of leap year */
        if (0 == (TempYear % 4) && ((TempYear % 100) != 0 || 0 == (TempYear % 400)))
        {
            current_time += 31622400; // the seconds of a leap year
        }
        else
        {
            /* not a leap year */
            current_time += 31536000;  // the seconds of a common year(not a leap one)
        }
    }

    /* calculate the current year's seconds */

    if ((month < 1) || (month > 12))
    {
        /* get the default value. */
        month       = 1;
        date->month = 1;
    }

    /* the current year is a leap one */
    if (0 == (date->year % 4) && ((date->year % 100) != 0 || 0 == (date->year % 400)))
    {
        for (i = 1; i < month; i++)
        {
            MonthToDays += leap_month_days[i];
        }
    }
    else
    {
        /* the current year is not a leap one */
        for (i = 1; i < month; i++)
        {
            MonthToDays += std_month_days[i];
        }
    }

    if ((date->day < 1) || (date->day > 31))
    {
        /* get the default value */
        date->day = 1;
    }
    MonthToDays += (date->day - 1);

    /* added the past days of this year(change to seconds) */
    current_time += MonthToDays * 24 * 3600;

    /* added the current day's time(seconds) */
    current_time += date->hour * 3600 + date->minute * 60 + date->second;

    return current_time;
}

