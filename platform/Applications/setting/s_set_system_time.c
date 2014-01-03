/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_system_time.c
 * @BRIEF set system time
 * @Author：Huang_ChuSheng
 * @Date：2008-04-25
 * @Version：
**************************************************************************/

//#include "m_state.h"
#include "eng_time.h"
#include "Fwl_Timer.h"
#include "eng_graph.h"
#include "eng_dataconvert.h"
#include "eng_font.h"
#include "eng_string.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Keypad.h"
#include "Eng_ImageResDisp.h"
#include "string.h"
#include "Apl_public.h"
#include "Fwl_FreqMgr.h"
#include "Eng_Profile.h"
#include "AlarmClock.h"
#include "Ctrl_Dialog.h"
#include "Alarm_Common.h"
#include "Fwl_RTC.h"
#include "M_event.h"
#include "M_event_api.h"

#if(NO_DISPLAY == 0)

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif

#define BUFFER_LEN                  256
//radio ui display flag
#define  SST_REFRESH_NONE           0X00000000
#define  SST_REFRESH_YEAR           0x00000001
#define  SST_REFRESH_MON            0x00000002      
#define  SST_REFRESH_DAY            0x00000004
#define  SST_REFRESH_HOUR           0x00000008
#define  SST_REFRESH_MINUTE         0x00000010
#define  SST_REFRESH_SECOND         0x00000020
#define  SST_REFRESH_OTHER          0x00000040
#define  SST_REFRESH_TITLE          0X00000080
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))    
#define  SST_REFRESH_TOPBAR         0x00001000      //TopBar_show(&pSetSysTime->systime_topbar);
#endif

#define  SST_REFRESH_ALL            0xFFFFFFFF

#define SST_TIME_BGD        RGB_COLOR(90,154,239)

#if (USE_COLOR_LCD)
#if (LCD_TYPE == 3)
#if (LCD_HORIZONTAL == 1)
    #define SST_ICON_SUB_TOP        (91)
    #define SST_ICON_ADD_TOP        (SST_ICON_SUB_TOP)
    #define SST_ICON_TIME_YMD_TOP   (60)                //year ,month ,day---60
    #define SST_ICON_TIME_HMS_TOP   (121)               //hour ,minute ,second
    #define SST_ICON_TIME_YEAR_LEFT (63)
    #define SST_ICON_TIME_MON_LEFT  (130)
    #define SST_ICON_TIME_DAY_LEFT  (192)
    #define SST_ICON_TIME_HOUR_LEFT (SST_ICON_TIME_YEAR_LEFT)
    #define SST_ICON_TIME_MIN_LEFT  (SST_ICON_TIME_MON_LEFT)
    #define SST_ICON_TIME_SEC_LEFT  (SST_ICON_TIME_DAY_LEFT)
    #define SST_ICON_SURE_TOP       (197)
    #define SST_ICON_EXIT_TOP       (SST_ICON_SURE_TOP)
    #define SST_ICON_SUB_LEFT       (16)
    #define SST_ICON_ADD_LEFT       (273)
    #define SST_ICON_SURE_LEFT      (61)
    #define SST_ICON_EXIT_LEFT      (196)
    #define SST_ICON_TIME_DAY_WIDTH (69)
    #define SST_ICON_TIME_DAY_HEIGHT    (30)
    #define SST_ICON_TIME_SEC_WIDTH     SST_ICON_TIME_DAY_WIDTH
    #define SST_ICON_TIME_SEC_HEIGHT    SST_ICON_TIME_DAY_HEIGHT

    #define SST_STR_BACK_LEFT   (29)
    #define SST_STR_BACK_TOP    (16)

    #define SST_TIME_ICOM_LEFT
        //title string position
    #define SST_STR_TITLE_LEFT  (80)
    #define SST_STR_TITLE_TOP   (24)

    #define SST_STR_YEAR_LEFT   (78)
    #define SST_STR_YEAR_TOP    (68)
    #define SST_BCK_YEAR_LEFT   (78)

    #define SST_STR_MON_LEFT    (146)
    #define SST_STR_MON_TOP     (68)
    #define SST_BCK_MON_LEFT    (146)

    #define SST_STR_DAY_LEFT    (208)
    #define SST_STR_DAY_TOP     (68)
    #define SST_BCK_DAY_LEFT    (208)

    #define SST_STR_HOUR_LEFT   (SST_STR_YEAR_LEFT+5)
    #define SST_STR_HOUR_TOP    (129)
    #define SST_BCK_HOUR_LEFT   (SST_STR_YEAR_LEFT+5)

    #define SST_STR_MIN_LEFT    (SST_STR_MON_LEFT)
    #define SST_STR_MIN_TOP     (SST_STR_HOUR_TOP)
    #define SST_BCK_MIN_LEFT    (SST_BCK_MON_LEFT)

    #define SST_STR_SEC_LEFT    (SST_STR_DAY_LEFT)
    #define SST_STR_SEC_TOP     (SST_STR_HOUR_TOP)
    #define SST_BCK_SEC_LEFT    (SST_BCK_DAY_LEFT)
#else
    #define SST_STR_BACK_LEFT   (29)
    #define SST_STR_BACK_TOP    (16)

    #define SST_TIME_ICOM_LEFT
        //title string position
    #define SST_STR_TITLE_LEFT  (80)
    #define SST_STR_TITLE_TOP   (34)

    #define SST_STR_YEAR_LEFT   (30)
    #define SST_STR_YEAR_TOP    (112)
    #define SST_BCK_YEAR_LEFT   (30)

    #define SST_STR_MON_LEFT    (110)
    #define SST_STR_MON_TOP     (112)
    #define SST_BCK_MON_LEFT    (110)

    #define SST_STR_DAY_LEFT    (180)
    #define SST_STR_DAY_TOP     (112)
    #define SST_BCK_DAY_LEFT    (180)

    #define SST_STR_HOUR_LEFT   (30)
    #define SST_STR_HOUR_TOP    (218)
    #define SST_BCK_HOUR_LEFT   (30)

    #define SST_STR_MIN_LEFT    (110)
    #define SST_STR_MIN_TOP     (218)
    #define SST_BCK_MIN_LEFT    (110)

    #define SST_STR_SEC_LEFT    (180)
    #define SST_STR_SEC_TOP     (218)
    #define SST_BCK_SEC_LEFT    (180)
 #endif
