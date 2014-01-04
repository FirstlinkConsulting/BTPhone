#include "Gbl_Global.h"
#include "Fwl_osFS.h"
#include "Eng_Debug.h"
#include "Ctrl_ListFile.h"
#include "Ctrl_Button.h"
//#include "Gui_Common.h"
#include "Fwl_FreqMgr.h"

#if(NO_DISPLAY == 0)

#define CTRL_ICON_MINSIZE               40  //略缩图大小
#define CTRL_ICON_SEPRATION             6   //略缩间隔
#define CTRL_LINE_WIDTH                 3   //边框线宽


//控件刷新子标记
#define CTRL_REFRESH_FILELIST_TITLE		0x1	//only repaint title
#define CTRL_REFRESH_FILELIST_ENTRY		0x2	//only repaint 
#define CTRL_REFRESH_FILELIST_ROOT		0x4 //only repaint big directory
#define CTRL_REFRESH_FILELIST_ENTRYEX_ALL 0xFFFF

//文件显示条目个数
#define CTRL_FILELIST_DISP_ENTRYS		(pListFile->entryLines & 0x00FF)

//文件标记
#define CTRL_FILELIST_DIRTAG			((T_U16)'/')
#define CTRL_FILELIST_ATTR_FILE			0x1
#define CTRL_FILELIST_ATTR_FOLD			0x2

//界面层次
#define CTRL_FILELIST_LEVEL_DRIVER		0xFF00 //current GUI is menulist mode
#define CTRL_FILELIST_LEVEL_FILE		0x0000 //current GUI is filelist mode

#define GET_DRIVERCOUNT(magic)			((T_U16)((magic) & 0x00FF))
#define GET_DISPLAYLEVL(magic)			((T_U16)((magic) & 0xFF00))
#define SET_DISPLAYLEVL(magic, mode)	((T_U16)(((magic)&0x00FF) | (mode)))
#define SET_DRIVERCOUNT(magic, cnt)		((T_U16)(((magic)&0xFF00) | (cnt)))

#ifdef USE_CONTROL_NOTITLE
#define CTRL_FILELIST_BEAUTIFY_INTVL    2
#else
#define CTRL_FILELIST_BEAUTIFY_INTVL    0
#endif
            
#define TYPENAMELEN                     5           //文件类型名长度


/* ************************** Porting Funcion Beg ****************************** */

static T_VOID	e_findclose(CListFileCtrl *pListFile);

#pragma arm section code = "_listfile_init_"

static T_VOID	e_findfirst(CListFileCtrl *pListFile, T_U8 cntNode)
{
	T_hFindCtrl hFSHandler = FS_INVALID_HANDLE;
	T_FINDBUFCTRL ctrl;
    T_U32 FileCnt, FolderCnt;

	ctrl.NodeCnt = cntNode;             
	ctrl.type    = pListFile->modeSearch;

	//set find filter string
	if(pListFile->filter && pListFile->filter[0])
	{
		ctrl.pattern = pListFile->filter;
		ctrl.patternLen = (T_U8)Utl_UStrLen(ctrl.pattern);
	}
	else
	{
		ctrl.pattern = AK_NULL;
		ctrl.patternLen = 0;
	}

	pListFile->cntEntrys = 0;
			
	//find entry whick fit to filter and count it
	hFSHandler = Fwl_FsFindFirst(pListFile->bufCache, &ctrl);
	VME_EvtQueueClearTimerEvent();//查找到目录文件后清除堵塞的事件
	if(hFSHandler == FS_INVALID_HANDLE)
	{
	    pListFile->hFSFind = FS_INVALID_HANDLE;
		AK_DEBUG_OUTPUT("FIND: none\n");
        return;
    }
	//Printf_UC(pListFile->bufCache);
	Fwl_FsGetFindInfo(hFSHandler, 0, &FileCnt, &FolderCnt);
	AK_DEBUG_OUTPUT("FIND:0x%x\n", FileCnt + FolderCnt);
	pListFile->cntEntrys = (T_U16)(FileCnt + FolderCnt);
	pListFile->hFSFind = hFSHandler ;	

	if((CTRL_LISTFILE_DISP_FILEONLY & pListFile->modeSearch)
		&& (CTRL_LISTFILE_DISP_PATTERONLY & pListFile->modeSearch))
	{
		//under pattern mode, we only show those fold and file that fit to filter
		//so if some fold not include these entry, it will be eliminate.
		if(0 == FileCnt)
		{
			e_findclose(pListFile);
			AK_DEBUG_OUTPUT("find none!");
			pListFile->hFSFind = FS_INVALID_HANDLE;
		}
	}
}

#pragma arm section code 


#pragma arm section code = "_listfile_handle_"

static T_U32		e_findnext(CListFileCtrl *pListFile, T_S8 reqCnt)
{
	if((FS_INVALID_HANDLE == pListFile->hFSFind )
	    || (0==reqCnt))
		return 0;
	Fwl_FsFindNext(pListFile->hFSFind, reqCnt);	
	return reqCnt;
}
 
#pragma arm section code 

static T_VOID	e_findclose(CListFileCtrl *pListFile)
{
	//close find handle
	if(FS_INVALID_HANDLE != pListFile->hFSFind) 
    {
        Fwl_FsFindClose(pListFile->hFSFind);
        pListFile->hFSFind = FS_INVALID_HANDLE;
    }   
}

#pragma arm section code = "_listfile_init_"

static T_U8		e_getfiledriver(CListFileCtrl *pListFile)
{
	//get how many drives in the flash.
	return (T_U8)Fwl_GetDriverNum(); 
}

#pragma arm section code

#pragma arm section code = "_listfile_show_"

/* ************************** Porting Funcion End ****************************** */
static T_U16*	ListFile_GetString(CListFileCtrl *pListFile, T_U8 pos, T_U8 *attr)
{
	T_U16 *string = AK_NULL;
	T_hFindCtrl hFSHandler = pListFile->hFSFind;
	T_hFILEINFO fileinfo;


	//如果当前显示的是目录盘<菜单列表模式>
	if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_DRIVER)
	{
		//string must be 'A' or 'B' or ..., these string configuration from resource.
		//this product only have 2 disk <e_getfiledriver>
		if(0 == pos)		string = (T_U16*)FILE_STRING_DRIVE_A;
		else if(1 == pos)	string = (T_U16*)FILE_STRING_DRIVE_B;
		else				string = (T_U16*)((T_U32)INVALID_STRINGRES_ID);

		*attr = CTRL_FILELIST_ATTR_FOLD;
	}

	//如果当前显示的是文件浏览<文件列表模式>
	else
	{
		if(!hFSHandler)
			return AK_NULL;
		fileinfo = Fwl_FsGetFindInfo(hFSHandler, pos, AK_NULL, AK_NULL);

		//get entry option
		if(FS_INVALID_HANDLE != fileinfo)
		{
			if(AK_TRUE == Fwl_FsFindIsFolder(hFSHandler, fileinfo))
				*attr = CTRL_FILELIST_ATTR_FOLD;
			else
				*attr = CTRL_FILELIST_ATTR_FILE;

			string = Fwl_FsFindGetName(hFSHandler, fileinfo);
		}

		//under disk root directory, we replace '/' with 'ROOT'.
		if(string && string[0]=='.' && string[1]=='.' && string[2]=='/')
		{
			string[0] = '\\';
			string[1] = 0;

			if((0==pListFile->curCachePos)//到了当前盘根目录
				&& (!(pListFile->dwStyle&CTRL_LISTFILE_STYLE_ROOTDISK_DISPSHOW)//不显示多盘府
				    || (GET_DRIVERCOUNT(pListFile->magicDriver)<=1)))//Only One Disk
			{
				string[0] = 'R';string[1] = 'O';string[2] = 'O';string[3] = 'T';
				string[4] = 0;//string is 256 buf
			}
		}
	}

	return string;
}

static T_VOID ListFile_GetFileType(const T_U16 *fileName, T_U16 *fileType)
{
    T_U16 strlen = 0;           //文件名长度
    T_U16 pos = 0;              //提取类型名的位置

    AK_DEBUG_OUTPUT("\nListFile_GetFileType fileType  ");
    
    if ((fileName != AK_NULL) && (fileType != AK_NULL)) //参数有效才操作
    {
        Printf_UC((T_U16 *)fileName);  
        strlen = Utl_UStrLen(fileName);
        pos = strlen - 1;

        while((*(fileName + pos) != '.') && (pos != 0))//类型名为'.'后的字符串
        {
            pos--;
        }

        if (pos != 0)
        {
            pos++;                                  //去掉'.'
            Utl_UStrCpyN(fileType, (T_U16 *)(fileName + pos), TYPENAMELEN);

            AK_DEBUG_OUTPUT("\nListFile_GetFileType fileType = ");
            Printf_UC(fileType);
        }
    }
    
}

