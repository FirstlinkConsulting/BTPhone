/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_repeat.c
 * @BRIEF display total memory and used memory
 * @Author£ºXP
 * @Date£º2008-06-05
 * @Version£º
**************************************************************************/

#include "m_event.h"
#include "M_Event_Api.h"
#include "Ctrl_ListMenu.h"
#include "Eng_Profile.h"
#include "Gbl_Global.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)
        #define REPEAT_IMAGE_LEFT_POS   0
        #define REPEAT_IMAGE_TOP_POS    16
        #define REPEAT_STR_LEFT_POS     32
        #define REPEAT_STR_TOP_POS      36
#endif

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_REP_SET_TITLE_HIGHT    32   //title hight
#define SYS_REP_SET_TITLE_WIDTH    128
#define SYS_REP_SET_TITLE_LEFT     0
#define SYS_REP_SET_TITLE_TOP      0

#endif



typedef struct
{
#if (USE_COLOR_LCD)
    CListMenuCtrl menu;
#else
    T_U8 painttype;    // 0 manu 1 auto
#endif
    T_AUDIO_CFG audConfig ;
}T_SET_REPEAT;

static T_SET_REPEAT *pRepeatSet;
extern T_VOID Radio_DealRadioRecord(T_VOID);

void initset_repeat(void)
{
    pRepeatSet = (T_SET_REPEAT*)Fwl_Malloc(sizeof(T_SET_REPEAT));
    AK_ASSERT_VAL_VOID(pRepeatSet, "alloc pRepeatSet structure error!");

    Profile_ReadData(eCFG_AUDIO, &pRepeatSet->audConfig);
#if (USE_COLOR_LCD)
    //ListMenu_Init(&pRepeatSet->menu, eRES_STR_RADIO_EMPTY_STR,eRES_STR_REPEAT_MANU_CLR, eRES_STR_REPEAT_AUTO_CLR, eRES_IMAGE_MENUICON);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_REP_SET_TITLE_LEFT, (T_POS)SYS_REP_SET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif
    ListMenu_InitEx(&pRepeatSet->menu, 
                    eRES_STR_RADIO_EMPTY_STR,
                    eRES_STR_REPEAT_MANU_CLR, 
                    eRES_STR_REPEAT_AUTO_CLR, 
                    eRES_IMAGE_MENUICON,
                    SYS_REP_SET_TITLE_LEFT,
                    SYS_REP_SET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_REP_SET_TITLE_HIGHT);

    ListMenu_SetStyle(&pRepeatSet->menu,CTRL_LISTMENU_STYLE_QUIT_DISPNONE|CTRL_LISTMENU_STYLE_SCROLLVER_DISPNONE);
    //ListMenu_SetTitle(&pRepeatSet->menu,eRES_IMAGE_TOP_SETUP);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pRepeatSet->menu.listmenu_topbar, eRES_STR_AB_SETTING);
    #endif
#else
    pRepeatSet->painttype = 0;
    Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
#endif

}

void exitset_repeat(void)
{
    Profile_WriteData(eCFG_AUDIO, &pRepeatSet->audConfig);

    if (AK_NULL != pRepeatSet)
    {
#if (USE_COLOR_LCD)
        ListMenu_Free(&pRepeatSet->menu);
#endif
        pRepeatSet = Fwl_Free(pRepeatSet);
    }

}

