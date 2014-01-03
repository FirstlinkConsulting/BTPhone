/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_poweroff_time.c
 * @BRIEF set auto power off time 
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-22
 * @Version£º
**************************************************************************/
#include "Fwl_Keypad.h"
#include "eng_autooff.h"
#include "Eng_Profile.h"
#include "Ctrl_ListMenu.h"
#include "Ctrl_Progress.h"
#include "Gbl_Global.h"
#include "Gui_Common.h"
#include "M_Event_Api.h"

#if(NO_DISPLAY == 0)

#define SAVE_MODE             0
#define SLEEP_MODE            1

#if (!USE_COLOR_LCD)
#define IMG_MODE_TOP_POS      16
#define STR_MODE_LEFT_POS     (FONT_WIDTH<<1)
#define STR_MODE_TOP_POS      40
#define IMG_MODE_LEFT_POS     ((GRAPH_WIDTH-Eng_GetResImageWidth(eRES_IMAGE_PWRSAVE))>>1)
#define IMG_MODE_LEFT_OFFSET  44
#endif

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_POFF_SET_TITLE_HIGHT    32   //title hight
#define SYS_POFF_SET_TITLE_WIDTH    128
#define SYS_POFF_SET_TITLE_LEFT     0
#define SYS_POFF_SET_TITLE_TOP      0

#endif


typedef struct _SET_POFF_TIME
{
    T_SYSTEM_CFG  sysCfg;
    CProgressCtrl prgress;
#if (USE_COLOR_LCD) 
    CListMenuCtrl menu;
#else
    T_U8          refreshFlag;
#endif
    T_U8          curMMI;
    T_U8          curMode;  
}T_SET_POFF_TIME;

#define POWEROFF_TIME_MIN_VALUE     0
#define POWEROFF_TIME_MAX_VALUE     60
#define POWEROFF_TIME_MAX_VALUE1    120

T_SET_POFF_TIME *pSetPoffTime = AK_NULL;

#if (!USE_COLOR_LCD)
#define SET_POFFTIME_REFRESH_ALL     0xff
#define SET_POFFTIME_REFRESH_NONE    0x00


static T_BOOL SetPoffTime_SetRefresh(T_U8 refresh)
{
    AK_ASSERT_PTR(pSetPoffTime, "SetPoffTime_SetRefresh(): pSetPoffTime null", AK_FALSE);
    
    if (SET_POFFTIME_REFRESH_NONE != refresh)
    {
        pSetPoffTime->refreshFlag |= refresh;
    }
    else
    {
        pSetPoffTime->refreshFlag = refresh;
    }
    return AK_TRUE;
}
#endif

void initset_poweroff_time(void)
{
    pSetPoffTime = (T_SET_POFF_TIME*)Fwl_Malloc(sizeof(T_SET_POFF_TIME));
    AK_ASSERT_VAL_VOID(pSetPoffTime, "alloc T_SET_POFF_TIME structure error!");

    pSetPoffTime->curMMI = 0;
    pSetPoffTime->curMode = SAVE_MODE;
    Profile_ReadData(eCFG_SYSTEM, &pSetPoffTime->sysCfg);

    
    if (/*pSetPoffTime->sysCfg.PoffTime<POWEROFF_TIME_MIN_VALUE || */pSetPoffTime->sysCfg.PoffTime>POWEROFF_TIME_MAX_VALUE)
    {
        Profile_CheckData(eCFG_SYSTEM, &pSetPoffTime->sysCfg, AK_FALSE);
    }
    pSetPoffTime->sysCfg.PoffTimeSleepMode = gb.PoffTimeSleepMode;
#if (USE_COLOR_LCD)    
    //ListMenu_Init(&pSetPoffTime->menu, eRES_STR_RADIO_EMPTY_STR, eRES_STR_OFF_TIME,eRES_STR_SLEEP_TIME, eRES_IMAGE_MENUICON);
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_POFF_SET_TITLE_LEFT, (T_POS)SYS_POFF_SET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
#endif
    ListMenu_InitEx(&pSetPoffTime->menu, 
                    eRES_STR_RADIO_EMPTY_STR, 
                    eRES_STR_OFF_TIME,eRES_STR_SLEEP_TIME, 
                    eRES_IMAGE_MENUICON,
                    SYS_POFF_SET_TITLE_LEFT,
                    SYS_POFF_SET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_POFF_SET_TITLE_HIGHT);

    ListMenu_SetFocus(&pSetPoffTime->menu, eRES_STR_OFF_TIME);
    ListMenu_SetStyle(&pSetPoffTime->menu, CTRL_LISTMENU_STYLE_QUIT_DISPNONE | CTRL_LISTMENU_STYLE_SCROLLVER_DISPNONE);
    //ListMenu_SetTitle(&pSetPoffTime->menu,eRES_IMAGE_TOP_SETUP);
#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pSetPoffTime->menu.listmenu_topbar, eRES_STR_AUTO_POWEROFF_TIME);
#endif

#else
    Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
    SetPoffTime_SetRefresh(SET_POFFTIME_REFRESH_ALL);
#endif
}

