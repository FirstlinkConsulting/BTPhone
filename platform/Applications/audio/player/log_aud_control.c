/**
 * @file    log_aud_control.c
 * @brief   manage audio logic layer
 * @author  WuShanwei
 * @date    2008-04-03
 * @version 1.0
 */


#include "akdefine.h"
#include "Apl_Public.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "log_aud_control.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "M_event_api.h"
#include "Eng_Font.h"
#include "log_pcm_player.h"
#include "Eng_Profile.h"
#include "Fwl_FreqMgr.h"
#include "Eng_String_UC.h"
#include "media_lib.h"
#include "Eng_AutoOff.h"
#include "Eng_usb.h"
#include "Fwl_System.h"
#include "Fwl_osFS.h"
#include "Fwl_LCD.h"
#include "Fwl_detect.h"
#include "Lib_avf_player.h"
#include "Ctrl_MenuConfig.h"
#include "Svc_MediaList.h"
#include "VoiceMsg.h"
//#include "Eng_VoiceTip.h"

#ifdef SUPPORT_MUSIC_PLAY

#define SYSTEM_USE_CPU2X 1

#define AUD_48K_SAMPLE_RATE     48000
#define AUD_320K_BITRATE        320000

#define SET_MENU_KEY        kbMODE
#define PLAY_CONTROL_KEY    kbOK


#define AUD_SEEK_TIMEOUT_CNT	2

#define AUD_DEFAULT_VOLUME      15

#pragma arm section zidata = "_bootbss_"
T_AUD_LOGIC_PARA* pAudLogic= AK_NULL;
#pragma arm section zidata
T_AUD_FILE_CTRL_PARA* pAudFileCtrl;

const T_U16 strUFilter[][MAX_FILTER_LEN] = {
        MUSIC_SUPPORT_TYPE,
        RECORD_SUPPORT_TYPE,
        };

const T_U16 folderFilter[][MAX_FILTER_LEN] = {
        {'.','*','/','R','E','C','O','R','D',0,},
        {'.','*','/',0,},
        };


const T_U16 mediaListName[eAUD_NUM][MEDIA_LISTNAME_SIZE] = {
        {'m','u','s','i','c',0},
        {'v','o','i','c','e',0},
        {'u','s','b','m','u','s','i','c',0},
        {'s','d','m','u','s','i','c',0},
        {'s','d','v','o','i','c','e',0},
        };


#define AUD_SEEK_V_MAX    (3)


const T_U32 aud_seeklen[AUD_SEEK_V_MAX] = {5000, 10000, 15000};


const T_STR_SHOW m_CycModePrintf[AUD_CYC_MODE_MAX] = {
                                                    "CYC_NORMAL",       
                                                    "CYC_SINGLE",   
                                                    "CYC_TOTAL",    
                                                };

const T_STR_SHOW m_EqPrintf[_SD_EQ_USER_DEFINE + 1] = {
                                                    "EQ_NORMAL",        
                                                    "EQ_CLASSIC",   
                                                    "EQ_DANCE",  
                                                    "EQ_POP",
                                                    "EQ_ROCK",      
                                                    "EQ_EXBASS",    
                                                    "EQ_VOCAL",  
                                                    "EQ_PERSONAL"
                                                };

const T_STR_SHOW m_SpeedPrintf[AUD_PLAY_SETG_MAX_SPEED_STEP] = {
                                                    "SPEED_0.5",        
                                                    "SPEED_0.6",    
                                                    "SPEED_0.7",    
                                                    "SPEED_0.8",
                                                    "SPEED_0.9",        
                                                    "SPEED_1.0",    
                                                    "SPEED_1.1",    
                                                    "SPEED_1.2",
                                                    "SPEED_1.3",        
                                                    "SPEED_1.4",    
                                                };

const T_STR_SHOW m_SeeklenPrintf[AUD_SEEK_V_MAX] = {
                                                    "SEEK_5s",        
                                                    "SEEK_10s",    
                                                    "SEEK_15s",    
                                                };

const T_STR_SHOW m_UpdateListPrintf[1] = {
                                                    "Update List",        
                                                };



static T_VOID Aud_AudCtrlFree(T_VOID);
static T_BOOL Aud_AudCtrlActCheckNeedDec(T_VOID);
static T_AUD_PLAYER* Aud_AudCtrlPlayHandelPre(T_VOID);
static T_VOID Aud_AudCtrlPlayHandelPost(T_S32 size, T_AUD_PLAYER* pAudPlayer);
static T_BOOL Aud_AudCtrlSupportSeek(T_VOID);
static T_BOOL Aud_AudCtrlChkActPost(T_U32 evt);
T_BOOL Aud_AudCtrlActPlay(T_VOID);
static T_BOOL Aud_AudCtrlActPause(T_VOID);
static T_BOOL Aud_AudCtrlActFastPlayBeg(T_S8 dir);
static T_BOOL Aud_AudCtrlActFastPlayEnd(T_S8 dir);
static T_BOOL Aud_AudCtrlActSetVol(T_S8 dir);
static T_BOOL Aud_AudListInit(T_AUD_LISTSTLE listStyle);
static T_BOOL Aud_AudCtrlUpdateHandle(T_MEM_DEV_ID driver);

T_BOOL MList_IsHaveItem(T_VOID);

extern T_U32 clk_get_asic(T_VOID);


/**
* @brief	set seek len by seeklenId
* @author	WuShanwei
* @date	2008-04-01
* @param T_U8 seeklenId
* @return T_BOOL
* @retval
**/
T_BOOL Aud_AudCtrlSetSeekLen(T_U8 seeklenId)
{
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }
	
    if (seeklenId >= AUD_SEEK_V_MAX)
    {
        pAudLogic->seeklen = aud_seeklen[0];
    }
	else
    {
        pAudLogic->seeklen = aud_seeklen[seeklenId];
    }

    return AK_TRUE;
}

/**************************************************
* @brief 
* 模拟按键动作，以执行须要的操作
***************************************************/
static void Aud_AudCtrlSetKeyAct(T_eKEY_ID keyid, T_ePRESS_TYPE presstype)
{
    T_EVT_PARAM pEventParm;
    pEventParm.c.Param1 = keyid;
    pEventParm.c.Param2 = presstype;
    VME_EvtQueuePut(M_EVT_USER_KEY, &pEventParm);
    AK_DEBUG_OUTPUT("\nAudCtrlSetKeyAct keyid:%d, presstype:%d\n", keyid, presstype);
}


/**************************************************
* @brief 
* 获取当前播放的媒体类型
* @author Zhao_Xiaowei
***************************************************/
static T_eMEDIALIB_MEDIA_TYPE Aud_GetMediaType(T_U16* filename)
{
    T_eMEDIALIB_MEDIA_TYPE eMediaType = MEDIALIB_MEDIA_UNKNOWN;
/*  
    switch(Aud_AudGetListStyle())
    {
        case eAUD_MUSIC:
            eMediaType= MEDIALIB_MEDIA_UNKNOWN;
            break;
        case eAUD_VOICE:
            eMediaType= MEDIALIB_MEDIA_WAV;
            break;     
         default:
            eMediaType= MEDIALIB_MEDIA_UNKNOWN;
            break;
    }*/

    AK_DEBUG_OUTPUT("Aud_GetMediaType:%d\n",eMediaType);
    return eMediaType;
}



/**
 * @brief   calculate fit frequency for audio 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_U8
 * @retval  
 **/
