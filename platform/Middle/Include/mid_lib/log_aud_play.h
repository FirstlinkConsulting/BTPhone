/**
 * @file    log_aud_play.h
 * @brief   audio play parameters config
 * @author  WuShanwei
 * @date    2008-04-03
 * @version 1.0
 */

#ifndef _LOG_AUD_PLAY_H_
#define _LOG_AUD_PLAY_H_


#include "Fwl_osFs.h"
#include "Gbl_Global.h"
#include "audio_lib.h"
#include "Fwl_FreqMgr.h"


/** audio info parameters*/
#define AUD_MSC_INFO_LEN_TITLE  14*2
#define AUD_MSC_INFO_LEN_ARTIST 14*2
#define AUD_MSC_INFO_LEN_ALBUM  14*2
#define MAX_AUD_MSC_INFO_LEN_ALL (AUD_MSC_INFO_LEN_TITLE+AUD_MSC_INFO_LEN_ARTIST+AUD_MSC_INFO_LEN_ALBUM+2*2+4)

/** setting parameters */
#define AUD_PLAY_SETG_MAX_VOL_STEP      10
#define AUD_PLAY_SETG_MAX_SPEED_STEP    10
#define AUD_PLAY_NORMAL_SPEED           5


/** audio freq param */
#define MAX_AUD_TYPE_ELE    5
#define MAX_AUD_BITSAMP_ELE 3
#define MAX_AUD_EQ_ELE      2
#define MAX_AUD_SPEED_ELE   2
#define MAX_AUD_BL_ELE      2
#define MAX_AUD_SAMPLE_RATE 72000   //48kHz
#define MAX_AUD_BIT_RATE    160000   //160kbps


#define PLAY_FREQ1  (T_U8)FREQ_APP_AUDIO_L1 
#define PLAY_FREQ2  (T_U8)FREQ_APP_AUDIO_L2
#define PLAY_FREQ3  (T_U8)FREQ_APP_AUDIO_L3
#define PLAY_FREQ4  (T_U8)FREQ_APP_AUDIO_L4 
#define PLAY_FREQ5  (T_U8)FREQ_APP_AUDIO_L5 


/** play status */  
typedef enum{
    AUDIOPLAY_STATE_NONE,
    AUDIOPLAY_STATE_PLAY,
    AUDIOPLAY_STATE_STOP,
    AUDIOPLAY_STATE_PAUSE
} T_AUD_PLAY_STATE;

typedef enum
{
    eAUD_MUSIC		= 0,	//nandflash music
    eAUD_VOICE		= 1,	//nandflash voice
    eAUD_USBMUSIC	= 2,	//USB HOST music
    eAUD_SDMUSIC	= 3,	//SD/TF music
    eAUD_SDVOICE	= 4,	//SD/TF voice
    eAUD_NUM
}T_AUD_LISTSTLE;


/** cyclic mode */
typedef enum{
    AUD_CYC_MODE_NORMAL,        
    AUD_CYC_MODE_SINGLE,    
    AUD_CYC_MODE_TOTAL, 
    AUD_CYC_MODE_MAX    
} T_AUD_CYC_MODE;   

    
#ifdef SUPPORT_MUSIC_PLAY

/** DA buffer */
typedef struct {
    T_U8*       pDMABuf;        /* DMA buffer               */
    T_U8*       pRead;          /* read PC flag             */
    T_U8*       pWrite;         /* write PC flag            */
    T_U32       len;            /* length of buffer         */
} T_AUD_DA_BUF;

/** player parameters */
typedef struct _AUDIOPLAYER {
    T_U32           curTime;        /* the current position of playing audio */  
    T_U32           totalTime;      /* the total length of playing audio    */ 
    T_U32           pastTime;       /* when set the mode,save the curtime   */
    T_hFILE         hFile;          /* file handle                          */
    T_hFILE         hVideoFile;
    T_U16*          pFileName;      /* file name                            */
    T_U32           sampleRate;     /* file sample rate                     */
    T_U32           bitRate;        /* file bit rate                        */
    volatile T_pVOID hMedia;            /* media handle                         */
#ifdef SUPORT_MUISE_EQ
	T_pVOID			pEQFilter;		/* EQ handle						*/
#endif
    //T_BOOL            flagHDPopen;    /* DA interrupt flag                    */
    T_U8            audType;        /* the type of playing audio            */
    T_U8            volume;         /* init value                           */ 
    T_U8            modeEQ;         /* eq mode                              */
    T_U8            modeSpeed;      /* eq mode                              */
    T_U8            decodetask_id;  /* eq mode                              */
    T_U8            readtask_id;    /* eq mode                              */
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))        
    T_U16           strMscInfo[AUD_MSC_INFO_LEN_TITLE+1];   /* music infomation     */
    T_U16           strMscAlbum[AUD_MSC_INFO_LEN_ALBUM+1]; /* music infomation     */
