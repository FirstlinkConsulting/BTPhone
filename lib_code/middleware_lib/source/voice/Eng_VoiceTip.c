#ifdef SUPPORT_VOICE_TIP
#include <string.h>
#include "anyka_types.h"
#include "log_pcm_player.h"
#include "audio_decoder.h"
#include "Eng_VoiceTip.h"
#include "Fwl_VoiceRead.h"
#include "Fwl_OsMalloc.h"
#include "Fwl_MicroTask.h"
#include "Eng_Debug.h"
#include "fwl_freqMgr.h"


#define VOICE_TIP_MAX_NUM			6
#define DEFAULT_VOICE_DEC_VOL		1024

typedef struct _voice_model_ctrl
{
	volatile T_U8	VoiceTaskID;
	T_U8	VoicePcmID;
	T_U8 	VoiceTypeCache[VOICE_TIP_MAX_NUM];
	T_Voice_PalyEndCallBack pCallBackList[VOICE_TIP_MAX_NUM];
	T_Voice_PalyEndCallBack pCurCallBack;
	T_U32	CurVoiceFileHadle; 
	T_U32	DecHandle;
	T_U8	*VoiceDecBuf;
}T_VOICE_MODE_CTRL, *T_PVOICE_MODE_CTRL;

volatile T_VOICE_MODE_CTRL SysVoiceHandle;
#ifdef SUPPORT_BLUETOOTH
extern T_BOOL g_test_mode;
#endif
T_BOOL Voice_PlayNextTip(T_VOID);

/*********************************************************************
  Function:     Voice_InitTip
  Description:     初始化提示音在存储设备上的信息，这个信息
  			提示音模块不会申请内存，只会保存指针地址。
  Input:            none
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/

T_VOID Voice_InitTip(T_VOID)
{
	memset(&SysVoiceHandle, 0, sizeof(SysVoiceHandle));
	memset(SysVoiceHandle.VoiceTypeCache, 0xFF, VOICE_TIP_MAX_NUM);
	SysVoiceHandle.VoiceTaskID = 0xFF;
}

/***********************************************
将完成读SPI，并解码相关操作
************************************************/	
T_VOID Voice_Decode(T_VOID)
{
	T_U8 *DecodeBuf;
	T_U32 PcmLen, DecodeLen = 0;
	T_U32 i;

	DecodeBuf = AudDecoder_GetFreeLen(SysVoiceHandle.DecHandle, &DecodeLen);
	DecodeLen = (DecodeLen > 512)? 512:DecodeLen;
	if(DecodeLen)
	{
		DecodeLen = Fwl_VoiceReadHandle(SysVoiceHandle.CurVoiceFileHadle, SysVoiceHandle.VoiceDecBuf, DecodeLen);
		if(DecodeLen)
		{
			AudDecoder_AddSrc(SysVoiceHandle.DecHandle, SysVoiceHandle.VoiceDecBuf, DecodeLen);
		}
	}

	for(i = 0; i< 100;i++)
	{
		if(256 > Pcm_GetFreeSize())
		{
			Pcm_Playing(SysVoiceHandle.VoicePcmID);
			break;
		}
		//备份旧的播放率，以便对比更新
		//解码并写入环形缓冲区
		PcmLen = AudDecoder_DecFrame(SysVoiceHandle.DecHandle, SysVoiceHandle.VoiceDecBuf, 512);
		
		if(PcmLen)
		{
			Pcm_Write(SysVoiceHandle.VoiceDecBuf, PcmLen);
		}
		else
		{
			Pcm_Playing(SysVoiceHandle.VoicePcmID);
			if(0 == DecodeLen)
			{
				//音频文件已经读完了，可以启动下一个提示音
				if(AK_NULL != SysVoiceHandle.pCurCallBack)
				{
					SysVoiceHandle.pCurCallBack();
				}

				if(AK_FALSE == Fwl_SysMallocFlag())
				{
					Voice_PlayNextTip();
				}
			}
			break;
		}
		Pcm_Playing(SysVoiceHandle.VoicePcmID);
		//根据解码及缓冲区进行mute操作全 0
	}
	
}

