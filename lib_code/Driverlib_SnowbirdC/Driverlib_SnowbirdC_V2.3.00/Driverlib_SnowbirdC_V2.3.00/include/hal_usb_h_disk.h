/**
   @file   hal_usb_h_disk.h
 * @brief  provide usb host api functions.
 *
 * This file describe frameworks of udisk host driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-10
 * @version 1.0
 */

#ifndef __USB_HOST_DISK_H__
#define __USB_HOST_DISK_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup udisk_host udisk_host
 *	@ingroup USB
 */
/*@{*/

 
#define DEV_STRING_BUF_LEN          0x24    
#define LUN_READY(x)                (1<<x)

typedef struct _H_UDISK_LUN_INFO
{
    T_U32       ulBytsPerSec;
    T_U32       ulCapacity;
    T_U8        InquiryStr[DEV_STRING_BUF_LEN + 1];
} T_H_UDISK_LUN_INFO, *T_pH_UDISK_LUN_INFO;


typedef T_VOID (*T_pfUDISK_HOST_CONNECT)(T_U16 lun_ready_flag);
typedef T_VOID (*T_pfUDISK_HOST_DISCONNECT)(T_VOID);
typedef T_VOID (*T_fUHOST_COMMON_INTR_CALLBACK)(T_VOID);

T_VOID udisk_host_connect_send_msg();

/**
 * @brief   init udisk host function
 *
 * Allocate udisk host buffer,init data strcut,register callback,open usb controller and phy.
 * @author Huang Xin
 * @date 2010-07-12
 * @param mode[in] usb mode 1.1 or 2.0
 * @return T_BOOL
 * @retval AK_FALSE init failed
 * @retval AK_TRUE init successful
 */
T_BOOL udisk_host_init(T_U32 mode);

/**
 * @brief   switch usb host speed.
 *
 * @author  
 * @date    2012-10-30
 * @param  mode [in] T_U32  full speed or high speed
 * @return  T_VOID
 */
T_VOID usb_bus_mode_set(T_U32 mode);

/**
 * @brief   get disk all logic unit number
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return T_U8
 * @retval  Total number of logic unit.
 */
T_U8 udisk_host_get_lun_num(T_VOID);

/**
 * @brief   get a logic unit number descriptor
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param   LUN[in] Index of logic unit.
 * @param   disk_info[out]  The information of the lun
 * @return  T_VOID.
 */
T_VOID udisk_host_get_lun_info(T_U32 LUN, T_pH_UDISK_LUN_INFO disk_info);

/**
 * @brief   usb host read sector from logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param data[in] Buffer to store data
 * @param sector[in] Start sector to read
 * @param size[in] Total sector to read
 * @return T_U32
 * @retval Really total sector have been read.
 */
T_U32 udisk_host_read(T_U32 LUN, T_U8 data[], T_U32 sector, T_U32 size);

/**
 * @brief   usb host write sector to logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] Index of logic unit.
 * @param data[in] The write data
 * @param sector[in] Start sector to write
 * @param size[in] Total sectors to write
 * @return T_U32
 * @retval Really total sectors have been wrote.
 */
T_U32 udisk_host_write(T_U32 LUN, T_U8 data[], T_U32 sector, T_U32 size);

/**
 * @brief   Udisk host set application level callback.
 *
 * This function must be called by application level after udisk host initialization.
 * @author Huang Xin
 * @date 2010-07-12
 * @param connect_callback[in] Application level callback
 * @param disconnect_callback[in] Application level callback
 * @return  T_VOID
 */
T_VOID udisk_host_set_callback(T_pfUDISK_HOST_CONNECT connect_callback, T_pfUDISK_HOST_DISCONNECT disconnect_callback);

/**
 * @brief Udisk host close function.
 *
 * This function is called by application level when eject the udisk and exit the udisk host.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  T_VOID
 */
T_VOID udisk_host_close(T_VOID);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif    //__USB_HOST_DISK_H__
