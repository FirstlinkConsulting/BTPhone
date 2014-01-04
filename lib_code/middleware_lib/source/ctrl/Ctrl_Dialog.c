#include "Ctrl_Dialog.h"
#include "Ctrl_Button.h"
#include "Fwl_osMalloc.h"


#if(NO_DISPLAY == 0)

//控件刷新子标记
#define CTRL_REFRESH_DIALOG_TITLE		0x1		//only repaint title
#define CTRL_REFRESH_DIALOG_ENTRY		0x2		//only repaint entry
#define CTRL_REFRESH_DIALOG_BTN			0x4		//only repaint button
#define CTRL_REFRESH_DIALOG_HINT		0x8		//only repaint hint string

//按钮个数
#define CTRL_DIALOG_RESPONSE_MIN		0x0001
#define CTRL_DIALOG_RESPONSE_MAX		0x0002//extend to 0x8000


T_BOOL	Dialog_InitEx(CDialogCtrl *pDialog, T_U16 *hintString, T_U16 modeRsp, T_U16 titleStringID, T_RES_IMAGE iconPtr, T_U16 modeDefault, T_POS left, T_POS top, T_LEN width, T_LEN height)
{
	T_U16 i;
	T_S16 len;

	if(!pDialog)
		return AK_FALSE;

	//now only support 'YES' and 'NO' button
	//hidden button is really a 'YES' button which is not display
	if(CTRL_DIALOG_RESPONSE_YES!=modeDefault && CTRL_DIALOG_RESPONSE_NO!=modeDefault)
		return AK_FALSE;

	pDialog->modeRsp = modeRsp;
	#if (!USE_COLOR_LCD)
	pDialog->iconPtr = iconPtr;
	#else
	pDialog->iconPtr = INVALID_IMAGERES_ID;
	#endif
	pDialog->titleStringID = titleStringID;
	pDialog->hintString = AK_NULL;
	pDialog->hintlines = 0;
	if(hintString)
	{
		//compute the lines of hintstring
		len = Utl_UStrLen(hintString);
		pDialog->hintString = Fwl_Malloc((len+1)* sizeof(T_U16));

		if(AK_NULL != pDialog->hintString)
		{
			Utl_UStrCpy(pDialog->hintString, hintString);
			
			i = 0;
			while(len-i > 0)
			{
				i = (T_U16)(i+((T_U16)GetStringDispWidth(CP_UNICODE, (T_U8*)(pDialog->hintString+i), (T_LEN)(CTRL_WND_WIDTH-CTRL_WND_FNTSZ)))/2);

				if(pDialog->hintString[i] == '\n')
					i += 1;
				pDialog->hintlines ++;
			}
		}
		else
		{
			return AK_FALSE;
		}
	}
	pDialog->refresh  = CTRL_REFRESH_ALL;
	pDialog->curSelect = modeDefault;

	pDialog->cntRsp = 0;
	if(CTRL_DIALOG_RESPONSE_HIDDEN != modeRsp)// pDialog->cntRsp)
	{	
		//compute the number of button
		while(modeRsp)
		{
			modeRsp &= modeRsp-1;
			pDialog->cntRsp ++;
		}
		if(0 == pDialog->cntRsp)
			return AK_FALSE;
	}

	pDialog->fixLines = 0;//default dialog is FULL LCD
	#if (USE_COLOR_LCD)
	pDialog->bkImage  = MSGB0X_BK_IMAGEPTR;
	#endif

	pDialog->fontColor = CTRL_WND_FONTCOLOR;
	pDialog->navigateID = INVALID_STRINGRES_ID;
	pDialog->bkimgTitle = INVALID_IMAGERES_ID;

	pDialog->left = left;
	pDialog->top  = top;
	pDialog->width = width;
	pDialog->height = height;
	
	#ifndef USE_CONTROL_NOTITLE 
	if(INVALID_STRINGRES_ID != pDialog->titleStringID)
	{
		pDialog->left   = CTRL_WND_LEFT;
		pDialog->top    = CTRL_WND_TOP+CTRL_WND_TITLEHTH;
		pDialog->width  = CTRL_WND_WIDTH;
		pDialog->height = CTRL_WND_HEIGHT-pDialog->top;
	}
	#endif
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	pDialog->top  = TITLE_HEIGHT;
	pDialog->height = MAIN_LCD_HEIGHT - TITLE_HEIGHT;
	return TopBar_init(&pDialog->dialog_topbar, pDialog->titleStringID, AK_TRUE ,AK_TRUE);
	#endif
	return AK_TRUE;
}

