#include "Gbl_global.h"
#include "Ctrl_ListMenu.h"
#include "Ctrl_Button.h"


#if (USE_COLOR_LCD)
#include "log_aud_play.h"
#endif

#if(NO_DISPLAY == 0)

//控件刷新子标记
#define CTRL_REFRESH_MENULIST_TITLE		0x1 //only repaint title
#define CTRL_REFRESH_MENULIST_ENTRY		0x2 //only repaint menu entry
#define CTRL_REFRESH_MENULIST_ENTRYEX_ALL 0xFFFF

//菜单显示条目个数
#define CTRL_MENULIST_DISP_ENTRYS		(pListMenu->entryLines)				

//
#define RES_GET_LOG_LEN(begID, endID)	GetResCount(begID, endID, CTRL_CUR_LANGUAGE)
#define RES_GET_PHY_LOC(begID, offset)	GetNearbyValidStrResID(begID, CTRL_CUR_LANGUAGE, offset)

#ifdef USE_CONTROL_NOTITLE
#define CTRL_MENULIST_BEAUTIFY_INTVL    2
#else
#define CTRL_MENULIST_BEAUTIFY_INTVL    0
#endif


////////////////////////////////////////
static T_S16	ListMenu_GetItemId(CListMenuCtrl *pListMenu, T_U16 id)
{
	if(pListMenu->idLoc) return pListMenu->idLoc[id];
	else				 return id;
}

static T_U32	ListMenu_GetString(CListMenuCtrl *pListMenu, T_U16 id)
{
	T_U32 string = INVALID_STRINGRES_ID;

	//get menu entry string
	if((id==pListMenu->stopID) && (CTRL_LISTMENU_STYLE_QUIT_DISPSHOW & pListMenu->dwStyle))
		string = STRING_QUIT_ENTRYID;//auto 'EXIT'
	else
	{
		TP_MListStringGet pStringGet= pListMenu->pStringGet;
		//菜单条目内容或者来自资源配置或者来自动态字符串数组
		if(AK_NULL == pListMenu->stringArray)
		{
			if(AK_NULL == pListMenu->pStringGet)
			{
				string = RES_GET_PHY_LOC(pListMenu->startID, (ListMenu_GetItemId(pListMenu, id)));//string resource ID is not one by one order
			}
			else
			{
				string = (T_U32)pStringGet->fItemGet(pStringGet->type, pStringGet->bSublist, pStringGet->classId, pStringGet->str, id);
			}
		}
		else
		{
			if(pListMenu->pStringGet != AK_NULL)
			{			
				string = (T_U32)pStringGet->fItemGet(pStringGet->type, pStringGet->bSublist, pStringGet->classId, pStringGet->str, id);
			}
			else
			{
				string = (T_U32)(pListMenu->stringArray[id]);
			}
		}
	}

	return string;
}

static T_U32	ListMenu_GetIcon(CListMenuCtrl *pListMenu, T_BOOL bHit)
{
	T_U32 iconPtr = INVALID_IMAGERES_ID;

	//get menu entry icon
	if(bHit)
	{
		//焦点菜单条目图标或者来自资源配置或者来自动态设定
		if(INVALID_IMAGERES_ID == pListMenu->hitIcon)
			iconPtr = ICON_SELECT_HINTPTR;
		else
			iconPtr = pListMenu->hitIcon;
	}
	else
	{
		//普通菜单条目图标或者来自资源配置或者来自动态设定
		if(INVALID_IMAGERES_ID == pListMenu->cmmIcon)
			iconPtr = pListMenu->iconPtr;
		else
			iconPtr = pListMenu->cmmIcon;
	}

	if (INVALID_IMAGERES_ID != iconPtr)
	{
		#if (USE_COLOR_LCD)
		if(INVALID_IMAGERES_ID == pListMenu->cmmIcon)
			iconPtr = ICON_COMMON_IMAGEPTR;
		#endif
	}	
	return iconPtr;
}

////////////////////////////////////////