static T_BOOL	ListFile_SetShowRect(CListFileCtrl *pListFile)
{

    T_U16 screenSize = 0;

    if (AK_NULL == pListFile)
    {
        return AK_FALSE;
    }

    screenSize = (T_U16)((pListFile->height > pListFile->width) ? (pListFile->width):(pListFile->height));

    if (pListFile->showMode >= NONENTITY_MODE)
    {
        pListFile->showMode = LISTFILE_MODE;                 //默认显示方式为列表形式
    }

    if (pListFile->wh < CTRL_ICON_MINSIZE)
    {
        pListFile->wh = CTRL_ICON_MINSIZE;     //略缩图的宽高
    }
   

     if (pListFile->sepration < CTRL_ICON_SEPRATION)
    {
        pListFile->sepration = CTRL_ICON_SEPRATION;        
    }

    if ((pListFile->wh + pListFile->sepration) > screenSize)
    {
        pListFile->sepration = CTRL_ICON_SEPRATION;
        pListFile->wh = screenSize - pListFile->sepration;
    }

    if (LISTFILE_MODE == pListFile->showMode)//获得一屏能显示的项数和第一项所在的位置
    {
        pListFile->entryLines = (T_U16)((pListFile->height-CTRL_FILELIST_BEAUTIFY_INTVL)/CTRL_WND_LINEHIGH);

	    if ((CTRL_LISTFILE_DISP_FILEONLY& pListFile->modeSearch)
		    && (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle))
	    {
		    pListFile->entryLines --;
	    }
    }
    else if (DIAGRAM_MODE == pListFile->showMode)
    {
        T_LEN heightTmp = pListFile->height - CTRL_WND_LINEHIGH;     //这一行用于显示当前项的名字
        T_U16 topTmp = CTRL_WND_LINEHIGH;
        
        if ((CTRL_LISTFILE_DISP_FILEONLY& pListFile->modeSearch)        //如果搜索的仅是文件
		&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle)) //要空出一行显示当前目录
	    {
            heightTmp -= CTRL_WND_LINEHIGH;
            topTmp += CTRL_WND_LINEHIGH;
        }

         
        pListFile->row = ((T_U16)heightTmp / (pListFile->wh + (2 * pListFile->sepration)));
        pListFile->column = ((T_U16)pListFile->width / (pListFile->wh + (2 * pListFile->sepration)));
        pListFile->entryLines = pListFile->row  * pListFile->column;//

        pListFile->addDis = ((T_U16)heightTmp % (pListFile->wh + (2 * pListFile->sepration))) / pListFile->row;

        pListFile->offset_x = (((T_U16)pListFile->width % (pListFile->wh + (2 * pListFile->sepration))) / 2) + pListFile->sepration;
        pListFile->offset_y = (T_U16)pListFile->top + topTmp + pListFile->sepration + pListFile->addDis / 2;
        
       }

    return AK_TRUE;
}

T_BOOL	ListFile_InitShowMode(CListFileCtrl *pListFile, T_U16 wh, T_U16 sepration, T_LIST_SHOW_MODE showMode)
{
    if (AK_NULL == pListFile)
    {
        return AK_FALSE;
    }

    pListFile->wh = wh;                         //略缩图大小
    pListFile->sepration = sepration;           //略缩图间的间隔 

    if (pListFile->showMode != showMode)
    {
        pListFile->showMode = showMode;         //预览方式0：文件列表，1：略缩图
        pListFile->refresh  = CTRL_REFRESH_ALL;
	    pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;
        ListFile_SetShowRect(pListFile);

        e_findclose(pListFile);

        //begin to explor parent directory.
		e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));

        if (FS_INVALID_HANDLE == pListFile->hFSFind)
		{
			AK_DEBUG_OUTPUT("Error : ListFile InitShowMode failure\r\n");
			return AK_FALSE;
		}
    }
    
    return AK_TRUE;
  
}


#pragma arm section code 

#pragma arm section code = "_listfile_init_"

T_BOOL	ListFile_InitEx(CListFileCtrl *pListFile, T_U16 *rootDIR, T_U16 spcTitleStringID, T_U32 modeSearch, T_U16 patFilter[CTRL_LISTFILE_FILTERLEN], T_POS left, T_POS top, T_LEN width, T_LEN height)
{
	T_U16 i = 0;
    T_U16 j = 0;
	T_U16 pos = 0;

	if (!pListFile)
    {
        return AK_FALSE;
    }   
		
	pListFile->begSrID  = 0;
	pListFile->curSubID = 0;
	pListFile->modeSearch = (T_U16)modeSearch;
	pListFile->spcTitleStringID = spcTitleStringID;
	pListFile->dwStyle  = CTRL_LISTFILE_STYLE_DEFAULT;
	pListFile->refresh  = CTRL_REFRESH_ALL;
	pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;

	pListFile->bkimgTitle = INVALID_IMAGERES_ID;

	//compute the number of lines to display file entry
	pListFile->left = left;
	pListFile->top  = top;
	pListFile->width = width;
	pListFile->height = height;
	
	#ifndef USE_CONTROL_NOTITLE 
	if (INVALID_STRINGRES_ID != pListFile->spcTitleStringID)
	{
		pListFile->left   = CTRL_WND_LEFT;
		pListFile->top    = CTRL_WND_TOP+CTRL_WND_TITLEHTH;
		pListFile->width  = CTRL_WND_WIDTH;
		pListFile->height = CTRL_WND_HEIGHT-pListFile->top;
	}
	#endif

    #if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	pListFile->top  = TITLE_HEIGHT;
	pListFile->height = MAIN_LCD_HEIGHT - TITLE_HEIGHT;
    #endif

    pListFile->showMode = LISTFILE_MODE;
    ListFile_SetShowRect(pListFile);
   
		
	pListFile->curCachePos = 0;
	pListFile->bufCache[0] = 0;
	pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-2] = 0;
	pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-1] = 0;	

    pListFile->hFSFind = FS_INVALID_HANDLE;

    pListFile->filter[0] = 0;
	if (patFilter)
    {
        Utl_UStrCpy(pListFile->filter, patFilter);
    }

	pos = e_getfiledriver(pListFile);
	#ifdef USE_HIDE_DRIVER
	if (pos > 2)
	{
		pos--;
	}

	#endif
	pListFile->magicDriver = 0;
	pListFile->magicDriver = SET_DRIVERCOUNT(pListFile->magicDriver, pos);
	pListFile->magicDriver = SET_DISPLAYLEVL(pListFile->magicDriver, CTRL_FILELIST_LEVEL_FILE);

	if(AK_NULL == rootDIR)
	{
		if(GET_DRIVERCOUNT(pListFile->magicDriver) <= 1)
		{
			//只有1个盘，空目录则自动转为A:/
			pListFile->bufCache[0] = 'A';
			pListFile->bufCache[1] = ':';
			pListFile->bufCache[2] = CTRL_FILELIST_DIRTAG;
			pListFile->bufCache[3] = 0;
			pListFile->curCachePos = 0;
		}
		else
		{
			//有多个盘，空目录则自动转为目录盘模式
			pListFile->magicDriver = SET_DISPLAYLEVL(pListFile->magicDriver, CTRL_FILELIST_LEVEL_DRIVER);
			pListFile->cntEntrys   = GET_DRIVERCOUNT(pListFile->magicDriver);
		}
	}
	else
	{
		Utl_UStrCpy(pListFile->bufCache, rootDIR);
		//将盘符大写
		if(pListFile->bufCache[0]>='a' && pListFile->bufCache[0]<='z')
			pListFile->bufCache[0] -= ('a'-'A');

		j = Utl_UStrLen(pListFile->bufCache);
		for(i=0; i<j; i++)
		{
			if(pListFile->bufCache[i] == '\\')
				pListFile->bufCache[i] = CTRL_FILELIST_DIRTAG;
		}

		if((pListFile->bufCache[1] != ':') && (pListFile->bufCache[j-1] == CTRL_FILELIST_DIRTAG))
			pListFile->bufCache[j-1] = 0;

		//将POS移动到当前目录名开始处, 根目录则为首地址
		{
			T_S16 tmpPos;
			
			tmpPos = Utl_UStrRevFndChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 0);
			pListFile->curCachePos = (T_U16)((tmpPos >= 0) ? (tmpPos+1) : 0);

			if(Utl_UStrLen(pListFile->bufCache) == pListFile->curCachePos)
				pListFile->curCachePos = 0;
		}
	}

	pListFile->pTxtScroll = AK_NULL;
	pListFile->bResetTxtScroll = AK_TRUE;

	if(0 != pListFile->bufCache[0])
	{
       
        //begin to explor parent directory.
		e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));
		if(FS_INVALID_HANDLE == pListFile->hFSFind)
		{
			AK_DEBUG_OUTPUT("Error : ListFile init failure\r\n");
			return AK_FALSE;
		}
	}

	pListFile->navigateID = INVALID_STRINGRES_ID;
	//keypad_start_intervaltimer(AK_TRUE);

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
	//pListFile->top  = TITLE_HEIGHT;
	//pListFile->height = MAIN_LCD_HEIGHT - TITLE_HEIGHT;
	return TopBar_init(&pListFile->listfile_topbar, pListFile->spcTitleStringID, AK_TRUE ,AK_TRUE);
