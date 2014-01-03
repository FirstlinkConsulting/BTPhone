#include "Apl_Public.h"
#include "Eng_Profile.h"
#include "Fwl_System.h"
#include "Eng_Font.h"
#include "Gbl_ImageRes.h"
#include "Eng_ImageResDisp.h"
#include "Fwl_Timer.h"
#include "Fwl_Keypad.h"
#include "Eng_AutoOff.h"
#include "Eng_Usb.h"
#include "log_radio_core.h"
#include "Fwl_LCD.h"
#include "Log_aud_control.h"
#include "Eng_Standby.h"
#include "Alarm_Common.h"
#if(USE_ALARM_CLOCK)
#include "AlarmClock.h"
#endif
#include "Fwl_Detect.h"
#include "Fwl_usb_s_state.h"
#include "Eng_LedHint.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"

#define MAX_POFF_PIC    14
#define DISP_LOWBAT_POS_X 9
#define DISP_LOWBAT_POS_Y 46

#define SUSPEND_DELAY   2   

#if(NO_DISPLAY == 0)
#if (STORAGE_USED != SPI_FLASH)
static const T_RES_IMAGE poff_img[MAX_POFF_PIC] = {
    eRES_IMAGE_POWEROFF01, eRES_IMAGE_POWEROFF02, eRES_IMAGE_POWEROFF03, eRES_IMAGE_POWEROFF04,
    eRES_IMAGE_POWEROFF05, eRES_IMAGE_POWEROFF06, eRES_IMAGE_POWEROFF07, eRES_IMAGE_POWEROFF08,
    eRES_IMAGE_POWEROFF09, eRES_IMAGE_POWEROFF10, eRES_IMAGE_POWEROFF11, eRES_IMAGE_POWEROFF12,
    eRES_IMAGE_POWEROFF13, eRES_IMAGE_POWEROFF14
};
#endif
#endif

typedef struct _SwitchOff
{
#if(NO_DISPLAY == 0)
    T_U8        show_step;
    T_TIMER      timer_id;
    T_BOOL       bSendAlarmPlay;
#else
    T_BOOL  suspend;
    T_U8    delay;
    T_U8    key_cnt;
#endif
}T_SWITCHOFF;

static T_SWITCHOFF *pPowerOff = AK_NULL;
#ifdef SUPPORT_USBHOST
static T_BOOL start_host;
#endif

extern T_VOID bglighton_freq_set(T_VOID);
extern T_U8 USB_Init(T_VOID);

void suspend_arm()
{
    T_SYSTEM_CFG syscfg;

#if(NO_DISPLAY == 0)
    #if(USE_ALARM_CLOCK)
        AlmClk_DealAlarm(DEAL_POWEROFF, AK_NULL);
    #endif
    if (pPowerOff->timer_id != ERROR_TIMER)
    {
        Fwl_TimerStop(pPowerOff->timer_id);
        pPowerOff->timer_id = ERROR_TIMER;
    }  
#endif
    Profile_ReadData(eCFG_SYSTEM, &syscfg);
    AK_DEBUG_OUTPUT("power off flag %d\n", syscfg.PowerOffFlag);
    if(POWEROFF_ABNORMAL == syscfg.PowerOffFlag)
    {
        syscfg.PowerOffFlag = POWEROFF_NORMAL;
        Profile_WriteData(eCFG_SYSTEM, &syscfg);
        // FlushUserdata();                            //because  system will go to sleep, so data must be write back 
    }   
    
    Fwl_SysPowerOff();
    AutoBgLightOffDisable();

#if(NO_DISPLAY == 0)
    pPowerOff->show_step = MAX_POFF_PIC;
#else
    pPowerOff->delay = 0;
    pPowerOff->key_cnt = 0;
#endif

}

