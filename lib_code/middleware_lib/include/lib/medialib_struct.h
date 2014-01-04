/**
 * @file medialib_struct.h
 * @brief Define the global public types for media lib
 *
 * @Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Su_Dan
 * @date 2007-11-22
 * @update data 2008-06-02
 * @version 1.0
 */

#ifndef _MEDIA_LIB_STRUCT_H_
#define _MEDIA_LIB_STRUCT_H_

#include "medialib_global.h"

#define DMX_INFO_EX_SIZE 24
#define _MEDIA_EQ_MAX_BANDS 10

//media handle
typedef T_pVOID	T_MEDIALIB_STRUCT;

typedef enum
{
	MEDIALIB_MEDIA_UNKNOWN,
	MEDIALIB_MEDIA_AKV,
	MEDIALIB_MEDIA_AVI,
	MEDIALIB_MEDIA_WAV,
	MEDIALIB_MEDIA_MP4,
	MEDIALIB_MEDIA_3GP,
	MEDIALIB_MEDIA_M4A,
	MEDIALIB_MEDIA_MOV,
	MEDIALIB_MEDIA_ASF,
	MEDIALIB_MEDIA_REAL,
	MEDIALIB_MEDIA_MP3,
	MEDIALIB_MEDIA_AKV2,	//add below
	MEDIALIB_MEDIA_AAC,
	MEDIALIB_MEDIA_AMR,
	MEDIALIB_MEDIA_MIDI,
	MEDIALIB_MEDIA_FLV,
	MEDIALIB_MEDIA_APE,
	MEDIALIB_MEDIA_FLAC,
	MEDIALIB_MEDIA_OGG_FLAC,
	MEDIALIB_MEDIA_AV,
	MEDIALIB_MEDIA_OGG_VORBIS,
	MEDIALIB_MEDIA_AC3,
	MEDIALIB_MEDIA_PCM_ALAW,
	MEDIALIB_MEDIA_PCM_ULAW,
	MEDIALIB_MEDIA_SPEEX,
    MEDIALIB_MEDIA_SBC
}T_eMEDIALIB_MEDIA_TYPE;


typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF				m_FunPrintf;

	MEDIALIB_CALLBACK_FUN_READ					m_FunRead;
	MEDIALIB_CALLBACK_FUN_WRITE					m_FunWrite;
	MEDIALIB_CALLBACK_FUN_SEEK					m_FunSeek;
	MEDIALIB_CALLBACK_FUN_TELL					m_FunTell;
	MEDIALIB_CALLBACK_FUN_MALLOC				m_FunMalloc;
	MEDIALIB_CALLBACK_FUN_FREE					m_FunFree;
	MEDIALIB_CALLBACK_FUN_DMA_MALLOC			m_FunDMAMalloc;
	MEDIALIB_CALLBACK_FUN_DMA_FREE				m_FunDMAFree;

	MEDIALIB_CALLBACK_FUN_RTC_DELAY				m_FunRtcDelay;
	MEDIALIB_CALLBACK_FUN_DMA_MEMCPY			m_FunDMAMemcpy;
	MEDIALIB_CALLBACK_FUN_CYC_BUFCPY            m_FunCpy;
}T_MEDIALIB_CB;




typedef struct
{
	T_MEDIALIB_CB	m_CBFunc;
}T_MEDIALIB_INIT;

typedef struct
{
	T_S32			m_hMediaSource;			//如果只播放音频文件，只用传如一个文件句柄，就是这个句柄
	T_S32			m_hAppendMediaSource;	//在播放视频文件的时候，增加一个文件句柄
	T_MEDIALIB_CB	m_CBFunc;
	
#if 1 //ndef SPOTLIGHT11	
	AKV_CB_VIDEO_OUT			m_FunVidioOut;
	T_U16						m_vOutOffsetX;
	T_U16						m_vOutOffsetY;
#endif

	T_BOOL					m_bFadeEnable;	//允许淡入淡出
	T_U32					m_bFadeInTime;	//淡入时间
	T_U32					m_bFadeOutTime;	//淡出时间
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;	//媒体类型

#if 1 //def SPOTLIGHT11
	T_U32			m_jdrv_himg;			//JDRV_T_hImg类型的句柄
#endif
	T_U32  m_buflen;  //set input buffer len
}T_MEDIALIB_OPEN_INPUT;

