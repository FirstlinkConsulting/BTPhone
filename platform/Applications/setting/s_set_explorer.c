/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_explorer.c
 * @BRIEF explorer
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-22
 * @Version£º
**************************************************************************/
#include "m_event.h"
#include "M_Event_Api.h"
#include "Eng_Profile.h"
#include "Eng_DataConvert.h"
#include "Fwl_Keypad.h"
#include "Fwl_osFS.h"
#include "Eng_String_UC.h"
#include "Ctrl_ListFile.h"

#if(NO_DISPLAY == 0)

#define MMI_FILE_INFO 1
#define MMI_FILE_LIST 0
#define IFNO_LEN      16

#define SET_EXPLORER_REFRESH_NONE 0x00
#define SET_EXPLORER_REFRESH_ALL  0xff

#define STR_FILE_DATE_LEFT_POS (FONT_WIDTH*3)
#define STR_FILE_TIME_LEFT_POS  STR_FILE_DATE_LEFT_POS
#define STR_FILE_DATE_TOP_POS  FONT_HEIGHT
#define STR_FILE_TIME_TOP_POS  (FONT_HEIGHT<<1)
#define STR_FILE_LEN_TOP_POS   (GRAPH_HEIGHT-FONT_HEIGHT)
#define STR_FILE_LEN_LEFT_POS  (FONT_WIDTH*3)
#define STR_FILE_DATE_TOP_POS  FONT_HEIGHT

typedef struct _SET_EXPLORER
{
    CListFileCtrl explorer;
    T_U16 *       pFileName;
    T_U8          curMMI;
    T_U8          refreshFlag;
}T_SET_EXPLORER;

#if 0
static T_SET_EXPLORER *pSetExplorer = AK_NULL;
#endif
#if 0

static T_BOOL SetExplorer_SetRefresh(T_U8 refresh)
{

    AK_ASSERT_PTR(pSetExplorer, "SetExplorer_SetRefresh(): pSetExplorer null", AK_FALSE);
    
    if (SET_EXPLORER_REFRESH_NONE != refresh)
    {
        pSetExplorer->refreshFlag |= refresh;
    }
    else
    {
        pSetExplorer->refreshFlag = refresh;
    }
    return AK_TRUE;

}


