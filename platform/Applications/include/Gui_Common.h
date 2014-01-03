
/**
 * @file Gui_Common.h
 * @brief ANYKA software
 * this file will provide gui painting function .
 *
 * @author XuPing
 * @date    2008-1013
 * @version 1.0
 */

#ifndef __GUI_COMMMON_H__
#define __GUI_COMMMON_H__

#if(NO_DISPLAY == 1)

#define Gui_DispResHint(ResID, fontColor, backColor, startY)

#else

typedef enum
{
    eAudPlayer = 0,
    eRecord,
    eRadio,
    eEBook,
    eAlarmSet,
    eSysSet,
    eImage,
    eGame,
    eCustomAdd,
    eOthers
}T_ePlayingType;


#include "akdefine.h"
#include "Fwl_LCD.h"
#include "Ctrl_ListFile.h"
#include "Ctrl_ListMenu.h"
#include "Ctrl_Progress.h"
#include "Ctrl_Dialog.h"

#define LCD_BUF_LEN    ((MAIN_LCD_WIDTH* MAIN_LCD_HEIGHT)/(FONT_HEIGHT* FONT_WIDTH)+ 1)
#define  RESSTRING_LEN         256

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#define TITLE_HEIGHT            25
#define DISPLAY_TITLE(eResImage)
#else
#define TITLE_HEIGHT            32
#define DISPLAY_TITLE(eResImage)       Eng_ImageResDisp(0, 0, eResImage, AK_FALSE);
#endif
#define IMAGE_ALARM_SET         eRES_IMAGE_TOP_CLOCK
#define GET_KEY_ID(pEvtParam)   (pEvtParam->c.Param1)
#define GET_KEY_TYPE(pEvtParam)    (pEvtParam->c.Param2)
#define GET_TIMER_ID(pEvtParam)     (pEvtParam->w.Param1)
#define GET_TIMER_DELAY(pEvtParam)   (pEvtParam->w.Param2)

/**************************************************************************
* @brief Get resStirng ,and '\0' in the end of the string 
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_U16 Gui_GetResString(T_U16* ucBuffer, T_RES_STRING resStringID, T_U16 bufferLen);

/**************************************************************************
* @brief init list progress support all lcd size
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL Gui_InitProgress(CProgressCtrl *pProgress, T_S16 minValue, T_S16 maxValue, T_U16 titleStringID, T_FLOAT curValue,  T_RES_IMAGE bkImg);

T_BOOL Gui_InitListMenuEx(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID);
/**************************************************************************
* @brief init list menu support all lcd size
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL  Gui_InitListMenu(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID, T_BOOL isShowEndMemu);

/**************************************************************************
* @brief init list dialog support all lcd size
* 
* @author zhao_xiaowei
* @date 2009-9
* @param 
* @return 
***************************************************************************/
T_BOOL  Gui_InitDialog(CDialogCtrl *pDialog, T_U16* hintString, T_U16 titleID, T_U16 modeType, T_U16 defaultMode);

#if (3 == LCD_TYPE && 1 == LCD_HORIZONTAL)
/**
 * @brief   display background play statue 
 * @author  luqizhou
 * @date    2011-03-04
 * @param   
 * @return  T_VOID
 * @retval  
 **/
T_VOID Gui_DispBackgroundPlay(T_POS x, T_POS y ,T_RES_IMAGE imgID);


/**
 * @brief   display time 
 * @author  luqizhou
 * @date    2011-03-04
 * @param   dataBuf: number images resource matrix
 * @return  T_VOID
 * @retval  
 **/
#if (STORAGE_USED == SPI_FLASH)
T_VOID Gui_DispTime(T_POS x, T_POS y, const T_RES_IMAGE dataBuf[10],T_RES_IMAGE ColonID);
#else
T_VOID Gui_DispTime(T_POS x, T_POS y, const T_RES_IMAGE dataBuf[4][10],T_RES_IMAGE ColonID);
#endif
/**
 * @brief   display batter info 
 * @author  XuPing
 * @date    2008-10-13
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
 #endif
T_BOOL Gui_DispBat(T_POS x, T_POS y);

/**
 * @brief   Gui_SetClrTitle
 * @author  Set UI title of color LCD
 * @date    2008-10-13
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/

T_VOID  Gui_SetClrTitle(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID, T_U16 imgID);

/**
 * @brief   get title image ID
 * @author  YHS
 * @date    2008-09-11
 * @param   
 * @return  ImageID
 **/
#if (USE_COLOR_LCD)
#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
T_U16 GetUITitleStrID(T_VOID);
#endif
T_U16 GetUITitleResID(T_VOID);
T_BOOL Gui_DispTitle(T_RES_STRING stringID, T_U16 fontColor, T_RES_IMAGE backImgID, T_U16 titleHight);
#endif
T_VOID Gui_FillRect(T_U16 x, T_U16 y, T_U16 width, T_U16 height, T_U16 backColor);
T_VOID Gui_DispHint(T_U16* pString, T_U16 fontColor, T_U16 backColor, T_S16 startY);
T_VOID Gui_DispResHint(T_U16 ResID, T_U16 fontColor, T_U16 backColor, T_S16 startY);
#endif

#endif