static T_U8 Aud_AudCtrlFreqCal(T_VOID)
{
    //return FREQ_APP_AUDIO_L4;
    if(AUD_STAT_LOG_STOP == pAudLogic->playStat)
    {
        return FREQ_APP_AUDIO_L1;
    }
    else if(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
    {
        return FREQ_APP_AUDIO_L1;
    }
    else if(AUD_STAT_LOG_NULL == pAudLogic->playStat)
    {
        return FREQ_APP_AUDIO_L1;
    }   
    else
    {
    //  有屏版本需要根据亮暗瓶来计算频率；
     //   return (APlayer_CalcFreq(pAudLogic->pPlayer,bglight_state_off(), 0));
         return (APlayer_CalcFreq(pAudLogic->pPlayer, AK_TRUE, 0));
    }       
}


#if (SYSTEM_USE_CPU2X)
T_BOOL	Aud_AudCtrlCPU2XSet(T_U8 freqRst)
{
 	T_BOOL setCpu2x = AK_FALSE;

	if(AK_NULL == pAudLogic)
	{
		return AK_FALSE;
	}

	if ((AUD_STAT_LOG_NULL == pAudLogic->playStat)
		||(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
		||(AUD_STAT_LOG_STOP == pAudLogic->playStat))
	{
		setCpu2x = AK_FALSE;
	}
	else
	{
		{
		    if(pAudLogic->pPlayer->speedLibType == _SD_WSOLA_ARITHMATIC_1)
   			{
				if((pAudLogic->pPlayer->modeSpeed != 8)
					&& (freqRst >= FREQ_APP_AUDIO_L3))
				{
					setCpu2x = AK_TRUE;
					goto EXIT_CPU2X;
				}        
		    }
			else
			{
				if (pAudLogic->pPlayer->modeSpeed > 8)
				{
					setCpu2x = AK_TRUE;
					goto EXIT_CPU2X;
				}
			}

			if (_SD_MEDIA_TYPE_APE == pAudLogic->pPlayer->audType)
			{
				 setCpu2x= AK_TRUE;
				 goto EXIT_CPU2X;
			}

			if (_SD_EQ_MODE_NORMAL != pAudLogic->audConfig.toneMode)
			{
				if ( (_SD_MEDIA_TYPE_WMA == pAudLogic->pPlayer->audType)
					|| (pAudLogic->pPlayer->sampleRate >= AUD_48K_SAMPLE_RATE))
				{   
	  			 	setCpu2x= AK_TRUE;
					goto EXIT_CPU2X;
				}
			}

			if (pAudLogic->bFreqLock &&(freqRst <= FREQ_APP_AUDIO_L4))
			{
				setCpu2x = AK_TRUE;
				goto EXIT_CPU2X;
			}

			if ((_SD_MEDIA_TYPE_WMA == pAudLogic->pPlayer->audType)
				&& (pAudLogic->pPlayer->bitRate >= AUD_320K_BITRATE))
			{
				setCpu2x = AK_TRUE;
				goto EXIT_CPU2X;
			}
		}
	}

	
  	if(!setCpu2x)
   	{
		if (pAudLogic->bClock2X)
		{
			AK_DEBUG_OUTPUT("reset 2x\n");
	    	Fwl_Freq_Clr_Add();
	    	pAudLogic->bClock2X = AK_FALSE;
		}
	}

EXIT_CPU2X:
	return setCpu2x;
}
#endif

/**
 * @brief   set freq 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_FALSE-no in audio modem
            AK_TRUE-in audio modem 
 **/
T_BOOL  Aud_AudCtrlFreqSet(T_VOID)
{
    T_U8 i,freqRst;
    T_BOOL bCpu2x = AK_FALSE;
    T_BOOL bNeedSetFreq = AK_TRUE;
    
    if(AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    freqRst = Aud_AudCtrlFreqCal(); 
#if (SYSTEM_USE_CPU2X)
        bCpu2x = Aud_AudCtrlCPU2XSet(freqRst);
        if (bCpu2x)
        {
            if (freqRst < FREQ_APP_AUDIO_L4)
            {
                freqRst = FREQ_APP_AUDIO_L4;
            }
            else
            {
                if (clk_get_asic() >= FREQ_PLL_MAX)
                {
                    bNeedSetFreq = AK_FALSE;
                }
            }
        }
        else
#endif
        {
            if (pAudLogic->bFreqLock && freqRst <= pAudLogic->freqCur)
            {
                AK_DEBUG_OUTPUT("in menu,set frq false\n");
                return AK_FALSE;
            }
        }

    if ((bNeedSetFreq)
        &&(freqRst !=  pAudLogic->freqCur))//not the same
    {
        //AK_DEBUG_OUTPUT("Aud_AudCtrlFreqSet,stackDepth:%d\n",pAudLogic->freqStackDepth);
        for(i=1; i<pAudLogic->freqStackDepth;i++) //基准频率不pop
        {
             #ifdef OS_ANYKA
            AK_PRINTK("pop",0, 1);
            #endif
            Fwl_FreqPop();
        }
        pAudLogic->freqCur = freqRst;//reset cur freg
        Fwl_FreqPush(freqRst);
        pAudLogic->freqStackDepth = 2;
    }

    if (bCpu2x)
    {
        if (!pAudLogic->bClock2X)
        {
            AK_DEBUG_OUTPUT("set 2x\n");
            Fwl_Freq_Add2Calc(FREQ_VAL_1);
            pAudLogic->bClock2X = AK_TRUE;
        }
    }
    
    //AK_DEBUG_OUTPUT("push,F:%d,freqStackDepth:%d\n",freqRst,pAudLogic->freqStackDepth);
    return AK_TRUE;
}


/**
 * @brief   lock freq (only can set higher frequence)
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   freq: frequence to lock
 * @return  T_BOOL
 * @retval  AK_FALSE-no in audio modem
            AK_TRUE-in audio modem 
 **/
#pragma arm section code = "_audioplayer_menu_"
T_BOOL  Aud_AudCtrlFreqLock(T_U8 freq)
{   
    if(AK_NULL == pAudLogic || pAudLogic->bFreqLock)
    {
        return AK_FALSE;
    }

    if (freq >  pAudLogic->freqCur)//not the same
    {
        pAudLogic->freqCur = freq;//reset cur freg
        Fwl_FreqPush(freq);
        pAudLogic->freqStackDepth++;
    }
    
    pAudLogic->bFreqLock = AK_TRUE;
    Aud_AudCtrlFreqSet();
    return AK_TRUE;
}

T_BOOL  Aud_AudCtrlFreqUnlock(T_VOID)
{
    if(AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    if (pAudLogic->bFreqLock )
    {
        pAudLogic->bFreqLock  = AK_FALSE;   
        Aud_AudCtrlFreqSet();
    }
    return AK_TRUE;
}

#pragma arm section code

#pragma arm section code = "_audioplayer_" 
/**
 * @brief   seek cyclic handle 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
static T_BOOL   Aud_AudCtrlSeekFBCycHandle(T_VOID)
{

    if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKFORWARD;

        return AK_TRUE;
    }
    else if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
    {   
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKBACKWARD;

        return AK_TRUE;
    }

    return AK_FALSE;
}
#pragma arm section code




/**
 * @brief   Audio control Init
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
#pragma arm section code = "_audioplay_pre_" 
static T_U32   Aud_AudCtrlInit(T_AUD_LISTSTLE listStyle)
{
    T_U16   offset = 0;
    
    pAudFileCtrl = AK_NULL;
   
    AK_DEBUG_OUTPUT("Aud_AudCtrlInit begin\n");
    Fwl_FreqPush(FREQ_APP_AUDIO_L2); //增加一个基准频率
    Fwl_FreqPush(FREQ_APP_AUDIO_L5); 
    AK_DEBUG_OUTPUT("push\n");

    //memery malloc
    if(pAudLogic != AK_NULL)
    {
        AkDebugOutput("*********AudCtrlInit no Free AudLogic\n");
    }
    else
    {
        pAudLogic = (T_AUD_LOGIC_PARA *)Fwl_Malloc(sizeof(T_AUD_LOGIC_PARA));
    }
    
    if(AK_NULL == pAudLogic)
    {
        AK_DEBUG_OUTPUT("pAudLogic malloc fail\n"); 
        return AUD_ERR_TYPE_MEMORYFULL;    
    }
    memset(pAudLogic,0,sizeof(T_AUD_LOGIC_PARA));

	pAudLogic->audConfig.ListStyle = listStyle;


    //read user data
    pAudLogic->freqStackDepth = 2;
    pAudLogic->freqCur  = FREQ_APP_AUDIO_L5;
    pAudLogic->audCtrlAct = AUD_ACT_NULL;
    pAudLogic->abplayMode = AK_FALSE;
    
    pAudLogic->Powerstate = 0xff;
    pAudLogic->bMaxVol = AK_FALSE;

    pAudLogic->audCtrlTimer = ERROR_TIMER;
    pAudLogic->pPlayer = AK_NULL;
    pAudLogic->bClock2X = AK_FALSE;

    pAudLogic->findnextplay = AK_FALSE;
    pAudLogic->findnextplaytimeout = 0;
    
    switch(listStyle)
    {
        case eAUD_MUSIC:
            offset =  eCFG_AUDIO;
            break;
#ifdef SUPPORT_SDCARD
    case eAUD_SDMUSIC:
        offset =  eCFG_SDAUDIO;
        break;
#endif
#ifdef SUPPORT_USBHOST
    case eAUD_USBMUSIC:
        offset =  eCFG_USBAUDIO;
        break;
#endif
#ifdef SUPPORT_VOICE_PLAY
        case eAUD_VOICE:
            offset =  eCFG_VOICE;
            break;
#ifdef SUPPORT_SDCARD
    case eAUD_SDVOICE:
        offset =  eCFG_SDVOICE;
        break;
#endif
#endif
        default:
            offset =  eCFG_AUDIO;
            break;
    }   
    Profile_ReadData(offset, &pAudLogic->audConfig);

	pAudLogic->seeklen = aud_seeklen[pAudLogic->audConfig.seeklenId];
    pAudLogic->saveMuteVol = pAudLogic->audConfig.volume;

	if (!Aud_AudListInit(listStyle))
	{
		return AUD_ERR_TYPE_NOFILE;
	}
    
    if (!MList_IsHaveItem())
    {
        pAudLogic->audConfig.curTimeMedia = 0;
    }
	
	if (!Aud_AudCtrlUpdateHandle(Aud_GetCurDriver()))
    {//no music 
        return AUD_ERR_TYPE_NOFILE;
    } 

    //audio player init
    pAudLogic->pPlayer = APlayer_Init(pAudLogic->audConfig.volume,pAudLogic->audConfig.speed,pAudLogic->audConfig.toneMode);
    if(AK_NULL == pAudLogic->pPlayer)
    {
        return AUD_ERR_TYPE_MEMORYFULL;
    }

    MediaLib_SetCPULevel(AK_FALSE);

    APlayer_RegHDLPre(Aud_AudCtrlPlayHandelPre);
    APlayer_RegHDLPost(Aud_AudCtrlPlayHandelPost);
        
    AK_DEBUG_OUTPUT("Aud_AudCtrlInit OK,vol%d,tone%d,speed%d\n",pAudLogic->audConfig.volume,pAudLogic->audConfig.toneMode,pAudLogic->audConfig.speed);

    return AUD_ERR_TYPE_NULL;
}

/**
 * @brief   Audio control update
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
static T_BOOL   Aud_AudCtrlUpdateHandle(T_MEM_DEV_ID driver)
{    
    return MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_CUR, AK_TRUE);
}

T_MEM_DEV_ID  Aud_GetCurDriver(T_VOID)
{
    #ifdef SUPPORT_SDCARD
    if (AK_NULL != pAudLogic)
	{
    	return Fwl_MemDevGetDriver(pAudLogic->musicPath[0]);
    }
    #endif

    return SYSTEM_STORAGE;
}


/**
 * @brief   Audio control destroy
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlDestroy(T_VOID)
{
    AK_DEBUG_OUTPUT("Destroy\n");

    Aud_AudCtrlFree();

    AutoBgLightOffEnable();
        
    return AK_TRUE;
}

/**
 * @brief   Save audio data
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlSaveData(T_VOID)
{
    T_U16 offset = 0;
    
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    switch(Aud_AudGetListStyle())
    {
    case eAUD_MUSIC:
        offset =  eCFG_AUDIO;
        break;
#ifdef SUPPORT_SDCARD
    case eAUD_SDMUSIC:
        offset =  eCFG_SDAUDIO;
        break;
#endif
#ifdef SUPPORT_USBHOST
    case eAUD_USBMUSIC:
        offset =  eCFG_USBAUDIO;
        break;
#endif
#ifdef SUPPORT_VOICE_PLAY
    case eAUD_VOICE:
        offset =  eCFG_VOICE;
        break;
#ifdef SUPPORT_SDCARD
    case eAUD_SDVOICE:
        offset =  eCFG_SDVOICE;
        break;
#endif
#endif
    default:
        offset =  eCFG_AUDIO;
        break;
    }

    if( (AUD_STAT_LOG_PLAY == pAudLogic->playStat) || \
            (AUD_STAT_LOG_PAUSE == pAudLogic->playStat) ||\
            (AUD_STAT_LOG_STOP == pAudLogic->playStat))
    {
        pAudLogic->audConfig.curTimeMedia = pAudLogic->pPlayer->curTime;
    } 
    
    if(0 == pAudLogic->audConfig.volume)
    {
        pAudLogic->audConfig.volume = AUD_DEFAULT_VOLUME;
    }
    
    //save audio name
    Profile_WriteData(offset, &pAudLogic->audConfig);

    return AK_TRUE;
}


#pragma arm section code

#pragma arm section code = "_audioplayer_" 
/**
 * @brief   play cycle handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   cnt
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlPlyCycHandle(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    //play control cyclic handle
    pAudLogic->ctrlCnt ++;

    //reflash handle    
    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_CYC_GET_TIME;

    Aud_AudCtrlSeekFBCycHandle(); //seek cyclic handle

    return AK_TRUE;
}
#pragma arm section code


/**
 * @brief   cyclic play arbitrate handle 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  0:  ok
            1: failed
            2: normal end 
 **/
 #pragma arm section code = "_audioplay_pre_" 
static T_BOOL Aud_AudCtrlCycPlayArb(T_VOID)
{
	T_BOOL ret = AK_FALSE;
	
    if(!MList_IsHaveItem() 
        && LIST_LOAD_END_FLAG == MList_GetLoadFlag())
    {
        return AK_FALSE;
    }

    if(AUD_CYC_MODE_NORMAL == pAudLogic->audConfig.cycMode)
    {
		ret = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_NEXT, AK_FALSE);
    }
    else if(AUD_CYC_MODE_SINGLE == pAudLogic->audConfig.cycMode)
    {
    	ret = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_CUR, AK_TRUE);
    }
    else if(AUD_CYC_MODE_TOTAL == pAudLogic->audConfig.cycMode)
    {
        ret = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_NEXT, AK_TRUE);
    }
    else
    {
        ret = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_NEXT, AK_TRUE);
    }

    return ret;
}
 

