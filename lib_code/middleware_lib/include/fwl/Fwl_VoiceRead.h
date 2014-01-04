#ifndef _FWL_VOICEREAD_H_
#define _FWL_VOICEREAD_H_

#include "Anyka_types.h"
/*********************************************************************
  Function:     Fwl_VoiceOpenHandle
  Description:     打开一个提示音，返回句柄，可以让系统读到提示音数据
  Input:            T_U32 VoiceID  :提示音ID
  Return:           返回提示音句柄
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_U32 Fwl_VoiceOpenHandle(T_U32 VoiceID);
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
T_U32 Fwl_VoiceReadHandle(T_U32 VoiceHandle, T_pVOID buf, T_U32 size);
/*********************************************************************
  Function:     Fwl_VoiceCloseHandle
  Description:     关闭提示音句柄，释放提示音申请的内存
  Input:           T_U32 VoiceHandle  :提示音句柄
  Return:           NONE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Fwl_VoiceCloseHandle(T_U32 VoiceHandle);
#endif

