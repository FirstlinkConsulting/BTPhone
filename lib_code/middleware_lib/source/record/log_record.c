/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME logic_audiorec.c
 * @BRIEF the file provide the api for audio-record application
 * @Author：Huang_ChuSheng
 * @Date：2008-04-04
 * @Version：
**************************************************************************/
#include "Fwl_osMalloc.h"
#include "Gbl_Global.h"
#include "log_record.h"
#include "Eng_DataConvert.h"
#include "Eng_String.h"
#include "Fwl_Timer.h"
#include "Fwl_FreqMgr.h"
#include "vme.h"
#include "Eng_Debug.h"
#include "Fwl_System.h"
#include "Fwl_WaveIn.h"
#include "Eng_CycBuf.h"


#define UPDATE_INTERVAL_10          (10000) //10 second
#define UPDATE_INTERVAL_30          (30000) //30 second
#define REC_TIME_LIMIT              (10*60*60*1000)  //10 hour
#define SINGLE_FILE_LEN_LIMIT       (0x80000000)//2*1024*1024*1024 = 2G

#define MP3_8K_PS                   (40<<7)// 40* 1024/8
#define MP3_22K5_PS                 (40<<7) // 40*1024/8
#define AMR_8K_PS                   (50*14)
//#define REC_BUF_COUNT             (32)
#define REC_BUF_COUNT               (MAX_CYCBUF_SIZE>>12)
#define USE_REC_DEBUG               (0)

#define CHANNEL_NUM_1               (1)
#define BITSPERSAMPLE_16            (16)
#define BITSPERSAMPLE_4             (4)

#define BITRATE_0                   (0)

#define SAMPLERATE_WAV8K            (8000)
#define CHANNEL_NUM_WAV8K           (1)
#define BYTES_PER_SEC_WAV8K      	(16000)

#define SAMPLERATE_WAV16K           (16000)
#define CHANNEL_NUM_WAV16K          (1)
#define BYTES_PER_SEC_WAV16K      	(32000)

#define SAMPLERATE_WAV48K         	(48000)
#define CHANNEL_NUM_WAV48K        	(1)
#define BYTES_PER_SEC_WAV48K      	(96000)

#define SAMPLERATE_WAV48K_2         (48000)
#define CHANNEL_NUM_WAV48K_2        (2)
#define BYTES_PER_SEC_WAV48K_2      (192000)

#define SAMPLERATE_ADPCM8K          (8000)
#define CHANNEL_NUM_ADPCM8K         (1)

#define SAMPLERATE_ADPCM16K         (16000)
#define CHANNEL_NUM_ADPCM16K        (1)

#define SAMPLERATE_ADPCM8K_2        (8000)
#define CHANNEL_NUM_ADPCM8K_2       (2)
#define BITRATE_ADPCM8K_HI			(64000)  //64kpbs

#define SAMPLERATE_ADPCM48K         (48000)
#define CHANNEL_NUM_ADPCM48K        (1)

#define SAMPLERATE_ADPCM48K_2       (48000)
#define CHANNEL_NUM_ADPCM48K_2      (2)

#define SAMPLERATE_MP3LOW           (8000)
#define CHANNEL_NUM_MP3LOW          (2)
#define BITRATE_MP3_LOW			    (32)  //32kpbs

#define SAMPLERATE_MP3HI            (22050)
#define CHANNEL_NUM_MP3HI           (1)
#define BITRATE_MP3_HI			    (32)  //32kpbs

#define SAMPLERATE_SPEEX_LOW        (8000)
#define CHANNEL_NUM_SPEEX_LOW       (1)
#define BITRATE_SPEEX_LOW			(6000)

#define SAMPLERATE_SPEEX_HI         (8000)
#define CHANNEL_NUM_SPEEX_HI        (1)
#define BITRATE_SPEEX_HI			(15000)

//sample rate = fact/calc
#define SAMPLE_8K_RATE                (0.994)
#define SAMPLE_16K_RATE               (1.001)
#define SAMPLE_22K_RATE               (0.980)
#define SAMPLE_48K_RATE               (1.000)

#define RECTIME_PER_SAMP_CHAN(samplerate, channels)      ((OUT_BUFFER_SIZE*1000)/((samplerate)*2*(channels)))//ms

typedef enum
{
    eREC_NORMAL,
    eREC_FULL,
    eREC_EMPTY
}T_eRecBufState;

typedef struct{
    T_U32           mallocSize;
    T_U8            *OutBuf;
    T_U32           bufsize;
    volatile T_eRecBufState recBufState;
    volatile T_U32   send_len;
    T_U32            bufCount;
    T_U32            maxLeftCount;
    T_eREC_MODE      recMode;
    #ifndef RECORD_PCM_ONLY
    T_MEDIALIB_ENC_OPEN_INPUT audio_record_enc;
    #endif    
    T_eREC_STAT      RecState;//pause flag
    T_U32            totalRecTime;  //total record time
    T_U32            currentRecTime;   //current record time
    T_U32            UpdateTime;
    //T_U32          currentBufFlag: 2;  //0 for pPingBuf is full, 1 for pPangBuf is full;
    T_U32            rec_len;//: 30;      //each time record length;
    T_U32            total_len;    //total record length;
    T_hFILE          rec_fd;     //record file handle;
    T_pVOID          hMedia;     //media handle
    T_eREC_RCVSTAT   revState;  //the recv data state
    T_VOICE_CTRL_REC_CALLBACK Vorcallback;//voice control Rec CALLBACK
    T_TIMER          vorRecTimer;//vor rec timer
    T_U32            VorCtlValue;//voice ctrl Rec value
    T_eVORREC_STATE  VorCtrlState;//vor rec last  state
    volatile T_U32   ADCounter;//ad int counter
    T_U64_INT        freeSize;
    GAIN_LEVEL       GainLevel;   
    T_BOOL           isBufMalloc;
    T_BOOL           bVideoRec;  //is video record
    T_BOOL           bVorRec;//whether vor rec is not
    T_BOOL           bCoding;//whether need to code
    T_U8             wavin_id;
    T_U8             channels;   //there is how many channels use in rec 
    T_U32            RecSampleRate;
    T_U16            RecBitsPerSample;
}AUDIOREC_STRUCT;

#pragma arm section zidata = "_bootbss1_"
static AUDIOREC_STRUCT *pAudioRecord = AK_NULL;
#pragma arm section zidata


#ifdef WIN32
extern T_U32 MMU_Vaddr2Paddr(T_U32 vaddr);
#endif
static T_U32 AudioRecord_DealData(T_U8 *pDataBuf, T_U32 size);
static T_VOID AudioRecord_WriteData(T_U8* pDest, T_U32 data, T_U32 bytenum);
static T_VOID AudioRecord_UpdateHead(T_VOID);
//static T_VOID AudioRecord_DataConvert(T_U8 *pdata);
static T_VOID AudioRecord_CtrlTimerCallBackFunc(T_TIMER timer_id, T_U32 delay);
static T_VOID AudioRecord_InterruptHandle(T_U8 **buf, T_U32 *len);
static T_VOID AudioRecord_CtrlTimerStart(T_VOID);
static T_BOOL Vor_default_cbk(T_BOOL bVorValid);
static T_U32 AudioRecord_ComputeTotalTime(T_U64_INT freesize,T_eREC_MODE recMode);

/* wave header structure */
typedef struct {
    T_CHR  riff[4];         // "RIFF"
    T_S32  file_size;       // in bytes
    T_CHR  wavefmt_[8];     // "WAVE"
    T_S32  chunk_size;      // in bytes (16 for PCM)
    T_S16  format_tag;      // 1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law
    T_S16  num_chans;       // 1=mono, 2=stereo
    T_S32  sample_rate;
    T_S32  bytes_per_sec;
    T_S16  bytes_per_samp;  // 2=16-bit mono, 4=16-bit stereo
    T_S16  bits_per_samp;
    T_CHR  data[4];         // "data"
    T_S32  data_length;     // in bytes
} T_WAVE_HEADER;


/* adpcm header structure */
typedef struct {
    T_S16  wFormatTag;      // wave_format_dvi_adpcm / wave_format_ima_adpcm (0x0011)
    T_S16  nChannels;       //channel 
    T_S32  nSamplesPerSec;  // samplerate
    T_S32  nAvgBytesPerSec; // 每秒多少个字节, 4055 = 256*8000/505  
    T_S16  nBlockAlign;     // 数据块的调整数, 取决于每个采样点的位数 - 256 
    T_S16  wBitsPerSample;  // 每样本数据位数 - 4  
    T_S16  cbSize;          // 保留参数- 2 
    T_S16  wSamplesPerBlock;// 每个数据块包含采样点数, 505
} T_IMA_ADPCM_WAVEFORMAT;

typedef struct {
    T_CHR  riff[4];         // "RIFF"
    T_S32  file_size;       // in bytes
    T_CHR  wavefmt_[8];     // "WAVE"
    T_S32  imachunk_size;   // in bytes (0x14 for IMA ADPCM)
    T_IMA_ADPCM_WAVEFORMAT ima_format;
    T_CHR  fact[4];         //"fact"
    T_S32  factchunk_size;  //fact chunk size
    T_S32  factdata_size;   //fact data size
    T_CHR  data[4];         // "data"
    T_S32  data_length;     // in bytes
} T_ADPCM_HEADER;


#ifndef RECORD_PCM_ONLY
static T_ADPCM_HEADER fileHead = {
    {'R','I','F','F'},                  //"RIFF"
    0x98D4,                             //file size
    {'W','A','V','E','f','m','t',' '},  //"WAVEfmt "
    0x14,                               //IMAADPCMWAVEFORMAT struct size(0x14 for IMA ADPCM)
    {0x11,                              //IMA adpcm format
    1,                                  //channel 1=mono, 2=stereo
    8000,                               //default 8k sample rate
    0x0FD7,                             //bytes_per_sec
    256,                                //block align
    4,                                  //bits per sample
    2,                                  //reserve bit
    505},                               //samples per packet
    {'f','a','c','t'},                  //"fact"
    4,                                  //fact chuck size
    0x12D0B,                            //the number of samples
    {'d','a','t','a'},                  //"data"
    0x98A0                              //data len
};
#endif


/*
 * default setting of wave header for recording
 * at sample rate of 8000, mono, 16bits, in PCM format
 */
static T_WAVE_HEADER fileHeadPcm = {
    {'R','I','F','F'},                  //"RIFF"
    0x24D4,                             //file size
    {'W','A','V','E','f','m','t',' '},  //"WAVEfmt "
    0x10,                               //in bytes (16 for PCM)
    0x1,                                //1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law
    1,                                  //1=mono, 2=stereo
    8000,                               //default 8k sample rate
    0x3E80,                             //bytes_per_sec
    2,                                  //bits_per_samp / 8 * num_chans
    16,                                 //default 16bits
    {'d','a','t','a'},                  //"data"
    0x2E2440                            //data_length
};