typedef enum
{
	MEDIALIB_VIDEO_UNKNOWN,
	MEDIALIB_VIDEO_MPEG4,
	MEDIALIB_VIDEO_H263,
	MEDIALIB_VIDEO_WMV,
	MEDIALIB_VIDEO_FLV263,
	MEDIALIB_VIDEO_H264,
	MEDIALIB_VIDEO_RV,
	MEDIALIB_VIDEO_MJPEG,
	MEDIALIB_VIDEO_MPEG2
}T_eMEDIALIB_VIDEO_CODE;	//这里只支持MJPEG

typedef enum
{
	MEDIALIB_AUDIO_UNKNOWN,
	MEDIALIB_AUDIO_AMR,
	MEDIALIB_AUDIO_MP3,
	MEDIALIB_AUDIO_AAC,
	MEDIALIB_AUDIO_PCM,
	MEDIALIB_AUDIO_WMA,
	MEDIALIB_AUDIO_MIDI,
	MEDIALIB_AUDIO_ADPCM,
	MEDIALIB_AUDIO_APE,
	MEDIALIB_AUDIO_FLAC,
	MEDIALIB_AUDIO_VORBIS,
	MEDIALIB_AUDIO_G711,
	MEDIALIB_AUDIO_ADPCM_IMA,
	MEDIALIB_AUDIO_ADPCM_MS,
	MEDIALIB_AUDIO_ADPCM_FLASH
}T_eMEDIALIB_AUDIO_CODE;

typedef struct _T_MEDIALIB_META_TYPE_INFO
{
	T_U8	VersionType;	//0: unicode; 1: non-unicode
	T_U8	TitleType;
	T_U8	ArtistType;
	T_U8	AlbumType;
	T_U8	YearType;
	T_U8	CommentType;
	T_U8	GenreType;
	T_U8	TrackType;
	T_U8	ComposerType;
}T_MEDIALIB_META_TYPE_INFO;

typedef struct _T_MEDIALIB_META_SIZE_INFO
{
	T_U16	uVersionLen;
	T_U16	uTitleLen;
	T_U16	uArtistLen;
	T_U16	uAlbumLen;
	T_U16	uYearLen;
	T_U16	uCommentLen;
	T_U16	uGenreLen;
	T_U16	uTrackLen;
	T_U16	uComposerLen;
}T_MEDIALIB_META_SIZE_INFO;

typedef struct _MEDIALIB_META_CONTENT
{
	T_VOID	*pVersion;		//metainfo version
	T_VOID	*pTitle;		//Title
	T_VOID	*pArtist;		//Artist
	T_VOID	*pAlbum;		//Album
	T_VOID	*pYear;			//Year
	T_VOID	*pComment;		//Comment
	T_VOID	*pGenre;		//Genre
	T_VOID	*pTrack;		//Track
	T_VOID	*pComposer;		//Composer
}T_MEDIALIB_META_CONTENT;

typedef struct _MEDIALIB_META_INFO
{
	T_MEDIALIB_META_TYPE_INFO	m_MetaTypeInfo;
	T_MEDIALIB_META_SIZE_INFO	m_MetaSizeInfo;
	T_MEDIALIB_META_CONTENT		m_MetaContent;
}T_MEDIALIB_META_INFO;

typedef struct
{
	T_U32	audio_type;			//音频类型
	T_U32	total_time_ms;		//总时间
	T_U32	audio_bitrate;		//音频码率
	T_U32	nSamplesPerSec;		//采样率
	T_U16	nChannels;			//声道数
	T_U16	wBitsPerSample;		//采样位数

	T_BOOL	m_bHasAudio;		//是否有音频
	T_BOOL	m_bHasVideo;		//是否有视频
	T_eMEDIALIB_MEDIA_TYPE	m_MediaType;	//媒体类型
	T_eMEDIALIB_VIDEO_CODE	m_VideoCode;	//视频编码,只支持MJPEG

	T_U16	m_uWidth;	//宽
	T_U16	m_uHeight;	//高
	T_U16	m_uFPS;		//帧率
	T_U32	m_ulBitrate;//码率

	T_MEDIALIB_META_INFO	*m_pMetaInfo;

    /*
    返回一些音频格式的私有信息，
    例如对ape来说，[8~11]表示level，[16~19]表示version
    */
    T_U8 szData[DMX_INFO_EX_SIZE]; 
}T_MEDIALIB_MEDIA_INFO;

