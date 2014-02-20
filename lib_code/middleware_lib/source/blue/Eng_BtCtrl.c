/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_audio_record.c
 * @BRIEF audio record process 
 * @Author：Huang_ChuSheng
 * @Date：2008-04-15
 * @Version：
**************************************************************************/


#include "Eng_BtCtrl.h"
#include <string.h>
#include "Eng_debug.h"
#include "fwl_timer.h"
#include "fwl_blue.h"
#include "fwl_osmalloc.h"
#include "fwl_system.h"
#include "ba_a2dp.h"
#include "ba_hfp.h"
#include "ba_avrcp.h"
#include "ba_gap.h"
#include "Apl_Public.h"
#include "Eng_LedHint.h"
#include "Fwl_detect.h"
#include "Fwl_Serial.h"

#ifdef SUPPORT_BLUETOOTH

static T_U32 Connected = 0;
static T_BOOL isLibOn = AK_FALSE;
static T_U8 BtStatus = eBTCTRL_NULL;
T_BOOL g_test_mode = AK_FALSE;

 
/*********************************************************************
  Function:		BtCtrl_IsPhoneDevice
  Description:	check if device is a phone device
  Input:			T_PAIRLIST_INFO* device
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsPhoneDevice(T_PAIRLIST_INFO* device)
{
    T_U32 device_id = device->classofDevice;
	if(device_id == 0)
	{
		akerror("error device",0,1);
		return AK_FALSE;
	}
    return (0x02 == ((device_id >> 8) & 0x1f));
}
 
/*********************************************************************
  Function:		BtCtrl_GetConnected
  Description:	get service that we have connected
  Input:			T_VOID
  Return:			T_U32
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_U32 BtCtrl_GetConnected(T_VOID)
{
	return Connected;
}
 
/*********************************************************************
  Function:		BtCtrl_Connect
  Description:	connect the given device
  Input:			T_PAIRLIST_INFO * info
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_Connect(T_PAIRLIST_INFO * info)
{
	if(BtCtrl_IsPhoneDevice(info)
		&& ((Connected & eBTDEV_HFP_SERVICE) == 0))
	{
		BA_HFP_Connect(info->BD_ADDR);
		return AK_TRUE;
	}
	else if((Connected & eBTDEV_A2DP_SERVICE) == 0)
	{
		BA_A2DP_Connect(info->BD_ADDR);
		return AK_TRUE;
	}
	else if((Connected & eBTDEV_AVRCP_SERVICE) == 0)
	{
		BA_AVRCP_Connect(info->BD_ADDR);
		return AK_TRUE;
	}
	else
	{
		return AK_FALSE;
	}
}
 
/*********************************************************************
  Function:		BtCtrl_AddService
  Description:	add the pointed service into device 
  Input:			T_eBTDEV_SERVICE service
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_AddService(T_eBTDEV_SERVICE service)
{
	Connected |= service;
	if(BtCtrl_GetCurStatus() < eBTCTRL_PHONE_INCOMING)
	{
		BtCtrl_SetCurStatus(eBTCTRL_CONNECTED);
	}
}

/*********************************************************************
  Function:		BtCtrl_DelService
  Description:	delete the pointed service from device 
  Input:			T_eBTDEV_SERVICE service
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_DelService(T_eBTDEV_SERVICE service)
{
	Connected &= ~service;
}

 
/*********************************************************************
  Function:		BtCtrl_HaveService
  Description:	check if device has connect the pointed service
  Input:			T_eBTDEV_SERVICE service
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_HaveService(T_eBTDEV_SERVICE service)
{
	return (0 !=(Connected & service));
}

/**
 * @BRIEF	 set gap info
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-23
 * @PARAM    T_BTDEV_INFO * btdev_crl 
 * @RETURN	T_VOID
 */