#if 0
#ifndef RECORD_PCM_ONLY
//record file head
static T_U8 fileHead[] = {0x52,0x49,0x46,0x46,0xD4,0x98,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,\
                           0x14,0x00,0x00,0x00,0x11,0x00,0x01,0x00,0x40,0x1F,0x00,0x00,0xD7,0x0F,0x00,0x00,\
                           0x00,0x01,0x04,0x00,0x02,0x00,0xF9,0x01,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00,\
                           0x0B,0x2D,0x01,0x00,0x64,0x61,0x74,0x61,0xA0,0x98,0x00,0x00};
#endif

static T_U8 fileHeadPcm[] = {0x52,0x49,0x46,0x46,0xD4,0x24,0x2E,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,\
                              0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x40,0x1F,0x00,0x00,0x80,0x3E,0x00,0x00,\
                              0x02,0x00,0x10,0x00,0x64,0x61,0x74,0x61,0x40,0x24,0x2E,0x00};
#endif
//extern volatile T_U8 rec_buf_flag;
extern volatile T_U8 send_flag;

#if (0)
T_VOID  Ram_Info(unsigned short *cntUsed, unsigned long *szUsed);
T_VOID AudioRecord_Print(T_U8* str)
{
    T_U16 szCntUsed;
    T_U32 szUsed;
    Ram_Info(&szCntUsed, &szUsed);
    AK_DEBUG_OUTPUT("%s szCntUsed:%d szUsed:%d\n", str, szCntUsed, szUsed);
}
#endif


T_VOID AudioRecord_UpdateAllFlags(T_VOID)
{
//  pAudioRecord->head= 0;
//  pAudioRecord->recIndex= pAudioRecord->head;
    pAudioRecord->recBufState= eREC_EMPTY;
}
/************************************************************************
 * @BRIEF init the audio record module
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_RECORD_CFG *recCfg: the record mode and save path
 * @RETURN T_VOID
 * @RETVAL
 **************************************************************************/
T_BOOL AudioRecord_Init(T_VOID)
{ 
    T_U32 i = 0;
    T_U32 size = 0;
    AK_DEBUG_OUTPUT("AudioRecord_Init size:%d!!!!\n", sizeof(AUDIOREC_STRUCT));

    //AudioRecord_Print("AudioRecord_Init 1");

    size = sizeof(AUDIOREC_STRUCT);
    i = (size / DMAMALLOC_PAGESIZE);
    if (size % DMAMALLOC_PAGESIZE)
    {
        i++;
    }
    size = DMAMALLOC_PAGESIZE*i;
    
    pAudioRecord = (AUDIOREC_STRUCT*)Fwl_DMAMalloc(size);
    //AudioRecord_Print("AudioRecord_Init 2");
    if (AK_NULL == pAudioRecord)
    {
        AK_DEBUG_OUTPUT("No more memory to alloc for pAudioRecord!!!!\n");
        return AK_FALSE;
    }

	#ifndef RECORD_PCM_ONLY
    memset(&(pAudioRecord->audio_record_enc), 0, sizeof(T_MEDIALIB_ENC_OPEN_INPUT));
    #endif    
 
    pAudioRecord->mallocSize = size;
    pAudioRecord->OutBuf = AK_NULL;
    pAudioRecord->bufsize = 0;
    pAudioRecord->isBufMalloc = AK_FALSE;
        
    pAudioRecord->recMode = eREC_MODE_WAV16K;
    pAudioRecord->bCoding = AK_FALSE;
    pAudioRecord->revState = eSTAT_RCV_MIC;
    pAudioRecord->Vorcallback = Vor_default_cbk;
    pAudioRecord->ADCounter=0;

    pAudioRecord->RecState = eSTAT_REC_STOP;
    pAudioRecord->rec_fd = FS_INVALID_HANDLE;
    pAudioRecord->rec_len = 0;
    pAudioRecord->total_len = 0;
    pAudioRecord->totalRecTime = 0;
    pAudioRecord->currentRecTime = 0;
    pAudioRecord->UpdateTime = 0;
    pAudioRecord->hMedia = AK_NULL;
    pAudioRecord->vorRecTimer = ERROR_TIMER;
    pAudioRecord->VorCtlValue = VORREC_CTRLVALUE;
    pAudioRecord->VorCtrlState = eSTAT_VORREC_CLOSE;
    
    pAudioRecord->freeSize.high = 0;
    pAudioRecord->freeSize.low = 0;

    pAudioRecord->bVideoRec = AK_FALSE;
    pAudioRecord->GainLevel = GAIN_LEVEL7;

    pAudioRecord->wavin_id = 0xff;
/*  
    for(i= 0; i <REC_BUF_COUNT; i++)
    {
        pAudioRecord->pRecBufs[i]= AK_NULL;
    }
    */
    AudioRecord_UpdateAllFlags();
    //AudioRecord_Print("AudioRecord_Init 4");
    return AK_TRUE;
}

static T_BOOL Vor_default_cbk(T_BOOL bVorValide) 
{
    T_EVT_PARAM EventParm;

    if(pAudioRecord->vorRecTimer==ERROR_TIMER)
    {
        //AK_DEBUG_OUTPUT("long bValideData=%d\n",bVorValide);
        if(!bVorValide)
        {
            if(eSTAT_VORREC_RECORDING == pAudioRecord->VorCtrlState)
            {
                Fwl_ConsoleWriteStr("long vor pause\n");
                pAudioRecord->VorCtrlState = eSTAT_VORREC_PAUSE;
                EventParm.c.Param1 = CTRL_EVENT_VORREC_PAUSE;
                VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
            }
            //rec_buf_flag = 0;
            return AK_FALSE;
        }
        else 
        {
            if(eSTAT_VORREC_PAUSE == pAudioRecord->VorCtrlState)
            {
                Fwl_ConsoleWriteStr("long vor resume\n");
                pAudioRecord->VorCtrlState = eSTAT_VORREC_RECORDING;
                EventParm.c.Param1 = CTRL_EVENT_VORREC_RESUME;
                VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
                AudioRecord_CtrlTimerStart();
            }               
        }
    }
    return AK_TRUE;
}


T_VOID AudioRecord_SetVorCbk(T_VOICE_CTRL_REC_CALLBACK callback, T_U32 LeastVor)
{
    if (callback == AK_NULL)
    {
        pAudioRecord->Vorcallback = Vor_default_cbk;
    }
    else
    {
        pAudioRecord->Vorcallback = callback;
    }

    pAudioRecord->VorCtlValue = LeastVor;
}

T_eREC_MODE AudioRecord_GetMode(T_VOID)
{
    return pAudioRecord->recMode;
}
#pragma arm section code = "_frequentcode_"

/************************************************************************
 * @BRIEF get record stop Status
 * @AUTHOR he_yuanlong
 * @DATE  2013-4-12
 * @PARAM T_VOID
 * @RETURN T_eREC_STAT ,eSTAT_REC_STOP, eSTAT_REC_PAUSE, eSTAT_REC_RECORDING
 **************************************************************************/
T_eREC_STAT AudioRecord_GetRecState(T_VOID)
{
    if (!pAudioRecord)
        return eSTAT_REC_STOP;
    else 
        return pAudioRecord->RecState;
}
#pragma arm section code

T_BOOL AudioRecord_IsVorCtrlRec(T_VOID)
{
    if (eSTAT_VORREC_RECORDING == pAudioRecord->VorCtrlState
        || eSTAT_VORREC_PAUSE == pAudioRecord->VorCtrlState)
    {
        return AK_TRUE;
    }
    else //if(eSTAT_VORREC_CLOSE == pAudioRecord->VorCtrlState)
    {
        return AK_FALSE;
    }    
}

/************************************************************************
 * @BRIEF vor rec timer for recording several minutes when start record
 * @AUTHOR guibing
 * @DATE  2008-06-19
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
#pragma arm section code = "_recording_"
static T_VOID   AudioRecord_CtrlTimerStart(T_VOID)
{
    if(ERROR_TIMER != pAudioRecord->vorRecTimer)
    {
        Fwl_TimerStop(pAudioRecord->vorRecTimer);
        pAudioRecord->vorRecTimer = ERROR_TIMER;
        AK_DEBUG_OUTPUT("vor timer id=%x\n",pAudioRecord->vorRecTimer);
    }
    
    pAudioRecord->vorRecTimer = Fwl_TimerStart(VOR_RECORD_TIMERVALUE, AK_FALSE, AudioRecord_CtrlTimerCallBackFunc);
    AK_DEBUG_OUTPUT("start vor timer id=%x\n",pAudioRecord->vorRecTimer);
}
#pragma arm section code 
#if 0
/************************************************************************
 * @BRIEF Get Dummy frame count
 * @AUTHOR hxq
 * @DATE  2011-12-14
 * @PARAM T_eREC_MODE 
 * @RETURN T_U32
 * @remark: now only deal 2 types record mode:eREC_MODE_WAV8K&eREC_MODE_WAV8K
 **************************************************************************/
static T_U32 AudioRecord_Get_Dummy_Cnt(T_eREC_MODE recMode)
{
    T_U32 ret = 0;
    switch(recMode)
    {
        case eREC_MODE_WAV8K :
            ret = 3;
            break;
        case eREC_MODE_WAV16K: //优质录音
            ret = 6;
            break;
        default:
            ret = 3;
            break;
    }
    return ret;
}
#endif
/************************************************************************
 * @BRIEF create a file and start to record
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_U16 **pRecFileName: the name of file which is created
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully start to record
 * @RETVAL AK_FALSE:start to record failed
 **************************************************************************/
