/*******************************************************************************
 * @file lcd.h
 * @brief LCD Control Register bits define file
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author ZGX
 * @date 2012-11-07
 * @version 1.0
 * @ref AK1180 programme manual.
*******************************************************************************/
#ifndef __LCD_H__
#define __LCD_H__


//REG_LCD_CFG
#define MPU_RS_SIG_CMD          (0 << 18)
#define MPU_RS_SIG_DATA         (1 << 18)

//REG_LCD_CTRL
#define POSITIVE                (1 << 0)
#define NEGATIVE                (0 << 0)
#define COLOR_MODE_4K           (0 << 13)
#define COLOR_MODE_64K_256K     (1 << 13)
#define COMMAND_WIDTH           (1 << 14)
#define DATA_WIDTH              (~(3 << 14))
#define DATA_FORMAT_RGB         (1 << 16)
#define DATA_FORMAT_BGR         (0 << 16)

//REG_LCD_OPT
#define MPU_GO                  (1 << 1)

//REG_LCD_STA
#define MPU_DISP_OK             (1 << 2)

//REG_LCD_BG_COLOR
#define DISP_RGB_BG             (1 << 24)

//REG_LCD_READBACK
#define RB_EN                   (1 << 19)
#define READ_RD_N               (1 << 18)
#define READ_CS_N               (1 << 17)
#define READ_RS                 (1 << 16)

#endif

