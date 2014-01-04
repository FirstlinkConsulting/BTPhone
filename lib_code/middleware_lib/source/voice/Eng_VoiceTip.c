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
  Description:     ��ʼ����ʾ���ڴ洢�豸�ϵ���Ϣ�������Ϣ
  			��ʾ��ģ�鲻�������ڴ棬ֻ�ᱣ��ָ���ַ��
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
����ɶ�SPI����������ز���
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
		//���ݾɵĲ����ʣ��Ա�Աȸ���
		//���벢д�뻷�λ�����
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
				//��Ƶ�ļ��Ѿ������ˣ�����������һ����ʾ��
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
		//���ݽ��뼰����������mute����ȫ 0
	}
	
}

/*********************************************************************
  Function:     Voice_PlayTip
  Description:     ����ʾ��������ʾ�����У������ǰ����ʾ���ڲ��ţ���
                       �Ŷӣ�֮���ٲ��ţ��˺���Ϊ��������ʽʵ��
  Input:            T_U32 TipType  :��ʾ��ID
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
		pcmInfo.samplerate = 16000; //Ĭ��ֵ
		Pcm_Start(SysVoiceHandle.VoicePcmID, &pcmInfo);	
		SysVoiceHandle.VoiceTaskID = Fwl_MicroTaskRegister(Voice_Decode, 1); 
		#ifdef OS_WIN32
		Audio_WINOpen(16000, 2, 5120, 16);//��Ĭ��ֵ����һ������������С����Ϊ�����д�һ��Ч���ǿ�������С�ˣ��ᶪ��
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
��������һ����ʾ��
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
		//û����ʾ���ˣ����ͷ����ģ����ڴ�
		if(Pcm_GetPcmSize() < 256)//��ʾ����������Ҫ�������end��������ж���pipa��
		{
			Voice_PLayEnd();
		}
	}
	return AK_TRUE;
}

/*********************************************************************
  Function:     Voice_PassCurTip
  Description:    ��������ǰ���ڲ��ŵ���ʾ��
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
  Description:    ���ȴ���ǰ��ʾ�����в�������˳���Ϊ������ʽʵ��
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
  Description:   ��ʾ���Ƿ񲥷���
  Input:            T_VOID
  Return:           �����ʾ��û���꣬������AK_TRUE,���򷵻�AK_FALSE
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
  Description:     �������ʾ�����е�������Ϣ����������ǰ��ʾ���Ĳ���
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

