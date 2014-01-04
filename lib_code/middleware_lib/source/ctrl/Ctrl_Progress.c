#include "Ctrl_Progress.h"
#include "Eng_String_UC.h"
#include "Ctrl_Button.h"
#if (USE_COLOR_LCD)
#include "log_aud_play.h"
#endif

#if(NO_DISPLAY == 0)

//控件刷新子标记
#define CTRL_REFRESH_PROGRESS_TITLE		0x1	//only repaint title
#define CTRL_REFRESH_PROGRESS_ENTRY		0x2 //only repaint entry
#define CTRL_REFRESH_PROGRESS_LIMIT		0x4 //only repaint min and max value
#define CTRL_REFRESH_PROGRESS_TPOS		0x8 //only repaint cur value


T_BOOL	Progress_InitEx(CProgressCtrl *pProgress, T_S16 minValue, T_S16 maxValue, T_U16 titleStringID, T_RES_IMAGE iconPtr, T_U16 nIntvl, T_FLOAT curValue, T_BOOL bFloat, T_RES_IMAGE riconPtr, T_RES_IMAGE bkImg, T_POS left, T_POS top, T_LEN width, T_LEN height)
{
	if(!pProgress)
		return AK_FALSE;

	if((maxValue <= minValue) || (0==nIntvl))
		return AK_FALSE;

	if((curValue>maxValue) || (curValue<minValue))
		return AK_FALSE;

	pProgress->minValue		= minValue;
	pProgress->maxValue		= maxValue;
	pProgress->curValue		= curValue;
	pProgress->stepIntvl	= (T_FLOAT)((maxValue-minValue)/(nIntvl*1.0));
	#if (!USE_COLOR_LCD)
	pProgress->iconPtr		= iconPtr;
	pProgress->riconPtr		= riconPtr;
	#else
	pProgress->iconPtr		= INVALID_IMAGERES_ID;
	pProgress->riconPtr		= INVALID_IMAGERES_ID;
	#endif
	pProgress->bkImage		= bkImg;
	pProgress->titleStringID = titleStringID;
	pProgress->bFloat		= bFloat;
	pProgress->xBefore		= (T_POS)-1;
	pProgress->refresh		= CTRL_REFRESH_ALL;

	pProgress->firstData	= (T_U32)-1;
	pProgress->secondData	= (T_U32)-1;
	pProgress->firstIcon	= INVALID_IMAGERES_ID;
	pProgress->secondIcon	= INVALID_IMAGERES_ID;
	pProgress->bScroll		= AK_TRUE;

	pProgress->navigateID	= INVALID_STRINGRES_ID;
	pProgress->bkimgTitle	= INVALID_IMAGERES_ID;

	pProgress->left = left;
	pProgress->top  = top;
	pProgress->width = width;
	pProgress->height = height;

	#ifndef USE_CONTROL_NOTITLE 
	{
		pProgress->left   = CTRL_WND_LEFT;
		pProgress->top    = CTRL_WND_TOP+CTRL_WND_TITLEHTH;
		pProgress->width  = CTRL_WND_WIDTH;
		pProgress->height = CTRL_WND_HEIGHT-pProgress->top;
	}
	#endif

	#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
	pProgress->height = MAIN_LCD_HEIGHT - TITLE_HEIGHT;
	pProgress->top = TITLE_HEIGHT;
	#endif
	//keypad_start_intervaltimer(AK_TRUE);

	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	return TopBar_init(&pProgress->pro_topbar, titleStringID, AK_TRUE, AK_TRUE);
	#endif
	return AK_TRUE;
}

T_VOID	Progress_Free(CProgressCtrl *pProgress)
{
	if(!pProgress)
		return;

	//keypad_start_intervaltimer(AK_FALSE);
		
}

