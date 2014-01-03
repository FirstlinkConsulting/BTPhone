/**
 * @file Fwl_osFS.h
 * @brief This header file is for OS related function prototype
 *
 */
#ifndef __FWL_OSFS_H__
#define __FWL_OSFS_H__

#include "akdefine.h"
#include "file.h"
#include "Fwl_detect.h"
#include "Eng_Time.h"

#ifndef T_hFILE
#define T_hFILE T_U32
#endif

#ifndef T_hFILEINFO
#define T_hFILEINFO T_U32
#endif

#ifndef T_hFindCtrl
#define T_hFindCtrl T_S32
#endif

#ifndef T_FILE_MODE
#define T_FILE_MODE T_U32
#endif

#ifndef T_FILE_FLAG
#define T_FILE_FLAG T_U32
#endif

#ifndef FS_INVALID_HANDLE
#define FS_INVALID_HANDLE 0
#endif

#ifndef FS_SEEK_SET
#define FS_SEEK_SET     FILE_SEEK_SET
#endif

#ifndef FS_SEEK_CUR
#define FS_SEEK_CUR     FILE_SEEK_CUR
#endif

#ifndef FS_SEEK_END
#define FS_SEEK_END     FILE_SEEK_END
#endif

#define     MAX_FILE_LEN            FIND_FILENAME_PATHLEN
#define     MAX_FILTER_LEN          100


typedef T_S8    T_STR_FILE[MAX_FILE_LEN];
typedef T_U16   T_USTR_FILE[MAX_FILE_LEN];

/* Fwl_FileOpen flag reference. */
#define _FMODE_READ     FILE_MODE_READ
#define _FMODE_WRITE    FILE_MODE_CREATE
#define _FMODE_CREATE   FILE_MODE_CREATE
#define _FMODE_OVERLAY  FILE_MODE_OVERLAY
#define _FMODE_APPEND   FILE_MODE_APPEND
#define _FILE_MODE_EXT_NO_SEPARATE_FAT  FILE_MODE_EXT_NO_SEPARATE_FAT


#define _FOPEN_FAIL     FS_INVALID_HANDLE


typedef struct
{
    T_U32 findCnt;
    T_U32 hFind;
    T_U16 index;
    T_U8 nodeCnt;
    T_BOOL bShowRoot:1;
    T_BOOL bIterate:1;
    T_BOOL rev:6;
    T_U16 *pFileName;
}T_FindCtrl, *T_pFindCtrl;


extern T_U16 g_usbPath[];
extern const T_U16 g_diskHidePath[];
extern T_U8 _gNandStartDriverID, _gNandDriverCnt;
extern const T_U16 g_diskPath[];




/************************************************************************
 * NAME:     Fwl_FSCurrentRWFlag
 * FUNCTION  Get current NFTL read&write status flag (only apply to AK11 platform).
 *                  
 * PARAM:    file handle; 
 *
 * RETURN:     success return AK_TRUE,  R&W is busy
 *                   fail retuen AK_FALSE, R&W is idle
**************************************************************************/
T_BOOL Fwl_FSCurrentRWFlag(T_hFILE file);

T_S16  Fwl_FsInit(T_VOID);

/************************************************************************
 * Brief：Open a certain file by its corresponding absolute unicode path.
 * Param: T_pCWSTR path    - The wanted file's absolute unicode path;
 *        T_FILE_FLAG flag - The open mode, such as _FMODE_READ, _FMODE_CREATE;
 *        T_FILE_MODE attr - The attribute such as FS_ASYSTEM(Only used when creating a file).
 * Ret  : The corresponding file's handler.
 *        When failed, it will return FS_INVALID_HANDLE.
 * Note : flag is the mode of opening, such as _FMODE_READ, while attr is the
 *  attribute of this file, used only in creating operation, such as ATTR_SYSTEM.
**************************************************************************/
T_hFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr);

/************************************************************************
 * Brief：Read count bytes into buffer from the file whose handler is hFile.
 * Param: T_hFILE hFile  - The wanted file's handler;
 *        T_pVOID buffer - The memory used to hold the wanted data;
 *        T_U32 count    - How many bytes you want to read.
 * Ret  : Actual bytes the func has read from the hFile
**************************************************************************/
T_U32   Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count);

