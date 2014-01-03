/************************************************************************
 * Copyright (c) 2010, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author£ºzhao_xiaowei
 * @Date£º
 * @Version£º
**************************************************************************/
#include "akdefine.h"
#include "m_event.h"
#include "m_event_api.h"
#include "eng_debug.h"
#include "Fwl_Keypad.h"
#include "log_aud_play.h"
#include "log_aud_control.h"
#include "log_aud_ab.h"
#include "Eng_AutoOff.h"
#include "Eng_String_UC.h"
#include "Fwl_Timer.h"

#define ABPLY_TIMER_TIME                200

#define ABPLY_CUR_PLAY_NONE             0x0
#define ABPLY_CUR_PLAY_REC              0x1
#define ABPLY_CUR_PLAY_MUSIC            0x2

#if(STORAGE_USED == NAND_FLASH)
#define AUD_AB_REC_FILE                 "B:/AUDIO/aud_ab_rec.wav"
#define AUD_AB_REC_FILE_PATH            "B:/AUDIO/"
#else 
#define AUD_AB_REC_FILE                 "A:/AUDIO/aud_ab_rec.wav"
#define AUD_AB_REC_FILE_PATH            "A:/AUDIO/"
#endif

typedef enum{
    AUD_AB_STATE_NULL = 0,
    AUD_AB_STATE_ABPLAY,
    AUD_AB_STATE_REC,
    AUD_AB_STATE_NEXT,
    AUD_AB_STATE_MAX
} T_AUD_AB_STATE;


typedef  struct{ 
    T_U8                    cur_play;
    T_AUD_PLAYER*           pPlayer;
    T_U32                   timeA;
    T_U32                   timeB;
    T_U32                   curTime;
    T_TIMER                 ctrltimer;
    T_AUD_AB_STATE          state;
    T_USTR_FILE             filepath;   
    T_USTR_FILE             RecPath;
    T_U32                   speed;
    T_U32                   tone;    
} AUD_ABPLAY;

static AUD_ABPLAY *pAudABPlay = AK_NULL;

#ifdef SUPPORT_AUDIO_AB


T_BOOL audio_isABPlayer(T_VOID)
{
    T_BOOL ret;

    
    if (AK_NULL != pAudABPlay)
        ret = AK_TRUE;
    else
        ret = AK_FALSE;
    akerror("  abplayer paudabplay:", ret, 1);
    return ret;
}

static T_VOID audio_ABplay_SetEnv(T_VOID)
{
    if (_SD_EQ_MODE_NORMAL != pAudABPlay->tone)
    {
        Aud_AudCtrlSetToneMode(pAudABPlay->tone);
    }
    else if (AUD_PLAY_NORMAL_SPEED != pAudABPlay->speed)
    {
        Aud_AudCtrlSetSpeed(pAudABPlay->speed);
    }
}


static T_VOID audio_ABplay_enter(T_EVT_PARAM *pEventParm)
{
    T_BOOL ret = AK_FALSE;
    T_AUD_LOGIC_PARA* pLogic = AK_NULL;
    T_U32 freq = 0;
    

    pLogic = (T_AUD_LOGIC_PARA*)pEventParm->p.pParam1;
    pAudABPlay->pPlayer = pLogic->pPlayer;
    Utl_UStrCpy(pAudABPlay->filepath, pLogic->musicPath);
    pAudABPlay->timeA = pLogic->timeA;
    pAudABPlay->timeB = pLogic->timeB;
    pAudABPlay->speed = pLogic->audConfig.speed;        
    pAudABPlay->tone  = pLogic->audConfig.toneMode;


    ret = Aud_CtrlABOpen(pAudABPlay->pPlayer, pAudABPlay->filepath, pAudABPlay->timeA, pAudABPlay->timeB);
    freq = APlayer_CalcFreq(pAudABPlay->pPlayer, AK_TRUE, 0);

    AK_DEBUG_OUTPUT("  abplayer set freq:%d\n", freq);
    
    Fwl_FreqPush(freq);
    
    if (ret)
    {
        audio_ABplay_SetEnv();
        ret = Aud_CtrlABPlay();
        
        if (ret)
        {
            pAudABPlay->cur_play = ABPLY_CUR_PLAY_MUSIC;
            pAudABPlay->state = AUD_AB_STATE_ABPLAY;
            pAudABPlay->ctrltimer = Fwl_TimerStartMilliSecond(ABPLY_TIMER_TIME, AK_TRUE);
        }
        else
        {
            AK_DEBUG_OUTPUT("Aud_CtrlABPlay failed!\n");
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        }
    }
    else
    {
        AK_DEBUG_OUTPUT("Aud_CtrlABOpen failed!\n");
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
    }
}