#ifndef USE_CONTROL_NOTITLE 
static T_VOID Progress_Show_InternalTitle(CProgressCtrl *pProgress)
{
	if(CTRL_REFRESH_ALL == pProgress->refresh)
	{
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, CTRL_WND_BACKCOLOR);
		#else
		Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP, TITLE_BKG_IMAGEPTR, AK_FALSE);
		#endif
	}

	if(CTRL_REFRESH_PROGRESS_TITLE & pProgress->refresh)
	{
		Display_Title((T_POS)(CTRL_WND_LEFT),  (T_POS)(CTRL_WND_TOP), (T_LEN)(CTRL_WND_WIDTH), (T_LEN)(CTRL_WND_TITLEHTH), (T_U16)pProgress->titleStringID, (T_U16)pProgress->bkimgTitle, (T_U16)pProgress->iconPtr, (T_U16)pProgress->riconPtr);

		if(INVALID_STRINGRES_ID != pProgress->navigateID)
			Display_NavgateBar(CTRL_WND_LEFT, CTRL_WND_TOP+CTRL_WND_TITLEHTH, CTRL_WND_WIDTH, CTRL_WND_LINEHIGH, pProgress->navigateID);
	}
}
#endif

T_VOID	Progress_Show(CProgressCtrl *pProgress)
{
	T_POS  x, y1;
	T_U16  strNum[16];
	T_S32  tmp;
	T_U8   lines;
	#if (USE_COLOR_LCD)
	T_BG_PIC pic;
	#endif

	if(!pProgress)
		return;

	#ifndef USE_CONTROL_NOTITLE 
	Progress_Show_InternalTitle(pProgress);
	#endif

////////////////lsk test
//	Eng_ImageResDisp(100, 50, eRES_IMAGE_0TEST, IMG_TRANSPARENT);
/////////////////IMG_TRANSPARENT

	if(CTRL_REFRESH_ALL == pProgress->refresh)
	{
		//clear and repaint LCD with color or background image		
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(pProgress->left, pProgress->top, pProgress->width, pProgress->height, CTRL_WND_BACKCOLOR);
		if(INVALID_IMAGERES_ID != pProgress->bkImage)
			Eng_ImageResDisp(pProgress->left, pProgress->top, pProgress->bkImage, AK_FALSE);
		
		#else
		#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
		Eng_ImageResDispEx(pProgress->left, pProgress->top, MSGB0X_BK_IMAGEPTR, pProgress->width, pProgress->height, 0, pProgress->top, AK_FALSE);
//		TopBar_show(&pProgress->pro_topbar);
		TopBar_SetReflesh(&pProgress->pro_topbar, TOPBAR_REFLESH_ALL);
		#else
		Eng_ImageResDispEx(pProgress->left, pProgress->top, MSGB0X_BK_IMAGEPTR, pProgress->width, pProgress->height, 0, 0, AK_FALSE);
		#endif
		#endif
	}

	#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
	TopBar_show(&pProgress->pro_topbar);
	#endif
	//Make POS string eg. <60>
	strNum[0] = '<';
	strNum[1] = '<';
	if(pProgress->bFloat)
		Utl_UFtoa(pProgress->curValue, strNum+2, 1);
	else
		Utl_UItoa((T_S32)pProgress->curValue, strNum+2, 10);
	tmp = Utl_UStrLen(strNum);
	strNum[tmp]   = '>';
	strNum[tmp+1] = '>';
	strNum[tmp+2] = 0;

	lines = (T_U8)(T_U16)(pProgress->height/CTRL_WND_LINEHIGH);

	//only repaint current value
	if(CTRL_REFRESH_PROGRESS_TPOS & pProgress->refresh)
	{
		if(lines <= 2)
		{
			//当屏幕小于等于2行时，我们将该内容直接画到title区域
			x = pProgress->left;
			if(INVALID_IMAGERES_ID != pProgress->iconPtr)
				x += CTRL_WND_ICONSZ;

			#if (!USE_COLOR_LCD)
			Fwl_FillRect((T_POS)(x+CTRL_WND_FNTSZ), pProgress->top, pProgress->width, CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
            DispStringInWidth(CP_UNICODE, (T_POS)(x+CTRL_WND_FNTSZ), pProgress->top, CONVERT2STR(strNum), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR);
			#else
			pic.hOffset = 0;
			pic.vOffset = (T_POS)0;
			pic.resId   = TITLE_BKG_IMAGEPTR;
			DispStringOnPic(CP_UNICODE, (T_POS)(x+CTRL_WND_FNTSZ), pProgress->top, CONVERT2STR(strNum), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR, &pic);
			#endif		
		}
	}
	
	//content
	if(CTRL_REFRESH_PROGRESS_ENTRY & pProgress->refresh)
	{
		T_POS x_slider;
		T_U16  j;
		T_U16  max_cnt = PROGRESS_HOR_WIDTH-PROGRESS_SLIDER_WTH;
		T_POS  i;

		//居中调节显示内容，使得美观
		//cord: Y
		y1 = (T_U16)(lines/2);
		if(0 == y1) y1 ++;
		y1 --;
		y1 = (T_POS)(pProgress->top+(y1*CTRL_WND_LINEHIGH));

		if(lines > 2)
			y1 += CTRL_WND_LINEHIGH;
		
		#if (!USE_COLOR_LCD)
		y1 += PROGRESS_VER_HEIGHT;
		#endif

		//cord: X
		j = (T_U16)(((pProgress->curValue-pProgress->minValue)*max_cnt)/((pProgress->maxValue-pProgress->minValue)*1.0));
		x_slider = (T_POS)(PROGRESS_EDGE_LEFT+j);

		if(lines > 2)
		{
			#if (USE_COLOR_LCD)
			T_U8 l, m;
			#endif
			y1 -= CTRL_WND_LINEHIGH;

			//居中显示当前进度数			
			#if (!USE_COLOR_LCD)
			j = GetStringDispWidth(CP_UNICODE, CONVERT2STR(strNum), 0);
			x = (j>=PROGRESS_HOR_WIDTH) ? 0: ((PROGRESS_HOR_WIDTH-j)>>1);
			x += PROGRESS_EDGE_LEFT;
			Fwl_FillRect((T_POS)(PROGRESS_EDGE_LEFT), y1, (T_LEN)(PROGRESS_HOR_WIDTH), CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
			DispStringInWidth(CP_UNICODE, (T_POS)(x), (T_POS)y1, CONVERT2STR(strNum), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR);
			#else
			#if ((LCD_TYPE != 3)||(1 != LCD_HORIZONTAL))
			Eng_ImageResDispEx((T_POS)(PROGRESS_EDGE_LEFT), y1, MSGB0X_BK_IMAGEPTR, (T_LEN)(PROGRESS_HOR_WIDTH), CTRL_WND_LINEHIGH, (T_POS)PROGRESS_EDGE_LEFT, (T_U8)((T_POS)(y1-CTRL_WND_TITLEHTH)), AK_FALSE);
			#else
			Eng_ImageResDispEx((T_POS)(PROGRESS_EDGE_LEFT), y1, MSGB0X_BK_IMAGEPTR, (T_LEN)(PROGRESS_HOR_WIDTH), CTRL_WND_LINEHIGH*2, (T_POS)PROGRESS_EDGE_LEFT, (T_U8)((T_POS)(y1)), AK_FALSE);
			#endif
			//显示模式：一行数据还是两行数据
			m = 1;
			if((pProgress->firstData != (T_U32)-1)
				&& (pProgress->secondData != (T_U32)-1))
			{
				m = 2;
			#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
				m = 1;
				Utl_UItoa((T_S32)pProgress->firstData, strNum+2, 10);
				tmp = Utl_UStrLen(strNum);
				strNum[tmp]   = 'M';
				strNum[tmp+1]   = '(';
				Utl_UItoa((T_S32)pProgress->secondData, (strNum+tmp+2), 10);
				tmp = Utl_UStrLen(strNum);
				strNum[tmp]   = 'B';
				strNum[tmp+1]   = ')';
				strNum[tmp+2]	= '>';
				strNum[tmp+3]	= '>';
				strNum[tmp+4]   = '\0';
			#endif
			}
			//下面的代码主要是用图片画出一个数字的内容
			//eg. 123 = '1' + '2' + '3'
			for(l=0; l<m; l++)
			{
				if(1 == m)
				{
					//jump  '<' and '>'
					tmp = Utl_UStrLen(strNum+2);
					strNum[2+tmp-2] = 0;
					j = 2; tmp -= 2;
					//if(strNum[j] == '-')
					//{
						//j ++;
						//tmp --;
					//}
				}
				else
				{
					if(0==l)	Utl_UItoa((T_S32)pProgress->firstData, strNum+2, 10);
					else		Utl_UItoa((T_S32)pProgress->secondData, strNum+2, 10);

					tmp = Utl_UStrLen(strNum+2);
					j = 2; 
				}
		//		#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
				x = (T_POS)((PROGRESS_HOR_WIDTH-tmp*DIGITAL_BIG12_WTH)/2);
				x = (T_POS)(x + PROGRESS_EDGE_LEFT);
				i = 0;
			//	#else
				
		//		#endif
				
				while(strNum[j])
				{
					switch(strNum[j])
					{
					case '0':
						tmp = DIGITAL_BIG12_0;
						break;
					case '1':
						tmp = DIGITAL_BIG12_1;
						break;		
					case '2':
						tmp = DIGITAL_BIG12_2;
						break;
					case '3':
						tmp = DIGITAL_BIG12_3;
						break;
					case '4':
						tmp = DIGITAL_BIG12_4;
						break;
					case '5':
						tmp = DIGITAL_BIG12_5;
						break;		
					case '6':
						tmp = DIGITAL_BIG12_6;
						break;
					case '7':
						tmp = DIGITAL_BIG12_7;
						break;	
					case '8':
						tmp = DIGITAL_BIG12_8;
						break;
					case '9':
						tmp = DIGITAL_BIG12_9;
						break;
					case '-':
						tmp = DIGITAL_BIGNEG;
						break;
					#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
					case '(':
						i++;
						tmp = eRES_IMAGE_LEFT_PARENTHESIS;
						break;
					case ')':
						i++;
						tmp = eRES_IMAGE_RIGHT_PARENTHESIS;
						break;
					case 'M':
						tmp = eRES_IMAGE_MEMORYM;
						break;
					case 'B':
						tmp = eRES_IMAGE_SYSTEMB;
						break;
					#endif
					}

					//draw digital image
					#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
					Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, tmp, AK_FALSE);	
					#else
					Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, tmp, IMG_TRANSPARENT);
					#endif
					i ++;
					j ++;
				}

				if(2 == m)
				{
					//两行数据模式还要在数字内容尾部添加图标
					if(0==l)
					{
						if(pProgress->firstIcon != INVALID_IMAGERES_ID)
						{
							#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
							Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, pProgress->firstIcon, AK_FALSE);
							#else
							Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, pProgress->firstIcon, IMG_TRANSPARENT);
							#endif
						}
					}
					else	
					{
						if(pProgress->secondIcon != INVALID_IMAGERES_ID)
						{
							#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
							Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, pProgress->secondIcon, AK_FALSE);
							#else
							Eng_ImageResDisp((T_POS)(x+i*DIGITAL_BIG12_WTH), y1, pProgress->secondIcon, IMG_TRANSPARENT);
							#endif
						}
					}
					#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
					if(0==l) y1 += CTRL_WND_LINEHIGH;
					#else
					if(0==l) y1 += CTRL_WND_LINEHIGH*2;
					#endif
				}
			}
			//pic.hOffset = x;
			//pic.vOffset = (T_POS)y1-startY;
			//pic.resId   = MSGB0X_BK_IMAGEPTR;
			//DispStringOnPic(CP_UNICODE, (T_POS)(x), (T_POS)y1, CONVERT2STR(strNum), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR, &pic);
			#endif