#endif
	return AK_TRUE;
}

#pragma arm section code 

#pragma arm section code = "_listfile_show_"

static T_VOID ListFile_ClearCachePos(CListFileCtrl *pListFile)
{
	T_S16 pos, pt;

	//必须总是保持buffer保存的当前目录路径
	//必须总是保持pos总是指向当前目录短名开始处
	if(0 != pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-1])
	{
		//当前目录存放的是选中的文件的绝对路径, must clear it.
		pos = Utl_UStrRevFndChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 0);
		pListFile->bufCache[pos] = 0;
		pt = pos;		
 		pos = Utl_UStrRevFndChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 0);
		if(-1 == pos)
		{
			pListFile->curCachePos = 0;
			pListFile->bufCache[pt] = CTRL_FILELIST_DIRTAG;
			pListFile->bufCache[pt+1] = 0;
		}
		else
			pListFile->curCachePos = (T_U16)(pos+1);

		//now buf content still is fold
		pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-1] = 0;
	}
}

#pragma arm section code 

T_VOID  ListFile_Free(CListFileCtrl *pListFile)
{
	if(pListFile)
		e_findclose(pListFile);

	if(pListFile->pTxtScroll)
	{
		TxtScroll_Free(pListFile->pTxtScroll);
		pListFile->pTxtScroll = Fwl_Free(pListFile->pTxtScroll);
	}

	//keypad_start_intervaltimer(AK_FALSE);

}

#pragma arm section code = "_listfile_show_"

#ifndef USE_CONTROL_NOTITLE 
static T_VOID ListFile_Show_InternalTitle(CListFileCtrl *pListFile)
{
	if(INVALID_STRINGRES_ID == pListFile->spcTitleStringID)
		return;

	if(CTRL_REFRESH_ALL == pListFile->refresh)
	{
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, CTRL_WND_BACKCOLOR);
		#else		
		Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP, TITLE_BKG_IMAGEPTR, AK_FALSE);
		#endif		
	}

	if(CTRL_REFRESH_FILELIST_TITLE & pListFile->refresh)
	{
		Display_Title(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_TITLEHTH, (T_U32)pListFile->spcTitleStringID, pListFile->bkimgTitle, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);

		if(INVALID_STRINGRES_ID != pListFile->navigateID)
			Display_NavgateBar(CTRL_WND_LEFT, CTRL_WND_TOP+CTRL_WND_TITLEHTH, CTRL_WND_WIDTH, CTRL_WND_LINEHIGH, pListFile->navigateID);
	}
}
#endif

static T_VOID ListFile_DrawLine(T_POS left, T_POS top, T_LEN width, T_LEN height, T_U16 color)
{
    Fwl_FillRect(left, (T_POS)(top - height), (T_LEN)(width + height), height, color);
    Fwl_FillRect((T_POS)(left - height), (T_POS)(top + width), (T_LEN)(width + height), height, color);
    Fwl_FillRect((T_POS)(left - height) , (T_POS)(top - height), height, (T_LEN)(width + height), color);
    Fwl_FillRect((T_POS)(left + width), top, height, (T_LEN)(width + height), color);
}

static T_VOID ListFile_GetDefaultIcon(CListFileCtrl *pListFile, T_U16 *string, T_U8  attr, T_RES_IMAGE *pIconPtr)
{
    //get entry default icon
    if (CTRL_FILELIST_ATTR_FILE == attr)
    {
        *pIconPtr = ICON_FILE_ENTRYPTR;

        //get user define file icon <postfix>
        if ((CTRL_LISTFILE_DISP_FILEONLY & pListFile->modeSearch) 
        	&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW&pListFile->dwStyle))
        {
            *pIconPtr = Gbl_GetFileIcon(string);
        }            
                        
    }
    else
    {
       if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_FILE)
       {
            *pIconPtr = ICON_FOLD_ENTRYPTR;//default fold icon
       }            
        					
       else
       {
            *pIconPtr = ICON_DRIVER_ENTRYPTR;//default driver icon
       }            
        					
    }//get entry default icon end
}

static T_VOID ListFile_DrawDefaultIconAndStr(CListFileCtrl *pListFile, T_U16 *string, T_RES_IMAGE iconPtr)
{
    T_POS pos_x = 0;
    T_POS pos_y = 0;
    
    #if (USE_COLOR_LCD)
	T_BG_PIC pic;
	#endif

    pos_x = pListFile->left;
    pos_y = pListFile->top;

    if ((CTRL_LISTFILE_DISP_FILEONLY& pListFile->modeSearch)        //如果搜索的仅是文件
		&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle)) //要空出一行显示当前目录
	{
        pos_y += CTRL_WND_LINEHIGH;
    }

    //first clear entry region
    #if (!USE_COLOR_LCD)
    Fwl_FillRect(pos_x, pos_y, pListFile->width, CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
    #else
    Eng_ImageResDispEx(pos_x, pos_y, MENU_BACK_IMAGEPTR, pListFile->width, CTRL_WND_LINEHIGH, 0, pos_y, AK_FALSE);          
    #endif

    #if (!USE_COLOR_LCD)
    if (0 == pListFile->curCachePos)
    {
       iconPtr = eRES_IMAGE_ROOTICON;
    }         	
    #endif
                    
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    Eng_ImageResDisp(pos_x, pos_y, iconPtr, IMG_TRANSPARENT);//draw root icon
    #else
    Eng_ImageResDisp(pos_x, pos_y, iconPtr, AK_FALSE);//draw root icon
    #endif
        			
    //draw root string.
    #if (!USE_COLOR_LCD)
    DispStringOnColor(CP_UNICODE, (T_POS)(pos_x+CTRL_WND_ICONSZ+2), pos_y, CONVERT2STR(string), 
                        FONT_WND_WIDTH, (T_U16)(CTRL_WND_FONTCOLOR),(T_U16)(CTRL_WND_BACKCOLOR));
    #else
    pic.hOffset = (T_POS)(pos_x+CTRL_WND_ICONSZ+3);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    pic.vOffset = (T_POS)(pos_y-TITLE_HEIGHT);
    #else
    pic.vOffset = pos_y;
    #endif
    pic.resId   = MENU_BACK_IMAGEPTR;
    DispStringOnPic(CP_UNICODE, (T_POS)(pos_x+CTRL_WND_ICONSZ+3), pos_y, CONVERT2STR(string), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR, &pic);
    #endif
}

