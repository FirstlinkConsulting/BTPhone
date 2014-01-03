/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_audio_record.c
 * @BRIEF audio record process 
 * @Author：Huang_ChuSheng
 * @Date：2008-04-15
 * @Version：
**************************************************************************/
#include "m_event.h"
#include "Ctrl_Public.h"
#include "log_file_com.h"
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
#include "Fwl_WaveOut.h"
#include "unicode.h"
#include "Fwl_Lcd.h"
#include "M_Event_Api.h"
#include "Apl_Public.h"
#include "Ctrl_MenuConfig.h"
#include "Fwl_Mount.h"
#include "Eng_LedHint.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"

#ifdef SUPPORT_AUDIO_RECORD
//暂时不支持ogg格式的编码，用宏暂时屏蔽菜单设置SPEEX编码.
#define SPEEX_SUPPORT           1

#define RECORD_SAVING_DELAYUS   400000
#define RECORD_LOWBAT_SHOWTIME  2

#define REC_MENU_KEY        kbMODE
#define REC_CONTROL_KEY     kbOK

#define REC_FILE_MAX        999
#define REC_FILENAME_LEN    80

#if SPEEX_SUPPORT   
#define REC_MODE_CNT        9
#else
#define REC_MODE_CNT        7
#endif

#define eREC_MODE_VOR_HI    0xff
#define eREC_MODE_VOR_LONG  0xfe

typedef enum{
    eRECMENU_MODE_RECMODE,
    eRECMENU_MODE_RECPATH,
    
    eRECMENU_MODE_MAX   
} T_eRECMENU_MODE;  

typedef enum{
    eRECMENU_PATH_NAND,
    eRECMENU_PATH_SDCARD,
    
    eRECMENU_PATH_MAX   
} T_eRECMENU_PATH;  


const T_U8 rec_mode[REC_MODE_CNT] = 
{
    eREC_MODE_ADPCM8K_2,
    eREC_MODE_WAV16K,
    eREC_MODE_WAV48K_2,
    eREC_MODE_MP3LOW,
    eREC_MODE_MP3HI,
    
#if SPEEX_SUPPORT       
    eREC_MODE_SPEEX_LOW,
    eREC_MODE_SPEEX_HI,
#endif

    eREC_MODE_VOR_HI,
    eREC_MODE_VOR_LONG,         
};

extern const T_U16 defrecord[];
extern const T_U16 defrecord_sd[];

const T_STR_SHOW m_RecMenuPrintf[eRECMENU_MODE_MAX] = {                                                 
                                                        "REC_MODE", 
                                                        "REC_PATH", 
                                                      };

const T_STR_SHOW m_RecPathPrintf[eRECMENU_PATH_MAX] = {                                                 
                                                        "REC_PATH_NAND",    
                                                        "REC_PATH_SDCARD",  
                                                      };

const T_STR_SHOW m_RecModePrintf[REC_MODE_CNT] = {                                                  
                                                    "eREC_MODE_ADPCM8K",    
                                                    "eREC_MODE_PCM16K", 
                                                    "eREC_MODE_PCM48K",                                                         
                                                    "eREC_MODE_MP3LOW", 
                                                    "eREC_MODE_MP3HI",  
                                                #if SPEEX_SUPPORT   
                                                    "eREC_SPEEX_LOW",
                                                    "eREC_SPEEX_HI",
                                                #endif  
                                                    "eREC_VOICE_HI",        
                                                    "eREC_VOICE_LONG",
                                                 };


static const T_pSTR m_recType[3]=
{
    ".mp3",
    ".ogg",
    ".wav",
};


#define FM_REC_PREFIX           "FM"


typedef struct _AUDIO_RECORD
{
    T_eREC_STAT  recStat;
    T_eREC_MODE  recMode;
    T_SYSTIME    date;
    T_RECORD_CFG *pRecCfg;    
    T_U32        totalRecTime;
    T_U32        recfilenum;    
    T_USTR_FILE  recRecPath;
    T_USTR_FILE  fullFileName;
    T_U16        recFileName[REC_FILENAME_LEN];//need full path 
    T_BOOL       bRecInRadio; //if true : in radio record state 
    T_BOOL       bRespondUpKey ;
    T_BOOL       bFilefull;//file is full or not
    T_BOOL       bNandSpacefull;//nandflash is full or not
    T_BOOL       bVorRec; //whether is voice ctrl record
    T_BOOL       bReturn;   
}T_AUDIO_RECORD;

static T_AUDIO_RECORD *pAudioRec = AK_NULL;

T_SYSTIME* Record_GetDate(T_VOID);
T_U16* Record_GetRecPath(T_VOID);
T_U32 Record_GetFileCnt(T_VOID);
T_BOOL Record_IsInRadio(T_VOID);
extern T_VOID Preproc_TriggerPoweroff(T_VOID);
extern T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt);


