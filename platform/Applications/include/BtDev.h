/*******************************************************************************
 * @file	Btdev.h
 * @brief	board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	he_yl
 * @date	2012-12-27
 * @version 1.0
*******************************************************************************/

	
#ifndef _BTDEV_H_
#define _BTDEV_H_

#include "BA_lib_api.h"
#include "Eng_BtCtrl.h"
#include "ba_gap.h"

//#define BTDEV_CONNECT_MAX_TIME 2//启动时重连设备列表的设备的循环次数

#define BTDEV_CONNECT_MAX_DEVNUM   1//重连的最大设备个数
#define BTDEV_RECONNECT_DEVNUM	((BTDEV_CONNECT_MAX_DEVNUM)<(PAIRED_LIST_MAX)?(BTDEV_CONNECT_MAX_DEVNUM):(PAIRED_LIST_MAX))
#define BTDEV_SYS_RESTART_RECONNECT_TIMES   1//重连的最大设备次数


#define BTDEV_CONNECT_MAX_DELAY (30*1000)//远离服务区自动重连超时


//以下宏值会目前会根据调用Blue_SetProcessTicks的参数要偏大于该过程的最大统计值
/************以下过程目前来看都不需要执行多次****************/
#define HFP_PROCESS_TICKS (8)//目前的统计HFP正常过程8~10ms，最常见8
#define A2DP_PROCESS_TICKS (7)//目前的统计A2DP正常过程7ms，最常见7,不过目前来看a2dp播放是正常的
#define A2DP_HFP_PROCESS_TICKS (40)//目前的统计HFP和A2DP切换过程40ms，最常见40
#define NULL_PROCESS_TICKS (7)//目前的统计没有HFP和A2DP过程7~10ms,最常见7


/************以下过程才有可能需要执行多次****************/
#define DISCONNECT_PROCESS_TICKS (65)//目前的统计断开过程16~100ms，最常见的是50~70
#define CONNECT_PROCESS_TICKS (70)//目前的统计连接过程50~120ms，最常见

#define DISCONNECT_TIMEOUT (10000)//断开服务超时10s


#define MAX_DEC_VOL			1984
#define DEFAULT_DEC_VOL		1024
#define MIN_DEC_VOL			0
#define DEC_VOL_INTERVAL	64

typedef enum
{
	eREMOTE_CONNECT = 0,
	eOUTOFAREA_CONNECT,//远离服务区重连
	eSTARTUP_CONNECT,//启动重连
}T_CONNECT_TYPE;

typedef enum
{
	eA2DP_FAIL = 0,
	eA2DP_OK,//远离服务区重连
	eCONNECT_FAIL,//启动重连
	eHFP_FAIL,
	eHFP_OK,//远离服务区重连
	eAVRCP_FAIL,//启动重连
	eAVRCP_OK,//启动重连
}T_CONNECT_EVENT;

typedef struct 
{
	T_U8 PcmId;
	T_U8 EQMode;
	T_BOOL IsRuning;
	T_S8  ReConnectTimes;		//重连的次数
	T_BOOL DelLinkKey;
	T_BTDEV_LOCAL_INFO  LocalInfo;
	T_U32 PreTickOrTime;//当前连接上的服务,主要用于重连，对比当前远端设备的服务可以发现哪些服务没有连接上
	T_U32 Event;
	T_BTDEV_REMOTE_INFO RemoteInfo;
	T_U32 a2dpCurVol;
} T_BTDEV_CTRL;


/**
 * @BRIEF	we start connect  if there is some event which we think we can start connect . 
 			all depends on user.
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_CONNECT_EVENT event
 * @RETURN	T_VOID
 */
T_VOID BtDev_EventConnect(T_CONNECT_EVENT event);

/**
 * @BRIEF	cancel current reconnect and if there is not connected we set it standby
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_BOOL connected
 * @RETURN	T_VOID
 */
T_VOID BtDev_CancelConnect(T_BOOL cancel);