T_VOID	ListFile_Show(CListFileCtrl *pListFile)
{
	T_U8  attr;
	T_U16 i, j;
    T_U16 show_x = pListFile->offset_x;//略缩图的显示坐标x
    T_U16 show_y = pListFile->offset_y;//略缩图的显示坐标y
	T_U16 *string;
	T_RES_IMAGE iconPtr;

    //T_U16 lineHigh = CTRL_WND_LINEHIGH;
    
	#if (USE_COLOR_LCD)
	T_BG_PIC pic;
	#if (1 == LCD_HORIZONTAL && 3 == LCD_TYPE)
	T_BOOL brefreshall = AK_FALSE;
	#endif
	#endif 

	if(!pListFile)
    {
        return;
    }   
    
 	if(pListFile->dwStyle & CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT)
    {
        TxtScroll_Show(pListFile->pTxtScroll);
    }  
			
	//must keep buffer content is fold path.
	ListFile_ClearCachePos(pListFile);

	#ifndef USE_CONTROL_NOTITLE 
	ListFile_Show_InternalTitle(pListFile);
	#endif

#if (1 == LCD_HORIZONTAL && 3 == LCD_TYPE)
    if (CTRL_REFRESH_ALL == pListFile->refresh || CTRL_REFRESH_FILELIST_ENTRYEX_ALL == pListFile->refreshEx)
    {
        brefreshall = AK_TRUE;
        Fwl_FreqPush(FREQ_APP_MAX);
        Fwl_Freq_Add2Calc(FREQ_VAL_1);
    }
#endif  

	if(CTRL_REFRESH_ALL == pListFile->refresh)//贴文件浏览控件的背景
	{
		//clear and repaint all LCD with color or background image.
		#if (!USE_COLOR_LCD)
		Fwl_FillRect(pListFile->left, pListFile->top, pListFile->width, pListFile->height, CTRL_WND_BACKCOLOR);
		#else
		#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
        TopBar_SetReflesh(&pListFile->listfile_topbar, TOPBAR_REFLESH_ALL);
		Eng_ImageResDispEx(pListFile->left, pListFile->top, MENU_BACK_IMAGEPTR, pListFile->width, pListFile->height, pListFile->left, pListFile->top, AK_FALSE);
		#else
		Eng_ImageResDispEx(pListFile->left, pListFile->top, MENU_BACK_IMAGEPTR, pListFile->width, pListFile->height, 0, 0, AK_FALSE);
		#endif
		#endif
	}
	
	#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))//贴标题
    TopBar_show(&pListFile->listfile_topbar);
	#endif
	j = (T_U16)((pListFile->height-(CTRL_WND_LINEHIGH*(pListFile->entryLines)))/2+pListFile->top);

	//under FILE only mode, 当前目录条目是不可移动选中的
	//只要求显示文件时，固定显示文件所在目录
	if((CTRL_LISTFILE_DISP_FILEONLY& pListFile->modeSearch) && (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle))
	{	
		j = (T_U16)((pListFile->height-(CTRL_WND_LINEHIGH*(pListFile->entryLines+1)))/2+pListFile->top);
			
		//only repaint root fold
		if((CTRL_REFRESH_FILELIST_ROOT & pListFile->refresh)
			&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle))
		{
			T_U16 string2[6];
			j = (T_U16)((pListFile->height-(CTRL_WND_LINEHIGH*(pListFile->entryLines+1)))/2+pListFile->top+CTRL_FILELIST_BEAUTIFY_INTVL);
			
            if (DIAGRAM_MODE == pListFile->showMode)
            {
                j = pListFile->top;
            }

            if(0 == pListFile->curCachePos)
			{
				//盘根目录下
				string2[0] = 'R';string2[1] = 'O';string2[2] = 'O';string2[3] = 'T';
				string2[4] = 0;	
				string = string2;
			}
			else
			{
				//当前目录下
				string = pListFile->bufCache+pListFile->curCachePos;
			}

			iconPtr = ICON_FOLD_ENTRYPTR;
			#if (!USE_COLOR_LCD)
			if (0 == pListFile->curCachePos)
            {
                iconPtr = eRES_IMAGE_ROOTICON;
            }         		
			#endif
			#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
			Eng_ImageResDisp(pListFile->left, (T_POS)j, iconPtr, IMG_TRANSPARENT);//draw root icon
			#else
			Eng_ImageResDisp(pListFile->left, (T_POS)j, iconPtr, AK_FALSE);//draw root icon
			#endif
			
			//draw root string.
			#if (!USE_COLOR_LCD)
            DispStringOnColor(CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+2), (T_POS)j, CONVERT2STR(string), 
                FONT_WND_WIDTH, (T_U16)(CTRL_WND_FONTCOLOR),(T_U16)(CTRL_WND_BACKCOLOR));
			#else
			pic.hOffset = (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3);
			#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
			pic.vOffset = (T_POS)(j-TITLE_HEIGHT);
			#else
			pic.vOffset = (T_POS)(j);
			#endif
			pic.resId   = MENU_BACK_IMAGEPTR;
			DispStringOnPic(CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)j, CONVERT2STR(string), FONT_WND_WIDTH, CTRL_WND_FONTCOLOR, &pic);
			#endif

		}

		if(CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW & pListFile->dwStyle)
			j +=CTRL_WND_LINEHIGH;
	}


	//only repaint file entry
	if(CTRL_REFRESH_FILELIST_ENTRY & pListFile->refresh)
	{
		T_U8 l = (T_U8)(pListFile->refreshEx >> 8);
		T_U8 r = (T_U8)pListFile->refreshEx;

		T_U16 fcsId = 0;
		T_U16 fcsJ = 0;

		if ((pListFile->dwStyle & CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT)
			&& pListFile->bResetTxtScroll)
        {
            TxtScroll_Pause(pListFile->pTxtScroll);
        }      
			
	    if (LISTFILE_MODE == pListFile->showMode)
        {
            
    		//File entry 图标+字符串
    		for(i=0; i<CTRL_FILELIST_DISP_ENTRYS; i++)
    		{
    			if (pListFile->begSrID+i >= pListFile->cntEntrys)
                {
                    continue;
                }         
    			
    			if (CTRL_REFRESH_FILELIST_ENTRYEX_ALL != pListFile->refreshEx)
    			{
    				//只显示更新条目, only reapint small region
    				if ((l!=i) && (r!=i))
    				{
    					j += CTRL_WND_LINEHIGH;
    					continue;
    				}
    			}

    			//get entry string
    			string = ListFile_GetString(pListFile, (T_U8)i, &attr);//要显示的字符串

    			//get entry default icon
    			if (CTRL_FILELIST_ATTR_FILE == attr)
    			{
    				iconPtr = ICON_FILE_ENTRYPTR;

    				//get user define file icon <postfix>
    				if ((CTRL_LISTFILE_DISP_FILEONLY & pListFile->modeSearch) 
    					&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW&pListFile->dwStyle))
                    {
                        iconPtr = Gbl_GetFileIcon(string);
                    }            
                    
    			}
    			else
    			{
    				if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_FILE)
                    {
                        iconPtr = ICON_FOLD_ENTRYPTR;//default fold icon
                    }            
    					
    				else
                    {
                        iconPtr = ICON_DRIVER_ENTRYPTR;//default driver icon
                    }            
    					
    			}//get entry default icon end
                

    			#if (!USE_COLOR_LCD)
    			if (pListFile->begSrID+i==0 && pListFile->curCachePos==0 && pListFile->modeSearch^CTRL_LISTFILE_DISP_FILEONLY)
                {
                    iconPtr = eRES_IMAGE_ROOTICON;
                }
    			//if(iconPtr==ICON_FOLD_ENTRYPTR && i+pListFile->begSrID==pListFile->curSubID)
    				//iconPtr = eRES_IMAGE_FOLDICONCLOSE;
    			#endif

    			//first clear entry region
    			#if (!USE_COLOR_LCD)
    			if (i+pListFile->begSrID != pListFile->curSubID)
                {
                    Fwl_FillRect(pListFile->left, j, pListFile->width, CTRL_WND_LINEHIGH, CTRL_WND_BACKCOLOR);
                }         	
    			else
                {
                    Fwl_FillRect(pListFile->left, j, pListFile->width, CTRL_WND_LINEHIGH, CTRL_WND_FONTCOLOR);
                }         
    			#else
                {
                    Eng_ImageResDispEx(pListFile->left, j, MENU_BACK_IMAGEPTR, pListFile->width, CTRL_WND_LINEHIGH, 0, (j), AK_FALSE); 
                }         
    			#endif
    			
    			//next paint entry icon
    			#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))//每个选项前的小图标
    			Eng_ImageResDisp(pListFile->left, (T_POS)j, iconPtr, IMG_TRANSPARENT);//draw root icon
    			#else
    			Eng_ImageResDisp(pListFile->left, (T_POS)j, iconPtr, AK_FALSE);
    			#endif//next paint entry icon end
    			
    			//finally paint entry string and background image
    			#if (!USE_COLOR_LCD)
                DispStringOnColor(CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)j, CONVERT2STR(string), 
                    FONT_WND_WIDTH, (T_U16)(i+pListFile->begSrID!=pListFile->curSubID? CTRL_WND_FONTCOLOR:CTRL_WND_BACKCOLOR),
                    (T_U16)(i+pListFile->begSrID!=pListFile->curSubID? CTRL_WND_BACKCOLOR:CTRL_WND_FONTCOLOR)); 			
    			#else
    			pic.hOffset = (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3);
    //			#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    //			pic.vOffset = (T_POS)(j-TITLE_HEIGHT);
    //			#else
    			pic.vOffset = (T_POS)(j);
    //			#endif
    			pic.resId   = MENU_BACK_IMAGEPTR;
    			if(i+pListFile->begSrID != pListFile->curSubID)
    				DispStringOnPic(CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)j, CONVERT2STR(string), (T_U16)(FONT_WND_WIDTH-CTRL_WND_ICONSZ-4), CTRL_WND_FONTCOLOR, &pic);
    			else
    			{						    
    				pic.hOffset = (T_POS)0;
    				pic.vOffset = (T_POS)0;
    				pic.resId   = FILE_SELBAR_IMAGEPTR;
    				
    				Eng_ImageResDisp((T_POS)(pListFile->left+CTRL_WND_ICONSZ+2), (T_POS)j, FILE_SELBAR_IMAGEPTR, AK_FALSE);
    				DispStringOnPic(CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)j, CONVERT2STR(string), (T_U16)(FONT_WND_WIDTH-CTRL_WND_ICONSZ-4), CLR_WHITE, &pic);
    			}
    			#endif//finally paint entry string and background image end

    	
    			if((pListFile->dwStyle & CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT)
    				&& pListFile->bResetTxtScroll && (i+pListFile->begSrID == pListFile->curSubID))
    			{
    				fcsId  = i;
    				fcsJ   = j;
    			}

    			j += CTRL_WND_LINEHIGH;

    		}

    		if((pListFile->dwStyle & CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT)
    				&& pListFile->bResetTxtScroll)
    		{
    			pListFile->bResetTxtScroll = AK_FALSE;

    			string = ListFile_GetString(pListFile, (T_U8)fcsId, &attr);

    			#if (!USE_COLOR_LCD)
    			TxtScrool_ReSetString(pListFile->pTxtScroll, (T_U32)string, AK_FALSE,
    					CTRL_WND_BACKCOLOR, CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)fcsJ, (T_U16)(FONT_WND_WIDTH-CTRL_WND_ICONSZ-4), CTRL_WND_FONTCOLOR, AK_NULL);
    			#else
    			pic.hOffset = (T_POS)0;
    			pic.vOffset = (T_POS)0;
    			pic.resId   = FILE_SELBAR_IMAGEPTR;
    			TxtScrool_ReSetString(pListFile->pTxtScroll, (T_U32)string, AK_FALSE,
    						(T_U8)(pListFile->left+CTRL_WND_ICONSZ+2), CP_UNICODE, (T_POS)(pListFile->left+CTRL_WND_ICONSZ+3), (T_POS)fcsJ, (T_U16)(FONT_WND_WIDTH-CTRL_WND_ICONSZ-4), CLR_WHITE, &pic);    
    			#endif
    		}
        }
      
        else if (DIAGRAM_MODE == pListFile->showMode)
        {
            const T_U16 fileType[][TYPENAMELEN] = {        //文件类型
                                {'j','p', 0, 0, 0},        //picture
                                {'J','P', 0, 0, 0},
                                {'B','M','P','0',0},
                                {'b','m','p', 0, 0},
                                {'g','i','f', 0, 0},
                                {'G','I','F', 0, 0},
                                {'j','g','f', 0, 0},
                                {'J','G','F', 0, 0},
                                {'w','a','v', 0, 0},        //record,or music
                                {'W','A','V', 0, 0},
                                {'m','p','3', 0, 0},
                                {'M','P','3', 0, 0},
                                {'w','m','a', 0, 0},
                                {'W','M','A', 0, 0},
                                {'a','p','e', 0, 0},
                                {'A','P','E', 0, 0},
                                {'f','l','a','c',0},
                                {'F','L','A','C', 0},
                                {'a','v','i', 0, 0},       //video
                                {'A','V','I', 0, 0},
                                {'t','x','t', 0, 0},        //book
                                {'T','X','T', 0, 0}
                                
                              };

            //g_fileType与g_fileTypeIcon的关系是为当取g_fileType[i]，相对应的取g_fileTypeIcon[i/2]
            const T_RES_IMAGE fileTypeIcon[] = {                                //相对应的文件图标
							#if (STORAGE_USED == SPI_FLASH)
								eRES_IMAGE_REC_CURTIME_0_1,
                                eRES_IMAGE_REC_CURTIME_0_0,
                            #else
                                eRES_IMAGE_REC_CURTIME_1_1,
                                eRES_IMAGE_REC_CURTIME_1_0,
							#endif
                                eRES_IMAGE_REC_CURTIME_0_9,
                                eRES_IMAGE_REC_CURTIME_0_8,
                                eRES_IMAGE_REC_CURTIME_0_7,
                                eRES_IMAGE_REC_CURTIME_0_6,
                                eRES_IMAGE_REC_CURTIME_0_5,
                                eRES_IMAGE_REC_CURTIME_0_4,
                                eRES_IMAGE_REC_CURTIME_0_3,
                                eRES_IMAGE_REC_CURTIME_0_2,
                                eRES_IMAGE_REC_CURTIME_0_1

                                };
            const T_RES_IMAGE unknownPicIcon = eRES_IMAGE_REC_CURTIME_0_0;   //不能打开的图片图标
            const T_RES_IMAGE unknownTypeIcon = eRES_IMAGE_REC_CURTIME_0_0;   //未知文件类型的图标
            const T_RES_IMAGE foldTypeIcon = eRES_IMAGE_REC_DOCUMENT;         //文件夹图标


            AK_DEBUG_OUTPUT("\npListFile->showMode\n");

            //File entry 图标+字符串
    		for (i=0; i<CTRL_FILELIST_DISP_ENTRYS; i++)
    		{

                AK_DEBUG_OUTPUT("\ni = %d\n", i);
                //get entry string
        	    string = ListFile_GetString(pListFile, (T_U8)i, &attr);

                show_y = (i / pListFile->column) * (pListFile->wh + (2 * pListFile->sepration) + pListFile->addDis) + pListFile->offset_y;
                show_x = (i % pListFile->column) * (pListFile->wh + (2 * pListFile->sepration)) + pListFile->offset_x;
                
                if ((pListFile->begSrID+i) >= pListFile->cntEntrys)
                {
                    AK_DEBUG_OUTPUT("\ncontinue1\n");
                    continue;
                }

                if ((pListFile->begSrID+i) == pListFile->curSubID || i == 0)
                {
                    ListFile_GetDefaultIcon(pListFile, string, attr, &iconPtr);
                    ListFile_DrawDefaultIconAndStr(pListFile, string, iconPtr);
                }
                    
    			if (CTRL_REFRESH_FILELIST_ENTRYEX_ALL == pListFile->refreshEx)
    			{
                    T_S16   hOffset = 0;
                    T_S16   vOffset = 0;

        			//get entry default icon
        			if (CTRL_FILELIST_ATTR_FILE == attr)//显示文件的略缩图
        			{
                        T_BOOL bImgRet = AK_FALSE;
                        T_U16  fileTypeBuf[5] = {0, 0, 0, 0, 0};
                        T_U16  pos = 0;
                        T_U16  loop = 0;
                        

                        ListFile_GetFileType(string, fileTypeBuf);//get file type

                        for (loop=0; loop<(sizeof(fileType)/sizeof(fileType[0])); loop++)
                        {
                            if (Utl_UStrCmpN(fileTypeBuf, fileType[loop], Utl_UStrLen(fileType[loop])) == 0) 
                            {
                                break;
                            }
                        }

                        AK_DEBUG_OUTPUT("\nloop = %d\n", loop);
                        
                        if (loop <= 7)  //picture
                        {
                            ListFile_ClearCachePos(pListFile);
                            pos = (T_U16)(Utl_UStrLen(pListFile->bufCache + pListFile->curCachePos) + 1);

                            if (0 != pListFile->curCachePos)
                            {
                                Utl_UStrCatChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 1);
                            }
            		        else
                            {
                                pos -= 1;
                            }      
            			    
            		        pListFile->curCachePos = (T_U16)(pListFile->curCachePos + pos);
                            
                            Utl_UStrCat(pListFile->bufCache, string);
            		        
            	            pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-1] = 1;

                            UStrCpy(g_pImageView->imageVSet.file, pListFile->bufCache);

                            AK_DEBUG_OUTPUT("ListFile_Show file:\n");
                            Printf_UC(g_pImageView->imageVSet.file);

                            
                            bImgRet = ShowImage(g_pImageView, g_pImageView->imageVSet.file, show_x, show_y, pListFile->wh, pListFile->wh); 
                            AK_DEBUG_OUTPUT("end ShowImage\n");

                            FreeImageViewResource(g_pImageView);//在打开下一张图片时先关闭之前打开的图片

                            if (bImgRet == AK_FALSE)            //  图片不能显示时，用一个图标来表示出错                                 
                            {   
                                hOffset = (pListFile->wh - Eng_GetResImageWidth(unknownPicIcon)) / 2;
                                vOffset = (pListFile->wh - Eng_GetResImageHeight(unknownPicIcon)) / 2;

                                Eng_ImageResDisp((T_POS)(show_x + hOffset), (T_POS)(show_y + vOffset), unknownPicIcon, AK_FALSE);
                                Printf_UC(g_pImageView->imageVSet.file);                                                
                            }
                        }
                        else if (loop < (sizeof(fileType)/sizeof(fileType[0]))) //file
                        {
                            hOffset = (pListFile->wh - Eng_GetResImageWidth(fileTypeIcon[loop/2])) / 2;
                            vOffset = (pListFile->wh - Eng_GetResImageHeight(fileTypeIcon[loop/2])) / 2;

                            Fwl_FillRect((T_POS)show_x, (T_POS)show_y, pListFile->wh, pListFile->wh, CTRL_WND_BACKCOLOR);
                            Eng_ImageResDisp((T_POS)(show_x + hOffset), (T_POS)(show_y + vOffset), fileTypeIcon[loop/2], AK_FALSE);
                        }
                      
                        else//unknown file type
                        {
                            hOffset = (pListFile->wh - Eng_GetResImageWidth(unknownTypeIcon)) / 2;
                            vOffset = (pListFile->wh - Eng_GetResImageHeight(unknownTypeIcon)) / 2;

                            Fwl_FillRect((T_POS)show_x, (T_POS)show_y, pListFile->wh, pListFile->wh, CTRL_WND_BACKCOLOR);
                            Eng_ImageResDisp((T_POS)(show_x + hOffset), (T_POS)(show_y + vOffset), unknownTypeIcon, AK_FALSE);
                            
                        }

                        // ListFile_MoveCachePos(pListFile, +1);
                        
        			}
                    else//显示文件夹的略缩图
                    {
                        hOffset = (pListFile->wh - Eng_GetResImageWidth(foldTypeIcon)) / 2;
                        vOffset = (pListFile->wh - Eng_GetResImageHeight(foldTypeIcon)) / 2;

                        Fwl_FillRect((T_POS)show_x, (T_POS)show_y, pListFile->wh, pListFile->wh, CTRL_WND_BACKCOLOR);
                        Eng_ImageResDisp((T_POS)(show_x + hOffset), (T_POS)(show_y + vOffset), foldTypeIcon, AK_FALSE);

                    }
                 }
                 else
                 {
                     //只显示更新条目, only reapint small region
        		    if((l!=i) && (r!=i))
        			{
                         continue;
        		    }
                 }

                 if ((i+pListFile->begSrID) != pListFile->curSubID)//画图缩图的边框
                 {
                     ListFile_DrawLine((T_POS)show_x, (T_POS)show_y, (T_LEN)pListFile->wh, CTRL_LINE_WIDTH, RGB_COLOR(176,216,255));
                 }
                 else
                 {
                     ListFile_DrawLine((T_POS)show_x, (T_POS)show_y, (T_LEN)pListFile->wh, CTRL_LINE_WIDTH, RGB_COLOR(11,133,255));
                 }

            }
        }      
	}
