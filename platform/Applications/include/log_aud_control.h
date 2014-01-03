/**
 * @file    log_aud_control.h
 * @brief   manage audio logic layer
 * @author  WuShanwei
 * @date    2008-04-03
 * @version 1.0
 */

#ifndef _LOG_AUD_CONTROL_H_
#define _LOG_AUD_CONTROL_H_

#include "gbl_global.h"
#include "log_aud_play.h"
#include "log_aud_ab.h"
#include "svc_medialist.h"

#ifdef SUPPORT_AMR_ENCODE
#define MUSIC_SUPPORT_TYPE          {'*','.','w','a','v','/','*','.','m','p','1','/','*','.','m','p','2','/','*','.','m','p','3','/','*','.','a','p','e','/','*','.','w','m','a','/','*','.','m','p','g','/','*','.','a','m','r','/','*','.','m','p','e','g','/','*','.','o','g','g','/','*','.','a','s','f','/','*','.','f','l','a','c',0,}
#else
#define MUSIC_SUPPORT_TYPE          {'*','.','w','a','v','/','*','.','m','p','1','/','*','.','m','p','2','/','*','.','m','p','3','/','*','.','a','p','e','/','*','.','w','m','a','/','*','.','m','p','g','/','*','.','m','p','e','g','/','*','.','o','g','g','/','*','.','a','s','f','/','*','.','f','l','a','c',0,}
#endif
#define RECORD_SUPPORT_TYPE         {'*','.','w','a','v',0,}
#define VIDIO_SUPPORT_TYPE          {'*','.','a','v','i',0,}


/** audio event type */
#define AUD_CTRL_EVENT_AUDSTOP      1
#define AUD_CTRL_EVENT_AUDPAUSE     2


/** param in    AUD_CTRL_EVENT_AUDSTOP event */
#define AUD_CTRL_STOP_EVT_NULL      0
#define AUD_CTRL_STOP_EVT_DECODE    1
#define AUD_CTRL_STOP_EVT_KEYPAD    2
#define AUD_CTRL_STOP_EVT_PREVIEW   4
#define AUD_CTRL_STOP_EVT_A_END     8
#define AUD_CTRL_STOP_EVT_DECODE_REC 16

/* param */
#ifdef OS_ANYKA
#define AUDIO_PLAYER_TIMER_TIME     200
#define AUDIO_CONTROL_TIMER_DIV     2
#else
#define AUDIO_PLAYER_TIMER_TIME     80
#define AUDIO_CONTROL_TIMER_DIV     4
#endif

#define DEFAULT_FADEIN_TIME         80
#define DEFAULT_FADEOUT_TIME        80

#define AUDIO_CONTROL_PREVIEW_PLAY_TIME     11000
#define AUD_CTRL_SETTING_AUTOEXIT_TIME      4
#define AUD_CTRL_PREPLAY_MAX_ERR    5
#define AUD_PLAY_SEEK_ADD_SPAC      400
#define AUD_PLAY_SEEK_ADD_TIME      2000

/** Audio control action */  
#define AUD_ACT_NULL                0x00000000
#define AUD_ACT_PLAY                0x00000001
#define AUD_ACT_STOP                0x00000002
#define AUD_ACT_PAUSE               0x00000004
#define AUD_ACT_RESUME              0x00000008
#define AUD_ACT_VOL_INC             0x00000010
#define AUD_ACT_VOL_DEC             0x00000020
#define AUD_ACT_PREVIEW             0x00000040
#define AUD_ACT_PRI_1               0x000000FF
#define AUD_ACT_SPEEDMODECHG        0x00000100
#define AUD_ACT_TONEMODECHG         0x00000200
#define AUD_ACT_REPMODECHG          0x00000400
#define AUD_ACT_ABMODECHG           0x00000800
#define AUD_ACT_PREVPLAY            0x00001000
#define AUD_ACT_NEXTPLAY            0x00002000
#define AUD_ACT_SEEKBACKWARD        0x00004000
#define AUD_ACT_SEEKFORWARD         0x00008000
#define AUD_ACT_PREVDIS             0x00010000
#define AUD_ACT_NEXTDIS             0x00020000
#define AUD_ACT_SEEKENDB            0x00040000
#define AUD_ACT_SEEKENDF            0x00080000
#define AUD_ACT_PRI_2               0x000FFF00
#define AUD_ACT_CYC_GETLYR          0x00100000
#define AUD_ACT_CYC_DISP            0x00200000
#define AUD_ACT_CYC_DISP_INFO       0x00400000
#define AUD_ACT_CYC_GET_TIME        0x00800000
#define AUD_ACT_PRI_3               0x00F00000
#define AUD_ACT_PLAY_END            0x01000000
#define AUD_ACT_AB_PAUSE            0x02000000
#define AUD_ACT_EXIT                0x04000000
#define AUD_ACT_POWEROFF            0x08000000
#define AUD_ACT_MUSICPLAY           0x10000000
#define AUD_ACT_USER_DEFINE         0x20000000
#define AUD_ACT_PRI_4               0xFF000000
#define AUD_ACT_ALL                 0XFFFFFFFF


