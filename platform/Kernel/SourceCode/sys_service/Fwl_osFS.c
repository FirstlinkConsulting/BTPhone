#ifdef OS_WIN32
#include <windows.h>
#include <Windowsx.h>
#include "string.h"
#endif

#include "string.h"
#include "akdefine.h"
#include "Eng_Debug.h"
#include "Fwl_FreqMgr.h"
#include "fwl_nandflash.h"
#include "remap.h"
#include "unicode.h"
#include "Eng_mmem.h"
#include "Eng_time.h"
#include "Fwl_System.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Fwl_Timer.h"
#include "fwl_sd.h"
#include "Fwl_Mount.h"
#include "Fwl_RTC.h"
#include "Eng_UStrPublic.h"
#include "fs.h"
#include "arch_mmc_sd.h"

#define FS_ROOTNAME_HANDLE  0xfffffffe  //../符号的T_hFILEINFO的句柄
#define FS_FINDNONE_HANDLE  0xfffffffe  //搜索不到任何文件且需要显示../符号时T_hFindCtrl的句柄

#define FS_FIND_MAX_DEEP    30

#define  WATCHDOG_SET_LONG_TIME  115 // 115+5S = 120
#define  WATCHDOG_DISABLE_LONG_TIME  0 // 1+5S = 6S


T_U8 _gNandStartDriverID = 0xFF, _gNandDriverCnt = 0;
#define BYTES_PER_SECTOR    512
//T_PMEDIUM cache_medium;
//SD/MMC Card Handle Flag

#pragma arm section rodata = "_commoncode_"

const T_U16 g_diskHidePath[] = {'A',':','/','\0'};
#ifdef OS_WIN32
const T_U16 g_diskPath[] = {'A',':','/','\0'};
#else
const T_U16 g_diskPath[] = {'B',':','/','\0'};
#endif

T_U16 g_usbPath[] = {'U',':','/','\0'};

static T_VOID Fwl_RemoveEndDirSymbol(T_U16 dir[]);
static T_BOOL Fwl_IsFolder(T_hFILEINFO hFileInfo);
static T_U16* Fwl_GetName(T_hFILEINFO hFileInfo);
static T_U32 Fwl_GetLen(T_hFILEINFO hFileInfo);
extern T_U32 Fwl_GetRand(T_U32 maxVal);
extern T_VOID Fwl_RandSeed(T_VOID);
extern T_U32 g_nPagePerBlock_asa;

extern T_S32 Eng_WideCharToMultiByte(const T_U16 *unicode, T_U32 ucLen,T_CHR *pDestBuf, T_U32 destLen, const T_CHR *defaultChr);
extern T_S32 Eng_MultiByteToWideChar(const T_CHR *src, T_U32 srcLen,T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);
extern T_U16 * Utl_UStrCpy(T_U16 * strDest, const T_U16* strSour);
extern T_U16 * Utl_UStrCpyN(T_U16 * strDest, T_U16* strSour, T_U32 length);
extern T_U16 Utl_UStrLen(const T_U16* strMain);
extern T_U16 * Utl_UStrCat(T_U16 * strDest, const T_U16* strSub);
extern T_U16 * Utl_UStrCatN(T_U16 * strDest, const T_U16* strSrc, T_U32 length);
extern T_S8 Utl_UStrCmp(T_U16* str1, T_U16* str2);
extern T_S16 UStrRevFndR(const T_U16* const strFndDes, const T_U16* const strFndSrc, T_U16 offsetEnd);
extern T_BOOL UStrSub(T_U16* const subStr, const T_U16* const strSrc, T_U16 srcStart, T_U16 desLen);
extern T_U16 Utl_StrLen(T_pCSTR strMain);



const T_U16 strUCurrentFolder[4] = {'.','/',0,0};
const T_U16 strUParentFolder[4] = {'.','.','/',0};
const T_U16 strUSolidus[2] = {'/',0};
const T_U16 strURevSolidus[2]={'\\', 0};
#pragma arm section rodata


/**************************************************************************
* @brief Convert UNICODE string to ansi string, just for file system lib
* @param[in]   UniStr     source UNICODE string
* @param[in]   UniStrLen       the length of source string, in UNICODE char unit
* @param[out]  pAnsibuf        the output ansi string buffer
* @param[in]   AnsiBufLen indicate the output ansi string buffer size, in bytes
* @param[in]   code      language
* @return T_S32
* @retval if AnsiBufLen is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string
* @retval if AnsiBufLen is not zero, the return value is the number of bytes written to the buffer pointed to by pAnsi
*******************************************************************/
static T_S32 Fwl_UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
    return Eng_WideCharToMultiByte( pUniStr, UniStrLen,pAnsibuf, AnsiBufLen + 1, AK_NULL);  
}

/**********************************************************************************
* @brief Convert ansi string to UNICODE string, just for file system lib
* @param[in] pAnsiStr       source ansi string
* @param[in] AnsiStrLen    the length of source string, in bytes
* @param[out] pUniBuf    the output UNICODE string buffer
* @param[in] UniBufLen  indicate the output UNICODE string buffer size, in UNICODE unit
* @return T_S32
* @retval if ucBufLen is zero, the return value is the required size, in UNICODE char, for a buffer that can receive the translated string
* @retval if ucBufLen is not zero, the return value is the number of UNICODE chars written to the buffer pointed to by ucBuf
**********************************************************************************************/
static T_S32 Fwl_AnsiStr2UniStr(const T_pSTR pAnsiStr, T_U32 AnsiStrLen, T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
    return Eng_MultiByteToWideChar(pAnsiStr, AnsiStrLen,pUniBuf, UniBufLen + 1, AK_NULL);
}

#pragma arm section code = "_fatlibrodata_"

T_hFILE Fwl_FileOpenFromParent(T_hFILE hParent, T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr)
{
    T_hFILE file = 0;

    file = File_OpenUnicode(hParent, path, flag);
    if (!File_Exist(file))
    {
        /* Whatever mode you open the file, it fails when it doesn't exist. */
        File_Close(file);
        return FS_INVALID_HANDLE;
    }

    /* Application layer shouldn't open a folder! */
    if (File_IsFolder(file))
    {
        File_Close(file);
        return FS_INVALID_HANDLE;
    }

    return file;
}

T_BOOL Fwl_FolderClose(T_hFILE hFolder)
{
    if (FS_INVALID_HANDLE == hFolder)
    {
        return AK_FALSE;
    }

    File_Close(hFolder);
    return AK_TRUE;
}


T_hFILE Fwl_FolderOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr)
{
    T_hFILE foler = AK_NULL;

    foler = File_OpenUnicode((T_hFILE)AK_NULL, path, flag);
    if (!File_Exist(foler))
    {
        /* Whatever mode you open the file, it fails when it doesn't exist. */
        File_Close(foler);
        return FS_INVALID_HANDLE;
    }

    /* Application layer shouldn't open a folder! */
    if (!File_IsFolder(foler))
    {
        File_Close(foler);
        return FS_INVALID_HANDLE;
    }

    return foler;
}


T_hFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr)
{
    T_hFILE file = AK_NULL;

    file = File_OpenUnicode((T_hFILE)AK_NULL, path, flag);
    if (!File_Exist(file))
    {
        /* Whatever mode you open the file, it fails when it doesn't exist. */
        File_Close(file);
        return FS_INVALID_HANDLE;
    }

    /* Application layer shouldn't open a folder! */
    if (File_IsFolder(file))
    {
        File_Close(file);
        return FS_INVALID_HANDLE;
    }

    return (T_hFILE)file;
}
//Fwl_SetFileSize只有在写文件时才会申请NAND容量,所以只要没写文件之前,获取剩余空间会是准确的
T_BOOL Fwl_SetFileSize(T_hFILE hFile, T_U32 fileSize)
{


    if (hFile != FS_INVALID_HANDLE)
    {
        /* Return the current read/write position. */
        return File_SetFileSize(hFile, fileSize);
    }
    return AK_FALSE;
}