void exitset_poweroff_time(void)
{
    if (AK_NULL != pSetPoffTime)
    {
        Profile_WriteData(eCFG_SYSTEM, &pSetPoffTime->sysCfg);
#if (USE_COLOR_LCD)
        ListMenu_Free(&pSetPoffTime->menu);
#endif
        Progress_Free(&pSetPoffTime->prgress);
        pSetPoffTime = Fwl_Free(pSetPoffTime);
    }
}

void paintset_poweroff_time(void)
{
#if (!USE_COLOR_LCD)
    if (SET_POFFTIME_REFRESH_ALL == pSetPoffTime->refreshFlag)
    {
#endif
        if (0 == pSetPoffTime->curMMI)
        {
#if (!USE_COLOR_LCD)   
            if (SAVE_MODE == pSetPoffTime->curMode)
            {
                Fwl_FillRect(0, STR_MODE_TOP_POS, GRAPH_WIDTH, FONT_HEIGHT, CLR_BLACK);
                Eng_ImageResDisp((T_POS)IMG_MODE_LEFT_POS, (T_POS)IMG_MODE_TOP_POS, eRES_IMAGE_PWRSAVE, AK_FALSE);
                DispStringInWidth(CP_UNICODE, (T_POS)STR_MODE_LEFT_POS, (T_POS)STR_MODE_TOP_POS, CONVERT2STR(eRES_STR_OFF_TIME), FONT_WND_WIDTH, CLR_WHITE);
            }
            else
            {
                Eng_ImageResDisp((T_POS)IMG_MODE_LEFT_POS, (T_POS)IMG_MODE_TOP_POS, eRES_IMAGE_PWRSAVE, AK_FALSE);
                Fwl_FillRect((T_POS)(IMG_MODE_LEFT_POS+28), IMG_MODE_TOP_POS, 16, FONT_HEIGHT, CLR_BLACK);
                Eng_ImageResDisp((T_POS)(IMG_MODE_LEFT_POS+IMG_MODE_LEFT_OFFSET), (T_POS)IMG_MODE_TOP_POS, eRES_IMAGE_PWRSLEEP, AK_FALSE);
                DispStringInWidth(CP_UNICODE, (T_POS)STR_MODE_LEFT_POS, (T_POS)STR_MODE_TOP_POS, CONVERT2STR(eRES_STR_SLEEP_TIME), FONT_WND_WIDTH, CLR_WHITE);
            }
#else        
            ListMenu_Show(&pSetPoffTime->menu);
#endif
        }
        else if (1 == pSetPoffTime->curMMI)
        {
            Progress_Show(&pSetPoffTime->prgress);
        } 
#if (!USE_COLOR_LCD)
        SetPoffTime_SetRefresh(SET_POFFTIME_REFRESH_NONE);
    }
#endif
}

unsigned char handleset_poweroff_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;
    T_U16 value;
#if (!USE_COLOR_LCD)  
    T_PRESS_KEY phyKey;

    if(M_EVT_USER_KEY  == event)
    {   //get key value and key type
        phyKey.id = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;

        if((phyKey.id == kbMODE)&&(phyKey.pressType == PRESS_SHORT))
        {
            m_triggerEvent(M_EVT_EXIT, pEventParm);

                if (event >= M_EVT_Z00_POWEROFF)
                    return 1;
                else
                    return 0;
               }

    }
