/*******************************************************************************
 * @file    gpio.c
 * @brief   gpio function file
 * This file provides gpio APIs: initialization, set gpio, get gpio,
 * gpio interrupt handler.
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-12-20
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "gpio.h"
#include "arch_gpio.h"
#include "interrupt.h"
#include "arch_timer.h"
#include "arch_pmu.h"
#include "drv_cfg.h"



/*******************************************************************************
 * @brief   change gpio to wakeup gpio
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in] gpio_num: common gpio number
 * @return  T_U8
 * @retval  wakeup gpio number
*******************************************************************************/
static T_U8 gpio_wakeup_remap(T_U32 gpio_num)
{
    if ((gpio_num >= 51) && (gpio_num <= 54))
        return (gpio_num - 28);
    else if ((gpio_num >= 25) && (gpio_num <= 29))
        return (gpio_num - 9);
    else if (gpio_num >= 22 && (gpio_num <= 23))
        return (gpio_num - 8);
    else if ((gpio_num >= 15)&&(gpio_num <= 16))
        return (gpio_num - 5);
    else if ((gpio_num >= 10)&&(gpio_num <= 13))
        return (gpio_num - 4);
    else if ((gpio_num >= 5)&&(gpio_num <= 7))
        return (gpio_num - 2);
    else if (gpio_num <= 2)
        return gpio_num;
    else
    {
        if (gpio_num == 47)
            return 22;
        else if (gpio_num == 43)
            return 21;
        else if (gpio_num == 18)
            return 13;
        else
            return INVALID_GPIO;
    }
}

/*******************************************************************************
 * @brief   enable or disable wakeup gpio 
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in] gpio_num
 * @param   [in] enable: AK_TRUE:enable wakeup; AK_FALSE:disable wakeup
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_wakeup_enable(T_U32 gpio_num, T_BOOL enable)
{
    T_U8 pin;

    gpio_set_pin_as_gpio(gpio_num);

    pin = gpio_wakeup_remap(gpio_num);
    if (AK_FALSE == enable)
    {
        REG32(REG_WGPIO_EN) &= ~(1 << pin);
    }
    else
    {
        REG32(REG_WGPIO_EN) |= (1 << pin);
    }
}

/*******************************************************************************
 * @brief   set wakeup gpio polarity
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in] gpio_num
 * @param   [in] polarity, 0:falling triggered; 1:rising triggered
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_wakeup_polarity(T_U32 gpio_num, T_BOOL polarity)
{
    T_U8 pin;

    pin = gpio_wakeup_remap(gpio_num);
    if (LEVEL_LOW == polarity)
    {
        REG32(REG_WGPIO_POL) |= (1 << pin);
    }
    else
    {
        REG32(REG_WGPIO_POL) &= ~(1 << pin);
    }
}

/*******************************************************************************
 * @brief   change wakeup gpio to common gpio
 * @author  zhanggaoxin
 * @date    2012-11-28
 * @param   [in] wakeup_gpio: wakeup gpio number
 * @return  T_U8
 * @retval  common gpio number
*******************************************************************************/
static T_U8 gpio_remap_wakeup(T_U32 wakeup_gpio)
{
    if (wakeup_gpio <= 2)
        return wakeup_gpio;
    else if ((wakeup_gpio >= 3) && (wakeup_gpio <= 5))
        return (wakeup_gpio + 2);
    else if ((wakeup_gpio >= 6) && (wakeup_gpio <= 9))
        return (wakeup_gpio + 4);
    else if ((wakeup_gpio >= 10) && (wakeup_gpio <= 11))
        return (wakeup_gpio + 5);
    else if ((wakeup_gpio >= 14) && (wakeup_gpio <= 15))
        return (wakeup_gpio + 8);
    else if ((wakeup_gpio >= 16) && (wakeup_gpio <= 20))
        return (wakeup_gpio + 9);
    else if ((wakeup_gpio >= 23) && (wakeup_gpio <= 26))
        return (wakeup_gpio + 28);
    else
    {
        if (wakeup_gpio == 13)
            return 18;
        else if (wakeup_gpio == 21)
            return 43;
        else if (wakeup_gpio == 22)
            return 47;
        else
            return INVALID_GPIO;
    }
}

/*******************************************************************************
 * @brief   get which pin wakeup
 * @author  zhanggaoxin
 * @date    2012-11-28
 * @param   T_VOID
 * @return  T_U8
 * @retval  gpio pin
*******************************************************************************/
T_U8 gpio_wakeup_status(T_VOID)
{
    T_U32 i, reason;

    reason = REG32(REG_WGPIO_STA) & REG32(REG_WGPIO_EN);
    REG32(REG_WGPIO_CLR) = 0xffffffff;
    REG32(REG_WGPIO_EN)  = 0;

    if (reason)
    {
        for (i=0; i<=31; i++)
        {
            if (((1<<i) & reason) != 0)
            {
                return gpio_remap_wakeup(i);
            }
        }
    }

    return INVALID_GPIO;
}

