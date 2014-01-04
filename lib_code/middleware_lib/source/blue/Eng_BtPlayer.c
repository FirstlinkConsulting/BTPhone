/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_bt_player.c
 * @BRIEF a2dp process 
 * @Author：zhuangyuping
 * @Date：2013-04-25
 * @Version：
**************************************************************************/


#include "Eng_BtPlayer.h"
#include "Fwl_MicroTask.h"
#include "Fwl_osMalloc.h"
#include <string.h>
#include "ba_a2dp.h"
#include "ba_hfp.h"
#include "ba_avrcp.h"
#include "ba_gap.h"
#include "eng_debug.h"
#include "audio_decoder.h"
#include "sdfilter.h"
#include "fwl_timer.h"
#include "Eng_VoiceTip.h"

#ifdef SUPPORT_BLUETOOTH


/********data********/
T_BTA2DP_PLAYER *gA2DPPlayer = AK_NULL;

/********macro definition********/
#define RESAMPLE_BUF_SIZE 512
#define BLUE_A2DP_EQ_MODE _SD_EQ_MODE_NORMAL
#define BLUE_A2DP_EQ_MODE_NUM _SD_EQ_USER_DEFINE

#define BTPLAYER_DEFAULTGAIN 31//默认最大声



#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
T_U8 gA2DPEQMode = BLUE_A2DP_EQ_MODE;

T_VOID BtPlayer_SetEQMode(T_U8 mode)
{
	gA2DPEQMode = mode;
}
T_U8 BtPlayer_GetEQMode( T_VOID)
{
	return gA2DPEQMode;
}
#endif

/********function********/
T_VOID AAkDebugOutput(T_pCSTR s, ...)
{
}



#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
#define EQAkDebugOutput AAkDebugOutput
#pragma arm section code = "_SYS_BLUE_A2DP_INIT_CODE_"


T_VOID BtPlayer_InitEQ(T_VOID)
{ 
    T_AUDIO_FILTER_INPUT s_ininfo; 
	
	if(AK_NULL != gA2DPPlayer)
	{
	    s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
	    s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
	    s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)EQAkDebugOutput;     
	    s_ininfo.m_info.m_BitsPerSample = 16;
	    s_ininfo.m_info.m_Channels = 2;
	    s_ininfo.m_info.m_SampleRate = 44100; //歌曲本身采样率
	    s_ininfo.m_info.m_Type = _SD_FILTER_EQ;
	    s_ininfo.m_info.m_Private.m_eq.eqmode = gA2DPEQMode;
	//    s_ininfo.m_info.m_Private.m_eq.bands = 0; 
	//    s_ininfo.m_info.m_Private.m_eq.bandfreqs[0] = 0; 
	//    s_ininfo.m_info.m_Private.m_eq.bandgains[0] = 0;
		AkDebugOutput("default EQ_Mode: %d\n", gA2DPEQMode);

	    gA2DPPlayer->EQHandle = _SD_Filter_Open(&s_ininfo);
	}
}
T_VOID BtPlayer_SetEQ(T_VOID)
{ 
    T_AUDIO_FILTER_IN_INFO setinfo; 
	if(AK_NULL != gA2DPPlayer)
    {
    	gA2DPEQMode = (gA2DPEQMode+1)%BLUE_A2DP_EQ_MODE_NUM;
        setinfo.m_SampleRate = gA2DPPlayer->CurDAC.samplerate;
        setinfo.m_BitsPerSample = gA2DPPlayer->CurDAC.bps;
        setinfo.m_Channels = gA2DPPlayer->CurDAC.channel;
		setinfo.m_Private.m_eq.eqmode = gA2DPEQMode;
        _SD_Filter_SetParam(gA2DPPlayer->EQHandle, &setinfo);
        AkDebugOutput("set EQ_Mode: %d\n", gA2DPEQMode);
    }
}
#pragma arm section code

T_VOID BtPlayer_DeInitEQ(T_VOID)
{ 
    if(AK_NULL != gA2DPPlayer)
    {
        _SD_Filter_Close(gA2DPPlayer->EQHandle);
        gA2DPPlayer->EQHandle = AK_NULL;
    }
}

#pragma arm section code =  "_SYS_BLUE_A2DP_CODE_"