void initpub_switch_off(void)
{
#ifdef SUPPORT_LEDHINT
    if(Fwl_DetectorGetStatus(DEVICE_CHG))
    {
        LedHint_Exec(LED_USB);
    }
    else
    {
        LedHint_Exec(LED_OFF);
    }
#endif
    AK_DEBUG_OUTPUT("initpub_switch_off\n");
    
    gb.init = SYSTEM_STATE_POWEROFF;
    gb.PoffTimeSleepMode = 0;
    pPowerOff = (T_SWITCHOFF*)Fwl_Malloc(sizeof(T_SWITCHOFF));
#if(NO_DISPLAY == 0)
    pPowerOff->show_step = 0;
    pPowerOff->timer_id = ERROR_TIMER;
    pPowerOff->bSendAlarmPlay= AK_FALSE;
#else
    pPowerOff->suspend = AK_FALSE;
    GblSaveSystime();
#endif
    Fwl_LCD_lock(AK_FALSE);
    if (bglight_state_off())
    {
        bglighton_freq_set();
    }
    AutoBgLightOffDisable();
    AutoPowerOffDisable();
    StandbyDisable();
#ifdef SUPPORT_USBHOST
    Fwl_DetectorEnable(DEVICE_UHOST, AK_FALSE);
#endif
    if (gb.power_on)
    {
        gb.power_on = AK_FALSE;
    }
#if(USE_ALARM_CLOCK)
    AlmClk_DealAlarm(DEAL_POWEROFF, AK_NULL);
#endif
}

void exitpub_switch_off(void)
{
#ifdef SUPPORT_LEDHINT
    if(Fwl_DetectorGetStatus(DEVICE_CHG))
    {
        LedHint_Stop(LED_USB);
    }
    else
    {
        LedHint_Stop(LED_OFF);
    }
#endif
    AK_DEBUG_OUTPUT("exitpub_switch_off\n");
#if(NO_DISPLAY == 0)    
    if (pPowerOff->timer_id != ERROR_TIMER)
    {
        Fwl_TimerStop(pPowerOff->timer_id);
        pPowerOff->timer_id = ERROR_TIMER;
    }
#endif
    pPowerOff = Fwl_Free(pPowerOff);
    pPowerOff = AK_NULL;
    Fwl_LCD_lock(AK_FALSE);
    AutoBgLightOffEnable();
    AutoPowerOffEnable();
    StandbyEnable();
#ifdef SUPPORT_USBHOST
    if (start_host)
    {
        Fwl_DetectorEnable(DEVICE_UHOST, AK_TRUE);
    }
#endif
    gb.init = SYSTEM_STATE_NORMAL;
}

void paintpub_switch_off(void)
{
}

