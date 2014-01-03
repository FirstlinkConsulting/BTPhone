/*******************************************************************************
 * @file    Fwl_usb_s_state.h
 * @brief   provde usb common operations.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_USB_S_STATE_H__
#define __FWL_USB_S_STATE_H__


#include "anyka_types.h"


#ifdef __cplusplus
extern "C" {
#endif


//********************************************************************
#define USB_NOTUSE      0   ///<usb close
#define USB_OK          1   ///<usb config ok,can transmit data
#define USB_ERROR       2   ///<usb error
#define USB_SUSPEND     3   ///<usb suspend by pc,can't to use
#define USB_CONFIG      4   ///<usb config by pc
#define USB_BULKIN      5   
#define USB_BULKOUT     6
#define USB_UPDATE      7
#define USB_START_STOP  8
//********************************************************************

#define USB_DISK        (1<<0)   ///<usb disk type
#define USB_CAMERA      (1<<1)   ///<usb camera type
#define USB_ANYKA       (1<<2)   ///<usb anyka type
#define USB_DEBUG       (1<<3)   ///<usb debug type
#define USB_CDC         (1<<4)   ///<usb cdc type
#define USB_UVC         (1<<5)   ///<usb cdc type

#define USB_MODE_20     (1<<8)   ///<usb high speed
#define USB_MODE_11     (1<<9)   ///<usb full speed


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
T_U8 Fwl_UsbSlaveGetState(T_VOID);


/*******************************************************************************
 * @brief   judge exit usb slave mode or not.
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE exit usb slave mode
 * @retval  AK_FALSE don't exit
*******************************************************************************/
T_BOOL Fwl_UsbSlaveIsExit(T_VOID);


/*******************************************************************************
 * @brief   detect whether usb cable is inserted or not
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE usb cable in
 * @retval  AK_FALSE usb cable not in
*******************************************************************************/
T_BOOL Fwl_UsbSlaveDetect(T_VOID);


/*******************************************************************************
 * @brief   Fwl_UsbOutHandler
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_UsbOutHandler(T_VOID);


#ifdef __cplusplus
}
#endif


#endif

