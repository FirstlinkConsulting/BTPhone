/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME btphone.c
 * @BRIEF hfp process 
 * @Author：zhuangyuping
 * @Date：2008-04-15
 * @Version：
**************************************************************************/

#include "Eng_BtPhone.h"
#include "fwl_osMalloc.h"
#include <string.h>
#include "ba_a2dp.h"
#include "ba_hfp.h"
#include "ba_avrcp.h"
#include "ba_gap.h"
#include "eng_debug.h"
#include "Aec_interface.h"
#include "fwl_Serial.h"
#include "Eng_VoiceTip.h"
#include "audio_decoder.h"
#include "sdfilter.h"
#include "fwl_timer.h"

#ifdef SUPPORT_BLUETOOTH

#define AD_DMA_BUF_SIZE		4096
#define DA_DMA_BUF_SIZE		4096
#define AD_SEND_BUF_SIZE	60
#define AEC_ONE_SIZE		256
#define SEND_SCO_BUF_SIZE		((AEC_ONE_SIZE) + (AD_SEND_BUF_SIZE))

T_BTHFP_PHONEMGR *gPhoneMgr = AK_NULL;

extern T_BOOL BtDev_IsPlayingRing(T_VOID);


/**
 * @BRIEF	set phone's Speaker Volume and save vol in local .
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL add
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPhone_SetSpeakerVolume(T_BOOL add)
{
	if(AK_NULL != gPhoneMgr)
	{
		//首先要防止按键太频繁，导致发送指令太快
		if(gPhoneMgr->pretime != 0)
		{
			T_U32 curtime = Fwl_GetTickCountMs();
			
			if((curtime > gPhoneMgr->pretime && (curtime - gPhoneMgr->pretime > 300))
				||(curtime < gPhoneMgr->pretime && (curtime + (T_U32_MAX - gPhoneMgr->pretime) > 300)))
			{
				gPhoneMgr->pretime = curtime;
			}
			else
			{
				return;
			}
		}
		else
		{
			gPhoneMgr->pretime = Fwl_GetTickCountMs();
		}
		if((gPhoneMgr->SpeakerVolume < (16 -1)) && (AK_TRUE == add))//当已经最大音量或者最小音量时不设置
		{
			BtPhone_SetSpeakerGain(gPhoneMgr->SpeakerVolume + 1);
			BA_HFP_UpdateSpeakerVolume(gPhoneMgr->SpeakerVolume);
		}
		else if((gPhoneMgr->SpeakerVolume != 0) && (AK_FALSE == add))
		{
			BtPhone_SetSpeakerGain(gPhoneMgr->SpeakerVolume - 1);
			BA_HFP_UpdateSpeakerVolume(gPhoneMgr->SpeakerVolume);
		}
	}
}
T_VOID BtPhone_SetSpeakerGain(T_U8 vol)
{
	if(AK_NULL != gPhoneMgr)
	{
		gPhoneMgr->SpeakerVolume = vol;
		if(AK_NULL != gPhoneMgr->AEChandle)
		{
			//调用agc的音量设置接口;
			AECLib_SetDaVolume(gPhoneMgr->AEChandle,vol<<8);
		}
	}
}


T_VOID AAkDebugOutput(T_pCSTR s, ...);

/**
 * @BRIEF	deinit aec lib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID AECLib_DeInit(T_VOID)
{	
	if(AK_NULL != gPhoneMgr && AK_NULL != gPhoneMgr->AEChandle)
	{
		AECLib_Close(gPhoneMgr->AEChandle);
		_SD_Filter_Close(gPhoneMgr->AECFilter);
		gPhoneMgr->AEChandle = AK_NULL;
		
		akerror("AEC Close OK",0,1);
	}
}

/**
 * @BRIEF	a callback function for aec lib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_U32 event
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID  AECLib_Notify(T_U32 event)
{
}

/**
 * @BRIEF	init aec lib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID AECLib_Init(T_U8 vol)
{
	T_AEC_INPUT p_aecin;
    T_AUDIO_FILTER_INPUT s_ininfo; 
	
	#define PHONE_MIN_NOISE_VALUE  		150000 //50000<=PHONE_MIN_NOISE_VALUE<400000	
	#define NN 128
	#define TAIL (NN*5)
	
	if (AK_NULL != gPhoneMgr->AEChandle)
	{
		return ;
	}
	memset(&p_aecin, 0, sizeof(p_aecin));
	p_aecin.cb_fun.Malloc = Fwl_Malloc;
	p_aecin.cb_fun.Free = Fwl_Free;
	p_aecin.cb_fun.printf = AkDebugOutput;
	p_aecin.m_info.m_Type = AEC_TYPE_1;
	p_aecin.m_info.m_BitsPerSample = 16;
	p_aecin.m_info.m_Channels = 1;
	p_aecin.m_info.m_SampleRate = 8000;
	p_aecin.m_info.m_Private.m_aec.m_framelen = NN;
	p_aecin.m_info.m_Private.m_aec.m_tail = TAIL;
    p_aecin.cb_fun.notify = AECLib_Notify;	
	/**********回音消除参数配置***********/
	p_aecin.m_info.m_Private.m_aec.m_aecBypass = 0;   // 1：不做回音消除（即把EchoS bypass掉）；0：做回音消除
	p_aecin.m_info.m_Private.m_aec.m_PreprocessEna = 1; //控制是否做降噪和AGC。1：做AGC； 0：不做agc
	p_aecin.m_info.m_Private.m_aec.AGClevel = 16384+4096;
	p_aecin.m_info.m_Private.m_aec.DacVolume = vol<<8;
	p_aecin.m_info.m_Private.m_aec.AdcMinSpeechPow = 1280; // 10C上，应该要改成768
	p_aecin.m_info.m_Private.m_aec.DacSpeechHoldTime = 900;
	p_aecin.m_info.m_Private.m_aec.AdcSpeechHoldTime = 900;
	p_aecin.m_info.m_Private.m_aec.maxGain = 3;
	p_aecin.m_info.m_Private.m_aec.AdcConvergTime = 10000;
	p_aecin.m_info.m_Private.m_aec.DacConvergTime = 10000;
	p_aecin.m_info.m_Private.m_aec.AdcCutTime = 400;
	p_aecin.m_info.m_Private.m_aec.voiceEqNear = 0;	//设置是否将滤波之后的数据送给远端
	/*
	  当双方都不说话的时候，选择是传远端还是近端声音给对方，即尽量保证哪方声音的连续性
	  0：尽量保证远端给近端数据的连续性，即只要近端不说话，喇叭就播放远端过来的声音。
		 总体感觉较好，只是远端听到的近端声音可能有点断续。
		 但是只适合腔体回声较小的模具。
	  1: 尽量保证近端给远端数据的连续性，即只要远端不说话，就将近端声音传给远端。
		 可能导致近端喇叭播放的声音有点断续，且远端切换到近端说话的耗时稍长。
		 对于例如球形等腔体回声较大的模具，可以选择这个参数。
	  默认值是0
	*/	
	//p_aecin.m_info.m_Private.m_aec.continueWay = 0;
	/*********************end***********************/
	
	gPhoneMgr->AEChandle = AECLib_Open(&p_aecin);
	
    if (AK_NULL == gPhoneMgr->AEChandle)
	{
        akerror("AEC Open Fail",0,1);
		return;
	}	
	
    s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
    s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
    s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)AAkDebugOutput;     
    s_ininfo.m_info.m_BitsPerSample = 16;
    s_ininfo.m_info.m_Channels = 1;
    s_ininfo.m_info.m_SampleRate = Fwl_GetWaveInRealSample(); //歌曲本身采样率
    s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE ;
    s_ininfo.m_info.m_Private.m_resample.maxinputlen = 256;
    s_ininfo.m_info.m_Private.m_resample.outSrindex = 0; 
    s_ininfo.m_info.m_Private.m_resample.outSrFree = Fwl_GetWaveOutRealSample(); // 我们DAC的实际采样率
    s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; //1是新算法，比较省内存；0是老算法，比较耗内存。
    gPhoneMgr->AECFilter = (T_HANDLE)_SD_Filter_Open(&s_ininfo);
	
	akerror("AEC Open OK",0,1);

}

