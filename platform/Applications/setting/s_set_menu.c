/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_audio_record_menu.c
 * @BRIEF audio record setting menu 
 * @Author：Huang_ChuSheng
 * @Date：2008-04-19
 * @Version：
**************************************************************************/

#include "m_event.h"
#include "Ctrl_ListMenu.h"
#include "M_Event_Api.h"
#include "Fwl_FreqMgr.h"
//#include "Arch_gpio.h"
#include "Gbl_Global.h"
#include "Eng_USB.h"
#include "Fwl_osMalloc.h"

#include "Fwl_Mount.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD
    #define ICON_INTERVAL           16  
#else     //for CW LCD

#if (LCD_TYPE != 3)
#define SYS_SET_MENU_HIGHT          32   //title hight
#define SYS_SET_MENU_WIDTH          128
#else
#define SYS_SET_MENU_HIGHT          32   //title hight
#define SYS_SET_MENU_WIDTH          240
#endif
#define SYS_SET_MENU_TITLE_LEFT     0
#define SYS_SET_MENU_TITLE_TOP      0

#endif

typedef struct _SET_MENU
{
    CListMenuCtrl menu;
}T_SET_MENU;

static T_SET_MENU *pSetMenu = AK_NULL;

#ifdef SUPPORT_USBHOST
static T_BOOL   gs_selfflash = AK_FALSE;
#endif

void initset_menu(void)
{
    pSetMenu = (T_SET_MENU*)Fwl_Malloc(sizeof(T_SET_MENU));
    AK_ASSERT_VAL_VOID(pSetMenu, "alloc pSetMenu structure error!");

    #if (!USE_COLOR_LCD)
    ListMenu_Init(&pSetMenu->menu, eRES_STR_SYSCFG, eRES_STR_SYSTIME,eRES_STR_UPDATA, eRES_IMAGE_MENUICON);
//ListMenu_SetStyle(&pSetMenu->menu, CTRL_LISTMENU_STYLE_DEFAULT|CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT);
    #else
    //SysSetDispTitle();
    #if ((3 != LCD_TYPE) || (1 != LCD_HORIZONTAL))  
    Eng_ImageResDisp(SYS_SET_MENU_TITLE_LEFT, (T_POS)SYS_SET_MENU_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif
    ListMenu_InitEx(&pSetMenu->menu, 
                    eRES_STR_RADIO_EMPTY_STR, 
                    eRES_STR_SYS_TIME,
                    eRES_STR_SYS_USB_SLAVE/*eRES_STR_SYS_UPDATA*/,  
                    eRES_IMAGE_MENUICON,
                    0,
                    SYS_SET_MENU_HIGHT,
                    MAIN_LCD_WIDTH,
                    (T_LEN)(MAIN_LCD_HEIGHT-SYS_SET_MENU_HIGHT));
    //ListMenu_Init(&pSetMenu->menu, eRES_STR_RADIO_EMPTY_STR, eRES_STR_SYS_TIME,eRES_STR_SYS_UPDATA, eRES_IMAGE_MENUICON); 
    //ListMenu_SetTitle(&pSetMenu->menu,eRES_IMAGE_TOP_SETUP);
//ListMenu_SetStyle(&pSetMenu->menu, CTRL_LISTMENU_STYLE_DEFAULT|CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT);
    #endif

#ifndef SUPPORT_CALENDAR
    #if (!USE_COLOR_LCD)
    ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_CALENDAR);
    #else
    ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_CALENDAR);
    #endif
#endif
#if(!USE_ALARM_CLOCK)
    #if(!USE_COLOR_LCD)
             ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_ALARMSET)
    #else
             ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_ALARMSET);
    #endif
#endif
#ifdef SUPPORT_512B_PAGE
    #if (!USE_COLOR_LCD)
    ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_UPDATA);
    #else
    ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_UPDATA);
    #endif
#endif

//QVGA横屏模式删除日历，闹钟菜单
#if (USE_COLOR_LCD)
    #if (3 == LCD_TYPE && 1 == LCD_HORIZONTAL)  
        TopBar_ConfigTitleSet(&pSetMenu->menu.listmenu_topbar, eRES_STR_SYS_SETTING);
        #ifdef  SUPPORT_CALENDAR
            ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_CALENDAR);
        #endif
        #if (USE_ALARM_CLOCK)
            ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_ALARMSET);
        #endif
    #endif
#endif

#ifdef SUPPORT_USBHOST
    gs_selfflash = Fwl_MemDevIsMount(USB_HOST_DISK);

    if (AK_FALSE == Fwl_MemDevIsMount(USB_HOST_DISK))     //显示当前的USB模式
    {
        ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_USB_SLAVE);
    }
    else
    {
        ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_USB_HOST);
    }
#else
        ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_USB_SLAVE);
        ListMenu_DeleteItem(&pSetMenu->menu, eRES_STR_SYS_USB_HOST);
#endif

}

