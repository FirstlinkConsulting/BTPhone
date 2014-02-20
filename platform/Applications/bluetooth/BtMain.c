

#include "ba_user_msg.h"
#include "anyka_types.h"
#include "vme.h"
#include "m_event.h"
#include "ba_lib_api.h"
#include "Eng_BtCtrl.h"
#include "eng_debug.h"
#include "m_event_api.h"
#include "Eng_BtPlayer.h"
#include "Eng_BtPhone.h"
#include "BtDev.h"
#include "ba_hfp.h"
#include "Eng_VoiceTip.h"
#include "fwl_timer.h"



extern T_VOID BtDev_ConnectTask(T_TIMER timer_id, T_U32 delay);
extern T_BOOL isinBtDev(T_VOID);
extern T_BOOL Blue_IsExitBtPhone(T_VOID);
extern T_VOID Blue_SetExitBtPhone(T_BOOL exit);
extern T_VOID Blue_SetProcessTicks(T_U32 ticks);

T_BOOL g_HFPSCO = AK_FALSE;//sco�Ƿ���
//T_BOOL gA2DPStart = AK_FALSE;//��ͨ������ǰ�Ƿ���a2dp �Ĳ�������
T_BOOL g_ExitBtPhone = AK_TRUE;//�Ƿ��Ѿ������˳�ͨ��
T_U32  g_ProcessTicks = 0;//ִ��ba_process�����ʱ�䣬Ŀǰֻ�ڶϿ��׶�ʹ��
T_TIMER ProcessDelay = ERROR_TIMER;
static T_TIMER SCOCntedCheck = ERROR_TIMER;
extern T_BOOL g_test_mode;

#pragma arm section code = "_SYS_BLUE_HFP_CODE_"

/**
 * @BRIEF	when connected with a non-phone device ,check the sco link is built after call ongoing, 
 			if link is not existed, initialize a SCO connection. this for mass production test.
 * @AUTHOR	lizhenyi
 * @DATE	2014-02-11
 * @PARAM	timer_id
			delay
 * @RETURN	T_VOID
 * @RETVAL	
 */
static T_VOID Blue_DelaySCOCheck(T_TIMER timer_id, T_U32 delay)
{

	if((g_HFPSCO != AK_TRUE) && !remoteDeviceIsPhone())
	{
		akerror("Device create a sco link to remote device ",0,1);
		BA_HFP_ConnectSCO();
	}
	
	SCOCntedCheck = ERROR_TIMER;
}


