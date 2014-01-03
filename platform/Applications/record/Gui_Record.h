/**
 * @file Gui_Record.h
 * @brief This file is for record gui function ( field strength, battery etc) prototype
 *
 */

#ifndef __GUI_RECORD_H__
#define __GUI_RECORD_H__

//#include "akdefine.h"
//#include "Gbl_Define.h"
//#include "Ctrl_ListMenu.h"
//#include "Fwl_LCD.h"
//#include "Eng_Font.h"
//#include "Fwl_osFS.h"
//#include "Log_record.h"
//#include "Gui_Common.h"

//mark return from where for Record all Module
typedef enum{
    EXIT_FROM_MAINDISK=0x11,
    EXIT_FROM_SETMODE,
    EXIT_FROM_QUIT
}T_RECORD_EXIT_MODE;

/****************************************
------note:Record Module UI config-------
*****************************************/
#define RECORD_REFRESH_ALL         0xffffffff
#define RECORD_REFRESH_NONE        0x00000000
#define RECORD_REFRESH_CURTIME     0x00000001
#define RECORD_REFRESH_TOTALTIME   0x00000002
#define RECORD_REFRESH_ICON        0x00000004
#define RECORD_REFRESH_FILENAME    0x00000008
#define RECORD_REFRESH_SAVING      0x00000010
#define RECORD_REFRESH_MEMORY_FULL 0x00000020
#define RECORD_REFRESH_ICON_MODE   0x00000040
#define RECORD_REFRESH_FILENUM     0x00000080
#define RECORD_REFRESH_INIT        0x00000100
#define RECORD_REFRESH_FILE_FULL   0x00000200
#define RECORD_REFRESH_STARTFAIL   0x00000201
#define RECORD_REFRESH_PATH        0x00000400
#define RECORD_REFRESH_STATE       0x00000800
#define RECORD_REFRESH_BACKGROUND  0x00001000
#define RECORD_REFRESH_DATE        0x00002000
#define RECORD_REFRESH_CURTIME_BG  0x00004000
#define RECORD_REFRESH_BATTERY     0x00008000
#define RECORD_REFRESH_WAVE        0x00010000
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#define RECORD_REFRESH_TOPBAR      0x00040000
#endif

#define RES_DISP_HCENTER(ResID)     ((GRAPH_WIDTH > GetStringDispWidth(CP_UNICODE, CONVERT2STR(ResID), 0)) ? \
    ((GRAPH_WIDTH - GetStringDispWidth(CP_UNICODE, CONVERT2STR(ResID), 0)) >>1) :0)

/*start record submodule config */
#if  (USE_COLOR_LCD)
#if (LCD_TYPE == 3)
#if (1 == LCD_HORIZONTAL)
#define SMALL_NUMIMAGE_DIST         8
#define BIG_NUMIMAGE_DIST           21
#define PROGRESS_LENGTH             222
#define TOTIME_IMAGE_DIST           10
//totalnum of files now  
#define CURRENT_FILENUM_TOP_POS     48
#define CURRENT_FILENUM_LEFT_POS    228

//current time
#define CURTIME_TOP_POS             (172)
#define CURTIME_LEFT_POS            (108)

#define CURTIME_HOUR_TOP_POS        CURTIME_TOP_POS
#define CURTIME_HOUR_LEFT_POS       CURTIME_LEFT_POS
#define CURTIME_LEFT_FLASH_POS      (CURTIME_LEFT_POS- BIG_NUMIMAGE_DIST)

#define CURTIME_MINS_TOP_POS        CURTIME_TOP_POS
#define CURTIME_MINS_LEFT_POS       (CURTIME_HOUR_LEFT_POS+55)  

#define CURTIME_SEC_TOP_POS         CURTIME_TOP_POS
#define CUTTIME_SEC_LEFT_POS        (CURTIME_MINS_LEFT_POS+55)  

//total  time position
#define TOTALTIME_TOP_POS           221
#define TOTALTIME_LEFT_POS          215

#define TOTALTIME_HOUR_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_HOUR_LEFT_POS     TOTALTIME_LEFT_POS

#define TOTALTIME_MINS_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_MINS_LEFT_POS     TOTALTIME_HOUR_LEFT_POS+23

#define TOTALTIME_SEC_TOP_POS       TOTALTIME_TOP_POS
#define TOTALTIME_SEC_LEFT_POS      TOTALTIME_MINS_LEFT_POS+23  

//file name position
#define FILENAME_TOP_POS            154 
#define FILENAME_LEFT_POS           148

