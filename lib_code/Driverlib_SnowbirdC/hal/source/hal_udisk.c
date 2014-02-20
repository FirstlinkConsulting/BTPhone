/**
 * @filename usb_udisk.c
 * @brief:how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-26
 * @version 1.0
 */
#include <stdio.h>
#include "usb_slave_drv.h"
#include "hal_udisk.h"
#include "hal_usb_s_std.h"
#include "hal_udisk_mass.h"
#include "usb_common.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "arch_init.h"
#include "drv_cfg.h"


#if (USLAVE_USE_INTR > 0)

#define UDISK_POLL_STATUS(condition) while((condition))

#else

#define UDISK_POLL_STATUS(condition)    \
    while((condition)){ \
        usb_slave_poll_status(); \
    } \

#endif


extern volatile T_U16 m_ep_status[];

static T_U8 configdata[100];
static T_MBOOT s_MassBoot = {0}; 
#pragma arm section zidata = "_udisk_bss_"
static volatile T_pUDISK_DEV s_pUdisk;
#if !(USB_VAR_MALLOC > 0)
static T_UDISK_DEV s_udisk_dev;
#endif
#if (CHIP_SEL_10C > 0)
T_fUSB_SCSI_EXT_CALLBACK ext_cmd_call;
#endif
#pragma arm section zidata

static T_VOID udisk_scsi_msg_proc(T_U32* pMsg,T_U32 len);
static T_BOOL udisk_enable();
static T_BOOL udisk_disable();
static T_BOOL udisk_class_callback(T_CONTROL_TRANS *pTrans);
static T_BOOL udisk_bo_reset(T_CONTROL_TRANS *pTrans);
static T_BOOL udisk_get_max_lun(T_CONTROL_TRANS *pTrans);
static T_VOID udisk_reset(T_U32 mode);
static T_VOID udisk_suspend();
static T_VOID udisk_resume();
static T_VOID udisk_send_finish();
static T_VOID udisk_receive_notify();
static T_VOID udisk_receive_finish();
static T_VOID udisk_configok();
static T_U8 *udisk_get_dev_qualifier_desc(T_U32 *count);
static T_U8 *udisk_get_dev_desc(T_U32 *count);
static T_U8 *udisk_get_cfg_desc(T_U32 *count);
static T_U8 * udisk_get_other_speed_cfg_desc(T_U32 *count);
static T_U8 *udisk_get_str_desc(T_U8 index, T_U32 *count);
static T_VOID init_serial_number();
static T_VOID  reverse(T_CHR *s); 
static T_VOID itoa(T_U32 n, T_CHR *s);
static T_BOOL udisk_mass_boot_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_read10_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_write10_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_inquiry_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_read_capacity_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_read_format_capacity_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_mode_sense6_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_request_sense_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_test_unit_ready_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_verify_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_prevent_remove_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_start_stop_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_unsupported_cmd_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_BOOL udisk_extend_cmd_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res);
static T_VOID udisk_error_recovery(T_VOID);
static T_BOOL udisk_send_csw_proc(T_U8 csw_status,T_U32 residue);


const static T_USB_DEVICE_DESCRIPTOR   s_UdiskDeviceDesc =
{
    0x12,                       ///<Descriptor size is 18 bytes  
    DEVICE_DESC_TYPE,           ///<device descriptor type
    0x0200,                     ///<USB Specification Release Number in binary-coded decimal
    0x00,                       ///<Each interface specifies its own class information  
    0x00,                       ///<Each interface specifies its own Subclass information  
    0x00,                       ///<No protocols the device basis  
    EP0_MAX_PAK_SIZE,           ///<Maximum packet size for endpoint zero is 64  
    0x04d6,                     ///<Vendor ID 
    0xE101,                     ///<Product ID
    0x0100,                     ///< Device release number in binary-coded
    0x1,                        ///<The manufacturer string descriptor index is 1  
    0x2,                        ///<The product string descriptor index is 2      
#ifndef BURN_TOOL
    0x3,                        ///< The serial number string descriptor index is 3  
#else
    0x0,
#endif
    0x01                        ///<The device has 1 possible configurations  
};
const static T_USB_DEVICE_QUALIFIER_DESCRIPTOR s_UdiskDeviceQualifierDesc=
{
    sizeof(T_USB_DEVICE_QUALIFIER_DESCRIPTOR),      ///<Descriptor size is 10 bytes  
    DEVICE_QUALIFIER_DESC_TYPE,                     ///<DEVICE_QUALIFIER Descriptor Type  
    0x0200,                                         ///<  USB Specification version 2.00  
    0x00,                                           ///<Each interface specifies its own class information  
    0x00,                                           ///<Each interface specifies its own Subclass information  
    0x00,                                           ///<No protocols the device basis 
    0x40,                                           ///<Maximum packet size for endpoint zero is 64  
    0x01,                                           ///<The device has 1 possible other-speed configurations  
    0                                               ///<Reserved for future use  
};


const static T_USB_CONFIGURATION_DESCRIPTOR s_UdiskConfigDesc =
{
    0x9,                        ///<Descriptor size is 9 bytes  
    CONFIG_DESC_TYPE,           ///<CONFIGURATION Descriptor Type  
    UDISK_DESC_TOTAL_LEN,       ///<The total length of data for this configuration is 32. This includes the combined length of all the descriptors
    0x01,                       ///<This configuration supports 1 interfaces 
    0x01,                       ///< The value 1 should be used to select this configuration  
    0x00,                       ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                       ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    200                         ///<Maximum power consumption of the device in this configuration is 400 mA  
};

const static T_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR s_UdiskOtherSpeedConfigDesc=
{
    9,                                      ///<Size of this descriptor in bytes
    OTHER_SPEED_CONFIGURATION_DESC_TYPE,    ///<ohter speed CONFIGURATION Descriptor Type
    UDISK_DESC_TOTAL_LEN,                   ///<The total length of data for this configuration is 32. This includes the combined length of all the descriptors
    0x01,                                   ///<This configuration supports 1 interfaces 
    0x01,                                   ///< The value 1 should be used to select this configuration  
    0x00,                                   ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                                   ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    200                                     ///<Maximum power consumption of the device in this configuration is 400 mA  
};

const static T_USB_INTERFACE_DESCRIPTOR s_UdiskInterfacefDesc =
{
    0x9,                            ///<Descriptor size is 9 bytes  
    IF_DESC_TYPE,                   ///<INTERFACE Descriptor Type  
    0x00,                           ///<The number of this interface is 0.  
    0x00,                           ///<The value used to select the alternate setting for this interface is 0  
    0x02,                           ///The number of endpoints used by this interface is 2 (excluding endpoint zero) 
    USB_DEVICE_CLASS_STORAGE,       ///<The interface implements the Mass Storage class  
    USB_MASSCLASS_COMMAND_SCSI,     ///<The interface implements the SCSI Transparent Subclass  
    USB_MASSCLASS_TRAN_BO,          ///<The interface uses the Bulk-Only Protocol 
    0x00                            ///<The device doesn't have a string descriptor describing this iInterface 
};


static T_USB_ENDPOINT_DESCRIPTOR s_UdiskEp1Desc =
{
    7,                                  ///<Descriptor size is 7 bytes  
    EP_DESC_TYPE,                       ///< ENDPOINT Descriptor Type  
    (ENDPOINT_DIR_OUT + USB_BULK_OUT_INDEX),     ///< This is an OUT endpoint with endpoint number 3 
    ENDPOINT_TYPE_BULK,                 ///< Types - Transfer: BULK Pkt Size Adjust: No  
    EP1_BUF_MAX_LEN,                    ///<Maximum packet size for this endpoint is 512 Bytes. If Hi-Speed, 0 additional transactions per frame  
    0x0A                                ///<The polling interval value is every 0 Frames. If Hi-Speed, 0 uFrames/NAK  
};

static T_USB_ENDPOINT_DESCRIPTOR s_UdiskEp2Desc =
{
    7,                                  ///<Descriptor size is 7 bytes 
    EP_DESC_TYPE,                       ///<ENDPOINT Descriptor Type
    (ENDPOINT_DIR_IN + USB_BULK_IN_INDEX),      ///<This is an IN endpoint with endpoint number 2  
    ENDPOINT_TYPE_BULK,                 ///<  Types - Transfer: BULK Pkt Size Adjust: No  
    EP2_BUF_MAX_LEN,                    ///<Maximum packet size for this endpoint is 512 Bytes. If Hi-Speed, 0 additional transactions per frame
    0x00                                ///< The polling interval value is every 0 Frames. If Hi-Speed, 0 uFrames/NAK  
};


/* language descriptor */
const static T_U8 s_UdiskString0[] = 
{
    4,              ///<Descriptor size is 4 bytes  
    3,              ///< Second Byte of this descriptor
    0x09, 0x04,     ///<Language Id: 1033  
};