/**
 * @BRIEF	deal with hfp msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32 HFP_BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	T_S32 ret = 1;
	switch(msg)
	{
		case HFP_CALL_STATUS_IND:
			akerror("HFP_CALL_STATUS_IND",param1,1);
			{
				switch(param1)
				{
					case BA_HFP_PHONE_NULL:
						akerror("BA_HFP_PHONE_NULL",0,1);
						break;

					case BA_HFP_PHONE_DISCONNECTING:
						if(BA_HFP_PHONE_DISCONNECTING_CONNECTION_FAILED == param2)
						{
							T_EVT_PARAM pEventParm;
							
							akerror("BA_HFP_PHONE_DISCONNECTING_CONNECTION_FAILED",0,1);
							pEventParm.c.Param1 = BS_FAIL;
							VME_EvtQueuePut(VME_EVT_CONNECT_HFP, &pEventParm);
						}
						else
						{
							#ifdef SUPPORT_VOICE_TIP
							BtDev_StopRing();//ֹͣ����
							#endif							//�Ͽ����̻��д���Ϣ����������ֻ��Ҫ�Ͽ����Ľ��
						}
						break;
					case BA_HFP_PHONE_DISCONNECTED:
						if(BtCtrl_HaveService(eBTDEV_HFP_SERVICE))//������������ӱ�ʾ��ʱ����Ϊ�Ͽ����Ӷ����ڵĶϿ�״̬
						{
							T_BOOL flag = BtPhone_IsWorking();
							akerror("BA_HFP_PHONE_DISCONNECTED",0,1);
							BtPhone_DeInit();
							BtCtrl_DelService(eBTDEV_HFP_SERVICE);
							if(!Blue_IsExitBtPhone() && flag)
							{					
								VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
								Blue_SetExitBtPhone(AK_TRUE);
							}	
						}
						else
						{
							;//����Ǳ�ʾ����ʱ����һ����Ϣ˵��ǰ�ǶϿ�����״̬�ˣ����Խ�������
						}
						break;
					case BA_HFP_PHONE_STANDBY:
						{
							akerror("BA_HFP_PHONE_STANDBY",0,1);
							Blue_SetProcessTicks(A2DP_HFP_PROCESS_TICKS*2);
							if(!BtCtrl_HaveService(eBTDEV_HFP_SERVICE))
							{
								T_EVT_PARAM pEventParm;
								//���֮ǰû�����ӹ�����ʾ��ǰ��standby�������ϵı��
								akerror("HFP CONNECT OK",0,1);
								#ifdef SUPPORT_VOICE_TIP
								BtDev_PlayTip(eBTCTRL_CONNECTED);
								#endif
								BtCtrl_AddService(eBTDEV_HFP_SERVICE);
								pEventParm.c.Param1 = BS_OK;
								VME_EvtQueuePut(VME_EVT_CONNECT_HFP, &pEventParm);
								Blue_SetExitBtPhone(AK_TRUE);
							}
							else
							{
								akerror("PHONE CLOSE",0,1);
								if(!Blue_IsExitBtPhone() && BtPhone_IsWorking())
								{
									//�������״̬��������û�н�����SCO��·��ֻ���ڴ˴��˳�
									#ifdef SUPPORT_VOICE_TIP
									VME_EvtQueuePut(VME_EVT_STOP_RING, AK_NULL);
									#endif									
									VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);	
									Blue_SetExitBtPhone(AK_TRUE);
								}
								BtCtrl_SetCurStatus(eBTCTRL_CONNECTED);
							}
							
							BA_BypassSCORxData(AK_TRUE);
							g_HFPSCO = AK_FALSE;
						}
						break;

					case BA_HFP_PHONE_INCOMING_CALL:
						if(eBTCTRL_DISCONNECTING != BtCtrl_GetCurStatus())
						{
							akerror("BA_HFP_PHONE_INCOMING_CALL",0,1);
							Blue_SetExitBtPhone(AK_FALSE);
							BtCtrl_SetCurStatus(eBTCTRL_PHONE_INCOMING);
							if(BtPlayer_IsWorking())
							{
								VME_EvtQueuePut(VME_EVT_BTPLAYER_DEINIT, AK_NULL);
								//�����ǰ����a2dp����Դ���ȹر��ͷ�
							}
							if(!BtPhone_IsWorking())
							{
								VME_EvtQueuePut(M_EVT_BTPHONE, AK_NULL);
							}
							#ifdef SUPPORT_VOICE_TIP
							Voice_PassAllTip(); 
							VME_EvtQueuePut(VME_EVT_BTPHONE_RING, AK_NULL);
							#endif
						}
						break;

					case BA_HFP_PHONE_OUTGOING_CALL:
						if(eBTCTRL_DISCONNECTING != BtCtrl_GetCurStatus())
						{
							akerror("BA_HFP_PHONE_OUTGOING_CALL",0,1);
							Blue_SetExitBtPhone(AK_FALSE);
							BtCtrl_SetCurStatus(eBTCTRL_PHONE_OUTGOING);
							if(BtPlayer_IsWorking())
							{
								//�����ǰ����a2dp����Դ���ȹر��ͷ�
								VME_EvtQueuePut(VME_EVT_BTPLAYER_DEINIT, AK_NULL);
							}
							#ifdef SUPPORT_VOICE_TIP
							Voice_PassAllTip(); 
							#endif
							if(!BtPhone_IsWorking())
							{
								VME_EvtQueuePut(M_EVT_BTPHONE, AK_NULL);
							}
						}
						break;

					case BA_HFP_PHONE_ONGOING_CALL:
						if(eBTCTRL_DISCONNECTING != BtCtrl_GetCurStatus())
						{
							akerror("BA_HFP_PHONE_ONGOING_CALL",0,1);
							Blue_SetExitBtPhone(AK_FALSE);
							BtCtrl_SetCurStatus(eBTCTRL_PHONE_ONGOING);
							if(!BtPhone_IsWorking())
							{
								VME_EvtQueuePut(M_EVT_BTPHONE, AK_NULL);
							}
							#ifdef SUPPORT_VOICE_TIP
							Voice_PassAllTip(); 
							VME_EvtQueuePut(VME_EVT_STOP_RING, AK_NULL);
							#endif
							VME_EvtQueuePut(VME_EVT_ACCEPT_PHONE, AK_NULL);
							
							if(SCOCntedCheck == ERROR_TIMER)
							{
								SCOCntedCheck = Fwl_TimerStart(500, AK_FALSE, Blue_DelaySCOCheck);
							}
							
						}
						break;

					default :
						break;
				}
			}
			break;
		case HFP_PHONE_NUMBER_IND:
			{
				akerror("HFP_PHONE_NUMBER_IND",0,0);
				#ifdef SUPPORT_VOICE_TIP
				if(!BtDev_IsGetPhone() && eBTCTRL_PHONE_ONGOING != BtCtrl_GetCurStatus())
				{
					//ֱ�ӱ��ţ�����һ��ȫ������������绰
					BtDev_SetPhone((T_U8*)param1, (T_U8)param2);
					if(BtPhone_IsWorking())
					{
						VME_EvtQueuePut(VME_EVT_BTPHONE_RING, AK_NULL);
					}	
				}
				#endif
			}
	        break;
		case HFP_WAVE_OPEN:
			if(eBTCTRL_DISCONNECTING != BtCtrl_GetCurStatus())
			{
				akerror("HFP_WAVE_OPEN",0,1);
				Blue_SetExitBtPhone(AK_FALSE);
				Blue_SetProcessTicks(HFP_PROCESS_TICKS*2);
				if(BtPlayer_IsWorking())
				{
					VME_EvtQueuePut(VME_EVT_BTPLAYER_DEINIT, AK_NULL);
					//�����ǰ����a2dp����Դ���ȹر��ͷ�
				}
				if(!BtPhone_IsWorking())
				{
	            	VME_EvtQueuePut(M_EVT_BTPHONE, AK_NULL);
					if(BA_HFP_PHONE_ONGOING_CALL == BtPhone_GetPhoneStatus())
					{
						VME_EvtQueuePut(VME_EVT_ACCEPT_PHONE, AK_NULL);
					}
				}
			}
			g_HFPSCO = AK_TRUE;
			break;
		case HFP_WAVE_CLOSE:
			akerror("HFP_WAVE_CLOSE",0,1);
			Blue_SetProcessTicks(A2DP_HFP_PROCESS_TICKS*2);
			if(!Blue_IsExitBtPhone() && BtPhone_IsWorking())
			{
				#ifdef SUPPORT_VOICE_TIP
				VME_EvtQueuePut(VME_EVT_STOP_RING, AK_NULL);
				#endif
            	VME_EvtQueuePut(M_EVT_EXIT, AK_NULL);
				Blue_SetExitBtPhone(AK_TRUE);
			}
			BA_BypassSCORxData(AK_TRUE);
			break;
		case HFP_IS_IPHONE_IND:
			akerror("HFP_IS_IPHONE_IND",param1,1);
			if(AK_TRUE == (T_BOOL)param1)
			{
				//֧�ֵ�������,��������֮��ı��뷢һ�˵����������µ����ı�ŷ�
				BtDev_SetSendBat(eIPHONE_BAT_MUST_SEND);
			}
			else
			{
				//��֧�ֵ������ͣ���Զ����
				BtDev_SetSendBat(eIPHONE_BAT_NO_SEND);
			}
			break;
			
		case HFP_WAVE_RECEIVE:
			//�����ڷǹر�״̬�Ž������ݣ�������
			if(isinBtDev())
			{
				#ifdef SUPPORT_VOICE_TIP
				if(!Voice_IsWroking() && !BtDev_IsPlayingRing() && BtPhone_IsWorking())
				#else
				if(BtPhone_IsWorking())
				#endif
				{
					BtPhone_LoadSCOData((T_U8*)param1,(T_U16)param2);
				}
			}
			break;
		case HFP_SPEAKER_GAIN_IND:
			akerror("HFP_SPEAKER_GAIN_IND:",param1,1);
			BtPhone_SetSpeakerGain((T_U8)param1);
			break;
		case HFP_STATUS_ERROR_IND:
			akerror("HFP_STATUS_ERROR_IND:",param1,1);
			break;
		default:
			break;
			
	}
	return ret;
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/**
 * @BRIEF	deal with a2dp msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32 A2DP_BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	T_S32 ret = 1;

	switch(msg)
	{
	case A2DP_CONNECT_IND:
		//ֻ�ڱ�����������ʱ�Ż����������û��Ƿ���ܣ�����1���ܡ�
		akerror("A2DP_CONNECT_IND ",param1,1);
		if(BS_REQUEST == param1)
		{
			ret = 1;
		}
		break;
	case A2DP_DISCONNECT_IND:
		{
			akerror("A2DP_DISCONNECT_IND",param1,1);
			BtCtrl_DelService(eBTDEV_A2DP_SERVICE);
			VME_EvtQueuePut(VME_EVT_AVRCP_DISCONNECT, AK_NULL);
		}
		break;
	case A2DP_STREAM_DATA_IND:
		if(isinBtDev() && BtCtrl_GetCurStatus() < eBTCTRL_PHONE_INCOMING)
		{
			#ifdef SUPPORT_VOICE_TIP
			if(!Voice_IsWroking() && BtPlayer_IsWorking())
			#else
			if(BtPlayer_IsWorking())
			#endif
			{
				BtPlayer_LoadSBCData((T_U8 *)param1, (T_U16)param2);
			}
			#ifdef SUPPORT_VOICE_TIP
			else if(!Voice_IsWroking() && !BtPhone_IsWorking())
			#else
			else if(!BtPhone_IsWorking())
			#endif
			{
				AkDebugOutput("L\n");
				VME_EvtQueuePut(VME_EVT_BTPLAYER_INIT, AK_NULL);
			}
		}
		break;
	case A2DP_STREAM_START:
		akerror("A2DP_STREAM_START",0,1);
		
		if(g_test_mode)
		{
			BtPlayer_Init(BtDev_GetVolume());
			break;
		}
		
		#ifdef SUPPORT_VOICE_TIP
		if(!BtPhone_IsWorking()&& !Voice_IsWroking())
		#else
		if(!BtPhone_IsWorking())
		#endif		
		{	
			VME_EvtQueuePut(VME_EVT_BTPLAYER_INIT, AK_NULL);
		}
		
		break;
	case A2DP_STREAM_STOP:
		akerror("A2DP_STREAM_STOP",0,1);
		VME_EvtQueuePut(VME_EVT_BTPLAYER_STOP, AK_NULL);
		break;	
	case A2DP_STREAM_OPEN:
		{
			//Ŀǰa2dp���ӳɹ������������Ϣ,�˴��������ӳɹ���ʾ��
			T_EVT_PARAM pEventParm;
	
			akerror("A2DP_STREAM_OPEN",0,1);
			#ifdef SUPPORT_VOICE_TIP
			BtDev_PlayTip(eBTCTRL_CONNECTED);
			#endif
			pEventParm.c.Param1 = BS_OK;
			BtCtrl_AddService(eBTDEV_A2DP_SERVICE);
			VME_EvtQueuePut(VME_EVT_CONNECT_A2DP, &pEventParm);
		}
		break;
	case A2DP_STREAM_CLOSE:
		{
			akerror("A2DP_STREAM_CLOSE",0,1);
			BtPlayer_DeInit();
			VME_EvtQueuePut(VME_EVT_BTPLAYER_DEINIT, AK_NULL);
		}
		break;
	case A2DP_PROFILE_STOP:
		akerror("A2DP_PROFILE_STOP",0,1);
		break;
	default:
		akerror("A2DP_UNDEFINE_MSG",msg,1);
		break;
	}	
	return ret;
}
#pragma arm section code

/**
 * @BRIEF	deal with avrcp msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32 AVRCP_BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	T_S32 ret = 1;

	switch(msg)
	{
	case AVRCP_CONNECT_IND:	
		akerror("AVRCP_CONNECT_IND",param1,1);
		if(BS_OK == param1)
		{
			//���ӳɹ�
			T_EVT_PARAM pEventParm;
			#ifdef SUPPORT_VOICE_TIP
			BtDev_PlayTip(eBTCTRL_CONNECTED);
			#endif
			BtCtrl_AddService(eBTDEV_AVRCP_SERVICE);
			pEventParm.c.Param1 = BS_OK;
			VME_EvtQueuePut(VME_EVT_CONNECT_AVRCP, &pEventParm);			
		}
		else if(BS_REQUEST == param1)
		{
			//����Է��������ӣ�����Ҫreturn 1��������������
			ret = 1;
		}
		break;
	case AVRCP_DISCONNECT_IND:
		{
			//avrcp���׶Ͽ�
			akerror("AVRCP_DISCONNECT_IND",param1,1);
			BtCtrl_DelService(eBTDEV_AVRCP_SERVICE);
		}
		break;
	case AVRCP_PROFILE_STOP:
		akerror("AVRCP_PROFILE_STOP",0,1);
		break;
	default:
		akerror("AVRCP_UNDEFINE_MSG",msg,1);
		break;
	}	
	return ret;
}

#pragma arm section code =  "_SYS_BLUE_CONNECT_CODE_"
/**
 * @BRIEF	deal with gap msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32 GAP_BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	T_S32 ret = 1;
	
	switch(msg)
	{
	case GAP_DEVICE_MODE_CHANGE:
		{
			T_EVT_PARAM pEventParm;
			
			pEventParm.c.Param1 = 0;
			akerror("GAP_DEVICE_MODE_CHANGE",isinBtDev(),1);
			
			if(g_test_mode)
			{
				if(isinBtDev())
				{
					#ifdef SUPPORT_HW_TEST
					extern T_BTDEV_CTRL *gBtDevCtrl;
					extern T_VOID BtDev_PrintAddr(T_BD_ADDR addr);
					
					BtCtrl_SetCurStatus(eBTCTRL_STANDBY);
					BtDev_PrintAddr(gBtDevCtrl->RemoteInfo.Info.BD_ADDR);
					BA_A2DP_Connect(gBtDevCtrl->RemoteInfo.Info.BD_ADDR);
					#endif
				}
				break;
			}
			
			if(BtDev_IsStartUpConnect())
			{
			
				//������ʱ����spi����������Ϣ��������
				BtCtrl_SetCurStatus(eBTCTRL_STANDBY);
				VME_EvtQueuePut(VME_EVT_CONNECT_SERVICE, &pEventParm);
			}
			else if(isinBtDev())
			{
				//������ʱ�����Ƿ�spi����������Ϣ��û�оͲ�����
				BtCtrl_SetCurStatus(eBTCTRL_STANDBY);
			}
			else 
			{
				//�ػ����߶ϵ�Ҳ���д���Ϣ
				BtCtrl_SetCurStatus(eBTCTRL_NULL);
			}
		}
		break;

	case GAP_LINK_KEY_REQUEST:
		{	
			T_BOOL ret;
			T_LINK_KEY pLinkKey;
			
			ret = BtDev_GetRemoteDevLinkKey((T_U8*)param1, pLinkKey);
			akerror("GAP_LINK_KEY_REQUEST",ret,1);
			//������linkkeyʱ����linkkey������null
			if(ret)
			{
				BtCtrl_SetLinKey_for((T_U8*)param1, pLinkKey);
			}
			else
			{
				BtCtrl_SetLinKey_for((T_U8*)param1, AK_NULL);
			}
		}
		break;
		
	case GAP_LINK_KEY_GENERATION:
		{		
			akerror("GAP_LINK_KEY_GENERATION",0,1);			
			BtDev_SetRemoteLinkKey((T_U8*)param1, (T_U8*)param2);
			VME_EvtQueuePut(VME_EVT_SAVE_DEVINFO, AK_NULL);
		}
		break;

	case GAP_LINK_KEY_DELETE_REQ:
		{
			//�˴����������ӣ�����������������������Ƚ����Ӧ�ſ��ԡ�
			akerror("GAP_LINK_KEY_DELETE_REQ",0,1);
			BtDev_DelCurRemoteDev((T_U8*)param1);
		}
		break;
	case GAP_LOCAL_BDADDR_CHANGE:
	case GAP_LOCAL_NAME_CHANGE:
	case GAP_DEVICE_CLASS_CHANGE:
	case GAP_INQUIRY_MODE_CHANGE:
	case GAP_PAGE_SPEED_CHANGE:
	case GAP_SSP_MODE_CHANGE:
		break;
		
	default:
		akerror("GAP_UNDEFINE_MSG",msg,1);
		break;
	}
	return ret;
}

/**
 * @BRIEF	deal with mgr msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32 MGR_BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	T_S32 ret = 1;
	switch(msg)
	{
	case MGR_ACL_CONNECTION_REQUEST:
		{
			BtDev_SetRemoteClassOfDevice((T_U8*)param1,(T_U32)param2);		
			VME_EvtQueuePut(VME_EVT_SAVE_DEVINFO, AK_NULL);	
		}
		break;
	case MGR_ACL_DISCONNECT_COMPLETE:
		{
			akerror("MGR_ACL_DISCONNECT_COMPLETE",param2,1);
			//�˴������Ͽ�������ʾ��
			if(0 == BtCtrl_GetConnected())
			{
				if(param2 == 0x8)//0x8 Զ�������,0x13Զ�˶Ͽ�,0x16���ضϿ�
				{
					#ifdef SUPPORT_VOICE_TIP
					BtDev_PlayTip(eBTCTRL_RECONNECTING);
					#endif
					Fwl_TimerStart(5000, AK_FALSE,BtDev_ConnectTask);
				}
				else if(BtDev_IsDelLinkKey())
				{
					//�������ΪԶ���豸linkkey��ʧ�����ǻ����³����������豸
					Fwl_TimerStart(2000, AK_FALSE,BtDev_TryConnectCurDev);
				}
				else
				{
					//�Ͽ����������ص�����standby
					#ifdef SUPPORT_VOICE_TIP
					BtDev_PlayTip(eBTCTRL_RECONNECTING);
					#endif
					BtCtrl_SetCurStatus(eBTCTRL_STANDBY);
				}	
			}
		}
		break;
	case MGR_ACL_CONNECTION_COMPLETE:
		{
			akerror("MGR_ACL_CONNECTION_COMPLETE",param2,1);
			if(0 == param2)
			{
				//���ӳɹ�
				VME_EvtQueuePut(VME_EVT_SAVE_DEVINFO, AK_NULL);
			}
			else
			{
				VME_EvtQueuePut(VME_EVT_CONNECT_FAIL,0);
			}
		}
		break;	
	default:
		break;
	}
	return ret;
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_CODE_"
/**
 * @BRIEF	deal with bluea msg 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BA_USER_MSG msg
			T_S32 param1
			T_S32 param2
 * @RETURN	T_S32
 * @RETVAL	
 */
