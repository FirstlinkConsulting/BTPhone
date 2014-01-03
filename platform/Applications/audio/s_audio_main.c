/**
 * @file    s_audio_main.c
 * @brief   audio player main UI handle
 * @author  WuShanwei
 * @date    2008-04-03
 * @version 1.0
 */
#include "akdefine.h"
#include "Fwl_lcd.h"
#include "log_aud_control.h"
#include "Fwl_System.h"
#include "Fwl_Keypad.h"
#include "Eng_AutoOff.h"
#include "media_lib.h"
#include "log_pcm_player.h"
#include "eng_debug.h"
#include "M_event_api.h"
#include "Ctrl_MenuConfig.h"
#include "Apl_Public.h"
#include "Eng_LedHint.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"

#ifdef SUPPORT_MUSIC_PLAY

#if(USE_ALARM_CLOCK)
extern T_BOOL  AlmClk_IsComing(T_VOID);
#endif
extern T_AUD_LOGIC_PARA* pAudLogic;

extern T_VOID VME_EvtQueueClearTimerEvent(T_VOID);
static T_VOID AudioMain_Enter(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
static T_VOID AudioMain_ChkDACOpened(T_VOID);
static T_BOOL AudioMain_DealPowerOff(T_EVT_CODE evtCode, T_EVT_PARAM *pEventParm);


#ifdef OS_ANYKA
#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)

extern T_U32        Image$$audioplayer_resident$$Base;
#define AUDIO_SECTION_BASE ((T_U32)&Image$$audioplayer_resident$$Base)
extern T_U32        Image$$audioplayer_resident$$Limit;
#define AUDIO_SECTION_LIMIT ((T_U32)&Image$$audioplayer_resident$$Limit)
T_BOOL remap_lock_page(T_U32 vaddr, T_U32 size, T_BOOL lock);
#endif
#endif

#endif

#define AUD_FINDNEXT_PLAY_TIMEOUT 10     //当按播放键如果当前歌曲可以播放就立即播放


void initaudio_main(void)
{
#ifdef SUPPORT_LEDHINT
    if(Fwl_DetectorGetStatus(DEVICE_CHG))
    {
        LedHint_Exec(LED_NORMAL_CHARGE);
    }
    else
    {
        LedHint_Exec(LED_NORMAL);
    }
#endif

#ifdef SUPPORT_MUSIC_PLAY
    AK_DEBUG_OUTPUT("initaudio_main\n");
#endif
}

void exitaudio_main(void)
{
#ifdef SUPPORT_MUSIC_PLAY

    AK_DEBUG_OUTPUT("exit\n");
    Aud_AudCtrlTimerStop();
    Aud_AudCtrlDestroy();
    

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
    /*{
        unsigned short cntUsed;
        unsigned long  szUsed;
        Ram_Info( &cntUsed, &szUsed);
        AK_DEBUG_OUTPUT("ram info cnt = %d ,sz = %d!!!!!\n", cntUsed, szUsed);
    }*/
#endif

#ifdef SUPPORT_LEDHINT
    if(Fwl_DetectorGetStatus(DEVICE_CHG))
    {
        LedHint_Stop(LED_NORMAL_CHARGE);
    }
    else
    {
        LedHint_Stop(LED_NORMAL);
    }
#endif
}

#pragma arm section code = "_audioplayer_"
void paintaudio_main(void)
{

}

#pragma arm section code

#ifdef SUPPORT_MUSIC_PLAY



static T_VOID AudioMain_Enter(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_AUD_LISTSTLE listStyle;
    T_U32 state= AUD_ERR_TYPE_NULL;

    switch (event)
    {
		#if 0
    case M_EVT_AUDIO_MAIN:
        listStyle = eAUD_MUSIC;
        break;
		#endif
    case M_EVT_AUDIO_VOICE:
        listStyle = eAUD_VOICE;
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_RECPLAY, AK_NULL);
        Voice_WaitTip();
#endif
        break;
    case M_EVT_SD_AUDIO:
        listStyle = eAUD_SDMUSIC;
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_TCARD, AK_NULL);
        Voice_WaitTip();
#endif
        break;
    case M_EVT_SD_VOICE:
        listStyle = eAUD_SDVOICE;
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_RECPLAY, AK_NULL);
        Voice_WaitTip();
#endif
        break;
    case M_EVT_USB_AUDIO:
        listStyle = eAUD_USBMUSIC;
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_UDISKPLAY, AK_NULL);
        Voice_WaitTip();