/**
 * @brief   action post handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlActPreHandle(T_AUD_LOG_PRE logPre)
{
    T_BOOL rst = AK_FALSE;

    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }
    pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 

    if(!MList_IsHaveItem()
        && LIST_LOAD_END_FLAG == MList_GetLoadFlag())
    {
        AK_DEBUG_OUTPUT("Aud_AudCtrlActPreHandle no music\n");
        return AK_FALSE;
    }

    switch(logPre)
    {
        case AUD_LOG_PRE_CURR:
            rst = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_CUR, AK_TRUE);
            break;
            
        case AUD_LOG_PRE_PREV:
            rst = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_PRE, AK_TRUE);
            break;  
        case AUD_LOG_PRE_NEXT:
            rst = MList_GetItem(pAudLogic->musicPath, LIST_FIND_DIR_NEXT, AK_TRUE);
            break;
            
        case AUD_LOG_PRE_AUTOCON:
            rst = Aud_AudCtrlCycPlayArb();
            break;
        default:
            rst = AK_FALSE;
            break;

    }

    if (rst)
    {
        //open audio
        if(!Aud_PlayerOpenEx(pAudLogic->pPlayer,pAudLogic->musicPath,Aud_GetMediaType(pAudLogic->musicPath),AK_TRUE))
        {
            AK_DEBUG_OUTPUT("PlayerOpen failed\n");
            pAudLogic->preHandleOK = AUD_PRO_HANDLE_PLAYERR; 

			if (AUD_OPEN_FLAG_OK == pAudLogic->openflag)
			{
				//只要有能打开的音频，就可以继续自动切换歌曲
				return AK_FALSE;
			}
			else if (AUD_OPEN_FLAG_NULL == pAudLogic->openflag)
			{
				pAudLogic->openflag = AUD_OPEN_FLAG_ERR;

				//第一首就不能打开，记录下音频路径名
				if (AK_NULL != pAudLogic->failpath)
				{
					pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
				}

				pAudLogic->failpath = Fwl_Malloc((Utl_UStrLen(pAudLogic->musicPath)+1)<<1);

				if (AK_NULL != pAudLogic->failpath)
				{
					Utl_UStrCpyN(pAudLogic->failpath, pAudLogic->musicPath, Utl_UStrLen(pAudLogic->musicPath));
				}

				AK_DEBUG_OUTPUT("failpath : ");
				Printf_UC(pAudLogic->failpath);

				if (pAudLogic->bDividFolders)
				{
					if ((AK_NULL != pAudLogic->failPathFirstFolder) 
						&& (0 == Utl_UStrCmp(pAudLogic->failPathFirstFolder, pAudLogic->musicPath)))
					{
						//循环切换一圈文件夹了，所有文件夹都没有能打开的音频
						pAudLogic->bNoValidFileAllFolder = AK_TRUE;
					}
				}
			}
			else if (AUD_OPEN_FLAG_ERR == pAudLogic->openflag)
			{
				if ((AK_NULL != pAudLogic->failpath) 
					&& (0 == Utl_UStrCmp(pAudLogic->failpath, pAudLogic->musicPath)))
				{
					pAudLogic->preHandleErrCnt++;
				}

				if (pAudLogic->preHandleErrCnt >= 2)
				{
					//循环切换一圈音频了，(此文件夹内)没有能打开的音频
					pAudLogic->bNoValidFile = AK_TRUE;

					if (pAudLogic->bDividFolders)
					{
						if (0 == pAudLogic->folderErrCnt)
						{
							//此文件夹不能播，记录下音频路径名
							if (AK_NULL != pAudLogic->failPathFirstFolder)
							{
								pAudLogic->failPathFirstFolder = Fwl_Free(pAudLogic->failPathFirstFolder);
							}

							pAudLogic->failPathFirstFolder = Fwl_Malloc((Utl_UStrLen(pAudLogic->musicPath)+1)<<1);

							if (AK_NULL != pAudLogic->failPathFirstFolder)
							{
								Utl_UStrCpyN(pAudLogic->failPathFirstFolder, pAudLogic->musicPath, Utl_UStrLen(pAudLogic->musicPath));
								AK_DEBUG_OUTPUT("failPathFirstFolder : ");
								Printf_UC(pAudLogic->failPathFirstFolder);
							}
							pAudLogic->folderErrCnt++;
						}
					}
				}
			}

            return AK_FALSE;
        }

#ifdef OS_ANYKA
#if (STORAGE_USED == NAND_FLASH)
        Fwl_FileSetBufSize(pAudLogic->pPlayer->hFile, Fwl_Nand_BytesPerSector(AK_NULL)>>10);
        AK_DEBUG_OUTPUT("\nnand page size:%d\n", Fwl_Nand_BytesPerSector(AK_NULL));
#endif         
#endif

        pAudLogic->preHandleOK = AUD_PRO_HANDLE_OK; 
		pAudLogic->openflag = AUD_OPEN_FLAG_OK;
		pAudLogic->bNoValidFile = AK_FALSE;
		pAudLogic->bNoValidFileAllFolder = AK_FALSE;
        pAudLogic->preHandleErrCnt = 0;
		pAudLogic->folderErrCnt = 0;
        pAudLogic->timeA = 0;
        pAudLogic->timeB = 0;

		if (AK_NULL != pAudLogic->failpath)
		{
			pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
		}

		if (AK_NULL != pAudLogic->failPathFirstFolder)
		{
			pAudLogic->failPathFirstFolder = Fwl_Free(pAudLogic->failPathFirstFolder);
		}

        AK_DEBUG_OUTPUT("PreHandle OK\n");
    }
    else
    {
        AK_DEBUG_OUTPUT("PreHandle failed\n");
		return AK_FALSE;
    }
    
    return AK_TRUE;
}

static T_VOID Aud_CtrlTriggerPost(T_EVT_CODE evtCode)
{            
    AK_DEBUG_OUTPUT("post event exit:%d\n", evtCode);

    if((BATTERY_STAT_LOW_SHUTDOWN == gb.batStat) 
        && !Fwl_DetectorGetStatus(DEVICE_CHG))
    {   
        evtCode= M_EVT_Z00_POWEROFF;
        pAudLogic->evtParam.w.Param1 =   EVENT_TYPE_SWITCHOFF_BATLOW;
    }
    else
    {
        if(M_EVT_Z00_POWEROFF == evtCode)
        {
             pAudLogic->evtParam.w.Param1 = EVENT_TYPE_SWITCHOFF_KEY;
        }
    }

    VME_EvtQueuePut(evtCode, &pAudLogic->evtParam);
}

/**
 * @brief   action post handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/

static T_BOOL   Aud_AudCtrlActPostHandle(T_VOID)
{
    T_BOOL bClearFlag = AK_FALSE;
    
    if(AUD_ACT_NULL == pAudLogic->audCtrlActPost)
    {
        return AK_FALSE;
    }

    //AK_DEBUG_OUTPUT("Aud_AudCtrlActPostHandle,playstates:%X,audCtrlActPost:%X\n",
            //pAudLogic->playStat,pAudLogic->audCtrlActPost);

    if (AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
    {
        if (0 != (pAudLogic->audCtrlActPost & AUD_ACT_PRI_1))
        {
            if (AUD_ACT_STOP == (pAudLogic->audCtrlActPost & AUD_ACT_STOP))
            {
                pAudLogic->audCtrlActPost &= ~AUD_ACT_STOP;
                Aud_AudCtrlActStop();
                //reset title
                pAudLogic->ctrlCnt = 0;
                Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR);
            }
            else if (AUD_ACT_PAUSE == (pAudLogic->audCtrlActPost & AUD_ACT_PAUSE))
            {
                pAudLogic->audCtrlActPost &= ~AUD_ACT_PAUSE;
                pAudLogic->audCtrlActPost = AUD_ACT_NULL ;
                pAudLogic->ctrlCnt = 0;

                if(pAudLogic->abplayMode)
                {
                    T_EVT_PARAM EventParm;

                    pAudLogic->abplayMode = AK_FALSE;
                    Aud_AudCtrlActStop();
                    EventParm.p.pParam1 = pAudLogic;
                    VME_EvtQueuePut(M_EVT_ABPLAY, &EventParm);
                }
                if(0xff != pAudLogic->Powerstate)
                {
                    T_EVT_PARAM EventParm;

                    EventParm.c.Param1 = pAudLogic->Powerstate;

                    VME_EvtQueuePut(VME_EVT_VOICE_TIP, &EventParm);
                }else if(pAudLogic->bMaxVol)
                {
                    T_EVT_PARAM EventParm;
                    
                    pAudLogic->bMaxVol = AK_FALSE;
                    //Aud_AudCtrlActStop();
                    EventParm.p.pParam1 = pAudLogic;
                    VME_EvtQueuePut(VME_EVT_MAX_VOLUME, &EventParm);
                }
                    
            }
        }
        else if( 0 != (pAudLogic->audCtrlActPost & AUD_ACT_PRI_2))
        {
            T_AUD_LOG_PRE logPre = AUD_LOG_PRE_NULL;

            if(AUD_ACT_PREVPLAY == (pAudLogic->audCtrlActPost & AUD_ACT_PREVPLAY))
            {           
                logPre = AUD_LOG_PRE_PREV;
                pAudLogic->audCtrlActPost = pAudLogic->audCtrlActPost & ~AUD_ACT_PREVPLAY;        
            }
            else
            {
                if(AUD_ACT_NEXTPLAY == (pAudLogic->audCtrlActPost & AUD_ACT_NEXTPLAY))
                {
                    logPre = AUD_LOG_PRE_NEXT;
                    pAudLogic->audCtrlActPost = pAudLogic->audCtrlActPost & ~AUD_ACT_NEXTPLAY;
                }
            }

            if (AUD_LOG_PRE_NULL != logPre)
            {
                Aud_AudCtrlActStop();

                if(!Aud_AudCtrlActPreHandle(logPre))
                {
                    AK_DEBUG_OUTPUT("AUD_LOG_PRE_NEXT failed\n");
                    pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
                    
                    {  
                        if (AUD_LOG_PRE_PREV == logPre)
                        {
                            Aud_AudCtrlSetKeyAct(kbLEFT, PRESS_SHORT);
                        }
                        else
                        {
                            Aud_AudCtrlSetKeyAct(kbRIGHT, PRESS_SHORT);
                        }

                        if(AK_TRUE != pAudLogic->findnextplay)
                        {
                            pAudLogic->findnextplay = AK_TRUE;
                            pAudLogic->findnextplaytimeout = 0;
                        }
                    }
                    return AK_FALSE;
                }
                
                pAudLogic->ctrlCnt = 0;
                AK_DEBUG_OUTPUT("Switch to play by user key\n");
                if(!Aud_AudCtrlActPlay())
                {
                    //Aud_OpenDACSignal(AK_FALSE);
                }
            }
        }

        if (0 == (pAudLogic->audCtrlActPost & AUD_ACT_PRI_4))
        {
            return AK_TRUE;
        }
    }
    else
    {
        bClearFlag = AK_TRUE;
        
        if (0 != (pAudLogic->audCtrlActPost & AUD_ACT_PRI_1))
        {
            if (AUD_ACT_PAUSE == (pAudLogic->audCtrlActPost & AUD_ACT_PAUSE))
            {
                pAudLogic->audCtrlActPost &= ~AUD_ACT_PAUSE; 
            }
        }
        else if( 0 != (pAudLogic->audCtrlActPost & AUD_ACT_PRI_2))
        {
            if( (AUD_ACT_NEXTPLAY == (pAudLogic->audCtrlActPost & AUD_ACT_NEXTPLAY) ) ||
                    (AUD_ACT_PREVPLAY == (pAudLogic->audCtrlActPost & AUD_ACT_PREVPLAY) ) )
            {                
                AK_DEBUG_OUTPUT("one over and one start");
                
                if(!Aud_AudCtrlActPlay())
                {
                    //Aud_OpenDACSignal(AK_FALSE);
                }               
            }
        }       
    }

    if (0 != (pAudLogic->audCtrlActPost & AUD_ACT_PRI_4))
    {
        if(AUD_ACT_MUSICPLAY== (pAudLogic->audCtrlActPost & AUD_ACT_MUSICPLAY) )
        {
            AK_DEBUG_OUTPUT("alarm play!\n");
            pAudLogic->audCtrlActPost &= ~AUD_ACT_MUSICPLAY;
            Aud_CtrlTriggerPost(M_EVT_Z01_MUSIC_PLAY);
        }
        else if (AUD_ACT_EXIT == (pAudLogic->audCtrlActPost & AUD_ACT_EXIT))
        {
            AK_DEBUG_OUTPUT("pause exit");
            pAudLogic->audCtrlActPost &= ~AUD_ACT_EXIT;     
            m_triggerEvent(M_EVT_EXIT, AK_NULL);        
        } 
        else if (AUD_ACT_POWEROFF == (pAudLogic->audCtrlActPost & AUD_ACT_POWEROFF))
        {            
            AK_DEBUG_OUTPUT("power off exit");
            pAudLogic->audCtrlActPost &= ~AUD_ACT_POWEROFF;                
            Aud_CtrlTriggerPost(M_EVT_Z00_POWEROFF);
        }
        else if(AUD_ACT_USER_DEFINE == (pAudLogic->audCtrlActPost & AUD_ACT_USER_DEFINE))
        {
            AkDebugOutput("use define exit: %d\n", pAudLogic->evtCode);

            pAudLogic->audCtrlActPost &= ~ AUD_ACT_USER_DEFINE;

            if(pAudLogic->evtCode)
            {
                    VME_EvtQueuePut(pAudLogic->evtCode, &pAudLogic->evtParam);
            }   

            //如果消息是发向audio main的则不销毁资源，因为audio main会重新利用这些资源进行播放
            //if(M_EVT_AUDIO_MAIN != pAudLogic->evtCode)
            {
                Aud_AudCtrlFree();
            }
                    
            return AK_TRUE;
        }
    }

    if (bClearFlag)
    {
        pAudLogic->audCtrlActPost = AUD_ACT_NULL;
    }
    
    return AK_TRUE;
}
#pragma arm section code


/**
 * @brief   stop play and Reserve Current Time
 * @author  mayeyu
 * @date    2011-11-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID  Aud_AudCtrlStopAndReserve(T_VOID)
{
    T_U32   curTime = 0;

    if (AK_NULL == pAudLogic)
    {
        return;
    }
    
    if (Aud_AudCtrlActCheckNeedDec())
    {
        APlayer_GetDuration(pAudLogic->pPlayer,&pAudLogic->pPlayer->curTime);
    }
   
    curTime = pAudLogic->pPlayer->curTime;
    //Aud_AudCtrlActPause();
    //Fwl_DelayUs(200000);
    Aud_AudCtrlActStop();
    pAudLogic->pPlayer->curTime = curTime; 
	pAudLogic->audConfig.curTimeMedia = pAudLogic->pPlayer->curTime;
    AK_DEBUG_OUTPUT("ReserveCurTime = %d\n", curTime);  
}


/**
 * @brief   action handle deal pri 1
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/

static T_BOOL Aud_CtrlAct1(T_VOID)
{
    if(AUD_ACT_PLAY == (pAudLogic->audCtrlAct & AUD_ACT_PLAY) )
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_PLAY;

        if(!MList_IsHaveItem() 
            && LIST_LOAD_END_FLAG == MList_GetLoadFlag())
        {//no music
            AK_DEBUG_OUTPUT("Aud_AudCtrlActHandle, no music item\n");
            return AK_FALSE;
        }

        if((AUD_STAT_LOG_STOP == pAudLogic->playStat) || (AUD_STAT_LOG_NULL == pAudLogic->playStat))
        {//in stop status
            if(AUD_PRO_HANDLE_OK != pAudLogic->preHandleOK)
            {//has pre handle OK
                if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
                {//pre handle failed
                    pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
                    AK_DEBUG_OUTPUT("bPreHandle failed,errCnt:%d\n",pAudLogic->preHandleErrCnt);

                    if(AK_TRUE != pAudLogic->findnextplay)
                    {
                        pAudLogic->findnextplay = AK_TRUE;
                        pAudLogic->findnextplaytimeout = 0;
                    }
                    Aud_AudCtrlSetKeyAct(kbRIGHT, PRESS_SHORT);
                    
                    return AK_FALSE;
                }                   
            }

            //pre handle OK
            pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL;
            if(!Aud_AudCtrlActPlay())
            {
                //Aud_OpenDACSignal(AK_FALSE);
            }
        }
    }
    else if(AUD_ACT_STOP == (pAudLogic->audCtrlAct & AUD_ACT_STOP) )
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_STOP;
        if (Aud_AudCtrlActPause())
        {
            pAudLogic->audCtrlActPost |= AUD_ACT_STOP;
        }
        else
        {
            Aud_AudCtrlActStop();
        }
    }
    else if(AUD_ACT_PAUSE == (pAudLogic->audCtrlAct & AUD_ACT_PAUSE) )
    {
        T_BOOL bPause;
        
        bPause = Aud_AudCtrlActPause();
          
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_PAUSE;

        
#ifdef OS_ANYKA
        if (bPause)
        {
            pAudLogic->audCtrlActPost |= AUD_ACT_PAUSE;
        }
        else
        {

            if(MediaLib_GetStatus(pAudLogic->pPlayer->hMedia) == MEDIALIB_END)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct |AUD_ACT_PAUSE;
            }
        }
#endif
    }
    else if(AUD_ACT_RESUME == (pAudLogic->audCtrlAct & AUD_ACT_RESUME) )
    {
        Aud_AudCtrlActResume();
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_RESUME;
    }
    else if(AUD_ACT_VOL_INC == (pAudLogic->audCtrlAct & AUD_ACT_VOL_INC) )
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_VOL_INC;
        Aud_AudCtrlActSetVol(1);
    }
    else if(AUD_ACT_VOL_DEC == (pAudLogic->audCtrlAct & AUD_ACT_VOL_DEC) )
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_VOL_DEC;
        Aud_AudCtrlActSetVol(-1);
    }
    else
    {
        NULL;
    }  

    return AK_TRUE;
}
/**
 * @brief   action handle deal pri 2
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/

static T_BOOL Aud_CtrlAct2(T_VOID)
{
    //AK_DEBUG_OUTPUT("Aud_AudCtrlActHandle,audCtrlAct:%X\n",pAudLogic->audCtrlAct);

    if(AUD_ACT_PREVPLAY == (pAudLogic->audCtrlAct & AUD_ACT_PREVPLAY) )
    {

        if(AUD_ACT_NULL != pAudLogic->audCtrlActPost)
        {
            AK_DEBUG_OUTPUT("Act failed,post processing\n");
            return AK_FALSE;
        }
        pAudLogic->audCtrlAct &= ~AUD_ACT_PREVPLAY;
        Aud_AudCtrlActPause();

        pAudLogic->audCtrlActPost |= AUD_ACT_PREVPLAY;
        #ifdef OS_WIN32
        {
            T_EVT_PARAM pEventParm;
            
            pEventParm.w.Param1 = AUD_CTRL_EVENT_AUDPAUSE;
            VME_EvtQueuePut(M_EVT_AUDIO_CTRL, &pEventParm);
        }
        #endif                  
    
    }
    else if(AUD_ACT_NEXTPLAY == (pAudLogic->audCtrlAct & AUD_ACT_NEXTPLAY) )
    {
        
        if(AUD_ACT_NULL != pAudLogic->audCtrlActPost)
        {
            AK_DEBUG_OUTPUT("Act failed,post processing\n");
            return AK_FALSE;
        }

        pAudLogic->audCtrlAct &= ~AUD_ACT_NEXTPLAY;
        Aud_AudCtrlActPause();

        pAudLogic->audCtrlActPost |= AUD_ACT_NEXTPLAY;
        #ifdef OS_WIN32
        {
            T_EVT_PARAM pEventParm;
            
            pEventParm.w.Param1 = AUD_CTRL_EVENT_AUDPAUSE;
            VME_EvtQueuePut(M_EVT_AUDIO_CTRL, &pEventParm);
        }
        #endif                
          
    }

    if(AUD_ACT_SEEKBACKWARD == (pAudLogic->audCtrlAct & AUD_ACT_SEEKBACKWARD) )
    {
        Aud_AudCtrlActFastPlayBeg(-1);
    }
    else if(AUD_ACT_SEEKFORWARD == (pAudLogic->audCtrlAct & AUD_ACT_SEEKFORWARD) )
    {
        Aud_AudCtrlActFastPlayBeg(1);
    }


    if(AUD_ACT_SEEKENDB == (pAudLogic->audCtrlAct & AUD_ACT_SEEKENDB) )
    {
        //处于seekB状态才结束seekb end
        if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
        {
            //如果支持SEEK ,则SEEK结束之前等待数据传输完毕
            if(Aud_AudCtrlSupportSeek())
            {
                Pcm_Pause(pAudLogic->pPlayer->pcm_id);
//              while(Fwl_DMA_IsSending(FWL_DMA_DAC));
            }
            Aud_AudCtrlActFastPlayEnd(-1);
        }
        else
        {
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKENDB;
        }
    }
    else if(AUD_ACT_SEEKENDF == (pAudLogic->audCtrlAct & AUD_ACT_SEEKENDF))
    {
        //处于seekf状态才结束seekf end
        if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
        {
            //如果支持SEEK ,SEEK结束之前等待数据传输完毕
            if(Aud_AudCtrlSupportSeek())
            {
                Pcm_Pause(pAudLogic->pPlayer->pcm_id);
//              while(Fwl_DMA_IsSending(FWL_DMA_DAC));
            }
            Aud_AudCtrlActFastPlayEnd(1);
        }
        else
        {
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKENDF;
        }
    }
    else
    {
        NULL;
    }

    if(AUD_ACT_PREVDIS == (pAudLogic->audCtrlAct & AUD_ACT_PREVDIS) )
    {
        if(AUD_ACT_NULL != pAudLogic->audCtrlActPost)
        {
            AK_DEBUG_OUTPUT("Act failed,post processing\n");
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_PREVDIS;
            return AK_FALSE;
        }
        pAudLogic->audCtrlActPost = AUD_ACT_NULL ;
        Aud_AudCtrlActStop();
        
        if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_PREV))
        {
            AK_DEBUG_OUTPUT("AUD_LOG_PRE_PREV failed\n");
            pAudLogic->audCtrlAct = AUD_ACT_NULL ;

            if (AK_TRUE == pAudLogic->findnextplay)//continu  find preview music
            {
                Aud_AudCtrlSetKeyAct(kbLEFT, PRESS_SHORT);
            }

            return AK_FALSE;
        }
        else
        {
            if (AK_TRUE == pAudLogic->findnextplay)//have aoto find preview music and play
            {
                Aud_AudCtrlSetKeyAct(PLAY_CONTROL_KEY, PRESS_SHORT);
                pAudLogic->findnextplay = AK_FALSE;
            }
        }
        pAudLogic->audCtrlAct = AUD_ACT_NULL ;
    }
    else if(AUD_ACT_NEXTDIS == (pAudLogic->audCtrlAct & AUD_ACT_NEXTDIS) )
    {
        
        if(AUD_ACT_NULL != pAudLogic->audCtrlActPost)
        {
            AK_DEBUG_OUTPUT("Act failed,post processing\n");
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_NEXTDIS;
            return AK_FALSE;
        }
        pAudLogic->audCtrlActPost = AUD_ACT_NULL ;

        Aud_AudCtrlActStop();
        if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_NEXT))
        {
            AK_DEBUG_OUTPUT("AUD_LOG_PRE_NEXT failed\n");
            pAudLogic->audCtrlAct = AUD_ACT_NULL ;

			if (pAudLogic->bNoValidFile || pAudLogic->bNoValidFileAllFolder)
			{
				if (pAudLogic->bDividFolders)
				{
					if (pAudLogic->bNoValidFileAllFolder)
					{
						//所有文件夹都没有能打开的音频，退出音乐播放模块
						AK_DEBUG_OUTPUT("there's no valid file in all folders, exit!\n");
						m_triggerEvent(M_EVT_EXIT, AK_NULL);
					}
					else
					{
						//此文件夹内没有能打开的音频，切换文件夹
						AK_DEBUG_OUTPUT("there's no valid file in this folder, change folder!\n");
						Aud_AudCtrlSetKeyAct(SET_MENU_KEY, PRESS_LONG);
					}
					
				}
				else
				{
					//循环切换一圈音频了，没有能打开的音频
					AK_DEBUG_OUTPUT("there's no valid file, exit!\n");
					m_triggerEvent(M_EVT_EXIT, AK_NULL);
				}
			}
			else
			{
                if (AK_TRUE == pAudLogic->findnextplay)
	            {
	                Aud_AudCtrlSetKeyAct(kbRIGHT, PRESS_SHORT);
	            }
			}

            
            return AK_FALSE;
        }
        else
        {
            if (AK_TRUE == pAudLogic->findnextplay)
            {
                Aud_AudCtrlSetKeyAct(PLAY_CONTROL_KEY, PRESS_SHORT);
                pAudLogic->findnextplay = AK_FALSE;
            }
        }
        pAudLogic->audCtrlAct = AUD_ACT_NULL ;
    }
    else
    {
    } 
    return AK_TRUE;
}

#pragma arm section code = "_audioplayer_"
T_BOOL  Aud_AudCtrlActHandle(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    if( 0 != (pAudLogic->audCtrlAct & AUD_ACT_PRI_1))
    {
        return Aud_CtrlAct1();
    }
    else if( 0 != (pAudLogic->audCtrlAct & AUD_ACT_PRI_2))
    {
        return Aud_CtrlAct2();
    }
    else if( 0 != (pAudLogic->audCtrlAct & AUD_ACT_PRI_3))
    {
        //AK_DEBUG_OUTPUT("Aud_AudCtrlActHandle,audCtrlAct:%X\n",pAudLogic->audCtrlAct);    
        if (AUD_ACT_CYC_GET_TIME == (pAudLogic->audCtrlAct & AUD_ACT_CYC_GET_TIME))
        {
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_CYC_GET_TIME;
             
            if (Aud_AudCtrlActCheckNeedDec())
            {
                APlayer_GetDuration(pAudLogic->pPlayer,&(pAudLogic->pPlayer->curTime));
            }
        }
        
        pAudLogic->audCtrlAct &= ~AUD_ACT_PRI_3;
    }
    return AK_TRUE;
}
#pragma arm section code


#if(STORAGE_USED == SPI_FLASH)
#pragma arm section code = "audioplayer"
#else
#pragma arm section code = "_music_playing_"
#endif

/**
 * @brief   Aud_AudCtrlGetCurPlayer
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_TIMER
            T_U32
 * @return  T_VOID
 * @retval  
 **/
