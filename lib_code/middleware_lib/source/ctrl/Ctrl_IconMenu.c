
/**************************************************************
*
* Copyrights (C) 2005, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctrl_IconMenu.c
* File flag: Icon Menu
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2005-11-16
*
****************************************************************/
#include "Ctrl_IconMenu.h"
#include "eng_string_uc.h"
#include "Eng_Font.h"

#if(NO_DISPLAY == 0)
#if (USE_COLOR_LCD)
#if (3 == LCD_TYPE && 1 == LCD_HORIZONTAL)
#if (STORAGE_USED == SPI_FLASH)

static T_BOOL IconMenu_GetItemSize(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_PlaceToRect(T_ICONMENU *pIconMenu, T_S32 place, T_U32 width, 
                                   T_U32 height, T_RECT *outRect);
static T_BOOL IconMenu_ShowIconAttach(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowIcon(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowOther(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowIconAnimate(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowAkBmp(T_RES_IMAGE IconId, T_RECT ShowRect);
static T_BOOL IconMenu_ShowAkBmpWithBack(T_RES_IMAGE IconId, T_RECT ShowRect, 
                                         T_RES_IMAGE IconBackId);

static T_BOOL IconMenu_ShowAkBmpPart(T_RES_IMAGE IconId, T_RECT PartRect, T_RECT ShowRect);
static T_BOOL IconMenu_ShowAkBmpWithBackPart(T_RES_IMAGE IconId, T_RECT ShowRect, 
                                             T_RES_IMAGE IconBackId, T_RECT PartRect);
static T_BOOL IconMenu_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect);
static T_BOOL IconMenu_CheckRect(T_RECT *Rect);


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
T_BOOL IconMenu_Init(T_ICONMENU *pIconMenu, T_RECT IconAttachRect, T_RECT IconRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->pItemHead = AK_NULL;
    pIconMenu->ItemQty = 0;
    pIconMenu->pItemFocus = AK_NULL;
    pIconMenu->pOldItemFocus = AK_NULL;

    IconMenu_SetIconAttachStyle(pIconMenu, ICONMENU_ALIGN_HCENTER | ICONMENU_ALIGN_VCENTER, 
                                CLR_BLACK, CLR_BLUE, eRES_IMAGE_NUM, AK_NULL);
    IconMenu_SetIconStyle(pIconMenu, ICONMENU_ICONINTERVAL_DEFAULT, ICONMENU_ICONINTERVAL_DEFAULT, 
                          CLR_BLUE, eRES_IMAGE_CTRLBKG, AK_NULL);
    IconMenu_SetIconImageNum(pIconMenu, ICONMENU_ICON_NUM, ICONMENU_ICON_DEFAULT);

    IconMenu_SetIconAttachRect(pIconMenu, IconAttachRect);
    IconMenu_SetIconRect(pIconMenu, IconRect);
    IconMenu_SetItemMatrix(pIconMenu, 1, 1);

    pIconMenu->IconAnimateCount = pIconMenu->IconImageDefault;
    pIconMenu->IconAnimateTimer = ERROR_TIMER;

    IconMenu_SetIconShowFlag(pIconMenu, AK_FALSE);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);

    pIconMenu->windowPlace = 0;
    pIconMenu->pageRow = 1;
    pIconMenu->pOtherRect = AK_NULL;
    pIconMenu->OtherShowCallBack = AK_NULL;
    
    return AK_TRUE;
}


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
T_BOOL IconMenu_Free(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_RECT *m, *n;
    T_ICONMENU_ITEM *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->pOtherRect != AK_NULL) 
	{
        m = pIconMenu->pOtherRect;
		
        while (m != AK_NULL) 
		{
            n = m->pNext;

            m = Fwl_Free(m);

            m = n;
        }

        pIconMenu->pOtherRect = AK_NULL;
        pIconMenu->OtherShowCallBack = AK_NULL;
    }

    if (pIconMenu->pItemHead != AK_NULL) 
	{
        p = pIconMenu->pItemHead;
		
        while (p != AK_NULL) 
		{
            q = p->pNext;

            p = Fwl_Free(p);
            pIconMenu->ItemQty--;

            p = q;
        }

        pIconMenu->pItemHead = AK_NULL;
    }

    return AK_TRUE;
}

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
T_BOOL IconMenu_Show(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    IconMenu_ShowIcon(pIconMenu);
    IconMenu_ShowIconAttach(pIconMenu);
    IconMenu_ShowOther(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_NONE);

    return AK_TRUE;
}


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
T_U16 IconMenu_Handler(T_ICONMENU *pIconMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_PRESS_KEY phyKey;
    T_U8 Id;

    if (pIconMenu == AK_NULL)
        return CTRL_RESPONSE_NONE;

    switch (Event) 
	{
    case M_EVT_USER_KEY:
        phyKey.id = pParam->c.Param1;
        phyKey.pressType = pParam->c.Param2;
		
        switch (phyKey.id) 
		{
        case kbOK:
            if (phyKey.pressType == PRESS_SHORT) 
			{
                if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) 
				{
                    IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                }
                else 
				{
                    //IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                    return IconMenu_GetFocusId(pIconMenu);
                }
            }
            break;
        
        case kbLEFT:
			if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) 
			{
                IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
            }
            else 
			{
	            if (phyKey.pressType == PRESS_SHORT) 
				{
	                IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_LEFT);
	                IconMenu_ShowIconAnimate(pIconMenu);
	            }
				else if (phyKey.pressType == PRESS_LONG) 
				{
	                IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_UP);
	                IconMenu_ShowIconAnimate(pIconMenu);
	            }
            }
            break;
        case kbRIGHT:
            if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) 
			{
                IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
            }
            else 
			{
	            if (phyKey.pressType == PRESS_SHORT) 
				{
	                IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_RIGHT);
	                IconMenu_ShowIconAnimate(pIconMenu);
	            }
				else if (phyKey.pressType == PRESS_LONG) 
				{
	                IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_DOWN);
	                IconMenu_ShowIconAnimate(pIconMenu);
	            }
            }
            break;
        default:
            break;
    	}
        break;
		
    case VME_EVT_TIMER:
        if (pParam->w.Param1 == (T_U32)pIconMenu->IconAnimateTimer)
            IconMenu_ShowIconAnimate(pIconMenu);
        break;
    default:
        break;
	}

    return CTRL_RESPONSE_NONE;
}

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
T_BOOL IconMenu_AddItem(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 Place, T_U32 ItemTextId)
{
    T_U8 i;
    T_ICONMENU_ITEM *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->ItemQty >= ICONMENU_ITEMQTY_MAX)
        return AK_FALSE;

    // check ID or Place, and find tail item
    q = pIconMenu->pItemHead;
    p = AK_NULL;
	
    while (q != AK_NULL) 
    {
        if ((q->Id == Id) || (q->Place == Place))
            return AK_FALSE;

        p = q;
        q = q->pNext;
    }

    q = (T_ICONMENU_ITEM *)Fwl_Malloc(sizeof(T_ICONMENU_ITEM));
	
    if (q == AK_NULL)
        return AK_FALSE;

    q->Id = Id;
    q->Place = Place;
    
    if (ItemTextId < eRES_STR_NUM)
		q->ItemTextId = ItemTextId;

    for (i=0; i<pIconMenu->IconImageNum; i++)
        q->IconId[i] = eRES_IMAGE_NUM;

    if (p == AK_NULL) 
    {
        // item head is AK_NULL
        q->pPrevious = AK_NULL;
        q->pNext = AK_NULL;
        pIconMenu->pItemHead = q;
    }
    else 
    {
        // item head is not AK_NULL, and q is tail item
        q->pPrevious = p;
        q->pNext = AK_NULL;
        p->pNext = q;
    }

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = q;

    pIconMenu->ItemQty++;
    IconMenu_SetItemMatrixAuto(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

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
T_BOOL IconMenu_ResetItemText(T_ICONMENU *pIconMenu, T_U8 Id, T_U32 ItemTextId)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (ItemTextId >= eRES_STR_NUM)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p ==  AK_NULL)
        return AK_FALSE;

	p->ItemTextId = ItemTextId;

    return AK_TRUE;
}

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
T_BOOL IconMenu_SetItemIcon(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 index, T_RES_IMAGE IconId)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (index >= pIconMenu->IconImageNum)
        return AK_FALSE;

    if (eRES_IMAGE_NUM == IconId)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    p->IconId[index] = IconId;

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}


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
T_BOOL IconMenu_DelItemById(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    if (p->pPrevious != AK_NULL) 
	{
        p->pPrevious->pNext = p->pNext;
        if (pIconMenu->pItemFocus == p)
            pIconMenu->pItemFocus = p->pPrevious;
    }
	
    if (p->pNext != AK_NULL) 
	{
        p->pNext->pPrevious = p->pPrevious;
        if (pIconMenu->pItemFocus == p)
            pIconMenu->pItemFocus = p->pNext;
    }
	
    if (pIconMenu->pItemFocus == p)
        pIconMenu->pItemFocus = AK_NULL;

    p = Fwl_Free(p);

    pIconMenu->ItemQty--;
    IconMenu_SetItemMatrixAuto(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}


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
T_BOOL IconMenu_DelItemByPlace(T_ICONMENU *pIconMenu, T_U8 Place)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
	{
        if (p->Place == Place)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    if (p->pPrevious != AK_NULL)
        p->pPrevious->pNext = p->pNext;
	
    if (p->pNext != AK_NULL)
        p->pNext->pPrevious = p->pPrevious;

    p = Fwl_Free(p);

    pIconMenu->ItemQty--;
    IconMenu_SetItemMatrixAuto(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}


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
T_BOOL IconMenu_MoveFocus(T_ICONMENU *pIconMenu, T_ICONMENU_DIRECTION Dir)
{
    T_U8 FocusPlace;
    T_ICONMENU_ITEM *p;

//    T_S32  nFocusBak = 0;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->pItemHead == AK_NULL) 
    {
        pIconMenu->pItemFocus = AK_NULL;
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
        return AK_FALSE;
    }

    if (pIconMenu->pItemFocus == AK_NULL) 
    {
        pIconMenu->pOldItemFocus = pIconMenu->pItemFocus;
        pIconMenu->pItemFocus = pIconMenu->pItemHead;
        pIconMenu->windowPlace = 0;
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON_ATTACH);
        return AK_TRUE;
    }

    FocusPlace = pIconMenu->pItemFocus->Place;
	
    //Fwl_Print(C4, M_CTRL, "befor ,FocusPlace = %d", FocusPlace);
	
    switch (Dir) 
	{
        case ICONMENU_DIRECTION_UP:
        {
            FocusPlace = 
                    (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-pIconMenu->ItemCol)%
                    (pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        }

        case ICONMENU_DIRECTION_DOWN:
        {            
            FocusPlace = (FocusPlace+pIconMenu->ItemCol)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        }
            
        case ICONMENU_DIRECTION_LEFT:
            FocusPlace = (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-1) 
                          %(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        case ICONMENU_DIRECTION_RIGHT:
            FocusPlace = (FocusPlace+1)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        default:
            IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
            return AK_FALSE;
    }

    //Fwl_Print(C4, M_CTRL, "after set ,FocusPlace = %d", FocusPlace);
	
    while (1) 
	{
        p = IconMenu_FindItemByPlace(pIconMenu, FocusPlace);
		
        if (p != AK_NULL)
            break;

        switch (Dir) 
        {
            case ICONMENU_DIRECTION_UP:
                FocusPlace = 
                        (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-pIconMenu->ItemCol)%
                        (pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_DOWN:
                FocusPlace = (FocusPlace+pIconMenu->ItemCol)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_LEFT:
                FocusPlace = (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-1) 
                              %(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_RIGHT:
                FocusPlace = (FocusPlace+1)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            default:
                IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                return AK_FALSE;
        }
    }

    if (p == AK_NULL) 
    {
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
        return AK_FALSE;
    }

    IconMenu_SetItemFocus(pIconMenu, FocusPlace);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON_ATTACH);

    return AK_TRUE;
}


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
T_BOOL IconMenu_SetItemFocus(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = IconMenu_FindItemById(pIconMenu, Id);
	
    if (p == AK_NULL)
        return AK_FALSE;

    pIconMenu->pOldItemFocus = pIconMenu->pItemFocus;
    pIconMenu->pItemFocus = p;

    if (pIconMenu->ItemQty > (pIconMenu->pageRow * pIconMenu->ItemCol))
    {
        if ((pIconMenu->pItemFocus->Place - pIconMenu->windowPlace ) < 0)
        {
            pIconMenu->windowPlace -= pIconMenu->pageRow * pIconMenu->ItemCol;
            if (pIconMenu->windowPlace < 0)
            {
                pIconMenu->windowPlace = 0;
            }
        }
        else if (pIconMenu->pItemFocus->Place >= pIconMenu->windowPlace 
                 + pIconMenu->pageRow * pIconMenu->ItemCol)
        {
            if (pIconMenu->windowPlace + 2*pIconMenu->pageRow * pIconMenu->ItemCol > pIconMenu->ItemRow * pIconMenu->ItemCol)
            {
                pIconMenu->windowPlace = (pIconMenu->ItemRow - pIconMenu->pageRow)*pIconMenu->ItemCol;
            } 
            else
            {
                pIconMenu->windowPlace += pIconMenu->pageRow * pIconMenu->ItemCol;
            }
        }
        
    }

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON
                        | ICONMENU_REFRESH_ICON_ATTACH);

    return AK_TRUE;
}


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
T_BOOL IconMenu_SetIconAttachRect(T_ICONMENU *pIconMenu, T_RECT IconAttachRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconAttachtRect = IconAttachRect;
    IconMenu_CheckRect(&pIconMenu->IconAttachtRect);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON_ATTACH);
    return AK_TRUE;
}


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
T_BOOL IconMenu_SetIconRect(T_ICONMENU *pIconMenu, T_RECT IconRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconRect = IconRect;
    IconMenu_CheckRect(&pIconMenu->IconRect);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}


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
T_BOOL IconMenu_SetItemMatrix(T_ICONMENU *pIconMenu, T_U8 ItemRow, T_U8 ItemCol)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->ItemRow = ItemRow;
    pIconMenu->ItemCol = ItemCol;

    if ((IconMenu_GetItemSize(pIconMenu) == AK_FALSE) 
            || ((pIconMenu->ItemRow*pIconMenu->ItemCol) < pIconMenu->ItemQty)
            || ((pIconMenu->ItemRow*pIconMenu->ItemCol) > ICONMENU_ITEMQTY_MAX))
    {
        IconMenu_SetItemMatrixAuto(pIconMenu);
    }
	
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON | ICONMENU_REFRESH_SCRBAR);
    return AK_TRUE;
}


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
T_BOOL IconMenu_SetItemMatrixAuto(T_ICONMENU *pIconMenu)
{
    IconMenu_GetItemSize(pIconMenu);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}


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
                                   T_RECT *pIconAttachPartRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconAttachTextAlign = TextAlign;
    pIconMenu->IconAttachTextColor = TextColor;
    pIconMenu->IconAttachTextBackColor = TextBackColor;
    pIconMenu->IconAttachBkImg = TextBackData;
	
    if (pIconAttachPartRect != AK_NULL) 
    {
        pIconMenu->IconAttachPartRect = *pIconAttachPartRect;
        IconMenu_CheckRect(&pIconMenu->IconAttachPartRect);
        pIconMenu->IconAttachPartRectFlag = AK_TRUE;
    }
    else 
    {
        pIconMenu->IconAttachPartRectFlag = AK_FALSE;
    }

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON_ATTACH);
    return AK_TRUE;
}


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
                             T_COLOR IconBackColor, T_RES_IMAGE IconBackId, T_RECT *pIconPartRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconHInterval = IconHInterval;
	
    if (pIconMenu->IconHInterval > ICONMENU_ICONINTERVAL_MAX)
        pIconMenu->IconHInterval = ICONMENU_ICONINTERVAL_MAX;
	
    pIconMenu->IconVInterval = IconVInterval;
	
    if (pIconMenu->IconVInterval > ICONMENU_ICONINTERVAL_MAX)
        pIconMenu->IconVInterval = ICONMENU_ICONINTERVAL_MAX;
	
    pIconMenu->IconBackColor = IconBackColor;
	
	
    pIconMenu->IconBackId = IconBackId;
	
    if (pIconPartRect != AK_NULL) 
	{
        pIconMenu->IconPartRect = *pIconPartRect;
        IconMenu_CheckRect(&pIconMenu->IconPartRect);
        pIconMenu->IconPartRectFlag = AK_TRUE;
    }
    else 
	{
        pIconMenu->IconPartRectFlag = AK_FALSE;
    }

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}


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
T_BOOL IconMenu_SetIconImageNum(T_ICONMENU *pIconMenu, T_U8 IconImageNum, T_U8 IconImageDefault)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (IconImageDefault >= IconImageNum)
        return AK_FALSE;

    pIconMenu->IconImageNum = IconImageNum;
    pIconMenu->IconImageDefault = IconImageDefault;

    return AK_TRUE;
}


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
T_BOOL IconMenu_SetIconShowFlag(T_ICONMENU *pIconMenu, T_BOOL IconShowFlag)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconShowFlag = IconShowFlag;

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}


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
T_BOOL IconMenu_SetRefresh(T_ICONMENU *pIconMenu, T_U8 RefreshFlag)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (RefreshFlag == ICONMENU_REFRESH_NONE)
        pIconMenu->RefreshFlag = ICONMENU_REFRESH_NONE;
    else
        pIconMenu->RefreshFlag |= RefreshFlag;

    if (pIconMenu->RefreshFlag == ICONMENU_REFRESH_ALL)
        pIconMenu->pOldItemFocus = AK_NULL;

    return AK_TRUE;
}


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
T_BOOL IconMenu_SetOtherShowCallBack(T_ICONMENU *pIconMenu, T_fICONMENU_CALLBACK OtherShowCallBack)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->OtherShowCallBack = OtherShowCallBack;

    return AK_TRUE;
}


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
T_BOOL IconMenu_AddOtherRect(T_ICONMENU *pIconMenu, T_RECT OtherRect)
{
    T_ICONMENU_RECT *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    q = pIconMenu->pOtherRect;
    p = AK_NULL;
	
    while (q != AK_NULL) 
	{
        p = q;
        q = q->pNext;
    }

    q = (T_ICONMENU_RECT *)Fwl_Malloc(sizeof(T_ICONMENU_RECT));
    if (q == AK_NULL)
        return AK_FALSE;

    q->Rect = OtherRect;
    IconMenu_CheckRect(&q->Rect);
    q->pNext = AK_NULL;

    if (p == AK_NULL)
        pIconMenu->pOtherRect = q;
    else
        p->pNext = q;

    return AK_TRUE;
}


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
T_U8 IconMenu_GetRefreshFlag(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_REFRESH_NONE;

    return pIconMenu->RefreshFlag;
}

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
T_BOOL IconMenu_GetIconShowFlag(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    return pIconMenu->IconShowFlag;
}


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
T_ICONMENU_ITEM *IconMenu_GetFocusItem(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_NULL;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    return pIconMenu->pItemFocus;
}


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
T_U8 IconMenu_GetFocusId(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_ID;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    if (pIconMenu->pItemFocus == AK_NULL)
        return ICONMENU_ERROR_ID;

    return pIconMenu->pItemFocus->Id;
}


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
T_U8 IconMenu_GetFocusPlace(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_PLACE;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    if (pIconMenu->pItemFocus == AK_NULL)
        return ICONMENU_ERROR_PLACE;

    return pIconMenu->pItemFocus->Place;
}


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
T_U8 IconMenu_GetIdByPoint(T_ICONMENU *pIconMenu, T_U16 x, T_U16 y)
{
    T_ICONMENU_ITEM *p;
    T_RECT ShowRect;

    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_ID;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
    {
        if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol)
               && (p->IconId[pIconMenu->IconImageDefault] != eRES_IMAGE_NUM)) 
        {
            IconMenu_PlaceToRect(pIconMenu,p->Place,0,0,&ShowRect);
			
            if (IconMenu_CheckPointInRect(x, y, ShowRect) == AK_TRUE)
                return p->Id;
        }

        p = p->pNext;
    }

    return ICONMENU_ERROR_ID;
}


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
T_ICONMENU_ITEM *IconMenu_FindItemById(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_NULL;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
    {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    return p;
}


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
T_ICONMENU_ITEM *IconMenu_FindItemByPlace(T_ICONMENU *pIconMenu, T_U32 Place)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_NULL;

    p = pIconMenu->pItemHead;
	
    while (p != AK_NULL) 
    {
        if ((T_U32)(p->Place) == Place)
            break;

        p = p->pNext;
    }

    return p;
}


