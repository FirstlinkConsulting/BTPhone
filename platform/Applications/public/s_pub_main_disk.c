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
#include "Ctrl_ListFile.h"
#include "Fwl_FreqMgr.h"
#include "Log_record.h"
#include "Fwl_osMalloc.h"
#include "Gui_Common.h"

#if(NO_DISPLAY == 0)

#define TITLE_HIGHT  32

#if (USE_COLOR_LCD)
#define           MDISK_TITLE             eRES_STR_PUB_MAIN_DISK_LIST
#else
#define           MDISK_TITLE             INVALID_STRINGRES_ID
#endif

#if(USE_COLOR_LCD)
extern T_VOID VME_EvtQueueClearTimerEvent(T_VOID);

#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
static T_VOID PubMainDisk_PaintTitle()
{
     Eng_ImageResDisp(0, 0, (T_RES_IMAGE)GetUITitleResID(), AK_FALSE); 
}
#endif
#endif

static CListFileCtrl* pListFile = AK_NULL;
static T_FileSel* pFileSel = AK_NULL;
static T_BOOL isFstPaint;

#if(USE_COLOR_LCD)
static T_BOOL flashFlag= AK_FALSE;
#endif

#ifdef SUPPORT_SDCARD
T_MEM_DEV_ID FileList_GetCurDriver(T_VOID)
{
    return Fwl_MemDevGetDriver(pFileSel->file[0]);
}
#endif

void initpub_main_disk(void)
{
    Fwl_FreqPush(FREQ_APP_AUDIO_L5);
    pListFile= (CListFileCtrl *)Fwl_Malloc(sizeof(CListFileCtrl));
    AK_ASSERT_PTR_VOID(pListFile, AK_NULL);
    memset(pListFile, 0, sizeof(CListFileCtrl));
    pListFile->hFSFind = FS_INVALID_HANDLE;
    pFileSel= AK_NULL;
    isFstPaint= AK_FALSE;
}

void exitpub_main_disk(void)
{
    ListFile_Free(pListFile);
    pListFile= (CListFileCtrl *)Fwl_Free(pListFile);
    Fwl_FreqPop();
}

void paintpub_main_disk(void)
{
#if(USE_COLOR_LCD)
    if(flashFlag)
    {
        #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
        TopBar_SetReflesh(&pListFile->listfile_topbar, TOPBAR_REFLESH_TITLE);
        TopBar_ConfigTitleSet(&pListFile->listfile_topbar, GetUITitleStrID());
        #else
        PubMainDisk_PaintTitle();
        #endif
        flashFlag= AK_FALSE;
    }
#endif
    ListFile_Show(pListFile);
    if(!isFstPaint)
    {
        isFstPaint= AK_TRUE;
    }
}

unsigned char handlepub_main_disk(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U32 ret;
    T_BOOL isFile= AK_FALSE;
    T_BOOL bListRet;
    T_EVT_PARAM evtParam;
    if(AK_NULL == pEventParm)
    {
        pEventParm= &evtParam;
    }

    
    if(M_EVT_MDISK_LIST == event)
    {
        T_U16 shortName[MAX_FILE_LEN];
        T_BOOL isFile= AK_FALSE;
        T_hFILE fh;
        if(pEventParm->p.pParam1 == AK_NULL)
        {
            m_triggerEvent(M_EVT_EXIT, pEventParm); //exit
            return 0;
        }
        pFileSel= (PFileSel)pEventParm->p.pParam1;

        #ifdef SUPPORT_SDCARD
        {
            Printf_UC(pEventParm->p.pParam1);
            if ((!Fwl_MemDevIsMount(MMC_SD_CARD))&& ((FileList_GetCurDriver() == MMC_SD_CARD)))
            {
                pFileSel->selResult = eFail;
                m_triggerEvent(M_EVT_EXIT, pEventParm); //exit
                return 0;
            }
        }
        #endif

        AK_DEBUG_OUTPUT("\nhandlepub_main_disk M_EVT_MDISK_LIST == event\n");
        Printf_UC(pFileSel->file);

        fh= Fwl_FileOpen(pFileSel->file, _FMODE_READ, _FMODE_READ);
        if( fh != FS_INVALID_HANDLE)
        {
            isFile= AK_TRUE;
            Fwl_FileClose(fh);
            Fwl_GetShortPath(shortName, pFileSel->file);
            Fwl_GetSelfParentDir(pFileSel->file);
        }

        if(pFileSel->displayStyle ==eFolderOnly)
        {
            AK_DEBUG_OUTPUT("\nhandlepub_main_disk displayStyle == eFolderOnly\n");

            #if USE_COLOR_LCD
            bListRet = ListFile_InitEx(pListFile, pFileSel->file, eRES_STR_PUB_DEL_HINT, CTRL_LISTFILE_DISP_FOLDONLY, (T_U16* )pFileSel->filter
                ,0, TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- TITLE_HIGHT) );
             flashFlag= AK_TRUE;
            #else
            bListRet = ListFile_Init(pListFile, pFileSel->file, MDISK_TITLE, CTRL_LISTFILE_DISP_FOLDONLY, (T_U16* )pFileSel->filter);
            #endif
            //ListFile_InitShowMode(pListFile, 40, 5, DIAGRAM_MODE);
            #ifdef USE_HIDE_DRIVER
            if (AudioRecord_GetRecState() != eSTAT_REC_STOP)
            {
                ListFile_SetStyle(pListFile, (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW | CTRL_LISTFILE_STYLE_ROOTDISK_DISPNONE));
            }
            #endif
        }
        else
        {
            AK_DEBUG_OUTPUT("\nhandlepub_main_disk displayStyle != eFolderOnly\n");

            #if USE_COLOR_LCD
            bListRet=  ListFile_InitEx(pListFile, pFileSel->file, eRES_STR_PUB_DEL_HINT, CTRL_LISTFILE_DISP_ALL|CTRL_LISTFILE_DISP_PATTERONLY, (T_U16* )pFileSel->filter
                ,0, TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- TITLE_HIGHT) );
             flashFlag= AK_TRUE;
            #else
            bListRet=  ListFile_Init(pListFile, pFileSel->file, MDISK_TITLE, CTRL_LISTFILE_DISP_ALL|CTRL_LISTFILE_DISP_PATTERONLY, (T_U16* )pFileSel->filter);
            #endif
            //ListFile_InitShowMode(pListFile, 40, 5, DIAGRAM_MODE);
            while(bListRet != AK_TRUE)
            {
                if(!Fwl_GetSelfParentDir(pFileSel->file))
                {
                    //pEventParam is pFileSel
                    #ifdef SUPPORT_SDCARD
                    pEventParm->w.Param1 = FileList_GetCurDriver();                     
                    #endif
                    //need updata file list
                    //Aud_AudCtrSetListUpdata();
                    m_triggerEvent(M_EVT_NO_FILE, pEventParm);
                    return 0;//without paint
                }
                ListFile_Free(pListFile);
                #if USE_COLOR_LCD
                bListRet=  ListFile_InitEx(pListFile, pFileSel->file, eRES_STR_PUB_DEL_HINT, CTRL_LISTFILE_DISP_ALL|CTRL_LISTFILE_DISP_PATTERONLY, (T_U16* )pFileSel->filter
                    ,0, TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- TITLE_HIGHT) );
                 flashFlag= AK_TRUE;
                #else
                bListRet=  ListFile_Init(pListFile, pFileSel->file, MDISK_TITLE, CTRL_LISTFILE_DISP_ALL|CTRL_LISTFILE_DISP_PATTERONLY, (T_U16* )pFileSel->filter);
                #endif
                //ListFile_InitShowMode(pListFile, 40, 5, DIAGRAM_MODE);
            }

            if(isFile)
            {
               ListFile_SetCurFile(pListFile, shortName);
            }
