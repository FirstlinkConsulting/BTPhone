/******************************************************************************************
**FileName  	:      s_RADIO_PLAYER.C
**brief        	:      radioplayer machine status control
**Copyright 	:      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author 		:	Hongshi Yao
**date		: 	2008-04-09
**version 	:	1.0
*******************************************************************************************/
//include file
#include "Gbl_Global.h"
#include "m_event.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "Fwl_System.h"
#include "Eng_Debug.h"
#include "Fwl_Radio.h"
//#include "Arch_gpio.h"
#include "Ctrl_Progress.h"
#include "Ctrl_Dialog.h"
#include "log_radio_core.h"
#include "Eng_AutoOff.h"
#include "Fwl_FreqMgr.h"
#include "M_Event_Api.h"
#include "vme.h"
#include "log_aud_play.h"
#include "Gui_Common.h"
#include "log_ram_res.h"
#include "Fwl_WaveOut.h"
#include "Ctrl_MenuConfig.h"
#include <stdlib.h>
#include "Apl_Public.h"
#include "Eng_LedHint.h"
#include "Fwl_detect.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"

#define SEARCH_TIMES				(2)

#define  WATCHDOG_SET_LONG_TIME  115 // 115+5S = 120
#define  WATCHDOG_DISABLE_LONG_TIME  0 // 1+5S = 6S
#define  SEARCH_VALID_STATION_PLAY_VOLUME 10     //搜到一个有效台播放一秒钟默认的音量
#define  SEARCH_VALID_STATION_PLAY_TIME   1000   //搜到一个有效台播放时长

const T_RADIO_AREA_PARM	 RadioAreaParam[MAX_RADIO_AREA] = {
    // min                  max                     if                         ref                step
    {87500000,      108000000,      10700000,       50000,      100000},         //EUROPE
    {76000000,       90000000,      10700000,       50000,       50000},          //JAPAN
    {87500000,      108000000,      10700000,       50000,      100000}          //AMERICA
};

const T_U8 VoiceFMTable[]=
{
    eBTPLY_SOUND_TYPE_ZERO,
    eBTPLY_SOUND_TYPE_ONE,
    eBTPLY_SOUND_TYPE_TWO,
    eBTPLY_SOUND_TYPE_THREE,
    eBTPLY_SOUND_TYPE_FOUR,
    eBTPLY_SOUND_TYPE_FIVE,
    eBTPLY_SOUND_TYPE_SIX,
    eBTPLY_SOUND_TYPE_SEVEN,
    eBTPLY_SOUND_TYPE_EIGHT,
    eBTPLY_SOUND_TYPE_NINE,
    eBTPLY_SOUND_TYPE_POINT,
};

//this flag is according to CheckCode
T_BOOL auto_search_flag = AK_FALSE;

//radio palyer data struct
RADIO_PLAYER_PARM	*pRadioPlayerParam = AK_NULL;
extern T_RADIO_PLAYER_PARM * pRadioParam;

T_VOID Play_FM_Fre(T_U32 fre)
{
	T_U8 i = 0;
	T_U8 Len = 0;
	T_U8 testshu[8] = {0};
	T_U32 temp_fre = 0; 
	if(fre/100000000)
    {
        Len = 5;
    }
	else
    {
        Len = 4;
    }
	temp_fre = fre/100000;
	//testshu[0] = 11;  //FM
    for ( i=0; i<Len; i++)
    {
        if(i == 1)
        {
            testshu[Len-2]= 10;	//POINT
            continue;
        }
        testshu[Len-1-i] = temp_fre%10;
        temp_fre = temp_fre/10; 
    }
#ifdef SUPPORT_VOICE_TIP
   //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
   Fwl_RadioSetVolume(0); 
   
   for(i=0;i<Len;i++)
   {
        Voice_PlayTip(VoiceFMTable[testshu[i]], AK_NULL);
   }
   Voice_WaitTip();
   
   //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
   Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
#endif
  //RadioSetVolume();
  
}

typedef T_VOID (*T_pRADIO_STOP_AUTOSEARCH_CALLBACK)(T_VOID);
typedef T_VOID (*T_pRADIO_PLAY_ONESECOND_CALLBACK)(T_U32 frq);

extern volatile T_U8 Fm_is_ats;
extern T_VOID radio_auto_search_set_callback(T_pRADIO_STOP_AUTOSEARCH_CALLBACK radio_stop_autosearch_cb,
    T_pRADIO_PLAY_ONESECOND_CALLBACK radio_play_onesecond_cb);

