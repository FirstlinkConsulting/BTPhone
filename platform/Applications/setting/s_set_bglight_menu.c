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
#include "M_Event_Api.h"
#include "Eng_Profile.h"
#include "Ctrl_ListMenu.h"
#include "Eng_AutoOff.h"
#include "Fwl_osMalloc.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_BG_MENU_TITLE_HIGHT    32   //title hight
#define SYS_BG_MENU_TITLE_WIDTH    128
#define SYS_BG_MENU_TITLE_LEFT     0
#define SYS_BG_MENU_TITLE_TOP      0
#endif


typedef struct _SET_BGLIGHT_TIME
{
    CListMenuCtrl SysBgMenu;
    T_BOOL        bMsg;
}T_SYS_BG_MENU;


static T_SYS_BG_MENU *pSetBgLightMenu = AK_NULL;

void initset_bglight_menu(void)
{
    #if (USE_COLOR_LCD)
    pSetBgLightMenu = (T_SYS_BG_MENU*)Fwl_Malloc(sizeof(T_SYS_BG_MENU));
    AK_ASSERT_VAL_VOID(pSetBgLightMenu, "alloc T_SET_BGLIGHT_TIME structure error!");

    //ListMenu_Init(&pSetBgLightMenu->SysBgMenu,eRES_STR_RADIO_EMPTY_STR,
    //              eRES_STR_SYS_LCD_OFF,
    //              eRES_STR_SYS_LCD_OFF,
    //              eRES_IMAGE_MENUICON);
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_BG_MENU_TITLE_LEFT, (T_POS)SYS_BG_MENU_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
#endif
    if(Fwl_Enable_Set_Contrast())
    {
            ListMenu_InitEx(&pSetBgLightMenu->SysBgMenu,eRES_STR_RADIO_EMPTY_STR,
                    eRES_STR_SYS_LCD_OFF,
                    eRES_STR_LCD_CONTRAST_CR,
                    eRES_IMAGE_MENUICON,
                    SYS_BG_MENU_TITLE_LEFT,
                    SYS_BG_MENU_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_BG_MENU_TITLE_HIGHT);
    }
    else
    {
            ListMenu_InitEx(&pSetBgLightMenu->SysBgMenu,eRES_STR_RADIO_EMPTY_STR,
                    eRES_STR_SYS_LCD_OFF,
                    eRES_STR_SYS_LCD_OFF,
                    eRES_IMAGE_MENUICON,
                    SYS_BG_MENU_TITLE_LEFT,
                    SYS_BG_MENU_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_BG_MENU_TITLE_HIGHT);
    }
    //ListMenu_SetTitle(&pSetBgLightMenu->SysBgMenu,eRES_IMAGE_TOP_SETUP);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pSetBgLightMenu->SysBgMenu.listmenu_topbar ,eRES_STR_SYS_BGLIGHT_TIME);
    #endif
    #endif
}

void exitset_bglight_menu(void)
{
    pSetBgLightMenu = Fwl_Free(pSetBgLightMenu);
}

void paintset_bglight_menu(void)
{
    #if (USE_COLOR_LCD)
    ListMenu_Show(&pSetBgLightMenu->SysBgMenu);
    #endif
}

unsigned char handleset_bglight_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    #if (USE_COLOR_LCD)
    T_U16 ret;
    //T_PRESS_KEY phyKey;

    if(M_EVT_EXIT == event)
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);

        if (event >= M_EVT_Z00_POWEROFF)
            return 1;
        else
            return 0;       
    }   

    ret = ListMenu_Handler(&pSetBgLightMenu->SysBgMenu, event, pEventParm);

    switch(ret)
    {
        case eRES_STR_SYS_LCD_OFF:
            m_triggerEvent(M_EVT_6, AK_NULL);
            break;
        case eRES_STR_LCD_CONTRAST_CR:
            m_triggerEvent(M_EVT_CST, AK_NULL);
            break;

        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        case CTRL_RESPONSE_NONE:
        default:
            break;
    }
    #endif

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;

}

#else
void initset_bglight_menu(void)
{
}
void exitset_bglight_menu(void)
{
}
void paintset_bglight_menu(void)
{
}
unsigned char handleset_bglight_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_bglight_menu\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}
#endif