/*string descriptor*/
static T_U8 s_UdiskString1[] =
{
    22,
    0x03,
    'A',0,
    'N',0,
    'Y',0,
    'K',0,
    'A',0,
    '.',0,          
    '.',0,
    '.',0,
    '.',0,
    '.',0
};

static T_U8 s_UdiskString2[] = 
{
    22,
    0x03,
    'D',0,
    'I',0,
    'S',0,
    'K',0,
    '.',0,
    '.',0,
    '.',0,
    '.',0,
    '.',0,
    '.',0
};

static T_U8 s_UdiskString3[] = 
{
    22,
    0x03,
    '0',0,
    '1',0,
    '2',0,
    '3',0,
    '4',0,
    '5',0,
    '6',0,
    '7',0,
    '8',0,
    '9',0
};
#if (CHIP_SEL_10C > 0)
T_VOID usb_set_ExtCmdCallbak(T_fUSB_SCSI_EXT_CALLBACK cmd_proc)
{
    ext_cmd_call = cmd_proc;
}
#endif

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
T_BOOL usbdisk_set_str_desc(T_eSTR_DESC index,T_CHR *str)
{
    T_U8 i = 0;

    if ( AK_NULL == str)
    {
        //akprintf(C1, M_DRVSYS, "str is null\n");
        return AK_FALSE;
    }
    switch (index)
    {
        case STR_MANUFACTURER:
            for (i = 2; i<=20; i+=2)
            {
                if (*str)
                {
                    s_UdiskString1[i] = *str++;
                }
                else
                {
                    s_UdiskString1[i] = ' ';
                }
            }
            break;
        case STR_PRODUCT:
            for (i = 2; i<=20; i+=2)
            {
                if (*str)
                {
                    s_UdiskString2[i] = *str++;
                }
                else
                {
                    s_UdiskString2[i] = ' ';
                }
            }
            break;
        default:
            //akprintf(C1, M_DRVSYS, "str type is invalid\n");
            return AK_FALSE;
    }
    return AK_TRUE;
}

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
T_VOID usbdisk_mboot_init(T_U32 mode)
{
    udisk_reset(mode);
    udisk_configok();
}
/**
 * @brief   init usb disk function
 *
 * Initialize usb disk buffer,creat usb disk task,creat message queue
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb mode 1.1 or 2.0
 * @return T_BOOL
 * @retval AK_FALSE init failed
 * @retval AK_TRUE init successful
 */

T_BOOL usbdisk_init(T_U32 mode)
{
    T_U8 i,j;
    T_U8* pBuffer[UDISK_BUFFER_NUM] = {AK_NULL};
#if (CHIP_SEL_10C > 0)
    ext_cmd_call = AK_NULL;
#endif
    if (AK_FALSE == msc_init_var())
    {
        drv_print("msc_init,alloc failed:", 0, AK_TRUE);
        return AK_FALSE;
    }

    for (i = 0; i < UDISK_BUFFER_NUM; i++)
    {
        pBuffer[i] = (T_U8 *)drv_malloc(UDISK_BUFFER_LEN);
        if (pBuffer[i] == AK_NULL)
        {
            drv_print("pBuffer,alloc failed:", (T_U32)pBuffer[i], AK_TRUE);
            for(j = 0; j < i; j++)
                drv_free(pBuffer[j]);
            return AK_FALSE;
        }
    }
#if USB_VAR_MALLOC > 0
    s_pUdisk = (T_UDISK_DEV *)drv_malloc(sizeof(T_UDISK_DEV));
    if (AK_NULL == s_pUdisk)
    {
        drv_print("s_pudisk_dev,alloc failed:", 0, AK_TRUE);
        for(j = 0; j < UDISK_BUFFER_NUM; j++)
            drv_free(pBuffer[j]);

        return AK_FALSE;
    }
#else
    s_pUdisk = &s_udisk_dev;
#endif
    //init s_pUdisk member
    memset(s_pUdisk,0,sizeof(T_UDISK_DEV));
    s_pUdisk->ulMode = mode;
    
    for (i = 0; i < UDISK_BUFFER_NUM; i++)
    {
        s_pUdisk->tTrans.tBuffer[i].pBuffer= pBuffer[i];
    }
    
    drv_print("usbdisk buffer ok, buffer num:",i, AK_TRUE);

    //init_serial_number();

#if USB_VAR_MALLOC > 0
    DrvModule_Init();
#endif
    //map message
    DrvModule_Map_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, udisk_scsi_msg_proc);
    
    return AK_TRUE;
}

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

 
#pragma arm section code = "_udisk_rw_"

T_BOOL usbdisk_proc(T_VOID)
{

#if !(USLAVE_USE_INTR > 0)
    usb_slave_poll_status();
#endif

    return DrvModule_Retrieve_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG);
}

#pragma arm section code

/**
 * @brief   start usb disk function, this function must be call after usbdisk_init
 *
 * Allocate L2 buffer , open usb controller,set usb disk callback,register interrupt process function
 * @author Huang Xin
 * @date  2010-08-04
 * @param T_VOID
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_start(T_VOID)
{
    //create task
    if (!DrvModule_Create_Task(DRV_MODULE_USB_DISK))
        return AK_FALSE;
    if(!udisk_enable())
        return AK_FALSE;
    
    return AK_TRUE;
}

/**
 * @brief   disable usb disk function.
 *
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author Huang Xin
 * @date 2010-08-04
 * @param T_VOID
 * @return  T_VOID
 */    
T_VOID usbdisk_stop(T_VOID)
{
    T_U8 i;
    
    DrvModule_Terminate_Task(DRV_MODULE_USB_DISK);
    udisk_disable();
    
    if (AK_NULL != s_pUdisk)
    {
        for (i = 0; i < UDISK_BUFFER_NUM; i++)
            drv_free(s_pUdisk->tTrans.tBuffer[i].pBuffer);

        #if USB_VAR_MALLOC > 0
        drv_free(s_pUdisk);
        #endif
        s_pUdisk = AK_NULL;
    }
    msc_free_var();
}

#ifndef BURN_TOOL