#ifdef USE_HIDE_DRIVER
    #ifdef SUPPORT_SDCARD
            if (FileList_GetCurDriver() == MMC_SD_CARD)
            {
                ListFile_SetStyle(pListFile, (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW | CTRL_LISTFILE_STYLE_ROOTDISK_DISPNONE));
            }
            else
    #endif
            {
                ListFile_SetStyle(pListFile, (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW | CTRL_LISTFILE_STYLE_ROOTDISK_DISPSHOW));
            }
#endif
        }
        
        if (AK_FALSE == bListRet)
        {        
            pFileSel->selResult = eFail;
            m_triggerEvent(M_EVT_EXIT, pEventParm); //exit       
        }       
#if (LCD_TYPE == 3)
        VME_EvtQueueClearTimerEvent();
#endif
        return 1;
    }


    if(event == M_EVT_EXIT)
    {
//        flashFlag= AK_TRUE;
//        ListFile_SetRefresh(pListFile);
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        return 0;
    }

    if(event == M_EVT_USER_KEY)
    {
        if(pEventParm->c.Param1 == kbMODE && pEventParm->c.Param2 == PRESS_SHORT)
        {
            pFileSel->selResult= eFail;
            pEventParm->p.pParam1 = pFileSel;// save the origal 
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            return 0;
        }
    }

    if(event == M_EVT_RETURN)
    {
        event= M_EVT_USER_KEY;
        pEventParm->c.Param1 = kbMODE;
        pEventParm->c.Param2= PRESS_SHORT;
    }
    ret= ListFile_Handler(pListFile, event, pEventParm);
    switch(ret)
    {
    case CTRL_RESPONSE_QUIT:
        if(pFileSel != AK_NULL)
        {
            pFileSel->selResult= eFail;
        }
        pEventParm->p.pParam1= pFileSel;
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    case CTRL_RESPONSE_NONE:
        //do nothing
        break;
        
    default:
        if(ret != 0)
        {
            Utl_UStrCpy(pFileSel->file, (T_U16* )ret);
            ListFile_GetFocusFile(pListFile, &isFile);    
            if(isFile)
            {
                pFileSel->selResult= eFile;
            }
            else
            {
                pFileSel->selResult= ePath;
            }            
        }
        else
        {
            pFileSel->selResult= eFail;
        }
        
        pEventParm->p.pParam1= pFileSel;
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    }    
    if (event >= M_EVT_Z00_POWEROFF)
    {
        return 1;
    }
    else
        return 0;
}

#else //#if(NO_DISPLAY == 0)
void initpub_main_disk(void)
{
}
void exitpub_main_disk(void)
{
}
void paintpub_main_disk(void)
{
}
unsigned char handlepub_main_disk(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handlepub_main_disk\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}
#endif
/* end of file */