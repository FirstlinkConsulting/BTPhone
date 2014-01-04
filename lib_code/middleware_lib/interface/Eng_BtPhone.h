/*******************************************************************************
 * @file	s_bt_phone.h
 * @brief	board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	he_yl
 * @date	2012-12-27
 * @version 1.0
*******************************************************************************/

	
#ifndef _BTPHONE_H_
#define _BTPHONE_H_

#include "anyka_types.h"
#include "log_pcm_record.h"
#include "log_pcm_player.h"
#include "ba_lib_api.h"




typedef struct {
	T_U8 PcmPly_ID;
	T_U8 PcmRec_ID;
	T_U8 SpeakerVolume;
	volatile T_BOOL EnableAec;
	//T_U8 MicVolume;
	T_U16	pcmUnitSize;//发送和接受的size
	T_U16	SendEndPos;	//数据缓冲区的总长度
	T_U16	SendCurPos;	//当前要发送的长度
	//T_U32	Recorder_Timer;//用于录音的采集timer
	T_VOID	*AEChandle;//用于进行回音消除用
	T_VOID	*AECFilter;//用于对AD SCO数据重采样,方便做AEC
#ifdef SUPPORT_HFP_RESAMPLE_CONTROL_EN
	T_VOID	*SCOFilter;//用于对远端SCO数据重采样
#endif
	//T_U8 *RecBuf;
	T_U8	*SCOFilterBuf;
	//T_U8	*RemoteSCOBuf;
	T_U8 	*SendBuf;	//发送远端数据的缓冲区
	T_U32	pretime;//防止按键发送音量太快
} T_BTHFP_PHONEMGR;

#define BTPHONE_BASE_VOICE_VALUE 8



/**
 * @BRIEF	set phone's Speaker Volume and save vol in local .
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL add
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPhone_SetSpeakerVolume(T_BOOL add);

/**
 * @BRIEF	set phone's Speaker Volume and save vol in local .
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 vol
 * @RETURN	
 * @RETVAL	
 */
T_VOID BtPhone_SetSpeakerGain(T_U8 vol);

/**
 * @BRIEF	deinit btphone status mechine
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_VOID BtPhone_DeInit(T_VOID);

/**
 * @BRIEF	init btphone status mechine
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_HANDLE BtPhone_Init(T_VOID);

/**
 * @BRIEF	check if we are in htphone status
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_IsWorking(T_VOID);

/**
 * @BRIEF	btphone main process
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_Handle(T_VOID);

/**
 * @BRIEF	accept the incoming phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_AnswerCall(T_VOID);

/**
 * @BRIEF	reject the incoming phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_RejectCall(T_VOID);

/**
 * @BRIEF	cancel the outgoing phone or ongoing phone
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtPhone_CancelCall(T_VOID);

/**
 * @BRIEF	dial the pointed phone with (psz != ak_null) or dial the last outgo phone with (psz == ak_null)
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_HANDLE
 * @RETVAL	
 */
T_BOOL BtPhone_CallDial(T_pCDATA psz);

/**
 * @BRIEF	get current phone status (standby,incoming,outgoing,ongoing)
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_U32
 * @RETVAL	
 */
T_U8 BtPhone_GetPhoneStatus(T_VOID);

/**
 * @BRIEF	get remote sco data
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_U8 *data
 			T_U16 data_len
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_LoadSCOData(T_U8 *data, T_U16 data_len);

/**
 * @BRIEF	when phone is ongoing, btphone accept,we reset ad and da info
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtPhone_Accept(T_VOID);

/**
 * @BRIEF	it will resample ad data to AEC lib
 * @AUTHOR	BtPhone_ADReSmaple
 * @DATE	2013-05-21
 * @PARAM	T_U32 SrcLen
 			T_U8 *SrcData
 			T_U32 DestLen
 			T_U8 *DestData
 * @RETURN	T_VOID
 * @RETVAL	
 */

T_U32 BtPhone_ADReSmaple(T_U32 SrcLen, T_U8 *SrcData, T_U32 DestLen, T_U8 *DestData);

/**
 * @BRIEF	it will send data to AEC lib
 * @AUTHOR	BtPhone_CheckVoice
 * @DATE	2013-05-21
 * @PARAM	T_U8 *src
 			T_U32 sampSize
 			T_BOOL AD_DA_Flag	ak_true for da, false for ad
 * @RETURN	T_VOID
 * @RETVAL	
 */

T_S32 BtPhone_CheckVoice(T_U8 *src, T_U32 sampSize, T_BOOL AD_DA_Flag);

#endif //end of _BTPHONE_H_
