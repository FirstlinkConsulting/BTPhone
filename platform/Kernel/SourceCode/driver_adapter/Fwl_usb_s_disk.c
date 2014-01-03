/*******************************************************************************
 * @file    Fwl_usb_s_disk.c
 * @brief   provide operations of how to use usb disk.
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-06-30
 * @version 1.0
*******************************************************************************/
#include "fwl_usb_s_disk.h"
#include "hal_usb_s_disk.h"
#include "Fwl_osFS.h"
#include "Fwl_nandflash.h"
#include "Fwl_Mount.h"
#include "Fwl_Keypad.h"
#include "fs.h"
#include "gpio_define.h"
#include "eng_debug.h"
#include <string.h>
#include "fwl_osmalloc.h"
#include "anyka_bsp.h"
#include "arch_mmc_sd.h"
#include "Fwl_sd.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef USB_WRT_CACHE_USE_HIT
#define USB_HIT_NUM            3
#define USB_CACHE_NUM          3
#define USB_CACHE_SEC_NUM      8 
#define USB_CACHE_FULL_NUM     2
#define USB_CACHE_ALL_NUM      USB_CACHE_NUM+USB_CACHE_FULL_NUM
#define USB_HIT_NUM_DEC        USB_HIT_NUM - 1
#define USB_CACHE_SEC_NUM_DEC  USB_CACHE_SEC_NUM - 1 
#define USB_CACHE_FULL         0xff
#define USB_CACHE_EMPTY        0xff
#define USB_CACHE_MAX_HIT_CNT  0xff
#define USB_CACHE_INI_ADDR     0xffffffff

#define USB_CACHE_BUFFER_SIZE       (32*1024)
#define USB_CACEHE_FLASH_REVERSE_NUM    3
#endif

#define pVendorStr      "anyka"
#define pProductRES     "Reserve Disk"
#define pProductNAND    "Nand DISK"
#define pProductSD      "SD Disk"
#define pProductMMC     "MMC Disk"
#define pRevisionStr    "1.00"

#define TestDriverMap(DriverMap, DriverID)(((DriverMap)[(DriverID)>>3]&(1<<((DriverID)&7))))
#define SetDriverMap(DriverMap, DriverID) ((DriverMap)[(DriverID)>>3] |= (1<<((DriverID)&7)))
#define ClrDriverMap(DriverMap, DriverID) ((DriverMap)[(DriverID)>>3] &= ~(1<<((DriverID)&7)))

#define IRQ_USB_STACK_SIZE  (4096)   //4k

const T_U8 NdRes_inquiry[]=
{
    0x24,
    0x00,//direct access media

    0x80,//can remove,need more scsi command handle
    //0x00,//can't remove

    0x02,//reserved ANSI code
    0x02,//international standard
    0x1f,
    0x00,
    0x00,
    0x00,
    'R','E','S',' ',' ',' ',' ',' ',
    ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    ' ',' ',' ',' '
};

const T_U8 NdOpn_inquiry[]=
{
    0x24,
    0x00,//direct access media

    0x80,//can remove,need more scsi command handle
    //0x00,//can't remove

    0x02,//reserved ANSI code
    0x02,//international standard
    0x1f,
    0x00,
    0x00,
    0x00,
    'M','P','4',' ',' ',' ',' ',' ',
    'M','P','4',' ','P','l','a','y','e','r',' ',' ',' ',' ',' ',' ',
    'V','1','.','0'
};

#pragma arm section zidata = "_bootbss_"
static T_U8 m_Driver_Map[4] = {0};
static T_pVOID irq_stack;
//static T_BOOL ischeckirq = AK_FALSE; //目前usb slave过程禁止usb irq中断。
#pragma arm section zidata

#ifdef SUPPORT_SDCARD
const T_U8 SD_CARD_PRODUCT[16]={
    pProductSD
};


const T_U8 SDCARD_inquiry[]=
{
        0x24,
        0x00,//direct access media

        0x80,//can remove,need more scsi command handle
        //0x00,//can't remove

        0x02,//reserved ANSI code
        0x02,//international standard
        0x1f,
        0x00,
        0x00,
        0x00,
        
        ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
        ' ',' ',' ',' '
        
};

static T_U8 sd_lun_cnt = 0;

#endif

#ifdef USB_WRT_CACHE_USE_HIT
/* for accelerate usb tranmit speed */
//static T_BOOL IsWrtBootSecFg      = AK_FALSE;

typedef struct tag_WrtUSBCache
{
    T_U16 BlkCnt;
    T_U32 BlkAddr;
    T_U32 NandAddInfo;
}WRTUSBCACHE, *PWRTUSBCACHE;

typedef struct tag_Usb_Wrt
{
    T_U8 *buf;
    T_U16 SectorPerCache;
    T_U16 CacheNum;
    T_U16 BytePerCache;
    T_U16 BytePerSector;
    WRTUSBCACHE PreWrtUSBCacheInfo;
    PWRTUSBCACHE  pCache;
}USB_WRT, *PUSB_WRT;