void BtCtrl_GapInit(T_BTDEV_INFO * btdev_crl)
{	
	T_U16 bt_name[BT_NAME_MAX_LEN] = {0};
	T_U32 startTime = 0;
	T_U32 nowTime = 0;
	T_U8 *name = btdev_crl->name;
	T_U8 *addr = btdev_crl->BD_ADDR;
	T_U8 i = 0;
	

	for (i=0;i<BT_NAME_MAX_LEN;i++)
	{
        bt_name[i] = (T_U16)name[i];
		if(name[i] == 0)
		{
			break;
		}
	}
	
	BA_GAP_SetLocalName(bt_name);
	BA_GAP_SetLocalBDAddr(addr);
    BA_GAP_SSPSwitch(AK_TRUE);
    BA_GAP_SetDeviceMode(DEVICE_MODE_BONDING|DEVICE_MODE_BONDABLE|DEVICE_MODE_CONNECTABLE|DEVICE_MODE_DISCOVERABLE);//
    BA_GAP_SetClassOfDevice(btdev_crl->classofDevice);
    BA_GAP_SetInquirySpeed(INQUIRY_SPEED_POWER_SAVE);
    BA_GAP_SetPagedSpeed(PAGED_SPEED_LEVEL4);
	
	startTime = Fwl_GetTickCountMs();
	while( eBTCTRL_NULL == BtCtrl_GetCurStatus())
	{
		BA_Process(0);
		nowTime = Fwl_GetTickCountMs();
		if((nowTime >= startTime)&&((nowTime-startTime)> GAP_MAX_INIT_TIME) ||
			((nowTime < startTime)&&(((T_U32_MAX-startTime)+nowTime)> GAP_MAX_INIT_TIME)))
		{
			break;
		}
		
	}
	
}
#pragma arm section code = "_SYS_BLUE_CODE_"

/**
 * @BRIEF	 bluea uart Interrupt enable callback
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-23
 * @PARAM    T_BOOL on 
 * @RETURN	T_VOID
 */
T_S32 BtCtrl_EnableInterrupt(T_BOOL on)
{
	Fwl_UartSetIntStatus(on);
	return AK_TRUE;
}

T_pVOID BtCtrl_Malloc(T_U32 size, T_CHR * fun, T_U32 line)
{
    return Fwl_Malloc(size);
}

T_VOID BtCtrl_Free(T_pVOID mem, T_CHR * fun, T_U32 line)
{
    Fwl_Free(mem);
}

T_pVOID BtCtrl_DMAMalloc(T_U32 size, T_CHR * fun, T_U32 line)
{
    return Fwl_DMAMalloc(size);
}

T_VOID BtCtrl_DMAFree(T_pVOID mem, T_CHR * fun, T_U32 line)
{
    Fwl_DMAFree(mem);
}
#pragma arm section code

T_U8 UartOverLoadFlag = 0;
#pragma arm section code = "_bootcode1_"
#if 1
T_VOID  BA_TransportReceiveTT(T_pCVOID buffer, T_U32 count)
{
	if(count != BA_TransportReceive(buffer, count))
	{
		UartOverLoadFlag = 1;
	}
}
#endif
#pragma arm section code
T_S32 BtCtrl_BlueInit(T_VOID)
{
	return Fwl_BlueInit(BA_TransportReceiveTT);
}
T_S32 BtCtrl_BlueDeInit(T_VOID)
{
	return Fwl_BlueDeInit();
}

#pragma arm section code = "_SYS_BLUE_CODE_"
/**
 * @BRIEF	callback of bluea ,which is to control uart
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_pCVOID buffer
 			T_U32 count
 * @RETURN	T_VOID
 */
T_S32 BtCtrl_BlueSendData(T_pCVOID buffer, T_U32 count)
{
	return Fwl_BlueSendData((T_U8*)buffer,(T_U16)count);
}
#pragma arm section code