T_BOOL AudioRecord_Start(const T_U16 *pRecName, T_eREC_MODE RecMode ,T_S32 volLarger, T_BOOL isVorRec)
{
    T_U32 nSampleRate;
    T_U16 nChannel;
    T_U16 BitsPerSample;
    T_WAVE_IN Wav_IO;    
    T_U32 bufsize = 0;    
    T_U32 count = 0;
    T_U8* pDataBuf;
#ifndef RECORD_PCM_ONLY
    const T_U8 amrHeader[]= "#!AMR\n";
#endif
    T_U32 size = 0;
    T_U64_INT   freesize = {0};

    AudioRecord_CalcFreeSize(pRecName[0], &freesize);
    if (freesize.high == 0 && freesize.low <=  REC_RESERVE_SPACE)
    {
       AK_DEBUG_OUTPUT("space less than 16M,record exit\n");
       return AK_FALSE;
    }
    
	AudioRecord_ComputeTotalTime(freesize, RecMode);

    AK_DEBUG_OUTPUT("AudioRecord_Start!!!!\n");
    AK_ASSERT_PTR(pAudioRecord, AK_NULL, AK_FALSE);

    pAudioRecord->recMode = RecMode;
    //whether is voice ctrl record
    if (isVorRec)
    {
        if (eSTAT_VORREC_CLOSE == pAudioRecord->VorCtrlState)
        {
            pAudioRecord->VorCtrlState = eSTAT_VORREC_PAUSE;
        }
    }
    else
    {
        pAudioRecord->VorCtrlState = eSTAT_VORREC_CLOSE;
    }

    if (!pAudioRecord->bVideoRec)            //if normal record open record file
    {
#ifdef OS_ANYKA
        // 屏蔽 sd插拔
//      Fwl_SD_Enable_Mark_Detect(AK_TRUE);
#endif
        
        pAudioRecord->rec_fd = Fwl_FileOpen(pRecName, _FMODE_CREATE, _FMODE_CREATE);
        if (_FOPEN_FAIL == pAudioRecord->rec_fd)
        {
            Printf_UC((T_U16 *)pRecName);
            return AK_FALSE;
        }
    }
    else
    {
        size = sizeof(AUDIOREC_STRUCT);
        if ((pAudioRecord->mallocSize) >= (SINGLE_OUT_BUFFER_SIZE+size))
        {
            size = pAudioRecord->mallocSize - SINGLE_OUT_BUFFER_SIZE;
            pAudioRecord->OutBuf = (((T_U8 *)pAudioRecord) + size);
            pAudioRecord->bufsize = SINGLE_OUT_BUFFER_SIZE;
            pAudioRecord->isBufMalloc = AK_FALSE;
        }
        else
        {
            AK_DEBUG_OUTPUT("AUDIOREC_STRUCT is over the half of page size:%d !\n", size);
        }
    }
    
    // normal record 
    if (AK_NULL == pAudioRecord->OutBuf)
    {
		if(pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
			|| pAudioRecord->recMode == eREC_MODE_SPEEX_HI)
		{
	    	pAudioRecord->bufsize = OUT_BUFFER_SIZE<<1;  //8K
		}
		else
		{
			pAudioRecord->bufsize = OUT_BUFFER_SIZE;
		}
		
		pAudioRecord->OutBuf = (T_U8 *)Fwl_DMAMalloc(pAudioRecord->bufsize);
        if (AK_NULL == pAudioRecord->OutBuf)
        {
            AK_DEBUG_OUTPUT("No more memory to alloc for pAudioRecord OutBuf!!!!\n");
            return AK_FALSE;
	    }		
        pAudioRecord->isBufMalloc = AK_TRUE;
    }
    else
    {   
        //if buffer is 4K and recmode is ogg, remalloc 8K
        if (OUT_BUFFER_SIZE == pAudioRecord->bufsize    
            && (pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
				|| pAudioRecord->recMode == eREC_MODE_SPEEX_HI)) 
        {
            Fwl_DMAFree(pAudioRecord->OutBuf);            
            pAudioRecord->bufsize = OUT_BUFFER_SIZE<<1;  //8K
            
            pAudioRecord->OutBuf = (T_U8 *)Fwl_DMAMalloc(pAudioRecord->bufsize);
            if (AK_NULL == pAudioRecord->OutBuf)
            {
                AK_DEBUG_OUTPUT("No more memory to Realloc for pAudioRecord Ogg OutBuf!!!!\n");
                return AK_FALSE;
    	    }		
            pAudioRecord->isBufMalloc = AK_TRUE;
        }

        //if buffer is 8K and recmode not ogg, remalloc 4K
        if ((OUT_BUFFER_SIZE<<1) == pAudioRecord->bufsize    
            && !(pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
				|| pAudioRecord->recMode == eREC_MODE_SPEEX_HI))
        {
            Fwl_DMAFree(pAudioRecord->OutBuf);            
            pAudioRecord->bufsize = OUT_BUFFER_SIZE;
            
            pAudioRecord->OutBuf = (T_U8 *)Fwl_DMAMalloc(pAudioRecord->bufsize);
            if (AK_NULL == pAudioRecord->OutBuf)
            {
                AK_DEBUG_OUTPUT("No more memory to Realloc for pAudioRecord OutBuf!!!!\n");
                return AK_FALSE;
            }       
            pAudioRecord->isBufMalloc = AK_TRUE;
        }
    }
    
//    Fwl_AD_Open();
//    Fwl_DelayMs(200);
    if (eSTAT_RCV_MIC == pAudioRecord->revState)
    {
        Wav_IO = MIC_ADC;
    }
    else
    {
        Wav_IO = LINEIN_ADC;
    }
    
    pAudioRecord->wavin_id = Fwl_WaveInOpen(Wav_IO, AudioRecord_InterruptHandle, AK_TRUE);
//  Fwl_DelayMs(200);
    AudioRecord_SetGain(pAudioRecord->GainLevel);

    nSampleRate = SAMPLERATE_WAV8K;
    nChannel = CHANNEL_NUM_1;
    BitsPerSample = BITSPERSAMPLE_16;    

    //pAudioRecord->audio_record_enc.in_info.enc_mode = 8;
    
    if ((pAudioRecord->recMode != eREC_MODE_WAV8K) 
        && (pAudioRecord->recMode != eREC_MODE_WAV16K)
        && (pAudioRecord->recMode != eREC_MODE_WAV48K)
        && (pAudioRecord->recMode != eREC_MODE_WAV48K_2))
    {   //ADPCM encode info
        #ifndef RECORD_PCM_ONLY
        pAudioRecord->audio_record_enc.in_info.pcmBitsPerSample = BITSPERSAMPLE_16; 
        pAudioRecord->audio_record_enc.in_info.pcmChannel = CHANNEL_NUM_1;
        pAudioRecord->audio_record_enc.in_info.pcmSampleRate = SAMPLERATE_WAV8K;
        if(pAudioRecord->recMode == eREC_MODE_MP3LOW 
            || pAudioRecord->recMode == eREC_MODE_MP3HI)
        {
             pAudioRecord->audio_record_enc.in_info.e_Type = _SD_MEDIA_TYPE_MP3;
        }
        else if(pAudioRecord->recMode == eREC_MODE_AMR)
        {
            pAudioRecord->audio_record_enc.in_info.e_Type= _SD_MEDIA_TYPE_AMR;
        }
        else if(pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
                || pAudioRecord->recMode == eREC_MODE_SPEEX_HI)
        {
            pAudioRecord->audio_record_enc.in_info.e_Type = _SD_MEDIA_TYPE_SPEEX;
        }
        else
        {
             pAudioRecord->audio_record_enc.in_info.e_Type = 7;// _SD_MEDIA_TYPE_ADPCM_IMA
        }
        pAudioRecord->audio_record_enc.in_info.e_nChannel = CHANNEL_NUM_1;
        pAudioRecord->audio_record_enc.in_info.e_BitsPerSample = BITSPERSAMPLE_4;
        pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_WAV8K;
        pAudioRecord->audio_record_enc.in_info.e_Bitrate = BITRATE_0;
        pAudioRecord->audio_record_enc.in_info.cbr = AK_FALSE;
        pAudioRecord->audio_record_enc.in_info.dtx_disable = AK_FALSE;
      
        pAudioRecord->audio_record_enc.in_info.e_nVolume= volLarger;    
                                                                                                                                                                                                                                                                                                                                               ;
        switch(pAudioRecord->recMode)
        {
        case eREC_MODE_AMR:
        case eREC_MODE_ADPCM8K:
        case eREC_MODE_ADPCM8K_RADIO:
            break;
        case eREC_MODE_ADPCM8K_2:
            nChannel = CHANNEL_NUM_ADPCM8K_2;
            pAudioRecord->audio_record_enc.in_info.pcmChannel = CHANNEL_NUM_ADPCM8K_2;
            pAudioRecord->audio_record_enc.in_info.e_nChannel = CHANNEL_NUM_ADPCM8K_2;
            pAudioRecord->audio_record_enc.in_info.e_Bitrate = BITRATE_ADPCM8K_HI;
            break;
        case eREC_MODE_ADPCM16K:
            nSampleRate = SAMPLERATE_ADPCM16K;
            pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_ADPCM16K;
            break;
        case eREC_MODE_ADPCM48K:
            nSampleRate = SAMPLERATE_ADPCM48K;//48000;
            pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_ADPCM48K;//48000;
            break;
        case eREC_MODE_ADPCM48K_2:
            nChannel = CHANNEL_NUM_ADPCM48K_2;
            nSampleRate = SAMPLERATE_ADPCM48K_2;//48000;
            pAudioRecord->audio_record_enc.in_info.pcmChannel = CHANNEL_NUM_ADPCM48K_2;
            pAudioRecord->audio_record_enc.in_info.e_nChannel = CHANNEL_NUM_ADPCM48K_2;
            pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_ADPCM48K_2;//48000;
            break;
        case eREC_MODE_MP3LOW:
            nChannel = CHANNEL_NUM_MP3LOW;
            nSampleRate = SAMPLERATE_MP3LOW;//48000;
            pAudioRecord->audio_record_enc.in_info.pcmChannel = CHANNEL_NUM_MP3LOW;
            pAudioRecord->audio_record_enc.in_info.e_nChannel = CHANNEL_NUM_MP3LOW;
            pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_MP3LOW;//48000;
            pAudioRecord->audio_record_enc.in_info.e_Bitrate  = BITRATE_MP3_LOW;
            break;

        case eREC_MODE_MP3HI:
            nChannel = CHANNEL_NUM_MP3HI;
            nSampleRate = SAMPLERATE_MP3HI;//48000;
            pAudioRecord->audio_record_enc.in_info.pcmChannel = CHANNEL_NUM_MP3HI;
            pAudioRecord->audio_record_enc.in_info.e_nChannel = CHANNEL_NUM_MP3HI;
            pAudioRecord->audio_record_enc.in_info.e_nSampleRate = SAMPLERATE_MP3HI;//48000;
            pAudioRecord->audio_record_enc.in_info.e_Bitrate  = BITRATE_MP3_HI;
            break;
            
        case eREC_MODE_SPEEX_LOW:
            pAudioRecord->audio_record_enc.in_info.e_BitsPerSample = BITSPERSAMPLE_16;  
            pAudioRecord->audio_record_enc.in_info.e_Bitrate  = BITRATE_SPEEX_LOW;
            pAudioRecord->audio_record_enc.in_info.cbr = AK_TRUE;
            pAudioRecord->audio_record_enc.in_info.dtx_disable = AK_TRUE;
            break;
        case eREC_MODE_SPEEX_HI:
            pAudioRecord->audio_record_enc.in_info.e_BitsPerSample = BITSPERSAMPLE_16; 
            pAudioRecord->audio_record_enc.in_info.e_Bitrate  = BITRATE_SPEEX_HI;
            pAudioRecord->audio_record_enc.in_info.cbr = AK_TRUE;
            pAudioRecord->audio_record_enc.in_info.dtx_disable = AK_TRUE;
            break;
        }
        
        pAudioRecord->audio_record_enc.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
        pAudioRecord->audio_record_enc.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
        #ifdef DEBUG
        pAudioRecord->audio_record_enc.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)AK_DEBUG_OUTPUT;
        #else
        pAudioRecord->audio_record_enc.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)AK_NULL;
        #endif
        pAudioRecord->hMedia = MediaLib_Encode_Open(&pAudioRecord->audio_record_enc);
        if (AK_NULL == pAudioRecord->hMedia)
        {
            if(!pAudioRecord->bVideoRec)
            {
                Fwl_FileClose(pAudioRecord->rec_fd);
                pAudioRecord->rec_fd = FS_INVALID_HANDLE;
            }

            AK_DEBUG_OUTPUT("record init error\r\n");
            return AK_FALSE;
        }
        #else
        AK_DEBUG_OUTPUT("only support pcm recording\n");
        return AK_FALSE;
        #endif
    }
    else if(eREC_MODE_WAV16K==pAudioRecord->recMode)
    {
        nSampleRate = SAMPLERATE_WAV16K;
    }
    else if(eREC_MODE_WAV48K == pAudioRecord->recMode)
    {
        nChannel= CHANNEL_NUM_WAV48K;
        nSampleRate= SAMPLERATE_WAV48K;
    }
	else if(eREC_MODE_WAV48K_2 == pAudioRecord->recMode)
    {
        nChannel= CHANNEL_NUM_WAV48K_2;
        nSampleRate= SAMPLERATE_WAV48K_2;
    }