T_S32  BALib2UserMsg(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2)
{
	if(msg >= MGR_ACL_CONNECTION_REQUEST && msg <= MGR_ACL_CONNECTION_COMPLETE)
	{
		return MGR_BALib2UserMsg(msg,  param1, param2);
	}
	else if(msg >= A2DP_STREAM_DATA_IND && msg <= A2DP_PROFILE_STOP)
	{
		return A2DP_BALib2UserMsg(msg,  param1, param2);
	}
	else if(msg >= AVRCP_CONNECT_IND && msg <= AVRCP_PROFILE_STOP)
	{
		return AVRCP_BALib2UserMsg(msg,  param1, param2);
	}
	else if(msg >= HFP_CALL_STATUS_IND && msg <= HFP_SUPPORTED_FEATURES_IND)
	{
		return HFP_BALib2UserMsg(msg,  param1, param2);
	}
	else if(msg >= GAP_PIN_TYPE_CHANGE && msg <= GAP_RSSI_RESULT_REPLY)
	{
		return GAP_BALib2UserMsg(msg, param1, param2);
	}
	else if(HOST_SHUTDOWN_CNF == msg)
	{
		akerror("HOST_SHUTDOWN_CNF",msg,1);
		//�������׹ر�
		BtCtrl_SetLibOn(AK_FALSE);
	}
	else if(BALIB_ERROR_IND == msg)
	{
		//����������Ϣ
		akerror("BALIB_ERROR_IND",param1,1);
		if(BA_ERR_TIMEOUT == param1)
		{
			VME_EvtQueuePut(VME_EVT_BLUEA_TIMEOUT, AK_NULL);
		}
	}
	else if(BALIB_UNLOADED == msg)
	{
		akerror("BALIB_UNLOADED",param1,1);
	}
	else 
	{
		akerror("UNDEFINE MSG",msg,1);
	}
	return 1;
}
#pragma arm section code


