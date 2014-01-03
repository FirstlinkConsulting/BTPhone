
/**************************************************************
*
* Copyrights (C) 2005, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctrl_IconMenu.h
* File flag: Icon Menu
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2005-11-16
*
****************************************************************/

#ifndef __CTRL_ICONMENU_H__
#define __CTRL_ICONMENU_H__

#include "Ctrl_Public.h"

#define ICONMENU_ICON_ADJUST_TOP_MARGIN 8
#define ICONMENU_ICONSIZE_MIN_WIDTH     60
#define ICONMENU_ICONSIZE_MIN_HEIGHT    60

#define ICONMENU_ICON_NUM               1
#define ICONMENU_REFRESH_FOCUS          (0x04|0x10)


#define ICONMENU_ICON_FOCUS_BACK        RGB_COLOR(135, 206, 235)
#define ICONMENU_ICON_DEFAULT           0
#define ICONMENU_ITEMQTY_MAX            256
#define ICONMENU_ICONINTERVAL_DEFAULT   0
#define ICONMENU_ICONINTERVAL_MAX       32
#define ICONMENU_ATTACH_FONT_INTERVAL   2
#define ICONMENU_ANIMATETIME            700
#define ICONMENU_ICONTRANS_DEFAULT      40
#define ICONMENU_ERROR_ID               0xff
#define ICONMENU_ERROR_PLACE            0xff
#define ICONMENU_ICON_TEXT_COLOR        CLR_WHITE

#define ICONMENU_REFRESH_NONE           0x00
#define ICONMENU_REFRESH_ALL            0xff
#define ICONMENU_REFRESH_ICON_ATTACH    0x01
#define ICONMENU_REFRESH_ICON           0x02
#define ICONMENU_REFRESH_SCRBAR         0x10

#define ICONMENU_REFRESH_OTHER          0x08

#define ICONMENU_ALIGN_HORIZONTAL       0x0f
#define ICONMENU_ALIGN_LEFT             0x01
#define ICONMENU_ALIGN_RIGHT            0x02
#define ICONMENU_ALIGN_HCENTER          0x04
#define ICONMENU_ALIGN_VERTICAL         0xf0
#define ICONMENU_ALIGN_UP               0x10
#define ICONMENU_ALIGN_DOWN             0x20
#define ICONMENU_ALIGN_VCENTER          0x40

typedef T_VOID (*T_fICONMENU_CALLBACK)(T_VOID);

typedef enum {
    ICONMENU_DIRECTION_UP = 0,
    ICONMENU_DIRECTION_DOWN,
    ICONMENU_DIRECTION_LEFT,
    ICONMENU_DIRECTION_RIGHT,
    ICONMENU_DIRECTION_NUM
} T_ICONMENU_DIRECTION;

typedef struct _ICONMENU_RECT {
    T_RECT                  Rect;                           // rect
    struct _ICONMENU_RECT   *pNext;                         // next rect point
} T_ICONMENU_RECT;

typedef struct _ICONMENU_ITEM {
    T_U8                    Id;                             // icon item id
    T_S8                    Place;                          // icon item place
    T_U32                   ItemTextId;                     // item text id
    T_RES_IMAGE             IconId[ICONMENU_ICON_NUM];      // icon graph id
    struct _ICONMENU_ITEM   *pPrevious;                     // previous item point
    struct _ICONMENU_ITEM   *pNext;                         // next item point
} T_ICONMENU_ITEM;                                      

