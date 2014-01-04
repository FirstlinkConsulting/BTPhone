/*
MODELNAME: VIDEO RAW RGB
DISCRIPTION:
AUTHOR:
DATE:2011-11
*/
#include "Gbl_Global.h"
#include "log_Raw_RGB.h"
#include "Fwl_osFS.h"
#include "Eng_Debug.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "Eng_String_UC.h"
#include "log_file_com.h"

#ifdef OS_ANYKA

#ifdef CAMERA_SUPPORT
#include <stdio.h>
#include "M_event.h"

// 只有32个字节，如果单独dam_malloc，会浪费4k的内存，不如放常驻里
#pragma arm section zidata = "_bootbss1_"
T_RAW_RGB_INFO Raw_RGB_info;
#pragma arm section zidata

#define RGB_FILELEN (1024*1024)

#define RAWRGB_FILEMAXINDEX    1000

extern T_U32 MMU_Vaddr2Paddr(T_U32 vaddr);
//T_BOOL Preview_CheckIndex(T_pWSTR pFilePath, T_pSTR pMatchName, T_U32 *index, T_U32 MaxInd);
//MMU_Vaddr2Paddr not in the permanent code

#pragma arm section code = "_cam_raw_"

/***********************************************************************************
FUNC NAME: Raw_RGB_Switch_Buf
DESCRIPTION: switch the buf;
INPUT: T_BOOL  bswitch: be switch or not
OUTPUT:nothing
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
static T_BOOL Raw_RGB_Switch_Buf(T_BOOL  bswitch)
{
#ifdef USE_DOUBLE_BUF
	if (bswitch)
	{
		if (RAW_BUF_A == Raw_RGB_info.rawBuf)
		{
			Raw_RGB_info.rawBuf = RAW_BUF_B;
		}
		else
		{
			Raw_RGB_info.rawBuf = RAW_BUF_A;
		}
	}
#else
	Raw_RGB_info.rawBuf = RAW_BUF_A;
#endif
	return  AK_TRUE;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Interrupt_CB
DESCRIPTION: the raw_rgb interrupt callback funciton
INPUT: T_BOOL  bswitch: be occur error or not
OUTPUT:nothing
RETURN:T_VOID.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/

T_VOID Raw_RGB_Interrupt_CB(T_BOOL bError)
{
	if(bError)
	{
		Raw_RGB_info.raw_rgb_state = RAW_RGB_ERROR;
		Raw_RGB_Switch_Buf(AK_FALSE);
	}
	else 
	{
		Raw_RGB_info.raw_rgb_state = RAW_RGB_COPYED;
		Raw_RGB_Switch_Buf(AK_TRUE);
	}
	if (! Raw_RGB_info.bkey_out)
	{
		Raw_RGB_Open();
	//	Raw_RGB_info.raw_rgb_state = RAW_RGB_QUIT;
	}
	else
	{
		Raw_RGB_info.raw_rgb_state = RAW_RGB_QUIT;
	}
}
#pragma arm section code

/***********************************************************************************
FUNC NAME: Raw_RGB_Get_ShotPath
DESCRIPTION: get the short path     eg:"A:/RAW_RGB"
INPUT: T_pWSTR pFileName: FileName
OUTPUT:nothing
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
static T_BOOL Raw_RGB_Get_ShotPath(T_pWSTR pFileName)
{
	T_U8 i = 0;
	T_U8 count = 0;
	T_BOOL flag = AK_FALSE;
	
	if ('\0' == pFileName[0])
	{
		akerror("invalid file name!", 0, 1);
		return AK_FALSE;
	}

	while ('\0' != pFileName[i++])
	{
		if ('/' == pFileName[i])
			count = i;

//estimate the pFileName whether is the full name or the short name
		if ('.' == pFileName[i])
			flag = AK_TRUE;
	}

//get the short path
	if (flag)
		pFileName[count] = '\0';

	return AK_TRUE;
}


/***********************************************************************************
FUNC NAME: Raw_GetCapName
DESCRIPTION: Raw_GetCapName   
INPUT: T_pWSTR path: shortpath  eg:"A:/RAW_RGB"
OUTPUT:pFileName  eg:"A:/RAW_RGB/test.RAW"
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
static T_BOOL Raw_GetCapName (T_pWSTR path, T_pWSTR pFileName)
{
	T_STR_FILE	TmpName;	//not unicode string!
	T_USTR_FILE RawName, RawPath;
	T_BOOL      bRet = AK_FALSE;
    T_CHR tmpStr[100];
	T_hFILE     folder;
    T_SYSTIME   curDate;
    T_U32  capFileNum = 0;
	
//使用时间命名方法

//	akerror("=====Raw_RGB_Init1  ", 0, 1);
//	Printf_UC(path);

	UStrCpyN(RawPath, path, MAX_FILE_LEN - 1);
	if (!Raw_RGB_Get_ShotPath(RawPath))
	{
		akerror("RAW_RGB:Raw_RGB_Get_ShotPath failed!", 0, 1);
		return AK_FALSE;
	}
	
//	akerror("=====Raw_GetCapName  ", 0, 1);
//	Printf_UC(RawPath);

	folder = Fwl_FolderOpen(RawPath, _FMODE_READ, _FMODE_READ);
	if (FS_INVALID_HANDLE == folder)
	{
		if(AK_FALSE == Fwl_FsMkDir(RawPath))
		{
			akerror("RAW_RGB:folder create failed!", 0, 1);
			return AK_FALSE;
		}
		akerror("RAW_RGB:folder create succeed!", 0, 1);
		Fwl_FolderClose(folder);
	}
	Fwl_FolderClose(folder);

    memset(&curDate,0,sizeof(T_SYSTIME));
    Fwl_GetRTCtime(&curDate);
    
    sprintf(tmpStr,"%s_%d-%02d-%02d_%02d-%02d-%02d_%s.RAW","SPOT11",curDate.year,curDate.month,curDate.day,
            curDate.hour,curDate.minute,curDate.second,"%06ld");
/* 
//使用索引命名方法
    if (SHOT_MULTISHOT == pCapture->pCapSetup->mode)
    {
        sprintf(tmpStr,"%s_%s.JPG","SPOT11","%06ld_1");
    }
    else
    {
        sprintf(tmpStr,"%s_%s.JPG","SPOT11","%06ld");
    }

    */

    if (FileCom_CheckIndex(RawPath, tmpStr, &capFileNum, RAWRGB_FILEMAXINDEX))
    {
        capFileNum++;
 		sprintf(TmpName, tmpStr, capFileNum);

		akerror("tmpStr:", 0, 0);
		Printf_UC(tmpStr);
		
		Utl_StrMbcs2Ucs(TmpName, RawName);

		UStrCpyN(pFileName, RawPath, MAX_FILE_LEN - 1);

		akerror("pFileName:", 0, 0);
		Printf_UC(pFileName);

		pFileName[UStrLen(RawPath)] = '/';
		pFileName[UStrLen(RawPath)+1] = '\0';
		UStrCat(pFileName, RawName);
		akerror("pFileName:", 0, 0);
		Printf_UC(pFileName);

		bRet = AK_TRUE;
    }
    else
    {
        bRet = AK_FALSE;
    //    pFileName[0] = 0;
    }
	return bRet;
}