#pragma arm section code = "_bootcode1_"
/**
 * @BRIEF	will call ba_process and phone handle
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_VOID
 * @RETURN	T_VOID
 * @RETVAL	
 */

T_VOID Blue_Process(T_VOID)
{
	if(BtCtrl_IsLibOn())
	{
		BA_Process(g_ProcessTicks);
		BtPhone_Handle();
	}

}
#pragma arm section code

T_VOID SnoopRecord_Write(T_VOID)
{
}

#ifdef OS_WIN32
T_U32 Pcm_Format(T_U8 *dest, T_U8 *src, T_U32 sampCnt)
{
    T_U32 i, val;
    
    for (i=0; i<sampCnt; i++)
    {
        val = ((T_U32 *)src)[i];
		((T_U16 *)dest)[i] = (T_U16)((val >> 16)&(0xFFFF));
    }
    return (sampCnt << 1);
}
#endif
#if 0
/**
 * @BRIEF	if a2dp start before phone close ,we have to save to do a2dp start
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM	T_BOOL flag
 * @RETURN	T_VOID
 */
T_VOID Blue_SetA2DPStart(T_BOOL flag)
{
	gA2DPStart = flag;
}

/**
 * @BRIEF	when phone close ,we have to get if we have got a2dp start msg
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-23
 * @PARAM	T_VOID
 * @RETURN	T_BOOL
 */