#else

    #define SST_STR_BACK_LEFT   (29)
    #define SST_STR_BACK_TOP    (16)

    #define SST_TIME_ICOM_LEFT
    #define SST_STR_TITLE_LEFT  (36)
    #define SST_STR_TITLE_TOP   (12)

    #define SST_STR_YEAR_LEFT   (12)
    #define SST_STR_YEAR_TOP    (51)
    #define SST_BCK_YEAR_LEFT   (13)

    #define SST_STR_MON_LEFT    (59)
    #define SST_STR_MON_TOP     (51)
    #define SST_BCK_MON_LEFT    (57)

    #define SST_STR_DAY_LEFT    (97)
    #define SST_STR_DAY_TOP     (51)
    #define SST_BCK_DAY_LEFT    (95)

    #define SST_STR_HOUR_LEFT   (19)
    #define SST_STR_HOUR_TOP    (104)
    #define SST_BCK_HOUR_LEFT   (18)

    #define SST_STR_MIN_LEFT    (57)
    #define SST_STR_MIN_TOP     (104)
    #define SST_BCK_MIN_LEFT    (56)

    #define SST_STR_SEC_LEFT    (96)
    #define SST_STR_SEC_TOP     (104)
    #define SST_BCK_SEC_LEFT    (94)
#endif
#else
    //title string position
    #define SST_STR_BACK_LEFT   (29)
    #define SST_STR_BACK_TOP    (20)
        
    #define SST_STR_TITLE_LEFT  (36)
    #define SST_STR_TITLE_TOP   (12)

    #define SST_STR_YEAR_LEFT   (SST_STR_BACK_LEFT+3)
    #define SST_STR_YEAR_TOP    (SST_STR_BACK_TOP+6)

    #define SST_STR_MON_LEFT    (SST_STR_BACK_LEFT+27)
    #define SST_STR_MON_TOP     (SST_STR_BACK_TOP+6)

    #define SST_STR_DAY_LEFT    (SST_STR_BACK_LEFT+47)
    #define SST_STR_DAY_TOP     (SST_STR_BACK_TOP+6)

    #define SST_STR_HOUR_LEFT   (SST_STR_BACK_LEFT+7)
    #define SST_STR_HOUR_TOP    (SST_STR_BACK_TOP+16)

    #define SST_STR_MIN_LEFT    (SST_STR_BACK_LEFT+21)
    #define SST_STR_MIN_TOP     (SST_STR_BACK_TOP+16)

    #define SST_STR_SEC_LEFT    (SST_STR_BACK_LEFT+41)
    #define SST_STR_SEC_TOP     (SST_STR_BACK_TOP+16)


    #define STR_ICON_LEFT_POS 8
    #define STR_ICON_TOP_POS  0
    #define STR_BK_LEFT_POS   29
    #define STR_BK_TOP_POS    16
    #define STR_DATE_LEFT_POS (STR_BK_LEFT_POS + 4)
    #define STR_DATE_TOP_POS  (STR_BK_TOP_POS + 8)
    #define STR_TIME_LEFT_POS STR_DATE_LEFT_POS
    #define STR_TIME_TOP_POS  (STR_BK_TOP_POS + 16)
#endif

#define MIN_YEAR          2000
#define MAX_YEAR          2098

typedef enum
{
   SET_YEAR  = 0,
   SET_MONTH,
   SET_DAY,
   SET_HOUR,
   SET_MINUTE,
   SET_SECOND,
   SET_NULL
}T_SET_WHAT;

typedef enum 
{
    SYSTIME_SET,
    CLOCKTIME_SET
}T_eFunction;

typedef struct _SET_SYSEM_TIME
{
    T_SYSTIME  date;
    T_U32 tickCount;
    T_U32 TickMax;
    T_U32 TickMin;
    T_U8 Buf[11];
    T_SET_WHAT setWhat;
    T_U32      timer;
    T_U32      refresh;
    T_BOOL     bEdit;
    T_BOOL     bShowSpace;
    T_eFunction function;
#if(USE_ALARM_CLOCK)
    T_AlarmClockParam *pClockParam;
    T_BOOL     isShowDialog;
    T_BOOL     isDialogInit;
    CDialogCtrl dialog;
    T_SYSTIME clockTimeSet;
    T_RES_STRING resStringId;
#endif
    T_BOOL flashFlag;
    T_EVT_PARAM *pEvtParam;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL systime_topbar;
#endif
}T_SET_SYSTEM_TIME;

static T_SET_SYSTEM_TIME *pSetSysTime = AK_NULL;

static  T_VOID  SysSetTimePaint(T_VOID);
static T_VOID   ST_Setrefresh(T_U32 flag);

