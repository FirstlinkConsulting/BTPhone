/******************************************************************************************
**FileName      :      Ctrl_HorMenu.h
**brief         :      横向的菜单控件
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi  Yao
**date          :   2008-06-07
**version       :   1.0
*******************************************************************************************/
#ifndef __CHORMENU_CTRL_H__
#define __CHORMENU_CTRL_H__

#include "Ctrl_Public.h"

#define  MENU_INVALID                   0xFFFF
//控件刷新子标记
#define CTRL_REFRESH_HORMENU_NONE       0x0000
#define CTRL_REFRESH_HORMENU_ENTRY(i)   (1 << i)
#define CTRL_REFRESH_HORMENU_TITLE      0x0008
#define CTRL_REFRESH_HORMENU_ALL        0xFFFF

#define MENU_LINE_MAX           16
#define MENU_ROW_MAX            2
#define MENU_PER_LINE           3
#define MENU_ICON_WIDTH         16
#define MENU_ICON_HIGH          16
#define MENU_MARGIN_WIDTH       16
#define MENU_STRING_LEFT        32

typedef struct {
    T_RES_IMAGE MenuIcon[MENU_LINE_MAX][MENU_ROW_MAX];
    T_U16   begStrID;
    T_U16   MenuMax;    
    T_U16   CrnStartPos;
    T_U16   CurStrPos;
    T_U32   refresh;
}CHorMenuCtrls;


/***********************************************************************************
**brief:        initialize radiopalyer default param
**param:        pHorMenu: 控件控制结构体指针
**              menuBegID : 起始字符串号 ，menuEndID : 终止字符串号pMenuIcon : 图标资源数组
**              CurID: 初始化时指定当前激活的menu
***********************************************************************************/
T_BOOL  HorMenu_Init(CHorMenuCtrls *pHorMenu, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE *pMenuIcon,T_U16 CurID);


/***********************************************************************************
**brief:        显示刷新
**param:        pHorMenu: 控件控制结构体指针
***********************************************************************************/
T_VOID  HorMenu_Show(CHorMenuCtrls *pHorMenu);
T_VOID  HorMenu_SetRefresh(CHorMenuCtrls *pHorMenu);


/***********************************************************************************
**brief:        主处理函数，负责对按键的处理
**param:        pHorMenu: 控件控制结构体指针
***********************************************************************************/
T_U16   HorMenu_Handler(CHorMenuCtrls *pHorMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);

#endif

//end of file