#ifndef USE_CONTROL_NOTITLE 
static T_VOID Dialog_Show_InternalTitle(CDialogCtrl *pDialog)
{
	if(CTRL_REFRESH_ALL == pDialog->refresh)
	{
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, CTRL_WND_BACKCOLOR);
		#else
		if(INVALID_STRINGRES_ID != pDialog->titleStringID)
			Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP, TITLE_BKG_IMAGEPTR, AK_FALSE);
		#endif
	}

	if(CTRL_REFRESH_DIALOG_TITLE & pDialog->refresh)
	{
		if(INVALID_STRINGRES_ID != pDialog->titleStringID)
			#if (!USE_COLOR_LCD)
			Display_Title(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, pDialog->titleStringID, pDialog->bkimgTitle, (T_U16)pDialog->iconPtr, 0xFFFC);
			#else
			Display_Title(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, pDialog->titleStringID, pDialog->bkimgTitle, pDialog->iconPtr, INVALID_IMAGERES_ID);
			#endif
			
		if(INVALID_STRINGRES_ID != pDialog->navigateID)
			Display_NavgateBar(CTRL_WND_LEFT, CTRL_WND_TOP+CTRL_WND_TITLEHTH, CTRL_WND_WIDTH, CTRL_WND_LINEHIGH, pDialog->navigateID);

	}

	if(CTRL_REFRESH_DIALOG_BTN & pDialog->refresh)
	{	
		if(CTRL_DIALOG_RESPONSE_HIDDEN != pDialog->modeRsp)
		{
			//repaint button
			#if (!USE_COLOR_LCD)
			//paint focused button
			if(CTRL_DIALOG_RESPONSE_YES == pDialog->curSelect)
				Eng_ImageResDisp((T_POS)(CTRL_WND_LEFT+CTRL_WND_WIDTH-CTRL_BTN_WIDTH-1), CTRL_WND_TOP, BUTTON_YES_IMAGPTR, AK_FALSE);
			else
				Eng_ImageResDisp((T_POS)(CTRL_WND_LEFT+CTRL_WND_WIDTH-CTRL_BTN_WIDTH-1), CTRL_WND_TOP, BUTTON_NO_IMAGEPTR, AK_FALSE);
			#endif		
		}
	}
}
#endif