void initset_system_time(void)
{
    //Freq_StackPush(FREQ_APP_MAX);

    pSetSysTime = (T_SET_SYSTEM_TIME*)Fwl_Malloc(sizeof(T_SET_SYSTEM_TIME));
    AK_ASSERT_PTR_VOID(pSetSysTime, "malloc memory error in initset_system_time\n")
   
#if(USE_ALARM_CLOCK)
    pSetSysTime->pClockParam= AK_NULL;
     pSetSysTime->isDialogInit= AK_FALSE;
    pSetSysTime->isShowDialog= AK_FALSE;
#endif
    //initialize data struct , current time, set pointer, and refresh flag;
    pSetSysTime->setWhat = SET_NULL;
    pSetSysTime->bEdit = AK_FALSE;
    pSetSysTime->bShowSpace = AK_FALSE;
    pSetSysTime->timer    = ERROR_TIMER;
#ifdef OS_WIN32
    pSetSysTime->tickCount = Fwl_GetSecond();
    Utl_Convert_SecondToDate(pSetSysTime->tickCount, &pSetSysTime->date);
#else
    //获取系统当前时间，并进行合法性判断
    Fwl_GetRTCtime(&pSetSysTime->date);
#endif
    pSetSysTime->refresh = -1;
    pSetSysTime->flashFlag= AK_FALSE;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    TopBar_init(&pSetSysTime->systime_topbar, eRES_STR_INVALID0, AK_TRUE, AK_TRUE);
    ST_Setrefresh(SST_REFRESH_TITLE);
//  pSetSysTime->refresh & SST_REFRESH_TITLE
#endif

}

void exitset_system_time(void)
{
    GblSaveSystime();
    if(ERROR_TIMER != pSetSysTime->timer)
    {
        Fwl_TimerStop(pSetSysTime->timer);
        pSetSysTime->timer = ERROR_TIMER; 
    }
    
#if(USE_ALARM_CLOCK)
    if(pSetSysTime->isDialogInit)
    {
        pSetSysTime->isDialogInit= AK_FALSE;
        Dialog_Free(&pSetSysTime->dialog);
    }
#endif
    pSetSysTime = Fwl_Free(pSetSysTime);
    //Freq_StackPop();
}

void paintset_system_time(void)
{  
#if(USE_ALARM_CLOCK)
    if(pSetSysTime->isShowDialog)
    {
        Dialog_Show(&pSetSysTime->dialog);
#if(USE_COLOR_LCD)
        if(pSetSysTime->flashFlag)
        {
            pSetSysTime->flashFlag= AK_FALSE;
            #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            DISPLAY_TITLE(IMAGE_ALARM_SET);
            #endif
        }
#endif
    }
    else
#endif
    {
        SysSetTimePaint();
    }
}

#if(USE_ALARM_CLOCK)
static T_VOID ClkSetTime_Trigger(T_VOID)
{
       pSetSysTime->pEvtParam->p.pParam1= pSetSysTime->pClockParam;
       if(WEEKDAYS == pSetSysTime->pClockParam->clockSet.responseType)
       {
           m_triggerEvent(M_EVT_WEEKDAY, pSetSysTime->pEvtParam);
       }
       else
       {
           if(POWEROFF_CLOCK == pSetSysTime->pClockParam->clockSet.clockType)
           {
               m_triggerEvent(M_EVT_EXIT, AK_NULL);
           }
           else
           {
                m_triggerEvent(M_EVT_NEXT, pSetSysTime->pEvtParam);
           }
       }
}

static T_VOID DealSaveClockTime(T_VOID)
{
    T_S8 state;
    T_U16 hintString[BUFFER_LEN];
    T_SYSTIME tempTime;
    pSetSysTime->isShowDialog= AK_TRUE;
    tempTime= pSetSysTime->pClockParam->clockSet.clockTime;
    pSetSysTime->pClockParam->clockSet.clockTime= pSetSysTime->date;
    pSetSysTime->clockTimeSet= pSetSysTime->date;
    if(pSetSysTime->pClockParam->index == ERROR_INDEX)
    {
        AlmClk_SetDefaultClockSet(&pSetSysTime->pClockParam->clockSet);
        state= AlmClk_Validate(&pSetSysTime->pClockParam->clockSet, ERROR_INDEX);

        if(state>= MAX_CLK_COUNT)// if full
        {
            pSetSysTime->resStringId= eRES_STR_ALARM_FULLHIT;
            Gui_GetResString(hintString, eRES_STR_ALARM_FULLHIT, BUFFER_LEN);
           pSetSysTime->isDialogInit= Gui_InitDialog(&pSetSysTime->dialog, hintString, eRES_STR_ALARM_SETTING,
                CTRL_DIALOG_RESPONSE_HIDDEN, CTRL_DIALOG_RESPONSE_YES);
            
        }else if(state < 0)// is conflictive
        {
            pSetSysTime->resStringId= eRES_STR_ALARM_CONFLICTHINT;
           Gui_GetResString(hintString, eRES_STR_ALARM_CONFLICTHINT, BUFFER_LEN);
            pSetSysTime->isDialogInit= Gui_InitDialog(&pSetSysTime->dialog, hintString, eRES_STR_ALARM_SETTING,
                CTRL_DIALOG_RESPONSE_HIDDEN, CTRL_DIALOG_RESPONSE_YES);
        }
        else
        {
            
            pSetSysTime->resStringId=  eRES_STR_ALARM_ADDHINT;
            Gui_GetResString(hintString, eRES_STR_ALARM_ADDHINT, BUFFER_LEN);
            pSetSysTime->isDialogInit= Gui_InitDialog(&pSetSysTime->dialog, hintString, eRES_STR_ALARM_SETTING,
                CTRL_DIALOG_RESPONSE_YES | CTRL_DIALOG_RESPONSE_NO, CTRL_DIALOG_RESPONSE_YES);            
        }
 
    }
    else
    {
        state= AlmClk_Validate(&pSetSysTime->pClockParam->clockSet, pSetSysTime->pClockParam->index);
        if(state < 0) // is conflictive
        {
            pSetSysTime->resStringId= eRES_STR_ALARM_CONFLICTHINT;
            Gui_GetResString(hintString, eRES_STR_ALARM_CONFLICTHINT, BUFFER_LEN);
            pSetSysTime->isDialogInit= Gui_InitDialog(&pSetSysTime->dialog, hintString, eRES_STR_ALARM_SETTING,
                CTRL_DIALOG_RESPONSE_HIDDEN, CTRL_DIALOG_RESPONSE_YES);
        }
        else
        { 
            if(IS_SAME_TIME(tempTime, pSetSysTime->date) && ERROR_INDEX != pSetSysTime->pClockParam->index)
            {
                pSetSysTime->isShowDialog= AK_FALSE;
                ClkSetTime_Trigger();
            }
            else
            {
                pSetSysTime->resStringId= eRES_STR_ALARM_UPDATEHINT;
                Gui_GetResString(hintString, eRES_STR_ALARM_UPDATEHINT, BUFFER_LEN);
                pSetSysTime->isDialogInit= Gui_InitDialog(&pSetSysTime->dialog, hintString, eRES_STR_ALARM_SETTING,
                    CTRL_DIALOG_RESPONSE_YES | CTRL_DIALOG_RESPONSE_NO, CTRL_DIALOG_RESPONSE_YES);   
            }
        }
    }

    if(pSetSysTime->isShowDialog)
    {
        pSetSysTime->pClockParam->clockSet.clockTime= tempTime;//set back
        pSetSysTime->flashFlag= AK_TRUE;
    }
    

}
#endif