typedef struct {
    T_ICONMENU_ITEM         *pItemHead;                     // first (head) item point
    T_ICONMENU_ITEM         *pItemFocus;                    // focus item point
    T_ICONMENU_ITEM         *pOldItemFocus;                 // old focus item point
    T_RECT                  IconAttachtRect;                // item text show rect
    T_RECT                  IconRect;                       // icon show rect
    T_ICONMENU_RECT         *pOtherRect;                    // other show rect point
    T_U8                    ItemRow;                        // items row
    T_U8                    ItemCol;                        // items column

    T_U8                    IconImageNum;                   // icon image number
    T_U8                    IconImageDefault;               // icon image static number
    T_U32                   IconHInterval;                  // icon horizontal interval
    T_U32                   IconVInterval;                  // icon vertical interval
    T_U32                   ItemWidth;                      // item show width
    T_U32                   ItemHeight;                     // item show height
    T_U16                   ItemQty;                        // items quantity

    T_BOOL                  IconAttachPartRectFlag;         // item text back graph part flag
    T_BOOL                  IconPartRectFlag;               // icon back graph part flag
    T_RECT                  IconAttachPartRect;             // item text back graph part rect
    T_RECT                  IconPartRect;                   // icon bakc graph part rect
    T_RES_IMAGE             IconAttachBkImg;                // item text back graph id, if eRES_IMAGE_NUM, show item text back color
    T_RES_IMAGE             IconBackId;                     // icon back graph id, if eRES_IMAGE_NUM, show icon back color
    T_COLOR                 IconBackColor;                  // icon back color
    T_COLOR                 IconAttachTextColor;            // item text color
    T_COLOR                 IconAttachTextBackColor;        // item text back color
    T_U8                    IconAttachTextAlign;            // item text align

    T_BOOL                  IconShowFlag;                   // icon show flag
    T_U8                    RefreshFlag;                    // refresh flag

    T_U8                    IconAnimateCount;               // icon animate refresh count
    T_TIMER                 IconAnimateTimer;               // icon animate refresh timer id

    T_S32                   windowPlace;
    T_S32                   pageRow;                        // number of row in one page
    T_fICONMENU_CALLBACK    OtherShowCallBack;              // other show call back funtion
} T_ICONMENU;


/**
* @brief init IconMenu
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_RECT IconAttachRect: IconAttachRect
* @param in T_RECT IconRect: IconRect
*
* @return T_BOOL
* @retval 
*/
T_BOOL IconMenu_Init(T_ICONMENU *pIconMenu, T_RECT IconAttachRect, T_RECT IconRect);

/**
* @brief free IconMenu
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval 
*/
T_BOOL IconMenu_Free(T_ICONMENU *pIconMenu);

/**
* @brief show IconMenu
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval 
*/
T_BOOL IconMenu_Show(T_ICONMENU *pIconMenu);

/**
* @brief IconMenu handler
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_EVT_CODE Event: Event
* @param in T_EVT_PARAM *pParam: pParam
*
* @return T_U16
* @retval focus Id or ctrl special retval
*/
T_U16 IconMenu_Handler(T_ICONMENU *pIconMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);