#if (1 == LCD_HORIZONTAL && 3 == LCD_TYPE)
    if (brefreshall)
    {
        Fwl_Freq_Clr_Add();
        Fwl_FreqPop();        
    }
#endif  

	pListFile->refresh = CTRL_REFRESH_NONE;
}

T_VOID	ListFile_SetRefresh(CListFileCtrl *pListFile)
{
	if(pListFile)
	{
		pListFile->refresh = CTRL_REFRESH_ALL;	
		pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;
		pListFile->bResetTxtScroll = AK_TRUE;
	}
}

#pragma arm section code

#pragma arm section code = "_listfile_handle_"

static T_BOOL ListFile_MoveFocus(CListFileCtrl *pListFile, T_S8 delta)
{
	T_S8 lg;
	T_S8 abcd;
	T_S8 xyzw;
	T_S8 shift_matrix[4][4][2][2] =	//[xyzw][abcd][lg][]
		{
			//x
			{
				//a
				{
					-1, 0, +1, 0,
				},
				//b
				{
					-1, 0, +1, 0,
				},
				//c	
				{
					-1, 0, +1, 0,
				},
				//d	
				{
					-1, 0, +1, 0,
				},
			},
			//y
			{
				//a
				{
					-1, -1, +1, 0,
				},
				//b
				{
					-126, -126, +1, 0,
				},
				//c	
				{
					-1, -1, +1, 0,
				},
				//d	
				{
					-126, -126, +1, 0,
				},
			},
			//z
			{
				//a
				{
					-1, 0, +1, +1,
				},
				//b
				{
					-1, 0, +1, +1,
				},
				//c	
				{
					-1, 0, -127, -127,
				},
				//d	
				{
					-1, 0, -127, -127,
				},
			},
			//w
			{
				//a
				{
					-1, -1, +1, +1,
				},
				//b
				{
					-126, -126, +1, +1,
				},
				//c	
				{
					-1, -1, -127, -127,
				},
				//d	
				{
					-126, -126, -127, -127,
				},
			},
		};
	T_U16  curID;
	T_U16  begID;
	T_BOOL bRet = AK_FALSE;

	if(1 == pListFile->cntEntrys)
		return AK_FALSE;

	//focus move, adjust by above matrix, this is a technics.
	if(CTRL_FILELIST_DISP_ENTRYS == 1)							xyzw = 3;
	else if(pListFile->curSubID == pListFile->begSrID)			xyzw = 1;
	else if(pListFile->curSubID == pListFile->begSrID+CTRL_FILELIST_DISP_ENTRYS-1)		xyzw = 2;
	else if(pListFile->curSubID == pListFile->begSrID+pListFile->cntEntrys-1)			xyzw = 2;
	else														xyzw = 0;
	
	if(pListFile->cntEntrys <= CTRL_FILELIST_DISP_ENTRYS)		abcd = 3;
	else
	{
		if(pListFile->begSrID == 0)								abcd = 1;
		else if(pListFile->begSrID+CTRL_FILELIST_DISP_ENTRYS-1 == pListFile->cntEntrys-1)	abcd = 2;
		else													abcd = 0;
	}

	if(delta < 0)												lg   = 0;
	else														lg   = 1;

	curID = pListFile->curSubID;
	begID = pListFile->begSrID;

	//如果焦点已经在底部，且要求继续往下移
	if(-127 == shift_matrix[xyzw][abcd][lg][0])
	{
		//jump to top
		pListFile->curSubID = pListFile->begSrID = 0;
		shift_matrix[xyzw][abcd][lg][1] = +1;//search to down
	}

	//如果焦点已经在顶部，且要求继续往上移
	else if(-126 == shift_matrix[xyzw][abcd][lg][0])
	{
		//jump to bottom
		pListFile->curSubID = (T_U16)(pListFile->cntEntrys-1);

		if(pListFile->cntEntrys > CTRL_FILELIST_DISP_ENTRYS)
			pListFile->begSrID = (T_U16)(pListFile->curSubID-CTRL_FILELIST_DISP_ENTRYS+1);
		else
			pListFile->begSrID = 0;

		shift_matrix[xyzw][abcd][lg][1] = -1;//search to up
	}
	else
	{
		//directly get location from matrix
		pListFile->curSubID = (T_U16)(pListFile->curSubID + shift_matrix[xyzw][abcd][lg][0]);
		pListFile->begSrID  = (T_U16)(pListFile->begSrID + shift_matrix[xyzw][abcd][lg][1]);
	}

	if(curID!=pListFile->curSubID || begID!=pListFile->begSrID)
	{
		if(0 != shift_matrix[xyzw][abcd][lg][1])
		{
			//如果需要更新显示内容，则搜索新内容
			e_findnext(pListFile, shift_matrix[xyzw][abcd][lg][1]);
			bRet = AK_TRUE;
		}
		
		pListFile->refresh = CTRL_REFRESH_FILELIST_ENTRY;
		pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;//first set to repaint all entry
		if(begID == pListFile->begSrID)
		{
			//如果仅仅是当前屏幕内条目内容的焦点切换, we only need to repaint two entry.
			pListFile->refreshEx = (T_U16)((curID-begID) << 8);
			pListFile->refreshEx |= (pListFile->curSubID-begID);
		}

		pListFile->bResetTxtScroll = AK_TRUE;
		
	}
    
	return bRet;
}

