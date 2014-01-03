/*******************************************************************************
 * @file    Fwl_usb_host_mount.h
 * @brief   implementation for mount sd card and usbhost disk etc.
 * This file describe frameworks of fs mount operation.
 * Copyright (C) 2011 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  
 * @date    2011-7-25
 * @version 1.0
 * @ref     
*******************************************************************************/
#ifndef __FWL_USB_HOST_MOUNT_H__
#define __FWL_USB_HOST_MOUNT_H__


#include "anyka_types.h"


/*******************************************************************************
 * @brief   mount usb disk .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success return AK_TRUE,fail return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_MountUSBDisk(T_VOID);


/*******************************************************************************
 * @brief   unmount usb disk .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success return AK_TRUE,fail return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_UnMountUSBDisk(T_VOID);


/*******************************************************************************
 * @brief   judge usb disk is available
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  yes return AK_TRUE,no return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_UsbDiskIsMnt(T_VOID);


const T_U16 *Fwl_GetCurUsbDiskPath(T_U8 index, T_U8 *DrvCnt);


T_BOOL Fwl_GetCurUsbDiskID(T_U16 pathname);


#endif //__FWL_USB_HOST_MOUNT_H__