T_U32 Fwl_FileTell(T_hFILE hFile)
{

    T_U32 high = 0;
    
    if (hFile != FS_INVALID_HANDLE)
    {
        // Return the current read/write position. 
        return File_GetFilePtr(hFile, &high);
    }
    
    return (T_U32)(-1);
}


T_BOOL Fwl_FileFlush(T_hFILE hFile)
{
    File_Flush(hFile);
    return AK_TRUE;
}


T_U32 Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count)
{
    /* Other para will be checked in File_ReadBlock. */
    if (FS_INVALID_HANDLE == hFile)
    {
        return 0;
    }

    return File_Read(hFile, (T_U8*)buffer, count);
}

T_U32 Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin)
{
    return File_SetFilePtr(hFile, offset, origin);
}

T_U32 Fwl_FileLongSeek(T_hFILE hFile, T_U32 offset, T_U16 origin)
{
    T_U32 ret;

    switch (origin)
    {
        case FS_SEEK_SET:
            if(0x7FFFFFFF >= offset)
            {
                return File_SetFilePtr(hFile , (T_S32)offset , FS_SEEK_SET);
            }
            else
            {
                ret = File_SetFilePtr(hFile , (T_S32)0x7FFFFFFF , FS_SEEK_SET);
                ret += File_SetFilePtr(hFile , (T_S32)(offset - 0x7FFFFFFF), FS_SEEK_CUR);
                return ret;
            }
            break;
        case FS_SEEK_CUR:
            return File_SetFilePtr(hFile , (T_S32)offset, FS_SEEK_CUR);
            break;
        case FS_SEEK_END:
            return File_SetFilePtr(hFile , (T_S32)offset , FS_SEEK_END);
            break;
        default:
            return (T_U32)(-1);
            break;
    }
}
#pragma arm section code

#pragma arm section code = "_fatlibrodata_w_"
T_U32 Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count)
{
    if (FS_INVALID_HANDLE == hFile)
    {
        return 0;
    }

    return File_Write(hFile, (T_U8*)buffer, count);
}

#pragma arm section code


T_BOOL Fwl_FileSetBufSize(T_hFILE hFile, T_U32 SecCnt)
{
    if (FS_INVALID_HANDLE == hFile)
    {
        return AK_FALSE;
    }
    return File_SetBufferSize(hFile, SecCnt);
}

T_BOOL Fwl_FileClose(T_hFILE hFile)
{
    if (FS_INVALID_HANDLE == hFile)
    {
        return AK_FALSE;
    }

    File_Close(hFile);
    return AK_TRUE;
}

T_U32 Fwl_GetFileLen(T_hFILE hFile)
{
    if (FS_INVALID_HANDLE == hFile)
    {
        return 0;
    }

    return File_GetLength(hFile, AK_NULL);
}

T_BOOL Fwl_GetFileCreatTime(T_hFILE hFile, T_pSYSTIME fileTime)
{
    T_FILETIME time;
    T_BOOL ret;
    
    if (FS_INVALID_HANDLE == hFile)
    {
        return AK_FALSE;
    }
    ret = File_GetCreateTime(hFile, &time);

    fileTime->year = time.year;
    fileTime->month = (T_U8)(time.month);
    fileTime->day = (T_U8)(time.day);
    fileTime->hour = (T_U8)(time.hour);
    fileTime->minute = (T_U8)(time.minute);
    fileTime->second = (T_U8)(time.second);
//  fileTime->week = time.Junk; // the exfat don't output the week information
    
    return ret;
}

T_BOOL Fwl_GetFileModTime(T_hFILE hFile, T_pSYSTIME fileTime)
{
    T_FILETIME time;
    T_BOOL ret;
    
    if (FS_INVALID_HANDLE == hFile)
    {
        return AK_FALSE;
    }
    ret = File_GetModTime(hFile, &time);
    
    fileTime->year = time.year;
    fileTime->month = (T_U8)(time.month);
    fileTime->day = (T_U8)(time.day);
    fileTime->hour = (T_U8)(time.hour);
    fileTime->minute = (T_U8)(time.minute);
    fileTime->second = (T_U8)(time.second);
//  fileTime->week = time.Junk; // the exfat don't output the week information
    
    return ret;
}

T_BOOL Fwl_FileDelete(T_pCWSTR path)
{
    return File_DelUnicode(path);
}
T_BOOL Fwl_FileDeleteHandle(T_hFILE hFile)
{
    if (FS_INVALID_HANDLE == hFile)
    {
        return AK_FALSE;
    }
    return File_DelFile(hFile);
}
T_BOOL Fwl_FileStat(T_pCWSTR path, T_hFILEINFO hFileInfo)
{
    T_hFILE file = 0;
    T_PFILEINFO  pFileInfo = (T_PFILEINFO)hFileInfo;

    if (AK_NULL == pFileInfo)
    {
        return AK_FALSE;
    }

    file = File_OpenUnicode((T_hFILE)AK_NULL, path, FILE_MODE_READ);
    if (!File_Exist(file))
    {
        File_Close(file);
        return AK_FALSE;
    }
    File_GetFileinfo(file, pFileInfo);
    File_Close(file);
    return AK_TRUE;
}

T_BOOL Fwl_FileStatEx(T_hFILE hFile, T_hFILEINFO hFileInfo)
{

    T_PFILEINFO  pFileInfo = (T_PFILEINFO)hFileInfo;

    if (FS_INVALID_HANDLE == hFile || AK_NULL == pFileInfo)
    {
        return AK_FALSE;
    }


    if (!File_Exist(hFile))
    {
        return AK_FALSE;
    }

    File_GetFileinfo(hFile, pFileInfo);

    return AK_TRUE;
}
#pragma arm section code= "_mem_" 

static T_pVOID Fwl_RamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
    return (T_pVOID)Fwl_Malloc(size);
}
static T_pVOID Fwl_RamRealloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID NewRam;
    NewRam = Fwl_Malloc(size);
    memcpy(NewRam, var, size);
    Fwl_Free(var);
    return NewRam;
}
static T_pVOID Fwl_RamFree(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
    return (T_pVOID)Fwl_Free(var);
}
#pragma arm section code


/* This function should return the ChipType of anyka. Till now, FatLib has included
   AK32XX, AK36XX, AK38XX, AK78XX. When The platform transplant to another different
   anyka chip, you should modify the return the corresponding value of E_AKCHIP_FS. */
static T_U32 Fwl_FsGetChipChip(T_VOID)
{
#ifdef BOARD_SPT1052C    
    return FS_AK10XXC;
#else
    return FS_AK1080L;
#endif    
}

static T_S32 Fwl_Create_Semaphore(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_S32 Fwl_Delete_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return 0;
}
static T_S32 Fwl_Obtain_Semaphore(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline)
{
    return 0;
}
static T_S32 Fwl_Release_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_VOID Fwl_SystemReset(T_VOID)
{
#ifdef OS_ANYKA
    Fwl_SysReboot();
#endif
}

static T_VOID Fwl_SystemSleep(T_U32 ms)
{
}

static T_VOID Fwl_SetSecond(T_U32 seconds)
{
}


#if (STORAGE_USED == NAND_FLASH)
T_BOOL Fwl_MountNand(T_VOID)
{
#ifdef OS_ANYKA
    T_PNANDFLASH gNand_base;
    T_U8 *pData = AK_NULL;
    T_U32  total_blk;
    T_U32 FsStartBlock;
    
    Fwl_Nand_BadBlBufInit(); 
    
    gNand_base = Nand_Init_MTD();
    if(AK_NULL == (T_pVOID)gNand_base)
    {
        AK_DEBUG_OUTPUT("Nand_Init():init error gNand_base\n");
        return AK_FALSE;
    }

    total_blk = Fwl_Nand_GetTotalBlk(gNand_base);
    
    #if 1
    AK_DEBUG_OUTPUT("nand total blk is %d\n", total_blk);
    {
        T_U32 i;
        for (i = 0; i < total_blk; i++)
        {
            if (AK_TRUE == Nand_IsBadBlock(gNand_base, 0, i))
            {
               AK_DEBUG_OUTPUT("BB%d ", i);
           }
        }
    }
    AK_DEBUG_OUTPUT("\nBad blk scan end\n");
    #endif
    
    //Get Reserve Block
    pData = (T_U8 *)Fwl_DMAMalloc(Fwl_Nand_BytesPerSector(gNand_base));

    if(AK_NULL == pData)
    {
        return AK_FALSE;
    }
    
    if(0 != Fwl_nand_remap_read(g_nPagePerBlock_asa - 2, pData, AK_NULL, AK_FALSE))
    {
        AK_DEBUG_OUTPUT("Fwl_nand_remap_read fail \r\n");
        Fwl_DMAFree(pData);
        return AK_FALSE;
    }
    
    FsStartBlock = *((T_U32*)(pData + 24));
    Fwl_DMAFree(pData);

    _gNandStartDriverID = FS_MountNandFlash(gNand_base, FsStartBlock, AK_NULL,&_gNandDriverCnt);  
    //调用installdriver加载driver, 以防等待触发的条件才去加载（比如打开文件）增加不必要的耗时
    FS_InstallDriver(_gNandStartDriverID, _gNandDriverCnt);

    Fwl_Nand_BadBlBufFree();
    return AK_TRUE;
#endif
}
#endif