static T_VOID SetExplorer_ShowFileInfo()
{

    T_hFILE fHandle = FS_INVALID_HANDLE;
    T_U32 fileLen = 0;
    T_U16 uStrTime[IFNO_LEN] = {0};
    T_U16 uStrDate[IFNO_LEN] = {0};
    T_U16 uLenInfo[22] = {0};
    T_S8 i = 0;
    T_U8 endPos = 0;
    T_S8 count = 0;
    T_FILETIME fileTime;
    T_U16 uStrTmp[3] = {0};

    if (pSetExplorer==AK_NULL || pSetExplorer->pFileName==AK_NULL)
    {
        return;
    }

    if (SET_EXPLORER_REFRESH_ALL == pSetExplorer->refreshFlag)
    {
#if (USE_COLOR_LCD)
        T_BG_PIC pic;
#endif
        i = (T_S8)Utl_UStrRevFndChr(pSetExplorer->pFileName, UNICODE_SOLIDUS, 0);

        fHandle = Fwl_FileOpen(pSetExplorer->pFileName, _FMODE_READ, _FMODE_READ);
        fileLen = Fwl_GetFileLen(fHandle);
        Fwl_GetFileCreatTime(fHandle, &fileTime);
        Fwl_FileClose(fHandle);

        Utl_UItoa(fileLen, uStrDate, 10);

        i = (T_S8)Utl_UStrLen(uStrDate);
        endPos = i + (i+2)/3 - 1; //format: XX,XXX,XXX
        uLenInfo[endPos--] = uStrDate[i--];
        while ((i>=0) && (i<IFNO_LEN))
        {
            count++;
            uLenInfo[endPos--] = uStrDate[i--];
            if ((count%4==3) && (i>=0))
            {
                uLenInfo[endPos--] = (T_U16)',';
                count++;
            }
        }
        Eng_MultiByteToWideChar(" B", 3,uStrTmp, sizeof(uStrTmp), AK_NULL);
        Utl_UStrCat(uLenInfo, uStrTmp);
        // foramt:XXXX/XX/XX
        
        Utl_UItoa(fileTime.year, uStrDate, 10);
        uStrDate[4] = UNICODE_SOLIDUS;
        uStrDate[5] = fileTime.month/10 + UNICODE_0;
        uStrDate[6] = fileTime.month%10 + UNICODE_0;
        uStrDate[7] = UNICODE_SOLIDUS;
        uStrDate[8] = fileTime.day/10 + UNICODE_0;
        uStrDate[9] = fileTime.day%10 + UNICODE_0;
        uStrDate[10] = 0;
        // format:XX:XX:XX
        uStrTime[0] = fileTime.hour/10 + UNICODE_0;
        uStrTime[1] = fileTime.hour%10 + UNICODE_0;
        uStrTime[2] = UNICODE_COLON;
        uStrTime[3] = fileTime.minute/10 + UNICODE_0;
        uStrTime[4] = fileTime.minute%10 + UNICODE_0;
        uStrTime[5] = UNICODE_COLON;
        uStrTime[6] = fileTime.second/10 + UNICODE_0;
        uStrTime[7] = fileTime.second%10 + UNICODE_0;
        uStrTime[8] = 0;

#if (USE_COLOR_LCD)
        Eng_ImageResDisp(0, 0, eRES_IMAGE_SET_BK, 0);
        pic.resId   = eRES_IMAGE_SET_BK;
        pic.hOffset = (T_POS)0;
        pic.vOffset = (T_POS)0;
        DispStringOnPic(CP_UNICODE, 0, 0, CONVERT2STR(&pSetExplorer->pFileName[i+1]), FONT_WND_WIDTH, CLR_WHITE, &pic);    

        pic.hOffset = (T_POS)0;
        pic.vOffset = (T_POS)STR_FILE_LEN_TOP_POS;
        DispStringOnPic(CP_UNICODE, 0, (T_POS)STR_FILE_LEN_TOP_POS, CONVERT2STR(eRES_STR_FILE_LENGTH), FONT_WND_WIDTH, CLR_WHITE, &pic);    

        pic.hOffset = (T_POS)STR_FILE_LEN_LEFT_POS;
        pic.vOffset = (T_POS)STR_FILE_LEN_TOP_POS;
        DispStringOnPic(CP_UNICODE, (T_POS)STR_FILE_LEN_LEFT_POS, (T_POS)STR_FILE_LEN_TOP_POS, CONVERT2STR(uLenInfo), FONT_WND_WIDTH, CLR_WHITE, &pic);    

        pic.hOffset = (T_POS)0;
        pic.vOffset = (T_POS)STR_FILE_DATE_TOP_POS;
        DispStringOnPic(CP_UNICODE, 0, (T_POS)STR_FILE_DATE_TOP_POS, CONVERT2STR(eRES_STR_FILE_TIME), FONT_WND_WIDTH, CLR_WHITE, &pic);    

        pic.hOffset = (T_POS)STR_FILE_DATE_LEFT_POS;
        pic.vOffset = (T_POS)STR_FILE_DATE_TOP_POS;
        DispStringOnPic(CP_UNICODE, (T_POS)STR_FILE_DATE_LEFT_POS, (T_POS)STR_FILE_DATE_TOP_POS, CONVERT2STR(uStrDate), FONT_WND_WIDTH, CLR_WHITE, &pic);    

        pic.hOffset = (T_POS)STR_FILE_TIME_LEFT_POS;
        pic.vOffset = (T_POS)STR_FILE_TIME_TOP_POS;
        DispStringOnPic(CP_UNICODE, (T_POS)STR_FILE_TIME_LEFT_POS, (T_POS)STR_FILE_TIME_TOP_POS, CONVERT2STR(uStrTime), FONT_WND_WIDTH, CLR_WHITE, &pic);    
#else      
        Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
        DispStringInWidth(CP_UNICODE, (T_POS)0, 0, CONVERT2STR(&pSetExplorer->pFileName[i+1]), FONT_WND_WIDTH, CLR_WHITE);
        DispStringInWidth(CP_UNICODE, (T_POS)0, (T_POS)STR_FILE_LEN_TOP_POS, CONVERT2STR(eRES_STR_FILE_LENGTH), FONT_WND_WIDTH, CLR_WHITE);
        DispStringInWidth(CP_UNICODE, (T_POS)STR_FILE_LEN_LEFT_POS, (T_POS)STR_FILE_LEN_TOP_POS, CONVERT2STR(uLenInfo), FONT_WND_WIDTH, CLR_WHITE);
        DispStringInWidth(CP_UNICODE, (T_POS)0, STR_FILE_DATE_TOP_POS, CONVERT2STR(eRES_STR_FILE_TIME), FONT_WND_WIDTH, CLR_WHITE);
        DispStringInWidth(CP_UNICODE, (T_POS)STR_FILE_DATE_LEFT_POS, (T_POS)STR_FILE_DATE_TOP_POS, CONVERT2STR(uStrDate), FONT_WND_WIDTH, CLR_WHITE);
        DispStringInWidth(CP_UNICODE, (T_POS)STR_FILE_TIME_LEFT_POS, (T_POS)STR_FILE_TIME_TOP_POS, CONVERT2STR(uStrTime), FONT_WND_WIDTH, CLR_WHITE);
#endif

        SetExplorer_SetRefresh(SET_EXPLORER_REFRESH_NONE);
    }

}
#endif