/////////////////////to be change 
			#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
        	y1 += CTRL_WND_LINEHIGH;
			#else
			y1 += CTRL_WND_LINEHIGH*2;
			#endif
			
		}

		//画进度条滑块右侧区域横长条
		#if (USE_COLOR_LCD)

		Eng_ImageResDisp((T_POS)PROGRESS_EDGE_LEFT, y1, PROGRESS_HOR_IMGPTR, AK_FALSE);
		#else
		j = (T_U8)-1;
		i = (T_U8)-1;

		if((T_POS)-1==pProgress->xBefore)
			Eng_ImageResDisp((T_POS)PROGRESS_EDGE_LEFT, y1, PROGRESS_HOR_IMGPTR, AK_FALSE);
		else if(x_slider>pProgress->xBefore)
		{
			j = pProgress->xBefore;
			i = x_slider;
		}
		else
		{
			j = x_slider;
			i = pProgress->xBefore+PROGRESS_SLIDER_WTH;
		}

		if((T_U8)-1 != j)
		{
			for(; j<=i; j+=PROGRESS_HOR_FRMWTH)
			{
				//fill rectangle
				if(j==PROGRESS_EDGE_LEFT)
					Eng_ImageResDisp((T_POS)(j+1), y1, PROGRESS_FRM_IMGPTR, AK_FALSE);
				else if(j+PROGRESS_HOR_FRMWTH >= PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH)
					Eng_ImageResDisp((T_POS)(PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH-1-PROGRESS_HOR_FRMWTH), y1, PROGRESS_FRM_IMGPTR, AK_FALSE);
				else if(j+PROGRESS_HOR_FRMWTH >= i)
					Eng_ImageResDisp((T_POS)(j), y1, PROGRESS_FRM_IMGPTR, AK_FALSE);
				else
					Eng_ImageResDisp((T_POS)j, y1, PROGRESS_FRM_IMGPTR, AK_FALSE);
			}
		}
		#endif

		#if (!USE_COLOR_LCD)
		//画进度条滑块左部区域
		if((T_POS)-1==pProgress->xBefore)
		{
			for(j=PROGRESS_EDGE_LEFT; j<=x_slider; j+=PROGRESS_HOR_GRDWTH)
				Eng_ImageResDisp((T_POS)j, y1, PROGRESS_SBK_IMGPTR, AK_FALSE);
		}
		else if(x_slider>pProgress->xBefore)
		{
			for(j=pProgress->xBefore-1; j<=x_slider; j+=PROGRESS_HOR_GRDWTH)
			{
				if(j+PROGRESS_HOR_GRDWTH >= PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH)
					Eng_ImageResDisp((T_POS)(PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH-PROGRESS_HOR_GRDWTH), y1, PROGRESS_SBK_IMGPTR, AK_FALSE);
				else
					Eng_ImageResDisp((T_POS)j, y1, PROGRESS_SBK_IMGPTR, AK_FALSE);
			}
		}
		else
		{
			//donothing
		}
		#endif
		
		//画进度条滑块
		#if (USE_COLOR_LCD)
		Eng_ImageResDisp(x_slider, y1, PROGRESS_SLIDER_PTR, AK_FALSE);	
		#else
		if(x_slider+PROGRESS_SLIDER_WTH > PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH)
			Eng_ImageResDisp((T_POS)(PROGRESS_EDGE_LEFT+PROGRESS_HOR_WIDTH-PROGRESS_SLIDER_WTH-1), y1, PROGRESS_SLIDER_PTR, AK_FALSE);
		else
			Eng_ImageResDisp(x_slider, y1, PROGRESS_SLIDER_PTR, AK_FALSE);	
		#endif


		pProgress->xBefore = x_slider;
		
	}

	pProgress->refresh = CTRL_REFRESH_NONE;
}

