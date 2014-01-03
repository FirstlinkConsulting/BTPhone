/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME logic_audiorec.h
 * @BRIEF the file provide the api for audio-record application
 * @Author：Huang_ChuSheng
 * @Date：2008-04-04
 * @Version：
**************************************************************************/
#ifndef _LOGIC_AUDIOREC_H_
#define _LOGIC_AUDIOREC_H_


#include "Gbl_Global.h"
#include "audio_lib.h"
#include "Fwl_osFS.h"
#include "Eng_Time.h"


#if(NO_DISPLAY == 1)
//#define RECORD_PCM_ONLY //只录制pcm，不使用任何库
#endif

#ifndef USE_CHIP_TYPE
#define USE_CHIP_TYPE               1050L
#endif
#define SHOW_WAVE                   0 //是否显示波形
#define REC_MAX_TIME_VALUE          (36*3600)
#define IN_BUFFER_SIZE              (4096)
#define OUT_BUFFER_SIZE             (IN_BUFFER_SIZE)
#define SINGLE_OUT_BUFFER_SIZE      (2048)
#define DMAMALLOC_PAGESIZE          (4096)

#define PING_BUF                0
#define PANG_BUF                1
#define PINGPANG_INVALID        2
#define AUDIOREC_MINIMAL_SPACE  (200*1024)
/* define the prefix of record file */
#define REC_FILE_PREFIX         "REC"
#define REC_FILE_PREFIX_LEN     3 //NOTE:when REC_FILE_PREFIX was changed,must modify it

/* define the extension name */
#define REC_FILE_WAVE           ".wav"
#define REC_FILE_WAVE_EX        "REC*.wav"

#define REC_RESERVE_SPACE       (16<<20)

//wav vor rec value to decide whether rec data is validate
#define VORREC_CTRLVALUE        1300

//vor rec timer for recording several minutes at least when record start
#define VOR_RECORD_TIMERVALUE   3500

//wav 16k 16bit
#define FINE_REC                256
//adpcm 16k 4bit
#define LONG_REC                64
//wav 8k 16bit
#define FINE_VOR                128
//adpcm 8k 4bit
#define LONG_VOR                32

typedef enum
{
    GAIN_LEVEL0 = 0, //10times (20db)
    GAIN_LEVEl1,    //12.5times(22db)
    GAIN_LEVEL2,    //16times(24db)
    GAIN_LEVEL3,    //20times(26db)
    GAIN_LEVEL4,    //25times(28db)
    GAIN_LEVEL5,    //32times(30db)
    GAIN_LEVEL6,    //40times(32db)
    GAIN_LEVEL7,    //60times(35.6db)
    GAIN_MAX
}GAIN_LEVEL;

#define REC_BIT_RATE_WAV16K        (FINE_REC) //byte
#define REC_BIT_RATE_WAV8K         (FINE_VOR) //byte
#define REC_BIT_RATE_ADPCM16K      (LONG_REC) //byte
#define REC_BIT_RETE_ADPCM8K       (LONG_VOR) //byte
#define REC_BIT_RETE_ADPCM8K_2     (2*LONG_VOR)
#define REC_BIT_RETE_ADPCM48K      (3*LONG_VOR)
#define REC_BIT_RETE_ADPCM48K_2    (4*LONG_VOR)

/* define record mode*/
typedef enum{
    eREC_MODE_WAV8K = 0,    //优质录音          
    eREC_MODE_WAV16K,       //优质录音
    eREC_MODE_WAV48K,		//高品质录音
    eREC_MODE_WAV48K_2,
    eREC_MODE_ADPCM8K,      //长时声控
    eREC_MODE_ADPCM8K_RADIO,//32K比特率
    eREC_MODE_ADPCM16K,     //长时录音
    eREC_MODE_ADPCM8K_2,    //64K比特率
    eREC_MODE_ADPCM48K,     //96k比特率
    eREC_MODE_ADPCM48K_2,   //129k比特率
    eREC_MODE_AMR,          
    eREC_MODE_MP3LOW,       //8K 双声道
    eREC_MODE_MP3HI,        //22K 单声道
    eREC_MODE_SPEEX_LOW,    //8K 双声道
    eREC_MODE_SPEEX_HI,     //22K 单声道
    eREC_MODE_UNKNOW
}T_eREC_MODE;
/* define all the state in recorder */
typedef enum {
    eSTAT_REC_STOP = 0,     //state of record stop
    eSTAT_RECORDING,        //state of recording voice
    eSTAT_REC_PAUSE,        //state of pausing
    eSTAT_MAX               //max state count
}T_eREC_STAT;

