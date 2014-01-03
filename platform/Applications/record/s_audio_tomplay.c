/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_audio_tomplay.c
 * @BRIEF audio record process & play
 * @Author：He_yuanlong
 * @Date：2013-02-04
 * @Version：
**************************************************************************/
#include "m_event.h"
#include "Ctrl_Public.h"
#include "M_Event_Api.h"
#include "log_record.h"
#include "Eng_Profile.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Eng_AutoOff.h"
#include "log_radio_core.h"
#include "log_record.h"
#include "Eng_USB.h"
#include "Gui_Common.h"
#include "Gui_Record.h"
#include "Gbl_Define.h"
#include "Gbl_Global.h"
#include "Fwl_osFS.h"
#include "Log_aud_control.h"
#include "Fwl_RTC.h"
#include "Fwl_System.h"
#include <stdio.h>
#include "Alarm_Common.h"
#include "Log_aud_play.h"
#include "Fwl_Waveout.h"
#include "Fwl_WaveIn.h"
#include "Ctrl_MenuConfig.h"


#define RECORD_ONCE_TIME       (5*60*1000)// 5*60*1000 (MS)
#define RECORD_LOWBAT_SHOWTIME  2
#define TOM_REC_VOLUME          25

extern T_VOID Preproc_TriggerPoweroff(T_VOID);
extern  T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt);

T_BOOL Vor_TomRec_cbk(T_BOOL bVorValid); 
static T_BOOL Tom_StartPlay(T_VOID);
static T_BOOL Tom_StartRec(T_VOID);
T_AUD_PLAYER* Tom_PlayerHandlePre(T_VOID);
T_VOID Tom_PlayerHandlePost(T_S32 size , T_AUD_PLAYER* pAudPlayer);

#if(STORAGE_USED == NAND_FLASH)
#ifdef OS_WIN32
const T_U16 tomfile[] = {'A',':','/','A', 'U', 'D', 'I', 'O', '_', 'R', 'E', 'C', '/',
						't','o','m','p','l','a','y','.','w','a','v','\0'};
#else
const T_U16 tomfile[] = {'B',':','/','A', 'U', 'D', 'I', 'O', '_', 'R', 'E', 'C', '/',
						't','o','m','p','l','a','y','.','w','a','v','\0'};
#endif
#else
const T_U16 tomfile[] = {'A',':','/','A', 'U', 'D', 'I', 'O', '_', 'R', 'E', 'C', '/',
						't','o','m','p','l','a','y','.','w','a','v','\0'};
#endif

typedef enum{
    eTOMMENU_MODE_TOMTYPE,

    eTOMMENU_MODE_MAX   
} T_eTOMMENU_MODE;  


const T_STR_SHOW m_TomTypePrintf[PITCH_RESERVE] = {
                                                "TOM_TYPE_NORMAL",
                                                "TOM_TYPE_CHILD",       
                                                "TOM_TYPE_MACHINE",
                                                "TOM_TYPE_ECHO",
                                            };


typedef struct _TOM_PLAY
{
    T_U8         TomMode;
    T_AUD_PLAYER* pAudPlayer;
    T_eREC_STAT  recStat;
    T_AUD_CONTROL_STATE playStat;/* play status */  
    T_BOOL       isTomState; //whether is in tom state machine;
}T_AUDIO_TOMPLAY;

static T_AUDIO_TOMPLAY* pTomPlayer = AK_NULL;
static volatile T_U8 RecFlag = 0;

/************************************************************************
 * @BRIEF the function Start Play the audio file which tom-record by MediaLib_SetTomSound
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_VOID: 
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to Play record file
 * @RETVAL AK_FALSE:Failure to Play record file
 **************************************************************************/
