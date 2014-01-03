/**
 * @file Ctrl_MenuConfig.h
 * @brief ANYKA software
 * 
 * @author 
 * @date  
 * @version 1,0 
 */


#ifndef _CTRL_MENUCONFIG_H_
#define _CTRL_MENUCONFIG_H_

#include "anyka_types.h"


#define RETURN_FROM_SET_MENU    (0x1234)
#define MAX_SHOW_LEN            (20)
#define MENU_ERROR_ID           (0xff)
#define MOVE_NEXT				(1)
#define MOVE_PREVIOUS			(-1)


typedef T_S8 T_STR_SHOW[MAX_SHOW_LEN];



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
T_BOOL MenuCfg_Init(T_U8 MenuMaxNum);


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
T_VOID MenuCfg_Free(T_VOID);


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
T_BOOL MenuCfg_AddMenu(T_U8 Menu, T_U8 ItemNum, T_U8 FocusItem, T_STR_SHOW *pStr);




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
T_BOOL MenuCfg_MoveMenu(T_S8 dir);



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
T_BOOL MenuCfg_Confirm(T_VOID);



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
T_BOOL MenuCfg_GetFocus(T_U8 *Menu, T_U8 *Item);


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
T_BOOL MenuCfg_MoveItem(T_S8 dir);



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
T_S8* MenuCfg_GetStr(T_VOID);


#endif	//_CTL_MENU_CONFIG_H_

