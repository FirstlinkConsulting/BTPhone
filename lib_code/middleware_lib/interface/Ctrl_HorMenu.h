/******************************************************************************************
**FileName      :      Ctrl_HorMenu.h
**brief         :      ����Ĳ˵��ؼ�
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi  Yao
**date          :   2008-06-07
**version       :   1.0
*******************************************************************************************/
#ifndef __CHORMENU_CTRL_H__
#define __CHORMENU_CTRL_H__

#include "Ctrl_Public.h"

#define  MENU_INVALID                   0xFFFF
//�ؼ�ˢ���ӱ��
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
**param:        pHorMenu: �ؼ����ƽṹ��ָ��
**              menuBegID : ��ʼ�ַ����� ��menuEndID : ��ֹ�ַ�����pMenuIcon : ͼ����Դ����
**              CurID: ��ʼ��ʱָ����ǰ�����menu
***********************************************************************************/
T_BOOL  HorMenu_Init(CHorMenuCtrls *pHorMenu, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE *pMenuIcon,T_U16 CurID);


/***********************************************************************************
**brief:        ��ʾˢ��
**param:        pHorMenu: �ؼ����ƽṹ��ָ��
***********************************************************************************/
T_VOID  HorMenu_Show(CHorMenuCtrls *pHorMenu);
T_VOID  HorMenu_SetRefresh(CHorMenuCtrls *pHorMenu);


/***********************************************************************************
**brief:        ��������������԰����Ĵ���
**param:        pHorMenu: �ؼ����ƽṹ��ָ��
***********************************************************************************/
T_U16   HorMenu_Handler(CHorMenuCtrls *pHorMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);

#endif

//end of file

