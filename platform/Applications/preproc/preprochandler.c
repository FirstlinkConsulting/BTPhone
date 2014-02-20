#include "preprochandler.h"
#include "Apl_Public.h"
#include "Fwl_Keypad.h"
#include "Eng_AutoOff.h"
#include "Eng_Standby.h"
#include "Fwl_Timer.h"
#include "log_aud_control.h"
#include "log_record.h"
#include "log_radio_core.h"
#include "Eng_USB.h"
#include "m_state.h"
#include "Fwl_osFS.h"
#include "Fwl_System.h"
#include "AlarmClock.h"
#include "Alarm_Common.h"
#include "Fwl_RTC.h"
#include "Eng_Profile.h"
#include "Fwl_detect.h"
#include "Fwl_usb_s_state.h"
#include "Fwl_LCD.h"
#include "Fwl_led.h"
#include "M_event_api.h"
#include "BtDev.h"
#include "Eng_LedHint.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"
#include "Eng_BtPlayer.h"
#include "sm_port.h"

#pragma arm section zidata = "_bootbss_"
static T_U32 pubTimerCnt = 0;
static T_U8 gPreBtVol = 0;
#pragma arm section zidata

T_BOOL press_bt_flag = AK_FALSE;     //是否通过按【BT】键进入蓝牙模式，状态返回时需要
T_BOOL press_rec_flag = AK_FALSE;    //是否通过按【REC】键进入录音模式，状态返回时需要
T_BOOL pull_out_usb = AK_FALSE;      //拔出USB线，用于蓝牙状态灯显示恢复
T_BOOL usb_in = AK_FALSE;            //不可以用全局变量：gb.usbstat,记录下USB IN，用于按【FUNC】进入U盘模式
T_BOOL enter_udisk_mode = AK_FALSE; //是否进入U盘模式，用于从U盘模式返回之前的状态。
T_BOOL charge_over = AK_FALSE;      //充电是否完成，用于充电满时拔出USB，停止满状态LED显示，恢复现在LED显示状态。

extern T_MEM_DEV_ID Ebk_GetCurDriver(T_VOID);
extern T_BOOL  IsInEbk(T_VOID);
extern T_MEM_DEV_ID Img_GetCurDriver(T_VOID);
extern T_BOOL  IsInImage(T_VOID);
extern T_VOID  MonitorSysTime(T_VOID);
extern T_MEM_DEV_ID FileList_GetCurDriver(T_VOID);
extern T_VOID  Aud_AudCtrlStopAndReserve(T_VOID);
extern T_VOID  usb_charger_detect_handler(T_TIMER timer_id);
extern T_BOOL  IsTomPlayerInit(T_VOID);
extern T_BOOL  IsLineInPlaying(T_VOID);
extern T_BOOL IsAudplaying(T_VOID);
extern T_BOOL IsRecording(T_VOID);
extern T_BOOL  IsRadioPlaying(T_VOID);

extern T_BOOL  Record_IsNULL(T_VOID);
extern T_MEM_DEV_ID Rec_GetCurDriver(T_VOID);
extern T_BOOL  audio_isABPlayer(T_VOID);
extern T_BOOL  IsInRadio(T_VOID);
extern T_BOOL isinBtDev(void);

extern T_BOOL stdb_IsSDMode(void);
extern T_BOOL stdb_IsUSBAudMode(void);
extern void stdb_ChangeSDMode(void);
extern void stdb_ChangeRecordMode(void);
extern void stdb_ChangeRecordPlayMode(void);
extern T_VOID stdb_set_spec_flag(T_BOOL flag);
extern void stdb_ChangeMode(void);
extern void stdb_ChangeUSBMode(void);
extern void stdb_ChangeSelfUpdateMode(void);
extern void stdb_ChangeUHOSTMode(void);
extern void stdb_ChangeBTMode(void);
extern T_BOOL stdb_IsLineInMode(void);
extern void stdb_ChangeLineInMode(void);
extern T_BOOL stdb_first_to_bt(T_VOID);