PUSB_WRT pUsb_Wrt_Cache = AK_NULL;
T_BOOL USBAllocCacheForTranSpd(T_U32 NandAddInfo);
T_BOOL USBCachePushItem(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
T_U16 USBCacheWriteUpdateData(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
T_U16 USBCacheReadData(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
T_BOOL USBFluchCache(PUSB_WRT pUSBCache, T_U8 flag);
void USBCacheCheckUpdateCacheInfo(PUSB_WRT pUSBCache,T_U32 BlkAddr, T_U32 NandAddInfo);
#endif

T_BOOL USB_ReadNand(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
T_BOOL USB_WriteNand(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
#ifdef SUPPORT_SDCARD
T_BOOL USB_WriteSD(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 SDAddInfo);
T_BOOL USB_ReadSD(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 SDAddInfo);
#endif


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
T_BOOL Fwl_UsbDiskSetStrDesc(T_eSTR_DESC_EX index,T_CHR *str)
{
    return usbdisk_set_str_desc( index,str);
}


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
T_BOOL Fwl_UsbDiskInit(T_U32 mode)
{
    irq_stack = Fwl_DMAMalloc(IRQ_USB_STACK_SIZE);
    if (!irq_stack)
    {
        AK_DEBUG_OUTPUT("Fwl_UsbDiskInit malloc failed!!\n");
        return AK_FALSE;
    }
    
    return usbdisk_init(mode);
}

extern T_VOID set_irq_stack(T_U32 stack_addr, T_U32 stackLen);

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
T_BOOL Fwl_UsbDiskStart(T_VOID)
{
    Fwl_KeypadEnable(AK_FALSE);

    set_irq_stack((T_U32)irq_stack+IRQ_USB_STACK_SIZE, IRQ_USB_STACK_SIZE);

#if (0)//目前usb slave过程禁止usb irq中断。
    if (AK_TRUE == progmanage_is_checkirq())
    {
        ischeckirq = AK_TRUE;
        progmanage_abort_checkirq(AK_FALSE);
    }
#endif
    return usbdisk_start();
}


#pragma arm section code = "_udisk_rw_"
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
T_BOOL Fwl_UsbDiskProc(T_VOID)
{
    return usbdisk_proc();
}
#pragma arm section


/*******************************************************************************
 * @brief   disable usb disk function.
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author  Huang Xin
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_UsbDiskStop(T_VOID)
{
    usbdisk_stop();
#if (0)//目前usb slave过程禁止usb irq中断。
    progmanage_abort_checkirq(ischeckirq);
#endif
    set_irq_stack(IRQ_MODE_STACK, IRQ_MODE_STACK_SIZE);
    Fwl_DMAFree(irq_stack);

    Fwl_KeypadEnable(AK_TRUE);
}


__inline T_MEM_DEV_ID GetMMC_DevIndex(T_U8 mmcDev)
{
    if(mmcDev == DEVICE_SD)
        return MMC_SD_CARD;
    else
        return SYSTEM_STORAGE;
}

extern T_U16 Utl_StrLen(T_pCSTR strMain);
/*******************************************************************************
 * @brief   mount the access media with logic unit number,support 16 LUN
 * Detail:here support 4 LUN,if need support more, chang define in usb_mass.c
 * @author  Deng Jian
 * @date    2005-10-31
 * @return  T_BOOL
 * @retval  success=AK_TURE;
*******************************************************************************/
T_BOOL Fwl_UsbDiskAddLun(T_ACCESS_MEDIUM disk_type)
{
    T_U8 i = 0, AddIndex = 0, tmp;
    T_U8 nDriverID, nDriverCnt;
    T_LUN_INFO add_lun;
    DRIVER_INFO DriverInfo;
    T_U8 *pSDdiskString;

    if (disk_type > SDCARD_DISK)//error disk type
    {   
        AK_PRINTK("Add lun fail, unsupported medium: ", disk_type, AK_TRUE);
        return AK_FALSE;
    }
    
    memset(&add_lun, 0, sizeof(T_LUN_INFO));
    //set Vendor description
    tmp = Utl_StrLen(pVendorStr);
    tmp = tmp < INQUIRY_STR_VENDOR_SIZE ? tmp : INQUIRY_STR_VENDOR_SIZE;
    memcpy(add_lun.Vendor, pVendorStr, tmp);
    ////set Revision 
    tmp = Utl_StrLen(pRevisionStr);
    tmp = tmp < INQUIRY_STR_REVISION_SIZE ? tmp : INQUIRY_STR_REVISION_SIZE;
    memcpy(add_lun.Revision, pRevisionStr, tmp);
    
    add_lun.sense   = MEDIUM_NOSENSE;
    add_lun.FastBlkCnt = 64;

#ifdef SUPPORT_SDCARD   
    if (SDCARD_DISK == disk_type)
    {
        nDriverID = _gSdDriverInfo.DriverStartID;
        nDriverCnt = _gSdDriverInfo.DriverCnt;
        //reserve a empty disk for sd which may be pushed in later
        nDriverCnt = nDriverCnt > 0 ? nDriverCnt : 1;
        sd_lun_cnt = nDriverCnt;
        add_lun.Read    = USB_ReadSD;
        add_lun.Write   = USB_WriteSD;
        pSDdiskString   = &SD_CARD_PRODUCT[0];
    }
#endif 

    if (NANDRESERVE_ZONE == disk_type || NANDFLASH_DISK == disk_type)
    {   
        nDriverID = _gNandStartDriverID;
        nDriverCnt = _gNandDriverCnt;
        add_lun.Read    = USB_ReadNand;
        add_lun.Write   = USB_WriteNand;
        
        if (NANDRESERVE_ZONE == disk_type)
            pSDdiskString = pProductRES;
        else
            pSDdiskString = pProductNAND;
    }

    //set Product description
    tmp = Utl_StrLen(pSDdiskString);
    tmp = tmp < INQUIRY_STR_PRODUCT_SIZE ? tmp : INQUIRY_STR_PRODUCT_SIZE;
    memcpy(add_lun.Product, pSDdiskString, tmp);

    for (i = 0; i < nDriverCnt; i++)
    {
        add_lun.LunIdx      = (T_U32)disk_type + AddIndex;
        
        //Mount SD card ,but SD not mounted or no SD decteted
        if ((SDCARD_DISK == disk_type)
            && (AK_FALSE == Fwl_MemDevIsMount(MMC_SD_CARD) 
                || AK_FALSE == Fwl_DetectorGetStatus(DEVICE_SD)))
        {
            add_lun.sense = MEDIUM_NOTPRESENT;
            AK_PRINTK("empty sd slot", add_lun.LunIdx, AK_TRUE);
        }
        else
        {
            if (FS_GetDriver(&DriverInfo, nDriverID + i))
            {
                if (NANDRESERVE_ZONE == disk_type || NANDFLASH_DISK == disk_type)
                {
                    //medium unmatch
                    if (MEDIUM_SD == DriverInfo.nMainType)
                        continue;
                    
                    //Partion unmatch
                    if (NANDRESERVE_ZONE == disk_type)
                    {
                        if (DriverInfo.nSubType == USER_PARTITION)
                            continue;
                    }
                    else if (DriverInfo.nSubType == SYSTEM_PARTITION)
                    {
                        continue;
                    }
                }
                
                add_lun.LunAddInfo  = (T_U32)DriverInfo.medium;
                add_lun.BlkSize     = 1 << DriverInfo.medium->SecBit;

                if (SDCARD_DISK == disk_type)
                    add_lun.BlkCnt = DriverInfo.medium->capacity;
                else
                    add_lun.BlkCnt = FS_GetDriverCapacity_SecCnt(nDriverID + i);//DriverInfo.medium->capacity;
                    
                FS_UnInstallDriver(DriverInfo.DriverID, 1);
            }
        }
        /* Added to close MtdMapTbl to speed up(but will consume more memory.
               Fortunately, we can afford it in USB process.) when USB is coming. */
        SetDriverMap(m_Driver_Map, add_lun.LunIdx);
        usbdisk_addLUN(&add_lun);
        AddIndex++;
    }
    
    return AK_TRUE;
}

#ifdef SUPPORT_SDCARD
T_BOOL Fwl_UsbDiskChgLun(T_MEM_DEV_ID index)
{
    T_LUN_INFO change_lun;
    T_U32 i, AddIndex;
    DRIVER_INFO DriverInfo;
    T_BOOL ret=AK_TRUE;
    
    change_lun.Read = USB_ReadSD;
    change_lun.Write = USB_WriteSD;
    change_lun.FastBlkCnt = 64;

    ///memcpy(change_lun.InquiryString, SDCARD_inquiry, sizeof(SDCARD_inquiry));
    if (Fwl_MemDevIsMount(index) && (Fwl_DetectorGetStatus(DEVICE_SD)))
    {
        AddIndex = 0;
        sd_lun_cnt = _gSdDriverInfo.DriverCnt;
        for (i = 0; i < sd_lun_cnt; i++)
        {
            if (FS_GetDriver(&DriverInfo, _gSdDriverInfo.DriverStartID + i))
            {
                if((DriverInfo.nSubType == USER_PARTITION)
                    && (DriverInfo.nMainType == MEDIUM_SD))
                {                
                    change_lun.sense = MEDIUM_NOTREADY_TO_READY;
                    change_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                    change_lun.BlkSize    = 1 << DriverInfo.medium->SecBit;
                    change_lun.BlkCnt     = DriverInfo.medium->capacity;
                    change_lun.LunIdx = (T_U32)SDCARD_DISK + AddIndex;
                    if (TestDriverMap(m_Driver_Map, change_lun.LunIdx))
                    {
                        usbdisk_changeLUN(&change_lun);
                        AddIndex++;
                    }
                    else
                    {
                        #if 0 //can't support hot push to add disk
                        usbdisk_addLUN(&change_lun);
                        SetGarbageMap(m_Driver_Map, change_lun.LunIdx);
                        AddIndex++;
                        #endif
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i < sd_lun_cnt; i++)
        {
            change_lun.LunIdx = (T_U32)SDCARD_DISK  + i;
            if (TestDriverMap(m_Driver_Map, change_lun.LunIdx))
            {
                change_lun.sense = MEDIUM_NOTPRESENT; 
                usbdisk_changeLUN(&change_lun);
            }
            else
            {
                AK_PRINTK("sd no add to pc:",change_lun.LunIdx,1);
                ret = AK_FALSE;
            }
        }
        
        sd_lun_cnt = 0;
    }

    return ret;
}
#endif



#pragma arm section code = "_udisk_rw_"

T_BOOL USB_WriteNand(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U32 ret;
    #ifdef USB_WRT_CACHE_USE_HIT
    T_U32 CacheSector;
    
    if(pUsb_Wrt_Cache == AK_NULL)
    {
        USBAllocCacheForTranSpd(NandAddInfo);
    }

    if (pUsb_Wrt_Cache != AK_NULL)
    {
        CacheSector = USBCacheWriteUpdateData(pUsb_Wrt_Cache, buf, BlkAddr, BlkCnt, NandAddInfo);
        if(CacheSector)
        {
            //printf("C:%d,%d\r\n",BlkAddr,CacheSector);
            BlkAddr += CacheSector;
            BlkCnt -= CacheSector;
            buf += CacheSector * pUsb_Wrt_Cache->BytePerSector;
            if(BlkCnt == 0)
                return AK_TRUE;
        }
        if ((BlkCnt <= pUsb_Wrt_Cache->SectorPerCache))
        {
            if(BlkCnt == pUsb_Wrt_Cache->SectorPerCache && pUsb_Wrt_Cache->PreWrtUSBCacheInfo.NandAddInfo == NandAddInfo &&
                BlkAddr ==  pUsb_Wrt_Cache->PreWrtUSBCacheInfo.BlkAddr)
            {
                //we will check all cache with the buffer data
        //      USBCacheCheckUpdateCacheInfo(pUsb_Wrt_Cache, BlkAddr, NandAddInfo);
                pUsb_Wrt_Cache->PreWrtUSBCacheInfo.BlkAddr = BlkAddr + BlkCnt;
                pUsb_Wrt_Cache->PreWrtUSBCacheInfo.NandAddInfo= NandAddInfo;
                return NF_SUCCESS == ((T_PMEDIUM)NandAddInfo)->write((T_PMEDIUM)NandAddInfo, buf, BlkAddr, BlkCnt);
            }
            else
                return USBCachePushItem(pUsb_Wrt_Cache, buf, BlkAddr, BlkCnt, NandAddInfo);
        }
    }
    #else
//    AK_PRINTK("W::",BlkAddr, 0);
//    AK_PRINTK(", ",BlkCnt, 1);    
    #endif  
    //AK_PRINTK("DW",0 , 1);      
    ret = ((T_PMEDIUM)NandAddInfo)->write((T_PMEDIUM)NandAddInfo, buf, BlkAddr, BlkCnt);
    FS_SpeedupUSBNand((T_PMEDIUM)NandAddInfo, buf, BlkAddr, BlkCnt);
    if(NF_SUCCESS == ret)
    {
        return AK_TRUE;
    }    
    else
    {
        return AK_FALSE;
    }    
}

T_BOOL USB_ReadNand(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    #ifdef USB_WRT_CACHE_USE_HIT
    T_U32 CacheSector;
    //T_U32 num;
    
    if (pUsb_Wrt_Cache)
    {       
        CacheSector = USBCacheReadData(pUsb_Wrt_Cache, buf, BlkAddr, BlkCnt, NandAddInfo);
        if(CacheSector)
        {
            BlkAddr += CacheSector;
            BlkCnt -= CacheSector;
            buf += CacheSector * pUsb_Wrt_Cache->BytePerSector;
            if(BlkCnt == 0)
                return AK_TRUE;
        }
    }
    #endif
    if(NF_SUCCESS == ((T_PMEDIUM)NandAddInfo)->read((T_PMEDIUM)NandAddInfo, buf, BlkAddr, BlkCnt))
        return AK_TRUE;
    else
        return AK_FALSE;
}


#ifdef SUPPORT_SDCARD
T_BOOL USB_WriteSD(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 SDAddInfo)
{ 
    if(0 == ((T_PMEDIUM)SDAddInfo)->write((T_PMEDIUM)SDAddInfo, buf, BlkAddr, BlkCnt))
        return AK_FALSE;
    else
    {
        return AK_TRUE;
    }
}

T_BOOL USB_ReadSD(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 SDAddInfo)
{   
    if(0 == ((T_PMEDIUM)SDAddInfo)->read((T_PMEDIUM)SDAddInfo, buf, BlkAddr, BlkCnt))
        return AK_FALSE;
    else
    {
        return AK_TRUE;
    }
}


#endif


#ifdef USB_WRT_CACHE_USE_HIT

#if 0
void  printfCacheBuf(PUSB_WRT pUSBCache)
{
    T_BOOL Ret = AK_TRUE;
    T_U16 i, FindPos = 0, MaxAddr = 0;
    PWRTUSBCACHE pCurCache;
    
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX)
        {
            FindPos = 1;
        }
    }
    if(FindPos == 0)
        return;
    AK_DEBUG_OUTPUT("\r\n");
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX)
        {
            AK_DEBUG_OUTPUT("%d,%d; ",pCurCache->BlkAddr, pCurCache->BlkCnt);
        }
        else 
            AK_DEBUG_OUTPUT(" , ;");
    }
    AK_DEBUG_OUTPUT("\r\n");
    while(1);
}
#endif
T_BOOL USBAllocCacheForTranSpd(T_U32 NandAddInfo)
{
    #ifdef USB_WRT_CACHE_USE_HIT
    //T_PMEDIUM pmedium = (T_PMEDIUM)NandAddInfo;
    
    pUsb_Wrt_Cache = (PUSB_WRT)Fwl_DMAMalloc(8 * sizeof(WRTUSBCACHE) + sizeof(USB_WRT));
    if(pUsb_Wrt_Cache == AK_NULL)
    {
        AK_PRINTK("it fails to malloc usb cache!\n",0, AK_TRUE);
        return AK_FALSE;
    }
    pUsb_Wrt_Cache->BytePerCache = 4096;
    pUsb_Wrt_Cache->SectorPerCache = 8;
    pUsb_Wrt_Cache->BytePerSector= 512;
    pUsb_Wrt_Cache->CacheNum = 8;
    pUsb_Wrt_Cache->pCache = (PWRTUSBCACHE)&pUsb_Wrt_Cache[1];
    pUsb_Wrt_Cache->buf = (T_U8 *)Fwl_DMAMalloc(USB_CACHE_BUFFER_SIZE);
    if(pUsb_Wrt_Cache->buf == AK_NULL)
    {
        pUsb_Wrt_Cache = Fwl_DMAFree(pUsb_Wrt_Cache);
        AK_PRINTK("it fails to malloc usb cache!",0, AK_TRUE);
        return AK_FALSE;
    }
    AK_PRINTK("it success to init usb cache!",0,AK_TRUE);
    // AK_DEBUG_OUTPUT("BytePerCache:%d, SectorPerCache:%d,BytePerSector:%d,CacheNum%d,",
    memset(&pUsb_Wrt_Cache->PreWrtUSBCacheInfo, 0xFF, sizeof(WRTUSBCACHE));
    memset(pUsb_Wrt_Cache->pCache, 0xFF, pUsb_Wrt_Cache->CacheNum * sizeof(WRTUSBCACHE));
    //AK_PRINTK("usb alloc cache is ok!",i, AK_TRUE); 
    return AK_TRUE;
    #endif
}

T_BOOL USBFluchCacheWithStartAddr(PUSB_WRT pUSBCache, T_U16 Addr, T_U16 flag)
{
    T_BOOL Ret = AK_TRUE;
    T_U16 i, ReFlag = AK_FALSE;
    T_U32 PreAddr, PreNand;
    PWRTUSBCACHE pCurCache;
    if(pUSBCache->pCache[Addr].BlkAddr == T_U32_MAX)
        return AK_TRUE;
    if(flag == AK_FALSE && (pUSBCache->pCache[Addr].BlkCnt == pUSBCache->SectorPerCache ))
        return AK_TRUE;
    PreAddr =  pUSBCache->pCache[Addr].BlkAddr;
    PreNand =  pUSBCache->pCache[Addr].NandAddInfo ;
    for(i = Addr; i < pUSBCache->CacheNum; i ++)
    {
        if(ReFlag == AK_TRUE)
        {
            ReFlag = AK_FALSE;
            i = 0;
        }
        pCurCache = pUSBCache->pCache + i;
        if(PreAddr == pCurCache->BlkAddr && PreNand == pCurCache->NandAddInfo)
        {
            pUSBCache->PreWrtUSBCacheInfo.BlkAddr = pCurCache->BlkAddr + pCurCache->BlkCnt ;
            pUSBCache->PreWrtUSBCacheInfo.NandAddInfo = pCurCache->NandAddInfo;
            //printf("Del:%d,%d\r\n", pCurCache->BlkAddr);
            if (NF_FAIL== ((T_PMEDIUM)(pCurCache->NandAddInfo))->write((T_PMEDIUM)(pCurCache->NandAddInfo), 
                pUSBCache->buf + i * pUSBCache->BytePerCache, pCurCache->BlkAddr, pCurCache->BlkCnt))
            {
                Ret = AK_FALSE;
            }
            pCurCache->BlkAddr = T_U32_MAX;
            PreAddr = pUSBCache->PreWrtUSBCacheInfo.BlkAddr;
            ReFlag = AK_TRUE;
            i = 0;
        }
    }
    // AK_DEBUG_OUTPUT("Start\r\n");
    return Ret;
}
T_BOOL USBFluchCacheWithMutliCache(PUSB_WRT pUSBCache)
{
    T_BOOL Ret = AK_TRUE;
    T_U16 i, Addr = T_U16_MAX, MaxAddr = 0;
    T_U32 PreAddr, PreNand, flag;
    PWRTUSBCACHE pCurCache;
    // we look for the max address
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX && pCurCache->BlkAddr > MaxAddr)
        {
            MaxAddr = pCurCache->BlkAddr;
            Addr = i;
        }
    }
    if(Addr == T_U16_MAX)
        return AK_FALSE;
    // we will look for the series min address with the found max address
    for(i = 0, flag = AK_FALSE; i < pUSBCache->CacheNum; i ++)
    {
        if(flag == AK_TRUE)
        {
            i = 0;
            flag = AK_FALSE;
        }
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX && MaxAddr == pCurCache->BlkAddr + pCurCache->BlkCnt)
        {
            flag = AK_TRUE;
            Addr = i;
            i = 0;
            MaxAddr = pCurCache->BlkAddr;
        }
    }
    // we will release the series address
    if(pUSBCache->pCache[Addr].BlkAddr == T_U32_MAX)
        return AK_TRUE;
    PreAddr =  pUSBCache->pCache[Addr].BlkAddr;
    PreNand =  pUSBCache->pCache[Addr].NandAddInfo ;
    for(i = Addr, flag = AK_FALSE; i < pUSBCache->CacheNum; i ++)
    {
        if(flag == AK_TRUE)
        {
            flag = AK_FALSE;
            i = 0;
        }
        pCurCache = pUSBCache->pCache + i;
        if(PreAddr == pCurCache->BlkAddr && PreNand == pCurCache->NandAddInfo)
        {
            pUSBCache->PreWrtUSBCacheInfo.BlkAddr = pCurCache->BlkAddr + pCurCache->BlkCnt;
            pUSBCache->PreWrtUSBCacheInfo.NandAddInfo = pCurCache->NandAddInfo;
            // AK_DEBUG_OUTPUT("##%d,%d##", pCurCache->BlkAddr, pCurCache->BlkCnt);
            //printf("Del:%d,%d\r\n", pCurCache->BlkAddr);
            if (NF_FAIL== ((T_PMEDIUM)(pCurCache->NandAddInfo))->write((T_PMEDIUM)(pCurCache->NandAddInfo), 
                pUSBCache->buf + i * pUSBCache->BytePerCache, pCurCache->BlkAddr, pCurCache->BlkCnt))
            {
                Ret = AK_FALSE;
            }
            pCurCache->BlkAddr = T_U32_MAX;
            PreAddr = pUSBCache->PreWrtUSBCacheInfo.BlkAddr;
            //we will go back to find other, beacuse all items isn't sorted
            flag = AK_TRUE;
            i = 0;
        }
    }
    // AK_DEBUG_OUTPUT("mutil\r\n");
    return AK_TRUE;
}
T_BOOL USBFluchCacheWithMaxAddr(PUSB_WRT pUSBCache)
{
    T_BOOL Ret = AK_TRUE;
    T_U16 i, FindPos = T_U16_MAX, MaxAddr = 0;
    PWRTUSBCACHE pCurCache; 
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX && pCurCache->BlkAddr >= MaxAddr)
        {
            MaxAddr = pCurCache->BlkAddr;
            FindPos = i;
        }
    }
    if(FindPos == T_U16_MAX)
        return AK_FALSE;
    pCurCache = pUSBCache->pCache + FindPos;
    if (NF_FAIL== ((T_PMEDIUM)(pCurCache->NandAddInfo))->write((T_PMEDIUM)(pCurCache->NandAddInfo), 
        pUSBCache->buf + FindPos * pUSBCache->BytePerCache, pCurCache->BlkAddr, pCurCache->BlkCnt))
    {
        Ret = AK_FALSE;
    }
    pCurCache->BlkAddr = T_U32_MAX;
    return Ret;
    
}
T_BOOL USBFluchCache(PUSB_WRT pUSBCache, T_U8 flag)
{
    T_BOOL Ret = AK_TRUE, CurRet;
    T_U16 i, FreeNum;
    PWRTUSBCACHE pCurCache; 
    if( pUSBCache->PreWrtUSBCacheInfo.BlkAddr != T_U32_MAX)
    {
        // we will look for the block with the previos address
        for(i = 0; i < pUSBCache->CacheNum; i ++)
        {
            pCurCache = pUSBCache->pCache + i;
            if(pCurCache->BlkAddr == pUSBCache->PreWrtUSBCacheInfo.BlkAddr&&
                pCurCache->NandAddInfo == pUSBCache->PreWrtUSBCacheInfo.NandAddInfo)
            {
                CurRet = USBFluchCacheWithStartAddr(pUSBCache, i, AK_TRUE);
                if(Ret)
                    Ret = CurRet;
                if(flag == AK_FALSE)
                    return Ret;
            }
        }
    }
    #if 0
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        CurRet = USBFluchCacheWithStartAddr(pUSBCache, i, AK_FALSE);
        if(Ret)
            Ret = CurRet;
    }
    #else
    if((USBFluchCacheWithMutliCache(pUSBCache) == AK_TRUE) && (flag == AK_FALSE))
        return AK_TRUE;
    #endif
    for(i = 0, FreeNum ++; i < pUSBCache->CacheNum; i ++)
    {
        pCurCache = pUSBCache->pCache + i;
        if(pCurCache->BlkAddr != T_U32_MAX)
        {
            FreeNum ++;
        }
    }
    if(flag == AK_TRUE)
         i = 0;
    else if(FreeNum != pUSBCache->CacheNum)
        i = FreeNum;
    else
        i = USB_CACEHE_FLASH_REVERSE_NUM;
    if(FreeNum > i)
    {
        for( ; FreeNum > i; FreeNum --)
        {
            CurRet = USBFluchCacheWithMaxAddr(pUSBCache);
            if(Ret)
                Ret = CurRet;
        }
    }
    return Ret;
}

