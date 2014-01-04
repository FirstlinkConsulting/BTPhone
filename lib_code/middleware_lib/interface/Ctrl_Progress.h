#ifndef __CPROGRESS_CTRL_H__
#define __CPROGRESS_CTRL_H__

#include "Ctrl_Public.h"
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif

/***********************************************************************************
**brief:        因为Progress_Handler函数返回的只能是整数
                而滑动点可能是浮点数，所以需要转换为10倍的整数传出，
                同理接收者也需要转换为0.1倍的实数
***********************************************************************************/
#define CTRL_PROGRESS_SWITCHINT2FLOAT(value)    ((T_FLOAT)((value)*0.1)) //整数转浮点数*0.1
#define CTRL_PROGRESS_SWITCHFLOAT2INT(value)    ((T_U16)((value)*10))    //浮点数转整数*10
#define CTRL_PROGRESS_SWITCHINT2INT(value)      ((T_U16)((value)/10))    //整数转整数/10

#define CTRL_PROGRESS_RESPONSE_CHANGED          0xFFFD


typedef struct tagCProgressCtrl
{
    T_S16   minValue;       //滚动条滑块最小点值
    T_S16   maxValue;       //滚动条滑块最大点值
    T_BOOL  bFloat;         //是否浮点数值显示
    T_U16   refresh;
    T_FLOAT stepIntvl;      //滑块每次跳跃间隔
    T_FLOAT curValue;       //滚动条滑块当前值
    T_RES_IMAGE iconPtr;    //左边显示的ICON
    T_RES_IMAGE riconPtr;   //右边显示的ICON
    T_RES_IMAGE bkImage;    //background image
    T_U16   titleStringID;
    T_POS   xBefore;        //滚动条滑块上次所处的坐标 <for only display small region>

    T_U32   firstData;      //特殊进度条有2行数据
    T_U32   secondData;
    T_U32   firstIcon;      //特殊进度条有2行数据，分别有不同ICON
    T_U32   secondIcon;
    T_BOOL  bScroll;        //当前状态受否允许移动进度条

    T_U16   navigateID;     //navigate bar menu id
    T_U16   bkimgTitle;

    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL pro_topbar;
#endif

}CProgressCtrl;

T_BOOL  Progress_InitEx(CProgressCtrl *pProgress, T_S16 minValue, T_S16 maxValue, T_U16 titleStringID, T_RES_IMAGE iconPtr, T_U16 nIntvl, T_FLOAT curValue, T_BOOL bFloat, T_RES_IMAGE riconPtr, T_RES_IMAGE bkImg, T_POS left, T_POS top, T_LEN width, T_LEN height);
#define Progress_Init(pProgress, minValue, maxValue, titleStringID, iconPtr, nIntvl, curValue, bFloat, riconPtr, bkImg) Progress_InitEx((pProgress), (minValue), (maxValue), (titleStringID), (iconPtr), (nIntvl), (curValue), (bFloat), (riconPtr), (bkImg), CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT)

T_VOID  Progress_Show(CProgressCtrl *pProgress);
T_U16   Progress_Handler(CProgressCtrl *pProgress, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID  Progress_SetRefresh(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:        获取当前滑块点值
***********************************************************************************/
T_BOOL  Progress_SetPos(CProgressCtrl *pProgress, T_FLOAT curValue);

/***********************************************************************************
**brief:        设置滑块点
**parm:         curValue:滑块点值
***********************************************************************************/
T_FLOAT Progress_GetPos(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:    设置特殊进度条的内容：
            包括2行字符串和各自的图标
***********************************************************************************/
T_BOOL  Progress_SetContent(CProgressCtrl *pProgress, T_U32 firstData, T_U16 firstIcon, T_U32 secondData, T_U16 secondIcon, T_BOOL bScroll);

T_VOID  Progress_Free(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:        设置菜单导航栏
                menuID: 导航菜单资源ID
***********************************************************************************/
T_VOID  Progress_SetNavigate(CProgressCtrl *pProgress, T_U16 menuID);

/***********************************************************************************
**brief:        设置菜单标题栏图片
                bkimgID: 背景图片资源ID
***********************************************************************************/
T_VOID  Progress_SetTitle(CProgressCtrl *pProgress, T_U16 bkimgID);


#endif