//date position
#define DATE_TOP_POS                5
#define DATE_LEFT_POS               137

//record mode Icon positon
#define MODE_ICON_TOP_POS           120
#define MODE_ICON_LEFT_POS          10

//record Status Icon positon
#define STATUS_ICON_TOP_POS         198
#define STATUS_ICON_LEFT_POS        11

//progress position
#define PROGRESS_ICON_TOP_POS       213
#define PROGRESS_ICON_LEFT_POS      50

//battery position
#define BATTERY_ICON_TOP_POS        5       
#define BATTERY_ICON_LEFT_POS       296

//path position 
#define PATH_TOP_POS                32           
#define PATH_LEFT_POS               7 
//save image position
#define IMG_SAVING_LEFT_POS         0
#define IMG_SAVING_TOP_POS          0
//save hint string position
#define STR_SAVING_LEFT_POS         120
#define STR_SAVING_TOP_POS          140

//init string position
#define STR_INIT_LEFT_POS           RES_DISP_HCENTER(eRES_STR_AUDIO_HINT_WAITING)
#define STR_INIT_TOP_POS            120

//nandflash full position
#define STR_NANDFLASH_FULL_TOP_POS  ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_NANDFLASH_FULL_LEFT_POS RES_DISP_HCENTER(eRES_STR_MEMORY_FULL)

//file full position
#define STR_FILE_FULL_TOP_POS       ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_FILE_FULL_LEFT_POS      RES_DISP_HCENTER(eRES_STR_FILE_FULL)
/*end record submodule config*/

//start fail position
#define STR_START_FAIL_TOP_POS      ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_START_FAIL_LEFT_POS     RES_DISP_HCENTER(eRES_STR_START_FAIL)


/*start recordmenu submodule config*/
#define RECORDMENU_TITLE_LEFT_POS   0
#define RECORDMENU_TITLE_TOP_POS    0
#define RECORDMENU_TITLE_WIDTH      320
#define RECORDMENU_TITLE_HIGHT      32
/*end recordmenu submodule config*/

/*start recordsetmode submodule config*/

#define RECORDSETMODE_TITLE_LEFT_POS   0
#define RECORDSETMODE_TITLE_TOP_POS    0
#define RECORDSETMODE_TITLE_WIDTH      320
#define RECORDSETMODE_TITLE_HIGHT      32

#else  //endif LCD_HORIZONTAL


#define SMALL_NUMIMAGE_DIST         16
#define BIG_NUMIMAGE_DIST           24
#define PROGRESS_LENGTH             150

//totalnum of files now  
#define CURRENT_FILENUM_TOP_POS     68 
#define CURRENT_FILENUM_LEFT_POS    196

//current time
#define CURTIME_TOP_POS             107
#define CURTIME_LEFT_POS            (90)

#define CURTIME_HOUR_TOP_POS        CURTIME_TOP_POS
#define CURTIME_HOUR_LEFT_POS       CURTIME_LEFT_POS
#define CURTIME_LEFT_FLASH_POS      (CURTIME_LEFT_POS- BIG_NUMIMAGE_DIST)

#define CURTIME_MINS_TOP_POS        CURTIME_TOP_POS
#define CURTIME_MINS_LEFT_POS       (CURTIME_HOUR_LEFT_POS+60)

#define CURTIME_SEC_TOP_POS         CURTIME_TOP_POS
#define CUTTIME_SEC_LEFT_POS        (CURTIME_MINS_LEFT_POS+60)

//total  time position
#define TOTALTIME_TOP_POS           248
#define TOTALTIME_LEFT_POS          130

#define TOTALTIME_HOUR_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_HOUR_LEFT_POS     TOTALTIME_LEFT_POS

#define TOTALTIME_MINS_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_MINS_LEFT_POS     TOTALTIME_HOUR_LEFT_POS+46

#define TOTALTIME_SEC_TOP_POS       TOTALTIME_TOP_POS
#define TOTALTIME_SEC_LEFT_POS      TOTALTIME_MINS_LEFT_POS+46  

//file name position
#define FILENAME_TOP_POS            220
#define FILENAME_LEFT_POS           90

//date position
#define DATE_TOP_POS                FILENAME_TOP_POS
#define DATE_LEFT_POS               148

//record mode Icon positon
#define MODE_ICON_TOP_POS           208
#define MODE_ICON_LEFT_POS          4

//record Status Icon positon
#define STATUS_ICON_TOP_POS         278
#define STATUS_ICON_LEFT_POS        6

