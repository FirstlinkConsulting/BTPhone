/**
 * @file
 * @brief ANYKA software
 * this file will constraint the access to the bottom layer display function,
 * avoid resource competition. Also, this file os for porting to 
 * different OS
 *
 * @author ZouMai
 * @date 2001-4-20
 * @version 1.0
 */
#include "akdefine.h"
#include "Fwl_LCD.h"
#include "Fwl_System.h"
#include "Eng_Graph.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Timer.h"
#include <string.h>
//#include "m_state.h"
#include "arch_lcd.h"
//#include "Eng_loadMem.h"
#include "eng_debug.h"
#include "gpio_define.h"

#define LCD_HEIGHT_UNIT     8
#define BYTE_WIDTH          8

#if(NO_DISPLAY == 0)
static T_U8 Lcd_Lock = AK_FALSE;

extern void m_paint(void);
extern void SM_SetPaintStatus(unsigned char paint);
/**
* @file
 * @brief draw a specified rect for black white lcd
 * different OS
 *
 * @author Justin.Zhao
 * @date 2008-5-9
 * @version 1.0
 */
T_VOID Fwl_DrawRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color, T_U8 filled_flag)
{
	T_U8 filled = filled_flag & 0x01;
	T_U8 left_fill  = !(filled_flag & 0x02);
	T_U8 right_fill = !(filled_flag & 0x04);
#if (!USE_COLOR_LCD)
	//T_U8* commbuf = Gbl_GetCommBuff();
	//T_U32 commbufsize = Gbl_GetCommBuffSize();
	T_U8* commbuf = AK_NULL;
	T_U32 commbufsize = 0;
#endif

	if (Lcd_Lock)
	{
		return;
	}

#if (USE_COLOR_LCD)
    if (filled)
    {
        Fwl_FillRect(left, top, width, height, color);
        return;
    }
    else
    {	
        Fwl_FillRect(left, top, width, 1, color);
		if(left_fill)
			Fwl_FillRect(left, top, 1, height, color);
		else
			Fwl_FillRect(left, top, 1, 1, color);
        Fwl_FillRect(left, (T_POS)(top+height-1), width, 1, color);
		if(right_fill)
			Fwl_FillRect((T_POS)(left+width-1), top, 1, height, color);
		else
			Fwl_FillRect((T_POS)(left+width-1), top, 1, 1, color);
    }
#else
    T_U8 drawtop, drawheight, row, col,lasttop, drawrow;
    T_U8 *pbyte;
    
    if ((top == (top & ~(LCD_HEIGHT_UNIT-1))) && (height == (height & ~(LCD_HEIGHT_UNIT-1))) && filled)
    {
        Fwl_FillRect(left, top, width, height, color);
		return;
	}
	
	commbufsize = DISPLAY_COMMONBUFFSIZE;
	commbuf = LoadMem_CommBuffMalloc(DISPLAY_COMMONBUFFSIZE);
	if (AK_NULL == commbuf)
	{
		AK_DEBUG_OUTPUT("malloc common buff is null.\n\r");
		return;
    }

    drawtop = top & ~(LCD_HEIGHT_UNIT-1);
    drawheight = ((height + top + LCD_HEIGHT_UNIT - 1) & ~(LCD_HEIGHT_UNIT-1)) - drawtop;
    lasttop = drawtop;
    memset(commbuf, 0, commbufsize);
    for(drawrow = drawtop; drawrow < drawtop+drawheight; )    
    {
        //draw 8 lines
        for(col = (T_U8)left; col < (T_U8)(left+width); col++)
        {
            //seek the buffer place
            pbyte = commbuf+((drawrow-lasttop)/BYTE_WIDTH)*width + col - left;
            //draw 8 pixels: 1 byte.
            for(row=drawrow; row < drawrow+LCD_HEIGHT_UNIT; row++)
            {
                if ((top <= row && row <= (top+height-1))
                    && (filled || row == top || row == (top+height-1)
                    || col == left || col == (left+width-1)))
                {	
                    if (color)
					{
						if((col==left+width-1) && !right_fill)
						{
							if((row!=top) && (row!=top+height-1)) 
								continue;
						}
						if((col==left) && !left_fill)
						{
							if((row!=top) && (row!=top+height-1)) 
								continue;
						}

						*pbyte |= (1 << (row & (BYTE_WIDTH-1)));
					}
                }
                else if (!color)
                    *pbyte |= (1 << (row & (BYTE_WIDTH-1)));
            }
        }

        drawrow += LCD_HEIGHT_UNIT;
        //if buff is full, refresh lcd
        if ((T_U32)(((drawrow-lasttop)/BYTE_WIDTH+1)*width)>=commbufsize)
        {            
        	T_RECT rect;

			rect.left	= left;
			rect.top	= lasttop;
			rect.width	= width;
			rect.height = (T_U8)(drawrow - lasttop);
			
            lcd_refresh(&rect, commbuf);
            memset(commbuf, 0, commbufsize);
            lasttop = drawrow;
        }
    }

    if (lasttop != drawrow)
    {
    	T_RECT rect;

		rect.left	= left;
		rect.top	= lasttop;
		rect.width	= width;
		rect.height = (T_U8)(drawrow - lasttop);
		
        lcd_refresh(&rect, commbuf);
    }

	LoadMem_CommBuffFree();
#endif
}

