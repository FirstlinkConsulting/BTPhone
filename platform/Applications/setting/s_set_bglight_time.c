/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_bglight.c
 * @BRIEF set background light time
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-22
 * @Version£º
**************************************************************************/

#include "m_event.h"
#include "Gbl_resource.h"
#include "gbl_global.h"
#include "Eng_Profile.h"
#include "Ctrl_Progress.h"
#include "Eng_AutoOff.h"
#include "Gui_Common.h"
#include "M_Event_Api.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_BG_TIMESET_TITLE_HIGHT    32   //title hight
#define SYS_BG_TIMESET_TITLE_WIDTH    128
#define SYS_BG_TIMESET_TITLE_LEFT     0
#define SYS_BG_TIMESET_TITLE_TOP      0
#endif

typedef struct _SET_BGLIGHT_TIME
{
    T_SYSTEM_CFG  sysCfg;
    CProgressCtrl prgress;
}T_SET_BGLIGHT_TIME;

#define BGLIGHT_TIME_MIN_VALUE 0
#define BGLIGHT_TIME_MAX_VALUE 30

static T_SET_BGLIGHT_TIME *pSetBgLightTime = AK_NULL;

void initset_bglight_time(void)
{
    pSetBgLightTime = (T_SET_BGLIGHT_TIME*)Fwl_Malloc(sizeof(T_SET_BGLIGHT_TIME));
    AK_ASSERT_VAL_VOID(pSetBgLightTime, "alloc T_SET_BGLIGHT_TIME structure error!");

    Profile_ReadData(eCFG_SYSTEM, (T_VOID*)&pSetBgLightTime->sysCfg);

    if (/*pSetBgLightTime->sysCfg.BgLightTime<BGLIGHT_TIME_MIN_VALUE ||*/ pSetBgLightTime->sysCfg.BgLightTime>BGLIGHT_TIME_MAX_VALUE)
    {
        Profile_CheckData(eCFG_SYSTEM, &pSetBgLightTime->sysCfg, AK_FALSE);
    }
#if (USE_COLOR_LCD)
    //Progress_Init(&pSetBgLightTime->prgress, BGLIGHT_TIME_MIN_VALUE, BGLIGHT_TIME_MAX_VALUE, eRES_STR_PUB_DEL_HINT, 
    //            INVALID_IMAGERES_ID, 10, pSetBgLightTime->sysCfg.BgLightTime, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_BG_TIMESET_TITLE_LEFT, (T_POS)SYS_BG_TIMESET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif
    Progress_InitEx(&pSetBgLightTime->prgress, 
                    BGLIGHT_TIME_MIN_VALUE, 
                    BGLIGHT_TIME_MAX_VALUE, 
                    eRES_STR_PUB_DEL_HINT, 
                    INVALID_IMAGERES_ID, 
                    10, 
                    pSetBgLightTime->sysCfg.BgLightTime, 
                    AK_FALSE, 
                    INVALID_IMAGERES_ID, 
                    INVALID_IMAGERES_ID,
                    SYS_BG_TIMESET_TITLE_LEFT,
                    SYS_BG_TIMESET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_BG_TIMESET_TITLE_HIGHT);

    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pSetBgLightTime->prgress.pro_topbar, eRES_STR_SYS_LCD_OFF);
    #endif
    //Progress_SetTitle(&pSetBgLightTime->prgress,GetUITitleResID());
#else
    Progress_Init(&pSetBgLightTime->prgress, BGLIGHT_TIME_MIN_VALUE, BGLIGHT_TIME_MAX_VALUE, INVALID_STRINGRES_ID, \
                eRES_IMAGE_BGLIGHT, 10, pSetBgLightTime->sysCfg.BgLightTime, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);
#endif
}

void exitset_bglight_time(void)
{
    Profile_WriteData(eCFG_SYSTEM,&pSetBgLightTime->sysCfg);
    pSetBgLightTime = Fwl_Free(pSetBgLightTime);
}

void paintset_bglight_time(void)
{
    Progress_Show(&pSetBgLightTime->prgress);
}

unsigned char handleset_bglight_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;
    T_U16 value;
    //T_PRESS_KEY phyKey;

    ret = Progress_Handler(&pSetBgLightTime->prgress, event, pEventParm);

    switch(ret)
    {
        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case CTRL_RESPONSE_NONE:
            break;
        case CTRL_PROGRESS_RESPONSE_CHANGED:
            value = (T_U8)Progress_GetPos(&pSetBgLightTime->prgress);

            if (value != pSetBgLightTime->sysCfg.BgLightTime)
            {
                pSetBgLightTime->sysCfg.BgLightTime = (T_U8)value;
                gb.BgLightTime = (T_U8)value;
                Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetBgLightTime->sysCfg);
                AutoBgLightOffSet(gb.BgLightTime);
            }
            break;
            
        default:

            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_bglight_time(void)
{
}
void exitset_bglight_time(void)
{
}
void paintset_bglight_time(void)
{
}
unsigned char handleset_bglight_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_bglight_time\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}
#endif