T_VOID radio_stop_autosearch(T_VOID)
{
	T_U8 key_index;
	key_index = Fwl_KeypadScan();

	if((kbLEFT == key_index)||(kbRIGHT == key_index)) //kbLEFT kbRIGHT
	{
	    //keypad_disable_scan();
		Fm_is_ats = 0;
	}
	else
	{
		Fm_is_ats = 1;
	}
}

T_VOID radio_play_onesecond(T_U32 freq)
{
    Fwl_RadioSetVolume(SEARCH_VALID_STATION_PLAY_VOLUME);
    //RadioSwitch(SWITCH_ON);
    Fwl_DelayMs(SEARCH_VALID_STATION_PLAY_TIME);
    Fwl_RadioSetVolume(0);
}


/*---------------------- BEGIN OF STATE s_radio_player ------------------------*/
void initradio_player(void)
{
    Fwl_FreqPush(FREQ_APP_MAX);
#ifdef SUPPORT_LEDHINT
        if(Fwl_DetectorGetStatus(DEVICE_CHG))
        {
            LedHint_Exec(LED_NORMAL_CHARGE);
        }
        else
        {
            LedHint_Exec(LED_NORMAL);
        }
#endif

#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_FM, AK_NULL);
        Voice_WaitTip();
#endif
	//alloc memory for radioplayer
	pRadioPlayerParam = (RADIO_PLAYER_PARM	*)Fwl_Malloc(sizeof(RADIO_PLAYER_PARM));
	AK_ASSERT_PTR_VOID(pRadioPlayerParam, "malloc memory error in initradio_player\n")
    memset(pRadioPlayerParam, 0, sizeof(RADIO_PLAYER_PARM));
	
	//initialize critical data struct
	pRadioPlayerParam->bInitFail = !Radio_PlayerInit(&pRadioPlayerParam->radioparam);

  //first enter current state, check wether hardware is ok
    if(AK_TRUE == pRadioPlayerParam->bInitFail)
    {  
    	AK_DEBUG_OUTPUT("#FM->:there is no 5876 fm!!!");
        Fwl_FreqPop();
        return;
    }
    
    radio_auto_search_set_callback(radio_stop_autosearch,radio_play_onesecond);
    
    if (AK_TRUE == auto_search_flag)
    {
        auto_search_flag = AK_FALSE;
        #ifdef SUPPORT_VOICE_TIP
            //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
            Fwl_RadioSetVolume(0);
            
            Voice_PlayTip(eBTPLY_SOUND_TYPE_SEARCHING, AK_NULL);
            //Voice_WaitTip();
        #endif
        //set watch dog time to 2 minute
        Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
		if (!Fwl_DetectorGetStatus(DEVICE_HP))
		{
	    	Fwl_SpkConnectSet(AK_TRUE);
		}
        Radio_AutoSearch();
        //clean watch dog long time
        Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    }

	Play_FM_Fre(pRadioParam->CurFreq);
    //Fwl_RadioSetVolume(pRadioParam->Volume); 
    Radio_Switch(SWITCH_ON);

    /*if (!RadioSwitch(SWITCH_ON))
    { //initialize radio hardware, if failed do following
        pRadioParam->RadioState   = RADIO_STOP;
        pRadioParam->bHWER        = AK_TRUE;
        AK_DEBUG_OUTPUT("#FM->:init failed!!!\n");
    }
	else
	{
    	pRadioParam->RadioState   = RADIO_PLAY;
	}

    if(pRadioParam->bHWER)
        return AK_FALSE;
    else
        return AK_TRUE;
    */
    
	//if(i != pRadioParam->Volume)
	//	Fwl_fm_SetVolume(pRadioParam->Volume);
	//RadioSetVolume();
     
	//auto power off
	AutoPowerOffDisable();
    
	AK_DEBUG_OUTPUT("#FM->:init radio player!");
    Fwl_FreqPop();
#ifdef SUPPORT_USBHOST
    Fwl_FreqPush(FREQ_APP_AUDIO_L1); //由于usbhost 正常检测需要14.5MHz
#else
    Fwl_FreqPush(FREQ_APP_RADIO);
#endif
}

void exitradio_player(void)
{
    Fwl_FreqPush(FREQ_APP_MAX);

    //save current channel information
	//close hardware
	Radio_SetMute();
    Radio_PlayerFree();

	//release memory
	pRadioPlayerParam = (RADIO_PLAYER_PARM*)Fwl_Free(pRadioPlayerParam);

	//enable power off
	AutoPowerOffEnable();
    
#ifdef SUPPORT_LEDHINT
        if(Fwl_DetectorGetStatus(DEVICE_CHG))
        {
            LedHint_Stop(LED_NORMAL_CHARGE);
        }
        else
        {
            LedHint_Stop(LED_NORMAL);
        }
#endif

	AK_DEBUG_OUTPUT("#FM->:exit radio player!");

    Fwl_FreqPop();
    Fwl_FreqPop();

}

