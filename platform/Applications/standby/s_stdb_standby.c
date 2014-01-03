/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd.
* All rights reserved.
*
* @FILENAME s_stdb_standby.c
* @BRIEF the standby UI
* @Author：Huang_ChuSheng
* @Date：2008-04-30
* @Version：
**************************************************************************/

#include "Apl_Public.h"
//#include "m_state.h"
#include "Eng_ImageResDisp.h"
#include "eng_font.h"
#include "Fwl_Keypad.h"
#include "Eng_USB.h"
#include "Eng_AutoOff.h"
#include "Gbl_Global.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Timer.h"
#include "Eng_USB.h"
#include "Eng_Profile.h"
#include "log_aud_control.h"
#include "Gui_common.h"
#include "Fwl_System.h"
#include "svc_medialist.h"
#include "ctrl_IconMenu.h"
#include "Fwl_RTC.h"
#include "eng_debug.h"


#if(USE_ALARM_CLOCK)
#include "AlarmClock.h"
#endif

typedef enum
{
#ifdef CAMERA_SUPPORT
    MODE_CAMERA = 0,
#endif


    MODE_BLUE,


#ifdef SUPPORT_USBHOST
    MODE_USBAUDIO,
#endif

#ifdef SUPPORT_SDCARD
    MODE_SDAUDIO,
#endif

#if SUPPORT_LINEIN_SPK
    MODE_LINEIN,
#endif

#if(SUPPORT_RADIO)
    MODE_FM,
#endif

#ifdef SUPPORT_AUDIO_RECORD
    MODE_RECORD,
#endif

#ifdef SUPPORT_VOICE_PLAY
    MODE_RECORD_PLAY,
#endif

#ifdef SUPPORT_AUDIO_RECORD
#ifdef SUPPORT_MUSIC_PLAY
    MODE_TOMPLAY,
#endif
#endif

    MODE_SELF_UPDATE,
    MODE_USBDISK,
    MODE_MAX
}T_STD_CHANGE_MODE;
//CAMERA未打开

extern const T_CONFIG_INFO gb_sysconfig;


static T_STD_CHANGE_MODE    gMode = MODE_BLUE;//MODE_BLUE=0
static T_STD_CHANGE_MODE    gModeReturn[MODE_MAX] = {0};
static T_BOOL               bSpecSwitch = AK_FALSE;

static T_BOOL first_to_bt = AK_TRUE;

T_BOOL stdb_first_to_bt(T_VOID)
{
	return first_to_bt;
}

T_VOID stdb_clear_bt_flag(T_VOID)
{
	first_to_bt = 0;
}

T_VOID stdb_set_bt_flag(T_VOID)
{
	first_to_bt = 1;
}

//重新设置栈
T_VOID stdb_reset_return_stack(T_VOID)
{
    T_U8 i = 0;

    for (i=0; i<MODE_MAX; i++)
    {
        gModeReturn[i] = MODE_MAX;
    }
}

//一般外设插入及录音、录音播放、自升级时，需要将状态压栈
static T_VOID stdb_push_return_mode(T_VOID)
{
    T_U8 i = 0;
    T_U8 j = MODE_MAX;
    T_U8 k = MODE_MAX;

    if (gMode >= MODE_SELF_UPDATE)	//don't push some special mode
    {
        return;
    }

    for (i=0; i<MODE_MAX; i++)
    {
        if (MODE_MAX == gModeReturn[i])
        {
            j = i;         //gModeReturn[0]=8,i=j=0;
            break;         //gModeReturn[0]=0,i++,gModeReturn[1]=8,i=j=1;
        }
    }

    if (0 == j) //stack is empty:此时的栈为空
    {
        gModeReturn[0] = gMode;//0状态压栈
        return;
    }

    for (i=0; i<j; i++)
    {
        if (gMode == gModeReturn[i])
        {
            k = i;  //find same one
            break;
        }
    }

    if (MODE_MAX == k && j < MODE_MAX)  //no same one:没有相同的mode,栈顶压入一个新的mode:gMode
    {
        for (i=j; i>0; i--)
        {
            gModeReturn[i] = gModeReturn[i-1];
        }

        gModeReturn[0] = gMode;
    }
    else
    {
        for (i=k; i>0; i--)
        {
            gModeReturn[i] = gModeReturn[i-1];
        }

        gModeReturn[0] = gMode;
    }

}

