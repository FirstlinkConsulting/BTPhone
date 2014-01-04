#include "Ctrl_TopBar.h"
//#include "Eng_AutoOff.h"
#include "Gbl_ImageRes.h"
//#include "Fwl_RTC.h"
//#include "Gui_Common.h"
#include "Gbl_Global.h"

#if(NO_DISPLAY == 0)
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#if (STORAGE_USED == SPI_FLASH)
const T_RES_IMAGE topbar_image_time[TOPBAR_TIME_IMAGE_NUMS] =
{
        eRES_IMAGE_REC_SYSTIME_0_0,
        eRES_IMAGE_REC_SYSTIME_0_1,
        eRES_IMAGE_REC_SYSTIME_0_2,
        eRES_IMAGE_REC_SYSTIME_0_3,
        eRES_IMAGE_REC_SYSTIME_0_4,
        eRES_IMAGE_REC_SYSTIME_0_5,
        eRES_IMAGE_REC_SYSTIME_0_6,
        eRES_IMAGE_REC_SYSTIME_0_7,
        eRES_IMAGE_REC_SYSTIME_0_8,
        eRES_IMAGE_REC_SYSTIME_0_9
};
#else
const T_RES_IMAGE topbar_image_time[TOPBAR_TIME_IMAGE_KINDS][TOPBAR_TIME_IMAGE_NUMS] =
{
    {
        eRES_IMAGE_REC_SYSTIME_0_0,
        eRES_IMAGE_REC_SYSTIME_0_1,
        eRES_IMAGE_REC_SYSTIME_0_2,
        eRES_IMAGE_REC_SYSTIME_0_3,
        eRES_IMAGE_REC_SYSTIME_0_4,
        eRES_IMAGE_REC_SYSTIME_0_5,
        eRES_IMAGE_REC_SYSTIME_0_6,
        eRES_IMAGE_REC_SYSTIME_0_7,
        eRES_IMAGE_REC_SYSTIME_0_8,
        eRES_IMAGE_REC_SYSTIME_0_9
    }, 
    {
        eRES_IMAGE_REC_SYSTIME_1_0,
        eRES_IMAGE_REC_SYSTIME_1_1,
        eRES_IMAGE_REC_SYSTIME_1_2,
        eRES_IMAGE_REC_SYSTIME_1_3,
        eRES_IMAGE_REC_SYSTIME_1_4,
        eRES_IMAGE_REC_SYSTIME_1_5,
        eRES_IMAGE_REC_SYSTIME_1_6,
        eRES_IMAGE_REC_SYSTIME_1_7,
        eRES_IMAGE_REC_SYSTIME_1_8,
        eRES_IMAGE_REC_SYSTIME_1_9
    },   
    {   
        eRES_IMAGE_REC_SYSTIME_2_0,
		eRES_IMAGE_REC_SYSTIME_2_1,
		eRES_IMAGE_REC_SYSTIME_2_2,
		eRES_IMAGE_REC_SYSTIME_2_3,
		eRES_IMAGE_REC_SYSTIME_2_4,
		eRES_IMAGE_REC_SYSTIME_2_5,
		eRES_IMAGE_REC_SYSTIME_2_6,
		eRES_IMAGE_REC_SYSTIME_2_7,
		eRES_IMAGE_REC_SYSTIME_2_8,
		eRES_IMAGE_REC_SYSTIME_2_9
    },  
    {
        eRES_IMAGE_REC_SYSTIME_3_0,
		eRES_IMAGE_REC_SYSTIME_3_1,
		eRES_IMAGE_REC_SYSTIME_3_2,
		eRES_IMAGE_REC_SYSTIME_3_3,
		eRES_IMAGE_REC_SYSTIME_3_4,
		eRES_IMAGE_REC_SYSTIME_3_5,
		eRES_IMAGE_REC_SYSTIME_3_6,
		eRES_IMAGE_REC_SYSTIME_3_7,
		eRES_IMAGE_REC_SYSTIME_3_8,
		eRES_IMAGE_REC_SYSTIME_3_9
    }
};
#endif
#endif