#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
/**
 * @BRIEF	init resample lib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_U16 SrcRate
 			T_U32 DestRate
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_InitReSmaple(T_U16 SrcRate, T_U32 DestRate)
{ 
    T_AUDIO_FILTER_INPUT s_ininfo; 
	
	if(AK_NULL != gPhoneMgr)
	{
	    s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
	    s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
	    s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)AAkDebugOutput;     
	    s_ininfo.m_info.m_BitsPerSample = 16;
	    s_ininfo.m_info.m_Channels = 1;
	    s_ininfo.m_info.m_SampleRate = SrcRate; //歌曲本身采样率
	    s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE ;
	    s_ininfo.m_info.m_Private.m_resample.maxinputlen = 120;
	    s_ininfo.m_info.m_Private.m_resample.outSrindex = 0; 
	    s_ininfo.m_info.m_Private.m_resample.outSrFree = DestRate; // 我们DAC的实际采样率
	    s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; //1是新算法，比较省内存；0是老算法，比较耗内存。
		AkDebugOutput("BtPhone_InitReSmaple: src:%d, dest:%d\n",SrcRate, DestRate);
	    gPhoneMgr->SCOFilter = (T_HANDLE)_SD_Filter_Open(&s_ininfo);
	}
}

/**
 * @BRIEF	deinit resample lib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_DeInitReSmaple(T_VOID)
{ 
	if(AK_NULL != gPhoneMgr)
    {
        _SD_Filter_Close((T_VOID *)gPhoneMgr->SCOFilter);
        gPhoneMgr->SCOFilter = (T_HANDLE)AK_NULL;
    }
}

#pragma arm section code =  "_SYS_BLUE_HFP_CODE_"
/**
 * @BRIEF	resample data
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_U32 SrcLen
			 T_U8 *SrcData
			 T_U8 *DestData
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_U32 BtPhone_ReSmaple(T_U32 SrcLen, T_U8 *SrcData, T_U8 *DestData)
{
    if(gPhoneMgr->SCOFilter)
    {
		#if 0
   		T_AUDIO_FILTER_BUF_STRC fbuf_strc; 
	    fbuf_strc.buf_in = SrcData;
	    fbuf_strc.buf_out = DestData; 
	    fbuf_strc.len_in = SrcLen; 
	    fbuf_strc.len_out = AD_SEND_BUF_SIZE << 1;
	    return _SD_Filter_Control((T_VOID *)gPhoneMgr->SCOFilter, &fbuf_strc); 
		#else
		return _SD_Filter_Audio_Scale((T_VOID *)gPhoneMgr->SCOFilter, DestData, SrcData , SrcLen);
		#endif
    }
    return 0;
}
#pragma arm section code
#endif

#pragma arm section code = "_SYS_BLUE_HFP_CODE_"
/**
 * @BRIEF	check if we are in htphone status
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_IsWorking(T_VOID)
{
	return (gPhoneMgr != 0);
}
#pragma arm section code

/**
 * @BRIEF	init btphone status mechine
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_HANDLE BtPhone_Init(T_VOID)
{
	T_PCM_INFO info;
	T_RECORD_INFO RecordInfo;

	if(AK_NULL != gPhoneMgr)
	{
		return (T_HANDLE)gPhoneMgr;
	}
	gPhoneMgr = Fwl_Malloc(sizeof(T_BTHFP_PHONEMGR) + SEND_SCO_BUF_SIZE + (AD_SEND_BUF_SIZE<<1));
	if(AK_NULL == gPhoneMgr)
	{
		akerror("HFP init fail",0,1);
		return 0;
	}
	gPhoneMgr->pretime = 0;
	gPhoneMgr->AEChandle = AK_NULL;
	gPhoneMgr->AECFilter = AK_NULL;
	gPhoneMgr->SendBuf = (T_U8*)gPhoneMgr + sizeof(T_BTHFP_PHONEMGR);
	gPhoneMgr->SCOFilterBuf = gPhoneMgr->SendBuf + SEND_SCO_BUF_SIZE;

	gPhoneMgr->pcmUnitSize = 0;

	gPhoneMgr->PcmPly_ID = Pcm_Open();
	gPhoneMgr->PcmRec_ID = Pcm_Record_Open(MIC_ADC,AK_FALSE);
	gPhoneMgr->EnableAec = AK_FALSE;
	gPhoneMgr->SpeakerVolume = BTPHONE_BASE_VOICE_VALUE - 1;
	gPhoneMgr->SendEndPos = 0;
	gPhoneMgr->SendCurPos = 0;
#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
	gPhoneMgr->SCOFilter = AK_NULL;
#endif
	BtPhone_SetSpeakerVolume(AK_TRUE);
	//phone_SetMicVolume(8);
	info.bps = 16;
	info.bufsize = DA_DMA_BUF_SIZE;
	info.channel = 1;
	info.samplerate = 8000;
	Pcm_Start(gPhoneMgr->PcmPly_ID,&info);
	
	RecordInfo.bps = 16;
	RecordInfo.bufsize = AD_DMA_BUF_SIZE;
	RecordInfo.channel = 2;
	RecordInfo.samplerate = 8000;
	Pcm_Record_Start(gPhoneMgr->PcmRec_ID,&RecordInfo);

	#ifdef OS_WIN32
    Audio_WINOpen(8000, 1, 5120, 16);
	#endif
	{
		//通常是资源开始才允许sco数据接通
		BA_BypassSCORxData(AK_FALSE);
	}
	return (T_HANDLE)gPhoneMgr;

}

/**
 * @BRIEF	deinit btphone status mechine
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_VOID BtPhone_DeInit(T_VOID)
{
	if(AK_NULL == gPhoneMgr)
	{
		return ;
	}
	
	BA_BypassSCORxData(AK_TRUE);
	gPhoneMgr->pcmUnitSize = 0;
	Pcm_Stop(gPhoneMgr->PcmPly_ID);
	Pcm_Close(gPhoneMgr->PcmPly_ID);
	Pcm_Record_Stop(gPhoneMgr->PcmRec_ID);
	Pcm_Record_Close(gPhoneMgr->PcmRec_ID);
	
	AECLib_DeInit();
#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
	BtPhone_DeInitReSmaple();
#endif
	Fwl_Free(gPhoneMgr);
#ifdef OS_WIN32
	Audio_WINClose();
#endif
	gPhoneMgr = AK_NULL;
}

/**
 * @BRIEF	when phone is ongoing, btphone accept,we reset ad and da info
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_Accept(T_VOID)
{
	//T_RECORD_INFO info;
	T_PCM_INFO Pcminfo;

	if(gPhoneMgr->pcmUnitSize != AD_SEND_BUF_SIZE)
	{
		//info.bps = 16;
		//info.bufsize = 0;
		//info.channel = 2;
		//info.samplerate = 8000;

		Pcminfo.bps = 16;
		Pcminfo.bufsize = 0;
		Pcminfo.channel = 1;
		Pcminfo.samplerate = 8000;
		
		Pcm_ReStart(gPhoneMgr->PcmPly_ID,&Pcminfo);
		//Pcm_Record_ReStart(gPhoneMgr->PcmRec_ID,&info);
		#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
		BtPhone_InitReSmaple(8000, Fwl_GetWaveOutRealSample());
		#endif
		AECLib_Init(gPhoneMgr->SpeakerVolume);
		gPhoneMgr->pcmUnitSize = AD_SEND_BUF_SIZE;
	}
}

#pragma arm section code = "_bootcode1_"

/**
 * @BRIEF	it will send data to AEC lib
 * @AUTHOR	BtPhone_CheckVoice
 * @DATE	2013-05-21
 * @PARAM	T_U8 *src
 			T_U32 sampSize
 			T_BOOL AD_DA_Flag	ak_true for da, false for ad
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_S32 BtPhone_CheckVoice(T_U8 *src, T_U32 sampSize, T_BOOL AD_DA_Flag)
{
	if((AK_NULL == gPhoneMgr) || (0 == gPhoneMgr->pcmUnitSize) || (AK_NULL == gPhoneMgr->AEChandle))
	{
		return -1;
	}
	
	if(AD_DA_Flag)
	{
		AECLib_DacInt(gPhoneMgr->AEChandle, src, sampSize);
		gPhoneMgr->EnableAec = AK_TRUE;
	}
	else if(gPhoneMgr->EnableAec == AK_TRUE)
	{
    	AECLib_AdcInt(gPhoneMgr->AEChandle, src, sampSize);
	}

    return sampSize;
}

/**
 * @BRIEF	it will resample ad data to AEC lib
 * @AUTHOR	BtPhone_ADReSmaple
 * @DATE	2013-05-21
 * @PARAM	T_U32 SrcLen
 			T_U8 *SrcData
 			T_U32 DestLen
 			T_U8 *DestData
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_U32 BtPhone_ADReSmaple(T_U32 SrcLen, T_U8 *SrcData, T_U32 DestLen, T_U8 *DestData)
{
	if(AK_NULL != gPhoneMgr->AECFilter)
    {
		#if 0
    	T_AUDIO_FILTER_BUF_STRC fbuf_strc; 
        fbuf_strc.buf_in = SrcData;
        fbuf_strc.buf_out = DestData; 
        fbuf_strc.len_in = SrcLen; 
        fbuf_strc.len_out = DestLen;
        return _SD_Filter_Control((T_VOID *)gPhoneMgr->AECFilter, &fbuf_strc); 
		#else
		return _SD_Filter_Audio_Scale((T_VOID *)gPhoneMgr->AECFilter, DestData, SrcData , SrcLen);
		#endif
    }
	else
	{
		memcpy(DestData, SrcData, SrcLen);
    	return SrcLen;
	}
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_CODE_"
T_U32 BtPhone_GetSendBufFreeSize()
{
	return SEND_SCO_BUF_SIZE - gPhoneMgr->SendEndPos;
}

T_U32 BtPhone_GetSendBufSize()
{
	return gPhoneMgr->SendEndPos - gPhoneMgr->SendCurPos;
}

T_U8 *BtPhone_GetSendBuf(T_U32 size)
{
	T_U8 *CurBufPos = AK_NULL;
	
	CurBufPos = gPhoneMgr->SendBuf + gPhoneMgr->SendCurPos;
	gPhoneMgr->SendCurPos += size;
	if(gPhoneMgr->SendEndPos < gPhoneMgr->SendCurPos + size)
	{
		if(gPhoneMgr->SendEndPos != gPhoneMgr->SendCurPos)
		{
			memcpy(gPhoneMgr->SendBuf, gPhoneMgr->SendBuf + gPhoneMgr->SendCurPos, gPhoneMgr->SendEndPos - gPhoneMgr->SendCurPos);
		}
		gPhoneMgr->SendEndPos = gPhoneMgr->SendEndPos - gPhoneMgr->SendCurPos;
		gPhoneMgr->SendCurPos = 0;
	}

	return CurBufPos;
}

/**
 * @BRIEF	btphone main process
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_Handle(T_VOID)
{
	T_S32 len = 0;
	
	if(AK_NULL != gPhoneMgr)
	{
	#ifdef SUPPORT_VOICE_TIP
		if(!Voice_IsWroking() && !BtDev_IsPlayingRing())
	#endif
		{
			if(BtPhone_GetSendBufFreeSize() >= AEC_ONE_SIZE)
			{
				T_U8 *AECbuf = gPhoneMgr->SendBuf + gPhoneMgr->SendEndPos;
				
				if(AK_NULL != gPhoneMgr->AEChandle)
				{
					T_AEC_BUF p_aecbufs;

					p_aecbufs.buf_out = AECbuf;
					p_aecbufs.len_out = AEC_ONE_SIZE;
					len = AECLib_Control(gPhoneMgr->AEChandle, &p_aecbufs);
				}
				
				if(len > 0)
				{
					gPhoneMgr->SendEndPos += len;
				}
			}
			
			if(Pcm_GetPcmSize() > (DA_DMA_BUF_SIZE >> 1))
			{
				Pcm_Playing(gPhoneMgr->PcmPly_ID);
			}
			if(AK_TRUE == Fwl_UartGetTxStatus() && gPhoneMgr->pcmUnitSize && (BtPhone_GetSendBufSize() >= gPhoneMgr->pcmUnitSize))
			{
				BA_HFP_SendDataSCO(BtPhone_GetSendBuf(gPhoneMgr->pcmUnitSize), gPhoneMgr->pcmUnitSize);
			}
		}

	}

}
#pragma arm section code


/**
 * @BRIEF	accept the incoming phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_AnswerCall(T_VOID)
{
	return BA_HFP_AnswerCall();
}

/**
 * @BRIEF	reject the incoming phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_RejectCall(T_VOID)
{
	return BA_HFP_CancelCall();
}

/**
 * @BRIEF	cancel the outgoing phone or ongoing phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_CancelCall(T_VOID)
{
	return BA_HFP_CancelCall();
}

/**
 * @BRIEF	dial the pointed phone with (psz != ak_null) or dial the last outgo phone with (psz == ak_null)
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_BOOL BtPhone_CallDial(T_pCDATA psz)
{
	return BA_HFP_CallDial(psz);
}
#pragma arm section code = "_SYS_BLUE_HFP_CODE_"

/**
 * @BRIEF	get current phone status (standby,incoming,outgoing,ongoing)
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_U32
 * @RETVAL	
 */