/** AB mode parameters */
#define AUD_AB_SPACE_TIME_MAX       5000
#define AUD_AB_REPEAT_TIME_MAX      5

/* preprocess status */
#define AUD_PRO_HANDLE_NULL         0
#define AUD_PRO_HANDLE_OK           1
#define AUD_PRO_HANDLE_PLAYERR      2
#define Aud_PlayerOpenEx(player, file, medType, bFade) APlayer_Open(player, file, medType, bFade, DEFAULT_FADEIN_TIME, DEFAULT_FADEOUT_TIME, AUD_SPEED_ARITHMETIC)


typedef enum{
    eAUDMENU_MODE_CYC,
    eAUDMENU_MODE_EQ,
    eAUDMENU_MODE_SPEED,
    eAUDMENU_MODE_SEEKLEN,
	eAUDMENU_MODE_UPDATELIST,

    eAUDMENU_MODE_MAX   
} T_eAUDMENU_MODE;  


/** Audio control pre handle */  
typedef enum{
    AUD_LOG_PRE_NULL,
    AUD_LOG_PRE_CURR,
    AUD_LOG_PRE_NEXT,
    AUD_LOG_PRE_PREV,
    AUD_LOG_PRE_AUTOCON
} T_AUD_LOG_PRE;    


/** Audio control status */  
typedef enum{
    AUD_STAT_LOG_NULL,
    AUD_STAT_LOG_PLAY,
    AUD_STAT_LOG_STOP,
    AUD_STAT_LOG_PAUSE,
    AUD_STAT_LOG_SEEKB,
    AUD_STAT_LOG_SEEKF
} T_AUD_CONTROL_STATE;

/** Audio control file handle */  
typedef enum{
    AUD_FILE_CTRL_NULL,
    AUD_FILE_CTRL_SEL_FILE,
    AUD_FILE_CTRL_SEL_PATH,
    AUD_FILE_CTRL_DEL_FILE,
    AUD_FILE_CTRL_DEL_ALL,
    AUD_FILE_CTRL_DEL_FAV,
    AUD_FILE_CTRL_ACT_FAILED
} T_eAUD_FILE_CTRL;

typedef enum
{
    JUMP_NORMAL = 0,
    JUMP_SWITCH_OFF,
    JUMP_ALARM_CLK
}T_eAUD_JUMP_TYPE;

typedef enum 
{
    OPREATE_NONE,
    CUSTOM_ALL_MUSICS,
    CUSTOM_MUSIC_DELETE,
    CUSTOM_CLEAN_LIST,
    CUSTOM_DELETE_LIST,
}T_eListOperate;

 /* error type */
typedef enum{
    AUD_ERR_TYPE_NULL           =   0,
    AUD_ERR_TYPE_FORMATERR,
    AUD_ERR_TYPE_NOFILE ,
    AUD_ERR_TYPE_MEMORYFULL
}T_AUD_ERR_TYPE;


 /* open flag */
typedef enum{
    AUD_OPEN_FLAG_NULL           =   0,
    AUD_OPEN_FLAG_OK,
    AUD_OPEN_FLAG_ERR
}T_AUD_OPEN_FLAG;

#ifdef SUPPORT_MUSIC_PLAY