#pragma arm section code = "_sysinit_"
T_S16 Fwl_FsInit(T_VOID)
{
    T_S16 ret = 0;
    T_FSCallback FSInitInfo;

    memset(&FSInitInfo, 0, sizeof(FSInitInfo));

    //AK_DEBUG_OUTPUT("Before Global_Initial()");
    FSInitInfo.fGetSecond = Fwl_GetSecond;
    FSInitInfo.fSetSecond = Fwl_SetSecond;
    FSInitInfo.fUniToAsc  = Fwl_UniStr2AnsiStr;
    FSInitInfo.fAscToUni  = Fwl_AnsiStr2UniStr;
#ifdef DEBUG
    FSInitInfo.fPrintf    = (F_Printf)AK_DEBUG_OUTPUT;
#else
    #ifdef OS_ANYKA
        FSInitInfo.fPrintf   = AK_NULL;
    #else
        FSInitInfo.fPrintf   = (F_Printf)AK_DEBUG_OUTPUT;
    #endif
#endif

    /* Initialize the callback functions needed by MtdLib. Here VC may have several
       type-unmatched warnings, but it doesn't matter. */
    FSInitInfo.fGetChipId  = Fwl_FsGetChipChip;
    FSInitInfo.fMemCpy   = (F_MemCpy)memcpy;
    FSInitInfo.fMemSet   = (F_MemSet)memset;
    FSInitInfo.fMemMov   = (F_MemMov)memmove;
    FSInitInfo.fMemCmp   = (F_MemCmp)memcmp;
    FSInitInfo.fPrintf   = (F_Printf)AkDebugOutput;
    FSInitInfo.fRamAlloc = Fwl_RamAlloc;
    FSInitInfo.fRamFree  = Fwl_RamFree;
    FSInitInfo.fRamRealloc  = Fwl_RamRealloc;
    FSInitInfo.fSystemSleep = Fwl_SystemSleep;
    
    #ifdef OS_ANYKA
    FSInitInfo.fSysRst  = (F_MtdSysRst1)Fwl_SystemReset;
    #else
    FSInitInfo.fSysRst  = AK_NULL;
    #endif
    
    FSInitInfo.fGetRand = (F_MtdGetRand1)Fwl_GetRand;
    FSInitInfo.fRandSeed = (F_MtdRandSeed1)Fwl_RandSeed;
    FSInitInfo.fCrtSem = Fwl_Create_Semaphore;
    FSInitInfo.fDelSem = Fwl_Delete_Semaphore;
    FSInitInfo.fObtSem = Fwl_Obtain_Semaphore;
    FSInitInfo.fRelSem = Fwl_Release_Semaphore;
    FS_InitCallBack(&FSInitInfo, 512);

    if (!Fwl_MemDevMount(SYSTEM_STORAGE))
    {
        AK_DEBUG_OUTPUT("Fwl_MemDevMount nandboot fail \r\n");
        return -1;
    }
    

#ifdef SUPPORT_SDCARD      
     _gSdDriverInfo.DriverStartID = T_U8_MAX;
    _gSdDriverInfo.DriverCnt = 0;

    if(Fwl_DetectorGetStatus(DEVICE_SD))
    {
        Fwl_MemDevMount(MMC_SD_CARD);
    }
#endif

    return ret;
}
#pragma arm section code

#pragma arm section code = "_File_data_align_"

T_BOOL Fwl_FsMkDir(T_pCWSTR path)
{
    return File_MkdirsUnicode(path);
}

T_BOOL Fwl_FsMkDirs(T_pCWSTR path)
{
    return File_MkdirsUnicode(path);
}

T_BOOL Fwl_FsRmDir(T_pCWSTR path)
{
    return File_DelUnicode(path);
}

T_BOOL Fwl_FsIsDir(T_pCWSTR path)
{
    T_BOOL  Ret  = AK_FALSE;
    T_hFILE file = 0;

    file = File_OpenUnicode((T_hFILE)AK_NULL, path, FILE_MODE_READ);
    Ret  = File_IsFolder(file);
    File_Close(file);

    return Ret;
}

T_BOOL  Fwl_FsMkDirTree(T_pCWSTR path)
{
    T_USTR_FILE fullDirPathName, tmpDirName;
    T_U32   curChIdx=0, pathStrLen;
    T_U8    tmpDirCnt = 0;

    if (AK_NULL == path)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FsMkDirTree:Error Param\n");
        return AK_FALSE;
    }
    
    Utl_UStrCpy(fullDirPathName, (T_U16 *)path);
    pathStrLen = Utl_UStrLen(fullDirPathName);
    
    while( 1 )
    {
        if(UNICODE_SOLIDUS == fullDirPathName[curChIdx])
        {
            tmpDirCnt++;
            Utl_UStrCpyN(tmpDirName, fullDirPathName, curChIdx);
            
            tmpDirName[curChIdx] = 0;

            if( tmpDirCnt > 1)
            {
                if (!File_MkdirsUnicode(tmpDirName))
                {
                    AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FsMkDirTree ERROR [%d]!\n", tmpDirCnt);
                    return AK_FALSE;
                }
            }
            curChIdx++;
        }
        else
        {
            curChIdx++;
        }
        if( curChIdx == pathStrLen )
        {
            break;
        }
    }
    return AK_TRUE;
}

#pragma arm section code 

#pragma arm section code = "_listfile_handle_"

T_BOOL  Fwl_FsFindIsFolder(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo)
{
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    
    if((FS_INVALID_HANDLE == findCtrl)
        || (FS_INVALID_HANDLE == hFileInfo))
    {
        return 0;
    }


    if(AK_TRUE == pFindCtrl->bShowRoot)
    {
        if(hFileInfo == FS_ROOTNAME_HANDLE)
        {
            return AK_TRUE;
        }
    }

    return Fwl_IsFolder(hFileInfo);
}

T_U16* Fwl_FsFindGetName(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo)
{
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    
    if((FS_INVALID_HANDLE == findCtrl)
        || (FS_INVALID_HANDLE == hFileInfo))
    {
        return 0;
    }

    if(AK_TRUE == pFindCtrl->bShowRoot)
    {
        if(hFileInfo == FS_ROOTNAME_HANDLE)
        {
            pFindCtrl->pFileName[0] = '.';
            pFindCtrl->pFileName[1] = '.';          
            pFindCtrl->pFileName[2] = '/';
            pFindCtrl->pFileName[3] =  0;
            return pFindCtrl->pFileName;
        }
    }

    return Fwl_GetName(hFileInfo);
}

T_U32 Fwl_FsFindGetLen(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo)
{
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    
    if((FS_INVALID_HANDLE == findCtrl)
        || (FS_INVALID_HANDLE == hFileInfo))
    {
        return 0;
    }   

    if(AK_TRUE == pFindCtrl->bShowRoot)
    {
        if(hFileInfo == FS_ROOTNAME_HANDLE)
        {
            return 0;
        }
    }

    return Fwl_GetLen(hFileInfo);
}

