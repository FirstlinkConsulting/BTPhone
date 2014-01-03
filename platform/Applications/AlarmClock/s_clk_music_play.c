/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF alarm play UI
 * @Author：zhao_xiaowei 
 * @Date： 2009-8
 * @Version：
**************************************************************************/
#include "Gbl_Global.h"
#include "Apl_Public.h"
#include "m_event.h"
#include "Gbl_Resource.h"
#include "Alarm_Common.h"
#include "AlarmClock.h"
#include "log_aud_control.h"
#include "Eng_AutoOff.h"
#include "Gui_Common.h"
#include "Eng_USB.h"
#include "Fwl_Timer.h"
#include "Fwl_osFS.h"
#include "log_aud_play.h"
#include "Gbl_ImageRes.h"
#include "log_radio_core.h"
#include "M_event_api.h"
#include "Eng_DataConvert.h"


#if(NO_DISPLAY == 0)

#if(USE_ALARM_CLOCK)
#define    BK_IMAGE             eRES_IMAGE_CLOCK
#if(LCD_TYPE == 3)
#if( LCD_HORIZONTAL == 1)
    #define TOP_OFFSET          13
    #define LEFT_OFFSET         67
#else
#define TOP_OFFSET              58
#define LEFT_OFFSET             28
#endif
#else
#define TOP_OFFSET              28
#define LEFT_OFFSET             14
#endif
//启动1S钟的Timer，防止Timer丢失

#define   REFRESH_ALL               0xFFFFFFFF
#define   REFRESH_NONE              0x00000000
#define   REFRESH_BACK              0x00000001
#define   BACK_COUNT               (sizeof(m_backImage)/sizeof(T_RES_IMAGE))

#define   ALARM_FIDEIN_TIME         150
#define   ALARM_FIDEOUT_TIME        150

typedef struct  {
    T_U32        refreshFlag;
    T_AlarmPlayParam alarmPlayParam;
    T_BOOL       isToExit;
    T_EVT_CODE    evtCode;
    T_EVT_PARAM   evtParam;
    T_eClockEnd   eClockEnd;
    T_U16         secCount;
    T_EVT_CODE    playType;
    T_U8          foucsWhat;
    T_U8          isTrigExit;
    T_BOOL        hasStartPlay;//
    T_U8          index;
    T_eAlarmDealType eAlarmDealType;
}T_ClkMusicPlay;

extern T_VOID AlmClk_SetCpu2xManual(T_BOOL bSet);
static T_ClkMusicPlay *m_pClkMusicPlay= AK_NULL;
extern T_VOID ImgError_Paint(T_RES_STRING resID);

#if (STORAGE_USED == SPI_FLASH)
static const T_RES_IMAGE m_backImage[]= {
eRES_IMAGE_CLOCK2, 
};
#else
static const T_RES_IMAGE m_backImage[]= {
eRES_IMAGE_CLOCK1,
eRES_IMAGE_CLOCK2, 
eRES_IMAGE_CLOCK3
#if(!USE_COLOR_LCD)
,eRES_IMAGE_CLOCK4
#endif
};
#endif

#ifdef SUPPORT_SDCARD
extern T_MEM_DEV_ID VoiceGetCurDriver(T_VOID);
#endif


#endif
extern void paintclk_music_play(void);
void initclk_music_play(void)
{
#if(USE_ALARM_CLOCK)
    m_pClkMusicPlay= (T_ClkMusicPlay *)Fwl_Malloc(sizeof(T_ClkMusicPlay));
    AK_ASSERT_PTR_VOID(m_pClkMusicPlay, "initclk_music_play is AK_NULL");

    m_pClkMusicPlay->secCount= 0;
    m_pClkMusicPlay->refreshFlag= REFRESH_ALL;
    m_pClkMusicPlay->isToExit= AK_FALSE;
    m_pClkMusicPlay->eClockEnd= CLOCK_PAUSE;
    m_pClkMusicPlay->foucsWhat= 0;
    m_pClkMusicPlay->hasStartPlay= AK_FALSE;
    if (bglight_state_off())
    {
        bglight_on(AK_TRUE);
    }
    AutoBgLightOffDisable();
    AutoPowerOffDisable();
    m_pClkMusicPlay->evtCode= 0;
    m_pClkMusicPlay->isTrigExit= AK_FALSE;
    Fwl_FreqPush(FREQ_APP_AUDIO_L4);    
    AK_DEBUG_OUTPUT("***alarm play!\n");
    
    paintclk_music_play();
#endif

}