/************************************************************************
 * Brief：Write count bytes into buffer from the file whose handler is hFile.
 * Param: T_hFILE hFile  - The wanted file's handler;
 *        T_pVOID buffer - The memory used to hold the wanted data;
 *        T_U32 count    - How many bytes you want to write.
 * Ret  : Actual bytes the func has write into the hFile
**************************************************************************/
T_U32   Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count);

/************************************************************************
 * Brief：Reset the file's buffer size to speed up.
 * Param: T_hFILE hFile  - The wanted file's handler;
 *        T_U32   SecCnt - The sector count of the file's buffer(MUST be 2^n).
 * Ret  : AK_TRUE:  Reset successfully
 *        AK_FALSE: Reset failed.
**************************************************************************/
T_BOOL Fwl_FileSetBufSize(T_hFILE hFile, T_U32 SecCnt);

/************************************************************************
 * Brief：Close the file, all the writen data will be flushed.
 * Param: T_hFILE hFile  - The wanted file's handler;
 * Ret  : AK_TRUE:  Close successfully
 *        AK_FALSE: Close failed, because it is not a valid file.
**************************************************************************/
T_BOOL  Fwl_FileClose(T_hFILE hFile);

/************************************************************************
 * Brief：Rename a file(both path and name can be modified).
 * Param: T_pCWSTR oldFile - The old file's absolute unicode path;
 *        T_pCWSTR newFile - The dst file's absolute unicode path;
 * Ret  : AK_TRUE:  Rename successfully
 *        AK_FALSE: Rename successfully.
 * Note : When succeed to rename it, the old one will not exist, replaced
 *        by the new one. When the oldFile is a folder, the newFile MUST be
 *        under the same parent folder. That is, only name can be modified.
**************************************************************************/
T_BOOL  Fwl_MoveFile(T_pCWSTR oldFile, T_pCWSTR newFile);

/************************************************************************
 * Brief:(See the following list)
        SrcStr DstStr Function
        file   folder Copy file to folder; if replace, delete the file with the
                      same name under folder before copying
        file   file   Copy file's data to another; if replace, delete the dest file
                       before copying data
        folder folder Copy sub-files/sub-folders from SrcStr to DstStr; if replace,
                       delete any file/folder under dest folder which has the same
                       name with some file/folder under source folder
        folder   file    Forbid
 * Para : T_pCSTR SrcStr : Source file's absolute path
          T_pCSTR DstStr : Dest file's absolute path
          T_BOOL  bFailIfExists: Delete or not
                   AK_TURE : When exist, not replace it and return fail.
                   AK_FALSE: When exist, replace it.
 * Ret  : Copy successfully or not.
           AK_TRUE : Copy successfully.
           AK_FALSE: Copy failed!
**************************************************************************/
T_BOOL  Fwl_CopyFile(T_pCWSTR sourFile, T_pCWSTR destFile, T_BOOL bFailIfExist);

/************************************************************************
 * Brief：Adjust the file's current reading/writing position.
 * Param: T_hFILE hFile - The wanted file's handler;
 *        T_S32 offset  - The offset relative to the origin;
 *        T_U16 origin  - The new position's base, ONLY can be one of
 *                     FS_SEEK_SET/FS_SEEK_CUR/FS_SEEK_END, respectively means
 *                     base point:0/Current position/File's length.
 * Ret  : The new reading/writing position after adjustment.
**************************************************************************/
T_U32 Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin);

/************************************************************************
 * Brief：Adjust the file's current reading/writing position.
 * Param: T_hFILE hFile - The wanted file's handler;
 *        offset  - origin=FS_SEEK_CUR: The offset as T_S32, relative to the origin;
                     origin=FS_SEEK_SET: The offset as T_U32, can reach 4G byte.                          
 *        T_U16 origin  - The new position's base, ONLY can be one of
 *                     FS_SEEK_SET/FS_SEEK_CUR/FS_SEEK_END, respectively means
 *                     base point:0/Current position/File's length.
 * Ret  : The new reading/writing position after adjustment.
**************************************************************************/
T_U32 Fwl_FileLongSeek(T_hFILE hFile, T_U32 offset, T_U16 origin);

