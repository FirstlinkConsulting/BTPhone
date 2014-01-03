/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_bt_phone.c
 * @BRIEF hfp process 
 * @Author：zhuangyuping
 * @Date：2008-04-15
 * @Version：
**************************************************************************/

#include "Eng_BtPhone.h"
#include "vme.h"
#include "m_event_api.h"
#include "fwl_keypad.h"
#include "m_event.h"
#include "eng_debug.h"
#include "BtDev.h"
#include "Fwl_osMalloc.h"

extern T_VOID Blue_SetExitBtPhone(T_BOOL exit);
extern T_BOOL Blue_IsExitBtPhone(T_VOID);


/**
 * @BRIEF	init hfp machine status
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	
 * @RETVAL	
 */
T_VOID initbt_phone(T_VOID)
{
    AkDebugOutput("initbt_phone:%d\n", Fwl_GetUsedMem());
	BtPhone_Init();
}

#pragma arm section code = "_SYS_BLUE_HFP_CODE_"
/**
 * @BRIEF	hfp machine process
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_EVT_CODE event
 			T_EVT_PARAM *pEventParm
 * @RETURN	unsigned char
 * @RETVAL	
 */
unsigned char handlebt_phone(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad; 
	switch (event)
	{
	case M_EVT_Z00_POWEROFF:
		{
			BA_BypassSCORxData(AK_TRUE);
			VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
			VME_EvtQueuePut(M_EVT_Z00_POWEROFF, AK_NULL);
		}
		break;
	case VME_EVT_IPHONE_BAT_CHANGE:
		{
			T_U8 bat = pEventParm->c.Param1;
			T_U8 dock = pEventParm->c.Param2;
			
			AkDebugOutput("Send Bat:%d,%d\n",bat,dock);
			BA_HFP_UpdateIphoneBattery(bat,dock);//向iphone手机发送蓝牙音响的电量
		}
		break;
	#ifdef SUPPORT_VOICE_TIP
	case VME_EVT_BTPHONE_RING:
		{
			akerror("VME_EVT_BTPHONE_RING",0,0);
			BtDev_PlayRing();//播报响铃提示音或者报号提示音
		}
		break;
	case VME_EVT_STOP_RING:
		{
			akerror("VME_EVT_STOP_RING",0,0);
			BtDev_StopRing();//停止响铃
		}
		break;
	#endif
	case VME_EVT_ACCEPT_PHONE:
		{
			akerror("VME_EVT_ACCEPT_PHONE",0,0);
			BtPhone_Accept();//接听电话，我们会设置开始回音消除以及发送录音，以及重新设置dac
		}
		break;
	case VME_EVT_BLUEA_TIMEOUT:
		{ 
			akerror("VME_EVT_BLUEA_TIMEOUT inPhone",0,0);
			VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
			VME_EvtQueuePut(VME_EVT_BLUEA_TIMEOUT, AK_NULL);
		}
		break;
	case M_EVT_USER_KEY:
		{
			T_U8 status = BtPhone_GetPhoneStatus();//获取当前电话状态
	        keyPad.id = (T_eKEY_ID)pEventParm->c.Param1;
	        keyPad.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;
			switch(keyPad.id)
			{
			case kbLEFT:
			case kbRIGHT:
				{
					if(keyPad.pressType == PRESS_SHORT && BA_HFP_PHONE_INCOMING_CALL == status)
					{
						//短按时如果来电则拒接
						akerror("Run BtPhone_RejectCall Result:",BtPhone_RejectCall(),1); 
					}	
					else if(keyPad.pressType == PRESSING || keyPad.pressType == PRESS_LONG)
					{
						//长按时如果正在通话或者来电或者去电,那就调节音量
						T_BOOL Add = (keyPad.id == kbRIGHT)?AK_TRUE:AK_FALSE;
						
						BtPhone_SetSpeakerVolume(Add);
					}
				}
				break;
			case kbOK:
				if(keyPad.pressType == PRESS_SHORT && BA_HFP_PHONE_INCOMING_CALL == status)
				{
					//如果来电按此键拒接
					akerror("Run BtPhone_RejectCall Result:",BtPhone_RejectCall(),1); 
				}
				break;
			case kbBT:
				if(keyPad.pressType == PRESS_SHORT && BA_HFP_PHONE_INCOMING_CALL == status)
				{
					//when incoming call ,we press BT to answer the incoming call
					akerror("Run BtPhone_AnswerCall Result:",BtPhone_AnswerCall(),1); 
				}
				else if(keyPad.pressType == PRESS_SHORT && 
					(BA_HFP_PHONE_ONGOING_CALL == status ||BA_HFP_PHONE_OUTGOING_CALL == status))
				{
					//when outgoing call or ongoing call,we press BT to hang up
					akerror("Run BtPhone_CancelCall Result:",BtPhone_CancelCall(),1); 
				}
				else if(keyPad.pressType == PRESS_LONG || keyPad.pressType == PRESSING)
				{
					if(!Blue_IsExitBtPhone())
					{
						BA_BypassSCORxData(AK_TRUE);
						VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
						Blue_SetExitBtPhone(AK_TRUE);
						VME_EvtQueuePut(VME_EVT_DISCONNECT, AK_NULL);
					}
				}
				break;
			case kbFUNC:
				//stdb_ChangeMode();
				
				//BA_BypassSCORxData(AK_TRUE);
				//VME_EvtQueuePut(M_EVT_EXIT_MODE, AK_NULL);前处理做了，此处不用做
				break;

			default:
				break;
				
			}
		}
		break;
	default:
		break;
	}
	return 0;

}
#pragma arm section code

/**
 * @BRIEF	exit hfp machine process
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID exitbt_phone(T_VOID)
{
	T_EVT_PARAM pEventParm;
	
	pEventParm.c.Param1 = BS_OK;
	BtDev_StopRing();
	BtPhone_DeInit();

	//为了避免是来电话是先连接上HFP导致A2DP相关信息还没有进行连接，
	//会在挂完电话之后进行检测A2DP的连接状况
	if(BtCtrl_HaveService(eBTDEV_A2DP_SERVICE))
	{
		if(!BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
		{
			VME_EvtQueuePut(VME_EVT_CONNECT_A2DP,&pEventParm);
		}
	}
	else
	{
		//check if other service has been connected
		VME_EvtQueuePut(VME_EVT_CONNECT_HFP,&pEventParm);
	}
    AkDebugOutput("exitbt_phone:%d\n", Fwl_GetUsedMem());
}

T_VOID paintbt_phone(void)
{
}