/* define all the state in recorder */
typedef enum {
    eSTAT_RCV_MIC = 0,      //recv data from mic
    eSTAT_RCV_LINEIN,       //recv data from line in
    eSTAT_RCV_CLOSE,        //close input
    eSTAT_RCV_MAX           //max state count
}T_eREC_RCVSTAT;

typedef enum {
    eSTAT_VORREC_PAUSE = 0,
    eSTAT_VORREC_RECORDING,
    eSTAT_VORREC_CLOSE
}T_eVORREC_STATE;

//vor rec state
typedef enum{
    CTRL_EVENT_VORREC_PAUSE = 0,
    CTRL_EVENT_VORREC_RESUME,
    CTRL_EVENT_MEMORY_FULL,
    CTRL_EVENT_SINGLE_FILE_LEN_LIMIT
}T_eREC_CTRL_EVENTPARAM;

typedef T_BOOL (*T_VOICE_CTRL_REC_CALLBACK)(T_BOOL isPhonation);


/************************************************************************
 * @BRIEF init the audio record module
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_RECORD_CFG *recCfg: the record mode and save path
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
T_BOOL AudioRecord_Init(T_VOID);

/************************************************************************
 * @BRIEF create a file and start to record
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_U16 **pRecFileName: the name of file which is created
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to record
 * @RETVAL AK_FALSE:start to record failed
 **************************************************************************/
T_BOOL AudioRecord_Start(const T_U16 *pRecName, T_eREC_MODE RecMode ,T_S32 volLarger, T_BOOL isVorRec);

/************************************************************************
 * @BRIEF pause recording
 * @AUTHOR GB
 * @DATE  2008-10-24
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_Pause(T_VOID);

/************************************************************************
 * @BRIEF resume recording
 * @AUTHOR GB
 * @DATE  2008-10-24
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_Resume(T_VOID);

/************************************************************************
 * @BRIEF stop recording
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully stop recording
 * @RETVAL AK_FALSE:stop recording failed
 **************************************************************************/
T_BOOL AudioRecord_Stop(T_VOID);

/************************************************************************
 * @BRIEF destory the resource which  was malloce fbr recording
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_Destroy(T_VOID);

/************************************************************************
 * @BRIEF Open record Wave in device
 * @AUTHOR hyl
 * @DATE  2013-03-8
 * @PARAM T_VOID
 * @RETURN T_BOOL: succese or fail
 * @RETVAL 
 **************************************************************************/
T_BOOL AudioRecord_OpenWavIn(T_VOID);
    
/************************************************************************
 * @BRIEF Close record Wave in device
 * @AUTHOR hyl
 * @DATE  2013-03-8
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL 
 **************************************************************************/
T_VOID AudioRecord_CloseWavIn(T_VOID);

/************************************************************************
 * @BRIEF Stop record Wave in device
 * @AUTHOR hxl
 * @DATE  2013-07-6
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL 
 **************************************************************************/
T_VOID AudioRecord_StopWaveIn(T_VOID);

/************************************************************************
 * @BRIEF Start record Wave in device
 * @AUTHOR hxl
 * @DATE  2013-07-6
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL 
 **************************************************************************/
T_VOID AudioRecord_StartWaveIn(T_VOID);

/************************************************************************
 * @BRIEF Get current record time
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL 
 **************************************************************************/