//    Fwl_AD_SetInfo(&pcmInfo);



    if ((pAudioRecord->recMode == eREC_MODE_WAV8K)
        ||(pAudioRecord->recMode == eREC_MODE_WAV16K)
        ||(eREC_MODE_WAV48K == pAudioRecord->recMode)
        ||(eREC_MODE_WAV48K_2 == pAudioRecord->recMode))
    {
        if (!pAudioRecord->bVideoRec)
        {
            Fwl_FileWrite(pAudioRecord->rec_fd, &fileHeadPcm, sizeof(fileHeadPcm));
        }
        pAudioRecord->bCoding = AK_FALSE;
    }
    #ifndef RECORD_PCM_ONLY
    else if(pAudioRecord->recMode == eREC_MODE_MP3LOW 
            || pAudioRecord->recMode == eREC_MODE_MP3HI)
    {
        pAudioRecord->bCoding = AK_TRUE;
    }
    else if(pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
            || pAudioRecord->recMode == eREC_MODE_SPEEX_HI)
    {
        pAudioRecord->bCoding = AK_TRUE;
    }
    else if(pAudioRecord->recMode == eREC_MODE_AMR)
    {
        if (!pAudioRecord->bVideoRec)
        {
            Fwl_FileWrite(pAudioRecord->rec_fd, amrHeader, 6);
        }
        pAudioRecord->bCoding = AK_TRUE;
    }
    else
    {
        if (!pAudioRecord->bVideoRec)
        {
             Fwl_FileWrite(pAudioRecord->rec_fd, &fileHead, sizeof(fileHead));
        }
        pAudioRecord->bCoding = AK_TRUE;
    }
    #endif
    
    switch(pAudioRecord->recMode)
    {            
        case eREC_MODE_WAV8K:  //优质录音          
        case eREC_MODE_WAV16K: //优质录音
            pAudioRecord->bufCount = 11;
            break;
            
        case eREC_MODE_WAV48K:
		case eREC_MODE_WAV48K_2:	
            pAudioRecord->bufCount = 12;
            break;
            
        default:
            pAudioRecord->bufCount = 4;
            break;
    }

    if (pAudioRecord->bufCount > REC_BUF_COUNT)
    {
        pAudioRecord->bufCount = REC_BUF_COUNT;
    }

    if (pAudioRecord->bufCount < 2)
    {
        pAudioRecord->bufCount = 2;
    }
    
    pAudioRecord->maxLeftCount = 1;
    if (pAudioRecord->maxLeftCount > pAudioRecord->bufCount)
    {
        pAudioRecord->maxLeftCount = 1;
    }

/*
    for(i= 0; i< pAudioRecord->bufCount; i++)
    {
        pAudioRecord->pRecBufs[i]= (T_U8*)Fwl_DMAMalloc(IN_BUFFER_SIZE);
        if(AK_NULL == pAudioRecord->pRecBufs[i])
        {
            AK_DEBUG_OUTPUT("Record Malloc Failed:%d", i);
            for(j= 0; j< i; j++)
            {
                if(pAudioRecord->pRecBufs[j] != AK_NULL)
                {
                    pAudioRecord->pRecBufs[j] = Fwl_DMAFree(pAudioRecord->pRecBufs[j]);             
                }
            }
            if(!pAudioRecord->bVideoRec)
            {
                Fwl_FileClose(pAudioRecord->rec_fd);
                pAudioRecord->rec_fd = FS_INVALID_HANDLE;
            }
            #ifndef RECORD_PCM_ONLY
            if(pAudioRecord->hMedia!=AK_NULL)
            {
                MediaLib_Encode_Close(pAudioRecord->hMedia);
                pAudioRecord->hMedia= AK_NULL;
            }
            #endif
            return AK_FALSE;
        }
    } 
*/
    if (!cycbuf_create(CYC_AD_BUF_ID, pAudioRecord->bufCount<<12))
    {
        if (!pAudioRecord->bVideoRec)
        {
            Fwl_FileClose(pAudioRecord->rec_fd);
            pAudioRecord->rec_fd = FS_INVALID_HANDLE;
        }
        #ifndef RECORD_PCM_ONLY
        if (pAudioRecord->hMedia!=AK_NULL)
        {
            MediaLib_Encode_Close(pAudioRecord->hMedia);
            pAudioRecord->hMedia= AK_NULL;
        }
        #endif
        return AK_FALSE;    
    }

    
   AK_DEBUG_OUTPUT("Rec BufCount:%d maxLeftCount:%d\n", pAudioRecord->bufCount, pAudioRecord->maxLeftCount);
   AudioRecord_UpdateAllFlags();
   //设置dma中断回调

   pAudioRecord->RecState = eSTAT_RECORDING;   //设为ak_false 保证l2中断可以启动

   pAudioRecord->send_len = 0;
   pAudioRecord->UpdateTime = 0;
   pAudioRecord->channels = nChannel;
   pAudioRecord->RecSampleRate = nSampleRate;
   pAudioRecord->RecBitsPerSample = BitsPerSample;
   
   if (!Fwl_WaveInStart(pAudioRecord->wavin_id, nSampleRate, nChannel, BitsPerSample))
   {
        AK_DEBUG_OUTPUT("start record interruput false!\n");
        return AK_FALSE;
   }
   
   //丢弃前500毫秒的数据，等ADC稳定后再录，避免PIPA声
   while(500 >= AudioRecord_GetCurrentTime())
   {
        bufsize = cycbuf_getdatabuf(CYC_AD_BUF_ID, &pDataBuf, IN_BUFFER_SIZE);
        cycbuf_read_updateflag(CYC_AD_BUF_ID, bufsize);

        if (0 != bufsize)
        {
            pAudioRecord->ADCounter++;
        }        
        else
        {
            if (10 < count++)
            {
                break;
            }
            Fwl_DelayUs(50000);
        }
   }
   pAudioRecord->ADCounter = 0;
   pAudioRecord->currentRecTime =0;

   return AK_TRUE;
}

/************************************************************************
 * @BRIEF stop recording
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL AK_TRUE:Successfully stop recording
 * @RETVAL AK_FALSE:stop recording failed
 **************************************************************************/
T_BOOL AudioRecord_Stop(T_VOID)
{    
    AK_DEBUG_OUTPUT("AudioRecord_Stop!!!!\n");
    AK_ASSERT_PTR(pAudioRecord, AK_NULL, AK_FALSE);
    pAudioRecord->RecState = eSTAT_REC_STOP;
    AudioRecord_UpdateAllFlags();
    
    AK_DEBUG_OUTPUT("stop send.\n");
//    Fwl_DMA_Ability_Enable(FWL_DMA_ADC, AK_FALSE);
    Fwl_WaveInStop(pAudioRecord->wavin_id);
    Fwl_WaveInClose(pAudioRecord->wavin_id);
    pAudioRecord->wavin_id = 0xff;
    
    #ifndef RECORD_PCM_ONLY
    if(pAudioRecord->hMedia!=AK_NULL)
    {
	    if(pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
			|| pAudioRecord->recMode == eREC_MODE_SPEEX_HI)
		{
			T_MEDIALIB_ENC_BUF_STRC enc_buf_strc;
			T_U32 rec_len = 0;
			
			enc_buf_strc.buf_in = AK_NULL;
			enc_buf_strc.len_in = 0;
			enc_buf_strc.buf_out = pAudioRecord->OutBuf;		
			enc_buf_strc.len_out = pAudioRecord->bufsize;

			rec_len = MediaLib_Encode_Last(pAudioRecord->hMedia, &enc_buf_strc);
			Fwl_FileWrite(pAudioRecord->rec_fd, pAudioRecord->OutBuf, rec_len);
		}
		
        MediaLib_Encode_Close(pAudioRecord->hMedia);
    }
    #endif
    
    AudioRecord_UpdateHead();
    if(!pAudioRecord->bVideoRec)
    {
        if (AK_FALSE == Fwl_FileClose(pAudioRecord->rec_fd))
        {
            AK_DEBUG_OUTPUT("REC: Error Can not close file. \n");
            //return AK_FALSE;
        }
        pAudioRecord->rec_fd = FS_INVALID_HANDLE; 
#ifdef OS_ANYKA
        // 关闭屏蔽 sd mount
//      Fwl_SD_Enable_Mark_Detect(AK_FALSE);
#endif
    }
    //pAudioRecord->ADCounter=0;//start时会清零。
    //rec_buf_flag = 0;
    pAudioRecord->rec_len = 0;
    pAudioRecord->total_len = 0;
    //pAudioRecord->currentRecTime = 0;//start时会清零。
    pAudioRecord->UpdateTime = 0;
    pAudioRecord->VorCtlValue = VORREC_CTRLVALUE;
    pAudioRecord->VorCtrlState = eSTAT_VORREC_CLOSE;
    pAudioRecord->hMedia = AK_NULL;
    //////////////////////for video
    pAudioRecord->bVideoRec = AK_FALSE;
    if(pAudioRecord->vorRecTimer != ERROR_TIMER)
    {
        Fwl_TimerStop(pAudioRecord->vorRecTimer);
        pAudioRecord->vorRecTimer =ERROR_TIMER;
    }


    cycbuf_destory(CYC_AD_BUF_ID, pAudioRecord->bufCount<<12);
/*  
    for(i= 0; i< pAudioRecord->bufCount; i++)
    {
        if(pAudioRecord->pRecBufs[i] != AK_NULL)
        {
            pAudioRecord->pRecBufs[i] = Fwl_DMAFree(pAudioRecord->pRecBufs[i]);             
        }
    }
    */
    return AK_TRUE;
}

