/*******************************************************************************
 * @file    BoardCfg_SPT1050L.h
 * @brief   gpio function header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  he_yl
 * @date    2012-12-27
 * @version 1.0
 * 1. ����mmiϵͳ���Ѷ����˵�gpio������Ҫɾ����ش��룬ֻ�轫�䶨��ΪINVALID_GPIO
*******************************************************************************/
#ifndef __BOARDCFG_SPT1050L_H__
#define __BOARDCFG_SPT1050L_H__


/******************************************************************************/
/*   ģ�������صİ弶��������                                               */
/*                                                                            */
/******************************************************************************/
#define ADVAL_MAX                   (ADVAL_KEY8+OFFSET)
#define ADVAL_MIN                   (ADVAL_KEY1-OFFSET)
#define ADKEY_NUMBER                (8)
#define OFFSET                      (30)    //mv

#define ADVAL_KEY1                  (30)    //(kbFUNC)
#define ADVAL_KEY2                  (232)   //(kbMODE)
#define ADVAL_KEY3                  (441)   //(kbRECORD)
#define ADVAL_KEY4                  (660)   //(kbVOLADD)
#define ADVAL_KEY5                  (866)   //(kbVOLSUB)
#define ADVAL_KEY6                  (1066)  //(kbRIGHT)
#define ADVAL_KEY7                  (1272)  //(kbOK)
#define ADVAL_KEY8                  (1485)  //(kbLEFT)


/******************************************************************************/
/*   GPIOʹ������İ弶����                                                   */
/*                                                                            */
/******************************************************************************/
#define GPIO_I2C_SCLK               16
#define GPIO_I2C_SDA                12

#define GPIO_CAMERA_RESET           13
#define GPIO_CAMERA_CHIP_ENABLE     4
#define GPIO_CAMERA_HIT_LED         INVALID_GPIO

#define GPIO_HP_DET                 22
#define GPIO_SPEAKER_CONTROL        18

#define GPIO_SD_DET                 23

#define GPIO_USB_DET                19

#define GPIO_SIMU_UART              INVALID_GPIO


/******************************************************************************/
/*   �弶�������ã���SDMMC�ӿڵ�ѡ��                                          */
/*                                                                            */
/******************************************************************************/
#define SD_INTERFACE_TYPE           INTERFACE_SD2


#endif //#ifndef __BOARDCFG_SPT1050L_H__

