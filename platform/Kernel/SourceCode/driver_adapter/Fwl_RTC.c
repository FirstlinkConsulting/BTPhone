
#include "akdefine.h"
#include "Fwl_RTC.h"
#include "hal_rtc.h"
#include "Eng_Time.h"


typedef enum
{
    E_PARSE_YEAR,
    E_PARSE_MONTH,
    E_PARSE_DAY,
    E_PARSE_HOUR,
    E_PARSE_MINUTE,
    E_PARSE_SECOND,
    E_PARSE_NUM
}E_PARSE_SYSTIME;

#define RTC_STRING_LEN   30
#define RTC_YEAR_LEN     4 

#define CHECK_NUM(x)        ((x)>=('0')&&(x)<=('9'))
#define TRANSFORM_NUM(x)    ((x)-('0'))

//static T_BOOL Fwl_ParseFile(T_hFILE fd, T_SYSTIME *date);
extern T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline);

#pragma arm section code = "_sysinit_"

static T_fRTC_CALLBACK Rtc_CallBack = AK_NULL;

T_VOID Fwl_RTCMsgDeal(T_VOID)
{
    if(AK_NULL != Rtc_CallBack)
    {
        Rtc_CallBack();
    }
}

#if 0

T_VOID Fwl_RTCInit(T_fRTC_CALLBACK pFuntion)
{
    Rtc_CallBack = pFuntion;
    RTC_Init(AK_NULL);
}

T_U32 Fwl_GetSecond(T_VOID)                                 
{
    T_SYSTIME date;
    T_U32 seconds = 0;

    RTC_GetTime(&date);

    seconds = Utl_Convert_DateToSecond(&date);
    return seconds;
}

#pragma arm section code 

T_VOID Fwl_SetRTCtime(T_SYSTIME *date)                            
{
    RTC_SetTime(date);
}

#pragma arm section code = "_video_stamp_"

T_VOID Fwl_GetRTCtime(T_SYSTIME *date)                            
{
    RTC_GetTime(date);
}
#pragma arm section code

T_BOOL Fwl_SetRTC_AlarmTime(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date)
{
#ifdef OS_ANYKA
    return RTC_Set_Alarm_Time(en, type, date);
#else 
    return AK_TRUE;
#endif
}

/***********************************************************************************
FUNC NAME: Fwl_GetRTC_FromFile
DESCRIPTION: 
        read the character string from the file, 
        and then parse parameters and set to the RTC.
INPUT:  srcPath = string of Path
OUTPUT: parseDate = systime from file.
RETURN: ture = set success. false = set false.
AUTHOR: liangxiong
CREATE DATE: 2011.11.9
MODIFY LOG:
***********************************************************************************/
T_BOOL Fwl_GetRTC_FromFile(const T_U16* srcPath, T_SYSTIME *parseDate)
{
#ifdef OS_ANYKA
    T_hFILE file = FS_INVALID_HANDLE;

    if (AK_NULL == srcPath)
    {
        akerror("RTC: Error Str is Null.",0,1);
        return AK_FALSE;
    }
    
    file = File_OpenUnicode(AK_NULL, srcPath, FILE_MODE_READ);
    if (!File_Exist(file))
    {
        /* Whatever mode you open the file, it fails when it doesn't exist. */
        File_Close(file);
        akerror("RTC: The Rtc file is not exit.",0,1);
        return AK_FALSE;
    }

    if (File_IsFolder(file))
    {
        File_Close(file);
        akerror("RTC: The path is not a file.",0,1);
        return AK_FALSE;
    }

    if (Fwl_ParseFile(file, parseDate))
    {
        akerror("RTC: year 0x",parseDate->year,0);
        akerror(" month 0x",parseDate->month,0);
        akerror(" day 0x",parseDate->day,1);
        akerror("RTC: hour 0x",parseDate->hour,0);
        akerror(" minute 0x",parseDate->minute,0);
        akerror(" second 0x",parseDate->second,1);
        File_Close(file);
        return AK_TRUE;
    }
    else
    {
        File_Close(file);
        akerror("RTC: The Rtc file is Parsed error.",0,1);
        return AK_FALSE;
    }
#else 
    return AK_FALSE;
#endif
}

