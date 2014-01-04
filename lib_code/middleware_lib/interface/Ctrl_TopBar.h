/****************************************
*file name: Ctrl_TopBar.h
*brief:     this module is use to show the top content
*author:        li_shengkai
*data:      2011-3-23
****************************************/

#ifndef __TOPBAR_CTRL_H__
#define __TOPBAR_CTRL_H__

#include "Ctrl_Public.h"    

#define TOPBAR_REFLESH_NONE         0x00
#define TOPBAR_REFLESH_TITLE        0x01
#define TOPBAR_REFLESH_TIME         0x02
#define TOPBAR_REFLESH_MUSICICON    0x04
#define TOPBAR_REFLESH_BATTERY      0x08
#define TOPBAR_REFLESH_BKIMG        0x10
#define TOPBAR_REFLESH_ALL          0xff

#define TOPBAR_FOCUS_NONE           0x00
#define TOPBAR_FOCUS_TITLE          0x01
#define TOPBAR_FOCUS_TIME           0x02
#define TOPBAR_FOCUS_MUSIC          0x04
#define TOPBAR_FOCUS_BAT            0x08
#define TOPBAR_FOCUS_TOPBAR         0x10
#define TOPBAR_FOCUS_ALL            0xff

#define TOPBAR_RESPONSE_OPERATED    1
#define TOPBAR_RESPONSE_NONE        -1
#define TOPBAR_INVALID_TITLEID      (eRES_STR_NUM + 1)

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#define TOPBAR_BK_IMG               eRES_IMAGE_PUB_TOPBAR_BK
#define TOPBAR_MUSIC_ICON           eRES_IMAGE_PUB_MUSIC_ICON
#endif

#define TOPBAR_LEFT                 CTRL_WND_LEFT
#define TOPBAR_TOP                  CTRL_WND_TOP
#define TOPBAR_TITLE_LEFT           TOPBAR_LEFT+5
#define TOPBAR_TITLE_TOP            TOPBAR_TOP+5
#define TOPBAR_SYSTIME_LEFT         TOPBAR_LEFT+137
#define TOPBAR_SYSTIME_TOP          TOPBAR_TOP+5
#define TOPBAR_MUSIC_ICON_LEFT      TOPBAR_LEFT+262
#define TOPBAR_MUSIC_ICON_TOP       TOPBAR_TOP+4
#define TOPBAR_BAT_ICON_LEFT        TOPBAR_LEFT+296
#define TOPBAR_BAT_ICON_TOP         TOPBAR_TOP+5

#define TOPBAR_TIME_IMAGE_KINDS     4
#define TOPBAR_TIME_IMAGE_NUMS      10
#define TOPBAR_BAT_IMAGE_NUMS       5
#define TOPBAR_REFLESH_MAX_COUNT    5


typedef struct
{
    T_U8 bTopbarShow:1;
    T_U8 bTitleShow:1;
    T_U8 bSysTimeShow:1;
    T_U8 bBatteryShow:1;
    T_U8 TimeCount:3;
    T_U8 bTchDwn:1;
    T_U8 reflesh;
    T_U8 oldBatterVal;
    T_U8 oldSysTimeMin;
    T_U8 oldTitleWidth;
    T_U8 focusWhat;
    #ifdef OS_ANYKA
    T_U8 disBatLev;
    #endif
    T_U16 titleID;
} T_TOP_BAR_CTRL;


T_BOOL  TopBar_init(T_TOP_BAR_CTRL *topbarctrl ,T_U16 titleID ,T_BOOL  bSysTimeShow ,T_BOOL  bBatteryShow);

T_BOOL  TopBar_show(T_TOP_BAR_CTRL *topbarctrl );

T_U32   TopBar_Handle(T_TOP_BAR_CTRL  *topbarctrl , T_EVT_CODE  Event, T_EVT_PARAM  *pParam);

T_VOID  TopBar_Free(T_TOP_BAR_CTRL  *topbarctrl);

T_BOOL  TopBar_ConfigbShowSet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL bshow);
T_BOOL  TopBar_ConfigTitleSet(T_TOP_BAR_CTRL  *topbarctrl ,T_U16 title);
T_BOOL  TopBar_ConfigSysTimeSet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL  bShow);
T_BOOL  TopBar_ConfigBatterySet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL  bShow);
T_BOOL  TopBar_SetReflesh(T_TOP_BAR_CTRL  *topbarctrl ,T_U8 reflesh);
T_BOOL  TopBar_ConfigFocusSet(T_TOP_BAR_CTRL  *topbarctrl ,T_U8 Focus);
T_VOID  TopBar_DispBatIcon(T_VOID);

#endif

