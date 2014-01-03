/*******************************************************************************
 * @file    Fwl_usb_s_disk.h
 * @brief   provide operations of how to use usb disk.
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-06-30
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_USB_S_DISK_H__
#define __FWL_USB_S_DISK_H__


#include "anyka_types.h"
#include "Fwl_Mount.h"


typedef enum {   
    NO_MEDIA = 0 ,
    NANDFLASH_DISK,
    NANDRESERVE_ZONE = 10,  //to store *.bin 
    SDCARD_DISK = 12   
}T_ACCESS_MEDIUM;

typedef enum
{
    STR_MANUFACTURER_EX = 0,
    STR_PRODUCT_EX
}T_eSTR_DESC_EX;


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * @brief   init usb disk function
 * Initialize usb disk buffer,creat HISR,creat usb disk task,creat message queue,creat event group
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   [in]mode: usb mode 1.1 or 2.0
 * @return  T_BOOL
 * @retval  AK_FALSE init failed
 * @retval  AK_TURE init successful
*******************************************************************************/
T_BOOL Fwl_UsbDiskInit(T_U32 mode);


/*******************************************************************************
 * @brief   init str desc with  reference to device desc
 * the str is truncated to 10  characters or less,this func may be called before usbdisk_start
 * @author  Huang Xin
 * @date    2010-08-04
 * @param   [in]index: type of string descriptor
 * @param   [in]str: the start address of stirng
 * @return  T_BOOL
 * @retval  AK_FALSE set failed
 * @retval  AK_TURE set successful
*******************************************************************************/
T_BOOL Fwl_UsbDiskSetStrDesc(T_eSTR_DESC_EX index,T_CHR *str);


/*******************************************************************************
 * @brief   start usb disk function, this function must be call after usbdisk_init
 * Allocate L2 buffer, open usb controller,set usb disk callback,register interrupt process function
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
*******************************************************************************/
T_BOOL Fwl_UsbDiskStart(T_VOID);


/*******************************************************************************
 * @brief   enter udisk task,poll udisk msg
 * This function is added for  bios version,and must be call after usbdisk_start
 * @author  Huang Xin
 * @date    2010-08-04
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE just received a scsi command
 * @retval  AK_FALSE currently no scsi
*******************************************************************************/
T_BOOL Fwl_UsbDiskProc(T_VOID);


/*******************************************************************************
 * @brief   disable usb disk function.
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_UsbDiskStop(T_VOID);


/*******************************************************************************
 * @brief   usb slave disk add a lun
 * This function is called when host is  mounting usb disk
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   [in]pAddLun: struct of lun information.
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
*******************************************************************************/
T_BOOL Fwl_UsbDiskAddLun(T_ACCESS_MEDIUM disk_type);


/*******************************************************************************
 * @brief   usb slave bulk disk change lun
 * When sd card is detected, change the lun for sd card
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   [in]pChgLun: struct of lun information.
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
*******************************************************************************/
T_BOOL Fwl_UsbDiskChgLun(T_MEM_DEV_ID index);

T_VOID usbFlushCacheForTranSpd(T_VOID);
T_VOID usbFreeCacheForTranSpd(T_VOID);


#ifdef __cplusplus
}
#endif


#endif