/**
* @brief calc  item width , height and row num in one page
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_GetItemSize(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->ItemWidth = (pIconMenu->IconRect.width-pIconMenu->IconHInterval
                            * (pIconMenu->ItemCol+1))/pIconMenu->ItemCol;
    pIconMenu->ItemHeight = (pIconMenu->IconRect.height-pIconMenu->IconVInterval
                             * (pIconMenu->ItemRow+1))/pIconMenu->ItemRow;
    
    if (pIconMenu->ItemWidth < ICONMENU_ICONSIZE_MIN_WIDTH) 
    {
        pIconMenu->ItemWidth = ICONMENU_ICONSIZE_MIN_WIDTH;
    }
    
    if (pIconMenu->ItemHeight < ICONMENU_ICONSIZE_MIN_HEIGHT)
    {
        pIconMenu->ItemHeight = ICONMENU_ICONSIZE_MIN_HEIGHT;
    }

    pIconMenu->pageRow = (pIconMenu->IconRect.height-pIconMenu->IconVInterval)
                          /(pIconMenu->IconVInterval + pIconMenu->ItemHeight);
    
    return AK_TRUE;
}


/**
* @brief show icon attach
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowIconAttach(T_ICONMENU *pIconMenu)
{
    T_S16 PosX, PosY;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON_ATTACH) != ICONMENU_REFRESH_ICON_ATTACH)
        return AK_FALSE;
    
    if (pIconMenu->IconShowFlag == AK_TRUE) 
    {
        if (pIconMenu->IconAttachBkImg != eRES_IMAGE_NUM) 
        {
            if (pIconMenu->IconAttachPartRectFlag == AK_TRUE)
                IconMenu_ShowAkBmpPart(pIconMenu->IconAttachBkImg, pIconMenu->IconAttachPartRect, 
                                       pIconMenu->IconAttachtRect);
            else
                IconMenu_ShowAkBmp(pIconMenu->IconAttachBkImg, pIconMenu->IconAttachtRect);
        }
    }
    else 
    {
        if (pIconMenu->IconBackId != eRES_IMAGE_NUM)
                IconMenu_ShowAkBmpPart(pIconMenu->IconBackId, pIconMenu->IconAttachtRect, 
                                       pIconMenu->IconAttachtRect);

        return AK_TRUE;
    }

    switch (pIconMenu->IconAttachTextAlign & ICONMENU_ALIGN_HORIZONTAL) 
    {
        case ICONMENU_ALIGN_LEFT:
            PosX = pIconMenu->IconAttachtRect.left;
            break;
        case ICONMENU_ALIGN_RIGHT:
            PosX = (T_S16)(pIconMenu->IconAttachtRect.left + pIconMenu->IconAttachtRect.width 
                           - GetStringDispWidth(CP_UNICODE, (T_U8*)pIconMenu->pItemFocus->ItemTextId, 0));
            break;
        case ICONMENU_ALIGN_HCENTER:
        default:
            PosX = (T_S16)(pIconMenu->IconAttachtRect.left + (pIconMenu->IconAttachtRect.width 
                           - GetStringDispWidth(CP_UNICODE, (T_U8*)pIconMenu->pItemFocus->ItemTextId, 0))/2);
            break;
    }

    switch (pIconMenu->IconAttachTextAlign & ICONMENU_ALIGN_VERTICAL) 
    {
        case ICONMENU_ALIGN_UP:
            PosY = pIconMenu->IconAttachtRect.top;
            break;
        case ICONMENU_ALIGN_DOWN:
            PosY = pIconMenu->IconAttachtRect.top+pIconMenu->IconAttachtRect.height-FONT_HEIGHT;
            break;
        case ICONMENU_ALIGN_VCENTER:
        default:
            PosY = pIconMenu->IconAttachtRect.top+(pIconMenu->IconAttachtRect.height-FONT_HEIGHT)/2;
            break;
    }

    return AK_TRUE;
}


/**
* @brief get rect by place
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
* @param in T_S32 place: place
* @param in T_U32 width: width
* @param in T_U32 height: height
* @param out T_RECT *outRect: rect
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_PlaceToRect(T_ICONMENU *pIconMenu, T_S32 place, T_U32 width, 
                                   T_U32 height, T_RECT *outRect)
{
    AK_ASSERT_PTR(outRect,"IconMenu_PlaceToRect outRect==null",AK_FALSE);

    outRect->left = (T_S16)(pIconMenu->IconRect.left + 
        pIconMenu->IconHInterval*(place%(T_S32)pIconMenu->ItemCol+1) + 
        pIconMenu->ItemWidth*(place%(T_S32)pIconMenu->ItemCol));
    outRect->top = (T_S16)(pIconMenu->IconRect.top + 
        pIconMenu->IconVInterval*((place - pIconMenu->windowPlace)/pIconMenu->ItemCol+1) + 
        pIconMenu->ItemHeight*((place - pIconMenu->windowPlace)/pIconMenu->ItemCol));

    outRect->top -= ICONMENU_ICON_ADJUST_TOP_MARGIN;

    if (width != 0)
    {
        outRect->left -= (T_S16)(width - pIconMenu->ItemWidth)/2;
        outRect->width = (T_POS)width;
    } 
    else 
    {
        outRect->width = (T_S16)(pIconMenu->ItemWidth);
    }

    if (height != 0)
    {
        outRect->top -= (T_S16)(height - pIconMenu->ItemHeight)/2;
        outRect->height = (T_POS)height;
    } 
    else
    {
        outRect->height = (T_S16)(pIconMenu->ItemHeight);
    }
	
    if (outRect->top < pIconMenu->IconRect.top
        || outRect->left < pIconMenu->IconRect.left
        ||(outRect->top + outRect->height) > (pIconMenu->IconRect.top + pIconMenu->IconRect.height)
        || (outRect->left + outRect->width) > (pIconMenu->IconRect.left + pIconMenu->IconRect.width))
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}


/**
* @brief show icons
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowIcon(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_ITEM *p = AK_NULL;
	T_U32 left = 0;
	T_U32 top = 0;
    T_RECT ShowRect, IconPartRect;
    T_S16 AkBmpWidth, AkBmpHeight;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON) != ICONMENU_REFRESH_ICON)
            && ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_FOCUS) != ICONMENU_REFRESH_FOCUS))
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON) == ICONMENU_REFRESH_ICON) 
    {
       IconPartRect = pIconMenu->IconPartRect;
      
       if (pIconMenu->IconBackId != eRES_IMAGE_NUM
	   	 && ICONMENU_REFRESH_ALL == pIconMenu->RefreshFlag) 
       {
            if (pIconMenu->IconPartRectFlag == AK_TRUE)
            {    
                IconMenu_ShowAkBmpPart(pIconMenu->IconBackId, IconPartRect, 
                                    pIconMenu->IconAttachtRect);                                    
            }
            else
            {
                IconMenu_ShowAkBmp(pIconMenu->IconBackId, pIconMenu->IconAttachtRect);
            }
       }

        if (pIconMenu->IconShowFlag == AK_FALSE)
            return AK_TRUE;

        p = pIconMenu->pItemHead;
        p = IconMenu_FindItemByPlace(pIconMenu,pIconMenu->windowPlace);


        while (p != AK_NULL) 
        {
            if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && 
                    (p->IconId[pIconMenu->IconImageDefault] != eRES_IMAGE_NUM)) 
            {
				AkBmpWidth = Eng_GetResImageWidth(p->IconId[pIconMenu->IconImageDefault]);
				AkBmpHeight = Eng_GetResImageHeight(p->IconId[pIconMenu->IconImageDefault]);
				
                if (IconMenu_PlaceToRect(pIconMenu,p->Place,AkBmpWidth,AkBmpHeight,&ShowRect))
                {
                	if (p->Place == pIconMenu->pItemFocus->Place
						|| ICONMENU_REFRESH_ALL == pIconMenu->RefreshFlag)
                	{
	                    if (p->Place == pIconMenu->pItemFocus->Place)
	                    {
	                        Fwl_FillRect((T_U16)(ShowRect.left-pIconMenu->IconHInterval/2),
	                                          (T_U16)(ShowRect.top-pIconMenu->IconVInterval/2), 
	                                          (T_U16)(ShowRect.width+pIconMenu->IconHInterval), 
	                                          (T_U16)(ShowRect.height+pIconMenu->IconVInterval), 
	                                          ICONMENU_ICON_FOCUS_BACK);
	                    }
						
	                    IconMenu_ShowAkBmp(p->IconId[pIconMenu->IconImageDefault], ShowRect);

						left = (T_U32)(ShowRect.left + (ShowRect.width - 
	                                GetStringDispWidth(CP_UNICODE, (T_U8*)p->ItemTextId, 0))/2);
					    top = (T_U32)(ShowRect.top+ShowRect.height+ICONMENU_ATTACH_FONT_INTERVAL);
						
	                    DispStringInWidth(CP_UNICODE, left, top, 
	                                (T_U8*)p->ItemTextId, GetStringDispWidth(CP_UNICODE, (T_U8*)p->ItemTextId, 0), 
	                                ICONMENU_ICON_TEXT_COLOR);
                	}

                }
            }
            p = p->pNext;
        }
    }

    if (((pIconMenu->RefreshFlag & ICONMENU_REFRESH_FOCUS) == ICONMENU_REFRESH_FOCUS) 
            && (pIconMenu->pOldItemFocus != AK_NULL)) 
    {
        p = pIconMenu->pOldItemFocus;
        pIconMenu->pOldItemFocus = AK_NULL;

        if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && 
            (p->IconId[pIconMenu->IconImageDefault] != eRES_IMAGE_NUM)) 
        {
        	AkBmpWidth = Eng_GetResImageWidth(p->IconId[pIconMenu->IconImageDefault]);
			AkBmpHeight = Eng_GetResImageHeight(p->IconId[pIconMenu->IconImageDefault]);
			
            if (IconMenu_PlaceToRect(pIconMenu,p->Place,AkBmpWidth,AkBmpHeight,&ShowRect))
            {
                //IconMenu_ShowAkBmp(p->IconId[pIconMenu->IconImageDefault], ShowRect);
				IconMenu_ShowAkBmpWithBack(p->IconId[pIconMenu->IconImageDefault], 
                                           ShowRect,pIconMenu->IconBackId);

				left = (T_U32)(ShowRect.left + (ShowRect.width - 
                                GetStringDispWidth(CP_UNICODE, (T_U8*)p->ItemTextId, 0))/2);
			    top = (T_U32)(ShowRect.top+ShowRect.height+ICONMENU_ATTACH_FONT_INTERVAL);
                DispStringInWidth(CP_UNICODE, left, top, 
                                (T_U8*)p->ItemTextId, GetStringDispWidth(CP_UNICODE, (T_U8*)p->ItemTextId, 0), 
                                ICONMENU_ICON_TEXT_COLOR);
            }
        }
    }

    return AK_TRUE;
}

/**
* @brief show other
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowOther(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_RECT *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_OTHER) != ICONMENU_REFRESH_OTHER)
        return AK_FALSE;

    if (((pIconMenu->IconShowFlag == AK_FALSE) || (pIconMenu->RefreshFlag == ICONMENU_REFRESH_ALL)) 
          &&(pIconMenu->pOtherRect != AK_NULL)) 
    {
        p = pIconMenu->pOtherRect;
		
        while (p != AK_NULL) 
        {
            if (pIconMenu->IconBackId != eRES_IMAGE_NUM)
                    IconMenu_ShowAkBmpPart(pIconMenu->IconBackId, p->Rect, p->Rect);

            p = p->pNext;
        }
    }

    if ((pIconMenu->IconShowFlag == AK_TRUE) && (pIconMenu->pOtherRect != AK_NULL) 
        && (pIconMenu->OtherShowCallBack != AK_NULL))
    {
        pIconMenu->OtherShowCallBack();
    }

    return AK_TRUE;
}


/**
* @brief show icon animate
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_ICONMENU *pIconMenu: the IconMenu handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowIconAnimate(T_ICONMENU *pIconMenu)
{
    T_RECT ShowRect;
    T_S16 AkBmpWidth, AkBmpHeight;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->IconShowFlag == AK_FALSE)
        return AK_TRUE;

    if (pIconMenu->pItemFocus != AK_NULL) 
    {
        if (pIconMenu->IconAnimateCount == pIconMenu->IconImageDefault)
            pIconMenu->IconAnimateCount = (pIconMenu->IconAnimateCount+1)%pIconMenu->IconImageNum;
        
        if ((pIconMenu->pItemFocus->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) 
            && (pIconMenu->pItemFocus->IconId[pIconMenu->IconAnimateCount] != eRES_IMAGE_NUM)) 
        {
            AkBmpWidth = Eng_GetResImageWidth(pIconMenu->pItemFocus->IconId[pIconMenu->IconAnimateCount]);
			AkBmpHeight = Eng_GetResImageHeight(pIconMenu->pItemFocus->IconId[pIconMenu->IconAnimateCount]);

            IconMenu_PlaceToRect(pIconMenu,pIconMenu->pItemFocus->Place, AkBmpWidth, AkBmpHeight,
                                &ShowRect);
            if (pIconMenu->IconPartRectFlag == AK_TRUE)
            {
                IconMenu_ShowAkBmpWithBackPart(pIconMenu->pItemFocus->IconId[pIconMenu->IconAnimateCount], 
                                               ShowRect, pIconMenu->IconBackId, pIconMenu->IconPartRect);
            }
            else
            {
                IconMenu_ShowAkBmpWithBack(pIconMenu->pItemFocus->IconId[pIconMenu->IconAnimateCount], 
                                           ShowRect, pIconMenu->IconBackId);
            }

        }

        pIconMenu->IconAnimateCount = (pIconMenu->IconAnimateCount+1)%pIconMenu->IconImageNum;
    }

    return AK_TRUE;
}

/**
* @brief show akbmp
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_RES_IMAGE IconId: icon id
* @param in T_RECT ShowRect: ShowRect
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowAkBmp(T_RES_IMAGE IconId, T_RECT ShowRect)
{
    T_S16 AkBmpWidth, AkBmpHeight;
    T_RECT AkBmpRect;

    if (eRES_IMAGE_NUM == IconId)
        return AK_FALSE;

	
	AkBmpWidth = Eng_GetResImageWidth(IconId);
	AkBmpHeight = Eng_GetResImageHeight(IconId);

    AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
    AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));

    Eng_ImageResDisp(AkBmpRect.left, AkBmpRect.top, IconId, IMG_TRANSPARENT);
	return AK_TRUE;

}


/**
* @brief show akbmp with back img
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_RES_IMAGE IconId: icon id
* @param in T_RECT ShowRect: ShowRect
* @param in T_RES_IMAGE IconBackId: IconBack id
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowAkBmpWithBack(T_RES_IMAGE IconId, T_RECT ShowRect, 
                                         T_RES_IMAGE IconBackId)
{
    T_S16 AkBmpWidth, AkBmpHeight;
    T_RECT AkBmpRect;

    if (eRES_IMAGE_NUM == IconId)
        return AK_FALSE;

	Eng_ImageResDispEx(ShowRect.left, ShowRect.top, IconBackId, 
		ShowRect.width, ShowRect.height , ShowRect.left, ShowRect.top, AK_FALSE);
	
    AkBmpWidth = Eng_GetResImageWidth(IconId);
	AkBmpHeight = Eng_GetResImageHeight(IconId);

    AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
    AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));

    Eng_ImageResDisp(AkBmpRect.left, AkBmpRect.top, IconId, IMG_TRANSPARENT);
    return AK_TRUE;
}


/**
* @brief show akbmp part
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_RES_IMAGE IconId: icon id
* @param in T_RECT PartRect: PartRect of the icon
* @param in T_RECT ShowRect: ShowRect
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowAkBmpPart(T_RES_IMAGE IconId, T_RECT PartRect, T_RECT ShowRect)
{
    T_S16 AkBmpWidth, AkBmpHeight;
    T_RECT AkBmpRect;

    if (eRES_IMAGE_NUM == IconId)
        return AK_FALSE;

    AkBmpWidth = Eng_GetResImageWidth(IconId);
	AkBmpHeight = Eng_GetResImageHeight(IconId);

	AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
    AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));
	
    Eng_ImageResDispEx(AkBmpRect.left, AkBmpRect.top, IconId, 
		PartRect.width, PartRect.height, PartRect.left, PartRect.top, IMG_TRANSPARENT);
    return AK_TRUE;
}


/**
* @brief show akbmp part with back img
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_RES_IMAGE IconId: icon id
* @param in T_RECT ShowRect: ShowRect
* @param in T_RES_IMAGE IconBackId: IconBack id
* @param in T_RECT PartRect: PartRect of the icon
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_ShowAkBmpWithBackPart(T_RES_IMAGE IconId, T_RECT ShowRect, 
                                             T_RES_IMAGE IconBackId, T_RECT PartRect)
{
    T_S16 AkBmpWidth, AkBmpHeight;
    T_RECT AkBmpRect;

    if (eRES_IMAGE_NUM == IconId)
        return AK_FALSE;

    Eng_ImageResDispEx(ShowRect.left, ShowRect.top, IconBackId, 
		ShowRect.width, ShowRect.height , ShowRect.left, ShowRect.top, AK_FALSE);
	
    AkBmpWidth = Eng_GetResImageWidth(IconId);
	AkBmpHeight = Eng_GetResImageHeight(IconId);

	AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
    AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));
	
    Eng_ImageResDispEx(AkBmpRect.left, AkBmpRect.top, IconId, 
		PartRect.width, PartRect.height, PartRect.left, PartRect.top, IMG_TRANSPARENT);

	return AK_TRUE;
}


/**
* @brief check the point is in the rect or not
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in T_U16 x: x
* @param in T_U16 y: y
* @param in T_RECT Rect: Rect
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect)
{
    if (((x >= Rect.left) && (x < Rect.left+Rect.width))
        && ((y >= Rect.top) && (y <= Rect.top+Rect.height)))
        return AK_TRUE;
    else
        return AK_FALSE;
}

/**
* @brief check the rect and adjust it
*
* @author Guanghua Zhang
* @date 2005-11-16
*
* @param in/out T_RECT *Rect: Rect
*
* @return T_BOOL
* @retval
*/
static T_BOOL IconMenu_CheckRect(T_RECT *Rect)
{
    if (Rect == AK_NULL)
        return AK_FALSE;

    if ((Rect->width < 0) || (Rect->width > MAIN_LCD_WIDTH))
        Rect->width = MAIN_LCD_WIDTH;
	
    if ((Rect->height < 0) || (Rect->height > MAIN_LCD_HEIGHT))
        Rect->height = MAIN_LCD_HEIGHT;
	
    if (((Rect->left+Rect->width) < 0)
            || ((Rect->left+Rect->width) > MAIN_LCD_WIDTH))
        Rect->left = MAIN_LCD_WIDTH - Rect->width;
	
    if (((Rect->top+Rect->height) < 0)  
            || ((Rect->top+Rect->height) > MAIN_LCD_HEIGHT))
        Rect->top = MAIN_LCD_HEIGHT - Rect->height;

    return AK_TRUE;
}

#endif
#endif
#endif
#endif

