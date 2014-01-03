/*******************************************************************************
 * @file    Fwl_usb_s_state.c
 * @brief   provde usb common operations.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
*******************************************************************************/
#include "Fwl_usb_s_state.h"
#include "hal_usb_s_state.h"


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * @brief   get usb slave stage.
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param   T_VOID
 * @return  T_U8
 * @retval  USB_OK usb config ok,can transmit data
 * @retval  USB_ERROR usb error
 * @retval  USB_SUSPEND usb suspend by pc,can't to use
 * @retval  USB_NOTUSE usb close
 * @retval  USB_CONFIG usb config by pc
*******************************************************************************/
T_U8 Fwl_UsbSlaveGetState(T_VOID)
{
    return AK_FALSE;
}


/*******************************************************************************
 * @brief   detect whether usb cable is inserted or not
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE usb cable in
 * @retval  AK_FALSE usb cable not in
*******************************************************************************/
T_BOOL Fwl_UsbSlaveDetect(T_VOID)
{
    return AK_FALSE;
}


#ifdef __cplusplus
}
#endif