static T_VOID Systime_RefreshFlash(T_VOID)
{
    #if (!USE_COLOR_LCD)
    if(!pSetSysTime->bEdit || CLOCKTIME_SET == pSetSysTime->function)
    {
        if(ERROR_TIMER != pSetSysTime->timer)
        {
            Fwl_TimerStop(pSetSysTime->timer);
            pSetSysTime->timer = ERROR_TIMER;
        }
        pSetSysTime->timer = Fwl_TimerStartMilliSecond(600, AK_TRUE);
        pSetSysTime->bShowSpace = AK_FALSE;
    }
    #endif
    ST_Setrefresh(1<<pSetSysTime->setWhat);
}


//T_TIMER Fwl_TimerStart(T_U16 milliSeconds, T_BOOL loop, T_fVTIMER_CALLBACK callback) 
//T_VOID touchevent_vtimer_callback_func(T_TIMER timer_id, T_U32 delay)



unsigned char handleset_system_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad;
    T_U8     days;
    const T_U8 month_days[]={31,28,31,30,31,30,31,31,30,31,30,31 };

    #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))    
    TopBar_Handle(&pSetSysTime->systime_topbar, event, pEventParm);
    #endif
     switch(event)
        {
        case M_EVT_7:
            pSetSysTime->function= SYSTIME_SET;
            break;
#if(USE_ALARM_CLOCK)    
        case M_EVT_CLK_TIME:
            AK_ASSERT_PTR(pEventParm, "handleset_system_time1", 0);
            pSetSysTime->pEvtParam= pEventParm;
            AK_ASSERT_PTR(pEventParm->p.pParam1, "handleset_system_time2", 0);
            pSetSysTime->pClockParam= pEventParm->p.pParam1;
            pSetSysTime->function= CLOCKTIME_SET;
            pSetSysTime->setWhat= SET_HOUR;
            if(pSetSysTime->pClockParam->index != ERROR_INDEX)
            {
                if(ONLYONCE == pSetSysTime->pClockParam->clockSet.clockType)
                {
                    pSetSysTime->date.year= pSetSysTime->pClockParam->clockSet.clockTime.year;
                    pSetSysTime->date.month= pSetSysTime->pClockParam->clockSet.clockTime.month;
                    pSetSysTime->date.day= pSetSysTime->pClockParam->clockSet.clockTime.day;
                }
                pSetSysTime->date.hour= pSetSysTime->pClockParam->clockSet.clockTime.hour;
                pSetSysTime->date.minute= pSetSysTime->pClockParam->clockSet.clockTime.minute;
            }
            pSetSysTime->bEdit= AK_TRUE;
            Systime_RefreshFlash();
            break;
#endif
        case M_EVT_RETURN:
            if(CLOCKTIME_SET == pSetSysTime->function)// return 1 class
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
            }
            else
            {
                m_triggerEvent(M_EVT_RETURN2, pEventParm);
            }
            break;
        }
#if(USE_ALARM_CLOCK)
    if(pSetSysTime->isShowDialog)
    {
        T_U16 ret= Dialog_Handler(&pSetSysTime->dialog, event, pEventParm);
        switch(ret)
        {
        case CTRL_DIALOG_RESPONSE_YES:
            #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
            TopBar_ConfigbShowSet(&pSetSysTime->systime_topbar, AK_TRUE);
            #endif
            switch(pSetSysTime->resStringId)
            {
            case eRES_STR_ALARM_ADDHINT:
                pSetSysTime->pClockParam->clockSet.clockTime= pSetSysTime->clockTimeSet;
                pSetSysTime->pClockParam->index= AlmClk_Add(&pSetSysTime->pClockParam->clockSet);
                ClkSetTime_Trigger();
                break;
            case eRES_STR_ALARM_UPDATEHINT:                
                 pSetSysTime->pClockParam->clockSet.clockTime= pSetSysTime->clockTimeSet;
                 AlmClk_Update(pSetSysTime->pClockParam->index, &pSetSysTime->pClockParam->clockSet);
                 ClkSetTime_Trigger();                
                break;
            default:
                break;
            } // no break;
       
        case CTRL_RESPONSE_QUIT:
        case CTRL_DIALOG_RESPONSE_NO:
            #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
            TopBar_ConfigbShowSet(&pSetSysTime->systime_topbar, AK_TRUE);
            #endif
            pSetSysTime->isShowDialog= AK_FALSE;
            pSetSysTime->isDialogInit= AK_FALSE;
            Dialog_Free(&pSetSysTime->dialog);
            ST_Setrefresh(SST_REFRESH_ALL);
            break;
            break;
        }
    }
    else
