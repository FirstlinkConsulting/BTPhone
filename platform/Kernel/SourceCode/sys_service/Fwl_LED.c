/*******************************************************************************
 * @file    Fwl_LED.c
 * @brief   this file will constraint the access to the bottom layer radio,
 *          avoid resource competition. Also, this file os for porting to
 *          different OS
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "Fwl_LED.h"
#include "Eng_Debug.h"
#include "Arch_gpio.h"
#include "gpio_define.h"

/*******************************************************************************
 * @brief   initialize LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_VOID Fwl_LEDInit(T_VOID)
{
    if (INVALID_GPIO != GPIO_LED_RED)
    {
        gpio_set_pin_dir(GPIO_LED_RED, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_LED_RED, GPIO_LEVEL_LOW);
        gpio_set_pullup_pulldown(GPIO_LED_RED, AK_FALSE);
    }

    if (INVALID_GPIO != GPIO_LED_BLUE)
    {
        gpio_set_pin_dir(GPIO_LED_BLUE, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_LED_BLUE, GPIO_LEVEL_LOW);
        gpio_set_pullup_pulldown(GPIO_LED_BLUE, AK_FALSE);
    }
}
#pragma arm section code = "_frequentcode_"


/*******************************************************************************
 * @brief   turn on LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]color:
 * @return  T_VOID
 * @retval  1: exit success; 0:exit failed
*******************************************************************************/
T_VOID Fwl_LEDOn(T_U8 color)
{
    if (color & LED_RED)
    {
        if (INVALID_GPIO != GPIO_LED_RED)
        {
            gpio_set_pin_level(GPIO_LED_RED, GPIO_LEVEL_HIGH);
        }
    }

    if (color & LED_BLUE)
    {
        if (INVALID_GPIO != GPIO_LED_BLUE)
        {
            gpio_set_pin_level(GPIO_LED_BLUE, GPIO_LEVEL_HIGH);
        }
    }
}


/*******************************************************************************
 * @brief   turn off LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]color:
 * @return  T_BOOL
 * @retval  1: exit success; 0:exit failed
*******************************************************************************/
T_VOID Fwl_LEDOff(T_U8 color)
{
    if (color & LED_RED)
    {
        if (INVALID_GPIO != GPIO_LED_RED)
        {
            gpio_set_pin_level(GPIO_LED_RED, GPIO_LEVEL_LOW);
        }
    }

    if (color & LED_BLUE)
    {
        if (INVALID_GPIO != GPIO_LED_BLUE)
        {
            gpio_set_pin_level(GPIO_LED_BLUE, GPIO_LEVEL_LOW);
        }
    }
}
#pragma arm section code
/* end of files */