typedef enum
{
	MEDIALIB_END,
	MEDIALIB_PLAYING,
	MEDIALIB_FF,
	MEDIALIB_FR,
	MEDIALIB_PAUSE,
	MEDIALIB_STOP,
	MEDIALIB_ERR,
	MEDIALIB_SEEK
}T_eMEDIALIB_STATUS;

typedef struct
{
	//原始数据大小
	T_U16	pcmChannel;		//立体声(2)、单声道(1)
	T_U16	pcmBitsPerSample;//16 bit固定(16)
	T_U32	pcmSampleRate;	//采样率(8000)

	T_U32	e_Type;			//encode type
	T_U16	e_nChannel;		//立体声(2)、单声道(1)
	T_U16	e_BitsPerSample;//16 bit固定(16)
	T_U32	e_nSampleRate;	//采样率(8000)
	T_S32	e_nVolume;		//音量
	T_U32	e_Bitrate;		//比特率设置，kbps

	T_BOOL  cbr;
	T_BOOL   dtx_disable;
}T_MEDIALIB_ENC_IN_INFO;

typedef struct
{
	T_U16	wFormatTag;
	T_U16	nChannels;
	T_U32	nSamplesPerSec;
	T_U32	nAvgBytesPerSec;
	T_U16	nBlockAlign;
	T_U16	wBitsPerSample;
	T_U16	nSamplesPerPacket;
}T_MEDIALIB_ENC_OUT_INFO;

typedef struct
{
	T_MEDIALIB_CB m_CBFunc;
	T_MEDIALIB_ENC_IN_INFO in_info;
	T_MEDIALIB_ENC_OUT_INFO out_info;
}T_MEDIALIB_ENC_OPEN_INPUT;

typedef struct
{
	T_VOID *buf_in;
	T_VOID *buf_out;
	T_U32 len_in;
	T_U32 len_out;
}T_MEDIALIB_ENC_BUF_STRC;

typedef struct
{
	T_S32			hMediaSource;
	T_MEDIALIB_CB	CBFunc;
	T_MEDIALIB_MEDIA_INFO media_info;
}T_MEDIALIB_AUDIO_OPEN_INPUT;

typedef struct
{
    T_U32 bands;      //1~10
    T_U32 bandfreqs[_MEDIA_EQ_MAX_BANDS];
	T_U32 bandgains[_MEDIA_EQ_MAX_BANDS];
}T_MEDIALIB_EQ_FILTERS;

//以下结构体定义来自da_buffer.h、sdfilter.h和sdcodec.h，外部需要使用，故定义在此
typedef enum
{
    _SD_FILTER_UNKNOWN ,
    _SD_FILTER_EQ ,
    _SD_FILTER_WSOLA ,
    _SD_FILTER_RESAMPLE,
    _SD_FILTER_3DSOUND,
    _SD_FILTER_DENOICE,
    _SD_FILTER_AGC,
    _SD_FILTER_VOICECHANGE
}T_AUDIO_FILTER_TYPE;

typedef enum
{
	_SD_EQ_MODE_NORMAL ,
	_SD_EQ_MODE_CLASSIC ,
	_SD_EQ_MODE_JAZZ ,
	_SD_EQ_MODE_POP ,
	_SD_EQ_MODE_ROCK ,
	_SD_EQ_MODE_EXBASS ,
	_SD_EQ_MODE_SOFT ,
	_SD_EQ_USER_DEFINE 
}T_EQ_MODE;

typedef enum
{
	_SD_WSOLA_0_5 ,
	_SD_WSOLA_0_6 ,
	_SD_WSOLA_0_7 ,
	_SD_WSOLA_0_8 ,
	_SD_WSOLA_0_9 ,
	_SD_WSOLA_1_0 ,
	_SD_WSOLA_1_1 ,
	_SD_WSOLA_1_2 ,
	_SD_WSOLA_1_3 ,
	_SD_WSOLA_1_4 ,
	_SD_WSOLA_1_5 ,
	_SD_WSOLA_1_6 ,
	_SD_WSOLA_1_7 ,
	_SD_WSOLA_1_8 ,
	_SD_WSOLA_1_9 ,
	_SD_WSOLA_2_0 
}T_WSOLA_TEMPO;

