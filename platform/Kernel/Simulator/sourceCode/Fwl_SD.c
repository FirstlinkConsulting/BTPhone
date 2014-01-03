#include "eng_debug.h"
#include "Fs.h"
#include "Fwl_System.h"
#include "gbl_global.h"
#include "Fwl_SD.h"
#include <windows.h>

#ifdef SUPPORT_SDCARD

static T_U8 SD_DriverNo = (T_U8)'H'; //ÐéÄâµÄÅÌ·ûÃû
static T_U8 g_DrvStartID = 0, g_DrvCnt=0;
static T_U16 g_driverPath[] = {'A',':','/','\0'};

T_U8 Fwl_SD_Read(T_pSD_HANDLE handle, T_U32 BlkAddr, T_U8 *buf, T_U32 BlkCnt)
{
    HANDLE hDev;
    T_U32 dwCB;
    T_S32 high;
    T_BOOL bRet;    
    char devName[10];    //"\\\\.\\g:";
    T_U8 *buf1;

    T_U32 ret = 0;
    
    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = SD_DriverNo;
    devName[5] = ':';
    devName[6] = 0;    
    hDev = CreateFile(devName, GENERIC_READ , FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    high = BlkAddr >> (32 - 9);
    SetFilePointer(hDev, BlkAddr << 9, &high, FILE_BEGIN);

    buf1 = (T_U8 *)malloc(512 * BlkCnt);
    bRet = ReadFile(hDev, buf1, 512 * BlkCnt, &dwCB, NULL);
    CloseHandle(hDev);
    if (bRet)
    {
        ret = BlkCnt;
    }
    memcpy(buf, buf1, 512 * BlkCnt);
    free(buf1);

    return 1;
}

T_U8 Fwl_SD_Write(T_pSD_HANDLE handle, T_U32 BlkAddr, const T_U8 *buf, T_U32 BlkCnt)
{
    DWORD dwCB;
    T_S32 high;
    HANDLE hDev;    
    char devName[10];    //"\\\\.\\g:";
    T_U8 *buf1;

    T_U32 ret = 0;
    
    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = SD_DriverNo;
    devName[5] = ':';
    devName[6] = 0;    

    hDev = CreateFile(devName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE) return 0;
    high = BlkAddr >> (32 - 9);
    SetFilePointer(hDev, BlkAddr << 9, &high, FILE_BEGIN);    

    buf1 = (T_U8 *)malloc(512 * BlkCnt);
    memcpy(buf1, buf, 512 * BlkCnt);
    if (WriteFile(hDev, buf1, 512 * BlkCnt, &dwCB, NULL))
    {
        ret = BlkCnt;
    }
    CloseHandle(hDev);        
    free(buf1);

    return 1;
}

T_BOOL Fwl_SD_GetDiskSize(LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{   
	LPCTSTR lpRootPathName;
	char devName[5];

	devName[0] = SD_DriverNo;	
	devName[1] = ':';
	devName[2] = '\\';
	devName[3] = '\\';
	devName[4] = '\0';
	
	lpRootPathName = (LPCTSTR)devName;
	
	//The GetDiskFreeSpace function cannot report volume sizes that are greater than 2 GB. 
	if (GetDiskFreeSpace(lpRootPathName, lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters))
		return AK_TRUE;
	else
		return AK_FALSE;   	
}


T_BOOL Fwl_SDCard_Is_Valid()
{
    if(T_U8_MAX == g_DrvStartID)
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}

static T_U32 Fwl_SDDisk_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size)
{	
	if (Fwl_SD_Read(0, sector, buf, size))
    {
        return size;
    }
    else
    {
        return 0;
    }
}

static T_U32 Fwl_SDDisk_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size)
{
    if (Fwl_SD_Write(0, sector, (T_U8*)buf, size))
    {
        return size;
    }
    else
    {
        return 0;
    }
}

static T_BOOL Fwl_SDDisk_Initial(PDRIVER_INFO pDriverInfo)
{       
    T_U32 capacity = 0, BytsPerSec = 0;
    T_U32 SectorsPerCluster=0;		// sectors per cluster
	T_U32 BytesPerSector=0;         // bytes per sector
	T_U32 NumberOfFreeClusters=0;	// free clusters
	T_U32 TotalNumberOfClusters=0;  // total clusters
	
	if (Fwl_SD_GetDiskSize(&SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters))
	{
		capacity = TotalNumberOfClusters*SectorsPerCluster;
		BytsPerSec = BytesPerSector; 

	}
	else
	{
		capacity = 512*1024; 
		BytsPerSec = 512;
	}

    pDriverInfo->nBlkCnt = capacity;
    pDriverInfo->nBlkSize = BytsPerSec;
    pDriverInfo->nMainType = MEDIUM_SD;
    pDriverInfo->nSubType = USER_PARTITION;
    pDriverInfo->fRead = Fwl_SDDisk_Read;
    pDriverInfo->fWrite = Fwl_SDDisk_Write;

    return AK_TRUE;
}


T_BOOL Fwl_MountSD(T_MEM_DEV_ID DrvIndx)
{ 
    DRIVER_INFO DriverInfo;
    T_U8  DrvCnt=0;
        
	Fwl_SDDisk_Initial(&DriverInfo);

    g_DrvStartID = FS_MountMemDev(&DriverInfo, &DrvCnt, T_U8_MAX);
    if(T_U8_MAX != g_DrvStartID)
    {
		g_DrvCnt = DrvCnt;
        FS_InstallDriver(g_DrvStartID, DrvCnt);
		if (FS_CheckInstallDriver('A'))
			return AK_TRUE;
		else
		{
			AK_DEBUG_OUTPUT("InstallDriver Failed %d, %d\n", g_DrvStartID, DrvCnt);
			return AK_FALSE;
		}
    }
	AK_DEBUG_OUTPUT("MountSD Failed %d, %d\n", g_DrvStartID, DrvCnt);
    return AK_FALSE;
}

T_BOOL Fwl_UnMountSD()
{
    T_BOOL ret = AK_FALSE;

    if(g_DrvStartID != T_U8_MAX)
    {
        FS_UnMountMemDev(g_DrvStartID);
        AK_DEBUG_OUTPUT("UnMountSD %d\n", g_DrvStartID);
        ret = AK_TRUE;
    }

    return ret;

}

const T_U16 *Fwl_GetCurSDDriverPath(T_U8 *DrvCnt)
{     
	return g_driverPath;
}

T_MEM_DEV_ID Fwl_GetCurDriver(T_U16 pathname)
{
    T_U32 i=0;

    if(pathname == *(Fwl_GetCurSDDriverPath(AK_NULL)))
    {
        return MMC_SD_CARD;
    }
    else
		return SYSTEM_STORAGE;
}

T_U32 Fwl_GetDriverNum(T_VOID)
{
    T_U32 DriverCnt = 0;
    
    if(T_U8_MAX != g_DrvStartID)
    {
        DriverCnt += g_DrvCnt;
    }
    return DriverCnt;
}

#endif //#ifdef SUPPORT_SDCARD




