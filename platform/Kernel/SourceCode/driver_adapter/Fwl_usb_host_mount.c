/*******************************************************************************
 * @file    Fwl_usb_host_mount.c
 * @brief   Mount USB DISK
 * Copyright (C) 2011 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author  
 * @date    2011-7-26
 * @version 1.0
 * @ref     
*******************************************************************************/
#include "hal_usb_h_disk.h"
#include "fs.h"
#include "Mtdlib.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "anyka_types.h"
#include "Fwl_Mount.h"
#include "Fs.h"
#include "Fwl_usb_host_mount.h"
#include <string.h>


#ifdef SUPPORT_USBHOST

#define MAX_LUN_NUM     5           //只操作U盘的前逻辑单元（分区）


typedef struct
{
    PDRIVER_INFO  DevInfo;          //逻辑单元挂载信息
    T_U8          StartID;          //逻辑单元挂载成功后对应的分区号
    T_U8          PartitionNum;     //逻辑单元被分成几个子逻辑
    T_BOOL        MountIsOk;        //逻辑单元是否挂载成功
}T_ST_DISK_INFO;


static T_U8             gs_usb_disk_num = 0;            //逻辑单元个数
static T_BOOL           gs_usb_disk_is_mnt = AK_FALSE;  //U盘是否挂载成功
static T_BOOL           gs_usb_disk_changestat = AK_FALSE;
static T_ST_DISK_INFO  *gp_usb_disk_info = AK_NULL;     //逻辑单元挂载信息
static T_U16            g_UsbDiskpath[] = {'U',':','/','\0'};



static T_U32 usb_host_read(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size);
static T_U32 usb_host_write(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size);


/*******************************************************************************
 * @brief   judge usb disk is available
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  yes return AK_TRUE,no return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_UsbDiskIsMnt(T_VOID)
{
    return gs_usb_disk_is_mnt;          //mount state
}


/*******************************************************************************
 * @brief   mount usb disk .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success return AK_TRUE,fail return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_MountUSBDisk(T_VOID)
{
    T_H_UDISK_LUN_INFO disk_lun_info;           //缓存一个逻辑单元信息
    T_U32 disk_num = 0;                         //一个U盘的逻辑单元数（即分区数，一个U盘有多少个分区在做U盘时是定好了的）
    T_U32 i = 0;                                //循环变量
    //T_U32 j = 0;                                                  //循环变量
    T_U8  disk_mounted = 0;                     //成功挂载U盘的逻辑单元数

    gs_usb_disk_num = 0;                        //初始化用全局变量
    gs_usb_disk_is_mnt = AK_FALSE;

    if (gp_usb_disk_info != AK_NULL)
    {
        Fwl_Free(gp_usb_disk_info);
        gp_usb_disk_info = AK_NULL;
    }

    disk_num = udisk_host_get_lun_num();        //获取U盘逻辑单元数（即分区数）
    if (0 == disk_num)
    {
        return AK_FALSE;
    }

    if (disk_num > MAX_LUN_NUM)
    {
        disk_num = MAX_LUN_NUM;
    }

    gs_usb_disk_num = disk_num;                 //获取有效的逻辑单元数

    gp_usb_disk_info = (T_ST_DISK_INFO *)Fwl_Malloc(disk_num * sizeof(T_ST_DISK_INFO));
    if (AK_NULL == gp_usb_disk_info)
    {
        return AK_FALSE;
    }
    memset(gp_usb_disk_info, 0, disk_num * sizeof(T_ST_DISK_INFO));

    AK_DEBUG_OUTPUT("gs_usb_disk_num= %u\n", gs_usb_disk_num);
    
    for (i=0; i<disk_num; i++)                  //准备挂载disk_num个逻辑单元
    {
        gp_usb_disk_info[i].DevInfo = (DRIVER_INFO *)Fwl_Malloc(sizeof(DRIVER_INFO));
        if (AK_NULL == gp_usb_disk_info[i].DevInfo)
        {
            akerror("usb host drive error", 0, 1);
            continue;
        }

        memset(gp_usb_disk_info[i].DevInfo, 0, sizeof(DRIVER_INFO));

        memset(&disk_lun_info, 0, sizeof(T_H_UDISK_LUN_INFO));
        udisk_host_get_lun_info(i, &disk_lun_info); //获取U盘的一个分区信息

        if ((0 == disk_lun_info.ulCapacity) || (0 == disk_lun_info.ulBytsPerSec))
        {
            if (gp_usb_disk_info[i].DevInfo != AK_NULL)
            {
                Fwl_Free(gp_usb_disk_info[i].DevInfo);
                gp_usb_disk_info[i].DevInfo = AK_NULL;
            }
            continue;
        }
        
        gp_usb_disk_info[i].DevInfo->nBlkCnt = disk_lun_info.ulCapacity;
        gp_usb_disk_info[i].DevInfo->nBlkSize = disk_lun_info.ulBytsPerSec;
        gp_usb_disk_info[i].DevInfo->nMainType = MEDIUM_USBHOST;
        gp_usb_disk_info[i].DevInfo->nSubType = USER_PARTITION;
        gp_usb_disk_info[i].DevInfo->fRead = usb_host_read;
        gp_usb_disk_info[i].DevInfo->fWrite = usb_host_write;
        
        //加载U盘的一个分区，gp_usb_disk_info[i].PartitionNum返回的值为1
        gp_usb_disk_info[i].StartID = FS_MountMemDev(gp_usb_disk_info[i].DevInfo, 
                                                  &(gp_usb_disk_info[i].PartitionNum),
                                                  (T_U8)-1);

        AK_DEBUG_OUTPUT("gs_usb_disk_info[i].StartID = %d,(gs_usb_disk_info[i].PartitionNum=%d\n",
                        gp_usb_disk_info[i].StartID, gp_usb_disk_info[i].PartitionNum);

        if(T_U8_MAX == gp_usb_disk_info[i].StartID)
        {
            Fwl_Free(gp_usb_disk_info[i].DevInfo);
            gp_usb_disk_info[i].DevInfo = AK_NULL;
            continue;
        }
        
        //以下是反向验证挂载，如果无法根据StartID获得DrvInfo
        //那持载上去了也访问不了
        if(!FS_InstallDriver(gp_usb_disk_info[i].StartID, gp_usb_disk_info[i].PartitionNum))//这一句会循环很长一段时间，以后可以去掉
        {
            if (AK_FALSE == FS_UnMountMemDev(gp_usb_disk_info[i].StartID))
            {
                akerror("unmount usbdisk error", 0, 1);
            }
            
            Fwl_Free(gp_usb_disk_info[i].DevInfo);
            memset(&gp_usb_disk_info[i], 0, sizeof(gp_usb_disk_info[0]));
            gp_usb_disk_info[i].MountIsOk = AK_FALSE;
        }
       else//到这里表示持载已经成功，且可用
       {
            gp_usb_disk_info[i].MountIsOk = AK_TRUE;
            disk_mounted++;                             //成功挂载的U盘个数
       }
    }

    if (disk_mounted > 0)
    {
        gs_usb_disk_is_mnt = AK_TRUE;
        gs_usb_disk_changestat = AK_TRUE;
        akerror("Fwl_MountUSBDisk ok", 0, 1);
    }
    else
    {
        gs_usb_disk_is_mnt = AK_FALSE;
    }
    
    return gs_usb_disk_is_mnt;
}


/*******************************************************************************
 * @brief   unmount usb disk .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  success return AK_TRUE,fail return AK_FALSE
*******************************************************************************/
T_BOOL Fwl_UnMountUSBDisk(T_VOID)
{
    T_U8 i = 0;
    T_BOOL ret = AK_FALSE;

    for (i=0; i<gs_usb_disk_num; i++)
    {
        if (AK_NULL == gp_usb_disk_info[i].DevInfo)
        {
            continue;
        }

        //无论是多分区还是单分区
        //卸载时只要传起始分区就可以全卸载完
        ret = FS_UnMountMemDev(gp_usb_disk_info[i].StartID);

        if (AK_FALSE == ret)
        {
            akerror("unmount usbdisk  error", 0, 1);
        }
        
        Fwl_Free(gp_usb_disk_info[i].DevInfo);
        memset(&gp_usb_disk_info[i], 0, sizeof(gp_usb_disk_info[0]));
        akerror("unmount ok", 0, 1);
    }
    
    if (gp_usb_disk_info != AK_NULL)
    {
        Fwl_Free(gp_usb_disk_info);
        gp_usb_disk_info = AK_NULL;
    }

    gs_usb_disk_num = 0;
    gs_usb_disk_is_mnt =AK_FALSE;           //卸载成功
    gs_usb_disk_changestat = AK_TRUE;
    
    return ret;
}