static T_AUD_PLAYER* Aud_AudCtrlGetCurPlayer(T_VOID)
{
    return pAudLogic->pPlayer;
}
/**
 * @brief   Aud_AudCtrlPlayHandelPre
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_TIMER
            T_U32
 * @return  T_VOID
 * @retval  
 **/
static T_AUD_PLAYER* Aud_AudCtrlPlayHandelPre(T_VOID)
{
    if (AK_NULL != pAudLogic)
    {
        //check whether need to decode 
        if(Aud_AudCtrlActCheckNeedDec())
        {
            return Aud_AudCtrlGetCurPlayer();
        }
    }

    return AK_NULL;
}
/**
 * @brief   Aud_AudCtrlPlayHandelPost
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_TIMER
            T_U32
 * @return  T_VOID
 * @retval  
 **/
static T_VOID   Aud_AudCtrlPlayHandelPost(T_S32 size, T_AUD_PLAYER* pAudPlayer)
{
    T_U8 decType = 0;
    T_EVT_PARAM pEventParm;

    //handle decode result
    if (size < 0)
    {//decode failed 
        T_eMEDIALIB_STATUS decStat = MEDIALIB_END;
        
        if (MEDIALIB_MEDIA_AV == pAudPlayer->mediaType)
        {
//            decStat = AVFPlayer_GetStatus(pAudPlayer->hMedia);
        }
        else
        {
            decStat = MediaLib_GetStatus(pAudPlayer->hMedia);
        }

        //AK_DEBUG_OUTPUT("decode failed,stat:%d",decStat);
        if(MEDIALIB_PAUSE == decStat || Aud_AudCtrlChkActPost(AUD_ACT_USER_DEFINE))
        {//pause end 
        
            //must seng event after the L2 buf is empty
            //must send once 
            if (!Pcm_CheckPause())
            {               
                Pcm_Pause(pAudPlayer->pcm_id);

                pEventParm.w.Param1 = AUD_CTRL_EVENT_AUDPAUSE;
                VME_EvtQueuePut(M_EVT_AUDIO_CTRL, &pEventParm);
            }
        }
        else
        {//other end including file end or decode error
            if (!Pcm_CheckPause())
            {               
                Pcm_Pause(pAudPlayer->pcm_id);

                pEventParm.w.Param1 = AUD_CTRL_EVENT_AUDSTOP;
                if(1 == decType)
                {
                    pEventParm.w.Param2 = AUD_CTRL_STOP_EVT_DECODE;
                }
                else if(2 == decType)
                {           
                    pEventParm.w.Param2 = AUD_CTRL_STOP_EVT_DECODE_REC;
                }
                else
                {
                    pEventParm.w.Param2 = AUD_CTRL_STOP_EVT_NULL;
                }

                VME_EvtQueuePut(M_EVT_AUDIO_CTRL, &pEventParm);
                AK_DEBUG_OUTPUT("Decode failed,Trigger AUD_STAT_LOG_STOP,p2:%d\n",pEventParm.w.Param2);
            }
        }
    }    
}
#pragma arm section code

/**
 * @brief   Audio control timer handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_TIMER
            T_U32
 * @return  T_VOID
 * @retval  
 **/
#if(STORAGE_USED == NAND_FLASH|| STORAGE_USED == SD_CARD)
#pragma arm section code = "_audioplayer_resident_"
#else
#pragma arm section code = "_bootcode1_"
#endif
static T_VOID   Aud_AudCtrlTimerCallBackFunc(T_TIMER timer_id, T_U32 delay)
{
    VME_EvtQueuePutUnique(M_EVT_AUDIO_CTRL_TIMER, AK_NULL, AK_FALSE);
}
#pragma arm section code