#pragma arm section code = "_drvbootcode_" 
/*******************************************************************************
 * @brief   set gpio output level
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]level: 0 or 1.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_level(T_U32 pin, T_U8 level)
{
    if (pin > 31)
    {
        if (GPIO_LEVEL_LOW == level)
        {
            REG32(REG_GPIO_OUT_2) &= ~(1 << (pin - 32));
        }
        else
        {
            REG32(REG_GPIO_OUT_2) |= (1 << (pin - 32));
        }
    }
    else
    {
        if (GPIO_LEVEL_LOW == level)
        {
            REG32(REG_GPIO_OUT_1) &= ~(1 << pin);
        }
        else
        {
            REG32(REG_GPIO_OUT_1) |= (1 << pin);
        }
    }
}

/*******************************************************************************
 * @brief   get gpio input level
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @return  T_U8
 * @retval  pin level; 1: high; 0: low;
*******************************************************************************/
T_U8 gpio_get_pin_level(T_U32 pin)
{
#if !(HP_MODE_DC > 0)
    if (HP_DETECT_GPIO == pin)
    {
        return (REG32(REG_INT_ANALOG) & ALG_STA_HP) ? LEVEL_LOW : LEVEL_HIGH;
    }
#endif
    if (USB_DETECT_GPIO == pin)
    {
        return ((REG32(REG_INT_ANALOG) >> ALG_STA_USB_IN) & 1);
    }

    if ((POWERKEY_GPIO == pin) || (GPIO_AIN0_INT == pin) 
        || (GPIO_AIN1_INT == pin))
    {
        return ((REG32(REG_GPIO_PULL_UD_1) >> pin) & 1);
    }

    if (pin > 31)
    {
        return ((REG32(REG_GPIO_IN_2) >> (pin - 32)) & 1);
    }
    else
    {
        return ((REG32(REG_GPIO_IN_1) >> pin) & 1);
    }
}

/*******************************************************************************
 * @brief   set gpio direction
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]dir: 0 means input; 1 means output;
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_dir(T_U32 pin, T_U8 dir)
{
    gpio_set_pin_as_gpio(pin);

    if (pin > 31)
    {
        if (GPIO_DIR_INPUT == dir)
        {
            REG32(REG_GPIO_DIR_2) |= (1 << (pin - 32));
        }
        else
        {
            REG32(REG_GPIO_DIR_2) &= ~(1 << (pin - 32));
        }
    }
    else
    {
        if (GPIO_DIR_INPUT == dir)
        {
            REG32(REG_GPIO_DIR_1) |= (1 << pin);
        }
        else
        {
            REG32(REG_GPIO_DIR_1) &= ~(1 << pin);
        }
    }
}

/*******************************************************************************
 * @brief   set gpio interrupt polarity.
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]polarity: 1 means active high interrupt. 
 *                        0 means active low interrupt.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_int_polarity(T_U32 pin, T_U8 polarity)
{
    if (pin > 31)
    {
        if (LEVEL_LOW == polarity)
        {
            REG32(REG_GPIO_INT_POL_2) |= (1 << (pin - 32));
        }
        else
        {
            REG32(REG_GPIO_INT_POL_2) &= ~(1 << (pin - 32));
        }
    }
    else
    {
        if (LEVEL_LOW == polarity)
        {
            REG32(REG_GPIO_INT_POL_1) |= (1 << pin);
        }
        else
        {
            REG32(REG_GPIO_INT_POL_1) &= ~(1 << pin);
        }
    }
}

/*******************************************************************************
 * @brief   gpio interrupt control
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]pin: gpio pin ID.
 * @param   [in]enable: AK_TRUE means enable interrupt.
 *                      AK_FALSE means disable interrupt.
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_int_enable(T_U32 pin, T_BOOL enable)
{
#if !(HP_MODE_DC > 0)
    if (HP_DETECT_GPIO == pin)
    {
        if (AK_FALSE == enable)
        {
            REG32(REG_INT_ANALOG) &= ~INT_EN_HP;
            REG32(REG_ANALOG_CTRL4) &= ~HP_PLUGIN_DET_EN;
        }
        else
        {
            REG32(REG_ANALOG_CTRL4) |= HP_PLUGIN_DET_EN;
            REG32(REG_INT_ANALOG) |= INT_EN_HP;
        }
    }
#endif
    if (pin > 31)
    {
        if (AK_FALSE == enable)
        {
            REG32(REG_GPIO_INT_EN_2) &= ~(1 << (pin - 32));
        }
        else
        {
            REG32(REG_GPIO_INT_EN_2) |= (1 << (pin - 32));
        }
    }
    else
    {
        if (AK_FALSE == enable)
        {
            REG32(REG_GPIO_INT_EN_1) &= ~(1 << pin);
        }
        else
        {
            REG32(REG_GPIO_INT_EN_1) |= (1 << pin);
        }
    }
}

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
T_VOID gpio_set_pullup_pulldown(T_U32 pin, T_BOOL mode)
{
    if (pin > 31)
    {
        if (AK_FALSE == mode)
        {
            REG32(REG_GPIO_PULL_UD_2) |= (1 << (pin - 32));
        }
        else
        {
            REG32(REG_GPIO_PULL_UD_2) &= ~(1 << (pin - 32));
        }
    }
    else
    {
        if (AK_FALSE == mode)
        {
            REG32(REG_GPIO_PULL_UD_1) |= (1 << pin);
        }
        else
        {
            REG32(REG_GPIO_PULL_UD_1) &= ~(1 << pin);
        }
    }
}
#pragma arm section code


#pragma arm section code = "_drvbootcode_" 
extern T_VOID gpio_int_detect_chk(T_U32 gpio_num, T_U8 level);
extern T_VOID keypad_scan_start_chk(T_U32 gpio_num, T_U8 level);
/*******************************************************************************
 * @brief   gpio interrupt service routine
 * @author  zhanggaoxin
 * @date    2012-11-29
 * @param   [in]pin: gpio pin id generated interrupt
 * @param   [in]polarity: the polarity of interrupt
 * @return  T_VOID
*******************************************************************************/
static T_VOID gpio_dispatch_int(T_U32 pin, T_U8 polarity)
{
    //长按POWERKEY 4s芯片会自动关机，在此取消芯片关机
    if ((POWERKEY_GPIO == pin) && (LEVEL_HIGH == polarity))
    {
		#if (SWITCH_MODE_SLIDE == 1)
        REG32(REG_PMU_CTRL1) &= ~PMU_HW_PD_GATE_DIS;
        REG32(REG_PMU_CTRL1) |=  PMU_HW_PD_GATE_DIS;
		#endif
    }

    if ((USB_DETECT_GPIO == pin) && (LEVEL_HIGH == polarity))
    {
        pmu_ldo12_sel(CORE12_12V);
    }

#if (DETECT_DEV_MAX >0)
    gpio_int_detect_chk(pin, polarity);
#endif
#if (DRV_SUPPORT_KEYPAD > 0)
    keypad_scan_start_chk(pin, polarity);
#endif

    gpio_int_polarity(pin, (1 - polarity));
    //gpio_int_enable(pin, AK_TRUE);
}