void paintset_repeat(void)
{
#if (USE_COLOR_LCD)
    ListMenu_Show(&pRepeatSet->menu);
#endif
}
unsigned char handleset_repeat(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#if (USE_COLOR_LCD)
    T_U16 ret = 0;
#endif

    if (event == M_EVT_SET_REPEAT)
    {
#if (USE_COLOR_LCD)
         if (0 == pRepeatSet->audConfig.abAutoRepMode)
         {
            ListMenu_SetFocus(&pRepeatSet->menu,eRES_STR_REPEAT_MANU_CLR);
         }
         else
         {
             ListMenu_SetFocus(&pRepeatSet->menu,eRES_STR_REPEAT_AUTO_CLR);
         }
         ListMenu_SetRefresh(&pRepeatSet->menu);
#else
        pRepeatSet->painttype = pRepeatSet->audConfig.abAutoRepMode;
        if (0 == pRepeatSet->painttype)
        {
            Eng_ImageResDisp(REPEAT_IMAGE_LEFT_POS,REPEAT_IMAGE_TOP_POS, eRES_IMAGE_REPMAN,AK_FALSE);
            Fwl_FillRect(0, REPEAT_STR_TOP_POS, GRAPH_WIDTH, FONT_HEIGHT, CLR_BLACK);
            DispStringInWidth(CP_UNICODE,REPEAT_STR_LEFT_POS,REPEAT_STR_TOP_POS,CONVERT2STR(eRES_STR_REPEAT_MANU),FONT_WND_WIDTH,CLR_WHITE);
        }else
        {
            Eng_ImageResDisp(REPEAT_IMAGE_LEFT_POS,REPEAT_IMAGE_TOP_POS, eRES_IMAGE_REPAUTO,0);
            Fwl_FillRect(0, REPEAT_STR_TOP_POS, GRAPH_WIDTH, FONT_HEIGHT, CLR_BLACK);
            DispStringInWidth(CP_UNICODE,REPEAT_STR_LEFT_POS,REPEAT_STR_TOP_POS,CONVERT2STR(eRES_STR_REPEAT_AUTO),FONT_WND_WIDTH,CLR_WHITE);
        }
#endif
    }

    if(M_EVT_RADIO_RECORD == event)
    {
        Radio_DealRadioRecord();
        return 0;
    }
#if (USE_COLOR_LCD)
    ret = ListMenu_Handler(&pRepeatSet->menu, event, pEventParm);

    switch(ret)
    {
        case eRES_STR_REPEAT_MANU_CLR:
        case eRES_STR_REPEAT_MANU:
            if (1 == pRepeatSet->audConfig.abAutoRepMode)
            {
                pRepeatSet->audConfig.abAutoRepMode = 0;
                Profile_WriteData(eCFG_AUDIO, &pRepeatSet->audConfig);
            }
            m_triggerEvent(M_EVT_EXIT, AK_NULL); 
            break;
        case eRES_STR_REPEAT_AUTO_CLR: 
        case eRES_STR_REPEAT_AUTO: 
            if (0 == pRepeatSet->audConfig.abAutoRepMode)
            {
                pRepeatSet->audConfig.abAutoRepMode = 1;
                Profile_WriteData(eCFG_AUDIO, &pRepeatSet->audConfig);
            }
            m_triggerEvent(M_EVT_EXIT, AK_NULL); 
            break;
        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL); 
            break;
        default:
            break;
    }
#else
    if (M_EVT_USER_KEY == event)
    {
        switch (pEventParm->c.Param1)
        {
            case kbLEFT:
            case kbRIGHT:
                pRepeatSet->painttype = !pRepeatSet->painttype;
                if (0 == pRepeatSet->painttype)
                {
                    Eng_ImageResDisp(REPEAT_IMAGE_LEFT_POS,REPEAT_IMAGE_TOP_POS, eRES_IMAGE_REPMAN,AK_FALSE);
                    Fwl_FillRect(0, REPEAT_STR_TOP_POS, GRAPH_WIDTH, FONT_HEIGHT, CLR_BLACK);
                    DispStringInWidth(CP_UNICODE,REPEAT_STR_LEFT_POS,REPEAT_STR_TOP_POS,CONVERT2STR(eRES_STR_REPEAT_MANU),FONT_WND_WIDTH,CLR_WHITE);
                }else
                {
                    Eng_ImageResDisp(REPEAT_IMAGE_LEFT_POS,REPEAT_IMAGE_TOP_POS, eRES_IMAGE_REPAUTO,0);
                    Fwl_FillRect(0, REPEAT_STR_TOP_POS, GRAPH_WIDTH, FONT_HEIGHT, CLR_BLACK);
                    DispStringInWidth(CP_UNICODE,REPEAT_STR_LEFT_POS,REPEAT_STR_TOP_POS,CONVERT2STR(eRES_STR_REPEAT_AUTO),FONT_WND_WIDTH,CLR_WHITE);
                }
                break;
            case kbOK:
                if (pRepeatSet->painttype != pRepeatSet->audConfig.abAutoRepMode)
                {
                    pRepeatSet->audConfig.abAutoRepMode = pRepeatSet->painttype;
                    Profile_WriteData(eCFG_AUDIO, &pRepeatSet->audConfig);              
                }
            case kbMODE:
                m_triggerEvent(M_EVT_EXIT, AK_NULL); 
                break;
            default:
                break;
       }
    }
#endif

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_repeat(void)
{
}
void paintset_repeat(void)
{
}
void exitset_repeat(void)
{
}
unsigned char handleset_repeat(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:exitset_repeat\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