/**
 * @brief   play stop event handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlEvtHandle(T_EVT_PARAM *pEventParm)
{
    T_U8 rst;
    
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    if (AUD_CTRL_EVENT_AUDSTOP == pEventParm->w.Param1)
    {
        if((AUD_CTRL_STOP_EVT_DECODE == pEventParm->w.Param2) ||
            (AUD_CTRL_STOP_EVT_NULL == pEventParm->w.Param2))
        {   
            T_U32 actPost;
            T_AUD_LOG_PRE logPre;
                
            actPost= pAudLogic->audCtrlActPost;

            Aud_AudCtrlActStop();

            if(AUD_ACT_PREVPLAY == (actPost & AUD_ACT_PREVPLAY))
            {           
                logPre = AUD_LOG_PRE_PREV;      
            }
            else if(AUD_ACT_NEXTPLAY == (actPost & AUD_ACT_NEXTPLAY))
            {
                logPre = AUD_LOG_PRE_NEXT;
            }
            else
            {
                if((pAudLogic->audCtrlAct & AUD_ACT_PAUSE) == AUD_ACT_PAUSE)
                {
                    actPost= AUD_ACT_PAUSE;//设置为暂停
                    AK_DEBUG_OUTPUT("Set actPost Pause\n");
                }
                logPre= AUD_LOG_PRE_AUTOCON;
            }       

            pAudLogic->audCtrlAct = 0;
            
            if(!Aud_AudCtrlActPreHandle(logPre))
            {
                AK_DEBUG_OUTPUT("one stop and open another fail\n");
                {  
                    if (AUD_LOG_PRE_AUTOCON == logPre 
                    && AUD_CYC_MODE_NORMAL == pAudLogic->audConfig.cycMode
                    && AUD_PRO_HANDLE_NULL == pAudLogic->preHandleOK)//普通模式播放最一首不用继续播了
                    {
                        ;
                    }
                    else
                    {
                        if (AUD_LOG_PRE_PREV == logPre)
                        {
                            Aud_AudCtrlSetKeyAct(kbLEFT, PRESS_SHORT);
                        }
                        else
                        {
                            Aud_AudCtrlSetKeyAct(kbRIGHT, PRESS_SHORT);
                        }
                        
                        if(AK_TRUE != pAudLogic->findnextplay)
                        {
                            pAudLogic->findnextplay = AK_TRUE;
                            pAudLogic->findnextplaytimeout = 0;
                        }
                    }
                }
                
                rst = 0;
                if (AK_NULL == pAudLogic)
                {
                    //后台播放音乐的时候，打开音乐失败，直接返回
                    AK_DEBUG_OUTPUT("Background play,open file failed\r\n");
                    return AK_FALSE;
                }
            }
            else
            {
                rst = 1;
            }
            
            if(AUD_PRO_HANDLE_PLAYERR == pAudLogic->preHandleOK )
            {               
                pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
                return AK_FALSE;
            }       
            else if (0 == rst)
            {
                pAudLogic->ctrlCnt = 0;
                Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR);
            }
            else
            {
                if(0 == actPost)
                {
                    pAudLogic->audCtrlActPost = pAudLogic->audCtrlActPost | AUD_ACT_NEXTPLAY;
                }
                else
                {
                    if (AUD_ACT_NEXTPLAY == (actPost & AUD_ACT_NEXTPLAY))
                    {
                        pAudLogic->audCtrlActPost = AUD_ACT_NEXTPLAY;
                    }
                    else if (AUD_ACT_PREVPLAY == (actPost & AUD_ACT_PREVPLAY))
                    {
                        pAudLogic->audCtrlActPost = AUD_ACT_PREVPLAY;
                    }
                    else
                    {
                        pAudLogic->audCtrlActPost= 0;

                        return AK_TRUE;
                    }  
                }
            }
            
            
        }
        else if(AUD_CTRL_STOP_EVT_DECODE_REC == pEventParm->w.Param2)
        {//record decode stop
            AK_DEBUG_OUTPUT("Play record end\n");
        }
        else
        {
            NULL; //do nothing
        }
    }
    else if (AUD_CTRL_EVENT_AUDPAUSE == pEventParm->w.Param1)
    {
    	/*防止把快进退的状态冲掉，
    		导致快进退操作失败，成了暂停状态
    		*/
    	if (AUD_STAT_LOG_SEEKB != pAudLogic->playStat
			&& AUD_STAT_LOG_SEEKF != pAudLogic->playStat)
    	{
        	pAudLogic->playStat = AUD_STAT_LOG_PAUSE;
    	}

        //Aud_OpenDACSignal(AK_FALSE);
        Aud_AudCtrlFreqSet();   
    }
    Aud_AudCtrlActPostHandle();
    return AK_TRUE;
}

/**
 * @brief   start timer
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
#pragma arm section code = "_audioplay_pre_" 

T_VOID  Aud_AudCtrlTimerStart(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
    	return;
    }

    if(ERROR_TIMER != pAudLogic->audCtrlTimer)
    {
        Fwl_TimerStop(pAudLogic->audCtrlTimer);
        pAudLogic->audCtrlTimer = ERROR_TIMER;
    }
    
    pAudLogic->audCtrlTimer = Fwl_TimerStart(AUDIO_PLAYER_TIMER_TIME, AK_TRUE, Aud_AudCtrlTimerCallBackFunc);
    AK_DEBUG_OUTPUT("Aud_AudCtrlTimerStart:%d\n",pAudLogic->audCtrlTimer);
}
#pragma arm section code


/**
 * @brief   stop timer
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID  Aud_AudCtrlTimerStop(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
    	return;
    }

    if(ERROR_TIMER != pAudLogic->audCtrlTimer)
    {
        Fwl_TimerStop(pAudLogic->audCtrlTimer);
        pAudLogic->audCtrlTimer = ERROR_TIMER;
    }
}


/**
 * @brief   set playing AB speed 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   value
 * @return  T_BOOL
 * @retval  
 **/
#pragma arm section code = "_audioplayer_para_"
T_BOOL Aud_AudCtrlSetABRepTime(T_U32 value)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    pAudLogic->audConfig.abRep = (T_U8)value;
    return AK_TRUE;
}

/**
 * @brief   set playing AB space 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   value
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL Aud_AudCtrlSetABRepSpac(T_U32 value)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    pAudLogic->audConfig.abSpac = (T_U8)value;
    return AK_TRUE;
}

/**
 * @brief   set speed
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetSpeed(T_U32 value)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    //if (AUD_STAT_LOG_PLAY == pAudLogic->playStat)
    {

        pAudLogic->audConfig.speed = (T_U8)value;

        if (AUD_PLAY_NORMAL_SPEED != pAudLogic->audConfig.speed
            && _SD_EQ_MODE_NORMAL != pAudLogic->audConfig.toneMode)
        {
            Aud_AudCtrlSetToneMode(_SD_EQ_MODE_NORMAL);             
        }

        APlayer_SetSpeed(pAudLogic->pPlayer,pAudLogic->audConfig.speed);
        Aud_AudCtrlFreqSet();
    }
    return AK_TRUE;
}
#pragma arm section code

/**
 * @brief   set cycric mode 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
#pragma arm section code = "_audioplayer_menu_"
T_BOOL  Aud_AudCtrlSetCycMode(T_AUD_CYC_MODE mode)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    pAudLogic->audConfig.cycMode = (T_U8)mode;
    return AK_TRUE;
}

T_VOID  Aud_AudCtrlUpdatePastTime(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
    	return;
    }

    if (AUD_STAT_LOG_PLAY == pAudLogic->playStat || AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
    {
        APlayer_GetDuration(pAudLogic->pPlayer,&pAudLogic->pPlayer->curTime);
    //  AK_DEBUG_OUTPUT("*pastTime = %d",pAudLogic->pPlayer->curTime);
    }
    pAudLogic->pPlayer->pastTime = pAudLogic->pPlayer->curTime;
}

/**
 * @brief   set tone mode 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetToneMode(T_EQ_MODE mode)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    //if (AUD_STAT_LOG_PLAY == pAudLogic->playStat)
    {       
        
        pAudLogic->audConfig.toneMode = (T_U8)mode; 

        //when the speed is not 8,reset the speed.  
        if (AUD_PLAY_NORMAL_SPEED != pAudLogic->audConfig.speed
            && _SD_EQ_MODE_NORMAL != pAudLogic->audConfig.toneMode)
        {
            Aud_AudCtrlSetSpeed(AUD_PLAY_NORMAL_SPEED);             
        }

        APlayer_SetToneMode(pAudLogic->pPlayer, (T_U8)mode);      
        Aud_AudCtrlFreqSet();
    }
    return AK_TRUE;
}
#pragma arm section code


/**
 * @brief   set volume
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetVolume(T_U32 value)
{
    T_BOOL ret = AK_FALSE;
    
    if (AK_NULL == pAudLogic)
    {
    	return ret;
    }

    pAudLogic->audConfig.volume = (T_U8)((value > MAX_VOLUME) ? MAX_VOLUME:value);
    pAudLogic->pPlayer->volume = pAudLogic->audConfig.volume;

    ret = APlayer_SetVolume(pAudLogic->audConfig.volume);
    AK_DEBUG_OUTPUT("Set vol:%d\n",value);

    return ret;
}


/**
 * @brief   enter standby handle
 * @author  WuShanwei
 * @date    2008-07-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_AudCtrlEnterStandby(T_EVT_CODE evtCode, T_EVT_PARAM* pEvtParam)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    AK_DEBUG_OUTPUT("Aud_AudCtrlEnterStandby,statJump=%d, actPost:%d playstate:%d\n"
        ,pAudLogic->statJump, pAudLogic->audCtrlActPost, pAudLogic->playStat);

    if( evtCode != M_EVT_Z01_MUSIC_PLAY)
    {       
        if (pAudLogic->statJump == JUMP_SWITCH_OFF)
        {     
            return AK_TRUE;
        }

        if (AUD_ACT_NULL != pAudLogic->audCtrlActPost)
        {
            return AK_FALSE;
        }

         pAudLogic->statJump = JUMP_SWITCH_OFF;  
    }
    else
    {
        pAudLogic->statJump = JUMP_ALARM_CLK;  
    }

    if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
    {
        Aud_AudCtrlActPause();
        #ifdef OS_ANYKA
        if(M_EVT_Z01_MUSIC_PLAY == evtCode)
        {
            pAudLogic->evtParam= *pEvtParam;
            pAudLogic->audCtrlActPost |= AUD_ACT_MUSICPLAY;
        }
        else if(M_EVT_Z00_POWEROFF == evtCode)
        {
            pAudLogic->audCtrlActPost |= AUD_ACT_POWEROFF;  
        }
        else
        {
            return AK_TRUE; // if not the post pro msg
        }
        return AK_FALSE;
        #endif
    }
    else if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat 
            || AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
    {
        T_U32 tmpTime = pAudLogic->pPlayer->curTime;

        AK_DEBUG_OUTPUT("Cur Time:%d\n", tmpTime);
        Aud_AudCtrlActStop();
        pAudLogic->pPlayer->curTime= tmpTime;
    }
#ifdef OS_WIN32
{
    
    pAudLogic->playStat = AUD_STAT_LOG_STOP;        
    Aud_AudCtrlActStop(); 
    Aud_CtrlTriggerPost(evtCode);
}
#endif
    return AK_TRUE;
}


/**
 * @brief   Exit standby handle
 * @author  WuShanwei
 * @date    2008-07-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_AudCtrlExitStandby(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }
    
    AK_DEBUG_OUTPUT("Aud_AudCtrlExitStandby\n");
   
    Aud_AudCtrlFreqSet();
 
    return AK_TRUE;
}

/**
 * @brief   Get display information by external use
 * @author  WuShanwei
 * @date    2008-07-04
 * @param   fileName
            info
 * @return  T_BOOL
 * @retval  AK_TRUE - get successful
            AK_FALSE - get failed 
 **/
T_BOOL Aud_AudCtrlGetExtFileInfo(T_U16* fileName,T_U16* info)
{
    T_AUD_PLAYER*   pPlayer = AK_NULL;

    if (AK_NULL == pAudLogic)
    {
    	return AK_FALSE;
    }

    pPlayer = pAudLogic->pPlayer;

    if(Aud_PlayerOpenEx(pPlayer,fileName,Aud_GetMediaType(fileName),AK_TRUE))
    {
        Utl_UStrCpyN(info,pPlayer->strMscInfo,MAX_AUD_MSC_INFO_LEN_ALL);
        APlayer_Stop(pPlayer);
        return AK_TRUE;
    }   

    return AK_FALSE;
}

/**
 * @brief   Check Audio modem is active
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  active
            AK_FALSE: inactive
 **/