/************************************************************************
 * @BRIEF destory the resource which  was malloce fbr recording
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
T_VOID AudioRecord_Destroy(T_VOID)
{
    AK_DEBUG_OUTPUT("AudioRecord_Destroy!!!!\n");
    //AudioRecord_Print("AudioRecord_Destroy1");
    if (AK_NULL != pAudioRecord)
    {
    /*
#if(USE_REC_DEBUG)
        if(pAudioRecord->pRecBufs[0] != AK_NULL)
            AK_DEBUG_OUTPUT("**Not Free RecBufs\n");
#endif
*/
        if (AK_TRUE == pAudioRecord->isBufMalloc)
        {
            Fwl_DMAFree(pAudioRecord->OutBuf);
        }
        pAudioRecord = Fwl_DMAFree(pAudioRecord);
    }
    
    //AudioRecord_Print("AudioRecord_Destroy4");
}


T_VOID AudioRecord_SetGain(GAIN_LEVEL gain)
{
    if (AK_NULL != pAudioRecord)
    {
        pAudioRecord->GainLevel = gain;
        if (pAudioRecord->wavin_id != 0xff)
        {
            Fwl_WaveInSetGain(pAudioRecord->wavin_id, pAudioRecord->GainLevel);
        }
    }
}


GAIN_LEVEL AudioRecord_GetGain(T_VOID)
{
    if (AK_NULL != pAudioRecord)
    {
        return (pAudioRecord->GainLevel);
    }
    return GAIN_MAX;
}


/************************************************************************
 * @BRIEF get the length of data it can save
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL the time it can record (s)
 * @note:
 * 1.(误差系数)miss distance value:因为采样率不准确,所以需要用误差系数
 *  误差系数计算方法：用实际采样率计算出一个中断到来的时间间隔除上理论
 *  的时间间隔，比如：AD缓冲区为2k bytes,16k采样，16bit,单声道 pcm,一个
 *  中断实际的时间间隔为2*1024*8/(16447*16*2)(注意：16447是根据芯片spec算
 *  得,并且是用的双声道采样来计算的),理论上的时间间隔是2*1024/(16000*2*2)
 *  两个的比值就是误差系数
 **************************************************************************/
static T_U32 AudioRecord_ComputeTotalTime(T_U64_INT freesize,T_eREC_MODE recMode)
{
    if ((freesize.high == 0) && (freesize.low < REC_RESERVE_SPACE))
    {
        freesize.low = 0;
    }
    else
    {
        if (freesize.low >= REC_RESERVE_SPACE)
        {
            freesize.low -= REC_RESERVE_SPACE;
        }
        else
        {
            freesize.high--;
            freesize.low = (0xffffffff - REC_RESERVE_SPACE) + freesize.low;
        }
    }

    AK_DEBUG_OUTPUT("freesize.low=0x%x\n",freesize.low);
    AK_DEBUG_OUTPUT("freesize.high=0x%x\n",freesize.high);
    AK_DEBUG_OUTPUT("**recMode:%d\n", recMode);
    
    switch (recMode)
    {
        case eREC_MODE_WAV8K:
            pAudioRecord->totalRecTime = \
                (T_U32)(freesize.low/(2*SAMPLERATE_WAV8K*CHANNEL_NUM_WAV8K)/SAMPLE_8K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                   (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_WAV8K*CHANNEL_NUM_WAV8K)*freesize.high/SAMPLE_8K_RATE);
            }
            break;
        case eREC_MODE_WAV16K:
            pAudioRecord->totalRecTime = \
                 (T_U32)(freesize.low/(2*SAMPLERATE_WAV16K*CHANNEL_NUM_WAV16K)/SAMPLE_16K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                   (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_WAV16K*CHANNEL_NUM_WAV16K)*freesize.high/SAMPLE_16K_RATE);
            }
            break;
        case eREC_MODE_WAV48K:
            pAudioRecord->totalRecTime = \
                 (T_U32)(freesize.low/(2*SAMPLERATE_WAV48K*CHANNEL_NUM_WAV48K)/SAMPLE_48K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                    (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_WAV48K*CHANNEL_NUM_WAV48K)*freesize.high/SAMPLE_48K_RATE);
            }
            break;

		case eREC_MODE_WAV48K_2:
            pAudioRecord->totalRecTime = \
                 (T_U32)(freesize.low/(2*SAMPLERATE_WAV48K_2*CHANNEL_NUM_WAV48K_2)/SAMPLE_48K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                    (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_WAV48K_2*CHANNEL_NUM_WAV48K_2)*freesize.high/SAMPLE_48K_RATE);
            }
            break;

        #ifndef RECORD_PCM_ONLY
        case eREC_MODE_ADPCM16K:
            pAudioRecord->totalRecTime = \
			    (T_U32)(freesize.low/(2*SAMPLERATE_ADPCM16K*CHANNEL_NUM_ADPCM16K)/SAMPLE_16K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                   (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_ADPCM16K*CHANNEL_NUM_ADPCM16K)*freesize.high/SAMPLE_16K_RATE);
            }
            break;
        case eREC_MODE_ADPCM8K_2:
            pAudioRecord->totalRecTime = \
			    (T_U32)(freesize.low/(2*SAMPLERATE_ADPCM8K_2*CHANNEL_NUM_ADPCM8K_2)/SAMPLE_8K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                   (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_ADPCM8K_2*CHANNEL_NUM_ADPCM8K_2)*freesize.high/SAMPLE_8K_RATE);
            }
            break;
        case eREC_MODE_ADPCM8K:
        case eREC_MODE_ADPCM8K_RADIO:
            pAudioRecord->totalRecTime = \
			    (T_U32)(freesize.low/(2*SAMPLERATE_ADPCM8K*CHANNEL_NUM_ADPCM8K)/SAMPLE_8K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                   (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_ADPCM8K*CHANNEL_NUM_ADPCM8K)*freesize.high/SAMPLE_8K_RATE);
            }
            break;
        
        case eREC_MODE_ADPCM48K:
            pAudioRecord->totalRecTime = \
                 (T_U32)(freesize.low/(2*SAMPLERATE_ADPCM48K*CHANNEL_NUM_ADPCM48K)/SAMPLE_48K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                    (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_ADPCM48K*CHANNEL_NUM_ADPCM48K)*freesize.high/SAMPLE_48K_RATE);
            }
            break;
        case eREC_MODE_ADPCM48K_2:
            pAudioRecord->totalRecTime = \
                 (T_U32)(freesize.low/(2*SAMPLERATE_ADPCM48K_2*CHANNEL_NUM_ADPCM48K_2)/SAMPLE_48K_RATE);
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += \
                    (T_U32)((T_U32)0xffffffff/(2*SAMPLERATE_ADPCM48K_2*CHANNEL_NUM_ADPCM48K_2)*freesize.high/SAMPLE_48K_RATE);
            }
            break;
        case eREC_MODE_MP3LOW:
            pAudioRecord->totalRecTime = (T_U32)(freesize.low/(MP3_8K_PS));
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += (T_U32)((T_U32)0xffffffff/(MP3_8K_PS)*freesize.high);
            }
            break;
        case eREC_MODE_MP3HI:
            pAudioRecord->totalRecTime = (T_U32)(freesize.low/(MP3_22K5_PS));
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += (T_U32)((T_U32)0xffffffff/(MP3_22K5_PS)*freesize.high);
            }
            break;
        case eREC_MODE_AMR:
            pAudioRecord->totalRecTime = (T_U32)(freesize.low/(AMR_8K_PS));
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += (T_U32)((T_U32)0xffffffff/(AMR_8K_PS)*freesize.high);
            }
            break;
        case eREC_MODE_SPEEX_LOW:
            pAudioRecord->totalRecTime = (T_U32)(freesize.low/(MP3_8K_PS));
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += (T_U32)((T_U32)0xffffffff/(MP3_8K_PS)*freesize.high);
            }
            break;
        case eREC_MODE_SPEEX_HI:
            pAudioRecord->totalRecTime = (T_U32)(freesize.low/(MP3_8K_PS));
            if (freesize.high > 0)
            {
                pAudioRecord->totalRecTime += (T_U32)((T_U32)0xffffffff/(MP3_8K_PS)*freesize.high);
            }
            break;
        #endif
        default:
            pAudioRecord->totalRecTime = 0;
            break;
    }
    AK_DEBUG_OUTPUT("totalRecTime= %d s.\n", pAudioRecord->totalRecTime);

    return pAudioRecord->totalRecTime;
}

T_U32 AudioRecord_GetTotalTime(T_U16 DrvIndx, T_eREC_MODE recMode)
{
    T_U64_INT freesize = {0};
    AudioRecord_CalcFreeSize(DrvIndx, &freesize);
    pAudioRecord->freeSize.low = freesize.low;
    pAudioRecord->freeSize.high = freesize.high;
    
    return AudioRecord_ComputeTotalTime(freesize,recMode);
}

/************************************************************************
 * @BRIEF destory the resource which  was malloce fbr recording
 * @AUTHOR hxq
 * @DATE  2012-01-30
 * @PARAM input:T_U16 DrvIndx:存储设备索引号;
 *               output:T_U64_INT freesize,存储设备空闲空间.
 * @RETURN T_VOID
 **************************************************************************/

T_VOID AudioRecord_CalcFreeSize(T_U16 DrvIndx, T_U64_INT *freesize)
{
    T_U8 DriverID;

    if(DrvIndx < 'Z')
    {
        DriverID = DrvIndx - 'A';
    }
    else
    {
        DriverID = DrvIndx - 'a';
    }
    Fwl_FsGetFreeSize(DriverID, freesize);

}

static T_VOID AudioRecord_CtrlTimerCallBackFunc(T_TIMER timer_id, T_U32 delay)
{   
    pAudioRecord->vorRecTimer = Fwl_TimerStop(pAudioRecord->vorRecTimer);
}


#pragma arm section code = "_bootcode1_"
/************************************************************************
 * @BRIEF select a empty buffer to recevie record data 
 * and update the record state
 * @author zhao_xiaowei
 * @date 2009-12
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL 
 **************************************************************************/
