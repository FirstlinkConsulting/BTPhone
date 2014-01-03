/**
 * @file Eng_Time.h
 * @brief This header file is for time process and format transfer function prototype
 * 
 */

#ifndef __ENG_TIME_H__
#define __ENG_TIME_H__ 

#include "anyka_types.h"
//#include "Gbl_Global.h"

#define MAX_TIME_LEN        40
typedef T_S8    T_STR_TIME[MAX_TIME_LEN+1];

#define INIT_YEAR_BEG       1980
typedef struct 
{
    T_U16   year;           /* 4 byte: 1-9999 */
    T_U8    month;          /* 1-12 */
    T_U8    day;            /* 1-31 */
    T_U8    hour;           /* 0-23 */
    T_U8    minute;         /* 0-59 */
    T_U8    second;         /* 0-59 */
    T_U8    week;           /* 0-6, 0: sunday, 1: monday */
} T_SYSTIME, *T_pSYSTIME;   /* system time structure */ 
T_VOID  Utl_Convert_SecondToDate(T_U32 seconds, T_SYSTIME *date);
T_U32   Utl_Convert_DateToSecond(T_SYSTIME *date);
T_VOID  Utl_Convert_DateToString(T_SYSTIME *SysTime, T_pSTR string);


#endif