typedef enum
{
	_SD_WSOLA_ARITHMATIC_0 ,	// 0:WSOLA, fast but tone bad
	_SD_WSOLA_ARITHMATIC_1		// 1:PJWSOLA, slow but tone well
}T_WSOLA_ARITHMATIC;

typedef enum
{
    RESAMPLE_ARITHMETIC_0 = 0,
    RESAMPLE_ARITHMETIC_1
}RESAMPLE_ARITHMETIC;

typedef enum
{
    _SD_OUTSR_UNKNOW = 0,
    _SD_OUTSR_48KHZ = 1,
    _SD_OUTSR_44KHZ,
    _SD_OUTSR_32KHZ,
    _SD_OUTSR_24KHZ,
    _SD_OUTSR_22KHZ,
    _SD_OUTSR_16KHZ,
    _SD_OUTSR_12KHZ,
    _SD_OUTSR_11KHZ,
    _SD_OUTSR_8KHZ
} T_RES_OUTSR;

typedef enum
{
    PITCH_NORMAL = 0,
    PITCH_CHILD_VOICE ,
    PITCH_MACHINE_VOICE,
    PITCH_ECHO_EFFECT,
    PITCH_RESERVE
}T_PITCH_MODES;

typedef enum
{
	_SD_MEDIA_TYPE_UNKNOWN ,
	_SD_MEDIA_TYPE_MIDI ,
	_SD_MEDIA_TYPE_MP3 ,
	_SD_MEDIA_TYPE_AMR ,
	_SD_MEDIA_TYPE_AAC ,
	_SD_MEDIA_TYPE_WMA ,
	_SD_MEDIA_TYPE_PCM ,
	_SD_MEDIA_TYPE_ADPCM_IMA ,
	_SD_MEDIA_TYPE_ADPCM_MS ,
	_SD_MEDIA_TYPE_ADPCM_FLASH ,
	_SD_MEDIA_TYPE_APE ,
	_SD_MEDIA_TYPE_FLAC ,
	_SD_MEDIA_TYPE_OGG_FLAC,
	_SD_MEDIA_TYPE_RA8LBR ,
	_SD_MEDIA_TYPE_DRA,
	_SD_MEDIA_TYPE_OGG_VORBIS,
	_SD_MEDIA_TYPE_AC3,
	_SD_MEDIA_TYPE_PCM_ALAW,
	_SD_MEDIA_TYPE_PCM_ULAW,
	_SD_MEDIA_TYPE_SBC,
	_SD_MEDIA_TYPE_SPEEX
}T_AUDIO_TYPE;

typedef enum
{
	DABUF_INIT ,	//初始化所有状态
	DABUF_PAUSE ,	//暂停
	DABUF_RESUME ,	//返回
	DABUF_END ,		//结束
	DABUF_FULL ,	//判断buffer 是否满
	DABUF_COUNT ,	//获取DA发送次数
	DABUF_SEEK		//是否处于seek状态
} T_eDA_STA_TYPE;

typedef enum
{
	SOUND_NO_BEGIN ,	//声音还没开始
	SOUND_BEGIN,		//声音已经开始
	SOUND_NORMAL_PLAY,	//正常播放声音状态
	SOUND_NEARLY_END,	//声音已经快接近尾声
	SOUGN_END			//已经结束尾声，进入无声 
} T_eDA_SAMPLE_STA_TYPE;


/****************** recoder *******************************/

typedef enum
{
	MEDIALIB_REC_AVI_NORMAL,
	MEDIALIB_REC_AVI_CYC,
	MEDIALIB_REC_3GP,
	MEDIALIB_REC_AVI_SEGMENT
}T_eMEDIALIB_REC_TYPE;

typedef enum
{
	MEDIALIB_V_ENC_H263,
	MEDIALIB_V_ENC_MJPG,
	MEDIALIB_V_ENC_MPEG
}T_eMEDIALIB_V_ENC_TYPE;

