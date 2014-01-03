#ifndef _VOICE_TIP_H_
#define _VOICE_TIP_H_

typedef T_VOID (*T_Voice_PalyEndCallBack)(T_VOID);

/*********************************************************************
  Function:     Voice_InitTip
  Description:     初始化提示音在存储设备上的信息，这个信息
  			提示音模块不会申请内存，只会保存指针地址。
  Input:            NONE
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/

T_VOID Voice_InitTip(T_VOID);

/*********************************************************************
  Function:     voice_PlayTip
  Description:     将提示音加入提示音队列，如果当前有提示音在播放，将
                       排队，之后再播放，此函数为无阻塞方式实现
  Input:            T_U32 TipType  :提示音ID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_PlayTip(T_U32 TipType, T_Voice_PalyEndCallBack pCallBack);

/*********************************************************************
  Function:     voice_PassCurTip
  Description:    将跳过当前正在播放的提示音
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassCurTip(T_VOID);

/*********************************************************************
  Function:     voice_WaitTip
  Description:    将等待当前提示音队列播放完才退出，为阻塞方式实现
  Input:            T_VOID
  Return:           T_VOID
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_WaitTip(T_VOID);

/*********************************************************************
  Function:     voice_IsWroking
  Description:   提示音是否播放完
  Input:            T_VOID
  Return:           如果提示音没有完，将返回AK_TRUE,否则返回AK_FALSE
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_BOOL Voice_IsWroking(T_VOID);

/*********************************************************************
  Function:     voice_PassAllTip
  Description:     将清空提示音队列的所有信息，并结束当前提示音的播放
  Input:           
  Author:           AIJUN
  Data:            2013-6-9
**********************************************************************/
T_VOID Voice_PassAllTip(T_VOID);

#endif

