/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_disp_memory.c
 * @BRIEF display total memory and used memory
 * @Author：Huang_ChuSheng
 * @Date：2008-04-21
 * @Version：
**************************************************************************/
#include "Apl_Public.h"
#include "m_event.h"
#include "Eng_time.h"
#include "fwl_osfs.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "Eng_ImageResDisp.h"
#include "Ctrl_Dialog.h"
#include "Gui_Common.h"
#include <stdio.h>
#include "Ctrl_Progress.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Keypad.h"
#include "Fwl_osMalloc.h"
#include "M_event_api.h"

#if(NO_DISPLAY == 0)

#define M_SIZE (1<<20)
#define NEXT_INDEX(index, isConnectSD) {if(index == 0 && isConnectSD)  index= 1; else index= 0;}
#define IMG_MEMORY_LEFT_POS     8
#if (USE_COLOR_LCD)
#define BKCOLOR                 RGB_COLOR(168,213,243)
#define PRECOLOR                RGB_COLOR(2,80,126)
#define STR_TITLE_LEFT_POS      32
#define STR_TITLE_TOP_POS       0
#define STR_INFO_LEFT_POS       (FONT_WIDTH << 1)
#define STR_INFO_TOP_POS        48
#define RECT_TOP_POS            64
#define RECT_LEFT_POS           (FONT_WIDTH >> 1)
//#define RECT_WIDTH              (GRAPH_WIDTH - (RECT_LEFT_POS << 1))
//#define RECT_HEIGHT             (FONT_WIDTH >> 2)
#else
#define STR_INFO_LEFT_POS       (FONT_WIDTH << 1)
#define STR_INFO_TOP_POS        16
#define RECT_TOP_POS            32
#define ICON_MEM_TOP            32
#define ICON_MEM_LEFT           14
#define RECT_LEFT_POS           (FONT_WIDTH >> 1)
//#define RECT_WIDTH              (GRAPH_WIDTH - (RECT_LEFT_POS << 1))
//#define RECT_HEIGHT             (FONT_WIDTH >> 2)
#endif


#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_MEMINFO_DISP_TITLE_HIGHT    32   //title hight
#define SYS_MEMINFO_DISP_TITLE_WIDTH    128
#define SYS_MEMINFO_DISP_TITLE_LEFT     0
#define SYS_MEMINFO_DISP_TITLE_TOP      0
#endif


static const T_U16 m_diskSymbol[] ={
#ifdef OS_ANYKA
    UNICODE_B-UNICODE_A,    //exfat使用0~n作为盘符
    UNICODE_C-UNICODE_A
#else
    UNICODE_A-UNICODE_A,
    UNICODE_B-UNICODE_A
#endif
};

typedef struct 
{
#if(USE_COLOR_LCD)
    CProgressCtrl   progress ;
#endif
    T_U8 curDiskIndex;
    T_U8 isConnectSD;
    T_U8 delayCnt;
    T_U64_INT diskSize[2];
    T_U64_INT diskUsedSize[2];
}T_MemInfo;

static T_MemInfo* m_pMemInfo= AK_NULL;

static T_VOID ShowMemory()
{
    T_U32 totalSizeM = 0;
    T_U32 usedSizeM = 0;
    T_U16 usedPercent;

    #if (!USE_COLOR_LCD)
    T_CHR info[12] = {0};
    T_U16 uInfo[12] = {0};  
    T_U16   i;
    T_U8    IconNum =0;
    #endif
    
    //AK_DEBUG_OUTPUT("diskSize(high:0x%x,low:0x%x),diskUsedSize(high:0x%x,low:0x%x\r\n",
                    //diskSize.high, diskSize.low, diskUsedSize.high, diskUsedSize.low);
    Fwl_FreqPush(FREQ_APP_USB);

#if ((1 == LCD_HORIZONTAL)&&(3==LCD_TYPE))
    TopBar_ConfigTitleSet(&m_pMemInfo->progress.pro_topbar,eRES_STR_IMG_MDISK_LIST);
#endif
    if(m_pMemInfo->isConnectSD)
    {
        if (m_pMemInfo->curDiskIndex == 1)
        {
        #if ((1 == LCD_HORIZONTAL)&&(3==LCD_TYPE))
            TopBar_ConfigTitleSet(&m_pMemInfo->progress.pro_topbar,eRES_STR_IMG_CARD1_LIST);
        #endif
            if (m_pMemInfo->diskSize[1].high == 0 && m_pMemInfo->diskSize[1].low == 0)
            {
                Fwl_FsGetSize(m_diskSymbol[1], &m_pMemInfo->diskSize[1]);
                Fwl_FsGetUsedSize(m_diskSymbol[1], &m_pMemInfo->diskUsedSize[1]);
            }
        }
    }
    totalSizeM = m_pMemInfo->diskSize[m_pMemInfo->curDiskIndex].high * 4096 + m_pMemInfo->diskSize[m_pMemInfo->curDiskIndex].low/M_SIZE;
    if (totalSizeM == 0)
    {
        Fwl_FreqPop();
        return;
    }
        
    usedSizeM = m_pMemInfo->diskUsedSize[m_pMemInfo->curDiskIndex].high * 4096 + m_pMemInfo->diskUsedSize[m_pMemInfo->curDiskIndex].low/M_SIZE;
    usedPercent = (T_U16)(100 * usedSizeM / totalSizeM);
#if (USE_COLOR_LCD)
    Progress_SetPos(&(m_pMemInfo->progress), usedPercent);
    Progress_SetContent(&(m_pMemInfo->progress),
                        totalSizeM,
                        eRES_IMAGE_MEMORYM,
                        usedPercent,
                        eRES_IMAGE_SYSTEMB,
                        AK_FALSE);
    Progress_SetRefresh(&(m_pMemInfo->progress));
    //Progress_SetTitle(meminfo,GetUITitleResID());
    
#else
    IconNum = (T_U8)(100 * usedSizeM / totalSizeM)/4;
    if(IconNum == 25)
        IconNum = 24;

    sprintf(info, "%dM<%d%%>", (unsigned int)totalSizeM, usedPercent);
    Eng_MultiByteToWideChar(info, Utl_StrLen(info), uInfo, 12, AK_NULL);

    Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
    Eng_ImageResDisp(IMG_MEMORY_LEFT_POS, 0, eRES_IMAGE_MEMORY, AK_FALSE);

    Eng_ImageResDisp(ICON_MEM_LEFT, ICON_MEM_TOP, eRES_IMAGE_MEMBAR, AK_TRUE);
    if(IconNum)
        for(i = 0; i<IconNum; i++)
            Eng_ImageResDisp((T_POS)(ICON_MEM_LEFT+1+i*4), ICON_MEM_TOP, eRES_IMAGE_PBKGRD, AK_TRUE);


    DispStringInWidth(CP_UNICODE, (T_POS)STR_INFO_LEFT_POS, (T_POS)STR_INFO_TOP_POS, CONVERT2STR(uInfo), FONT_WND_WIDTH, CLR_WHITE);
#endif
    Fwl_FreqPop();
}