typedef struct
{
	T_U32	m_Type;			//media type
	T_U16	m_nChannel;		//立体声(2)、单声道(1)
	T_U16	m_BitsPerSample;//16 bit固定(16)
	T_U32	m_nSampleRate;	//采样率(8000)
	T_U16	m_ulDuration;	//每个音频包持续时间
}T_AUDIO_RECORD_INFO;

typedef struct
{
	T_U16	m_nWidth;	//视频宽
	T_U16	m_nHeight;	//视频高
	T_U16	m_nFPS;		//帧率
	T_U16	m_nKeyframeInterval;			//关键桢间隔,MJPEG没影响
	T_U32	m_nvbps;						//视频的码率
	T_eMEDIALIB_V_ENC_TYPE	m_eVideoType;	//录像类型,spotlight只能是MJPEG
}T_VIDEO_RECORD_INFO;

typedef enum
{
	MEDIALIB_OK,
	MEDIALIB_FORMAT_ERR,
	MEDIALIB_PARAM_ERR,
	MEDIALIB_SYSTEM_ERR,
	MEDIALIB_SUPPORT_ERR
}T_eMEDIALIB_STATE;


typedef struct
{
//	T_eMEDIALIB_REC_TYPE	m_MediaRecType;
	T_S32					m_hMediaDest;	//media file
	T_S32					m_hIndexFile;	//tempfile for recod index
	T_AUDIO_RECORD_INFO		m_AudioRecInfo;	//must set Audio info
	T_VIDEO_RECORD_INFO		m_VideoRecInfo;	//must set video info
	T_MEDIALIB_CB			m_CBFunc;

	T_U16					m_SectorSize;

	T_BOOL	m_bCaptureAudio;
	T_BOOL	m_bIdxInMem;		//flag indicating Index is saved in memory
	T_U32	m_IndexMemSize;		//index size set by system

//	T_U16	m_RecordSecond;		//add for time limit record

	
	MEDIALIB_EXFUNC_SET_QUALITY				m_ExFunSetQuality;		//图象库接口,设置图像质量,控制视频码率
	MEDIALIB_EXFUNC_GET_VIDEO_SIZE			m_ExFunGetVideoSize;	//图象库接口,获取当前帧大小
	MEDIALIB_EXFUNC_WRITE					m_ExFunWrite;			//图象库接口,写当前帧
	MEDIALIB_EXFUNC_GET_AUDIO_DATA			m_ExFunGetAudioData;	//系统平台接口,获取当前音频数据
	
	T_S32	m_Reserved; 		//reserved
	
}T_MEDIALIB_REC_OPEN_INPUT;

typedef struct
{	
	T_eMEDIALIB_STATE		m_State;
	T_U32					m_ulAudioEncBufSize;	//音频编码缓冲区的期望大小
	T_U32					m_ulVideoEncBufSize;	//视频编码缓冲区的期望大小
}T_MEDIALIB_REC_OPEN_OUTPUT;

//录像状态
typedef enum
{
	MEDIALIB_REC_OPEN,		//打开,准备好录像
	MEDIALIB_REC_STOP,		//停止,完成录像
	MEDIALIB_REC_DOING,		//进行中,正在录像
	MEDIALIB_REC_SYSERR,	//系统错误,无法继续
	MEDIALIB_REC_MEMFULL,	//磁盘满,一般判断的
	MEDIALIB_REC_SYNERR		//同步错误,音视频时间相差太大
}T_eMEDIALIB_REC_STATUS;


typedef struct
{
	//fix
	T_U16	ori_width;
	T_U16	ori_height;
	T_U16	fps;
	T_U16	keyframeInterval;
	T_BOOL	bCaptureAudio;
	T_U16	record_second;			//add for time limit record

	//dynamic
	T_eMEDIALIB_REC_STATUS record_status;
	T_U32	total_frames;		//total frames, include video frames and audio packets
	T_U32	total_video_frames;	//total video frames
	T_U32	info_bytes;			//expect free space, to save header or index
	T_U32	file_bytes;			//current file size
	T_U32	total_time_ms;		//record time in millisecond
}T_MEDIALIB_REC_INFO;

#endif//_MEDIA_LIB_STRUCT_H_
