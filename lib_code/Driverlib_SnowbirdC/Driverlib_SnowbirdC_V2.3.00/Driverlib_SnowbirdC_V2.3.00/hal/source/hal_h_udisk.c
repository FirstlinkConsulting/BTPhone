/**
 * @filename usb_h_udisk.c
 * @brief:how to use usb disk host.
 *
 * This file describe frameworks of usb disk host driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-08-31
 * @version 1.0
 */
#include <stdio.h>
#include "usb_host_drv.h"
#include "hal_h_udisk.h"
#include "hal_usb_h_std.h"
#include "hal_h_udisk_mass.h"
#include "usb_bus_drv.h"
#include "usb_common.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "hal_int_msg.h"
#include "hal_clk.h"
#include "drv_cfg.h"

#if DRV_SUPPORT_UHOST > 0


#if UHOST_USE_INTR == 0
#pragma arm section zidata = "_drvbootbss_"
#else
#pragma arm section zidata = "_usb_data_"
#endif
static volatile T_U16 s_LunReadyFlag;
static T_pfUDISK_HOST_DISCONNECT s_pfDisconnectCb;
#if UHOST_USE_INTR == 0
T_U32 connect_flag;
#endif
#pragma arm section zidata

static T_U8 s_AsicIndex = INVALID_ASIC_INDEX;

#pragma arm section zidata = "_usb_data_"
static volatile T_H_UDISK_DEV s_HostUdisk = 0;
static T_pfUDISK_HOST_CONNECT s_pfConnectCb;
#pragma arm section zidata


extern T_U32        Image$$usb_data_resident$$Base;
#define USBHOST_SECTION_BASE ((T_U32)&Image$$usb_data_resident$$Base)

extern T_U32        Image$$usb_data_resident$$Limit;
#define USBHOST_SECTION_LIMIT ((T_U32)&Image$$usb_data_resident$$Limit)


#define udisk_host_send_message(status) post_int_message(IM_UHOST, status, 0)

static T_VOID udisk_host_enum_ok_cb(T_VOID);
static T_VOID udisk_host_discon_cb(T_VOID);
static T_VOID udisk_host_connect_cb(T_VOID);
static T_VOID udisk_host_timer_interrupt(T_TIMER timer_id, T_U32 delay);



/**
 * @brief   init udisk host function
 *
 * Allocate udisk host buffer,init data strcut,register callback,open usb controller and phy.
 * @author Huang Xin
 * @date 2010-07-12
 * @param mode[in] usb mode 1.1 or 2.0
 * @return T_BOOL
 * @retval AK_FALSE init failed
 * @retval AK_TURE init successful
 */
T_BOOL udisk_host_init(T_U32 mode)
{
    T_USB_BUS_HANDLER tBusHandle = {0};

    tBusHandle.class_code = USB_DEVICE_CLASS_STORAGE;
    tBusHandle.enumok_callback = udisk_host_enum_ok_cb;
    tBusHandle.discon_callback = udisk_host_discon_cb;

    remap_lock_page(USBHOST_SECTION_BASE,USBHOST_SECTION_LIMIT-USBHOST_SECTION_BASE,AK_TRUE);    
    if (!usb_bus_reg_class(&tBusHandle))
    {
        return AK_FALSE;
    }

    //init s_pHostUdisk member
    memset((T_U8 *)&s_HostUdisk,0,sizeof(T_H_UDISK_DEV));
    s_LunReadyFlag = 0;

    //s_HostUdisk.lTimerId = ERROR_TIMER;
    if (INVALID_ASIC_INDEX == s_AsicIndex)
    {
        s_AsicIndex = clk_request_min_asic(14000000);
    }

    usb_bus_open(mode);
    return AK_TRUE;
}

T_VOID usb_bus_mode_set(T_U32 mode)
{
    usb_host_speed_sw(mode);
}

/**
 * @brief   get disk all logic unit number
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return T_U8
 * @retval  Total number of logic unit.
 */
T_U8 udisk_host_get_lun_num(T_VOID)
{
    return s_HostUdisk.ucMaxLun+1;
}

