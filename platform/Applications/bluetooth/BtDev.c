#include "BtDev.h"
#include "Eng_BtCtrl.h"
#include "Gbl_Global.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_math.h"
#include "fwl_timer.h"
#include "eng_profile.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"
#include "log_pcm_player.h"
#include "Eng_BtPlayer.h"



extern T_S32  BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2);
T_VOID BtDev_PrintDev(T_PAIRLIST_INFO *info);
T_VOID BtDev_PrintAddr(T_BD_ADDR addr);
T_VOID BtDev_PrintLinkKey(T_LINK_KEY key);
extern T_BOOL Blue_IsExitBtPhone(T_VOID);
extern T_VOID BtPlayer_SetEQMode(T_U8 mode);
extern T_U8 BtPlayer_GetEQMode(T_VOID);


#define BT_DEV_NAME "AK-SC-RD6P-B"

T_BTDEV_CTRL *gBtDevCtrl = AK_NULL;
T_BTDEV_CFG *g_Btdev_cfg = AK_NULL;
T_INCOMING_PHONE *g_InPhone = AK_NULL;
static T_TIMER ConnectDelay = ERROR_TIMER;
static T_BOOL  DeviceHaveConnected = AK_FALSE;

extern T_BOOL g_test_mode;



#pragma arm section code = "_frequentcode_"

/**
 * @BRIEF	check if bluea is on and is not going to off
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */
T_BOOL isinBtDev(T_VOID)
{
	return (AK_NULL != gBtDevCtrl && gBtDevCtrl->IsRuning);
}

/**
 * @BRIEF	check if the reomter device is the phone device class
 * @AUTHOR	lizhenyi
 * @DATE	2014-03-11
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */

T_BOOL remoteDeviceIsPhone(T_VOID)
{
	T_U32 remoteClass = gBtDevCtrl->RemoteInfo.Info.classofDevice;
	
	if((remoteClass&0x00001F00) == DEVICE_CLASS_IS_PHONE)
	{
		return AK_TRUE;
	}
	else
	{
		return AK_FALSE;
	}
}


/**
 * @BRIEF	check if we can send iphone's bat
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */
T_U8 BtDev_IsSendBat(T_VOID)
{
	if(gBtDevCtrl)
	{
		if(gBtDevCtrl->RemoteInfo.isSendBat == eIPHONE_BAT_MUST_SEND)
		{
			gBtDevCtrl->RemoteInfo.isSendBat = eIPHONE_BAT_MAY_SEND;
			return eIPHONE_BAT_MUST_SEND;
		}
		return gBtDevCtrl->RemoteInfo.isSendBat;
	}
	else
	{
		return eIPHONE_BAT_NO_SEND;
	}
}

#pragma arm section code


#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/**
 * @BRIEF	set player volume 
 * @AUTHOR	lizhenyi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */
T_VOID BtDev_SetVolume(T_BOOL add)
{
	T_U32 set = AK_FALSE;
	T_U32 setVolme = BtDev_GetVolume();
		
	if(add)
	{
		if(setVolme < MAX_DEC_VOL)
		{
			setVolme += DEC_VOL_INTERVAL;
			set = AK_TRUE;
		}
		if(setVolme > MAX_DEC_VOL)				
		{
			setVolme = MAX_DEC_VOL;
		}
	}
	else
	{
		if(setVolme > MIN_DEC_VOL)
		{
			if(setVolme >= DEC_VOL_INTERVAL)
			{
				setVolme -= DEC_VOL_INTERVAL;
			}
			else
			{
				setVolme = MIN_DEC_VOL;
			}
			set = AK_TRUE;
		}
		else
		{
			;//setVolme = MIN_DEC_VOL;
		}
	}
	if(set)
	{
		BtPlayer_SetVolume(setVolme);
	}
	AkDebugOutput("vol:%d\n", setVolme);
	gBtDevCtrl->a2dpCurVol = setVolme;
}

/**
 * @BRIEF	get player volume 
 * @AUTHOR	lizhenyi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	T_BOOL
 */
T_U32 BtDev_GetVolume(T_VOID)
{
	return gBtDevCtrl->a2dpCurVol;
}
#pragma arm section code

T_VOID BtDev_SetRuning(T_BOOL runing)
{
	gBtDevCtrl->IsRuning = runing;
}

/**
 * @BRIEF	set if we can send iphone's bat
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM  T_U8 Type
 * @RETURN	T_VOID
 */
T_VOID BtDev_SetSendBat(T_U8 Type)
{
	gBtDevCtrl->RemoteInfo.isSendBat = Type;
}

/**
 * @BRIEF	get spi btdev info from spi
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @PARAM  T_VOID
 * @RETURN	global g_Btdev_cfg
 */
T_BTDEV_CFG * BtDev_LoadConfig(T_VOID)
{
	if(AK_NULL == g_Btdev_cfg)
	{
		g_Btdev_cfg = Fwl_Malloc(sizeof(T_BTDEV_CFG));
		if(AK_NULL != g_Btdev_cfg)
		{
			if(!Profile_ReadData(eCFG_BTDEV, g_Btdev_cfg))
			{
				BtDev_DestroyConfig();
				return AK_NULL;
			}
				
			g_Btdev_cfg->connectIdx = T_U8_MAX;
		}
	}
	
	return g_Btdev_cfg;
}