#pragma arm section code = "_listfile_init_"
/**
 * @brief	TopBar ³õÊ¼»¯
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:	
 * @param	[in] titleID:	
 * @param	[in] bSysTimeShow:	
 * @param	[in] bBatteryShow:	
 * @return	T_BOOL
 * @retval	AK_TRUE :  ³õÊ¼»¯³É¹¦
 * @retval	AK_FALSE :	³õÊ¼»¯Ê§°Ü
 */
T_BOOL	TopBar_init(T_TOP_BAR_CTRL *topbarctrl ,T_U16 titleID ,T_BOOL  bSysTimeShow ,T_BOOL  bBatteryShow)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl)
		return AK_FALSE;
	
	topbarctrl->titleID = titleID;
	if(TOPBAR_INVALID_TITLEID != topbarctrl->titleID)
	{
		
		topbarctrl->oldTitleWidth = (T_U8)GetStringDispWidth(CP_UNICODE, CONVERT2STR(topbarctrl->titleID), 0);
		if(topbarctrl->oldTitleWidth != 0)
		{
			topbarctrl->bTitleShow = AK_TRUE;
		}
		else
		{
			topbarctrl->bTitleShow = AK_FALSE;
		}
	}
	else
	{
		topbarctrl->bTitleShow = AK_FALSE;
		topbarctrl->oldTitleWidth = 0;
	}
	topbarctrl->bSysTimeShow = bSysTimeShow;
	topbarctrl->bBatteryShow = bBatteryShow;
	topbarctrl->bTchDwn = AK_FALSE;
	topbarctrl->oldBatterVal = 0xff;
	topbarctrl->oldSysTimeMin = 0xff;
	topbarctrl->TimeCount = 0;
	topbarctrl->reflesh = TOPBAR_REFLESH_ALL;
	topbarctrl->bTopbarShow = AK_TRUE;
	topbarctrl->focusWhat = TOPBAR_FOCUS_NONE;
	#ifdef OS_ANYKA
	topbarctrl->disBatLev = 0;
	#endif
#endif
	return AK_TRUE;
}

#pragma arm section code

#pragma arm section code = "_listfile_show_"

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
/**
 * @brief	TopBar ±³¾°ÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] T_VOID:	
 * @return	T_VOID
 */
static T_VOID disp_topbar_bk_img(T_VOID)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	Eng_ImageResDisp(TOPBAR_LEFT ,TOPBAR_TOP ,TOPBAR_BK_IMG ,AK_FALSE);
#endif
}

/**
 * @brief	TopBar ±êÌâÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:	
 * @return	T_VOID
 */
static T_VOID disp_topbar_title(T_TOP_BAR_CTRL *topbarctrl )
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(topbarctrl->bTitleShow)
	{
		T_U8 tmp = (T_U8)GetStringDispWidth(CP_UNICODE, CONVERT2STR(topbarctrl->titleID), 0);
//		if (topbarctrl->oldTitleWidth != 0)
		{
			Eng_ImageResDispEx(TOPBAR_TITLE_LEFT, TOPBAR_TITLE_TOP, TOPBAR_BK_IMG, \
			(T_U16)(topbarctrl->oldTitleWidth + FONT_WIDTH), FONT_HEIGHT, TOPBAR_TITLE_LEFT, TOPBAR_TITLE_TOP, AK_FALSE);
			topbarctrl->oldTitleWidth = tmp;
		}
		if (TOPBAR_FOCUS_TITLE & topbarctrl->focusWhat)
			DispStringOnColor(CP_UNICODE, TOPBAR_TITLE_LEFT, TOPBAR_TITLE_TOP, (T_U8*)topbarctrl->titleID, MAIN_LCD_WIDTH,CLR_WHITE, CLR_BLUE);
		else
			DispStringInWidth(CP_UNICODE, TOPBAR_TITLE_LEFT, TOPBAR_TITLE_TOP, (T_U8*)topbarctrl->titleID, MAIN_LCD_WIDTH,CLR_WHITE);
	}