#endif
        break;
    default:
        listStyle = eAUD_MUSIC;
        break;
    }

    if (AK_NULL != pAudLogic)
    {
        if (listStyle != Aud_AudGetListStyle())
        {
            Aud_AudCtrlTimerStop();
            Aud_AudCtrlDestroy();
        }
        else
        {
            return;
        }
    }
    
    state = Aud_AudCtrlStart(listStyle);
    
            
#if(NO_DISPLAY == 1)
    if(AUD_ERR_TYPE_NULL == state)
    {
        pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
        AudioMain_ChkDACOpened();
    }
    else if(AUD_ERR_TYPE_NOFILE == state)
    {
        #ifdef SUPPORT_VOICE_TIP
            Voice_PlayTip(eBTPLY_SOUND_TYPE_NOMUSIC, AK_NULL);
            Voice_WaitTip();
        #endif
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
    }
    else
    {

    }
#endif
}

static T_VOID AudioMain_ChkDACOpened(T_VOID)
{
    AK_DEBUG_OUTPUT("initing ok\n");
    Fwl_LCD_lock(AK_FALSE);

    VME_EvtQueueClearTimerEvent();
    Aud_AudCtrlFreqSet();// reset frquence

    Aud_AudCtrlTimerStart();

    AutoBgLightOffEnable();
}
/*
#ifdef SUPPORT_SDCARD
static T_BOOL   Aud_ExitStandbyUpdateSD(T_VOID)
{

    Fwl_FreqPush(FREQ_APP_AUDIO_L5);
    pAudLogic->freqCur = FREQ_APP_AUDIO_L5;
    pAudLogic->freqStackDepth++;

    Fwl_Freq_Add2Calc(58000000);
    Aud_listAllPlayBuild(pAudLogic->pMusicList->pFileList,
                    pAudLogic->pMusicList->bUseHideDriver, pAudLogic->pMusicList->pFilter);        
    Fwl_Freq_Add2Calc(0);
    
    if(0 == Aud_AudCtrlGetIdxAll())
    {
        Aud_AudUISetErr(AUD_ERR_TYPE_NOFILE);
    }
    else
    {
        //search spacial name music
        if(Aud_listGetSpecNameItem(pAudLogic->pMusicList,pAudLogic->pMusicList->musicPath,0))
        {//get special name music successful
            Aud_AudCtrlActStop();
            if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
            {//pre handle error
                AK_DEBUG_OUTPUT("Aud_AudCtrlActPreHandle failed.3\n");
            }
            else
            {
                Aud_AudCtrlActPlySetOldPos();
            }
        }
        else
        {
            if (Aud_listGetItem(pAudLogic->pMusicList,3,0))
            {
                Aud_AudCtrlActStop();
                if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
                {//pre handle error
                    AK_DEBUG_OUTPUT("Aud_AudCtrlActPreHandle failed.4\n");
                }
            }
        }
    }

    //reset frquence
    Aud_AudCtrlFreqSet();


    return AK_TRUE;
}
#endif
*/
static T_VOID AudioMain_DealReturnMenu(T_VOID)
{
#ifdef OS_ANYKA
#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)
//AUDIO_SECTION_BASE对应的代码是本来是需要放在bootcode1中的,但由于内存不够的关系被裁出来,所以必须在用到的时候锁住对应的页
    remap_lock_page(AUDIO_SECTION_BASE,AUDIO_SECTION_LIMIT - AUDIO_SECTION_BASE,AK_TRUE);//锁1个页
#endif
#endif
    
    if(JUMP_NORMAL != pAudLogic->statJump)
    {
        Aud_AudCtrlExitStandby();

        if (JUMP_ALARM_CLK == pAudLogic->statJump)
        {
            AK_DEBUG_OUTPUT("back from Alarm state:%d\n", pAudLogic->playStat);
            //Aud_AudCtrlSetHpFlag(AK_TRUE);
            if (AUD_STAT_LOG_STOP == pAudLogic->playStat)
            {
                Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR);
            }
        }
        
        pAudLogic->statJump = JUMP_NORMAL;
        Aud_AudCtrlActPlySetOldPos();
    }
    else
    {
        Aud_AudCtrlUpdatePastTime();// note 这个不能下移
    }

#if(USE_ALARM_CLOCK)
    if (!AlmClk_IsComing())
    {
        Fwl_LCD_lock(AK_FALSE);
    }
