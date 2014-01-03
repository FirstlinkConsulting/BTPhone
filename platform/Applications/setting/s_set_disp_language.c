/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_disp_language.c
 * @BRIEF display current language ,choose another language
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-22
 * @Version£º
**************************************************************************/

#include "m_event.h"
#include "Ctrl_ListMenu.h"
#include "M_Event_Api.h"
#include "Eng_Profile.h"
#include "Eng_DataConvert.h"
#include "Gbl_global.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD
#else
#define SYS_LANG_SET_TITLE_HIGHT    32   //title hight
#define SYS_LANG_SET_TITLE_WIDTH    128
#define SYS_LANG_SET_TITLE_LEFT     0
#define SYS_LANG_SET_TITLE_TOP      0

#endif


typedef struct _SET_DISP_LANG
{
    T_SYSTEM_CFG  sysCfg;
    CListMenuCtrl menu;
}T_SET_DISP_LANG;

static T_SET_DISP_LANG *pSetDispLang = AK_NULL;
extern const T_CONFIG_INFO gb_sysconfig;

void initset_disp_language(void)
{
    pSetDispLang = (T_SET_DISP_LANG*)Fwl_Malloc(sizeof(T_SET_DISP_LANG));
    AK_ASSERT_VAL_VOID(pSetDispLang, "alloc pSetDispLang structure error!");
    
    Profile_ReadData(eCFG_SYSTEM, (T_VOID*)&pSetDispLang->sysCfg);
    #if  (USE_COLOR_LCD)
    //ListMenu_Init(&pSetDispLang->menu, eRES_STR_RADIO_EMPTY_STR, eRES_STR_LANG_CHINESE_SIMPLE,eRES_STR_LANG_SPAIN, eRES_IMAGE_MENUICON);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_LANG_SET_TITLE_LEFT, (T_POS)SYS_LANG_SET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif

    ListMenu_InitEx(&pSetDispLang->menu, 
                    eRES_STR_RADIO_EMPTY_STR, 
                    eRES_STR_LANG_CHINESE_SIMPLE,
                    eRES_STR_LANG_THAI, 
                    eRES_IMAGE_MENUICON,
                    SYS_LANG_SET_TITLE_LEFT,
                    SYS_LANG_SET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_LANG_SET_TITLE_HIGHT);

    //ListMenu_SetTitle(&pSetDispLang->menu,eRES_IMAGE_TOP_SETUP);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pSetDispLang->menu.listmenu_topbar, eRES_STR_LANGUAGE_SELECT);
    #endif
    #else
    ListMenu_Init(&pSetDispLang->menu, eRES_STR_SYSCFG, eRES_STR_LANG_CHINESE_SIMPLE,eRES_STR_LANG_THAI, eRES_IMAGE_MENUICON);
    #endif
    ListMenu_SetStyle(&pSetDispLang->menu, CTRL_LISTMENU_STYLE_QUIT_DISPNONE|CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW);
    
    if(gb_sysconfig.language_set.Chinese_simply == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_CHINESE_SIMPLE);

    if(gb_sysconfig.language_set.Chinese_tradition == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_CHINESE_TRADITION);

    if(gb_sysconfig.language_set.Chinese_big5 == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_CHINESE_BIG5);

    if(gb_sysconfig.language_set.English == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_ENGLISH);

    if(gb_sysconfig.language_set.Thai == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_THAI);
    
    if(gb_sysconfig.language_set.Spain == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_SPAIN);

    if(gb_sysconfig.language_set.Japanese == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_JAPANESE);

    if(gb_sysconfig.language_set.Korean == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_KOREAN);

    if(gb_sysconfig.language_set.French == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_FRENCH);

    if(gb_sysconfig.language_set.Genman == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_GERMAN);

    if(gb_sysconfig.language_set.Italy == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_ITALY);

    if(gb_sysconfig.language_set.Netherlands == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_NETHERLANDS);

    if(gb_sysconfig.language_set.Poutugal == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_PORTUGAL);

    if(gb_sysconfig.language_set.Swendish == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_SWEDISH);

    if(gb_sysconfig.language_set.Czech == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_CZECH);

    if(gb_sysconfig.language_set.Danish == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_DANISH);

    if(gb_sysconfig.language_set.Polish == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_POLISH);

    if(gb_sysconfig.language_set.Russian == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_RUSSIAN);

    if(gb_sysconfig.language_set.Turkish == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_TURKISH);

    if(gb_sysconfig.language_set.Hebrew == 0)
        ListMenu_DeleteItem(&pSetDispLang->menu, eRES_STR_LANG_HEBREW);   
    
    BETWEAEN_WITH(pSetDispLang->sysCfg.Lang, 0, eRES_LANG_NUM- 1);
    ListMenu_SetFocus(&pSetDispLang->menu,(T_U16)GetNearbyValidStrResID(eRES_STR_LANG_CHINESE_SIMPLE,gb.Lang,(T_S16)pSetDispLang->sysCfg.Lang));
}