/************************************************************************
 * Brief：Get the file's current reading/writing position.
 * Param: T_hFILE hFile - The wanted file's handler;
 * Ret  : the file's current reading/writing position.
**************************************************************************/
T_U32 Fwl_FileTell(T_hFILE hFile);

/************************************************************************
 * Brief：Get the file's length.
 * Param: T_hFILE hFile - The wanted file's handler;
 * Ret  : the file's length.
**************************************************************************/
T_U32 Fwl_GetFileLen(T_hFILE hFile);

/************************************************************************
 * Brief：Get the file's Create time.
 * Param: T_hFILE hFile        - The wanted file's handler;
 *        T_PFILETIME fileTime - Output para to hold the file's create time.
 * Ret  : AK_TRUE : Get successfully.
 *        AK_FALSE: Get failed!
**************************************************************************/
T_BOOL Fwl_GetFileCreatTime(T_hFILE hFile, T_pSYSTIME fileTime);

/************************************************************************
 * Brief：Get the file's Modified time.
 * Param: T_hFILE hFile        - The wanted file's handler;
 *        T_PFILETIME fileTime - Output para to hold the file's Modified time.
 * Ret  : AK_TRUE : Get successfully.
 *        AK_FALSE: Get failed!
**************************************************************************/
T_BOOL Fwl_GetFileModTime(T_hFILE hFile, T_pSYSTIME fileTime);

/************************************************************************
 * Brief：Delete the file, given its corresponding absolute unicode path.
 * Param: T_pCWSTR path - The wanted file's absolute unicode path;
 * Ret  : AK_TRUE : Delete successfully.
 *        AK_FALSE: Delete failed!
**************************************************************************/
T_BOOL Fwl_FileDelete(T_pCWSTR path);

/************************************************************************
 * Brief：Get some of the file's information given its absolute unicode path.
 * Param: T_pCWSTR path         - The wanted file's absolute unicode path;
 *        T_PFILEINFO pFileInfo - Output para to hold file's information.
 * Ret  : AK_TRUE : Get successfully.
 *        AK_FALSE: Get failed!
**************************************************************************/
T_BOOL Fwl_FileStat(T_pCWSTR path, T_hFILEINFO hFileInfo);

/************************************************************************
 * Brief：Get some of the file's information given its handler.
 * Param: T_hFILE hFile         - The wanted file's handler;
 *        T_PFILEINFO pFileInfo - Output para to hold file's information.
 * Ret  : AK_TRUE : Get successfully.
 *        AK_FALSE: Get failed!
**************************************************************************/
T_BOOL Fwl_FileStatEx(T_hFILE hFile, T_hFILEINFO hFileInfo);

/************************************************************************
 * Brief：Create a directory whose absolute unicode path is the input para.
 * Param: T_pCWSTR path - The wanted folder's absolute unicode path;
 * Ret  : AK_TRUE : Create successfully.
 *        AK_FALSE: Create failed!
 * Note : The func can create ONLY one level deep. That is the folders in the path
 *        except the deepest one MUST have been existed.
**************************************************************************/
T_BOOL Fwl_FsMkDir(T_pCWSTR path);

/************************************************************************
 * Brief：Create directories whose absolute unicode path is the input para.
 * Param: T_pCWSTR path - The wanted folder's absolute unicode path;
 * Ret  : AK_TRUE : Create successfully.
 *        AK_FALSE: Create failed!
 * Note : The func can create many levels. That is if the folders in the path
 *        before the deepest one do not exist, the func will create it first
 *        till succeed to create the deepest one.
**************************************************************************/
T_BOOL Fwl_FsMkDirs(T_pCWSTR path);

/************************************************************************
 * Brief：Delete a directory whose absolute unicode path is the input para.
 * Param: T_pCWSTR path - The wanted folder's absolute unicode path;
 * Ret  : AK_TRUE : Delete successfully.
 *        AK_FALSE: Delete failed!
 * Note : The func can NOT delete a folder which has any children. That is,
 *        the folder MUST be empty.
**************************************************************************/
T_BOOL Fwl_FsRmDir(T_pCWSTR path);