T_U8 BtPhone_GetPhoneStatus(T_VOID)
{
	return BA_HFP_GetCurrentStatus();
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_HFP_CODE_"



T_U8* BtPhone_CheckFarNoice(T_U8 *data, T_U16 data_len)
{
	#if 1
	T_AEC_BUF p_aecbufs={0};
	
	if(gPhoneMgr->AEChandle &&gPhoneMgr->EnableAec)
	{
		p_aecbufs.buf_far = data;
		p_aecbufs.len_far = data_len;
		p_aecbufs.buf_out = gPhoneMgr->SCOFilterBuf;
		p_aecbufs.len_out = data_len;
		AECLib_FarPreprocess(gPhoneMgr->AEChandle, &p_aecbufs);
		return gPhoneMgr->SCOFilterBuf;
	}
	else
		return data;
	#else
	return data;
	#endif
}




/**
 * @BRIEF	get remote sco data
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_U8 *data
 			T_U16 data_len//sco:60,esco:120
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_LoadSCOData(T_U8 *data, T_U16 data_len)
{	
	T_U8 * result;
	if(Pcm_GetFreeSize() < (data_len<<1))
	{
		//如果下次过来不够存放数据了，就暂停不在进行process	
		BA_ExitProcess();
	}
	result = BtPhone_CheckFarNoice(data, data_len);
	#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
	data_len = BtPhone_ReSmaple(data_len, data, gPhoneMgr->SCOFilterBuf);
	Pcm_Write(gPhoneMgr->SCOFilterBuf, data_len);
	#else
	Pcm_Write(result, data_len);
	#endif
}


#pragma arm section code


#endif