#pragma arm section code =  "_SYS_BLUE_CONNECT_CODE_"
/**
 * @BRIEF	get next btdev to connect
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM  T_BOOL *OverLoad
 * @RETURN	T_PAIRLIST_INFO *
 */
T_PAIRLIST_INFO *Btdev_GetNextDevice(T_BOOL *OverLoad)
{
	T_PAIRLIST_INFO * info;
	
	if(AK_NULL == g_Btdev_cfg)
	{
		return AK_NULL;
	}
	if(AK_NULL != OverLoad)
	{
		*OverLoad = AK_FALSE;
	}
REGET_NEXTDEVICE:
	g_Btdev_cfg->connectIdx ++;
	if(g_Btdev_cfg->connectIdx >= BTDEV_RECONNECT_DEVNUM)
	{
		if(AK_NULL != OverLoad)
		{
			*OverLoad = AK_TRUE;
		}
		g_Btdev_cfg->connectIdx = 0;
	}
	info = &g_Btdev_cfg->pairedList[g_Btdev_cfg->connectIdx];
	if(!BtDev_AddrValid(info->BD_ADDR))
	{
		if(g_Btdev_cfg->connectIdx == 0)
		{
			return AK_NULL;
		}
		else
		{
			g_Btdev_cfg->connectIdx = T_U8_MAX;
			if(AK_NULL != OverLoad)
			{
				*OverLoad = AK_TRUE;
			}
			goto REGET_NEXTDEVICE;
		}
	}
	memcpy (&gBtDevCtrl->RemoteInfo.Info,info,sizeof(T_PAIRLIST_INFO));
	
	return &gBtDevCtrl->RemoteInfo.Info;
}
#pragma arm section code

/**
 * @BRIEF	convert the bluetooth structure address(NAP-UAP-LAP) from six bytes format address
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @PARAM [IN] t_bdaddr *byteAddr
 * @PARAM [OUT] T_U16 * NAP-UAP-LAP
 * @RETURN	T_S32
 * @RETVAL	BTCTRL_SUCCESS: successful
 *			else:  failed.
 */
static T_BOOL BtDev_AddrToStructure(T_U8 *byteAddr, T_U16 *NAP, T_U8 *UAP, T_U32 *LAP)
{
    if (AK_NULL != NAP)
    {
        // 16bit
        *NAP = (T_U16)(((T_U16)byteAddr[5] << 8) + byteAddr[4]);
    }

    if (AK_NULL != UAP)
    {
        // 8 bit
        *UAP = (T_U8)byteAddr[3];
    }
    
    if (AK_NULL != LAP)
    {
        // 24 bit
        *LAP = ((T_U32)byteAddr[2] << 16) + \
            ((T_U32)byteAddr[1] << 8) + byteAddr[0];
    }
    
    return AK_TRUE;
}

/**
 * @BRIEF	check if btdev addr is valid
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 *byteAddr
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_AddrValid(T_U8 *byteAddr)
{
    T_U16 NAP = 0;
    T_U8  UAP = 0;
    T_U32 LAP = 0;

    if (AK_FALSE == BtDev_AddrToStructure(byteAddr, &NAP, &UAP, &LAP))
    {
        return AK_FALSE;
    }

    if ((0 == LAP) && (0 == UAP) && (0 == NAP))
    {
        return  AK_FALSE; 
    }
    else if (LAP >= 0x9E8B00 && LAP <= 0x9E8B3F)
    {
        return  AK_FALSE; 
    }
    else if (LAP > 0xFFFFFF)
    {
        return  AK_FALSE; 
    }
    
    return AK_TRUE;
}

/**
 * @BRIEF	get the default local bluetooth address
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @RETURN	T_BOOL
 * @RETVAL	BTCTRL_SUCCESS: successful
 *			else:  failed.
 */
static T_BOOL BtDev_GetDefaultAddr(T_U8 *byteAddr, T_BOOL isResetLap)
{
    T_U16 napAddr = 0x0075;//16bit[vendor]
    T_U8  uapAddr = 0x58;//8bit[vendor]
    T_U32 lapAddr = 0x007199;//24bit [user]
    T_U32 timeOut = 200;

    if (isResetLap)
    {
		byteAddr[0] = (napAddr>>8)&0xFF;
		byteAddr[1] = napAddr&0xFF;
     	byteAddr[2] = uapAddr;   
		while(timeOut--)
        {
            lapAddr = Fwl_GetRand(0xFFFFFF);

			byteAddr[3] = (T_U8)(lapAddr>>16)&0xFF;
			byteAddr[4] = (T_U8)(lapAddr>>8)&0xFF;
			byteAddr[5] = (T_U8)lapAddr &0xFF;
            if (AK_TRUE == BtDev_AddrValid(byteAddr))
            {
                break;
            }
        }
    }

    return AK_TRUE;
}