T_BOOL	ListMenu_InitEx(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE menuIconPtr, T_POS left, T_POS top, T_LEN width, T_LEN height)
{
	if(!pListMenu)
		return AK_FALSE;

	pListMenu->startID	= menuBegID;
	pListMenu->stopID   = RES_GET_LOG_LEN(menuBegID, menuEndID);//last entry logic index
	pListMenu->begSrID	= 0;
	pListMenu->curSubID = 0;
	pListMenu->titleID  = titleID;
	pListMenu->iconPtr  = menuIconPtr;
	pListMenu->dwStyle  = CTRL_LISTMENU_STYLE_DEFAULT;
	pListMenu->refresh  = CTRL_REFRESH_ALL;
	pListMenu->refreshEx = CTRL_REFRESH_MENULIST_ENTRYEX_ALL;

	pListMenu->stringArray = AK_NULL;
	pListMenu->cmmIcon  = INVALID_IMAGERES_ID;
	pListMenu->hitIcon  = INVALID_IMAGERES_ID;
	pListMenu->code     = CP_UNICODE;

	pListMenu->idLoc	= AK_NULL;

	pListMenu->navigateID = INVALID_STRINGRES_ID;
	pListMenu->bkimgTitle = INVALID_IMAGERES_ID;

	pListMenu->left = left;
	pListMenu->top  = top;
	pListMenu->width = width;
	pListMenu->height = height;

	pListMenu->pTxtScroll = AK_NULL;
	pListMenu->bResetTxtScroll = AK_TRUE;
	
	#ifndef USE_CONTROL_NOTITLE 
	{
		pListMenu->left   = CTRL_WND_LEFT;
		pListMenu->top    = CTRL_WND_TOP+CTRL_WND_TITLEHTH;
		pListMenu->width  = CTRL_WND_WIDTH;
		pListMenu->height = CTRL_WND_HEIGHT-pListMenu->top;
	}
	#endif

	pListMenu->entryLines = (T_U16)((pListMenu->height-CTRL_MENULIST_BEAUTIFY_INTVL)/CTRL_WND_LINEHIGH);

	//keypad_start_intervaltimer(AK_TRUE);
	pListMenu->pStringGet= AK_NULL;

	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	pListMenu->top  = TITLE_HEIGHT;
	pListMenu->height = MAIN_LCD_HEIGHT - TITLE_HEIGHT;
	return TopBar_init(&pListMenu->listmenu_topbar, pListMenu->titleID, AK_TRUE ,AK_TRUE);
	#endif
	return AK_TRUE;
}

T_VOID  ListMenu_Free(CListMenuCtrl *pListMenu)
{
	if(!pListMenu)
		return;

	//keypad_start_intervaltimer(AK_FALSE);
	if(pListMenu->idLoc)
		pListMenu->idLoc = Fwl_Free(pListMenu->idLoc);

	if(pListMenu->pTxtScroll)
	{
		TxtScroll_Free(pListMenu->pTxtScroll);
		pListMenu->pTxtScroll = Fwl_Free(pListMenu->pTxtScroll);
	}
}

#ifndef USE_CONTROL_NOTITLE 
static T_VOID ListMenu_Show_InternalTitle(CListMenuCtrl *pListMenu)
{
	if(CTRL_REFRESH_ALL == pListMenu->refresh)
	{
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, CTRL_WND_BACKCOLOR);
		#else
		Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP, TITLE_BKG_IMAGEPTR, AK_FALSE);
		#endif
	}

	if(CTRL_REFRESH_MENULIST_TITLE & pListMenu->refresh)
	{
		Display_Title(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, pListMenu->titleID, pListMenu->bkimgTitle, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);		

		if(INVALID_STRINGRES_ID != pListMenu->navigateID)
			Display_NavgateBar(CTRL_WND_LEFT, CTRL_WND_TOP+CTRL_WND_TITLEHTH, CTRL_WND_WIDTH, CTRL_WND_LINEHIGH, pListMenu->navigateID);		
	}
}
#endif

