/************************************************************************/
/* pstn telephone dial     ly2014                                       */
/************************************************************************/

#include "m_state.h"
#include "Fwl_Keypad.h"


void initpstn_dial(void)
{
}

unsigned char handlepstn_dial(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
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

void paintpstn_dial(void)
{
}

void exitpstn_dial(void)
{
}