void exitset_menu(void)
{
    if (AK_NULL != pSetMenu)
    {
        ListMenu_Free(&pSetMenu->menu);
        pSetMenu = Fwl_Free(pSetMenu);
    }
}

void paintset_menu(void)
{
    ListMenu_Show(&pSetMenu->menu);
}

unsigned char handleset_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;

    //在set_menu界面拔出U盘，让set_menu界面自刷新一下，让退出主机模式字样变成主机模式字样
#ifdef SUPPORT_USBHOST

    if (gs_selfflash != Fwl_MemDevIsMount(USB_HOST_DISK)) 
    {
        m_triggerEvent(M_EVT_SELFFLASH, AK_NULL);
        return 0;
    } 
#endif
    
    //AK_DEBUG_OUTPUT("handleset_menu\n");

    if (M_EVT_EXIT == event)
    {
        #if (USE_COLOR_LCD)
        #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
        Eng_ImageResDisp(SYS_SET_MENU_TITLE_LEFT, (T_POS)SYS_SET_MENU_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
        #endif
        #endif
        ListMenu_SetRefresh(&pSetMenu->menu);
    }
    
    ret = ListMenu_Handler(&pSetMenu->menu, event, pEventParm);
    
    switch(ret)
    {
        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LCD_CONTRAST:
            m_triggerEvent(M_EVT_1, AK_NULL); //disp contrast
            break;
        case eRES_STR_VERSION:
        case eRES_STR_SYS_VERSION:
            m_triggerEvent(M_EVT_2, AK_NULL); //disp version
            break;
        case eRES_STR_MEMORY_INFOR:
        case eRES_STR_SYS_MEMORY_INFOR:
            m_triggerEvent(M_EVT_3, AK_NULL); //disp memory
            break;
        case eRES_STR_LANGUAGE_SELECT:
        case eRES_STR_SYS_LANGUAGE_SELECT:
            m_triggerEvent(M_EVT_4, AK_NULL); //choose language
            break;
        case eRES_STR_AUTO_POWEROFF_TIME:
        case eRES_STR_SYS_AUTO_POWEROFF_TIME:
            m_triggerEvent(M_EVT_5, AK_NULL); //set auto power off time
            break;
        case eRES_STR_BGLIGHT_TIME:
        case eRES_STR_SYS_BGLIGHT_TIME: 
            #if (!USE_COLOR_LCD)
            m_triggerEvent(M_EVT_6, AK_NULL); //set back ground light time 
            #else
            m_triggerEvent(M_EVT_9, AK_NULL); 
            #endif
            break;
        case eRES_STR_SYSTIME: 
        case eRES_STR_SYS_TIME:
            m_triggerEvent(M_EVT_7, AK_NULL); //set current system time
            break;
        case eRES_STR_AB_SETTING:
        case eRES_STR_SYS_AB_SETTING:
            m_triggerEvent(M_EVT_SET_REPEAT, AK_NULL); //repeat setting
            break;
        case eRES_STR_CONNECT_MODE: // connect mode
        case eRES_STR_SYS_CONNECT_MODE:
            m_triggerEvent(M_EVT_CONNECT, AK_NULL);
            break;
        case eRES_STR_ALARMSET:
        case eRES_STR_SYS_ALARMSET:
            m_triggerEvent(M_EVT_CLK, AK_NULL);
            break;
        case eRES_STR_UPDATA:   //firmware updata
        case eRES_STR_SYS_UPDATA:
            //pEventParm->w.Param1 = USB_DETECT_LCD_LOCK;
            // pEventParm->w.Param2 = USB_DISK_PARAM;
            //m_triggerEvent(M_EVT_EXIT, pEventParm);
            //m_triggerEvent(M_EVT_USB_DETECT, pEventParm);
            //Fwl_LCD_lock(AK_TRUE);
            m_triggerEvent(M_EVT_UPDATA, pEventParm);
            break;
#ifdef SUPPORT_CALENDAR
        case eRES_STR_CALENDAR:   //calendar
        case eRES_STR_SYS_CALENDAR:
            m_triggerEvent(M_EVT_CALENDAR, pEventParm);
            break;
#endif
#ifdef SUPPORT_USBHOST
        case eRES_STR_USB_HOST:   //firmware updata
        case eRES_STR_SYS_USB_HOST:
        case eRES_STR_USB_SLAVE:   //firmware updata
        case eRES_STR_SYS_USB_SLAVE:
        {
            m_triggerEvent(M_EVT_SET_USB_HOST, pEventParm);

        }
        break;
#endif

        default:
            break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}


T_BOOL IsInSysSet(T_VOID)
{
    if(pSetMenu == AK_NULL)
        return AK_FALSE;
    else
        return AK_TRUE;
}

#else
void initset_menu(void)
{
}
void exitset_menu(void)
{
}
void paintset_menu(void)
{
}
unsigned char handleset_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("enter setting menu\n");
    m_triggerEvent(M_EVT_EXIT, NULL);
    return 0;
}
#endif
