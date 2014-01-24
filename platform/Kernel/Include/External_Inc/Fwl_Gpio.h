
#ifndef __FWL_GPIO_H__
#define __FWL_GPIO_H__


#include "akdefine.h"


typedef T_VOID (*T_fGPIO_CB)(T_U8 *add, T_U32 cnt);


T_BOOL Fwl_GpioInit(T_U32 baudrate);

T_BOOL Fwl_GpioFree(T_VOID);

T_BOOL Fwl_GpioSetBaudrate(T_U32 baudrate);

T_BOOL Fwl_GpioWriteData(T_U8 *data, T_U32 len);

T_BOOL Fwl_GpioGetTxStatus(T_VOID);

T_BOOL Fwl_GpioSetIntStatus(T_BOOL status);

T_VOID Fwl_GpioSetCallback(T_fGPIO_CB callback_func);

#endif