T_U16 USBCacheFindEmptyPos(PUSB_WRT pUSBCache)
{
    T_U16 i;
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        if(pUSBCache->pCache[i].BlkAddr == T_U32_MAX)
            return i;
    }
    return T_U16_MAX;
}

T_BOOL USBCachePushItem(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U8 ret = AK_TRUE;
    T_U16 NewPos;
    T_U32 offset;
    //printf("ADD:%d,%d#\r\n",BlkAddr,BlkCnt);
    NewPos = USBCacheFindEmptyPos(pUSBCache);
    if(NewPos == T_U16_MAX)
    {
        ret = USBFluchCache(pUSBCache, AK_FALSE);
        NewPos = USBCacheFindEmptyPos(pUSBCache);
    }
    if(BlkCnt == pUSBCache->SectorPerCache && pUSBCache->PreWrtUSBCacheInfo.NandAddInfo == NandAddInfo &&
        BlkAddr ==  pUSBCache->PreWrtUSBCacheInfo.BlkAddr)
    {
        // AK_DEBUG_OUTPUT("#%d,%d#\r\n",BlkAddr,BlkCnt);
        pUsb_Wrt_Cache->PreWrtUSBCacheInfo.BlkAddr = BlkAddr + BlkCnt;
        pUsb_Wrt_Cache->PreWrtUSBCacheInfo.NandAddInfo= NandAddInfo;
        return NF_SUCCESS == ((T_PMEDIUM)NandAddInfo)->write((T_PMEDIUM)NandAddInfo, buf, BlkAddr, BlkCnt);
    }
    // AK_DEBUG_OUTPUT("**%d**\r\n", NewPos);
    pUSBCache->pCache[NewPos].BlkAddr = BlkAddr;
    pUSBCache->pCache[NewPos].BlkCnt = BlkCnt;
    pUSBCache->pCache[NewPos].NandAddInfo = NandAddInfo;
    offset = NewPos * pUSBCache->BytePerCache ;
    memcpy(pUSBCache->buf + offset, buf, BlkCnt * pUSBCache->BytePerSector);
//  printfCacheBuf(pUSBCache);
    return ret;
}

