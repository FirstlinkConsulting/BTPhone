/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_firmware_updata.c
 * @BRIEF display total memory and used memory
 * @Author£ºXP
 * @Date£º2008-06-05
 * @Version£º
**************************************************************************/
#include "Gbl_Global.h"
#include "m_event.h"
#include "M_Event_Api.h"
#include "Eng_ImageResDisp.h"
#include "Fwl_Keypad.h"
#include "Eng_DataConvert.h"
#include "log_file_com.h"
#include "Ctrl_Listfile.h"
#include "Ctrl_ListMenu.h"
#include "Gui_Common.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Fwl_System.h"
#include "Fwl_FreqMgr.h"
#include "Apl_Public.h"
//#include "Fwl_SelfUpdate.h"
//#include "VoiceMsg.h"
//#include "EngVoiceTip.h"

#if (UPDATA_USED == 0)

extern T_BOOL remap_lock_page(T_U32 vaddr, T_U32 size, T_BOOL lock);
extern T_BOOL FS_FormatDriver(T_U8 DriverID, E_FATFS FsType);
extern T_U32        Image$$update$$Base;
extern T_U32        Image$$update$$Limit;
#define update_base ((T_U32)(&Image$$update$$Base))
#define update_end  ((T_U32)(&Image$$update$$Limit))
#define UPDATE_FILE_PATH_LENGTH  32
#define UPDATA_FILE_PATH_CNT  (sizeof(g_upDataPath) / (sizeof(T_U16) * UPDATE_FILE_PATH_LENGTH))
#define UPDATA_FILE_NAND_PATH_CNT (UPDATA_FILE_PATH_CNT - 1)

#define  WATCHDOG_SET_LONG_TIME  175 // 115+5S = 180S
#define  WATCHDOG_DISABLE_LONG_TIME  0 // 1+5S = 6S

#if (STORAGE_USED == NAND_FLASH)
const T_U16 g_upDataPath[][UPDATE_FILE_PATH_LENGTH] =
{   
    {'A',':','/','u','p','d','a','t','e','.','u','p','d','\0'},
    {'B',':','/','u','p','d','a','t','e','.','u','p','d','\0'}, 
    {'C',':','/','u','p','d','a','t','e','.','u','p','d','\0'}, //mmc
};
#if(NO_DISPLAY == 0)
#if (USE_COLOR_LCD)
#define FIRMWARE_MENU_TITLE_LEFT     0
#define FIRMWARE_MENU_TITLE_TOP      0
#define FIRMWARE_MENU_TITLE_HIGHT    32   //title hight
#define FIRMWARE_MENU_TITLE_WIDTH          MAIN_LCD_WIDTH
#endif//#if (USE_COLOR_LCD)

typedef struct
{
    CListMenuCtrl menu;
    T_FileSel fileSel;
    T_BOOL  updateFlag;
}T_SET_FIRMWARE_UPDATA;

static T_SET_FIRMWARE_UPDATA *pFirmwareUpdata = AK_NULL;
#endif // #if(NO_DISPLAY == 0)


#else// #if (STORAGE_USED == NAND_FLASH)
const T_U16 g_upDataPath[][UPDATE_FILE_PATH_LENGTH] =
{
    {'A',':','/','u','p','d','a','t','e','.','u','p','d','\0'}, 
};

#endif//#if (STORAGE_USED == NAND_FLASH)

#pragma arm section code = "_update_"


static T_BOOL firmwareUpdata(T_hFILE hUpdateFile)
{
#ifdef OS_ANYKA

    AK_DEBUG_OUTPUT("start update\r\n");

    //start update
    if(!Fwl_SelfUpdateStart(hUpdateFile))
    {
        //update fail
        AK_DEBUG_OUTPUT("update fail\r\n");
        return AK_FALSE;
    }  
    
    AK_DEBUG_OUTPUT("update success\r\n");

    #if(NO_DISPLAY == 0)
    Gui_DispResHint(eRES_STR_UPDATING_SUCCESS,CLR_WHITE,CLR_BLACK,-1);
    #endif
    
    //Fwl_DelayUs(1000000);
#endif
	return AK_TRUE;
}