//退出时，在栈中查找，如果有直接弹栈
static T_STD_CHANGE_MODE stdb_pop_return_mode(T_VOID)
{
    T_U8 i = 0;
    T_STD_CHANGE_MODE returnmode = MODE_MAX;

    if (MODE_MAX != gModeReturn[0])
    {
        returnmode = gModeReturn[0];
            
        for (i=0; i<MODE_MAX-1; i++)
        {
            gModeReturn[i] = gModeReturn[i+1];
        }

        gModeReturn[MODE_MAX-1] = MODE_MAX;
    }

    return returnmode;
}

T_VOID stdb_set_spec_flag(T_BOOL flag)
{
    bSpecSwitch = flag;
}


T_STD_CHANGE_MODE stdb_GetMode(T_VOID)
{
    return gMode;
}

void stdb_ChangeBTMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_BLUE;
    if (SYSTEM_STATE_NORMAL == gb.init)
    {
        bSpecSwitch = AK_TRUE;
    }
    
    AK_DEBUG_OUTPUT("stdb_ChangeBTMode gMode = %d\n", gMode);
}


#ifdef SUPPORT_USBHOST
void stdb_ChangeUHOSTMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_USBAUDIO;
    if (SYSTEM_STATE_NORMAL == gb.init)
    {
        bSpecSwitch = AK_TRUE;
    }

    AK_DEBUG_OUTPUT("stdb_ChangeUHOSTMode gMode = %d\n", gMode);
}
#endif

void stdb_ChangeUSBMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_USBDISK;
    if (SYSTEM_STATE_NORMAL == gb.init)
    {
        bSpecSwitch = AK_TRUE;
    }

    AK_DEBUG_OUTPUT("stdb_ChangeUSBMode gMode = %d\n", gMode);
}


#ifdef SUPPORT_SDCARD
void stdb_ChangeSDMode(void)
{
    if (SYSTEM_STATE_NORMAL == gb.init)
    {
    	stdb_push_return_mode();
    	gMode = MODE_SDAUDIO;
        bSpecSwitch = AK_TRUE;
    }
    
    AK_DEBUG_OUTPUT("stdb_ChangeSDMode gMode = %d\n", gMode);
}
#endif

#if SUPPORT_LINEIN_SPK
void stdb_ChangeLineInMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_LINEIN;
    bSpecSwitch = AK_TRUE;
    
    AK_DEBUG_OUTPUT("stdb_ChangeLineInMode gMode = %d\n", gMode);
}
#endif

void stdb_ChangeRecordMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_RECORD;
    bSpecSwitch = AK_TRUE;

    AK_DEBUG_OUTPUT("stdb_ChangeRecordMode gMode = %d\n", gMode);
}

void stdb_ChangeRecordPlayMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_RECORD_PLAY;
    bSpecSwitch = AK_TRUE;
    
    AK_DEBUG_OUTPUT("stdb_ChangeRecordPlayMode gMode = %d\n", gMode);
}

void stdb_ChangeSelfUpdateMode(void)
{
    stdb_push_return_mode();
    gMode = MODE_SELF_UPDATE;
    bSpecSwitch = AK_TRUE;
    
    AK_DEBUG_OUTPUT("stdb_ChangeSelfUpdateMode gMode = %d\n", gMode);
}

#ifdef SUPPORT_SDCARD
T_BOOL stdb_IsSDMode(void)
{
    if ((MODE_SDAUDIO == gMode) || (MODE_RECORD_PLAY == gMode) || (MODE_RECORD == gMode))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}
#endif

#ifdef SUPPORT_USBHOST
T_BOOL stdb_IsUSBAudMode(void)
{
    if (MODE_USBAUDIO == gMode)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}
#endif

#if SUPPORT_LINEIN_SPK
T_BOOL stdb_IsLineInMode(void)
{
    if (MODE_LINEIN == gMode)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}
#endif