#endif

}

/**
 * @brief	TopBar Ê±¼äÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:	
 * @return	T_VOID
 */
static T_VOID disp_topbar_time(T_TOP_BAR_CTRL *topbarctrl)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
//	T_U8 tmp = 0;
	if(topbarctrl->bSysTimeShow)
	{
		T_SYSTIME  date;
	    
	    #ifdef OS_WIN32
	    T_U32 tick;       
	    tick = Fwl_GetSecond();
	    Utl_Convert_SecondToDate(tick, &date);
	    #else
	    Fwl_GetRTCtime(&date);
	    #endif

		if(date.minute != topbarctrl->oldSysTimeMin)
		{	
			topbarctrl->oldSysTimeMin = date.minute;
			Gui_DispTime(TOPBAR_SYSTIME_LEFT ,TOPBAR_SYSTIME_TOP ,topbar_image_time ,eRES_IMAGE_TNUM910);
		}
	}
#endif
}

/**
 * @brief	TopBar ÒôÀÖÍ¼±êÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:	
 * @return	T_VOID
 */
static T_VOID disp_topbar_music_icon(T_TOP_BAR_CTRL *topbarctrl )
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	Eng_ImageResDispEx(TOPBAR_MUSIC_ICON_LEFT, TOPBAR_MUSIC_ICON_TOP, TOPBAR_BK_IMG, \
	Eng_GetResImageWidth(TOPBAR_MUSIC_ICON), Eng_GetResImageHeight(TOPBAR_MUSIC_ICON), TOPBAR_MUSIC_ICON_LEFT, TOPBAR_MUSIC_ICON_TOP, AK_FALSE);
	Gui_DispBackgroundPlay(TOPBAR_MUSIC_ICON_LEFT ,TOPBAR_MUSIC_ICON_TOP ,TOPBAR_MUSIC_ICON);
//	{
//		Eng_ImageResDisp(TOPBAR_MUSIC_ICON_LEFT, TOPBAR_MUSIC_ICON_TOP, TOPBAR_MUSIC_ICON ,AK_FALSE);
//	}
#endif
}

/**
 * @brief	TopBar µçÁ¿ÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] x:
 * @param	[in] y:
 * @param	[in] BatLev:µçÁ¿¶àÉÙ
 * @return	T_VOID
 */
static T_BOOL TopBar_DispBat(T_POS x, T_POS y,  T_U32 BatLev)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	switch(BatLev)
	{
		case BATTERY_STAT_LOW_WARN:
		case BATTERY_STAT_NOR_L0:
        case BATTERY_STAT_LOW_SHUTDOWN:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_NONE, AK_FALSE);			
		   break;
		case BATTERY_STAT_NOR_L1:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_ONE, AK_FALSE);			
		   break;
		case BATTERY_STAT_NOR_L2:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_TWO, AK_FALSE);			
		   break;
		case BATTERY_STAT_NOR_L3:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_THREE, AK_FALSE);			
		   break;
		case BATTERY_STAT_NOR_FULL:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_FULL, AK_FALSE);			
		   break;
		default:
		   Eng_ImageResDisp(x, y, eRES_IMAGE_PUB_BATTERY_FULL, AK_FALSE);			
		   break;
	}
#endif
	return AK_TRUE;
}

/**
 * @brief	TopBar µçÁ¿ÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] busb_detect:ÊÇ·ñÁ¬½Óusb
 * @return	T_VOID
 */
static T_VOID disp_topbar_bat(T_TOP_BAR_CTRL *topbarctrl ,T_BOOL busb_detect)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(topbarctrl->bBatteryShow)
	{
		T_U8 tempBatLev = 0;

#ifdef OS_ANYKA
//		if(USB_Detect() && BATTERY_STAT_EXCEEDVOLT != gb.batStat)
		if(busb_detect)
		{//charging
			topbarctrl->disBatLev = ((topbarctrl->disBatLev+1) % BATTERY_STAT_NOR_FULL);
			tempBatLev = topbarctrl->disBatLev+1;
		}
		else
#endif
		{
			tempBatLev = gb.batStat;
		}
		if(tempBatLev != topbarctrl->oldBatterVal)
		{	
			TopBar_DispBat(TOPBAR_BAT_ICON_LEFT, TOPBAR_BAT_ICON_TOP, tempBatLev);
			topbarctrl->oldBatterVal= tempBatLev;
		}
	}