/************************************************************************
 * @BRIEF find the next recoding file
 * @AUTHOR xp
 * @DATE  2008-06-07
 * @PARAM T_U16 *pRecFileName
 * @PARAM T_U16 *path
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE: successfully to 
 * @RETVAL AK_FALSE: failed to create afile
 * @//note fileName format RECXXX.wav
 *modify by :hxq 
 *DATE : 2011-11-17
 *note: fileName format Rec_year-month-date_hour-minute-second_XXXXXXX.wav.
 **************************************************************************/
T_BOOL Record_GetFileName(T_pWSTR path, T_U16 *pFileName, T_pSTR suffix)
{
    T_STR_FILE  TmpName;    //not unicode string!
    T_USTR_FILE RecName;
    T_BOOL      bRet = AK_FALSE;
    T_CHR tmpStr[100];
    T_U32  FileCount = T_U32_MAX;

    if (!Record_IsInRadio())
    {
        sprintf(tmpStr,"%s%s%s", REC_FILE_PREFIX, "%03ld", suffix);
    }
    else
    {
        sprintf(tmpStr,"%s%s%s", FM_REC_PREFIX, "%03ld", suffix);
    }

    if (FileCom_CheckIndex(path, tmpStr, &FileCount, REC_FILE_MAX))
    {
        if (T_U32_MAX == FileCount  || FileCount < REC_FILE_MAX)
        {
            FileCount++;
        }
        else
        {
            AK_DEBUG_OUTPUT("False Get Fileindex:%d\n", FileCount);
            bRet = AK_FALSE;
            return bRet;
        }
        
        sprintf(TmpName, tmpStr, FileCount);
        
        Utl_StrMbcs2Ucs(TmpName, RecName);
        UStrCpyN(pFileName, RecName, REC_FILENAME_LEN);

        akerror("pFileName:", 0, 0);
        Printf_UC(pFileName);
        
        bRet = AK_TRUE;
    }
    else
    {
        bRet = AK_FALSE;
    }
    return bRet;
}
/************************************************************************
 * @BRIEF Set Frequence for Record when there are different Mode.
 * @AUTHOR hxq
 * @DATE  2012-1-6
 * @PARAM T_eREC_MODE 
 * @RETURN T_U32
 **************************************************************************/
static T_VOID Record_SetFreq(T_eREC_MODE recMode)
{
    switch(recMode)
    {
        case eREC_MODE_WAV8K :
            Fwl_FreqPush(FREQ_APP_AUDIO_REC); //优质录音(29M)
            break;
        case eREC_MODE_WAV16K:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //优质录音(58M)
            break;
        case eREC_MODE_WAV48K_2:
            Fwl_FreqPush(FREQ_APP_AUDIO_L4);    //高品质录音(58M)
            break;
        case eREC_MODE_ADPCM8K:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //长时声控
            break;
        case eREC_MODE_ADPCM8K_RADIO:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //32K比特率
            break;
        case eREC_MODE_ADPCM16K:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //长时录音
            break;
        case eREC_MODE_ADPCM8K_2:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //64K比特率
            break;
        case eREC_MODE_ADPCM48K:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //96k比特率
            break;
        case eREC_MODE_ADPCM48K_2:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //129k比特率
            break;
        case eREC_MODE_AMR:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //AMR录音
            break;
        case eREC_MODE_MP3LOW:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //8K 双声道
            break;
        case eREC_MODE_MP3HI:
            Fwl_FreqPush(FREQ_APP_AUDIO_L3);    //22K 单声道
            break;
        case eREC_MODE_SPEEX_LOW:
            Fwl_FreqPush(FREQ_APP_MAX);     //8K 
            break;
        case eREC_MODE_SPEEX_HI:
            Fwl_FreqPush(FREQ_APP_MAX); //8K
            break;
    }
}

/************************************************************************
 * @BRIEF Reset Frequence for Record when there are different Mode.
 * @AUTHOR hxq
 * @DATE  2012-1-6
 * @PARAM T_eREC_MODE 
 * @RETURN T_U32
 **************************************************************************/
static T_VOID Record_ResetFreq(T_eREC_MODE recMode)
{
    switch(recMode)
    {
        default:
            Fwl_FreqPop();
            break;
    }
}

T_U16* Record_GetRecPath(T_VOID)
{
    return pAudioRec->recRecPath;
}

#pragma arm section code = "_frequentcode_"
T_BOOL Record_IsNULL(T_VOID)
{
    if(pAudioRec==AK_NULL) return AK_TRUE;
    return AK_FALSE;
}
T_BOOL IsRecording(T_VOID)
{
	if((AK_NULL != pAudioRec) && (eSTAT_REC_PAUSE != pAudioRec->recStat)
        &&(eSTAT_REC_STOP != pAudioRec->recStat))
	{
    	return AK_TRUE;
	}
	else
	{
    	return AK_FALSE;
	}
}

#pragma arm section code


#pragma arm section code = "_recording_"
T_eREC_STAT Record_status(T_VOID)
{
    if (!pAudioRec)
        return eSTAT_MAX;
    else 
        return pAudioRec->recStat;
}

