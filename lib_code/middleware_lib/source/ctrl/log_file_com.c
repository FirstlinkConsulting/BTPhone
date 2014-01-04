#include "Gbl_Global.h"
#include "log_file_com.h"
#include "Fwl_FreqMgr.h"
#include "Eng_debug.h"
#include "Eng_DataConvert.h"
#include "fwl_system.h"

#include <stdio.h>

#define WATCHDOG_SET_LONG_TIME      (55) // 1min
#define WATCHDOG_DISABLE_LONG_TIME  (0)

#if(NO_DISPLAY == 0)

extern T_VOID    EbookDeleteBookMark(T_U16 * file, T_BOOL bDeleteAll);

T_VOID TryDeleteEbookMarker(T_U16* file, T_BOOL bDeleteAll)
{
    T_U16 ebookExt[10];
    ToWideChar(ebookExt, "*.txt");
    if(Fwl_FilterMatch(ebookExt, file))
    {
        EbookDeleteBookMark(file, bDeleteAll);
    }
}

//#pragma arm section code = "_fatlibrodata_"
T_BOOL DelDirAllFiles(const T_U16* fromDir, T_FileDelAll* pFileDelAll)
{
    T_hFindCtrl find;
    T_hFILEINFO fileInfo;
    T_U16 tmpPath[MAX_FILE_LEN];
    T_FINDBUFCTRL findBuf;
    T_U16* filename;
    
    AK_ASSERT_PTR(pFileDelAll,"DelDirAllFiles pFileDelAll is NULL", AK_FALSE);
    ASSERT_SDFILE_AVIAL(fromDir, AK_FALSE)
    findBuf.NodeCnt= 1;
    findBuf.type = FILTER_FOLDER| FILTER_NOTITERATE;    //由于是递归查找，必须使用FILTER_NOTITERATE，否则递归没有退出条件
    findBuf.pattern=(T_U16* )pFileDelAll->filter;
    findBuf.patternLen= (T_U8)Utl_UStrLen(pFileDelAll->filter);
    
    find= Fwl_FsFindFirst(fromDir, &findBuf);
    if(find == FS_INVALID_HANDLE)
    {
        pFileDelAll->bResult= AK_FALSE;
    }
    else
    {
        do
        {
            if(IS_SDFILE_INAVIAL(fromDir))
            {
                break;
            }
            fileInfo = Fwl_FsGetFindInfo(find, 0, AK_NULL, AK_NULL);
            pFileDelAll->bResult= AK_TRUE;//开始初始化为TRUE
            filename = Fwl_FsFindGetName(find, fileInfo);
            if(Fwl_DirIsRealPath(filename))
            {
                Fwl_GetLongPath(tmpPath, fromDir, filename);
              
                if(Fwl_FsFindIsFolder(find, fileInfo))
                {
                   pFileDelAll->bResult= DelDirAllFiles(tmpPath, pFileDelAll);            
                }
                else
                {
                    pFileDelAll->bResult= Fwl_FileDelete(tmpPath);            
                }

                if(pFileDelAll->bResult == AK_FALSE)
                {
                    break;
                }
            }
            
        }while(Fwl_FsFindNext(find, 1));
    
        Fwl_FsFindClose(find);
    }
    return pFileDelAll->bResult;
}
//#pragma arm section code 

#endif

static T_BOOL FileCom_CheckFileExist(T_hFILE fparent, T_pSTR pMatchName, T_U32 index)
{
    T_hFILE     fp;
    T_STR_FILE    TmpName;    //not unicode string!
    T_USTR_FILE fileName;
    
    AK_ASSERT_PTR(pMatchName, "pMatchName:Input parameter is error", AK_FALSE);

    if (fparent == _FOPEN_FAIL)
    {
        return AK_FALSE;
    }
    
    sprintf(TmpName, pMatchName, index);
    Utl_StrMbcs2Ucs(TmpName, fileName);
    
    if ((fp = Fwl_FileOpenFromParent(fparent, fileName, _FMODE_READ, _FMODE_READ)) != _FOPEN_FAIL)
    {
        Fwl_FileClose(fp);
        return AK_TRUE;
    }
    return AK_FALSE;
}

