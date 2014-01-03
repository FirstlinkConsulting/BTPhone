#ifndef __FWL_RTC_H__
#define __FWL_RTC_H__



typedef void (*T_fRTC_CALLBACK)(T_VOID);

typedef enum 
{
    RTC_ALARM1 = 0,
    RTC_ALARM2,
    RTC_ALARM_NUM
}RTC_ALARM_TYPE;



#if 0
T_VOID Fwl_RTCInit(T_fRTC_CALLBACK pFuntion);

T_U32 Fwl_GetSecond(T_VOID);

T_VOID Fwl_SetRTCtime(T_SYSTIME *date);

T_VOID Fwl_GetRTCtime(T_SYSTIME *date);

T_BOOL Fwl_SetRTC_AlarmTime(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date);

//#define Fwl_SetSecond(second)      RTC_SetSecond(second)

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
T_BOOL Fwl_GetRTC_FromFile(const T_U16* srcPath, T_SYSTIME *parseDate);
#endif

#define Fwl_RTCInit(pFuntion)  AK_FALSE

#define Fwl_SetRTCtime(date)  AK_FALSE

#define Fwl_GetRTCtime(date)   AK_FALSE

#define  Fwl_SetRTC_AlarmTime(en, type, date) AK_FALSE

#define Fwl_GetRTC_FromFile(srcPath,parseDate) AK_FALSE

T_U32 Fwl_GetSecond(T_VOID);


#endif //__FWL_RTC_H__