T_U32 BtPlayer_EQWork(T_U8 *data, T_U16 len)
{ 
	T_AUDIO_FILTER_BUF_STRC filter;

    if(AK_NULL != gA2DPPlayer)
    {
    	filter.buf_in = data;
		filter.buf_out = data;
		filter.len_in = len;
		filter.len_out = len;
		filter.buf_in2 = AK_NULL;
		filter.len_in2 = 0;
        return _SD_Filter_Control(gA2DPPlayer->EQHandle,&filter);
    }
	else
	{
		return len;
	}
}
#pragma arm section code

#endif

#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN
#define ReAkDebugOutput AAkDebugOutput
#pragma arm section code = "_SYS_BLUE_A2DP_INIT_CODE_"
T_VOID BtPlayer_InitReSmaple(T_VOID)
{ 
    T_AUDIO_FILTER_INPUT s_ininfo; 
	
	if(AK_NULL != gA2DPPlayer)
	{
	    s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
	    s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
	    s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)ReAkDebugOutput;     
	    s_ininfo.m_info.m_BitsPerSample = 16;
	    s_ininfo.m_info.m_Channels = 2;
	    s_ininfo.m_info.m_SampleRate = 44100; //歌曲本身采样率
	    s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE ;
	    s_ininfo.m_info.m_Private.m_resample.maxinputlen = 512;
	    s_ininfo.m_info.m_Private.m_resample.outSrindex = 0; 
	    s_ininfo.m_info.m_Private.m_resample.outSrFree = 44170; // 我们DAC的实际采样率
	    s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; //1是新算法，比较省内存；0是老算法，比较耗内存。

	    gA2DPPlayer->ReDAHandle = (T_HANDLE)_SD_Filter_Open(&s_ininfo);
	}
}
#pragma arm section code

T_VOID BtPlayer_DeInitReSmaple(T_VOID)
{ 
	if(AK_NULL != gA2DPPlayer)
    {
        _SD_Filter_Close((T_VOID *)gA2DPPlayer->ReDAHandle);
        gA2DPPlayer->ReDAHandle = (T_HANDLE)AK_NULL;
    }
}

#pragma arm section code =  "_SYS_BLUE_A2DP_CODE_"
T_VOID BtPlayer_SetReSmaple(T_PCM_INFO *info)
{ 
    T_AUDIO_FILTER_IN_INFO setinfo; 
	
	if(AK_NULL != gA2DPPlayer)
    {
        setinfo.m_SampleRate = info->samplerate;
        setinfo.m_Private.m_resample.outSrFree = Fwl_GetWaveOutRealSample();
        setinfo.m_BitsPerSample = 16;
        setinfo.m_Channels = 2; 
        _SD_Filter_SetParam((T_VOID *)gA2DPPlayer->ReDAHandle, &setinfo);
        AkDebugOutput("set resample: src:%d, dest:%d\n", info->samplerate, setinfo.m_Private.m_resample.outSrFree);
    }
}

T_U32 BtPlayer_ReSmaple(T_U32 SrcLen, T_U8 *data)
{
    
	if(AK_NULL != gA2DPPlayer)
    {
		#if 0
    	T_AUDIO_FILTER_BUF_STRC fbuf_strc; 
        fbuf_strc.buf_in = data;
        fbuf_strc.buf_out = data; 
        fbuf_strc.len_in = SrcLen; 
        fbuf_strc.len_out = BTA2DP_DEC_BUFLEN;
        return _SD_Filter_Control((T_VOID *)gA2DPPlayer->ReDAHandle, &fbuf_strc); 
		#else
		return _SD_Filter_Audio_Scale((T_VOID *)gA2DPPlayer->ReDAHandle, data, data, SrcLen);
		#endif
    }
    return SrcLen;
}
#pragma arm section code
#endif