static T_BOOL Tom_StartPlay(T_VOID)
{
    if (AUD_STAT_LOG_PLAY == pTomPlayer->playStat)
    {
        return AK_FALSE;
    }

    pTomPlayer->pAudPlayer = APlayer_Init(TOM_REC_VOLUME,AUD_PLAY_NORMAL_SPEED,_SD_EQ_MODE_NORMAL);
    if (AK_NULL == pTomPlayer->pAudPlayer)
    {   
        AK_DEBUG_OUTPUT("tom player init fail\n");
        return AK_FALSE;
    }

    APlayer_RegHDLPre(Tom_PlayerHandlePre);
    APlayer_RegHDLPost(Tom_PlayerHandlePost);

    if (!APlayer_Open(pTomPlayer->pAudPlayer, (T_pWSTR)tomfile, MEDIALIB_MEDIA_WAV, 
                    AK_FALSE, DEFAULT_FADEIN_TIME, DEFAULT_FADEOUT_TIME,
                    (T_WSOLA_ARITHMATIC)AUD_SPEED_ARITHMETIC))
    {
        AK_DEBUG_OUTPUT("tom player open fail\n");
        return AK_FALSE;
    }

    MediaLib_SetTomSound(pTomPlayer->pAudPlayer->hMedia, pTomPlayer->TomMode);
    if (APlayer_Play(pTomPlayer->pAudPlayer))
    {   
        //play OK
        AK_DEBUG_OUTPUT("Tom Play OK\n");
    }

    pTomPlayer->playStat = AUD_STAT_LOG_PLAY;
    return AK_TRUE;
}

/************************************************************************
 * @BRIEF the function Start Record the sound which use for voice change play
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_VOID: 
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to record file
 * @RETVAL AK_FALSE:Failure to record file
 **************************************************************************/
static T_BOOL Tom_StartRec(T_VOID)
{
    T_S32 volLarger= 1024;               
    
    if (eSTAT_RECORDING == pTomPlayer->recStat)
    {   
        return AK_TRUE;
    }
    AudioRecord_Init();     
    RecFlag = 0;

    AutoPowerOffDisable();
    AudioRecord_SetDataSource(eSTAT_RCV_MIC);
    AudioRecord_SetVorCbk(Vor_TomRec_cbk, VORREC_CTRLVALUE);
    AudioRecord_SetVorCtrlState(eSTAT_VORREC_PAUSE); 

    AudioRecord_GetTotalTime(tomfile[0], eREC_MODE_WAV8K);
    AK_DEBUG_OUTPUT("\nvolLarger:%d\n", volLarger* 10/1024);
    if (!AudioRecord_Start(tomfile, eREC_MODE_WAV8K, volLarger, AK_TRUE))
    {
        AutoPowerOffEnable();
		return AK_FALSE;
    }
    pTomPlayer->recStat = eSTAT_RECORDING;
	return AK_TRUE;
}

/************************************************************************
 * @BRIEF the function Stop Record the sound which use for voice change play
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_VOID: 
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to record file
 * @RETVAL AK_FALSE:Failure to record file
 **************************************************************************/
static T_VOID Tom_StopRec(T_VOID)
{
    if (eSTAT_RECORDING == pTomPlayer->recStat);
    {
        AudioRecord_Stop();     
        AudioRecord_Destroy();
        pTomPlayer->recStat = eSTAT_REC_STOP;
    }
}

/************************************************************************
 * @BRIEF the function Stop Play the audio file which tom-record by MediaLib_SetTomSound
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_VOID: 
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to Play record file
 * @RETVAL AK_FALSE:Failure to Play record file
 **************************************************************************/
static T_VOID Tom_StopPlay(T_VOID)
{
    if (AK_NULL != pTomPlayer->pAudPlayer
        && AUD_STAT_LOG_PLAY == pTomPlayer->playStat)
    {
        //force stop
        APlayer_Stop(pTomPlayer->pAudPlayer);
        if (APlayer_Destroy(pTomPlayer->pAudPlayer))  
        {
            pTomPlayer->pAudPlayer = AK_NULL;
        }
        pTomPlayer->playStat = AUD_STAT_LOG_STOP;
    }
}

/************************************************************************
 * @BRIEF Set the audio Play PreProcess function
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_VOID: 
 * @RETURN T_AUD_PLAYER* 
 * @RETVAL T_AUD_PLAYER* :Retrun audio player parameters pointer
 **************************************************************************/
T_AUD_PLAYER* Tom_PlayerHandlePre(T_VOID)
{
    if (AK_NULL != pTomPlayer->pAudPlayer)
        return pTomPlayer->pAudPlayer;
    else 
        return AK_NULL;
}

/************************************************************************
 * @BRIEF Set the audio Play PostProcess function,control start record after play
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_S32 size: size of decode data
 * @PARAM T_AUD_PLAYER* pAudPlayer: audio player parameters pointer
 * @RETURN T_VOID 
 * @RETVAL T_AUD_PLAYER* :Retrun audio player parameters pointer
 **************************************************************************/