T_U32 AudioRecord_GetCurrentTime(T_VOID);

/************************************************************************
 * @BRIEF get the length of data it can save
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_U16 DrvIndx,T_eREC_MODE recMode
 * @RETURN T_U32
 * @RETVAL the time it can record (s)
 **************************************************************************/
T_U32 AudioRecord_GetTotalTime(T_U16 DrvIndx, T_eREC_MODE recMode);

/************************************************************************
 * @BRIEF destory the resource which  was malloce fbr recording
 * @AUTHOR hxq
 * @DATE  2012-01-30
 * @PARAM input:T_U16 DrvIndx:存储设备索引号;
 *               output:T_U64_INT freesize,存储设备空闲空间.
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_CalcFreeSize(T_U16 DrvIndx, T_U64_INT *freesize);

/************************************************************************
 * @BRIEF check currentBufFlag ,encode and save the data into file
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL Successfully save the data into file
 * @RETVAL save data failed
 **************************************************************************/
T_BOOL AudioRecord_Process(T_VOID);

/************************************************************************
 * @BRIEF get record mode
 * @AUTHOR GB
 * @DATE  2008-10-24
 * @PARAM T_VOID
 * @RETURN T_eREC_MODE recMode
 **************************************************************************/
T_eREC_MODE AudioRecord_GetMode(T_VOID);

/************************************************************************
 * @BRIEF get record stop Status
 * @AUTHOR he_yuanlong
 * @DATE  2013-4-12
 * @PARAM T_VOID
 * @RETURN T_eREC_STAT ,eSTAT_REC_STOP, eSTAT_REC_PAUSE, eSTAT_REC_RECORDING
 **************************************************************************/
T_eREC_STAT AudioRecord_GetRecState(T_VOID);

/************************************************************************
 * @BRIEF Get whether is vor record 
 * @AUTHOR
 * @DATE  2013-3-13
 * @PARAM T_VOID
 * @RETURN T_BOOL bVorRec
 **************************************************************************/
T_BOOL  AudioRecord_IsVorCtrlRec(T_VOID);

/************************************************************************
 * @BRIEF set data source 
 * @AUTHOR GB
 * @DATE  2008-10-24
 * @PARAM T_eREC_RCVSTAT state:linein/mic
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_SetDataSource(T_eREC_RCVSTAT state);

/************************************************************************
 * @BRIEF set audiorecord gain.
 * @AUTHOR hxq
 * @DATE  2011-11-16
 * @PARAM GAIN_LEVEL.
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_SetGain(GAIN_LEVEL gain);

/************************************************************************
 * @BRIEF get audiorecord gain.
 * @AUTHOR hxq
 * @DATE 2011-11-16
 * @PARAM T_VOID
 * @RETURN GAIN_LEVEL
 **************************************************************************/
GAIN_LEVEL AudioRecord_GetGain(T_VOID);

/************************************************************************
 * @BRIEF set Voice ctrl Rec callback func.
 * @AUTHOR hxq
 * @DATE 2013-2-26
 * @PARAM T_VOICE_CTRL_REC_CALLBACK
 * @RETURN void
 **************************************************************************/
T_VOID AudioRecord_SetVorCbk(T_VOICE_CTRL_REC_CALLBACK callback, T_U32 LeastVor);

/************************************************************************
 * @BRIEF SET Voice ctrl Rec State.
 * @AUTHOR he_yuanlong
 * @DATE 2013-2-26
 * @PARAM T_VOID
 * @RETURN void
 **************************************************************************/
T_VOID AudioRecord_SetVorCtrlState(T_eVORREC_STATE state);

T_eVORREC_STATE AudioRecord_GetVorCtrlState(T_VOID);

#ifndef SUPPORT_AUDIO_RECORD
#define Rec_GetCurDriver() (SYSTEM_STORAGE)
#else
T_MEM_DEV_ID Rec_GetCurDriver(T_VOID);
#endif

#endif