#endif
    {
        switch(event)
        {
        case M_EVT_USER_KEY:

            keyPad.id = (T_eKEY_ID)pEventParm->c.Param1;
            keyPad.pressType = (T_ePRESS_TYPE)pEventParm->c.Param2;

            switch (keyPad.id)
            {
                case kbOK:
                    if (PRESS_SHORT == keyPad.pressType)
                    {
                        //if (pSetSysTime->bEdit)// whether is edited , when press ok, just return
                        {
                            if(SYSTIME_SET == pSetSysTime->function)
                            {
                                //pSetSysTime->tickCount = Utl_Convert_DateToSecond(&pSetSysTime->date);
                                //Fwl_SetRTCCnt(pSetSysTime->tickCount); 
                                if (pSetSysTime->date.year <= MAX_YEAR)
                                {
                                    Fwl_SetRTCtime(&pSetSysTime->date);
                                    GblSaveSystime();
                                    #if(USE_ALARM_CLOCK)
                                    AlmClk_Init();// re init alarm clock
                                    #endif
                                }
                                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                            }
                        }                   

                    }
                    
                    if(CLOCKTIME_SET == pSetSysTime->function)
                    {
#if(USE_ALARM_CLOCK)
                        DealSaveClockTime();
                        #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
                        TopBar_ConfigbShowSet(&pSetSysTime->systime_topbar, AK_FALSE);
                        #endif
#endif
                    }
                    break;

                case kbVOLADD:
                case kbVOLUME:
                    ST_Setrefresh(1<<pSetSysTime->setWhat);
                    if(CLOCKTIME_SET == pSetSysTime->function)
                    {
                        if(pSetSysTime->setWhat >= SET_MINUTE)
                            pSetSysTime->setWhat= SET_HOUR;
                        else
                            pSetSysTime->setWhat++;
                    }
                    else
                    {
                        if (pSetSysTime->setWhat >= SET_NULL)
                            pSetSysTime->setWhat = SET_YEAR;
                        else
                            pSetSysTime->setWhat++;
                    }
                    #if (!USE_COLOR_LCD)
                    if(!pSetSysTime->bEdit || CLOCKTIME_SET == pSetSysTime->function)
                    {
                        if(ERROR_TIMER != pSetSysTime->timer)
                        {
                            Fwl_TimerStop(pSetSysTime->timer);
                            pSetSysTime->timer = ERROR_TIMER;
                        }
                        pSetSysTime->timer = Fwl_TimerStartMilliSecond(600, AK_TRUE);
                        pSetSysTime->bShowSpace = AK_FALSE;
                    }
                    #endif
                    ST_Setrefresh(1<<pSetSysTime->setWhat);
                    if(pSetSysTime->bEdit != AK_TRUE)
                        pSetSysTime->bEdit = AK_TRUE;                
                    break;

                case kbVOLSUB:
                    ST_Setrefresh(1<<pSetSysTime->setWhat);

                    if(CLOCKTIME_SET == pSetSysTime->function)
                    {
                        if(SET_HOUR >= pSetSysTime->setWhat)
                            pSetSysTime->setWhat= SET_MINUTE;
                        else
                            pSetSysTime->setWhat--;
                    }
                    else
                    {
                        if (pSetSysTime->setWhat == SET_YEAR)
                            pSetSysTime->setWhat = SET_NULL;
                        else
                            pSetSysTime->setWhat--;     
                    }
                
                    #if (!USE_COLOR_LCD)
                    if(!pSetSysTime->bEdit || CLOCKTIME_SET == pSetSysTime->function)
                    {
                        if(ERROR_TIMER != pSetSysTime->timer)
                        {
                            Fwl_TimerStop(pSetSysTime->timer);
                            pSetSysTime->timer = ERROR_TIMER;
                        }
                        pSetSysTime->timer = Fwl_TimerStartMilliSecond(600, AK_TRUE);
                        pSetSysTime->bShowSpace = AK_FALSE;
                    }
                    #endif
                    ST_Setrefresh(1<<pSetSysTime->setWhat);
                    if(pSetSysTime->bEdit != AK_TRUE)
                        pSetSysTime->bEdit = AK_TRUE;                
                    break;

                case kbLEFT:
                    if (pSetSysTime->bEdit)
                    {
                        switch (pSetSysTime->setWhat)
                        {
                            case SET_YEAR:
                                if (pSetSysTime->date.year > MIN_YEAR)
                                    pSetSysTime->date.year--;
                                else
                                    pSetSysTime->date.year = MAX_YEAR;
                                ST_Setrefresh(SST_REFRESH_YEAR);
                                break;
                                
                            case SET_MONTH:
                                if (pSetSysTime->date.month > 1)
                                    pSetSysTime->date.month--;
                                else
                                    pSetSysTime->date.month = 12;
                                ST_Setrefresh(SST_REFRESH_MON);
                                break;
                                
                            case SET_DAY:
                                days = month_days[pSetSysTime->date.month-1];    //days this month
                                if ((pSetSysTime->date.year % 4 == 0) && (pSetSysTime->date.month == 2))
                                    days++;    //leapyear + 1 
                            
                                if (pSetSysTime->date.day > 1)
                                    pSetSysTime->date.day--;
                                else
                                    pSetSysTime->date.day = days;
                                ST_Setrefresh(SST_REFRESH_DAY);
                                break;
                                
                            case SET_HOUR:
                                if (pSetSysTime->date.hour > 0)
                                    pSetSysTime->date.hour--;
                                else
                                    pSetSysTime->date.hour = 23;
                                ST_Setrefresh(SST_REFRESH_HOUR);
                                break;
                                
                            case SET_MINUTE:
                                if (pSetSysTime->date.minute > 0)
                                    pSetSysTime->date.minute--;
                                else
                                    pSetSysTime->date.minute = 59;
                                ST_Setrefresh(SST_REFRESH_MINUTE);
                                break;
                                
                            case SET_SECOND:
                                if (pSetSysTime->date.second > 0)
                                    pSetSysTime->date.second--;
                                else
                                    pSetSysTime->date.second = 59;
                                ST_Setrefresh(SST_REFRESH_SECOND);

                                break;
                                
                            default:
                                break;
                        }
                    }
                    break;
                case kbRIGHT:
                    if (pSetSysTime->bEdit)
                    {
                        switch (pSetSysTime->setWhat)
                        {
                            case SET_YEAR:
                                if (pSetSysTime->date.year < MAX_YEAR)
                                    pSetSysTime->date.year++;
                                else
                                    pSetSysTime->date.year = MIN_YEAR;
                                ST_Setrefresh(SST_REFRESH_YEAR);
                                break;
                                
                            case SET_MONTH:
                                if (pSetSysTime->date.month < 12)
                                    pSetSysTime->date.month++;
                                else
                                    pSetSysTime->date.month = 1;
                                ST_Setrefresh(SST_REFRESH_MON);
                                break;
                                
                            case SET_DAY:
                                days = month_days[pSetSysTime->date.month-1];    //days this month
                                if ((pSetSysTime->date.year % 4 == 0) && (pSetSysTime->date.month == 2))
                                    days++;    //leapyear + 1 

                                if (pSetSysTime->date.day < days)
                                    pSetSysTime->date.day++;
                                else
                                    pSetSysTime->date.day = 1;
                                ST_Setrefresh(SST_REFRESH_DAY);
                                break;
                            case SET_HOUR:
                                if (pSetSysTime->date.hour < 23)
                                    pSetSysTime->date.hour++;
                                else
                                    pSetSysTime->date.hour = 0;
                                ST_Setrefresh(SST_REFRESH_HOUR);
                                break;
                                
                            case SET_MINUTE:
                                if (pSetSysTime->date.minute < 59)
                                    pSetSysTime->date.minute++;
                                else
                                    pSetSysTime->date.minute = 0;
                                ST_Setrefresh(SST_REFRESH_MINUTE);
                                break;
                                
                            case SET_SECOND:
                                if (pSetSysTime->date.second < 59)
                                    pSetSysTime->date.second++;
                                else
                                    pSetSysTime->date.second = 0;
                                ST_Setrefresh(SST_REFRESH_SECOND);
                                break;
                                
                            default:
                            break;
                        }
                    }
                    break;

            case kbMODE:
                if(PRESS_SHORT == keyPad.pressType)
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
            
                default:
                break;
            }
            break;
        case M_EVT_PUB_TIMER: 
            if(!pSetSysTime->bEdit)
            {    T_SYSTIME  date;

                Fwl_GetRTCtime(&date);
                ST_Setrefresh(SST_REFRESH_SECOND);
                if(pSetSysTime->date.minute != date.minute)
                    ST_Setrefresh(SST_REFRESH_MINUTE);  

                if(pSetSysTime->date.hour != date.hour)
                    ST_Setrefresh(SST_REFRESH_HOUR);    

                if(pSetSysTime->date.day != date.day)
                    ST_Setrefresh(SST_REFRESH_DAY);

                if(pSetSysTime->date.month != date.month)
                    ST_Setrefresh(SST_REFRESH_MON);    
        
                if(pSetSysTime->date.year != date.year)
                    ST_Setrefresh(SST_REFRESH_YEAR);

            }           
            break;
        case VME_EVT_TIMER:
            if(pEventParm->w.Param1 == pSetSysTime->timer)
            {
                pSetSysTime->bShowSpace = !pSetSysTime->bShowSpace;
                ST_Setrefresh(1<<pSetSysTime->setWhat);
            }
            break;

        default:
            break;
        }
    }

    if (event >= M_EVT_Z00_POWEROFF)
    {
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        return 1;
    }
    else
        return 0;
}


