/************************************************************************/
/*      pstn phone handle ly 201401                                     */
/************************************************************************/

/************************************************************************/
/* pstn telephone main     ly2014                                       */
/************************************************************************/

#include "vme.h"
#include "m_event_api.h"
#include "fwl_keypad.h"
#include "m_event.h"
#include "eng_debug.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Gpio.h"

void initpstn_phone(void)
{
/*	Fwl_GpioInit();*/
}

unsigned char handlepstn_phone(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
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
	default:
		break;
	}
	return 0;

}

void exitpstn_phone(void)
{
}

void paintpstn_phone(void)
{
}