T_VOID	Progress_SetRefresh(CProgressCtrl *pProgress)
{
	//repaint progress all content
	if(!pProgress)
		return;
	pProgress->xBefore  = (T_POS)-1;
	pProgress->refresh = CTRL_REFRESH_ALL;
}

static T_VOID Progress_MoveFocus(CProgressCtrl *pProgress, T_S8 delta)
{
	if(!pProgress->bScroll)
		return;

	//进度条滑块移动仅仅增加或减少一个步距
	if((pProgress->curValue==pProgress->minValue && -1==delta)
		|| (pProgress->curValue==pProgress->maxValue && 1==delta))
		return;

	if (pProgress->curValue==pProgress->maxValue)
	{
		pProgress->xBefore  = (T_POS)-1;
	}

	pProgress->curValue += (delta*pProgress->stepIntvl);

	if(pProgress->curValue < pProgress->minValue)
		pProgress->curValue = pProgress->minValue;

	else if(pProgress->curValue > pProgress->maxValue)
		pProgress->curValue = pProgress->maxValue;
	
	pProgress->refresh = CTRL_REFRESH_PROGRESS_ENTRY;

	if(CTRL_WND_LINES == 2)
		pProgress->refresh |= CTRL_REFRESH_PROGRESS_TPOS;
}