T_VOID  MonitorSysTime(T_VOID)
{
    T_SYSTIME  date;
    T_U32       ticks;

    Fwl_FreqPush(FREQ_APP_MAX);    
    Fwl_GetRTCtime(&date);
    ticks = Utl_Convert_DateToSecond(&date);

    if(ticks > gb.TickMax)
    {   //设置当前时间为2000.01.01.00.00.00
        SetMinDate(date);
        Fwl_SetRTCtime(&date);
    }

    if(ticks < gb.TickMin)
    {   //设置当前时间为2098.12.31.23.59.59
        SetMaxDate(date);
        Fwl_SetRTCtime(&date);
    }  
    Fwl_FreqPop();
}

/***************************************************
**brief:
**author: Yao Hongshi
**date:     2008-06-26
***************************************************/
static  T_VOID  SysSetTimePaint(T_VOID)
{
    T_U8    hintMsg[16] ={0,0};
    #if (!USE_COLOR_LCD)
    T_U32   i;
    #endif  

    if (!pSetSysTime->bEdit)
    {
#ifdef OS_WIN32
        pSetSysTime->tickCount = Fwl_GetSecond();
        Utl_Convert_SecondToDate(pSetSysTime->tickCount, &pSetSysTime->date);
#else
        Fwl_GetRTCtime(&pSetSysTime->date);
#endif
    }
 
    if (pSetSysTime->refresh == SST_REFRESH_ALL)
    {
        //pclint -e(572)
        #if (USE_COLOR_LCD)
        Eng_ImageResDisp(0, 0, eRES_IMAGE_SYSCLKBCK, 0);
        #else
        Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);   
        Eng_ImageResDisp(8, 2,eRES_IMAGE_POWERTIME, AK_FALSE);
        Eng_ImageResDisp(SST_STR_BACK_LEFT, SST_STR_BACK_TOP, eRES_IMAGE_POWERTIMEBG, AK_FALSE);    
        #endif
    }

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    if (pSetSysTime->refresh & SST_REFRESH_TOPBAR)
    {
        TopBar_SetReflesh(&pSetSysTime->systime_topbar, TOPBAR_REFLESH_ALL);
    }