/**
 * @BRIEF	start up btdev,including set dev name ,addr,init btlib
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_Init(T_VOID)
{
	T_BTDEV_INFO *btdev_crl;
	
	if(AK_NULL != gBtDevCtrl)
	{
		return ;
	}
	gBtDevCtrl = Fwl_Malloc(sizeof(T_BTDEV_CTRL));
	if(AK_NULL == gBtDevCtrl)
	{
		akerror("Btdev_Init fail",0,1);
		return ;
	}
	
	memset(gBtDevCtrl,0,sizeof(T_BTDEV_CTRL));
	gBtDevCtrl->PcmId = Pcm_Open();
	BtDev_LoadConfig();
	btdev_crl = &gBtDevCtrl->LocalInfo.info;
	if(AK_NULL == g_Btdev_cfg || !BtDev_AddrValid(g_Btdev_cfg->localInfo.info.BD_ADDR))
	{
		//第一次启动没有配置过
		btdev_crl->classofDevice = 0X240404;
		BtDev_GetDefaultAddr(btdev_crl->BD_ADDR,AK_TRUE);
		memcpy(btdev_crl->name,BT_DEV_NAME,sizeof(BT_DEV_NAME));
		
		{
			//默认启动这三个服务
			btdev_crl->Service = eBTDEV_AVRCP_SERVICE | eBTDEV_A2DP_SERVICE | eBTDEV_HFP_SERVICE;
		}
		
		gBtDevCtrl->a2dpCurVol = DEFAULT_DEC_VOL;
		#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
		BtPlayer_SetEQMode(_SD_EQ_MODE_NORMAL);
		#endif
		BtDev_SaveLocalConfig();
	}
	else
	{	
		memcpy(&gBtDevCtrl->RemoteInfo.Info,&g_Btdev_cfg->pairedList[0],sizeof(T_PAIRLIST_INFO));
		memcpy(&gBtDevCtrl->LocalInfo,&g_Btdev_cfg->localInfo,sizeof(T_BTDEV_LOCAL_INFO));
		gBtDevCtrl->a2dpCurVol = g_Btdev_cfg->a2dpCurVol;
		#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
		BtPlayer_SetEQMode(g_Btdev_cfg->a2dpEQMode);
		#endif
	}
	AkDebugOutput("Btdev default vol:%d\n",gBtDevCtrl->a2dpCurVol);
	BtDev_DestroyConfig();
	
	gBtDevCtrl->IsRuning = AK_TRUE;
	//启动蓝牙初始化蓝牙库
	BtCtrl_LibInit(btdev_crl, BALib2UserMsg);
	
	DeviceHaveConnected = AK_FALSE;
}

/**
 * @BRIEF	free config info which is a spi buf
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_DestroyConfig(T_VOID)
{
	if(AK_NULL != g_Btdev_cfg)
	{
		Fwl_Free(g_Btdev_cfg);
		g_Btdev_cfg = AK_NULL;
	}
}

/**
 * @BRIEF	exit bluetooth
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 */
T_VOID BtDev_DeInit(T_VOID)
{
	if(AK_NULL == gBtDevCtrl)
	{
		return;
	}
	
	gBtDevCtrl->IsRuning = AK_FALSE;
	BtDev_DisConnect();
	//由于推出过程肯定会有断开过程,所收到的蓝牙消息可能不会抛到
	//btplayer状态机，所以要在此处进行资源释放。
	BtPlayer_DeInit();
	BtCtrl_LibDeInit();
	
	Pcm_Close(gBtDevCtrl->PcmId);
	
	BtDev_SaveA2DPConfig(gBtDevCtrl->a2dpCurVol);
	Fwl_Free(gBtDevCtrl);
	gBtDevCtrl = AK_NULL;
}
#pragma arm section code =  "_SYS_BLUE_CONNECT_CODE_"

/**
 * @BRIEF	callback function which can check weather there are service(s) we dont connect,
 			and we connect it
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_TIMER timer_id
 			T_U32 delay
 * @RETURN	T_VOID
 */
T_VOID BtDev_DelayConnect(T_TIMER timer_id, T_U32 delay)
{
	//必须是有服务连接上才可以调用此函数
	T_PAIRLIST_INFO *info = &gBtDevCtrl->RemoteInfo.Info;

	if(isinBtDev() && Blue_IsExitBtPhone())
	{
	//有可能在连接过程被切出去，所以还是要在蓝牙状态才发起重连
		BtCtrl_Connect(info);	
	}
	ConnectDelay = ERROR_TIMER;
}

/**
 * @BRIEF	check if to read spi to connect dev when start up dev
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 */
T_BOOL BtDev_IsStartUpConnect(T_VOID)
{
	return (isinBtDev() && BtDev_AddrValid(gBtDevCtrl->RemoteInfo.Info.BD_ADDR));
}

/**
 * @BRIEF	start a timer to check connect status
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_U16 delay
 * @RETURN	T_VOID
 */
T_VOID BtDev_CheckConnect(T_U16 delay)
{
	if(ConnectDelay == ERROR_TIMER)
	{
		ConnectDelay = Fwl_TimerStart(delay, AK_FALSE, BtDev_DelayConnect);
	}
}

