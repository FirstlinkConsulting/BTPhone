/*******************************************************************************
 * @file	Btconnect.h
 * @brief	board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	he_yl
 * @date	2012-12-27
 * @version 1.0
*******************************************************************************/

	
#ifndef _BTCTRL_H_
#define _BTCTRL_H_


#include "anyka_types.h"
#include "ba_lib_api.h"
#include "ba_hfp.h"
#include "ba_a2dp.h"
#include "ba_avrcp.h"





#define BT_NAME_MAX_LEN  32
#define BT_ADDR_MAX_LEN  6
#define PAIRED_LIST_MAX 128//配对列表的长度
#define BT_LINKKEY_LEN 16//配对列表的长度
#define GAP_MAX_INIT_TIME  1000



typedef struct{
	T_LINK_KEY LinkKey;
	//T_U8 Key_Type;
	//T_U32 Service;
	T_U32 classofDevice;//远端设备的设备类型
	T_BD_ADDR BD_ADDR;
}T_PAIRLIST_INFO;

typedef struct{
	T_U8 name[BT_NAME_MAX_LEN];
	T_U32 Service;//本地音响的支持的服务
	T_U32 classofDevice;//本地音响的设备类型
	T_BD_ADDR BD_ADDR;
}T_BTDEV_INFO;



typedef struct 
{
	//T_U8 name[BT_NAME_MAX_LEN];//通过sdp搜寻远端设备的名字
	T_U8 isSendBat;
	T_PAIRLIST_INFO Info;
} T_BTDEV_REMOTE_INFO;//远端设备信息

typedef struct 
{
	//T_LINK_KEY LinkKey;
	//T_U8 Key_Type;
	T_BTDEV_INFO info;
} T_BTDEV_LOCAL_INFO;//本地设备信息

typedef struct {
	T_U8              CheckCode;
	T_U8			connectIdx;
#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
	T_U8 			a2dpEQMode;
#endif
	T_BTDEV_LOCAL_INFO localInfo;
	T_PAIRLIST_INFO pairedList[PAIRED_LIST_MAX];
	T_U32             a2dpCurVol;
} T_BTDEV_CFG;//spi信息


typedef enum
{
	eBTDEV_AVRCP_SERVICE = 1<<0,
	eBTDEV_A2DP_SERVICE = 1<<1,
	eBTDEV_HFP_SERVICE = 1<<2,
}T_eBTDEV_SERVICE;

typedef enum 
{
	eBTCTRL_NULL,
	eBTCTRL_STANDBY,
	eBTCTRL_RECONNECTING,
	eBTCTRL_CONNECTING,	
	eBTCTRL_CONNECTED,
	eBTCTRL_DISCONNECTING,
	eBTCTRL_A2DP,
	eBTCTRL_PHONE_INCOMING,
	eBTCTRL_PHONE_OUTGOING,
	eBTCTRL_PHONE_ONGOING,
} T_BTCTRL_STATUS;

/*********************************************************************
  Function:		BtCtrl_LEDHint
  Description:	set current bt led status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_LEDHint(T_BTCTRL_STATUS status);

/*********************************************************************
  Function:		BtCtrl_LEDStop
  Description:	stop current bt led status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_LEDStop(T_BTCTRL_STATUS status);

/*********************************************************************
  Function:		BtCtrl_SetCurStatus
  Description:	set current bt status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_SetCurStatus(T_BTCTRL_STATUS status);

/*********************************************************************
  Function:		BtCtrl_GetCurStatus
  Description:	get current bt status
  Input:			T_VOID
  Return:			T_BTCTRL_STATUS
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BTCTRL_STATUS BtCtrl_GetCurStatus(T_VOID);

/*********************************************************************
  Function:		BtCtrl_IsStandby
  Description:	check if current bt status is not phone status,for led
  Input:			T_VOID
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsStandby(T_VOID);

/*********************************************************************
  Function:		BtCtrl_Connect
  Description:	connect the given device
  Input:			T_PAIRLIST_INFO * info
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_Connect(T_PAIRLIST_INFO * info);

/*********************************************************************
  Function:		BtCtrl_DelService
  Description:	delete the pointed service from device 
  Input:			T_eBTDEV_SERVICE service
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_DelService(T_eBTDEV_SERVICE service);

/*********************************************************************
  Function:		BtCtrl_AddService
  Description:	add the pointed service into device 
  Input:			T_eBTDEV_SERVICE service
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_AddService(T_eBTDEV_SERVICE service);
 
/*********************************************************************
  Function:		BtCtrl_HaveService
  Description:	check if device has connect the pointed service
  Input:			T_eBTDEV_SERVICE service
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_HaveService(T_eBTDEV_SERVICE service);

/*********************************************************************
  Function:		BtCtrl_IsPhoneDevice
  Description:	check if device is a phone device
  Input:			T_PAIRLIST_INFO* device
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsPhoneDevice(T_PAIRLIST_INFO* device);

/*********************************************************************
  Function:		BtCtrl_GetConnected
  Description:	get service that we have connected
  Input:			T_VOID
  Return:			T_U32
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_U32  BtCtrl_GetConnected(T_VOID);

/*********************************************************************
  Function:		Btdev_LibInit
  Description:	Init BlueA lib,and start BlueTooth
  Input:			T_BTDEV_INFO * btdev_crl
  				BA_CALLBACK_FUN_USER_MSG MsgCB
  Return:			SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_LibInit(T_BTDEV_INFO * btdev_crl, BA_CALLBACK_FUN_USER_MSG BALib2UserMsg);

/*********************************************************************
  Function:		BtCtrl_LibDeInit
  Description:      DeInit BlueA lib,and stop BlueTooth
  Input:			T_VOID
  Return:			SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_LibDeInit(T_VOID);

/*********************************************************************
  Function:		BtCtrl_IsLibOn
  Description:	check if lib is on
  Input:			T_VOID
  Return:			on:AK_TRUE   off:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsLibOn(T_VOID);

/*********************************************************************
  Function:		BtCtrl_SetLibOn
  Description:	set lib on or off,called when start up or shut down
  Input:			T_BOOL on
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_SetLibOn(T_BOOL on);

/*********************************************************************
  Function:		BtCtrl_SetLinKey_for
  Description:	check if lib is on
  Input:			T_BD_ADDR DevAddr
  				T_LINK_KEY Linkey
  Return:			succeed:AK_TRUE   failed:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_SetLinKey_for(T_BD_ADDR DevAddr, T_LINK_KEY Linkey);

#endif//end of _BTCTRL_H_