T_BOOL Record_IsInRadio(T_VOID)
{
    return pAudioRec->bRecInRadio;
}

#pragma arm section code 


#pragma arm section code = "_recording_"

/**************************************************************************
* @brief Save config file
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
static T_VOID Record_SaveProfile(T_VOID)
{
    if(AK_TRUE == pAudioRec->bRecInRadio)
    {
        Utl_UStrCpy(pAudioRec->pRecCfg->radioDefaultPath, pAudioRec->recRecPath) ;
        pAudioRec->pRecCfg->radioRecMode = AudioRecord_GetMode();
    }
    else
    {
        Utl_UStrCpy(pAudioRec->pRecCfg->defaultPath, pAudioRec->recRecPath);
        pAudioRec->pRecCfg->recMode = pAudioRec->recMode;
    }
    
    pAudioRec->pRecCfg->isVorRec = AudioRecord_IsVorCtrlRec();
    Profile_WriteData(eCFG_REC, pAudioRec->pRecCfg);
}


T_MEM_DEV_ID Rec_GetCurDriver(T_VOID)
{
#ifdef SUPPORT_SDCARD
    return Fwl_MemDevGetDriver(pAudioRec->recRecPath[0]);
#else
    return SYSTEM_STORAGE;
#endif
}

T_VOID Record_DealSaving(T_VOID)
{
    AK_DEBUG_OUTPUT("Record_ExitRecording\n");
    Fwl_FreqPush(FREQ_APP_MAX);
    AudioRecord_Stop();

#ifdef SUPPORT_SDCARD
    #ifdef OS_ANYKA
    if(Rec_GetCurDriver() == MMC_SD_CARD)
    {

        if(!Fwl_DetectorGetStatus(DEVICE_SD))
        {
            Fwl_LCD_lock(AK_TRUE);
        }
    }
    #endif
#endif

    //Fwl_DelayUs(RECORD_SAVING_DELAYUS);
    //pAudioRec->totalRecTime = AudioRecord_GetTotalTime(pAudioRec->recRecPath[0], AudioRecord_GetMode());
    pAudioRec->recStat = eSTAT_REC_STOP;
    pAudioRec->bRespondUpKey = AK_FALSE;

    Fwl_FreqPop();
    AutoPowerOffEnable();    
    Record_ResetFreq(AudioRecord_GetMode());

}

T_VOID Record_ExitRecording(T_VOID)
{
    Record_DealSaving();            
   
    if(pAudioRec->recfilenum >= REC_FILE_MAX)
    {
        pAudioRec->bFilefull = AK_TRUE;
        pAudioRec->bReturn = AK_TRUE;
    }            
    VME_EvtQueueClearTimerEvent();
}

/**************************************************************************
* @brief deal post message
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
static T_U8 Record_DealExit(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U32 param1;

    if(eSTAT_REC_STOP != pAudioRec->recStat)
    {
        Fwl_LCD_lock(AK_FALSE);
        Record_ExitRecording();
    }
    if(M_EVT_RETURN == event)
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        return 0;
    }
    else if(event >= M_EVT_Z00_POWEROFF)
    {
        Fwl_LCD_lock(AK_FALSE);
        if(event == M_EVT_Z01_MUSIC_PLAY)
        {
            param1= EVENT_TYPE_SWITCHOFF_MANUAL;
        }
        else
        {
            param1= pEventParm->w.Param1;
        }

        if (pAudioRec->bRecInRadio)
        {
            AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            if(param1 != EVENT_TYPE_SWITCHOFF_AUTO)
            {
                 VME_EvtQueuePut(event, pEventParm);
            }
            return 0;
        }
        else
        {
            Record_SaveProfile();
            return 1;
        }
    }
    return 1;
}

#pragma arm section code


/**************************************************************************
* @brief make default folder
* 
* @author lx
* @date 2013-6-17
* @param 
* @return 
***************************************************************************/
static T_VOID Record_MakeDefPath(T_VOID)
{
    Fwl_FsMkDir(defrecord);
#if(STORAGE_USED == NAND_FLASH)
#ifdef SUPPORT_SDCARD
    Fwl_FsMkDir(defrecord_sd);
#endif
#endif
}

/**************************************************************************
* @brief make default folder
* 
* @author lx
* @date 2013-6-17
* @param 
* @return 
***************************************************************************/
static T_BOOL Record_PathConfig(T_VOID)
{
    Record_MakeDefPath();
#ifdef SUPPORT_SDCARD
    {
        T_BOOL flag = AK_FALSE;
        
        if (Fwl_MemDevIsMount(MMC_SD_CARD)
            && 0 == Utl_UStrCmpN(pAudioRec->recRecPath,
                        Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL),
                        Utl_UStrLen(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL))))
        {
            flag = AK_TRUE;
        }
        if((AK_FALSE == flag))
        {
            AK_DEBUG_OUTPUT("Sd not exist\n");
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
        }
    }