/**
 * @BRIEF	cancel current reconnect and if there is not connected we set it standby
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_BOOL connected
 * @RETURN	T_VOID
 */
T_VOID BtDev_CancelConnect(T_BOOL cancel)
{
	if(cancel && gBtDevCtrl->PreTickOrTime)
	{
		BA_GAP_CancelCreateConnect(gBtDevCtrl->RemoteInfo.Info.BD_ADDR);
	}
	
	gBtDevCtrl->PreTickOrTime = 0;
	if(0 == BtCtrl_GetConnected())
	{
		BtCtrl_SetCurStatus(eBTCTRL_STANDBY);
	}
}

/**
 * @BRIEF	we start connect  if there is some event which we think we can start connect . 
 			all depends on user.
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_CONNECT_EVENT event
 * @RETURN	T_VOID
 */
T_VOID BtDev_EventConnect(T_CONNECT_EVENT event)
{
	T_PAIRLIST_INFO *info;
	T_U32 tick;
	T_BOOL isretry = AK_FALSE;
	T_BOOL OverLoad;
	
	if(g_test_mode)
	{
		return;
	}
	
	tick = Fwl_GetTickCountMs();
	switch (event)
	{
	case eCONNECT_FAIL:
	case eA2DP_FAIL:
	case eAVRCP_FAIL:
	case eHFP_FAIL:
		if( gBtDevCtrl->PreTickOrTime == 0)
		{
			//如果是别人连过来的时候我们不尝试重连
			isretry = AK_FALSE;
		}
		else if (0 != BtCtrl_GetConnected())
		{
			//	如果当前有链接成功的服务，我们看情况要不要在尝试，这里不尝试
			isretry = AK_FALSE;
		}
		else
		{
			isretry = AK_TRUE;
		}
		break;

	case eA2DP_OK:
		BtDev_CheckConnect(1000);
		isretry = AK_FALSE;
		break;
	case eAVRCP_OK:
		BtDev_CheckConnect(1000);
		isretry = AK_FALSE;
		break;
	case eHFP_OK:
		//检测A2DP的连接，仅在主动重连时检测对其他的检测
		if(gBtDevCtrl->PreTickOrTime != 0)
		{
			BtDev_CheckConnect(1000);
		}
		isretry = AK_FALSE;
		break;

	default:
		break;
	}

	if(AK_TRUE != isretry)
	{
		BtDev_CancelConnect(AK_FALSE);	
		BtDev_DestroyConfig();
		return;
	}
	
	if(eOUTOFAREA_CONNECT == gBtDevCtrl->Event)
	{
		if(((gBtDevCtrl->PreTickOrTime > tick) && ((tick+(T_U32_MAX-gBtDevCtrl->PreTickOrTime)) >= BTDEV_CONNECT_MAX_DELAY))
			||((gBtDevCtrl->PreTickOrTime < tick) && ((tick - gBtDevCtrl->PreTickOrTime) >= BTDEV_CONNECT_MAX_DELAY)))		
		{
			//如果超时就不再连了
			BtDev_CancelConnect(AK_TRUE);	
			BtDev_DestroyConfig();
			return ;
		}
		else
		{
	        BtDev_CheckConnect(5000);
			return;
		}
	}
	else
	{
	//如果是启动重连，就再次发起重连
		info = Btdev_GetNextDevice(&OverLoad);
		if(AK_NULL == info)
		{
			BtDev_CancelConnect(AK_TRUE);
			BtDev_DestroyConfig();
			return;
		}
		if(AK_TRUE == OverLoad)
		{			
			gBtDevCtrl->ReConnectTimes --;
		}
		if(0 == gBtDevCtrl->ReConnectTimes)
		{
			//如果达到重连次数就退出重连
			BtDev_CancelConnect(AK_TRUE);		
			BtDev_DestroyConfig();
			return ;
		}
	}
	
	BtDev_PrintDev(info);
	if(AK_FALSE == BtCtrl_Connect(info))
	{
		//表示此设备没有可连接的服务了，已经全部连接成功
		BtDev_CancelConnect(AK_TRUE);		
		BtDev_DestroyConfig();
	}
}

/**
 * @BRIEF	connect device
 * @AUTHOR	zhuangyuping
 * @DATE	2013-05-11
 * @PARAM	T_CONNECT_TYPE type: 当前的连接类型，当前是别人连接过来还是我们发起还是断线重连
 * @RETURN	T_VOID
 */
