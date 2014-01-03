/************************************************************************
 * Copyright (c) 2008, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author£ºzhao_xiaowei
 * @Date£º
 * @Version£º
**************************************************************************/

#include "Apl_Public.h"
#include "Fwl_LCD.h"
#include "m_event.h"
#include "Fwl_osFS.h"
#include "Gbl_Resource.h"
#include "Fwl_Timer.h"
#include "Gui_Common.h"
#include "Fwl_osMalloc.h"
#include "M_event_api.h"

typedef struct 
{
    T_U32       delayCnt;
}T_NoSDUI;

static T_NoSDUI* pNoSDUI = AK_NULL;

void initpub_pullout_sd(void)
{
    Fwl_LCD_lock(AK_FALSE);
    pNoSDUI= Fwl_Malloc(sizeof(T_NoSDUI));
    pNoSDUI->delayCnt = 0;  
#if (USE_COLOR_LCD)
    Gui_DispResHint(eRES_STR_CARD_PULLOUT, CLR_BLACK, CLR_OKBG, -1);
#else
    Gui_DispResHint(eRES_STR_CARD_PULLOUT, CLR_WHITE, CLR_BLACK, -1);
#endif  

}

void exitpub_pullout_sd(void)
{
    pNoSDUI= Fwl_Free(pNoSDUI);
}

void paintpub_pullout_sd(void)
{
}

unsigned char handlepub_pullout_sd(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if(event == M_EVT_USER_KEY)
    {
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
    }
    else if(event == M_EVT_PUB_TIMER)
    {
        pNoSDUI->delayCnt++;
    }

    if (pNoSDUI->delayCnt >= 2)
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
    }
    
    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

/* end of file */