#endif
    pAudioRec->totalRecTime = AudioRecord_GetTotalTime(pAudioRec->recRecPath[0], pAudioRec->recMode);

#if(STORAGE_USED == NAND_FLASH)
    #ifdef SUPPORT_SDCARD
    if(pAudioRec->totalRecTime < 3)
    {
        T_BOOL flag = AK_FALSE;

        if(Fwl_MemDevIsMount(MMC_SD_CARD))  
        {
            if(pAudioRec->recRecPath[0] == *(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL)))
            {
                Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
                flag = AK_TRUE;
            }
        } 
        if(AK_FALSE == flag)
        {
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord_sd);
        }
        pAudioRec->totalRecTime = AudioRecord_GetTotalTime(pAudioRec->recRecPath[0], pAudioRec->recMode);
    }
    #endif
#endif
    
    if (pAudioRec->totalRecTime < 3)
    {   
        //display hint for memory full,
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_NOFREE, AK_NULL);
        Voice_WaitTip();
#endif
        pAudioRec->bNandSpacefull = AK_TRUE;
        pAudioRec->bReturn = AK_TRUE;
        return AK_FALSE;
    }

    if(AK_FALSE == Fwl_FsIsDir(pAudioRec->recRecPath))
    {
        #if(STORAGE_USED == NAND_FLASH)
            #ifdef SUPPORT_SDCARD
            if (Rec_GetCurDriver() == SYSTEM_STORAGE)
            {
                Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
            }
            else
            {
                Utl_UStrCpy(pAudioRec->recRecPath, defrecord_sd);
            }
            #else
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
            #endif
        #else
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
        #endif
    }
    //Record_GetFileName(pAudioRec->recRecPath,pAudioRec->recFileName);
    if(pAudioRec->recfilenum >= REC_FILE_MAX)
    {
        pAudioRec->bFilefull = AK_TRUE;
        pAudioRec->bReturn = AK_TRUE;
        VME_EvtQueueClearTimerEvent();
        return AK_FALSE;
    }
    
    Fwl_GetRTCtime(&pAudioRec->date);
    return AK_TRUE;
}