//通过加1来切换到下一状态
void stdb_ChangeMode(void)
{
    stdb_reset_return_stack();//复位一下状态栈
    gMode = (gMode + 1)% MODE_RECORD;
    AK_DEBUG_OUTPUT("stdb_ChangeMode gMode = %d\n", gMode);
}

#if 0
T_BOOL stdb_IsLineInMode(void)
{
    if (MODE_LINEIN == gMode)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}
#endif
//进入相应的状态
static T_BOOL stdb_enterCorrespondState(T_VOID)
{
    T_EVT_PARAM evtParam = {0};
    T_BOOL  ret = AK_FALSE;
    
    switch (gMode)
    {
		case MODE_BLUE:
            AK_DEBUG_OUTPUT("Enter : MODE_BLUE!\n");
            stdb_clear_bt_flag();
            VME_EvtQueuePut(M_EVT_BTPLAYER, &evtParam);
            ret = AK_TRUE;
            break;
#ifdef CAMERA_SUPPORT
        case MODE_CAMERA:
            AK_DEBUG_OUTPUT("Enter : MODE_CAMERA!\n");
            VME_EvtQueuePut(M_EVT_CAM_CAP, &evtParam);
            ret = AK_TRUE;
            break;
#endif

#ifdef SUPPORT_AUDIO_RECORD
        case MODE_RECORD:
            AK_DEBUG_OUTPUT("Enter : MODE_RECORD!\n");
            VME_EvtQueuePut(M_EVT_RECORD, &evtParam);
            ret = AK_TRUE;
            break;
#endif

#ifdef SUPPORT_VOICE_PLAY
        case MODE_RECORD_PLAY:
            AK_DEBUG_OUTPUT("Enter : MODE_RECORD_PLAY!\n");
            VME_EvtQueuePut(M_EVT_AUDIO_VOICE, &evtParam);
            ret = AK_TRUE;
            break;
#endif
#if 0
#ifdef SUPPORT_MUSIC_PLAY
        case MODE_AUDIO:
            AK_DEBUG_OUTPUT("Enter : MODE_AUDIO!\n");
            VME_EvtQueuePut(M_EVT_AUDIO_MAIN, &evtParam);
            ret = AK_TRUE;
            break;
#endif
#endif
#ifdef SUPPORT_SDCARD
        case MODE_SDAUDIO:
            AK_DEBUG_OUTPUT("Enter : MODE_SDAUDIO!\n");
            if (Fwl_DetectorGetStatus(DEVICE_SD) && Fwl_MemDevIsMount(MMC_SD_CARD))
            {
                VME_EvtQueuePut(M_EVT_SD_AUDIO, &evtParam);
                ret = AK_TRUE;
                return ret;
            } 

            ret = AK_FALSE;
            break;
#endif

#ifdef SUPPORT_USBHOST

        case MODE_USBAUDIO:
            AK_DEBUG_OUTPUT("Enter : MODE_USBAUDIO!\n");
            if(!Fwl_MemDevIsMount(USB_HOST_DISK))
            {
                Fwl_MemDevMount(USB_HOST_DISK);
            }
            if (Fwl_DetectorGetStatus(DEVICE_UHOST) && Fwl_MemDevIsMount(USB_HOST_DISK))
            {       
                VME_EvtQueuePut(M_EVT_USB_AUDIO, &evtParam);
                ret = AK_TRUE;
            }
            else
            {
                ret = AK_FALSE;
            }
            break;
#endif

#if SUPPORT_LINEIN_SPK
        case MODE_LINEIN:
            if(Fwl_DetectorGetStatus(DEVICE_LINEIN))
            {
                AK_DEBUG_OUTPUT("Enter : MODE_LINEIN!\n");
                VME_EvtQueuePut(M_EVT_LINEIN_PLAY, &evtParam);            
                ret = AK_TRUE;
            }
            else
            {
                ret = AK_FALSE;
            }
            break;
#endif      

#ifdef SUPPORT_AUDIO_RECORD
#ifdef SUPPORT_MUSIC_PLAY
        case MODE_TOMPLAY:
            AK_DEBUG_OUTPUT("Enter : MODE_TOMPLAY!\n");
            VME_EvtQueuePut(M_EVT_TOMPLAY, &evtParam);
            ret = AK_TRUE;
            break;
#endif
#endif

#if(SUPPORT_RADIO)
        case MODE_FM:
            AK_DEBUG_OUTPUT("Enter : MODE_FM!\n");
            VME_EvtQueuePut(M_EVT_RADIO, &evtParam);
            ret = AK_TRUE;
            break;
#endif
        
        case MODE_USBDISK:
            AK_DEBUG_OUTPUT("Enter : MODE_USBDISK!\n");
            /*if(Fwl_DetectorGetStatus(DEVICE_CHG))
            {
                VME_EvtQueuePut(M_EVT_USBDISK, &evtParam);
                ret = AK_TRUE;
            }
            else*/
            {
                ret = AK_FALSE;
            }
            break;

        case MODE_SELF_UPDATE:
            AK_DEBUG_OUTPUT("Enter : MODE_SELF_UPDATE!\n");
            VME_EvtQueuePut(M_EVT_UPDATA, &evtParam);
            ret = AK_TRUE;
            break;
        default:
            break;
    }

    return ret;
}