T_VOID Tom_PlayerHandlePost(T_S32 size , T_AUD_PLAYER* pAudPlayer)
{
    if (AK_NULL != pAudPlayer)
    {
        //handle decode result
        if (size < 0)
        {   
            if (pTomPlayer->isTomState)
            {           
                Tom_StopPlay();
                AK_DEBUG_OUTPUT("Tom_post!\n");
                if (!Tom_StartRec())
	            {
					m_triggerEvent(M_EVT_EXIT,AK_NULL);
				}   
            }
        }  
    } 
}

/************************************************************************
 * @BRIEF control record write data while has voice
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @PARAM T_BOOL bVorValid: whether has voice
 * @RETURN T_BOOL 
 * @RETVAL AK_TRUE : write data to file
 * @RETVAL AK_FALSE: do not write data to file
 **************************************************************************/
T_BOOL Vor_TomRec_cbk(T_BOOL bVorValid) 
{
    T_EVT_PARAM EventParm;
    
    //AK_DEBUG_OUTPUT("bVorValid=%d\n", bVorValid);
    if (!bVorValid)
    {
        if (eSTAT_VORREC_RECORDING == AudioRecord_GetVorCtrlState()
            && 1<RecFlag)
        {
            Fwl_ConsoleWriteStr("tom rec pause\n");
            AudioRecord_SetVorCtrlState(eSTAT_VORREC_PAUSE);
            
            EventParm.c.Param1 = CTRL_EVENT_VORREC_PAUSE;
            VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
            RecFlag = 0;
            return AK_FALSE;
        }

        if (eSTAT_VORREC_RECORDING != AudioRecord_GetVorCtrlState())
        {
            return AK_FALSE;
        }
    }
    else 
    {
        if (eSTAT_VORREC_PAUSE == AudioRecord_GetVorCtrlState())
        {
            Fwl_ConsoleWriteStr("tom rec resume\n");
            AudioRecord_SetVorCtrlState(eSTAT_VORREC_RECORDING);
            RecFlag = 0;
        }
        else if(eSTAT_VORREC_RECORDING == AudioRecord_GetVorCtrlState())
        {
            if (RECORD_ONCE_TIME <= AudioRecord_GetCurrentTime())
            {
                Fwl_ConsoleWriteStr("tom rec pause\n");
                AudioRecord_SetVorCtrlState(eSTAT_VORREC_PAUSE);
                
                EventParm.c.Param1 = CTRL_EVENT_VORREC_PAUSE;
                VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
                RecFlag = 0;
                return AK_FALSE;
            }
            else if(0 < RecFlag)
            {
                RecFlag = 0;
            }
        }
    }
    return AK_TRUE;
}
#pragma arm section code = "_frequentcode_"

/************************************************************************
 * @BRIEF whether is Tom player initialization
 * @AUTHOR He_yuanlong
 * @DATE  2013-02-04
 * @RETURN T_BOOL 
 * @RETVAL AK_TRUE : Tom Player is initialize
 * @RETVAL AK_FALSE: Tom Player not initialize
 **************************************************************************/
T_BOOL  IsTomPlayerInit(T_VOID)
{
    if (pTomPlayer == AK_NULL) 
        return AK_FALSE;
    else
        return AK_TRUE;
}
#pragma arm section code

void initaudio_tomplay(void)
{
    pTomPlayer = (T_AUDIO_TOMPLAY*)Fwl_Malloc(sizeof(T_AUDIO_TOMPLAY));
    if (AK_NULL == pTomPlayer)
    {
        AK_DEBUG_OUTPUT("No more memory to malloc for pTomplayer\n");
        return;
    }
    
    pTomPlayer->TomMode = PITCH_CHILD_VOICE;
    pTomPlayer->playStat = AUD_STAT_LOG_STOP;
    pTomPlayer->recStat = eSTAT_REC_STOP;
    pTomPlayer->isTomState  = AK_TRUE;
    pTomPlayer->pAudPlayer = AK_NULL;
    

    Fwl_FreqPush(FREQ_APP_MAX);
    AK_DEBUG_OUTPUT("tomplay init\n");
}