static T_BOOL  Raw_RGB_Set_Szie(T_U32 width, T_U32 height)
{
	//estimate the paraments are valid or not;
	//if (width > 320 || height >240)

	Raw_RGB_info.P_RAW_INFO->cap_height = height;
	Raw_RGB_info.P_RAW_INFO->cap_width = width;

	return AK_TRUE;
}

T_RAW_RGB_STATE  Raw_RGB_Get_state(T_VOID)
{
	return Raw_RGB_info.raw_rgb_state;
}


/***********************************************************************************
FUNC NAME: Raw_RGB_Reset
DESCRIPTION: Raw_RGB_Reset   
INPUT: void
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
static T_BOOL Raw_RGB_Reset()
{
	Raw_RGB_info.bkey_out = AK_FALSE;
	Raw_RGB_info.rawBuf = 0;
	Raw_RGB_info.dmabuf[0] = AK_NULL;
	Raw_RGB_info.dmabuf[1] = AK_NULL;
	Raw_RGB_info.raw_rgb_state = RAW_RGB_INVALID;

	if (AK_NULL != Raw_RGB_info.P_RAW_INFO)
	{
		Raw_RGB_info.P_RAW_INFO->cap_height = 240;
		Raw_RGB_info.P_RAW_INFO->cap_width = 320;
		Raw_RGB_info.P_RAW_INFO->path[0] = '\0';
		Raw_RGB_info.P_RAW_INFO->pbufa = AK_NULL;
		Raw_RGB_info.P_RAW_INFO->pbufb = AK_NULL;
		Raw_RGB_info.P_RAW_INFO->raw_rgb_file = FS_INVALID_HANDLE;
		Raw_RGB_info.P_RAW_INFO->bclose = AK_FALSE;
	}
	else
	{
		return AK_FALSE;
	}

	return AK_TRUE;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Init
DESCRIPTION: Raw_RGB_Init   
INPUT: path width height
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL  Raw_RGB_Init(T_U16 *path, T_U32 width, T_U32 height)
{
	Raw_RGB_info.P_RAW_INFO = (T_RAW_RGB_INFO_EX *)Fwl_Malloc(sizeof(T_RAW_RGB_INFO_EX));

	if (AK_NULL == Raw_RGB_info.P_RAW_INFO)
	{
		AK_DEBUG_OUTPUT("RAW_RGB:malloc failed!\n");

		Raw_RGB_info.binit_ok = AK_FALSE;
		return AK_FALSE;
	}
	
	Raw_RGB_Reset();

	Raw_RGB_Set_Szie(width, height);

	//need to create a function that get the filename;
	
	//get_file_name();
	Raw_GetCapName(path, Raw_RGB_info.P_RAW_INFO->path);
	
	if (!Raw_RGB_Start_Next_file())
	{
		AK_DEBUG_OUTPUT("RAW_RGB:open file failed!\n");
		Fwl_Free(Raw_RGB_info.P_RAW_INFO);
		Raw_RGB_info.binit_ok = AK_FALSE;
		
		return AK_FALSE;
	}

/*//	akerror("=====Raw_RGB_Init1  ", 0, 0);
//	Printf_UC(Raw_RGB_info.path);

	Raw_RGB_info.raw_rgb_file = Fwl_FileOpen(Raw_RGB_info.path, _FMODE_CREATE, _FMODE_CREATE);

	if (FS_INVALID_HANDLE == Raw_RGB_info.raw_rgb_file)
	{
		Printf_UC(Raw_RGB_info.path);
		
		AK_DEBUG_OUTPUT("RAW_RGB:open file failed!\n");
		return AK_FALSE;
	}
	*/


	// raw_RGB open
	if (!(Fwl_RawRGB_Open(Raw_RGB_info.P_RAW_INFO->cap_width, Raw_RGB_info.P_RAW_INFO->cap_height, Raw_RGB_Interrupt_CB)))
	{
		AK_DEBUG_OUTPUT("RAW_RGB:RawRGB_Open failed\n");
		
		Fwl_FileClose(Raw_RGB_info.P_RAW_INFO->raw_rgb_file);
		Fwl_FileDelete(Raw_RGB_info.P_RAW_INFO->path);		
		Fwl_Free(Raw_RGB_info.P_RAW_INFO);
		Raw_RGB_info.binit_ok = AK_FALSE;
		
		return AK_FALSE;
	}
	
	Raw_RGB_info.raw_rgb_state = RAW_RGB_START;

	Raw_RGB_info.P_RAW_INFO->pbufa = (T_U8 *)Fwl_DMAMalloc(Raw_RGB_info.P_RAW_INFO->cap_width * Raw_RGB_info.P_RAW_INFO->cap_height);