extern T_BOOL pmu_charge_status(T_VOID);


T_U32 GetPubTimerCnt(T_VOID)
{
    return pubTimerCnt;
}

T_VOID Preproc_TriggerPoweroff(T_VOID)
{
    T_EVT_PARAM EventParm; 
    
    EventParm.w.Param1 = EVENT_TYPE_SWITCHOFF_KEY;
    VME_EvtQueuePut(M_EVT_TIMEOUT, &EventParm);
    Aud_BGProcessMsg(M_EVT_Z00_POWEROFF, &EventParm);
    if (SYSTEM_STATE_NORMAL == gb.init )
    {
        Fwl_LCD_lock(AK_TRUE);
    }
}

#ifdef SUPPORT_LEDHINT
T_VOID LED_reset_show(T_VOID)
{
    T_U8 btstate = 0;
    btstate = BtCtrl_GetCurStatus();

    if(AK_TRUE == charge_over)    //BATTERY_STAT_EXCEEDVOLT == gb.batStat
    {
        charge_over = AK_FALSE;
        LedHint_Stop(LED_FULLBAT);
    }
    
    if(btstate)
    {
        BtCtrl_LEDStop(btstate);
        pull_out_usb = AK_FALSE;
    }
    else
    {
        LedHint_Stop(LED_NORMAL_CHARGE);
    }
    
    switch (SM_GetCurrentSM())
    {  
    case eM_s_linein_play:
    case eM_s_radio_player:
    //case eM_s_uac_main:
    case eM_s_audio_main: 
    case eM_s_audio_record:
        // LED: RED<on:1s, off:1s>
        LedHint_Exec(LED_NORMAL);
        AK_DEBUG_OUTPUT("NORMAL PLAYING AND DISCONNECT CHARGING!\r\n");
        break;
    case eM_s_bt_phone:
    case eM_s_bt_player:
        //according to current BT status
        //BtDev_UpdateCurStatus(AK_TRUE);
        BtCtrl_LEDHint(BtCtrl_GetCurStatus());
        AK_DEBUG_OUTPUT("BT PLAYING AND DISCONNECT CHARGING!\r\n");
        break;
    default:
        LedHint_Exec(LED_NORMAL);
        AK_DEBUG_OUTPUT("BT SPEAKER PLAYING AND DISCONNECT CHARGING!\r\n");
        break;
    }
}

T_VOID LED_change_show(T_VOID)
{
    switch (SM_GetCurrentSM())
    {  
    case eM_s_linein_play:
    case eM_s_radio_player:
    //case eM_s_uac_main:
    case eM_s_audio_main: 
    case eM_s_audio_record:
        // LED: RED<on:1s, off:1s>
        LedHint_Exec(LED_NORMAL_CHARGE);
        AK_DEBUG_OUTPUT("NORMAL PLAYING AND BATTERY CHARGING!\r\n");
        break;
    case eM_s_bt_phone:
    case eM_s_bt_player:
        //according to current BT status
        BtCtrl_LEDHint(BtCtrl_GetCurStatus());
        AK_DEBUG_OUTPUT("BT PLAYING AND BATTERY CHARGING!\r\n");
        break;
    default:
        LedHint_Exec(LED_NORMAL_CHARGE);
        AK_DEBUG_OUTPUT("BATTERY CHARGING!\r\n");
        break;
    }
}
#endif