T_VOID	Dialog_Show(CDialogCtrl *pDialog)
{
	T_POS y1, y2;
	T_U16 i, k;
	T_U16  lines;
	#if (USE_COLOR_LCD)
	T_U16  x;
	T_BG_PIC pic;
	#endif

	if(!pDialog)
		return;

	#ifndef USE_CONTROL_NOTITLE 
	Dialog_Show_InternalTitle(pDialog);
	#endif


	if(CTRL_REFRESH_ALL == pDialog->refresh)
	{
		//first paint dialog, clear all LCD
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(pDialog->left, pDialog->top, pDialog->width, pDialog->height, CTRL_WND_BACKCOLOR);
		#else
		
		#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
		Eng_ImageResDispEx(pDialog->left, pDialog->top, pDialog->bkImage, pDialog->width, pDialog->height, 0, pDialog->top, AK_FALSE);
//		TopBar_show(&pDialog->dialog_topbar);
		TopBar_SetReflesh(&pDialog->dialog_topbar, TOPBAR_REFLESH_ALL);
		#else
		Eng_ImageResDisp(pDialog->left, pDialog->top, pDialog->bkImage, AK_FALSE);
		#endif
		#endif
	}
	lines = (T_U16)(pDialog->height/CTRL_WND_LINEHIGH);

	//topbar_show :if reflesh_none ,it will operate none and return;
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	TopBar_show(&pDialog->dialog_topbar);
	#endif
	//middle aligned hint string in the FULL LCD lines height
	#if (USE_COLOR_LCD)
    if(CTRL_DIALOG_RESPONSE_HIDDEN != pDialog->modeRsp)
    {
        lines --;
#if (LCD_TYPE == 3)
        lines --;
#endif
    }
	#endif

	if(pDialog->hintlines > lines)
		pDialog->hintlines = lines;

	y1 = (T_POS)(((lines-pDialog->hintlines)*CTRL_WND_LINEHIGH)/2);
	//if((lines-pDialog->hintlines)%2 != 0)
		//y1 += CTRL_WND_LINEHIGH/2;

	y1 = (T_POS)(pDialog->top + y1);
	y2 = (T_POS)(y1+pDialog->hintlines*CTRL_WND_LINEHIGH);


	//content
	if((AK_NULL != pDialog->hintString) && (CTRL_REFRESH_DIALOG_ENTRY & pDialog->refresh))
	{

		//repaint content region
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(pDialog->left, (T_POS)y1, pDialog->width, (T_POS)(pDialog->hintlines*CTRL_WND_LINEHIGH), CTRL_WND_BACKCOLOR);
		#else
		#if	((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
		Eng_ImageResDispEx(pDialog->left, (T_POS)y1, pDialog->bkImage, pDialog->width, (T_POS)(pDialog->hintlines*CTRL_WND_LINEHIGH), 0, (T_POS)(y1-pDialog->top), AK_FALSE);
		#else
		if(CTRL_REFRESH_ALL != pDialog->refresh)
		{
		    Eng_ImageResDispEx(pDialog->left, (T_POS)(y1), pDialog->bkImage, pDialog->width, (T_POS)(pDialog->hintlines*CTRL_WND_LINEHIGH), 0, (T_POS)(y1), AK_FALSE);
        }
		#endif
		#endif
		

		if(pDialog->hintString)
		{
			i = 0;
			//draw each hint string content
			for(k=0; k<pDialog->hintlines; k++)
			{
				//draw text aligned each line in the LCD width
				T_U16 s, t;
				s = GetStringDispWidth(CP_UNICODE, CONVERT2STR(pDialog->hintString+i), 0);
				t = (T_U16)(pDialog->left+((s >= pDialog->width) ? 0: ((pDialog->width-s)>>1)));

				#if (!USE_COLOR_LCD)
				i = i+DispStringInWidth(CP_UNICODE, (T_POS)(t), (T_POS)(y1+k*CTRL_WND_FNTSZ), CONVERT2STR(pDialog->hintString+i), (T_LEN)(pDialog->width), CTRL_WND_FONTCOLOR)/2;
				#else
				pic.hOffset = (T_POS)t;//CTRL_WND_LEFT+CTRL_WND_FNTSZ/2;
				#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
				pic.vOffset = (T_POS)(y1+k*CTRL_WND_FNTSZ-pDialog->top);
				#else
				pic.vOffset = (T_POS)(y1+k*CTRL_WND_FNTSZ);
				#endif
				pic.resId   = (T_U16)(pDialog->bkImage);
				//i = (T_U16)(i+DispStringOnPic(CP_UNICODE, (T_POS)(t), (T_POS)(y1+k*CTRL_WND_FNTSZ), CONVERT2STR(pDialog->hintString+i), (T_LEN)pDialog->width, pDialog->fontColor, &pic)/2);             
                i = (T_U16)(i+DispStringInWidth(CP_UNICODE, (T_POS)(t), (T_POS)(y1+k*CTRL_WND_FNTSZ), CONVERT2STR(pDialog->hintString+i), (T_LEN)pDialog->width, pDialog->fontColor)/2);                                       
				#endif	
				
				if(pDialog->hintString[i] == '\n')
					i += 1;
			}

		}
	}
	
	//button
	if(CTRL_REFRESH_DIALOG_BTN & pDialog->refresh)
	{	
		if(CTRL_DIALOG_RESPONSE_HIDDEN != pDialog->modeRsp)
		{
			//repaint button
			#if (USE_COLOR_LCD)
			x = (T_U16)(pDialog->left+(pDialog->width-CTRL_BTN_WIDTH)/2);
			//repaint background image in these region
			#if	((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
			Eng_ImageResDispEx(pDialog->left, (T_POS)y2, pDialog->bkImage, pDialog->width, CTRL_WND_LINEHIGH, 0, (T_POS)(y2-pDialog->top), AK_FALSE);
			#endif
//			Eng_ImageResDispEx(pDialog->left, (T_POS)y2, pDialog->bkImage, pDialog->width, CTRL_WND_LINEHIGH, 0, (T_POS)(y2), AK_FALSE);

			//paint focused button
			if(CTRL_DIALOG_RESPONSE_YES == pDialog->curSelect)
			{
				#if(LCD_TYPE == 3)
					#if (1 == LCD_HORIZONTAL)
					Eng_ImageResDisp((T_POS)(x), (T_POS)(y2), BUTTON_YES_IMAGPTR, AK_FALSE);
					#else
					Eng_ImageResDisp((T_POS)(x-16), y2, BUTTON_YES_IMAGPTR, AK_FALSE);	
					#endif
				#else
					Eng_ImageResDisp(x, y2, BUTTON_YES_IMAGPTR, AK_FALSE);
				#endif				
			}
			else
			{
				#if(LCD_TYPE == 3)
					#if (1 == LCD_HORIZONTAL)
					Eng_ImageResDisp((T_POS)(x), (T_POS)(y2), BUTTON_NO_IMAGEPTR, AK_FALSE);
					#else
					Eng_ImageResDisp((T_POS)(x-16), y2, BUTTON_NO_IMAGEPTR, AK_FALSE);	
					#endif
			//		Eng_ImageResDisp((T_POS)(x-16), y2, BUTTON_NO_IMAGEPTR, AK_FALSE);	
				#else
					Eng_ImageResDisp(x, y2, BUTTON_NO_IMAGEPTR, AK_FALSE);
				#endif
			}
	    	#endif			

		}
	}

	pDialog->refresh = CTRL_REFRESH_NONE;
}

