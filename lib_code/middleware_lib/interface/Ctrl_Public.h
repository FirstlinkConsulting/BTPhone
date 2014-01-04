#ifndef __CTRL_PUBLIC_H__
#define __CTRL_PUBLIC_H__

#include "anyka_types.h"
#include "Gbl_global.h"
//#include "keypad.h"
#include "Eng_ImageResDisp.h"
#include "Eng_font.h"



/* 微系统控件窗口区域 */
#define CTRL_WND_LEFT               0
#define CTRL_WND_TOP                0
#define CTRL_WND_WIDTH              GRAPH_WIDTH
#define CTRL_WND_HEIGHT             GRAPH_HEIGHT

/* 微系统字体/图标和行数定义 */
#define CTRL_WND_FNTSZ              FONT_HEIGHT
#if (!USE_COLOR_LCD)
#define CTRL_WND_LINEHIGH           CTRL_WND_FNTSZ
#else
#define CTRL_WND_LINEHIGH           20
#endif
#define CTRL_WND_LINES              ((CTRL_WND_HEIGHT)/CTRL_WND_LINEHIGH)
#define CTRL_WND_ICONSZ             CTRL_WND_FNTSZ
#define CTRL_WND_LINEFONTMAX        (4+(2*MAIN_LCD_WIDTH)/FONT_WIDTH)
#define CTRL_WND_LINESMAX           (MAIN_LCD_HEIGHT/FONT_HEIGHT)

/* 控件是否支持显示标题栏 */
#if (USE_COLOR_LCD)
    //#ifndef USE_CONTROL_NOTITLE
        #define USE_CONTROL_NOTITLE
    //#endif
#else
    //#ifndef USE_CONTROL_NOTITLE
        //#define USE_CONTROL_NOTITLE
    //#endif    
#endif
//注：默认状态黑白屏使用控件title, 彩屏不使用控件title


/* 部分必须固定调节的控件区域 */
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
#define CTRL_BTN_WIDTH              28//(3*CTRL_WND_FNTSZ)
#define CTRL_BTN_HEIGHT             CTRL_WND_LINEHIGH
#else
#define CTRL_BTN_WIDTH              92//(3*CTRL_WND_FNTSZ)
#define CTRL_BTN_HEIGHT             50
#endif

/* 微系统响应事件列表 */
#define CTRL_EVT_KEY_UP             kbLEFT 
#define CTRL_EVT_KEY_DOWN           kbRIGHT
#define CTRL_EVT_KEY_LEFT           kbLEFT 
#define CTRL_EVT_KEY_RIGHT          kbRIGHT
#define CTRL_EVT_KEY_OK             kbOK
#define CTRL_EVT_KEY_CANCEL         kbRECORD
#define CTRL_EVT_KEY_SELECT         kbMODE
#define CTRL_EVT_KEY_VOLSUB         kbVOLSUB    //only for progress
#define CTRL_EVT_KEY_VOLADD         kbVOLADD    //only for progress
  

/* 微系统刷新标记 */
#define CTRL_REFRESH_ALL            0xFFFF
#define CTRL_REFRESH_NONE           0x0000

//控件返回特殊值
#define CTRL_RESPONSE_QUIT          0xFFFE
#define CTRL_RESPONSE_NONE          0xFFFF

/* 微系统变量 */
#define CTRL_CUR_LANGUAGE           gb.Lang

/* 显示细则*/
#define CTRL_WND_STRINGWTH(x)       ((T_U16)(x+CTRL_WND_FNTSZ))     
#if (USE_COLOR_LCD)
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    #define CTRL_WND_FONTCOLOR      CLR_BLUE
#else
    #define CTRL_WND_FONTCOLOR      CLR_WHITE
#endif
    #define CTRL_WND_BACKCOLOR      RGB_COLOR(10,36,106)
    #define CTRL_WND_SELFONT        CLR_WHITE
    #define CTRL_BAR_COLOR          RGB_COLOR(0, 128, 128)
#else
    #define CTRL_WND_FONTCOLOR      CLR_WHITE
    #define CTRL_WND_BACKCOLOR      CLR_BLACK
    #define CTRL_BAR_COLOR          CLR_WHITE
#endif

/* 特殊资源位置 */
#include "Gbl_Resource.h"

#define STRING_QUIT_ENTRYID         eRES_STR_QUIT           // '退出'' string ID <待定:由资源配置文件决定>
#define STRING_OK_ENTRYID           eRES_STR_OK
#define STRING_CANCEL_ENTRYID       eRES_STR_CANCEL