/**
 * @brief   usb slave bulk disk add a lun
 *
 * This function is called when host is  mounting usb disk
 * @author Huang Xin
 * @date 2010-08-04
 * @param pAddLun[in] struct of lun information.
 * @return T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_addLUN(T_pLUN_INFO pAddLun)
{
    if (s_pUdisk->ucLunNum > UDISK_MAXIMUM_LUN)
    {
        return AK_FALSE;
    }
    memcpy((T_U8 *)&s_pUdisk->tLunInfo[s_pUdisk->ucLunNum], (const T_U8*)pAddLun, sizeof(T_LUN_INFO));
    s_pUdisk->ucLunNum++;
    return AK_TRUE;
}

/**
 * @brief   usb slave bulk disk change lun
 *
 * When sd card is detected, change the lun for sd card
 * @author Huang Xin
 * @date 2010-08-04
 * @param pChgLun[in] struct of lun information.
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL usbdisk_changeLUN(T_pLUN_INFO pChgLun)
{
    T_U8 i;

    if (AK_NULL == pChgLun)
    {
        //akprintf(C1, M_DRVSYS, "lun info is null\n");
        return AK_FALSE;
    }
    for(i = 0; i < s_pUdisk->ucLunNum; i++)
    {
        if (s_pUdisk->tLunInfo[i].LunIdx == pChgLun->LunIdx)
        {
            memcpy((T_U8 *)&s_pUdisk->tLunInfo[i], (const T_U8*)pChgLun, sizeof(T_LUN_INFO));
            return AK_TRUE;
        }
    }
    return AK_FALSE;
}

/**
 * @brief   scsi cmd 'read format capacity' process
 *
 * send SCSI_READ_FORMAT_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_read_format_capacity(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
      
    msg.ucCmd = SCSI_READ_FORMAT_CAPACITY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        drv_print("send  format cap msg failed", 0, AK_TRUE);
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send format cap msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi cmd 'read capacity' process
 *
 * send SCSI_READ_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_read_capacity(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
  
    msg.ucCmd = SCSI_READ_CAPACITY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send read capacity failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send read capacity msg ok\n");
    return AK_TRUE;
}

#pragma arm section code = "_udisk_rw_"

/**
 * @brief   scsi cmd 'read10' or 'read12' process
 *
 * send SCSI_READ_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
 
T_BOOL udisk_read(T_U8 lun,T_U32 start_sector,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
    
    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucReadIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_READ_10;
    msg.ucLun = lun;
    msg.ulParam2 = start_sector;
    msg.ulParam1  = expected_bytes;  
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send read msg fail\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "(%d,%d,%d)",lun,start_sector,expected_bytes);
    //akprintf(C1, M_DRVSYS, "send read msg ok\n");
    return AK_TRUE; 
}

/**
 * @brief   scsi cmd 'write10' or 'write12' process
 *
 * send SCSI_WRITE_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_write(T_U8 lun,T_U32 start_sector,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
    
    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_WRITE_10;
    msg.ucLun = lun;
    msg.ulParam2 = start_sector;
    msg.ulParam1  = expected_bytes;  
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send scsi write msg fail\n");
        return AK_FALSE;
    }
   // akprintf(C1, M_DRVSYS, "[%d,%d,%d]",lun,start_sector,expected_bytes);
   // akprintf(C1, M_DRVSYS, "send write msg ok, 0x%x\n",s_pUdisk->tTrans.ulTransBytes);
    return AK_TRUE;
}

#pragma arm section code


/**
 * @brief   scsi cmd 'mode sense' or 'mode sense6' process
 *
 * send SCSI_MODESENSE_6 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_mode_sense6(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
  
    msg.ucCmd = SCSI_MODESENSE_6;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send mode sense6 msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send mode sense6 msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi cmd 'verify' process
 *
 * send SCSI_VERIFY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_verify(T_U8 lun)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_VERIFY;
    msg.ucLun = lun;
    msg.ulParam1 = 0;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send verify msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send verify msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi cmd 'start stop' process
 *
 * send SCSI_START_STOP msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_stop[in] start ,stop or eject disk expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_start_stop(T_U8 lun,T_U32 start_stop)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_START_STOP;
    msg.ucLun = lun;
    msg.ulParam1 = start_stop;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send start/stop msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send start/stop msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi cmd 'prevent remove' process
 *
 * send 'prevent remove' msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_prevent_remove(T_U8 lun)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_PREVENT_REMOVE;
    msg.ucLun = lun;
    msg.ulParam1 = 0;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send prevent remove msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send verify msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi extend cmd  process
 *
 * send SCSI_EXTEND msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param cmd[in] command
 * @param param[in] data
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_extend(T_U8 lun, T_U8 cmd, T_U8* param)
{
    T_SCSI_MSG msg;
    
    s_pUdisk->tTrans.ulTransBytes = 0;
    msg.ucCmd = SCSI_EXTEND;
    msg.ucLun = lun;
    msg.ulParam1 = (T_U32)cmd;
    msg.ulParam2 = (T_U32)param;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send cmd unsupported msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send cmd unsupported msg ok\n");
    return AK_TRUE;
}
#endif

/**
 * @brief   scsi cmd unsupported process
 *
 * send SCSI_UNSUPPORTED msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_unsupported(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;
    
    udisk_set_sense(INVALID_COMMAND_OPERATION_CODE);
    s_pUdisk->tTrans.ulTransBytes = 0;
    msg.ucCmd = SCSI_UNSUPPORTED;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send cmd unsupported msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send cmd unsupported msg ok\n");
    return AK_TRUE;
}

/**
 * @brief   scsi cmd 'inquiry' process
 *
 * send SCSI_INQUIRY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_inquiry(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_INQUIRY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        drv_print("send inquiry msg fail", 0, AK_TRUE);
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send inquiry msg ok\n");
    return AK_TRUE;
}

#pragma arm section code = "_udisk_rw_"

/**
 * @brief   scsi cmd 'test unit ready' process
 *
 * send SCSI_TEST_UNIT_READY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_test_unit_ready(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;

    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_TEST_UNIT_READY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        drv_print("send test ready msg failed", 0, AK_TRUE);
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send test rdy msg ok\n");
    return AK_TRUE;
}
#if (CHIP_SEL_10C > 0)
T_BOOL udisk_user_define(T_U8 lun,T_U32 expected_bytes,T_U8 *cmd_param)
{
    T_SCSI_MSG msg;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_USER_DEFINE;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = (T_U32)cmd_param;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send req sense msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send req sense msg ok\n");
    return AK_TRUE;
}
#endif
/**
 * @brief   scsi cmd 'request sense' process
 *
 * send SCSI_REQUEST_SENSE msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_req_sense(T_U8 lun,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_REQUEST_SENSE;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send req sense msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send req sense msg ok\n");
    return AK_TRUE;
}

static T_BOOL udisk_test_unit_ready_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();

    switch (stage)

    {
        case MSC_STAGE_STATUS:
            if(s_pUdisk->tLunInfo[pMsg->ucLun].sense == MEDIUM_NOTREADY_TO_READY)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED);

                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOSENSE;
            }
            else if (s_pUdisk->tLunInfo[pMsg->ucLun].sense == MEDIUM_NOTPRESENT)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;

#ifndef BURN_TOOL
        //usb-if cv test case9, data out stall may be too late,so call udisk_write10_proc()
        case MSC_STAGE_DATA_OUT :
            if(!udisk_write10_proc(pMsg,csw_status,res))
            {
                return AK_FALSE;
            }
            break;
#endif
        default:
            drv_print("test unit ready at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_request_sense_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{    
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
            if (DRV_MODULE_SUCCESS != status)
            {
                drv_print("wait EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                return AK_FALSE;
            }
            s_pUdisk->tTrans.ulTransBytes = UDISK_SENSE_LEN;
            if (expected_trans_bytes < UDISK_SENSE_LEN)
            {
                s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
            }
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //produce buffer
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,s_pUdisk->Sense,s_pUdisk->tTrans.ulTransBytes);
            //buf  is ready to tx
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
            if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
            {
                s_pUdisk->tTrans.enmTransState = TRANS_TX;
                expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                usb_slave_start_send(USB_BULK_IN_INDEX);
                usb_slave_data_in(USB_BULK_IN_INDEX,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
            }
            else
            {
                drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                return AK_FALSE;
            }
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("request sense at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

/**
 * @brief   set sense that contain error information
 *
 * called when scsi cmd parse or process is finish
 * @author Huang Xin
 * @date 2010-08-04
 * @param sense[in] error information
 * @return  T_VOID
 */
T_VOID udisk_set_sense(T_U32 sense)
{
    s_pUdisk->Sense[0] = SENSE_VALID | SENSE_ERRORCODE;
    s_pUdisk->Sense[1] = 0;
    //sense key
    s_pUdisk->Sense[2] = (sense>>16)&0xFF;
    //addtional sense length
    s_pUdisk->Sense[7] = 10;
    // Additional Sense Code
    s_pUdisk->Sense[12] = (sense>>8)&0xFF;
    // Additional Sense Code Qualifier
    s_pUdisk->Sense[13] = sense&0xFF;
}

/**
 * @brief   get udisk lun num
 *
 * called when scsi cmd parse
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_U8
 */
T_U8 udisk_lun_num(T_VOID)
{
    return s_pUdisk->ucLunNum;
}
#pragma arm section code