T_VOID	Dialog_SetRefresh(CDialogCtrl *pDialog)
{
	if(!pDialog)
		return;

	//set repaint all
	pDialog->refresh = CTRL_REFRESH_ALL;
}

static T_VOID Dialog_MoveFocus(CDialogCtrl *pDialog, T_S8 delta)
{
	if(pDialog->cntRsp <= 1)
		return;

	//move selected button
	if(CTRL_DIALOG_RESPONSE_YES == pDialog->curSelect)
		pDialog->curSelect = CTRL_DIALOG_RESPONSE_NO;
	else
		pDialog->curSelect = CTRL_DIALOG_RESPONSE_YES;

/*
	if((CTRL_DIALOG_RESPONSE_MIN==pDialog->curSelect && +1==delta)
		|| (CTRL_DIALOG_RESPONSE_MAX==pDialog->curSelect && -1==delta))
		return;

	if(delta < 0)
	{
		do
		{
			pDialog->curSelect <<= 1;
		}while(!(pDialog->curSelect & pDialog->modeRsp));
	}
	else
	{
		do
		{
			pDialog->curSelect >>= 1;
		}while(!(pDialog->curSelect & pDialog->modeRsp));
	}
*/
	pDialog->refresh = CTRL_REFRESH_DIALOG_BTN;
}