//progress position
#define PROGRESS_ICON_TOP_POS       288
#define PROGRESS_ICON_LEFT_POS      60

//battery position
#define BATTERY_ICON_TOP_POS        8       
#define BATTERY_ICON_LEFT_POS       194

//path position 
#define PATH_TOP_POS                169           
#define PATH_LEFT_POS               47  
//save image position
#define IMG_SAVING_LEFT_POS         68
#define IMG_SAVING_TOP_POS          90
//save hint string position
#define STR_SAVING_LEFT_POS         50
#define STR_SAVING_TOP_POS          180

//init string position
#define STR_INIT_LEFT_POS           RES_DISP_HCENTER(eRES_STR_AUDIO_HINT_WAITING)
#define STR_INIT_TOP_POS            160

//nandflash full position
#define STR_NANDFLASH_FULL_TOP_POS  ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_NANDFLASH_FULL_LEFT_POS RES_DISP_HCENTER(eRES_STR_MEMORY_FULL)

//file full position
#define STR_FILE_FULL_TOP_POS       ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_FILE_FULL_LEFT_POS      RES_DISP_HCENTER(eRES_STR_FILE_FULL)
/*end record submodule config*/

//start fail position
#define STR_START_FAIL_TOP_POS      ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_START_FAIL_LEFT_POS     RES_DISP_HCENTER(eRES_STR_START_FAIL)

/*start recordmenu submodule config*/
#define RECORDMENU_TITLE_LEFT_POS   0
#define RECORDMENU_TITLE_TOP_POS    0
#define RECORDMENU_TITLE_WIDTH      240
#define RECORDMENU_TITLE_HIGHT      32
/*end recordmenu submodule config*/

/*start recordsetmode submodule config*/

#define RECORDSETMODE_TITLE_LEFT_POS   0
#define RECORDSETMODE_TITLE_TOP_POS    0
#define RECORDSETMODE_TITLE_WIDTH      240
#define RECORDSETMODE_TITLE_HIGHT      32

/*end recordsetmode submodule config*/
#endif
#else
#define SMALL_NUMIMAGE_DIST         8
#define BIG_NUMIMAGE_DIST           12
#define PROGRESS_LENGTH             75

//totalnum of files now  
#define CURRENT_FILENUM_TOP_POS     34 
#define CURRENT_FILENUM_LEFT_POS    105

//current time
#define CURTIME_TOP_POS             52
#define CURTIME_LEFT_POS            (36+ 7)
#define CURTIME_LEFT_FLASH_POS      (CURTIME_LEFT_POS- BIG_NUMIMAGE_DIST)

#define CURTIME_HOUR_TOP_POS        CURTIME_TOP_POS
#define CURTIME_HOUR_LEFT_POS       CURTIME_LEFT_POS

#define CURTIME_MINS_TOP_POS        CURTIME_TOP_POS
#define CURTIME_MINS_LEFT_POS       (CURTIME_HOUR_LEFT_POS+36)

#define CURTIME_SEC_TOP_POS         CURTIME_TOP_POS
#define CUTTIME_SEC_LEFT_POS        (CURTIME_MINS_LEFT_POS+36)

//total  time position
#define TOTALTIME_TOP_POS           125
#define TOTALTIME_LEFT_POS          70

#define TOTALTIME_HOUR_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_HOUR_LEFT_POS     TOTALTIME_LEFT_POS

#define TOTALTIME_MINS_TOP_POS      TOTALTIME_TOP_POS
#define TOTALTIME_MINS_LEFT_POS     TOTALTIME_HOUR_LEFT_POS+24

#define TOTALTIME_SEC_TOP_POS       TOTALTIME_TOP_POS
#define TOTALTIME_SEC_LEFT_POS      TOTALTIME_MINS_LEFT_POS+24  

//file name position
#define FILENAME_TOP_POS            108
#define FILENAME_LEFT_POS           45

//date position
#define DATE_TOP_POS                108
#define DATE_LEFT_POS               67

//record mode Icon positon
#define MODE_ICON_TOP_POS           104
#define MODE_ICON_LEFT_POS          2

//record Status Icon positon
#define STATUS_ICON_TOP_POS         140
#define STATUS_ICON_LEFT_POS        3

//progress position
#define PROGRESS_ICON_TOP_POS       143
#define PROGRESS_ICON_LEFT_POS      22

//battery position
#define BATTERY_ICON_TOP_POS        4       
#define BATTERY_ICON_LEFT_POS       (GRAPH_WIDTH -Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0) - 3)