#ifdef USE_DOUBLE_BUF
	Raw_RGB_info.P_RAW_INFO->pbufb = (T_U8 *)Fwl_DMAMalloc(Raw_RGB_info.P_RAW_INFO->cap_width * Raw_RGB_info.P_RAW_INFO->cap_height);
#endif
	if ((Raw_RGB_info.P_RAW_INFO->pbufa == AK_NULL))//||(Raw_RGB_info.pbufb == AK_NULL))
	{
		//close the file
		Fwl_FileClose(Raw_RGB_info.P_RAW_INFO->raw_rgb_file);

		akerror("RAW_RGB:", 0, 0);
		Printf_UC(Raw_RGB_info.P_RAW_INFO->path);

		Fwl_FileDelete(Raw_RGB_info.P_RAW_INFO->path);
		Raw_RGB_info.P_RAW_INFO->raw_rgb_file = FS_INVALID_HANDLE;

		//free the ram space
		Fwl_DMAFree(Raw_RGB_info.P_RAW_INFO->pbufa);
		Raw_RGB_info.P_RAW_INFO->pbufa = AK_NULL;

#ifdef USE_DOUBLE_BUF			
		Fwl_DMAFree(Raw_RGB_info.P_RAW_INFO->pbufb);
		Raw_RGB_info.P_RAW_INFO->pbufb = AK_NULL;
#endif
		Fwl_RawRGB_Close();
		Fwl_FileClose(Raw_RGB_info.P_RAW_INFO->raw_rgb_file);
		Fwl_FileDelete(Raw_RGB_info.P_RAW_INFO->path);
		Fwl_Free(Raw_RGB_info.P_RAW_INFO);
		Raw_RGB_info.binit_ok = AK_FALSE;

		return AK_FALSE;
	}
	
	memset(Raw_RGB_info.P_RAW_INFO->pbufa, 0x0, (Raw_RGB_info.P_RAW_INFO->cap_width * Raw_RGB_info.P_RAW_INFO->cap_height));
	Raw_RGB_info.dmabuf[0] = MMU_Vaddr2Paddr((T_U32)Raw_RGB_info.P_RAW_INFO->pbufa);
	Raw_RGB_info.rawBuf = 0;

