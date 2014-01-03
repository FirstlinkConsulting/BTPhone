#include "Apl_Public.h"
#include "Fwl_Keypad.h"
#include "Fwl_System.h"
#include "Fwl_FreqMgr.h"
#include "M_event.h"
#include "M_event_api.h"
#include "Fwl_waveout.h"
#include "Eng_LedHint.h"
#include "Fwl_detect.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"
//#include "Arch_Analog.h"
static T_U8 wavId = INVAL_WAVEOUT_ID;
static T_U8 Volume = 15;
static T_BOOL IsMute = AK_FALSE, LineINInitFlag = AK_FALSE;

T_VOID linein_stop(T_VOID)
{
    if (INVAL_WAVEOUT_ID != wavId)
    {
        Fwl_WaveOutClose(wavId);
        wavId = INVAL_WAVEOUT_ID;
    }
}

T_VOID linein_start(T_VOID)
{
    if (INVAL_WAVEOUT_ID != wavId)
    {
        Fwl_WaveOutClose(wavId);
    }    
    wavId = Fwl_WaveOutOpen(LINEIN_HP, AK_NULL);    
    if (INVAL_WAVEOUT_ID == wavId)
    {
        AK_DEBUG_OUTPUT("Fwl_WaveOutOpen failed!\n");
        Fwl_FreqPop();
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        return;
    }    
    Fwl_WaveOutStart(wavId,0,0,0);
    Fwl_WaveOutSetGain(wavId, Volume);
}

void initlinein_play(void)
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
    Voice_PlayTip(eBTPLY_SOUND_TYPE_LINEIN, AK_NULL);
    Voice_WaitTip();
#endif
	linein_start();
    Fwl_FreqPop();
#ifdef SUPPORT_USBHOST
    Fwl_FreqPush(FREQ_APP_AUDIO_L1); //由于usbhost 正常检测需要14.5MHz
#else
    Fwl_FreqPush(FREQ_APP_RADIO);
#endif
    LineINInitFlag = AK_TRUE;

}
#pragma arm section code = "_frequentcode_"

T_BOOL  IsLineInInit(T_VOID)
{
	return LineINInitFlag;
}
T_BOOL  IsLineInPlaying(T_VOID)
{
	return LineINInitFlag && (AK_FALSE == IsMute);
}
#pragma arm section code

void exitlinein_play(void)
{
    Fwl_FreqPop();
    Fwl_FreqPush(FREQ_APP_MAX);

    if (INVAL_WAVEOUT_ID != wavId)
    {
        Fwl_WaveOutStop(wavId);
        Fwl_WaveOutClose(wavId);
        wavId = INVAL_WAVEOUT_ID;
    }
    IsMute = AK_FALSE;
    
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
	LineINInitFlag = AK_FALSE;

    AK_DEBUG_OUTPUT("exitlinein_play\n");
    Fwl_FreqPop();
}

void paintlinein_play(void)
{
}

