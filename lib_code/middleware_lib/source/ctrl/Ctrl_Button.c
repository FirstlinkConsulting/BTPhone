#include "Gbl_Global.h"
#include "Ctrl_Button.h"  
#include "Log_aud_play.h"

#if(NO_DISPLAY == 0)

#define CTRL_BTN_LINEWIDTH	1

T_VOID	Display_Button(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U16 stringID, T_U8 bFocus)
{
	T_POS x1;
	T_U16 wordWth;
	#if (USE_COLOR_LCD)
	T_BG_PIC pic;
	#endif 

	x1 = x;

	//really button is a rectangle filled different color or background image
	#if (!USE_COLOR_LCD)
	Fwl_DrawRect(x, y, width, height, CLR_WHITE, bFocus);//black LCD is must be 8bit aligned
	#else
	if(bFocus)
		Eng_ImageResDispEx(x, y, BUTTON_FC_IMAGEPTR, width, height, 0, 0, AK_FALSE);
	else
		Eng_ImageResDispEx(x, y, BUTTON_BK_IMAGEPTR, width, height, 0, 0, AK_FALSE);
	#endif

	//draw text in rectangle
	wordWth = GetStringDispWidth(CP_UNICODE, CONVERT2STR(stringID), 0);
	x = (T_POS)(x+((wordWth >= width) ? 0: ((width-wordWth)>>1)));

	#if (USE_COLOR_LCD)
	pic.hOffset = (T_POS)(x-x1);
	pic.vOffset = 0;
	#endif

	if(bFocus)	
	{
		#if (!USE_COLOR_LCD)
		DispStringInWidth(CP_UNICODE, (T_POS)x, y, CONVERT2STR(stringID), CTRL_WND_STRINGWTH(wordWth), CTRL_WND_BACKCOLOR);
		#else
		pic.resId   = BUTTON_FC_IMAGEPTR;
		DispStringOnPic(CP_UNICODE, (T_POS)x, (T_POS)y, CONVERT2STR(stringID), CTRL_WND_STRINGWTH(wordWth), CLR_BLUE, &pic);
		#endif
	}
	else
	{
		#if (!USE_COLOR_LCD)
		DispStringInWidth(CP_UNICODE | FORMAT_UNDERLINE | FORMAT_TOPLINE, (T_POS)x, y, CONVERT2STR(stringID), CTRL_WND_STRINGWTH(wordWth), CTRL_WND_FONTCOLOR);
		#else
		pic.resId   = BUTTON_BK_IMAGEPTR;
		DispStringOnPic(CP_UNICODE, (T_POS)x, (T_POS)y, CONVERT2STR(stringID), CTRL_WND_STRINGWTH(wordWth), CLR_GREEN, &pic);
		#endif
	}
	
}

