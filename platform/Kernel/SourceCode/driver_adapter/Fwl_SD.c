#include "Arch_gpio.h"
#include "Gpio_define.h"
#include "Fwl_SD.h"
#include "Fwl_Mount.h"
#include "arch_mmc_sd.h"
#include "mtdlib.h"
#include "Fwl_FreqMgr.h"
#include "eng_debug.h"
#include "Fwl_detect.h"
#include "Fs.h"
#include "gbl_global.h"
#include "utils.h"
#include "fha.h"



#define SD_BUSMODE              USE_ONE_BUS
#define SDBOOT_INTERFACE_TYPE   INTERFACE_MMC1

#ifdef SUPPORT_SDCARD
SD_USB_RW_INFO g_sd_usb_rw_info = {AK_FALSE, 0, 0};


#endif
static T_U16 g_driverPath[] = {'A',':','/','\0'};

T_pCARD_HANDLE psdHandle = AK_NULL;



#pragma arm section zidata = "_bootbss1_"
T_SD_ID_INFO _gSdDriverInfo;
T_pSD_HANDLE SD_Handle = AK_NULL;
#pragma arm section zidata

#if(STORAGE_USED == SD_CARD)
extern T_MEM_DEV_ID bootSdIndex;
extern T_pCARD_HANDLE sys_sd_handle;
#endif



#ifdef OS_ANYKA

#pragma arm section code = "_fatlibrodata_" 

T_U8 Fwl_SD_Read(T_pCARD_HANDLE handle,T_U32 src, T_U8 *databuf, T_U32 size)
{
    T_BOOL bLocked = AK_FALSE , ret;
    
    bLocked = lock_valid_addr(databuf, size*512, AK_TRUE,  bLocked);
    ret = sd_read_block(handle,src, databuf, size);
    lock_valid_addr(databuf, size*512, AK_FALSE,  bLocked);
    return ret;
}


#pragma arm section code

#pragma arm section code = "_update_"

T_U8 Fwl_SD1_Read(T_U32 src, T_U8 *databuf, T_U32 size)
{  
	return Fwl_SD_Read(psdHandle,src, databuf, size);
}
#pragma arm section code

#pragma arm section code = "_fatlibrodata_w_"

T_U8 Fwl_SD_Write(T_pCARD_HANDLE handle,T_U32 dest, const T_U8 *databuf, T_U32 size)
{
    T_BOOL bLocked = AK_FALSE , ret;
    
    bLocked = lock_valid_addr(databuf, size*512, AK_TRUE,  bLocked);

    ret = sd_write_block(handle, dest, databuf, size);
    lock_valid_addr(databuf, size*512, AK_FALSE,  bLocked);
    return ret;
}
#pragma arm section code 

#pragma arm section code = "_sd_ctrl_" 

T_pCARD_HANDLE Fwl_SD_Card_Init(T_MEM_DEV_ID index)
{
    T_pCARD_HANDLE pHandle=AK_NULL;

    if(SYSTEM_STORAGE == index)
    {
//1050L¾ÍÖ»ÓÐMMC2
#if(10523 == USE_CHIP_TYPE)
        pHandle = sd_initial(SDBOOT_INTERFACE_TYPE, SD_BUSMODE);   
		
#endif		
#if (1080L == USE_CHIP_TYPE)
        pHandle = sd_initial(SDBOOT_INTERFACE_TYPE, SD_BUSMODE);   
//#else
//        pHandle = sd_initial(SD_INTERFACE_TYPE, SD_BUSMODE);  
#endif

    }
    else if(MMC_SD_CARD == index)
    {
        pHandle = sd_initial(SD_INTERFACE_TYPE, SD_BUSMODE);    
        AK_PRINTK("sd_initial:", pHandle, 1);
    }
    psdHandle = pHandle;
    return pHandle;
}

#pragma arm section code 


T_VOID Fwl_SD_Free(T_VOID)
{
    if(SD_Handle != AK_NULL)
    {
        sd_free( SD_Handle);
    }    
}

#ifdef SUPPORT_SDCARD

#pragma arm section code = "_frequentcode_"
T_BOOL Fwl_SDCard_Is_Valid(T_VOID)
{
    if(T_U8_MAX == _gSdDriverInfo.DriverStartID)
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}
#pragma arm section code



#pragma arm section code = "_fatlibrodata_"

static T_U32 Fwl_SDDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size, T_MEM_DEV_ID index)
{
    if (!Fwl_DetectorGetStatus(DEVICE_SD))
    {
        AK_PRINTK("SD: OUT R", index, 1);
        return 0;
    }
    if (Fwl_SD_Read(SD_Handle,sector, buf, size))
    {
        g_sd_usb_rw_info.SD_RW_ERR_CNT = 0;
        return size;
    }
    else
    {
        if(Fwl_DetectorGetStatus(DEVICE_USB))
        {
            g_sd_usb_rw_info.bSD_USB_RW_ERR = AK_TRUE;
            if (SD_USB_RW_ERR_LIMIT != g_sd_usb_rw_info.SD_RW_ERR_CNT)
            {
                g_sd_usb_rw_info.SD_RW_ERR_CNT++;
            }
        }
        return 0;
    }
}

static T_U32 Fwl_MMCSDDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{
    return Fwl_SDDisk_Read(medium, buf, sector, size, MMC_SD_CARD);       
}