/**
 * @brief   anyka mass boot cmd  process
 *
 * send mass boot msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] scsi struct
 * @param expected_bytes[in] trans bytes expected by host
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
T_BOOL udisk_anyka_mass_boot(T_U8* scsi_data,T_U32 expected_bytes)
{
    T_SCSI_MSG msg;

    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucReadIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    
    msg.ucCmd = SCSI_ANYKA_MASS_BOOT;
    msg.ucLun = 0;
    msg.ulParam1 = expected_bytes;
    msg.ulParam2 = (T_U32)scsi_data;

    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (T_U32*)&msg))
    {
        //akprintf(C1, M_DRVSYS, "send anyka_mass_boot msg failed\n");
        return AK_FALSE;
    }
    //akprintf(C1, M_DRVSYS, "send cmd unsupported msg ok\n");
    return AK_TRUE;
}

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
T_BOOL usbdisk_mboot_set_cb(T_fMBOOT_HANDLE_CMD hnd_cmd, T_fMBOOT_HANDLE_SEND hnd_send, T_fMBOOT_HANDLE_RCV hnd_rcv)
{
    if(AK_NULL == hnd_cmd || AK_NULL == hnd_send || AK_NULL == hnd_rcv)
        return AK_FALSE;
    
    s_MassBoot.fMbootCmdCb = hnd_cmd;
    s_MassBoot.fMbootSendCb = hnd_send;
    s_MassBoot.fMbootRcvCb = hnd_rcv;
    return AK_TRUE;
}
static T_BOOL udisk_mass_boot_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_BOOL ret = AK_TRUE;
    T_U8 stage;
    T_U32 xfer_bytes = 0;
    T_U32 expected_trans_bytes = 0;
    T_S32 status;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;

    //akprintf(C1, M_DRVSYS, "(%d,%d,%d)",*((T_U8*)pMsg->ulParam2+1),expected_trans_bytes,stage);

    if(AK_NULL != s_MassBoot.fMbootCmdCb)
    {
        ret = s_MassBoot.fMbootCmdCb((T_U8 *)pMsg->ulParam2, expected_trans_bytes);
    }
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucReadIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                    return AK_FALSE;
                }

                //cmd<=128
                if(*((T_U8 *)pMsg->ulParam2 + 1) <= 128)
                {
                    s_pUdisk->tTrans.ulTransBytes = 13;
                    if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                    {
                        s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                    }
                    //produce buffer
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0x41;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0x4e;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0x59;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0x4b;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = 0x41;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = 0x20;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = 0x44;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = 0x45;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8) = 0x53;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+9) = 0x49;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+10) = 0x47;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+11) = 0x4e;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+12) = 0x45;

                    //buf  is ready to tx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
                    if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                    {
                        s_pUdisk->tTrans.enmTransState = TRANS_TX;
                        expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                        usb_slave_start_send(USB_BULK_IN_INDEX);
                        usb_slave_data_in(USB_BULK_IN_INDEX,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                    }
                    else
                    {
                        drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                        return AK_FALSE;
                    }
                    if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
                    {
                        *csw_status = CMD_PASSED;
                    }
                    *res = expected_trans_bytes;
                    return ret;
                }
                
                if (expected_trans_bytes > UDISK_READ_BUF_LEN)
                {
                    xfer_bytes = UDISK_READ_BUF_LEN;
                }
                else
                {
                    xfer_bytes = expected_trans_bytes;
                }
                //produce buffer
                if(AK_NULL != s_MassBoot.fMbootSendCb)
                {
                    if (!s_MassBoot.fMbootSendCb((T_U32)s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer, xfer_bytes))
                    {
                        *csw_status = CMD_FAILED;
                    }
                }

                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].bValidate = AK_TRUE;
                if((s_pUdisk->tTrans.enmTransState == TRANS_IDLE) || (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_TX))
                {
                    //continue usb tx
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    usb_slave_start_send(USB_BULK_IN_INDEX);
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                    {
                        usb_slave_data_in( USB_BULK_IN_INDEX,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            UDISK_READ_BUF_LEN);
                    }
                    else
                    {
                        usb_slave_data_in( USB_BULK_IN_INDEX,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                    }
                }
                s_pUdisk->tTrans.ucReadIndex = (s_pUdisk->tTrans.ucReadIndex+1)%UDISK_BUFFER_NUM;
                expected_trans_bytes -= xfer_bytes;
            }while(expected_trans_bytes > 0);
            if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
            {
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        case MSC_STAGE_DATA_OUT:
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucWriteIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait EVENT_USB_RX_FINISH fail or timeout 0x", status, AK_TRUE);
                    return AK_FALSE;
                }
                if(expected_trans_bytes > UDISK_WRITE_BUF_LEN)
                {
                    //trans data size per transfer 
                    xfer_bytes = UDISK_WRITE_BUF_LEN;
                }
                else
                {
                    //trans data size per transfer 
                    xfer_bytes = expected_trans_bytes;
                }
                if(AK_NULL != s_MassBoot.fMbootRcvCb)
                {
                    if(!s_MassBoot.fMbootRcvCb((T_U32)s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].pBuffer, xfer_bytes))
                    {
                        *csw_status = CMD_FAILED;
                    }
                }
                //buf  is ready to rx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].bValidate = AK_FALSE; 
                if (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_RX)
                {
                    //continue usb rx
                    s_pUdisk->tTrans.enmTransState = TRANS_RX;
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                    {
                        usb_slave_data_out( USB_BULK_OUT_INDEX,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            UDISK_WRITE_BUF_LEN);
                    }
                    else
                    {
                        usb_slave_data_out( USB_BULK_OUT_INDEX,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                    }
                }
                s_pUdisk->tTrans.ucWriteIndex = (s_pUdisk->tTrans.ucWriteIndex+1)%UDISK_BUFFER_NUM;
                expected_trans_bytes -= xfer_bytes ;
            }while(expected_trans_bytes > 0);
            
            if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
            {
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        case MSC_STAGE_STATUS:
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;  
            break;
        default:
            drv_print("mboot_cmd at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }

    return ret;
}

#ifndef BURN_TOOL

#pragma arm section code = "_udisk_rw_"

static T_BOOL udisk_read10_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{    
    T_U8 stage;
    T_U32 xfer_sectors = 0;
    T_U32 xfer_bytes = 0;
    T_U32 start_sector;
    T_U32 expected_trans_bytes = 0;
    T_U32 sectors_per_buf = 0;
    T_U8* read_buf = AK_NULL;
    T_S32 status;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            //all data bytes to be transfered
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                start_sector = pMsg->ulParam2;
                sectors_per_buf = UDISK_READ_BUF_LEN /s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                do
                {
                    status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucReadIndex), 1000);
                    if (DRV_MODULE_SUCCESS != status)
                    {
                        drv_print("wait EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucReadIndex, AK_TRUE);
                        return AK_FALSE;
                    }
                    else
                    {
                        read_buf = s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer;
                        if(expected_trans_bytes >= UDISK_READ_BUF_LEN)
                        {
                            //trans data size per transfer 
                            xfer_sectors =  sectors_per_buf;
                            xfer_bytes = UDISK_READ_BUF_LEN;
                            if ((start_sector == s_pUdisk->pre_read_start_sector) && (s_pUdisk->pre_read_sector_num > 0) && (s_pUdisk->pre_read_lun == pMsg->ucLun))
                            {
                                start_sector += s_pUdisk->pre_read_sector_num;
                                xfer_sectors -= s_pUdisk->pre_read_sector_num;
                                read_buf += s_pUdisk->pre_read_sector_num * s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                            }
                        }
                        else
                        {
                            //trans data size per transfer 
                            xfer_sectors =  expected_trans_bytes/ s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                            xfer_bytes = expected_trans_bytes;
                        }
                        s_pUdisk->pre_read_sector_num = 0;
                        //produce buffer
                        s_pUdisk->tLunInfo[pMsg->ucLun].Read(read_buf, start_sector, xfer_sectors, s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                        //buf  is ready to tx
                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].bValidate = AK_TRUE;
                        if((s_pUdisk->tTrans.enmTransState == TRANS_IDLE) || (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_TX))
                        {
                            //continue usb tx
                            s_pUdisk->tTrans.enmTransState = TRANS_TX;
                            usb_slave_start_send(USB_BULK_IN_INDEX);
                            if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                            { 
                                usb_slave_data_in( USB_BULK_IN_INDEX,
                                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                    UDISK_READ_BUF_LEN);                                 
                            }
                            else
                            {
                                usb_slave_data_in( USB_BULK_IN_INDEX,
                                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                    s_pUdisk->tTrans.ulTransBytes);
                            }
                        }
                        s_pUdisk->tTrans.ucReadIndex = (s_pUdisk->tTrans.ucReadIndex+1)%UDISK_BUFFER_NUM;
                        
                        //if  <read10> expect 64kbytes, use pre read to improve read speed, and read speed is fastest when pre read size is 32kbytes
                        if (UDISK_READ_BUF_LEN == expected_trans_bytes)
                        {
                            s_pUdisk->pre_read_sector_num = 0;//(32*1024)/s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                        }
                        expected_trans_bytes -= xfer_bytes ;
                        start_sector += xfer_sectors;

                        if (s_pUdisk->pre_read_sector_num > 0)
                        {
                            //pre read, produce buffer
                            s_pUdisk->pre_read_lun = pMsg->ucLun;
                            s_pUdisk->pre_read_start_sector = start_sector;
                            s_pUdisk->tLunInfo[pMsg->ucLun].Read(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer, 
                                                                        s_pUdisk->pre_read_start_sector, 
                                                                        s_pUdisk->pre_read_sector_num, 
                                                                        s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                        }
                    }
                }while(expected_trans_bytes > 0);
             
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        //usb-if cv test case10,data out stall may be too late,so call udisk_write10_proc()
        case MSC_STAGE_DATA_OUT:
            if(!udisk_write10_proc(pMsg,csw_status,res))
            {
                return AK_FALSE;
            }
            *csw_status = PHASE_ERROR;
            break;
        default:
            drv_print("read10 at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_write10_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_U32 xfer_sectors = 0;
    T_U32 xfer_bytes = 0;
    T_U32 start_sector;
    T_U32 expected_trans_bytes = 0;
    T_U32 sectors_per_buf = 0;
    T_S32 status;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_OUT :
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //LBA at which the write begin
            start_sector = pMsg->ulParam2;
            sectors_per_buf = UDISK_WRITE_BUF_LEN /s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucWriteIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait EVENT_USB_RX_FINISH fail or timeout 0x", status, AK_TRUE);
                    return AK_FALSE;
                }
                else
                {
                    if(expected_trans_bytes > UDISK_WRITE_BUF_LEN)
                    {
                        //trans data size per transfer 
                        xfer_sectors =  sectors_per_buf;
                        xfer_bytes = UDISK_WRITE_BUF_LEN;
                    }
                    else
                    {
                        //trans data size per transfer 
                        xfer_sectors =  expected_trans_bytes / s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;;
                        xfer_bytes = expected_trans_bytes;
                    }
                    
                    s_pUdisk->tLunInfo[pMsg->ucLun].Write(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].pBuffer, 
                                                                        start_sector, 
                                                                        xfer_sectors, 
                                                                        s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                    //buf  is ready to rx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].bValidate = AK_FALSE; 
                    if (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_RX)  
                    {
                        //continue usb rx
                        s_pUdisk->tTrans.enmTransState = TRANS_RX;
                        if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                        { 
                            usb_slave_data_out( USB_BULK_OUT_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                UDISK_WRITE_BUF_LEN);
                        }
                        else
                        {
                            usb_slave_data_out( USB_BULK_OUT_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                s_pUdisk->tTrans.ulTransBytes);
                        }
                    }
                    s_pUdisk->tTrans.ucWriteIndex = (s_pUdisk->tTrans.ucWriteIndex+1)%UDISK_BUFFER_NUM;
                    expected_trans_bytes -= xfer_bytes;
                    start_sector += xfer_sectors;
                }
            }while(expected_trans_bytes > 0);
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
        //usb-if cv test case8
        case MSC_STAGE_DATA_IN :
            DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
            *csw_status = PHASE_ERROR;
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("write10 at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

#pragma arm section code

static T_BOOL udisk_read_capacity_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait  EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                    return AK_FALSE;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_CAPACITY_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //produce buffer
                //last LBA
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF)>>0);
                //blk len in bytes
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF)>>0);
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(USB_BULK_IN_INDEX);
                    usb_slave_data_in(USB_BULK_IN_INDEX,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                    return AK_FALSE;
                }
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("read capacity at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_read_format_capacity_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait  EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                    return AK_FALSE;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_FORMAT_CAPACITY_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //produce buffer
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0;
                //capacity list len
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = UDISK_FORMAT_CAPACITY_LEN-4;
                //blk num
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6)= (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF)>>0);
                //desciptor of media,01=unformatted,02=formatted,no Cartridge in dirver
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8) = 0x2;
                //blk len in bytes
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+9) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+10)= (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+11) = (T_U8)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF)>>0);
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(USB_BULK_IN_INDEX);
                    usb_slave_data_in(USB_BULK_IN_INDEX,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                    return AK_FALSE;
                }
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("read format capacity at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_mode_sense6_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait  EVENT_USB_TX_FINISH fail ,bufid= ", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                    return AK_FALSE;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_MODE_PARAMETER_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //produce buffer
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = UDISK_MODE_PARAMETER_LEN-1;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0;
                 //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(USB_BULK_IN_INDEX);
                    usb_slave_data_in(USB_BULK_IN_INDEX,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                    return AK_FALSE;
                }
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("mode sense6 at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_verify_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)

    {
        case MSC_STAGE_STATUS:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("verify at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_prevent_remove_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)

    {
        case MSC_STAGE_STATUS:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("prevent remove at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_start_stop_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)

    {
        case MSC_STAGE_STATUS:
            //eject media
            if (UDISK_EJECT_MEDIA == pMsg->ulParam1)
            {
                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOTPRESENT;
                usb_slave_set_state(USB_START_STOP);
            }
            //start media
            else if (UDISK_START_MEDIA == pMsg->ulParam1 )
            {
                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOSENSE;
            }
            //stop media
            else
            {
                 s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOTREADY;
            }
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("start stop at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL udisk_extend_cmd_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_U32 cmd = pMsg->ulParam1;
    T_U8 *data = (T_U8 *)pMsg->ulParam2;

    stage = msc_get_stage();

    switch (cmd)

    {
        case SCSI_EXT_USB_UPDATE:
            if(stage == MSC_STAGE_DATA_IN)
            {
                //send stall
                usb_slave_ep_stall(USB_BULK_IN_INDEX);
                UDISK_POLL_STATUS(1 == usb_slave_get_ep_status(USB_BULK_IN_INDEX));

                msc_set_stage(MSC_STAGE_STATUS);
            }

            if('A' == data[0] && 'N' == data[1] && 'Y' == data[2] &&
               'K' == data[3] && 'A' == data[4] )
            {
                pMsg->ulParam2 = 0x0;
            }
            
            break;
            
        default:
            break;
    }

    *csw_status = CMD_PASSED;
    return AK_TRUE;
}

#endif

static T_BOOL udisk_inquiry_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_S32 status;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
            if (DRV_MODULE_SUCCESS != status)
            {
                drv_print("wait  EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucTransIndex, AK_TRUE);
                return AK_FALSE;
            }
            s_pUdisk->tTrans.ulTransBytes = UDISK_INQ_STR_LEN;
            if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
            {
                s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
            }  
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //produce buffer
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0x80;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0x02;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0x02;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = 0x1f;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = 0x00;
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8,s_pUdisk->tLunInfo[pMsg->ucLun].Vendor,INQUIRY_STR_VENDOR_SIZE);
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+16,s_pUdisk->tLunInfo[pMsg->ucLun].Product,INQUIRY_STR_PRODUCT_SIZE);
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+32,s_pUdisk->tLunInfo[pMsg->ucLun].Revision,INQUIRY_STR_REVISION_SIZE);
            //buf  is ready to tx
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
            if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
            {
                s_pUdisk->tTrans.enmTransState = TRANS_TX;
                expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                usb_slave_start_send(USB_BULK_IN_INDEX);
                usb_slave_data_in(USB_BULK_IN_INDEX,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
            }
            else
            {
                drv_print("trans status error : 0x", s_pUdisk->tTrans.enmTransState, AK_TRUE);
                return AK_FALSE;
            }
            *csw_status = CMD_PASSED; 
            *res = expected_trans_bytes;
            break;
        default:
            drv_print("inquiry at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}
#if (CHIP_SEL_10C > 0)
static T_BOOL udisk_userdefine_cmd_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_U32 expected_trans_bytes = 0;
    T_S32 userdef_ret;
    T_S32 status;
    T_U32 xfer_bytes;
    T_U8 cmd = (*((T_U8*)pMsg->ulParam2));
    T_U8 *parm_cmd = (T_U8*)(pMsg->ulParam2+1);
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    userdef_ret = ext_cmd_call(cmd,parm_cmd,expected_trans_bytes,MSC_STAGE_COMMAND);
    switch(stage)
    {
        case MSC_STAGE_DATA_OUT:
            s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucWriteIndex;
            //drv_print("userdefine_cmd MSC_STAGE_DATA_OUT", expected_trans_bytes, AK_TRUE);
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucWriteIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait EVENT_USB_RX_FINISH fail or timeout 0x", status, AK_TRUE);
                    return AK_FALSE;
                }
                else
                {
                    if(expected_trans_bytes > UDISK_WRITE_BUF_LEN)
                    {
                        xfer_bytes = UDISK_WRITE_BUF_LEN;
                    }
                    else
                    {
                        xfer_bytes = expected_trans_bytes;
                    }
                    
                    ext_cmd_call(cmd,s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].pBuffer, 
                                                                        xfer_bytes, 
                                                                        MSC_STAGE_DATA_OUT);
                    //buf  is ready to rx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].bValidate = AK_FALSE; 
                    if (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_RX)  
                    {
                        //continue usb rx
                        s_pUdisk->tTrans.enmTransState = TRANS_RX;
                        if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                        { 
                            usb_slave_data_out( USB_BULK_OUT_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                UDISK_WRITE_BUF_LEN);
                        }
                        else
                        {
                            usb_slave_data_out( USB_BULK_OUT_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                s_pUdisk->tTrans.ulTransBytes);
                        }
                    }
                    s_pUdisk->tTrans.ucWriteIndex = (s_pUdisk->tTrans.ucWriteIndex+1)%UDISK_BUFFER_NUM;
                    expected_trans_bytes -= xfer_bytes;
                }
            }while(expected_trans_bytes > 0);
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
            break;
        case MSC_STAGE_DATA_IN:
            s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucReadIndex;
            //drv_print("userdefine_cmd MSC_STAGE_DATA_IN", expected_trans_bytes, AK_TRUE);
            //返回需要返回数据的长度
            do
            {
                //drv_print("MSC_STAGE_DATA_IN WaitEvent:", 0, AK_TRUE);
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucReadIndex), 1000);
                //drv_print("MSC_STAGE_DATA_IN Wait Finish:", 0, AK_TRUE);
                if (DRV_MODULE_SUCCESS != status)
                {
                    drv_print("wait EVENT_USB_TX_FINISH fail ,bufid=0x", s_pUdisk->tTrans.ucReadIndex, AK_TRUE);
                    return AK_FALSE;
                }
                else
                {
                    if(expected_trans_bytes >= UDISK_READ_BUF_LEN)
                    {
                        xfer_bytes = UDISK_READ_BUF_LEN;
                    }
                    else
                    {
                        xfer_bytes = expected_trans_bytes;
                    }
                    //produce buffer
                    userdef_ret = ext_cmd_call(cmd,s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer,xfer_bytes,MSC_STAGE_DATA_IN);
                    //buf  is ready to tx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].bValidate = AK_TRUE;
                    //drv_print("MSC_STAGE_DATA_IN ready Send:", userdef_ret, AK_TRUE);
                    if(userdef_ret < xfer_bytes)
                    {
                        s_pUdisk->tTrans.ulTransBytes = userdef_ret;
                    }
                    if((s_pUdisk->tTrans.enmTransState == TRANS_IDLE) || (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_TX))
                    {
                        //continue usb tx
                        s_pUdisk->tTrans.enmTransState = TRANS_TX;
                        usb_slave_start_send(USB_BULK_IN_INDEX);
                        if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                        { 
                            //drv_print("MSC_STAGE_DATA_IN Send 1:", UDISK_READ_BUF_LEN, AK_TRUE);
                            usb_slave_data_in( USB_BULK_IN_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                UDISK_READ_BUF_LEN);                                 
                        }
                        else
                        {
                            //drv_print("MSC_STAGE_DATA_IN Send 2:", s_pUdisk->tTrans.ulTransBytes, AK_TRUE);
                            usb_slave_data_in( USB_BULK_IN_INDEX,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                s_pUdisk->tTrans.ulTransBytes);
                        }
                    }
                    s_pUdisk->tTrans.ucReadIndex = (s_pUdisk->tTrans.ucReadIndex+1)%UDISK_BUFFER_NUM;
                    expected_trans_bytes -= xfer_bytes ;
                }
            }while((expected_trans_bytes > 0)&&(xfer_bytes == userdef_ret));
            //drv_print("MSC_STAGE_DATA_IN End:", s_pUdisk->tTrans.ulTransBytes, AK_TRUE);
            *csw_status = CMD_PASSED;
            if(xfer_bytes == userdef_ret)
                *res = expected_trans_bytes;
            else
                *res = (expected_trans_bytes+xfer_bytes-userdef_ret);
            break;
        case MSC_STAGE_STATUS:
            *csw_status = CMD_PASSED;
            *res = 0; 
            break;
        default:
            drv_print("userdefine_cmd at error stage!", 0, AK_TRUE);
            return AK_FALSE;
            break;
    }
    userdef_ret = ext_cmd_call(cmd,AK_NULL,0,MSC_STAGE_STATUS);
    return AK_TRUE;
}
#endif
static T_BOOL udisk_unsupported_cmd_proc(T_pSCSI_MSG pMsg,T_U8* csw_status,T_U32* res)
{
    T_U8 stage;
    T_U32 expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)

    {
        case MSC_STAGE_DATA_OUT:
            //bug:data out stall may be too late
            usb_slave_ep_stall(USB_BULK_OUT_INDEX);
            //wait clr stall
            UDISK_POLL_STATUS(1 == usb_slave_get_ep_status(USB_BULK_OUT_INDEX));

            break;
        case MSC_STAGE_DATA_IN:
        case MSC_STAGE_STATUS:
            usb_slave_ep_stall(USB_BULK_IN_INDEX);
            //wait clr stall
            UDISK_POLL_STATUS(1 == usb_slave_get_ep_status(USB_BULK_IN_INDEX));

            break;
        default: 
            drv_print("unsupported cmd at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    msc_set_stage(MSC_STAGE_STATUS);
    *csw_status = CMD_FAILED;
    *res = expected_trans_bytes;
    return AK_TRUE;
}

#pragma arm section code = "_udisk_rw_"

static T_BOOL udisk_send_csw_proc(T_U8 csw_status,T_U32 residue)
{
    T_U8 stage;

    stage = msc_get_stage();
    switch (stage)

    {
        case MSC_STAGE_DATA_IN:
            DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE, 1000);
            //device real data is less than host expected data
            if (residue > 0)
            {
                usb_slave_ep_stall(USB_BULK_IN_INDEX);
                //wait clr stall
                UDISK_POLL_STATUS(1 == usb_slave_get_ep_status(USB_BULK_IN_INDEX));

            }
            //data stage finish,send csw
            msc_set_stage(MSC_STAGE_STATUS);
            msc_send_csw(csw_status,residue);
            break;
        case MSC_STAGE_DATA_OUT:
            DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE, 1000);
            //device real data is less than host expected data
            if (residue > 0)
            {
                //bug:data out stall may be too late
                usb_slave_ep_stall(USB_BULK_OUT_INDEX);
                //wait clr stall
                UDISK_POLL_STATUS(1 == usb_slave_get_ep_status(USB_BULK_OUT_INDEX));
            }
            msc_set_stage(MSC_STAGE_STATUS);
            msc_send_csw(csw_status,residue);
            break;
        case MSC_STAGE_STATUS:
            msc_send_csw(csw_status,residue);
            break;
        default:
            drv_print("send csw at error stage!", 0, AK_TRUE);
            return AK_FALSE;
    }
    return AK_TRUE;
}

static T_VOID udisk_error_recovery(T_VOID)
{
    drv_print("error recovery!", 0, AK_TRUE);
    usb_slave_std_hard_stall(AK_TRUE);
    usb_slave_ep_stall(USB_BULK_IN_INDEX);
    usb_slave_ep_stall(USB_BULK_OUT_INDEX);
}

/**
 * @brief   scsi msg process 
 *
 * called by udisk task when scsi msg sent successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pMsg[in] scsi msg parameter
 * @param len[in] scsi msg parameter len
 * @return  T_VOID
 */