T_VOID AudioRecord_InterruptHandle(T_U8 **buf, T_U32 *len)
{
    if(pAudioRecord==AK_NULL)
        return ;

    if (eSTAT_REC_PAUSE == pAudioRecord->RecState)
    {
       // rec_buf_flag = 0;
       	pAudioRecord->send_len = 0;
	    pAudioRecord->recBufState = eREC_FULL;
	    *buf = AK_NULL;
	    *len = 0;
        return ;
    }
	
    Fwl_ConsoleWriteChr('<');

    //更新上一次的发送数据
    cycbuf_write_updateflag(CYC_AD_BUF_ID, pAudioRecord->send_len);
    pAudioRecord->send_len = 0;
    if (pAudioRecord->recBufState != eREC_FULL)
    {
        //如果IN_BUFFER_SIZE 不是4096，则bufsize可能就不是IN_BUFFER_SIZE这么大；那样会对录音的时间计算有影响；
        pAudioRecord->send_len = cycbuf_getwritebuf(CYC_AD_BUF_ID, buf, IN_BUFFER_SIZE);
        
        pAudioRecord->recBufState= eREC_NORMAL;//when it finish one data recive ,it's in Normal state
        if(pAudioRecord->send_len)
        {
            *len = pAudioRecord->send_len;
        }
        else
        {
            pAudioRecord->recBufState = eREC_FULL;
            *buf = AK_NULL;
            *len = 0;
        }
    }
}
#pragma arm section code
/************************************************************************
 * @BRIEF encode and save the index buffer data into file
 * @AUTHOR Zhao_Xiaowei
 * @DATE  2008-12-26
 * @PARAM savBufIndex the index buffer data 
 * @RETURN T_BOOL
 * @RETVAL Successfully save the data into file
 * @RETVAL save data failed
 **************************************************************************/
#pragma arm section code = "_recording_"

T_BOOL AudioRecord_SaveDataOnce(T_VOID)
{
    T_U32 bufsize = 0, writeByteLen = 0;
    T_U8* pDataBuf;
    T_EVT_PARAM EventParm; 

    if (eSTAT_REC_PAUSE == pAudioRecord->RecState)
    {
       // rec_buf_flag = 0;
        return AK_FALSE;
    }

    if (FS_INVALID_HANDLE != pAudioRecord->rec_fd)
    { 
        bufsize = cycbuf_getdatabuf(CYC_AD_BUF_ID, &pDataBuf, IN_BUFFER_SIZE);

        cycbuf_read_updateflag(CYC_AD_BUF_ID, bufsize);

        if (!bufsize)
        {
            return AK_FALSE;
        }

        
        if ((eSTAT_VORREC_PAUSE != pAudioRecord->VorCtrlState)
            && (eSTAT_RECORDING == pAudioRecord->RecState))
        {
            pAudioRecord->ADCounter++;
        }
    }
    else
    {
        return AK_FALSE;
    }
    
    AudioRecord_GetCurrentTime();

    if (pAudioRecord->VorCtrlState == eSTAT_VORREC_RECORDING
        || pAudioRecord->VorCtrlState == eSTAT_VORREC_PAUSE)
    {            
        T_BOOL bVorValid = AK_FALSE;
#ifndef RECORD_PCM_ONLY
        //接口有变，第二个参数不是传位进去，改为传字节数进去
        bVorValid = (T_BOOL)MediaLib_CheckRecData(pDataBuf, IN_BUFFER_SIZE>>3, 
                                                    pAudioRecord->VorCtlValue);
#endif
        if (!pAudioRecord->Vorcallback(bVorValid))
            return AK_FALSE;
    }


    AudioRecord_DealData(pDataBuf, bufsize);

    writeByteLen = Fwl_FileWrite(pAudioRecord->rec_fd, pAudioRecord->OutBuf, pAudioRecord->rec_len);
    pAudioRecord->total_len += writeByteLen;    

    if ((writeByteLen!=pAudioRecord->rec_len)||pAudioRecord->total_len >= SINGLE_FILE_LEN_LIMIT \
        || (pAudioRecord->currentRecTime >= pAudioRecord->totalRecTime*1000)\
        ||(pAudioRecord->freeSize.high == 0 && pAudioRecord->freeSize.low <= (pAudioRecord->total_len + REC_RESERVE_SPACE))
        || REC_TIME_LIMIT < pAudioRecord->currentRecTime)
    {
    #if (OS_ANYKA)
        AK_PRINTK("writelen=", writeByteLen, AK_TRUE);
        AK_PRINTK("reclen=", pAudioRecord->rec_len, AK_TRUE);
        AK_PRINTK("curtime=", pAudioRecord->currentRecTime, AK_TRUE);
        AK_PRINTK("total=", pAudioRecord->totalRecTime,AK_TRUE);                
    #endif                    
        //rec_buf_flag = 0;
        VME_EvtQueueClearTimerEvent();
        EventParm.c.Param1 = CTRL_EVENT_MEMORY_FULL;
        if(SINGLE_FILE_LEN_LIMIT <= pAudioRecord->total_len
            || REC_TIME_LIMIT < pAudioRecord->currentRecTime)
        {
            EventParm.c.Param1 = CTRL_EVENT_SINGLE_FILE_LEN_LIMIT;
        }
        VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
        return AK_FALSE;
    }

    //保存在SD卡
	if (0 < pAudioRecord->currentRecTime
		&& UPDATE_INTERVAL_30 <= (pAudioRecord->currentRecTime-
                                    pAudioRecord->UpdateTime))
    {
        AudioRecord_UpdateHead();
        pAudioRecord->UpdateTime = pAudioRecord->currentRecTime;
    }
	else if((UPDATE_INTERVAL_10 <= (pAudioRecord->currentRecTime-
                                    pAudioRecord->UpdateTime))
			&& SYSTEM_STORAGE == Rec_GetCurDriver())
    {       
        AudioRecord_UpdateHead();
        pAudioRecord->UpdateTime = pAudioRecord->currentRecTime;
    }

    pAudioRecord->rec_len = 0;  

    return AK_TRUE;
}

/*
///////////////////////lsk change param
T_BOOL AudioRecord_SaveIndexData(T_U32 savBufIndex)
{
    T_U32 writeByteLen = 0, bufsize = 0;
    T_EVT_PARAM EventParm;  
    T_U8* pDataBuf;

    
    if (savBufIndex < pAudioRecord->bufCount)
    {
        //AK_DEBUG_OUTPUT("IN AudioRecord_SaveData\n");

        if (AudioRecord_IsInPauseStatue())
        {
           // rec_buf_flag = 0;
            return AK_FALSE;
        }
        //////////////////////lsk add condition for vedio record
        if (FS_INVALID_HANDLE != pAudioRecord->rec_fd)
        { 
//          T_U8* pDataBuf= pAudioRecord->pRecBufs[savBufIndex];
            bufsize = cycbuf_getdatabuf(&pDataBuf, IN_BUFFER_SIZE);
    
            //encode
            if(MMU_Vaddr2Paddr((T_U32)pDataBuf) == 0)
            {
                AK_PRINTK(" RE2:", 0, 1);
                while(1);
            }
            AudioRecord_GetCurrentTime();
            if(pAudioRecord->bVorRec)       ///////lsk add conditon for vedio record
            {//声控
                if(pAudioRecord->vorRecTimer==ERROR_TIMER)
                {
                    #ifndef RECORD_PCM_ONLY
                    //接口有变，第二个参数不是传位进去，改为传字节数进去
                    pAudioRecord->bVorValideData = (T_BOOL)MediaLib_CheckRecData(pDataBuf,bufsize>>3,VORREC_CTRLVALUE);
                    #endif  
                    //AK_DEBUG_OUTPUT("long bValideData=%d\n",pAudioRecord->bVorValideData);
                    if(!pAudioRecord->bVorValideData)
                    {
                        if(eSTAT_VORREC_RECORDING == pAudioRecord->VorCtrlState)
                        {
                            Fwl_UartWriteStr("long vor pause\n");
                            pAudioRecord->VorCtrlState = eSTAT_VORREC_PAUSE;
                            EventParm.c.Param1 = CTRL_EVENT_VORREC_PAUSE;
                            VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
                        }
                        //rec_buf_flag = 0;
                        return AK_FALSE;
                    }
                    else 
                    {
                        if(eSTAT_VORREC_PAUSE == pAudioRecord->VorCtrlState)
                        {
                            Fwl_UartWriteStr("long vor resume\n");
                            pAudioRecord->VorCtrlState = eSTAT_VORREC_RECORDING;
                            EventParm.c.Param1 = CTRL_EVENT_VORREC_RESUME;
                            VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
                            AudioRecord_CtrlTimerStart();
                        }
                        
                    }
                }
            }


            AudioRecord_DealData(pDataBuf, bufsize);
            
            writeByteLen = Fwl_FileWrite(pAudioRecord->rec_fd, pAudioRecord->OutBuf, pAudioRecord->rec_len);
            pAudioRecord->total_len += writeByteLen;

            if ((writeByteLen!=pAudioRecord->rec_len)||pAudioRecord->total_len >= SINGLE_FILE_LEN_LIMIT \
                || (pAudioRecord->currentRecTime>=pAudioRecord->totalRecTime*1000)\
                ||(pAudioRecord->freeSize.high == 0 && pAudioRecord->freeSize.low <= (pAudioRecord->total_len + REC_RESERVE_SPACE)))
            {
            #if (OS_ANYKA)
                AK_PRINTK("writelen=", writeByteLen, AK_TRUE);
                AK_PRINTK("reclen=", pAudioRecord->rec_len, AK_TRUE);
                AK_PRINTK("curtime=", pAudioRecord->currentRecTime, AK_TRUE);
                AK_PRINTK("total=", pAudioRecord->totalRecTime,AK_TRUE);                
            #endif                    
                //rec_buf_flag = 0;
                VME_EvtQueueClearTimerEvent();
                EventParm.c.Param1 = CTRL_EVENT_MEMORY_FULL;
                if(pAudioRecord->total_len >= SINGLE_FILE_LEN_LIMIT)
                {
                    EventParm.c.Param1 = CTRL_EVENT_SINGLE_FILE_LEN_LIMIT;
                }
                VME_EvtQueuePut(VME_EVT_RECORDCTRL, &EventParm);
                return AK_FALSE;
            }

            pAudioRecord->rec_len = 0; 
        }
    }
#if(USE_REC_DEBUG)
    else
    {
#ifdef OS_ANYKA
        AK_PRINTK("RE1:", savBufIndex, 1);
        while(1);
#endif
    }
#endif
    
    return AK_TRUE;
}
*/

