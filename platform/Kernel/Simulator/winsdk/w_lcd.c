/*****************************************************************************
 * Copyright (C) Anyka 2005
 *****************************************************************************
 *   Project:
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "vme.h"
#include "w_winvme.h"
#include "Eng_Graph.h"
#include "Gbl_Global.h"
#include "Fwl_lcd.h"
#include "eng_debug.h"

#define EXTRACT_RED(clr)            (T_U8)((((clr) >>11) & 0xFF) << 3)
#define EXTRACT_GREEN(clr)          (T_U8)((((clr) >>5) & 0xFF) << 2)
#define EXTRACT_BLUE(clr)           (T_U8)(((clr) & 0xFF) << 3)

static T_U8 s_MainDispBuf[MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 3 + 1];

T_VOID lcd_set_disp_buf(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content);

/**
 * @brief Initialize the LCD
 * Initialize master LCD and slave LCD and then turn on them
 * @author JinBo
 * @date 2004-09-14
 * @param T_VOID
 * @return T_VOID
 */
#if(NO_DISPLAY == 0)
T_VOID lcd_initial(T_VOID)
{
    GRAPH_WIDTH  = MAIN_LCD_WIDTH;
    GRAPH_HEIGHT = MAIN_LCD_HEIGHT;
    gb.Lcd_Lock = AK_FALSE;
    return;
}
#endif

T_VOID lcd_lock(T_U8 locked)
{
    gb.Lcd_Lock = locked;
}

/**
 * @brief Turn on the LCD
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @return T_VOID
 */
#if(NO_DISPLAY == 0)
T_VOID lcd_turn_on(T_VOID)
{
    winvme_DisplayOn();
    winvme_DisplayUpdate();

    return;
}
#endif


/**
 * @brief Turn off the LCD
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @return T_VOID
 */
#if(NO_DISPLAY == 0)
T_VOID lcd_turn_off(T_VOID)
{
    winvme_DisplayOff();
    winvme_DisplayUpdate();

    return;
}
#endif
/**
 * @brief Turn on the LCD backlight
 * @author ZouMai
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @param T_U8 val: backlight brightness value: 0 - LCD_MAX_BRIGHTNESS
 * @return T_VOID
 * @retval
 */
T_VOID lcd_backlight_on(T_VOID)
{
}

/**
 * @brief Turn off the LCD backlight
 * @author ZouMai
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @return T_VOID
 * @retval
 */
T_VOID lcd_backlight_off(T_VOID)
{
}

/**
 * @brief Set pixel
 * Set the color of the specified pixel
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @param T_U16 x, T_U16 y: the coordinate of the pixel
 * @param T_COLOR color: the color of the pixel
 * @return T_VOID
 */
T_VOID lcd_set_pixel(T_U16 x, T_U16 y, T_U16 color)
{
    T_U8    *pDispBuf;
    T_COLOR clr;

    if (gb.Lcd_Lock)
        return;

    /* if ponint is out of screen then return */
    if (x >= GRAPH_WIDTH || y >= GRAPH_HEIGHT)
        return;

#if (USE_COLOR_LCD)
    clr = EXTRACT_RED(color) << 16 | EXTRACT_GREEN(color) << 8 | EXTRACT_BLUE(color);
#else
    if (color)
        clr = 0xffffff;
    else
        clr = 0x0;
#endif

    pDispBuf = s_MainDispBuf + (y * GRAPH_WIDTH + x) * 3;

    *pDispBuf++ = (T_U8)(clr & 0xff);             /* red color */
    *pDispBuf++ = (T_U8)((clr & 0xff00) >> 8);    /* green color */
    *pDispBuf = (T_U8)((clr & 0xff0000) >> 16);   /* blue color */
    winvme_DisplayUpdate();
}   /* end of lcd_set_pixel() */

/**
 * @brief Get pixel
 * Get the color of the specified pixel
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @param T_U16 x, T_U16 y: the coordinate of the pixel
 * @return T_COLOR: The color of the pixel
 */
T_U16 lcd_get_pixel(T_U16 x, T_U16 y)
{
    T_U8    *pDispBuf;

    /* if ponint is out of screen then return */
    if (x >= GRAPH_WIDTH || y >= GRAPH_HEIGHT)
        return 0;

    pDispBuf = s_MainDispBuf + (y * GRAPH_WIDTH + x) * 3;

#if (USE_COLOR_LCD)
    return RGB_COLOR(*pDispBuf, *(pDispBuf+1), *(pDispBuf+2));
#else
    if (0xff == *pDispBuf)
        return CLR_WHITE;
    else
        return CLR_BLACK;
#endif
}   /* end of lcd_get_pixel() */

/**
 * @brief Set display buffer
 * Set display buffer with given content
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @param T_U16 left, T_U16 top: the coordinate of the rect in LCD
 * @param T_U16 width, T_U16 height: the width and right of the content
 * @param const T_U8 *content: the content that will be set to display buffer
 * @return T_VOID
 */
T_VOID lcd_set_disp_buf(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content)
{
    T_U16       i;
    T_U16       disp_w, disp_h;
    T_U16       line_offset;
    T_U8        *pDispBuf;

    if (content == AK_NULL)
        return;

    if (left >= GRAPH_WIDTH || top >= GRAPH_HEIGHT)
        return;

    if (left + width > GRAPH_WIDTH)
        disp_w = GRAPH_WIDTH - left;
    else
        disp_w = width;

    if (top + height > GRAPH_HEIGHT)
        disp_h = GRAPH_HEIGHT - top;
    else
        disp_h = height;

    pDispBuf = s_MainDispBuf + (top * GRAPH_WIDTH+ left) * 3;
    line_offset = GRAPH_WIDTH * 3;

    for (i = 0; i < disp_h; i++)
    {
        memcpy(pDispBuf, content + (i * width) * 3, disp_w * 3);
        pDispBuf += line_offset;
    }
}   /* end of lcd_set_disp_buf() */