T_PFILEINFO Fwl_FindInfo(T_U32 pFindCtrl, T_U32 Position, T_U32 * FileCnt, T_U32 * FolderCnt)
{
  if(pFindCtrl != 0)
   return File_FindInfo(pFindCtrl, Position, FileCnt,FolderCnt);
  else
    return AK_NULL;
}

T_hFILEINFO Fwl_FsGetFindInfo(T_hFindCtrl findCtrl, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt)
{
    T_U32 TmpFileCnt=0, TmpFolderCnt=0;
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    T_hFILEINFO fileinfo = FS_INVALID_HANDLE;

    if(FS_INVALID_HANDLE == findCtrl)   //无效句柄
    {
        return FS_INVALID_HANDLE;
    }  

    if(pFindCtrl->bShowRoot == AK_TRUE) //需要显示../符号
    {
        T_U32 pos = pFindCtrl->index + Position;
        //满足下面条件则返回../的自定义句柄
        if(pos == 0 || pos == pFindCtrl->findCnt || FS_FINDNONE_HANDLE == pFindCtrl->hFind)
        {   
            fileinfo = FS_ROOTNAME_HANDLE;
        }
        //如果缓冲区包含了0节点，则调整Position，以便上层能够读取到想要的句柄
        else if(pos > pFindCtrl->findCnt)
        {
            Position--;         
        }
        if(0 == pFindCtrl->index && pos > 0)
        {
            Position--;         
        }
    }
    
    if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)
    {
        if(FS_ROOTNAME_HANDLE != fileinfo)
        {   
            fileinfo = (T_hFILEINFO)File_FindInfo(pFindCtrl->hFind, Position, &TmpFileCnt, &TmpFolderCnt);
        }
        else
        {
            File_FindInfo(pFindCtrl->hFind, Position, &TmpFileCnt, &TmpFolderCnt);
        }
    }

    if(FileCnt)
    {
        *FileCnt = TmpFileCnt;
    }

    if(FolderCnt)
    {       
        if(AK_TRUE == pFindCtrl->bShowRoot) 
        {
            TmpFolderCnt += 1;  //加上../这个文件夹
        }
        *FolderCnt = TmpFolderCnt;
    }
    return (fileinfo == 0)? FS_INVALID_HANDLE : fileinfo;
}

//搜索时带着父句柄，这样在文件夹很多的情况下，搜索效率明显高些
//Fwl_FileFindFirstFromHandld必须与Fwl_FileFindCloseWithHandle合用
T_U32 Fwl_FileFindFirstFromHandle(T_U32 file, T_PFINDBUFCTRL pBufCtrl)
{
    T_pFindCtrl pFindCtrl = AK_NULL;
    T_U32 fileCnt=0, folderCnt=0;

    pFindCtrl = Fwl_Malloc(sizeof(T_FindCtrl));       
    if(AK_NULL == pFindCtrl)
    {
        AK_DEBUG_OUTPUT("find first malloc false\n");
        return FS_INVALID_HANDLE;
    }

    pFindCtrl->hFind = File_FindFirstFromHandle(file, pBufCtrl);
    if (0 == pFindCtrl->hFind)
    {
        if(0 != (pBufCtrl->type  & FILTER_ONLYFILE))
        {
            pFindCtrl = Fwl_Free(pFindCtrl); 
            return FS_INVALID_HANDLE;  
        }
        pFindCtrl->hFind = FS_FINDNONE_HANDLE;  //如果找不到文件，但上层需要显示../，则自定义返回句柄
    }
    
    if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)
    {
        File_FindInfo(pFindCtrl->hFind, 0, &fileCnt, &folderCnt);
    }

    pFindCtrl->findCnt = fileCnt + folderCnt;
    pFindCtrl->index = 0;
    pFindCtrl->nodeCnt = (T_U8)pBufCtrl->NodeCnt;

    if(0 != (pBufCtrl->type  & FILTER_NOTITERATE))  //是否需要循环查找
    {
        pFindCtrl->bIterate = AK_FALSE;
    }
    else
    {
        pFindCtrl->bIterate = AK_TRUE;
    }
   
    if(!(pBufCtrl->type  & FILTER_ONLYFILE))    //需要显示../
    {
        pFindCtrl->findCnt++;
        pFindCtrl->bShowRoot = AK_TRUE;
        //开辟空间保存../符号，因为上层有可能对该缓冲区进行写操作，所以不能用const
        pFindCtrl->pFileName = Fwl_Malloc(MAX_FILE_LEN*sizeof(T_U16));  
        if(AK_NULL == pFindCtrl->pFileName)
        {
            Fwl_Free(pFindCtrl);
            AK_DEBUG_OUTPUT("Fwl_FsFindGetName malloc false\n");
            File_FindClose(pFindCtrl->hFind);
            return FS_INVALID_HANDLE;
        }
    }
    else
    {
        pFindCtrl->bShowRoot = AK_FALSE;
        pFindCtrl->pFileName = AK_NULL;
    }
    
    //AK_DEBUG_OUTPUT("find first !!!\n");
    return (T_hFindCtrl)pFindCtrl;

}

T_U32 Fwl_FileFindOpen(T_U32 parent, T_U32 FileInfo)
{
    T_U32 file;

    if ((0 < parent) && (0 < FileInfo))
    {
        file = (T_U32)File_FindOpen(parent, (T_PFILEINFO)FileInfo);
    }
    
    return file;
}

T_VOID Fwl_FileFindCloseWithHandle(T_hFindCtrl findCtrl)
{
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    
    if (findCtrl != FS_INVALID_HANDLE)
    {    
        if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)
        {
            File_FindCloseWithHandle(pFindCtrl->hFind);
        }
        if(AK_NULL != pFindCtrl->pFileName)
        {
            Fwl_Free(pFindCtrl->pFileName);
        }
        Fwl_Free(pFindCtrl);
    } 
}


T_hFindCtrl Fwl_FsFindFirst(T_pCWSTR path, T_PFINDBUFCTRL pBufCtrl)
{
    T_pFindCtrl pFindCtrl = AK_NULL;
    T_U32 fileCnt=0, folderCnt=0;

    pFindCtrl = Fwl_Malloc(sizeof(T_FindCtrl));       
    if(AK_NULL == pFindCtrl)
    {
        AK_DEBUG_OUTPUT("find first malloc false\n");
        return FS_INVALID_HANDLE;
    }

    pFindCtrl->hFind = File_FindFirst(path, pBufCtrl);
    if (0 == pFindCtrl->hFind)
    {
        if(0 != (pBufCtrl->type  & FILTER_ONLYFILE))
        {
            pFindCtrl = Fwl_Free(pFindCtrl); 
            return FS_INVALID_HANDLE;  
        }
        pFindCtrl->hFind = FS_FINDNONE_HANDLE;  //如果找不到文件，但上层需要显示../，则自定义返回句柄
    }
    
    if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)
    {
        File_FindInfo(pFindCtrl->hFind, 0, &fileCnt, &folderCnt);
    }

    pFindCtrl->findCnt = fileCnt + folderCnt;
    pFindCtrl->index = 0;
    pFindCtrl->nodeCnt = (T_U8)pBufCtrl->NodeCnt;

    if(0 != (pBufCtrl->type  & FILTER_NOTITERATE))  //是否需要循环查找
    {
        pFindCtrl->bIterate = AK_FALSE;
    }
    else
    {
        pFindCtrl->bIterate = AK_TRUE;
    }
   
    if(!(pBufCtrl->type  & FILTER_ONLYFILE))    //需要显示../
    {
        pFindCtrl->findCnt++;
        pFindCtrl->bShowRoot = AK_TRUE;
        //开辟空间保存../符号，因为上层有可能对该缓冲区进行写操作，所以不能用const
        pFindCtrl->pFileName = Fwl_Malloc(MAX_FILE_LEN*sizeof(T_U16));  
        if(AK_NULL == pFindCtrl->pFileName)
        {
            Fwl_Free(pFindCtrl);
            AK_DEBUG_OUTPUT("Fwl_FsFindGetName malloc false\n");
            File_FindClose(pFindCtrl->hFind);
            return FS_INVALID_HANDLE;
        }
    }
    else
    {
        pFindCtrl->bShowRoot = AK_FALSE;
        pFindCtrl->pFileName = AK_NULL;
    }
    
    //AK_DEBUG_OUTPUT("find first !!!\n");
    return (T_hFindCtrl)pFindCtrl;
}