static T_U16 Progress_DealKey(CProgressCtrl *pProgress, T_EVT_PARAM *pParam)
{
    	switch(pParam->c.Param1)
	{
	case CTRL_EVT_KEY_LEFT:
	case CTRL_EVT_KEY_VOLSUB:
		Progress_MoveFocus(pProgress, -1);//decrease one step value
		return CTRL_PROGRESS_RESPONSE_CHANGED;
	
	case CTRL_EVT_KEY_RIGHT:
	case CTRL_EVT_KEY_VOLADD:
		Progress_MoveFocus(pProgress, +1);//increase one step value
		return CTRL_PROGRESS_RESPONSE_CHANGED;

	case CTRL_EVT_KEY_OK:
	case CTRL_EVT_KEY_SELECT:
		if(CTRL_EVT_KEY_SELECT==pParam->c.Param1
			&& pParam->c.Param2 == PRESS_LONG)
		{
			return CTRL_RESPONSE_NONE;	//exit if long push
		}

		return CTRL_PROGRESS_SWITCHFLOAT2INT(pProgress->curValue);//return current progress value

	case CTRL_EVT_KEY_CANCEL:
		return CTRL_RESPONSE_QUIT;	

	default:
		break;
	}

	return CTRL_RESPONSE_NONE;	
}

