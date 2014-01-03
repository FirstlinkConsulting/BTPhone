/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_connecting.c
 * @BRIEF display total memory and used memory
 * @Author£ºXP
 * @Date£º2008-06-05
 * @Version£º
**************************************************************************/
#include "m_event.h"
#include "M_Event_Api.h"
#include "Ctrl_ListMenu.h"
#include "Eng_Profile.h"
#include "Gbl_global.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD
#else
#define SYS_CONNECT_SET_TITLE_HIGHT    32   //title hight
#define SYS_CONNECT_SET_TITLE_WIDTH    128
#define SYS_CONNECT_SET_TITLE_LEFT     0
#define SYS_CONNECT_SET_TITLE_TOP      0
#endif

typedef struct
{
    CListMenuCtrl   menu;
     T_SYSTEM_CFG   sysCfg;
}T_SET_CONNECTING;

static T_SET_CONNECTING *pConnectingMode = AK_NULL;

void initset_connecting(void)
{
    pConnectingMode = (T_SET_CONNECTING*)Fwl_Malloc(sizeof(T_SET_CONNECTING));
    AK_ASSERT_VAL_VOID(pConnectingMode, "alloc pConnectingMode structure error!");

    Profile_ReadData(eCFG_SYSTEM, &pConnectingMode->sysCfg);

    #if (USE_COLOR_LCD)
    //ListMenu_Init(&pConnectingMode->menu, eRES_STR_RADIO_EMPTY_STR,eRES_STR_MULTI_DRIVE, eRES_STR_ENCPT_ONLY, eRES_IMAGE_MENUICON);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_CONNECT_SET_TITLE_LEFT, (T_POS)SYS_CONNECT_SET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif

    ListMenu_InitEx(&pConnectingMode->menu, 
                    eRES_STR_RADIO_EMPTY_STR,
                    eRES_STR_MULTI_DRIVE, 
                    eRES_STR_ENCPT_ONLY, 
                    eRES_IMAGE_MENUICON,
                    SYS_CONNECT_SET_TITLE_LEFT,
                    SYS_CONNECT_SET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_CONNECT_SET_TITLE_HIGHT);

    //ListMenu_SetTitle(&pConnectingMode->menu,eRES_IMAGE_TOP_SETUP);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pConnectingMode->menu.listmenu_topbar, eRES_STR_CONNECT_MODE);
    #endif
    #else
    ListMenu_Init(&pConnectingMode->menu, eRES_STR_SYSCFG,eRES_STR_MULTI_DRIVE, eRES_STR_ENCPT_ONLY, eRES_IMAGE_MENUICON);  
    #endif
    ListMenu_SetFocus(&pConnectingMode->menu, pConnectingMode->sysCfg.ConnectMode);
    ListMenu_SetStyle(&pConnectingMode->menu,CTRL_LISTMENU_STYLE_QUIT_DISPNONE|CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW);

}
void exitset_connecting(void)
{
    if (AK_NULL != pConnectingMode)
    {
        Profile_WriteData(eCFG_SYSTEM, &pConnectingMode->sysCfg);
        ListMenu_Free(&pConnectingMode->menu);
        pConnectingMode = Fwl_Free(pConnectingMode);
    }
}

void paintset_connecting(void)
{
    ListMenu_Show(&pConnectingMode->menu);
}
unsigned char handleset_connecting(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret = 0;
    //T_PRESS_KEY phyKey;

    ret = ListMenu_Handler(&pConnectingMode->menu, event, pEventParm);

    switch(ret)
    {
        case eRES_STR_MULTI_DRIVE:
            pConnectingMode->sysCfg.ConnectMode = (T_RES_STRING)ret;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;

        case eRES_STR_ENCPT_ONLY:
            pConnectingMode->sysCfg.ConnectMode = (T_RES_STRING)ret;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;

        case eRES_STR_NORMAL_ONLY:
            pConnectingMode->sysCfg.ConnectMode = (T_RES_STRING)ret;

        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL); 
            break;

        default:
            break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_connecting(void)
{
}
void exitset_connecting(void)
{
}
void paintset_connecting(void)
{
}
unsigned char handleset_connecting(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_connecting\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}
#endif