/************************************************************************
 * Brief：Check whether the path is a directory or not.
 * Param: T_pCWSTR path - The wanted file's absolute unicode path;
 * Ret  : AK_TRUE : The file with path is a folder.
 *        AK_FALSE: The file with path is not a folder!
**************************************************************************/
T_BOOL Fwl_FsIsDir(T_pCWSTR path);

/************************************************************************
 * Brief：List certain files under the path.
 * Param: T_pCWSTR path           - The wanted folder's absolute unicode path;
 *        T_PFINDBUFCTRL pBufCtrl - Controler para.
 * Ret  : When failed, return FS_INVALID_HANDLE.
 *        When success, return the corresponding handler.
**************************************************************************/
T_hFindCtrl Fwl_FsFindFirst(T_pCWSTR path, T_PFINDBUFCTRL pBufCtrl);

/************************************************************************
 * Brief：List certain files under the path.
 * Param: T_hFindCtrl findCtrl - The listing handler returned by FindFirst;
 *        T_S32 Cnt - How many files/folders this time should list.
 * Ret  : Actually we have listed this time.
**************************************************************************/
T_U32  Fwl_FsFindNext(T_hFindCtrl findCtrl, T_S32 Cnt);

/************************************************************************
 * Brief：Close the listing operation.
 * Param: T_hFindCtrl findCtrl - The listing handler returned by FindFirst.
 * Ret  : T_VOID.
**************************************************************************/
T_VOID Fwl_FsFindClose(T_hFindCtrl findCtrl);

/* ************************************************************
* NAME: 	  Fwl_FsFindFirstEX
* FUNCTION	   in order to shorten  time that find a file or folder.once found a  satisfied file or folder,function exit.
                      from "PreFDT" position of "T_U16 *path" ,start find file and folder satisfying the setting "pattern" condition  
* PARAM:	  const T_U16 *path : path for find
* PARAM:	  T_U16 *pattern : file format filter
* PARAM:	  T_U16 *filter_pattern : folder filter
* PARAM:	  T_U32 PreFDT : fdt info of the last found file
* RETURN:	  >0 success, 0 fail
**************************************************************************/
T_U32 Fwl_FsFindFirstEX(const T_U16 *path, T_U16 *pattern, T_U16 *filter_pattern, T_U32 PreFDT);

/************************************************************************
* NAME: 	  Fwl_FsFindNextEX
* FUNCTION	  use together with  Fwl_FsFindFirstEX, only find a folder and file    
* PARAM:	  T_U32 pFindCtrl : retval of Fwl_FsFindFirstEX
* RETURN:	  >0 success, 0 fail
**************************************************************************/
T_U32 Fwl_FsFindNextEX(T_U32 pFindCtrl);

/************************************************************************
 * NAME:        Fwl_FsFindCloseEX
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl : retval of Fwl_FsFindFirstEX
 * RETURN:     NONE
**************************************************************************/
T_VOID Fwl_FsFindCloseEX(T_U32 pFindCtrl);


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
									T_U16 *path_buf, T_PFINDBUFINFO find_fileinfo);


/************************************************************************
 * NAME:     Fwl_FsFindNext_WithID
 * FUNCTION read  data of the first cluster  
 * PARAM:    const T_U32 obj, -- the hande of the file
 * PARAM:     T_S32 Cnt,  -1 or 1 
 * PARAM:     T_U32 *ret_cnt        find the num of file
  * RETURN:     T_U32
**************************************************************************/
T_U32 Fwl_FsFindNext_WithID(T_U32 obj, T_S32 Cnt, T_U32 *ret_cnt);

/************************************************************************
 * NAME:     Fwl_FsGet_findfile_info_WithID
 * FUNCTION read  data of the first cluster  
 * PARAM:    T_U32 obj -- the hande of the file
 * PARAM:    T_PFILEINFO_BEFOREPOWER fileinfo
* RETURN:     t_void
**************************************************************************/
T_VOID Fwl_FsGet_findfile_info_WithID(T_U32 obj, T_PFILEINFO_BEFOREPOWER fileinfo);