static T_U8 lcd_state = AK_FALSE;

#pragma arm section rwdata = "_cachedata_"
static T_TIMER lcd_timer_id = ERROR_TIMER;
#pragma arm section rwdata

//#pragma arm section code = "_bootcode1_"

T_VOID LCD_turn_on_callback(T_TIMER timer_id, T_U32 delay)
{
    Fwl_DisplayBacklightOn();
    lcd_timer_id = Fwl_TimerStop(lcd_timer_id);
}

//#pragma arm section code


#pragma arm section code = "_frequentcode_"
/**
 * @brief       Turn on LCD
 * @author    Chen weiwen
 * @date        2008-05-30
 * @param    T_BOOL[in]: 
 *                  bDelayEnable AK_TRUE : add delay
 *                                      AK_FALSE : do nothing
 * @return    T_VOID
 */
void Fwl_LCD_on(T_BOOL bDelayEnable)
{    
    if (lcd_state)
        return;

    Fwl_DelayUs(200000);
    Fwl_DisplayOn();
    if(bDelayEnable)
    {
#if (USE_COLOR_LCD)
        Fwl_DelayUs(100000);
#else
        Fwl_DelayUs(300000);
#endif
    }
    lcd_state = AK_TRUE;
    SM_SetPaintStatus(AK_TRUE);

    if(bDelayEnable)
    {
        Fwl_DisplayBacklightOn();
    }
    else
    {
#if (USE_COLOR_LCD)
        lcd_timer_id = Fwl_TimerStart(100, AK_FALSE, LCD_turn_on_callback);
#else
        lcd_timer_id = Fwl_TimerStart(300, AK_FALSE, LCD_turn_on_callback);
#endif
    }
}
#pragma arm section code

void Fwl_LCD_off()
{    
    if (!lcd_state)
        return;

    Fwl_DisplayBacklightOff();
    Fwl_DisplayOff();    
    lcd_state = AK_FALSE;
    SM_SetPaintStatus(AK_FALSE);
}


#pragma arm section code = "_audioplayer_" 
T_VOID Fwl_FillRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color)
{
	T_RECT rect;

	if (Lcd_Lock)
	{
		return;
	}
	
    //提高主频
    Fwl_FreqPush(FREQ_APP_MAX);

	rect.left	= left;
	rect.top	= top;
	rect.width	= width;
	rect.height = height;
	
    lcd_fill_rect(&rect, color);
    Fwl_FreqPop();
}
#pragma arm section code

#pragma arm section code = "_sysinit_"

T_VOID Fwl_DisplayInit(T_VOID)          
{
	Lcd_Lock = AK_FALSE;
	lcd_initial();
}

#pragma arm section code 

#pragma arm section code = "_bootcode1_"

T_VOID Fwl_LCD_lock(T_U8 locked)             
{
	Lcd_Lock = locked;
}

