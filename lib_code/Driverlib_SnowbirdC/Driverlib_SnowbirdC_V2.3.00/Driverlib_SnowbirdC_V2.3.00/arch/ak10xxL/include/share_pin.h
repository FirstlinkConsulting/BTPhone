/*******************************************************************************
 * @file    share_pin.h
 * @brief   share pin configuration function header file.
 * Copyright (C) 2012 Anyka (GuangZhou) Technology Co., Ltd.
 * @author  ZGX
 * @date    2012-11-5
 * @version 1.0
 * @ref     chip AK1080L technical manual.
*******************************************************************************/
#ifndef __SHARE_PIN_H__
#define __SHARE_PIN_H__


#include "anyka_types.h"
#include "drv_cfg.h"

#if (CHIP_SEL_10C > 0)

typedef enum
{
    ePIN_AS_NANDFLASH,
    ePIN_AS_MCI1,
    ePIN_AS_MCI2_1LINE,
    ePIN_AS_MCI2_4LINE,
    ePIN_AS_SPI1,
    ePIN_AS_SPI1_4LINE,
    ePIN_AS_SPI2,
    ePIN_AS_SPI2_4LINE,
    ePIN_AS_LCD,
    ePIN_AS_PWM,
    ePIN_AS_PWM2,
    ePIN_AS_UART1,
    ePIN_AS_UART1_HD,
    ePIN_AS_UART2,
    ePIN_AS_CAMERA,
    ePIN_AS_I2S,
    ePIN_AS_PCM,
    ePIN_AS_IRDA,
    ePIN_AS_GPIO            //last one must <= 31        
} T_eSHARE_PIN_CFG;


#else

typedef enum
{
    ePIN_AS_NANDFLASH,
    ePIN_AS_MCI1,
    ePIN_AS_MCI1_8LINE,
    ePIN_AS_SPI1,
    ePIN_AS_SPI1_4LINE,
    ePIN_AS_SPI2,
    ePIN_AS_SPI2_4LINE,
    ePIN_AS_LCD,
    ePIN_AS_PWM,
    ePIN_AS_UART1,
    ePIN_AS_UART1_HD,
    ePIN_AS_CAMERA,
    ePIN_AS_MCI2,
    ePIN_AS_I2S,
    ePIN_AS_PCM,
    ePIN_AS_GPIO            //last one must <= 31
} T_eSHARE_PIN_CFG;

#endif


typedef struct
{
    T_U8 m_gpio_begin;
    T_U8 m_gpio_end;
    T_U8 m_bits_base;
    T_U8 m_bits_num;
} T_SHARE_PIN_GPIO;



/*******************************************************************************
 * @brief   set share pin group
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_VOID
 * @remark  pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_VOID sys_share_pin_lock(T_eSHARE_PIN_CFG pin_cfg);


/*******************************************************************************
 * @brief   release share pin group
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_VOID
 * @remark  pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_VOID sys_share_pin_unlock(T_eSHARE_PIN_CFG pin_cfg);


/*******************************************************************************
 * @brief   check share pin group has been set
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_BOOL
 * @retval  AK_TRUE: set, AK_FALSE, not set
 * @remark  this function is not recommended to use.
 *          pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_BOOL sys_share_pin_is_lock(T_eSHARE_PIN_CFG pin_cfg);


/*******************************************************************************
 * @brief   set share pin as gpio
 * @brief   for gpio share-pin 's different setting
 * @author  zhanggaoxin
 * @date    2012-12-12
 * @param   [in]pin gpio pin num
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_as_gpio(T_U32 pin);


#endif  /* end of __SHARE_PIN_H__ */

