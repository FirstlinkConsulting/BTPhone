/**@file hal_usb_s_disk.h
 * @brief provide operations of how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __HAL_USB_S_DISK_H__
#define __HAL_USB_S_DISK_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_disk USB_disk
 *@ingroup USB
 */
/*@{*/

typedef T_BOOL  (*F_AccessLun)(T_U8 *buf, T_U32 BlkStart, T_U32 BlkCnt, T_U32 LunAddInfo);
typedef T_BOOL (*T_fMBOOT_HANDLE_CMD)(T_U8* scsi_data, T_U32 data_len);
typedef T_BOOL (*T_fMBOOT_HANDLE_SEND)(T_U32 buf, T_U32 count);
typedef T_BOOL (*T_fMBOOT_HANDLE_RCV)(T_U32 buf, T_U32 count);


#define INQUIRY_STR_VENDOR_SIZE     8
#define INQUIRY_STR_PRODUCT_SIZE    16
#define INQUIRY_STR_REVISION_SIZE   4


typedef enum
{
    STR_MANUFACTURER = 0,
    STR_PRODUCT
}T_eSTR_DESC;


enum SENSESTATUS
{
    MEDIUM_NOSENSE = 0,
    MEDIUM_NOTPRESENT,
    MEDIUM_NOTREADY,
    MEDIUM_NOTREADY_TO_READY 
};
typedef enum SENSESTATUS E_SENSESTATUS;

typedef struct
{
    F_AccessLun Read;///<logic unit nunber read function
    F_AccessLun Write;///<lun write function
    T_U32 BlkCnt;///<blk cnt of this lun
    T_U32 BlkSize;///<blk size of this lun
    T_U32 LunIdx;///<index of this lun
    T_U32 LunAddInfo;///<the lun information which we want to tranfer in F_AccessLun para 4
    T_U8 FastBlkCnt;///<how many block is fast for this medium
    E_SENSESTATUS sense;///<sense of this lun
    T_U8 Vendor[INQUIRY_STR_VENDOR_SIZE];
    T_U8 Product[INQUIRY_STR_PRODUCT_SIZE];
    T_U8 Revision[INQUIRY_STR_REVISION_SIZE];
}T_LUN_INFO,*T_pLUN_INFO;

/**
 * @brief   mass boot init
 *
 * udisk reset,configok,this func must be called because mass boot at usb1.1 will not run enum 
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb mode 1.1 or 2.0
 * @return T_BOOL
 * @retval AK_FALSE init failed
 * @retval AK_TRUE init successful
 */
T_VOID usbdisk_mboot_init(T_U32 mode);
/** 
 * @brief set procduce callback 
 *
 * used by produce
 * @author Huang Xin
 * @date 2010-08-04
 * @param hnd_cmd[in] handle cmd callback
 * @param hnd_send[in] handle send callback
 * @paramhnd_rcv[in] handle receive callback
 * @return  T_BOOL
 * @retval AK_FALSE set failed
 * @retval AK_TRUE set successful
 */
T_BOOL usbdisk_mboot_set_cb(T_fMBOOT_HANDLE_CMD hnd_cmd, T_fMBOOT_HANDLE_SEND hnd_send, T_fMBOOT_HANDLE_RCV hnd_rcv);

/** 
 * @brief init  str desc with  reference to device desc
 *
 * the str is truncated to 10  characters or less,this func may be called before usbdisk_start
 * @author Huang Xin
 * @date 2010-08-04
 * @param index[in] type of string descriptor
 * @param str[in] the start address of stirng
 * @return  T_BOOL
 * @retval AK_FALSE set failed
 * @retval AK_TRUE set successful
 */
T_BOOL usbdisk_set_str_desc(T_eSTR_DESC index,T_CHR *str);

/**
 * @brief   init usb disk function
 *
 * Initialize usb disk buffer,creat HISR,creat usb disk task,creat message queue,creat event group
 * @author Huang Xin
 * @date 2010-06-30
 * @param mode[in] usb mode 1.1 or 2.0
 * @return T_BOOL
 * @retval AK_FALSE init failed
 * @retval AK_TRUE init successful
 */

T_BOOL usbdisk_init(T_U32 mode);


/**
 * @brief   start usb disk function, this function must be call after usbdisk_init
 *
 * Allocate L2 buffer , open usb controller,set usb disk callback,register interrupt process function
 * @author Huang Xin
 * @date  2010-06-30
 * @param T_VOID
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_start(T_VOID);

/**
 * @brief  enter udisk task,poll udisk msg
 *
 * This function is added for  bios version,and must be call after usbdisk_start
 * @author Huang Xin
 * @date  2010-08-04
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE just received a scsi command
 * @retval AK_FALSE currently no scsi
 */
T_BOOL usbdisk_proc(T_VOID);

/**
 * @brief   disable usb disk function.
 *
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author Huang Xin
 * @date 2010-06-30
 * @param T_VOID
 * @return  T_VOID
 */    
T_VOID usbdisk_stop(T_VOID);

/**
 * @brief   usb slave bulk disk add a lun
 *
 * This function is called when host is  mounting usb disk
 * @author Huang Xin
 * @date 2010-06-30
 * @param pAddLun[in] struct of lun information.
 * @return T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_addLUN(T_LUN_INFO *pAddLun);

/**
 * @brief   usb slave bulk disk change lun
 *
 * When sd card is detected, change the lun for sd card
 * @author Huang Xin
 * @date 2010-06-30
 * @param pChgLun[in] struct of lun information.
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_changeLUN(T_LUN_INFO *pChgLun);


 
typedef T_U32 (*T_fUSB_SCSI_EXT_CALLBACK)(T_U8 cmd,T_U8 *param,T_U32 szParam,T_U8 Stage);

/**
 * @brief   set extern SCSI command callback
 *
 * @author  liaozhijun
 * @date    2013-08-19
 * @param   T_VOID
 * @return  T_VOID
 */

T_VOID usb_set_ExtCmdCallbak(T_fUSB_SCSI_EXT_CALLBACK cmd_proc);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif    //__HAL_USB_S_DISK_H__