//Dialog控件对按键信息的处理
static T_U16 Dialog_DealKey(CDialogCtrl *pDialog, T_EVT_PARAM *pParam)
{
    	switch(pParam->c.Param1)
	{
	case CTRL_EVT_KEY_LEFT:
		Dialog_MoveFocus(pDialog, -1);//left move focus
		break;
	
	case CTRL_EVT_KEY_RIGHT:
		Dialog_MoveFocus(pDialog, +1);//right move focus
		break;

	case CTRL_EVT_KEY_OK:
		if(CTRL_DIALOG_RESPONSE_HIDDEN == pDialog->modeRsp)
			return CTRL_RESPONSE_QUIT;//no button , no response, just exit after hint
		return pDialog->curSelect;	//return selected button flag

	case CTRL_EVT_KEY_CANCEL:
	case CTRL_EVT_KEY_SELECT:
		if(CTRL_EVT_KEY_SELECT==pParam->c.Param1
			&& pParam->c.Param2 == PRESS_LONG)
		{
			return CTRL_RESPONSE_QUIT;	//just exit, and do nothing else
		}		
		return CTRL_RESPONSE_QUIT;	

	default:
		break;
	}

	return CTRL_RESPONSE_NONE;	
}

T_U16   Dialog_Handler(CDialogCtrl *pDialog, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
	#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	TopBar_Handle(&pDialog->dialog_topbar ,Event ,pParam);
	#endif
    if(!pDialog || !pParam)
    {
        return CTRL_RESPONSE_NONE;
    }
    
    if((M_EVT_USER_KEY !=Event) && (M_EVT_TOUCHSCREEN != Event))
    {
        return CTRL_RESPONSE_NONE;
    }

    return  Dialog_DealKey(pDialog,  pParam);	
}

T_VOID	Dialog_Free(CDialogCtrl *pDialog)
{
	if(!pDialog)
		return;

	if(pDialog->hintString)
		pDialog->hintString = Fwl_Free(pDialog->hintString);
}

T_BOOL	Dialog_FixedLines(CDialogCtrl *pDialog, T_U8 lines)
{
	if(!pDialog)
		return AK_FALSE;

	return AK_TRUE;
/*
	if(lines > CTRL_WND_LINES)
		return AK_FALSE;

	if(lines < 1+1+pDialog->hintlines)
		return AK_FALSE;

	pDialog->fixLines = lines;
	return AK_TRUE;*/
}

T_VOID	Dialog_SetBkImage(CDialogCtrl *pDialog, T_U16 imageID)
{
	if(!pDialog)
		return;

	#if (USE_COLOR_LCD)
	pDialog->bkImage = imageID;
	#endif
}

T_VOID	Dialog_SetFontColor(CDialogCtrl *pDialog, T_U16 color)
{
	if(!pDialog)
		return;

	#if (USE_COLOR_LCD)
	pDialog->fontColor = color;
	#endif
}

T_VOID	Dialog_SetNavigate(CDialogCtrl *pDialog, T_U16 menuID)
{
	if(!pDialog)
		return;
	if(INVALID_STRINGRES_ID == menuID)
		return;
	
	#if (USE_COLOR_LCD)
	pDialog->navigateID = menuID;
	{
		#ifndef USE_CONTROL_NOTITLE 
		pDialog->top += CTRL_WND_LINEHIGH;
		pDialog->height = CTRL_WND_HEIGHT-pDialog->top;
		#endif
	}
	#endif

}

T_VOID	Dialog_SetTitle(CDialogCtrl *pDialog, T_U16 bkimgID)
{
	if(!pDialog)
		return;
	if(INVALID_IMAGERES_ID == bkimgID)
		return;

	#if (USE_COLOR_LCD)
	pDialog->bkimgTitle = bkimgID;
	#endif
	
}

#endif