#endif

//////////////////////////////////////////need to be changed lsk
    //display title string
    #if (USE_COLOR_LCD)
    if(pSetSysTime->refresh & SST_REFRESH_TITLE)
    {
        T_RES_STRING resStringID= eRES_STR_SYS_TIME;
#if(USE_ALARM_CLOCK)
        if(CLOCKTIME_SET == pSetSysTime->function)
        {
            if(POWEROFF_CLOCK == pSetSysTime->pClockParam->clockSet.clockType)
            {
                resStringID= eRES_STR_TIMER_POWOFF;
            }
            else
            {
                resStringID= eRES_STR_ALARM_TIME;
            }
        }
#endif

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    TopBar_ConfigTitleSet(&pSetSysTime->systime_topbar, (T_U16)resStringID);
    TopBar_SetReflesh(&pSetSysTime->systime_topbar, TOPBAR_REFLESH_TITLE);
#else
    DispStringInWidth(CP_UNICODE, SST_STR_TITLE_LEFT, SST_STR_TITLE_TOP, (T_U8*)resStringID, MAIN_LCD_WIDTH,CLR_WHITE);
#endif
    }
    #endif

    //this function should be executed after the function TopBar_SetReflesh
#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_show(&pSetSysTime->systime_topbar);
#endif

    //display year string
    if(pSetSysTime->refresh & SST_REFRESH_YEAR)
    {
        Utl_Itoa(pSetSysTime->date.year,hintMsg);
        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_YEAR)
            && (pSetSysTime->bShowSpace))
            for(i = 0; i < 4; i++)
                hintMsg[i] = ' ';
        #endif
        
        #if (USE_COLOR_LCD)
        if (pSetSysTime->setWhat == SET_YEAR)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                //Eng_ImageResDisp(SST_ICON_TIME_YEAR_LEFT, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_YEAR_P, AK_FALSE);
                Fwl_FillRect(SST_ICON_TIME_YEAR_LEFT+6, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            else
            #endif
//          #endif
                Fwl_FillRect(SST_BCK_YEAR_LEFT-1, SST_STR_YEAR_TOP+ 2, 2*FONT_WIDTH-4, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                //Eng_ImageResDisp(SST_ICON_TIME_YEAR_LEFT, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_YEAR, AK_FALSE);
                Fwl_FillRect(SST_ICON_TIME_YEAR_LEFT+6, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            else
            #endif
//          #endif
                Fwl_FillRect(SST_BCK_YEAR_LEFT-1, SST_STR_YEAR_TOP+ 2, 2*FONT_WIDTH-4, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_YEAR_LEFT,SST_STR_YEAR_TOP,(T_U8 *)hintMsg,4*FONT_WIDTH,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_YEAR_LEFT,SST_STR_YEAR_TOP,hintMsg,4*SMALL_FONT_WIDTH,CLR_BLACK,0);         
        #endif
    }

    //display month string
    if(pSetSysTime->refresh & SST_REFRESH_MON)
    {
        hintMsg[0] = '-';
        if(pSetSysTime->date.month < 10)
        {
            hintMsg[1] = '0';
            Utl_Itoa((T_U32)pSetSysTime->date.month,&hintMsg[2]);
        }else
            Utl_Itoa((T_U32)pSetSysTime->date.month,&hintMsg[1]);
        
        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_MONTH)
            &&pSetSysTime->bShowSpace)
            for(i = 1; i < 4; i++)
                hintMsg[i] = ' ';       
        #endif

        #if (USE_COLOR_LCD)
        if(pSetSysTime->setWhat == SET_MONTH)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_MON_P, AK_FALSE);
                Fwl_FillRect(SST_ICON_TIME_MON_LEFT+2, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            else
            #endif
//          #endif          
                Fwl_FillRect(SST_BCK_MON_LEFT, SST_STR_MON_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_MON_LEFT+2, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_MON, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_MON_LEFT, SST_STR_MON_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_MON_LEFT,SST_STR_MON_TOP,(T_U8 *)(&hintMsg[1]),2*FONT_WIDTH*2,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_MON_LEFT,SST_STR_MON_TOP,hintMsg,3*SMALL_FONT_WIDTH,CLR_BLACK,0);           
        #endif
    }



    //display day string
    if(pSetSysTime->refresh & SST_REFRESH_DAY)
    {
        hintMsg[0] = '-';
        if(pSetSysTime->date.day < 10)
        {
            hintMsg[1] = '0';
            Utl_Itoa(pSetSysTime->date.day,&hintMsg[2]);    
        }else
            Utl_Itoa(pSetSysTime->date.day,&hintMsg[1]);

        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_DAY)
            && (pSetSysTime->bShowSpace))
            for(i = 1; i < 4; i++)
                hintMsg[i] = ' ';
        #endif

        #if (USE_COLOR_LCD)
        if(pSetSysTime->setWhat == SET_DAY)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_DAY_LEFT+2, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_DAY_P, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_DAY_LEFT, SST_STR_DAY_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_DAY_LEFT+2, SST_ICON_TIME_YMD_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_YMD_TOP, eRES_IMAGE_SYSTIME_DAY, AK_FALSE);
            else
            #endif
