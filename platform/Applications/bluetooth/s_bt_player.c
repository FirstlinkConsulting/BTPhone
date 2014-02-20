/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_bt_player.c
 * @BRIEF a2dp process 
 * @Author：zhuangyuping
 * @Date：2013-04-25
 * @Version：
**************************************************************************/
#include "Eng_BtPlayer.h"
#include "vme.h"
#include "m_event.h"
#include "fwl_keypad.h"
#include "fwl_FreqMgr.h"
#include "m_event_api.h"
#include "eng_debug.h"
#include "Eng_BtPhone.h"
#include "BtDev.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"
#include "Fwl_MicroTask.h"
#include "Fwl_mount.h"
#include "Fwl_osMalloc.h"

/********macro definition********/



extern T_VOID BtDev_ConnectTask(T_TIMER timer_id, T_U32 delay);

extern T_BOOL isinBtDev(T_VOID);
void stdb_ChangeMode(void);


/**
 * @BRIEF	init a2dp machine
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	
 * @RETVAL	
 */
T_VOID initbt_player(T_VOID)
{
	//配置本地信息
	Fwl_FreqPush(FREQ_APP_BLUE);
	Fwl_MemDevUnMount(MMC_SD_CARD);
	Fwl_Freq_Add2Calc(FREQ_PLL_MAX >> 1);
    AkDebugOutput("initbt_player:%d\n", Fwl_GetUsedMem());//方便统计内存泄露
	BtDev_Init();
#ifdef SUPPORT_VOICE_TIP	 
	Voice_PlayTip(eBTPLY_SOUND_TYPE_PAIRING, AK_NULL);
#endif
	BtCtrl_SetLibOn(AK_TRUE);
}

/**
 * @BRIEF	a2dp machine process
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_EVT_CODE event
 			T_EVT_PARAM *pEventParm
 * @RETURN	unsigned char
 * @RETVAL	
 */
