/************************************************************************
* Copyright (c) 2008, Anyka Co., Ltd.
* All rights reserved.
*
* @FILENAME 
* @BRIEF 
* @Author：zhao_xiaowei
* @Date：
* @Version：
**************************************************************************/

#include "Apl_Public.h"
#include "M_event_api.h"
#include "m_event.h"
#include "Gbl_Resource.h"
#include "Fwl_osFS.h"
#include "Ctrl_ListFile.h"
#include "Ctrl_Dialog.h"
#include "log_file_com.h"
#include "log_aud_control.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Gui_Common.h"

#if(NO_DISPLAY == 0)

#define TIMEOUT   6000
#define TITLE_HIGHT  32
#define YESNO_TITLE_HIGHT  32
#define ONDEL_TITLE_HIGHT 32

typedef struct{
#if(USE_COLOR_LCD)
    CListFileCtrl listFile;
    T_U16 parentPath[MAX_FILE_LEN];
#endif
    T_TIMER timeout;
    T_BOOL  flashFlag;
    T_eActiveCtrlType activeType;
    CDialogCtrl dialog;
    CDialogCtrl onDelDialog;
    T_S16     displayCount;
}T_FileDelUI;

static T_FileDelUI* pFileDelUI;
static T_FileDel* pFileDel;

#if(USE_COLOR_LCD)
extern T_VOID   EbookDeleteBookMark(T_U16 * file, T_BOOL bDeleteAll);

#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))

static T_VOID PubDeleteFile_PaintYesNoTitle()
{
     Eng_ImageResDisp(0, 0, eRES_IMAGE_TOP_TITLE, AK_FALSE); 
     Gui_DispTitle(eRES_STR_PUB_DEL_FILE, CLR_WHITE, eRES_IMAGE_TOP_TITLE, YESNO_TITLE_HIGHT);
}

static T_VOID PubDeleteFile_PaintOnDelTitle()
{ 
    Eng_ImageResDisp(0, 0, eRES_IMAGE_TOP_TITLE, AK_FALSE); 
    Gui_DispTitle(eRES_STR_PUB_DEL_FILE, CLR_WHITE, eRES_IMAGE_TOP_TITLE, ONDEL_TITLE_HIGHT);
}

static T_VOID PubDeleteFile_PaintTitle()
{
     Eng_ImageResDisp(0, 0, (T_RES_IMAGE)GetUITitleResID(), AK_FALSE); 
}

#endif

#endif

void initpub_delete_file(void)
{
    Fwl_FreqPush(FREQ_APP_GAME);
    pFileDelUI= (T_FileDelUI *)Fwl_Malloc(sizeof(T_FileDelUI));
#if(USE_COLOR_LCD)
    Dialog_InitEx(&pFileDelUI->dialog, AK_NULL, CTRL_DIALOG_RESPONSE_NO| CTRL_DIALOG_RESPONSE_YES, 
        eRES_STR_PUB_DEL_HINT, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO
        ,0, YESNO_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- YESNO_TITLE_HIGHT) );
#else
    Dialog_Init(&pFileDelUI->dialog, AK_NULL, CTRL_DIALOG_RESPONSE_NO| CTRL_DIALOG_RESPONSE_YES, 
        eRES_STR_PUB_DEL_FILE, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO);
#endif

#if(USE_COLOR_LCD)
    // in order to match Dialog_Free
    Dialog_InitEx(&pFileDelUI->onDelDialog,AK_NULL, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT 
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES,
                0, ONDEL_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- ONDEL_TITLE_HIGHT) );
#else
     Dialog_Init(&pFileDelUI->onDelDialog,AK_NULL, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT 
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES);
#endif
#if(USE_COLOR_LCD)
    pFileDelUI->activeType = eListFile;
#else
    pFileDelUI->activeType= eDialog;
#endif
    pFileDelUI->timeout= ERROR_TIMER;

    pFileDelUI->displayCount= -1;
    
    
}