/**************************************************************************
* @brief visit all buf get data and write file according to param bSaveFile
* @author zhao_xiaowei
* @date 2009-12
* @param bSaveFile AK_TRUE to save file ,otherwise not saveFile
* @return 
***************************************************************************/
T_BOOL AudioRecord_SaveData(T_BOOL bSaveFile)
{
    if(pAudioRecord == AK_NULL)
    {
        return AK_FALSE;
    }
	
    if (eSTAT_REC_PAUSE == pAudioRecord->RecState)
    {
		return AK_TRUE;
    }
	
    if(pAudioRecord->recBufState != eREC_EMPTY)
    {
        T_U32 datasize= 0;
        T_BOOL bRealSaveData= bSaveFile;
        //T_BOOL bHasSetCpu2x= AK_FALSE;
        
        T_BOOL bCircleDealData = AK_TRUE;       //如果是把数据保存到录音文件中则循环

        while(bCircleDealData)
        {
            if(bRealSaveData)
            {
               bRealSaveData= AudioRecord_SaveDataOnce();
            }
            else
            {
                T_U8 *pDataBuf;
                T_U32 size;
                size = cycbuf_getdatabuf(CYC_AD_BUF_ID, &pDataBuf, IN_BUFFER_SIZE);
                AudioRecord_DealData(pDataBuf, size);
                bCircleDealData = AK_FALSE;         //为录像录音不需要循环，把数据取出来
            }
//           pAudioRecord->head= (pAudioRecord->head+ 1)% pAudioRecord->bufCount;
//           dataCount= (pAudioRecord->recIndex+ pAudioRecord->bufCount - pAudioRecord->head)%pAudioRecord->bufCount;
             datasize = cycbuf_getdatasize(CYC_AD_BUF_ID);

             if (eREC_FULL ==  pAudioRecord->recBufState)
             {
                if(0 == datasize)
                {
                    pAudioRecord->recBufState= eREC_NORMAL;
                    Fwl_ConsoleWriteStr("mc\n");
                    Fwl_WaveInEnsureSending(pAudioRecord->wavin_id);
                    break;
                }
             }
             else if(eREC_NORMAL == pAudioRecord->recBufState)
             {
                if(0 == datasize)
                 {               
                    pAudioRecord->recBufState= eREC_EMPTY;
                    break;
                 }
                 
                 if(datasize <= (pAudioRecord->maxLeftCount<<12))
                 {
                    break;
                 }
             }
             
             
        }
    }
    else
    {
        /*
        AK_PRINTK(" Ep:",pAudioRecord->bInPauseState,1);
        AK_PRINTK(" r:", pAudioRecord->recIndex, 0);
        AK_PRINTK(" h:", pAudioRecord->head, 0);
        AK_PRINTK(" s:", pAudioRecord->recState, 1);
        */
        return AK_FALSE;
    }
    
    return AK_TRUE;

}
#pragma arm section code




#pragma arm section code = "_frequentcode_"
T_BOOL AudioRecord_Process(T_VOID)
{
    if ((AK_NULL == pAudioRecord) || (pAudioRecord->bVideoRec))
        return AK_FALSE;

    return AudioRecord_SaveData(AK_TRUE);
}
#pragma arm section code

/************************************************************************
 * @BRIEF Get current record time
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-8
 * @PARAM T_U32 *recDuration: get current record time(ms)
 * @RETURN T_VOID
 * @RETVAL 
 **************************************************************************/
 #pragma arm section code = "_recording_"
T_U32 AudioRecord_GetCurrentTime(T_VOID)
{
    if (AK_NULL == pAudioRecord)
    {
        return AK_FALSE;
    }
    switch (pAudioRecord->recMode)
    {
        case eREC_MODE_WAV8K:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_WAV8K, CHANNEL_NUM_WAV8K)*SAMPLE_8K_RATE);
            break;
        case eREC_MODE_WAV16K:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_WAV16K, CHANNEL_NUM_WAV16K)*SAMPLE_16K_RATE);
            break; 
        case eREC_MODE_WAV48K:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_WAV48K, CHANNEL_NUM_WAV48K)*SAMPLE_48K_RATE);
            break;
		case eREC_MODE_WAV48K_2:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_WAV48K_2, CHANNEL_NUM_WAV48K_2)*SAMPLE_48K_RATE);
            break;
            
        #ifndef RECORD_PCM_ONLY
        case eREC_MODE_ADPCM8K:
        case eREC_MODE_ADPCM8K_RADIO:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_ADPCM8K, CHANNEL_NUM_ADPCM8K)*SAMPLE_8K_RATE);
            break; 
        case eREC_MODE_ADPCM16K:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_ADPCM16K, CHANNEL_NUM_ADPCM16K)*SAMPLE_16K_RATE);
            break;
        case eREC_MODE_ADPCM8K_2:  
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_ADPCM8K_2, CHANNEL_NUM_ADPCM8K_2)*SAMPLE_8K_RATE);
            break; 
        case eREC_MODE_ADPCM48K:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_ADPCM48K, CHANNEL_NUM_ADPCM48K)*SAMPLE_48K_RATE);
            break;     
        case eREC_MODE_ADPCM48K_2:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_ADPCM48K_2, CHANNEL_NUM_ADPCM48K_2)*SAMPLE_48K_RATE);
            break;
        case eREC_MODE_MP3LOW:   
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_MP3LOW, CHANNEL_NUM_MP3LOW)*SAMPLE_8K_RATE);
            break;
        case eREC_MODE_MP3HI:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_MP3HI, CHANNEL_NUM_MP3HI)*SAMPLE_22K_RATE);
            break;
        case eREC_MODE_SPEEX_LOW:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_SPEEX_LOW, CHANNEL_NUM_SPEEX_LOW)*SAMPLE_8K_RATE);
            break;
        case eREC_MODE_SPEEX_HI:
            pAudioRecord->currentRecTime = \
                 pAudioRecord->ADCounter*(RECTIME_PER_SAMP_CHAN(SAMPLERATE_SPEEX_HI, CHANNEL_NUM_SPEEX_HI)*SAMPLE_8K_RATE);
            break;
        #endif
        
        default:
            pAudioRecord->currentRecTime = 0;
            break;
    }

    return pAudioRecord->currentRecTime;
}
#pragma arm section code 

T_VOID AudioRecord_Pause(T_VOID)
{
    pAudioRecord->RecState = eSTAT_REC_PAUSE;
}

T_VOID AudioRecord_Resume(T_VOID)
{
    pAudioRecord->RecState = eSTAT_RECORDING;
}

T_BOOL AudioRecord_OpenWavIn(T_VOID)
{
    T_WAVE_IN Wav_IO;
    T_U32 nSampleRate;
    T_U16 nChannel;
    T_U16 BitsPerSample;

    if(eSTAT_RCV_MIC == pAudioRecord->revState)
    {
        Wav_IO = MIC_ADC;
    }
    else
    {
        Wav_IO = LINEIN_ADC;
    }

    //设置dma中断回调
    pAudioRecord->wavin_id = Fwl_WaveInOpen(Wav_IO, AudioRecord_InterruptHandle, AK_TRUE);
    AudioRecord_SetGain(pAudioRecord->GainLevel);

    nSampleRate = SAMPLERATE_WAV8K;
    nChannel = CHANNEL_NUM_1;
    BitsPerSample = BITSPERSAMPLE_16;

    if ((pAudioRecord->recMode != eREC_MODE_WAV8K) 
        && (pAudioRecord->recMode != eREC_MODE_WAV16K)
        && (pAudioRecord->recMode != eREC_MODE_WAV48K)
        && (pAudioRecord->recMode != eREC_MODE_WAV48K_2))
    {   //ADPCM encode info
        #ifndef RECORD_PCM_ONLY
        switch(pAudioRecord->recMode)
        {
        case eREC_MODE_AMR:
        case eREC_MODE_ADPCM8K:
        case eREC_MODE_ADPCM8K_RADIO:
            break;
        case eREC_MODE_ADPCM8K_2:
            nChannel = CHANNEL_NUM_ADPCM8K_2;
            break;
        case eREC_MODE_ADPCM16K:
            nSampleRate = SAMPLERATE_ADPCM16K;
            break;
        case eREC_MODE_ADPCM48K:
            nSampleRate = SAMPLERATE_ADPCM48K;//48000;
            break;
        case eREC_MODE_ADPCM48K_2:
            nChannel = CHANNEL_NUM_ADPCM48K_2;
            nSampleRate = SAMPLERATE_ADPCM48K_2;//48000;
          break;
          case eREC_MODE_MP3LOW:
            nChannel = CHANNEL_NUM_MP3LOW;
            nSampleRate = SAMPLERATE_MP3LOW;//48000;
          break;

          case eREC_MODE_MP3HI:
            nChannel = CHANNEL_NUM_MP3HI;
            nSampleRate = SAMPLERATE_MP3HI;//48000;
            break;
        }    
        #endif
    }
    else if(eREC_MODE_WAV16K==pAudioRecord->recMode)
    {
        nSampleRate = SAMPLERATE_WAV16K;
    }
    else if(eREC_MODE_WAV48K == pAudioRecord->recMode)
    {
        nChannel= CHANNEL_NUM_WAV48K;
        nSampleRate= SAMPLERATE_WAV48K;
    }
	else if(eREC_MODE_WAV48K_2 == pAudioRecord->recMode)
    {
        nChannel= CHANNEL_NUM_WAV48K_2;
        nSampleRate= SAMPLERATE_WAV48K_2;
    }
    
    if (!Fwl_WaveInStart(pAudioRecord->wavin_id, nSampleRate,
                       nChannel , BitsPerSample))
    {
        AK_DEBUG_OUTPUT("start record interruput false!\n");
        return AK_FALSE;
    }
    AK_DEBUG_OUTPUT("rec s:%d, n:%d, b:%d\n", nSampleRate, nChannel, BitsPerSample);    

    return AK_TRUE;
}

T_VOID AudioRecord_CloseWavIn(T_VOID)
{
    Fwl_WaveInStop(pAudioRecord->wavin_id);
    Fwl_WaveInClose(pAudioRecord->wavin_id);
    pAudioRecord->wavin_id = 0xff;
}

T_VOID AudioRecord_StopWaveIn(T_VOID)
{
    Fwl_WaveInStop(pAudioRecord->wavin_id);
}

T_VOID AudioRecord_StartWaveIn(T_VOID)
{
    Fwl_WaveInStart(pAudioRecord->wavin_id, pAudioRecord->RecSampleRate, pAudioRecord->channels, pAudioRecord->RecBitsPerSample);
}

//设置音频数据源
T_VOID AudioRecord_SetDataSource(T_eREC_RCVSTAT state)
{
    pAudioRecord->revState = state;
}

#pragma arm section code = "_recording_"
T_VOID AudioRecord_SetVorCtrlState(T_eVORREC_STATE state)
{
    if(AK_NULL != pAudioRecord)
    {   
        pAudioRecord->VorCtrlState = state;
    }    
    AK_DEBUG_OUTPUT("lastState:%d\n", pAudioRecord->VorCtrlState);
}

#pragma arm section code


#pragma arm section code = "_frequentcode_"

T_eVORREC_STATE AudioRecord_GetVorCtrlState(T_VOID)
{
    if(AK_NULL == pAudioRecord)
    {   
        return eSTAT_VORREC_CLOSE;
    }
    
    return pAudioRecord->VorCtrlState;
}

#pragma arm section code 