/***********************************************************************************
FUNC NAME: FileCom_GetMaxIndex
DESCRIPTION: 二分法获取当前文件夹中已存在的文件名匹配字符串最大索引号
INPUT: fparent = handle of the doc.
       pMatchName = match string of file name.
       start = start number of file index.
       end = end number of file index.
OUTPUT: index = get the number of a null file.
RETURN: -1 = error. -2 = start to end is null.
AUTHOR:liangxiong
CREATE DATE:2011-12-2
MODIFY LOG:
***********************************************************************************/
static T_S32 FileCom_GetMaxIndex(T_hFILE fparent, T_pSTR pMatchName, T_U32 start, T_U32 end)
{
    T_U32 mid;
    T_U32 i = 0;
    
    AK_ASSERT_PTR(pMatchName, "pMatchName:Input parameter is error", AK_FALSE);

    if ((end == 0 && start >= end)||(fparent == _FOPEN_FAIL))
    {
        return -1;
    }

    if (!FileCom_CheckFileExist(fparent, pMatchName, start)) // 判断当前值是否存在
    {
        return -2;
    }
    else
    {
        start++;
    }
    
      while (1)
    {
        if (i > 1024)
        {
            return -1;
        }

        mid = (start+end)/2;
        
        if (FileCom_CheckFileExist(fparent, pMatchName, start)) // 判断当前值是否存在
        {
            start++;
        }
        else
        {
            //AK_DEBUG_OUTPUT("Get start:%d\n", (start-1));
            return (start-1);
        }

        if (FileCom_CheckFileExist(fparent, pMatchName, end))
        {
            return end;
        }

        if (FileCom_CheckFileExist(fparent, pMatchName, mid))
        {
            start = mid;
        }
        else
        {
            end = mid;
        }
        
        if(start == (end - 1))
        {
            if(FileCom_CheckFileExist(fparent, pMatchName, start))
            {
                AK_DEBUG_OUTPUT("Get Max count:%d\n", i);
                return start;
            }
            else
            {
                return -1;
            }
        }
        i++;
    } 
}