void BtDev_Connect(T_CONNECT_TYPE type)
{
	T_PAIRLIST_INFO *info;
	T_U32 tick;
	
	tick = Fwl_GetTickCountMs();
	if(gBtDevCtrl->PreTickOrTime == 0)
	{
		gBtDevCtrl->Event = type;
		gBtDevCtrl->PreTickOrTime = tick;
		if(eSTARTUP_CONNECT  == type)
		{
			gBtDevCtrl->ReConnectTimes = BTDEV_SYS_RESTART_RECONNECT_TIMES;
			//开机时的重连，需要从spi中获取所需的配对信息
			BtDev_LoadConfig();
		}
	}
	
	if(eOUTOFAREA_CONNECT == type)
	{	
		info = &gBtDevCtrl->RemoteInfo.Info;
	}
	else
	{
		info = Btdev_GetNextDevice(AK_NULL);
		if(AK_NULL == info)
		{
			BtDev_CancelConnect(AK_TRUE);
			BtDev_DestroyConfig();
			return;
		}
	}
	BtDev_PrintDev(info);
	if(AK_FALSE == BtCtrl_Connect(info))
	{
		//表示此设备没有可连接的服务了，已经全部连接成功
		BtDev_CancelConnect(AK_TRUE);
		BtDev_DestroyConfig();
	}

}
T_VOID BtDev_ConnectTask(T_TIMER timer_id, T_U32 delay)
{
	akerror("VME_EVT_RECONNECT_SERVICE",0,1);
	BtCtrl_SetCurStatus(eBTCTRL_RECONNECTING);
	BtDev_Connect(eOUTOFAREA_CONNECT);//远离服务区的重连
}

/**
 * @BRIEF	when remote device delete linkkey,we have to retry to connect curdev again
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_TryConnectCurDev(T_TIMER timer_id, T_U32 delay)
{
	akerror("VME_EVT_TRY_RECONNECT_SERVICE",0,1);
	BtCtrl_Connect(&gBtDevCtrl->RemoteInfo.Info);
	gBtDevCtrl->DelLinkKey = AK_FALSE;
}

/**
 * @BRIEF	disconnect with current dev
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */

 T_VOID BtDev_DisConnect(T_VOID)
 {
	T_U32 connected = 0 ;

	if(BtDev_AddrValid(gBtDevCtrl->RemoteInfo.Info.BD_ADDR) && BtCtrl_IsLibOn())
	{	
		BA_GetConnectionStatus(gBtDevCtrl->RemoteInfo.Info.BD_ADDR, &connected);
		
		if(connected)
		{
			AkDebugOutput("++ Start disconnect routine\n");
			BtCtrl_SetCurStatus(eBTCTRL_DISCONNECTING);
			BA_Disconnect(gBtDevCtrl->RemoteInfo.Info.BD_ADDR);
			
			while(connected)
			{			
				BA_Process(0);
				BA_GetConnectionStatus(gBtDevCtrl->RemoteInfo.Info.BD_ADDR, &connected);
			}
			
			AkDebugOutput("-- Finish disconnect routine\n");
		}
	} 
    
 }



/**
 * @BRIEF	check if there have been deleted linkkey 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_IsDelLinkKey(T_VOID)
{
	return gBtDevCtrl->DelLinkKey;
}

/**
 * @BRIEF	check if current dev is phone dev
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8* addr
			T_U32 classOfDivice
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_IsPhoneDevice(T_VOID)
{

	if(g_test_mode)
	{
		return AK_FALSE;
	}
	
	return BtCtrl_IsPhoneDevice(&gBtDevCtrl->RemoteInfo.Info);
}

/**
 * @BRIEF	save remote classofdevice  into ram
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8* addr
			T_U32 classOfDivice
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SetRemoteClassOfDevice(T_U8* addr,T_U32 classOfDivice)
{
	T_PAIRLIST_INFO *remote;
	
	remote = &gBtDevCtrl->RemoteInfo.Info;
	if (0 != memcmp(remote->BD_ADDR,addr,sizeof(T_BD_ADDR)))
	{
		memset(remote->LinkKey,0,sizeof(T_LINK_KEY));
		memcpy(remote->BD_ADDR,addr,sizeof(T_BD_ADDR));
	}
	akerror("classofDevice",classOfDivice,1);
	remote->classofDevice = classOfDivice;
}

/**
 * @BRIEF	save remote linkkey into ram
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8* addr
 			T_U8* LinkKey
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SetRemoteLinkKey(T_U8* addr,T_U8* LinkKey)
{
	T_PAIRLIST_INFO *remote;
	
	remote = &gBtDevCtrl->RemoteInfo.Info;
	if (0 != memcmp(remote->BD_ADDR,addr,sizeof(T_BD_ADDR)))
	{
		memcpy(remote->BD_ADDR,addr,sizeof(T_BD_ADDR));
	}
	BtDev_PrintLinkKey(LinkKey);
	
	memcpy(remote->LinkKey,LinkKey,sizeof(remote->LinkKey));
}
#pragma arm section code 




/**
 * @BRIEF	when remote device request a linkkey ,we get linkkey from spi
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BD_ADDR DevAddr
 			T_LINK_KEY pLinkKey
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE:we have linkkey,AK_FALSE:we have not linkkey
 */