#endif
    VME_EvtQueueClearTimerEvent();

    Aud_AudCtrlTimerStart();
    Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
}

static T_BOOL AudioMain_DealPowerOff(T_EVT_CODE evtCode, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("aud->power off state:%d \n", pAudLogic->playStat);
    if(pAudLogic != AK_NULL)
    {
        pAudLogic->audCtrlActPost = 0;
        pAudLogic->audCtrlAct = 0;
    }
    if (EVENT_TYPE_SWITCHOFF_BATLOW == pEventParm->w.Param1)
    {
        T_EVT_PARAM EventParm;

        m_triggerEvent(M_EVT_EXIT, AK_NULL);

        EventParm.w.Param1 = EVENT_TYPE_SWITCHOFF_BATLOW;
        VME_EvtQueuePut(evtCode, &EventParm);
        return AK_FALSE;
    }
    else
    {
        if(M_EVT_Z01_MUSIC_PLAY != evtCode)
        {
            APlayer_EntDepStdby(pAudLogic->pPlayer);
        }

        Aud_AudCtrlSaveData();
        Aud_AudCtrlTimerStop();
        Aud_AudCtrlFreqSet();
       
        return AK_TRUE;
    }
}

#endif

#if(NO_DISPLAY == 1)

#pragma arm section code = "_audioplayer_"
unsigned char handleaudio_main(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_MUSIC_PLAY
//    T_BOOL press_key_quit_standby = AK_FALSE;

    if( M_EVT_RETURN == event)
    {
        event = M_EVT_USER_KEY;
        pEventParm->c.Param1 = kbOK;
        pEventParm->c.Param2 = PRESS_LONG;
    }

    switch(event)
    {
    case M_EVT_EXIT:
		#if 0
        if(0xFF == Fwl_KeypadScan())
        {
            press_key_quit_standby = AK_FALSE;
        }
        else
        {
            press_key_quit_standby = AK_TRUE;
        }
        #endif
        AudioMain_DealReturnMenu();
        
        if (RETURN_FROM_SET_MENU == pEventParm->s.Param1)
        {
            if (pEventParm->s.Param2)
            {
                T_U8 submode = MENU_ERROR_ID;
                T_U8 item = MENU_ERROR_ID;

                MenuCfg_GetFocus(&submode, &item);
                
                if (eAUDMENU_MODE_CYC == submode)
                {
                    pAudLogic->audConfig.cycMode = item;
                }
                else if (eAUDMENU_MODE_EQ == submode)
                {
                    Aud_AudCtrlSetToneMode(item);
                }
                else if (eAUDMENU_MODE_SPEED == submode)
                {
                    Aud_AudCtrlSetSpeed(item);
                }
                else if (eAUDMENU_MODE_SEEKLEN == submode)
                {
                    pAudLogic->audConfig.seeklenId = item;
                    Aud_AudCtrlSetSeekLen(pAudLogic->audConfig.seeklenId);
                }
                else if (eAUDMENU_MODE_UPDATELIST == submode)
                {
                    Aud_AudDelList(Aud_AudGetListStyle());
                    Aud_AudCtrlReStart(pAudLogic->audConfig.ListStyle);
                    MenuCfg_Free();
                    //调用Aud_AudCtrlReStart后不要再调用Aud_AudCtrlActResume，因此break
                    break;
                }
            }
 
            MenuCfg_Free();
        }
        if (RETURN_FORM_ABPALYER == pEventParm->w.Param1)
        {
            akerror("   exit from abplayer!", 0, 1);
            if(!APlayer_Open(pAudLogic->pPlayer,pAudLogic->musicPath,MEDIALIB_MEDIA_UNKNOWN,AK_FALSE,0, 0, AUD_SPEED_ARITHMETIC))
            {
                akerror("  exit from abplayer open failed!", 0, 1);
                return AK_FALSE;
            }           
            
            APlayer_Seek(pAudLogic->pPlayer, pAudLogic->timeB);
            if (_SD_EQ_MODE_NORMAL != pAudLogic->audConfig.toneMode)
            {
                Aud_AudCtrlSetToneMode(pAudLogic->audConfig.toneMode);
            }
            else if (AUD_PLAY_NORMAL_SPEED != pAudLogic->audConfig.speed)
            {
                Aud_AudCtrlSetSpeed(pAudLogic->audConfig.speed);
            }                     
            akerror("  seek timeB", pAudLogic->timeB, 1);
            
            APlayer_PrePostPop();
        }
        if (M_EVT_RESTART == pEventParm->w.Param1)
        {
            akerror("exit from deepstandby!", 0, 1);

            if ((eAUD_SDMUSIC == Aud_AudGetListStyle())|| (eAUD_SDVOICE == Aud_AudGetListStyle()))
            {
                //Aud_AudCtrlReStart(pAudLogic->audConfig.ListStyle);
                APlayer_LevDepStdby(pAudLogic->pPlayer);
#if 0
                if(AK_FALSE == press_key_quit_standby)
                {
                    Aud_AudCtrlActResume();
                }
     #endif
           //调用Aud_AudCtrlReStart后不要再调用Aud_AudCtrlActResume，因此break
                break;
            }
            //APlayer_LevDepStdby(pAudLogic->pPlayer);*/
        }
		else
		{
        	Aud_AudCtrlActResume();     //这个函数应该放在Aud_AudCtrlActPlySetOldPos后，因为media_play要在MediaLib_SetPosition后
		}
        break;
    case M_EVT_AUDIO_CTRL_TIMER:
        if(0 != pAudLogic->audCtrlAct)
        {
            Aud_AudCtrlActHandle();
        }
        break;
    case M_EVT_AUDIO_VOICE:
//    case M_EVT_AUDIO_MAIN:
    case M_EVT_SD_AUDIO:
    case M_EVT_SD_VOICE:
    case M_EVT_USB_AUDIO:
        //Fwl_FreqPush(FREQ_APP_MAX);
        AudioMain_Enter(event, pEventParm);
        //Fwl_FreqPop();
        break;
    case M_EVT_USER_KEY:
        {   
            return Aud_AudCtrlKey2Act(pEventParm);
        }
            
        break;
#ifdef SUPPORT_VOICE_TIP
    case VME_EVT_POWER_CHANGE:
        pAudLogic->Powerstate = pEventParm->c.Param1;
        Aud_AudCtrlActPowChgPause();
        break;
    case VME_EVT_VOICE_TIP:
        if(eBTPLY_SOUND_TYPE_LOWPOWER == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_LOWPOWER, AK_NULL);
        }
        else if(eBTPLY_SOUND_TYPE_CHARGING == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGING, AK_NULL);
        }
        else if(eBTPLY_SOUND_TYPE_CHARGEOK == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGEOK, AK_NULL);
        }
        Voice_WaitTip();
        
        Aud_AudCtrlActAutoResume();
        break;
    case VME_EVT_MAX_VOLUME:
        Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
        Voice_WaitTip();
        Aud_AudCtrlActAutoResume();
        break;