/** logic control */
typedef struct{
    T_U16                   musicPath[MAX_FILE_LEN];    /*music path playing*/

    T_AUD_PLAYER*           pPlayer;            /* play                     */

    T_AUDIO_CFG             audConfig;          /* config parameters        */
    T_AUD_CONTROL_STATE     playStat;           /* play status              */
    T_BOOL                  abplayMode;           /* audio play mode          */
    T_U8                    Powerstate;         /*battery low or charging or charged ok*/
    T_BOOL                  bMaxVol;
    T_U32                   audCtrlAct;         /* control action           */
    T_U32                   audCtrlActPost;     /* control action post handle*/ 
    T_TIMER                 audCtrlTimer;       /* AudioPlayer play timer   */ 
    T_U32                   ctrlCnt;            /* audio control timer counter*/

    T_U8                    preHandleOK;        /* preprocess handle OK     */
    T_U8                    preHandleErrCnt;    /* preprocess handle error cnt*/
    T_U8                    freqStackDepth;     /* freq stack depth         */
    T_BOOL                  setVolMode;         /* set volume mode(video)   */
    T_U8                    statJump;           /* jump flag for status changing*/
    T_U8                    saveMuteVol;         /*save the volume before mute*/
    T_U8                    freqCur;            /* aurrent frequence*/
    T_BOOL                  bClock2X;           /* clock 2x*/
    T_BOOL                  bFreqLock;          /* frquence lock mode:AK_TRUE, only can set higher frquence*/
    T_BOOL                  findnextplay;       /*auto to find next music to play*/
    T_U8                    findnextplaytimeout;/*stop auto to find next music to play*/
    T_EVT_PARAM             evtParam;
    T_EVT_CODE              evtCode;
    T_U32                   timeA;
    T_U32                   timeB;
    T_U32                   seeklen;			/*ms*/
    
    T_AUD_OPEN_FLAG			openflag;			/*one file at least can open, this flag is ok*/ 
	T_U16*					failpath;			/*path of the error file*/ 
	T_BOOL					bNoValidFile;		/*no valid file (in the current folder if Divid Folders)*/ 
	
	T_BOOL					bDividFolders;		/*Divid Folders or not*/
	
	T_BOOL					bNoValidFileAllFolder;	/*no valid file in all folders*/
	T_U8                    folderErrCnt;			/*count of the same folder which can't play*/
	T_U16*					failPathFirstFolder;	/*path of the error file in the first folder*/ 
} T_AUD_LOGIC_PARA;

/** file control play */  
typedef struct{
    T_U16                   selName[MAX_FILE_LEN];  /* select name          */
    T_eAUD_FILE_CTRL        rstCtrlType;            /* control type         */
} T_AUD_FILE_CTRL_PARA;

extern T_AUD_LOGIC_PARA* pAudLogic;
extern T_AUD_FILE_CTRL_PARA* pAudFileCtrl;

extern const T_U16  strUFilter[][MAX_FILTER_LEN];



/**
 * @brief   Audio control destroy
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlDestroy(T_VOID);

/**
 * @brief   Save audio data
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlSaveData(T_VOID);



/**
 * @brief   start timer
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID  Aud_AudCtrlTimerStart(T_VOID);


/**
 * @brief   stop timer
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID  Aud_AudCtrlTimerStop(T_VOID);

/**
 * @brief   play stop event handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlEvtHandle(T_EVT_PARAM *pEventParm);

/**
 * @brief   action handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlActHandle(T_VOID);



/**
 * @brief   action post handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlActPreHandle(T_AUD_LOG_PRE logPre);


/**
 * @brief   play cycle handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlPlyCycHandle(T_VOID);

/**
 * @brief   set playing AB speed 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   value
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL Aud_AudCtrlSetABRepTime(T_U32 value);

/**
 * @brief   set playing AB space 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   value
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL Aud_AudCtrlSetABRepSpac(T_U32 value);


/**
 * @brief   set cycric mode 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetCycMode(T_AUD_CYC_MODE mode);

/**
 * @brief   set tone mode 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetToneMode(T_EQ_MODE mode);

/**
 * @brief   set volume
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetVolume(T_U32 value);



/**
 * @brief   set speed
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
T_BOOL  Aud_AudCtrlSetSpeed(T_U32 value);


/**
 * @brief   auto power off is available
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  active
            AK_FALSE: inactive
 **/
T_BOOL  Aud_AudCtrlAvaPowerOff(T_VOID);

/**
 * @brief   auto backlight off is available
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  active
            AK_FALSE: inactive
 **/
T_BOOL  Aud_AudCtrlAvaBLOff(T_VOID);

/**
 * @brief   set freq 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_FALSE-no in audio modem
            AK_TRUE-in audio modem 
 **/
