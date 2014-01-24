/**
 * @FILENAME: s_pub_usbdisk.h
 * @BRIEF usbdisk state machine
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR Justin.Zhao
 * @DATE 2008-4-22
 * @VERSION 1.0
 * @REF
 */
#include "Apl_Public.h"
#include "Gbl_resource.h"
#include "Eng_ImageResDisp.h"
#include "Eng_Font.h"
#include "Fwl_FreqMgr.h"
#include "eng_autooff.h"
#include "Eng_USB.h"
#include "Gbl_Global.h"
#include "Log_aud_control.h"
#include "Fwl_RTC.h"
#include "Eng_Time.h"
#include "Fwl_detect.h"
#include "Fwl_usb_s_state.h"
#include "Eng_LedHint.h"
#include "Fwl_LCD.h"
#include "M_event_api.h"

#if (USE_COLOR_LCD)
    #define PIC_X               0
    #define PIC_Y               0
#else
    #define PIC_X               15
    #define PIC_Y               24
#endif

#if (STORAGE_USED == NAND_FLASH)
const T_U16 s_RtcPath[]= {'B',':','/','t','i','m','e','.','t','x','t','\0'};
#else
const T_U16 s_RtcPath[]= {'A',':','/','t','i','m','e','.','t','x','t','\0'};
#endif

static T_BOOL usbstate;//  

void usbdisk_changeRtc(void)
{
    T_SYSTIME date;
    T_SYSTIME limitDate;
    T_U32 senson;
        
    // 退出 U盘读取文件 time.txt更新 RTC参数
    if (Fwl_GetRTC_FromFile(s_RtcPath, &date))
    {
        SetMinDate(limitDate);
        if (limitDate.year > date.year)
        {
            AK_DEBUG_OUTPUT("UDK: Error time.year:%d older limitDate:%d.\n", 
                date.year, limitDate.year);
            return;
        }
        
        SetMaxDate(limitDate);
        if (limitDate.year < date.year)
        {
            AK_DEBUG_OUTPUT("UDK: Error time.year:%d over limitDate:%d.\n", 
                date.year, limitDate.year);
            return;
        }
        
        senson = Utl_Convert_DateToSecond(&date);
        if (senson >= gb.TickMin && senson <= gb.TickMax)
        {
            Fwl_SetRTCtime(&date);
            if (Fwl_FileDelete(s_RtcPath))
            {
                AK_DEBUG_OUTPUT("UDK: Delete file of time.txt success.\n");
            }
            else
            {
                AK_DEBUG_OUTPUT("UDK: Delete file of time.txt false.\n");
            }
        }
        else
        {
            AK_DEBUG_OUTPUT("UDK: Error time:%x over limit min:%x max:%x.\n", 
                senson, gb.TickMin, gb.TickMax);
        }
    }
}

void initpub_usbdisk(void)
{
    Fwl_FreqPush(FREQ_APP_USB);    

    usbstate = AK_TRUE;
    Fwl_LCD_on(AK_TRUE);
#ifdef SUPPORT_LEDHINT
    LedHint_Exec(LED_USB);
#endif
}

void exitpub_usbdisk(void)
{
    AutoBgLightOffSet(gb.BgLightTime);
    Fwl_FreqPop();
    usbstate = AK_FALSE;
#ifdef OS_ANYKA 
    if (AK_FALSE == gb.power_on)
    {        
        VME_EvtQueuePut(M_EVT_Z00_POWEROFF, AK_NULL);
    }
#endif

    usbdisk_changeRtc();
#ifdef SUPPORT_LEDHINT
    LedHint_Stop(LED_USB);
#endif
}

void paintpub_usbdisk(void)
{
    if (usbstate)
    {
        usbstate = AK_FALSE;

    #ifdef OS_ANYKA
        usbdisk_main();
        AutoBgLightOffSet(gb.BgLightTime);
        AutoPowerOffCountSet(gb.PoffTime);
        AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);
        usb_using_exit_handle();
    #endif
        
    }
}

unsigned char handlepub_usbdisk(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (!usbstate)
    {
        if (event == M_EVT_PUB_TIMER || event == M_EVT_USER_KEY)
        {
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        }
    }
    return 0;
}


/* end of files */