T_U16 USBCacheWriteUpdateCacheData(PUSB_WRT pUSBCache, T_U8 *buf,T_U16 CacheID,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U16 CacheSector = 0, NewCacheCnt;
    T_U32 StartCacheAddr, EndCacheAddr, offset;
    if(pUSBCache->pCache[CacheID].BlkAddr == T_U32_MAX)
        return 0;
    if(pUSBCache->pCache[CacheID].NandAddInfo != NandAddInfo)
    {
        return 0;
    }
    StartCacheAddr = pUSBCache->pCache[CacheID].BlkAddr;
    EndCacheAddr = StartCacheAddr + pUSBCache->SectorPerCache;
    if(BlkAddr >=StartCacheAddr && BlkAddr < EndCacheAddr)
    {
        CacheSector = EndCacheAddr - BlkAddr;
        if(BlkAddr > pUSBCache->pCache[CacheID].BlkAddr + pUSBCache->pCache[CacheID].BlkCnt)
        {
            ((T_PMEDIUM)NandAddInfo)->read((T_PMEDIUM)NandAddInfo, 
                pUSBCache->buf + CacheID * pUSBCache->BytePerCache + (pUSBCache->pCache[CacheID].BlkCnt * pUSBCache->BytePerSector), 
                pUSBCache->pCache[CacheID].BlkAddr + pUSBCache->pCache[CacheID].BlkCnt, 
                BlkAddr -( pUSBCache->pCache[CacheID].BlkAddr + pUSBCache->pCache[CacheID].BlkCnt));
            pUSBCache->pCache[CacheID].BlkCnt = (BlkAddr - pUSBCache->pCache[CacheID].BlkAddr);
        }
        if(CacheSector > BlkCnt)
            CacheSector = BlkCnt;
        offset = CacheID * pUSBCache->BytePerCache + (BlkAddr - StartCacheAddr) * pUSBCache->BytePerSector;
        memcpy(pUSBCache->buf + offset, buf, CacheSector * pUSBCache->BytePerSector);
        NewCacheCnt = BlkAddr + CacheSector -  StartCacheAddr;  
        if(pUSBCache->pCache[CacheID].BlkCnt < NewCacheCnt)
            pUSBCache->pCache[CacheID].BlkCnt = NewCacheCnt;    
//      AK_DEBUG_OUTPUT("%d@\r\n", pUSBCache->pCache[CacheID].BlkCnt);
//      printfCacheBuf(pUSBCache);
    }
    else if(BlkAddr < StartCacheAddr && 
        (BlkAddr + BlkCnt) >= (pUSBCache->pCache[CacheID].BlkAddr + pUSBCache->pCache[CacheID].BlkCnt))
    {
        pUSBCache->pCache[CacheID].BlkAddr = T_U32_MAX;
    }
    else if((BlkAddr + BlkCnt) > pUSBCache->pCache[CacheID].BlkAddr && 
        (BlkAddr + BlkCnt) <= (pUSBCache->pCache[CacheID].BlkAddr + pUSBCache->pCache[CacheID].BlkCnt))
    {
        memcpy(pUSBCache->buf + CacheID * pUSBCache->BytePerCache, 
            buf + (pUSBCache->pCache[CacheID].BlkAddr - BlkAddr) * pUSBCache->BytePerSector, 
            (BlkAddr + BlkCnt - pUSBCache->pCache[CacheID].BlkAddr) * pUSBCache->BytePerSector);
    }
    return CacheSector;
}