#pragma arm section code = "_audioplayer_menu_"
T_VOID	Display_Title(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID, T_U16 imgID, T_U16 icon_l, T_U16 icon_r)
{
	T_U8  i;
	T_U16 j;
	#if (USE_COLOR_LCD)
	#ifndef USE_CONTROL_NOTITLE
	T_BG_PIC pic;
	#endif
	T_U16    img = imgID;
	#endif 

	//really title is a rectangle filled color or background image

	//draw background
	#if (!USE_COLOR_LCD)
	Fwl_FillRect(x, y, width, height, CTRL_WND_BACKCOLOR);
	#else
	if(INVALID_IMAGERES_ID == img)
		img = TITLE_BKG_IMAGEPTR;
	Eng_ImageResDisp(0, (T_POS)CTRL_WND_TOP, img, AK_FALSE);
	#endif

	//draw icon
	if(INVALID_IMAGERES_ID != icon_l)
	{
		if(0xFFFC == icon_r)
			x -= CTRL_WND_ICONWTH/2;//special dialog need

		Eng_ImageResDisp((T_POS)(x+CTRL_WND_ICONWTH), y, icon_l, 0x02);//AK_FALSE);
		x += (CTRL_WND_ICONSZ+CTRL_WND_ICONWTH);
		width -= (CTRL_WND_ICONSZ+CTRL_WND_ICONWTH);
	}

	if(INVALID_IMAGERES_ID != icon_r)
	{
		if(0xFFFC != icon_r)//special dialog need
			Eng_ImageResDisp((T_POS)(x+width-CTRL_WND_ICONSZ-CTRL_WND_ICONWTH), y, icon_r, 0x02);//AK_FALSE);	
		width -= (CTRL_WND_ICONSZ+CTRL_WND_ICONWTH);
	}

	if(INVALID_STRINGRES_ID != stringID)
	{
		j = GetStringDispWidth(CP_UNICODE, CONVERT2STR(stringID), 0);
		i = (T_U8)(x+((j >= width) ? 0: ((width-j)>>1)));

		#if (!USE_COLOR_LCD)
		DispStringInWidth(CP_UNICODE, (T_POS)i, (T_POS)CTRL_WND_TOP, CONVERT2STR(stringID), CTRL_WND_STRINGWTH(j), CTRL_WND_FONTCOLOR);
		#else
		#ifndef USE_CONTROL_NOTITLE
		j = CTRL_WND_TOP;
		if(height > CTRL_WND_LINEHIGH)
			j += 8;
		pic.hOffset = (T_POS)i;
		pic.vOffset = (T_POS)j;
		pic.resId   = img;	
		DispStringOnPic(CP_UNICODE, (T_POS)i, (T_POS)j, CONVERT2STR(stringID), (T_U16)(CTRL_WND_WIDTH - i), CLR_WHITE, &pic);
		#endif
		#endif
	}
}

T_VOID	Display_NavgateBar(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID)
{
	//really title is a rectangle filled color or background image
	#if (USE_COLOR_LCD)
	T_BG_PIC pic;	
	pic.hOffset = 0;
	pic.vOffset = 0;
	pic.resId   = NAVGATE_BK_IMAGEPTR;
	Eng_ImageResDisp(x, y, NAVGATE_BK_IMAGEPTR, AK_FALSE);
	DispStringOnPic(CP_UNICODE, (T_POS)x+2, (T_POS)y+2, CONVERT2STR(stringID), width, CLR_WHITE, &pic);
	#endif
}



#pragma arm section code




/* 
 @	Scroll Text Rect Control
*/
#pragma arm section zidata = "_bootbss_"
volatile T_U32	gb_txtScrollCnt = 0;
#pragma arm section zidata
 
#pragma arm section code = "_audioplayer_menu_"
T_VOID	TxtScroll_Show(CTxtScrollCtrl *pTxtScroll)
{
	if((gb_txtScrollCnt & 0xFF) && (gb_txtScrollCnt & 0xFF00))
	{
		pTxtScroll->curTicks = (T_U8)((gb_txtScrollCnt >> 16) & 0xFF);

		#if (USE_COLOR_LCD)

		#if(LCD_TYPE == 3)
		
		Eng_ImageResDispEx((T_POS)pTxtScroll->s_val, pTxtScroll->y, pTxtScroll->Gg.resId,\
                       (T_U16)(Eng_GetResImageWidth(pTxtScroll->Gg.resId)- pTxtScroll->s_val),\
                       (T_U16)(Eng_GetResImageHeight(pTxtScroll->Gg.resId)),\
                       0,\
                       0,\
                       AK_FALSE);
		#else
		Eng_ImageResDisp((T_POS)pTxtScroll->s_val, pTxtScroll->y, pTxtScroll->Gg.resId, AK_FALSE);
		#endif
		DispStringOnPic(pTxtScroll->codepage, pTxtScroll->x, pTxtScroll->y, (T_U8*)(pTxtScroll->string+pTxtScroll->curTicks),
			pTxtScroll->width, pTxtScroll->color, &(pTxtScroll->Gg));
		#else
			Fwl_FillRect(pTxtScroll->x, pTxtScroll->y, pTxtScroll->width, CTRL_WND_LINEHIGH, CTRL_WND_FONTCOLOR);
			DispStringOnColor(pTxtScroll->codepage, pTxtScroll->x, pTxtScroll->y, (T_U8*)(pTxtScroll->string+pTxtScroll->curTicks), 
				pTxtScroll->width, pTxtScroll->s_val, pTxtScroll->color);		
		#endif

		gb_txtScrollCnt &= 0xFFFF00FF;
	}
}

