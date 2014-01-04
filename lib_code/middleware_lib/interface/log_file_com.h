#ifndef __LOG_FILE_COM__
#define __LOG_FILE_COM__
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

#include "anyka_types.h"
#include "vme.h"
#include "Fwl_osFS.h"


#define  DATENAME_MAXINDEX       1000 
#define  FILENAME_MAXINDEX       999999 

#ifdef SUPPORT_SDCARD
//#define IS_SDFILE_INAVIAL(file) (file[0] == g_sdPath[0] && (!Fwl_SDCard_Is_Valid(0)))
__inline T_BOOL IS_SDFILE_INAVIAL(const T_U16 *file)
{
    if(file[0] >= *(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL)))
    {
        if((file[0] == *(Fwl_MemDevGetPath(MMC_SD_CARD, AK_NULL))) && (Fwl_MemDevIsMount(MMC_SD_CARD)))
        {
            return AK_FALSE;
        }
        return AK_TRUE;
    }
    return AK_FALSE;
}
#else 
#define IS_SDFILE_INAVIAL(file)  AK_FALSE
#endif

#define ASSERT_SDFILE_AVIAL(file, errorRet)  { \
    if(IS_SDFILE_INAVIAL(file))             \
    {                                       \
        AK_DEBUG_OUTPUT("The SD is Out while file is using\n");\
        return errorRet;                     \
    }                                        \
}


#define HINT_DISPLAY_COUNT 0
typedef enum
{ 
        eFail= 0, //选择失败
        eFile,//选择的是文件
        ePath //选择的是路径
}T_eSelResult;

typedef enum
{
        eComNone,
        eMainDiskList,
        eDelFile,
        eDelAll,
        eViewSet
}T_eFileComType;

typedef enum
{
    eDisplayAll= 0,
    eFolderOnly,
    eFileOnly
}T_eDisplayStyle;

typedef struct
{
    T_U16 file[MAX_FILE_LEN]; //文件或目录名 [in (file)]
    T_U16 filter[MAX_FILTER_LEN];//文件过滤器  [in]
    T_BOOL bResult; //[out] 是否成功删除文件
}T_FileDel, *PFileDel;

typedef struct {
    T_U16 file[MAX_FILE_LEN]; //文件或目录名[in(Dir)/ out(file or Dir)]
    T_U16 filter[MAX_FILTER_LEN];//文件过滤器  [in]
    T_eSelResult selResult; //[out]  操作结果状态
    T_eDisplayStyle displayStyle; //显示模式，全部显示还是只显示文件夹
}T_FileSel, *PFileSel;

typedef struct
{
    T_MEM_DEV_ID driver;
    T_U16 filter[MAX_FILTER_LEN];//文件过滤器  [in]
    T_BOOL bResult; //[out] 是否成功删除该类型的所有文件。
}T_FileDelAll, *PFileDelAll;

/**************************************************************************
* @brief delete all files from the directory formDir
* @author zhao_xiaowei
* @date 2008
* @param fromDir[in] the start directory 
* @param pFileDelAll[in] the delete files information
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/
T_BOOL DelDirAllFiles(const T_U16* fromDir, T_FileDelAll* pFileDelAll);

T_VOID TryDeleteEbookMarker(T_U16* file, T_BOOL bDeleteAll);

T_BOOL FileCom_CheckIndex(T_pWSTR pFilePath, T_pSTR pMatchName, T_U32 *index, T_U32 MaxInd);

/***********************************************************************************
FUNC NAME: FileCom_GetFreeIndex
DESCRIPTION: 获取当前文件夹中第一个空闲的索引号，默认标准命名
为RECXXX.wav 这样类似的，数字位必须为3位，后缀名必须为3位，前缀随意。
INPUT: pFilePath = path of the new file.
       pMatchName = match string of file name.
       MaxInd = limit the number of file index.
OUTPUT: index = get the number of a null file.
AUTHOR:songmengxing
CREATE DATE:2013-08-01
MODIFY LOG:
***********************************************************************************/
T_BOOL FileCom_GetFreeIndex(T_pWSTR pFilePath, T_pSTR pMatchName, T_U32 *index, T_U32 MaxInd);

#endif
