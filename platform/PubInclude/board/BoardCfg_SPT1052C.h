/*******************************************************************************
 * @file    BoardCfg_SPT1052C.h
 * @brief   board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  he_yl
 * @date    2012-12-27
 * @version 1.0
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
*******************************************************************************/
#ifndef __BOARDCFG_SPT1052C_H__
#define __BOARDCFG_SPT1052C_H__


/******************************************************************************/
/*   模拟键盘相关的板级参数配置                                               */
/*                                                                            */
/******************************************************************************/
#define ADVAL_MIN                   (ADVAL_KEY1-OFFSET)

#define OFFSET                      (0x1e)  //mv

#if (SYS_KEY_MODE == 4)
#define ADKEY_NUMBER                (6)//六键模式
#define ADVAL_MAX                   (ADVAL_KEY6+OFFSET)

#define ADVAL_KEY1                  (0x1e)   //(kbLEFT)
#define ADVAL_KEY2                  (0xe8)   //(kbRIGHT)
#define ADVAL_KEY3                  (0x1bc)    //(kbREC)
#define ADVAL_KEY4                  (0x297)   //(kbBT)
#define ADVAL_KEY5                  (0x35c)   //(kbFUNC)
#define ADVAL_KEY6                  (0x427)  //(kbPLAY)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)
#elif (SYS_KEY_MODE == 3)
#define ADKEY_NUMBER                (5)//五键模式
#define ADVAL_MAX                   (ADVAL_KEY5+OFFSET)

#define ADVAL_KEY1                  (0x1e)   //(kbLEFT)
#define ADVAL_KEY2                  (0xe8)   //(kbRIGHT)
#define ADVAL_KEY3                  (0x297)   //(kbBT)
#define ADVAL_KEY4                  (0x35c)   //(kbFUNC)
#define ADVAL_KEY5                  (0x427)  //(kbPLAY)
#define ADVAL_KEY6                  (0xffff)//(kbNULL)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)
#elif(SYS_KEY_MODE == 2)
#define ADKEY_NUMBER                (4)//四键模式二
#define ADVAL_MAX                   (ADVAL_KEY4+OFFSET)

#define ADVAL_KEY1                  (0x1e)   //(kbLEFT)
#define ADVAL_KEY2                  (0xe8)   //(kbRIGHT)
#define ADVAL_KEY3                  (0x35c)   //(kbFUNC)
#define ADVAL_KEY4                  (0x427)  //(kbPLAY)
#define ADVAL_KEY5                  (0xffff)//(kbNULL)
#define ADVAL_KEY6                  (0xffff)//(kbNULL)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)
#elif(SYS_KEY_MODE == 1)
#define ADKEY_NUMBER                (4)//四键模式一
#define ADVAL_MAX                   (ADVAL_KEY4+OFFSET)

#define ADVAL_KEY1                  (0x1e)   //(kbLEFT)
#define ADVAL_KEY2                  (0xe8)   //(kbRIGHT)
#define ADVAL_KEY3                  (0x297)   //(kbBT)
#define ADVAL_KEY4                  (0x427)  //(kbPLAY)
#define ADVAL_KEY5                  (0xffff)//(kbNULL)
#define ADVAL_KEY6                  (0xffff)//(kbNULL)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)
#elif(SYS_KEY_MODE == 0)
#define ADKEY_NUMBER                (3)//三键模式
#define ADVAL_MAX                   (ADVAL_KEY3+OFFSET)

#define ADVAL_KEY1                  (0x1e)   //(kbLEFT)
#define ADVAL_KEY2                  (0xe8)   //(kbRIGHT)
#define ADVAL_KEY3                  (0x427)  //(kbPLAY)
#define ADVAL_KEY4                  (0xffff)//(kbNULL)
#define ADVAL_KEY5                  (0xffff)//(kbNULL)
#define ADVAL_KEY6                  (0xffff)//(kbNULL)
#define ADVAL_KEY7                  (0xffff)//(kbNULL)
#define ADVAL_KEY8                  (0xffff)//(kbNULL)
#endif

/******************************************************************************/
/*   GPIO使用情况的板级配置                                                   */
/*                                                                            */
/******************************************************************************/
#define GPIO_I2C_SCLK               0
#define GPIO_I2C_SDA                16

#define GPIO_CAMERA_RESET           INVALID_GPIO
#define GPIO_CAMERA_CHIP_ENABLE     INVALID_GPIO
#define GPIO_CAMERA_HIT_LED         INVALID_GPIO

#define GPIO_HP_DET                 38
#define GPIO_LINEIN_DET             41
#define GPIO_SPEAKER_CONTROL        50

#define GPIO_SD_DET                 2

#define GPIO_USB_DET                19

#define GPIO_LED_BLUE               48
#define GPIO_LED_RED                49

#define USB_DETECT_GPIO             19
#define POWERKEY_GPIO               20
#define GPIO_AIN1_INT               21
#define GPIO_AIN0_INT               24
#define GPIO_BT_LDO_ON			    7

/******************************************************************************/
/*   板级杂项配置，如SDMMC接口的选择                                          */
/*                                                                            */
/******************************************************************************/
//T卡接口选择
#define SD_INTERFACE_TYPE           INTERFACE_SD2 
//打印串口ID选择
#define CONSOLE_UART_ID             uiUART2
//蓝牙串口ID选择
#define SERIAL_UART                 uiUART1


#endif //#ifndef __BOARDCFG_SPT1052C_H__