//          #endif  
            Fwl_FillRect(SST_BCK_DAY_LEFT, SST_STR_DAY_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_DAY_LEFT,SST_STR_DAY_TOP,(T_U8 *)(&hintMsg[1]),2*FONT_WIDTH,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_DAY_LEFT,SST_STR_DAY_TOP,hintMsg,3*SMALL_FONT_WIDTH,CLR_BLACK,0);           
        #endif
    }

    //dispaly hour string
    if(pSetSysTime->refresh & SST_REFRESH_HOUR)
    {
        if(pSetSysTime->date.hour < 10)
        {
            hintMsg[0] = '0';
            Utl_Itoa(pSetSysTime->date.hour,&hintMsg[1]);
        }else
            Utl_Itoa(pSetSysTime->date.hour,hintMsg);       

        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_HOUR)
            && (pSetSysTime->bShowSpace))
            for(i = 0; i < 4; i++)
                hintMsg[i] = ' ';
        #endif

        #if (USE_COLOR_LCD)
        if(pSetSysTime->setWhat == SET_HOUR)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_HOUR_LEFT+6, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            //  Eng_ImageResDisp(SST_ICON_TIME_HOUR_LEFT, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_HOUR_P, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_HOUR_LEFT, SST_STR_HOUR_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_HOUR_LEFT+6, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            //  Eng_ImageResDisp(SST_ICON_TIME_HOUR_LEFT, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_HOUR, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_HOUR_LEFT, SST_STR_HOUR_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_HOUR_LEFT,SST_STR_HOUR_TOP,(T_U8 *)hintMsg,2*FONT_WIDTH,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_HOUR_LEFT,SST_STR_HOUR_TOP,hintMsg,2*SMALL_FONT_WIDTH,CLR_BLACK,0);         
        #endif
    }

    //display minute string
    if(pSetSysTime->refresh & SST_REFRESH_MINUTE)
    {
        hintMsg[0] = ':';
        if(pSetSysTime->date.minute < 10)
        {
            hintMsg[1] = '0';
            Utl_Itoa(pSetSysTime->date.minute,&hintMsg[2]); 
        }else
            Utl_Itoa(pSetSysTime->date.minute,&hintMsg[1]);

        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_MINUTE)
            && (pSetSysTime->bShowSpace))
            for(i = 1; i < 4; i++)
                hintMsg[i] = ' ';
        #endif

        #if (USE_COLOR_LCD)
        if(pSetSysTime->setWhat == SET_MINUTE)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_MIN_LEFT+2, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_MIN_P, AK_FALSE);
            else
            #endif
//          #endif  
            Fwl_FillRect(SST_BCK_MIN_LEFT, SST_STR_MIN_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_MIN_LEFT+2, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_MIN, AK_FALSE);
            else
            #endif
//          #endif  
            Fwl_FillRect(SST_BCK_MIN_LEFT, SST_STR_MIN_TOP+ 2, FONT_WIDTH, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_MIN_LEFT,SST_STR_MIN_TOP,(T_U8 *)(&hintMsg[1]),2*FONT_WIDTH,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_MIN_LEFT,SST_STR_MIN_TOP,hintMsg,3*SMALL_FONT_WIDTH,CLR_BLACK,0);       
        #endif
    }

    //display second string
    if(pSetSysTime->refresh & SST_REFRESH_SECOND)
    {
        hintMsg[0] = ':';
        if(pSetSysTime->date.second < 10)
        {
            hintMsg[1] = '0';
            Utl_Itoa(pSetSysTime->date.second,&hintMsg[2]); 
        }else
            Utl_Itoa(pSetSysTime->date.second,&hintMsg[1]);

        #if (!USE_COLOR_LCD)
        if((pSetSysTime->setWhat == SET_SECOND)
            && (pSetSysTime->bShowSpace))
            for(i = 1; i < 4; i++)
                hintMsg[i] = ' ';
        #endif

        #if (USE_COLOR_LCD)
        if(pSetSysTime->setWhat == SET_SECOND)
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_SEC_LEFT+2, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(0, 255, 0));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_SEC_P, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_SEC_LEFT, SST_STR_SEC_TOP+2, FONT_WIDTH, FONT_HEIGHT- 4, SST_TIME_BGD);
        }
        else
        {
//          #ifdef SUPPORT_TSR
            #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
            if (AK_TRUE)
                Fwl_FillRect(SST_ICON_TIME_SEC_LEFT+2, SST_ICON_TIME_HMS_TOP+ 5, 44, 21, RGB_COLOR(255, 255, 255));
            //  Eng_ImageResDisp(xpos, SST_ICON_TIME_HMS_TOP, eRES_IMAGE_SYSTIME_SEC, AK_FALSE);
            else
            #endif
//          #endif  
                Fwl_FillRect(SST_BCK_SEC_LEFT, SST_STR_SEC_TOP+2, FONT_WIDTH, FONT_HEIGHT- 4, CLR_BLACK);
        }
        DispStringInWidth(CP_936,SST_STR_SEC_LEFT,SST_STR_SEC_TOP,(T_U8 *)(&hintMsg[1]),2*FONT_WIDTH,CLR_CUSTOM);
        #else
        DispStringInWidth_S(SST_STR_SEC_LEFT,SST_STR_SEC_TOP,hintMsg,4*SMALL_FONT_WIDTH,CLR_BLACK,0);       
        #endif
    }

    ST_Setrefresh(SST_REFRESH_NONE);
}

/*****************************************************
** brief:  设置刷新标志位
**param:   
******************************************************/
static T_VOID   ST_Setrefresh(T_U32 flag)
{
    if(flag == SST_REFRESH_NONE)
        pSetSysTime->refresh = SST_REFRESH_NONE;
    else
        pSetSysTime->refresh |= flag;

    return;
}

#else
void initset_system_time(void)
{
}
void paintset_system_time(void)
{
}
void exitset_system_time(void)
{
}
unsigned char handleset_system_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_system_time\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

