#ifndef _VOICE_TIP_H_
#define _VOICE_TIP_H_

typedef T_VOID (*T_Voice_PalyEndCallBack)(T_VOID);

/*********************************************************************
  Function:     Voice_InitTip
  Description:     ��ʼ����ʾ���ڴ洢�豸�ϵ���Ϣ�������Ϣ
  			��ʾ��ģ�鲻�������ڴ棬ֻ�ᱣ��ָ���ַ��
  Input:            NONE
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/

T_VOID Voice_InitTip(T_VOID);

/*********************************************************************
  Function:     voice_PlayTip
  Description:     ����ʾ��������ʾ�����У������ǰ����ʾ���ڲ��ţ���
                       �Ŷӣ�֮���ٲ��ţ��˺���Ϊ��������ʽʵ��
  Input:            T_U32 TipType  :��ʾ��ID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_PlayTip(T_U32 TipType, T_Voice_PalyEndCallBack pCallBack);

/*********************************************************************
  Function:     voice_PassCurTip
  Description:    ��������ǰ���ڲ��ŵ���ʾ��
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassCurTip(T_VOID);

/*********************************************************************
  Function:     voice_WaitTip
  Description:    ���ȴ���ǰ��ʾ�����в�������˳���Ϊ������ʽʵ��
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_WaitTip(T_VOID);

/*********************************************************************
  Function:     voice_IsWroking
  Description:   ��ʾ���Ƿ񲥷���
  Input:            T_VOID
  Return:           �����ʾ��û���꣬������AK_TRUE,���򷵻�AK_FALSE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_IsWroking(T_VOID);

/*********************************************************************
  Function:     voice_PassAllTip
  Description:     �������ʾ�����е�������Ϣ����������ǰ��ʾ���Ĳ���
  Input:           
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassAllTip(T_VOID);

#endif