unsigned char handlelinein_play(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY phyKey;

    switch (event)
    {
    case M_EVT_USER_KEY:
        //get key value and key type
        phyKey.id = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;

        switch(phyKey.id)
        {
        case kbVOLSUB:
            if(AK_FALSE == IsMute)
            {
                if((0 < Volume)&&(PRESS_SHORT == phyKey.pressType || PRESSING == phyKey.pressType))
                {
                    Volume--;
                    Fwl_WaveOutSetGain(wavId, Volume);
                    AK_DEBUG_OUTPUT("linein vol:%d\n", Volume);
                }
            }
            break;        
        case kbVOLADD:
            if(AK_FALSE == IsMute)
            {
                if((L2HP_MAX_VOLUME > Volume)&&(PRESS_SHORT == phyKey.pressType || PRESSING == phyKey.pressType))
                {
                    /*if (!(Fwl_DetectorGetStatus(DEVICE_HP))&&(Fwl_DetectorGetStatus(DEVICE_CHG)))
        			{
        				if (Volume > 16)
        					break;
        			}*/
                    Volume++;
                    Fwl_WaveOutSetGain(wavId, Volume);
    
                    if(L2HP_MAX_VOLUME == Volume)
                    {
#ifdef SUPPORT_VOICE_TIP
                        Fwl_WaveOutStop(wavId);
                        //Fwl_WaveOutSetGain(wavId,0);
                        
                        Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                        Voice_WaitTip();
                        
                        Fwl_WaveOutStart(wavId,0,0,0);
                        //Fwl_WaveOutSetGain(wavId, Volume);
#endif
                    }
                    AK_DEBUG_OUTPUT("linein vol:%d\n", Volume);
                }
                else if((L2HP_MAX_VOLUME == Volume)&&(PRESS_SHORT == phyKey.pressType || PRESS_LONG == phyKey.pressType))
                {
#ifdef SUPPORT_VOICE_TIP
                    Fwl_WaveOutStop(wavId);
                    //Fwl_WaveOutSetGain(wavId,0);
                    
                    Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                    Voice_WaitTip();
                    
                    Fwl_WaveOutStart(wavId,0,0,0);
                    //Fwl_WaveOutSetGain(wavId, Volume);
#endif
                }
            }
            break;
        case kbOK:
        case kbMUTE:
            if(PRESS_SHORT == phyKey.pressType)
            {
                if(IsMute)
                {
                    Fwl_WaveOutSetGain(wavId, Volume);           
                }
                else
                {
                    Fwl_WaveOutSetGain(wavId,0);
                }
                IsMute = !(IsMute);
            }
            break;
        case kbRIGHT:
            if(AK_FALSE == IsMute)
            {
                if((L2HP_MAX_VOLUME > Volume) &&((PRESS_LONG == phyKey.pressType) || (PRESSING == phyKey.pressType)))
                {
                    /*if (!(Fwl_DetectorGetStatus(DEVICE_HP))&&(Fwl_DetectorGetStatus(DEVICE_CHG)))
        			{
        				if (Volume > 16)
        					break;
        			}*/
                    Volume++;
                    Fwl_WaveOutSetGain(wavId, Volume);
    
                    if(L2HP_MAX_VOLUME == Volume)
                    {
#ifdef SUPPORT_VOICE_TIP
                        Fwl_WaveOutStop(wavId);
                        //Fwl_WaveOutSetGain(wavId,0);
                        
                        Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                        Voice_WaitTip();
                        
                        Fwl_WaveOutStart(wavId,0,0,0);
                        //Fwl_WaveOutSetGain(wavId, Volume);
#endif
                    }
                    AK_DEBUG_OUTPUT("linein vol:%d\n", Volume);
                }
                else if((L2HP_MAX_VOLUME == Volume)&&(PRESS_LONG == phyKey.pressType))
                {
#ifdef SUPPORT_VOICE_TIP
                    Fwl_WaveOutStop(wavId);
                    //Fwl_WaveOutSetGain(wavId,0);
                    
                    Voice_PlayTip(eBTPLY_SOUND_TYPE_DI, AK_NULL);
                    Voice_WaitTip();
                    
                    Fwl_WaveOutStart(wavId,0,0,0);
                    //Fwl_WaveOutSetGain(wavId, Volume);
#endif
                }
            }
            break;
        case kbLEFT:
            if((AK_FALSE == IsMute) && (0 < Volume) && (PRESS_LONG==phyKey.pressType || PRESSING ==phyKey.pressType))
            {
                Volume--;
                Fwl_WaveOutSetGain(wavId, Volume);
                AK_DEBUG_OUTPUT("linein vol:%d\n", Volume);
            }
            break;
        default:
            break;
        }
        break;
#ifdef SUPPORT_VOICE_TIP
    case VME_EVT_POWER_CHANGE:
        //analog_setconnect(2,2,1);
        Fwl_WaveOutStop(wavId);
        //Fwl_WaveOutSetGain(wavId,0);

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
        
        Fwl_WaveOutStart(wavId,0,0,0);
        //Fwl_WaveOutSetGain(wavId, Volume);
        break;
#endif
	case M_EVT_Z02_STANDBY:
    case M_EVT_Z00_POWEROFF:
        //m_triggerEvent(M_EVT_EXIT,AK_NULL);
        //m_triggerEvent(M_EVT_Z00_POWEROFF, pEventParm);
        linein_stop();
        return 1;
        break;

     /*   
    case M_EVT_PUB_TIMER:
        if (!Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        }
        break;
    */
    case M_EVT_EXIT:
        IsMute = AK_TRUE;
        linein_start();
        Fwl_WaveOutSetGain(wavId,0);
        break;
    default:
        break;
    }
    return 0;
}


/* end of files */

