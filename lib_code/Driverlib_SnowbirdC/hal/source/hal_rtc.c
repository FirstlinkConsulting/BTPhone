/*******************************************************************************
 * @file    hal_rtc.c
 * @brief   rtc driver API
 * This file provides all the APIs RTC Function
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @version 1.0
 * @ref     AK10X6X technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "rtc_device.h"
#include "hal_rtc.h"
#include "arch_init.h"
#include "hal_int_msg.h"
#include "drv_cfg.h"


#if (DRV_SUPPORT_RTC > 0)


#pragma arm section zidata = "_drvbootbss_"
static T_fRTC_CALLBACK rtc_int_callback;
#pragma arm section zidata


#define rtc_send_message()  post_int_message(IM_RTC, 0, 0)

extern T_VOID rtc_alarm_clear_int_sta(T_VOID);


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   rtc device initial
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]pFuntion
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_Init(T_fRTC_CALLBACK pFuntion)
{
    rtc_int_callback = pFuntion;
    rtc_driver_init();
}
#pragma arm section code


/*******************************************************************************
 * @brief   set date from rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_SetTime(T_SYSTIME *date)
{
    if (AK_NULL == date)
    {
        drv_print("set rtc time date = AK_NULL", 0, 1);
        return;
    }

    rtc_driver_set_time(date);
}


#pragma arm section code = "_video_stamp_"
/*******************************************************************************
 * @brief   get date from rtc
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID RTC_GetTime(T_SYSTIME *date)
{
    if (AK_NULL == date)
    {
        drv_print("get rtc time date = AK_NULL", 0, 1);
        return;
    }

    rtc_driver_get_time(date);
}
#pragma arm section code


/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   [in]en: enable or disable
 * @param   [in]type: rtc alarm type
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL RTC_Set_Alarm_Time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date)
{
    if (AK_NULL == date)
    {
        drv_print("set alarm time date = AK_NULL", 0, 1);
        return AK_FALSE;
    }

    return rtc_driver_set_alarm_time(en, type, date);
}


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   rtc regular interrupt handler
 * @author  zhanggaoxin
 * @date    2013-1-8
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_interrupt_handler(T_VOID)
{
    if(rtc_int_callback)
    {
        rtc_int_callback();
    }
    else
    {
        rtc_send_message();
    }
    rtc_alarm_clear_int_sta();
}
#pragma arm section code

#endif