void exitpub_delete_file(void)
{
#if (USE_COLOR_LCD)
    ListFile_Free(&pFileDelUI->listFile);
#endif
    Dialog_Free(&pFileDelUI->onDelDialog);
    if(ERROR_TIMER != pFileDelUI->timeout)
    {
         pFileDelUI->timeout= Fwl_TimerStop(pFileDelUI->timeout);
    }
    Dialog_Free(&pFileDelUI->dialog);
    pFileDelUI= (T_FileDelUI* )Fwl_Free(pFileDelUI);

     Fwl_FreqPop();
}

void paintpub_delete_file(void)
{
    if(pFileDelUI->activeType == eDelHint)
    {
#if(USE_COLOR_LCD)
         if(pFileDelUI->flashFlag)
        {        
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            PubDeleteFile_PaintOnDelTitle();
#endif
            pFileDelUI->flashFlag= AK_FALSE;
        }
#endif
        Dialog_Show(&pFileDelUI->onDelDialog);
    }
#if(USE_COLOR_LCD)

    if(pFileDelUI->activeType == eListFile)
    {
        if(pFileDelUI->flashFlag)
        {        
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            PubDeleteFile_PaintTitle();
#endif
            pFileDelUI->flashFlag= AK_FALSE;
        }
        ListFile_Show(&pFileDelUI->listFile);
    }
    else if(pFileDelUI->activeType == eDialog)
    {
         if(pFileDelUI->flashFlag)
        {
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            PubDeleteFile_PaintYesNoTitle();
#endif
            pFileDelUI->flashFlag= AK_FALSE;
        }
        Dialog_Show(&pFileDelUI->dialog);
    }
#else
    if(pFileDelUI->activeType == eDialog)
    {
        Dialog_Show(&pFileDelUI->dialog);
    }
#endif
}

static T_VOID QuitDelFile(T_EVT_PARAM* pEvtParam)
{
    pEvtParam->p.pParam1= pFileDel;
    m_triggerEvent(M_EVT_EXIT, pEvtParam);
}

#if(USE_COLOR_LCD)
static T_VOID RefleshListFile()
{ 
    pFileDelUI->activeType= eListFile;
    pFileDelUI->flashFlag= AK_TRUE;
    ListFile_SetRefresh(&pFileDelUI->listFile); 
}

static T_BOOL InitListFile(T_BOOL isFirst)
{
    T_BOOL bRet;
    if(ERROR_TIMER != pFileDelUI->timeout)
    {
        pFileDelUI->timeout= Fwl_TimerStop(pFileDelUI->timeout);       
    }

    Fwl_GetCurrentDir(pFileDelUI->parentPath, pFileDel->file);
    bRet= ListFile_InitEx(&pFileDelUI->listFile, pFileDelUI->parentPath, eRES_STR_PUB_DEL_HINT,
        CTRL_LISTFILE_DISP_FILEONLY/*| CTRL_LISTFILE_DISP_PATTERONLY*/, pFileDel->filter    //加入CTRL_LISTFILE_DISP_PATTERONLY则显示文件夹和../等
        ,0, TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- TITLE_HIGHT) );
    pFileDelUI->flashFlag= AK_TRUE;
    pFileDelUI->activeType= eListFile;
    
    if(bRet)
    {
        ListFile_SetStyle(&pFileDelUI->listFile, CTRL_LISTFILE_STYLE_ROOTDIR_DISPNONE);
        if(isFirst)
        {
            Fwl_GetShortPath(pFileDelUI->parentPath, pFileDel->file);
            ListFile_SetCurFile(&pFileDelUI->listFile, pFileDelUI->parentPath);
        }
    }
    
     pFileDelUI->timeout= Fwl_TimerStartMilliSecond(TIMEOUT, AK_FALSE);
    return bRet; 
}

static T_BOOL ReInitListFile()
{
 
    ListFile_Free(&pFileDelUI->listFile);
    return InitListFile(AK_FALSE);
}
#endif
unsigned char handlepub_delete_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY pressKey;
    T_U32 ret;
    T_U16 uStrDel[LCD_BUF_LEN+ 1];
    T_U16 count= 0;
    T_U16 uHintDel[10];

    
#if (USE_COLOR_LCD == 0)
    T_U16 fileName[MAX_FILE_LEN]={0 };  