static T_VOID Record_ChangeStat(T_PRESS_KEY key, T_EVT_PARAM *pEventParm)
{
    if((PRESS_LONG == key.pressType) && ((kbRIGHT == key.id) || (kbLEFT == key.id )))
    {
        if (!Record_IsInRadio())
        {
            GAIN_LEVEL gain = AudioRecord_GetGain();
            if(kbRIGHT == key.id)
            {
                if(gain < (GAIN_MAX - 1))
                {                   
                    AudioRecord_SetGain(++gain);
                }
            }
            else
            {
                if(AudioRecord_GetGain() > GAIN_LEVEL0)
                {                   
                    AudioRecord_SetGain(--gain);
                }
            }
            
            AK_DEBUG_OUTPUT("MIC_GAIN = %d\n",AudioRecord_GetGain());
        }
        else //FM RECORD
        {
            if(kbRIGHT== key.id)
            {
                Radio_AddVolume();
            }
            else
            {
                Radio_SubVolume();
            }
        }        
    }
    else if(REC_MENU_KEY == key.id && !Record_IsInRadio())
    {
        if (PRESS_SHORT == key.pressType)
        {   
            T_U8 i = 0;
            T_U8 focusMode = 0;
            T_U8 focusPath = 0;

            for (i=0; i<REC_MODE_CNT; i++)
            {
                if (AK_FALSE == pAudioRec->bVorRec)
                {
                    if (rec_mode[i] == pAudioRec->recMode)
                    {
                        focusMode = i;
                        break;
                    }
                }
                else if((eREC_MODE_VOR_HI == rec_mode[i]
                        && eREC_MODE_WAV16K == pAudioRec->recMode)
                        || (eREC_MODE_VOR_LONG == rec_mode[i]
                            && eREC_MODE_ADPCM8K_2 == pAudioRec->recMode))
                { 
                    focusMode = i;
                    break;
                }
            }

            if (SYSTEM_STORAGE == Rec_GetCurDriver())
            {
                focusPath = eRECMENU_PATH_NAND;
            }
            else
            {
                focusPath = eRECMENU_PATH_SDCARD;
            }

            MenuCfg_Init(eRECMENU_MODE_MAX);
            MenuCfg_AddMenu(eRECMENU_MODE_RECMODE, REC_MODE_CNT, focusMode, (T_STR_SHOW*)(&m_RecModePrintf));
            MenuCfg_AddMenu(eRECMENU_MODE_RECPATH, eRECMENU_PATH_MAX, focusPath, (T_STR_SHOW*)(&m_RecPathPrintf));

            if(eSTAT_REC_STOP != pAudioRec->recStat)
            {
                Record_ExitRecording();
            }

            m_triggerEvent(M_EVT_MENU, pEventParm);
        }
    }

    switch(pAudioRec->recStat)
    {
    case eSTAT_REC_STOP:
        if(kbRECORD==key.id&&PRESS_UP==key.pressType&&AK_FALSE==pAudioRec->bRespondUpKey)
        {//filter key:REC keytype:press_up after saving 
            pAudioRec->bRespondUpKey = AK_TRUE;
            break;
        }
        
        if ((REC_CONTROL_KEY== key.id&&PRESS_SHORT == key.pressType)
            ||(kbRECORD==key.id&&(PRESS_SHORT==key.pressType||PRESS_UP==key.pressType)))
        {  
            T_USTR_FILE filename;
            T_U16       uSlide[] = {'/','\0'};
            T_S32       volLarger= 1024;
            T_U8        type;

            if(pAudioRec->recMode == eREC_MODE_MP3LOW 
                || pAudioRec->recMode == eREC_MODE_MP3HI)
            {
                type = 0;
            }
            else if(pAudioRec->recMode == eREC_MODE_SPEEX_HI
                    || pAudioRec->recMode == eREC_MODE_SPEEX_LOW)
            {
                type = 1;
            }
            else
            {
                type = 2;
            }

            Record_SetFreq(pAudioRec->recMode);

            if (!Record_GetFileName(pAudioRec->recRecPath, pAudioRec->recFileName, m_recType[type])
                || ((pAudioRec->recfilenum++) >= REC_FILE_MAX))
            {
				//录音超过1000个文件
                #ifdef SUPPORT_VOICE_TIP
				Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
				Voice_WaitTip();
				#endif
                pAudioRec->recStat = eSTAT_REC_STOP;
            	m_triggerEvent(M_EVT_EXIT, pEventParm);
				return;
			}

            Utl_UStrCpy(filename,pAudioRec->recRecPath);
            if(*(filename+Utl_UStrLen(filename)-1)!=UNICODE_SOLIDUS)
            {
                Utl_UStrCat(filename,uSlide);
            }
            Utl_UStrCat(filename, pAudioRec->recFileName);
            Utl_UStrCpy(pAudioRec->fullFileName, filename);

            AutoPowerOffDisable();
            switch(pAudioRec->recMode)
            {
            case eREC_MODE_MP3HI:
                volLarger = 1024;
                break;

            case eREC_MODE_MP3LOW:
                volLarger = 1024;
                break;

            case eREC_MODE_WAV8K:  //优质录音          
            case eREC_MODE_WAV16K: //优质录音          
                volLarger =1024* 1;
                break;

            case eREC_MODE_WAV48K_2:
                volLarger =1024* 1;
                break;

            case eREC_MODE_AMR:
                volLarger = (T_S32)(1024* 2.5);
                break;

            case eREC_MODE_ADPCM8K: //长时声控
            case eREC_MODE_ADPCM16K:  //长时录音
                volLarger = 1024* 1;
                break;

            case eREC_MODE_ADPCM8K_RADIO: //32K比特率
            case eREC_MODE_ADPCM8K_2:  //64K比特率
            case eREC_MODE_ADPCM48K:   //96k比特率
            case eREC_MODE_ADPCM48K_2:  //129k比特率
                if (DEF_RADIO == RADIO_RDA5807)
                {
                    T_U8 vol = Radio_GetVolume();

                    if(vol <= 20)
                    {
                        volLarger = (T_S32)(1024 * 3.2);//4;
                    }
                    else 
                    {
                        volLarger = (T_S32)(1024 * 2.3);//3;
                    }
                }
                else
                {
                    volLarger =(T_S32) (1024* 1);
                }
                break;

            case eREC_MODE_SPEEX_LOW:  //优质录音          
            case eREC_MODE_SPEEX_HI: //优质录音          
                volLarger =1024* 1;          
                break;                      
            }
            AK_DEBUG_OUTPUT("\nvolLarger:%d\n", volLarger* 10/1024);
            if (!AudioRecord_Start(filename, pAudioRec->recMode, volLarger, pAudioRec->bVorRec))
            {
                pAudioRec->recStat = eSTAT_REC_STOP;
                AutoPowerOffEnable();
                
                #ifdef SUPPORT_VOICE_TIP
				    Voice_PlayTip(eBTPLY_SOUND_TYPE_NOFREE, AK_NULL);
				    Voice_WaitTip();
                #endif
                
            	m_triggerEvent(M_EVT_EXIT, pEventParm);
                Fwl_FreqPop();
				return;
            }
            else
            {
                pAudioRec->recStat = eSTAT_RECORDING;
            }
             /*
              if it uses medialib encoding , starts and stops recording
              frequently, the pages of savings rec data's page count is lower
              then other code pages , and it will be swapped out from memory and
              swapped into memory frequntly, which will cause saving rec data
              expecially encoding cost lots of time , and the system will lost 
              recording data

            #ifdef OS_ANYKA
            for (i=0; i<40; i++)
            {
                //page_counter(i) = 1;
                if (Fwl_L2_Get_PageCounter(i) == 1)
                {
                    volLarger= (T_S32)(1024* 3.2);//4;
                }
                else 
                {
                    volLarger= (T_S32)(1024* 2.3);//3;
                }
            }
            #endif
            */                
        }
        else
        {
            if(kbPOWER == key.id && PRESS_LONG == key.pressType)
            {
                #ifdef OS_ANYKA
                AK_PRINTK("***key poweroff3:",key.id, 1);
                #endif
                Preproc_TriggerPoweroff();
            }
        }
        break;

    case eSTAT_RECORDING:
        if (REC_CONTROL_KEY == key.id || kbPOWER == key.id)
        {
            if (PRESS_SHORT == key.pressType)
            {
                AK_DEBUG_OUTPUT("record pause\n");
                AudioRecord_Pause();
                pAudioRec->recStat = eSTAT_REC_PAUSE;
                AutoPowerOffEnable();
            }
            else if((PRESS_LONG == key.pressType) && (kbPOWER == key.id))
            {
                Record_ExitRecording();
                if(kbPOWER == key.id)
                {
                    Preproc_TriggerPoweroff();
                }
            }
        }
        if (kbRECORD == key.id && PRESS_SHORT == key.pressType)
        {
             Record_ExitRecording();             
        }
        break;
    case eSTAT_REC_PAUSE:
        if (REC_CONTROL_KEY== key.id||kbRECORD==key.id || kbPOWER == key.id)
        {
            if (PRESS_SHORT == key.pressType)
            {
                AK_DEBUG_OUTPUT("record resume\n");
                AudioRecord_Resume();
                pAudioRec->recStat = eSTAT_RECORDING;
                AutoPowerOffDisable();
            }
        }
        //if pause ,press kbMODE ang presss long ,then save         change by lsk 2011 - 5 -30
        if (((kbPOWER == key.id) || (kbRECORD==key.id) || (REC_CONTROL_KEY == key.id))&&PRESS_LONG==key.pressType)
        {
            Record_ExitRecording();
            if(kbPOWER == key.id)
            {
                Preproc_TriggerPoweroff();
            }                
        }
        else
        {
#ifdef DEEP_STANDBY
            if(kbPOWER == key.id && PRESS_LONG == key.pressType)
//      change by lsk 2011 - 5 -30
//#else
//                if(REC_CONTROL_KEY == key.id && PRESS_LONG == key.pressType)
//#endif
            {
                #ifdef OS_ANYKA
                AK_PRINTK("***key poweroff4:", key.id, 1);
                #endif
                Preproc_TriggerPoweroff();
            }
#endif
        }
        break;
    default:
        AK_DEBUG_OUTPUT("unkonw rec mode!!!!\n");
        break;
    }
}