T_VOID initaudio_abplay(T_VOID)
{
    /*disable that auto closing the light*/
    AutoBgLightOffDisable();
    
    /**disable auto power off*/
    AutoPowerOffDisable();  

    pAudABPlay = (AUD_ABPLAY*)Fwl_Malloc(sizeof(AUD_ABPLAY));
    AK_ASSERT_PTR_VOID(pAudABPlay, "initaudio_ABplay pAudABPlay");
    memset(pAudABPlay, 0, sizeof(AUD_ABPLAY));

    AK_DEBUG_OUTPUT("enter:ABplay\n");

    Utl_StrMbcs2Ucs((AUD_AB_REC_FILE), pAudABPlay->RecPath);
    Fwl_FileDelete(pAudABPlay->RecPath);

    pAudABPlay->ctrltimer = ERROR_TIMER;
    pAudABPlay->cur_play = ABPLY_CUR_PLAY_NONE;

    Aud_CtrlABInit();

}

T_VOID exitaudio_abplay(T_VOID)
{               
    if (ERROR_TIMER != pAudABPlay->ctrltimer)
    {
        Fwl_TimerStop(pAudABPlay->ctrltimer);
    }
    
    Aud_CtrlABDestroy();

    Fwl_FileDelete(pAudABPlay->RecPath);

    Fwl_FreqPop();
    
    /*disable that auto closing the light*/
        AutoBgLightOffEnable();
        
    /**disable auto power off*/
        AutoPowerOffEnable();
    akerror("  abplayer exit !!!", 0, 1);
    pAudABPlay = Fwl_Free(pAudABPlay);
}

T_VOID paintaudio_abplay(T_VOID)
{
}