static T_VOID ListFile_MoveCachePos(CListFileCtrl *pListFile, T_S8 delta)
{
	T_U8  attr;
	T_U16 *string;
	T_U16 pos;

	ListFile_ClearCachePos(pListFile);

	if(-1 == delta)
	{
		//当前目录路径回退一级到父目录
		pos = Utl_UStrRevFndChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 0);
		pListFile->bufCache[pos] = 0;
		pos = Utl_UStrRevFndChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 0);
		if((T_U16)-1 != pos)
			pListFile->curCachePos = (T_U16)(pos+1);
		else
		{
			Utl_UStrCatChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 1);
			pListFile->curCachePos = 0;
		}
	}
	else
	{
		//当前目录路径增加一级短名<目录或者文件>
		pos = (T_U16)(Utl_UStrLen(pListFile->bufCache+pListFile->curCachePos)+1);
		if(0 != pListFile->curCachePos)
			Utl_UStrCatChr(pListFile->bufCache, CTRL_FILELIST_DIRTAG, 1);
		else
			pos -= 1;
		pListFile->curCachePos = (T_U16)(pListFile->curCachePos + pos);

        string = ListFile_GetString(pListFile, (T_U8)(pListFile->curSubID-pListFile->begSrID), &attr);
		Utl_UStrCat(pListFile->bufCache, string);

        Printf_UC(string);
	}
}