/**
 * @BRIEF	check if btdev addr is valid
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 *byteAddr
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_AddrValid(T_U8 *byteAddr);

/**
 * @BRIEF	free config info which is a spi buf
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_DestroyConfig(T_VOID);

/**
 * @BRIEF	start up btdev,including set dev name ,addr,init btlib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_Init(T_VOID);

/**
 * @BRIEF	exit bluetooth
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_DeInit(T_VOID);

/**
 * @BRIEF	connect device
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_CONNECT_TYPE type: 当前的连接类型，当前是别人连接过来还是我们发起还是断线重连
 * @RETURN	T_VOID
 */
T_VOID BtDev_Connect(T_CONNECT_TYPE type);

/**
 * @BRIEF	check if to read spi to connect dev when start up dev
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 */
T_BOOL BtDev_IsStartUpConnect(T_VOID);

/**
 * @BRIEF	check if it have got the incoming phone number to play ring
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 */
T_BOOL BtDev_IsGetPhone(T_VOID);

/**
 * @BRIEF	get spi btdev info from spi
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	global g_Btdev_cfg
 */
T_BTDEV_CFG *BtDev_LoadConfig(T_VOID);

/**
 * @BRIEF	set player volume 
 * @AUTHOR	lizhenyi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */

T_VOID BtDev_SetVolume(T_BOOL add);

/**
 * @BRIEF	get player volume 
 * @AUTHOR	lizhenyi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */

T_U32 BtDev_GetVolume(T_VOID);


/**
 * @BRIEF	save remote classofdevice  into ram
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8* addr
			T_U32 classOfDivice
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SetRemoteClassOfDevice(T_U8* addr,T_U32 classOfDivice);

/**
 * @BRIEF	save remote linkkey into ram
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8* addr
 			T_U8* LinkKey
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SetRemoteLinkKey(T_U8* addr,T_U8* LinkKey);

/**
 * @BRIEF	save dev local info into spi and we clear pairedlist 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveLocalConfig(T_VOID);

/**
 * @BRIEF	save dev vol into spi 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U32 vol
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveA2DPConfig(T_U32 vol);

/**
 * @BRIEF	save dev pairedlist info into spi and we updata pairedlist 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveConfig(T_VOID);

/**
 * @BRIEF	when remote device delete linkkey,we have to retry to connect curdev again
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_TryConnectCurDev(T_VOID);

/**
 * @BRIEF	when remote device delete linkkey,we have to delete dev info from local spi
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BD_ADDR DevAddr
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_DelCurRemoteDev(T_BD_ADDR DevAddr);

/**
 * @BRIEF	when remote device request a linkkey ,we get linkkey from spi
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BD_ADDR DevAddr
 			T_LINK_KEY pLinkKey
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE:we have linkkey,AK_FALSE:we have not linkkey
 */
T_BOOL BtDev_GetRemoteDevLinkKey(T_BD_ADDR DevAddr, T_LINK_KEY pLinkKey);

/**
 * @BRIEF	check if there have been deleted linkkey 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_IsPhoneDevice(T_VOID);

/**
 * @BRIEF	check if there have been deleted linkkey 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_IsDelLinkKey(T_VOID);

/**
 * @BRIEF	check if we can save info into spi,we must have got remote addr,linkkey and classofdevice
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_CheckDevInfo(T_VOID);

typedef enum
{
	eIPHONE_BAT_NO_SEND = 0,
	eIPHONE_BAT_MAY_SEND,
	eIPHONE_BAT_MUST_SEND,
}T_IPHONE_BATSEND;


//*************************来电响铃管理*************************//
typedef struct 
{
	T_U8 number[20];
	T_BOOL Len;
	T_U8 Index;
	T_TIMER THdl;
} T_INCOMING_PHONE;

T_BOOL BtDev_IsGetPhone(T_VOID);
T_BOOL BtDev_SetPhone(T_U8 *phone,T_U8 len);
T_VOID BtDev_StopRing(T_VOID);
T_VOID BtDev_PlayRing(T_VOID);
T_VOID BtDev_DisConnect(T_VOID);
T_VOID BtDev_PlayTip(T_BTCTRL_STATUS Status);
T_BOOL BtDev_IsPlayingRing(T_VOID);
T_VOID BtDev_SetSendBat(T_U8 Type);
T_U8   BtDev_IsSendBat(T_VOID);


#endif//end of _BTDEV_H_