#if(STORAGE_USED == SPI_FLASH)
#pragma arm section code = "audioplayer"
#else
#pragma arm section code = "_frequentcode_"
#endif
T_BOOL  Aud_AudCtrlAvaPowerOff(T_VOID)
{
    if(AK_NULL == pAudLogic)
    {
        return AK_TRUE;
    }

    if( (AUD_STAT_LOG_STOP == pAudLogic->playStat) || 
        (AUD_STAT_LOG_NULL == pAudLogic->playStat) ||
        (AUD_STAT_LOG_PAUSE == pAudLogic->playStat) )
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_BOOL  Aud_AudCtrlAvaBLOff(T_VOID)
{
    if(AK_NULL == pAudLogic)
    {
        return AK_TRUE;
    }

    if((AUD_STAT_LOG_SEEKB == pAudLogic->playStat) || (AUD_STAT_LOG_SEEKF == pAudLogic->playStat))
    {
        return AK_FALSE;
    }


    return AK_TRUE;
}
#pragma arm section code

#pragma arm section code = "_audioplayer_music_"


static T_BOOL Aud_AudCtrlSupportSeek(T_VOID)
{
#ifdef OS_ANYKA
//  if (_SD_MEDIA_TYPE_AMR == pAudLogic->pPlayer->audType)
    {
        T_BOOL seekAble;
        seekAble= MediaLib_AudioIsSeekable(pAudLogic->pPlayer->hMedia);

        if(seekAble)
        {
            AK_DEBUG_OUTPUT("Aud Support Seek\n");
        }
        else
        {
            AK_DEBUG_OUTPUT("Aud Not Support seek\n");
        }

        return seekAble;
    }
#endif
    return AK_TRUE;
}
#pragma arm section code

T_VOID Aud_AudCtrlSetStatJump(T_U8 value)
{
    if (pAudLogic == AK_NULL)
    {
        return;
    }
    pAudLogic->statJump = value;
}


#pragma arm section code = "_frequentcode_"
T_BOOL IsAudplayer(T_VOID)
{
    return (T_BOOL)(AK_NULL != pAudLogic);
}
T_BOOL IsAudplaying(T_VOID)
{
	if((AK_NULL != pAudLogic) && (AUD_STAT_LOG_PAUSE != pAudLogic->playStat) )
	{
    	return AK_TRUE;
	}
	else
	{
    	return AK_FALSE;
	}
}
#pragma arm section code 


#pragma arm section code = "_frequentcode_"

/**
 * @brief   free source of logic layer
 * @author  
 * @date    
 * @param   
 * @return  T_VOID
 * @retval  
 **/
static T_VOID Aud_AudCtrlFree(T_VOID)
{
    T_U8 i;
    T_U32 tmpTime;

    if (AK_NULL == pAudLogic)
    {
    	return;
    }

	if (pAudLogic->pPlayer)
	{
	    if (Aud_AudCtrlActCheckNeedDec())
	    {
	        APlayer_GetDuration(pAudLogic->pPlayer,&pAudLogic->pPlayer->curTime); //更新pAudLogic->pPlayer->curTime
	    }
	    
	    tmpTime = pAudLogic->pPlayer->curTime;//stop时,curTime被置0,在此备份
        //Aud_AudCtrlActPause();
        //Fwl_DelayUs(200000);

	    Aud_AudCtrlActStop();
	    pAudLogic->pPlayer->curTime = tmpTime;
	}

    Aud_AudCtrlSaveData();  
    Fwl_FreqPush(FREQ_APP_MAX);
    APlayer_Destroy(pAudLogic->pPlayer);
    
    Fwl_FreqPop();

    //deal with usb charge 
    if (pAudLogic->bClock2X)
    {
        AK_DEBUG_OUTPUT("retset  clock 2x\n");
        Fwl_Freq_Clr_Add();
        pAudLogic->bClock2X = AK_FALSE;
    }   
    
    for(i=0; i<pAudLogic->freqStackDepth;i++)
    {
        #ifdef OS_ANYKA
        akerror("pop",0, 1);
        #endif
        Fwl_FreqPop();
    }

	if (AK_NULL != pAudLogic->failpath)
	{
		pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
	}

	if (AK_NULL != pAudLogic->failPathFirstFolder)
	{
		pAudLogic->failPathFirstFolder = Fwl_Free(pAudLogic->failPathFirstFolder);
	}

	MList_Destroy();

    AK_DEBUG_OUTPUT("*******Aud_AudCtrlDestroy end\n");

    pAudLogic = Fwl_Free(pAudLogic);
}

#pragma arm section code 

#endif  //#end of define SUPPORT_MUSIC_PLAY

#if(STORAGE_USED == SPI_FLASH)
#pragma arm section code = "audioplayer"
#else
#pragma arm section code = "_frequentcode_"
#endif

/**
 * @brief   background send message(send message while current playing fade out)
 * @author  
 * @date    
 * @param   evtCode:message  
                   pEvtParam:parameter
 * @return  T_BOOL
 * @retval    AK_TURE
                    AK_FALSE
 **/
T_BOOL Aud_BGProcessMsg(T_EVT_CODE evtCode, T_EVT_PARAM *pEvtParam)
{
    if(evtCode)
    {
#ifdef SUPPORT_MUSIC_PLAY
        if(Aud_AudCtrlChkActPost(AUD_ACT_USER_DEFINE))      //如果有有后处理则不直接发送事件，避免双击事件导致消息重发
        {
            pAudLogic->evtParam= *pEvtParam;
            pAudLogic->evtCode= evtCode;
        }
        else
#endif
        {
            VME_EvtQueuePut(evtCode, pEvtParam);
        }
    }

//#endif
    return AK_FALSE;
}

#pragma arm section code 


#ifdef SUPPORT_MUSIC_PLAY

#pragma arm section code = "_audioplay_pre_" 


/**
 * @brief   music play start
 * @author  
 * @date    
 * @param   listStyle:play list style
 *
 * @return  T_VOID
 * @retval    T_U32 error type
 **/
T_U32 Aud_AudCtrlStart(T_AUD_LISTSTLE listStyle)
{    
    T_U32 ret = AUD_ERR_TYPE_NULL;
    
    AK_DEBUG_OUTPUT("Aud_AudCtrlStart liststyle %d!!!!\n", listStyle);
	
	ret = Aud_AudCtrlInit(listStyle);
	
    if(AUD_ERR_TYPE_NULL != ret)
    {
        return ret;
    }

    if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
    {
        AK_DEBUG_OUTPUT("Aud_AudCtrlActPreHandle failed.4\n");
        ret = AUD_ERR_TYPE_NULL;//AUD_ERR_TYPE_FORMATERR;
    }
    else
    {
        Aud_AudCtrlActPlySetOldPos();
    }
    
    AutoBgLightOffDisable();    

    return ret;
}


/**
 * @brief   music play restart
 * @author  
 * @date    
 * @param   listStyle:play list style
 *
 * @return  T_U32
 * @retval     error type
 **/
T_U32 Aud_AudCtrlReStart(T_AUD_LISTSTLE listStyle)
{
	T_U32 ret = AUD_ERR_TYPE_NULL;	

	if(AK_NULL == pAudLogic)
    {
        return ret;
    }

	Aud_AudCtrlActStop();

	pAudLogic->audConfig.curTimeMedia  = 0;
	pAudLogic->openflag = AUD_OPEN_FLAG_NULL;
	pAudLogic->bNoValidFile = AK_FALSE;
	pAudLogic->bNoValidFileAllFolder = AK_FALSE;
    pAudLogic->preHandleErrCnt = 0;
	pAudLogic->folderErrCnt = 0;
	pAudLogic->findnextplay = AK_FALSE;
    pAudLogic->findnextplaytimeout = 0;

	if (AK_NULL != pAudLogic->failpath)
	{
		pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
	}

	if (AK_NULL != pAudLogic->failPathFirstFolder)
	{
		pAudLogic->failPathFirstFolder = Fwl_Free(pAudLogic->failPathFirstFolder);
	}

	if (!Aud_AudListInit(listStyle))
	{
		return AUD_ERR_TYPE_NOFILE;
	}
	
	if (!Aud_AudCtrlUpdateHandle(Aud_GetCurDriver()))
	{//no music 
	    return AUD_ERR_TYPE_NOFILE;
	}
	
	if(!Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
	{
		AK_DEBUG_OUTPUT("Aud_AudCtrlActPreHandle failed.5\n");
		ret = AUD_ERR_TYPE_NULL;
	}
	
	pAudLogic->audCtrlAct |= AUD_ACT_PLAY;	//自动开始播放
	
	return ret;
}


#pragma arm section code 

#pragma arm section code = "_audioplayer_" 

/**
 * @brief   check the audCtrlActPost bit
 * @author  
 * @date    
 * @param   evt : which bit to be check
 * @return  T_VOID
 * @retval    AK_TURE
                   AK_FALSE
 **/
static T_BOOL Aud_AudCtrlChkActPost(T_U32 evt)
{
    if(pAudLogic != AK_NULL)
    {
        if((pAudLogic->audCtrlActPost & evt) == evt)
        {
            return AK_TRUE;
        }
    } 
    return AK_FALSE;
}

T_AUD_CONTROL_STATE Aud_AudCtrlGetPlayState(void)
{
    if(pAudLogic != AK_NULL)
    {
         return pAudLogic->playStat;
    } 
    return AUD_STAT_LOG_NULL; 
}
#pragma arm section code 




/**
 * @brief   action handle - play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL   Aud_AudCtrlActPlay(T_VOID)
{
    //Aud_PlayerOpenHP(pAudLogic->pPlayer);
	pAudLogic->playStat = AUD_STAT_LOG_PLAY;

    Aud_AudCtrlFreqSet();
        
    if(APlayer_Play(pAudLogic->pPlayer))
    {//play OK
        AK_DEBUG_OUTPUT("Play OK\n");
        
        pAudLogic->pPlayer->pastTime = pAudLogic->pPlayer->curTime;

        if (AUD_STAT_LOG_SEEKB != pAudLogic->playStat &&
            AUD_STAT_LOG_SEEKF != pAudLogic->playStat )
        {
            pAudLogic->ctrlCnt = 0;
        }
        
        pAudLogic->playStat = AUD_STAT_LOG_PLAY; 
        
        return AK_TRUE;
    }
    else
    {
        AK_DEBUG_OUTPUT("Aud_AudCtrlActPlay failed\n");
        pAudLogic->playStat = AUD_STAT_LOG_STOP; 

        if(AK_TRUE != pAudLogic->findnextplay)
        {
            pAudLogic->findnextplay = AK_TRUE;
            pAudLogic->findnextplaytimeout = 0;
        }
        
        Aud_AudCtrlSetKeyAct(kbRIGHT, PRESS_SHORT);
        
        return AK_FALSE;
    }
}


/**
 * @brief   action handle - stop
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlActStop(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    APlayer_Stop(pAudLogic->pPlayer); 
    pAudLogic->playStat = AUD_STAT_LOG_STOP; 
    pAudLogic->audCtrlActPost= AUD_ACT_NULL;
    Aud_AudCtrlFreqSet();
    return AK_TRUE;
}

/**
 * @brief   action handle - pause
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
static T_BOOL   Aud_AudCtrlActPause(T_VOID)
{
    if(AUD_STAT_LOG_PLAY != pAudLogic->playStat)
    {
        return AK_FALSE;
    }
    
#ifdef OS_WIN32
    pAudLogic->playStat = AUD_STAT_LOG_PAUSE; 
#endif
    
    {
        T_AUD_PLAYER* pCurPlayer = AK_NULL;

        pCurPlayer = Aud_AudCtrlGetCurPlayer();
        if (AK_NULL != pCurPlayer)
        {
            return APlayer_Pause(pCurPlayer);
        }
        else
        {
            return AK_FALSE;
        }
    }
    return AK_TRUE;
}

/**
 * @brief   action handle - pause
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlActResume(T_VOID)
{
    AK_DEBUG_OUTPUT("Resume\n");
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    pAudLogic->playStat = AUD_STAT_LOG_PLAY;
        
    Aud_AudCtrlFreqSet();
    
    {
        T_AUD_PLAYER* pCurPlayer = AK_NULL;

        pCurPlayer = Aud_AudCtrlGetCurPlayer();
        if (AK_NULL != pCurPlayer)
        {
            if(!APlayer_Resume(pCurPlayer))
            {
                return AK_FALSE;
            }
            pAudLogic->pPlayer->pastTime = pAudLogic->pPlayer->curTime;
        }
    }
    return AK_TRUE;
}

/**
 * @brief   action handle - fast play begin
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   dir
            -1: backward, 1-forward
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
static T_BOOL   Aud_AudCtrlActFastPlayBeg(T_S8 dir)
{
    T_U32 seekTime;

    if( -1 == dir)
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKBACKWARD;
        seekTime = (pAudLogic->pPlayer->curTime < pAudLogic->seeklen) ? 
                    0 : (pAudLogic->pPlayer->curTime-pAudLogic->seeklen);
		
		AK_DEBUG_OUTPUT("before seek back curtime = %d\n", pAudLogic->pPlayer->curTime);
        pAudLogic->pPlayer->curTime = seekTime;
		AK_DEBUG_OUTPUT("after seek back curtime = %d\n", pAudLogic->pPlayer->curTime);
        pAudLogic->playStat = AUD_STAT_LOG_SEEKB;
    }
    else if(1 == dir)
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKFORWARD;
        seekTime = ((pAudLogic->pPlayer->curTime+pAudLogic->seeklen) > pAudLogic->pPlayer->totalTime) ? 
                    (pAudLogic->pPlayer->totalTime) : (pAudLogic->pPlayer->curTime+pAudLogic->seeklen);
		
		AK_DEBUG_OUTPUT("before seek fast curtime = %d\n", pAudLogic->pPlayer->curTime);
		pAudLogic->pPlayer->curTime = seekTime;
		AK_DEBUG_OUTPUT("after seek fast curtime = %d\n", pAudLogic->pPlayer->curTime);
        pAudLogic->playStat = AUD_STAT_LOG_SEEKF;
    }

    return AK_TRUE;
}

/**
 * @brief   action handle - fast play end
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   dir
            -1: backward, 1-forward
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
static T_BOOL   Aud_AudCtrlActFastPlayEnd(T_S8 dir)
{
    T_S32 rstSeek;

    if(-1 == dir)
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKENDB;
        if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
        {
            rstSeek = APlayer_Seek(pAudLogic->pPlayer,pAudLogic->pPlayer->curTime); 
            AK_DEBUG_OUTPUT("AUD_STAT_LOG_SEEKB -> AUD_STAT_LOG_PLAY,rstSeek:%d\n",rstSeek);
            if((rstSeek >= 0) && (pAudLogic->pPlayer->totalTime != pAudLogic->pPlayer->curTime))
            {//seek OK
                //pAudLogic->playStat = AUD_STAT_LOG_PLAY;
            //  Aud_AudCtrlActPlay();
                Aud_AudCtrlActResume();
                return AK_TRUE;
            }
            else
            {//seek failed
                AK_DEBUG_OUTPUT("failed\n");
                pAudLogic->playStat = AUD_STAT_LOG_PLAY;
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_NEXTPLAY;
                return AK_FALSE;
            }
        }
    }
    else if(1 == dir)
    {
        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct & ~AUD_ACT_SEEKENDF;
        if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
        {
            rstSeek = APlayer_Seek(pAudLogic->pPlayer,pAudLogic->pPlayer->curTime); 
            AK_DEBUG_OUTPUT("AUD_ACT_SEEKENDF -> AUD_STAT_LOG_PLAY,rstSeek:%d\n",rstSeek);
            if((rstSeek >= 0) && (pAudLogic->pPlayer->totalTime != pAudLogic->pPlayer->curTime))
            {//seek OK
                //pAudLogic->playStat = AUD_STAT_LOG_PLAY;
                //Aud_AudCtrlActPlay();
                Aud_AudCtrlActResume();
                return AK_TRUE;
            }
            else
            {//seek failed
                T_EVT_PARAM pEventParm;
                
                AK_DEBUG_OUTPUT("seek end failed\n");
                //pAudLogic->playStat = AUD_STAT_LOG_PAUSE;
                pEventParm.w.Param1 = AUD_CTRL_EVENT_AUDSTOP;   
                pEventParm.w.Param2 = AUD_CTRL_STOP_EVT_NULL;        
                VME_EvtQueuePut(M_EVT_AUDIO_CTRL, &pEventParm);
                return AK_FALSE;
            }
        }
    }

    return AK_FALSE;
}

/**
 * @brief   action handle - set old position
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
T_BOOL  Aud_AudCtrlActPlySetOldPos(T_VOID)
{
    T_S32 rst; 
    
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    AK_DEBUG_OUTPUT("Aud_AudCtrlActPlySetOldPos,curT:%d\n",pAudLogic->audConfig.curTimeMedia);

    if(pAudLogic->audConfig.curTimeMedia != 0)
    {
        if ( pAudLogic->audConfig.curTimeMedia >= pAudLogic->pPlayer->totalTime)
        {
            pAudLogic->audConfig.curTimeMedia = pAudLogic->pPlayer->totalTime;
        }
            
        rst = APlayer_Seek(pAudLogic->pPlayer,pAudLogic->audConfig.curTimeMedia); 
        if(rst >= 0)
        {
            return AK_TRUE;
        }
    }

    return AK_FALSE;
}

#pragma arm section code = "_audioplayer_" 

/**
 * @brief   action handle - set volume
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   dir
            -1: decrease
            1: increase
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
static T_BOOL   Aud_AudCtrlActSetVol(T_S8 dir)
{
    T_U32 vol;
    if(1 == dir)
    {
        vol = ((pAudLogic->audConfig.volume+1) > MAX_VOLUME) ? MAX_VOLUME:(pAudLogic->audConfig.volume+1);
        Aud_AudCtrlSetVolume(vol);
        if(MAX_VOLUME == vol)
        {
            Aud_AudCtrlActMaxVolPause();
        }
    }
    else if(-1 == dir)
    {
        vol = ((pAudLogic->audConfig.volume) > 0) ? (pAudLogic->audConfig.volume-1):0;
        Aud_AudCtrlSetVolume(vol);
    }

    return AK_TRUE;
}


/**
 * @brief   action handle - set config volume mode
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_BOOL
            AK_FALSE: stop config volume mode   
            AK_TRUE: begin config volume mode
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
T_BOOL  Aud_AudCtrlActSetCfgVolMode(T_BOOL volMode)
{
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }
    
    pAudLogic->setVolMode = volMode;
    return AK_TRUE;
}

/**
 * @brief   action handle - get config volume mode
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_BOOL
            AK_FALSE: stop config volume mode   
            AK_TRUE: begin config volume mode
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
T_BOOL  Aud_AudCtrlActGetCfgVolMode(T_VOID)
{
    if (AK_NULL == pAudLogic)
    {
        return AK_FALSE;
    }

    return pAudLogic->setVolMode;
}
#pragma arm section code 


/**
 * @brief   action handle - check whether need decode process
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  need to decode 
            AK_FALSE:  no need to decode 
 **/
#pragma arm section code = "_music_playing_"

static T_BOOL   Aud_AudCtrlActCheckNeedDec(T_VOID)
{
    if(AUD_STAT_LOG_PLAY != pAudLogic->playStat ) 
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}
#pragma arm section code

static T_BOOL Aud_AudCtrlDealKeyVol(T_EVT_PARAM *pEventParm)
{
    T_BOOL ret = AK_FALSE;
    T_eKEY_ID keyID = pEventParm->c.Param1;
    T_ePRESS_TYPE type = pEventParm->c.Param2;
    
    switch(keyID)
    {
    case kbVOLSUB:
        //if(Aud_AudCtrlActGetCfgVolMode())
        {
            if(PRESS_SHORT == type || PRESSING == type)
            {
                if (pAudLogic->audConfig.volume > 0)
                {
                    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_VOL_DEC;
                }
            }
            ret = AK_TRUE;
        }
        break;

    case kbVOLADD:
        //if(Aud_AudCtrlActGetCfgVolMode())
        {
            if(PRESS_SHORT == type || PRESSING == type)
            {
                if (pAudLogic->audConfig.volume < MAX_VOLUME)
                {
                    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_VOL_INC;
                }
            }
            ret = AK_TRUE;
        }
        break;

    case kbVOLUME:
        AK_DEBUG_OUTPUT("M_EVT_AUD_PARA\n");
        ret = AK_TRUE;
        break;
    case kbLEFT:
        if((PRESS_LONG == type || PRESSING == type))
        {
            if (pAudLogic->audConfig.volume > 0)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_VOL_DEC;
            }
            ret = AK_TRUE;
        }
        break;
    case kbRIGHT:
        if((PRESS_LONG == type || PRESSING == type))
        {
            if (pAudLogic->audConfig.volume < MAX_VOLUME)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_VOL_INC;
            }
            ret = AK_TRUE;
        }
        break;
   case kbMUTE:
       if(PRESS_SHORT == type)
       {
           if(pAudLogic->audConfig.volume == 0)
           {
                Aud_AudCtrlSetVolume(pAudLogic->saveMuteVol);
                AK_DEBUG_OUTPUT("RECOVER!\r\n");
           }
           else
           {
                pAudLogic->saveMuteVol = pAudLogic->audConfig.volume;
                Aud_AudCtrlSetVolume(0);
                AK_DEBUG_OUTPUT("MUTE!\r\n");
           }
           ret = AK_TRUE;
       }
       break;
    default:
        break;
    }
    
    return ret;     
}