T_U32 Fwl_FsFindNext(T_hFindCtrl findCtrl, T_S32 Cnt)
{
    T_U32 ret = Cnt < 0 ? (-Cnt) : Cnt;
    T_pFindCtrl pFindCtrl =  (T_pFindCtrl)findCtrl;

    if (FS_INVALID_HANDLE == findCtrl 
        || FS_FINDNONE_HANDLE == pFindCtrl->hFind)  //自定义句柄只有一个文件夹，不支持findnext操作
    {
        return 0;
    }

    if(pFindCtrl->nodeCnt < pFindCtrl->findCnt)     //只有搜索到的文件大于缓冲区数量，才有必要进行findnext操作
    {
        //如果pFindCtrl->index + Cnt小于0或者大于总数量，则需要适当调整cnt以适应列表控件的需求
        if(0 > Cnt)
        {
            if(pFindCtrl->index + Cnt < 0)
            {
                Cnt = 0 - pFindCtrl->nodeCnt + pFindCtrl->index;
            }
        }
        else
        {
            if(pFindCtrl->index + Cnt + pFindCtrl->nodeCnt - 1 >=  (T_S32)pFindCtrl->findCnt)
            {
                Cnt = pFindCtrl->findCnt - pFindCtrl->index;
            }
        }
    }     
    else if(AK_TRUE == pFindCtrl->bIterate)
    {
         Cnt = 0;
    } 

    //调整pFindCtrl->index
    if(AK_TRUE == pFindCtrl->bShowRoot && 0 != Cnt)
    {
        if(0 == pFindCtrl->index && 0 < Cnt)    
        {   
            pFindCtrl->index++;
            Cnt -= 1;
        }
        else if((pFindCtrl->index + Cnt) == 0)
        {
            pFindCtrl->index--;
            Cnt += 1;
        }
    }
     
    
    if(0 != Cnt)
    {
        AK_PRINTK("next:0x", Cnt, 1);
        
        if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)  //可去掉
        {
            ret = File_FindNext(pFindCtrl->hFind, Cnt);    
        }

        if (ret != (T_U32)Cnt && ret != (T_U32)(0 - Cnt))
        {
            AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FsFindNext: %d\n", ret);
        }
        
        //调整pFindCtrl->index
        if(0 > Cnt)
        {
            pFindCtrl->index += (T_U16)(pFindCtrl->findCnt - ret);
        }
        else if(0 < Cnt)
        {
            pFindCtrl->index += (T_U16)ret;
        }
    }

    //防止溢出
    if(pFindCtrl->index >= pFindCtrl->findCnt)  
    {
        pFindCtrl->index -= (T_U16)pFindCtrl->findCnt;
    }
    
    return ret;
}

T_VOID Fwl_FsFindClose(T_hFindCtrl findCtrl)
{
    T_pFindCtrl pFindCtrl = (T_pFindCtrl)findCtrl;
    
    if (findCtrl != FS_INVALID_HANDLE)
    {    
        if(FS_FINDNONE_HANDLE != pFindCtrl->hFind)
        {
            File_FindClose(pFindCtrl->hFind);
        }
        if(AK_NULL != pFindCtrl->pFileName)
        {
            Fwl_Free(pFindCtrl->pFileName);
        }
        Fwl_Free(pFindCtrl);
    }    
    //AK_DEBUG_OUTPUT("find close !!!\n");
}


/* ************************************************************
* NAME:       Fwl_FsFindFirstEX
* FUNCTION       in order to shorten  time that find a file or folder.once found a  satisfied file or folder,function exit.
                      from "PreFDT" position of "T_U16 *path" ,start find file and folder satisfying the setting "pattern" condition  
* PARAM:      const T_U16 *path : path for find
* PARAM:      T_U16 *pattern : file format filter
* PARAM:      T_U16 *filter_pattern : folder filter
* PARAM:      T_U32 PreFDT : fdt info of the last found file
* RETURN:      >0 success, 0 fail
**************************************************************************/
T_U32 Fwl_FsFindFirstEX(const T_U16 *path, T_U16 *pattern, T_U16 *filter_pattern, T_U32 PreFDT)
{
    return File_FindFirstEX(path, pattern, filter_pattern, PreFDT);
}

/************************************************************************
* NAME:       Fwl_FsFindNextEX
* FUNCTION      use together with  Fwl_FsFindFirstEX, only find a folder and file    
* PARAM:      T_U32 pFindCtrl : retval of Fwl_FsFindFirstEX
* RETURN:      >0 success, 0 fail
**************************************************************************/
T_U32 Fwl_FsFindNextEX(T_U32 pFindCtrl)
{
    return File_FindNextEX(pFindCtrl);
}

/************************************************************************
 * NAME:        Fwl_FsFindCloseEX
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl : retval of Fwl_FsFindFirstEX
 * RETURN:     NONE
**************************************************************************/
T_VOID Fwl_FsFindCloseEX(T_U32 pFindCtrl)
{
    if (0 != pFindCtrl)
    {
        File_FindClose(pFindCtrl);
    }
}


/************************************************************************
* NAME:     Fwl_FsFindFirst_WithID
* FUNCTION read  data of the first cluster  
* PARAM:    const T_U16 *path -- the path of the file
* PARAM:     T_U32 *ret_cnt -- find the num of file
* PARAM:     T_U16 *path_buf -- get the path of the file
* PARAM:     T_PFINDBUFINFO find_fileinfo -- the info of  file  before  power down 
* RETURN:     T_U32
**************************************************************************/
T_U32 Fwl_FsFindFirst_WithID(const T_U16 *path, T_U32 *ret_cnt,  
									T_U16 *path_buf, T_PFINDBUFINFO find_fileinfo)
{
    T_U32 ret;

    //set watch dog time to 2 minute
    Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
	ret = File_FindFirst_WithID(path, ret_cnt, path_buf, find_fileinfo);
    //clean watch dog long time
    Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);

    return ret;
}


/************************************************************************
 * NAME:     Fwl_FsFindNext_WithID
 * FUNCTION read  data of the first cluster  
 * PARAM:    const T_U32 obj, -- the hande of the file
 * PARAM:     T_S32 Cnt,  -1 or 1 
 * PARAM:     T_U32 *ret_cnt        find the num of file
  * RETURN:     T_U32
**************************************************************************/
T_U32 Fwl_FsFindNext_WithID(T_U32 obj, T_S32 Cnt, T_U32 *ret_cnt)
{

    T_U32 ret;
    
    //set watch dog time to 2 minute
    Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
	ret = File_FindNext_WithID(obj, Cnt, ret_cnt);
    //clean watch dog long time
    Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);

    return ret;
}


/************************************************************************
 * NAME:     Fwl_FsGet_findfile_info_WithID
 * FUNCTION read  data of the first cluster  
 * PARAM:    T_U32 obj -- the hande of the file
 * PARAM:    T_PFILEINFO_BEFOREPOWER fileinfo
* RETURN:     t_void
**************************************************************************/
T_VOID Fwl_FsGet_findfile_info_WithID(T_U32 obj, T_PFILEINFO_BEFOREPOWER fileinfo)
{
	File_get_findfile_info_WithID(obj, fileinfo);
}



/************************************************************************
 * NAME:        Fwl_FsFindInfo_WithID
 * FUNCTION  get file info of the input pFindCtrl
 * PARAM:      T_U32 find_handle
                      T_U32 position :the file's position of pFindCtrl's blink 
                      T_U32 *FileCnt :     file count
                      T_U32 *FolderCnt :     folder count
 * RETURN:     it will return file info structure pointer 
**************************************************************************/
T_PFILEINFO Fwl_FsFindInfo_WithID(T_U32 obj, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt)
{
	return File_FindInfo_WithID(obj, Position, FileCnt, FolderCnt);
}