#define ICON_FOLD_ENTRYPTR          eRES_IMAGE_FOLDICON     // 目录 icon ID      <待定:由资源配置文件决定>
#define ICON_FILE_ENTRYPTR          eRES_IMAGE_FILEICON     // 文件 icon ID      <待定:由资源配置文件决定>
#if (USE_COLOR_LCD)
#define ICON_HINT_ENTRYPTR          INVALID_IMAGERES_ID     // 提示 icon ID      <待定:由资源配置文件决定>
#define ICON_SELECT_HINTPTR         eRES_IMAGE_MENUARR3
#define ICON_COMMON_IMAGEPTR        eRES_IMAGE_MENUARR3
#define ICON_DRIVER_ENTRYPTR        eRES_IMAGE_DRIVEICON
#else
#define ICON_HINT_ENTRYPTR          eRES_IMAGE_HINTICON     // 提示 icon ID      <待定:由资源配置文件决定>
#define ICON_SELECT_HINTPTR         eRES_IMAGE_ICONTHROW
#define ICON_COMMON_IMAGEPTR        eRES_IMAGE_MENUICON
#define ICON_DRIVER_ENTRYPTR        eRES_IMAGE_FOLDICON
#endif



#define PROGRESS_HOR_IMGPTR         eRES_IMAGE_SBAR 
#define PROGRESS_SLIDER_PTR         eRES_IMAGE_SSLIDER  
#define BUTTON_YES_IMAGPTR          eRES_IMAGE_CDEL_YES
#define BUTTON_NO_IMAGEPTR          eRES_IMAGE_CDEL_NO

#if (USE_COLOR_LCD)
#if ((3 != LCD_TYPE)||(1 != LCD_HORIZONTAL))
#define DIGITAL_BIG12_WTH           12
#else
#define DIGITAL_BIG12_WTH           25
#endif
#define DIGITAL_BIG12_0             eRES_IMAGE_CHN0
#define DIGITAL_BIG12_1             eRES_IMAGE_CHN1
#define DIGITAL_BIG12_2             eRES_IMAGE_CHN2
#define DIGITAL_BIG12_3             eRES_IMAGE_CHN3
#define DIGITAL_BIG12_4             eRES_IMAGE_CHN4
#define DIGITAL_BIG12_5             eRES_IMAGE_CHN5
#define DIGITAL_BIG12_6             eRES_IMAGE_CHN6
#define DIGITAL_BIG12_7             eRES_IMAGE_CHN7
#define DIGITAL_BIG12_8             eRES_IMAGE_CHN8
#define DIGITAL_BIG12_9             eRES_IMAGE_CHN9
#define DIGITAL_BIGNEG              eRES_IMAGE_FMNEG
#endif

#if (USE_COLOR_LCD)
#if ((3 != LCD_TYPE)||(1 != LCD_HORIZONTAL))
#define MENU_BACK_IMAGEPTR          eRES_IMAGE_CTRLBKG
#define PROGRESS_HOR_WIDTH          100
#define PROGRESS_VER_HEIGHT         16
#else
#define MENU_BACK_IMAGEPTR          eRES_IMAGE_CTRLBKG
#define PROGRESS_HOR_WIDTH          289
#define PROGRESS_VER_HEIGHT         20
#endif
#define PROGRESS_SLIDER_WTH         8
#define MSGB0X_BK_IMAGEPTR          eRES_IMAGE_CTRLBKG
#define BUTTON_BK_IMAGEPTR          eRES_IMAGE_BUTTON
#define BUTTON_FC_IMAGEPTR          eRES_IMAGE_BUTTON_SEL
#define ICON_MASK_COLOR             IMG_TRNSPRNT_COLOR
#define TITLE_BKG_IMAGEPTR          eRES_IMAGE_PUB_TOPBAR_BK
#define CTRL_WND_ICONWTH            16
#define PROGRESS_HOR_IMGPTR         eRES_IMAGE_SBAR
#define MENU_SELBAR_IMAGEPTR        eRES_IMAGE_FILENBK
#define FILE_SELBAR_IMAGEPTR        eRES_IMAGE_FILEBCK
#define CTRL_WND_TITLEHTH           28
#define NAVGATE_BK_IMAGEPTR         eRES_IMAGE_NAVGATEBAR
#else
#define SCROLL_VER_IMAGEPTR         eRES_IMAGE_SITEM1
#define SCROLL_VER_LEVECNT          0xB
#define SCROLL_VER_WIDTH            8
#define CTRL_WND_ICONWTH            8
#define PROGRESS_HOR_WIDTH          64
#define PROGRESS_VER_HEIGHT         8
#define PROGRESS_HOR_GRDWTH         4
#define PROGRESS_SLIDER_WTH         5
#define PROGRESS_HOR_FRMWTH         4
#define PROGRESS_SBK_IMGPTR         eRES_IMAGE_SBKGRD
#define PROGRESS_FRM_IMGPTR         eRES_IMAGE_SBKFRM
#define CTRL_WND_TITLEHTH           16
#endif  

#define PROGRESS_EDGE_LEFT          ((CTRL_WND_WIDTH-PROGRESS_HOR_WIDTH)/2)
#define FILE_DRIVE_MENUBEG          eRES_STR_DRIVER_MOUNT
#define FILE_DRIVE_STRINGID         eRES_STR_LOCAL_FOLDER

#define FILE_STRING_DRIVE_A         eRES_STR_DRIVER_AFLASH
#define FILE_STRING_DRIVE_B         eRES_STR_DRIVER_BSD


#endif