static T_VOID udisk_scsi_msg_proc(T_U32* pMsg,T_U32 len)
{
    T_BOOL ret;
    T_pSCSI_MSG message = (T_pSCSI_MSG)pMsg;
    T_U8 csw_status = 0;
    T_U32 residue = 0;

    switch (message->ucCmd)
    {
#ifndef BURN_TOOL
        case SCSI_READ_FORMAT_CAPACITY:
            ret = udisk_read_format_capacity_proc(message, &csw_status, &residue);
            break;
        case SCSI_READ_CAPACITY:
            ret = udisk_read_capacity_proc(message, &csw_status, &residue);
            break;
        case SCSI_READ_10:
            ret = udisk_read10_proc(message, &csw_status, &residue);
            break;
        case SCSI_WRITE_10:
            ret = udisk_write10_proc(message, &csw_status, &residue);
            break;
        case SCSI_MODESENSE_6:
            ret = udisk_mode_sense6_proc(message, &csw_status, &residue);
            break;
        case SCSI_VERIFY:
            ret = udisk_verify_proc(message, &csw_status, &residue);
            break;
        case SCSI_START_STOP:
            ret = udisk_start_stop_proc(message, &csw_status, &residue);
            break;
        case SCSI_PREVENT_REMOVE:
            ret = udisk_prevent_remove_proc(message, &csw_status, &residue);
            break;

        case SCSI_EXTEND:
            ret = udisk_extend_cmd_proc(message, &csw_status, &residue);
            break;
#endif
        case SCSI_INQUIRY:
            ret = udisk_inquiry_proc(message, &csw_status, &residue);
            break;
        case SCSI_TEST_UNIT_READY:
            ret = udisk_test_unit_ready_proc(message, &csw_status, &residue);
            break;
        case SCSI_REQUEST_SENSE:
            ret = udisk_request_sense_proc(message, &csw_status, &residue);
            break; 
        case SCSI_UNSUPPORTED:
            ret = udisk_unsupported_cmd_proc(message, &csw_status, &residue);
            break;
        case SCSI_USER_DEFINE:
#if (CHIP_SEL_10C > 0)
            if(ext_cmd_call)
                ret = udisk_userdefine_cmd_proc(message, &csw_status, &residue);
            else
#endif  
                ret = udisk_unsupported_cmd_proc(message, &csw_status, &residue);
            break;
        default:
            ret = udisk_mass_boot_proc(message, &csw_status, &residue);
            break;
    }
    if (ret)
    {
        if(!udisk_send_csw_proc(csw_status,residue))
        {
            udisk_error_recovery();
        }
        if(SCSI_ANYKA_MASS_BOOT == message->ucCmd)
        {
            //wait csw send finish
            UDISK_POLL_STATUS( MSC_STAGE_STATUS == msc_get_stage());

            if(AK_NULL != s_MassBoot.fMbootCmdCb)
                s_MassBoot.fMbootCmdCb(AK_NULL,message->ulParam1);
        }

        if(SCSI_EXTEND == message->ucCmd)
        {
            //do some extra stuff to extend command
            if(SCSI_EXT_USB_UPDATE == message->ulParam1 && 0 == message->ulParam2)
            {
                UDISK_POLL_STATUS(MSC_STAGE_STATUS == msc_get_stage());
                
                usb_slave_set_state(USB_UPDATE);
            }
        }
    }
    else
    {
        udisk_error_recovery();
    }
}