void paintradio_player(void)
{
}

T_VOID Radio_DealRadioRecord(T_VOID)
{
	m_triggerEvent(M_EVT_RADIO_REC, AK_NULL);
}

T_VOID Radio_userKeyAction(T_EVT_PARAM *pEventParm)
{   
    T_eKEY_ID keyID = pEventParm->c.Param1;
    T_ePRESS_TYPE type = pEventParm->c.Param2;

	switch(keyID)
	{
        case kbVOLSUB:
        if((AK_FALSE == pRadioPlayerParam->bMute)&&(PRESS_SHORT== type || PRESSING == type))
        {
            if(pRadioPlayerParam->radioparam.Volume > 0)
            {
                pRadioPlayerParam->radioparam.Volume --;
				Radio_SetVolume();
            }
        }
        break;        
    case kbVOLADD:
        if((AK_FALSE == pRadioPlayerParam->bMute)&&(PRESS_SHORT == type || PRESSING == type))
        {
            if (pRadioPlayerParam->radioparam.Volume < L2HP_MAX_VOLUME)
            {          
                pRadioPlayerParam->radioparam.Volume ++;
                Radio_SetVolume();
                if(L2HP_MAX_VOLUME == pRadioPlayerParam->radioparam.Volume)
                {
#ifdef SUPPORT_VOICE_TIP
                    //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
                    Fwl_RadioSetVolume(0); 

                    Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                    Voice_WaitTip();

                    //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
                    Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
#endif
                }
            }
            else if((L2HP_MAX_VOLUME == pRadioPlayerParam->radioparam.Volume)&&(PRESS_SHORT == type || PRESS_LONG == type))
            {
#ifdef SUPPORT_VOICE_TIP
                //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
                Fwl_RadioSetVolume(0); 

                Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                Voice_WaitTip();

                //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
                Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
#endif
            }
        }
        break;
    case kbPRE:
	case kbLEFT:	//handle left key
		if (PRESS_SHORT == type)
		{
            pRadioPlayerParam->bMute = AK_FALSE;
			AK_DEBUG_OUTPUT("#FM->:left pre chan");
			Radio_JumpChannel(0);
            Play_FM_Fre(pRadioParam->CurFreq);
            Radio_Switch(SWITCH_ON);
		}
		else if ((kbPRE != keyID)&&(AK_FALSE == pRadioPlayerParam->bMute)&&(PRESS_LONG == type|| PRESSING == type))
		{
			if (pRadioPlayerParam->radioparam.Volume > 0)
			{              
				pRadioPlayerParam->radioparam.Volume --;
				Radio_SetVolume();
			}
		}
        break;
	
	//right key handle
	case kbNEXT:
	case kbRIGHT:
		if (PRESS_SHORT == type)
		{
            pRadioPlayerParam->bMute = AK_FALSE;
			AK_DEBUG_OUTPUT("#FM->:right next chan");
			Radio_JumpChannel(1);
	        Play_FM_Fre(pRadioParam->CurFreq);
            Radio_Switch(SWITCH_ON);
		}
		else if ((kbPRE != keyID)&&(AK_FALSE == pRadioPlayerParam->bMute)&&(PRESS_LONG == type || PRESSING == type))
		{
			if (pRadioPlayerParam->radioparam.Volume < L2HP_MAX_VOLUME)
			{          
				pRadioPlayerParam->radioparam.Volume ++;
				Radio_SetVolume();
                if(L2HP_MAX_VOLUME == pRadioPlayerParam->radioparam.Volume)
                {
#ifdef SUPPORT_VOICE_TIP
                    //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
                    Fwl_RadioSetVolume(0); 

                    Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                    Voice_WaitTip();

                    //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
                    Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
#endif
                }
			}
            else if((L2HP_MAX_VOLUME == pRadioPlayerParam->radioparam.Volume)&&(PRESS_LONG == type))
            {
#ifdef SUPPORT_VOICE_TIP
                //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
                Fwl_RadioSetVolume(0); 

                Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                Voice_WaitTip();

                //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
                Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
#endif
            }
		}
		break;
	//stop and play handle
	case kbOK:
    case kbMUTE:
		if (PRESS_SHORT == type)
		{
			if (pRadioPlayerParam->bMute)
			{
                //Fwl_WaveOutSetGain(wavId, pRadioPlayerParam->radioparam.Volume);
                Radio_Switch(SWITCH_ON);
                Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);             
			}
			else
			{

                //Fwl_WaveOutSetGain(wavId, 0);
				Fwl_RadioSetVolume(0);
			}
			pRadioPlayerParam->bMute = !pRadioPlayerParam->bMute;
		}
		else if ((PRESS_LONG == type)&&(kbOK == keyID))
		{
            #ifdef SUPPORT_VOICE_TIP
                //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
                Fwl_RadioSetVolume(0); 
                
                Voice_PlayTip(eBTPLY_SOUND_TYPE_SEARCHING, AK_NULL);
                //Voice_WaitTip();
            #endif
			if (!Fwl_DetectorGetStatus(DEVICE_HP))
			{
				Fwl_SpkConnectSet(AK_TRUE);
			}
			Radio_AutoSearch();
            Play_FM_Fre(pRadioParam->CurFreq);
            pRadioPlayerParam->bMute = AK_FALSE;
            Radio_Switch(SWITCH_ON);
		}
		break;

	default:
		break;

	} 
}