T_BOOL BtDev_GetRemoteDevLinkKey(T_BD_ADDR DevAddr, T_LINK_KEY pLinkKey)
{
	T_U32 i;
	T_BTDEV_CFG *cfg;
	T_BOOL ret = AK_FALSE;
	T_BOOL flag = AK_FALSE;
	T_LINK_KEY link = {0};

	if(BtDev_IsDelLinkKey())
	{
		return AK_FALSE;
	}
	
	if(0 == memcmp(DevAddr,gBtDevCtrl->RemoteInfo.Info.BD_ADDR,BT_ADDR_MAX_LEN))
	{
		 if(0 != memcmp(gBtDevCtrl->RemoteInfo.Info.LinkKey, link,sizeof(T_LINK_KEY)))
		 {
		 	memcpy(pLinkKey, gBtDevCtrl->RemoteInfo.Info.LinkKey, sizeof(T_LINK_KEY));
		 	return AK_TRUE;
		 }
	}
	else
	{
		memset(gBtDevCtrl->RemoteInfo.Info.LinkKey,0, sizeof(T_LINK_KEY));
	}

	if(AK_NULL != g_Btdev_cfg)
	{
		flag = AK_TRUE;
	}		
	cfg = BtDev_LoadConfig();

	if(AK_NULL == cfg)
	{
		return AK_FALSE;
	}
	
	for(i = 0;i< PAIRED_LIST_MAX; i++)
	{
		if(!BtDev_AddrValid(cfg->pairedList[i].BD_ADDR))
		{	
			 break;
		}
		if(0 == memcmp(DevAddr,cfg->pairedList[i].BD_ADDR,BT_ADDR_MAX_LEN))
		{	
			if(0 != memcmp(cfg->pairedList[i].LinkKey, link,sizeof(T_LINK_KEY)))//其实默认从spi读出来的就是ok的
			{
				ret = AK_TRUE;
				memcpy(&gBtDevCtrl->RemoteInfo.Info, &cfg->pairedList[i], sizeof(T_PAIRLIST_INFO));
				memcpy(pLinkKey, cfg->pairedList[i].LinkKey, sizeof(T_LINK_KEY));
				break;
			}
		}
	}
	if(AK_FALSE == flag)
	{
		BtDev_DestroyConfig();
	}
	
	return ret;

}

/**
 * @BRIEF	save dev local info into spi and we clear pairedlist 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveLocalConfig(T_VOID)
{
	T_BTDEV_CFG *cfg;
	T_BTDEV_INFO *pLocalDevInfo = &gBtDevCtrl->LocalInfo.info;
	
	cfg = BtDev_LoadConfig();
	if(AK_NULL == cfg)
	{
		return;
	}
	
	memset(&cfg->localInfo, 0, sizeof(cfg->localInfo));
	memcpy(&cfg->localInfo.info, pLocalDevInfo, sizeof(T_BTDEV_INFO));

	cfg->a2dpCurVol = gBtDevCtrl->a2dpCurVol;
    Profile_WriteData(eCFG_BTDEV, cfg);
	BtDev_DestroyConfig();

}

/**
 * @BRIEF	save dev vol into spi 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U32 vol
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveA2DPConfig(T_U32 vol)
{
	T_BTDEV_CFG *cfg;
	
	cfg = BtDev_LoadConfig();
	if(AK_NULL == cfg)
	{
		return;
	}
	if(vol != cfg->a2dpCurVol)
	{
		cfg->a2dpCurVol = vol;
		#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
		cfg->a2dpEQMode = BtPlayer_GetEQMode();
		#endif
	    Profile_WriteData(eCFG_BTDEV, cfg);
	}
	BtDev_DestroyConfig();

}

/**
 * @BRIEF	when remote device delete linkkey,we have to delete dev info from local spi
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_DelCurRemoteDev(T_BD_ADDR DevAddr)
{
	T_U32 i;
	T_BTDEV_CFG *cfg;
	
	cfg = BtDev_LoadConfig();
	if(AK_NULL == cfg)
	{
		return;
	}
	
	for(i = 0;i< PAIRED_LIST_MAX; i++)
	{
		if(!BtDev_AddrValid(cfg->pairedList[i].BD_ADDR))
		{	
			i = PAIRED_LIST_MAX;
			break;
		}
		if(0 == memcmp(DevAddr,cfg->pairedList[i].BD_ADDR,BT_ADDR_MAX_LEN))
		{	
			break;
		}
	}
	if(PAIRED_LIST_MAX != i)
	{
		memset(&gBtDevCtrl->RemoteInfo.Info.LinkKey,0,sizeof(T_LINK_KEY));
		memmove(&cfg->pairedList[i],&cfg->pairedList[i + 1],(PAIRED_LIST_MAX - (1 + i)) * sizeof(T_PAIRLIST_INFO));
		memset(&cfg->pairedList[PAIRED_LIST_MAX - 1], 0, sizeof(T_PAIRLIST_INFO));
		Profile_WriteData(eCFG_BTDEV, cfg);
		gBtDevCtrl->DelLinkKey = AK_TRUE;
	}
	BtDev_DestroyConfig();

}

/**
 * @BRIEF	check if we can save info into spi,we must have got remote addr,linkkey and classofdevice
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 * @RETVAL	
 */
