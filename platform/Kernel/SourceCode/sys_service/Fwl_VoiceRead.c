
#include "Fwl_VoiceRead.h"
#include "VoiceMsg.h"
#include "VoiceMap.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osMalloc.h"
#include "Fwl_SysData.h"
#include "Eng_Debug.h"

typedef struct _Voice_Tip_Handle
{
	T_U32 VoiceOff;
	T_U32 VoiceSize;
	T_U32 BinOff;
	T_U32 BinFileHandle;
}T_VOICE_TIP_HANDLE, *T_PVOICE_TIP_HANDLE;

static T_VOICE_TIP_HANDLE *pVoiceTipFileHandle;

T_BOOL Fwl_VoiceOpenSondFile(T_U32 VoiceID, T_PVOICE_TIP_HANDLE pVoiceHandle)
{
	T_U32 i;

	for(i = 0; i < sizeof(bin_map)/ sizeof(T_BIN_MAP); i ++)
	{
		if(bin_map[i].VoiceID == VoiceID)
		{
			AkDebugOutput("voice file :%s\n", bin_map[i].PathFile);
			break;
		}
	}

	if(i == sizeof(bin_map)/ sizeof(T_BIN_MAP))
	{
		return AK_FALSE;
	}
	
	pVoiceHandle->BinOff = bin_map[i].OffsetFile;
	pVoiceHandle->VoiceOff = 0;
	pVoiceHandle->VoiceSize = bin_map[i].SzFile;
	return AK_TRUE;
}


/*********************************************************************
  Function:     Fwl_VoiceOpenHandle
  Description:     打开一个提示音，返回句柄，可以让系统读到提示音数据
  Input:            T_U32 VoiceID  :提示音ID
  Return:           返回提示音句柄
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_U32 Fwl_VoiceOpenHandle(T_U32 VoiceID)
{
	if(pVoiceTipFileHandle)
	{
		AkDebugOutput("VOICE MODEL:Pls close voice handle at first!\n");
		return AK_NULL;
	}
	
	pVoiceTipFileHandle = Fwl_Malloc(sizeof(T_VOICE_TIP_HANDLE));
	pVoiceTipFileHandle->BinFileHandle = Fwl_Sysdata_Open("VOICE", AK_FALSE);
	if(T_U32_MAX == pVoiceTipFileHandle->BinFileHandle)
	{
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
		AkDebugOutput("VOICE MODEL:it fails to open the bin file\n");
		return AK_NULL;
	}
	
	if(AK_FALSE == Fwl_VoiceOpenSondFile(VoiceID, pVoiceTipFileHandle))
	{
		Fwl_Sysdata_Close(pVoiceTipFileHandle->BinFileHandle);
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
		return AK_NULL;
	}
	return pVoiceTipFileHandle;
}

/*********************************************************************
  Function:     Fwl_VoiceReadHandle
  Description:     读提示音数据，给音频库解码
  Input:            T_U32 VoiceHandle  :提示音句柄
  			  T_pVOID buf :提示音缓冲器
  			  T_U32 size    :提示音缓冲器大小
  Return:           返回提示音读到的数据大小
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_U32 Fwl_VoiceReadHandle(T_U32 VoiceHandle, T_pVOID buf, T_U32 size)
{
	
	if(pVoiceTipFileHandle)
	{
		if(pVoiceTipFileHandle->VoiceOff + size >= pVoiceTipFileHandle->VoiceSize)
		{
			size = pVoiceTipFileHandle->VoiceSize - pVoiceTipFileHandle->VoiceOff;
		}
		
		if(size)
		{
			size = Fwl_Sysdata_Read(pVoiceTipFileHandle->BinFileHandle, pVoiceTipFileHandle->BinOff + pVoiceTipFileHandle->VoiceOff, size, buf);
		}
		pVoiceTipFileHandle->VoiceOff += size;
	}
	return size;
}

/*********************************************************************
  Function:     Fwl_VoiceCloseHandle
  Description:     关闭提示音句柄，释放提示音申请的内存
  Input:           T_U32 VoiceHandle  :提示音句柄
  Return:           NONE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Fwl_VoiceCloseHandle(T_U32 VoiceHandle)
{
	if(pVoiceTipFileHandle)
	{
		Fwl_Sysdata_Close(pVoiceTipFileHandle->BinFileHandle);
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
	}
}


