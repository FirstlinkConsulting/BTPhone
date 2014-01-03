/*******************************************************************************
 * @file    BoardCfg_SPT1052C.h
 * @brief   board configure header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  he_yl
 * @date    2012-12-27
 * @version 1.0
 * 1. ����mmiϵͳ���Ѷ����˵�gpio������Ҫɾ����ش��룬ֻ�轫�䶨��ΪINVALID_GPIO
*******************************************************************************/
#ifndef __BOARDCFG_SPT1052C_H__
#define __BOARDCFG_SPT1052C_H__


/******************************************************************************/
/*   ģ�������صİ弶��������                                               */
/*                                                                            */
/******************************************************************************/
#define ADVAL_MIN                   (ADVAL_KEY1-OFFSET)

#define OFFSET                      (0x1e)  //mv

#if (SYS_KEY_MODE == 4)
#define ADKEY_NUMBER                (6)//����ģʽ
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
#define ADKEY_NUMBER                (5)//���ģʽ
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
#define ADKEY_NUMBER                (4)//�ļ�ģʽ��
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
#define ADKEY_NUMBER                (4)//�ļ�ģʽһ
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
#define ADKEY_NUMBER                (3)//����ģʽ
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
/*   GPIOʹ������İ弶����                                                   */
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
/*   �弶�������ã���SDMMC�ӿڵ�ѡ��                                          */
/*                                                                            */
/******************************************************************************/
//T���ӿ�ѡ��
#define SD_INTERFACE_TYPE           INTERFACE_SD2 
//��ӡ����IDѡ��
#define CONSOLE_UART_ID             uiUART2
//��������IDѡ��
#define SERIAL_UART                 uiUART1


#endif //#ifndef __BOARDCFG_SPT1052C_H__

