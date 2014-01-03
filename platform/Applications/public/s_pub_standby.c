#include "Apl_Public.h"
#include "Eng_Profile.h"
#include "Fwl_System.h"
#include "Eng_Font.h"
#include "Fwl_Timer.h"
#include "Fwl_Keypad.h"
#include "Eng_AutoOff.h"
#include "Eng_Standby.h"
#include "Eng_Usb.h"
#include "log_radio_core.h"
#include "M_event_api.h"
#include "Log_aud_control.h"
#include "Alarm_Common.h"
#include "Fwl_usb_s_state.h"


#define SUSPEND_DELAY   2   
// wake up type define
typedef enum {
    WAKE_NULL   = 0,
    WAKE_GPIO   = (1<<0),
    WAKE_ALARM  = (1<<1),
    WAKE_USB    = (1<<2),
    WAKE_VOICE  = (1<<3),
    WAKE_ANALOG = (1<<4),
    WAKE_POWER  = (1<<5)
} T_WU_TYPE;
        
#define WAKE_MASK               0xff
#define STANDBY_DELAY           2 

typedef struct _STANDBY
{
    T_BOOL  power_on;
    T_U8    delay;
    T_U8    key_cnt;
    T_U16   WakeupType; 
}T_STANDBY;

static T_STANDBY *pStandBy = AK_NULL;
extern T_VOID Preproc_TriggerPoweroff(T_VOID);
extern T_U8 USB_Init(T_VOID);
extern T_BOOL  IsLineInInit(T_VOID);
extern T_BOOL  IsInRadio(T_VOID);
extern T_VOID linein_start(T_VOID);

T_VOID Pub_Standby()
{
    T_EVT_PARAM param;
    param.w.Param1 = M_EVT_RESTART;
    Fwl_DetectorEnable(DEVICE_USB, AK_FALSE);
    pStandBy->WakeupType = Fwl_SysSleep(AK_TRUE);  
    Fwl_DetectorEnable(DEVICE_USB, AK_TRUE);
    
    if(WAKE_USB == pStandBy->WakeupType)
    {
        AK_DEBUG_OUTPUT("usb wakeup! s:%d\n", Fwl_DetectorGetStatus(DEVICE_CHG));    
    
        if (Fwl_DetectorGetStatus(DEVICE_CHG))
        {
            if(Fwl_UsbSlaveDetect())
            {
                VME_EvtQueuePut(M_EVT_EXIT, &param);
                USB_Init();
            }
            else
            {
                VME_EvtQueuePut(M_EVT_EXIT, &param);
            }
        }
        else
        {
            VME_EvtQueuePut(M_EVT_EXIT, &param);
        }
    }
    else if(pStandBy->WakeupType != WAKE_NULL)
    {
        VME_EvtQueuePut(M_EVT_EXIT, &param);
    }
	else
	{		
	}
    pStandBy->key_cnt = 0;
    pStandBy->delay   = 0;
}

void initpub_standby(void)
{
    AK_DEBUG_OUTPUT("initpub_standby\n");
    pStandBy = (T_STANDBY*)Fwl_Malloc(sizeof(T_STANDBY));
    pStandBy->WakeupType = WAKE_NULL;
    pStandBy->delay      = STANDBY_DELAY;

    StandbyDisable();
    AutoPowerOffDisable();

    if (gb.power_on)
    {   
        //if (eSTAT_REC_STOP != AudioRecord_GetRecState()
        //    && eSTAT_VORREC_PAUSE == AudioRecord_GetVorCtrlState())
        if(eSTAT_REC_PAUSE == AudioRecord_GetRecState())
        {               
            AudioRecord_CloseWavIn();
        }                       
    }
    // Aud_PlayerCloseHP_DA();
    //Aud_AudCtrlSetHpFlag(AK_FALSE);
}

void exitpub_standby(void)
{
    AK_DEBUG_OUTPUT("exitpub_standby\n");
    if (gb.power_on)
    {
        UserCountDownReset();
        //AutoPowerOffEnable();
        //if (eSTAT_REC_STOP != AudioRecord_GetRecState()            
        //    && eSTAT_VORREC_PAUSE == AudioRecord_GetVorCtrlState())
        
        if(eSTAT_REC_PAUSE == AudioRecord_GetRecState())
        {
            AudioRecord_OpenWavIn();
        }
		/*else if(IsLineInInit())
		{
			linein_start();
		}
        else if(IsInRadio())
        {
            Fwl_RadioInit();
        }*/
    }
    
    pStandBy = Fwl_Free(pStandBy);
    pStandBy = AK_NULL;    
    StandbyEnable();
    AutoPowerOffEnable();
}

void paintpub_standby(void)
{
}

unsigned char handlepub_standby(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("handlepub_standby event:%x\n", event);

    switch (event)
    {
    case M_EVT_Z02_STANDBY:
        AK_DEBUG_OUTPUT("stdby p_on:%x,Param:%x\n", gb.power_on
                            ,pEventParm->w.Param1);
        Pub_Standby();
        break;  
        
    case M_EVT_PUB_TIMER:
        if (STANDBY_DELAY < pStandBy->delay++)
        {
            Pub_Standby();
        }
        break;

    case M_EVT_Z00_POWEROFF:
        {
            m_triggerEvent(M_EVT_EXIT, pEventParm);                   
            VME_EvtQueuePut(M_EVT_Z00_POWEROFF, AK_NULL);            
            return 0;
        }       
        break;

    case M_EVT_USER_KEY:
    {
        T_PRESS_KEY phyKey;
  
        phyKey.id = pEventParm->c.Param1;
        phyKey.pressType = pEventParm->c.Param2;        
        pStandBy->delay = 0;

        if (WAKE_ANALOG == pStandBy->WakeupType
            && gb.power_on
            && 0 < pStandBy->key_cnt++)
        {
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        }
    }
        break;
        
    default:
        break;
    }

    
//   Fwl_Print(C3, M_PUBLIC, "Leave Handle Standby event---------\n");

    return 0;
}
/* end of files */