#endif
    case M_EVT_PUB_TIMER:
        {
            if (AK_TRUE == pAudLogic->findnextplay)//判是否要查找下一首可以播放的歌曲
            {
                pAudLogic->findnextplaytimeout++;

                if (pAudLogic->findnextplaytimeout >= AUD_FINDNEXT_PLAY_TIMEOUT)//查找时间超时，退出
                {
                    pAudLogic->findnextplay = AK_FALSE;
                    pAudLogic->findnextplaytimeout = 0;
                    VME_EvtQueuePut(M_EVT_EXIT, NULL);
                }
            }
        }
        break;

    case M_EVT_Z02_STANDBY:
        //break;
    
    case M_EVT_Z00_POWEROFF:  
        if (EVENT_TYPE_SWITCHOFF_KEY == pEventParm->w.Param1)
        {
            Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
        }

        if (!Aud_AudCtrlEnterStandby(event, pEventParm))
        {
            return 0;
        }
    case M_EVT_RESTART:
        Aud_AudCtrlTimerStart();
        break;
    case M_EVT_TIMEOUT: 
        if(Aud_AudCtrlActGetCfgVolMode())
        {
            Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
        }    
        break;
    default:
        break;
    }

    if ((pAudLogic->statJump != JUMP_NORMAL)&& (event >= M_EVT_Z00_POWEROFF) 
        && (AUD_STAT_LOG_PLAY != pAudLogic->playStat))
    {          
        return AudioMain_DealPowerOff(event,pEventParm);
    }
    
    return 0;
#else

    AK_DEBUG_OUTPUT("error state:handleaudio_main\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;     

#endif
}
#pragma arm section code 
#endif

