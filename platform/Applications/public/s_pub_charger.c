/**
 * @FILENAME: s_pub_charger.c
 * @BRIEF charger state machine
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR Justin.Zhao
 * @DATE 2008-8-22
 * @VERSION 1.0
 * @REF
 */
#include "Apl_Public.h"
#include "Gbl_resource.h"
#include "Eng_ImageResDisp.h"
#include "Eng_Font.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Keypad.h"
#include "Gbl_ImageRes.h"
#include "Fwl_Timer.h"
#include "Eng_AutoOff.h"
#include "Eng_USB.h"
#include "Gbl_Global.h"
#include "Fwl_LCD.h"
#include "m_event_api.h"

void initpub_charger(void)
{    
    if (bglight_state_off())
        bglight_on(AK_TRUE);
    AutoPowerOffDisable();
}

void exitpub_charger(void)
{
    Fwl_FreqPop();
    if (bglight_state_off())
        bglight_on(AK_TRUE);
    AutoPowerOffEnable();
    if (AK_FALSE == gb.power_on)
    {        
        VME_EvtQueuePut(M_EVT_Z00_POWEROFF, AK_NULL);
    }
	AK_PRINTK("exit charger",0,1);
}

void paintpub_charger(void)
{
}

unsigned char handlepub_charger(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad;

    switch(event)
    {
    case M_EVT_USB_OUT:
		m_triggerEvent(M_EVT_EXIT, pEventParm);
		break;
    case M_EVT_USER_KEY: 
        keyPad.id = pEventParm->c.Param1;
        keyPad.pressType = pEventParm->c.Param2;

        if (PRESS_SHORT == keyPad.pressType
            && kbOK == keyPad.id)
            VME_EvtQueuePut(M_EVT_EXIT, pEventParm);

        break;
    case M_EVT_Z00_POWEROFF:
        Fwl_LCD_lock(AK_FALSE);
        break;
    }
    return 0;
}


/* end of files */