T_U8 *lcd_get_disp_buf_addr_large(T_VOID)
{
    return s_MainDispBuf;
}

/**
 * @brief Refresh the LCD
 * Send the display buffer to specified LCD and refresh the screen
 * @author JinBo
 * @date 2004-09-14
 * @param T_eLCD lcd: selected LCD, must be LCD_M or LCD_S
 * @param T_U16 left, T_U16 top: the coordinate of the rect that will be refreshed
 * @param T_U16 width, T_U16 height: the width and height of the refresh rect
 * @return T_VOID
 */
T_VOID lcd_refresh(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content, T_BOOL reversed)
{    
    T_U8 temp_buffer[MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 3];
#if (USE_COLOR_LCD)   
    T_U32 i, j;
    T_U16 *pcontent = (T_U16 *)content;
#else
    T_U32 page = height >> 3, page_temp, column;
    T_U8 content_temp, bit, bit_info, *pbuffer;
#endif

    if (gb.Lcd_Lock)
        return;

    if (left+width>GRAPH_WIDTH || top+height>GRAPH_HEIGHT)
    {
        AK_DEBUG_OUTPUT("WARNING, overflow the lcd boundary: left+width=%d,top+height=%d!\r\n", left+width, top+height);
        return;
    }

#if (USE_COLOR_LCD)
    for (i = 0, j = 0; i < (T_U32)(width * height * 3); i+=3)
    {
        if (reversed)
        {
            *(temp_buffer + i) =        ~EXTRACT_RED(*(pcontent+j));
            *(temp_buffer + i + 1) =    ~EXTRACT_GREEN(*(pcontent+j));
            *(temp_buffer + i + 2) =    ~EXTRACT_BLUE(*(pcontent+j));
        }
        else
        {
            *(temp_buffer + i) =        EXTRACT_RED(*(pcontent+j));
            *(temp_buffer + i + 1) =    EXTRACT_GREEN(*(pcontent+j));
            *(temp_buffer + i + 2) =    EXTRACT_BLUE(*(pcontent+j));
        }
        j++;
    }
#else
    for(page_temp = 0; page_temp < page; page_temp++)
    {
        for(column = 0; column < width; column++)
        {
            content_temp = *(content + (page_temp * width + column));
            
            for (bit = 0; bit < 8; bit++)
            {
                bit_info = content_temp >> bit;
                pbuffer = (temp_buffer + (width * ((page_temp << 3) + bit) * 3) + (column * 3));
                if (pbuffer >= temp_buffer+MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 3)
                    continue;
                if (((0x1 & bit_info) && (reversed)) || (!(0x1 & bit_info) && (!reversed)))
                {
                    memset(pbuffer, 0, 3);
                }
                else
                {
                    memset(pbuffer, 255, 3);                    
                }
            }
        }
    }
#endif

    lcd_set_disp_buf(left, top, width, height, temp_buffer);
    
    winvme_DisplayUpdate();
}

/**
 * @brief fill lcd rect as a specified color
 * @author YiRuoxiang
 * @date 2008-04-02
 * @param[in] left, top the coordinate of the rect that will be refreshed
 * @param[in] width, height the width and height of the refresh rect
 * @param[in] black refresh to black or white, AK_TRUE means black, AK_FALSE means white
 * @return T_VOID
 */
T_VOID lcd_fill_rect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color)
{
    T_U8 temp_buffer[MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 3];
    T_U32 i;
#if (!USE_COLOR_LCD)
    T_U8 clr;
#endif

    if (gb.Lcd_Lock)
        return;

    if (left+width>GRAPH_WIDTH || top+height>GRAPH_HEIGHT)
        AK_DEBUG_OUTPUT("WARNING, overflow the lcd boundary: left+width=%d,top+height=%d!\r\n", left+width, top+height);

    if (left+width>GRAPH_WIDTH)
        width = GRAPH_WIDTH - left;
    if (top+height>GRAPH_HEIGHT)
        height = GRAPH_HEIGHT - top;
    
#if (USE_COLOR_LCD)    
    for (i = 0; i < (T_U32)(width * height * 3); i+=3)
    {
        *(temp_buffer + i) =        EXTRACT_RED(color);
        *(temp_buffer + i + 1) =    EXTRACT_GREEN(color);
        *(temp_buffer + i + 2) =    EXTRACT_BLUE(color);
    }
#else
    if (color)
        clr = 0xFF;
    else
        clr = 0x0;
    
    for (i = 0; i < (T_U32)(width * height * 3); i++)
        *(temp_buffer + i) = clr;
#endif

    lcd_set_disp_buf(left, top, width, height, temp_buffer);
    
    winvme_DisplayUpdate();
}

T_U8 lcd_set_contrast(T_U8 contrast)
{
    return contrast;
}

T_BOOL lcd_enable_set_contrast(T_VOID)
{
    return AK_TRUE;
}

#if(NO_DISPLAY == 0)
T_VOID lcd_sleep(T_VOID)
{

}
#endif

// the end of file