#if(NO_DISPLAY == 0)
unsigned char handlepub_switch_off(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad;
    AK_DEBUG_OUTPUT("handlepub_switch_off event:%x\n", event);

    if (M_EVT_Z00_POWEROFF == event && 0 == pPowerOff->show_step)
    {
        GblSaveSystime();
        if (AK_NULL != pEventParm)
        {
            if (EVENT_TYPE_SWITCHOFF_BATLOW == pEventParm->w.Param1)
            {
                // when in switch off ,not disp the eRES_STR_SYS_BAT_LOW
                if (ERROR_TIMER == pPowerOff->timer_id)
                {
                #if (USE_COLOR_LCD)
                    T_BG_PIC pic;
                    T_U32 i;
                    T_U16 imgWidth= Eng_GetResImageWidth(eRES_IMAGE_IMG_ERROR);
                    T_U16 imgHeight= Eng_GetResImageHeight(eRES_IMAGE_IMG_ERROR);
                    T_U16 x= (T_U16)((GRAPH_WIDTH- imgWidth)>>1);
                    T_U16 y= (T_U16)((GRAPH_HEIGHT- imgHeight)>>1);
                    i = GetStringDispWidth(CP_UNICODE, CONVERT2STR(eRES_STR_SYS_BAT_LOW), 0);
                    i = ((i >= Eng_GetResImageWidth(eRES_IMAGE_IMG_ERROR)) ? 0: ((Eng_GetResImageWidth(eRES_IMAGE_IMG_ERROR) - i)>>1)); 
                    #if (!(3 == LCD_TYPE && 1 == LCD_HORIZONTAL))
                    Fwl_FillRect(0, 0, GRAPH_WIDTH,GRAPH_HEIGHT, CLR_BLACK);
                    #endif
                    Eng_ImageResDisp(x, y, eRES_IMAGE_IMG_ERROR, AK_FALSE);

                    pic.hOffset = (T_U8)i;
                    #if (3 == LCD_TYPE && 1 == LCD_HORIZONTAL)                              
                    pic.vOffset = FONT_HEIGHT* 8;
                    #else   
                    pic.vOffset = FONT_HEIGHT* 2+ 7;
                    #endif
                    
                    pic.resId   = eRES_IMAGE_IMG_ERROR;
                    DispStringOnPic(CP_UNICODE, (T_POS)(x+pic.hOffset), (T_POS)(y +pic.vOffset), \
                    CONVERT2STR(eRES_STR_SYS_BAT_LOW), (T_U8)(Eng_GetResImageWidth(eRES_IMAGE_IMG_ERROR) - i), CLR_BLACK,&pic);
                #else
                    Fwl_FillRect(0, 0, GRAPH_WIDTH,GRAPH_HEIGHT, CLR_BLACK);
                    DispStringInWidth(CP_UNICODE, (T_U8)(GRAPH_WIDTH/2-32), (T_U8)(GRAPH_HEIGHT/2-8), 
                    CONVERT2STR(eRES_STR_SYS_BAT_LOW), FONT_WND_WIDTH, CLR_WHITE);  
                #endif
                }
                Fwl_DelayUs(2000000);
            }
        }
        
        if (ERROR_TIMER == pPowerOff->timer_id)
        {
            AK_DEBUG_OUTPUT("**Start timer\r\n");
            pPowerOff->timer_id = Fwl_TimerStartMilliSecond(150, AK_TRUE);
        }
    }

    switch (event)
    {
    case VME_EVT_TIMER:
        if (pPowerOff->timer_id == (T_TIMER)pEventParm->w.Param1)
        {
            if (!pPowerOff->show_step)
            {
                //AK_DEBUG_OUTPUT("black\r\n");
                #if (!USE_COLOR_LCD)
                Fwl_FillRect(0, 0, GRAPH_WIDTH,GRAPH_HEIGHT, CLR_BLACK);
                #endif
            }

            if (pPowerOff->show_step < MAX_POFF_PIC)
            {
                if(pPowerOff->show_step == 0)
                    Fwl_LCD_on(AK_TRUE);
            #if (STORAGE_USED == SPI_FLASH)
                pPowerOff->show_step = MAX_POFF_PIC;
            #else
                Eng_ImageResDisp(0, POFF_PIC_POS_Y, poff_img[pPowerOff->show_step], AK_FALSE);
            #endif
            }

            if (pPowerOff->show_step >= MAX_POFF_PIC)
            {                   
                Fwl_FillRect(0, 0, GRAPH_WIDTH,GRAPH_HEIGHT, CLR_BLACK);
                Fwl_LCD_off();
                if (pPowerOff->timer_id != ERROR_TIMER)
                {
                    Fwl_TimerStop(pPowerOff->timer_id);
                    pPowerOff->timer_id = ERROR_TIMER;
                }
                //VME_Terminate();
                if (!pPowerOff->bSendAlarmPlay)
                {
                    AK_DEBUG_OUTPUT("***VME_TIMER Suspend\n");
                    // FlushUserdata();
                    if (Fwl_DetectorGetStatus(DEVICE_USB))
                    {
                        VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
                        USB_Init();
                    }
                    else
                        suspend_arm();
                    // 清除关机前保留 M_EVT_PUB_TIMER 事件消息。
                    VME_EvtQueueClearTimerEventEx(M_EVT_PUB_TIMER);
                }
            }
            else
                pPowerOff->show_step++;
        }
        break;
    case M_EVT_USER_KEY: 
        keyPad.id = pEventParm->c.Param1;
        keyPad.pressType = pEventParm->c.Param2;
        //关机过程不对OK键进行处理
        /*
                if (pPowerOff->show_step < MAX_POFF_PIC
                    && PRESS_SHORT == keyPad.pressType
                    && keyPad.keyID == kbOK)
                    pPowerOff->show_step = MAX_POFF_PIC;
        */

        if (pPowerOff->show_step > MAX_POFF_PIC
            && keyPad.id == kbPOWER)
        {
            pPowerOff->show_step = MAX_POFF_PIC;
        }
        break;
    case M_EVT_Z00_POWEROFF:
    //case M_EVT_USBDISK:
    //case M_EVT_CHARGER:
    case M_EVT_USB_DETECT:
        if (pPowerOff->show_step >= MAX_POFF_PIC)
        {
            AK_DEBUG_OUTPUT("***M_EVT_RESTART\r\n");
            if (M_EVT_Z00_POWEROFF == event 
                && (IsAudplayer()||IsInRadio() || IsInVoicePlay()))
            {
                AK_DEBUG_OUTPUT("*enter audio\n");
                Aud_AudCtrlSetStatJump(JUMP_SWITCH_OFF);
            }
            VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
        }
        break;
    case M_EVT_Z01_MUSIC_PLAY:
        if (IsAudplayer() || IsInRadio() || IsInVoicePlay())
        {
            AK_DEBUG_OUTPUT("*enter audio\n");
            Aud_AudCtrlSetStatJump(JUMP_SWITCH_OFF);
        }
        VME_EvtQueuePut(M_EVT_ALARM_PLAY, pEventParm);
        pPowerOff->bSendAlarmPlay = AK_TRUE;
        AK_DEBUG_OUTPUT("***Send Alarm play\n");
        break;
    case M_EVT_PUB_TIMER:            
        if (pPowerOff->show_step >= MAX_POFF_PIC)
        {
            pPowerOff->show_step++;
            if (pPowerOff->show_step > MAX_POFF_PIC+2)
            {
                AK_DEBUG_OUTPUT("**PUB_TIMER Suspend\n");
                if(!pPowerOff->bSendAlarmPlay)
                {
                    suspend_arm();
                }
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

#else
unsigned char handlepub_switch_off(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("handlepub_switch_off event:%x\n", event);
    
    if((M_EVT_Z00_POWEROFF == event) && (EVENT_TYPE_SWITCHOFF_BATLOW == pEventParm->w.Param1))
    {
        #ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_LOWESTPOWER, AK_NULL);
        Voice_WaitTip();
        #endif
    }
    if(pPowerOff->suspend == AK_FALSE)  
    {
        if(M_EVT_PUB_TIMER == event)
        {
            if(pPowerOff->delay++ >= SUSPEND_DELAY)
            {
                AK_DEBUG_OUTPUT("suspend_arm\n");
                // FlushUserdata();

                pPowerOff->suspend = AK_TRUE;
                suspend_arm();
            }
        }
        else if(M_EVT_USER_KEY == event)
        {
            pPowerOff->delay = 0;
        }
    }
    else 
    {
        if(M_EVT_USER_KEY == event)
        {
            T_PRESS_KEY keyPad;
            keyPad.id = pEventParm->c.Param1;
            keyPad.pressType = pEventParm->c.Param2;

            pPowerOff->delay = 0;
            
            AK_DEBUG_OUTPUT("\nkeyPad id=%x, Type=%x, cnt:%x\n",
                            keyPad.id, keyPad.pressType, pPowerOff->key_cnt);
            
            //count 10 times, power on by press 3 seconds.
            if (kbPOWER == keyPad.id && 
                ((PRESS_LONG == keyPad.pressType)||(PRESSING == keyPad.pressType)))
            {
                if (5 <= pPowerOff->key_cnt++)
                {
                    if (IsAudplayer())
                    {
                        AK_DEBUG_OUTPUT("*enter audio\n");
                        Aud_AudCtrlSetStatJump(JUMP_SWITCH_OFF);          
                    }
                    AK_DEBUG_OUTPUT("***M_EVT_RESTART\r\n");
                    VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
                    pPowerOff->suspend = AK_FALSE;
                    gb.power_on = AK_TRUE;
                }
            }
            else 
            {
                suspend_arm();
            }
        }
        //此处是用于防止按键抖动唤醒却并未形成按键事件的情况
        else if(M_EVT_PUB_TIMER == event 
                && SUSPEND_DELAY <= pPowerOff->delay++)
        {
            if (Fwl_DetectorGetStatus(DEVICE_CHG))
            {
                if(Fwl_UsbSlaveDetect())
                {
                    VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
                    USB_Init();
#ifdef SUPPORT_USBHOST
                    start_host = AK_FALSE;
#endif
                    return 0;
                }
            }

            suspend_arm();
        }
    }
#ifdef SUPPORT_USBHOST
    start_host = AK_TRUE;
#endif
    return 0;
}

#endif
/* end of files */