void initaudio_record(void)
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

#ifdef SUPPORT_VOICE_TIP
            Voice_PlayTip(eBTPLY_SOUND_TYPE_MICREC, AK_NULL);
            Voice_WaitTip();
#endif

    pAudioRec = (T_AUDIO_RECORD*)Fwl_Malloc(sizeof(T_AUDIO_RECORD));
    if (AK_NULL == pAudioRec)
    {
        AK_DEBUG_OUTPUT("No more memory to malloc for pAudioRec\n");
        return;
    }
    Fwl_FreqPush(FREQ_APP_MAX);

    pAudioRec->recStat = eSTAT_REC_STOP;   
    pAudioRec->bRecInRadio = AK_FALSE;
    pAudioRec->recfilenum = 0;
    pAudioRec->bReturn = AK_FALSE;  
    pAudioRec->bRespondUpKey = AK_TRUE;
    pAudioRec->bFilefull = AK_FALSE;
    pAudioRec->bNandSpacefull = AK_FALSE;
    pAudioRec->recMode = eREC_MODE_ADPCM8K_2;
    pAudioRec->bVorRec = AK_FALSE;
    
    memset(pAudioRec->recFileName, 0, (REC_FILENAME_LEN<<1));
    AudioRecord_Init(); 
    pAudioRec->recMode = eREC_MODE_ADPCM8K_2;
    Fwl_FreqPop();
}

#pragma arm section code = "_recording_"    

void paintaudio_record(void)
{    
}

#pragma arm section code