//extern T_VOID Play_FM_Fre(T_U32 fre);
unsigned char handleradio_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	if(pRadioPlayerParam->bInitFail)
	{
		T_EVT_PARAM evtParam;
		evtParam.c.Param1 = 0xff;
		
		if (M_EVT_EXIT != event)
		{
			AK_DEBUG_OUTPUT("#FM->: m_evt_exit");
			VME_EvtQueuePut(M_EVT_EXIT, &evtParam);
		}
		return 0;	
	}	

	//if exit from menu or adjust volume , refresh lcd
	if(M_EVT_EXIT == event)
	{
		if(pEventParm != AK_NULL)
		{
  			if(pEventParm->w.Param1 == M_EVT_RESTART)
			{
                pRadioPlayerParam->bMute = AK_TRUE;
                
                Fwl_RadioInit();
                
                Radio_Switch(SWITCH_ON);
                
                if(0 == pRadioPlayerParam->radioparam.Volume)
                {
                    pRadioPlayerParam->radioparam.Volume = DEF_VOLUME;
                }
                
                Fwl_RadioSetVolume(0);
                if(Fwl_DetectorGetStatus(DEVICE_CHG))//解决USB唤醒，可能无声输出。
                {
                    Fwl_DelayMs(2000);
                }
                
    			pRadioPlayerParam->radioparam.RadioState = RADIO_PAUSE;      
			}
        }

        //auto power off
        AutoPowerOffDisable();
    }
	
	//event handle
	switch (event)
	{
	case M_EVT_RADIO_RECORD:
	     Radio_DealRadioRecord();
		 return 0;
		break;
	case M_EVT_USER_KEY:
		Radio_userKeyAction(pEventParm);
		break;    //end of key handle of event
#ifdef SUPPORT_VOICE_TIP
    case VME_EVT_POWER_CHANGE:       
        //Fwl_WaveOutSetSignal(LINEIN_HP, 0);
        Fwl_RadioSetVolume(0);
        
        if(eBTPLY_SOUND_TYPE_LOWPOWER == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_LOWPOWER, AK_NULL);
        }
        else if(eBTPLY_SOUND_TYPE_CHARGING == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGING, AK_NULL);
        }
        else if(eBTPLY_SOUND_TYPE_CHARGEOK == pEventParm->c.Param1)
        {
            Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGEOK, AK_NULL);
        }
        Voice_WaitTip();
        
        //Fwl_WaveOutSetSignal(LINEIN_HP, 1);
        Fwl_RadioSetVolume(pRadioPlayerParam->radioparam.Volume);
        break;
#endif
    case M_EVT_Z02_STANDBY:
	case M_EVT_Z00_POWEROFF:
		Fwl_RadioFree();
        break;

	case VME_EVT_TIMER:
        VME_EvtQueueClearTimerEvent();
		break;
	
	default:
		break;
	}
		
    if (event >= M_EVT_Z00_POWEROFF)
    {       
        Radio_ConserveData();        
        return 1;
    }
    else
        return 0;
}
#pragma arm section code = "_frequentcode_"
T_BOOL  IsInRadio(T_VOID)
{
    if(pRadioPlayerParam == AK_NULL) 
        return AK_FALSE;
    else
        return AK_TRUE;
}

T_BOOL  IsRadioPlaying(T_VOID)
{
	return (AK_NULL != pRadioPlayerParam) && (AK_FALSE == pRadioPlayerParam->bMute);
}
#pragma arm section code

T_BOOL IsRadioPausing(T_VOID)
{
    return (T_BOOL)(AK_TRUE == pRadioPlayerParam->bMute);
}

/* end of the file */