void exitset_disp_language(void)
{
    if (pSetDispLang->sysCfg.Lang != gb.Lang)
    {
        if(AK_TRUE == CodePage_Set(GetLangCodePage(gb.Lang)))
        {
            pSetDispLang->sysCfg.Lang = gb.Lang;
            Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetDispLang->sysCfg);
        }
        else
        {
            gb.Lang = pSetDispLang->sysCfg.Lang;
        }    
    }
    if (pSetDispLang != AK_NULL)
    {
        ListMenu_Free(&pSetDispLang->menu);
        pSetDispLang = Fwl_Free(pSetDispLang);
    }
}

void paintset_disp_language(void)
{
    ListMenu_Show(&pSetDispLang->menu);
}

unsigned char handleset_disp_language(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;
    //T_PRESS_KEY phyKey;
        
    ret = ListMenu_Handler(&pSetDispLang->menu, event, pEventParm);

    switch(ret)
    {
        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_CHINESE_SIMPLE:
            gb.Lang = eRES_LANG_CHINESE_SIMPLE;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_CHINESE_TRADITION:
            gb.Lang = eRES_LANG_CHINESE_TRADITION;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_CHINESE_BIG5:
            gb.Lang = eRES_LANG_CHINESE_BIG5;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_ENGLISH:
            gb.Lang = eRES_LANG_ENGLISH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_THAI:
            gb.Lang = eRES_LANG_THAI;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_SPAIN:
            gb.Lang = eRES_LANG_SPAIN;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_JAPANESE:
            gb.Lang = eRES_LANG_JAPANESE;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_KOREAN:
            gb.Lang = eRES_LANG_KOREAN;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_FRENCH:
            gb.Lang = eRES_LANG_FRENCH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_GERMAN:
            gb.Lang = eRES_LANG_GERMAN;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_ITALY:
            gb.Lang = eRES_LANG_ITALY;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_NETHERLANDS:
            gb.Lang = eRES_LANG_NETHERLANDS;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_PORTUGAL:
            gb.Lang = eRES_LANG_PORTUGAL;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_SWEDISH:
            gb.Lang = eRES_LANG_SWEDISH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_CZECH:
            gb.Lang = eRES_LANG_CZECH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_DANISH:
            gb.Lang = eRES_LANG_DANISH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_POLISH:
            gb.Lang = eRES_LANG_POLISH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_RUSSIAN:
            gb.Lang = eRES_LANG_RUSSIAN;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_TURKISH:
            gb.Lang = eRES_LANG_TURKISH;
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case eRES_STR_LANG_HEBREW:
            gb.Lang = eRES_LANG_HEBREW;
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
void initset_disp_language(void)
{
}
void paintset_disp_language(void)
{
}
void exitset_disp_language(void)
{
}
unsigned char handleset_disp_language(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_disp_language\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