/**
 * @BRIEF	deinit btplayer decoder when phone comes or a2dp disconnect
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_DeInit(T_VOID)
{
	if(AK_NULL == gA2DPPlayer)
	{
		return;
	}
	Fwl_MicroTaskPause(gA2DPPlayer->DecTask_ID);   
	Fwl_MicroTaskUnRegister(gA2DPPlayer->DecTask_ID);
	
	Pcm_Stop(gA2DPPlayer->PcmPly_ID);
	Pcm_Close(gA2DPPlayer->PcmPly_ID);
#ifdef OS_WIN32    
	Audio_WINClose(); 
#endif
#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN
	BtPlayer_DeInitReSmaple();
#endif
#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
	BtPlayer_DeInitEQ();
#endif
	AudDecoder_Close(gA2DPPlayer->decHdl);

	Fwl_Free(gA2DPPlayer);
	gA2DPPlayer = AK_NULL;
}

#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/**
 * @BRIEF	check if we have init a btplayer and decoder 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPlayer_IsWorking(T_VOID)
{
	return (0 != gA2DPPlayer);
}
#pragma arm section code



#pragma arm section code = "_SYS_BLUE_A2DP_INIT_CODE_"

/**
 * @BRIEF	init btplayer decoder when a2dp start msg comes
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_HANDLE BtPlayer_Init(T_U32 dec_volume)
{
	if(gA2DPPlayer)
	{
		return (T_HANDLE)gA2DPPlayer;
	}
	
	gA2DPPlayer = (T_BTA2DP_PLAYER *)Fwl_Malloc(sizeof(T_BTA2DP_PLAYER) + BTA2DP_DEC_BUFLEN);
	if(AK_NULL == gA2DPPlayer)
	{
		return 0;
	}
	memset(gA2DPPlayer,0,sizeof(T_BTA2DP_PLAYER));
	//解码缓冲区中转，用于传入解码器，然后做回音消除,
	//如果不需要重采样可能不需要这个buf
	gA2DPPlayer->DecBuf = (T_U8*)gA2DPPlayer + sizeof(T_BTA2DP_PLAYER);
	gA2DPPlayer->decHdl = AudDecoder_Open(_SD_MEDIA_TYPE_SBC, AK_TRUE, dec_volume);
	if(0 == gA2DPPlayer->decHdl)
	{
		Fwl_Free(gA2DPPlayer);
		return 0;
	}
    gA2DPPlayer->PcmPly_ID = Pcm_Open();
	if(INVAL_WAVEOUT_ID == gA2DPPlayer->PcmPly_ID)
	{
		AudDecoder_Close(gA2DPPlayer->decHdl);
		Fwl_Free(gA2DPPlayer);
		return 0;
	}
	gA2DPPlayer->isMuteNow = AK_TRUE;
	gA2DPPlayer->CurDAC.bps = 16;
	gA2DPPlayer->CurDAC.bufsize = 8 * 1024; 
	gA2DPPlayer->CurDAC.channel = 2;
	gA2DPPlayer->CurDAC.samplerate = 48000; //默认值
	Pcm_Start(gA2DPPlayer->PcmPly_ID, &gA2DPPlayer->CurDAC);
	
	BtPlayer_RegisterTask();
#ifdef OS_WIN32
    Audio_WINOpen(48000, 2, 5120, 16);//，默认值，第一个参数宁大勿小，因为仿真中大一点效果是卡，但是小了，会丢包
#endif
#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN
	BtPlayer_InitReSmaple();		   
	BtPlayer_SetReSmaple(&gA2DPPlayer->CurDAC);
#endif
#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
	BtPlayer_InitEQ();
#endif
	
	return (T_HANDLE)gA2DPPlayer;
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"

/**
 * @BRIEF	when a2dp comes from bluea message,we load its a2dp data ,and save into dec buf
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 * data
 			T_U16 data_len
 * @RETURN	T_VOID
 * @RETVAL	
 */
 #define MUTE_TIME_NO_SBC_DATA	1000
T_VOID BtPlayer_LoadSBCData(T_U8 *data, T_U16 data_len)
{
	T_U32 freeLen;

	AudDecoder_GetFreeLen(gA2DPPlayer->decHdl,&freeLen);
	if(freeLen < (data_len<<1))
	{
		//如果下次process过来不够解码缓冲区存储将中止执行process
		BA_ExitProcess();
	}
	
	gA2DPPlayer->SBCSysComeTime = Fwl_GetTickCountMs();
	AudDecoder_AddSrc(gA2DPPlayer->decHdl, data, data_len);
}

