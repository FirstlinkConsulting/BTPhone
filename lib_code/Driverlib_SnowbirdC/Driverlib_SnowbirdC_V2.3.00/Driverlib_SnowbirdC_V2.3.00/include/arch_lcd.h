/**
 * @file lcd.h
 * @brief LCD driver header file
 * This file provides all the APIs of LCD dirver, such as initial LCD,
 * turn on/off LCD, clean LCD and refresh LCD etc.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ChenWeiwen
 * @date 2008-06-14
 * @version 1.0
 * @ref AK10X6X technical manual.
 */
#ifndef __ARCH_LCD_H__
#define __ARCH_LCD_H__


#include "anyka_types.h"


typedef enum 
{
    DEGREE_0 = 0,
    DEGREE_90,
    DEGREE_180,
    DEGREE_270
}T_eLCD_DEGREE;

typedef enum
{
    LCD_CPU_MODE,
    LCD_L2_MODE
}T_eLCD_MODE;

typedef enum
{
    LCD_ONE_COLOR=0,
    LCD_MUL_COLOR
}T_eLCD_COLOR;


/**
 * @brief   initialize the LCD
 * Initialize master LCD 
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID lcd_initial(T_VOID);

/**
 * @brief   turn on the LCD
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID lcd_turn_on(T_VOID);

/**
 * @brief   turn off the LCD
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID lcd_turn_off(T_VOID);

/**
 * @brief   get width of the LCD
 * @author  zhanggaoxin
 * @date    2012-12-7
 * @param   T_VOID
 * @return  T_U16
 * @retval  the width of the LCD
 */
T_U16 lcd_get_width(T_VOID);

/**
 * @brief   get height of the LCD
 * @author  zhanggaoxin
 * @date    2012-12-7
 * @param   T_VOID
 * @return  T_U16
 * @retval  the height of the LCD
 */
T_U16 lcd_get_height(T_VOID);

/**
 * @brief   rotate the lcd
 * @author  LianGenhui
 * @date    2010-06-18
 * @param   [in]degree rotate degree
 * @return  T_VOID
 */
T_VOID lcd_rotate(T_eLCD_DEGREE degree);

/**
 * @brief   get current lcd degree
 * @author  LianGenhui
 * @date    2010-06-18
 * @param   T_VOID
 * @return  T_eLCD_DEGREE
 * @retval  degree of LCD
 */
T_eLCD_DEGREE lcd_degree(T_VOID);

/**
 * @brief   refresh the LCD
 * Send the display buffer to specified LCD and refresh the screen
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   [in]dsp_rect the rect that will be refreshed
 * @param   [in]content data buffer to be displayed
 * @param   [in]type
 * @return  T_VOID
 */
T_VOID lcd_refresh(const T_RECT *dsp_rect, const T_U8 *content, T_eLCD_COLOR type);

/**
 * @brief   choose lcd mode 
 * @author  lhd
 * @date    2011-04-21
 * @param   [in]mode
 * @return  T_VOID
 */
T_VOID lcd_set_mode(T_eLCD_MODE mode);

/**
 * @brief   get lcd mode 
 * @author  lhd
 * @date    2011-04-21
 * @param   T_VOID
 * @return  T_eLCD_MODE
 * @retval
 */
T_eLCD_MODE lcd_get_mode(T_VOID);

/**
 * @brief   set contrast value
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   [in]contrast contrast value
 * @return  T_U8
 * @retval  new contrast value after setting
 */
T_U8 lcd_set_contrast(T_U8 contrast);


#endif    //__ARCH_LCD_H__

