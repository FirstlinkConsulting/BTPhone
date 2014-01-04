/**
 * @file svc_medialist.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include <string.h>
#include "svc_medialist.h"
#include "fwl_osfs.h"
#include "eng_string_uc.h"
#include "eng_string.h"
#include "eng_debug.h"



#define MUSICLIST_DEF_NAME      "musiclist.lst"

typedef enum {
    eMLIST_SEARCH_FIRST = 0,
	eMLIST_SEARCH_NEXT,
	eMLIST_SEARCH_PRE,
	eMLIST_SEARCH_LAST,
    eMLIST_SEARCH_FLAG_NUM
} T_eMLIST_SEARCH_FLAG;

typedef struct
{
	T_FILEINFO_BEFOREPOWER		Info; 						//需要保存的查找信息
	T_FILEINFO_BEFOREPOWER		FolderInfo; 				//需要保存的文件夹查找信息
	T_U16 						*pSearchPath;				//需要查找的最高级目录指针
	T_U16 						*pFileFormat;				//查找文件格式的过滤器
	T_U16 						*pSkipFolder;				//查找文件夹的过滤器
	T_U32						pFindHandle;				//文件查找的句柄 
	T_U16						MusicPath[MAX_FILE_LEN];	//音频文件全路径

}T_SearchInfo;


typedef struct {
	
	T_hFILE 		Fd;					//列表文件的句柄
	T_BOOL  		bDividFolders;		//是否分文件夹
	T_BOOL  		bHaveItem;			//是否有音频
	T_U16			LoadFlag;			//搜索完成与否的标记
	T_U16 			Listname[20];		//列表名称（与应用一一对应的标识）
	T_SearchInfo	SearchInfo;
}T_MEDIA_LIST;

static T_MEDIA_LIST *pMList = AK_NULL;


static T_BOOL MList_GetListDir(T_pWSTR path, T_pCWSTR listname, T_pCWSTR searchpath);
static T_BOOL MList_GetFileListPath(T_pWSTR path, T_MEDIA_LIST *pList);
static T_BOOL MList_CheckOldFile(T_MEDIA_LIST *pList, T_pCWSTR listPath);
static T_BOOL MList_SearchFirstFile(T_MEDIA_LIST *pList);

static T_BOOL MList_CheckFileList(T_MEDIA_LIST *pList, T_pCWSTR listPath);
static T_BOOL MList_IsInFolder(T_MEDIA_LIST *pList, T_pCWSTR path, T_pWSTR folderPath);
static T_BOOL MList_SearchFolderFile(T_MEDIA_LIST *pList, T_eMLIST_SEARCH_FLAG flag);
static T_BOOL MList_SearchRootFile(T_MEDIA_LIST *pList, T_eMLIST_SEARCH_FLAG flag, T_BOOL bCyc);
static T_VOID MList_FindFirst(T_MEDIA_LIST *pList, const T_U16 *path, T_U32 *ret_cnt, 
								T_PFILEINFO_BEFOREPOWER info, T_BOOL deepflag);

static T_VOID MList_FindNext(T_MEDIA_LIST *pList, T_S32 cnt, T_U32 *ret_cnt, T_BOOL deepflag);
static T_VOID MList_FindClose(T_MEDIA_LIST *pList);
/**
* @brief media list init
*
* @author Songmengxing
* @date 2013-04-19
*
* @param in T_pCWSTR listname : listname 
* @param in T_pCWSTR searchpath : the highest path for search. 
* @param in T_BOOL bDividFolders : divide  folders or not. 
* @param in T_U16* fileFormat : file format for search. 
* @param in T_U16* skipFolder : folder name for skip. 
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Init(T_pCWSTR listname, T_pCWSTR searchpath, 
					T_BOOL bDividFolders, T_U16* fileFormat, T_U16* skipFolder)
{
	T_USTR_FILE 	listpath = {0};
	T_USTR_FILE		rootpath = {0};
	T_U16 			len = 0;
	T_U16 			SOLIDUS[2] = {'/',0};
	
	AK_ASSERT_PTR(listname, "MList_Init(): listname", AK_FALSE);
	AK_ASSERT_VAL(*listname, "MList_Init(): *listname", AK_FALSE);
	AK_ASSERT_PTR(searchpath, "MList_Init(): searchpath", AK_FALSE);
	AK_ASSERT_VAL(*searchpath, "MList_Init(): *searchpath", AK_FALSE);
	AK_ASSERT_PTR(fileFormat, "MList_Init(): fileFormat", AK_FALSE);
	AK_ASSERT_PTR(skipFolder, "MList_Init(): skipFolder", AK_FALSE);
	
	if (AK_NULL != pMList)
	{
		AK_DEBUG_OUTPUT("MList_Init pMList is not null!\n");
		MList_Destroy();
	}

	pMList = (T_MEDIA_LIST *)Fwl_Malloc(sizeof(T_MEDIA_LIST));
	AK_ASSERT_PTR(pMList, "MList_Init(): pList malloc fail", AK_FALSE);
	memset(pMList, 0, sizeof(T_MEDIA_LIST));

	pMList->bDividFolders = bDividFolders;
	Utl_UStrCpyN(pMList->Listname, (T_U16*)listname, (T_U32)(MEDIA_LISTNAME_SIZE - 1));
	pMList->Fd = FS_INVALID_HANDLE;

	Utl_UStrCpyN(rootpath, searchpath, (T_U32)(MAX_FILE_LEN - 1));

	len = Utl_UStrLen(rootpath);

	if (rootpath[len-1] != UNICODE_SOLIDUS)
	{
		Utl_UStrCat(rootpath, SOLIDUS);
	}

	len = (Utl_UStrLen(rootpath)+1) << 1;

	//pSearchPath
	pMList->SearchInfo.pSearchPath = Fwl_Malloc(len);

	if (AK_NULL == pMList->SearchInfo.pSearchPath)
	{
		AK_DEBUG_OUTPUT("MList_Init(): pMList->SearchInfo.pSearchPath malloc fail\n");
		MList_Destroy();
		return AK_FALSE;
	}
	
    Utl_UStrCpyN(pMList->SearchInfo.pSearchPath, rootpath, Utl_UStrLen(rootpath));
	AK_DEBUG_OUTPUT("MList_Init :");
	Printf_UC(pMList->SearchInfo.pSearchPath);

	//pFileFormat
	len = (Utl_UStrLen(fileFormat)+1) << 1;

	pMList->SearchInfo.pFileFormat = Fwl_Malloc(len);

	if (AK_NULL == pMList->SearchInfo.pFileFormat)
	{
		AK_DEBUG_OUTPUT("MList_Init(): pMList->SearchInfo.pFileFormat malloc fail\n");
		MList_Destroy();
		return AK_FALSE;
	}
	
    Utl_UStrCpyN(pMList->SearchInfo.pFileFormat, fileFormat, Utl_UStrLen(fileFormat));

	//pSkipFolder
	len = (Utl_UStrLen(skipFolder)+1) << 1;

	pMList->SearchInfo.pSkipFolder = Fwl_Malloc(len);

	if (AK_NULL == pMList->SearchInfo.pSkipFolder)
	{
		AK_DEBUG_OUTPUT("MList_Init(): pMList->SearchInfo.pSkipFolder malloc fail\n");
		MList_Destroy();
		return AK_FALSE;
	}
	
    Utl_UStrCpyN(pMList->SearchInfo.pSkipFolder, skipFolder, Utl_UStrLen(skipFolder));
	
	MList_GetListDir(listpath, pMList->Listname, rootpath);

	if(!Fwl_FileExist(listpath))
    {
        if (!Fwl_FsMkDirs(listpath))
        {
        	AK_DEBUG_OUTPUT("MList_Init Fwl_FsMkDirs fail\n");
        	MList_Destroy();
            return AK_FALSE;
        }
    }
	
	MList_GetFileListPath(listpath, pMList);
		
	if (!MList_CheckFileList(pMList, listpath))
	{
		MList_Destroy();
        return AK_FALSE;
	}	

	AK_DEBUG_OUTPUT("list init OK!\n");

	return AK_TRUE;
}


/**
* @brief Destroy Media List 
*
* @author Songmengxing
* @date 2013-04-19
*
* @param T_VOID
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Destroy(T_VOID)
{
	if (AK_NULL == pMList)
	{
		return AK_FALSE;
	}

	if (FS_INVALID_HANDLE != pMList->Fd)
	{
		Fwl_FileClose(pMList->Fd);
	}

	MList_FindClose(pMList);

	if (AK_NULL != pMList->SearchInfo.pSearchPath)
	{
		pMList->SearchInfo.pSearchPath = Fwl_Free(pMList->SearchInfo.pSearchPath);
	}

	if (AK_NULL != pMList->SearchInfo.pFileFormat)
	{
		pMList->SearchInfo.pFileFormat = Fwl_Free(pMList->SearchInfo.pFileFormat);
	}

	if (AK_NULL != pMList->SearchInfo.pSkipFolder)
	{
		pMList->SearchInfo.pSkipFolder = Fwl_Free(pMList->SearchInfo.pSkipFolder);
	}

	pMList = Fwl_Free(pMList);

	AK_DEBUG_OUTPUT("list close OK!\n");

	return AK_TRUE;	
}


/**
* @brief have item or not
*
* @author Songmengxing
* @date 2013-05-27
*
* @param T_VOID
*
* @return T_BOOL 
* @retval 
*/
T_BOOL MList_IsHaveItem(T_VOID)
{
	if (AK_NULL == pMList)
	{
		return AK_FALSE;
	}

	return pMList->bHaveItem;
}