#endif
    T_EVT_PARAM evtParam;


    if(AK_NULL == pEventParm)
    {
        pEventParm= &evtParam;
    }
    
    if(M_EVT_EXIT == event)
    {
       if(pFileDelUI->activeType == eDelHint)
        {
    #if(USE_COLOR_LCD)
             pFileDelUI->flashFlag= AK_TRUE;
    #endif
            Dialog_SetRefresh(&pFileDelUI->onDelDialog);
        }
    #if(USE_COLOR_LCD)

        if(pFileDelUI->activeType == eListFile)
        {
            pFileDelUI->flashFlag= AK_TRUE; 
            ListFile_SetRefresh(&pFileDelUI->listFile);
        }
        else if(pFileDelUI->activeType == eDialog)
        {
             pFileDelUI->flashFlag= AK_TRUE;
            Dialog_SetRefresh(&pFileDelUI->dialog);
        }
    #else
        if(pFileDelUI->activeType == eDialog)
        {
            Dialog_SetRefresh(&pFileDelUI->dialog);
        }
    #endif
    }

    if(pFileDelUI->activeType == eDelHint)
    {
        if(pFileDelUI->displayCount == 0)
        {
            pFileDelUI->displayCount= -1;
            //if delete once success then pFileDel->bResult is AK_TURE
            TryDeleteEbookMarker(pFileDel->file, AK_FALSE);

            if(pFileDel->bResult == AK_FALSE)
            {
                    AK_DEBUG_OUTPUT("\n in color mode\n");
               // Printf_UC(pFileDel->file);
                pFileDel->bResult= Fwl_FileDelete(pFileDel->file);
            }
            else
            {
                Fwl_FileDelete(pFileDel->file);
                //if pFileDel->bResult is AK_TURE not give the value
            }         

#if (USE_COLOR_LCD)
            if(pFileDel->bResult)
            {
                if( ReInitListFile() ) 
                {
                    return 1;
                }
                else
                {
                    QuitDelFile(pEventParm);
                    return 0;
                }
            }
            else
            {
                QuitDelFile(pEventParm);
                return 0;
            }
#else
            QuitDelFile(pEventParm);
            return 0;
#endif
        }
        else
        {
            pFileDelUI->displayCount--;
            return 1;
        }
    }
    
    if(event == M_EVT_DEL_FILE)
    {
        pFileDel= (T_FileDel*)pEventParm->p.pParam1;
        pFileDel->bResult= AK_FALSE;
        
#if(USE_COLOR_LCD)
        AK_ASSERT_VAL(InitListFile(AK_TRUE), AK_NULL, 0);
#else
        Fwl_GetShortPath(fileName, pFileDel->file);
        if(Fwl_FilterMatch((T_U16*)strUFilter[0], fileName))
        {
#ifdef  OS_ANYKA
            if(!Aud_AudCtrlGetExtFileInfo(pFileDel->file, fileName))
            {
                Fwl_GetShortPath(fileName, pFileDel->file);
            }
            else
            {
                if(UStrLen(fileName) == 0)
                {
                    Fwl_GetShortPath(fileName, pFileDel->file);
                }
            }
#endif
        }
        else
        {
            #if 0
            dotPos= UStrRevFndR(uDot, fileName, 0);
            if(dotPos > 0)
            {
                len= UStrLen(fileName);
                if(len != MAX_FILE_LEN)
                {
                    for(i= len-1; i> dotPos; i--)
                    {
                        fileName[i+ 1]= fileName[i];
                    }
                    fileName[dotPos]= ' ';
                    fileName[dotPos+ 1]= ' ';
                }
                fileName[len+ 1]= 0;
            }
            #endif
        }
        
        pFileDelUI->activeType= eDialog;
        Dialog_Free(&pFileDelUI->dialog);
       
        Dialog_Init(&pFileDelUI->dialog, fileName, CTRL_DIALOG_RESPONSE_NO| CTRL_DIALOG_RESPONSE_YES, 
            eRES_STR_BW_DEL_FILE, eRES_IMAGE_FILEICON, CTRL_DIALOG_RESPONSE_NO);
        pFileDelUI->timeout= Fwl_TimerStartMilliSecond(TIMEOUT, AK_FALSE);
#endif
        
    }
    
    if(event == M_EVT_USER_KEY)
    {
        if(ERROR_TIMER != pFileDelUI->timeout)
        {
            pFileDelUI->timeout= Fwl_TimerStop(pFileDelUI->timeout);
            pFileDelUI->timeout= Fwl_TimerStartMilliSecond(TIMEOUT, AK_FALSE);
        }

        pressKey.id = GET_KEY_ID(pEventParm);
        if(pressKey.id == kbMODE)
        {
#if (USE_COLOR_LCD)
            if(pFileDelUI->activeType == eDialog)
            {
                RefleshListFile();// exit dialog into listFile
                return 1;
            }
            else
            {
                QuitDelFile(pEventParm);
                return 0;
            }
#else
            QuitDelFile(pEventParm);
            return 0;
#endif
        }
    }

    if(VME_EVT_TIMER == event && GET_TIMER_ID(pEventParm) == pFileDelUI->timeout)
    {
        pFileDelUI->timeout= Fwl_TimerStop(pFileDelUI->timeout);
        m_triggerEvent(M_EVT_EXIT, pEventParm);
    }


    if(pFileDelUI->activeType == eListFile)
    {
#if(USE_COLOR_LCD)
        ret= ListFile_Handler(&pFileDelUI->listFile, event, pEventParm);
        
        switch(ret)
        {
        case CTRL_RESPONSE_QUIT:
            QuitDelFile(pEventParm);
            break;
        case CTRL_RESPONSE_NONE:
            break;
        default:
                    
        UMemSet(uHintDel, 0, 10);
        #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
        GetResString(eRES_STR_BW_DEL_FILE, uHintDel, 10);
        #endif
            Utl_UStrCpy(pFileDel->file, (T_U16* )ret);
            
            Dialog_Free(&pFileDelUI->dialog);
            #if USE_COLOR_LCD
            Dialog_InitEx(&pFileDelUI->dialog, uHintDel, CTRL_DIALOG_RESPONSE_NO| CTRL_DIALOG_RESPONSE_YES, 
                eRES_STR_PUB_DEL_HINT, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO,
                0, YESNO_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- YESNO_TITLE_HIGHT) );
           pFileDelUI->flashFlag= AK_TRUE;
            #else
            Dialog_Init(&pFileDelUI->dialog, AK_NULL, CTRL_DIALOG_RESPONSE_NO| CTRL_DIALOG_RESPONSE_YES, 
                eRES_STR_PUB_DEL_FILE, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO);

            #endif
            pFileDelUI->activeType= eDialog;
        }