#pragma arm section code

#pragma arm section code = "_bootcode1_"
T_VOID	TxtScroll_Hndler(T_TIMER timerId, T_U32 delay)
{
	if((gb_txtScrollCnt & 0xFF) && !(gb_txtScrollCnt & 0xFF00))
	{
		T_U8 curTicks = (T_U8)((gb_txtScrollCnt >> 16) & 0xFF);
		T_U8 sumTicks = (T_U8)(gb_txtScrollCnt >> 24);
		
		curTicks = (T_U8)((curTicks+1)%sumTicks);

		gb_txtScrollCnt &= 0xFF00FFFF;
		gb_txtScrollCnt |= (((T_U32)(curTicks & 0xFF)) << 16) | 0xFF00;
	}
}
#pragma arm section code



T_BOOL	TxtScroll_Init(CTxtScrollCtrl *pTxtScroll, T_U16 msDelay)
{
	if(!pTxtScroll)
		return AK_FALSE;

	memset(pTxtScroll, 0, sizeof(CTxtScrollCtrl));
	
	pTxtScroll->timer = Fwl_TimerStart(msDelay, AK_TRUE, TxtScroll_Hndler);
	if(ERROR_TIMER == pTxtScroll->timer)
	{
		AK_DEBUG_OUTPUT("TxtScroll_Init:start timer error\r\n");
		return AK_FALSE;
	}

	gb_txtScrollCnt = 0x0;

	return AK_TRUE;
}

T_VOID	TxtScroll_Free(CTxtScrollCtrl *pTxtScroll)
{	
	if(pTxtScroll)
	{
		Fwl_TimerStop(pTxtScroll->timer);
		pTxtScroll->timer = ERROR_TIMER;

		gb_txtScrollCnt = 0x0;
	}
}


T_VOID	TxtScrool_ReSetString(CTxtScrollCtrl *pTxtScroll, T_U32 string, T_BOOL bStrFromRes, 
							  T_U16 s_val, T_U32 codepage, T_U32 x, T_U32 y, T_U16 Width,T_U16 color, T_BG_PIC *pGg)
{
	if(pTxtScroll)
	{
		gb_txtScrollCnt = 0x0;

		if(GetStringDispWidth(CP_UNICODE, CONVERT2STR(string), 0) < Width-CTRL_WND_FNTSZ*2)
			return ;

		if(bStrFromRes)
		{
			pTxtScroll->sumTicks = GetResString(string, pTxtScroll->string, 127);
			pTxtScroll->string[pTxtScroll->sumTicks] = 0;
		}
		else
		{
			Utl_UStrCpyN(pTxtScroll->string, (T_U16*)string, 127);
			pTxtScroll->string[127] = 0;

			pTxtScroll->sumTicks = Utl_UStrLen(pTxtScroll->string);
		}

		pTxtScroll->sumTicks = 1;
		while(GetStringDispWidth(CP_UNICODE, (T_U8*)(pTxtScroll->string+pTxtScroll->sumTicks), 0) > Width-CTRL_WND_FNTSZ*2)
			pTxtScroll->sumTicks ++;


		pTxtScroll->curTicks = 0;

		pTxtScroll->codepage = codepage;
		pTxtScroll->x =(T_U16)x;
		pTxtScroll->y = (T_U16)y;
		pTxtScroll->width = Width;
		pTxtScroll->color = color;
		if(pGg) memcpy(&(pTxtScroll->Gg), pGg, sizeof(T_BG_PIC));

		pTxtScroll->s_val = s_val;

		gb_txtScrollCnt = pTxtScroll->sumTicks;
		gb_txtScrollCnt <<= 24;
		gb_txtScrollCnt |= ((T_U32)(pTxtScroll->curTicks & 0xFF)) << 16;
		gb_txtScrollCnt |= 0xFF;	
		gb_txtScrollCnt &= 0xFFFF00FF;
	}
}

T_VOID	TxtScroll_Pause(CTxtScrollCtrl *pTxtScroll)
{
	if(pTxtScroll)
		gb_txtScrollCnt &= 0xFFFFFF00;
}

#endif