/************************************************************************
 * NAME:        Fwl_FsFindInfo_WithID
 * FUNCTION  get file info of the input pFindCtrl
 * PARAM:      T_U32 find_handle
                      T_U32 position :the file's position of pFindCtrl's blink 
                      T_U32 *FileCnt :     file count
                      T_U32 *FolderCnt :     folder count
 * RETURN:     it will return file info structure pointer 
**************************************************************************/
T_PFILEINFO Fwl_FsFindInfo_WithID(T_U32 obj, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt);


/************************************************************************
 * NAME:        Fwl_FsFindClose_WithID
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 obj  -- the finding hande  
 * RETURN:     NONE
**************************************************************************/
T_VOID Fwl_FsFindClose_WithID(T_U32 obj);


/************************************************************************
 * Brief：Check whether the folder holds any file matched with pattern or not.
 * Param: T_pCWSTR path  - The wanted folder with absolute unicode path path;
 *        T_U16 *pattern - The wanted pattern with unicode, such as "*.mp3 *.wav";
 *        T_U16 *outPath - Hold the first founded absolute path which holds
 *                         at least one matched file with pattern
 * Ret  : AK_TRUE : The folder(or sub-folders) hold the matched file.
 *        AK_FALSE: The folder and all sub-folders hold no matched file.
 * Note : Only when the function return AK_TRUE can we check the outPath. When
 *        it return AK_FALSE, the outPath is meaningless.
**************************************************************************/
T_BOOL Fwl_FsExistDeepMatch(T_pCWSTR path, T_U16 *pattern, T_U16 *outPath, T_U32 deep);

/************************************************************************
 * Brief：Get the capacity of the corresponding driver.
 * Param: T_U16 DrvIndx     - The driver whose id is DrvIndx;
 *        T_U64_INT *size64 - Output para to hold the capacity of the driver.
 * Ret  : T_VOID
**************************************************************************/
T_VOID Fwl_FsGetSize(T_U16 DrvIndx, T_U64_INT *size64);

/************************************************************************
 * Brief：Get used size of the corresponding driver.
 * Param: T_U16 DrvIndx     - The driver whose id is DrvIndx;
 *        T_U64_INT *size64 - Output para to hold the capacity of the driver.
 * Ret  : T_VOID
**************************************************************************/
T_VOID Fwl_FsGetUsedSize(T_U16 DrvIndx, T_U64_INT *size64);

/************************************************************************
 * Brief：Get un-used size of the corresponding driver.
 * Param: T_U16 DrvIndx     - The driver whose id is DrvIndx;
 *        T_U64_INT *size64 - Output para to hold the capacity of the driver.
 * Ret  : T_VOID
**************************************************************************/
T_VOID Fwl_FsGetFreeSize(T_U16 DrvIndx, T_U64_INT *size64);


/************************************************************************
 * Brief：Check whether the path is a root one.
 * Param: T_pCWSTR pFilePath - The wanted absolute unicode path.
 * Ret  : AK_TRUE : The path is a root one.
 *        AK_FALSE: The path is not a root one.
**************************************************************************/
T_BOOL Fwl_IsRootDir(T_pCWSTR pFilePath);

/************************************************************************
 * Brief：Get the root directory corresponding to pFilePath.
 * Param: T_pCWSTR pFilePath - One absolute path which is under the wanted root;
 *        T_pWSTR  pRootDir  - Output para to hold the desired root diretory.
 * Ret  : AK_TRUE : Get successfully.
 *        AK_FALSE: Get failed.
**************************************************************************/
T_BOOL Fwl_GetRootDir(T_pCWSTR pFilePath, T_pWSTR pRootDir);



//******************************file directory op*****************************************



/**************************************************************************
* @brief jusify fromDir is real path 
* 
* @author zhao_xiaowei
* @date 2008
* @param formDir[in] 
* @return T_BOOL
* @retval  if fromDir is ../ or ./ then return AK_FALSE
***************************************************************************/
T_BOOL Fwl_DirIsRealPath(T_U16* fromDir);


//if path if file , then call Fwl_GetFileDir, else the path is the currentDic

/**************************************************************************
* @brief the the diretory of path. if path is directory then currentDir is 
* the path, else if path is file, then  get the directory of the file
* @author zhao_xiaowei
* @date 2008
* @param currentDir[out]
* @param path[in]
* @return T_BOOL
* @retval  AK_TURE is sucesss 
***************************************************************************/
T_BOOL Fwl_GetCurrentDir(T_U16 currentDir[], T_U16 path[]);