#pragma arm section code = "_frequentcode_"
static __inline T_BOOL batteryStatCycHandle(T_U32 cnt)
{
	T_U8 Grade;
    T_U8 old_BatStat, old_usbstat, old_chgstat;
    T_EVT_PARAM EventParm;
    old_BatStat = gb.batStat;
    old_usbstat = gb.usbstate;
    old_chgstat = gb.chgStat;
    
    if(0 == cnt%2)
    {
        if((SYSTEM_STATE_INIT==gb.init)||(SYSTEM_STATE_POWEROFF==gb.init))
            return AK_TRUE;
#ifdef OS_ANYKA
        gb.batStat = Eng_VolDetect(&Grade);
        gb.usbstate = Fwl_DetectorGetStatus(DEVICE_CHG);
        gb.chgStat = pmu_charge_status();
        //AK_DEBUG_OUTPUT("O:%d,C:%d\n",old_chgstat,gb.chgStat);
        //AK_DEBUG_OUTPUT("O:%d,C:%d\n",old_usbstat,gb.usbstate);
#else
        gb.batStat = BATTERY_STAT_NOR_L2;
        gb.usbstate = AK_FALSE;
        gb.chgStat = AK_FALSE;
#endif

        if((BATTERY_STAT_LOW_SHUTDOWN == gb.batStat) 
            && (!gb.usbstate))
        {
            EventParm.w.Param1 = EVENT_TYPE_SWITCHOFF_BATLOW;
            VME_EvtQueuePut(M_EVT_TIMEOUT, &EventParm);
            AK_DEBUG_OUTPUT("BATTERY LOW!\r\n");
            VME_EvtQueuePut(M_EVT_Z00_POWEROFF, &EventParm);
            Fwl_LCD_lock(AK_TRUE);
			return AK_TRUE;
        }
        else if((BATTERY_STAT_LOW_WARN == gb.batStat) 
            && (!gb.usbstate)
            && (old_BatStat != gb.batStat))
        {
            #ifdef SUPPORT_VOICE_TIP
                EventParm.c.Param1 = eBTPLY_SOUND_TYPE_LOWPOWER;
                VME_EvtQueuePut(VME_EVT_POWER_CHANGE, &EventParm);
            #endif
            AK_DEBUG_OUTPUT("BATTERY LOW WARN!\r\n");
			#ifdef SUPPORT_LEDHINT
            LedHint_Exec(LED_LOWBAT);
			#endif
        }
        else if((BATTERY_STAT_LOW_WARN == old_BatStat) 
            && (BATTERY_STAT_LOW_WARN != gb.batStat
                 && BATTERY_STAT_LOW_SHUTDOWN != gb.batStat))
        {
            AK_DEBUG_OUTPUT("Exit BATTERY LOW WARN!\r\n");
			#ifdef SUPPORT_LEDHINT
            LedHint_Stop(LED_LOWBAT);
			#endif
        }
        else if((BATTERY_STAT_EXCEEDVOLT == gb.batStat)
            &&(gb.usbstate))
        {
			//充电完成
            //if(!IS_BAT_FULL())
            //if(AK_FALSE == charge_over)           //拔出USB前，电池充满只报一次。
            //if(AK_FALSE == pmu_charge_status()    //断开充电
            if((AK_FALSE == charge_over)
                &&(!gb.chgStat)
                &&(old_chgstat != gb.chgStat))
            {
                charge_over = AK_TRUE;
                
                AK_DEBUG_OUTPUT("BATTERY FULL!\r\n");
                #ifdef SUPPORT_VOICE_TIP
                EventParm.c.Param1 = eBTPLY_SOUND_TYPE_CHARGEOK;
                VME_EvtQueuePut(VME_EVT_POWER_CHANGE, &EventParm);
                #endif
                #ifdef SUPPORT_LEDHINT
                LedHint_Exec(LED_FULLBAT);
                #endif          
            }
        }
        if(old_usbstat != gb.usbstate)
        {
			//充电-退出充电处理
            if(gb.usbstate)
            {
                #ifdef SUPPORT_VOICE_TIP
                EventParm.c.Param1 = eBTPLY_SOUND_TYPE_CHARGING;
                VME_EvtQueuePut(VME_EVT_POWER_CHANGE, &EventParm);
                #endif
				#ifdef SUPPORT_LEDHINT
                LED_change_show();
				#endif
            }
            else
            {
                pull_out_usb = AK_TRUE;
				#ifdef SUPPORT_LEDHINT
                LED_reset_show();
				#endif
            }
        }
		if(eIPHONE_BAT_MUST_SEND == BtDev_IsSendBat() ||
			(isBtEnableSendBat(Grade, gPreBtVol) && eIPHONE_BAT_MAY_SEND == BtDev_IsSendBat()))
		{
			//支持电量发送
			gPreBtVol = Grade;
			EventParm.c.Param1 = gPreBtVol;
			EventParm.c.Param2 = gb.usbstate;
			VME_EvtQueuePut(VME_EVT_IPHONE_BAT_CHANGE, &EventParm);
		}
    }
    return AK_TRUE;
}