#pragma arm section code 


static T_VOID ListFile_EnterFold(CListFileCtrl *pListFile)
{
	T_U8  attr;
	T_S8  delta = 0;
	T_U8  bBacktoDriver = AK_FALSE;

	switch(pListFile->curSubID)
	{
	case 0:
		if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_FILE)
		{
			//回退目录		
			delta = -1;//返回上一级根目录
			//AK_DEBUG_OUTPUT("lf:pos:%d,style:%d\n",pListFile->curCachePos,pListFile->dwStyle);
			//Printf_UC(pListFile->bufCache); 
			if(0 == pListFile->curCachePos)
			{
				if((pListFile->dwStyle&CTRL_LISTFILE_STYLE_ROOTDISK_DISPSHOW) && (GET_DRIVERCOUNT(pListFile->magicDriver) > 1))
					bBacktoDriver = AK_TRUE;//退往系统盘府
				else
					delta = 0;		
			}
			AK_DEBUG_OUTPUT("delta:%d\n",delta);
		}
		else
		{
			//进入根目录
			pListFile->bufCache[0] = 'A';
			delta = +1;
		}

		break;

	//case 1:
		//本级目录
		//if(0 != pListFile->curCachePos)
		//{
			//delta = 0;
			//break;
		//}

	default:
		if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_FILE)
		{
			ListFile_GetString(pListFile, (T_U8)(pListFile->curSubID-pListFile->begSrID), &attr);
			if(CTRL_FILELIST_ATTR_FOLD == attr)
				delta = +1;//进入下一级子目录
		}
		else
		{
			pListFile->bufCache[0] = (T_U16)('A'+pListFile->curSubID);
			delta = +1;//进入目录盘内
		}
		break;
	}

	if(0 == delta)
		return;

	if(GET_DISPLAYLEVL(pListFile->magicDriver) == CTRL_FILELIST_LEVEL_FILE)
	{
        T_U16 lstPos = pListFile->curCachePos;
        T_U16 lstChr = pListFile->bufCache[pListFile->curCachePos];

		ListFile_MoveCachePos(pListFile, delta);

		pListFile->begSrID  = 0;
		pListFile->curSubID = 0;

		//File_FindFirst
		e_findclose(pListFile);

		if(!bBacktoDriver)
        {
			//search new fold content
            e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));
            if(-1 == delta)
            {
                if(0 == pListFile->curCachePos)
                    pListFile->bufCache[lstPos] = lstChr;

				//当回退目录时，将焦点定位父目录条目处
                ListFile_SetCurFile(pListFile, pListFile->bufCache+lstPos);

                if(0 == pListFile->curCachePos)
                    pListFile->bufCache[lstPos] = 0;
            }
        }
		else
		{
			if(GET_DRIVERCOUNT(pListFile->magicDriver) > 1)
			{
				//如果回退到目录盘，且有至少2个盘，进入菜单模式
				pListFile->magicDriver = SET_DISPLAYLEVL(pListFile->magicDriver, CTRL_FILELIST_LEVEL_DRIVER);
				pListFile->cntEntrys   = GET_DRIVERCOUNT(pListFile->magicDriver);
			}
		}
	}
	else 
	{
		pListFile->begSrID  = 0;
		pListFile->curSubID = 0;
		pListFile->curCachePos = 0;
		pListFile->refresh  = CTRL_REFRESH_FILELIST_ENTRY;
		pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;

		//进入盘根目录下
		e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));
		pListFile->magicDriver = SET_DISPLAYLEVL(pListFile->magicDriver, CTRL_FILELIST_LEVEL_FILE);
	}


	pListFile->refresh = CTRL_REFRESH_ALL;//CTRL_REFRESH_FILELIST_ENTRY;
	pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;

}

#pragma arm section code = "_listfile_handle_"

static T_U32 ListFile_SelectItem(CListFileCtrl *pListFile)
{
	//if((0==pListFile->curSubID) && !(CTRL_LISTFILE_DISP_FILEONLY&pListFile->modeSearch))
	if(0 == pListFile->curSubID)
	{
		if(!(CTRL_LISTFILE_DISP_FILEONLY & pListFile->modeSearch))
			return (T_U32)pListFile->bufCache;
		//上级目录无法选择
		//ListFile_EnterFold(pListFile);
		//return CTRL_RESPONSE_NONE;
	}

	//选择目录/文件
	ListFile_MoveCachePos(pListFile, +1);
	pListFile->bufCache[CTRL_LISTFILE_FILEPATHLEN-1] = 1;

	return (T_U32)pListFile->bufCache;
}

T_U32 ListFile_DealKey(CListFileCtrl *pListFile, T_EVT_PARAM *pParam)
{
    T_U8  attr;
    
    switch(pParam->c.Param1)
    {
        case CTRL_EVT_KEY_UP:
            ListFile_MoveFocus(pListFile, -1);//move focus up
            break;
        
        case CTRL_EVT_KEY_DOWN:
            ListFile_MoveFocus(pListFile, +1);//move focus down
            break;
    
        case CTRL_EVT_KEY_SELECT:
            if(CTRL_EVT_KEY_SELECT==pParam->c.Param1
                && pParam->c.Param2 == PRESS_LONG)
            {
                return CTRL_RESPONSE_QUIT;  //exit GUI if long push 
            }           
    
            return ListFile_SelectItem(pListFile);//get select entry path
            break;
    
        case CTRL_EVT_KEY_OK:
            if (pParam->c.Param2 == PRESS_SHORT)
            {
                ListFile_GetString(pListFile, (T_U8)(pListFile->curSubID-pListFile->begSrID), &attr);
                if (CTRL_FILELIST_ATTR_FILE == attr)
                    return ListFile_SelectItem(pListFile);//get select file path
                else 
                    ListFile_EnterFold(pListFile);//enter select fold to explor
                break;
            }
            else
                return CTRL_RESPONSE_QUIT;
    
        case CTRL_EVT_KEY_CANCEL:
            return CTRL_RESPONSE_QUIT;  
    
        default:
            break;
    }
    
    return CTRL_RESPONSE_NONE;
}