T_BOOL  Aud_AudCtrlFreqSet(T_VOID);



/**
 * @brief   enter standby handle
 * @author  WuShanwei
 * @date    2008-07-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_AudCtrlEnterStandby(T_EVT_CODE evtCode, T_EVT_PARAM* pEvtParam);

/**
 * @brief   Exit standby handle
 * @author  WuShanwei
 * @date    2008-07-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_AudCtrlExitStandby(T_VOID);


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
T_BOOL Aud_AudCtrlGetExtFileInfo(T_U16* fileName,T_U16* info);


/**
 * @brief   lock freq (only can set higher frequence)
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   freq: frequence to lock
 * @return  T_BOOL
 * @retval  AK_FALSE-no in audio modem
            AK_TRUE-in audio modem 
 **/
T_BOOL  Aud_AudCtrlFreqLock(T_U8 freq);
T_BOOL  Aud_AudCtrlFreqUnlock(T_VOID);

/**
 * @brief   Aud_AudCtrlUpdatePastTime
 * @author  Yanjunping
 * @date    2008-09-08
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID  Aud_AudCtrlUpdatePastTime(T_VOID);


/**
 * @brief   Aud_AudCtrlSetStatJump
 * @author  Yanjunping
 * @date    2008-11-21
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_VOID Aud_AudCtrlSetStatJump(T_U8 value);


/* * @brief IsAudplayer
 * @author  Yanjunping
 * @date    2008-11-21
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL IsAudplayer(T_VOID);

T_MEM_DEV_ID  Aud_GetCurDriver(T_VOID);


T_U32 Aud_AudCtrlStart(T_AUD_LISTSTLE listStyle);

/**
 * @brief   music play restart
 * @author  
 * @date    
 * @param   listStyle:play list style
 *
 * @return  T_U32
 * @retval     error type
 **/
T_U32 Aud_AudCtrlReStart(T_AUD_LISTSTLE listStyle);

T_AUD_CONTROL_STATE Aud_AudCtrlGetPlayState(void);


/**
 * @brief   action handle - stop
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlActStop(T_VOID);



/**
 * @brief   action handle - pause
 * @author  WuShanwei
 * @date    2008-06-23
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlActResume(T_VOID);


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
T_BOOL  Aud_AudCtrlActSetCfgVolMode(T_BOOL volMode);

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
T_BOOL  Aud_AudCtrlActGetCfgVolMode(T_VOID);


/**
 * @brief   action handle - set old position
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE:  failed
 **/
T_BOOL  Aud_AudCtrlActPlySetOldPos(T_VOID);


/**
 * @brief   convert key to action base on status
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   pEventParm
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL  Aud_AudCtrlKey2Act(T_EVT_PARAM *pEventParm);

//播放提示音时暂停/恢复播放
T_VOID Aud_AudCtrlActPowChgPause(T_VOID);

T_VOID Aud_AudCtrlActMaxVolPause(T_VOID);

T_VOID Aud_AudCtrlActAutoResume(T_VOID);

/**
* @brief	set seek len by seeklenId
* @author	WuShanwei
* @date	2008-04-01
* @param T_U8 seeklenId
* @return T_BOOL
* @retval
**/
T_BOOL Aud_AudCtrlSetSeekLen(T_U8 seeklenId);


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
T_AUD_LISTSTLE Aud_AudGetListStyle(T_VOID);


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
T_BOOL Aud_AudDelList(T_AUD_LISTSTLE listStyle);

/**
 * @brief   action handle - play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: init ok
            AK_FALSE: init failed
 **/
T_BOOL   Aud_AudCtrlActPlay(T_VOID);


#else

#define IsAudplayer()               (AK_FALSE)
#define Aud_GetCurDriver()          (SYSTEM_STORAGE)
#define Aud_AudCtrlGetStat()        (0)
#define Aud_AudCtrlEvtHandle(p)     (AK_TRUE)
#define Aud_AudCtrlPlyCycHandle()   (AK_TRUE)
#define Aud_AudCtrlFreqSet()        (AK_FALSE)
#define Aud_AudCtrlAvaPowerOff()    (AK_TRUE)
#define Aud_AudCtrlAvaBLOff()       (AK_TRUE)


#endif  //end of ifdef SUPPORT_MUSIC_PLAY

T_BOOL Aud_BGProcessMsg(T_EVT_CODE evtCode, T_EVT_PARAM *pEvtParam);

#endif