#if(NO_DISPLAY == 0)

void initset_firmware_updata(void)
{
    pFirmwareUpdata = (T_SET_FIRMWARE_UPDATA*)Fwl_Malloc(sizeof(T_SET_FIRMWARE_UPDATA));
    AK_ASSERT_PTR_VOID(pFirmwareUpdata, "pFirmwareUpdata alloc error\n");
    memset(pFirmwareUpdata, 0, sizeof(T_SET_FIRMWARE_UPDATA));
    pFirmwareUpdata->updateFlag = AK_FALSE;

#if (USE_COLOR_LCD)    
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(FIRMWARE_MENU_TITLE_LEFT, (T_LEN)FIRMWARE_MENU_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
#endif
    ListMenu_InitEx(&(pFirmwareUpdata->menu),eRES_STR_RADIO_EMPTY_STR,
        eRES_STR_MAIN_CATOLOG,eRES_STR_CARD2_CATOLOG,eRES_IMAGE_MENUICON,
        FIRMWARE_MENU_TITLE_LEFT, FIRMWARE_MENU_TITLE_HIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT - FIRMWARE_MENU_TITLE_HIGHT));        

    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&(pFirmwareUpdata->menu.listmenu_topbar), eRES_STR_UPDATA);
    #endif    
#else
    ListMenu_Init(&(pFirmwareUpdata->menu),eRES_STR_SYSCFG,
            eRES_STR_MAIN_CATOLOG,eRES_STR_CARD2_CATOLOG,eRES_IMAGE_MENUICON);            
#endif
    {
        if (!Fwl_MemDevIsMount(MMC_SD_CARD))
        {
            ListMenu_DeleteItem(&(pFirmwareUpdata->menu), (T_U16)(eRES_STR_CARD1_CATOLOG));
        }
    }
    ListMenu_SetFocus(&(pFirmwareUpdata->menu), eRES_STR_MAIN_CATOLOG);
}


void exitset_firmware_updata(void)
{
    if (pFirmwareUpdata != AK_NULL)
    {
        pFirmwareUpdata = Fwl_Free(pFirmwareUpdata);
    }
}

void paintset_firmware_updata(void)
{
    ListMenu_Show(&(pFirmwareUpdata->menu));
}

