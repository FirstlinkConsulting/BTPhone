#include "Apl_Public.h"
#include "Gbl_resource.h"
#include "Eng_ImageResDisp.h"
#include "Eng_Font.h"
#include "Fwl_FreqMgr.h"
#include "eng_autooff.h"
#include "Eng_USB.h"
#include "Fwl_Timer.h"
#include "Fwl_Keypad.h"
#include "log_aud_control.h"
#include "Fwl_detect.h"
#include "Fwl_usb_s_state.h"
#include "Fwl_LCD.h"
#include "M_event_api.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"

#if (USE_COLOR_LCD)
#define USB_PIC_POS_X   0
#define USB_PIC_POS_Y   0
#else
#define USB_PIC_POS_X   0
#define USB_PIC_POS_Y   16
#endif


//static T_TIMER  usb_detect_timer_id;
static T_U8     usb_detect_counter = 0;
static T_BOOL   usb_update_flag = AK_FALSE;
static T_BOOL   bDispOnce = AK_FALSE;

extern T_VOID stdb_set_spec_flag(T_BOOL flag);

void initpub_usb_detect(void)
{   
    usb_detect_counter = 0;
    usb_update_flag = AK_FALSE;
    bDispOnce = AK_FALSE;
    
    //usb_detect_timer_id = Fwl_TimerStartMilliSecond(100, AK_TRUE);
    if (bglight_state_off())
    {
        bglight_on(AK_TRUE);
    }
    Fwl_LCD_lock(AK_FALSE);

	stdb_set_spec_flag(AK_FALSE);
}

void exitpub_usb_detect(void)
{
    //if(usb_detect_timer_id != ERROR_TIMER)
    //{
    //    usb_detect_timer_id= Fwl_TimerStop(usb_detect_timer_id);
    //}
}

void paintpub_usb_detect(void)
{
#if(NO_DISPLAY == 0)
    if(0 == usb_detect_counter)
    {
#if(!USE_COLOR_LCD)
        Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
#endif
        if (!bDispOnce) //for Spot300000553
        {
            bDispOnce = AK_TRUE;
            Eng_ImageResDisp(USB_PIC_POS_X, USB_PIC_POS_Y, eRES_IMAGE_UDISKIDLE, AK_FALSE);
            USB_ShowBat();
        }
    }
#endif
/*
    if ((10 == usb_detect_counter) || (20 == usb_detect_counter) || (30 == usb_detect_counter))
    {
        USB_ShowBat();
    }
*/
}

unsigned char handlepub_usb_detect(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
 
#ifdef OS_ANYKA  
    T_U8 status = 0;

    if (((M_EVT_USER_KEY == event) && (kbOK == pEventParm->c.Param1) \
        && (PRESS_SHORT == pEventParm->c.Param2)))
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        Fwl_UsbOutHandler();
//      usb_slave_reset();
        AK_DEBUG_OUTPUT("keypad out!!\n");
        return 0;                              
    } 

    if((pEventParm != AK_NULL) && (pEventParm->w.Param2 == USB_DISK_PARAM))
    {
        AK_DEBUG_OUTPUT("usb_update_flag == AK_TRUE!\n");
        usb_update_flag = AK_TRUE;
    }

    //cann't understand, delete
    //....
    

    status = Fwl_DetectorGetStatus(DEVICE_USB);
    AK_DEBUG_OUTPUT("stat = %d\n", status);
    if (status)
    {
        Fwl_DelayUs(500000);//delay is need when use hub,otherwise setup_end happen

        AK_DEBUG_OUTPUT("usb connect pc !\r\n");
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_UDISK, AK_NULL);
        Voice_WaitTip();
#endif
        //Aud_BGProcessMsg(M_EVT_USBDISK, AK_NULL);
        m_triggerEvent(M_EVT_USBDISK, AK_NULL);
    }
    else if(usb_update_flag)
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        usb_update_flag = AK_FALSE;
        AK_DEBUG_OUTPUT("update err\n");
    }
    else
    {
        AK_DEBUG_OUTPUT("go to charger\n");
#ifdef SUPPORT_VOICE_TIP
        Voice_PlayTip(eBTPLY_SOUND_TYPE_CHARGING, AK_NULL);
        Voice_WaitTip();
#endif
        m_triggerEvent(M_EVT_CHARGER, AK_NULL);

    }
#else
    m_triggerEvent(M_EVT_EXIT, AK_NULL);
    
#endif
    return 0;
}