T_U8 handleaudio_abplay(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    
    if (M_EVT_ABPLAY == event)
    {
        audio_ABplay_enter(pEventParm);
    }

    switch(event)
    {
        case VME_EVT_TIMER:
            if (pEventParm->w.Param1 == (T_U32)pAudABPlay->ctrltimer)
            {
                if(AUD_AB_STATE_ABPLAY == pAudABPlay->state)
                {
                    APlayer_GetDuration(pAudABPlay->pPlayer, &pAudABPlay->curTime);
                    akerror("  muc_time:", pAudABPlay->curTime, 0);
//                  akerror("  timeB:", pAudABPlay->timeB-pAudABPlay->timeA, 1);            
                    akerror("  timeB:", pAudABPlay->timeB, 1);  

//                  if (pAudABPlay->curTime >=(pAudABPlay->timeB-pAudABPlay->timeA))
                    if (pAudABPlay->curTime >=(pAudABPlay->timeB))
                    {
                        goto play_loop;
                    }
                }
                else if (AUD_AB_STATE_NEXT == pAudABPlay->state)
                {
                    pAudABPlay->state = AUD_AB_STATE_ABPLAY;
                    goto play_loop;
                }
                else if (AUD_AB_STATE_REC == pAudABPlay->state)
                {
                    pAudABPlay->curTime = AudioRecord_GetCurrentTime();
//                  akerror("  rec_time:", pAudABPlay->curTime, 0);
//                  akerror("  timeB:", pAudABPlay->timeB-pAudABPlay->timeA, 1);
                    
                    if (pAudABPlay->curTime >= (pAudABPlay->timeB - pAudABPlay->timeA))
                    {
                        AK_DEBUG_OUTPUT("ABplay, stop rec!\n");
                        Aud_CtrlABStopRec();
                        Aud_CtrlABOpen(pAudABPlay->pPlayer, pAudABPlay->filepath, pAudABPlay->timeA, pAudABPlay->timeB);
                        audio_ABplay_SetEnv();
                        Aud_CtrlABPlay();
                        pAudABPlay->state = AUD_AB_STATE_ABPLAY;
                    }
                }
                break;

play_loop:      
                Aud_CtrlABStop();
            
                if ((ABPLY_CUR_PLAY_MUSIC == pAudABPlay->cur_play) && (Fwl_FileExist(pAudABPlay->RecPath)))
                {
                    pAudABPlay->cur_play = ABPLY_CUR_PLAY_REC;
                    AK_DEBUG_OUTPUT("  ABplay, open rec play!\n");
                    Aud_CtrlABOpen(pAudABPlay->pPlayer, pAudABPlay->RecPath, 0, pAudABPlay->timeB - pAudABPlay->timeA);
                }
                else
                {
                    pAudABPlay->cur_play = ABPLY_CUR_PLAY_MUSIC;
                    AK_DEBUG_OUTPUT("  ABplay, open music play!\n");
                    Aud_CtrlABOpen(pAudABPlay->pPlayer, pAudABPlay->filepath, pAudABPlay->timeA, pAudABPlay->timeB);
                    audio_ABplay_SetEnv();
                }
            
                Aud_CtrlABPlay();
            

            }
            break;

        case M_EVT_USER_KEY:
            {
                T_ePRESS_TYPE type = pEventParm->c.Param2;

                if(PRESS_SHORT == type)
                {
                    if (AUD_AB_STATE_REC == pAudABPlay->state)
                    {
                        Aud_CtrlABStopRec();
                    }
                    else if (AUD_AB_STATE_ABPLAY == pAudABPlay->state)
                    {
                        Aud_CtrlABStop();
                    }
                    
                    AK_DEBUG_OUTPUT("  ABplay, exit!\n");
                    pEventParm->w.Param1 = RETURN_FORM_ABPALYER;
                    m_triggerEvent(M_EVT_EXIT, pEventParm);
                }           
            }
            break;

        case M_EVT_ABPLAY_REC:
            if (AUD_AB_STATE_ABPLAY == pAudABPlay->state)
            {           
                Fwl_FileDelete(pAudABPlay->RecPath);
                
                Aud_CtrlABStop();
                AK_DEBUG_OUTPUT("  ABplay, start rec!\n");
                if (!Aud_CtrlABStartRec(pAudABPlay->RecPath, pAudABPlay->timeB - pAudABPlay->timeA))
                {
                    Aud_CtrlABStopRec();
                    m_triggerEvent(M_EVT_EXIT, pEventParm);
                }
            }       
            
            pAudABPlay->state = AUD_AB_STATE_REC;           
            break;

        case M_EVT_ABPLAY_FILEEND:
        case M_EVT_ABPLAY_DECERR:
            if (AUD_AB_STATE_ABPLAY == pAudABPlay->state)
            {
                akerror("  ABplay goto next:", event, 1);
                pAudABPlay->state = AUD_AB_STATE_NEXT;
            }       
            break;

        default:
            break;
    }

    

    if (event >= M_EVT_Z00_POWEROFF)
    {   
        AK_DEBUG_OUTPUT("  ABplay, M_EVT_Z00_POWEROFF exit!\n");
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        m_triggerEvent(M_EVT_Z00_POWEROFF, pEventParm);
        return 0;
    }

    return 0;
}

#else
T_BOOL audio_isABPlayer(T_VOID)
{
    return AK_FALSE;
}
T_VOID initaudio_abplay(T_VOID)
{
}

T_VOID exitaudio_abplay(T_VOID)
{
}

T_VOID paintaudio_abplay(T_VOID)
{
}

T_U8 handleaudio_abplay(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}
#endif