void initset_disp_memory(void)
{
    T_U32 i;
    m_pMemInfo = (T_MemInfo*)Fwl_Malloc(sizeof(T_MemInfo));
    AK_ASSERT_VAL_VOID(m_pMemInfo, "alloc meminfo structure error!");

     #if (USE_COLOR_LCD)
     Progress_InitEx(&(m_pMemInfo->progress),0, 100,
                eRES_STR_PUB_DEL_HINT,
                INVALID_IMAGERES_ID,
                100,
                0,
                AK_FALSE,
                INVALID_IMAGERES_ID,
                INVALID_IMAGERES_ID,
                SYS_MEMINFO_DISP_TITLE_LEFT,
                SYS_MEMINFO_DISP_TITLE_HIGHT,
                MAIN_LCD_WIDTH,
                MAIN_LCD_HEIGHT-SYS_MEMINFO_DISP_TITLE_HIGHT);
     #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
        Eng_ImageResDisp(SYS_MEMINFO_DISP_TITLE_LEFT, (T_U8)SYS_MEMINFO_DISP_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
     #endif
     #endif

    m_pMemInfo->curDiskIndex= 0;
    m_pMemInfo->diskSize[1].high = 0;
    m_pMemInfo->diskSize[1].low = 0;
    m_pMemInfo->delayCnt = 0;

    if(Fwl_MemDevIsMount(MMC_SD_CARD))
    {
        m_pMemInfo->isConnectSD = AK_TRUE;
    }

    //AK_DEBUG_OUTPUT("curDiskIndex:%d m_isConnectedSD:%d\n", m_curDiskIndex, m_isConnectSD);
    Fwl_FsGetSize(m_diskSymbol[0], &m_pMemInfo->diskSize[0]);
    Fwl_FsGetUsedSize(m_diskSymbol[0], &m_pMemInfo->diskUsedSize[0]);
    ShowMemory();
}

void exitset_disp_memory(void)
{
    //AK_DEBUG_OUTPUT("release memory\n");
#if(USE_COLOR_LCD)
    Progress_Free(&(m_pMemInfo->progress));
#endif
    m_pMemInfo= Fwl_Free(m_pMemInfo);
}

void paintset_disp_memory(void)
{
#if(USE_COLOR_LCD)
    Progress_Show(&(m_pMemInfo->progress));
#endif
}

unsigned char handleset_disp_memory(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{       
    if (M_EVT_USER_KEY == event)
    {
        switch (pEventParm->c.Param1)
        {
            case kbOK:
            case kbMODE:
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
            case kbLEFT:
            case kbRIGHT:    
                if(m_pMemInfo->isConnectSD)
                {
                    NEXT_INDEX(m_pMemInfo->curDiskIndex, m_pMemInfo->isConnectSD);  
                    AK_DEBUG_OUTPUT("curDiskIndex:%d\n", m_pMemInfo->curDiskIndex);
                    m_pMemInfo->delayCnt = 0;
                    ShowMemory();
                }
                break;
            default:
                break;
        }
    }

    if (event == M_EVT_PUB_TIMER)
    {
        m_pMemInfo->delayCnt++;

        if (m_pMemInfo->delayCnt > 6)
        {
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            return 0;
        }
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_disp_memory(void)
{
}
void paintset_disp_memory(void)
{
}
void exitset_disp_memory(void)
{
}
unsigned char handleset_disp_memory(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_disp_memory\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

