/*******************************************************************************
 * @file    usb_int.c
 * @brief   usb interrupt handler.
 * This file describe udriver of usb in host mode.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-20
 * @version 1.0
 * @ref     
*******************************************************************************/
#include "anyka_types.h"
#include "arch_init.h"
#include "drv_cfg.h"

extern T_VOID usb_slave_intr_handler(T_VOID);
extern T_VOID usb_host_intr_handler(T_VOID);


#pragma arm section zidata = "_drvbootbss_"
T_U8 g_usbtype;    //usb type, 0: slave, 1: host
#pragma arm section zidata


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   usb interrupt handler
 * @author  liao_zhijun
 * @date    2011-08-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_intr_handler(T_VOID)
{
    if(0 == g_usbtype)
    {
        //usb type is slave
        usb_slave_intr_handler();
    }
    else if(1 == g_usbtype)
    {
#if DRV_SUPPORT_UHOST > 0
        //usb type is host
        usb_host_intr_handler();
#endif
    }
    else
    {
        //error
        //drv_print("error usb int", 0, 1);
        while(1);
    }
}
#pragma arm section code


