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
	return AK_FALSE;
}


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
    return AK_FALSE;
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
    return AK_FALSE;
}

/* end of files */