static T_VOID AudioRecord_WriteData(T_U8* pDest, T_U32 data, T_U32 bytenum)
{
    if(!pDest)
        return;
    else
    {
        *pDest++ = (T_U8)(data&0xff);
        *pDest++ = (T_U8)((data&0xff00)>>8);
        if(bytenum>2)
        {
            *pDest++ = (T_U8)((data&0xff0000)>>16);
            *pDest = (T_U8)((data&0xff000000)>>24);
        }
    }

}

#pragma arm section code = "_recording_"
static T_VOID AudioRecord_UpdateHead(T_VOID)
{
    T_U32 oldPos;
    
    if(pAudioRecord->recMode == eREC_MODE_MP3HI 
        || pAudioRecord->recMode == eREC_MODE_MP3LOW
        || pAudioRecord->recMode == eREC_MODE_SPEEX_LOW 
        || pAudioRecord->recMode == eREC_MODE_SPEEX_HI)
    {
        Fwl_FileFlush(pAudioRecord->rec_fd);
        return;
    }

    oldPos = Fwl_FileTell(pAudioRecord->rec_fd);
    
    #ifndef RECORD_PCM_ONLY
    if(eREC_MODE_AMR == pAudioRecord->recMode)
    {
        T_U8 endData[]={0, 'A','N','Y','K','A', 0x0D,0x0A};
        Fwl_FileWrite(pAudioRecord->rec_fd, endData, 8);
        return;
    }
    #endif
    if((eREC_MODE_WAV8K==pAudioRecord->recMode)
        ||(eREC_MODE_WAV16K==pAudioRecord->recMode)
        ||(eREC_MODE_WAV48K == pAudioRecord->recMode)
        ||(eREC_MODE_WAV48K_2 == pAudioRecord->recMode))
    {
        AudioRecord_WriteData(&fileHeadPcm.file_size, pAudioRecord->total_len + 44 - 8, 4);
        AudioRecord_WriteData(&fileHeadPcm.data_length, pAudioRecord->total_len , 4);
    }
    #ifndef RECORD_PCM_ONLY
    else
    {
        AudioRecord_WriteData(&fileHead.file_size, pAudioRecord->total_len + 60 - 8 , 4);
        AudioRecord_WriteData(&fileHead.ima_format.nAvgBytesPerSec, pAudioRecord->audio_record_enc.out_info.nAvgBytesPerSec , 4);
        AudioRecord_WriteData(&fileHead.ima_format.nBlockAlign, pAudioRecord->audio_record_enc.out_info.nBlockAlign , 2);
        AudioRecord_WriteData(&fileHead.ima_format.wSamplesPerBlock, pAudioRecord->audio_record_enc.out_info.nSamplesPerPacket , 2);
        AudioRecord_WriteData(&fileHead.data_length, pAudioRecord->total_len , 4);
    }
    #endif
    switch(pAudioRecord->recMode)
    {
    #ifndef RECORD_PCM_ONLY
    case eREC_MODE_ADPCM8K:
    case eREC_MODE_ADPCM8K_RADIO:
        AudioRecord_WriteData(&fileHead.ima_format.nChannels, CHANNEL_NUM_ADPCM8K , 2);
        AudioRecord_WriteData(&fileHead.ima_format.nSamplesPerSec, SAMPLERATE_ADPCM8K , 4);
        AudioRecord_WriteData(&fileHead.factdata_size, 8*pAudioRecord->currentRecTime , 4);
        break;
    case eREC_MODE_ADPCM8K_2:
        AudioRecord_WriteData(&fileHead.ima_format.nChannels, CHANNEL_NUM_ADPCM8K_2 , 2);
        AudioRecord_WriteData(&fileHead.ima_format.nSamplesPerSec, SAMPLERATE_ADPCM8K_2 , 4);
        AudioRecord_WriteData(&fileHead.factdata_size, 8*pAudioRecord->currentRecTime , 4);
        break;
    case eREC_MODE_ADPCM16K:
        AudioRecord_WriteData(&fileHead.ima_format.nChannels, CHANNEL_NUM_ADPCM16K , 2);
        AudioRecord_WriteData(&fileHead.ima_format.nSamplesPerSec, SAMPLERATE_ADPCM16K , 4);
        AudioRecord_WriteData(&fileHead.factdata_size, 16*pAudioRecord->currentRecTime , 4);
        break;
    case eREC_MODE_ADPCM48K:
        AudioRecord_WriteData(&fileHead.ima_format.nChannels, CHANNEL_NUM_ADPCM48K , 2);
        AudioRecord_WriteData(&fileHead.ima_format.nSamplesPerSec, SAMPLERATE_ADPCM48K, 4);//48000 , 4);
        AudioRecord_WriteData(&fileHead.factdata_size, 24*pAudioRecord->currentRecTime , 4);
        break;
    case eREC_MODE_ADPCM48K_2:
        AudioRecord_WriteData(&fileHead.ima_format.nChannels, CHANNEL_NUM_ADPCM48K_2 , 2);
        AudioRecord_WriteData(&fileHead.ima_format.nSamplesPerSec, SAMPLERATE_ADPCM48K_2, 4);//48000 , 4);
        AudioRecord_WriteData(&fileHead.factdata_size, 16*pAudioRecord->currentRecTime , 4);
        break;
    #endif
    case eREC_MODE_WAV8K: 
        AudioRecord_WriteData(&fileHeadPcm.num_chans, CHANNEL_NUM_WAV8K , 2);
        AudioRecord_WriteData(&fileHeadPcm.sample_rate, SAMPLERATE_WAV8K , 4);
        AudioRecord_WriteData(&fileHeadPcm.bytes_per_sec, BYTES_PER_SEC_WAV8K , 4);
        break;
    case eREC_MODE_WAV16K:
        AudioRecord_WriteData(&fileHeadPcm.num_chans, CHANNEL_NUM_WAV16K , 2);
        AudioRecord_WriteData(&fileHeadPcm.sample_rate, SAMPLERATE_WAV16K , 4);
        AudioRecord_WriteData(&fileHeadPcm.bytes_per_sec, BYTES_PER_SEC_WAV16K , 4);
        break;
    case eREC_MODE_WAV48K:
        AudioRecord_WriteData(&fileHeadPcm.num_chans, CHANNEL_NUM_WAV48K , 2);
        AudioRecord_WriteData(&fileHeadPcm.sample_rate, SAMPLERATE_WAV48K , 4);
        AudioRecord_WriteData(&fileHeadPcm.bytes_per_sec, BYTES_PER_SEC_WAV48K , 4);
        break;
	case eREC_MODE_WAV48K_2:
        AudioRecord_WriteData(&fileHeadPcm.num_chans, CHANNEL_NUM_WAV48K_2 , 2);
        AudioRecord_WriteData(&fileHeadPcm.sample_rate, SAMPLERATE_WAV48K_2 , 4);
        AudioRecord_WriteData(&fileHeadPcm.bytes_per_sec, BYTES_PER_SEC_WAV48K_2 , 4);
        break;
    }
    
    Fwl_FileSeek(pAudioRecord->rec_fd,  0, FS_SEEK_SET);

    if ((pAudioRecord->recMode == eREC_MODE_WAV8K)
        ||(pAudioRecord->recMode == eREC_MODE_WAV16K)
        || (eREC_MODE_WAV48K ==  pAudioRecord->recMode)
        || (eREC_MODE_WAV48K_2 ==  pAudioRecord->recMode))
    {
        Fwl_FileWrite(pAudioRecord->rec_fd, &fileHeadPcm, sizeof(fileHeadPcm));        
    }
    #ifndef RECORD_PCM_ONLY
    else
    {
        Fwl_FileWrite(pAudioRecord->rec_fd, &fileHead, sizeof(fileHead));
    }
    #endif
    Fwl_FileFlush(pAudioRecord->rec_fd);
    Fwl_FileSeek(pAudioRecord->rec_fd,  oldPos, FS_SEEK_SET);
}

#if (1050L == USE_CHIP_TYPE)
static T_VOID AudioRecord_DataConvert(T_U8* pdata)
{
    T_U32 i =0;
    T_U16 *p,*q ;

    if(AK_NULL==pdata)
        return;
//ak1050L芯片，每四个字节，高两个有效，低两个无效；
    if(2 == pAudioRecord->channels)//双声道录音
    {    
		q = (T_U16*)pdata;
	    p = q+1;

        while(i < (IN_BUFFER_SIZE>>2))
        {
        //ak1050L芯片只有一个adc，因此低两个字节也是无效的
        //要把左声道的数据拷给右声道；
        //ak1080L芯片则不需要此项操作
            *p++ = *q++;
			q++;
			p++;
            i++;
        }
    }
}
#endif
#pragma arm section code 


#pragma arm section code = "_recording_"
/*********************************************
*this function will be used in AudioRecord_SaveData() and AudioRecord_SaveIndexData();
*AUTHOR: li_shengkai
*DATA:2011-3-17
*PARAM  : T_U8 *[in]pDataBuf    (the pcm data)
*RETURN :T_U32
*RETVAL :the data len after encoded;
*********************************************/
static T_U32 AudioRecord_DealData(T_U8 *pDataBuf, T_U32 size)
{
    T_S32 reclen = 0;

#ifndef RECORD_PCM_ONLY

    T_MEDIALIB_ENC_BUF_STRC enc_buf_strc;


#if (1050L == USE_CHIP_TYPE)  
	    AudioRecord_DataConvert(pDataBuf);
#endif

    enc_buf_strc.len_in = size;
        
    if(AK_TRUE==pAudioRecord->bCoding)
    {//adpcm coding               

        enc_buf_strc.buf_in = pDataBuf;                
        enc_buf_strc.buf_out = pAudioRecord->OutBuf;        
        enc_buf_strc.len_out = pAudioRecord->bufsize;
        reclen = MediaLib_Encode(pAudioRecord->hMedia,&enc_buf_strc);

        if (0 < reclen)
        {
            pAudioRecord->rec_len = (T_U32)reclen;
        }
        else
        {
            pAudioRecord->rec_len = 0;
        }
    }
    else
    {
        memcpy(pAudioRecord->OutBuf ,pDataBuf ,enc_buf_strc.len_in);
        pAudioRecord->rec_len = enc_buf_strc.len_in;                
    }
    
#else

    T_U32 len_in;


#if (1050L == USE_CHIP_TYPE)      
        AudioRecord_DataConvert(pDataBuf);
#endif

    len_in = size;
    
    memcpy(pAudioRecord->OutBuf ,pDataBuf ,len_in);
    
    //akerror("addr@deal:",pAudioRecord->OutBuf,1);
    //akerror("data@deal:",*((T_U16*)pAudioRecord->OutBuf),1);
    //akerror("data1@deal:",*((T_U16*)pAudioRecord->OutBuf+4),1);
    
    pAudioRecord->rec_len = len_in;
    
#endif
    return pAudioRecord->rec_len ;
}

#pragma arm section code

//the end of file


