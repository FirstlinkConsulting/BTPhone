/*******************************************************************************
 * @file    arch_gpio.h
 * @brief   gpio function header file
 * This file provides gpio APIs: initialization, set gpio, get gpio,
 * gpio interrupt handler.
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-11-07
 * @version 1.0
 * @ref     AK1180 technical manual.
*******************************************************************************/
#ifndef __ARCH_GPIO_H__
#define __ARCH_GPIO_H__


#include "anyka_types.h"


#define LEVEL_LOW               0
#define LEVEL_HIGH              1

#define GPIO_LEVEL_LOW          0
#define GPIO_LEVEL_HIGH         1

#define GPIO_DIR_INPUT          0
#define GPIO_DIR_OUTPUT         1

#define GPIO_INT_DISABLE        0
#define GPIO_INT_ENABLE         1

#define GPIO_NUMBER             56

#define USB_DETECT_GPIO         19
#define POWERKEY_GPIO           20
#define GPIO_AIN1_INT           21
#define GPIO_AIN0_INT           24
#define HP_DETECT_GPIO          42  //由于AC模式的耳机可以使用独立的引脚HPVMID
                                    //...监测,而这个引脚的中断是发送到模拟中断;
                                    //...又由于DETECTOR模块仅支持GPIO和ADC两种
                                    //...方式,现将AC模式的耳机检测也集成到
                                    //...DETECTOR模块,故需要将其假扮成GPIO,在
                                    //...其中断处主动调用一次DETECTOR的GPIO中断
                                    //...处理函数即可.在此处将检测引脚其定义成
                                    //...一个保留的GPIO.

#define INVALID_GPIO            0xff


/*******************************************************************************
 * @brief   set gpio output level
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]level: 0 or 1.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_level(T_U32 gpio_num, T_U8 level);


/*******************************************************************************
 * @brief   get gpio input level
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @return  T_U8
 * @retval  pin level; 1: high; 0: low;
*******************************************************************************/
T_U8 gpio_get_pin_level(T_U32 gpio_num);


/*******************************************************************************
 * @brief   set gpio direction
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]dir: 0 means input; 1 means output;
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_dir(T_U32 gpio_num, T_U8 dir);


/*******************************************************************************
 * @brief   set gpio interrupt polarity.
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]polarity: 1 means active high interrupt. 
 *                        0 means active low interrupt.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_int_polarity(T_U32 gpio_num, T_U8 polarity);


/*******************************************************************************
 * @brief   gpio interrupt control
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]enable: AK_TRUE means enable interrupt.
 *                      AK_FALSE means disable interrupt.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_int_enable(T_U32 gpio_num, T_BOOL enable);


/*******************************************************************************
 * @brief   set gpio pull-up or pull-down connection(connect to pull up... 
 *          ...or pull down resistance).
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]mode: AK_FALSE means do not connect.
 *                    AK_TRUE means connect.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pullup_pulldown(T_U32 gpio_num, T_BOOL mode);


/*******************************************************************************
 * @brief   enable or disable wakeup gpio 
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in] gpio_num
 * @param   [in] enable: AK_TRUE:enable wakeup; AK_FALSE:disable wakeup
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_wakeup_enable(T_U32 gpio_num, T_BOOL enable);


/*******************************************************************************
 * @brief   set wakeup gpio polarity
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in] gpio_num
 * @param   [in] polarity, 0:falling triggered; 1:rising triggered
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_wakeup_polarity(T_U32 gpio_num, T_BOOL polarity);


#endif //__ARCH_GPIO_H__

