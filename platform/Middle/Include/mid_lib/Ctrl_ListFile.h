#ifndef __CLISTFILE_CTRL_H__
#define __CLISTFILE_CTRL_H__

#include "Ctrl_Public.h"
#include "Ctrl_Button.h"
#include "Fwl_osFS.h"
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif

#define CTRL_LISTFILE_FILTERLEN         MAX_FILTER_LEN 
#define CTRL_LISTFILE_FILEPATHLEN       (260+4)

#define CTRL_LISTFILE_DISP_FOLDONLY     FILTER_FOLDER    | FILTER_ONLYFOLDER//只显示所有目录
#define CTRL_LISTFILE_DISP_FILEONLY     FILTER_ONLYFILE //只显示文件
#define CTRL_LISTFILE_DISP_ALL          0x0000  //显示文件和目录 <目录在前，文件在后>
#define CTRL_LISTFILE_DISP_PATTERONLY   FILTER_DEEP//只显示满足过滤条件的那些目录或文件

#define CTRL_LISTFILE_DELETE_FOCUS      0x30    
#define CTRL_LISTFILE_DELETE_ALL        0x31

#define CTRL_LISTFILE_RESPONSE_DELETE   0x0001  

#define CTRL_LISTFILE_BEFORE_FILE       0xFFFFFFFE
#define CTRL_LISTFILE_FIRST_FILE        0x0
#define CTRL_LISTFILE_LAST_FILE         0xFFFFFFFF

#ifdef USE_HIDE_DRIVER
#define CTRL_LISTFILE_STYLE_DEFAULT             (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW | CTRL_LISTFILE_STYLE_ROOTDISK_DISPSHOW)
#else
#define CTRL_LISTFILE_STYLE_DEFAULT             (CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW | CTRL_LISTFILE_STYLE_ROOTDISK_DISPNONE)
#endif
#define CTRL_LISTFILE_STYLE_ROOTDIR_DISPNONE    0x00000000  //不显示盘符根目录
#define CTRL_LISTFILE_STYLE_ROOTDIR_DISPSHOW    0x00000001  //显示盘符根目录 ：根目录显示为'ROOT'
#define CTRL_LISTFILE_STYLE_ROOTDISK_DISPNONE   0x00000000  //不显示所有盘模式
#define CTRL_LISTFILE_STYLE_ROOTDISK_DISPSHOW   0x00000010  //显示所有盘模式
#define CTRL_LISTFILE_STYLE_SCROLLTXT_SELECT    0x00000100


typedef struct tagCListFileCtrl
{
    T_U16   cntEntrys;      //amount of file and fold number under parent directory
    T_U16   begSrID;        //current display entry pos within LCD 
    T_U16   curSubID;       //current focused entry pos within LCD
    T_U16   modeSearch;     //file find and search flag
    T_U16   refresh;        //repaint flag
    T_U16   refreshEx;      //only repaint small region flag

    T_U16   curCachePos;    //buffer to record parent direcotry and focused file absolute path 
    T_U16   bufCache[CTRL_LISTFILE_FILEPATHLEN];//buffer to record parent direcotry and focused file absolute path 
    T_U16   filter[CTRL_LISTFILE_FILTERLEN];//file find and search filter flag
    T_hFindCtrl hFSFind;        //find handle 

    T_U16   spcTitleStringID;
    T_U16   magicDriver;    //menu GUI mode or file GUI mode switcher
    T_U32   dwStyle;        //display style see above
    T_U16   entryLines;     //usable LCD lines to display file or fold entry
    T_U16   navigateID;     //navigate bar menu id
    T_U16   bkimgTitle;

    T_U16   wh;             //show rect width and height        
    T_U16   sepration;      //
    T_U16   addDis;
    T_U16   row;
    T_U16   column;
    T_U16   offset_x;
    T_U16   offset_y; 
    T_U16   showMode;       //list show mode

    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height;

    CTxtScrollCtrl *pTxtScroll;
    T_BOOL  bResetTxtScroll;

#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL  listfile_topbar;
#endif
        
}CListFileCtrl;

typedef enum
{
    LISTFILE_MODE = 0,
    DIAGRAM_MODE,
    NONENTITY_MODE
}T_LIST_SHOW_MODE;


T_BOOL  ListFile_InitShowMode(CListFileCtrl *pListFile, T_U16 wh, T_U16 sepration, T_LIST_SHOW_MODE showMode);
T_BOOL  ListFile_InitEx(CListFileCtrl *pListFile, T_U16 *rootDIR, T_U16 spcTitleStringID, T_U32 modeSearch, T_U16 patFilter[CTRL_LISTFILE_FILTERLEN], T_POS left, T_POS top, T_LEN width, T_LEN height);
#define ListFile_Init(pListFile, rootDIR, spcTitleStringID, modeSearch, patFilter)  ListFile_InitEx((pListFile), (rootDIR), (spcTitleStringID), (modeSearch), (patFilter), CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT)

T_VOID  ListFile_SetStyle(CListFileCtrl *pListFile, T_U32 dwStyle);
T_VOID  ListFile_Show(CListFileCtrl *pListFile);
T_U32   ListFile_Handler(CListFileCtrl *pListFile, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID  ListFile_SetRefresh(CListFileCtrl *pListFile);

T_VOID  ListFile_Free(CListFileCtrl *pListFile);

T_VOID  ListFile_DeleteItem(CListFileCtrl *pListFile, T_U8 delMode);

/***********************************************************************************
**brief:        获取焦点信息
**return:       返回焦点条目的绝对路径 <NOTE:必须立即使用, 否则将失效soon>
                bFile==AK_TRUE 焦点是文件， 否则是目录
***********************************************************************************/
T_U16*  ListFile_GetFocusFile(CListFileCtrl *pListFile, T_BOOL *bFile);

/***********************************************************************************
**brief:        获取焦点信息
**return:       返回焦点条目的顺序ID
***********************************************************************************/
T_U32   ListFile_GetCurFileID(CListFileCtrl *pListFile);

/***********************************************************************************
**brief:        设置焦点条目
**parm:         shortname:焦点文件/目录的名称 <不包括路径>
***********************************************************************************/
T_VOID  ListFile_SetCurFile(CListFileCtrl *pListFile, T_U16 *shortname);

/***********************************************************************************
**brief:        设置不同文件类型的自定义显示图标
**parm:         filename:包含后缀名的文件名, <NOTE:该函数需要人工添加函数内部内容>
***********************************************************************************/
extern  T_U32   Gbl_GetFileIcon(T_U16 *filename);

/***********************************************************************************
**brief:        设置文件列表导航栏
                menuID: 导航菜单资源ID
***********************************************************************************/
T_VOID  ListFile_SetNavigate(CListFileCtrl *pListFile, T_U16 menuID);

/***********************************************************************************
**brief:        设置标题栏图片
                bkimgID: 背景图片资源ID
***********************************************************************************/
T_VOID  ListFile_SetTitle(CListFileCtrl *pListFile, T_U16 bkimgID);


#endif

 

  

 