/*********************************************************************
  Function:     Voice_PlayTip
  Description:     将提示音加入提示音队列，如果当前有提示音在播放，将
                       排队，之后再播放，此函数为无阻塞方式实现
  Input:            T_U32 TipType  :提示音ID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_PlayTip(T_U32 TipType, T_Voice_PalyEndCallBack pCallBack)
{
	T_U32 i;
	T_PCM_INFO pcmInfo;

#ifdef SUPPORT_BLUETOOTH
	if(g_test_mode)
	{
		return AK_TRUE;
	}
#endif	
	if(0xFF != SysVoiceHandle.VoiceTaskID)
	{
		Fwl_MicroTaskPause(SysVoiceHandle.VoiceTaskID);
	}
	
	if(SysVoiceHandle.VoiceTaskID == 0xFF)
	{
		Fwl_FreqPush(FREQ_APP_VIDEO);
		SysVoiceHandle.VoiceDecBuf = Fwl_Malloc(512);
		SysVoiceHandle.CurVoiceFileHadle = Fwl_VoiceOpenHandle(TipType);
		SysVoiceHandle.DecHandle = AudDecoder_Open(_SD_MEDIA_TYPE_SBC, AK_FALSE, DEFAULT_VOICE_DEC_VOL);
		SysVoiceHandle.VoicePcmID = Pcm_Open();
		SysVoiceHandle.pCurCallBack = pCallBack;
		pcmInfo.bps = 16;
		pcmInfo.bufsize = 4 * 1024; 
		pcmInfo.channel = 1;
		pcmInfo.samplerate = 16000; //默认值
		Pcm_Start(SysVoiceHandle.VoicePcmID, &pcmInfo);	
		SysVoiceHandle.VoiceTaskID = Fwl_MicroTaskRegister(Voice_Decode, 1); 
		#ifdef OS_WIN32
		Audio_WINOpen(16000, 2, 5120, 16);//，默认值，第一个参数宁大勿小，因为仿真中大一点效果是卡，但是小了，会丢包
		#endif 	
		Fwl_MicroTaskResume(SysVoiceHandle.VoiceTaskID);	
	}
	else
	{
		
		for(i = 0; i < VOICE_TIP_MAX_NUM; i++)
		{
			if(SysVoiceHandle.VoiceTypeCache[i] == 0xFF)
			{
				break;
			}
		}
		
		if(VOICE_TIP_MAX_NUM == i)
		{
			Fwl_MicroTaskResume(SysVoiceHandle.VoiceTaskID);
			return AK_FALSE;
		}
		SysVoiceHandle.VoiceTypeCache[i] = TipType;
		SysVoiceHandle.pCallBackList[i] = pCallBack;

		Fwl_MicroTaskResume(SysVoiceHandle.VoiceTaskID);
	}
	return AK_TRUE;
}
T_VOID Voice_PLayEnd(T_VOID)
{
	if(SysVoiceHandle.VoiceTaskID == 0xFF)
	{
		return;
	}
	
	Fwl_MicroTaskPause(SysVoiceHandle.VoiceTaskID);
	Fwl_MicroTaskUnRegister(SysVoiceHandle.VoiceTaskID);
	SysVoiceHandle.VoiceTaskID = 0xFF;
	Fwl_Free(SysVoiceHandle.VoiceDecBuf);
	Fwl_VoiceCloseHandle(SysVoiceHandle.CurVoiceFileHadle);
	SysVoiceHandle.CurVoiceFileHadle = 0;
	AudDecoder_Close(SysVoiceHandle.DecHandle);
	Pcm_Stop(SysVoiceHandle.VoicePcmID);
	Pcm_Close(SysVoiceHandle.VoicePcmID);
	#ifdef OS_WIN32    
	Audio_WINClose(); 
	#endif
	Fwl_FreqPop();
	AkDebugOutput("voice play end\n");
}

/***************************************************************
将播放下一个提示音
****************************************************************/
T_BOOL Voice_PlayNextTip(T_VOID)
{
	if(0xFF != SysVoiceHandle.VoiceTypeCache[0])
	{
		Fwl_MicroTaskPause(SysVoiceHandle.VoiceTaskID);
		Fwl_VoiceCloseHandle(SysVoiceHandle.CurVoiceFileHadle);
		AudDecoder_Clear(SysVoiceHandle.DecHandle);
		SysVoiceHandle.CurVoiceFileHadle = Fwl_VoiceOpenHandle(SysVoiceHandle.VoiceTypeCache[0]);
		SysVoiceHandle.pCurCallBack = SysVoiceHandle.pCallBackList[0];
		memmove(SysVoiceHandle.VoiceTypeCache, SysVoiceHandle.VoiceTypeCache + 1, VOICE_TIP_MAX_NUM - 1);
		memmove(SysVoiceHandle.pCallBackList, SysVoiceHandle.pCallBackList + 1, (VOICE_TIP_MAX_NUM - 1)<<2);
		SysVoiceHandle.VoiceTypeCache[VOICE_TIP_MAX_NUM - 1] = 0xFF;		
		Fwl_MicroTaskResume(SysVoiceHandle.VoiceTaskID);
	}
	else
	{
		//没有提示音了，将释放这个模块的内存
		if(Pcm_GetPcmSize() < 256)//提示音单声道，要求播完才能end，否则会有断音pipa声
		{
			Voice_PLayEnd();
		}
	}
	return AK_TRUE;
}

/*********************************************************************
  Function:     Voice_PassCurTip
  Description:    将跳过当前正在播放的提示音
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassCurTip(T_VOID)
{
	if(SysVoiceHandle.VoiceTaskID != T_U8_MAX)
	{
		Voice_PlayNextTip();
	}
}

/*********************************************************************
  Function:     Voice_WaitTip
  Description:    将等待当前提示音队列播放完才退出，为阻塞方式实现
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_WaitTip(T_VOID)
{
	while(SysVoiceHandle.VoiceTaskID != T_U8_MAX);
}
#pragma arm section code = "_bootcode1_"

/*********************************************************************
  Function:     Voice_IsWroking
  Description:   提示音是否播放完
  Input:            T_VOID
  Return:           如果提示音没有完，将返回AK_TRUE,否则返回AK_FALSE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_IsWroking(T_VOID)
{
#ifdef SUPPORT_BLUETOOTH
	if(g_test_mode)
	{
		return AK_FALSE;
	}
#endif
	return (SysVoiceHandle.VoiceTaskID != T_U8_MAX);
}
#pragma arm section code

/*********************************************************************
  Function:     Voice_PassAllTip
  Description:     将清空提示音队列的所有信息，并结束当前提示音的播放
  Input:           
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassAllTip(T_VOID)
{
	if(SysVoiceHandle.VoiceTaskID != T_U8_MAX)
	{
		AkDebugOutput("voice_Pass_all_tip\n");
		Voice_PLayEnd();
		memset(SysVoiceHandle.VoiceTypeCache, 0xFF, VOICE_TIP_MAX_NUM);
	}
}
#endif