/***********************************************************************************
FUNC NAME: FileCom_CheckIndex
DESCRIPTION: 获取当前文件夹中已存在的文件名匹配字符串最大索引号
INPUT: pFilePath = path of the new file.
       pMatchName = match string of file name.
       MaxInd = limit the number of file index.
OUTPUT: index = get the number of a null file.
AUTHOR:liangxiong
CREATE DATE:2011-12-2
MODIFY LOG:
***********************************************************************************/
T_BOOL FileCom_CheckIndex(T_pWSTR pFilePath, T_pSTR pMatchName, T_U32 *index, T_U32 MaxInd)
{
    T_hFILE     fp, fparent;
    T_U32        i = 0;
    T_U32        count = 0;
    T_S32        gMax = 0;
    T_BOOL      bRet = AK_FALSE;
    T_BOOL      bCheckFlag = AK_TRUE;
    T_STR_FILE    TmpName;               //not unicode string!
    T_USTR_FILE fileName; 
    
    AK_ASSERT_PTR(pFilePath, "pFilePath:Input parameter is error", AK_FALSE);
    AK_ASSERT_PTR(pMatchName, "pMatchName:Input parameter is error", AK_FALSE);
    Fwl_FreqPush(FREQ_APP_MAX); 
    Fwl_Freq_Add2Calc(FREQ_VAL_1);
    
    Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
    
    //打开路径文件夹
    Utl_UStrCpyN(fileName, pFilePath, (T_U32)(MAX_FILE_LEN - 1));
    if(_FOPEN_FAIL == (fparent = Fwl_FolderOpen(pFilePath, _FMODE_READ, _FMODE_READ)))
    {
        Fwl_Freq_Clr_Add();
        Fwl_FreqPop();
        Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
        return AK_FALSE;
    }

    //检测下一个索引文件
    if (!FileCom_CheckFileExist(fparent, pMatchName, (*index)+1))
    {
        AK_DEBUG_OUTPUT("check next file seccess: %d\n", (*index));
        bCheckFlag = AK_FALSE;
        bRet = AK_TRUE;
    }
    else
    {
        (*index)++; //已检测此数值文件名存在，自加一。
    }

    if (MaxInd > FILENAME_MAXINDEX) //限制最大值 不大于 999999
    {
        MaxInd = FILENAME_MAXINDEX;
    }
    
    //二分法查找最大文件
    if (bCheckFlag)
    {
        gMax = FileCom_GetMaxIndex(fparent, pMatchName, (*index), MaxInd);
        
        if (gMax < 0 || gMax >= MaxInd)
        {
            count = (*index);
            AK_DEBUG_OUTPUT("error gMax is:%d, set count is:%d", gMax, count);
        }
        else
        {
            (*index) = (T_U32)gMax;
            bCheckFlag = AK_FALSE;
            bRet = AK_TRUE;
            AK_DEBUG_OUTPUT("get max:%d.\n", (*index));           
        }
    }
    
    //顺序查找最大文件
    if (bCheckFlag)
      {
        while (1)
        {
            if ((count > MaxInd) || (count == 0))
            {
                AK_DEBUG_OUTPUT("error max is:%d count is:%d so set to 1.\n", gMax, count);
                count = 1;
            }
            //akerror("count :", count, 1);

            if (i > 1024)
            {
                (*index) = count;
                break;
            }

            sprintf(TmpName, pMatchName, count);
            Utl_StrMbcs2Ucs(TmpName, fileName);
            pFilePath = fileName;
            
            if ((fp = Fwl_FileOpenFromParent(fparent, pFilePath, _FMODE_READ, _FMODE_READ)) != _FOPEN_FAIL)
            {
                Fwl_FileClose(fp);
            }
            else
            {
                (*index) = count - 1;
                //akerror("check :", (*index), 1);                
                bRet = AK_TRUE;
                break;
            }
            count++;
            i++;
        }
    }
    
    if(_FOPEN_FAIL != fparent)
    {
        Fwl_FolderClose(fparent);
        fparent = _FOPEN_FAIL;
    }
    
    Fwl_Freq_Clr_Add();
    Fwl_FreqPop(); 

    Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    return bRet;
}

/**
* @brief Find the first free id by bitmap
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @return T_U16 id
* @retval 
*/
static T_U16 FileCom_FindFirstFreeId(T_U8* pbitmap, T_U32 bitmapSize)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U16	id = bitmapSize << 3;
	T_U16	i = 0;
	T_U32	bitnum = bitmapSize << 3;
	
	AK_ASSERT_PTR(pbitmap, "FileCom_FindFirstFreeId(): pbitmap", id);

	for (i=0; i<bitnum; i++)
	{
		pos = i >> 3;
		bitOffset = (T_U8)(i & 0x7);

		if (0 == (pbitmap[pos] & (1 << bitOffset)))
		{
			id = i;
			break;
		}
	}

	return id;
}

