/**
 * @file Ctrl_MenuConfig.c
 * @brief common menu config
 * @copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * 
 * @author 
 * @date  
 * @version 1,0 
 */



#include "Ctrl_MenuConfig.h"
#include "eng_debug.h"
#include "fwl_osmalloc.h"
#include <string.h>



typedef struct _T_MENU_STRUCT
{
    T_U8                Menu;		    	//该菜单的id
    T_U8                ItemNum;		    //该菜单的item总数
    T_U8                FocusItem;		    //该菜单保存的焦点item 的id，保存确认用
    T_U8                SelectItem;		    //该菜单临时所选的item 的id，临时加减操作用
    T_STR_SHOW		    *pStr;			    //打印字串指针
}T_MENU_STRUCT;


typedef struct
{
    T_U8                MenuMaxNum;			//菜单最大总数
    T_U8				MenuNum;			//菜单实际总数
    T_U8                FocusMenuIndex;		//焦点菜单的数组下标号
    T_MENU_STRUCT		*pMenu;		    	//菜单数组指针
}T_MENU_PARM;




static T_MENU_PARM menu_parm = {0};



/**
* @brief init menu_parm, malloc menu array by MenuMaxNum
*
* @author Songmengxing
* @date 2013-04-10
*
* @param in T_U8 MenuMaxNum: menu max num
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_Init(T_U8 MenuMaxNum)
{
	T_U8 i = 0;
	
	menu_parm.MenuMaxNum     = MenuMaxNum;
	menu_parm.MenuNum        = 0;
    menu_parm.FocusMenuIndex = 0;

	if (AK_NULL != menu_parm.pMenu)
	{
		AK_DEBUG_OUTPUT("MenuCfg_Init pMenu is not null!\n");
		menu_parm.pMenu = Fwl_Free(menu_parm.pMenu);
	}

	menu_parm.pMenu = (T_MENU_STRUCT*)Fwl_Malloc(MenuMaxNum * sizeof(T_MENU_STRUCT));
	AK_ASSERT_PTR(menu_parm.pMenu, "MenuCfg_Init pMenu", AK_FALSE);
	memset(menu_parm.pMenu, 0, MenuMaxNum * sizeof(T_MENU_STRUCT));

	for (i=0; i<MenuMaxNum; i++)
	{
		menu_parm.pMenu[i].Menu = MENU_ERROR_ID;
	}

	return AK_TRUE;
}


/**
* @brief free menu array, clean menu_parm
*
* @author Songmengxing
* @date 2013-04-10
*
* @param T_VOID
*
* @return T_VOID
* @retval 
*/
T_VOID MenuCfg_Free(T_VOID)
{
	if (AK_NULL != menu_parm.pMenu)
	{
		menu_parm.pMenu = Fwl_Free(menu_parm.pMenu);
	}

	menu_parm.MenuMaxNum     = 0;
	menu_parm.MenuNum        = 0;
    menu_parm.FocusMenuIndex = 0;
}