void paintaudio_tomplay(void)
{    
}

void exitaudio_tomplay(void)
{  
    AK_DEBUG_OUTPUT("tomplay exit\n");
    pTomPlayer->isTomState = AK_FALSE;
    Tom_StopRec();
    Tom_StopPlay(); 
    Fwl_FileDelete(tomfile);
    pTomPlayer = Fwl_Free(pTomPlayer);
    Fwl_FreqPop();
}

unsigned char handleaudio_tomplay(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{ 
    T_PRESS_KEY keyPad;

    switch (event)
    {
        case M_EVT_TOMPLAY:
            Fwl_FreqPush(FREQ_APP_MAX);  
            /* Start to record voice*/
            if (!Tom_StartRec())
            {
				m_triggerEvent(M_EVT_EXIT,AK_NULL);
			}
            Fwl_FreqPop();
            break;

        case M_EVT_Z00_POWEROFF:
            m_triggerEvent(M_EVT_EXIT,AK_NULL);//先退出然后再发关机事件
            m_triggerEvent(event, pEventParm);
            return 0;
            break;
            //if exit from menu
        case M_EVT_EXIT:
            if(pEventParm != AK_NULL)
            {
                if (RETURN_FROM_SET_MENU == pEventParm->s.Param1)
                {
                	if (pEventParm->s.Param2)
	            	{
	                	T_U8 submode = MENU_ERROR_ID;
						T_U8 item = MENU_ERROR_ID;

						MenuCfg_GetFocus(&submode, &item);
						
	                    if (eTOMMENU_MODE_TOMTYPE == submode)
	                    {
	                        //set tom voice type
	                        pTomPlayer->TomMode = item;
	                    }
                	}

                    /* Start to record voice*/
					if (!Tom_StartRec())
		            {
						m_triggerEvent(M_EVT_EXIT, AK_NULL);
					}

	                pTomPlayer->isTomState = AK_TRUE;

					MenuCfg_Free();
                }
            }
        case VME_EVT_TIMER:
            VME_EvtQueueClearTimerEventEx(VME_EVT_TIMER);
            break;

        case VME_EVT_RECORDCTRL:
            if (CTRL_EVENT_MEMORY_FULL == pEventParm->c.Param1)
            {
                m_triggerEvent(M_EVT_EXIT,AK_NULL);
                VME_EvtQueueClearTimerEvent();          
                return 0;
            }
            else if(CTRL_EVENT_SINGLE_FILE_LEN_LIMIT == pEventParm->c.Param1)
            {
                VME_EvtQueueClearTimerEvent();
                Tom_StopRec();
                
                //START TOM PLAY
                if (pTomPlayer->isTomState)
                {
                    Tom_StartPlay();
                }
                return 0;           
            }       
            else if(CTRL_EVENT_VORREC_PAUSE == pEventParm->c.Param1)
            {
                Tom_StopRec();
                //START TOM PLAY
                if (pTomPlayer->isTomState)
                {
                    Tom_StartPlay();        
                }
                return 0;
            }
            break;

        case M_EVT_USER_KEY:     
            keyPad.id = (T_eKEY_ID)pEventParm->c.Param1;
            keyPad.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;
            if (kbMODE == keyPad.id 
                && PRESS_SHORT == keyPad.pressType)
            {
                /*Enter Menu, stop play or record*/
                pTomPlayer->isTomState = AK_FALSE;
                Tom_StopRec();
                Tom_StopPlay();

				MenuCfg_Init(eTOMMENU_MODE_MAX);
				MenuCfg_AddMenu(eTOMMENU_MODE_TOMTYPE, PITCH_RESERVE, pTomPlayer->TomMode, (T_STR_SHOW*)(&m_TomTypePrintf));
                
                m_triggerEvent(M_EVT_MENU,pEventParm);
            }           
            return 0;
            break;

        case M_EVT_PUB_TIMER:
            if (eSTAT_RECORDING == pTomPlayer->recStat)
            {
                RecFlag++;
            }
            VME_EvtQueueClearTimerEventEx(M_EVT_PUB_TIMER);
            break;
        case M_EVT_RETURN:
             m_triggerEvent(M_EVT_EXIT,AK_NULL);
             return 0;
            
        default:
            break;
    }
    
    return 0;
}