/************************************************************************
 * NAME:        Fwl_FsFindClose_WithID
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 obj  -- the finding hande  
 * RETURN:     NONE
**************************************************************************/
T_VOID Fwl_FsFindClose_WithID(T_U32 obj)
{
	if (0 != obj)
	{
		File_FindClose_WithID(obj);
	}
}

#pragma arm section code

T_BOOL Fwl_FsExistDeepMatch(T_pCWSTR path, T_U16 *pattern, T_U16 *outPath, T_U32 deep)
{
    T_FINDBUFCTRL bufCtrl;
    T_U32 FindHandle;
    T_U32 FileCnt = 0;
    T_U32 FolderCnt = 0;
#ifdef SUPPORT_MUSIC_PLAY
    //T_U32 count = 0;
#endif
    T_hFILEINFO hFileInfo;
    T_U16 SubDirLen;
    T_BOOL FindFlag = AK_FALSE;
    
    AK_ASSERT_PTR(outPath, "DeepMatch", AK_FALSE);

    if(path == AK_NULL)
    {
        return AK_FALSE;
    }

    if(deep >= FS_FIND_MAX_DEEP)
    {
        return AK_FALSE;
    }

    #if 0
    {
        T_U16 cnt;
        T_U32 use;

        Ram_Info(&cnt, &use);
        AK_PRINTK("fs find deep: 0x", deep, 0);
        AK_PRINTK(" cnt: 0x", cnt, 0);
        AK_PRINTK(" use: 0x", use, 1);
    }
    #endif

    bufCtrl.NodeCnt = 1; 
    bufCtrl.pattern = pattern;
    bufCtrl.patternLen = UStrLen(pattern);
    bufCtrl.type = FILTER_NOTITERATE | FILTER_FOLDER; 
    
    FindHandle = File_FindFirst(path, &bufCtrl);
    
    if(0 == FindHandle)
    {
        return AK_FALSE;
    }

    if(outPath != path)
    {
        UStrCpy(outPath, path);
    }

    do
    {   
        hFileInfo = (T_hFILEINFO)File_FindInfo(FindHandle, 0, &FileCnt, &FolderCnt);

        if((0 != FileCnt) || (!Fwl_IsFolder(hFileInfo)))
        {               
            FindFlag = AK_TRUE; 
            break;
        }
        else
        {   
            SubDirLen = UStrLen(outPath);
            if(outPath[SubDirLen - 1] != '\\' && outPath[SubDirLen - 1] != '/') 
            {
                outPath[SubDirLen] = '\\';
                outPath[SubDirLen+1] = 0;               
            }
            
            if(MAX_FILE_LEN > SubDirLen + 1)
            {   //限制字符串长度
                Utl_UStrCatN(outPath, Fwl_GetName(hFileInfo), MAX_FILE_LEN - SubDirLen - 1);
                if(AK_TRUE == Fwl_FsExistDeepMatch(outPath, pattern, outPath, deep+1))
                {
                    FindFlag = AK_TRUE;
                    break;
                }
            } 
        }
        outPath[SubDirLen] = 0; 
        
    }while(File_FindNext(FindHandle, 1));
    
    File_FindClose(FindHandle); 

    return FindFlag;
}

T_VOID Fwl_FsGetSize(T_U16 DrvIndx, T_U64_INT *size64)
{
    size64->low = FS_GetDriverCapacity((T_U8)DrvIndx, &size64->high); 
}

T_VOID Fwl_FsGetUsedSize(T_U16 DrvIndx, T_U64_INT *size64)
{
    size64->low = FS_GetDriverUsedSize((T_U8)DrvIndx, &size64->high);
}

T_VOID Fwl_FsGetFreeSize(T_U16 DrvIndx, T_U64_INT *size64)
{
    size64->low = FS_GetDriverFreeSize((T_U8)DrvIndx, &size64->high);
}
 
T_BOOL Fwl_MoveFile(T_pCWSTR oldFile,T_pCWSTR newFile)
{
    T_BOOL ret = AK_FALSE;
    T_hFILE   source = 0;
    T_hFILE   dest   = 0;

    if (AK_NULL == oldFile || AK_NULL == newFile)
    {
        return AK_FALSE;
    }
    
    source = File_OpenUnicode((T_hFILE)AK_NULL, oldFile, FILE_MODE_READ);
    if (FS_INVALID_HANDLE == source)
    {
        return AK_FALSE;
    }

    dest = File_OpenUnicode((T_hFILE)AK_NULL, newFile, FILE_MODE_READ);
    if (FS_INVALID_HANDLE== dest)
    {
        File_Close(source);
        return AK_FALSE;
    }
    ret = File_RenameTo(source, dest);

    File_Close(source);
    File_Close(dest);

    return ret;
}

/************************************************************************
 * @FUNCTION：(See the following list)
       SrcStr   DstStr  Function
       file     folder  Copy file to folder; if replace, delete the file with the
                        same name under folder before copying
       file     file    Copy file's data to another; if replace, delete the dest file
                        before copying data
       folder   folder  Copy sub-files/sub-folders from SrcStr to DstStr; if replace,
                        delete any file/folder under dest folder which has the same
                        name with some file/folder under source folder
       folder   file    Forbid
 * @PARAM: T_pCSTR SrcStr : Source file's absolute path
           T_pCSTR DstStr : Dest file's absolute path
           T_BOOL  bFailIfExists: Delete or not
                   AK_TURE : When exist, not replace it and return fail.
                   AK_FALSE: When exist, replace it.
 * @RETURN: Copy successfully or not.
           AK_TRUE : Copy successfully.
           AK_FALSE: Copy failed!
**************************************************************************/
T_BOOL Fwl_CopyFile(T_pCWSTR SrcStr, T_pCWSTR DstStr, T_BOOL bFailIfExist)
{
    return File_CopyUnicode(SrcStr, DstStr, bFailIfExist, AK_NULL, AK_NULL);
}

T_BOOL Fwl_FileExist(T_pCWSTR path)
{
    T_hFILE pFile = 0;
    T_BOOL  ret = AK_FALSE;

    if (AK_NULL != path)
    {
        pFile = File_OpenUnicode((T_hFILE)AK_NULL, path, FILE_MODE_READ);

        if (FS_INVALID_HANDLE == pFile)
        {
            pFile = File_OpenUnicode((T_hFILE)AK_NULL, path, FILE_MODE_ASYN | FILE_MODE_READ);
            if (FS_INVALID_HANDLE == pFile)
            {
                //AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileExist:open fail %d\n", pFile);
                return AK_FALSE;
            }
        }
        
        ret  = File_Exist(pFile);
        File_Close(pFile);
    }
    
    return ret;
}



#pragma arm section code = "_nftllibrodata_r_"

/************************************************************************
 * NAME:     Fwl_FSCurrentRWFlag
 * FUNCTION  Get current NFTL read&write status flag (only apply to AK11 platform).
 *                  
 * PARAM:    file handle; 
 *
 * RETURN:     success return AK_TRUE,  R&W is busy
 *                   fail retuen AK_FALSE, R&W is idle
**************************************************************************/
T_BOOL Fwl_FSCurrentRWFlag(T_hFILE file)
{
    return AK_TRUE;
}
#pragma arm section code


T_BOOL Fwl_IsRootDir(T_pCWSTR pFilePath)
{
    T_USTR_FILE filePath;
    T_BOOL ret = AK_TRUE;
    T_U32 i;
    T_U32 charNo;
    T_U32 count = 0;


    Utl_UStrCpy(filePath, (T_U16 *)pFilePath);
    charNo = Utl_UStrLen(filePath);

    if (filePath[charNo - 1] != UNICODE_SOLIDUS && filePath[charNo - 1] != UNICODE_RES_SOLIDUS)
    {
        filePath[charNo] = UNICODE_SOLIDUS;
        filePath[charNo + 1] = 0;
        charNo++;
    }

    for(i=0; i<charNo; i++)
    {
        if (filePath[i] == UNICODE_SOLIDUS || filePath[i] == UNICODE_RES_SOLIDUS)
        {
            count++;
            if (count > 1)
            {
                ret = AK_FALSE;
                break;
            }
        }
    }

    return ret;
}