/**
 * @brief   get a logic unit number descriptor
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param   LUN[in] Index of logic unit.
 * @param   disk_info[out]  The information of the lun
 * @return  T_VOID.
 */
T_VOID udisk_host_get_lun_info(T_U32 lun, T_pH_UDISK_LUN_INFO disk_info)
{
    if(lun > s_HostUdisk.ucMaxLun || disk_info == AK_NULL)
    {
        return;
    }
    memcpy(disk_info, (T_U8 *)(&(s_HostUdisk.LunInfo[lun])), sizeof(T_H_UDISK_LUN_INFO));
}

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
#pragma arm section code = "_usb_host_"
T_U32 udisk_host_read(T_U32 lun, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U8  status = 0xff;
    T_U32 byte_offset = 0,sector_offset = 0;
    T_U32 Sectors = size;
    T_U32 SectorMax = HOST_UDISK_MAX_RW_SIZE / (s_HostUdisk.LunInfo[lun]).ulBytsPerSec;
    
    if (lun > s_HostUdisk.ucMaxLun || AK_NULL == data ||
        sector + size > (s_HostUdisk.LunInfo[lun]).ulCapacity)
    {
        drv_print("read param error", 0, AK_TRUE);
        return 0;
    }
    if ((s_LunReadyFlag & LUN_READY(lun)) == 0)
    {
        drv_print("read lun not ready", 0, AK_TRUE);
        return 0;
    }

    do
    {
        if (Sectors > SectorMax)
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_read10(lun, data + byte_offset, sector + sector_offset, SectorMax, SectorMax * (s_HostUdisk.LunInfo[lun]).ulBytsPerSec, &status))
            {
                Sectors -= SectorMax;
                sector_offset += SectorMax;
                byte_offset += SectorMax* (s_HostUdisk.LunInfo[lun]).ulBytsPerSec;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_read10(lun, data + byte_offset, sector + sector_offset, Sectors, Sectors * (s_HostUdisk.LunInfo[lun]).ulBytsPerSec, &status))
            {
                Sectors = 0;
            }
            else
            {
                break;
            }
        }
    }while(Sectors > 0);
    //s_pHostUdisk->lTimerId = timer_start(uiTIMER1, 1000, AK_TRUE, udisk_host_timer_interrupt );
    //DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    return size-Sectors;
}

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
T_U32 udisk_host_write(T_U32 lun, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U8  status = 0xff;
    T_U32 byte_offset = 0,sector_offset = 0;
    T_U32 Sectors = size;
    T_U32 SectorMax = HOST_UDISK_MAX_RW_SIZE / (s_HostUdisk.LunInfo[lun]).ulBytsPerSec;
    
    if (lun > s_HostUdisk.ucMaxLun || AK_NULL == data ||
        sector + size > (s_HostUdisk.LunInfo[lun]).ulCapacity)
    {
        drv_print("write param error", 0, AK_TRUE);
        return 0;
    }
    if ((s_LunReadyFlag & LUN_READY(lun)) == 0)
    {
        drv_print("write lun not ready", 0, AK_TRUE);
        return 0;
    }
    //DrvModule_Protect(DRV_MODULE_UDISK_HOST);
    //timer_stop(s_pHostUdisk->lTimerId);
    do
    {
        if (Sectors > SectorMax)
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_write10(lun, data + byte_offset, sector + sector_offset, SectorMax, SectorMax * (s_HostUdisk.LunInfo[lun]).ulBytsPerSec, &status))
            {
                Sectors -= SectorMax;
                sector_offset += SectorMax;
                byte_offset += SectorMax* (s_HostUdisk.LunInfo[lun]).ulBytsPerSec;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_write10(lun, data + byte_offset, sector + sector_offset, Sectors, Sectors * (s_HostUdisk.LunInfo[lun]).ulBytsPerSec, &status))
            {
                Sectors = 0;
            }
            else
            {
                break;
            }
        }
    }while(Sectors > 0);
    //s_pHostUdisk->lTimerId = timer_start(uiTIMER1, 1000, AK_TRUE, udisk_host_timer_interrupt );
    //DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    return size-Sectors;
}
#pragma arm section code 