#ifndef BURN_TOOL
/*******************************************************************************
 * @brief   gpio interrupt service routine
 * @author  zhanggaoxin
 * @date    2012-11-29
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID gpioset_interrupt_handler(T_VOID)
{
    T_U32 reg_inte, reg_intp, reg_in;
    T_U32 reg_temp;
    T_U32 i = 0;


    reg_inte = REG32(REG_GPIO_INT_EN_1);
    reg_intp = REG32(REG_GPIO_INT_POL_1);
    reg_in   = REG32(REG_GPIO_IN_1);

    reg_temp = reg_in ^ reg_intp;
    reg_temp = reg_inte & reg_temp;
    reg_temp &= ~((1 << USB_DETECT_GPIO) | ONOFF_STA | AIN1_STA | AIN0_STA);

    if (0 == reg_temp)
    {
        reg_in   = REG32(REG_INT_ANALOG);

        //for align
        reg_in   = reg_in >> (ALG_STA_USB_IN - USB_DETECT_GPIO);
        reg_temp = reg_in ^ reg_intp;
        reg_temp = reg_inte & reg_temp;
        reg_temp &= (1 << USB_DETECT_GPIO);

        if (0 == reg_temp)
        {
            reg_in = REG32(REG_GPIO_PULL_UD_1);

            reg_temp = reg_in ^ reg_intp;
            reg_temp = reg_inte & reg_temp;
            reg_temp &= ONOFF_STA | AIN1_STA | AIN0_STA;

            if (0 == reg_temp)
            {
                reg_inte = REG32(REG_GPIO_INT_EN_2);
                reg_intp = REG32(REG_GPIO_INT_POL_2);
                reg_in   = REG32(REG_GPIO_IN_2);

                reg_temp = reg_in ^ reg_intp;
                reg_temp = reg_inte & reg_temp;

                if (0 == reg_temp)
                {
                    return;
                }

                i = 32;
            }
        }
    }

    if ((reg_temp & 0xffff) == 0)
    {
        reg_temp = reg_temp>>16;
        i += 16;
    }
    if ((reg_temp & 0xff) == 0)
    {
        reg_temp = reg_temp>>8;
        i += 8;
    }
    if ((reg_temp & 0xf) == 0)
    {
        i += 4;
        reg_temp = reg_temp>>4;
    }
    if ((reg_temp & 0x3) == 0)
    {
        i += 2;
        reg_temp = reg_temp>>2;
    }
    i += ((reg_temp&0x03)>>1);

    gpio_dispatch_int(i, (reg_in>>(i&0x1f))&0x01);
}
#endif

#pragma arm section code

T_VOID gpio_init(T_VOID)
{
    INT_ENABLE(INT_EN_SYSCTL);
    INT_ENABLE_SCM(INT_EN_GPIO);
}