/**************************************************************************
* @brief this fuction like Fwl_GetFileDir 
* 
* @author zhao_xiaowei
* @date 2008
* @param  path [in/ out]
* @return T_BOOL
* @retval AK_TRUE success
* @retval 
***************************************************************************/
T_BOOL Fwl_GetSelfParentDir(T_U16 path[]);

/**************************************************************************
* @brief this fuction like the Fwl_GetCurrentDir
* 
* @author zhao_xiaowei
* @date 2008
* @param  currentPath[in/out] 
* @return T_BOOL
* @retval AK_TRUE Success
***************************************************************************/
T_BOOL Fwl_GetSelfCurDir(T_U16 currentPath[]);

//from the last '/' start string
/**************************************************************************
* @brief get the behind the last disk symbol '/' or '\' string 
* eg lPathName: A:/image, out sPathName: image
* @author zhao_xiaowei
* @date 2008
* @param sPathName[out]
* @param lPathName[in]
* @return T_BOOL
* @retval AK_TRUE Success
***************************************************************************/
T_BOOL  Fwl_GetShortPath(T_U16 sPathName[], T_U16 lPathName[]);
/**************************************************************************
* @brief this fuction like the Fwl_GetCurrentDir
* 
* @author zhao_xiaowei
* @date 2008
* @param  currentPath[in/out] 
* @return T_VOID
* @retval AK_TRUE Success
****************************/
T_VOID Fwl_RemoveEndFileSymbol(T_U16* filename);
//combile dir and pat
/**************************************************************************
* @brief combile basePath and rltPath
* eg basePath A:/ rltPaht image out lPathName A:/image
* @author zhao_xiaowei
* @date 2008
* @param lPathName [out]
* @param  basePath [in]
* @param rltPath[in]
* @return T_BOOL
* @retval AK_TRUE success
***************************************************************************/
T_BOOL  Fwl_GetLongPath(T_U16 lPathName[], const T_U16 basePath[], const T_U16 rltPath[]);

T_BOOL  Fwl_FsFindIsFolder(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo);

T_U16* Fwl_FsFindGetName(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo);

T_U32 Fwl_FsFindGetLen(T_hFindCtrl findCtrl, T_hFILEINFO hFileInfo);

T_PFILEINFO Fwl_FindInfo(T_U32 pFindCtrl, T_U32 Position, T_U32 * FileCnt, T_U32 * FolderCnt);

T_hFILEINFO Fwl_FsGetFindInfo(T_hFindCtrl findCtrl, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt);

T_BOOL Fwl_FileFlush(T_hFILE hFile);  
/**end of file  directory op **/

T_BOOL  Fwl_FsMkDirTree(T_pCWSTR path);


T_BOOL Fwl_FilterMatch(T_U16 *pat, T_U16 *str);
T_U32 Fwl_GetDriverNum(T_VOID);
T_BOOL Fwl_FsChkDsk(T_VOID);
T_BOOL Fwl_FsChkDsk_WithFolder(const T_U8 *path);
T_BOOL Fwl_SetFileSize(T_hFILE hFile, T_U32 fileSize);
T_hFILE Fwl_FileOpenFromParent(T_hFILE hParent, T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr);
T_hFILE Fwl_FolderOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE attr);
T_BOOL Fwl_FolderClose(T_hFILE hFolder);
T_BOOL Fwl_FileDeleteHandle(T_hFILE hFile);
T_BOOL Fwl_FileExist(T_pCWSTR path);

//搜索时带着父句柄，这样在文件夹很多的情况下，搜索效率明显高些
//Fwl_FileFindFirstFromHandld必须与Fwl_FileFindCloseWithHandle合用
T_U32 Fwl_FileFindFirstFromHandle(T_U32 file, T_PFINDBUFCTRL pBufCtrl);

T_U32 Fwl_FileFindOpen(T_U32 parent, T_U32 FileInfo);

T_VOID Fwl_FileFindCloseWithHandle(T_hFindCtrl findCtrl);


#endif

