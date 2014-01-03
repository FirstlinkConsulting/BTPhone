
#include "Fwl_Mount.h"
#include "Fwl_sd.h"
#include "Fwl_usb_host_mount.h"
#include "eng_debug.h"
//#include "Gbl_Global.h"


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
		//for win32, mount SD disk as SYSTEM_STORAGE
        if (!Fwl_MountSD(MemId))
        {
            AK_DEBUG_OUTPUT("Error: Fwl_MountSD boot fail!\r\n");
            ret = AK_FALSE;
        }
    }
    else if(MemId == MMC_SD_CARD)
    {
        //mount sd
    }
    else if(MemId == USB_HOST_DISK)
    {
        //mount usb
    }
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

    }
    else if(MemId == USB_HOST_DISK)
    {

    }
    else
    {
        AK_DEBUG_OUTPUT("Error: Fwl_MemDevUnMount MemId is error ID\r\n");
        return AK_FALSE;
    }
    
    return AK_TRUE;
}

T_BOOL Fwl_MemDevIsMount(T_MEM_DEV_ID MemId)
{
    if(MemId >= MEMDEV_MAX_NUM)
    {
        AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_FALSE;
    }
    
	return AK_FALSE;
}

T_MEM_DEV_ID Fwl_MemDevGetDriver (T_U16 pathname)
{
    
    return SYSTEM_STORAGE;
}


const T_U16*  Fwl_MemDevGetPath (T_MEM_DEV_ID MemId, T_U8 *DrvCnt)
{      
    if(MemId >= MEMDEV_MAX_NUM)
    {
        AK_DEBUG_OUTPUT("SD Card too many!\r\n");
        return AK_NULL;
    }
    
    return AK_NULL;
   
}