T_BOOL BtDev_CheckDevInfo(T_VOID)
{
	T_PAIRLIST_INFO *Info = &gBtDevCtrl->RemoteInfo.Info;
	T_LINK_KEY link = {0};
	if(BtDev_AddrValid(Info->BD_ADDR) && (Info->classofDevice != 0) && 
		(0 != memcmp(Info->LinkKey ,link,sizeof(T_LINK_KEY))))
	{
		return AK_TRUE;
	}
	else
	{
		return AK_FALSE;
	}
}

/**
 * @BRIEF	save dev pairedlist info into spi and we updata pairedlist 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_SaveConfig(T_VOID)
{
	T_U32 i;
	T_BTDEV_CFG *cfg;
	
	cfg = BtDev_LoadConfig();
	if(AK_NULL == cfg)
	{
		return;
	}

	for(i = 0;i< PAIRED_LIST_MAX; i++)
	{
		if(!BtDev_AddrValid(cfg->pairedList[i].BD_ADDR))
		{	
			break;
		}
		if(0 == memcmp(gBtDevCtrl->RemoteInfo.Info.BD_ADDR,cfg->pairedList[i].BD_ADDR,BT_ADDR_MAX_LEN))
		{	
			if((0 == i) && (0 == memcmp(gBtDevCtrl->RemoteInfo.Info.LinkKey,cfg->pairedList[i].LinkKey,BT_LINKKEY_LEN)))
			{
				BtDev_DestroyConfig();
				return;
			}
			break;
		}
	}
	if(i != 0)
	{		
		if(PAIRED_LIST_MAX == i)
		{
			i --;
		}
		memmove(&cfg->pairedList[1],&cfg->pairedList[0],i * sizeof(T_PAIRLIST_INFO));
	}
	memcpy(&cfg->pairedList[0],&gBtDevCtrl->RemoteInfo.Info,sizeof(T_PAIRLIST_INFO));

    Profile_WriteData(eCFG_BTDEV, cfg);
	BtDev_DestroyConfig();

}

T_VOID BtDev_PrintAddr(T_BD_ADDR addr)
{
	T_U8 i = 0,j = 0,tmp;
	T_U8 btaddr[20];
	
	for(; i < 6;i++,j+=3)
	{
		tmp = (addr[i]>>4)&0xF;
		if(tmp <= 9)
		{
			btaddr[j] = tmp + '0';
		}
		else
		{
			btaddr[j] = tmp + 'A' - 10;
		}
		
		tmp = addr[i]&0xF;
		if(tmp <= 9)
		{
			btaddr[j+1] = tmp + '0';
		}
		else
		{
			btaddr[j+1] = tmp + 'A' - 10;
		}
		
		btaddr[j+2] = ':';
	}
	
	btaddr[j-1] = '\0';
	AkDebugOutput("btaddr:%s\n",btaddr);

}
T_VOID BtDev_PrintLinkKey(T_LINK_KEY key)
{
	T_U8 i = 0;
	AkDebugOutput("LinkKey:",key[i]);		
	for(; i < 16;i++)
	{
		AkDebugOutput("%x ",key[i]);		
	}
	AkDebugOutput("\n");		
}

T_VOID BtDev_PrintDev(T_PAIRLIST_INFO *info)
{
	BtDev_PrintAddr(info->BD_ADDR);
	AkDebugOutput("classofDevice:0x%x\n",info->classofDevice);
	BtDev_PrintLinkKey(info->LinkKey);
}



//*************************连接提示音管理*************************//
#ifdef SUPPORT_VOICE_TIP
#pragma arm section code =  "_SYS_BLUE_CODE_"
T_VOID BtDev_PlayTip(T_BTCTRL_STATUS Status)
{
	T_U32 preStatus;

	if(!isinBtDev())
	{
		return;
	}
	preStatus = BtCtrl_GetCurStatus();
	switch(Status)
	{
	case eBTCTRL_RECONNECTING:
		if(DeviceHaveConnected == AK_TRUE)//如果是之前连接成功并播放过连接成功的提示音就播放
		{
			DeviceHaveConnected = AK_FALSE;
			Voice_PlayTip(eBTPLY_SOUND_TYPE_PAIRING, AK_NULL);
		}
		break;	
	case eBTCTRL_CONNECTED:
		if(eBTCTRL_CONNECTED > preStatus)//之前没有报过连接成功的才进行播报
		{		
			Voice_PlayTip(eBTPLY_SOUND_TYPE_CONNECTED, AK_NULL);
			DeviceHaveConnected = AK_TRUE;
		}
		break;
	}
}
#pragma arm section code

//*************************来电响铃管理*************************//

/**
 * @BRIEF	check if it have got the incoming phone number to play ring
 * @AUTHOR	wangxi
 * @DATE	2012-05-23
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 */
#pragma arm section code =  "_SYS_BLUE_HFP_CODE_"
T_BOOL BtDev_IsGetPhone(T_VOID)
{
	if (AK_NULL != g_InPhone)
	{
		return (0 != g_InPhone->Len);
	}
	else
	{
		return AK_FALSE;
	}
}
T_BOOL BtDev_IsPlayingRing(T_VOID)
{
	return (AK_NULL != g_InPhone && g_InPhone->THdl != ERROR_TIMER);
}