/*********************************************************************
  Function:		Btdev_LibInit
  Description:	Init BlueA lib,and start BlueTooth
  Input:			T_BTDEV_INFO * btdev_crl
  				BA_CALLBACK_FUN_USER_MSG MsgCB
  Return:			SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_LibInit(T_BTDEV_INFO * btdev_crl, BA_CALLBACK_FUN_USER_MSG MsgCB)
{
	T_BA_INPUT input;
	
	memset(&input, 0, sizeof(input));
	input.cb_fun.Malloc = BtCtrl_Malloc;
	input.cb_fun.Free = BtCtrl_Free;
	input.cb_fun.DmaMalloc = BtCtrl_DMAMalloc;
	input.cb_fun.DmaFree = BtCtrl_DMAFree;
	input.cb_fun.Printf = (BA_CALLBACK_FUN_PRINTF)AkDebugOutput;
	input.cb_fun.InitTransport = BtCtrl_BlueInit;
	input.cb_fun.DeinitTransport = BtCtrl_BlueDeInit;
	input.cb_fun.TransportSend = BtCtrl_BlueSendData;
	input.cb_fun.PlatformInterrupt = BtCtrl_EnableInterrupt;
	input.cb_fun.GetTick = (BA_CALLBACK_FUN_GET_TICK)Fwl_GetTickCountMs;
	input.cb_fun.UserMsg = MsgCB;
	input.cb_fun.DumpHciPacket = AK_NULL;

	input.info.strVersion = BA_LIB_VERSION_STRING;
	input.info.msPerTick = 1;//对应input.cb_fun.GetTick设置
	input.info.msLinkSupervisionTimeout = 2000;//设置音箱断电后,手机反应时间
	input.info.msPageTimeout = 5000;

	//input.info.msPageTimeout = 5120;//系统连接超时设置默认5120
#ifdef OS_WIN32
	input.info.TransportRxBufFreeSize = 32*1024;
	input.info.TransportRxBufMaxSize = 60*1024;
#else
	input.info.TransportRxBufFreeSize = 16*1024;
	input.info.TransportRxBufMaxSize = 32*1024;
#endif
	
	BA_Init(&input);
	BA_Start();
	
	//注册服务
	if(g_test_mode)
	{
		BA_A2DP_Start();
	}
	else
	{
		if(0 != (btdev_crl->Service & eBTDEV_HFP_SERVICE))
			BA_HFP_Start();
		if(0 != (btdev_crl->Service & eBTDEV_A2DP_SERVICE))
			BA_A2DP_Start();
		if(0 != (btdev_crl->Service & eBTDEV_AVRCP_SERVICE))
			BA_AVRCP_Start();
	}

	//BA_SPP_Start();
	//结束注册服务

	
	//gap 设置
	BtCtrl_GapInit(btdev_crl);
	//结束gap 设置
	return AK_TRUE;
}

#pragma arm section code = "_bootcode1_"

/*********************************************************************
  Function:		BtCtrl_IsLibOn
  Description:	check if lib is on
  Input:			T_VOID
  Return:			on:AK_TRUE   off:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsLibOn(T_VOID)
{
	return isLibOn;
}
#pragma arm section code

/*********************************************************************
  Function:		BtCtrl_SetLibOn
  Description:	set lib on or off,called when start up or shut down
  Input:			T_BOOL on
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_SetLibOn(T_BOOL on)
{
	isLibOn = on;
}

/*********************************************************************
  Function:		BtCtrl_SetLinKey_for
  Description:	check if lib is on
  Input:			T_BD_ADDR DevAddr
  				T_LINK_KEY Linkey
  Return:			succeed:AK_TRUE   failed:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_SetLinKey_for(T_BD_ADDR DevAddr, T_LINK_KEY Linkey)
{
	return BA_GAP_SetLinKey_for(DevAddr, Linkey);
}

/*********************************************************************
  Function:		BtCtrl_LibDeInit
  Description:      DeInit BlueA lib,and stop BlueTooth
  Input:			T_VOID
  Return:			SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_LibDeInit(T_VOID)
{
	BA_Stop(AK_TRUE);
	
	while(BtCtrl_IsLibOn())
	{
		BA_Process(0);		
	}

	BA_Deinit();		
    BtCtrl_SetCurStatus(eBTCTRL_NULL);
	return AK_TRUE;
}

/*********************************************************************
  Function:		BtCtrl_LEDHint
  Description:	set current bt led status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_LEDHint(T_BTCTRL_STATUS status)
{
#ifdef SUPPORT_LEDHINT
    if(status == eBTCTRL_PHONE_INCOMING)
	{
		if(Fwl_DetectorGetStatus(DEVICE_CHG))
		{
			LedHint_Exec(LED_BT_CALLED_CHARGE);//充电被叫状态：紫色灯闪烁两次，间隔两秒
		}
		else
		{
			LedHint_Exec(LED_BT_CALLED);//非充电被叫状态：红色灯闪烁两次，间隔两秒
		}
	}
    else
    {
	    switch(status)
        {
        //可被搜索的状态，每0.2秒闪一下
		//case eBTCTRL_NULL:
		case eBTCTRL_STANDBY:
		case eBTCTRL_RECONNECTING:
		case eBTCTRL_CONNECTING:
			if(Fwl_DetectorGetStatus(DEVICE_CHG))
			{
				LedHint_Exec(LED_BT_RECONNECT_CHARGE);
			}
			else
			{
				LedHint_Exec(LED_BT_RECONNECT);
			}
            break;
		//蓝牙连接成功、蓝牙立体声、接听后状态，每秒快速闪动1次，隔5秒闪一次
		case eBTCTRL_CONNECTED:
		case eBTCTRL_A2DP:
		case eBTCTRL_PHONE_OUTGOING:
		case eBTCTRL_PHONE_ONGOING:
            if(Fwl_DetectorGetStatus(DEVICE_CHG))
			{
				LedHint_Exec(LED_BT_CONNECTED_CHARGE);
			}
			else
			{
				LedHint_Exec(LED_BT_CONNECTED);
			}
            break; 
        default:
            break;
        }
	}
#endif
}
extern T_BOOL pull_out_usb;
/*********************************************************************
  Function:		BtCtrl_LEDStop
  Description:	stop current bt led status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_LEDStop(T_BTCTRL_STATUS status)
{
#ifdef SUPPORT_LEDHINT
    if(status == eBTCTRL_PHONE_INCOMING)
	{
		if(Fwl_DetectorGetStatus(DEVICE_CHG)||(AK_TRUE == pull_out_usb))
		{
			LedHint_Stop(LED_BT_CALLED_CHARGE);//充电被叫状态：紫色灯闪烁两次，间隔两秒
		}
		else
		{
			LedHint_Stop(LED_BT_CALLED);//非充电被叫状态：红色灯闪烁两次，间隔两秒
		}
	}
    else
    {
	    switch(status)
        {
        //可被搜索的状态，每0.2秒闪一下
		//case eBTCTRL_NULL:
		case eBTCTRL_STANDBY:
		case eBTCTRL_RECONNECTING:
		case eBTCTRL_CONNECTING:
			if(Fwl_DetectorGetStatus(DEVICE_CHG)||(AK_TRUE == pull_out_usb))
			{          
				LedHint_Stop(LED_BT_RECONNECT_CHARGE);
			}
			else
			{
				LedHint_Stop(LED_BT_RECONNECT);
			}
            break;
		//蓝牙连接成功、蓝牙立体声、接听后状态，每秒快速闪动1次，隔5秒闪一次
		case eBTCTRL_CONNECTED:
		case eBTCTRL_A2DP:
		case eBTCTRL_PHONE_OUTGOING:
		case eBTCTRL_PHONE_ONGOING:
            if(Fwl_DetectorGetStatus(DEVICE_CHG)||(AK_TRUE == pull_out_usb))
			{
				LedHint_Stop(LED_BT_CONNECTED_CHARGE);
			}
			else
			{
				LedHint_Stop(LED_BT_CONNECTED_CHARGE);
			}
            break; 
        default:
            break;
        }
	}
#endif
}

/*********************************************************************
  Function:		BtCtrl_SetCurStatus
  Description:	set current bt status
  Input:			T_BTCTRL_STATUS status
  Return:			T_VOID
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_VOID BtCtrl_SetCurStatus(T_BTCTRL_STATUS status)
{
    if(BtStatus != status)
    {
        BtCtrl_LEDStop(BtStatus);
        
    	BtStatus = status;
        BtCtrl_LEDHint(BtStatus);
    	#ifdef OS_WIN32
    	{
    		char *str[]=
    			{
    			"eBTCTRL_NULL",
    			"eBTCTRL_STANDBY",
    			"eBTCTRL_RECONNECTING",
    			"eBTCTRL_CONNECTING",	
    			"eBTCTRL_CONNECTED",
    			"eBTCTRL_A2DP",
    			"eBTCTRL_PHONE_INCOMING",
    			"eBTCTRL_PHONE_OUTGOING",
    			"eBTCTRL_PHONE_ONGOING",
    			};
    		AkDebugOutput("......%s.....\n",str[status]);

    	}
    	#endif
    }
}

/*********************************************************************
  Function:		BtCtrl_GetCurStatus
  Description:	get current bt status
  Input:			T_VOID
  Return:			T_BTCTRL_STATUS
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
#pragma arm section code = "_SYS_BLUE_CODE_"
T_BTCTRL_STATUS BtCtrl_GetCurStatus(T_VOID)
{
	return BtStatus;
}

/*********************************************************************
  Function:		BtCtrl_IsStandby
  Description:	check if current bt status is not phone status,for led
  Input:			T_VOID
  Return:			T_BOOL
  Author:			zhuangyuping
  Data:			2013-3-6
**********************************************************************/
T_BOOL BtCtrl_IsStandby(T_VOID)
{
    T_BTCTRL_STATUS curStatus;
    curStatus = BtCtrl_GetCurStatus();

    if ((eBTCTRL_NULL == curStatus)\
        ||(eBTCTRL_STANDBY == curStatus)\
        ||(eBTCTRL_CONNECTING == curStatus)\
        ||(eBTCTRL_CONNECTED == curStatus)\
        ||(eBTCTRL_A2DP == curStatus))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}
#pragma arm section code

#endif