/***********************************************************************************
FUNC NAME: Fwl_ParseFile
DESCRIPTION: 
    setting mode as this ¡°2011/10/13 16:00:00¡±
    parse the rtc parameters.
INPUT:  fd = file of time.
OUTPUT: date = time param.
RETURN: ture = parse success. false = parse false.
AUTHOR: liangxiong
CREATE DATE: 2011.11.9
MODIFY LOG:
***********************************************************************************/
static T_BOOL Fwl_ParseFile(T_hFILE fd, T_SYSTIME *date)
{
#ifdef OS_ANYKA
    T_U16 std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 *pMouth = AK_NULL;
    T_U8 buffer[RTC_STRING_LEN];
    T_U8 str[RTC_YEAR_LEN+1];
    T_U8 i = 0;
    T_U8 j = 0;
    T_U8 strlen = 0;
    T_U32 num = 0;
    E_PARSE_SYSTIME parse = E_PARSE_YEAR;
    T_BOOL error = AK_FALSE;

    if (File_Read(fd, buffer, RTC_STRING_LEN))
    {
        for (i=0, strlen=0; i<RTC_STRING_LEN; i++)
        {
            //akerror(" chr:",0,0);
            str[strlen] = buffer[i];
            
            if (strlen > RTC_YEAR_LEN)
            {
                akerror("RTC: num is error.",0,1);
                return AK_FALSE;
            }
            else if (!CHECK_NUM(buffer[i]))
            {
                if (!strlen)
                {
                    akerror("RTC: num is null.",0,1);
                    return AK_FALSE;
                }

                for (j=0,num=0; j<strlen; j++)
                {
                    num = 10*num + TRANSFORM_NUM(str[j]);
                }
                //akerror(" num",num,0);
                switch(parse)
                {
                    case E_PARSE_YEAR:
                        if ((num >= 1980) && str[strlen]=='/')
                        {
                            date->year = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                    case E_PARSE_MONTH:
                        if ((num > 0) && (num <= 12) && str[strlen]=='/')
                        {
                            date->month = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                    case E_PARSE_DAY:
                        /* the case of leap year */
                        if (0 == (date->year % 4) && ((date->year % 100) != 0 || 0 == (date->year % 400)))
                        {
                            pMouth = leap_month_days;
                        }
                        else
                        {
                            pMouth = std_month_days;
                        }
                        if ((num > 0) && (num <= pMouth[date->month]) && str[strlen]==' ')
                        {
                            date->day = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                    case E_PARSE_HOUR:
                        if ((num <= 24) && str[strlen]==':')
                        {
                            date->hour = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                    case E_PARSE_MINUTE:
                        if ((num <= 60) && str[strlen]==':')
                        {
                            date->minute = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                    case E_PARSE_SECOND:
                        if (num <= 60)
                        {
                            date->second = num;
                        }
                        else
                        {
                            error = AK_TRUE;
                        }
                        break;
                }
                parse++;
                strlen = 0;
            }
            else
            {
                strlen++;
            }
            
            if (parse == E_PARSE_NUM)
            {
                akerror("RTC: Fwl_ParseFile is success.",0,1);
                return AK_TRUE;
            }
            if (error)
            {
                akerror("RTC: Parse error.",0,1);
                return AK_FALSE;
            }
        }
    }
    akerror("RTC: The Rtc file is read error.",0,1);
    return AK_FALSE;
#else 
    return AK_FALSE;
#endif
}

//#define Fwl_SetSecond(second) 
#else
T_U32 Fwl_GetSecond(T_VOID)                                 
{
    T_U32 seconds = 0;

    return seconds;
}

#endif