static T_BOOL Aud_AudCtrlChkActPostEvt(T_VOID)
{
     if (0 < (pAudLogic->audCtrlAct & (~AUD_ACT_CYC_GET_TIME)) ||
            AUD_ACT_NULL != pAudLogic->audCtrlActPost)
     {
            //have action
        return AK_TRUE;
     }
     
     return AK_FALSE;
}
/**
 * @brief   convert key to action base on status in Audio player
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   pEventParm
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/

static T_BOOL   Aud_AudCtrlKey2ActAudio(T_EVT_PARAM *pEventParm)
{
    T_eKEY_ID keyID = pEventParm->c.Param1;
    T_ePRESS_TYPE type = pEventParm->c.Param2;
    T_S8 file_dir = 1;
    
    AK_DEBUG_OUTPUT("keyId = %d, type = %d\n",keyID,type);

    switch(keyID)
    {
    case kbBACKWARD:
        if (PRESS_LONG == type) //|| (PRESS_SHORT == type))
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
        
            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                if(Aud_AudCtrlSupportSeek())
                {
                    APlayer_Pause(pAudLogic->pPlayer);
                    MediaLib_SetAudioSeek();
                    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKBACKWARD;
                    pAudLogic->ctrlCnt = AUD_SEEK_TIMEOUT_CNT;
                }
            }
        }
        else if(PRESS_UP == type)
        {
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKENDB; 
        }
        else if (PRESSING == type)
		{
			if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
		    {
		        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKBACKWARD;
				pAudLogic->ctrlCnt = AUD_SEEK_TIMEOUT_CNT;
		    }
		}
        break;

    case kbFORWARD:
        if (PRESS_LONG == type) //|| (PRESS_SHORT == type))
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }

            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                if(Aud_AudCtrlSupportSeek())
                {
                    APlayer_Pause(pAudLogic->pPlayer);
                    MediaLib_SetAudioSeek();
                    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKFORWARD;
                    pAudLogic->ctrlCnt = AUD_SEEK_TIMEOUT_CNT;
                }
            }
        }
        else if(PRESS_UP == type)
        {
            pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKENDF; 
        }
        else if (PRESSING == type)
		{
			if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
		    {
		        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKFORWARD;
				pAudLogic->ctrlCnt = AUD_SEEK_TIMEOUT_CNT;
		    }
		}
        break;
        break;
    case kbLEFT:
    case kbPRE:
        if(PRESS_SHORT == type)
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PREVPLAY;
            }
            else if(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
            {
                pAudLogic->findnextplay = AK_TRUE;
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PREVDIS;
            }
            else if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKENDB; 
            }
            else
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PREVDIS;
            }
        }
        break;
    case kbRIGHT:
    case kbNEXT: 
        if(PRESS_SHORT == type)
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_NEXTPLAY;
            }
            else if(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
            {
                pAudLogic->findnextplay = AK_TRUE;
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PREVDIS;
            }
            else if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKENDF; 
            }
            else
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_NEXTDIS;
            }
        }
        break;
    case kbEQ:
        if(PRESS_SHORT == type)
        {
            pAudLogic->pPlayer->bset_eq_flag = AK_TRUE;

            pAudLogic->pPlayer->modeEQ++;
            pAudLogic->pPlayer->modeEQ = pAudLogic->pPlayer->modeEQ > 7?0:pAudLogic->pPlayer->modeEQ;
            pAudLogic->audConfig.toneMode = pAudLogic->pPlayer->modeEQ;
            
            Aud_AudCtrlFreqSet();
            AK_DEBUG_OUTPUT("\n%s!\n", m_EqPrintf[pAudLogic->pPlayer->modeEQ]);
        }
        break;
    case kbPREFILE:
    case kbNEXTFILE:
    case PLAY_CONTROL_KEY:
        if((PLAY_CONTROL_KEY == keyID)&&(PRESS_SHORT == type))
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
            
            if(Aud_AudCtrlActGetCfgVolMode())
            {
                Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
            }

            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PAUSE;
            }
            else if(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_RESUME;
            }
            else if((AUD_STAT_LOG_STOP == pAudLogic->playStat) || (AUD_STAT_LOG_NULL == pAudLogic->playStat)) 
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PLAY;
            }
            else if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKBACKWARD;
            }
            else if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKFORWARD;
            }
            else
            {
            }
        }
        else if(((kbPREFILE == keyID)||(kbNEXTFILE == keyID))&&(PRESS_SHORT== type) || (PLAY_CONTROL_KEY == keyID)&&(PRESS_LONG == type))
        {
            file_dir = keyID == kbPREFILE?LIST_FIND_DIR_PRE:LIST_FIND_DIR_NEXT;
            
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
            
            Aud_AudCtrlStopAndReserve();
            
            if (MList_ChangeFolder(file_dir))
            {
                if(LIST_FIND_DIR_NEXT == file_dir)
                {
                    AK_DEBUG_OUTPUT("Change Next Folder!\n");
                }
                else
                {
                    AK_DEBUG_OUTPUT("Change Pre Folder!\n");
                }

                pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
                pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
                pAudLogic->openflag = AUD_OPEN_FLAG_NULL;
                pAudLogic->bNoValidFile = AK_FALSE;
                pAudLogic->preHandleErrCnt = 0;
                pAudLogic->findnextplay = AK_FALSE;
                pAudLogic->findnextplaytimeout = 0;
        
                if (AK_NULL != pAudLogic->failpath)
                {
                    pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
                }
            }
            else 
            {
                if (pAudLogic->bNoValidFile)
                {
                    AK_DEBUG_OUTPUT("there's no valid file in this folder and no other folder, exit!\n");
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);
                }
				else
				{
					if(Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
				    {
				    	Aud_AudCtrlActPlySetOldPos();
				    }

					pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
				}
            }
        }
        break;
    case SET_MENU_KEY:
        if(PRESS_SHORT == type)
        {           
            //cancel the vol display
            if(Aud_AudCtrlActGetCfgVolMode())
            {
                Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
            }

            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                //音频快进或快退等状态切换未完成不进行状态机的切换
                if (((AUD_ACT_NULL == pAudLogic->audCtrlAct) || (AUD_ACT_CYC_GET_TIME == pAudLogic->audCtrlAct))
                    && AUD_ACT_NULL == pAudLogic->audCtrlActPost)
                {
                    Aud_AudCtrlActPause();
                    
                    MenuCfg_Init(eAUDMENU_MODE_MAX);
					MenuCfg_AddMenu(eAUDMENU_MODE_CYC, AUD_CYC_MODE_MAX, pAudLogic->audConfig.cycMode, (T_STR_SHOW*)(&m_CycModePrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_EQ, _SD_EQ_USER_DEFINE + 1, pAudLogic->audConfig.toneMode, (T_STR_SHOW*)(&m_EqPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_SPEED, AUD_PLAY_SETG_MAX_SPEED_STEP, pAudLogic->audConfig.speed, (T_STR_SHOW*)(&m_SpeedPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_SEEKLEN, AUD_SEEK_V_MAX, pAudLogic->audConfig.seeklenId, (T_STR_SHOW*)(&m_SeeklenPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_UPDATELIST, 1, 0, (T_STR_SHOW*)(&m_UpdateListPrintf));

					m_triggerEvent(M_EVT_MENU, pEventParm);//播放模式选择
                }
                return AK_FALSE;
            }
            else
            {
                //音频快进或快退等状态切换未完成不进行状态机的切换
                if (((AUD_ACT_NULL == pAudLogic->audCtrlAct) || (AUD_ACT_CYC_GET_TIME == pAudLogic->audCtrlAct))
                    && AUD_ACT_NULL == pAudLogic->audCtrlActPost)
                {
                    //m_triggerEvent(M_EVT_EXIT, AK_NULL);
                    MenuCfg_Init(eAUDMENU_MODE_MAX);
					MenuCfg_AddMenu(eAUDMENU_MODE_CYC, AUD_CYC_MODE_MAX, pAudLogic->audConfig.cycMode, (T_STR_SHOW*)(&m_CycModePrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_EQ, _SD_EQ_USER_DEFINE + 1, pAudLogic->audConfig.toneMode, (T_STR_SHOW*)(&m_EqPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_SPEED, AUD_PLAY_SETG_MAX_SPEED_STEP, pAudLogic->audConfig.speed, (T_STR_SHOW*)(&m_SpeedPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_SEEKLEN, AUD_SEEK_V_MAX, pAudLogic->audConfig.seeklenId, (T_STR_SHOW*)(&m_SeeklenPrintf));
					MenuCfg_AddMenu(eAUDMENU_MODE_UPDATELIST, 1, 0, (T_STR_SHOW*)(&m_UpdateListPrintf));
					
                    m_triggerEvent(M_EVT_MENU, pEventParm);
                }
                return AK_FALSE;
            }
        }
        /*else if(PRESS_LONG == type)
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
			
			Aud_AudCtrlStopAndReserve();

			if (MList_ChangeFolder(LIST_FIND_DIR_NEXT))
			{
				AK_DEBUG_OUTPUT("Change Folder!\n");  
				//Aud_AudCtrlActStop();
				pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
				pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
				pAudLogic->openflag = AUD_OPEN_FLAG_NULL;
				pAudLogic->bNoValidFile = AK_FALSE;
			    pAudLogic->preHandleErrCnt = 0;
				pAudLogic->findnextplay = AK_FALSE;
    			pAudLogic->findnextplaytimeout = 0;

				if (AK_NULL != pAudLogic->failpath)
				{
					pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
				}
			}
			else 
			{
				if (pAudLogic->bNoValidFile)
				{
					AK_DEBUG_OUTPUT("there's no valid file in this folder and no other folder, exit!\n");
					m_triggerEvent(M_EVT_EXIT, AK_NULL);				
				}
				else
				{
					if(Aud_AudCtrlActPreHandle(AUD_LOG_PRE_CURR))
				    {
				    	Aud_AudCtrlActPlySetOldPos();
				    }

					pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
				}
				
			}
        }*/
        break;

    case kbRECORD:
        AK_DEBUG_OUTPUT("AB/REC\n");
        
        if (Aud_AudCtrlChkActPostEvt())
        {
            break;
        }
    
        //cancel the vol display
        if(Aud_AudCtrlActGetCfgVolMode())
        {
            Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
        }

        break;

    /*case PLAY_CONTROL_KEY:
        if(PRESS_SHORT == type)
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
            
            if(Aud_AudCtrlActGetCfgVolMode())
            {
                Aud_AudCtrlActSetCfgVolMode(AK_FALSE);
            }

            if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PAUSE;
            }
            else if(AUD_STAT_LOG_PAUSE == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_RESUME;
            }
            else if((AUD_STAT_LOG_STOP == pAudLogic->playStat) || (AUD_STAT_LOG_NULL == pAudLogic->playStat)) 
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PLAY;
            }
            else if(AUD_STAT_LOG_SEEKB == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKBACKWARD;
            }
            else if(AUD_STAT_LOG_SEEKF == pAudLogic->playStat)
            {
                pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_SEEKFORWARD;
            }
            else
            {
            }
        }
        else if(PRESS_LONG == type)
        {
            if (Aud_AudCtrlChkActPostEvt())
            {
                break;
            }
        
            if (MList_ChangeFolder(LIST_FIND_DIR_NEXT))
            {
                AK_DEBUG_OUTPUT("Change Folder!\n");  
                Aud_AudCtrlActStop();
                pAudLogic->audCtrlAct |= AUD_ACT_PLAY;
                pAudLogic->preHandleOK = AUD_PRO_HANDLE_NULL; 
                pAudLogic->openflag = AUD_OPEN_FLAG_NULL;
                pAudLogic->bNoValidFile = AK_FALSE;
                pAudLogic->preHandleErrCnt = 0;
                pAudLogic->findnextplay = AK_FALSE;
                pAudLogic->findnextplaytimeout = 0;
        
                if (AK_NULL != pAudLogic->failpath)
                {
                    pAudLogic->failpath = Fwl_Free(pAudLogic->failpath);
                }
            }
            else 
            {
                if (pAudLogic->bNoValidFile)
                {
                    AK_DEBUG_OUTPUT("there's no valid file in this folder and no other folder, exit!\n");
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);
                }
            }
        }
        break;*/
        
    /*case kbLEFT:
        if(PRESS_SHORT == type)
        {
            APlayer_GetDuration(pAudLogic->pPlayer,&pAudLogic->pPlayer->curTime);
            pAudLogic->timeA = pAudLogic->pPlayer->curTime;
            AK_DEBUG_OUTPUT("timeA = %d\n", pAudLogic->timeA);
        }
        break;
    case kbRIGHT:
        if(PRESS_SHORT == type)
        {
            if (pAudLogic->timeA > 0 && pAudLogic->timeA < pAudLogic->pPlayer->totalTime)
            {
                APlayer_GetDuration(pAudLogic->pPlayer,&pAudLogic->pPlayer->curTime);
                pAudLogic->timeB = pAudLogic->pPlayer->curTime;
                AK_DEBUG_OUTPUT("timeB = %d\n", pAudLogic->timeB);

                if (pAudLogic->timeB > pAudLogic->timeA)
                {
                    if(AUD_STAT_LOG_PLAY == pAudLogic->playStat)
                    {
                        pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PAUSE;
                        pAudLogic->abplayMode = AK_TRUE;
                    }
                    else
                    {
                        pAudLogic->timeB = 0;
                        pAudLogic->timeA = 0;
                    }
                }
            }
        }
        break;*/
    default:
        break;

    }

    AK_DEBUG_OUTPUT("Key2Act,Act:0x%X,post:0x%X\n",\
        pAudLogic->audCtrlAct, pAudLogic->audCtrlActPost);
    return AK_TRUE;
}