//处理退出
static T_VOID stdb_deal_exit(T_VOID)
{
    T_STD_CHANGE_MODE returnmode = MODE_MAX;
    
    do
    {
        returnmode = stdb_pop_return_mode();//在栈中查找，如果有直接弹栈
        
        AK_DEBUG_OUTPUT("returnmode = %d\n", returnmode);

        if (returnmode < MODE_MAX)
        {
            gMode = returnmode;
        }
        else
        {
            stdb_ChangeMode();//栈中没有，自加一
        }
    }while(!stdb_enterCorrespondState());
}



T_BOOL USB_Init(T_VOID)
{
    T_BOOL stat;

    stat = Fwl_DetectorGetStatus(DEVICE_USB);
    AK_DEBUG_OUTPUT("usb cable state:%d\n", stat);

    if(stat)
    {
        VME_EvtQueuePut(M_EVT_USB_IN, AK_NULL);
    }

    return stat;
}

void initstdb_standby(void)
{
    Fwl_FreqPush(FREQ_APP_STANDBY); //change freq

    gb.init = SYSTEM_STATE_NORMAL;
    stdb_set_bt_flag();
    
    AutoPowerOffEnable();
    AutoBgLightOffEnable();
}

void exitstdb_standby(void)
{
    Fwl_FreqPop();
}

void paintstdb_standby(void)
{
}

extern T_BOOL enter_udisk_mode;

unsigned char handlestdb_standby(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("stdb event:%d\n", event-VME_EVT_USER);

    if (M_EVT_NEXT == event)
    {
        /*if(Fwl_DetectorGetStatus(DEVICE_CHG))
        {
            Fwl_DelayMs(1000);
        }
        else*/
        {
            stdb_enterCorrespondState();
        }
    }
    
    if (M_EVT_EXIT == event || M_EVT_CHARGER_OUT == event)
    {
        if(M_EVT_EXIT == event)
        {               
            if((pEventParm != AK_NULL) && (EVENT_TYPE_SWITCHOFF_MANUAL == pEventParm->w.Param1))
            {
                VME_EvtQueuePut(M_EVT_TIMEOUT, AK_NULL);
                VME_EvtQueuePut(M_EVT_Z00_POWEROFF, AK_NULL);
            }           

            if (bSpecSwitch)     //按键切换到下一个模块
            {
                bSpecSwitch = AK_FALSE;
                
                if (!stdb_enterCorrespondState())
                {
                    stdb_deal_exit();
                }
            }
            else if(AK_TRUE == enter_udisk_mode)    //从U盘模式退出，返回上一模式
            {
                enter_udisk_mode = AK_FALSE;
				#if 0
                if(0 == gpio_get_pin_level(20)) //如果power按键拨到off，直接关机
                {
                    AK_DEBUG_OUTPUT("Quit Udisk Mode Then Power off!\n");
                }
                else
				#endif
				if (!stdb_enterCorrespondState())
                {
                    stdb_deal_exit();
                }
            }
            else                       //自动切换到下一个状态
            {
                stdb_deal_exit();
            }
        }
                    
        VME_EvtQueueClearTimerEvent();
        GblSaveSystime();
    }  

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}


/* end of file */
