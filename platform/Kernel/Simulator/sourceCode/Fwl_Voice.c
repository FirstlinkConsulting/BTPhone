
#include "windows.h"

#include "Fwl_VoiceRead.h"
#include "VoiceMsg.h"
#include "VoiceMap.h"

typedef struct _Voice_Tip_Handle
{
	T_U32 BinOff;
	T_U32 VoiceOff;
	T_U32 VoiceSize;
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
			break;
		}
	}

	if(i == sizeof(bin_map)/ sizeof(T_BIN_MAP))
	{
		return AK_FALSE;
	}
	
	pVoiceHandle->BinOff = bin_map[i].OffsetFile ;
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
    T_S32 high = 0;
	if(pVoiceTipFileHandle)
	{
		AkDebugOutput("VOICE MODEL:Pls close voice handle at first!\n");
		return AK_NULL;
	}
	
	pVoiceTipFileHandle = Fwl_Malloc(sizeof(T_VOICE_TIP_HANDLE));
	pVoiceTipFileHandle->BinFileHandle = CreateFile ("..\\BoardTarget\\VOICE_BIN\\sound.bin", 
		GENERIC_READ | GENERIC_WRITE , FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(T_U32_MAX == pVoiceTipFileHandle->BinFileHandle)
	{
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
		AkDebugOutput("VOICE MODEL:it fails to open the bin file\n");
		return AK_NULL;
	}
	
	if(AK_FALSE == Fwl_VoiceOpenSondFile(VoiceID, pVoiceTipFileHandle))
	{
		CloseHandle(pVoiceTipFileHandle->BinFileHandle);
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
		return AK_NULL;
	}
	SetFilePointer(pVoiceTipFileHandle->BinFileHandle, pVoiceTipFileHandle->BinOff, &high, FILE_BEGIN);
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
	T_U32 ReadLen = 0;

	if(pVoiceTipFileHandle)
	{
		if(pVoiceTipFileHandle->VoiceOff + size >= pVoiceTipFileHandle->VoiceSize)
		{
			size = pVoiceTipFileHandle->VoiceSize - pVoiceTipFileHandle->VoiceOff;
		}
		
		if(size)
		{
			ReadFile(pVoiceTipFileHandle->BinFileHandle, buf, size, &ReadLen, NULL);
		}
		pVoiceTipFileHandle->VoiceOff += ReadLen;
	}
	return ReadLen;
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
		CloseHandle(pVoiceTipFileHandle->BinFileHandle);
		pVoiceTipFileHandle = Fwl_Free(pVoiceTipFileHandle);
	}
}


