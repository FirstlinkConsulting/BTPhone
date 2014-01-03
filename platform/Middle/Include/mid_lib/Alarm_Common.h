/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author£ºzhao_xiaowei 
 * @Date£º 2009-9
 * @Version£º
**************************************************************************/
#ifndef __ALARM_COMMON_H__
#define __ALARM_COMMON_H__

#include "anyka_types.h"

#if(NO_DISPLAY == 0)

#include "Ctrl_ListMenu.h"
#include "Ctrl_Dialog.h"
#include "Ctrl_Progress.h"
#include "log_aud_control.h"
#include "Gui_Common.h"

#define NORMAL_SPEED                  8
#define NORMAL_EQ                     0
#define DEF_FADE_IN                   100
#define DEF_FADE_OUT                  100

/**************************************************************************
* @brief start music play
* you must check call AlmClk_PlayerStop to stop the pre music
* and wait AlmClk_IsPlayerStopped() return AK_TURE to ensue the pre
* music is over
* @author zhao_xiaowei
* @date 2009-9
* @param filename the music play 
* @param volume the music volume
* @param isCyclePlay if cycle the music when it plays end
* @param bFade whether  it support fade out or fade in
* @return 
***************************************************************************/
T_BOOL AlmClk_PlayerStart(T_U16* fileName, T_BOOL isCyclePlay, T_U8 volume, T_U8 eq , T_U8 speed,T_U32 fideInTime, T_U32 fideOutTime);

/**************************************************************************
* @brief set the next voice play param
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL AlmClk_PlayerSet(T_U16* fileName, T_BOOL isCyclePlay, T_U8 volume, T_U8 eq , T_U8 speed,T_U32 fideInTime, T_U32 fideOutTime);


/**************************************************************************
* @brief start next , when you call AlmClk_PlayerSet to set the play info, 
* if not ,it uses the pre playing voice infomation
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL AlmClk_PlayerNext(T_VOID);
/**************************************************************************
* @brief pause the music
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_PlayerPause(T_VOID);

/**************************************************************************
* @brief resume music 
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_PlayerResume(T_VOID);

/**************************************************************************
* @brief stop music  if it support fade ,it just pauses ,when the fade music
* is over, if it not support cycle playing ,it will stop, or it play from the
* start again
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_PlayerStop(T_BOOL isForceStop);

/**************************************************************************
* @brief pause  music  if it support fade ,it just pauses ,when the fade music
* is over, if it not support cycle playing ,it will stop, or it play from the
* start again
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_PlayerPause(T_VOID);

/**************************************************************************
* @brief force it stop with no fade music playing
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_PlayerDestroy(T_VOID);

/**************************************************************************
* @brief jude the music is whether really stop
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL AlmClk_IsPlayerStopped(T_VOID);

/**************************************************************************
* @brief jude it in VOICE PLay
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL IsInVoicePlay(T_VOID);

/**************************************************************************
* @brief fix complie complain in win32
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/

T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline);
/**************************************************************************
* @brief judge the alarm voice is really playing include 
* @pre_stop pre_pause and play state
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL AlmClk_IsRealPlaying(T_VOID);

T_BOOL IsInVoicePlay(T_VOID);

#ifdef SUPPORT_SDCARD
T_MEM_DEV_ID VoiceGetCurDriver(T_VOID);
#endif

#else   //#if(NO_DISPLAY == 0)

#define IsInVoicePlay()     (AK_FALSE)
#define AlmClk_IsPlayerStopped()  (AK_TRUE)    

#endif

#endif