#pragma arm section code

#pragma arm section code = "_fatlibrodata_w_"


static T_U32 Fwl_SDDisk_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size, T_MEM_DEV_ID index)
{
    if (!Fwl_DetectorGetStatus(DEVICE_SD))
    {
        AK_PRINTK("SD: OUT W", index, 1);
        return 0;
    }
    
    if (Fwl_SD_Write(SD_Handle,sector, buf, size))
    {
        g_sd_usb_rw_info.SD_RW_ERR_CNT = 0;
        return size;
    }
    else
    {
        if(Fwl_DetectorGetStatus(DEVICE_USB))
        {
            g_sd_usb_rw_info.bSD_USB_RW_ERR = AK_TRUE;
            if (SD_USB_RW_ERR_LIMIT != g_sd_usb_rw_info.SD_RW_ERR_CNT)
            {
                g_sd_usb_rw_info.SD_RW_ERR_CNT++;
            }
        }
        return 0;
    }
}

static T_U32 Fwl_MMCSDDisk_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size)
{
    return Fwl_SDDisk_Write(medium, buf, sector, size, MMC_SD_CARD);       
}


#pragma arm section code

static T_BOOL Fwl_SDDisk_Initial(PDRIVER_INFO pDriverInfo, T_MEM_DEV_ID index)
{       
    T_U32 capacity = 0, BytsPerSec = 0;
    
    SD_Handle = Fwl_SD_Card_Init(index);
    if(SD_Handle)  
    {
        sd_get_info(SD_Handle, &capacity, &BytsPerSec);
    }
    else
    {
        AK_PRINTK("SD Init Error", 0, 1);
        return AK_FALSE;
    }
    pDriverInfo->nBlkCnt = capacity;
    pDriverInfo->nBlkSize = BytsPerSec;
    pDriverInfo->nMainType = MEDIUM_SD;
    pDriverInfo->nSubType = USER_PARTITION;
    pDriverInfo->fRead = Fwl_MMCSDDisk_Read;
    pDriverInfo->fWrite = Fwl_MMCSDDisk_Write;

    return AK_TRUE;
}


T_BOOL Fwl_MountSD(T_MEM_DEV_ID DrvIndx)
{ 
    DRIVER_INFO DriverInfo;

    if(!Fwl_DetectorGetStatus(DEVICE_SD) && (DrvIndx == MMC_SD_CARD))
    {
        return AK_FALSE;
    }
    
    if (_gSdDriverInfo.DriverStartID != T_U8_MAX)
    {
        return AK_TRUE;
    }
        
    if (!Fwl_SDDisk_Initial(&DriverInfo, DrvIndx))
        return AK_FALSE;
    
    if(SD_Handle)
    {
        _gSdDriverInfo.DriverStartID = FS_MountMemDev(&DriverInfo, &_gSdDriverInfo.DriverCnt, T_U8_MAX);
        if(T_U8_MAX != _gSdDriverInfo.DriverStartID)
        {
                return AK_TRUE;
        }
    }
    return AK_FALSE;
}

T_BOOL Fwl_UnMountSD()
{
    T_BOOL ret = AK_FALSE;

    if(_gSdDriverInfo.DriverStartID != T_U8_MAX)
    {
        FS_UnMountMemDev(_gSdDriverInfo.DriverStartID);
        sd_free(SD_Handle);
        _gSdDriverInfo.DriverStartID= T_U8_MAX;
        _gSdDriverInfo.DriverCnt = 0;
        AK_DEBUG_OUTPUT("UnMountSD %d\n", _gSdDriverInfo.DriverStartID);
        ret = AK_TRUE;
    }

    return ret;
}

//get the driver name and the drvcnt
const T_U16 *Fwl_GetCurSDDriverPath(T_U8 *DrvCnt)
{
    T_U8 nameIdx;
    
    nameIdx = _gSdDriverInfo.DriverStartID;
    g_driverPath[0] = 'A' + nameIdx;
    
    if(DrvCnt != AK_NULL)
    {
        *DrvCnt = _gSdDriverInfo.DriverCnt;
    }

    return g_driverPath;
}

#endif
#endif
#pragma arm section code = "_update_"
#if ((STORAGE_USED == SD_CARD) && (UPDATA_USED == 1))
T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage)
{
    return 0;  
}

/**
 * @brief:      FHA read sd card
 * @author:   ChenWeiwen
 * @date:      2008-06-06
 * @param:   nChip:chip
 * @param:   nPage:page
 * @param:   pData:data 
 * @param:   nDataLen:data length
 * @return:   T_U32:fact read data length
 */
T_U32 FHA_SD_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{  
    if(Fwl_SD_Read(sys_sd_handle, nPage, pData, nDataLen))//read sd block
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }  
}

/**
 * @brief:      FHA write sd card
 * @author:   ChenWeiwen
 * @date:      2008-06-06
 * @param:   nChip:chip
 * @param:   nPage:page
 * @param:   pData:data 
 * @param:   nDataLen:data length
 * @return:   T_U32:fact write data length
 */
T_U32 FHA_SD_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    if(Fwl_SD_Write(sys_sd_handle, nPage, pData, nDataLen))//write sd block
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }  
}
#endif //((STORAGE_USED == SD_CARD) && (UPDATA_USED == 1))
#pragma arm section code