const T_U16 *Fwl_GetCurUsbDiskPath(T_U8 index, T_U8 *DrvCnt)
{
    T_U8 nameIdx;

    if (AK_NULL == gp_usb_disk_info || index >= gs_usb_disk_num)
    {
        return AK_NULL;
    }

    nameIdx = gp_usb_disk_info[index].StartID;
    g_UsbDiskpath[0] = 'A' + nameIdx;
    
    if(DrvCnt != AK_NULL)
    {
        *DrvCnt = gs_usb_disk_num;
    }
    
    return g_UsbDiskpath;
}


T_BOOL Fwl_GetCurUsbDiskID(T_U16 pathname)
{
    T_U8 i;
    
    for(i = 0; i < gs_usb_disk_num; i++)
    {
        g_UsbDiskpath[0] = (gp_usb_disk_info[i].StartID) + 'A';
        
        if(pathname == g_UsbDiskpath[0])
        {
            return AK_TRUE;
        }
    }
    
    return AK_FALSE;
}


/*******************************************************************************
 * @brief   usb host read .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   [in]medium:
 * @param   [out]data:
 * @param   [in]sector:
 * @param   [in]size:
 * @return  T_U32
 * @retval  
*******************************************************************************/
static T_U32 usb_host_read(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U32 ret = 0;
    T_U32 Lun = 0;
    
    for (Lun=0; Lun<gs_usb_disk_num; Lun++)
    {
        if ((gp_usb_disk_info[Lun].DevInfo != AK_NULL)
            && (medium == gp_usb_disk_info[Lun].DevInfo->medium))
        {
            break;
        }
    }

    if (Lun == gs_usb_disk_num)
    {
        return 0;
    }
    
    ret = udisk_host_read(Lun, data, sector, size);
    
    return ret;
}


/*******************************************************************************
 * @brief   usb host write .
 * @author  mayeyu
 * @date    2011-7-27
 * @param   [in]medium:
 * @param   [in]data:
 * @param   [in]sector:
 * @param   [in]size:
 * @return  T_U32
 * @retval  
*******************************************************************************/
static T_U32 usb_host_write(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U32 ret = 0;
    T_U32 Lun = 0;

    for (Lun=0; Lun<gs_usb_disk_num; Lun++)
    {
        if ((gp_usb_disk_info[Lun].DevInfo != AK_NULL)
            && (medium == gp_usb_disk_info[Lun].DevInfo->medium))
        {
            break;
        }
    }

    if (Lun == gs_usb_disk_num)
    {
        return 0;
    }
    
    ret = udisk_host_write(Lun, data, sector, size);
    
    return ret;
}


#endif

