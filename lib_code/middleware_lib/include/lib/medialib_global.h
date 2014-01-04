/**
 * @file medialib_global.h
 * @brief Define the global public types for media lib
 *
 * @Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Su_Dan
 * @date 2007-08-20
 * @update data 2007-11-27
 * @version 2.0
 */

#ifndef _MEDIA_LIB_GLOBAL_H_
#define _MEDIA_LIB_GLOBAL_H_

#include "anyka_types.h"


typedef T_U16 (*MEDIALIB_EXFUNC_SET_QUALITY)(T_U16 MJPEG_Quality);//20110810 �޸ķ���ֵ
typedef T_U32 (*MEDIALIB_EXFUNC_GET_VIDEO_SIZE)(T_VOID);
typedef T_S32 (*MEDIALIB_EXFUNC_WRITE)(T_S32 fHandle);
typedef T_U32 (*MEDIALIB_EXFUNC_GET_AUDIO_DATA)(T_U8 **buf);

//#define FOR_SPOTLIGHT

typedef T_VOID (*MEDIALIB_CALLBACK_FUN_PRINTF)(T_pCSTR format, ...);
 
//typedef T_S32 (*MEDIALIB_CALLBACK_FUN_OPEN)(T_pVOID lpData);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_READ)(T_S32 hFile, T_pVOID buf, T_U32 size);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_WRITE)(T_S32 hFile, T_pCVOID buf, T_U32 size);
typedef T_U32 (*MEDIALIB_CALLBACK_FUN_SEEK)(T_S32 hFile, T_U32 offset, T_U16 whence); 
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_TELL)(T_S32 hFile);
//typedef T_VOID (*MEDIALIB_CALLBACK_FUN_CLOSE)(T_S32 hFile);
 
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_MALLOC)(T_U32 size); 
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_FREE)(T_pVOID mem);
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_DMA_MALLOC)(T_U32 size); 
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_DMA_FREE)(T_pVOID mem);
 
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_RTC_DELAY) (T_U32 ulTicks);
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_DMA_MEMCPY)(T_pVOID dest, T_pCVOID src, T_U32 size);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE)(T_VOID);
 
typedef T_S32 (*AKV_CB_VIDEO_OUT)(T_pVOID rgbBuf, T_S32 x, T_S32 y, T_S32 width, T_S32 height);

typedef T_S32 (*MEDIALIB_CALLBACK_FUN_CMMBSYNCTIME)(T_U32 timestamp);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_CMMBAUDIORECDATA)(T_U8 *buf,T_S32 len);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_CYC_BUFCPY)(T_U8* buf, T_U32 size);
typedef struct
{
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample 

	T_U32	m_ulSize;
	T_U32	m_ulDecDataSize;
	T_U8	*m_pBuffer;
}T_AUDIO_DECODE_OUT;

typedef struct
{
	T_S32	real_time;

	union {
		struct{
			T_U32 nCurBitIndex;
			T_U32 nFrameIndex;
		} m_ape;
		struct{
			T_U32 Indx;
			T_U32 offset;
			T_BOOL flag;
			T_U32  last_granu;
			T_U32  now_granu;
			T_BOOL is_eos;
			T_U32  re_data;
			T_U32  pack_no;
			T_U32  list[255];
		}m_speex;
		struct{
			T_U8   secUse;       //�Ѿ���ȡ��section��Ŀ
			T_U8   secLen;      //һ��page�а�����section��
			T_U8   tmpSec;      //�Ѿ������section��Ŀ
			T_BOOL is_eos;      //�ǲ������һ��page
			T_BOOL is_bos;      //�ǲ��ǵ�һ��page
			T_U8   endpack;     //��ǰpage�����һ��packet��λ��
			//�������sample����һ��64λ������Ŀǰֻȡ��32λ
			T_U32  gos;         //�����굱ǰpage�������ܵ�sample������32λ
			T_U32  high_gos;    //�����굱ǰpage�������ܵ�sample������32λ��(��ʱ���ã������Ժ���Ҫ)
			T_U8  list[255];   //��¼һ��page��ÿ��section�Ĵ�С��һ��page����ຬ��255��section
		}m_vorbis;
	}m_Private;
}T_AUDIO_SEEK_INFO;
#endif//_MEDIA_LIB_GLOBAL_H_
