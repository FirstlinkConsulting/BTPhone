#ifndef __FWL_LED_CTRL_H__
#define __FWL_LED_CTRL_H__


#define LED_RED     (1 << 0)
#define LED_BLUE    (1 << 1)

/*******************************************************************************
 * @brief   initialize LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_VOID Fwl_LEDInit(T_VOID);

/*******************************************************************************
 * @brief   turn on LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]color:
 * @return  T_VOID
 * @retval  1: exit success; 0:exit failed
*******************************************************************************/
T_VOID Fwl_LEDOn(T_U8 color);

/*******************************************************************************
 * @brief   turn off LED
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]color:
 * @return  T_BOOL
 * @retval  1: exit success; 0:exit failed
*******************************************************************************/
T_VOID Fwl_LEDOff(T_U8 color);
#endif