unsigned char handleset_firmware_updata(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U32 index;
    T_hFILE hUpdateFileHandle = FS_INVALID_HANDLE;      //updatepackage file handle
    T_U16 ret;
    T_UpdateRetCode retVal = INIT_FHA_FAIL;

    if (M_EVT_UPDATA == event)
    {
        return 0;
    }

     if (event == M_EVT_EXIT)
     {
         if(eFile == pFirmwareUpdata->fileSel.selResult)
        {
            AK_DEBUG_OUTPUT("Updata:");
            Printf_UC(pFirmwareUpdata->fileSel.file);
            PublicTimerStop();
            Fwl_FreqPush(FREQ_APP_MAX);
            Gui_DispResHint(eRES_STR_UPDATING,CLR_WHITE,CLR_BLACK,-1);

            //open update file
            hUpdateFileHandle = Fwl_FileOpen((T_pCWSTR)pFirmwareUpdata->fileSel.file, _FMODE_READ, _FMODE_READ);

            if(FS_INVALID_HANDLE == hUpdateFileHandle) 
            {
                PublicTimerStart();    
                m_triggerEvent(M_EVT_EXIT, AK_NULL);     //updatepakege can not open
                AK_DEBUG_OUTPUT("\nupdatepackage can not open\n");
                Fwl_FreqPop();
                return 0;
            }
            
            //init update environment
            retVal = Fwl_SelfUpdateInit(hUpdateFileHandle, AK_FALSE);
            AK_DEBUG_OUTPUT("\nfirmwareUpdata_Init retVal=%d\n", retVal);

            if (SUCCESS == retVal)
            {
                //begin update
                AK_DEBUG_OUTPUT("\n++++++++++update beginning, can release key++++++++++++++\n:");
                
                if (!firmwareUpdata(hUpdateFileHandle))
                {
                    Gui_DispResHint(eRES_STR_UPDATING_FAIL,CLR_WHITE,CLR_BLACK,-1);
                    Fwl_Freq_Clr_Add();
                }
                
                Fwl_FileClose(hUpdateFileHandle);          //close update file
            }
            else
            {
                Fwl_FileClose(hUpdateFileHandle);          //close update file
                PublicTimerStart();    
                m_triggerEvent(M_EVT_EXIT, AK_NULL);  
                Fwl_FreqPop();
                return 0;
            }


            AK_DEBUG_OUTPUT("\n++++++++++update finish reset++++++++++++++\n:");
#ifdef OS_ANYKA
            //reset system
            Fwl_SelfUpdateFinish();
#endif
        }         
     }

    ret = ListMenu_Handler(&pFirmwareUpdata->menu, event, pEventParm);
    
    switch(ret)
    {
       case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            break;
       case eRES_STR_MAIN_CATOLOG:
            Eng_MultiByteToWideChar("*.upd", Utl_StrLen("*.upd"), pFirmwareUpdata->fileSel.filter, CTRL_LISTFILE_FILTERLEN, AK_NULL);
            pFirmwareUpdata->fileSel.displayStyle = eDisplayAll;
            Utl_UStrCpy(pFirmwareUpdata->fileSel.file, g_diskPath);
            Fwl_GetSelfCurDir(pFirmwareUpdata->fileSel.file);
            pEventParm->p.pParam1= &(pFirmwareUpdata->fileSel);
            AK_DEBUG_OUTPUT("mdisk\r\n:");
            m_triggerEvent(M_EVT_MDISK_LIST, pEventParm);
            break;
            
#ifdef SUPPORT_SDCARD       
       case eRES_STR_CARD1_CATOLOG:           
       case eRES_STR_CARD2_CATOLOG:
            Eng_MultiByteToWideChar("*.upd", Utl_StrLen("*.upd"), pFirmwareUpdata->fileSel.filter, CTRL_LISTFILE_FILTERLEN, AK_NULL);
            pFirmwareUpdata->fileSel.displayStyle = eDisplayAll;

            index = (eRES_STR_CARD1_CATOLOG==ret)? 0: 1;
            Utl_UStrCpy(pFirmwareUpdata->fileSel.file, Fwl_MemDevGetPath(index, AK_NULL));
            Fwl_GetSelfCurDir(pFirmwareUpdata->fileSel.file);
            pEventParm->p.pParam1= &(pFirmwareUpdata->fileSel);
            AK_DEBUG_OUTPUT("mdisk\r\n:");
            if (!Fwl_MemDevIsMount(index))
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                return 0;
            }

            m_triggerEvent(M_EVT_MDISK_LIST, pEventParm);
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

#else
static T_BOOL firmwareUpdataFile_IsExist(T_U8 *index)
{
    T_U8 i = 0;
    T_U8 disk_index = 0;
    T_BOOL retVal = AK_TRUE;
#ifdef OS_ANYKA
    T_hFILE filehandle = FS_INVALID_HANDLE;

    for (i = 0; i < UPDATA_FILE_PATH_CNT; i++)
    {
        if ((UPDATA_FILE_PATH_CNT > (i + 1)) || Fwl_MemDevIsMount(MMC_SD_CARD))//the first path points to NandFlash disk
        {
            //check updatepackage 
            AK_DEBUG_OUTPUT("check file\n");
            Printf_UC(g_upDataPath[disk_index]);
            filehandle = Fwl_FileOpen(g_upDataPath[disk_index], _FMODE_READ, _FMODE_READ);
            if (FS_INVALID_HANDLE != filehandle)
            {
                 //updatepackage exist
                 AK_DEBUG_OUTPUT("+++++updata file is exist+++++\n", retVal);
                 Fwl_FileClose(filehandle);
                 break;
            }
            else
            {   
                disk_index++;
                AK_DEBUG_OUTPUT("get no file\n");
            }
        }
    }

    if (FS_INVALID_HANDLE == filehandle)//updatepackage is not exist
    {
        retVal = AK_FALSE;
        AK_DEBUG_OUTPUT("+++++updata file is not exist+++++\n", retVal);
    }
    
#endif
   
    *index = disk_index;
    return retVal;  
}

void initset_firmware_updata(void)
{
#ifdef SUPPORT_VOICE_TIP
    Voice_PlayTip(eBTPLY_SOUND_TYPE_UPDATE, AK_NULL);
    Voice_WaitTip();
#endif
    //set watch dog time to 3 minute
    Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);

}
void paintset_firmware_updata(void)
{
}
void exitset_firmware_updata(void)
{
    //clean watch dog long time
    Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
}
unsigned char handleset_firmware_updata(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef OS_ANYKA
    T_U8 index = 0;
    T_hFILE hUpdateFileHandle = FS_INVALID_HANDLE;      //updatepackage file handle
    T_UpdateRetCode retVal = INIT_FHA_FAIL;

    if (M_EVT_UPDATA == event)
    {
        PublicTimerStop();
        Fwl_FreqPush(FREQ_APP_MAX);
        
        //judge update file is exist
        if (AK_FALSE == firmwareUpdataFile_IsExist(&index))
        {
            PublicTimerStart(); 
            m_triggerEvent(M_EVT_EXIT, AK_NULL);     //no updatepakege
            AK_DEBUG_OUTPUT("\nno updatepackage\n");
            Fwl_FreqPop();
            return 0;
        }
        
        //open update file
        hUpdateFileHandle = Fwl_FileOpen((T_pCWSTR)g_upDataPath[index], _FMODE_READ, _FMODE_READ);

        if(FS_INVALID_HANDLE == hUpdateFileHandle) 
        {
            PublicTimerStart();    
            m_triggerEvent(M_EVT_EXIT, AK_NULL);     //updatepakege can not open
            AK_DEBUG_OUTPUT("\nupdatepackage can not open\n");
            Fwl_FreqPop();
            return 0;
        }
        
        //init update environment
        retVal = Fwl_SelfUpdateInit(hUpdateFileHandle, AK_FALSE);

        AK_DEBUG_OUTPUT("\nfirmwareUpdata_Init retVal=%d\n", retVal);

        if (SUCCESS == retVal)
        {
            //begin update
            AK_DEBUG_OUTPUT("\n++++++++++update beginning, please release key++++++++++++++\n");
            Fwl_DelayUs(1000000);
            firmwareUpdata(hUpdateFileHandle);
            Fwl_FileClose(hUpdateFileHandle);          //close update file
        }
        else    //file not match 
        {
            Fwl_FileClose(hUpdateFileHandle);          //close update file
            PublicTimerStart();    
            
            m_triggerEvent(M_EVT_EXIT, AK_NULL);  
            Fwl_FreqPop();
            return 0;
        }
       

        AK_DEBUG_OUTPUT("\n++++++++++update finish reset system++++++++++++++\n");

        //reset system
        Fwl_SelfUpdateFinish();
    }

    if (event >= M_EVT_Z00_POWEROFF)
    {
        return 1;
    }   
    else
    {
        return 0;
    }
#else   //ifdef win32
    m_triggerEvent(M_EVT_EXIT, AK_NULL);
    return 0;
#endif
          
}

#endif
#else
void initset_firmware_updata(void)
{
    
}
void paintset_firmware_updata(void)
{
}
void exitset_firmware_updata(void)
{
}
unsigned char handleset_firmware_updata(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}
#pragma arm section code

#endif