/**
* @brief Get item Path
*
* @author Songmengxing
* @date 2013-04-19
*
* @param out T_pWSTR path : Path 
* @param in T_S8 dir : -1, previous; 0, current; 1, next
* @param in T_BOOL bCyc : cycle or not
*
* @return T_BOOL 
* @retval
*/
T_BOOL MList_GetItem(T_pWSTR path, T_S8 dir, T_BOOL bCyc)
{
	T_U32 ret_cnt = 0;
	
	AK_ASSERT_PTR(path, "MList_GetItem(): path", AK_FALSE);
		
	if (AK_NULL == pMList)
	{
		return AK_FALSE;
	}

	if (!pMList->bHaveItem)
	{
		if (MList_SearchFirstFile(pMList))
		{
			pMList->bHaveItem = AK_TRUE;
		}
		else
		{
			pMList->bHaveItem = AK_FALSE;
			pMList->LoadFlag = LIST_LOAD_END_FLAG;
			return AK_FALSE;
		}
	}

	switch (dir)
	{
	case LIST_FIND_DIR_CUR:
		Fwl_FileSeek(pMList->Fd, 2 * sizeof(T_FILEINFO_BEFOREPOWER), FS_SEEK_SET);
		Fwl_FileRead(pMList->Fd, pMList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
		break;

	case LIST_FIND_DIR_NEXT:
		if (!pMList->bDividFolders)	//不分文件夹的情况
		{
			MList_FindNext(pMList, 1, &ret_cnt, AK_TRUE);
			
			if (0 == ret_cnt)
			{
				if (bCyc)
				{					
					MList_FindFirst(pMList, pMList->SearchInfo.pSearchPath, &ret_cnt, AK_NULL, AK_TRUE);
				}
				else
				{
					AK_DEBUG_OUTPUT("MList_GetItem cann't find the next one!\n");
					return AK_FALSE;
				}
			}
		}
		else	//分文件夹的情况
		{
			T_USTR_FILE	FolderPath = {0};
			
			if (MList_IsInFolder(pMList, pMList->SearchInfo.MusicPath, FolderPath))
			{
				//在某个文件夹内
				MList_FindNext(pMList, 1, &ret_cnt, AK_TRUE);
				
				if (0 == ret_cnt)
				{
					if (bCyc)
					{					
						MList_FindFirst(pMList, FolderPath, &ret_cnt, AK_NULL, AK_TRUE);
					}
					else
					{
						AK_DEBUG_OUTPUT("MList_GetItem cann't find the next one!\n");
						return AK_FALSE;
					}
				}
			}
			else	//在根目录
			{
				if (!MList_SearchRootFile(pMList, eMLIST_SEARCH_NEXT, bCyc))
				{
					AK_DEBUG_OUTPUT("MList_GetItem cann't find the next one!\n");
					return AK_FALSE;
				}
			}
		}

		Fwl_FsGet_findfile_info_WithID(pMList->SearchInfo.pFindHandle, &pMList->SearchInfo.Info);
		Fwl_FileSeek(pMList->Fd, 0, FS_SEEK_SET);
		Fwl_FileWrite(pMList->Fd, &pMList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
		Fwl_FileWrite(pMList->Fd, &pMList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
		Fwl_FileWrite(pMList->Fd, pMList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
		Fwl_FileFlush(pMList->Fd);

		break;

	case LIST_FIND_DIR_PRE:
		if (pMList->bDividFolders 
			&& !MList_IsInFolder(pMList, pMList->SearchInfo.MusicPath, AK_NULL))
		{
			MList_SearchRootFile(pMList, eMLIST_SEARCH_PRE, AK_TRUE);
		}
		else
		{
			MList_FindNext(pMList, -1, &ret_cnt, AK_TRUE);
		}

		Fwl_FsGet_findfile_info_WithID(pMList->SearchInfo.pFindHandle, &pMList->SearchInfo.Info);
		Fwl_FileSeek(pMList->Fd, 0, FS_SEEK_SET);
		Fwl_FileWrite(pMList->Fd, &pMList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
		Fwl_FileWrite(pMList->Fd, &pMList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
		Fwl_FileWrite(pMList->Fd, pMList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
		Fwl_FileFlush(pMList->Fd);
		break;

	default:
		Fwl_FileSeek(pMList->Fd, 2 * sizeof(T_FILEINFO_BEFOREPOWER), FS_SEEK_SET);
		Fwl_FileRead(pMList->Fd, pMList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
		break;
	}

	if (Utl_UStrLen(pMList->SearchInfo.MusicPath) >= MAX_FILE_LEN)
	{
		AK_DEBUG_OUTPUT("MList_GetItem MusicPath error!\n");
		return AK_FALSE;
	}

	Utl_UStrCpyN(path, pMList->SearchInfo.MusicPath, (T_U32)(MAX_FILE_LEN - 1));

	return AK_TRUE;
}


/**
* @brief change folder
*
* @author Songmengxing
* @date 2013-04-19
*
* @param in T_S8 dir : -1, previous; 0, current; 1, next
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_ChangeFolder(T_S8 dir)
{		
	T_USTR_FILE	FolderPath = {0};
	T_U32 ret_cnt = 0;
	
	if (AK_NULL == pMList)
	{
		return AK_FALSE;
	}

	if (!pMList->bDividFolders)
	{
		return AK_FALSE;
	}

	if (MList_IsInFolder(pMList, pMList->SearchInfo.MusicPath, AK_NULL))
	{
		//在某个文件夹内
		if (LIST_FIND_DIR_NEXT == dir)
		{
			//找下一个文件夹的音乐
			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_NEXT))
			{
				return AK_TRUE;
			}

			//找根目录的零散音乐
			if (MList_SearchRootFile(pMList, eMLIST_SEARCH_FIRST, AK_FALSE))
			{
				return AK_TRUE;
			}

			//找第一个文件夹的音乐
			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_FIRST))
			{
				return AK_TRUE;
			}
		}
		else if (LIST_FIND_DIR_PRE == dir)
		{
			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_PRE))
			{
				return AK_TRUE;
			}

			if (MList_SearchRootFile(pMList, eMLIST_SEARCH_FIRST, AK_FALSE))
			{
				return AK_TRUE;
			}

			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_LAST))
			{
				return AK_TRUE;
			}
		}
			
	}
	else	//在根目录
	{
		if (LIST_FIND_DIR_NEXT == dir)
		{
			//找第一个文件夹的音乐
			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_FIRST))
			{
				return AK_TRUE;
			}
		}
		else if (LIST_FIND_DIR_PRE == dir)
		{
			if (MList_SearchFolderFile(pMList, eMLIST_SEARCH_LAST))
			{
				return AK_TRUE;
			}
		}
	}


	Fwl_FileSeek(pMList->Fd, 0, FS_SEEK_SET);
	Fwl_FileRead(pMList->Fd, &pMList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
	Fwl_FileRead(pMList->Fd, &pMList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
	Fwl_FileRead(pMList->Fd, pMList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
	AK_DEBUG_OUTPUT("MList_ChangeFolder OldPath : \n");
	Printf_UC(pMList->SearchInfo.MusicPath);

	if (MList_IsInFolder(pMList, pMList->SearchInfo.MusicPath, FolderPath))	//在某个文件夹内
	{
		AK_DEBUG_OUTPUT("MList_ChangeFolder it is in folder : \n");
		Printf_UC(FolderPath);
		
		MList_FindFirst(pMList, FolderPath, &ret_cnt, &pMList->SearchInfo.Info, AK_TRUE);
	}
	else	//在根目录
	{
		AK_DEBUG_OUTPUT("MList_ChangeFolder it is not in folder!\n");
		
		MList_FindFirst(pMList, pMList->SearchInfo.pSearchPath, 
				&ret_cnt, &pMList->SearchInfo.Info, AK_FALSE);
	}
	
	return AK_FALSE;
}


/**
* @brief get load flag
*
* @author Songmengxing
* @date 2013-04-19
*
* @param T_VOID
*
* @return T_U16
* @retval load flag
*/
T_U16 MList_GetLoadFlag(T_VOID)
{
	if (AK_NULL == pMList)
	{
		return 0;
	}

	return pMList->LoadFlag;
}


/**
* @brief delete media list file
*
* @author Songmengxing
* @date 2013-04-19
*
* @param in T_pCWSTR listname : list name
* @param in T_pCWSTR searchpath : the highest path. 
*
* @return T_BOOL
* @retval
*/
T_BOOL MList_DelList(T_pCWSTR listname, T_pCWSTR searchpath)
{
	T_USTR_FILE 	listdir = {0};

	AK_ASSERT_PTR(listname, "MList_DelList(): listname", AK_FALSE);
	AK_ASSERT_VAL(*listname, "MList_DelList(): *listname", AK_FALSE);
	AK_ASSERT_PTR(searchpath, "MList_DelList(): searchpath", AK_FALSE);
	AK_ASSERT_VAL(*searchpath, "MList_DelList(): *searchpath", AK_FALSE);

	/*
	若文件处于打开状态，则会删除失败，
	为保证文件能被删除，先执行MList_Destroy
	*/
	if (AK_NULL != pMList)
	{
		if (0 == Utl_UStrCmpN(listname, pMList->Listname, MEDIA_LISTNAME_SIZE))
		{
			MList_Destroy();
		}
	}

	if (MList_GetListDir(listdir, listname, searchpath))
	{
		return Fwl_FileDelete(listdir);
	}

	return AK_FALSE;
}


/**
* @brief Get the list file dir
*
* @author Songmengxing
* @date 2013-04-19
*
* @param out T_pWSTR path : list file path
* @param in T_pCWSTR listname : list name
* @param in T_pCWSTR searchpath : the highest path for search. 
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_GetListDir(T_pWSTR path, T_pCWSTR listname, T_pCWSTR searchpath)
{	
	T_U16 	str[] =  {'l','i','s','t','\0'};
	
	AK_ASSERT_PTR(path, "MList_GetListDir(): path", AK_FALSE);
	AK_ASSERT_PTR(listname, "MList_GetListDir(): listname", AK_FALSE);
	AK_ASSERT_PTR(searchpath, "MList_GetListDir(): searchpath", AK_FALSE);

	Utl_UStrCpyN(path, (T_U16*)searchpath, 3);
	Utl_UStrCat(path, listname);
	Utl_UStrCat(path, str);
		
	return AK_TRUE;
}


/**
* @brief Get the music list file full path
*
* @author Songmengxing
* @date 2013-04-19
*
* @param out T_pWSTR path : list file path
* @param in T_MEDIA_LIST *pList : list handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_GetFileListPath(T_pWSTR path, T_MEDIA_LIST *pList)
{
	T_U16 SOLIDUS[2] = {'/',0};
	T_USTR_FILE 	name = {0};
		
	AK_ASSERT_PTR(path, "MList_GetFileListPath(): path", AK_FALSE);
	AK_ASSERT_PTR(pList, "MList_GetFileListPath(): pList", AK_FALSE);

	if (MList_GetListDir(path, pList->Listname, pList->SearchInfo.pSearchPath))
	{
		Utl_UStrCat(path, SOLIDUS);

		Utl_StrMbcs2Ucs((MUSICLIST_DEF_NAME), name);
		Utl_UStrCat(path, name);

		return AK_TRUE;
	}

	return AK_FALSE;
}


/**
* @brief check the path is in folder or not
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_pCWSTR path : path
* @param out T_pWSTR folderPath : folderPath
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_IsInFolder(T_MEDIA_LIST *pList, T_pCWSTR path, T_pWSTR folderPath)
{
	T_U16 i = 0;

	AK_ASSERT_PTR(pList, "MList_IsInFolder(): pList", AK_FALSE);
	AK_ASSERT_PTR(path, "MList_IsInFolder(): path", AK_FALSE);
	
	for (i=Utl_UStrLen(pList->SearchInfo.pSearchPath); i<Utl_UStrLen(path); i++)
	{
		if (UNICODE_SOLIDUS == path[i] || UNICODE_RES_SOLIDUS == path[i])
		{
			AK_DEBUG_OUTPUT("MList_IsInFolder it is in folder!\n");
			
			if (AK_NULL != folderPath)
			{
				Utl_UStrCpyN(folderPath, (T_U16*)path, i);
			}
			
			return AK_TRUE;
		}
	}

	return AK_FALSE;
}



/**
* @brief check old file
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_pCWSTR listPath : listPath
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_CheckOldFile(T_MEDIA_LIST *pList, T_pCWSTR listPath)
{
	T_USTR_FILE	OldPath = {0};
	T_USTR_FILE	FolderPath = {0};
	T_U32 ret_cnt = 0;
	
	
	AK_ASSERT_PTR(pList, "MList_CheckOldFile(): pList", AK_FALSE);
	AK_ASSERT_PTR(listPath, "MList_CheckOldFile(): listPath", AK_FALSE);

	if (FS_INVALID_HANDLE != pList->Fd)
	{
		return AK_TRUE;
	}
	
	pList->Fd = Fwl_FileOpen(listPath, _FMODE_OVERLAY, _FMODE_OVERLAY);
	
	if (FS_INVALID_HANDLE != pList->Fd)
	{
		if (Fwl_GetFileLen(pList->Fd) == 2 * sizeof(T_FILEINFO_BEFOREPOWER) + MEDIA_PATH_SIZE)
		{
			Fwl_FileRead(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileRead(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileRead(pList->Fd, OldPath, MEDIA_PATH_SIZE);
			AK_DEBUG_OUTPUT("MList_CheckOldFile OldPath : \n");
			Printf_UC(OldPath);

			if (!pList->bDividFolders)	//不分文件夹的情况
			{				
				MList_FindFirst(pList, pList->SearchInfo.pSearchPath, 
						&ret_cnt, &pList->SearchInfo.Info, AK_TRUE);
			}
			else	//分文件夹的情况
			{
				if (MList_IsInFolder(pList, OldPath, FolderPath))	//在某个文件夹内
				{
					AK_DEBUG_OUTPUT("MList_CheckOldFile it is in folder : \n");
					Printf_UC(FolderPath);
					
					MList_FindFirst(pList, FolderPath, &ret_cnt, &pList->SearchInfo.Info, AK_TRUE);
				}
				else	//在根目录
				{
					AK_DEBUG_OUTPUT("MList_CheckOldFile it is not in folder!\n");
					
					MList_FindFirst(pList, pList->SearchInfo.pSearchPath, 
							&ret_cnt, &pList->SearchInfo.Info, AK_FALSE);
				}
				
			}

			AK_DEBUG_OUTPUT("MList_CheckOldFile MusicPath : \n");
			Printf_UC(pList->SearchInfo.MusicPath);

			if ((0 != ret_cnt) 
				&& (0 == Utl_UStrCmp(pList->SearchInfo.MusicPath, OldPath)))
			{
				AK_DEBUG_OUTPUT("MList_CheckOldFile old file is ok!\n");
				return AK_TRUE;
			}
			else	//保存的旧信息无效了
			{
				AK_DEBUG_OUTPUT("MList_CheckOldFile file need update, delete it!\n");
				Fwl_FileDeleteHandle(pList->Fd);
				Fwl_FileClose(pList->Fd);
				MList_FindClose(pList);
			}
			
		}
		else	//文件大小不对，怀疑是损坏文件，删除
		{
			AK_DEBUG_OUTPUT("MList_CheckOldFile file error, delete it!\n");
			Fwl_FileDeleteHandle(pList->Fd);
			Fwl_FileClose(pList->Fd);
		}
	}

	return AK_FALSE;
}


/**
* @brief search the first music file
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_SearchFirstFile(T_MEDIA_LIST *pList)
{
	T_U32 ret_cnt = 0;
	
	AK_ASSERT_PTR(pList, "MList_SearchFirstFile(): pList", AK_FALSE);

	if (!pList->bDividFolders)	//不分文件夹的情况
	{
		MList_FindFirst(pList, pList->SearchInfo.pSearchPath, &ret_cnt, AK_NULL, AK_TRUE);
		
		if (0 != ret_cnt)
		{
			Fwl_FsGet_findfile_info_WithID(pList->SearchInfo.pFindHandle, &pList->SearchInfo.Info);
			Fwl_FileSeek(pList->Fd, 0, FS_SEEK_SET);
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, pList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
			Fwl_FileFlush(pList->Fd);
			return AK_TRUE;
		}
		else
		{
			AK_DEBUG_OUTPUT("MList_SearchFirstFile no music!\n");
			memset(&pList->SearchInfo.Info, 0, sizeof(T_FILEINFO_BEFOREPOWER));
			memset(&pList->SearchInfo.FolderInfo, 0, sizeof(T_FILEINFO_BEFOREPOWER));
			memset(pList->SearchInfo.MusicPath, 0, MEDIA_PATH_SIZE);

			Fwl_FileSeek(pList->Fd, 0, FS_SEEK_SET);
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, pList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
			Fwl_FileFlush(pList->Fd);
			return AK_FALSE;
		}
	}
	else	//分文件夹的情况
	{
		//先搜索根目录下的零散音乐
		if (MList_SearchRootFile(pList, eMLIST_SEARCH_FIRST, AK_FALSE))
		{
			return AK_TRUE;
		}

		//根目录没有零散的音乐，再找第一个文件夹的音乐
		if (MList_SearchFolderFile(pList, eMLIST_SEARCH_FIRST))
		{
			return AK_TRUE;
		}

		AK_DEBUG_OUTPUT("MList_SearchFirstFile no music!\n");

		return AK_FALSE;
	}

	return AK_FALSE;
}




/**
* @brief check file list
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_pCWSTR listPath : listPath
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_CheckFileList(T_MEDIA_LIST *pList, T_pCWSTR listPath)
{
	AK_ASSERT_PTR(pList, "MList_CheckFileList(): pList", AK_FALSE);
	AK_ASSERT_PTR(listPath, "MList_CheckFileList(): listPath", AK_FALSE);
	
	if (MList_CheckOldFile(pList, listPath))
	{
		pList->bHaveItem = AK_TRUE;
		return AK_TRUE;
	}
	
	pList->Fd = Fwl_FileOpen(listPath, _FMODE_CREATE, _FMODE_CREATE);

	if (FS_INVALID_HANDLE == pList->Fd)
	{
		AK_DEBUG_OUTPUT("MList_CheckFileList pMList->Fd create fail\n");
		return AK_FALSE;
	}

	return AK_TRUE;
}


/**
* @brief search music file in folder
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_eMLIST_SEARCH_FLAG flag : flag
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_SearchFolderFile(T_MEDIA_LIST *pList, T_eMLIST_SEARCH_FLAG flag)
{
	T_S32 cnt = 0;
	T_U32 pFolderFindHd = 0;
	T_USTR_FILE	FolderPath = {0};
	T_U32 ret_cnt = 0;
	T_PFILEINFO FileInfo = AK_NULL;
	T_FINDBUFINFO findinfo = {0};
	
	AK_ASSERT_PTR(pList, "MList_SearchFolderFile(): pList", AK_FALSE);
	AK_ASSERT_VAL(flag < eMLIST_SEARCH_FLAG_NUM, "MList_SearchFolderFile(): flag", AK_FALSE);

	switch (flag)
	{
		case eMLIST_SEARCH_NEXT:
			cnt = 1;
			findinfo.find_fileinfo = &pList->SearchInfo.FolderInfo;
			findinfo.find_type = SEARCH_TYPE_ITERATE;
			break;

		case eMLIST_SEARCH_PRE: 
			cnt = -1;
			findinfo.find_fileinfo = &pList->SearchInfo.FolderInfo;
			findinfo.find_type = SEARCH_TYPE_NOTITERATE;
			break;

		case eMLIST_SEARCH_FIRST: 
			cnt = 1;
			findinfo.find_type = SEARCH_TYPE_ITERATE;
			break;

		case eMLIST_SEARCH_LAST: 
			cnt = -1;
			findinfo.find_type = SEARCH_TYPE_ITERATE;
			break;

		default:
			return AK_FALSE;
	}
	
SEARCH_FOLDERFILE_FIRST:

	AK_DEBUG_OUTPUT("find cur folder path:");
	Printf_UC(pList->SearchInfo.pSearchPath);

	MList_FindClose(pList);

	findinfo.deep_flag = AK_FALSE;
	findinfo.pattern = pList->SearchInfo.pFileFormat;
	findinfo.fileter_pattern = pList->SearchInfo.pSkipFolder;

	//先找到第一个文件夹或当前的文件夹
	pFolderFindHd = Fwl_FsFindFirst_WithID(pList->SearchInfo.pSearchPath, 
											&ret_cnt, FolderPath, &findinfo);

	AK_DEBUG_OUTPUT("find cur folder :");
	Printf_UC(FolderPath);

	if (0 == ret_cnt)
	{
		Fwl_FsFindClose_WithID(pFolderFindHd);
		return AK_FALSE;
	}

	if (eMLIST_SEARCH_FIRST != flag)
	{
		goto SEARCH_FOLDERFILE_NEXT;
	}

	while (0 != ret_cnt)
	{		
		AK_DEBUG_OUTPUT("find a file or folder :");
		Printf_UC(FolderPath);
		FileInfo = Fwl_FsFindInfo_WithID(pFolderFindHd, 0, AK_NULL, AK_NULL);
		
		if (Fwl_FsFindIsFolder(pFolderFindHd, FileInfo))
        {
			Fwl_FsGet_findfile_info_WithID(pFolderFindHd, &pList->SearchInfo.FolderInfo);

			Fwl_FsFindClose_WithID(pFolderFindHd);
			pFolderFindHd = 0;

			//找该文件夹内第一首音乐
			MList_FindFirst(pList, FolderPath, &ret_cnt, AK_NULL, AK_TRUE);

			if (0 != ret_cnt)
			{
				AK_DEBUG_OUTPUT("find a file :");
				Printf_UC(pList->SearchInfo.MusicPath);
				Fwl_FsGet_findfile_info_WithID(pList->SearchInfo.pFindHandle, &pList->SearchInfo.Info);
				Fwl_FileSeek(pList->Fd, 0, FS_SEEK_SET);
				Fwl_FileWrite(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
				Fwl_FileWrite(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
				Fwl_FileWrite(pList->Fd, pList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
				Fwl_FileFlush(pList->Fd);
				
				return AK_TRUE;
			}
			else
			{
				findinfo.find_fileinfo = &pList->SearchInfo.FolderInfo;
				
				if (eMLIST_SEARCH_FIRST == flag)
				{
					flag = eMLIST_SEARCH_NEXT;
				}
				
				goto SEARCH_FOLDERFILE_FIRST;
			}
		}


SEARCH_FOLDERFILE_NEXT:
		pFolderFindHd = Fwl_FsFindNext_WithID(pFolderFindHd, cnt, &ret_cnt);

	}
	

	Fwl_FsFindClose_WithID(pFolderFindHd);
	return AK_FALSE;
}


/**
* @brief search music file in the root path
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_eMLIST_SEARCH_FLAG flag : flag
* @param in T_BOOL bCyc : cycle or not
*
* @return T_BOOL
* @retval
*/
static T_BOOL MList_SearchRootFile(T_MEDIA_LIST *pList, T_eMLIST_SEARCH_FLAG flag, T_BOOL bCyc)
{
	T_U32 ret_cnt = 0;
	T_PFILEINFO FileInfo = AK_NULL;
	T_S32 cnt = 1;

	AK_ASSERT_PTR(pList, "MList_SearchRootFile(): pList", AK_FALSE);
	AK_ASSERT_VAL(flag < eMLIST_SEARCH_FLAG_NUM, "MList_SearchRootFile(): flag", AK_FALSE);

	if (eMLIST_SEARCH_PRE == flag
		|| eMLIST_SEARCH_LAST == flag)
	{
		cnt = -1;
	}

	if (eMLIST_SEARCH_FIRST == flag
		|| eMLIST_SEARCH_LAST == flag)
	{
		MList_FindFirst(pList, pList->SearchInfo.pSearchPath, &ret_cnt, AK_NULL, AK_FALSE);

		if (0 == ret_cnt)
		{
			AK_DEBUG_OUTPUT("MList_SearchRootFile no music!\n");
			memset(&pList->SearchInfo.Info, 0, sizeof(T_FILEINFO_BEFOREPOWER));
			memset(&pList->SearchInfo.FolderInfo, 0, sizeof(T_FILEINFO_BEFOREPOWER));
			memset(pList->SearchInfo.MusicPath, 0, MEDIA_PATH_SIZE);

			Fwl_FileSeek(pList->Fd, 0, FS_SEEK_SET);
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, pList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
			Fwl_FileFlush(pList->Fd);
			MList_FindClose(pList);
			return AK_FALSE;
		}
	}
	
	if (eMLIST_SEARCH_FIRST != flag)
	{
		goto SEARCH_ROOTFILE_NEXT;
	}
	
	while (0 != ret_cnt)
	{
		FileInfo = Fwl_FsFindInfo_WithID(pList->SearchInfo.pFindHandle, 0, AK_NULL, AK_NULL);
		
		if (!Fwl_FsFindIsFolder(pList->SearchInfo.pFindHandle, FileInfo))
        {
			Fwl_FsGet_findfile_info_WithID(pList->SearchInfo.pFindHandle, &pList->SearchInfo.Info);
			Fwl_FileSeek(pList->Fd, 0, FS_SEEK_SET);
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.Info, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, &pList->SearchInfo.FolderInfo, sizeof(T_FILEINFO_BEFOREPOWER));
			Fwl_FileWrite(pList->Fd, pList->SearchInfo.MusicPath, MEDIA_PATH_SIZE);
			Fwl_FileFlush(pList->Fd);
			return AK_TRUE;
		}
		
SEARCH_ROOTFILE_NEXT:

		MList_FindNext(pList, cnt, &ret_cnt, AK_FALSE);

		if (0 == ret_cnt)
		{				
			if (bCyc && (eMLIST_SEARCH_NEXT == flag))	//需要循环到第一首音乐
			{
				MList_FindFirst(pList, pList->SearchInfo.pSearchPath, &ret_cnt, AK_NULL, AK_FALSE);
			}
			else
			{
				return AK_FALSE;
			}
			
		}
	}

	return AK_FALSE;
}

/**
* @brief find first
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in const T_U16 *path : search path
* @param out T_U32 ret_cnt : 0 fail
* @param in T_PFILEINFO_BEFOREPOWER infobeforeppower : info
* @param in T_BOOL deepflag : deepflag
*
* @return T_VOID
* @retval
*/
static T_VOID MList_FindFirst(T_MEDIA_LIST *pList, const T_U16 *path, T_U32 *ret_cnt, 
								T_PFILEINFO_BEFOREPOWER info, T_BOOL deepflag)
{
	T_FINDBUFINFO findinfo = {0};
	
	AK_ASSERT_PTR_VOID(pList, "MList_FindFirst(): pList");
	AK_ASSERT_PTR_VOID(path, "MList_FindFirst(): path");
	AK_ASSERT_PTR_VOID(ret_cnt, "MList_FindFirst(): ret_cnt");

	AK_DEBUG_OUTPUT("MList_FindFirst path : ");
	Printf_UC(path);

	MList_FindClose(pList);

	findinfo.find_fileinfo = info;
	findinfo.deep_flag = deepflag;
	findinfo.find_type = SEARCH_TYPE_ITERATE;
	findinfo.pattern = pList->SearchInfo.pFileFormat;
	findinfo.fileter_pattern = pList->SearchInfo.pSkipFolder;

	pList->SearchInfo.pFindHandle = Fwl_FsFindFirst_WithID(path, ret_cnt, 
													pList->SearchInfo.MusicPath, &findinfo);

	AK_DEBUG_OUTPUT("MList_FindFirst MusicPath : ");
	Printf_UC(pList->SearchInfo.MusicPath);
}


/**
* @brief find next
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_MEDIA_LIST *pList : audio list handle
* @param in T_S32 cnt : 1 or -1
* @param out T_U32 ret_cnt : 0 fail
* @param in T_BOOL deepflag : deepflag
*
* @return T_VOID
* @retval
*/
static T_VOID MList_FindNext(T_MEDIA_LIST *pList, T_S32 cnt, T_U32 *ret_cnt, T_BOOL deepflag)
{
	AK_ASSERT_PTR_VOID(pList, "MList_FindNext(): pList");
	AK_ASSERT_PTR_VOID(ret_cnt, "MList_FindNext(): ret_cnt");

	if (0 == pList->SearchInfo.pFindHandle)
	{
		AK_DEBUG_OUTPUT("MList_FindNext pFindHandle is null!\n");
		return;
	}

	pList->SearchInfo.pFindHandle = Fwl_FsFindNext_WithID(pList->SearchInfo.pFindHandle, 
															cnt, ret_cnt);
	AK_DEBUG_OUTPUT("MList_FindNext MusicPath : ");
	Printf_UC(pList->SearchInfo.MusicPath);
}


static T_VOID MList_FindClose(T_MEDIA_LIST *pList)
{
	AK_ASSERT_PTR_VOID(pList, "MList_FindClose(): pList");

	Fwl_FsFindClose_WithID(pList->SearchInfo.pFindHandle);
	pList->SearchInfo.pFindHandle = 0;
}
