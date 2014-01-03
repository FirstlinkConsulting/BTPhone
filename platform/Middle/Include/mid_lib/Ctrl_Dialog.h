#ifndef __CDIALOG_CTRL_H__
#define __CDIALOG_CTRL_H__

#include "Ctrl_Public.h"
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif
               
#define CTRL_DIALOG_RESPONSE_NO     0x01    //显示NO按钮
#define CTRL_DIALOG_RESPONSE_YES    0x02    //显YES按钮
#define CTRL_DIALOG_RESPONSE_HIDDEN 0xFF    //不显示任何按钮


typedef struct tagCDialogCtrl
{
    T_U16   modeRsp;            //响应类型：包含YES, NO, …选项;
    T_U16   titleStringID;
    T_U16   curSelect;          //tag current selected focus button id
    T_U16   cntRsp;             //amount of button number within the dialog
    T_U16   refresh;            //repaint flag
    T_U16   hintlines;          //amount lines of hint string
    T_RES_IMAGE iconPtr;        
    T_RES_IMAGE bkImage;        //background image
    T_U16   *hintString;        //附加提示信息字符串
    T_U8    fixLines;           //fixed dialog lines, eg. only 4 line in the middle of LCD
    T_U16   fontColor;          //hint string color
    T_U16   navigateID;         //navigate bar menu id
    T_U16   bkimgTitle;

    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL dialog_topbar;
#endif

}CDialogCtrl;

T_BOOL  Dialog_InitEx(CDialogCtrl *pDialog, T_U16 *hintString, T_U16 modeRsp, T_U16 titleStringID, T_RES_IMAGE iconPtr, T_U16 modeDefault, T_POS left, T_POS top, T_LEN width, T_LEN height);
#define Dialog_Init(pDialog, hintString, modeRsp, titleStringID, iconPtr, modeDefault)  Dialog_InitEx((pDialog), (hintString), (modeRsp), (titleStringID), (iconPtr), (modeDefault), CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT)

T_VOID  Dialog_Show(CDialogCtrl *pDialog);
T_U16   Dialog_Handler(CDialogCtrl *pDialog, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID  Dialog_SetRefresh(CDialogCtrl *pDialog);
T_BOOL  Dialog_FixedLines(CDialogCtrl *pDialog, T_U8 lines);//set dialog lines, default is full LCD
T_VOID  Dialog_SetBkImage(CDialogCtrl *pDialog, T_U16 imageID);//set dialog background image, default from configuration 
T_VOID  Dialog_SetFontColor(CDialogCtrl *pDialog, T_U16 color);//set dialog hint string color, default from configuration

T_VOID  Dialog_Free(CDialogCtrl *pDialog);

/***********************************************************************************
**brief:        设置菜单标题栏图片
                bkimgID: 背景图片资源ID
***********************************************************************************/
T_VOID  Dialog_SetTitle(CDialogCtrl *pDialog, T_U16 bkimgID);

/***********************************************************************************
**brief:        设置菜单导航栏
                menuID: 导航菜单资源ID
***********************************************************************************/
T_VOID  Dialog_SetNavigate(CDialogCtrl *pDialog, T_U16 menuID);


#endif