#else   
    T_U16           strMscInfo[MAX_AUD_MSC_INFO_LEN_ALL];   /* music infomation     */
#endif
    T_U16           discard_size;
    T_WSOLA_ARITHMATIC speedLibType;   // 2009-2 by Zhao_Xiaowei
    T_eMEDIALIB_MEDIA_TYPE  mediaType;
    T_U32           streamBufLen;   /* length of stream buffer , byte               */
    T_U32           expReadLen;     /* length of expect to read file , byte         */
    T_U32           emptyDecLen;    /* length of stream buffer can't to decode , byte*/
    T_U32           daBufLen;       /* length of da buffer , byte                    */
    volatile T_U8   pcm_id;
    T_BOOL          bFade;
    #ifdef SUPPORT_BLUETOOTH
    T_BOOL          bset_eq_flag;
    #endif
} T_AUD_PLAYER;

//handle 回调函数
typedef T_AUD_PLAYER* (*AUDPLAYER_CALLBACK_HANDLEPRE)(T_VOID);
typedef T_VOID (*AUDPLAYER_CALLBACK_HANDLEPOST)(T_S32 size , T_AUD_PLAYER* pAudPlayer);


#define MAX_DECODE_BUF_LEN      4*1024
#define MAX_EQ_BUF_LEN          4*1024
#define MAX_PINGPANG_BUF_LEN    4*1024

#define MAX_VOLUME              31     //music max volume



/**
 * @brief   player Init
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_AUD_PLAYER*
 * @retval  
 **/
T_AUD_PLAYER* APlayer_Init(T_U8 vol, T_U8 speed, T_U8 eq);


/**
 * @brief   Aud_PlayerSetVolume
 * @author  liangxiong
 * @date    2012-11-22
 * @param   vol (0~1024)
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_SetVolume(T_U8 vol);

/**
 * @brief   player Exit
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Destroy(T_AUD_PLAYER* player);


/**
 * @brief   player open 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            file
            medType: media type, 0-audio, 1-vedio
            bFade:  AK_TRUE - fade enable  AK_FALSE-fade disable
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
 /*
Modify by Zhao_Xiaowei @ 2009-02-17
medType 一般为MEDIALIB_MEDIA_UNKNOWN 即0 ，但是系统会耗大量时间去判断其类型
如果想节省这部分时间则传入具体的类型如MEDIALIB_MEDIA_MP3
*/
T_BOOL  APlayer_Open(T_AUD_PLAYER* player,T_USTR_FILE file,T_eMEDIALIB_MEDIA_TYPE medType, T_BOOL bFade,  T_U32 bFadeInTime, T_U32  bFadeOutTime,T_WSOLA_ARITHMATIC speedLibType);

/**
 * @brief   player play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            file
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Play(T_AUD_PLAYER* player);

/**
 * @brief   player pause
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Pause(T_AUD_PLAYER* player);

/**
 * @brief   player stop
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Stop(T_AUD_PLAYER* player);

/**
 * @brief   resume play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Resume(T_AUD_PLAYER* player);

/**
 * @brief   Set play speed
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_U16
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_SetSpeed(T_AUD_PLAYER* player,T_U32 speed);

/**
 * @brief   Set EQ mode
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_U8
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_SetToneMode(T_AUD_PLAYER* player,T_U8 toneMode);

/**
 * @brief   player seek
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            time
 * @return  T_U32
 * @retval  
 **/
T_S32   APlayer_Seek(T_AUD_PLAYER* player,T_U32 time);


/**
 * @brief   Set Volume
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_U32
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_GetDuration(T_AUD_PLAYER* player,T_U32 *duration);

/**
 * @brief   calculate fit frequency for audio 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_U8
 * @retval  
 **/
T_U8 APlayer_CalcFreq(T_AUD_PLAYER* player,T_BOOL enableBL, T_U8 medType);

/**
 * @brief   Aud_PlayerRegitHDLPre
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  
 * @retval  
 **/
T_VOID APlayer_RegHDLPre(AUDPLAYER_CALLBACK_HANDLEPRE pCallBackFun);
/**
 * @brief   Aud_PlayerRegitHDLPOST
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_AUD_PLAYER*
 * @retval  
 **/
T_VOID APlayer_RegHDLPost(AUDPLAYER_CALLBACK_HANDLEPOST pCallBackFun);

/**
 * @brief   Open WAVE DAC, start task.
 * @author  hyl
 * @date    2013-03_09
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  APlayer_LevDepStdby(T_AUD_PLAYER* player);

/**
 * @brief   Close WAVE DAC and stop task.
 * @author  hyl
 * @date    2013-03_09
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  APlayer_EntDepStdby(T_AUD_PLAYER* player);

/**
 * @brief    pop HDLPre HDLPost handle
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  
 * @retval  
 **/
T_VOID APlayer_PrePostPop(T_VOID);

#endif

#endif