#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
unsigned char handlebt_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad; 

	switch(event)
	{
	case M_EVT_Z00_POWEROFF:
		{
			BtDev_DeInit();
			return 1;
		}
		break;
	case M_EVT_RESTART:
		{
			BtDev_Init();
			BtCtrl_SetLibOn(AK_TRUE);
		}
		break;
	case VME_EVT_DISCONNECT:
		if(0 != BtCtrl_GetConnected())
		{
			BtDev_DisConnect();
			if(!isinBtDev())
			{
				stdb_ChangeMode();
				VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);//如果断链超时，会调用forcedeinit,这个后面可能就没响应了
			}
		}
		break;
	case VME_EVT_IPHONE_BAT_CHANGE:
		{
			//必须hfp连接成功才可以发送电量
			if(BtCtrl_HaveService(eBTDEV_HFP_SERVICE))
			{
				T_U8 bat = pEventParm->c.Param1;
				T_U8 dock = pEventParm->c.Param2;
				
				AkDebugOutput("Send Bat:%d,%d\n",bat,dock);
				BA_HFP_UpdateIphoneBattery(bat,dock);
			}
		}
		break;
	#ifdef SUPPORT_VOICE_TIP
    case VME_EVT_POWER_CHANGE:
		{
			
			BtPlayer_DeInit();
	        if(eBTPLY_SOUND_TYPE_LOWPOWER == pEventParm->c.Param1)
	        {
	            Voice_PlayTip(eBTPLY_SOUND_TYPE_LOWPOWER, AK_NULL);//a2dp播歌时进行低电提示
	        }
	        else if(eBTPLY_SOUND_TYPE_CHARGING == pEventParm->c.Param1)
	        {
	            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGING, AK_NULL);//a2dp播歌时进行充电提示
	        }
	        else if(eBTPLY_SOUND_TYPE_CHARGEOK == pEventParm->c.Param1)
	        {
	            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGEOK, AK_NULL);//a2dp播歌时进行充电完毕提示
	        }	
    	}
        break;
	#endif
	case VME_EVT_AVRCP_DISCONNECT:
		//电脑主动断开时，不会主动断开avrcp，需要手动断开
		if(BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
		{
			BA_AVRCP_Disconnect();
		}
		break;

	case VME_EVT_SAVE_DEVINFO:
		//必须检测到所需要的linkkey和classofdevice都有效时，才进行保存
		if(AK_TRUE == BtDev_CheckDevInfo())
		{
			akerror("VME_EVT_SAVE_DEVINFO",0,1);
			BtDev_SaveConfig();
		}
		break;
	case VME_EVT_BTPLAYER_INIT:
		{
			if(BtCtrl_HaveService(eBTDEV_A2DP_SERVICE))
			{
				//由于目前的初始化解码器都是在A2DP_STREAM_START事件触发的，默认就是会进行播放。
				akerror("VME_EVT_BTPLAYER_INIT",BtDev_GetVolume(),1);
				BtPlayer_Init(BtDev_GetVolume());
			}
		}
		break;
	case VME_EVT_BTPLAYER_STOP:
		{
			#ifdef OS_WIN32
			T_U32 len = 1;
			while(len != 0)
			{
				Wave_DefaultCB(AK_NULL, &len);
			}
			#endif
			BtPlayer_SetDecing(AK_FALSE);//先暂停解码，否则有可能播放状态会在解码过程中被重新赋值
			BtPlayer_SetPlaying(AK_FALSE);
		}
		break;		
	case VME_EVT_BTPLAYER_DEINIT:
		{
			//彻底关闭解码器，收到A2DP_STREAM_CLOSE事件时触发
			BtPlayer_DeInit();
			akerror("VME_EVT_BTPLAYER_DEINIT",0,1);
		}
		break;
		
	case VME_EVT_CONNECT_SERVICE:
		{
			if(isinBtDev())
			{
				akerror("VME_EVT_CONNECT_SERVICE",0,1);
				//开机启动时进行重连，主动重连由系统自身进行驱动
				BtCtrl_SetCurStatus(eBTCTRL_RECONNECTING);
				BtDev_Connect(eSTARTUP_CONNECT);
			}
		}
		break;
		
	case VME_EVT_CONNECT_FAIL:
		{
			akerror("VME_EVT_CONNECT_FAIL",0,1);
			BtDev_EventConnect(eCONNECT_FAIL);
		}
		break;
	case VME_EVT_CONNECT_HFP:
		{
			T_U32 connectevent = ((pEventParm->c.Param1 == BS_OK)?eHFP_OK:eHFP_FAIL);
			if(pEventParm->c.Param1 == BS_OK && !BtCtrl_HaveService(eBTDEV_HFP_SERVICE))
			{
				;//有可能此时已经断开
			}
			else
			{
				akerror("VME_EVT_CONNECT_HFP",connectevent,1);
				BtDev_EventConnect(connectevent);
			}
		}
		break;
	case VME_EVT_CONNECT_A2DP:	
		{
			T_U32 connectevent = ((pEventParm->c.Param1 == BS_OK)?eA2DP_OK:eA2DP_FAIL);
			

			if(pEventParm->c.Param1 == BS_OK && !BtCtrl_HaveService(eBTDEV_A2DP_SERVICE))
			{
				;//有可能此时已经断开
			}
			else
			{
				akerror("VME_EVT_CONNECT_A2DP",connectevent,1);
				BtDev_EventConnect(connectevent);
			}
		}
		break;
	case VME_EVT_CONNECT_AVRCP:
		{
			T_U32 connectevent = ((pEventParm->c.Param1 == BS_OK)?eAVRCP_OK:eAVRCP_FAIL);
			if(pEventParm->c.Param1 == BS_OK && !BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
			{
				;//有可能此时已经断开
			}
			else
			{
				akerror("VME_EVT_CONNECT_AVRCP",connectevent,1);
				BtDev_EventConnect(connectevent);
			}
		}
		break;
	case VME_EVT_BLUEA_TIMEOUT:
		{
			akerror("VME_EVT_BLUEA_TIMEOUT",0,0);
			BA_ForceDeinit();
			VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
		}
		break;
	case M_EVT_USER_KEY:
		{
	        keyPad.id = (T_eKEY_ID)pEventParm->c.Param1;
	        keyPad.pressType= (T_ePRESS_TYPE)pEventParm->c.Param2;
			switch(keyPad.id)
			{
            case kbEQ:
				#ifdef SUPPORT_A2DP_EQ_CONTROL_EN
				BtPlayer_SetEQ();//其实是按键响应模式
				#endif
				break;
            case kbVOLADD:
            case kbVOLSUB:
                if((keyPad.pressType == PRESS_SHORT)||(keyPad.pressType == PRESSING))
                {
                    T_BOOL flag = (keyPad.id == kbVOLADD)?AK_TRUE:AK_FALSE;
				    BtDev_SetVolume(flag);
                }
                break;
			case kbLEFT:
			case kbRIGHT:
				if(BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
				{
					if(keyPad.pressType == PRESS_SHORT)
					{
						T_BOOL flag = (keyPad.id != kbRIGHT)?AK_TRUE:AK_FALSE;
						BtPlayer_Switch(flag);
					}
					else
					{
						T_BOOL flag = (keyPad.id == kbRIGHT)?AK_TRUE:AK_FALSE;
						BtDev_SetVolume(flag);
					}
				}
				break;
			case kbPRE:
			case kbNEXT:
				if(BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
				{
					if(keyPad.pressType == PRESS_SHORT)
					{
						T_BOOL flag = (keyPad.id != kbNEXT)?AK_TRUE:AK_FALSE;
						BtPlayer_Switch(flag);
					}
				}
                break;
            case kbMUTE:
                if(keyPad.pressType == PRESS_SHORT)
                {
                    if(BtPlayer_IsAutoMute())
                    {
                        BtPlayer_AutoMute(AK_FALSE);
                    }
                    else
                    {
                        BtPlayer_AutoMute(AK_TRUE);
                    }
                }
                break;
			case kbOK:
				if(keyPad.pressType == PRESS_SHORT)
				{
					if(BA_HFP_PHONE_ONGOING_CALL == BtPhone_GetPhoneStatus())
					{
						BA_HFP_ConnectSCO();
					}
					else if(eBTCTRL_RECONNECTING == BtCtrl_GetCurStatus())
					{
						BtDev_CancelConnect(AK_TRUE);
					}
					else if(BtCtrl_HaveService(eBTDEV_AVRCP_SERVICE))
					{
						BtPlayer_Play((T_BOOL)!BtPlayer_IsPlaying());//如果播放，发暂停，如果暂停，发播放
					}
				}
				break;
			case kbBT://BT键
				if(keyPad.pressType == PRESS_SHORT)
				{
					if(BtCtrl_HaveService(eBTDEV_HFP_SERVICE) && BtDev_IsPhoneDevice())
					{
						//如果在bt_player状态机中按此键，就是拨打电话
						T_U8 status = BtPhone_GetPhoneStatus();//获取当前电话状
						akerror("Get hfp status: ",status,1);
						if(BA_HFP_PHONE_STANDBY == status)
						{
							akerror("Run BtPhone_CallDial Result:",BtPhone_CallDial(AK_NULL),1); 
						}
						else if(BA_HFP_PHONE_ONGOING_CALL == status)
						{
							akerror("Run BtPhone_CancelCall Result:",BtPhone_CancelCall(),1);
						}
					}
					else if(0 == BtCtrl_GetConnected() && eBTCTRL_STANDBY == BtCtrl_GetCurStatus())
					{
						VME_EvtQueuePut(VME_EVT_CONNECT_SERVICE, AK_NULL);
					}
					else
					{
						akerror("you can't use call function",0,1);
					}
				}
				else if(keyPad.pressType == PRESS_LONG || keyPad.pressType == PRESSING)
				{
					if(0 != BtCtrl_GetConnected())
					{
						BtDev_DisConnect();
						if(!isinBtDev())
						{
							stdb_ChangeMode();
							VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);//如果断链超时，会调用forcedeinit,这个后面可能就没响应了
						}
					}
				}
				break;
			case kbFUNC:
				//stdb_ChangeMode();
				//VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);前处理做了，此处不用做
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
 * @BRIEF	exit a2dp machine 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	
 * @RETURN	
 * @RETVAL	
 */
T_VOID exitbt_player(T_VOID)
{

	BtDev_DeInit();
    AkDebugOutput("exitbt_player:%d\n", Fwl_GetUsedMem());
	Fwl_MemDevMount(MMC_SD_CARD);
	Fwl_FreqPop();
	Fwl_Freq_Clr_Add();
}

T_VOID paintbt_player(void)
{
}