T_BOOL Blue_GetA2DPStart(T_VOID)
{
	T_BOOL old = gA2DPStart;
	
	gA2DPStart = AK_FALSE;
	return old;
}
#endif
#pragma arm section code = "_SYS_BLUE_CODE_"
/**
 * @BRIEF	check if it is send exit phone event
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL exit
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_BOOL Blue_IsExitBtPhone(T_VOID)
{
	return g_ExitBtPhone;
}

/**
 * @BRIEF	set send exit phone event
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_BOOL exit
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID Blue_SetExitBtPhone(T_BOOL exit)
{
	g_ExitBtPhone = exit;
}

/**
 * @BRIEF	recovery ba_process max process time(ms) 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U32 ticks
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID Blue_DelayProcessTicks(T_TIMER timer_id, T_U32 delay)
{
	ProcessDelay = ERROR_TIMER;
	g_ProcessTicks = 0;
}

/**
 * @BRIEF	set ba_process max process time(ms) 
 * @AUTHOR	zhuangyuping
 * @DATE	2012-05-21
 * @PARAM	T_U32 ticks
 * @RETURN	T_VOID
 * @RETVAL	
 */
T_VOID Blue_SetProcessTicks(T_U32 ticks)
{
	if(ProcessDelay != ERROR_TIMER)
	{
		Fwl_TimerStop(ProcessDelay);
	}
	g_ProcessTicks = ticks;//��ִ�е���������
	ProcessDelay = Fwl_TimerStart(3000, AK_FALSE, Blue_DelayProcessTicks);
}
#pragma arm section code