#ifdef USE_DOUBLE_BUF	
	memset(Raw_RGB_info.P_RAW_INFO->pbufb, 0x0, (Raw_RGB_info.P_RAW_INFO->cap_width * Raw_RGB_info.P_RAW_INFO->cap_height));
	Raw_RGB_info.dmabuf[1] = MMU_Vaddr2Paddr((T_U32)Raw_RGB_info.P_RAW_INFO->pbufb);
#endif

	akerror("RAW_RGB: init succeed!", 0, 1);

	Raw_RGB_info.binit_ok = AK_TRUE;
	return AK_TRUE;
}

#pragma arm section code = "_cam_raw_"
/***********************************************************************************
FUNC NAME: Raw_RGB_Open
DESCRIPTION: start to capture   
INPUT: T_VOID
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL 	Raw_RGB_Open(T_VOID)
{
	//asynchronism start raw_RGB capture
	if (Fwl_RawRGBCapture_Start(Raw_RGB_info.dmabuf[Raw_RGB_info.rawBuf], AK_FALSE))
	{
		Raw_RGB_info.raw_rgb_state = RAW_RGB_COPYING;
	}
	else
	{
		Raw_RGB_info.raw_rgb_state = RAW_RGB_ERROR;
		return AK_FALSE;
	}
	
	return AK_TRUE;
}
#pragma arm section code
/***********************************************************************************
FUNC NAME: Raw_RGB_Handle
DESCRIPTION:Raw_RGB_Handle
INPUT: event   pEventParm
OUTPUT:void
RETURN:T_RAW_RGB_HANDLE_STATE
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_RAW_RGB_HANDLE_STATE  Raw_RGB_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{

	T_U32 state  = RAW_RGB_HANLDE_OPERATE_NONE;

	if (!Raw_RGB_info.binit_ok)
	{
		return RAW_RGB_HANLDE_INIT_ERR;
	}
	
	switch(event)
	{
		case M_EVT_USER_KEY:
			AK_DEBUG_OUTPUT("key:%d, id:%d\n", pEventParm->c.Param1, pEventParm->c.Param2);
			if(kbOK == pEventParm->c.Param1)//&&(PRESS_LONG == pEventParm->c.Param2))		//用户按键事件
			{
				Raw_RGB_info.bkey_out =	AK_TRUE;
				state  = RAW_RGB_HANLDE_QUIT;
			}
			else if((kbMODE == pEventParm->c.Param1)&&(PRESS_SHORT == pEventParm->c.Param2))		//用户按键事件
			{

				if (RAW_RGB_START == Raw_RGB_Get_state())
				{
					Raw_RGB_info.bkey_out = AK_FALSE;
					if (Raw_RGB_Open())
					{
						AK_DEBUG_OUTPUT("RAW_RGB:open!\n");
					}
					else
					{
						AK_DEBUG_OUTPUT("RAW_RGB:close!\n");
						Raw_RGB_info.bkey_out =	AK_TRUE;
						state  = RAW_RGB_HANLDE_QUIT;
					}
				}
				else
				{
					Raw_RGB_info.bkey_out =	AK_TRUE;
					state = RAW_RGB_HANLDE_SAVE;
				}
			}
			break;
		case M_EVT_Z00_POWEROFF:
		case M_EVT_Z01_MUSIC_PLAY:
			AK_DEBUG_OUTPUT("RAW_RGB:power off close!\n");
			Raw_RGB_info.bkey_out =	AK_TRUE;
			state  = RAW_RGB_HANLDE_QUIT;
			Fwl_DelayUs(50000);
			break;
		default:
			break;
	}

	return state;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Close
DESCRIPTION:quit the raw_rgb 
INPUT: T_VOID
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL 	Raw_RGB_Close(T_VOID)
{
	T_BOOL ret = AK_TRUE;
	T_U32 len = 0;
	T_U8 *buf_cur;

	if (!Raw_RGB_Get_init_state())
	{
		return AK_FALSE;
	}
	
	if (RAW_BUF_A == Raw_RGB_info.rawBuf)
	{
		buf_cur = Raw_RGB_info.P_RAW_INFO->pbufa;
	}
	else
	{
		buf_cur = Raw_RGB_info.P_RAW_INFO->pbufb;
	}

	
	if (FS_INVALID_HANDLE != Raw_RGB_info.P_RAW_INFO->raw_rgb_file)
	{
		len = Fwl_FileWrite(Raw_RGB_info.P_RAW_INFO->raw_rgb_file, buf_cur, (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width));

		AK_DEBUG_OUTPUT("RAW_RGB:file close!\n");
		Fwl_FileClose(Raw_RGB_info.P_RAW_INFO->raw_rgb_file);

		if ((len != (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width)) || (RAW_RGB_START == Raw_RGB_Get_state()))
		{
			AK_DEBUG_OUTPUT("len:%d, write_len:%d\n", len , (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width));
			
			akerror("RAW_RGB:delete ", 0, 0);
			Printf_UC(Raw_RGB_info.P_RAW_INFO->path);

			Fwl_FileDelete(Raw_RGB_info.P_RAW_INFO->path);
		}
	}
	else
	{
		ret = AK_FALSE;
	}
	
	Fwl_RawRGB_Close();
	Raw_RGB_info.P_RAW_INFO->bclose = AK_TRUE;

	return ret;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Save
DESCRIPTION:Raw_RGB_Save and start next file
INPUT: T_VOID
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL 	Raw_RGB_Save(T_VOID)
{
	T_BOOL ret = AK_TRUE;
	T_U32 len = 0;
	T_U8 *buf_cur;

	if (!Raw_RGB_Get_init_state())
	{
		return AK_FALSE;
	}
		
	if (RAW_BUF_A == Raw_RGB_info.rawBuf)
	{
		buf_cur = Raw_RGB_info.P_RAW_INFO->pbufa;
	}
	else
	{
		buf_cur = Raw_RGB_info.P_RAW_INFO->pbufb;
	}

	
	if (FS_INVALID_HANDLE != Raw_RGB_info.P_RAW_INFO->raw_rgb_file)
	{
		len = Fwl_FileWrite(Raw_RGB_info.P_RAW_INFO->raw_rgb_file, buf_cur, (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width));
	}
	else
	{
		ret = AK_FALSE;
	}

	AK_DEBUG_OUTPUT("RAW_RGB:file close!\n");
	Fwl_FileClose(Raw_RGB_info.P_RAW_INFO->raw_rgb_file);

	if (len != (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width))
	{

		akerror("len:", len, 1);
		akerror("expected len:", (Raw_RGB_info.P_RAW_INFO->cap_height * Raw_RGB_info.P_RAW_INFO->cap_width), 1);
		
		akerror("RAW_RGB:delete file!", 0, 1);
		Printf_UC(Raw_RGB_info.P_RAW_INFO->path);

		Fwl_FileDelete(Raw_RGB_info.P_RAW_INFO->path);
	}

	Raw_RGB_Start_Next_file();

	Raw_RGB_info.raw_rgb_state = RAW_RGB_START;
		
	return ret;
}


/***********************************************************************************
FUNC NAME: Raw_RGB_Destroy
DESCRIPTION:Raw_RGB_Destroy
INPUT: T_VOID
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/

T_BOOL  Raw_RGB_Destroy(T_VOID)
{
	if (!Raw_RGB_Get_init_state())
	{
		return AK_FALSE;
	}

	Fwl_DMAFree(Raw_RGB_info.P_RAW_INFO->pbufa);
	Raw_RGB_info.P_RAW_INFO->pbufa = AK_NULL;
	AK_DEBUG_OUTPUT("RAW_RGB:free!\n");

#ifdef USE_DOUBLE_BUF
	Fwl_DMAFree(Raw_RGB_info.P_RAW_INFO->pbufb);
	Raw_RGB_info.P_RAW_INFO->pbufb = AK_NULL;
#endif

	Raw_RGB_Reset();

	Fwl_Free(Raw_RGB_info.P_RAW_INFO);
	
	return AK_TRUE;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Start_Next_file
DESCRIPTION:Raw_RGB_Start_Next_file
INPUT: T_VOID
OUTPUT:void
RETURN:ture = success . false = error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/

T_BOOL Raw_RGB_Start_Next_file(T_VOID)
{	
	Raw_GetCapName(Raw_RGB_info.P_RAW_INFO->path, Raw_RGB_info.P_RAW_INFO->path);
	akerror("Raw_RGB:Raw_RGB_Start_Next_file  ", 0, 0);
	Printf_UC(Raw_RGB_info.P_RAW_INFO->path);
		
	Raw_RGB_info.P_RAW_INFO->raw_rgb_file = Fwl_FileOpen(Raw_RGB_info.P_RAW_INFO->path, _FMODE_CREATE, _FMODE_CREATE);

	if (FS_INVALID_HANDLE != Raw_RGB_info.P_RAW_INFO->raw_rgb_file)
	{
		Fwl_SetFileSize(Raw_RGB_info.P_RAW_INFO->raw_rgb_file, RGB_FILELEN);
		return AK_TRUE;
	}
	else
		return AK_FALSE;
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Get_Close_state
DESCRIPTION:Raw_RGB_Get_Close_state
INPUT: T_VOID
OUTPUT:T_BOOL
RETURN:ture = close ok . false = close error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL  Raw_RGB_Get_Close_state(T_VOID)
{
	if (Raw_RGB_info.P_RAW_INFO->bclose)
	{
		return AK_TRUE;
	}
	else
	{
		Raw_RGB_info.bkey_out =	AK_TRUE;
		return AK_FALSE;
	}
}

/***********************************************************************************
FUNC NAME: Raw_RGB_Get_init_state
DESCRIPTION:Raw_RGB_Get_init_state
INPUT: T_VOID
OUTPUT:T_BOOL
RETURN:ture = init ok . false = init error.
AUTHOR:
CREATE DATE:2011-11
MODIFY LOG:
***********************************************************************************/
T_BOOL Raw_RGB_Get_init_state(T_VOID)
{
	return Raw_RGB_info.binit_ok;
}

#endif
#else
T_BOOL  Raw_RGB_Init(T_U16 *path, T_U32 width, T_U32 height)
{
	return AK_FALSE;
}

/*
T_BOOL  Raw_RGB_Set_Szie(T_U32 width, T_U32 height)
{
	return AK_FALSE;
}
*/
T_BOOL 	Raw_RGB_Open(T_VOID)
{
	return AK_FALSE;
}
T_RAW_RGB_HANDLE_STATE  Raw_RGB_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	return RAW_RGB_HANLDE_INVALID;
}
T_BOOL 	Raw_RGB_Close(T_VOID)
{
	return AK_FALSE;
}

T_BOOL 	Raw_RGB_Save(T_VOID)
{
	return AK_FALSE;
}
T_BOOL  Raw_RGB_Destroy(T_VOID)
{
	return AK_TRUE;
}

T_RAW_RGB_STATE  Raw_RGB_Get_state(T_VOID)
{
	return RAW_RGB_INVALID;
}

T_BOOL Raw_RGB_Start_Next_file(T_VOID)
{
	return AK_FALSE;
}
T_BOOL  Raw_RGB_Get_Close_state(T_VOID)
{
	return AK_TRUE;
}
#endif