#endif

    if (0 == pSetPoffTime->curMMI)
    {
#if (USE_COLOR_LCD)    
        ret = ListMenu_Handler(&pSetPoffTime->menu, event, pEventParm);

        switch(ret)
        {
            case CTRL_RESPONSE_QUIT:
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
            case eRES_STR_OFF_TIME:
                pSetPoffTime->curMode = SAVE_MODE;
                pSetPoffTime->curMMI = 1;
                break;
            case eRES_STR_SLEEP_TIME:
                pSetPoffTime->curMode = SLEEP_MODE;
                pSetPoffTime->curMMI = 1;
                break;
       }
       if (1 == pSetPoffTime->curMMI)
       {
            if (pSetPoffTime->curMode == SLEEP_MODE)
            {   value = gb.PoffTimeSleepMode/60;
                if((value <=120))   
                {
                    #if  USE_COLOR_LCD
                    //Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE1, eRES_STR_PUB_DEL_HINT, 
                    //    INVALID_IMAGERES_ID, 12, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);

                    Progress_InitEx(&pSetPoffTime->prgress, 
                                    POWEROFF_TIME_MIN_VALUE, 
                                    POWEROFF_TIME_MAX_VALUE1, 
                                    eRES_STR_PUB_DEL_HINT, 
                                    INVALID_IMAGERES_ID, 
                                    12, 
                                    value, 
                                    AK_FALSE, 
                                    INVALID_IMAGERES_ID, 
                                    INVALID_IMAGERES_ID,
                                    SYS_POFF_SET_TITLE_LEFT,
                                    SYS_POFF_SET_TITLE_HIGHT,
                                    MAIN_LCD_WIDTH,
                                    MAIN_LCD_HEIGHT-SYS_POFF_SET_TITLE_HIGHT);

                    //Progress_SetTitle(&pSetPoffTime->prgress,GetUITitleResID());
                    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
                    TopBar_ConfigTitleSet(&pSetPoffTime->prgress.pro_topbar, eRES_STR_SLEEP_TIME);
                    #endif
                    #else
                    Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE1, eRES_STR_SLEEP_TIME, \
                            INVALID_IMAGERES_ID, 12, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);                
                    #endif
                }
                else
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);

            }else
            {
                value = pSetPoffTime->sysCfg.PoffTime;
                if((value<=60))
                {
                    #if  USE_COLOR_LCD
                    //Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE, eRES_STR_PUB_DEL_HINT, 
                    //    INVALID_IMAGERES_ID, 6, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);

                    Progress_InitEx(&pSetPoffTime->prgress, 
                                    POWEROFF_TIME_MIN_VALUE, 
                                    POWEROFF_TIME_MAX_VALUE, 
                                    eRES_STR_PUB_DEL_HINT, 
                                    INVALID_IMAGERES_ID, 
                                    6, 
                                    value, 
                                    AK_FALSE, 
                                    INVALID_IMAGERES_ID, 
                                    INVALID_IMAGERES_ID,
                                    SYS_POFF_SET_TITLE_LEFT,
                                    SYS_POFF_SET_TITLE_HIGHT,
                                    MAIN_LCD_WIDTH,
                                    MAIN_LCD_HEIGHT-SYS_POFF_SET_TITLE_HIGHT);

                    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
                    TopBar_ConfigTitleSet(&pSetPoffTime->prgress.pro_topbar, eRES_STR_OFF_TIME);
                    #endif
                    //Progress_SetTitle(&pSetPoffTime->prgress,GetUITitleResID());
                    #else
                    Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE, eRES_STR_OFF_TIME, \
                            INVALID_IMAGERES_ID, 6, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);                
                    #endif
                }
                else
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);
            }
        }
