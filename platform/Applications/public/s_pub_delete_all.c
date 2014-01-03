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
#include "log_file_com.h"
#include "Ctrl_Dialog.h"
#include "Fwl_FreqMgr.h"
#include "Gui_Common.h"
#include "Fwl_osFS.h"
#include "Fwl_osMalloc.h"

#if(NO_DISPLAY == 0)

#define YESNO_TITLE_HIGHT  32
#define ONDEL_TITLE_HIGHT 32

#if(USE_COLOR_LCD)

#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
static T_VOID PubDeleteAll_PaintYesNoTitle()
{
     Eng_ImageResDisp(0, 0, eRES_IMAGE_TOP_TITLE, AK_FALSE); 
     Gui_DispTitle(eRES_STR_PUB_DEL_ALL, CLR_WHITE, eRES_IMAGE_TOP_TITLE, YESNO_TITLE_HIGHT);
}

static T_VOID PubDeleteAll_PaintOnDelTitle()
{
     Eng_ImageResDisp(0, 0, eRES_IMAGE_TOP_TITLE, AK_FALSE); 
     Gui_DispTitle(eRES_STR_PUB_DEL_ALL, CLR_WHITE, eRES_IMAGE_TOP_TITLE, ONDEL_TITLE_HIGHT);

}
#endif

#endif

typedef struct {
    CDialogCtrl dialog;
    CDialogCtrl onDelDialog;
    T_eActiveCtrlType activeType;
    T_S16 displayCount;
    T_BOOL flashFlag;
}T_DelAllUI;

static T_FileDelAll* pFileDelAll;
static T_DelAllUI* pDelAllUI;
extern T_BOOL IsAudplayer(T_VOID);

void initpub_delete_all(void)
{
    T_U16 uStrHint[10];
    Fwl_FreqPush(FREQ_APP_GAME);
    pDelAllUI= (T_DelAllUI* )Fwl_Malloc(sizeof(T_DelAllUI));
    pDelAllUI->activeType= eDialog;
    pDelAllUI->flashFlag= AK_TRUE;
    pDelAllUI->displayCount= -1;
#if(USE_COLOR_LCD)
    UMemSet(uStrHint, 0, 10);
    #if ((1 == LCD_HORIZONTAL)) 
    GetResString(eRES_STR_BW_DEL_ALL, uStrHint, 10);
    #endif
    Dialog_InitEx(&pDelAllUI->dialog, uStrHint, CTRL_DIALOG_RESPONSE_YES | CTRL_DIALOG_RESPONSE_NO, eRES_STR_RADIO_EMPTY_STR, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO
        , 0, YESNO_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- YESNO_TITLE_HIGHT) ); 
#else
        Dialog_Init(&pDelAllUI->dialog, AK_NULL, CTRL_DIALOG_RESPONSE_YES | CTRL_DIALOG_RESPONSE_NO, eRES_STR_BW_DEL_ALL, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO);  
#endif
}

void exitpub_delete_all(void)
{
    Dialog_Free(&pDelAllUI->dialog);
    Dialog_Free(&pDelAllUI->onDelDialog);
    pDelAllUI= (T_DelAllUI*)Fwl_Free(pDelAllUI);

    Fwl_FreqPop();
}

void paintpub_delete_all(void)
{
    if(pDelAllUI->activeType == eDialog)
    {
#if(USE_COLOR_LCD)
        if(pDelAllUI->flashFlag)
        {
        #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            PubDeleteAll_PaintYesNoTitle(); 
        #endif
            pDelAllUI->flashFlag= AK_FALSE;
        }
#endif
        Dialog_Show(&pDelAllUI->dialog);
    }
    else if(pDelAllUI->activeType == eDelHint)
    {
#if(USE_COLOR_LCD)
        if(pDelAllUI->flashFlag)
        {
        #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            PubDeleteAll_PaintOnDelTitle();
        #endif
            pDelAllUI->flashFlag= AK_FALSE;
        }
#endif
        Dialog_Show(&pDelAllUI->onDelDialog);
    }
}