static T_BOOL pre_processKey(T_VOID)
{
    if (bglight_state_off())
    {      
        if(IsAudplayer() || !AlmClk_IsPlayerStopped())
        {
            bglight_on(AK_FALSE);
        }
        else
        {
            bglight_on(AK_TRUE);
        }
        #if(NO_DISPLAY == 0)
        return 0;   //只亮屏，不响应按键
        #endif
    }
    
    AutoBgLightOffSet(gb.BgLightTime);
    AutoPowerOffCountSet(gb.PoffTime);
    AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);
    return 1;   
}

unsigned char dduserkeyhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    T_PRESS_KEY phyKey;
  
    phyKey.id = (*pEventParm)->c.Param1;
    phyKey.pressType = (*pEventParm)->c.Param2;
     
    switch(phyKey.id)
    {  
        case kbFUNC:
            if (SYSTEM_STATE_NORMAL == gb.init
                && gb.power_on)//在系统正常启动前不处理
            {                
                if (PRESS_SHORT == phyKey.pressType)
                {                   
                    if(isinBtDev() && (AK_TRUE == press_bt_flag))
                    {
                        press_bt_flag = AK_FALSE;
                        //BT button first,then press the FUNC,so it can return to the last app.
                        VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                    }
                    else if(!Record_IsNULL() && (AK_TRUE == press_rec_flag))
                    {
                        press_rec_flag = AK_FALSE;
                        //REC button first,then press the FUNC,so it can return to the last app.
                        VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                    }
                    else if(usb_in)
                    {
                        if (gb.init != SYSTEM_STATE_INIT)
                        {
                            usb_in = AK_FALSE;
                            enter_udisk_mode = AK_TRUE;
                            (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
                            m_triggerEvent(M_EVT_RETURN_ROOT, *pEventParm);
                            VME_EvtQueuePut(M_EVT_USB_DETECT, AK_NULL);
                            stdb_ChangeUSBMode();
                        }
                    }
                    else
                    {
                        stdb_ChangeMode();
                        stdb_set_spec_flag(AK_TRUE);
                        VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                    }
                    return 0;
                }
                /*else if (PRESS_LONG == phyKey.pressType)
                {
                    //self update
                    //stdb_ChangeSelfUpdateMode();
                    //VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                    #ifdef SUPPORT_VOICE_TIP
                        Voice_PlayTip(eBTPLY_SOUND_TYPE_UPDATE, AK_NULL);
                        Voice_WaitTip();
                    #endif
                    anyka_spi_update_self(AK_FALSE);
                    return 0;
                }*/
            }
            break;
        case kbPOWER:
            if (phyKey.pressType == PRESS_LONG)
            {
                if ((SYSTEM_STATE_NORMAL == gb.init || SYSTEM_STATE_POWEROFF == gb.init)
                    && gb.power_on)
                {
                    Fwl_LEDOn(LED_RED);
                    #ifdef OS_ANYKA
                    AK_PRINTK("***key power off:",phyKey.id, 1);
                    #endif
                    Preproc_TriggerPoweroff();
                }
                /*else
                {
                    #ifdef OS_ANYKA
                    AK_PRINTK("***key powero on:",phyKey.id, 1);
                    #endif
                    Fwl_SysReboot();
                }*/
            }
            break;
        case kbOK:
			//modify by zuangyuping to resolve cq00000038
            //if(!IsInRadio())
            //if (phyKey.pressType == PRESS_LONG)
            //        *event = M_EVT_RETURN;
            break;
        case kbBT:
            if (PRESS_SHORT== phyKey.pressType)
            {
                if (isinBtDev())//||Isuac_valid()
                {
                    //do nothing
                }
                else
                {
                    press_bt_flag = AK_TRUE;
                    stdb_ChangeBTMode();
                    VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                    return 0;
                }
            }
            break;
        case kbRECORD:
            if ((SYSTEM_STATE_NORMAL == gb.init )&&(Fwl_DetectorGetStatus(DEVICE_SD)))//在系统正常启动前不处理record消息
            {    
                if(!isinBtDev() || BtCtrl_IsStandby())
                {
                    if (PRESS_SHORT== phyKey.pressType)
                    {
                        if (Record_IsNULL())
                        {
                            AK_DEBUG_OUTPUT("User Key to Record\n");

#if 0
                            if(IsInRadio())
                            {
                                T_EVT_PARAM evtParam;
                                evtParam.c.Param1= phyKey.id;
                                evtParam.c.Param2= phyKey.pressType;
                                VME_EvtQueuePut(M_EVT_RADIO_REC, AK_NULL);
                                VME_EvtQueuePut(M_EVT_USER_KEY, &evtParam);  
    							stdb_ChangeRecordMode();
    							stdb_set_spec_flag(AK_FALSE);
                            }
                            else 
#endif
                         
                            if(audio_isABPlayer())
                            {
                                akerror("  abplayer record0!", 0, 1);
                                *event = M_EVT_ABPLAY_REC;
                            }
                            else
                            {
                                press_rec_flag = AK_TRUE;
                            	stdb_ChangeRecordMode();
                                VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                                return 0;
                            }
                        }
                    }
                    else if(PRESS_LONG == phyKey.pressType)
                    {
                        //!(IsAudplayer() || (IsAudplayer() && (Aud_AudGetListStyle() != eAUD_VOICE)
                        //AkDebugOutput("\n\nlong record test:%d,%d\n\n", IsAudplayer(), Aud_AudGetListStyle());
						if (IsAudplayer() 
							&& (eAUD_VOICE == Aud_AudGetListStyle() || eAUD_SDVOICE == Aud_AudGetListStyle()))
                        {
                            VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);     //return to the last app
                        }
                        else
                        {
                            stdb_ChangeRecordPlayMode();
                            VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
                            return 0;
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    
    UserCountDownReset();
    return pre_processKey();
}
#pragma arm section code
extern T_U8 UartOverLoadFlag;

#pragma arm section code = "_frequentcode_"
unsigned char ddpubtimerhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    pubTimerCnt ++;
    batteryStatCycHandle(pubTimerCnt);
	#ifndef NO_DISPLAY
    AutoBgLightOffCountDecrease();
	#endif
    AutoPowerOffCountDecrease();
    
    //clear feed watchdog flag
    Fwl_FeedWatchdog();

	if(UartOverLoadFlag)
	{
		AkDebugOutput("system err: the blue uart is overflow!\n");
		UartOverLoadFlag = 0;
	}
	#if 1
    if (StandbyIsOn()
        && !IsAudplaying()  
        && !IsTomPlayerInit()
        && !IsLineInPlaying()
        && !Fwl_DetectorGetStatus(DEVICE_CHG)
        && !IsRadioPlaying()
        && !isinBtDev()
        && !IsRecording())
    {
        StandbyCountDecrease((*pEventParm)->w.Param2);
    }
    else if (eSTAT_VORREC_RECORDING == AudioRecord_GetVorCtrlState())
    {
        UserCountDownReset();
    }
	#endif

    if(0 == pubTimerCnt%2)
    {
        Fwl_SysInfoPrint();
    }
 
    return 1;
}

#ifdef SUPPORT_SDCARD

T_BOOL IsUseSD(T_MEM_DEV_ID index)
{
    T_BOOL needRet = AK_FALSE;
    
#if(NO_DISPLAY == 0)    
    if( IsInVoicePlay() && VoiceGetCurDriver() == MMC_SD_CARD)
    {
        needRet= AK_TRUE;
    }
#endif

    if(!needRet)
    {
        if (IsAudplayer() && Aud_GetCurDriver() == MMC_SD_CARD)
        {
            needRet = AK_TRUE;
        }
        else if ((AudioRecord_GetRecState()!= eSTAT_REC_STOP) && Rec_GetCurDriver() == MMC_SD_CARD)
        {
            needRet = AK_TRUE;
        }
#if(NO_DISPLAY == 0)        
        else if(FileList_GetCurDriver() == MMC_SD_CARD)
        {
            needRet = AK_TRUE;
        }
        else if (IsInEbk() && Ebk_GetCurDriver() == MMC_SD_CARD)
        {
            needRet = AK_TRUE;
        }
#if USE_COLOR_LCD
        else if (IsInImage() && Img_GetCurDriver() == MMC_SD_CARD)
        {
            needRet = AK_TRUE;
        }
#endif
#endif
        else
        {
            needRet = AK_FALSE;
        }
    }
    
    return needRet;
}
#endif


unsigned char ddsdcardhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
#ifdef SUPPORT_SDCARD
    
    T_MEM_DEV_ID  index = (*pEventParm)->c.Param1;
    T_U32 state = (*pEventParm)->c.Param2;
    
    if (SD_DET_PUSH_IN == state)
    {
        //mount the sd 
        //while( CHK_SENDFLAG(send_flag, L2_SENDING));
        if(IsAudplayer())
        {
            Fwl_LCD_lock(AK_TRUE);
            *event = M_EVT_RETURN_ROOT;
            (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
            #if(NO_DISPLAY == 0)
            Aud_BGProcessMsg(M_EVT_SD_IN, AK_NULL);
            #else
            Aud_AudCtrlStopAndReserve();
            #endif
            AK_DEBUG_OUTPUT("audio play sd in\n");
            if(Fwl_MemDevMount(index))
            {
                stdb_ChangeSDMode();
                AK_DEBUG_OUTPUT("stdb_ChangeSDMode\n");
            }
            return 1;
        }
        else if (IsInVoicePlay() && !AlmClk_IsPlayerStopped())
        {
            stdb_ChangeSDMode();
            AK_DEBUG_OUTPUT("stdb_ChangeSDMode11\n");
            VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
            return 0;
        }

		if(isinBtDev() && BtCtrl_IsStandby())
		{
			BtDev_DeInit();//蓝牙和sd不能同时使用,l2不够，所以要先卸载蓝牙
		}
		if (BtCtrl_IsStandby())
		{
			if(Fwl_MemDevMount(index))
	        {      
	            if (bglight_state_off())
	            {
	                bglight_on(AK_TRUE);
	            }

                stdb_ChangeSDMode();
                AK_DEBUG_OUTPUT("stdb_ChangeSDMode22\n");
                VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
            }
			else
			{
				VME_EvtQueuePut(M_EVT_RESTART,AK_NULL);
			}
        }

        return 0;
    }
    else
    {
        if (IsAudplayer() 
            && (eAUD_SDMUSIC == Aud_AudGetListStyle() || eAUD_SDVOICE == Aud_AudGetListStyle() || eAUD_VOICE == Aud_AudGetListStyle()))
        {
            Aud_AudCtrlStopAndReserve();
        }
        //unmount the sd
        Fwl_MemDevUnMount(index);
        //有可能快速插拔，如果加入判断有可能会感应不到sd卡已拔出
        if (stdb_IsSDMode())
        {
            Fwl_LCD_lock(AK_TRUE);
            *event = M_EVT_RETURN_ROOT;
            (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
            #if(NO_DISPLAY == 0)
            Aud_BGProcessMsg(M_EVT_SD_OUT, AK_NULL);
            #endif
            AK_DEBUG_OUTPUT("audio player sd out\n");

            return 1;
        }
        else if (IsUseSD(index))
        {       
            *event = M_EVT_RETURN_ROOT;
            (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
            m_triggerEvent(M_EVT_PULLOUT_SD, AK_NULL);
            AK_DEBUG_OUTPUT("sd message sent\n");
        }
    }
    UserCountDownReset();
#endif
    return 1;
}


#pragma arm section code
unsigned char ddusbhostdiskhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
#ifdef SUPPORT_USBHOST
    if (AK_TRUE == (*pEventParm)->c.Param2)
    {
        if (!stdb_first_to_bt() && Fwl_MemDevMount(USB_HOST_DISK))                     //将U盘MOUNT到文件系统
        {                                      
            *event = M_EVT_RETURN_ROOT;
            stdb_ChangeUHOSTMode();
            AK_DEBUG_OUTPUT("mount udisk ok\n");
        }
        else
        {
            AK_DEBUG_OUTPUT("mount USBDISK failed\n");
        }
    }      
    else
    {
        if (stdb_IsUSBAudMode())
        m_triggerEvent(M_EVT_RETURN_ROOT, AK_NULL);
        
        if(Fwl_MemDevIsMount(USB_HOST_DISK))
        {
            Fwl_MemDevUnMount(USB_HOST_DISK);
        }
    }     
#endif
  return 1;
}

T_BOOL ddusbdetecthandle(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    /*if (gb.init != SYSTEM_STATE_INIT)
    {
        (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
        m_triggerEvent(M_EVT_RETURN_ROOT, *pEventParm);
        VME_EvtQueuePut(M_EVT_USB_DETECT, AK_NULL);
        stdb_ChangeUSBMode();
    }
    else*/
    usb_in = AK_TRUE;
    
    if (gb.init == SYSTEM_STATE_INIT)
    {
        AK_DEBUG_OUTPUT("PRE USB IN!!\n ");
        gb.usbstate = AK_TRUE;
    }
    else
	{
		enter_udisk_mode = AK_TRUE;
        (*pEventParm)->w.Param1 = USB_DETECT_LCD_LOCK;
        m_triggerEvent(M_EVT_RETURN_ROOT, *pEventParm);
        m_triggerEvent(M_EVT_USB_DETECT, AK_NULL);
        stdb_ChangeUSBMode();
		akerror("enter usbslave detect", 0, 1);
	}
    return 0;
}

T_BOOL ddlineinhandle(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
#if SUPPORT_LINEIN_SPK
    if (gb.init != SYSTEM_STATE_INIT)
    {
        if (!Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            if (stdb_IsLineInMode())
            {
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
            }
        }
        else
        {
            if(!isinBtDev()|| BtCtrl_IsStandby())
            {
                m_triggerEvent(M_EVT_RETURN_ROOT, *pEventParm);
                stdb_ChangeLineInMode();
            }
        }
    }  
#endif
	return 1;
}

#pragma arm section code = "_audioplayer_"
unsigned char ddusbouthandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    usb_in = AK_FALSE;
    AK_PRINTK("*****usb out", 0, 1);
    return 1;
}
unsigned char ddaudctrlhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    if(!IsAudplayer())
        return 1;
    if(AUD_CTRL_EVENT_AUDSTOP == (*pEventParm)->w.Param1 || 
        AUD_CTRL_EVENT_AUDPAUSE== (*pEventParm)->w.Param1)
    {
        Aud_AudCtrlEvtHandle(*pEventParm);
    }
    return 1;
}

unsigned char ddaudctrltimerhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    if(!IsAudplayer())
        return 1;
    Aud_AudCtrlPlyCycHandle();

    return 1;
}

unsigned char ddalarmclockhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
#if(USE_ALARM_CLOCK)
   #ifdef OS_ANYKA
   Fwl_SetRTC_AlarmTime(AK_FALSE, 0, AK_NULL);
   #endif
    
   AlmClk_ProcessComingMsg((T_BOOL)IS_BAT_LOW());
#endif
    return 0;
}

#pragma arm section code



