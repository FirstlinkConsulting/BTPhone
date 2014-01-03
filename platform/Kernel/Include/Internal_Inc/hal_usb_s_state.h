/**@file hal_usb_s_state.h
 * @brief provde usb common operations.
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_SLAVE_STATE_H__
#define __HAL_USB_SLAVE_STATE_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_state USB_state group
 *  @ingroup USB
 */
/*@{*/

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

#define USB_DISK                (1<<0)   ///<usb disk type
#define USB_CAMERA              (1<<1)   ///<usb camera type
#define USB_ANYKA               (1<<2)   ///<usb anyka type
#define USB_DEBUG               (1<<3)   ///<usb debug type
#define USB_CDC                 (1<<4)   //usb cdc type
#define USB_UVC                 (1<<5)   //usb cdc type

#define USB_MODE_20             (1<<8)   ///<usb high speed
#define USB_MODE_11             (1<<9)   ///<usb full speed

/**
 * @brief   get usb slave stage.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  T_U8
 * @retval  USB_OK usb config ok,can transmit data
 * @retval  USB_ERROR usb error
 * @retval  USB_SUSPEND usb suspend by pc,can't to use
 * @retval  USB_NOTUSE usb close
 * @retval  USB_CONFIG usb config by pc
 */
T_U8 usb_slave_get_state(T_VOID);

/**
 * @brief   set usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  state [in] T_U8.
 * @return  T_VOID
 */
T_VOID usb_slave_set_state(T_U8 stage);

/**
 * @brief   usb interrupt handler
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_slave_intr_handler(T_VOID);


/**
 * @brief   detect whether usb cable connect PC or not
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @return  T_BOOL
 * @retval  AK_TRUE usb cable in
 * @retval  AK_FALSE usb cable not in
 * @note    两者的区别在于usb_slave_detect直接返回结果，耗时较长；而
 *          usb_slave_detect_start只是启动检测，检测结果则通过注册的
 *          回调或者发送消息传回。
 */
T_BOOL usb_slave_detect(T_VOID);

T_VOID usb_slave_detect_start(T_pVOID callback);


/**
 * @brief  usb interrupt handler
 *
 * @author  liao_zhijun
 * @date    2011-08-08
 * @return  T_VOID
 */
T_VOID usb_intr_handler(T_VOID);

/**
 * @brief  get usb mode, used by burntool
 *
 * @author  liao_zhijun
 * @date    2013-03-06
 * @return  T_U32, USB_MODE_20 or USB_MODE_11
 */
T_U32 usb_slave_get_mode(T_VOID);
//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif    //__HAL_USB_SLAVE_STATE_H__