void exitclk_music_play(void)
{
#if (USE_ALARM_CLOCK)
    if(M_EVT_Z01_MUSIC_PLAY == m_pClkMusicPlay->playType)
    {
        AlmClk_DealPlayEnd(AK_NULL, m_pClkMusicPlay->eClockEnd);
    }    
    AlmClk_PlayerDestroy();
    AutoBgLightOffEnable();
    AutoPowerOffEnable();
    Fwl_Free(m_pClkMusicPlay);

    #ifdef SUPPORT_SDCARD
    #ifdef OS_ANYKA 
    {
        if(Fwl_DetectorGetStatus(DEVICE_SD))
        {
            Fwl_MemDevMount(MMC_SD_CARD);
        }
    }
    #endif
    #endif
    if(IsAudplayer())
    {
    //以后用这个闹铃再调
//        Aud_PlayerMediaLibInit();
    }

    Fwl_FreqPop();    
#endif
}

void paintclk_music_play(void)
{
#if(USE_ALARM_CLOCK)
   if(m_pClkMusicPlay->refreshFlag & REFRESH_BACK)
    {
        T_RES_IMAGE imgId = 0;
        
        m_pClkMusicPlay->foucsWhat %= BACK_COUNT;
#if(USE_COLOR_LCD)

        AlmClk_SetCpu2xManual(AK_TRUE);
        
#if (STORAGE_USED == SPI_FLASH)
        imgId = m_backImage[0];
#else
        imgId = m_backImage[m_pClkMusicPlay->foucsWhat];
#endif

        if(REFRESH_ALL ==m_pClkMusicPlay->refreshFlag)
        {
             Fwl_LCD_lock(AK_FALSE);
             Eng_ImageResDisp(0, 0, imgId, AK_FALSE);
        }
        else
        {
            Eng_ImageResDispEx(LEFT_OFFSET, TOP_OFFSET, imgId,
                MAIN_LCD_WIDTH- (LEFT_OFFSET<<1), MAIN_LCD_HEIGHT- (TOP_OFFSET<<1), LEFT_OFFSET, 
                TOP_OFFSET, AK_FALSE);
        }
        AlmClk_SetCpu2xManual(AK_FALSE);
#else
        Eng_ImageResDisp(0, 0, imgId, AK_FALSE);
#endif
        m_pClkMusicPlay->foucsWhat++;
    }

    m_pClkMusicPlay->refreshFlag= REFRESH_NONE;
#endif
}

#if(USE_ALARM_CLOCK)
#if 0
static T_VOID ClkPlay_SetDefault(T_AlarmPlayParam *pPlayParam)
{
    AK_ASSERT_PTR_VOID(pPlayParam, "ClkPlay_SetDefault AK_NULL");
    ToWideChar(pPlayParam->musicName, DEF_CLOCKMUSICNAME);
    pPlayParam->lastMinute= DEF_CLOCKDUARATION;
    pPlayParam->volume= DEF_VOLUME;
}
#endif

static T_VOID ClkPlay_SaveMsg(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if(m_pClkMusicPlay->evtCode != M_EVT_USB_DETECT)
    {       
        m_pClkMusicPlay->isToExit= AK_TRUE;
        m_pClkMusicPlay->evtCode= event;
        if(pEventParm != AK_NULL)
        {
            m_pClkMusicPlay->evtParam= *pEventParm;
            AK_DEBUG_OUTPUT("event:%x pParam:%x\n", event, pEventParm->p.pParam1);
        }
        else
        {
            m_pClkMusicPlay->evtParam.w.Param1= M_EVT_RESTART;
        }
        AlmClk_PlayerStop(AK_FALSE);
    }

}

T_VOID ClkPlay_DealError()
{
    T_EVT_PARAM evtParam;
    evtParam.w.Param1= M_EVT_RESTART;
    ImgError_Paint(eRES_STR_IMG_ERR);
    Fwl_DelayUs(1000000);
    m_triggerEvent(M_EVT_EXIT, &evtParam);  
    m_pClkMusicPlay->refreshFlag= REFRESH_NONE;
    m_pClkMusicPlay->isTrigExit= AK_TRUE;
}

