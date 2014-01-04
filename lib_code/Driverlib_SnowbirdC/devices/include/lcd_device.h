/*******************************************************************************
 * @file lcd_device.h
 * @brief LCD device header file
 * This file provides all the APIs of LCD device, such as initial LCD, turn on/off LCD,
 * clean LCD and refresh LCD etc.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ChenWeiwen
 * @date 2008-06-14
 * @version 1.0
 * @ref AK10X6X technical manual.
*******************************************************************************/
#ifndef __LCD_DEVICE_H__
#define __LCD_DEVICE_H__


#include "lcd.h"
#include "arch_lcd.h"

#define DATA                    0xFC
#define CMD                     0xFD
#define DELAY_FLAG              0xFE  ///<first parameter is 0xfffe, then 2nd parameter is delay time count
#define END_FLAG                0xFF  ///<first parameter is 0xffff, then parameter table is over

/**
 * @brief lcd info define.
 * define the info of all lcd
 */
typedef struct
{

    T_U8    wr_low_bit;                    ///< Clock cycle, Unit Hz
    T_U8    wr_cycle_bit;
    T_U8    lcd_BusWidth;                   ///< data bus width(8,9,16,18 or 24)  
    T_U8    lcd_type;                       ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    
    T_VOID  *lcd_reginfo;                   ///< refer to T_RGBLCD_INFO or T_STNLCD_INFO define
    T_U32  (*lcd_read_type_func)(T_VOID);
    T_VOID (*lcd_init_func)(T_VOID);        ///< init for lcd driver 
    T_VOID (*lcd_turn_on_func)(T_VOID);
    T_VOID (*lcd_turn_off_func)(T_VOID);
    T_VOID (*lcd_set_disp_address_func)(T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2);
    T_VOID (*lcd_rotate_func)(T_eLCD_DEGREE degree);
    T_VOID (*lcd_start_dma_func)(T_VOID);
}T_LCD_FUNCTION_HANDLER;



typedef struct
{
    T_U32 DeviceID;
    T_LCD_FUNCTION_HANDLER *handler;
}T_LCD_INFO;


extern T_eLCD_DEGREE lcd_avi_mode;
/**
 * @brief Initialize the device LCD
 * Initialize master LCD
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param T_VOID
 * @return T_VOID
 */
T_VOID lcd_device_initial(T_VOID);


/**
 * @brief Turn on the LCD
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param T_VOID
 * @return T_VOID
 */
T_VOID lcd_device_turn_on(T_VOID);


/**
 * @brief Turn off the LCD
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param T_VOID
 * @return T_VOID
 */
T_VOID lcd_device_turn_off(T_VOID);


/**
 * @brief get width of the LCD
 * @author zhanggaoxin
 * @date 2012-12-7
 * @param T_VOID
 * @return T_U16
 * @retval the width of the LCD
 */
T_U16 lcd_device_get_width();


/**
 * @brief get height of the LCD
 * @author zhanggaoxin
 * @date 2012-12-7
 * @param T_VOID
 * @return T_U16
 * @retval the height of the LCD
 */
T_U16 lcd_device_get_height();


/**
 * @brief enable lcd device contrast
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param T_VOID
 * @return T_BOOL
 * @retval
 */
T_BOOL lcd_device_enable_set_contrast(T_VOID);


/**
 * @brief Set lcd device contrast value
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param T_U8 contrast: contrast value
 * @return T_VOID
 * @retval
 */
T_VOID lcd_device_set_contrast(T_U8 contrast);


/**
 * @brief fill rect area
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param[in] left, top the coordinate of the rect that will be refreshed
 * @param[in] width, height the width and height of the refresh rect
 * @param[in] color filled color
 * @return T_VOID
 */
T_VOID lcd_device_fill_rect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color);


/**
 * @brief Refresh the LCD
 * Send the display buffer to specified LCD and refresh the screen
 * @author ChenWeiwen
 * @date 2008-06-12
 * @param[in] left, top the coordinate of the rect that will be refreshed
 * @param[in] width, height the width and height of the refresh rect
 * @param[in] content data buffer to be displayed
 * @param[in] reversed reverse all points, from black to white or from white to black
 * @return T_VOID
 */
T_VOID lcd_device_refresh(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content, T_BOOL reversed);


T_VOID lcd_device_set_pixel(T_U16 x, T_U16 y, T_U16 color);

T_VOID lcd_device_set_mode(T_eLCD_DEGREE avimode);

T_VOID index_out(const T_U16 cmd);

T_VOID data_out(const T_U16 data);

T_BOOL lcd_reg_dev(T_U32 id, T_LCD_FUNCTION_HANDLER *handler);

T_LCD_FUNCTION_HANDLER *lcd_probe(T_VOID);


#endif