T_BOOL Fwl_GetRootDir(T_pCWSTR pFilePath, T_pWSTR pRootDir)
{
    T_USTR_FILE Ustr_tmp;

    AK_ASSERT_PTR(pFilePath, "Fwl_GetRootDir: pFilePath is NULL", AK_FALSE);
    AK_ASSERT_PTR(pRootDir, "Fwl_GetRootDir: pRootDir is NULL", AK_FALSE);

    Eng_MultiByteToWideChar(":\\",Utl_StrLen(":\\"),Ustr_tmp,MAX_FILE_LEN ,AK_NULL);
    Utl_UStrCpyN(pRootDir, (T_U16 *)pFilePath, 1);
    *(pRootDir + 1) = 0;
    Utl_UStrCat(pRootDir, Ustr_tmp);

    return AK_TRUE;
}


/************************************************************************
 * Copyright (c) 2008, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author：zhao_xiaowei
 * @Date：
 * @Version：
**************************************************************************/
//******************************file op*****************************************
T_BOOL Fwl_DirIsRealPath(T_U16* fromDir)
{
    if(Utl_UStrCmp(fromDir, (T_U16* )strUParentFolder) == 0 
        || Utl_UStrCmp(fromDir, (T_U16* )strUCurrentFolder) == 0)
    {
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }
}

// suppose lFileName is correct
//if lFileFile is directory ,then get the parent directory by first remove if end char is /
//else just get the directory of the file 
/**************************************************************************
* @brief if lFileName is file then get the directory else if lFileName is 
* directory then get the parent directory
* @author zhao_xiaowei
* @date 2008
* @param  longDir[out] 
* @param  lFileName[in]
* @return T_BOOL
* @retval  AK_TURE is success
* @retval 
***************************************************************************/
static T_BOOL  Fwl_GetFileDir(T_U16 longDir[], T_U16 lFileName[])
{
    T_S16 pos;
    T_U16 len;
    T_BOOL bRet;

    len= UStrLen(lFileName);
    
    if(len <= 3) //A A: A:\  disk path have no parent dir
    {
        return AK_FALSE;
    }
    else
    {
        Fwl_RemoveEndDirSymbol(lFileName);
    }
    pos= UStrRevFndR(strUSolidus, lFileName, 0);

    if(pos < 0)
    {
        pos= UStrRevFndR(strURevSolidus, lFileName, 0);

        if(pos < 0)
        {
            return AK_FALSE;
        }
    }

    if(pos == 2)
    {
        pos++; // A:/ pos=2 len=2+1
    }

    bRet= UStrSub(longDir, lFileName, 0, pos);    

    if(bRet)
    {
        bRet= Fwl_FsIsDir(longDir);//justify is file or not
    }

    return bRet;
}

T_BOOL Fwl_GetSelfParentDir(T_U16 path[])
{
    T_U16 desPath[MAX_FILE_LEN];
    T_BOOL bRet;
    bRet= Fwl_GetFileDir(desPath, path);
    if(bRet)
    {
        UStrCpy(path, desPath);
    }
    return bRet;
}

//if path if file , then call Fwl_GetFileDir, else the path is the currentDic
T_BOOL Fwl_GetCurrentDir(T_U16 currentDir[], T_U16 path[])
{
    if(Fwl_FsIsDir(path))
    {
        Fwl_RemoveEndDirSymbol(path);
        UStrCpy(currentDir, path);
        return AK_TRUE;
    }
    else
    {
        return Fwl_GetFileDir(currentDir, path);
    }
}

T_BOOL Fwl_GetSelfCurDir(T_U16 currentPath[])
{
    T_U16 tmpCurPath[MAX_FILE_LEN];
    T_BOOL ret;
    ret= Fwl_GetCurrentDir(tmpCurPath, currentPath);
    UStrCpy(currentPath, tmpCurPath);

    return ret;
}

#pragma arm section code = "_commoncode_"


/**************************************************************************
* @brief if dir is disk path then do nothing else remove the directory Symbol
* '/' or '\'
* @author zhao_xiaowei
* @date 2008
* @param dir a file or directory
* @return T_VOID
***************************************************************************/
static T_VOID Fwl_RemoveEndDirSymbol(T_U16 dir[])
{
    T_U16 len= UStrLen(dir);
    if(len <= 2)
    {
        return ;
    }
    if(dir[len- 1] == UNICODE_SOLIDUS || dir[len- 1] == UNICODE_RES_SOLIDUS)
    {
        if(len != 3)//A:/
        {
            dir[len- 1]= 0;
        }
    }
}

T_BOOL  Fwl_GetShortPath(T_U16 sPathName[], T_U16 lPathName[])
{
    T_S16 pos;
    T_U16 lPathLen;

//    if(Fwl_FsIsDir(lPathName))
//    {
//        return AK_FALSE;
//    }

    Fwl_RemoveEndDirSymbol(lPathName);

    //is only file so the last char if not '/'
    pos= UStrRevFndR(strUSolidus, lPathName, 0);
    if(pos < 0)
    {
        pos= UStrRevFndR(strURevSolidus, lPathName, 0);
        if(pos < 0)
        {
            UStrCpy(sPathName, lPathName);
            return AK_FALSE;
        }
    }

    lPathLen= Utl_UStrLen(lPathName);
    if(lPathLen <=  (pos+1))//A:/
    {
        UStrCpy(sPathName, lPathName);
        return AK_FALSE;
    }
    else
    {
        return UStrSub(sPathName, lPathName, 
            (T_U16)(pos+ 1), (T_U16)(lPathLen- (pos+ 1)) );
    }
}
T_VOID Fwl_RemoveEndFileSymbol(T_U16* filename)
{
    T_U16 i = UStrLen(filename);

    while(i--)
    {
        if (UNICODE_DOT == filename[i])
        {
            filename[i] = UNICODE_END;
            break;
        }
    }
    return;
}

#pragma arm section code
//like A: is error path ,must be A:/
T_BOOL  Fwl_GetLongPath(T_U16 lPathName[], const T_U16 basePath[], const T_U16 rltPath[])
{
    T_U16 basePathLen= UStrLen(basePath);    
    T_U16 endChar= basePath[basePathLen- 1];
    T_U16 pathSymbol= basePath[2];
    UStrCpy(lPathName, basePath); //A:/ A:/record the second char is the path symbol
    
    if(endChar != UNICODE_SOLIDUS && endChar != UNICODE_RES_SOLIDUS)
    {
        lPathName[basePathLen]= pathSymbol;
        lPathName[basePathLen+ 1]= '\0';
    }
    UStrCat(lPathName, rltPath);
    return AK_TRUE;  
}

#pragma arm section code = "_listfile_handle_"

static T_BOOL  Fwl_IsFolder(T_hFILEINFO hFileInfo)
{
    T_PFILEINFO pFileInfo = (T_PFILEINFO)hFileInfo;
    
    AK_ASSERT_PTR(pFileInfo, "Fwl_IsFolder", AK_FALSE);
    
    if(FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & pFileInfo->attr))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

static T_U16* Fwl_GetName(T_hFILEINFO hFileInfo)
{
    T_PFILEINFO pFileInfo = (T_PFILEINFO)hFileInfo;

    AK_ASSERT_PTR(pFileInfo, "Fwl_GetName", AK_NULL);
    
    return pFileInfo->LongName;
}

static T_U32 Fwl_GetLen(T_hFILEINFO hFileInfo)
{
    T_PFILEINFO pFileInfo = (T_PFILEINFO)hFileInfo;

    AK_ASSERT_PTR(pFileInfo, "Fwl_GetFileLen", AK_NULL);
    
    return pFileInfo->length;
}

#pragma arm section code 