void exitaudio_record(void)
{  
    AK_DEBUG_OUTPUT("exit record");
    if (eSTAT_REC_STOP != pAudioRec->recStat)
    {
        //for usbdisk input
        Record_DealSaving();        
    }

    Fwl_FreqPush(FREQ_APP_MAX);
	Record_SaveProfile();
    pAudioRec->pRecCfg = (T_RECORD_CFG *)Fwl_Free(pAudioRec->pRecCfg);
    pAudioRec = (T_AUDIO_RECORD *)Fwl_Free(pAudioRec);
    AudioRecord_Destroy();
    Fwl_FreqPop();
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


#pragma arm section code = "_recording_"    
unsigned char handleaudio_record(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{ 
    T_PRESS_KEY keyPad;    
    T_EVT_PARAM evtParam;

    switch (event)
    {
#ifdef SUPPORT_RADIO
    case M_EVT_RADIO_REC:
        Fwl_FreqPush(FREQ_APP_MAX);
        AudioRecord_SetDataSource(eSTAT_RCV_LINEIN);
        pAudioRec->bRecInRadio = AK_TRUE;
        pAudioRec->pRecCfg = (T_RECORD_CFG*)Fwl_Malloc(sizeof(T_RECORD_CFG));
        if(AK_NULL== pAudioRec->pRecCfg)
        {
            m_triggerEvent(M_EVT_EXIT,AK_NULL);
            AK_DEBUG_OUTPUT("malloc RecCfg failed\n");
            Fwl_FreqPop();
            return 0;
        }
        Profile_ReadData(eCFG_REC, pAudioRec->pRecCfg);
        pAudioRec->recMode = pAudioRec->pRecCfg->radioRecMode;
        Utl_UStrCpy(pAudioRec->recRecPath, pAudioRec->pRecCfg->radioDefaultPath);
        if(!Record_PathConfig())
        {
            Fwl_FreqPop();
            return 0;
        }
        AutoPowerOffEnable();
        Fwl_FreqPop();
        break;
#endif
    case M_EVT_Z00_POWEROFF:
        return Record_DealExit(event, pEventParm);
        break;

    case M_EVT_Z02_STANDBY:
        if (eSTAT_REC_PAUSE == AudioRecord_GetRecState() || eSTAT_REC_STOP == AudioRecord_GetRecState())//if( eSTAT_VORREC_PAUSE == AudioRecord_GetVorCtrlState())
        {
            return 1;
        }
        break;

    case VME_EVT_TIMER:
        VME_EvtQueueClearTimerEventEx(VME_EVT_TIMER);
        break;
		
	case M_EVT_PUB_TIMER:
		VME_EvtQueueClearTimerEventEx(M_EVT_PUB_TIMER);
		break;

    case VME_EVT_RECORDCTRL:
        if(CTRL_EVENT_MEMORY_FULL == pEventParm->c.Param1)
        {
            Record_DealSaving();
            VME_EvtQueueClearTimerEvent();
            pAudioRec->bReturn = AK_TRUE;
            pAudioRec->bNandSpacefull = AK_TRUE; 
            #ifdef SUPPORT_VOICE_TIP
                Voice_PlayTip(eBTPLY_SOUND_TYPE_NOFREE, AK_NULL);
                Voice_WaitTip();
            #endif
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            return 0;
        }
        if(CTRL_EVENT_SINGLE_FILE_LEN_LIMIT == pEventParm->c.Param1)
        {
            Record_DealSaving();
            pEventParm->c.Param1 = REC_CONTROL_KEY;
            pEventParm->c.Param2 = PRESS_SHORT;
            VME_EvtQueuePut(M_EVT_USER_KEY, pEventParm);//M_EVT_RECORD
            AK_DEBUG_OUTPUT("Restart to Record!!\n");
            return 0;           
        }
        break;
    //////////////hxq 处理拔出SD卡时.退出录音模块.
#ifdef  SUPPORT_SDCARD
    case M_EVT_SDCARD:
        if(SD_DET_PULL_OUT == (T_U32)(pEventParm->c.Param2))
        {
            //判断当前录音路径是否在SD卡上,如果是,则退出状态机
            if(UStrCmpN(Fwl_MemDevGetPath (MMC_SD_CARD, AK_NULL), Record_GetRecPath(),1) == 0)
            {
                AK_DEBUG_OUTPUT("rec SD1 OUT!\n");
                m_triggerEvent(M_EVT_EXIT,AK_NULL);//退出状态机.
            }
            else
            {
                //若不是SD路径,则不动作,因为中断处理过程已经卸载SD卡.
                ;
            }
        }
        else
            AK_DEBUG_OUTPUT("SD_IN\n");
        break;
#endif
    case M_EVT_RECORD:
    case M_EVT_SD_RECORD:
        Fwl_FreqPush(FREQ_APP_MAX);
        AudioRecord_SetDataSource(eSTAT_RCV_MIC);
        pAudioRec->bRecInRadio = AK_FALSE;
        pAudioRec->pRecCfg = (T_RECORD_CFG*)Fwl_Malloc(sizeof(T_RECORD_CFG));
        if(AK_NULL== pAudioRec->pRecCfg)
        {
            m_triggerEvent(M_EVT_EXIT,AK_NULL);
            AK_DEBUG_OUTPUT("malloc RecCfg failed\n");
            Fwl_FreqPop();
            return 0;
        }
        Profile_ReadData(eCFG_REC, pAudioRec->pRecCfg);
        pAudioRec->recMode = pAudioRec->pRecCfg->recMode;
        pAudioRec->bVorRec = pAudioRec->pRecCfg->isVorRec;
            
        if (M_EVT_SD_RECORD == event)
        {
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord_sd);//若支持usbhost，此处需要修改盘符管理内容
        }
        else
        {
            Utl_UStrCpy(pAudioRec->recRecPath, defrecord);
        }
        Utl_UStrCpy(pAudioRec->recRecPath, pAudioRec->pRecCfg->defaultPath);
        if(! Record_PathConfig())
        {
            if( pAudioRec->bNandSpacefull == AK_TRUE)
            {
                AK_DEBUG_OUTPUT("nand full\n");
            }
            else if(pAudioRec->bFilefull == AK_TRUE)
            {
                AK_DEBUG_OUTPUT("file full\n");
            }
            else
            {
                AK_DEBUG_OUTPUT("Record_PathConfig fail\n");
            }
            m_triggerEvent(M_EVT_EXIT,AK_NULL);
            Fwl_FreqPop();
            return 0;
        }

        //Start Record
        evtParam.c.Param1 = kbRECORD;
        evtParam.c.Param2 = PRESS_SHORT;
        VME_EvtQueuePut(M_EVT_USER_KEY, &evtParam); 
        
        Fwl_FreqPop();
        break;
        
    case M_EVT_USER_KEY:     
        keyPad.id = (T_eKEY_ID)pEventParm->c.Param1;
        keyPad.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;
        Record_ChangeStat(keyPad, pEventParm);
        if(pAudioRec->bReturn == AK_TRUE)
        {
            AK_DEBUG_OUTPUT("Record_ChangeStat fail\n");
             m_triggerEvent(M_EVT_EXIT,AK_NULL);
             return 0;
        }
        break;
        
#ifdef SUPPORT_VOICE_TIP
    case VME_EVT_POWER_CHANGE:
        AudioRecord_Pause(); 
        AudioRecord_StopWaveIn();
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
        
        AudioRecord_Resume();
        AudioRecord_StartWaveIn();
        break;
#endif


    case M_EVT_RETURN:
         m_triggerEvent(M_EVT_EXIT,AK_NULL);
         return 0;

    case M_EVT_EXIT:
        if (RETURN_FROM_SET_MENU == pEventParm->s.Param1)
        {
        	if (pEventParm->s.Param2)
            {
	            T_U8 submode = MENU_ERROR_ID;
	            T_U8 item = MENU_ERROR_ID;

	            MenuCfg_GetFocus(&submode, &item);

	            if (eRECMENU_MODE_RECMODE == submode)
	            {
	                //声控录音
	                if (eREC_MODE_VOR_LONG == rec_mode[item])
	                {
	                    pAudioRec->bVorRec = AK_TRUE;
	                    pAudioRec->recMode = eREC_MODE_ADPCM8K_2;
	                }
	                else if(eREC_MODE_VOR_HI == rec_mode[item])
	                {
	                    pAudioRec->bVorRec = AK_TRUE;
	                    pAudioRec->recMode = eREC_MODE_WAV16K;
	                }
	                else
	                {
	                    pAudioRec->bVorRec = AK_FALSE;
	                    pAudioRec->recMode = rec_mode[item];
	                }
	            }
	            else if(eRECMENU_MODE_RECPATH == submode)
	            {
	                if (eRECMENU_PATH_NAND == item)
	                {
	                    Utl_UStrCpy(pAudioRec->recRecPath, defrecord);//若支持usbhost，此处需要修改盘符管理内容
	                }
	                else
	                {
	                    Utl_UStrCpy(pAudioRec->recRecPath, defrecord_sd);
	                }

	                if(!Record_PathConfig())
	                {
	                    if( pAudioRec->bNandSpacefull == AK_TRUE)
	                    {
	                        AK_DEBUG_OUTPUT("space full\n");
	                    }
	                    else if(pAudioRec->bFilefull == AK_TRUE)
	                    {
	                        AK_DEBUG_OUTPUT("file full\n");
	                    }
	                    else
	                    {
	                        AK_DEBUG_OUTPUT("Record_PathConfig fail\n");
	                    }

	                    m_triggerEvent(M_EVT_EXIT,AK_NULL);
	                    MenuCfg_Free();
	                    return 0;
	                }
	            }
        	}
            //Start Record
            evtParam.c.Param1= kbRECORD;
            evtParam.c.Param2= PRESS_SHORT;
            VME_EvtQueuePut(M_EVT_USER_KEY, &evtParam);
            MenuCfg_Free();
        }
        if (M_EVT_RESTART == pEventParm->w.Param1)
        {
            akerror("exit from deepstandby!", 0, 1);
            if (eSTAT_REC_PAUSE == AudioRecord_GetRecState())
            {
                pAudioRec->recStat = eSTAT_RECORDING;
                AudioRecord_Resume();
                AudioRecord_StartWaveIn();
            }
        }
        break;
        
    default:
        break;
    }
    
    return 0;
}

#pragma arm section code 
#else

void initaudio_record(void)
{
    
}
void paintaudio_record(void)
{   

}
void exitaudio_record(void)
{
    
}
unsigned char handleaudio_record(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleaudio_record\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;    
}
#endif