T_U32  ListFile_Handler(CListFileCtrl *pListFile, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
//注:这个TopBar_Handle函数应放于最前面，否则下面返回了，就没有对其进行处理；
	#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
	TopBar_Handle(&pListFile->listfile_topbar, Event, pParam);
	#endif
    if(!pListFile || !pParam || pParam->c.Param2 == PRESS_UP)
    {
        return CTRL_RESPONSE_NONE;
    }
    
    if(M_EVT_USER_KEY != Event && M_EVT_TOUCHSCREEN != Event)
    {
        return CTRL_RESPONSE_NONE;
    }
    
    {   
        return ListFile_DealKey(pListFile, pParam);
    }  
}

T_U16*	ListFile_GetFocusFile(CListFileCtrl *pListFile, T_BOOL *bFile)
{
	T_U8  attr;

	if(!pListFile)
		return AK_NULL;
	
	//get focus entry path.
    ListFile_GetString(pListFile, (T_U8)(pListFile->curSubID-pListFile->begSrID), &attr);
		    
	if (CTRL_FILELIST_ATTR_FILE == attr)
		*bFile = AK_TRUE;
	else
		*bFile = AK_FALSE;

	return (T_U16*)ListFile_SelectItem(pListFile);
}

#pragma arm section code 

T_U32	ListFile_GetCurFileID(CListFileCtrl *pListFile)
{
	T_U32 cntFolds;
	T_hFindCtrl hFSHandler = pListFile->hFSFind;

	if(!pListFile || !hFSHandler)
		return CTRL_LISTFILE_LAST_FILE;

    Fwl_FsGetFindInfo(hFSHandler, 0, AK_NULL, &cntFolds);

	if(pListFile->curSubID < cntFolds)
		return CTRL_LISTFILE_BEFORE_FILE;

	if(pListFile->curSubID == cntFolds)
		return CTRL_LISTFILE_FIRST_FILE;

	if(pListFile->curSubID == pListFile->cntEntrys-1)
		return CTRL_LISTFILE_LAST_FILE;

	return (T_U32)pListFile->curSubID;
}

#pragma arm section code 


T_VOID	ListFile_SetStyle(CListFileCtrl *pListFile, T_U32 dwStyle)
{
	T_U32 style = pListFile->dwStyle;

	if(!pListFile)
		return;

	pListFile->dwStyle = dwStyle;
	if((CTRL_LISTFILE_STYLE_ROOTDIR_DISPNONE!=style)
		&& (CTRL_LISTFILE_STYLE_ROOTDIR_DISPNONE==pListFile->dwStyle))
	{
		//如果之前风格是显示'ROOT'项，而现在要求不显示
		pListFile->entryLines ++;
		ListFile_ClearCachePos(pListFile);

		e_findclose(pListFile);
		e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));	
	}

	if((pListFile->dwStyle & CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT)
		&& !pListFile->pTxtScroll)
	{
		pListFile->pTxtScroll = (CTxtScrollCtrl*)Fwl_Malloc(sizeof(CTxtScrollCtrl));
		if(AK_NULL == pListFile->pTxtScroll)
		{
			AK_DEBUG_OUTPUT("ListFile_SetStyle:alloc pTxtScroll error\r\n");
			pListFile->dwStyle &= !CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT;
		}

		if(AK_FALSE == TxtScroll_Init(pListFile->pTxtScroll, 500))
		{
			AK_DEBUG_OUTPUT("ListFile_SetStyle:init pTxtScroll error\r\n");
			pListFile->dwStyle &= !CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT;
		}
	}



}

T_VOID	ListFile_SetCurFile(CListFileCtrl *pListFile, T_U16 *shortname)
{
    T_U8  cnt = 0;
	T_U8  attr = 0;             //
	T_U16 i = 0;                //屏幕上显示的所有选项中的任一项的索引
    T_U16 j = 0;                //遍历到了所有结果中的第几个
    T_U16 k = 0;
	
	if(!pListFile || !shortname)
		return;

	//loop to find the required focus file entry
    cnt = (T_U8)(CTRL_FILELIST_DISP_ENTRYS);
    
	for(i=0, j=0; j<pListFile->cntEntrys; )
	{
		//get each entry
		T_U16 *string = ListFile_GetString(pListFile, (T_U8)i, &attr);

		if(Utl_UStrCmpC(string, shortname) == 0)//是要设置的当前项，退出
		{
			//find and focus to it
			pListFile->curSubID = (T_U16)(pListFile->begSrID+i);
			pListFile->refresh = CTRL_REFRESH_ALL;
			pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;
			return;
		}

		i ++;
		j ++;
        
		if (j >= pListFile->cntEntrys)
        {
            break;                      //遍历了一遍没找到之前的当项，                 
        }      
			

		if(i >= CTRL_FILELIST_DISP_ENTRYS)
		{
            if(CTRL_FILELIST_DISP_ENTRYS==cnt)
            {
                i = 0;
            }
			    
            else
            {
                i = (T_U16)(CTRL_FILELIST_DISP_ENTRYS-1);
            }
                

			//move focus down one by one
			for(k=0; k<cnt; k++)
            {
                i = (T_U16)(i + (!ListFile_MoveFocus(pListFile, +1)));
            }         
				
            cnt = 1;
            
		}
	}

	pListFile->begSrID  = 0;                    //重新设置开始项和当前项
	pListFile->curSubID = 0;
	pListFile->refresh  = CTRL_REFRESH_ALL;
	pListFile->refreshEx = CTRL_REFRESH_FILELIST_ENTRYEX_ALL;

	e_findfirst(pListFile, (T_U8)(CTRL_FILELIST_DISP_ENTRYS));

}


#pragma arm section code = "_listfile_show_"

extern  T_U32	Gbl_GetFileIcon(T_U16 *filename)
{
#if (USE_COLOR_LCD)
	T_S16 pt;
#endif
	
	if(AK_NULL == filename)
		return ICON_FILE_ENTRYPTR;
	
#if (USE_COLOR_LCD)

	pt = Utl_UStrRevFndChr(filename, '.', 0);

	if(pt >= 0)
	{
		T_U16 *str = filename+pt+1;

		if((str[0]=='J' || str[0]=='j')
			&& (str[1]=='P' || str[1]=='p'))
			return eRES_IMAGE_IMG_ICON;//jpg file icon
		
		if((str[0]=='B' || str[0]=='b')
			&& (str[1]=='M' || str[1]=='m')
			&& (str[2]=='P' || str[2]=='p'))
			return eRES_IMAGE_IMG_ICON;//bmp file icon
		
		if((str[0]=='G' || str[0]=='g')
			&& (str[1]=='I' || str[1]=='i')
			&& (str[2]=='F' || str[2]=='f'))
			return eRES_IMAGE_IMG_ICON;//gif file icon
		if((str[0]=='T' || str[0]=='t')
			&& (str[1]=='X' || str[1]=='x')
			&& (str[2]=='T' || str[2]=='t'))
			return eRES_IMAGE_EBK_FILE;//txt file icon

	}
#endif

	return ICON_FILE_ENTRYPTR;
}

#pragma arm section code 

T_VOID	ListFile_SetNavigate(CListFileCtrl *pListFile, T_U16 menuID)
{
	if(!pListFile)
		return;
	if(INVALID_STRINGRES_ID == menuID)
		return;

	#if (USE_COLOR_LCD)
	pListFile->navigateID = menuID;
	{
		#ifndef USE_CONTROL_NOTITLE 
		pListFile->top += CTRL_WND_LINEHIGH;
		pListFile->height = CTRL_WND_HEIGHT-pListFile->top;
		pListFile->entryLines = (pListFile->height-CTRL_FILELIST_BEAUTIFY_INTVL)/CTRL_WND_LINEHIGH;
		#endif
	}
	#endif	

}

T_VOID	ListFile_SetTitle(CListFileCtrl *pListFile, T_U16 bkimgID)
{
	if(!pListFile)
		return;
	if(INVALID_IMAGERES_ID == bkimgID)
		return;

	#if (USE_COLOR_LCD)
	pListFile->bkimgTitle = bkimgID;
	#endif
}

#endif