#endif
    }
    else
    {
        ret= Dialog_Handler(&pFileDelUI->dialog, event, pEventParm);
        switch(ret)
        {
        case CTRL_DIALOG_RESPONSE_YES:
            count= GetResString(eRES_STR_PUB_IN_DEL, uStrDel, LCD_BUF_LEN);
            uStrDel[count]= 0;
            
            Dialog_Free(&pFileDelUI->onDelDialog);
            #if  USE_COLOR_LCD
            Dialog_InitEx(&pFileDelUI->onDelDialog, uStrDel, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT 
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES,
                0, ONDEL_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- ONDEL_TITLE_HIGHT) );
            pFileDelUI->flashFlag= AK_TRUE;
            #else
            Dialog_Init(&pFileDelUI->onDelDialog, uStrDel, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_PUB_DEL_HINT 
                , ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_YES);
            #endif
            pFileDelUI->activeType= eDelHint;
            pFileDelUI->displayCount= HINT_DISPLAY_COUNT;            
            return 1;
            
            break;
            
        case CTRL_DIALOG_RESPONSE_NO:
        case CTRL_RESPONSE_QUIT:
            
#if (USE_COLOR_LCD)
            //重新初始化ListFile
            RefleshListFile();
#else
            QuitDelFile(pEventParm);
#endif         
            break;
            
        }
    }

    if (event >= M_EVT_Z00_POWEROFF)
    {       
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        VME_EvtQueuePut(event, pEventParm);
    }    
    return 0;
}

#else
void initpub_delete_file(void)
{
}
void paintpub_delete_file(void)
{
}
void exitpub_delete_file(void)
{
}
unsigned char handlepub_delete_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{    
    AK_DEBUG_OUTPUT("enter pub_delete_file\n");
    m_triggerEvent(M_EVT_EXIT, NULL);
    return 0;
}

#endif

/* end of file */