unsigned char handlepub_delete_all(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;
    T_U16 uStrDel[16];
    T_EVT_PARAM evtParam;
    
    if(AK_NULL == pEventParm)
    {
        pEventParm = &evtParam;
    }

    if(pDelAllUI->activeType == eDelHint)
    {
        if(pDelAllUI->displayCount == HINT_DISPLAY_COUNT+1)
        {
            TryDeleteEbookMarker(pFileDelAll->filter, AK_TRUE);

            #ifdef SUPPORT_SDCARD

            if(IsAudplayer())
            {
                if(AK_TRUE == Fwl_MemDevIsMount(MMC_SD_CARD))
                {
                    pFileDelAll->bResult= DelDirAllFiles(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL), pFileDelAll);
                }
              
                pFileDelAll->bResult= DelDirAllFiles(g_diskPath, pFileDelAll);
            }
            else
            {
                if (pFileDelAll->driver == MMC_SD_CARD)
                {
                    if(AK_TRUE == Fwl_MemDevIsMount(MMC_SD_CARD))
                    {
                        pFileDelAll->bResult= DelDirAllFiles(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL), pFileDelAll);
                    }
                }
                else
                {
                    pFileDelAll->bResult= DelDirAllFiles(g_diskPath, pFileDelAll);
                }
            }
            #else
            pFileDelAll->bResult= DelDirAllFiles(g_diskPath, pFileDelAll);
            #endif 
            pDelAllUI->displayCount--;    
            return 1;
        }
        else if(pDelAllUI->displayCount == 0)
        {
            pEventParm->p.pParam1= pFileDelAll;
            //m_triggerEvent(M_EVT_EXIT, pEventParm);
            //success to delete all files
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            pDelAllUI->displayCount--;
            return 0;
        }
        else
        {
            pDelAllUI->displayCount--;
            return 1;
        }
    }

    if(event == M_EVT_DEL_ALL)
    {
        pFileDelAll= (T_FileDelAll* )pEventParm->p.pParam1;
        pFileDelAll->bResult= AK_FALSE;
    }

    if(M_EVT_EXIT == event)
    {
        #if(USE_COLOR_LCD)
                pDelAllUI->flashFlag= AK_TRUE;
        #endif
        if(pDelAllUI->activeType == eDialog)
        {
            Dialog_SetRefresh(&pDelAllUI->dialog);
        }
        else if(pDelAllUI->activeType == eDelHint)
        {

            Dialog_SetRefresh(&pDelAllUI->onDelDialog);
        }
    }

    //TEST_EXIT_MENU(event, pEventParm)
    ret= Dialog_Handler(&pDelAllUI->dialog, event, pEventParm);

    switch(ret)
    {
    case CTRL_DIALOG_RESPONSE_YES:
        UMemSet(uStrDel, 0, 16);
        GetResString(eRES_STR_PUB_IN_DEL, uStrDel, 16);
        #if(USE_COLOR_LCD)
        Dialog_InitEx(&pDelAllUI->onDelDialog, uStrDel, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES,
                 0, ONDEL_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- ONDEL_TITLE_HIGHT) );
         pDelAllUI->flashFlag= AK_TRUE;
        #else
        Dialog_Init(&pDelAllUI->onDelDialog, uStrDel, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES);
        #endif
        pDelAllUI->activeType= eDelHint;
        pDelAllUI->displayCount= HINT_DISPLAY_COUNT+ 1;
        return 1;

    case CTRL_DIALOG_RESPONSE_NO:
    case CTRL_RESPONSE_QUIT:
        pFileDelAll->bResult= AK_FALSE;
        //fail to delete all
        pEventParm->p.pParam1= pFileDelAll;
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    }
    
    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initpub_delete_all(void)
{
}
void paintpub_delete_all(void)
{
}
void exitpub_delete_all(void)
{
}
unsigned char handlepub_delete_all(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{    
    AK_DEBUG_OUTPUT("enter pub_delete_all\n");
    m_triggerEvent(M_EVT_EXIT, NULL);
    return 0;
}

#endif

/* end of file */