T_U16   Progress_Handler(CProgressCtrl *pProgress, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	TopBar_Handle(&pProgress->pro_topbar, Event, pParam);
	#endif
    if(!pProgress || !pParam)
    {
        return CTRL_RESPONSE_NONE;
    }
    
    if((M_EVT_USER_KEY !=Event) && (M_EVT_TOUCHSCREEN != Event))
    {
        return CTRL_RESPONSE_NONE;
    }
    {
        return Progress_DealKey(pProgress, pParam);	
    }

}

T_BOOL	Progress_SetPos(CProgressCtrl *pProgress, T_FLOAT curValue)
{
	if(!pProgress)
		return AK_FALSE;

	if((curValue < (T_FLOAT)pProgress->minValue)
		|| (curValue > (T_FLOAT)pProgress->maxValue))
		return AK_FALSE;

	pProgress->curValue = curValue;
	pProgress->xBefore  = (T_POS)-1;

	if(CTRL_REFRESH_ALL ==pProgress->refresh)
		return AK_TRUE;

	pProgress->refresh = CTRL_REFRESH_PROGRESS_ENTRY;
	if(CTRL_WND_LINES == 2)
		pProgress->refresh |= CTRL_REFRESH_PROGRESS_TPOS;

	return AK_TRUE;
}

T_FLOAT Progress_GetPos(CProgressCtrl *pProgress)
{
	if(!pProgress)
		return (T_FLOAT)0.0;

	return pProgress->curValue;
}

T_BOOL	Progress_SetContent(CProgressCtrl *pProgress, T_U32 firstData, T_U16 firstIcon, T_U32 secondData, T_U16 secondIcon, T_BOOL bScroll)
{
	if(!pProgress)
		return AK_FALSE;

	//该函数用于设置2行模式数据

	#if (USE_COLOR_LCD)
	pProgress->firstData = firstData;
	pProgress->firstIcon = firstIcon;
	pProgress->secondData = secondData;
	pProgress->secondIcon = secondIcon;
	pProgress->bScroll	  = bScroll;
	#endif

	return AK_FALSE;
}

T_VOID	Progress_SetNavigate(CProgressCtrl *pProgress, T_U16 menuID)
{
	if(!pProgress)
		return;
	if(INVALID_STRINGRES_ID == menuID)
		return;
	
	#if (USE_COLOR_LCD)
	pProgress->navigateID = menuID;
	{
		#ifndef USE_CONTROL_NOTITLE 
		pProgress->top += CTRL_WND_LINEHIGH;
		pProgress->height = CTRL_WND_HEIGHT-pProgress->top;
		#endif
	}
	#endif

}

T_VOID	Progress_SetTitle(CProgressCtrl *pProgress, T_U16 bkimgID)
{
	if(!pProgress)
		return;
	if(INVALID_IMAGERES_ID == bkimgID)
		return;

	#if (USE_COLOR_LCD)
	pProgress->bkimgTitle = bkimgID;
	#endif
}

#endif