#endif
}

T_VOID TopBar_DispBatIcon(T_VOID)
{
    TopBar_DispBat(TOPBAR_BAT_ICON_LEFT, TOPBAR_BAT_ICON_TOP, gb.batStat);
}

#pragma arm section code

#endif

/************************************************
show ½Ó¿ÚÓ¦·ÅÓÚ×´Ì¬»úµÄpaintº¯ÊýÖÐ£»
************************************************/
/**
 * @brief	TopBar ÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @return	T_BOOL
 */
T_BOOL  TopBar_show(T_TOP_BAR_CTRL *topbarctrl )
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl->bTopbarShow)
		return AK_FALSE;
	if(!topbarctrl)
		return AK_FALSE;
	if(TOPBAR_REFLESH_NONE == topbarctrl->reflesh)
		return AK_FALSE;

	if(TOPBAR_REFLESH_BKIMG & topbarctrl->reflesh)
	{
		disp_topbar_bk_img();
	}
	if(TOPBAR_REFLESH_TITLE & topbarctrl->reflesh)
	{		
		disp_topbar_title(topbarctrl);
	}
	if(TOPBAR_REFLESH_TIME & topbarctrl->reflesh)
	{
		disp_topbar_time(topbarctrl);
		topbarctrl->TimeCount = 0;
	}
	if(TOPBAR_REFLESH_MUSICICON & topbarctrl->reflesh)
	{
		disp_topbar_music_icon(topbarctrl);
	}
	if(TOPBAR_REFLESH_BATTERY & topbarctrl->reflesh)
	{
		disp_topbar_bat(topbarctrl ,AK_FALSE);	
	}
	topbarctrl->reflesh = TOPBAR_REFLESH_NONE;
#endif
	return AK_TRUE;
}

#pragma arm section code 


#pragma arm section code = "_listfile_handle_"


/************************************************
TopBar_Handle ½Ó¿ÚÓ¦·ÅÓÚ×´Ì¬»úµÄhandleº¯ÊýÖÐ;
Ã¿¸ô5Ãë¾Í»áË¢ÐÂÉèÖÃÒ»ÏÂµç³Ø£¬±³¾°ÒôÀÖ£¬ÏµÍ³Ê±¼äµÄË¢ÐÂ±êÖ¾£»
È»ºóÔÚTopBar_showÖ´ÐÐË¢ÐÂ£»
************************************************/
T_U32  TopBar_Handle(T_TOP_BAR_CTRL  *topbarctrl , T_EVT_CODE  Event, T_EVT_PARAM  *pParam)
{
	T_U32 tmp = TOPBAR_FOCUS_NONE;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	switch (Event)
	{
		case M_EVT_PUB_TIMER:
		{
			T_U8 reflesh = TOPBAR_REFLESH_NONE;
		#ifdef OS_ANYKA
			if(Fwl_Charge_CntState() && BATTERY_STAT_EXCEEDVOLT != gb.batStat)
			{
			//	disp_topbar_bat(topbarctrl ,AK_TRUE);
				reflesh = TOPBAR_REFLESH_TIME|TOPBAR_REFLESH_MUSICICON|TOPBAR_REFLESH_BATTERY;
			}
		#endif
			if (TOPBAR_REFLESH_MAX_COUNT < topbarctrl->TimeCount++)
			{
				reflesh = TOPBAR_REFLESH_TIME|TOPBAR_REFLESH_MUSICICON|TOPBAR_REFLESH_BATTERY;
				topbarctrl->TimeCount = 0;
			}
			TopBar_SetReflesh(topbarctrl ,reflesh);
//			if (reflesh != TOPBAR_REFLESH_NONE)
//				return TOPBAR_RESPONSE_OPERATED;
		}
		break;
		case M_EVT_USER_KEY:
			break;
		default :
			break;
	}
#endif
	return tmp;
}