//path position 
#define PATH_TOP_POS                80           
#define PATH_LEFT_POS               25  
//save image position
#define IMG_SAVING_LEFT_POS         9
#define IMG_SAVING_TOP_POS          46
//save hint string position
#define STR_SAVING_LEFT_POS         5
#define STR_SAVING_TOP_POS          95

//init string position
#define STR_INIT_LEFT_POS           RES_DISP_HCENTER(eRES_STR_AUDIO_HINT_WAITING)
#define STR_INIT_TOP_POS            80

//nandflash full position
#define STR_NANDFLASH_FULL_TOP_POS  ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_NANDFLASH_FULL_LEFT_POS RES_DISP_HCENTER(eRES_STR_MEMORY_FULL)

//file full position
#define STR_FILE_FULL_TOP_POS       ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_FILE_FULL_LEFT_POS      RES_DISP_HCENTER(eRES_STR_FILE_FULL)

//start fail position
#define STR_START_FAIL_TOP_POS      ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_START_FAIL_LEFT_POS     RES_DISP_HCENTER(eRES_STR_START_FAIL)
/*end record submodule config*/

/*start recordmenu submodule config*/
#define RECORDMENU_TITLE_LEFT_POS   0
#define RECORDMENU_TITLE_TOP_POS    0
#define RECORDMENU_TITLE_WIDTH      128
#define RECORDMENU_TITLE_HIGHT      32
/*end recordmenu submodule config*/

/*start recordsetmode submodule config*/

#define RECORDSETMODE_TITLE_LEFT_POS   0
#define RECORDSETMODE_TITLE_TOP_POS    0
#define RECORDSETMODE_TITLE_WIDTH      128
#define RECORDSETMODE_TITLE_HIGHT      32

/*end recordsetmode submodule config*/
#endif

#else//BW lcd
/*start record submodule config*/
//totalnum of files after record  
#define AFTERREC_FILENUM_TOP_POS     0 
#define AFTERREC_FILENUM_LEFT_POS    0 

//current time position
#define CURTIME_TOP_POS              0
#define CURTIME_LEFT_POS             32

//totalnum of files now  
#define CURRENT_FILENUM_TOP_POS      16 
#define CURRENT_FILENUM_LEFT_POS     0 

//battery position
#define BATTERY_ICON_TOP_POS         0      
#define BATTERY_ICON_LEFT_POS        (GRAPH_WIDTH-Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0))

//total  time position
#define TOTALTIME_TOP_POS            16
#define TOTALTIME_LEFT_POS           32

//file name position
#define FILENAME_TOP_POS             32
#define FILENAME_LEFT_POS            0

//date position
#define DATE_TOP_POS                 32
#define DATE_LEFT_POS                62

//record mode Icon positon
#define MODE_ICON_TOP_POS            48
#define MODE_ICON_LEFT_POS           18

//record Status Icon positon
#define STATUS_ICON_TOP_POS          48
#define STATUS_ICON_LEFT_POS         42

//record Icon position  
#define MIC_ICON_TOP_POS             48
#define REC_ICON_TOP_POS             MIC_ICON_TOP_POS
#define MIC_ICON_LEFT_POS            0       
#define REC_ICON_LEFT_POS            MIC_ICON_LEFT_POS

//save hint string position
#define STR_SAVING_LEFT_POS          RES_DISP_HCENTER(eRES_STR_SAVE)
#define STR_SAVING_TOP_POS           16

//init hint string position
#define STR_INIT_LEFT_POS            RES_DISP_HCENTER(eRES_STR_STARTING)
#define STR_INIT_TOP_POS             24

//nandflash full position
#define STR_NANDFLASH_FULL_TOP_POS  ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_NANDFLASH_FULL_LEFT_POS RES_DISP_HCENTER(eRES_STR_MEMORY_FULL)

//file full position
#define STR_FILE_FULL_TOP_POS       ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_FILE_FULL_LEFT_POS      (RES_DISP_HCENTER(eRES_STR_FILE_FULL))

//start fail position
#define STR_START_FAIL_TOP_POS      ((GRAPH_HEIGHT-FONT_HEIGHT)>>1)
#define STR_START_FAIL_LEFT_POS     RES_DISP_HCENTER(eRES_STR_START_FAIL)

/*end record submodule config*/

#endif//end USE_COLOR_LCD
/***********End Record Module UI Config***********/


//void record_paint();

//void record_menu_painttitle();

//void record_setmode_painttitle();

#endif
