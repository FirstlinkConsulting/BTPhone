
#include "Fwl_Mount.h"
#include "Fwl_sd.h"
#include "Fwl_usb_host_mount.h"
#include "eng_debug.h"
#include "Gbl_Global.h"
#include "fwl_system.h"
#include "fwl_FreqMgr.h"

#define  WATCHDOG_SET_LONG_TIME  10 // 10+5S = 15S
#define  WATCHDOG_DISABLE_LONG_TIME  0 // 1+5S = 6S

extern T_BOOL Fwl_MountNand(T_VOID);

T_BOOL Fwl_MemDevMount (T_MEM_DEV_ID MemId)
{
	T_BOOL ret = AK_TRUE;
	
    if(MemId >= MEMDEV_MAX_NUM)
    {
        AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_FALSE;
    }
    
    Fwl_FreqPush(FREQ_APP_DEFAULT);
    if (MemId == SYSTEM_STORAGE)
    {
#if (STORAGE_USED == NAND_FLASH)
        if(AK_FALSE == Fwl_MountNand())
        {
            AK_DEBUG_OUTPUT("Error: Fwl_MountNand fail!\r\n");
            ret = AK_FALSE;
        }
#else
        if (!Fwl_MountSD(MemId))
        {
            AK_DEBUG_OUTPUT("Error: Fwl_MountSD boot fail!\r\n");
            ret = AK_FALSE;
        }
#endif
    }    
    else if(MemId == MMC_SD_CARD)
    {
		Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
        //mount sd
        if (!Fwl_MountSD(MemId))
        {
            AK_DEBUG_OUTPUT("Error: Fwl_MountSD fail!\r\n");
            ret = AK_FALSE;
        }
		Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    }
    #ifdef SUPPORT_USBHOST
    else if(MemId == USB_HOST_DISK)
    {
        Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
        //mount usb
        if (!Fwl_MountUSBDisk())
        {
            AK_DEBUG_OUTPUT("Error: Fwl_MountUSBDisk fail!\r\n");
            ret = AK_FALSE;
        }
        Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    }
    #endif
    else
    {
        AK_DEBUG_OUTPUT("Error: Fwl_MemDevMount MemId is error ID\r\n");
		ret = AK_FALSE;
    }
   
   Fwl_FreqPop();
    return ret;
}

T_BOOL Fwl_MemDevUnMount (T_MEM_DEV_ID MemId)
{
    
    if(MemId >= MEMDEV_MAX_NUM)
    {
        AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_FALSE;
    }
    

    if(MemId == MMC_SD_CARD)
    {
        if (!Fwl_UnMountSD())
        {
            AK_DEBUG_OUTPUT("Error: Fwl_UnMountSD fail!\r\n");
            return AK_FALSE;
        }
    }
    #ifdef SUPPORT_USBHOST
    else if(MemId == USB_HOST_DISK)
    {
        if (!Fwl_UnMountUSBDisk())
        {
             AK_DEBUG_OUTPUT("Error: Fwl_UnMountUSBDisk fail!\r\n");
            return AK_FALSE;
        }
    }
    #endif
    else
    {
        AK_DEBUG_OUTPUT("Error: Fwl_MemDevUnMount MemId is error ID\r\n");
        return AK_FALSE;
    }
    
    return AK_TRUE;
}

#pragma arm section code = "_frequentcode_"
T_BOOL Fwl_MemDevIsMount(T_MEM_DEV_ID MemId)
{
    if(MemId >= MEMDEV_MAX_NUM)
    {
        //AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_FALSE;
    }    

    if(MemId == MMC_SD_CARD)
    {
        if (!Fwl_SDCard_Is_Valid())
        {
            //AK_DEBUG_OUTPUT("Error: Fwl_SDCard_Is_Valid fail!\r\n");
            return AK_FALSE;
        }
    }
    #ifdef SUPPORT_USBHOST
    else if(MemId == USB_HOST_DISK)
    {
        if (!Fwl_UsbDiskIsMnt())
        {
            //AK_DEBUG_OUTPUT("Error: Fwl_UsbDiskIsMnt fail!\r\n");
            return AK_FALSE;
        }
    }
    #endif
    else
    {
        //AK_DEBUG_OUTPUT("Error: Fwl_MemDevIsMount MemId is error ID\r\n");
        return AK_FALSE;
    }
    
    return AK_TRUE;
}
#pragma arm section code

T_MEM_DEV_ID Fwl_MemDevGetDriver (T_U16 pathname)
{
  T_U32 i;
  
    for(i = 0; i < MEMDEV_MAX_NUM; i++)
    {
        if(i == MMC_SD_CARD)
        {
            // if sd is mount 
            if(Fwl_SDCard_Is_Valid())
            {
                if(pathname == *(Fwl_GetCurSDDriverPath(AK_NULL)))
                {
                    return MMC_SD_CARD;
                }
            }
        }
        #ifdef SUPPORT_USBHOST
        else if(i == USB_HOST_DISK)
        {
            //if usb disk is mount
            if(Fwl_UsbDiskIsMnt())
            {
                if(Fwl_GetCurUsbDiskID(pathname))
                {
                    return USB_HOST_DISK;
                }
            }
        }
        #endif
    }
    
    return SYSTEM_STORAGE;
}


const T_U16*  Fwl_MemDevGetPath (T_MEM_DEV_ID MemId, T_U8 *DrvCnt)
{      
    if(MemId >= MEMDEV_MAX_NUM)
    {
        AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_NULL;
    }
    
    if(MemId == MMC_SD_CARD)
    {
        // if sd is mount 
        return Fwl_GetCurSDDriverPath(DrvCnt);
    }
    #ifdef SUPPORT_USBHOST
    else if(MemId == USB_HOST_DISK)
    {
        //if usb disk is mount
        return Fwl_GetCurUsbDiskPath(0, DrvCnt);
    }
    #endif
    else
    {
        AK_DEBUG_OUTPUT("Error: Fwl_MemDevGetPath MemId is error ID\r\n");
        return AK_NULL;
    }
}


