
#include "akdefine.h"
#include "Fwl_RTC.h"
#include "hal_rtc.h"
#include "Eng_Time.h"

static T_fRTC_CALLBACK Rtc_CallBack = AK_NULL;

T_VOID Fwl_RTCMsgDeal(T_VOID)
{
    if(AK_NULL != Rtc_CallBack)
    {
        Rtc_CallBack();
    }
}

T_U32 Fwl_GetSecond(T_VOID)                                 
{
    T_U32 seconds = 0;

    return seconds;
}