/**
 * @BRIEF	get phone number into a global array to play phone 'number' tips
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U8 *phone
 			T_U8 len
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_BOOL BtDev_SetPhone(T_U8 *phone,T_U8 len)
{
	T_BOOL ret = AK_TRUE;
	
	if(AK_NULL == g_InPhone)
	{
		g_InPhone = Fwl_Malloc(sizeof(T_INCOMING_PHONE));
		ret = AK_FALSE;
		g_InPhone->THdl = ERROR_TIMER;
	}		
	g_InPhone->Len = len;
	memcpy(g_InPhone->number,phone,len);
	g_InPhone->Index = T_U8_MAX;
	return ret;
}
T_VOID BtDev_PlayNumber(T_TIMER timer_id, T_U32 delay);

T_VOID BtDev_PlayDu(T_TIMER timer_id, T_U32 delay)
{
	
	if(0 != g_InPhone->Len && g_InPhone->Index != g_InPhone->Len)
	{
		if(ERROR_TIMER != g_InPhone->THdl)
		{
			Fwl_TimerStop(g_InPhone->THdl);
		}
		BtDev_PlayNumber(0,0);//我们会自动再播放一个du，避免时间上的不同步
		g_InPhone->THdl = Fwl_TimerStart(1200, AK_TRUE, BtDev_PlayNumber);//1.5s一次报号
	}
	else
	{
		Voice_PlayTip(eBTPLY_SOUND_TYPE_DU, AK_NULL);
		//播du
		#ifdef OS_WIN32
		AkDebugOutput("\nDU......\n");
		#endif
	}
}

extern T_BOOL g_HFPSCO;
/**
 * @BRIEF	we play 'number' and when we all number is played,we play 'du' again 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_PlayNumber(T_TIMER timer_id, T_U32 delay)
{
	g_InPhone->Index ++;
	if(g_InPhone->Index < g_InPhone->Len)
	{
		//播放相应index的号码
		#ifdef OS_WIN32
		AkDebugOutput("\n%d......\n",g_InPhone->number[g_InPhone->Index] - '0');
		#endif
		{
			T_U32 number[] = {
				eBTPLY_SOUND_TYPE_ZERO,
			    eBTPLY_SOUND_TYPE_ONE,
				eBTPLY_SOUND_TYPE_TWO,
			    eBTPLY_SOUND_TYPE_THREE,
			    eBTPLY_SOUND_TYPE_FOUR,
			    eBTPLY_SOUND_TYPE_FIVE,
			    eBTPLY_SOUND_TYPE_SIX,
			    eBTPLY_SOUND_TYPE_SEVEN,
			    eBTPLY_SOUND_TYPE_EIGHT,
			    eBTPLY_SOUND_TYPE_NINE
			};
			Voice_PlayTip(number[g_InPhone->number[g_InPhone->Index] - '0'], AK_NULL);
		}
	}
	else//当报号报完了，停止报号
	{
		if(g_InPhone->THdl != ERROR_TIMER)
		{
			Fwl_TimerStop(g_InPhone->THdl);
		}
		if(AK_FALSE == g_HFPSCO)
		{
			//改成报du
			BtDev_PlayDu(0,0);//默认启动一次播放
			g_InPhone->THdl = Fwl_TimerStart(2500, AK_TRUE, BtDev_PlayDu);//2.5s一次du
		}
		else
		{
			//如果有sco数据，那么选择播放彩铃
			g_InPhone->THdl = ERROR_TIMER;
		}
	}
}
/**
 * @BRIEF	we stop playing ring
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_StopRing(T_VOID)
{
	//此函数只在几个地方调用:
	//1.接电话，2.退出状态机(包括本地拒接电话，或者对方挂断电话)
	if(AK_NULL != g_InPhone)
	{
		if(g_InPhone->THdl != ERROR_TIMER)
		{
			Fwl_TimerStop(g_InPhone->THdl);
		}
		Voice_PassAllTip(); 
		Fwl_Free(g_InPhone);
		AkDebugOutput("BtDev_StopRing\n");
		g_InPhone = AK_NULL;
	}
}

/**
 * @BRIEF	when there is a incoming phone ,we may play 'du' or play 'number' 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID BtDev_PlayRing(T_VOID)
{
	//此函数最多只能调用两次，
	//一次是来电电话状态改变，表示来电
	//一次是获取到号码
	if(AK_NULL == g_InPhone)
	{
		g_InPhone = Fwl_Malloc(sizeof(T_INCOMING_PHONE));
		memset(g_InPhone,0,sizeof(T_INCOMING_PHONE));
		g_InPhone->THdl = ERROR_TIMER;
	}	
	if(g_InPhone->THdl == ERROR_TIMER)
	{
		BtDev_PlayDu(0,0);//默认启动一次播放
		if(g_InPhone->THdl == ERROR_TIMER)
		{
			g_InPhone->THdl = Fwl_TimerStart(2500, AK_TRUE, BtDev_PlayDu);//2.5s一次du
		}
	}
}
#pragma arm section code
#endif