T_U16 USBCacheWriteUpdateData(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U16 i,CacheSector = 0, CurCacheSector, flag = AK_FALSE;
    for(i = 0; i < pUSBCache->CacheNum; i ++)
    {
        if(flag == AK_TRUE)
        {
            flag = AK_FALSE;
            i = 0;
        }
        if((CurCacheSector = USBCacheWriteUpdateCacheData(pUSBCache , buf, i, BlkAddr, BlkCnt, NandAddInfo)) != 0)
        {
            CacheSector += CurCacheSector ;
            BlkAddr += CurCacheSector;
            BlkCnt -= CurCacheSector;
            buf += CurCacheSector * pUSBCache->BytePerSector;
            if(BlkCnt == 0)
                return CacheSector;
            flag = AK_TRUE;
            i = 0;
        }
    }
    return CacheSector;
}

T_U16 USBCacheReadCacheData(PUSB_WRT pUSBCache, T_U8 *buf,T_U16 CacheID,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U16 CacheSector = 0;
    T_U32 StartCacheAddr, EndCacheAddr, offset;
    if(pUSBCache->pCache[CacheID].NandAddInfo != NandAddInfo)
        return 0;
    StartCacheAddr = pUSBCache->pCache[CacheID].BlkAddr;
    EndCacheAddr = StartCacheAddr + pUSBCache->pCache[CacheID].BlkCnt;
    if(BlkAddr >= StartCacheAddr && BlkAddr < EndCacheAddr)
    {
        CacheSector = EndCacheAddr - BlkAddr;
        if(CacheSector > BlkCnt)
            CacheSector = BlkCnt;
        offset = CacheID * pUSBCache->BytePerCache  + 
            (BlkAddr - StartCacheAddr) * pUSBCache->BytePerSector;
        memcpy(buf, pUSBCache->buf + offset, CacheSector * pUSBCache->BytePerSector);       
//      AK_DEBUG_OUTPUT("%d\r\n", CacheSector);
//      printfCacheBuf(pUSBCache);
    }
    else if(BlkAddr < StartCacheAddr && (BlkAddr + BlkCnt)  > StartCacheAddr)
    {
        ((T_PMEDIUM)NandAddInfo)->write((T_PMEDIUM)pUSBCache->pCache[CacheID].NandAddInfo, 
            pUSBCache->buf + CacheID * pUSBCache->BytePerCache, 
            pUSBCache->pCache[CacheID].BlkAddr, 
            pUSBCache->pCache[CacheID].BlkCnt);
        pUSBCache->pCache[CacheID].BlkAddr = T_U32_MAX;
    }
    return CacheSector;
}
T_U16 USBCacheReadData(PUSB_WRT pUSBCache,T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_U16 i,CacheSector = 0, CurCacheSector, flag ;
    for(i = 0, flag = AK_FALSE; i < pUSBCache->CacheNum; i ++)
    {
        if(pUSBCache->pCache[i].BlkAddr == T_U32_MAX)
            continue;
        if(flag == AK_TRUE)
        {
            flag = AK_FALSE;
            i = 0;
        }
        if((CurCacheSector = USBCacheReadCacheData(pUSBCache , buf, i, BlkAddr, BlkCnt, NandAddInfo)) != 0)
        {
            CacheSector += CurCacheSector ;
            BlkAddr += CurCacheSector;
            BlkCnt -= CurCacheSector;
            buf += CurCacheSector * pUSBCache->BytePerSector;
            if(BlkCnt == 0)
                return CacheSector;
            i = 0;
            flag = AK_TRUE;
        }
    }
    return CacheSector;
}
#endif
T_VOID usbFlushCacheForTranSpd(T_VOID)
{
    #ifdef USB_WRT_CACHE_USE_HIT
    if(pUsb_Wrt_Cache)
    {
        USBFluchCache(pUsb_Wrt_Cache, AK_TRUE);
    //  printfCacheBuf(pUsb_Wrt_Cache);
    }
    #endif
}
T_VOID usbFreeCacheForTranSpd(T_VOID)
{
    #ifdef USB_WRT_CACHE_USE_HIT
    if(pUsb_Wrt_Cache)
    {
        usbFlushCacheForTranSpd();
        pUsb_Wrt_Cache->buf = Fwl_DMAFree(pUsb_Wrt_Cache->buf);
        pUsb_Wrt_Cache = Fwl_DMAFree(pUsb_Wrt_Cache);   
        AK_PRINTK("usb free cache !\r\n",0, AK_TRUE);
    }
    #endif
}

#pragma arm section


#ifdef __cplusplus
}
#endif