#pragma arm section code

#pragma arm section code = "_lcd_ctrl_"

T_VOID Fwl_SetPixel(T_U16 x, T_U16 y, T_U16 color) 
{
	if (Lcd_Lock)
	{
		return;
	}
	
	lcd_set_pixel(x, y, color);
}

T_VOID Fwl_RefreshRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content, T_BOOL reversed)  
{
    //if(left + width > MAIN_LCD_WIDTH || top+height > MAIN_LCD_HEIGHT)
    //{
    //    AK_DEBUG_OUTPUT("show out of LCd l:%d,t:%d,w:%d,h:%d\r\n",left,top,width,height);
    //}
    T_RECT rect;

	if (Lcd_Lock)
	{
		return;
	}

	rect.left	= left;
	rect.top	= top;
	rect.width	= width;
	rect.height = height;
		
	lcd_refresh(&rect, content);
}

T_U8 Fwl_SetContrast(T_U8 contrast) 
{
	return lcd_set_contrast(contrast);
}

T_BOOL Fwl_Enable_Set_Contrast(T_VOID)
{
	return AK_FALSE;// lcd_enable_set_contrast();此接口驱动库未提供，待日后看需要
}

T_VOID Fwl_Lcd_Rotate1(T_BOOL rotate)		
{
    if(rotate)
	    lcd_rotate(DEGREE_270);
    else
        lcd_rotate(DEGREE_0);
}


T_eLCD_DEGREE Fwl_Lcd_Get_Degree(T_VOID)
{
	return lcd_degree();
}

T_VOID Fwl_Lcd_Set_L2_Refresh(T_VOID)
{
#ifdef OS_ANYKA
    lcd_set_mode(L2_MODE);
#endif
}
T_VOID Fwl_Lcd_Set_VEDIO_Refresh(T_VOID)
{
#ifdef OS_ANYKA
    lcd_set_mode(VIDEO_MODE);
#endif
}
T_VOID Fwl_Lcd_Set_CPU_Refresh(T_VOID)
{
#ifdef OS_ANYKA
    lcd_set_mode(CPU_MODE);
#endif
}

T_VOID Fwl_Enable_L2Refresh1(T_BOOL enable)        
{
#ifdef OS_ANYKA
	if(enable)
	    lcd_set_mode(L2_MODE);
    else
        lcd_set_mode(CPU_MODE);
#endif
}


T_eLCD_MODE Fwl_Lcd_Get_Mode(T_VOID)
{
	return lcd_get_mode();
}


T_VOID Fwl_DisplayOn(T_VOID) 
{
    store_all_int();
	lcd_turn_on();
    restore_all_int();
}

T_VOID Fwl_DisplayOff(T_VOID)  
{
    store_all_int();
	lcd_turn_off();
    restore_all_int();
}

T_VOID Fwl_DisplayBacklightOn(T_VOID)   
{
	gpio_set_pin_level(GPIO_LCD_BACKLIGHT, 1);
}

T_VOID Fwl_DisplayBacklightOff(T_VOID)  
{
	gpio_set_pin_level(GPIO_LCD_BACKLIGHT, 0);
}

#pragma arm section code

T_VOID Fwl_DisplaySleep(T_VOID)
{
	gpio_set_pin_level(GPIO_LCD_BACKLIGHT, 0);
    lcd_turn_off();
}



#pragma arm section code = "_bootcode1_"
T_BOOL Fwl_LCDVMCheckRefresh(T_VOID)
{
#ifdef OS_ANYKA
	return lcd_check_refresh_finish();
#else
	return AK_TRUE;
#endif
}
#pragma arm section code

T_VOID Fwl_LCDVMSetRefresh(T_U16 left, T_U16 top, T_U32 width, T_U32 height, T_BOOL reversed)
{
	T_RECT rect;

	if (Lcd_Lock)
	{
		return;
	}

	rect.left	= left;
	rect.top	= top;
	rect.width	= (T_U16)width;
	rect.height = (T_U16)height;
	
    store_all_int();
    lcd_refresh(&rect, AK_NULL);
    restore_all_int();
}



#endif

/* end of files */