T_U32 Fwl_GetDriverNum(T_VOID)
{
    T_U32 DriverCnt = 0;
    
    if(T_U8_MAX != _gNandStartDriverID)
    {
        DriverCnt += _gNandStartDriverID;
    }
    if(T_U8_MAX != _gSdDriverInfo.DriverStartID)
    {
        DriverCnt += _gSdDriverInfo.DriverCnt;
    }
    return DriverCnt;
}
static T_U32 Fwl_FilterCmpWithWildChar(const T_U16 *pat, const T_U16 *str,
                                    T_U32 PatLen, T_U32 StrLen)
{
    while (PatLen > 0 && StrLen > 0 && *str && *pat)
    {
        if ('?' == *pat)
        {
            /* '?' can correspond with nothing. */
            if (Fwl_FilterCmpWithWildChar(pat + 1, str, PatLen-1, StrLen))
            {
                return 1;
            }

            /* '?' correspond with a valid char, go on to check next one. */
            str++;
            pat++;
            PatLen--;
            StrLen--;
        }
        else if ('*' == *pat)
        {
            /* Filter the '*' in the pattern string. */
            while ('*' == *pat || '?' == *pat)
            {
                pat++;
                PatLen--;
                if (0 == PatLen)
                {
                    break;
                }
            }
            /* Pattern string left only '*' or '?' character. */
            if (!(*pat) || 0 == PatLen)
            {
                return 1;
            }

            while (*str)
            {
                if (Fwl_FilterCmpWithWildChar(pat, str, PatLen, StrLen))
                {
                    return 1;
                }
                str++;
                StrLen--;
                if (0 == StrLen)
                {
                    break;
                }
            }

            return 0;
        }
        else
        {
            if (*pat != *str)
            {
                if (*str < 'A' || *str > 'Z' || *pat != (T_U16)(*str + 32))
                {
                    return 0;
                }
            }
            str++;
            pat++;
            StrLen--;
            PatLen--;
        }
    }

    /* Length has been decreased into 0. */
    if (0 == PatLen || 0 == StrLen)
    {
        if (0 == PatLen && 0 == StrLen)
        {
            return 1;
        }

        /* str has been scanned over. */
        if (0 == StrLen)
        {
            while ('*' == *pat || '?' == *pat)
            {
                pat++;
                PatLen--;
                if (0 == PatLen)
                {
                    return 1;
                }
            }
            return !(*pat);
        }

        /* pat has been scanned over. */
        return !(*str);
    }

    /* At lease one string has reached the end. */

    /* pattern has been scanned over. */
    if (*str != 0)
    {
        return 0;
    }

    /* str has been scanned over. */
    while ('*' == *pat || '?' == *pat)
    {
        pat++;
        PatLen--;
        if (0 == PatLen)
        {
            return 1;
        }
    }
    return !(*pat);
}
T_BOOL Fwl_FilterMatch(T_U16 *pat, T_U16 *str)
{
    T_U16 *name = pat;
    T_U16 *pStr = str;
    T_U32  Loop = 0;
    T_U16  TmpBuf[256];
    T_U32  PatLenN = (T_U32)(-1);
    T_U32  StrLenN = (T_U32)(-1);
    T_U32  PatLenE = 0;
    T_U32  StrLenE = 0;
    T_BOOL Ret = AK_FALSE;
 
    /* Sep.24,07 - Validate the input parameters. */
    if (AK_NULL == name || 0 == *name)
    {
        return AK_TRUE;
    }
    if (AK_NULL == pStr || 0 == *pStr)
    {
        return AK_FALSE;
    }
 
    /* Calculate the last '.' in the input str. */
    Loop = 0;
    while (*pStr != 0)
    {
        if ('.' == *pStr)
        {
            StrLenN = Loop;
        }
        Loop++;
        pStr++;
    }
    if ((T_U32)(-1) == StrLenN)
    {
        StrLenN = Loop;
        StrLenE = 0;
    }
    else
    {
        /* Loop is always larger than StrLen. */
        StrLenE = (Loop - StrLenN) - 1;
    }
    pStr = str;
 
    while (1)
    {
        Loop = 0;
        TmpBuf[0] = 0;
        PatLenN   = (T_U32)(-1);
        while (*name != 0 && *name != '/' && *name != '\\')
        {
            if (*name >= 'A' && *name <= 'Z')
            {
                TmpBuf[Loop] = (T_U16)(*name + 32);
            }
            else
            {
                TmpBuf[Loop] = *name;
            }
 
            /* Record the last '.' */
            if ('.' == TmpBuf[Loop])
            {
                PatLenN = Loop;
            }
            name++;
            Loop++;
        }
        if (0 == Loop)
        {
            break;
        }
        TmpBuf[Loop] = 0;
        /* Added to check "*.*" */
        if (3 == Loop && '*' == TmpBuf[0] && '.' == TmpBuf[1] && '*' == TmpBuf[2])
        {
            Ret = AK_TRUE;
            break;
        }
 
        if ((T_U32)(-1) == PatLenN)
        {
            PatLenN = Loop;
            PatLenE = 0;
        }
        else
        {
            /* Loop is always larger than PatLenN. */
            PatLenE = (Loop - PatLenN) - 1;
        }
 
        if (Fwl_FilterCmpWithWildChar(TmpBuf, pStr, PatLenN, StrLenN))
        {
            /* Name part matches, then compare the ext part. When the pointer has
               expanded the '\0', PatLenE/StrLenE must be zero. */
            if (Fwl_FilterCmpWithWildChar(TmpBuf+(PatLenN+1),
                                       pStr+(StrLenN+1), PatLenE, StrLenE))
            {
                Ret = AK_TRUE;
                break;
            }
        }
 
        if (*name != 0)
        {
            name++; // Ignore the solidus.
        }
        else
        {
            break;
        }
    }
 
    return Ret;
}

T_BOOL Fwl_FsChkDsk(T_VOID)
{
    T_U8 i;
    T_BOOL ret = AK_FALSE;
        
#ifdef SUPPORT_SDCARD
    if(T_U8_MAX != _gSdDriverInfo.DriverStartID)
    {
        if(AK_FALSE == (ret = FS_ChkDsk(_gSdDriverInfo.DriverStartID, AK_NULL, AK_NULL)))
        {
            AK_DEBUG_OUTPUT("SD FS_ChkDsk false, num = %d", _gSdDriverInfo.DriverStartID);
        }
    }
#endif

    if(T_U8_MAX != _gNandStartDriverID)
    {
        for(i =  0; i < _gNandDriverCnt; i++) 
        {
            if(AK_FALSE == (ret = FS_ChkDsk((T_U8)(_gNandStartDriverID+i), AK_NULL, AK_NULL)))
            {
                AK_DEBUG_OUTPUT("NAND FS_ChkDsk false, num = %d", _gNandStartDriverID);
            }
        }
    }

    return ret;
}


T_BOOL Fwl_FsChkDsk_WithFolder(const T_U8 *path)
{
	return File_chkdsk_withfolder(path);
}



#if (STORAGE_USED == NAND_FLASH)
#ifdef OS_WIN32
T_U32 Medium_GetMTDRsvInfo(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 TolBlock, 
                           T_U16 RsvBlockPerMTD, T_U16 *RsvBlock, T_U16 *RsvNum)
{
    return 0;
}

#endif
#endif

#if (STORAGE_USED != NAND_FLASH)

T_BOOL Medium_CurrentRWFlag(T_VOID)
{
    return 0;
}

T_U32 Medium_GetMTDRsvInfo(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 TolBlock, T_U16 RsvBlockPerMTD, T_U16 *RsvBlock, T_U16 *RsvNum)
{
    return 0;
}

T_U32 NandMtd_Format(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 BlkCnt)
{
    return 0;
}

T_PMEDIUM Medium_CreatePartition(T_PMEDIUM large, T_U32 StartPage, T_U32 PageCnt, T_U32 SecSize, T_U32 ProtectLevel)
{
    return 0;
}

T_PMEDIUM Nand_CreateMedium(T_PNANDFLASH nand,  T_U32 StartBlk, T_U32 BlkCnt, T_U16 *RsvBlkNum)
{
    return 0;
}

T_VOID MtdLib_SetCallBack(T_pVOID pMtdConfig)
{
}

void Medium_ConnectFS(T_PMEDIUM medium, const T_U8 data[], T_U32 sector,T_U32 size)
{
}

T_VOID Medium_Destroy(T_PMEDIUM obj)
{
}

T_BOOL Medium_IsSafePoweroff(T_VOID)
{
    return AK_TRUE;
}


#endif
/* end of file directory op  */

/* end of file */