T_VOID	ListMenu_Show(CListMenuCtrl *pListMenu)
{
	T_U16 i, j;
	T_U16 id;
	T_POS x;
	#if (USE_COLOR_LCD)
	T_BG_PIC pic;
	#endif 
	T_BOOL bChangeFreq= AK_FALSE;

	if(!pListMenu)
		return;
	if(pListMenu->pStringGet != AK_NULL && pListMenu->refresh != CTRL_REFRESH_NONE)
	{
		Fwl_FreqPush(FREQ_APP_MAX);
		bChangeFreq= AK_TRUE;
	}

	if(pListMenu->dwStyle & CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT)
		TxtScroll_Show(pListMenu->pTxtScroll);

	#ifndef USE_CONTROL_NOTITLE 
	ListMenu_Show_InternalTitle(pListMenu);
	#endif

	
	if(CTRL_REFRESH_ALL == pListMenu->refresh)
	{
		//clear and repaint LCD with color or background image
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(pListMenu->left, pListMenu->top, pListMenu->width, pListMenu->height, CTRL_WND_BACKCOLOR);
		#else	
		#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
//		TopBar_show(&pListMenu->listmenu_topbar);
		TopBar_SetReflesh(&pListMenu->listmenu_topbar, TOPBAR_REFLESH_ALL);
		Eng_ImageResDispEx(pListMenu->left, pListMenu->top, MENU_BACK_IMAGEPTR, pListMenu->width, pListMenu->height, pListMenu->left, pListMenu->top, AK_FALSE);
        #else
		Eng_ImageResDispEx(pListMenu->left, pListMenu->top, MENU_BACK_IMAGEPTR, pListMenu->width, pListMenu->height, 0, 0, AK_FALSE);
		#endif
		#endif
	}

#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
		TopBar_show(&pListMenu->listmenu_topbar);
#endif
	//repaint menu entry content
	if(CTRL_REFRESH_MENULIST_ENTRY & pListMenu->refresh)
	{
		T_U32 string = INVALID_STRINGRES_ID;
		T_RES_IMAGE icon = INVALID_IMAGERES_ID;
		T_U8 l = (T_U8)(pListMenu->refreshEx >> 8);
		T_U8 r = (T_U8)pListMenu->refreshEx;
#if (!USE_COLOR_LCD)
        T_U8 intvl = 8;
#endif
		T_U16  fcsId = 0;
		T_U16  fcsJ= 0;
		T_U16  fcsVal= 0;


		if((pListMenu->dwStyle & CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT)
					&& pListMenu->bResetTxtScroll)
		{
			TxtScroll_Pause(pListMenu->pTxtScroll);
		}

		//scroll ver
		//先画右侧的滚动条，再画左侧的菜单条目可以变面滚动条闪屏
		#if (!USE_COLOR_LCD)
		if(CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW & pListMenu->dwStyle)
		{
			//compute scroll bar location image
			x = pListMenu->left+pListMenu->width-SCROLL_VER_WIDTH;
			j = ((pListMenu->curSubID)*SCROLL_VER_LEVECNT)/(pListMenu->stopID+ 1);
			if(j >= SCROLL_VER_LEVECNT) j = SCROLL_VER_LEVECNT-1;
			Eng_ImageResDisp(x, pListMenu->top, SCROLL_VER_IMAGEPTR+j, AK_TRUE);

            //intvl = SCROLL_VER_WIDTH;
		}
		//Eng_ImageResDispEx(x, CTRL_WND_LINEHIGH, SCROLL_VER_IMAGEPTR, SCROLL_VER_WIDTH, CTRL_WND_LINEHIGH*CTRL_MENULIST_DISP_ENTRYS, 0, 0, AK_FALSE);
		#endif

		j = (T_U16)((pListMenu->height-(CTRL_WND_LINEHIGH*pListMenu->entryLines))/2+pListMenu->top+CTRL_MENULIST_BEAUTIFY_INTVL);

		if(CTRL_REFRESH_MENULIST_ENTRYEX_ALL == pListMenu->refreshEx)
		{
		    #if (USE_COLOR_LCD)
		    //增加decode解决进入菜单播放音频卡
		    
            #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
			Eng_ImageResDispEx(pListMenu->left, (T_POS)(j), MENU_BACK_IMAGEPTR, pListMenu->width, (T_POS)(pListMenu->height + pListMenu->top - j), 0, (T_POS)j, AK_FALSE);
            #else
			Eng_ImageResDispEx(pListMenu->left, (T_POS)(j), MENU_BACK_IMAGEPTR, pListMenu->width, (T_POS)(pListMenu->height-j), 0, (T_POS)j, AK_FALSE);
            #endif
            
			#endif	
		}

		//menu entry 图标+字符串
		for(i=0; i<CTRL_MENULIST_DISP_ENTRYS; i++)
		{
			id = (T_U16)(pListMenu->begSrID+i);

			if(id >= pListMenu->stopID+1)
				continue;

			if(CTRL_REFRESH_MENULIST_ENTRYEX_ALL != pListMenu->refreshEx)
			{
				//只显示更新条目, only repaint small region
				if(l!=i && r!=i)
				{
					j += CTRL_WND_LINEHIGH;
					continue;
				}
			}

			//get menu string
			string = ListMenu_GetString(pListMenu, id);

			//clear and repaint menu entry with color or background image 
			#if (!USE_COLOR_LCD)
			if(i+pListMenu->begSrID != pListMenu->curSubID)
				Fwl_FillRect(pListMenu->left, j, (T_U16)(pListMenu->width-intvl), CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
			else
				Fwl_FillRect(pListMenu->left, j, (T_U16)(pListMenu->width-intvl), CTRL_WND_LINEHIGH, CTRL_WND_FONTCOLOR);
			#else
			Eng_ImageResDispEx(pListMenu->left, j, MENU_BACK_IMAGEPTR, pListMenu->width, CTRL_WND_LINEHIGH, 0, (j), AK_FALSE);
			#endif
			
			//get menu icon
			x = pListMenu->left;
			icon = ListMenu_GetIcon(pListMenu, (T_BOOL)(i+pListMenu->begSrID==pListMenu->curSubID));

			if (INVALID_IMAGERES_ID != icon)
			{
				//draw icon
				#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
				Eng_ImageResDisp(x, (T_POS)j, icon, AK_FALSE);
				#else
				Eng_ImageResDisp(x, (T_POS)j, icon, IMG_TRANSPARENT);
				#endif
    			x += CTRL_WND_ICONWTH;
				#if (USE_COLOR_LCD)
					x += 8;
				#endif
				
			}

			//align and draw menu string within one LCD line width
			#if (!USE_COLOR_LCD)			
			DispStringOnColor(pListMenu->code, x, (T_POS)j, CONVERT2STR(string), (T_U16)(FONT_WND_WIDTH-intvl-CTRL_WND_ICONWTH), 
                (T_U16)(i+pListMenu->begSrID!=pListMenu->curSubID? CTRL_WND_FONTCOLOR:CTRL_WND_BACKCOLOR),
                (T_U16)(i+pListMenu->begSrID!=pListMenu->curSubID? CTRL_WND_BACKCOLOR:CTRL_WND_FONTCOLOR));
			
			#else
			pic.hOffset = (T_POS)x;
			pic.vOffset = (T_POS)j;
			pic.resId   = MENU_BACK_IMAGEPTR;

			if(i+pListMenu->begSrID != pListMenu->curSubID)
				DispStringOnPic(pListMenu->code, x, (T_POS)j, CONVERT2STR(string), (T_U16)(pListMenu->width -2-x), CTRL_WND_FONTCOLOR, &pic);
			else
			{							    
				pic.hOffset = (T_POS)(x - 2);
				pic.vOffset = (T_POS)0;
				pic.resId   = MENU_SELBAR_IMAGEPTR;
				
				if(AK_NULL == pListMenu->stringArray && AK_NULL == pListMenu->pStringGet)
					Eng_ImageResDisp((T_POS)(pListMenu->left+2), (T_POS)j, MENU_SELBAR_IMAGEPTR, AK_FALSE);
				else
				{
					pic.resId = (T_U16)(icon);
					//Eng_ImageResDispEx(x, (T_POS)j, MENU_SELBAR_IMAGEPTR, (T_U8)(pListMenu->width-4-x), CTRL_WND_FNTSZ, x, 0, AK_FALSE);
				}

				DispStringOnPic(pListMenu->code, x, (T_POS)j, CONVERT2STR(string), (T_U16)(pListMenu->width -2-x), CLR_WHITE, &pic);
				//Fwl_FillRect(x, (T_U8)j, pListMenu->width, CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
				//DispStringOnColor(pListMenu->code, x, (T_POS)j, CONVERT2STR(id), FONT_WND_WIDTH, CTRL_WND_SELFONT, CTRL_WND_BACKCOLOR);

			}
			#endif

			if((pListMenu->dwStyle & CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT)
				&& pListMenu->bResetTxtScroll && (i+pListMenu->begSrID == pListMenu->curSubID))
			{
				fcsId  = id;
				fcsVal = x;
				fcsJ   = j;
			}

			j += CTRL_WND_LINEHIGH;
		}

		if((pListMenu->dwStyle & CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT)
				&& pListMenu->bResetTxtScroll)
		{
			pListMenu->bResetTxtScroll = AK_FALSE;

			string = ListMenu_GetString(pListMenu, fcsId);

			#if (!USE_COLOR_LCD)
			TxtScrool_ReSetString(pListMenu->pTxtScroll, (T_U32)string, (T_BOOL)(AK_NULL==pListMenu->stringArray && AK_NULL == pListMenu->pStringGet),
					CTRL_WND_BACKCOLOR, pListMenu->code, fcsVal, fcsJ, (T_U16)(FONT_WND_WIDTH-intvl-CTRL_WND_ICONWTH),(T_U16)CTRL_WND_FONTCOLOR, AK_NULL);
			#else
			pic.hOffset = (T_POS)(fcsVal - 2);
			pic.vOffset = 0;
			pic.resId   = (T_U16)((AK_NULL == pListMenu->stringArray && AK_NULL == pListMenu->pStringGet) ? MENU_SELBAR_IMAGEPTR : pListMenu->hitIcon);	
			TxtScrool_ReSetString(pListMenu->pTxtScroll, (T_U32)string, (T_BOOL)(AK_NULL==pListMenu->stringArray && AK_NULL == pListMenu->pStringGet),
				(T_U8)(pListMenu->left+2), pListMenu->code, fcsVal, fcsJ, (T_U16)(pListMenu->width -2-fcsVal), CLR_WHITE, &pic);
			#endif
		}

	}

	pListMenu->refresh = CTRL_REFRESH_NONE;
	if(bChangeFreq)
	{
		Fwl_FreqPop();
	}
}

T_VOID	ListMenu_SetRefresh(CListMenuCtrl *pListMenu)
{
	//repaint all content
	if(pListMenu)
	{
		pListMenu->refresh = CTRL_REFRESH_ALL;	
		pListMenu->refreshEx = CTRL_REFRESH_MENULIST_ENTRYEX_ALL;
		pListMenu->bResetTxtScroll = AK_TRUE;
	}
}


static T_VOID ListMenu_MoveFocus(CListMenuCtrl *pListMenu, T_S8 delta)
{
	T_S8 lg;
	T_S8 abcd;
	T_S8 xyzw;
	T_S8 shift_matrix[4][4][2][2] =	//[xyzw][abcd][lg][]
		{
			//x
			{
				//a
				{
					-1, 0, +1, 0,
				},
				//b
				{
					-1, 0, +1, 0,
				},
				//c	
				{
					-1, 0, +1, 0,
				},
				//d	
				{
					-1, 0, +1, 0,
				},
			},
			//y
			{
				//a
				{
					-1, -1, +1, 0,
				},
				//b
				{
					-126, -126, +1, 0,
				},
				//c	
				{
					-1, -1, +1, 0,
				},
				//d	
				{
					-126, -126, +1, 0,
				},
			},
			//z
			{
				//a
				{
					-1, 0, +1, +1,
				},
				//b
				{
					-1, 0, +1, +1,
				},
				//c	
				{
					-1, 0, -127, -127,
				},
				//d	
				{
					-1, 0, -127, -127,
				},
			},
			//w
			{
				//a
				{
					-1, -1, +1, +1,
				},
				//b
				{
					-126, -126, +1, +1,
				},
				//c	
				{
					-1, -1, -127, -127,
				},
				//d	
				{
					-126, -126, -127, -127,
				},
			},
		};
	T_U16 curID;
	T_U16 begID;

	//focus move, adjust by above matrix, this is a technics.
	
	if(CTRL_MENULIST_DISP_ENTRYS == 1)							xyzw = 3;
	else if(pListMenu->curSubID == pListMenu->begSrID)			xyzw = 1;
	else if(pListMenu->curSubID == pListMenu->begSrID+CTRL_MENULIST_DISP_ENTRYS-1)		xyzw = 2;
	else if(pListMenu->curSubID == pListMenu->begSrID+pListMenu->stopID)				xyzw = 2;
	else														xyzw = 0;
	
	if(pListMenu->stopID+1 <= CTRL_MENULIST_DISP_ENTRYS)	
																abcd = 3;
	else
	{
		if(pListMenu->begSrID == 0)								abcd = 1;
		else if(pListMenu->begSrID+CTRL_MENULIST_DISP_ENTRYS-1 == pListMenu->stopID)	abcd = 2;
		else													abcd = 0;
	}

	if(delta < 0)												lg   = 0;
	else														lg   = 1;

	if(pListMenu->begSrID == pListMenu->stopID)
		return ;

	curID = pListMenu->curSubID;
	begID = pListMenu->begSrID;

	//如果焦点已经在底部，且要求继续往下移
	if(-127 == shift_matrix[xyzw][abcd][lg][0])
	{
		//jump to top
		//because T_S8 is small
		pListMenu->curSubID = pListMenu->begSrID = 0;
	}

	//如果焦点已经在顶部，且要求继续往上移
	else if(-126 == shift_matrix[xyzw][abcd][lg][0])
	{
		//jump to bottom
		pListMenu->curSubID = pListMenu->stopID;

		if(pListMenu->stopID+1 > CTRL_MENULIST_DISP_ENTRYS)
			pListMenu->begSrID = (T_U16)(pListMenu->curSubID-CTRL_MENULIST_DISP_ENTRYS+1);
		else
			pListMenu->begSrID = 0;
	}
	else
	{
		//directly get location from matrix
		pListMenu->curSubID = (T_U16)(pListMenu->curSubID + shift_matrix[xyzw][abcd][lg][0]);
		pListMenu->begSrID  = (T_U16)(pListMenu->begSrID + shift_matrix[xyzw][abcd][lg][1]);
	}


	if(curID!=pListMenu->curSubID || begID!=pListMenu->begSrID)
	{
		//如果需要更新显示内容
		pListMenu->refresh = CTRL_REFRESH_MENULIST_ENTRY;

		pListMenu->refreshEx = CTRL_REFRESH_MENULIST_ENTRYEX_ALL;//first set to repaint all entry
		if(begID == pListMenu->begSrID)
		{
			//如果仅仅是当前屏幕内条目内容的焦点切换, we only need to repaint two entry.
			pListMenu->refreshEx =  (T_U16)((curID-begID) << 8);
			pListMenu->refreshEx |= (pListMenu->curSubID-begID);
		}

		pListMenu->bResetTxtScroll = AK_TRUE;
	}
	
}

static T_U16 ListMenu_DealKey(CListMenuCtrl *pListMenu, T_EVT_PARAM *pParam)
{
    
    switch(pParam->c.Param1)
    {
        case CTRL_EVT_KEY_UP:
            ListMenu_MoveFocus(pListMenu, -1);//move focus up
            break;
        
        case CTRL_EVT_KEY_DOWN:
            ListMenu_MoveFocus(pListMenu, +1);//move focus down
            break;
    
        case CTRL_EVT_KEY_OK:
            if (pParam->c.Param2 == PRESS_SHORT)
            {
                if((pListMenu->curSubID==pListMenu->stopID) && (CTRL_LISTMENU_STYLE_QUIT_DISPSHOW & pListMenu->dwStyle))
                    return CTRL_RESPONSE_QUIT;//exit GUI if long push 
                else
                {
                    if(pListMenu->stringArray != AK_NULL || pListMenu->pStringGet != AK_NULL)
                        return pListMenu->curSubID;
                    else
                        return (T_U16)(RES_GET_PHY_LOC(pListMenu->startID, (ListMenu_GetItemId(pListMenu, pListMenu->curSubID))));//get selected entry ID
                }
            }
            else
                return CTRL_RESPONSE_QUIT;
    
        case CTRL_EVT_KEY_CANCEL:
        case CTRL_EVT_KEY_SELECT:
            if(CTRL_EVT_KEY_SELECT==pParam->c.Param1
                && pParam->c.Param2 == PRESS_LONG)
            {
                return CTRL_RESPONSE_NONE;  
            }
    
            return CTRL_RESPONSE_QUIT;  
    
        default:
            break;
        }

    return CTRL_RESPONSE_NONE;
}

T_U16  ListMenu_Handler(CListMenuCtrl *pListMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
//注:TopBar_Handle应放于最前面，否则下面返回了，这个功能就没执行过
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
		TopBar_Handle(&pListMenu->listmenu_topbar, Event, pParam);
#endif

    if(!pListMenu || !pParam || pParam->c.Param2 == PRESS_UP)
    {
        return CTRL_RESPONSE_NONE;
    }
    
    if(M_EVT_USER_KEY != Event && M_EVT_TOUCHSCREEN != Event)
    {
        return CTRL_RESPONSE_NONE;
    }

    {   
        return ListMenu_DealKey(pListMenu, pParam);
    }  
}

/**************************************************************************
* @brief extend for use define ListMenu
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/

T_BOOL	ListMenu_SetFocusEx(CListMenuCtrl *pListMenu, T_U16 offset)
{
    T_U16 i;
    AK_ASSERT_PTR(pListMenu, "ListMenu_SetFocusEx", AK_FALSE);
    if(AK_NULL != pListMenu->idLoc)
    {
        for(i=0; i<pListMenu->stopID; i++)
        {
            if(pListMenu->idLoc[i] == offset)
                break;
        }
        if(i > pListMenu->stopID)
            return AK_FALSE;
        offset = i;
    }

 
 	if(offset>pListMenu->stopID)
		return AK_FALSE;       

	//uploop to location focus entry
	while(pListMenu->curSubID > offset)
		ListMenu_MoveFocus(pListMenu, -1);

	//downloop to location focus entry
	while(pListMenu->curSubID < offset)
		ListMenu_MoveFocus(pListMenu, +1);

	pListMenu->refresh  = CTRL_REFRESH_ALL;
	pListMenu->refreshEx = CTRL_REFRESH_MENULIST_ENTRYEX_ALL;
	
	return AK_TRUE;
}
T_BOOL	ListMenu_SetFocus(CListMenuCtrl *pListMenu, T_U16 focusID)
{
	T_S16 offset;

	offset = (T_S16)(RES_GET_LOG_LEN(pListMenu->startID, focusID)-1);
    return ListMenu_SetFocusEx(pListMenu, offset);
    
}

T_U16	ListMenu_GetFocus(CListMenuCtrl *pListMenu)
{
	if(!pListMenu)
        return AK_FALSE;

    if(pListMenu->startID == 0)
        return pListMenu->curSubID;
    else
        return (T_U16)(RES_GET_PHY_LOC(pListMenu->startID, (ListMenu_GetItemId(pListMenu, pListMenu->curSubID))));
}

T_BOOL	ListMenu_SetEntryEx(CListMenuCtrl *pListMenu, T_U16 **string, T_U16 count, T_U16 cmmIcon, T_U16 hitIcon, T_U32 dwStyle, T_U16 codepage)
{
	if(!pListMenu || !count)
		return AK_FALSE;

	pListMenu->stringArray = string;
	pListMenu->startID = 0;
	pListMenu->stopID = (T_U16)(count-1);
	pListMenu->cmmIcon = cmmIcon;
	pListMenu->hitIcon = hitIcon;
	pListMenu->code    = codepage;

	pListMenu->dwStyle = dwStyle;
	if(CTRL_LISTMENU_STYLE_QUIT_DISPNONE == pListMenu->dwStyle)
		pListMenu->stopID -= 1;

	return AK_TRUE;
}

T_VOID	ListMenu_SetStyle(CListMenuCtrl *pListMenu, T_U32 dwStyle)
{
	T_U32 style;
	T_U8  flg1, flg2;

	if(!pListMenu)
		return;

	style = pListMenu->dwStyle;
	pListMenu->dwStyle = dwStyle;

	flg1 = AK_TRUE;
	if((CTRL_LISTMENU_STYLE_QUIT_DISPNONE&pListMenu->dwStyle)
		|| !(CTRL_LISTMENU_STYLE_QUIT_DISPSHOW&pListMenu->dwStyle))
		flg1 = AK_FALSE;

	flg2 = AK_TRUE;
	if((CTRL_LISTMENU_STYLE_QUIT_DISPNONE&style)
		|| !(CTRL_LISTMENU_STYLE_QUIT_DISPSHOW&style))
		flg2 = AK_FALSE;

	//adjust entry total number
	if(!flg1 && flg2)
		pListMenu->stopID -= 1;

	if((pListMenu->dwStyle & CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT)
		&& !pListMenu->pTxtScroll)
	{
		pListMenu->pTxtScroll = (CTxtScrollCtrl*)Fwl_Malloc(sizeof(CTxtScrollCtrl));
		if(AK_NULL == pListMenu->pTxtScroll)
		{
			AK_DEBUG_OUTPUT("ListMenu_SetStyle:alloc pTxtScroll error\r\n");
			pListMenu->dwStyle &= !CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT;
		}

		if(AK_FALSE == TxtScroll_Init(pListMenu->pTxtScroll, 500))
		{
			AK_DEBUG_OUTPUT("ListMenu_SetStyle:init pTxtScroll error\r\n");
			pListMenu->dwStyle &= !CTRL_LISTMENU_STYLE_SCROLLTXT_SELECT;
		}
	}

}

T_VOID	ListMenu_SetNavigate(CListMenuCtrl *pListMenu, T_U16 menuID)
{
	if(!pListMenu)
		return;
	if(INVALID_STRINGRES_ID == menuID)
		return;

	#if (USE_COLOR_LCD)
	pListMenu->navigateID = menuID;
	{
		#ifndef USE_CONTROL_NOTITLE 
		pListMenu->top += CTRL_WND_LINEHIGH;
		pListMenu->height = CTRL_WND_HEIGHT-pListMenu->top;
		pListMenu->entryLines = (pListMenu->height-CTRL_MENULIST_BEAUTIFY_INTVL)/CTRL_WND_LINEHIGH;
		#endif
	}
	#endif

}

T_VOID	ListMenu_SetTitle(CListMenuCtrl *pListMenu, T_U16 bkimgID)
{
	if(!pListMenu)
		return;
	if(INVALID_IMAGERES_ID == bkimgID)
		return;

	#if (USE_COLOR_LCD)
	pListMenu->bkimgTitle = bkimgID;
	#endif
}

T_BOOL	ListMenu_DeleteItem(CListMenuCtrl *pListMenu, T_U16 menuItem)
{
	T_U16 i, j;

	if(!pListMenu)
		return AK_FALSE;

	if(pListMenu->stringArray)
		return AK_FALSE;
	if(AK_NULL != pListMenu->pStringGet)
		return AK_FALSE;

	if(pListMenu->stopID == 0)
		return AK_FALSE;

	j = (T_U16)(RES_GET_LOG_LEN(pListMenu->startID, menuItem)-1);

	if(!pListMenu->idLoc)
	{
		if(AK_NULL == (pListMenu->idLoc = (T_U16*)Fwl_Malloc((pListMenu->stopID+1)*sizeof(T_U16))))
			return AK_FALSE;

		for(i=0; i<=pListMenu->stopID; i++)
			pListMenu->idLoc[i] = i;
	}

	for(i=0; i<=pListMenu->stopID; i++)
	{
		if(pListMenu->idLoc[i] == j)
			break;
	}

	if(i > pListMenu->stopID)
		return AK_FALSE;

	for(; i<=pListMenu->stopID-1; i++)
		pListMenu->idLoc[i] = pListMenu->idLoc[i+1];

	pListMenu->stopID --;
	pListMenu->begSrID	= 0;
	pListMenu->curSubID = 0;

	ListMenu_SetRefresh(pListMenu);
	return AK_TRUE;
}

T_VOID LitMenu_SetStringGet(CListMenuCtrl *pListMenu, TP_MListStringGet pStringGet)
{
	pListMenu->pStringGet= pStringGet;
}

#endif
