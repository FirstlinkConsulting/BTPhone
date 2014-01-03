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
#include "Fwl_Timer.h"
#include "Eng_Debug.h"
#include "vme.h"
#include "M_event.h"
#include "Fwl_Detect.h"
#include "Fwl_Keypad.h"


#ifdef __cplusplus
extern "C" {
#endif


extern volatile T_BOOL s_usb_conn_pc;


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
    return usb_slave_get_state();
}


/*******************************************************************************
 * @brief   judge exit usb slave mode or not.
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE exit usb slave mode
 * @retval  AK_FALSE don't exit
*******************************************************************************/
T_BOOL Fwl_UsbSlaveIsExit(T_VOID)
{
    if (kbOK == Fwl_KeypadScan())
    {
        AK_DEBUG_OUTPUT("key out usb disk.\n");
        return AK_TRUE;
    }

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
    s_usb_conn_pc = usb_slave_detect();

    return s_usb_conn_pc;
}


/*******************************************************************************
 * @brief   Fwl_UsbOutHandler
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_UsbOutHandler(T_VOID)
{
    Fwl_DelayUs(300000);   //delay 300ms for next detect

    s_usb_conn_pc = AK_FALSE;
}

/*******************************************************************************
 * @brief   Fwl_UsbSlaveMsgDeal
 * @param   T_BOOL bStatus
 * @return  T_VOID
*******************************************************************************/

T_VOID Fwl_UsbSlaveMsgDeal(T_BOOL bStatus)
{
    if(bStatus)
    {
        s_usb_conn_pc = AK_TRUE;
        VME_EvtQueuePut(M_EVT_USB_IN, AK_NULL);

        AK_DEBUG_OUTPUT("USB IN\n");
    }
    else
    {
        Fwl_DetectorEnable(DEVICE_UHOST, AK_TRUE);
    }
}

#ifdef __cplusplus
}
#endif

