/*******************************************************************************
 * @file	s_bt_player.h
 * @brief	board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	he_yl
 * @date	2012-12-27
 * @version 1.0
*******************************************************************************/

	
#ifndef _BTPLAYER_H_
#define _BTPLAYER_H_

#include "anyka_types.h"
#include "ba_lib_api.h"
#include "log_pcm_player.h"


#define BTA2DP_DEC_BUFLEN (512 + 256)
#define BTA2DP_DEC_LEN (512)

typedef struct {
	T_U8		PcmPly_ID;
	T_U8		DecTask_ID;
	T_BOOL		isMuteNow;
	T_BOOL		decing;//表示当前本地是否正在进行微任务
	T_BOOL		playing;//表示当前远方是否正在播放
    T_HANDLE	decHdl;//解码器句柄
#ifdef SUPPORT_A2DP_RESAMPLE_CONTROL_EN
	T_HANDLE	ReDAHandle;
#endif
#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
	T_HANDLE	EQHandle;
#endif	
	//T_U32		muteReadyTime;//PCM 端自动mute检测时间，初始值0，当需要开始计算mute时进行间隔计算吗，超出间隔开始mute
    //T_U32		curSrcLen;
    //T_U32		oneFrameDecSize;
    
	T_U32		SBCSysComeTime;//上次来数据到现在的时间
	T_U8		*DecBuf;//用于解码中转，因为可能需要重采样。Pcm_Write(DecBuf,512)	
	T_PCM_INFO	CurDAC;
} T_BTA2DP_PLAYER;






/**
 * @BRIEF	when a2dp comes from bluea message,we load its a2dp data ,and save into dec buf
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 * data
 			T_U16 data_len
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_LoadSBCData(T_U8 * data, T_U16 data_len);

/**
 * @BRIEF	check wether mute hp,depends on audec or current pcm data
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL isForceMute
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPlayer_AutoMute(T_BOOL isForceMute);

/**
 * @BRIEF	whether a2dp play or mute
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL play
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_BOOL BtPlayer_IsAutoMute(T_VOID);

/**
 * @BRIEF	for a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPlayer_Decode(T_VOID);

/**
 * @BRIEF	deinit btplayer decoder when phone comes or a2dp disconnect
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_DeInit(T_VOID);

/**
 * @BRIEF	init btplayer decoder when a2dp start msg comes
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_BTA2DP_PLAYER *
 * @RETVAL	
 */
T_HANDLE BtPlayer_Init(T_U32 dec_volume);

/**
 * @BRIEF	check if we have init a btplayer and decoder 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPlayer_IsWorking(T_VOID);

/**
 * @BRIEF	set a2dp status,if pause a2dp,we pause pcm
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL playing
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_SetPlaying(T_BOOL playing);

/**
 * @BRIEF	check if a2dp is playing 
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE if a2dp is playing ,or AK_FALSE
 */
T_BOOL BtPlayer_IsPlaying(T_VOID);

/**
 * @BRIEF	Register a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BTA2DP_PLAYER *player
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_RegisterTask(T_VOID);

/**
 * @BRIEF	UnRegister a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BTA2DP_PLAYER *player
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Unregistertask(T_VOID);

/**
 * @BRIEF	select music to play,if back is AK_TRUE play the backward music,or play the forward music
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL vol
 * @RETURN	T_VOID
 * @RETVAL 
 */
T_VOID BtPlayer_Switch(T_BOOL back);

/**
 * @BRIEF	set a2dp volume,if vol if ak_TRUE,increase vol,or decrease vol
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL vol
 * @RETURN	T_VOID
 * @RETVAL 
 */
T_VOID BtPlayer_SetVolume(T_U32 playVol);

/**
 * @BRIEF	set a2dp play or pause,if play is ak_true start a2dp music,or stop a2dp music
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL play
 * @RETURN	T_VOID
 * @RETVAL 
 */
T_VOID BtPlayer_Play(T_BOOL play);

/**
 * @BRIEF	pause a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */

T_VOID BtPlayer_Pause(T_VOID);

/**
 * @BRIEF	start a2dp decode task
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_Start(T_VOID);

/**
 * @BRIEF	set a2dp dec,if pause dec,we pause microtask and so on
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_BOOL decing
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPlayer_SetDecing(T_BOOL decing);

/**
 * @BRIEF	check if a2dp is decing 
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE if a2dp is decing ,or AK_FALSE
 */
T_BOOL BtPlayer_IsDecing(T_VOID);


/**
 * @BRIEF	set eq mode
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	
 * @RETURN	T_BOOL
 * @RETVAL	T_VOID
 */
T_VOID BtPlayer_SetEQ(T_VOID);


#endif//end of _BTPLAYER_H_
