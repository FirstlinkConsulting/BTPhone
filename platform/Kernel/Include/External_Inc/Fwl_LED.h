#ifndef __FWL_LED_CTRL_H__
#define __FWL_LED_CTRL_H__


#define LED_RED     (1 << 0)
#define LED_BLUE    (1 << 1)

T_VOID Fwl_LEDInit(T_VOID);
T_VOID Fwl_LEDOn(T_U8 color);
T_VOID Fwl_LEDOff(T_U8 color);
#endif
