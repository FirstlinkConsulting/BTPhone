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

#define MAX_LUN_NUM     5           //ֻ����U�̵�ǰ�߼���Ԫ��������


typedef struct
{
    PDRIVER_INFO  DevInfo;          //�߼���Ԫ������Ϣ
    T_U8          StartID;          //�߼���Ԫ���سɹ����Ӧ�ķ�����
    T_U8          PartitionNum;     //�߼���Ԫ���ֳɼ������߼�
    T_BOOL        MountIsOk;        //�߼���Ԫ�Ƿ���سɹ�
}T_ST_DISK_INFO;


static T_U8             gs_usb_disk_num = 0;            //�߼���Ԫ����
static T_BOOL           gs_usb_disk_is_mnt = AK_FALSE;  //U���Ƿ���سɹ�
static T_BOOL           gs_usb_disk_changestat = AK_FALSE;
static T_ST_DISK_INFO  *gp_usb_disk_info = AK_NULL;     //�߼���Ԫ������Ϣ
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
    T_H_UDISK_LUN_INFO disk_lun_info;           //����һ���߼���Ԫ��Ϣ
    T_U32 disk_num = 0;                         //һ��U�̵��߼���Ԫ��������������һ��U���ж��ٸ���������U��ʱ�Ƕ����˵ģ�
    T_U32 i = 0;                                //ѭ������
    //T_U32 j = 0;                                                  //ѭ������
    T_U8  disk_mounted = 0;                     //�ɹ�����U�̵��߼���Ԫ��

    gs_usb_disk_num = 0;                        //��ʼ����ȫ�ֱ���
    gs_usb_disk_is_mnt = AK_FALSE;

    if (gp_usb_disk_info != AK_NULL)
    {
        Fwl_Free(gp_usb_disk_info);
        gp_usb_disk_info = AK_NULL;
    }

    disk_num = udisk_host_get_lun_num();        //��ȡU���߼���Ԫ��������������
    if (0 == disk_num)
    {
        return AK_FALSE;
    }

    if (disk_num > MAX_LUN_NUM)
    {
        disk_num = MAX_LUN_NUM;
    }

    gs_usb_disk_num = disk_num;                 //��ȡ��Ч���߼���Ԫ��

    gp_usb_disk_info = (T_ST_DISK_INFO *)Fwl_Malloc(disk_num * sizeof(T_ST_DISK_INFO));
    if (AK_NULL == gp_usb_disk_info)
    {
        return AK_FALSE;
    }
    memset(gp_usb_disk_info, 0, disk_num * sizeof(T_ST_DISK_INFO));

    AK_DEBUG_OUTPUT("gs_usb_disk_num= %u\n", gs_usb_disk_num);
    
    for (i=0; i<disk_num; i++)                  //׼������disk_num���߼���Ԫ
    {
        gp_usb_disk_info[i].DevInfo = (DRIVER_INFO *)Fwl_Malloc(sizeof(DRIVER_INFO));
        if (AK_NULL == gp_usb_disk_info[i].DevInfo)
        {
            akerror("usb host drive error", 0, 1);
            continue;
        }

        memset(gp_usb_disk_info[i].DevInfo, 0, sizeof(DRIVER_INFO));

        memset(&disk_lun_info, 0, sizeof(T_H_UDISK_LUN_INFO));
        udisk_host_get_lun_info(i, &disk_lun_info); //��ȡU�̵�һ��������Ϣ

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
        
        //����U�̵�һ��������gp_usb_disk_info[i].PartitionNum���ص�ֵΪ1
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
        
        //�����Ƿ�����֤���أ�����޷�����StartID���DrvInfo
        //�ǳ�����ȥ��Ҳ���ʲ���
        if(!FS_InstallDriver(gp_usb_disk_info[i].StartID, gp_usb_disk_info[i].PartitionNum))//��һ���ѭ���ܳ�һ��ʱ�䣬�Ժ����ȥ��
        {
            if (AK_FALSE == FS_UnMountMemDev(gp_usb_disk_info[i].StartID))
            {
                akerror("unmount usbdisk error", 0, 1);
            }
            
            Fwl_Free(gp_usb_disk_info[i].DevInfo);
            memset(&gp_usb_disk_info[i], 0, sizeof(gp_usb_disk_info[0]));
            gp_usb_disk_info[i].MountIsOk = AK_FALSE;
        }
       else//�������ʾ�����Ѿ��ɹ����ҿ���
       {
            gp_usb_disk_info[i].MountIsOk = AK_TRUE;
            disk_mounted++;                             //�ɹ����ص�U�̸���
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

        //�����Ƕ�������ǵ�����
        //ж��ʱֻҪ����ʼ�����Ϳ���ȫж����
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
    gs_usb_disk_is_mnt =AK_FALSE;           //ж�سɹ�
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