/**
 * @BRIEF	for a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPlayer_Decode(T_VOID)
{
	T_U32 PcmLen;
	T_U32	oldSample,newSample;
	T_U32 i;
	
	if(AK_NULL != gA2DPPlayer)
	{	
		if(Fwl_GetTickCountMs() > (MUTE_TIME_NO_SBC_DATA + gA2DPPlayer->SBCSysComeTime))
		{
			BtPlayer_SetDecing(AK_FALSE);
			BtPlayer_SetPlaying(AK_FALSE);
			return ;
		}	
		BtPlayer_SetPlaying(!AudDecoder_CheckOutput(gA2DPPlayer->decHdl, AK_FALSE));

		for(i = 0; i< 100;i++)
		{
			if(BTA2DP_DEC_BUFLEN > Pcm_GetFreeSize())
			{
				Pcm_Playing(gA2DPPlayer->PcmPly_ID);
				break;
			}
			//备份旧的播放率，以便对比更新
			oldSample =  gA2DPPlayer->CurDAC.samplerate;
			//解码并写入环形缓冲区
			PcmLen = AudDecoder_DecFrame(gA2DPPlayer->decHdl, gA2DPPlayer->DecBuf, BTA2DP_DEC_LEN);
			
			if(PcmLen)
			{
				#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN 			   
				PcmLen = BtPlayer_ReSmaple(PcmLen, gA2DPPlayer->DecBuf);
				#endif
				#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
				PcmLen = BtPlayer_EQWork(gA2DPPlayer->DecBuf, PcmLen);
				#endif
				Pcm_Write(gA2DPPlayer->DecBuf, PcmLen);
			}
			else
			{
				Pcm_Playing(gA2DPPlayer->PcmPly_ID);
				break;
			}
			newSample =  AudDecoder_GetCurSampleRate(gA2DPPlayer->decHdl);
			if (newSample != oldSample)
	        {
				AkDebugOutput("A2DP sample is different!:new:%d,old:%d\n",newSample , oldSample);
				//发现采样率不一样时更换播放率，重新播放
				AudDecoder_GetCurDaInfo(gA2DPPlayer->decHdl, &gA2DPPlayer->CurDAC);
				gA2DPPlayer->CurDAC.bufsize = 0;
				Pcm_ReStart(gA2DPPlayer->PcmPly_ID, &gA2DPPlayer->CurDAC);
				#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN 			   
				BtPlayer_SetReSmaple(&gA2DPPlayer->CurDAC);
				#endif	
	        }
			
			Pcm_Playing(gA2DPPlayer->PcmPly_ID);
		}	
	}
}

/**
 * @BRIEF	check wether mute hp,depends on audec or current pcm data
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL isForceMute
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPlayer_AutoMute(T_BOOL isForceMute)
{
	T_U8 gain = BTPLAYER_DEFAULTGAIN;
	if(AK_NULL == gA2DPPlayer)
	{
		return;
	}
	if(gA2DPPlayer->isMuteNow != isForceMute)
	{
		gA2DPPlayer->isMuteNow = isForceMute;
		if(AK_TRUE == isForceMute)
		{
			//mute时只要将gain设置成0就可以
			AkDebugOutput("value:%d   ",OUTPUT_MUTE_THRESHOLD_DEFAULT);
			gain = 0;
		}
		
		AkDebugOutput("mute:%d\n",isForceMute);
		Fwl_WaveOutSetGain(gA2DPPlayer->PcmPly_ID,gain);
	}
}

/**
 * @BRIEF	whether a2dp play or mute
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL play
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_BOOL BtPlayer_IsAutoMute(T_VOID)
{
    if(AK_NULL == gA2DPPlayer)
	{
		return AK_FALSE;
	}
    return gA2DPPlayer->isMuteNow;
}
#pragma arm section code

/**
 * @BRIEF	set a2dp play or pause,if play is ak_true start a2dp music,or stop a2dp music
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL play
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Play(T_BOOL play)
{
	T_U8 key = play?OPID_PLAY:OPID_PAUSE;
	
	BA_AVRCP_PressKey(key);
}

/**
 * @BRIEF	select music to play,if back is AK_TRUE play the backward music,or play the forward music
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL vol
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Switch(T_BOOL back)
{
	T_U8 key = back?OPID_BACKWARD:OPID_FORWARD;
	
	BA_AVRCP_PressKey(key);
}

#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/**
 * @BRIEF	set a2dp volume,if vol if ak_TRUE,increase vol,or decrease vol
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL vol
 * @RETURN	T_VOID
 * @RETVAL	
 */

