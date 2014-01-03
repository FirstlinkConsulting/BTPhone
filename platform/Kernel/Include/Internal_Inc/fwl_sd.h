#ifndef __FWL_SD_H__
#define __FWL_SD_H__

#include "anyka_types.h"
#include "Fwl_Mount.h"

typedef T_VOID * T_pSD_HANDLE;

typedef struct _SD_ID_INFO
{
    T_U8    DriverStartID;
    T_U8    DriverCnt;
    T_U8    DriverPathIndex;
}T_SD_ID_INFO, *TP_SD_ID_INFO;

extern T_SD_ID_INFO _gSdDriverInfo;


T_U8 Fwl_SD_Read(T_pSD_HANDLE handle,T_U32 src, T_U8 *databuf, T_U32 size);

T_U8 Fwl_SD_Write(T_pSD_HANDLE handle,T_U32 dest, const T_U8 *databuf, T_U32 size);

T_pSD_HANDLE Fwl_SD_Card_Init(T_MEM_DEV_ID index);


T_VOID Fwl_SD_Free(T_VOID);


T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage);

T_U32 FHA_SD_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);

T_U32 FHA_SD_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);



/************************************************************************
 * Brief£ºMount the SD driver with DrvIndx.
 * Param: T_U16 DrvIndx - The driver whose id is DrvIndx;
 * Ret  : AK_TRUE : Mount successfully.
 *        AK_FALSE: Mount failed!
**************************************************************************/
T_BOOL Fwl_MountSD(T_MEM_DEV_ID DrvIndx);

/************************************************************************
 * Brief£ºUnmount the SD driver with DrvIndx.
 * Param: T_U16 DrvIndx - The driver whose id is DrvIndx;
 * Ret  : AK_TRUE : Unmount successfully.
 *        AK_FALSE: Unmount failed!
**************************************************************************/
T_BOOL Fwl_UnMountSD(T_VOID);

T_MEM_DEV_ID Fwl_GetCurDriver(T_U16 pathname);

const T_U16 *Fwl_GetCurSDDriverPath(T_U8 *DrvCnt);

T_BOOL Fwl_SDCard_Is_Valid(T_VOID);



#endif