/**
* @brief Set bit value
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @param in T_U32 id : id
* @param in T_BOOL val : 0 or 1
* @return T_BOOL
* @retval 
*/
static T_BOOL FileCom_SetBitValue(T_U8* pbitmap, T_U32 bitmapSize, T_U32 id, T_BOOL val)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U32	bitnum = bitmapSize << 3;
	T_BOOL	ret = AK_FALSE;
	T_U8	mask = 0;
	T_U8	rmask = 0;
	
	AK_ASSERT_PTR(pbitmap, "FileCom_SetBitValue(): pbitmap", ret);
	AK_ASSERT_VAL(id < bitnum, "FileCom_SetBitValue(): id err", ret);

	pos = id >> 3;
	bitOffset = (T_U8)(id & 0x7);

	mask = 1<<bitOffset;
    rmask = ~mask;
    pbitmap[pos] = (pbitmap[pos]&rmask)|(val<<bitOffset);

	ret = AK_TRUE;

	return ret;
}



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
T_BOOL FileCom_GetFreeIndex(T_pWSTR pFilePath, T_pSTR pMatchName, T_U32 *index, T_U32 MaxInd)
{
	T_U8*			pBitmap = AK_FALSE;
	T_U32			bitmapSize = 0;
	T_FINDBUFINFO	findinfo = {0};
	T_U16			pattern[6] = {'*','.',0};
	T_U16			suffix[4] = {0};
	T_U32			pFindHandle = 0;
	T_USTR_FILE 	fileName = {0}; 
	T_USTR_FILE 	name = {0}; 
	T_STR_FILE		TmpName;	//not unicode string!
    T_USTR_FILE		UcsTmpName; 
	T_U32			ret_cnt = 0;
	T_S32			id = 0;
	T_U16			freeId = MaxInd;
	T_U32 			count = 0;
	T_BOOL			ret = AK_FALSE;
	
	AK_ASSERT_PTR(pFilePath, "pFilePath:Input parameter is error", AK_FALSE);
    AK_ASSERT_PTR(pMatchName, "pMatchName:Input parameter is error", AK_FALSE);
	AK_ASSERT_PTR(index, "index:Input parameter is error", AK_FALSE);
	
    Fwl_FreqPush(FREQ_APP_MAX); 
    Fwl_Freq_Add2Calc(FREQ_VAL_1);
    
    Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
	

	bitmapSize = (MaxInd >> 3) + 1;
	pBitmap = (T_U8*)Fwl_Malloc(bitmapSize * sizeof(T_U8));
	AK_ASSERT_PTR(pBitmap, "pBitmap malloc fail", AK_FALSE);
	memset(pBitmap, 0, bitmapSize * sizeof(T_U8));
	
	findinfo.find_fileinfo = AK_NULL;
	findinfo.deep_flag = AK_FALSE;
	findinfo.find_type = SEARCH_TYPE_NOTITERATE;
	Utl_StrMbcs2Ucs(&pMatchName[Utl_StrLen(pMatchName)-3], suffix);//获取后缀
	Utl_UStrCat(pattern, suffix);
	findinfo.pattern = pattern;
	findinfo.fileter_pattern = AK_NULL;

	pFindHandle = Fwl_FsFindFirst_WithID(pFilePath, &ret_cnt, fileName, &findinfo);

	//搜索一遍，整理好位图信息
	while ((0 != ret_cnt) && (count < MaxInd))
	{
		id = Utl_UAtoi(&fileName[Utl_UStrLen(fileName)-7]);//例如REC001.wav,len-7位开始为数字

		if ((id > 0) && (id <= MaxInd))
		{
			sprintf(TmpName, pMatchName, id);
	        Utl_StrMbcs2Ucs(TmpName, UcsTmpName);

			Utl_USplitFilePath(fileName, AK_NULL, name);

			if (0 == Utl_UStrCmp(name, UcsTmpName))
			{
				FileCom_SetBitValue(pBitmap, bitmapSize, id - 1, 1);
				count++;
			}
		}
		
		pFindHandle = Fwl_FsFindNext_WithID(pFindHandle, 1, &ret_cnt);
	}

	Fwl_FsFindClose_WithID(pFindHandle);

	freeId = FileCom_FindFirstFreeId(pBitmap, bitmapSize);

	AK_DEBUG_OUTPUT("freeId : %d\n", freeId);

	if (freeId < MaxInd)
	{
		*index = freeId;
		ret = AK_TRUE;
	}

	pBitmap = Fwl_Free(pBitmap);
	
	Fwl_Freq_Clr_Add();
    Fwl_FreqPop(); 

    Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    return ret;
}