#pragma arm section code


/**
 * @brief   udisk enable 
 *
 * called by usbdisk_start() when start udisk task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL udisk_enable()
{
    Usb_Slave_Standard.usb_get_device_descriptor = udisk_get_dev_desc;
    Usb_Slave_Standard.usb_get_config_descriptor = udisk_get_cfg_desc;
    Usb_Slave_Standard.usb_get_string_descriptor = udisk_get_str_desc;
    Usb_Slave_Standard.usb_get_device_qualifier_descriptor = udisk_get_dev_qualifier_desc;
    Usb_Slave_Standard.usb_get_other_speed_config_descriptor = udisk_get_other_speed_cfg_desc;
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer  =             (T_U8 *)drv_malloc(4096); 
    Usb_Slave_Standard.buf_len =            4096;
    
    //usb slave init,alloc L2 buffer,register irq
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    //usb std init,set ctrl callback
    usb_slave_std_init();
    //set class req callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, udisk_class_callback);
    usb_slave_set_callback(udisk_reset,udisk_suspend,udisk_resume, udisk_configok);
    usb_slave_set_tx_callback(USB_BULK_IN_INDEX, udisk_send_finish);
    usb_slave_set_rx_callback(USB_BULK_OUT_INDEX, udisk_receive_notify, udisk_receive_finish);
    usb_slave_device_enable(s_pUdisk->ulMode);
    return AK_TRUE;
}

/**
 * @brief   udisk disable 
 *
 * called by usbdisk_stop() when terminate udisk task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL udisk_disable()
{
    drv_free(Usb_Slave_Standard.Buffer);
    memset(&Usb_Slave_Standard,0,sizeof(Usb_Slave_Standard));
    usb_slave_free();
    usb_slave_device_disable();
    //clear  callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, AK_NULL);
    usb_slave_set_callback(AK_NULL,AK_NULL,AK_NULL, AK_NULL);
    usb_slave_set_tx_callback(USB_BULK_IN_INDEX, AK_NULL);
    usb_slave_set_rx_callback(USB_BULK_OUT_INDEX, AK_NULL, AK_NULL);
    return AK_TRUE;
}

/**
 * @brief   set udisk class req callback
 *
 * called by usb drv  when msc class req is received successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL udisk_class_callback(T_CONTROL_TRANS *pTrans)
{
    T_U8 req_type;

    req_type = (pTrans->dev_req.bmRequestType >> 5) & 0x3;
    if (req_type != REQUEST_CLASS)
        return AK_FALSE;
    switch(pTrans->dev_req.bRequest)
    {
        //mass storage bulk only reset
        case 0xFF:
            udisk_bo_reset(pTrans);
            break;
        //mass storage bulk only get max lun
        case 0xFE:
            udisk_get_max_lun(pTrans);
            break;
        default:
            break;
    }
    return AK_TRUE;
    
}

/**
 * @brief   msc bulk only reset
 *
 * called  when msc class req is bulk only reset
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL udisk_bo_reset(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage == CTRL_STAGE_STATUS)
    {
        usb_slave_std_hard_stall(AK_FALSE);
        udisk_reset(s_pUdisk->ulMode);
        msc_set_stage(MSC_STAGE_READY);
    }
    return AK_TRUE;
}

/**
 * @brief   msc get max lun
 *
 * called  when msc class req is get max lun
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL udisk_get_max_lun(T_CONTROL_TRANS *pTrans)
{
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return AK_TRUE;
    }
    
    drv_print("udisk max lun is ",s_pUdisk->ucLunNum-1, AK_TRUE);
    
    if(s_pUdisk->ucLunNum > 0)
    {
        pTrans->buffer[0] = (s_pUdisk->ucLunNum-1);
    }
    else
    {
        pTrans->buffer[0] = 0;
    }
    pTrans->data_len = 1;
    return AK_TRUE; 
}

/**
 * @brief  reset callback
 *
 * called  when usb reset
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb1.1 or usb2.0
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_VOID udisk_reset(T_U32 mode)
{
    T_U32 i;
    
    usb_slave_set_ep(USB_BULK_IN_INDEX,EP_ATTRIBUTE_BULK,USB_EP_IN_TYPE,512);
    usb_slave_set_ep(USB_BULK_OUT_INDEX,EP_ATTRIBUTE_BULK,USB_EP_OUT_TYPE,512);

    //clr stall status to avoid die when udisk task is waiting clr stall to send csw 
    usb_slave_set_ep_status(USB_BULK_IN_INDEX,0);
    usb_slave_set_ep_status(USB_BULK_OUT_INDEX,0);
    usb_slave_std_hard_stall(AK_FALSE);
    if(mode == USB_MODE_20)
    {
        s_UdiskEp2Desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        s_UdiskEp1Desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
    }
    else
    {
        s_UdiskEp2Desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        s_UdiskEp1Desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
    }
    //reinit s_pUdisk member,usb1.1 or usb2.0
    s_pUdisk->ulMode = mode;  
    s_pUdisk->tTrans.ulTransBytes = 0;
    s_pUdisk->tTrans.ucReadIndex = 0;
    s_pUdisk->tTrans.ucWriteIndex = 0;
    s_pUdisk->tTrans.ucTransIndex = 0;
    s_pUdisk->tTrans.enmTransState = 0;
    for (i = 0; i<UDISK_BUFFER_NUM; i++)
    {
        s_pUdisk->tTrans.tBuffer[i].bValidate = AK_FALSE;
    }

    //clear all buffer status
    for(i=0; i<UDISK_BUFFER_NUM;i++)
    {
        DrvModule_ClrEvent(DRV_MODULE_USB_DISK, EVENT_USB_RX_FINISH(i));
    }

    msc_set_stage(MSC_STAGE_READY);
}

/**
 * @brief  suspend callback
 *
 * called  when usb suspend
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_suspend()
{
    //clr stall status to avoid die when udisk task is waiting clr stall to send csw 
    usb_slave_set_ep_status(USB_BULK_IN_INDEX,0);
    usb_slave_set_ep_status(USB_BULK_OUT_INDEX,0);
    usb_slave_std_hard_stall(AK_FALSE);

    DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
    DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex));
    DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucTransIndex));
}

/**
 * @brief  resume callback
 *
 * called  when usb resume
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_resume()
{
    drv_print("udisk resume ", 0, AK_TRUE);
}


#pragma arm section code = "_udisk_rw_"


/**
 * @brief  bulk in ep send finish callback
 *
 * called  when bulk in ep send finish
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_send_finish()
{
    T_U8 stage;
    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex)))
            {
                //akprintf(C1, M_DRVSYS, "set EVENT_USB_TX_FINISH fail bufid=0x%x\n",s_pUdisk->tTrans.ucTransIndex);
                break;
            }
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_FALSE;
            s_pUdisk->tTrans.ucTransIndex = (s_pUdisk->tTrans.ucTransIndex+1)%UDISK_BUFFER_NUM;
            if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
            { 
                s_pUdisk->tTrans.ulTransBytes -= UDISK_READ_BUF_LEN;
                if (s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate)
                {
                    usb_slave_start_send(USB_BULK_IN_INDEX);
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                    { 
                        usb_slave_data_in(USB_BULK_IN_INDEX,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        UDISK_READ_BUF_LEN);
                    }
                    else
                    {
                        usb_slave_data_in(USB_BULK_IN_INDEX,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);                  
                    }
                }
                else
                {
                    //no data to send
                    s_pUdisk->tTrans.enmTransState= TRANS_SUSPEND_TX;
                }
            }
            else
            {
                s_pUdisk->tTrans.ulTransBytes = 0;
                s_pUdisk->tTrans.enmTransState = TRANS_IDLE;
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
            }
            break;
        case MSC_STAGE_STATUS:
            msc_set_stage(MSC_STAGE_READY);
            break;
        default:
            drv_print("tx finish at error stage 0x", stage, AK_TRUE);
            break;
    }
}

/**
 * @brief  bulk out ep receive start callback
 *
 * called  when bulk out ep receive start
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_receive_notify()
{
    T_SCSI_MSG pMsg;
    T_U32 rcv_cnt = 0 ;
    T_U8 cbw[512]={0};
    T_U8 stage, i;

    stage = msc_get_stage();

    switch (stage)
    {
        case MSC_STAGE_READY:
            //start usb rx cbw
            s_pUdisk->tTrans.enmTransState = TRANS_RX;
            msc_set_stage(MSC_STAGE_COMMAND);
            rcv_cnt = usb_slave_data_out(USB_BULK_OUT_INDEX, cbw, CBW_PKT_SIZE);
            msc_parse_cbw(cbw, rcv_cnt);

            break;
        case MSC_STAGE_DATA_OUT:
            if (!s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate)
            {
                //start usb rx data
                s_pUdisk->tTrans.enmTransState = TRANS_RX;
                if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                {      
                    usb_slave_data_out(USB_BULK_OUT_INDEX,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    UDISK_WRITE_BUF_LEN);
                }
                else
                {
                    usb_slave_data_out(USB_BULK_OUT_INDEX,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
                }
            }
            else
            {
                //no buffer to reveive
                s_pUdisk->tTrans.enmTransState= TRANS_SUSPEND_RX;
            }
            break;

        default:
            drv_print("rx notify at error stage: ", stage, AK_TRUE);
            break;
    }
}

/**
 * @brief  bulk out ep receive finish callback
 *
 * called  when bulk out ep receive finish
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_receive_finish()
{
    T_U8 stage;

    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_COMMAND:
            s_pUdisk->tTrans.enmTransState= TRANS_IDLE;
            break;
        case MSC_STAGE_DATA_OUT:
            if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucTransIndex)))
            {
                //akprintf(C1, M_DRVSYS, "set EVENT_USB_RX_FINISH fail,bufid = 0x%x\n",s_pUdisk->tTrans.ucTransIndex);
            }
            else
            {
                if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                { 
                    s_pUdisk->tTrans.ulTransBytes -= UDISK_WRITE_BUF_LEN;
                }
                else
                {
                    s_pUdisk->tTrans.ulTransBytes = 0;
                    s_pUdisk->tTrans.enmTransState= TRANS_IDLE;
                    DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                }
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = AK_TRUE;
                s_pUdisk->tTrans.ucTransIndex = (s_pUdisk->tTrans.ucTransIndex+1)%UDISK_BUFFER_NUM;
            }
            break;
        default:
            drv_print("rx finish at error stage 0x", stage, AK_TRUE);
            break;
    }
}

#pragma arm section code

/**
 * @brief config ok callback
 *
 * called  when enum successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  T_VOID
 */
