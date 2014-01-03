#ifndef __CLISTMENU_CTRL_H__
#define __CLISTMENU_CTRL_H__

#include "Ctrl_Public.h"
#include "Ctrl_Button.h"
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif

#define CTRL_LISTMENU_STYLE_DEFAULT         (CTRL_LISTMENU_STYLE_QUIT_DISPSHOW | CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW)
#define CTRL_LISTMENU_STYLE_QUIT_DISPNONE   0x00000002  //不自动添加'EXIT'菜单
#define CTRL_LISTMENU_STYLE_QUIT_DISPSHOW   0x00000001  //自动添加'EXIT'菜单，且显示在最后
#define CTRL_LISTMENU_STYLE_SCROLLVER_DISPNONE  0x00000004  //不在菜单右测显示滚动条
#define CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW  0x00000010  //在菜单右测显示滚动条
#define CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT    0x00000100

#define LMENU_ITEM_HEIGHT               CTRL_WND_LINEHIGH

#if(NO_DISPLAY == 0)

typedef struct tagCListMenuCtrl
{
    T_U16   titleID;
    T_U16   startID;        //menu resource string begin id
    T_U16   stopID;         //menu resource string end id
    T_U16   navigateID;     //navigate bar menu id
    T_RES_IMAGE iconPtr;
    T_U16   begSrID;        //current display entry pos within LCD 
    T_U16   curSubID;       //current focused entry pos within LCD
    T_U16   refresh;        //repaint flag
    T_U16   refreshEx;      //only repaint small region flag

    T_U32   dwStyle;        //display style
    T_U16   **stringArray;  //dynamic menu string array
    T_U16   cmmIcon;        //dynamic menu common icon
    T_U16   hitIcon;        //dynamic menu focused icon
    T_U16   code;           //menu entry string color
    T_U16   *idLoc;

    T_U16   entryLines;     //usable LCD lines to display file or fold entry
    T_U16   bkimgTitle;

    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height;

    CTxtScrollCtrl *pTxtScroll;
    T_BOOL  bResetTxtScroll;
    TP_MListStringGet pStringGet;

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL  listmenu_topbar;
#endif

}CListMenuCtrl;


T_BOOL  ListMenu_InitEx(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE menuIconPtr, T_POS left, T_POS top, T_LEN width, T_LEN height);
#define ListMenu_Init(pListMenu, titleID, menuBegID, menuEndID, menuIconPtr) ListMenu_InitEx((pListMenu), (titleID), (menuBegID), (menuEndID), (menuIconPtr), CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT)
                
T_VOID  ListMenu_SetStyle(CListMenuCtrl *pListMenu, T_U32 dwStyle);
T_VOID  ListMenu_Show(CListMenuCtrl *pListMenu);
T_U16   ListMenu_Handler(CListMenuCtrl *pListMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID  ListMenu_SetRefresh(CListMenuCtrl *pListMenu);

/***********************************************************************************
**brief:        设置焦点条目
**parm:         focusID:焦点菜单的资源ID
***********************************************************************************/
T_BOOL  ListMenu_SetFocus(CListMenuCtrl *pListMenu, T_U16 focusID);

/***********************************************************************************
**brief:        获取焦点条目
**return:       focusID:焦点菜单的资源ID
***********************************************************************************/
T_U16   ListMenu_GetFocus(CListMenuCtrl *pListMenu);

/***********************************************************************************
**brief:        设置动态菜单信息
                该菜单项不是来自资源配置    
***********************************************************************************/
T_BOOL  ListMenu_SetEntryEx(CListMenuCtrl *pListMenu, T_U16 **string, T_U16 count, T_U16 cmmIcon, T_U16 hitIcon, T_U32 dwStyle, T_U16 codepage);

/***********************************************************************************
**brief:        设置菜单导航栏
                menuID: 导航菜单资源ID
***********************************************************************************/
T_VOID  ListMenu_SetNavigate(CListMenuCtrl *pListMenu, T_U16 menuID);

/***********************************************************************************
**brief:        设置菜单标题栏图片
                bkimgID: 背景图片资源ID
***********************************************************************************/
T_VOID  ListMenu_SetTitle(CListMenuCtrl *pListMenu, T_U16 bkimgID);

/***********************************************************************************
**brief:        删除菜单表项
***********************************************************************************/
T_BOOL  ListMenu_DeleteItem(CListMenuCtrl *pListMenu, T_U16 menuItem);

T_VOID  ListMenu_Free(CListMenuCtrl *pListMenu);

T_BOOL  ListMenu_SetFocusEx(CListMenuCtrl *pListMenu, T_U16 offset);

T_VOID LitMenu_SetStringGet(CListMenuCtrl *pListMenu, TP_MListStringGet pStringGet);
#endif
#endif