#else
        if (M_EVT_USER_KEY == event)
        {
            switch (pEventParm->c.Param1)
            {
                case kbLEFT:
                case kbRIGHT:
                    if (SAVE_MODE == pSetPoffTime->curMode)
                    {
                        pSetPoffTime->curMode = SLEEP_MODE;
                    }
                    else if (SLEEP_MODE == pSetPoffTime->curMode)
                    {
                        pSetPoffTime->curMode = SAVE_MODE;
                    }
                    break;
                case kbOK:
                    if (pEventParm->c.Param2 == PRESS_SHORT)
                    {
                        pSetPoffTime->curMMI = 1;
        
                        if (SLEEP_MODE == pSetPoffTime->curMode)
                        {
                            value = gb.PoffTimeSleepMode/60;
                            #if  USE_COLOR_LCD
                            //Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE1, INVALID_STRINGRES_ID, 
                            //              eRES_IMAGE_POWEROFF, 12, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);

                            Progress_InitEx(&pSetPoffTime->prgress, 
                                            POWEROFF_TIME_MIN_VALUE, 
                                            POWEROFF_TIME_MAX_VALUE1, 
                                            INVALID_STRINGRES_ID, 
                                            eRES_IMAGE_POWEROFF, 
                                            12, 
                                            value, 
                                            AK_FALSE, 
                                            INVALID_IMAGERES_ID, 
                                            INVALID_IMAGERES_ID,
                                            SYS_POFF_SET_TITLE_LEFT,
                                            SYS_POFF_SET_TITLE_HIGHT,
                                            MAIN_LCD_WIDTH,
                                            MAIN_LCD_HEIGHT-SYS_POFF_SET_TITLE_HIGHT);

                            //Progress_SetTitle(&pSetPoffTime->prgress,GetUITitleResID());
                            #else
                            Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE1, INVALID_STRINGRES_ID, \
                                            eRES_IMAGE_POWEROFF, 12, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);                            
                            #endif
                        }else
                        {
                            value = pSetPoffTime->sysCfg.PoffTime;
                            #if  USE_COLOR_LCD
                            //Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE, INVALID_STRINGRES_ID, 
                            //          eRES_IMAGE_POWEROFF, 6, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);

                            Progress_InitEx(&pSetPoffTime->prgress, 
                                            POWEROFF_TIME_MIN_VALUE, 
                                            POWEROFF_TIME_MAX_VALUE, 
                                            INVALID_STRINGRES_ID, 
                                            eRES_IMAGE_POWEROFF, 
                                            6, 
                                            value, 
                                            AK_FALSE, 
                                            INVALID_IMAGERES_ID, 
                                            INVALID_IMAGERES_ID,
                                            SYS_POFF_SET_TITLE_LEFT,
                                            SYS_POFF_SET_TITLE_HIGHT,
                                            MAIN_LCD_WIDTH,
                                            MAIN_LCD_HEIGHT-SYS_POFF_SET_TITLE_HIGHT);

                            //Progress_SetTitle(&pSetPoffTime->prgress,GetUITitleResID());
                            #else
                            Progress_Init(&pSetPoffTime->prgress, POWEROFF_TIME_MIN_VALUE, POWEROFF_TIME_MAX_VALUE, INVALID_STRINGRES_ID, \
                                            eRES_IMAGE_POWEROFF, 6, value, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);
                            #endif
                        }

                    }
                    else
                    {
                        m_triggerEvent(M_EVT_EXIT, AK_NULL);
                    }
                    break;
                default:
                    break;
            }
            SetPoffTime_SetRefresh(SET_POFFTIME_REFRESH_ALL);
        }
#endif
    }

    else if (1 == pSetPoffTime->curMMI)
    {        
        ret = Progress_Handler(&pSetPoffTime->prgress, event, pEventParm);

        switch(ret)
        {
            case CTRL_RESPONSE_QUIT:
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
            case CTRL_RESPONSE_NONE:
                break;
            case CTRL_PROGRESS_RESPONSE_CHANGED:
                value = 0xffff;
                value = (T_U16)Progress_GetPos(&pSetPoffTime->prgress);

                if (SAVE_MODE == pSetPoffTime->curMode)
                {
                    if((value <= 60)
                        &&pSetPoffTime->sysCfg.PoffTime != value)
                        pSetPoffTime->sysCfg.PoffTime = value;

                    gb.PoffTime = pSetPoffTime->sysCfg.PoffTime;
                    Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetPoffTime->sysCfg);
                }
                else if (SLEEP_MODE == pSetPoffTime->curMode)
                {
                    if((value <= 120)
                        &&gb.PoffTimeSleepMode != (value*60))
                        pSetPoffTime->sysCfg.PoffTimeSleepMode = value*60;

                    gb.PoffTimeSleepMode= pSetPoffTime->sysCfg.PoffTimeSleepMode;
                        //Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetPoffTime->sysCfg);
                }

                AutoPowerOffCountSet(gb.PoffTime);
                AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);

                break;
            default:
  /*              value = 0xffff;
                value = (T_U16)Progress_GetPos(&pSetPoffTime->prgress);

                if (SAVE_MODE == pSetPoffTime->curMode)
                {
                    if((0<=value)&&(value <= 60)
                        &&pSetPoffTime->sysCfg.PoffTime != value)
                        pSetPoffTime->sysCfg.PoffTime = value;

                    gb.PoffTime = pSetPoffTime->sysCfg.PoffTime;
                    Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetPoffTime->sysCfg);
                }
                else if (SLEEP_MODE == pSetPoffTime->curMode)
                {
                    if((0<=value)&&(value <= 120)
                        &&gb.PoffTimeSleepMode != (value*60))
                        gb.PoffTimeSleepMode = value*60;
                        //Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetPoffTime->sysCfg);
                }

                AutoPowerOffCountSet(gb.PoffTime);
                AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);
*/
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
        }
#if (!USE_COLOR_LCD)    
        SetPoffTime_SetRefresh(SET_POFFTIME_REFRESH_ALL);
#endif
    }
    
    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_poweroff_time(void)
{
}
void paintset_poweroff_time(void)
{
}
void exitset_poweroff_time(void)
{
}
unsigned char handleset_poweroff_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_poweroff_time\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

