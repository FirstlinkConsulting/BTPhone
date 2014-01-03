/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_explorer_menu.c
 * @BRIEF set explorer menu
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-05-09
 * @Version£º
**************************************************************************/

#include "m_event.h"
#include "Ctrl_ListMenu.h"
#include "Ctrl_ListFile.h"
#include "M_Event_Api.h"
#include "Ctrl_Dialog.h"

#if(NO_DISPLAY == 0)

#define MMI_MENU   0
#define MMI_DIALOG 1
typedef struct _SET_EXPLORER_MENU
{
    T_U8          curMMI;
    CListMenuCtrl menu;
    CListFileCtrl *pListFile;
    CDialogCtrl   dlg;
    T_U8          delMode;
}T_SET_EXPLORER_MENU;

#if 0
static T_SET_EXPLORER_MENU *pSetExplorerMenu = AK_NULL;
#endif
void initset_explorer_menu(void)
{
#if 0
    pSetExplorerMenu = (T_SET_EXPLORER_MENU*)Fwl_Malloc(sizeof(T_SET_EXPLORER_MENU));
    AK_ASSERT_VAL_VOID(pSetExplorerMenu, "alloc pSetExplorerMenu structure error!");

    pSetExplorerMenu->pListFile = AK_NULL;
    pSetExplorerMenu->curMMI = MMI_MENU;
    ListMenu_Init(&pSetExplorerMenu->menu, eRES_STR_FILE_EXPLORER_OPER, eRES_STR_DELETE_CURRENTFILE,eRES_STR_DELETE_ALLFILE, eRES_IMAGE_MENUICON);
#endif
}

void exitset_explorer_menu(void)
{
#if 0
    Dialog_Free(&pSetExplorerMenu->dlg);
    pSetExplorerMenu = Fwl_Free(pSetExplorerMenu);
#endif
}

void paintset_explorer_menu(void)
{
#if 0
    if (pSetExplorerMenu->curMMI == MMI_MENU)
    {
        ListMenu_Show(&pSetExplorerMenu->menu);
    }
    else if (pSetExplorerMenu->curMMI == MMI_DIALOG)
    {
        Dialog_Show(&pSetExplorerMenu->dlg);
    }
#endif
}

unsigned char handleset_explorer_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#if 0
    T_U16  ret;
    T_BOOL bFile;

    if(M_EVT_1 == event)
    {
        pSetExplorerMenu->pListFile = (CListFileCtrl*)pEventParm->p.pParam1;
    }
    
    
    AK_DEBUG_OUTPUT("handleset_explorer_menu\n");
    if (pSetExplorerMenu->curMMI == MMI_MENU)
    {
            
        ret = ListMenu_Handler(&pSetExplorerMenu->menu, event, pEventParm);
    
        switch(ret)
        {
            case CTRL_RESPONSE_QUIT:
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
                break;

            case eRES_STR_DELETE_CURRENTFILE:
                pSetExplorerMenu->delMode = CTRL_LISTFILE_DELETE_FOCUS;
                Dialog_Init(&pSetExplorerMenu->dlg, ListFile_GetFocusFile(pSetExplorerMenu->pListFile, &bFile), CTRL_DIALOG_RESPONSE_NO|CTRL_DIALOG_RESPONSE_YES, eRES_STR_DELETE_CURRENTFILE, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO);
                pSetExplorerMenu->curMMI = MMI_DIALOG;
                Dialog_SetRefresh(&pSetExplorerMenu->dlg);
                break;
            case eRES_STR_DELETE_ALLFILE:
                pSetExplorerMenu->delMode = CTRL_LISTFILE_DELETE_ALL;
                Dialog_Init(&pSetExplorerMenu->dlg, AK_NULL, CTRL_DIALOG_RESPONSE_NO|CTRL_DIALOG_RESPONSE_YES, eRES_STR_DELETE_ALLFILE, ICON_HINT_ENTRYPTR, CTRL_DIALOG_RESPONSE_NO);
                pSetExplorerMenu->curMMI = MMI_DIALOG;
                Dialog_SetRefresh(&pSetExplorerMenu->dlg);
                break;
            default:
                break;
        }
    }
    else if (pSetExplorerMenu->curMMI == MMI_DIALOG)
    {
        
        ret = Dialog_Handler(&pSetExplorerMenu->dlg, event, pEventParm);

        switch(ret)
        {
            case CTRL_RESPONSE_QUIT:
                pSetExplorerMenu->curMMI = MMI_MENU;
                ListMenu_SetRefresh(&pSetExplorerMenu->menu);
                break;
            case CTRL_RESPONSE_NONE:
                //nothing to do
                break;

            default:            
                if (CTRL_DIALOG_RESPONSE_YES == ret)
                {
                    ListFile_DeleteItem(pSetExplorerMenu->pListFile, pSetExplorerMenu->delMode);
                }
                m_triggerEvent(M_EVT_EXIT, AK_NULL);
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
void initset_explorer_menu(void)
{
}
void paintset_explorer_menu(void)
{
}
void exitset_explorer_menu(void)
{
}
unsigned char handleset_explorer_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_explorer_menu\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