T_VOID Aud_AudCtrlActPowChgPause(T_VOID)
{
    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PAUSE;
}

T_VOID Aud_AudCtrlActMaxVolPause(T_VOID)
{
    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_PAUSE;
    pAudLogic->bMaxVol = AK_TRUE;
}

T_VOID Aud_AudCtrlActAutoResume(T_VOID)
{
    pAudLogic->audCtrlAct = pAudLogic->audCtrlAct | AUD_ACT_RESUME;
    pAudLogic->Powerstate = 0xff;
}

/**
 * @brief   convert key to action base on status
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   pEventParm
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlKey2Act(T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VOICE_TIP
    if((MAX_VOLUME == pAudLogic->audConfig.volume)&&
        ((kbRIGHT == pEventParm->c.Param1)&&(PRESS_LONG == pEventParm->c.Param2) ||
        (kbVOLADD == pEventParm->c.Param1)&&(PRESS_LONG == pEventParm->c.Param2 || PRESS_SHORT == pEventParm->c.Param2)))
    {
        Aud_AudCtrlActMaxVolPause();
        return AK_TRUE;
    }
    else if(Aud_AudCtrlDealKeyVol(pEventParm))
    {
        return AK_TRUE;
    }
#else
    if(Aud_AudCtrlDealKeyVol(pEventParm))
    {
        return AK_TRUE;
    }
#endif

    return Aud_AudCtrlKey2ActAudio(pEventParm);
}


/**
* @brief get list style
*
* @author Songmengxing
* @date 2013-04-19
*
* @param T_VOID
*
* @return T_AUD_LISTSTLE
* @retval listStyle
*/
T_AUD_LISTSTLE Aud_AudGetListStyle(T_VOID)
{
	if (AK_NULL != pAudLogic)
	{
		return pAudLogic->audConfig.ListStyle;
	}

	return eAUD_NUM;
}


/**
* @brief media list init
*
* @author Songmengxing
* @date 2013-04-19
*
* @param in T_AUD_LISTSTLE listStyle : listStyle 
*
* @return T_BOOL
* @retval 
*/
static T_BOOL Aud_AudListInit(T_AUD_LISTSTLE listStyle)
{
	T_USTR_FILE searchpath = {0};
	T_U16 SOLIDUS[2] = {'/',0};
	T_U16 REC_fold[11] = {'A', 'U', 'D', 'I', 'O', '_', 'R', 'E', 'C', '/',0};
	T_U16* fileFormat = AK_NULL;
	T_U16* skipFolder = AK_NULL;

	AK_ASSERT_VAL((listStyle < eAUD_NUM), "Aud_AudListInit(): listStyle", AK_FALSE);

	if (AK_NULL == pAudLogic)
	{
		return AK_FALSE;
	}

	switch (listStyle)
	{
	case eAUD_MUSIC:
		Utl_UStrCpy(searchpath, g_diskPath);
		pAudLogic->bDividFolders = AK_TRUE;
		fileFormat = (T_U16*)strUFilter[0];
		skipFolder = (T_U16*)folderFilter[0];
		break;
		
	case eAUD_VOICE:	
		Utl_UStrCpy(searchpath, defrecord);
		Utl_UStrCat(searchpath, SOLIDUS);
		pAudLogic->bDividFolders = AK_FALSE;
		fileFormat = (T_U16*)strUFilter[1];
		skipFolder = (T_U16*)folderFilter[1];
		break;

	case eAUD_USBMUSIC:
		if (Fwl_MemDevIsMount(USB_HOST_DISK))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(USB_HOST_DISK, AK_NULL));
    	}
		pAudLogic->bDividFolders = AK_TRUE;
		fileFormat = (T_U16*)strUFilter[0];
		skipFolder = (T_U16*)folderFilter[0];
		break;

	case eAUD_SDMUSIC:
		if (Fwl_MemDevIsMount(MMC_SD_CARD))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL));
    	}
		pAudLogic->bDividFolders = AK_TRUE;
		fileFormat = (T_U16*)strUFilter[0];
		skipFolder = (T_U16*)folderFilter[0];
		break;

	case eAUD_SDVOICE:
		if (Fwl_MemDevIsMount(MMC_SD_CARD))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL));
			Utl_UStrCat(searchpath, REC_fold);
    	}
		pAudLogic->bDividFolders = AK_FALSE;
		fileFormat = (T_U16*)strUFilter[1];
		skipFolder = (T_U16*)folderFilter[1];
		break;

	default:
		break;
	}

	return MList_Init(mediaListName[listStyle], searchpath, pAudLogic->bDividFolders, fileFormat, skipFolder);
}

/**
* @brief delete media list file
*
* @author Songmengxing
* @date 2013-04-19
*
* @param in T_AUD_LISTSTLE listStyle : listStyle
*
* @return T_BOOL
* @retval
*/
T_BOOL Aud_AudDelList(T_AUD_LISTSTLE listStyle)
{
	T_USTR_FILE searchpath = {0};
	T_U16 SOLIDUS[2] = {'/',0};
	T_U16 REC_fold[11] = {'A', 'U', 'D', 'I', 'O', '_', 'R', 'E', 'C', '/',0};
	
	AK_ASSERT_VAL((listStyle < eAUD_NUM), "Aud_AudDelList(): listStyle", AK_FALSE);

	switch (listStyle)
	{
	case eAUD_MUSIC:
		Utl_UStrCpy(searchpath, g_diskPath);
		break;
		
	case eAUD_VOICE:	
		Utl_UStrCpy(searchpath, defrecord);
		Utl_UStrCat(searchpath, SOLIDUS);
		break;

	case eAUD_USBMUSIC:
		if (Fwl_MemDevIsMount(USB_HOST_DISK))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(USB_HOST_DISK, AK_NULL));
    	}
		break;

	case eAUD_SDMUSIC:
		if (Fwl_MemDevIsMount(MMC_SD_CARD))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL));
    	}
		break;

	case eAUD_SDVOICE:
		if (Fwl_MemDevIsMount(MMC_SD_CARD))
    	{
    		Utl_UStrCpy(searchpath, Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL));
			Utl_UStrCat(searchpath, REC_fold);
    	}
		break;

	default:
		break;
	}
	return MList_DelList(mediaListName[listStyle], searchpath);
}

#endif