void initset_explorer(void)
{
#if 0
    T_U16 diskPath[] = {'A',':','/','\0'};
    
    pSetExplorer = (T_SET_EXPLORER*)Fwl_Malloc(sizeof(T_SET_EXPLORER));
    AK_ASSERT_VAL_VOID(pSetExplorer, "alloc pSetExplorer structure error!");

    ListFile_Init(&pSetExplorer->explorer, AK_NULL, CTRL_LISTFILE_DISP_ALL, AK_NULL/*filter*/);
    pSetExplorer->curMMI = 0;
    pSetExplorer->pFileName = AK_NULL;
    pSetExplorer->refreshFlag = SET_EXPLORER_REFRESH_ALL;
    //Aud_listExtSetNeedUpdate();
#endif
}

void exitset_explorer(void)
{
#if 0
    ListFile_Free(&pSetExplorer->explorer);
    pSetExplorer = Fwl_Free(pSetExplorer);
#endif
}

void paintset_explorer(void)
{
#if 0
    if (MMI_FILE_LIST == pSetExplorer->curMMI)
    {
        ListFile_Show(&pSetExplorer->explorer); 
    }
    else
    {
        SetExplorer_ShowFileInfo();
    }
#endif
}

unsigned char handleset_explorer(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#if 0
    T_U32 ret;
    
    if (MMI_FILE_LIST == pSetExplorer->curMMI)
    {
        if (M_EVT_EXIT == event)
        {
            ListFile_SetRefresh(&pSetExplorer->explorer);
        }
        
        ret = ListFile_Handler(&pSetExplorer->explorer, event, pEventParm);
        switch(ret)
        {
            case CTRL_RESPONSE_QUIT:
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;
            case CTRL_RESPONSE_NONE:
                //do nothing
                break;
            case CTRL_LISTFILE_RESPONSE_DELETE:
                pEventParm->p.pParam1 = (T_pVOID)&pSetExplorer->explorer;
                m_triggerEvent(M_EVT_1, pEventParm);
                break;              
            default:
                pSetExplorer->pFileName = (T_U16*)ret;
                pSetExplorer->curMMI = MMI_FILE_INFO;
                SetExplorer_SetRefresh(SET_EXPLORER_REFRESH_ALL);
                break;
        }
    }
    else
    {
        if (M_EVT_USER_KEY == event)
        {
            if (kbOK == (T_eKEY_ID)pEventParm->c.Param1)
            {
                if (PRESS_SHORT == (T_ePRESS_TYPE)pEventParm->c.Param2)
                {
                    pSetExplorer->curMMI = MMI_FILE_LIST;
                    ListFile_SetRefresh(&pSetExplorer->explorer);

                }
                else
                {
                    m_triggerEvent(M_EVT_EXIT, AK_NULL);
                }
            }
        }
    }
#endif    

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_explorer(void)
{
}
void paintset_explorer(void)
{
}
void exitset_explorer(void)
{
}
unsigned char handleset_explorer(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_explorer\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

