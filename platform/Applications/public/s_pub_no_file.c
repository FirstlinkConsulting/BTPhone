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
#include "M_event_api.h"
#include "m_event.h"
#include "Gbl_Resource.h"
#include "Ctrl_Dialog.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"

#if(NO_DISPLAY == 0)

#define SHOW_HINT_TIME 1500
typedef struct  {
    CDialogCtrl dialog;
    T_TIMER     tmExit;
}T_NoFileUI;

static T_NoFileUI* pNoFileUI = AK_NULL;

#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
extern T_U16 GetUITitleStrID(T_VOID);
#endif

void initpub_no_file(void)
{
     pNoFileUI= (T_NoFileUI* )Fwl_Malloc(sizeof(T_NoFileUI));  
     memset(pNoFileUI, 0, sizeof(T_NoFileUI));
     pNoFileUI->tmExit = ERROR_TIMER;
}

void exitpub_no_file(void)
{
    if(pNoFileUI->tmExit != ERROR_TIMER)
    {
        pNoFileUI->tmExit= Fwl_TimerStop(pNoFileUI->tmExit);
    }

    Dialog_Free(&pNoFileUI->dialog);
    pNoFileUI= (T_NoFileUI* )Fwl_Free(pNoFileUI);
}

void paintpub_no_file(void)
{
    Dialog_Show(&pNoFileUI->dialog);
}

unsigned char handlepub_no_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if ((event == M_EVT_AUD_HINT_NOFILE)||
        (event == M_EVT_NO_FILE) || (M_EVT_ERR_LIST == event))
    {
        T_U16 uHint[16];
        T_U16 uSpace[24];
        T_U8 i= 0;
        
#if(USE_COLOR_LCD)
        uSpace[i++]= '\n';
        uSpace[i++]= '\n';
        uSpace[i++]= '\n';
#endif
        uSpace[i++]= 0;
        UMemSet(uHint, 0, 16);
        if(M_EVT_ERR_LIST == event)
        {
            GetResString(eRES_STR_AUD_LIST_NOFILE, uHint, 16);
        }
        else
        {
#ifdef SUPPORT_SDCARD
            if (pEventParm != AK_NULL && pEventParm->w.Param1 == MMC_SD_CARD)
            {
                GetResString(eRES_STR_CARD_NO_FILE, uHint, 16);
            }
            else
#endif
            {
                GetResString(eRES_STR_DISK_NO_FILE, uHint, 16);
            }
        }

        UStrCat(uSpace, uHint);

#if (USE_COLOR_LCD)
        Dialog_InitEx(&pNoFileUI->dialog, uSpace, CTRL_DIALOG_RESPONSE_HIDDEN, INVALID_STRINGRES_ID
                    , INVALID_IMAGERES_ID, CTRL_DIALOG_RESPONSE_YES, 0 , 0, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT);
        Dialog_SetBkImage(&pNoFileUI->dialog, eRES_IMAGE_IMG_ERROR);
        #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
        TopBar_ConfigTitleSet(&pNoFileUI->dialog.dialog_topbar, GetUITitleStrID());
        #endif
#else
        Dialog_Init(&pNoFileUI->dialog, uHint, CTRL_DIALOG_RESPONSE_HIDDEN, INVALID_STRINGRES_ID
                    , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES);
#endif
        if(pNoFileUI->tmExit != ERROR_TIMER)
        {
            pNoFileUI->tmExit= Fwl_TimerStop(pNoFileUI->tmExit);
        }
        pNoFileUI->tmExit= Fwl_TimerStartMilliSecond(SHOW_HINT_TIME, AK_FALSE);
        //AK_DEBUG_OUTPUT("timerstart:pNoFileUI->tmExit = %d\n");
        return 0;
    }
    
    if(event == M_EVT_USER_KEY)
    {
#if OS_ANYKA
        AK_PRINTK("**user key", 0, 1);
#endif
        pNoFileUI->tmExit= Fwl_TimerStop(pNoFileUI->tmExit);
        m_triggerEvent(M_EVT_EXIT, pEventParm);
    }
    else if(event == VME_EVT_TIMER && GET_TIMER_ID(pEventParm) == pNoFileUI->tmExit)
    {
        pNoFileUI->tmExit= Fwl_TimerStop(pNoFileUI->tmExit);
        m_triggerEvent(M_EVT_EXIT, pEventParm);
    }
    else if(M_EVT_RETURN == event || M_EVT_TIMEOUT == event)
    {
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else //#if(NO_DISPLAY == 0)
void initpub_no_file(void)
{
}
void exitpub_no_file(void)
{
}
void paintpub_no_file(void)
{
}
unsigned char handlepub_no_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("enter pub_no_file\n");
    m_triggerEvent(M_EVT_EXIT, NULL);
    return 0;    
}
#endif
/* end of file */