/**
 * @brief   Udisk host set application level callback.
 *
 * This function must be called by application level before udisk host initialization.
 * @author Huang Xin
 * @date 2010-07-12
 * @param connect_callback[in] Application level callback
 * @param disconnect_callback[in] Application level callback
 * @return  T_VOID
 */
T_VOID udisk_host_set_callback(T_pfUDISK_HOST_CONNECT connect_callback, T_pfUDISK_HOST_DISCONNECT disconnect_callback)
{
    s_pfConnectCb = connect_callback;
    s_pfDisconnectCb = disconnect_callback;

    //connect intr handle send msg by app
    usb_host_set_common_intr_callback(USB_HOST_CONNECT, udisk_host_connect_cb);    
}

/**
 * @brief Udisk host disconnect function.
 *
 * This function is called by application level when eject the udisk and exit the udisk host.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  T_VOID
 */
T_VOID udisk_host_close(T_VOID)
{
    //DrvModule_Protect(DRV_MODULE_UDISK_HOST);
    //if(s_pHostUdisk->lTimerId != ERROR_TIMER)
    //timer_stop(s_pHostUdisk->lTimerId);

    //s_pHostUdisk->lTimerId = ERROR_TIMER;

    usb_bus_close();
    if (INVALID_ASIC_INDEX != s_AsicIndex)
    {
        clk_cancel_min_asic(s_AsicIndex);
        s_AsicIndex = INVALID_ASIC_INDEX;
    }
    remap_lock_page(USBHOST_SECTION_BASE,USBHOST_SECTION_LIMIT-USBHOST_SECTION_BASE,AK_FALSE);

    //DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    drv_print("usbhost close ok", 0, AK_TRUE);
}

static T_VOID udisk_host_timer_interrupt(T_TIMER timer_id, T_U32 delay)
{
    T_U32 msg_param = USB_DEVICE_CLASS_STORAGE;
    
    DrvModule_Send_Message(DRV_MODULE_USB_BUS, MESSAGE_CONNECT, &msg_param);
}

/**
 * @brief Udisk host enum ok call back func.
 *
 * This function is called by bus drv when enum success.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  T_VOID
 */