T_VOID ClkPlay_TriggerSDOut(T_VOID)
{
    m_pClkMusicPlay->evtParam.w.Param1 = USB_DETECT_LCD_LOCK;
    VME_EvtQueuePut(M_EVT_RET_ROOT, &m_pClkMusicPlay->evtParam);
    VME_EvtQueuePut(M_EVT_PULLOUT_SD, AK_NULL);
}

#endif
unsigned char handleclk_music_play(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#if(USE_ALARM_CLOCK)
    switch(event)
    {
    case M_EVT_Z01_MUSIC_PLAY:   
    case M_EVT_PREVIEW:
        if(m_pClkMusicPlay->hasStartPlay)
        {
            if(M_EVT_Z01_MUSIC_PLAY == event)
            {
                //m_pClkMusicPlay->playType= M_EVT_Z01_MUSIC_PLAY;
                #ifdef OS_ANYKA
                 AK_PRINTK("p:",m_pClkMusicPlay->playType, 1);
                 AK_PRINTK("e:", event, 1);;
                #endif
                ClkPlay_SaveMsg(M_EVT_Z01_MUSIC_PLAY, pEventParm);
                
            }
            return 0;
        }
        else
        {
            m_pClkMusicPlay->hasStartPlay= AK_TRUE;
            if(M_EVT_PREVIEW == event)
            {
                T_AlarmPlayParam  *pAlarmPlayParam= AK_NULL;
                AK_ASSERT_PTR(pEventParm, "handleclk_music_play pEvtParam Ak_NULL", 0);
                AK_ASSERT_PTR(pEventParm->p.pParam1, "handleclk_music_play 2 AK_NULL", 0);
                pAlarmPlayParam= (T_AlarmPlayParam *)pEventParm->p.pParam1; 
                m_pClkMusicPlay->alarmPlayParam= *pAlarmPlayParam;
            }
            else
            {
                AlmClk_DealAlarm(DEAL_ALARMPLAY, &m_pClkMusicPlay->alarmPlayParam);
            }
        }
        Fwl_LCD_lock(AK_FALSE);
        m_pClkMusicPlay->playType= event;    
     
#ifdef OS_ANYKA
        AK_PRINTK("play duaration:", m_pClkMusicPlay->alarmPlayParam.lastMinute, 1);
#endif
       #if (!USE_COLOR_LCD)
   //      Gui_DispResHint(eRES_STR_AUDIO_HINT_WAITING, CLR_WHITE, CLR_BLACK, 24);
        #else
   //     Gui_DispResHint(eRES_STR_AUDIO_HINT_WAITING, CLR_WHITE, CLR_BLACK,  (MAIN_LCD_HEIGHT- FONT_HEIGHT)>> 1);
        #endif

        
       if( AK_FALSE == AlmClk_PlayerStart(m_pClkMusicPlay->alarmPlayParam.musicName,
             AK_TRUE, m_pClkMusicPlay->alarmPlayParam.volume, NORMAL_EQ, NORMAL_SPEED, ALARM_FIDEIN_TIME, ALARM_FIDEOUT_TIME))
       {
           if(M_EVT_PREVIEW == event)
           {
               ClkPlay_DealError();
               return 0;
           }
           else
           {
           // if start fail start the default music
             ToWideChar(m_pClkMusicPlay->alarmPlayParam.musicName, DEF_CLOCKMUSICNAME);
             if(! AlmClk_PlayerStart(m_pClkMusicPlay->alarmPlayParam.musicName,
             AK_TRUE, m_pClkMusicPlay->alarmPlayParam.volume, NORMAL_EQ, NORMAL_SPEED, ALARM_FIDEIN_TIME, ALARM_FIDEOUT_TIME))
             {
                 ClkPlay_DealError();
                 return 0;
             }
           }
       }
       VME_EvtQueueClearTimerEvent();
       m_pClkMusicPlay->secCount= 0;
        return 0;
        
    case M_EVT_SD_IN:
    case M_EVT_SD_OUT:
    case M_EVT_RETURN:
    case M_EVT_Z00_POWEROFF:
    case M_EVT_USB_DETECT:
        ClkPlay_SaveMsg(event, pEventParm);
#ifdef OS_ANYKA
        AK_PRINTK("pre to exit evtCode:", event, 1);
#endif
        return 0;

    case M_EVT_PULLOUT_SD:
#ifdef SUPPORT_SDCARD
        if(VoiceGetCurDriver() == MMC_SD_CARD)
        {
            ClkPlay_TriggerSDOut();
            AlmClk_PlayerStop(AK_TRUE);
        }
        else
#endif
        {
            ClkPlay_SaveMsg(event, pEventParm);
        }
        return 0;

    case M_EVT_PUB_TIMER:
        if(!m_pClkMusicPlay->isTrigExit)
        {
            m_pClkMusicPlay->refreshFlag |= REFRESH_BACK;
        }
        else
        {
            #ifdef OS_ANYKA
            AK_PRINTK("***AlmClk NoPaint",0, 1);
            #endif
        }
        break;

    }
    if(AK_TRUE == m_pClkMusicPlay->isToExit)
    {
        if(AlmClk_IsPlayerStopped() && !m_pClkMusicPlay->isTrigExit)
        {
            AK_DEBUG_OUTPUT("Begin Exit event:%x param1:%x\n",m_pClkMusicPlay->evtCode, m_pClkMusicPlay->evtParam.p.pParam1);
            switch(m_pClkMusicPlay->evtCode)
            {
            case M_EVT_EXIT:
            case M_EVT_RETURN:
                VME_EvtQueuePut(M_EVT_EXIT, &m_pClkMusicPlay->evtParam);
                break;
            case M_EVT_Z01_MUSIC_PLAY:
            case M_EVT_Z00_POWEROFF:
                VME_EvtQueuePut(M_EVT_EXIT, &m_pClkMusicPlay->evtParam);//exit alarm play
                VME_EvtQueuePut(m_pClkMusicPlay->evtCode, &m_pClkMusicPlay->evtParam);//to power down system
                Fwl_LCD_lock(AK_TRUE);
                break;
            case M_EVT_USB_DETECT:
                //VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
                m_pClkMusicPlay->evtParam.w.Param1 = USB_DETECT_LCD_LOCK;
                VME_EvtQueuePut(M_EVT_RET_ROOT, &m_pClkMusicPlay->evtParam);
                VME_EvtQueuePut(M_EVT_USB_DETECT, AK_NULL);
                Fwl_LCD_lock(AK_TRUE);
                break;
            case M_EVT_PULLOUT_SD:
                ClkPlay_TriggerSDOut();
                break;
            case M_EVT_SD_IN:
            case M_EVT_SD_OUT:
                VME_EvtQueuePut(M_EVT_RET_ROOT, AK_NULL);
                VME_EvtQueuePut(m_pClkMusicPlay->evtCode, AK_NULL);
                break;
            }
            m_pClkMusicPlay->isTrigExit= AK_TRUE;
        }
        return 0;
    }
    
    switch(event)
    {
    case M_EVT_USER_KEY:
        {
            if(m_pClkMusicPlay->secCount > 0)
            {
                if(kbMODE == GET_KEY_ID(pEventParm))
                {
                    m_pClkMusicPlay->eClockEnd= CLOCK_STOP;
                }
                ClkPlay_SaveMsg(M_EVT_EXIT, AK_NULL);
                AK_PRINTK("M_EVT_USER_KEY:", GET_KEY_ID(pEventParm), 1);
            }
        }
        break;
    case M_EVT_PUB_TIMER:
        m_pClkMusicPlay->secCount++;
        if(m_pClkMusicPlay->secCount > (m_pClkMusicPlay->alarmPlayParam.lastMinute* 60))
        {
            ClkPlay_SaveMsg(M_EVT_EXIT, AK_NULL);
        }
        break;        
    }
#endif

    return 0;
}

#else
void initclk_music_play(void)
{
}
void exitclk_music_play(void)
{  
}
void paintclk_music_play(void)
{
}
unsigned char handleclk_music_play(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:initclk_music_play\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif
//file end

/* end of file */
