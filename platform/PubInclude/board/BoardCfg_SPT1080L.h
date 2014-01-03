/*******************************************************************************
 * @file    BoardCfg_SPT1080L.h
 * @brief   board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  he_yl
 * @date    2012-12-27
 * @version 1.0
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
*******************************************************************************/
#ifndef __BOARDCFG_SPT1080L_H__
#define __BOARDCFG_SPT1080L_H__


/******************************************************************************/
/*   模拟键盘相关的板级参数配置                                               */
/*                                                                            */
/******************************************************************************/
#define ADVAL_MAX                   (ADVAL_KEY6+OFFSET)
#define ADVAL_MIN                   (ADVAL_KEY1-OFFSET)
#define ADKEY_NUMBER                (6)
#define OFFSET                      (30)  //mv

#define ADVAL_KEY1                  (30)    //kbRECORD(kbVOLSUB)
#define ADVAL_KEY2                  (232)   //kbBT(kbVOLADD)
#define ADVAL_KEY3                  (440)   //(kbRIGHT)
#define ADVAL_KEY4                  (660)   //(kbLEFT)
#define ADVAL_KEY5                  (866)   //(kbFUNC)
#define ADVAL_KEY6                  (1108)  //(kbOK)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)


/******************************************************************************/
/*   GPIO使用情况的板级配置                                                   */
/*                                                                            */
/******************************************************************************/
#define GPIO_I2C_SCLK               2
#define GPIO_I2C_SDA                0

#define GPIO_CAMERA_RESET           26
#define GPIO_CAMERA_CHIP_ENABLE     INVALID_GPIO
#define GPIO_CAMERA_HIT_LED         INVALID_GPIO

#define GPIO_HP_DET                 1
#define GPIO_LINEIN_DET             7
#define GPIO_SPEAKER_CONTROL        18

#define GPIO_SD_DET                 23

#define GPIO_USB_DET                19

#define GPIO_LED_BLUE               4
#define GPIO_LED_RED                3


#define USB_DETECT_GPIO             19
#define POWERKEY_GPIO               20
#define GPIO_AIN1_INT               21
#define GPIO_AIN0_INT               24
#define GPIO_BT_LDO_ON			    INVALID_GPIO

/******************************************************************************/
/*   板级杂项配置，如SDMMC接口的选择                                          */
/*                                                                            */
/******************************************************************************/
#define SD_INTERFACE_TYPE           INTERFACE_MMC1

//打印串口ID选择
#define CONSOLE_UART_ID             uiUART2
//蓝牙串口ID选择
#define SERIAL_UART                 uiUART1
#endif //#ifndef __BOARDCFG_SPT1080L_H__