/**
* @brief add menu
*
* @author Songmengxing
* @date 2013-04-10
*
* @param in T_U8 Menu : menu id
* @param in T_U8 ItemNum : item num
* @param in T_U8 FocusItem : focus item id
* @param in T_STR_SHOW *pStr : print str array pointer
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_AddMenu(T_U8 Menu, T_U8 ItemNum, T_U8 FocusItem, T_STR_SHOW *pStr)
{
	T_U8 index = MENU_ERROR_ID;

	if (AK_NULL == menu_parm.pMenu)
	{
		AK_DEBUG_OUTPUT("MenuCfg_AddMenu pMenu is null, please call MenuCfg_Init first!\n");
		return AK_FALSE;
	}

	if (menu_parm.MenuNum == menu_parm.MenuMaxNum)
	{
		AK_DEBUG_OUTPUT("MenuCfg_AddMenu menu array is full, can't add one more!\n");
		return AK_FALSE;
	}

	if (0 == ItemNum)
	{
		AK_DEBUG_OUTPUT("MenuCfg_AddMenu ItemNum is 0!\n");
		return AK_FALSE;
	}

	index = menu_parm.MenuNum;

	menu_parm.pMenu[index].Menu    = Menu;
	menu_parm.pMenu[index].ItemNum = ItemNum;
	menu_parm.pMenu[index].pStr    = pStr;

	if (FocusItem >= ItemNum)
	{
		AK_DEBUG_OUTPUT("MenuCfg_AddMenu FocusItem error, set 0!\n");
		menu_parm.pMenu[index].FocusItem = 0;
	}
	else
	{
	    menu_parm.pMenu[index].FocusItem = FocusItem;
	}

	menu_parm.pMenu[index].SelectItem = menu_parm.pMenu[index].FocusItem;

	menu_parm.MenuNum++;

	return AK_TRUE;
}




/**
* @brief move menu
*
* @author Songmengxing
* @date 2013-04-10
*
* @param in T_S8 dir : 1, next; -1, previous
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_MoveMenu(T_S8 dir)
{
	T_U8 index = MENU_ERROR_ID;
	
	if ((AK_NULL == menu_parm.pMenu)
		|| (0 == menu_parm.MenuNum))
	{
		return AK_FALSE;
	}

	if (1 == menu_parm.MenuNum)
	{
		return AK_TRUE;
	}

	index = menu_parm.FocusMenuIndex;

	menu_parm.pMenu[index].SelectItem = menu_parm.pMenu[index].FocusItem;

	if (MOVE_NEXT == dir)
	{
		index = (index + 1) % menu_parm.MenuNum;
	}
	else
	{
		index = (index + menu_parm.MenuNum - 1) % menu_parm.MenuNum;
	}

	menu_parm.FocusMenuIndex = index;
		
	return AK_TRUE;
}



/**
* @brief Save selectItem to focusItem
*
* @author Songmengxing
* @date 2013-04-10
*
* @param T_VOID
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_Confirm(T_VOID)
{
	if ((AK_NULL == menu_parm.pMenu)
		|| (0 == menu_parm.MenuNum))
	{
		return AK_FALSE;
	}

	menu_parm.pMenu[menu_parm.FocusMenuIndex].FocusItem 
		= menu_parm.pMenu[menu_parm.FocusMenuIndex].SelectItem;
	
	return AK_TRUE;
}



/**
* @brief Get Focus menu id and focus item Id
*
* @author Songmengxing
* @date 2013-04-10
*
* @param out T_U8* Menu : focus Submode Id
* @param out T_U8* Item : focus item id
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_GetFocus(T_U8 *Menu, T_U8 *Item)
{
	AK_ASSERT_PTR(Menu, "MenuCfg_GetFocus Menu", AK_FALSE);
	AK_ASSERT_PTR(Item, "MenuCfg_GetFocus Item", AK_FALSE);

	if ((AK_NULL == menu_parm.pMenu)
		|| (0 == menu_parm.MenuNum))
	{
		*Menu = MENU_ERROR_ID;
		*Item = MENU_ERROR_ID;
		
		return AK_FALSE;
	}

	*Menu = menu_parm.pMenu[menu_parm.FocusMenuIndex].Menu;
	*Item = menu_parm.pMenu[menu_parm.FocusMenuIndex].FocusItem;
	
	return AK_TRUE;
}


/**
* @brief move selectItem
*
* @author Songmengxing
* @date 2013-04-10
*
* @param in T_S8 dir : 1, next; -1, previous
*
* @return T_BOOL
* @retval 
*/
T_BOOL MenuCfg_MoveItem(T_S8 dir)
{
	T_U8 index = MENU_ERROR_ID;
	
	if ((AK_NULL == menu_parm.pMenu)
		|| (0 == menu_parm.MenuNum))
	{
		return AK_FALSE;
	}

	index = menu_parm.FocusMenuIndex;

	if (MOVE_NEXT == dir)
	{
		menu_parm.pMenu[index].SelectItem = (menu_parm.pMenu[index].SelectItem + 1) 
											% menu_parm.pMenu[index].ItemNum;
	}
	else
	{
		menu_parm.pMenu[index].SelectItem = (menu_parm.pMenu[index].SelectItem 
											+ menu_parm.pMenu[index].ItemNum - 1) 
											% menu_parm.pMenu[index].ItemNum;
	}	

	return AK_TRUE;
}



/**
* @brief Get string for print
*
* @author Songmengxing
* @date 2013-04-10
*
* @param T_VOID
*
* @return T_S8*
* @retval string pointer
*/
T_S8* MenuCfg_GetStr(T_VOID)
{
	T_U8 index = MENU_ERROR_ID;
	
	if ((AK_NULL == menu_parm.pMenu)
		|| (0 == menu_parm.MenuNum))
	{
		return AK_NULL;
	}

	index = menu_parm.FocusMenuIndex;

	if (AK_NULL != menu_parm.pMenu[index].pStr)
	{
		return menu_parm.pMenu[index].pStr[menu_parm.pMenu[index].SelectItem];
	}

	return AK_NULL;
}


