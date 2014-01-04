/*******************************************************************************
 * @file lcd.c
 * @brief LCD driver header file
 * This file provides all the APIs of LCD dirver, such as initial LCD, turn on/off LCD,
 * clean LCD and refresh LCD etc.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ChenWeiwen
 * @date 2008-06-14
 * @version 1.0
 * @ref AK1036X technical manual.
 * @NOTE
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "l2.h"
#include "share_pin.h"
#include "clk.h"
#include "lcd.h"
#include "arch_lcd.h"
#include "mmu.h"
#include "interrupt.h"
#include "lcd_device.h"
#include "arch_timer.h"
#include "drv_cfg.h"
#include "hal_errorstr.h"

#if DRV_SUPPORT_LCD > 0

#define LCD_HORIZONTAL

typedef enum
{
    REFRESH_END = 0,
    REFRESH_BEGIN
}T_eREFRESH_STATE;

#pragma arm section rwdata = "_drvbootcache_"
T_eLCD_MODE lcd_mode = LCD_CPU_MODE;
T_LCD_FUNCTION_HANDLER *lcd_param = AK_NULL;

#ifdef LCD_HORIZONTAL
    T_eLCD_DEGREE lcd_avi_mode = DEGREE_90;
#else
    T_eLCD_DEGREE lcd_avi_mode = DEGREE_0;
#endif

T_eREFRESH_STATE m_refreshState = REFRESH_END;
#pragma arm section rwdata


#pragma arm section code = "_drvbootcode_"
/* the following code must be put in bootcode */
static T_VOID lcd_oprate_enter(T_VOID)
{
    sys_module_enable(eVME_LCD_CLK, AK_TRUE);
}
#pragma arm section code


/**
 * @brief   initialize the LCD
 * Initialize master LCD 
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
#pragma arm section code = "_sysinit_"
T_VOID lcd_initial(T_VOID)
{
    sys_module_reset(eVME_LCD_CLK);

    lcd_oprate_enter();
    sys_share_pin_lock(ePIN_AS_LCD);

    lcd_param = lcd_probe();
    REG32(REG_LCD_CTRL) = (DATA_FORMAT_RGB| COLOR_MODE_64K_256K \
                             | lcd_param->wr_low_bit<<1| lcd_param->wr_cycle_bit<<6| POSITIVE);

    //set bus width
    REG32(REG_LCD_CTRL) &= DATA_WIDTH;
    REG32(REG_LCD_CTRL) |= lcd_param->lcd_BusWidth;

    lcd_param->lcd_init_func();
    lcd_param->lcd_rotate_func(lcd_avi_mode);

    m_refreshState = REFRESH_END;
}
#pragma arm section code


#pragma arm section code = "_lcd_ctrl_"
/**
 * @brief   turn on the LCD
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID lcd_turn_on(T_VOID)
{
    lcd_oprate_enter();
    lcd_device_turn_on();
}

/**
 * @brief   turn off the LCD
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID lcd_turn_off(T_VOID)
{
    lcd_oprate_enter();
    lcd_device_turn_off();
}

T_VOID index_out(const T_U16 cmd)
{
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14); //set bit16 mode first
    REG32(REG_LCD_CTRL) |= (0x1 << 14);

    REG32(REG_LCD_CFG) = (MPU_RS_SIG_CMD | 0x00);
    REG32(REG_LCD_CFG) = (MPU_RS_SIG_CMD | (cmd&0xff));
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14); //set bit8 mode
}

T_VOID data_out(const T_U16 data)
{
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14); //set bit16 mode first
    REG32(REG_LCD_CTRL) |= (0x1 << 14);

    REG32(REG_LCD_CFG) = (MPU_RS_SIG_DATA | ((data>>8)&0xff));
    REG32(REG_LCD_CFG) = (MPU_RS_SIG_DATA | (data&0xff));
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14); //set bit8 mode 
}


static T_VOID send_point_data(T_U16 color)
{
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14); //set bit16 mode first
    REG32(REG_LCD_CTRL) |= (0x1 << 14);

    REG32(REG_LCD_CFG) = (MPU_RS_SIG_DATA | ((color>>8)&0xff));
    REG32(REG_LCD_CFG) = (MPU_RS_SIG_DATA | (color&0xff));
    REG32(REG_LCD_CTRL) &= ~(0x3 << 14);
}

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
T_VOID lcd_refresh(const T_RECT *dsp_rect, const T_U8 *content, T_eLCD_COLOR type)
{
    T_U32 i;
    T_U32 width,height;

    lcd_oprate_enter();

    if (m_refreshState == REFRESH_BEGIN)
    {
        while (!(REG32(REG_LCD_STA) & MPU_DISP_OK));
        m_refreshState = REFRESH_END;
    }
    REG32(REG_LCD_IMG_SIZE) = ((dsp_rect->width << 10) | dsp_rect->height);

    lcd_param->lcd_set_disp_address_func(dsp_rect->left, dsp_rect->top, dsp_rect->width-1, dsp_rect->height-1);
    switch (lcd_mode)
    {
        case LCD_L2_MODE:
            REG32(REG_LCD_FRAME_ADDR) = (T_U32)((MMU_Vaddr2Paddr((T_U32)content)/4)&0x1ffff);
            REG32(REG_LCD_OPT) |= MPU_GO;
            REG32(REG_LCD_OPT) &= ~MPU_GO;
            m_refreshState = REFRESH_BEGIN;
            drv_print("L2 mode", 0, AK_TRUE);
            break;
        case LCD_CPU_MODE:
            width  = dsp_rect->width;
            height = dsp_rect->height;
            if (LCD_ONE_COLOR == type)
            {
                for (i=0; i<width*height; i++)
                {
                    send_point_data(*((T_U16 *)content));
                }
            }
            else
            {
                for (i=0; i<width*height*2; i+=2)
                {
                    send_point_data((*((T_U16*)(content + i))));
                }
            }
            //drv_print("CPU mode", 0, AK_TRUE);
            break;
        defualt:
            drv_print("error mode", 0, AK_TRUE);
            break;
    }
}

T_eLCD_MODE lcd_get_mode(T_VOID)
{
    return lcd_mode;
}

T_VOID lcd_set_mode(T_eLCD_MODE mode)
{
    lcd_mode = mode;
}

/**
 * @brief   set contrast value
 * @author  ChenWeiwen
 * @date    2008-06-14
 * @param   [in]contrast contrast value
 * @return  T_U8
 * @retval  new contrast value after setting
 */
T_U8 lcd_set_contrast(T_U8 contrast)
{
#if(USE_COLOR_LCD)
    if (contrast>10)
    {
        contrast = 10;
    }
#else
    if (contrast>16)
    {
        contrast = 16;
    }
    if (contrast<1)
    {
        contrast = 1;
    }
#endif

    lcd_oprate_enter();
    lcd_device_set_contrast(contrast);

    delay_us(5);    // wait 2 frame or more
    return contrast;
}

/**
 * @brief   rotate the lcd
 * @author  LianGenhui
 * @date    2010-06-18
 * @param   [in]degree rotate degree
 * @return  T_BOOL
 */
T_VOID lcd_rotate(T_eLCD_DEGREE degree)
{
    lcd_oprate_enter();
    lcd_device_set_mode(degree);
}
#pragma arm section code 

#endif  //DRV_SUPPORT_LCD > 0

/* end of file */