#pragma arm section code 

T_VOID  TopBar_Free(T_TOP_BAR_CTRL  *topbarctrl)
{
	
}

//ÉèÖÃTopBarÊÇ·ñÏÔÊ¾
/**
 * @brief	ÉèÖÃTopBarÊÇ·ñÏÔÊ¾
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] bshow:
 * @return	T_BOOL
 */
T_BOOL	TopBar_ConfigbShowSet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL bshow)
{
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	if(!topbarctrl)
		return AK_FALSE;
	topbarctrl->bTopbarShow = bshow;
	#endif
	return AK_TRUE;
}

//ÉèÖÃTopBarµÄ±êÌâ£»
/**
 * @brief	ÉèÖÃTopBarµÄ±êÌâ£»
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] title:
 * @return	T_BOOL
 */
T_BOOL	TopBar_ConfigTitleSet(T_TOP_BAR_CTRL  *topbarctrl ,T_U16 title)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl)
		return AK_FALSE;
	
	topbarctrl->titleID = title;
	if(TOPBAR_INVALID_TITLEID != topbarctrl->titleID)
	{
		topbarctrl->bTitleShow = AK_TRUE;
	}
	else
	{
		topbarctrl->bTitleShow = AK_FALSE;
	}
#endif
	return AK_TRUE;
}

//ÉèÖÃTopBarÊÇ·ñÏÔÊ¾ÏµÍ³Ê±¼ä£»
/**
 * @brief	ÉèÖÃTopBarÊÇ·ñÏÔÊ¾ÏµÍ³Ê±¼ä£»
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] bShow:
 * @return	T_BOOL
 */
T_BOOL	TopBar_ConfigSysTimeSet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL  bShow)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl)
		return AK_FALSE;
	topbarctrl->bSysTimeShow = bShow;
#endif
	
	return AK_TRUE;
}

//ÉèÖÃTopBarÊÇ·ñÏÔÊ¾µç³Ø£»
/**
 * @brief	ÉèÖÃTopBarÊÇ·ñÏÔÊ¾µç³Ø£
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] bShow:
 * @return	T_BOOL
 */
T_BOOL	TopBar_ConfigBatterySet(T_TOP_BAR_CTRL  *topbarctrl ,T_BOOL  bShow)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl)
		return AK_FALSE;
	topbarctrl->bBatteryShow = bShow;
#endif
	return AK_TRUE;
}

//ÉèÖÃTopBarµÄË¢ÐÂ±êÖ¾£»

#pragma arm section code = "_listfile_show_"
/**
 * @brief	ÉèÖÃTopBarµÄË¢ÐÂ±êÖ¾£»
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] reflesh:
 * @return	T_BOOL
 */
T_BOOL TopBar_SetReflesh(T_TOP_BAR_CTRL  *topbarctrl ,T_U8 reflesh)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))

	if(!topbarctrl)
		return AK_FALSE;
	topbarctrl->reflesh |= reflesh;
	topbarctrl->oldBatterVal = 0xff;
	topbarctrl->oldSysTimeMin = 0xff;
#endif
	return AK_TRUE;
}
 
#pragma arm section code
/**
 * @brief	ÉèÖÃTopBar¾Û½¹ÔÚÄÇ¸öÄ£¿é
 * @author	lishengkai
 * @date	
 * @param	[in] topbarctrl:
 * @param	[in] Focus:
 * @return	T_BOOL
 */
T_BOOL TopBar_ConfigFocusSet(T_TOP_BAR_CTRL * topbarctrl, T_U8 Focus)
{
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	if(!topbarctrl)
		return AK_FALSE;
	topbarctrl->focusWhat = Focus;
	#endif
	return AK_TRUE;
}

#endif

