#ifndef _FWL_VOICEREAD_H_
#define _FWL_VOICEREAD_H_

#include "Anyka_types.h"
/*********************************************************************
  Function:     Fwl_VoiceOpenHandle
  Description:     ��һ����ʾ�������ؾ����������ϵͳ������ʾ������
  Input:            T_U32 VoiceID  :��ʾ��ID
  Return:           ������ʾ�����
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_U32 Fwl_VoiceOpenHandle(T_U32 VoiceID);
/*********************************************************************
  Function:     Fwl_VoiceReadHandle
  Description:     ����ʾ�����ݣ�����Ƶ�����
  Input:            T_U32 VoiceHandle  :��ʾ�����
  			  T_pVOID buf :��ʾ��������
  			  T_U32 size    :��ʾ����������С
  Return:           ������ʾ�����������ݴ�С
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_U32 Fwl_VoiceReadHandle(T_U32 VoiceHandle, T_pVOID buf, T_U32 size);
/*********************************************************************
  Function:     Fwl_VoiceCloseHandle
  Description:     �ر���ʾ��������ͷ���ʾ��������ڴ�
  Input:           T_U32 VoiceHandle  :��ʾ�����
  Return:           NONE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Fwl_VoiceCloseHandle(T_U32 VoiceHandle);
#endif

