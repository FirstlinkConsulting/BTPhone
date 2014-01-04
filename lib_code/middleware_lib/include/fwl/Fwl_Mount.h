/*******************************************************************************
 * @file    Fwl_Mount.h
 * @brief   implementation for mount sd card and usbhost disk etc.
 * This file describe frameworks of fs mount operation.
 * Copyright (C) 2011 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  lixingjian
 * @date    2011-7-25
 * @version 1.0
 * @ref
*******************************************************************************/
#ifndef __FWL_MOUNT_H__
#define __FWL_MOUNT_H__


#include "anyka_types.h"


#define SD_USB_RW_ERR_LIMIT             4

typedef enum
{
    SYSTEM_STORAGE = 0, //系统启动盘可以是Nand或sd
    MMC_SD_CARD,        //MMC SD卡
    USB_HOST_DISK,      //U盘
    MEMDEV_MAX_NUM
} T_MEM_DEV_ID;

#ifdef SUPPORT_SDCARD
typedef struct
{
    T_BOOL bSD_USB_RW_ERR:1;
    T_U8   reserve:7;
    T_U8   SD_RW_ERR_CNT;
}SD_USB_RW_INFO, *P_SD_USB_RW_INFO;

extern SD_USB_RW_INFO g_sd_usb_rw_info;
#endif


/*******************************************************************************
 * Brief：Mount the MemDev driver with DrvIndx.
 * Param: T_MEM_DEV_ID MemId - T_MEM_DEV_ID;
 * Ret  : AK_TRUE : Mount successfully.
 *        AK_FALSE: Mount failed!
*******************************************************************************/
T_BOOL Fwl_MemDevMount (T_MEM_DEV_ID MemId);


/*******************************************************************************
 * Brief：Unmount theMemDev with DrvIndx.
 * Param: T_MEM_DEV_ID MemId - T_MEM_DEV_ID;
 * Ret  : AK_TRUE : Unmount successfully.
 *        AK_FALSE: Unmount failed!
*******************************************************************************/
T_BOOL Fwl_MemDevUnMount (T_MEM_DEV_ID MemId);


/*******************************************************************************
 * Brief：check the driver is or not mount.
 * Param: T_MEM_DEV_ID MemId - T_MEM_DEV_ID;
 * Ret  : AK_TRUE : mount successfully.
 *        AK_FALSE: mount failed!
*******************************************************************************/
T_BOOL Fwl_MemDevIsMount(T_MEM_DEV_ID MemId);


/*******************************************************************************
 * Brief：get  the driver  id with the pathname
 * Param: T_U16 pathname -- the driver path name;
 * Ret  : T_MEM_DEV_ID : driver ID.
*******************************************************************************/
T_MEM_DEV_ID Fwl_MemDevGetDriver (T_U16 pathname);


/*******************************************************************************
 * Brief：get   the pathname with the driver  id 
 * Param: T_MEM_DEV_ID MemId, 
 * Param: T_U8 *DrvCnt;
 * Ret  : T_U16*  : pathname.
*******************************************************************************/
const T_U16*  Fwl_MemDevGetPath (T_MEM_DEV_ID MemId, T_U8 *DrvCnt);


#endif //__FWL_MOUNT_H__