static T_VOID udisk_configok()
{
    T_U8 i;
    
    //clear all buffer
    for(i=0; i<UDISK_BUFFER_NUM;i++)
    {
        if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(i)))
        {
            //akprintf(C1, M_DRVSYS, "before read, set EVENT_USB_TX_FINISH fail bufid=0x%x\n",i);
        }
    }
    DrvModule_ClrEvent(DRV_MODULE_USB_DISK, EVENT_STATUS_STAGE);
    msc_set_stage(MSC_STAGE_READY);
}

/** 
 * @brief get dev qualifier descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev qualifier descriptor
 * @return  T_U8 *
 * @retval  addr of dev qualifier descriptor
 */
static T_U8 *udisk_get_dev_qualifier_desc(T_U32 *count)
{
    *count = sizeof(s_UdiskDeviceQualifierDesc);
    return (T_U8 *)&s_UdiskDeviceQualifierDesc;
}

/** 
 * @brief get dev descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev descriptor
 * @return  T_U8 *
 * @retval  addr of dev descriptor
 */
static T_U8 *udisk_get_dev_desc(T_U32 *count)
{
    *count = sizeof(s_UdiskDeviceDesc);
    return (T_U8 *)&s_UdiskDeviceDesc;
}

/** 
 * @brief get all config descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of all config descriptor
 * @return  T_U8 *
 * @retval  addr of all config descriptor
 */