static T_VOID udisk_host_enum_ok_cb(T_VOID)
{
    T_U8 i,lun,status;
    T_U8 data[0xff] = {0};
    T_U8 sense[18] = {0};
    T_U8 retry = 0;
    T_U8 csw_status = NO_CSW_STATUS;
    T_U16 lun_ready_flag;

    

    if (!msc_get_max_lun((T_U8 *)&s_HostUdisk.ucMaxLun))
    {
        drv_print("get max lun failed:", s_HostUdisk.ucMaxLun, AK_TRUE);
    }
    drv_print("max lun:", s_HostUdisk.ucMaxLun, AK_TRUE);

    if (MAX_LUN_NUM < s_HostUdisk.ucMaxLun)
    {
        s_HostUdisk.ucMaxLun = MAX_LUN_NUM;
        drv_print("lun overtop", 0, AK_TRUE);
    }
    
    for (lun = 0; lun <= s_HostUdisk.ucMaxLun; lun++)
    {
        //inquiry
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_inquiry(lun, (T_U8 *)(s_HostUdisk.LunInfo[lun]).InquiryStr, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        drv_print("req sense failed_1:", status, AK_TRUE);
                        return; 
                    }
                }
                drv_print("retry:", retry, AK_TRUE);
                drv_print("inquiry success:", status, AK_TRUE);
                drv_print("disk name:<", 0, AK_TRUE);
                /*for(i = 8; i < DEV_STRING_BUF_LEN; i++)
                {
                    uart_write_chr(*((s_HostUdisk.LunInfo[lun]).InquiryStr + i));
                }
                drv_print(">\n",0,1);*/
                break;
            }
            else
            {
                drv_print("inquiry failed:", status, AK_TRUE);
                return; 
            }
        }
    }

    //s_HostUdisk.lTimerId = timer_start(uiTIMER1, 1000, AK_TRUE, udisk_host_timer_interrupt );

    //get lun info
    lun_ready_flag = s_LunReadyFlag;
    for (lun = 0; lun <= s_HostUdisk.ucMaxLun; lun++)
    {
        //test unit ready
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_test_unit_ready(lun, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    s_LunReadyFlag &= ~(LUN_READY(lun));
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        drv_print("req sense failed_2:", status, AK_TRUE);
                        return;
                    }
                }
                s_LunReadyFlag |= LUN_READY(lun);
                //akprintf(C1, M_DRVSYS, "retry: %d,test unit ready success: %x\n", retry,status);
                break;
            }
            else
            {
                drv_print("test unit ready failed:", status, AK_TRUE);
                return; 
            }
        }
        if (s_LunReadyFlag == lun_ready_flag)
        {
            //akprintf(C1, M_DRVSYS, "lun[%d] not change,ready flag = 0x%x\n",lun,s_LunReadyFlag);
            continue;
        }
        
        //read format capacity
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_read_format_capacity(lun, data, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        drv_print("req sense failed_3:", status, AK_TRUE);
                        return;
                    }
                }
                drv_print("retry:", retry, AK_TRUE);
                drv_print("read f_capacity success:", status, AK_TRUE);
                break;
            }
            else
            {
                drv_print("read format capacity failed:", status, AK_TRUE);
                break; 
            }
        }
        
        //read  capacity
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_read_capacity(lun, data, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        drv_print("req sense failed_4:", status, AK_TRUE);
                        return;
                    }
                }
                drv_print("retry:", retry, AK_TRUE);
                drv_print("read capacity success:", status, AK_TRUE);
                (s_HostUdisk.LunInfo[lun]).ulCapacity = 1 + ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]); 
                (s_HostUdisk.LunInfo[lun]).ulBytsPerSec = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
                drv_print("lun:", lun, AK_TRUE);
                drv_print("sector num:", (s_HostUdisk.LunInfo[lun]).ulCapacity, AK_TRUE);
                drv_print("sector size:", (s_HostUdisk.LunInfo[lun]).ulBytsPerSec, AK_TRUE);

                break;
            }
            else
            {
                drv_print("read  capacity failed:", lun, AK_TRUE);
                return;
            }
        }
    }

    if (AK_NULL != s_pfConnectCb)
    {
        //akprintf(C1, M_DRVSYS, "lun ready flag is 0x%x\n",s_LunReadyFlag);
        s_pfConnectCb(s_LunReadyFlag);
    }
    /*else
    {
        drv_print("s_pfConnectCb is null", 0, AK_TRUE);
    }*/
}

#if UHOST_USE_INTR == 0
#pragma arm section code = "_drvbootcode_"
#endif 
/**
 * @brief   Udisk host connect call back func.
 *
 * This function is called by bus drv when device is connected.
 * @author  Huang Xin
 * @date    2010-07-12
 * @param   T_VOID
 * @return  T_VOID
 */
static T_VOID udisk_host_connect_cb(T_VOID)
{
    udisk_host_send_message(AK_TRUE);
    #if UHOST_USE_INTR == 0
    connect_flag = USB_CONNECT_ENUM;
    #else
    udisk_host_connect_send_msg();
    #endif
}

/**
 * @brief   Udisk host disconnect call back func.
 *
 * This function is called by bus drv when device is disconnected.
 * @author  Huang Xin
 * @date    2010-07-12
 * @param   T_VOID
 * @return  T_VOID
 */
static T_VOID udisk_host_discon_cb(T_VOID)
{
    s_LunReadyFlag = 0;
    if (AK_NULL != s_pfDisconnectCb)
    {
        s_pfDisconnectCb();
    }
    else
    {
        udisk_host_send_message(AK_FALSE);
    }
    
    #if UHOST_USE_INTR == 0
    connect_flag = USB_COMMECT_NOT;
    #endif
}
#if UHOST_USE_INTR == 0
#pragma arm section code 
#endif


#endif //#if DRV_SUPPORT_UHOST > 0