T_VOID BtPlayer_SetVolume(T_U32 playVol)
{
	if(AK_NULL != gA2DPPlayer)
	{	
		AudDecoder_SetVolume(gA2DPPlayer->decHdl, playVol);
	}
}

/**
 * @BRIEF	check if a2dp is playing 
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE if a2dp is playing ,or AK_FALSE
 */
T_BOOL BtPlayer_IsPlaying(T_VOID)
{
	if(AK_NULL != gA2DPPlayer)
	{
		return gA2DPPlayer->playing;
	}
	return AK_FALSE;
}

/**
 * @BRIEF	check if a2dp is decing 
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE if a2dp is decing ,or AK_FALSE
 */
T_BOOL BtPlayer_IsDecing(T_VOID)
{
	if(AK_NULL != gA2DPPlayer)
	{
		return gA2DPPlayer->decing;
	}
	return AK_FALSE;
}

/**
 * @BRIEF	set a2dp status,if pause a2dp,we pause pcm
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL playing
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_SetPlaying(T_BOOL playing)
{
	if(AK_NULL != gA2DPPlayer)
	{
		if(gA2DPPlayer->playing != playing)
		{
			AkDebugOutput(playing?"play\n":"pause\n");
			gA2DPPlayer->playing = playing;
			BtPlayer_AutoMute(!playing);
		}
	}
}

/**
 * @BRIEF	set a2dp dec,if pause dec,we pause microtask and so on
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL decing
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_SetDecing(T_BOOL decing)
{
	if(AK_NULL != gA2DPPlayer)
	{
#ifdef SUPPORT_VOICE_TIP
		if(Voice_IsWroking() && (AK_TRUE == decing))
		{
			return ;
		}
#endif
		if(gA2DPPlayer->decing != decing)
		{
			gA2DPPlayer->decing = decing;
			if(AK_FALSE == decing)
			{
				BtPlayer_Pause();
				Pcm_Pause(gA2DPPlayer->PcmPly_ID);			
				//AudDecoder_Clear(gA2DPPlayer->decHdl);	
			}
			else
			{
				gA2DPPlayer->CurDAC.bufsize = 0;
				Pcm_ReStart(gA2DPPlayer->PcmPly_ID, &gA2DPPlayer->CurDAC);		
				BtPlayer_Start();
			}
			AkDebugOutput(decing?"decode\n":"undecode\n");
		}
	}
}
#pragma arm section code

/**
 * @BRIEF	Register a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BTA2DP_PLAYER *player
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_RegisterTask(T_VOID)
{
	gA2DPPlayer->DecTask_ID = Fwl_MicroTaskRegister(BtPlayer_Decode, 1);   
}

/**
 * @BRIEF	start a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Start(T_VOID)
{
	Fwl_MicroTaskResume(gA2DPPlayer->DecTask_ID);   	
}

/**
 * @BRIEF	pause a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Pause(T_VOID)
{
	Fwl_MicroTaskPause(gA2DPPlayer->DecTask_ID);   
}

/**
 * @BRIEF	UnRegister a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BTA2DP_PLAYER *player
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Unregistertask(T_VOID)
{
    if (Fwl_MicroTaskUnRegister(gA2DPPlayer->DecTask_ID))              //销毁音频任务处理函数
    {
        gA2DPPlayer->DecTask_ID = 0xff;
    }
}
#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"

T_VOID BtPlayer_CheckStart(T_U32 freelen)
{
	T_U32 TotalLen;
	
	if(AK_NULL == gA2DPPlayer)
	{
		//不是a2dp不需要使用load，因为提示音和这个公用一套逻辑
		return;
	}
	
	TotalLen = BLUE_SBC_BUF_LEN;
	if(AK_FALSE == BtPlayer_IsDecing() && (freelen < (TotalLen >> 2)))
	{
		AkDebugOutput("resume:%d\n",TotalLen - freelen);	
		//数据量到达一定程度开始解码
		BtPlayer_SetDecing(AK_TRUE);
	}
	else
	{
		if(freelen > (TotalLen - (TotalLen >> 3)))
		{
			//数据量少于一定程度暂时不解码
			BtPlayer_SetDecing(AK_FALSE);
		}
		if(AK_FALSE == BtPlayer_IsDecing())
		{
			AkDebugOutput("load:%d\n",TotalLen - freelen);
		}
	}

}
#pragma arm section code

#endif