/**
* @brief IconMenu add item
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: id
* @param in T_U8 Place: place
* @param in T_U32 ItemTextId: item text id
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_AddItem(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 Place, T_U32 ItemTextId);


/**
* @brief IconMenu reset item text
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: id
* @param in T_U32 ItemTextId: item text id
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_ResetItemText(T_ICONMENU *pIconMenu, T_U8 Id, T_U32 ItemTextId);

/**
* @brief IconMenu set item icon
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: item id
* @param in T_U8 index: icon index
* @param in T_RES_IMAGE IconId: Icon Id
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetItemIcon(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 index, T_RES_IMAGE IconId);

/**
* @brief delete item by id
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: item id
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_DelItemById(T_ICONMENU *pIconMenu, T_U8 Id);

/**
* @brief delete item by place
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Place: item Place
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_DelItemByPlace(T_ICONMENU *pIconMenu, T_U8 Place);

/**
* @brief move focus
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_ICONMENU_DIRECTION Dir: dir
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_MoveFocus(T_ICONMENU *pIconMenu, T_ICONMENU_DIRECTION Dir);

/**
* @brief set item focus
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: item id
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetItemFocus(T_ICONMENU *pIconMenu, T_U8 Id);

/**
* @brief set iconAttachRect
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_RECT IconAttachRect: IconAttachRect
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconAttachRect(T_ICONMENU *pIconMenu, T_RECT IconAttachRect);

/**
* @brief set iconRect
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_RECT IconRect: IconRect
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconRect(T_ICONMENU *pIconMenu, T_RECT IconRect);

/**
* @brief set item row and col qty
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 ItemRow: row qty
* @param in T_U8 ItemCol: col qty
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetItemMatrix(T_ICONMENU *pIconMenu, T_U8 ItemRow, T_U8 ItemCol);

/**
* @brief set item Matrix auto
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetItemMatrixAuto(T_ICONMENU *pIconMenu);

/**
* @brief set text style
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 TextAlign: TextAlign
* @param in T_COLOR TextColor: TextColor
* @param in T_COLOR TextBackColor: TextBackColor
* @param in T_RES_IMAGE TextBackData: Text Back image id
* @param in T_RECT *pTextPartRect: TextPartRect
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconAttachStyle(T_ICONMENU *pIconMenu, T_U8 TextAlign, T_COLOR TextColor, 
                                   T_COLOR TextBackColor, T_RES_IMAGE TextBackData, 
                                   T_RECT *pTextPartRect);

/**
* @brief set icon style
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U32 IconHInterval:  IconHInterval
* @param in T_U32 IconVInterval: IconVInterval
* @param in T_COLOR IconBackColor: IconBackColor
* @param in T_RES_IMAGE IconBackId: IconBackId
* @param in T_RECT *pIconPartRect: IconPartRect
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconStyle(T_ICONMENU *pIconMenu, T_U32 IconHInterval, T_U32 IconVInterval, 
                             T_COLOR IconBackColor, T_RES_IMAGE IconBackId, T_RECT *pIconPartRect);

/**
* @brief set icon style
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 IconImageNum:   Icon Image Num of one item
* @param in T_U8 IconImageDefault: default icon image index
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconImageNum(T_ICONMENU *pIconMenu, T_U8 IconImageNum, T_U8 IconImageDefault);

/**
* @brief set show flag
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_BOOL IconShowFlag:  IconShowFlag
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetIconShowFlag(T_ICONMENU *pIconMenu, T_BOOL IconShowFlag);

/**
* @brief set refresh flag
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 RefreshFlag: RefreshFlag
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetRefresh(T_ICONMENU *pIconMenu, T_U8 RefreshFlag);

/**
* @brief set OtherShowCallBack function
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_fICONMENU_CALLBACK OtherShowCallBack: OtherShowCallBack
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_SetOtherShowCallBack(T_ICONMENU *pIconMenu, T_fICONMENU_CALLBACK OtherShowCallBack);

/**
* @brief set OtherRect
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_RECT OtherRect: OtherRect
*
* @return T_BOOL
* @retval
*/
T_BOOL IconMenu_AddOtherRect(T_ICONMENU *pIconMenu, T_RECT OtherRect);


/**
* @brief get refresh flag
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_U8
* @retval refresh flag
*/
T_U8 IconMenu_GetRefreshFlag(T_ICONMENU *pIconMenu);

/**
* @brief get show flag
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval 
*/
T_BOOL IconMenu_GetIconShowFlag(T_ICONMENU *pIconMenu);

/**
* @brief get focus item
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_ICONMENU_ITEM *
* @retval focus item pointer
*/
T_ICONMENU_ITEM *IconMenu_GetFocusItem(T_ICONMENU *pIconMenu);

/**
* @brief get focus id
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_U8
* @retval focus id
*/
T_U8 IconMenu_GetFocusId(T_ICONMENU *pIconMenu);

/**
* @brief get focus place
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_U8
* @retval focus place
*/
T_U8 IconMenu_GetFocusPlace(T_ICONMENU *pIconMenu);

/**
* @brief get item id by point
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U16 x: x
* @param in T_U16 y: y
*
* @return T_U8
* @retval item id
*/
T_U8 IconMenu_GetIdByPoint(T_ICONMENU *pIconMenu, T_U16 x, T_U16 y);

/**
* @brief find item by id
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U8 Id: item id
*
* @return T_ICONMENU_ITEM *
* @retval item  pointer
*/
T_ICONMENU_ITEM *IconMenu_FindItemById(T_ICONMENU *pIconMenu, T_U8 Id);

/**
* @brief find item by place
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_U32 Place: item place
*
* @return T_ICONMENU_ITEM *
* @retval item  pointer
*/
T_ICONMENU_ITEM *IconMenu_FindItemByPlace(T_ICONMENU *pIconMenu, T_U32 Place);

#endif