static T_U8 *udisk_get_cfg_desc(T_U32 *count)
{
    T_U32 cnt = 0;
    
    memcpy(configdata, (T_U8 *)&s_UdiskConfigDesc, *(T_U8 *)&s_UdiskConfigDesc);
    cnt += *(T_U8 *)&s_UdiskConfigDesc;
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskInterfacefDesc, *(T_U8 *)&s_UdiskInterfacefDesc);
    cnt += *(T_U8 *)&s_UdiskInterfacefDesc;
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskEp2Desc, *(T_U8 *)&s_UdiskEp2Desc);
    cnt += *(T_U8 *)&s_UdiskEp2Desc;
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskEp1Desc, *(T_U8 *)&s_UdiskEp1Desc);
    cnt += *(T_U8 *)&s_UdiskEp1Desc;
    *count = cnt;
    
    return configdata;
}

/** 
 * @brief get other speed config descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of other speed config descriptor
 * @return  T_U8 *
 * @retval  addr of other speed config descriptor
 */
static T_U8 * udisk_get_other_speed_cfg_desc(T_U32 *count)
{
    T_U32 cnt = 0;
    
    memcpy(configdata, (T_U8 *)&s_UdiskOtherSpeedConfigDesc, *(T_U8 *)&s_UdiskOtherSpeedConfigDesc);
    cnt += *(T_U8 *)&s_UdiskOtherSpeedConfigDesc;
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskInterfacefDesc, *(T_U8 *)&s_UdiskInterfacefDesc);
    cnt += *(T_U8 *)&s_UdiskInterfacefDesc;
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskEp2Desc, *(T_U8 *)&s_UdiskEp2Desc);
    cnt += *(T_U8 *)&s_UdiskEp2Desc;
    //other speed is full speed,ep2 maxpktsize is 64  
    if (s_pUdisk->ulMode == USB_MODE_20)
    {
        configdata[cnt-3]=0x40;
        configdata[cnt-2]=0x00;
    }
    //other speed is high speed,ep2 maxpktsize is 512  
    else
    {
        configdata[cnt-3]=0x00;
        configdata[cnt-2]=0x02;
    }
    memcpy(configdata + cnt, (T_U8 *)&s_UdiskEp1Desc, *(T_U8 *)&s_UdiskEp1Desc);
    cnt += *(T_U8 *)&s_UdiskEp1Desc;
    //other speed is full speed,ep2 maxpktsize is 64  
    if (s_pUdisk->ulMode == USB_MODE_20)
    {
        configdata[cnt-3]=0x40;
        configdata[cnt-2]=0x00;
    }
    //other speed is high speed,ep2 maxpktsize is 512  
    else
    {
        configdata[cnt-3]=0x00;
        configdata[cnt-2]=0x02;
    }
    *count = cnt;
    return configdata;
}

/** 
 * @brief get string descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param index[in] index of string descriptor
 * @param count[out] len of stirng descriptor
 * @return  T_U8 *
 * @retval  addr of string descriptor
 */
static T_U8 *udisk_get_str_desc(T_U8 index, T_U32 *count)
{
    if(index == 0)
    {
        *count = sizeof(s_UdiskString0);
        return (T_U8 *)s_UdiskString0;
    }
    else if(index == 1)
    {
        *count = sizeof(s_UdiskString1);
        return (T_U8 *)s_UdiskString1;
    }
    else if(index == 2)
    {
        *count = sizeof(s_UdiskString2);
        return (T_U8 *)s_UdiskString2;
    }
      else if(index == 3)
    {
        *count = sizeof(s_UdiskString3);
        return (T_U8 *)s_UdiskString3;
    }
    return AK_NULL;
}
/** 
 * @brief init serial number str in device desc
 *
 * the random str is truncated to 10  characters or less
 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */
static T_VOID init_serial_number()
{
    T_U8 i = 0;
    T_U32 random = 0;
    T_CHR str[20] = {0};
    T_CHR *p = str;
    
    random = get_tick_count_ms();
    itoa(random,str);
    for (i = 2; i<=20; i+=2)
    {
        if (*p)
        {
            s_UdiskString3[i] = *p++;
        }
        else
        {
            s_UdiskString3[i] = ' ';
        }
    }
}
/** 
 * @brief reverse str
 *
 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */
static T_VOID reverse(T_CHR *s)
{
    T_CHR *c;
    T_U32  i;

    c = s + strlen(s) - 1;
    while(s < c)
    {
        i = *s;
        *s++ = *c;
        *c-- = i;
    }
}
/** 
 * @brief convert int to str
 *
 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */
static T_VOID itoa(T_U32 n, T_CHR *s)
{
    T_CHR *ptr;

    ptr = s;
    do
    {
        *ptr++ = n % 10 + '0';
    }while ((n = n / 10) > 0);
    *ptr = '\0';
    reverse(s);
}


