///Gbl_ImageRes.h
#ifndef __GBL_IMAGERES_H__
#define __GBL_IMAGERES_H__

#if (USE_COLOR_LCD)
typedef enum {
eRES_IMAGE_AUDBK,
eRES_IMAGE_AUDBATL0,
eRES_IMAGE_AUDBATL1,
eRES_IMAGE_AUDBATL2,
eRES_IMAGE_AUDBATL3,
eRES_IMAGE_AUDBATL4,
eRES_IMAGE_AUDCYCNORMAL,
eRES_IMAGE_AUDCYCRANDOM,
eRES_IMAGE_AUDCYCSINGLE,
eRES_IMAGE_AUDCYCTOTAL,
eRES_IMAGE_AUDEQ,
eRES_IMAGE_AUDERRMESSAGE,
eRES_IMAGE_AUDMUSICICON,
eRES_IMAGE_AUDMUSICLYCICON,
eRES_IMAGE_AUDNEXT,
eRES_IMAGE_AUDNEXTDOWN,
eRES_IMAGE_AUDPREV,
eRES_IMAGE_AUDPREVDOWN,
eRES_IMAGE_AUDPRGRS,
eRES_IMAGE_AUDPRGRSBACK,
eRES_IMAGE_AUDSTATLYRPLAY,
eRES_IMAGE_AUDSTATPAUSE,
eRES_IMAGE_AUDSTATPAUSEDOWN,
eRES_IMAGE_AUDSTATPLAY,
eRES_IMAGE_AUDSTATPLAYDOWN,
eRES_IMAGE_AUDSTATSEEKB,
eRES_IMAGE_AUDSTATSEEKF,
eRES_IMAGE_AUDTYPEAMR,
eRES_IMAGE_AUDTYPEAMR_P,
eRES_IMAGE_AUDUIJUMPEFF1,
eRES_IMAGE_AUDUIJUMPEFF2,
eRES_IMAGE_AUDUIJUMPEFF3,
eRES_IMAGE_AUDUIJUMPEFF4,
eRES_IMAGE_AUDUIJUMPEFF5,
eRES_IMAGE_AUDVOL,
eRES_IMAGE_AUDVOL0,
eRES_IMAGE_AUDVOL1,
eRES_IMAGE_AUDVOL10,
eRES_IMAGE_AUDVOL2,
eRES_IMAGE_AUDVOL3,
eRES_IMAGE_AUDVOL4,
eRES_IMAGE_AUDVOL5,
eRES_IMAGE_AUDVOL6,
eRES_IMAGE_AUDVOL7,
eRES_IMAGE_AUDVOL8,
eRES_IMAGE_AUDVOL9,
eRES_IMAGE_BUTTON,
eRES_IMAGE_BUTTON_SEL,
eRES_IMAGE_BKDPLAY,
eRES_IMAGE_CSETUP0,
eRES_IMAGE_CSETUP1,
eRES_IMAGE_CSETUP2,
eRES_IMAGE_CHARGER0,
eRES_IMAGE_CHARGER1,
eRES_IMAGE_CHARGER2,
eRES_IMAGE_CHARGER3,
eRES_IMAGE_CHARGER4,
eRES_IMAGE_CHARGER5,
eRES_IMAGE_CHARGERBG,
eRES_IMAGE_CPHOTO0,
eRES_IMAGE_CPHOTO1,
eRES_IMAGE_CPHOTO2,
eRES_IMAGE_CTRLBKG,
eRES_IMAGE_DRIVEICON,
eRES_IMAGE_FMSMALL,
eRES_IMAGE_FILEICON,
eRES_IMAGE_FOLDICON,
eRES_IMAGE_HINTICON,
eRES_IMAGE_ICONTHROW,
eRES_IMAGE_MBPS,
eRES_IMAGE_MENUICON,
eRES_IMAGE_MENU_TITLE,
eRES_IMAGE_MENUDOT,
eRES_IMAGE_MENUDOTA,
eRES_IMAGE_MFOLDER,
eRES_IMAGE_MSGBOX,
eRES_IMAGE_RE_0,
eRES_IMAGE_RE_1,
eRES_IMAGE_RE_2,
eRES_IMAGE_RE_3,
eRES_IMAGE_RE_4,
eRES_IMAGE_RE_5,
eRES_IMAGE_RE_6,
eRES_IMAGE_RE_7,
eRES_IMAGE_RE_8,
eRES_IMAGE_RE_9,
eRES_IMAGE_RE_910,
eRES_IMAGE_RECMICON,
eRES_IMAGE_START,
eRES_IMAGE_UDISKIDLE,
eRES_IMAGE_UDISKOK,
eRES_IMAGE_VIDEOPLAY,
eRES_IMAGE_ALARM_FOCUS,
eRES_IMAGE_ALARM_ICON,
eRES_IMAGE_ALBUM,
eRES_IMAGE_ALBUM_FOCOUS,
eRES_IMAGE_AMR5K,
eRES_IMAGE_AMR5K_P,
eRES_IMAGE_ARROW,
eRES_IMAGE_ARROW1,
eRES_IMAGE_ARROW2,
eRES_IMAGE_ARTIST ,
eRES_IMAGE_ARTIST_FOCOUS,
eRES_IMAGE_AUD_BAC_SGL,
eRES_IMAGE_BACKREC,
eRES_IMAGE_BATVAL0,
eRES_IMAGE_BATVAL1,
eRES_IMAGE_BATVAL2,
eRES_IMAGE_BATVAL3,
eRES_IMAGE_BATVALFULL,
eRES_IMAGE_CCALENDAR0,
eRES_IMAGE_CCALENDAR1,
eRES_IMAGE_CCALENDAR2,
eRES_IMAGE_CFM0,
eRES_IMAGE_CFM1,
eRES_IMAGE_CFM2,
eRES_IMAGE_CREC0,
eRES_IMAGE_CREC1,
eRES_IMAGE_CREC2,
eRES_IMAGE_CALARM0,
eRES_IMAGE_CALARM1,
eRES_IMAGE_CALARM2,
eRES_IMAGE_CDEL_NO,
eRES_IMAGE_CDEL_YES,
eRES_IMAGE_CEBOOK0,
eRES_IMAGE_CEBOOK1,
eRES_IMAGE_CEBOOK2,
eRES_IMAGE_CHN0,
eRES_IMAGE_CHN1,
eRES_IMAGE_CHN2,
eRES_IMAGE_CHN3,
eRES_IMAGE_CHN4,
eRES_IMAGE_CHN5,
eRES_IMAGE_CHN6,
eRES_IMAGE_CHN7,
eRES_IMAGE_CHN8,
eRES_IMAGE_CHN9,
eRES_IMAGE_CHNUM0,
eRES_IMAGE_CHNUM1,
eRES_IMAGE_CHNUM2,
eRES_IMAGE_CHNUM3,
eRES_IMAGE_CHNUM4,
eRES_IMAGE_CHNUM5,
eRES_IMAGE_CHNUM6,
eRES_IMAGE_CHNUM7,
eRES_IMAGE_CHNUM8,
eRES_IMAGE_CHNUM9,
eRES_IMAGE_CLOCK1,
eRES_IMAGE_CLOCK2,
eRES_IMAGE_CLOCK3,
eRES_IMAGE_CMOVIE0,
eRES_IMAGE_CMOVIE1,
eRES_IMAGE_CMOVIE2,
eRES_IMAGE_CMUSIC0,
eRES_IMAGE_CMUSIC1,
eRES_IMAGE_CMUSIC2,
eRES_IMAGE_CNES0,
eRES_IMAGE_CNES1,
eRES_IMAGE_CNES2,
eRES_IMAGE_COMPROGBK,
eRES_IMAGE_CUSTOM,
eRES_IMAGE_CUSTOM_FOCOUS,
eRES_IMAGE_CVOICE0,
eRES_IMAGE_CVOICE1,
eRES_IMAGE_CVOICE2,
eRES_IMAGE_DWNARROW,
eRES_IMAGE_EBK_BMK,
eRES_IMAGE_EBK_BMKFOCUS,
eRES_IMAGE_EBK_FILE,
eRES_IMAGE_EBK_LASTPAGE,
eRES_IMAGE_EBK_LASTPAGE_P,
eRES_IMAGE_EBK_LIST_BG,
eRES_IMAGE_EBK_MENU,
eRES_IMAGE_EBK_MENU_P,
eRES_IMAGE_EBK_NEXTPAGE,
eRES_IMAGE_EBK_NEXTPAGE_P,
eRES_IMAGE_EBK_RETURN,
eRES_IMAGE_EBK_RETURN_P,
eRES_IMAGE_FILEBCK,
eRES_IMAGE_FILENAME,
eRES_IMAGE_FILENAME_FOCOUS,
eRES_IMAGE_FILENBK,
eRES_IMAGE_FLOATITEM,
eRES_IMAGE_FLOATITEM_P,
eRES_IMAGE_FMCHCHAR,
eRES_IMAGE_FMCHBCK,
eRES_IMAGE_FM_MHZ,
eRES_IMAGE_FMCURJPBK1,
eRES_IMAGE_FMCURUSBK,
eRES_IMAGE_FMJPBCK,
eRES_IMAGE_FMNEG,
eRES_IMAGE_FMNUM0,
eRES_IMAGE_FMNUM1,
eRES_IMAGE_FMNUM2,
eRES_IMAGE_FMNUM3,
eRES_IMAGE_FMNUM4,
eRES_IMAGE_FMNUM5,
eRES_IMAGE_FMNUM6,
eRES_IMAGE_FMNUM7,
eRES_IMAGE_FMNUM8,
eRES_IMAGE_FMNUM9,
eRES_IMAGE_FMNUM910,
eRES_IMAGE_FMNUMBCK,
eRES_IMAGE_FMSTEJPBCK,
eRES_IMAGE_FMSTEUSBCK,
eRES_IMAGE_FMTOPBARBCK,
eRES_IMAGE_FMUSBCK,
eRES_IMAGE_FMVOL,
eRES_IMAGE_FMVOL0,
eRES_IMAGE_FMVOL1,
eRES_IMAGE_FMVOL10,
eRES_IMAGE_FMVOL2,
eRES_IMAGE_FMVOL3,
eRES_IMAGE_FMVOL4,
eRES_IMAGE_FMVOL5,
eRES_IMAGE_FMVOL6,
eRES_IMAGE_FMVOL7,
eRES_IMAGE_FMVOL8,
eRES_IMAGE_FMVOL9,
eRES_IMAGE_FMVOL_NORMAL_MUTE,
eRES_IMAGE_FMVOL_PRESS,
eRES_IMAGE_FMVOL_PRESS_MUTE,
eRES_IMAGE_FMVOLMBCK,
eRES_IMAGE_FNUM0,
eRES_IMAGE_FNUM1,
eRES_IMAGE_FNUM2,
eRES_IMAGE_FNUM3,
eRES_IMAGE_FNUM4,
eRES_IMAGE_FNUM5,
eRES_IMAGE_FNUM6,
eRES_IMAGE_FNUM7,
eRES_IMAGE_FNUM8,
eRES_IMAGE_FNUM9,
eRES_IMAGE_FNUM910,
eRES_IMAGE_FORMAT_ERROR,
eRES_IMAGE_GENRE,
eRES_IMAGE_GENRE_FOCOUS,
eRES_IMAGE_IMG_ERROR,
eRES_IMAGE_IMG_ICON,
eRES_IMAGE_IMG_WAIT,
eRES_IMAGE_KBPS,
eRES_IMAGE_LEFT_PARENTHESIS,
eRES_IMAGE_MACT8K,
eRES_IMAGE_MAMV,
eRES_IMAGE_MBIT0,
eRES_IMAGE_MBIT1,
eRES_IMAGE_MBIT2,
eRES_IMAGE_MBIT3,
eRES_IMAGE_MBIT4,
eRES_IMAGE_MBIT5,
eRES_IMAGE_MBIT6,
eRES_IMAGE_MBIT7,
eRES_IMAGE_MBIT8,
eRES_IMAGE_MBIT9,
eRES_IMAGE_MBIT910,
eRES_IMAGE_MEMORYM,
eRES_IMAGE_MENUARR3,
eRES_IMAGE_MESSAGE,
eRES_IMAGE_MLRCBCK,
eRES_IMAGE_MNUM0,
eRES_IMAGE_MNUM1,
eRES_IMAGE_MNUM2,
eRES_IMAGE_MNUM3,
eRES_IMAGE_MNUM4,
eRES_IMAGE_MNUM5,
eRES_IMAGE_MNUM6,
eRES_IMAGE_MNUM7,
eRES_IMAGE_MNUM8,
eRES_IMAGE_MNUM9,
eRES_IMAGE_MNUM910,
eRES_IMAGE_MP332K,
eRES_IMAGE_MP332K_P,
eRES_IMAGE_MP340K,
eRES_IMAGE_MP340K_P,
eRES_IMAGE_MUSICLIST,
eRES_IMAGE_MUSICLIST_FOCOUS,
eRES_IMAGE_MVOL,
eRES_IMAGE_MWAV32K,
eRES_IMAGE_NAVGATEBAR,
eRES_IMAGE_NO_FILE,
eRES_IMAGE_PAUSE_BK,
eRES_IMAGE_POWEROFF01,
eRES_IMAGE_POWEROFF02,
eRES_IMAGE_POWEROFF03,
eRES_IMAGE_POWEROFF04,
eRES_IMAGE_POWEROFF05,
eRES_IMAGE_POWEROFF06,
eRES_IMAGE_POWEROFF07,
eRES_IMAGE_POWEROFF08,
eRES_IMAGE_POWEROFF09,
eRES_IMAGE_POWEROFF10,
eRES_IMAGE_POWEROFF11,
eRES_IMAGE_POWEROFF12,
eRES_IMAGE_POWEROFF13,
eRES_IMAGE_POWEROFF14,
eRES_IMAGE_POWERON01,
eRES_IMAGE_POWERON02,
eRES_IMAGE_POWERON03,
eRES_IMAGE_POWERON04,
eRES_IMAGE_POWERON05,
eRES_IMAGE_POWERON06,
eRES_IMAGE_POWERON07,
eRES_IMAGE_POWERON08,
eRES_IMAGE_POWERON09,
eRES_IMAGE_PUB_BATTERY_FULL,
eRES_IMAGE_PUB_BATTERY_NONE,
eRES_IMAGE_PUB_BATTERY_ONE,
eRES_IMAGE_PUB_BATTERY_THREE,
eRES_IMAGE_PUB_BATTERY_TWO,
eRES_IMAGE_PUB_MUSIC_ICON,
eRES_IMAGE_PUB_TOPBAR_BK,
eRES_IMAGE_RADIO_STEREO,
eRES_IMAGE_READN,
eRES_IMAGE_READNA,
eRES_IMAGE_REC_CURTIME_0_0,
eRES_IMAGE_REC_CURTIME_0_1,
eRES_IMAGE_REC_CURTIME_0_2,
eRES_IMAGE_REC_CURTIME_0_3,
eRES_IMAGE_REC_CURTIME_0_4,
eRES_IMAGE_REC_CURTIME_0_5,
eRES_IMAGE_REC_CURTIME_0_6,
eRES_IMAGE_REC_CURTIME_0_7,
eRES_IMAGE_REC_CURTIME_0_8,
eRES_IMAGE_REC_CURTIME_0_9,
eRES_IMAGE_REC_CURTIME_1_0,
eRES_IMAGE_REC_CURTIME_1_1,
eRES_IMAGE_REC_CURTIME_1_2,
eRES_IMAGE_REC_CURTIME_1_3,
eRES_IMAGE_REC_CURTIME_1_4,
eRES_IMAGE_REC_CURTIME_1_5,
eRES_IMAGE_REC_CURTIME_1_6,
eRES_IMAGE_REC_CURTIME_1_7,
eRES_IMAGE_REC_CURTIME_1_8,
eRES_IMAGE_REC_CURTIME_1_9,
eRES_IMAGE_REC_CURTIME_2_0,
eRES_IMAGE_REC_CURTIME_2_1,
eRES_IMAGE_REC_CURTIME_2_2,
eRES_IMAGE_REC_CURTIME_2_3,
eRES_IMAGE_REC_CURTIME_2_4,
eRES_IMAGE_REC_CURTIME_2_5,
eRES_IMAGE_REC_CURTIME_2_6,
eRES_IMAGE_REC_CURTIME_2_7,
eRES_IMAGE_REC_CURTIME_2_8,
eRES_IMAGE_REC_CURTIME_2_9,
eRES_IMAGE_REC_CURTIME_3_0,
eRES_IMAGE_REC_CURTIME_3_1,
eRES_IMAGE_REC_CURTIME_3_2,
eRES_IMAGE_REC_CURTIME_3_3,
eRES_IMAGE_REC_CURTIME_3_4,
eRES_IMAGE_REC_CURTIME_3_5,
eRES_IMAGE_REC_CURTIME_3_6,
eRES_IMAGE_REC_CURTIME_3_7,
eRES_IMAGE_REC_CURTIME_3_8,
eRES_IMAGE_REC_CURTIME_3_9,
eRES_IMAGE_REC_CURTIME_4_0,
eRES_IMAGE_REC_CURTIME_4_1,
eRES_IMAGE_REC_CURTIME_4_2,
eRES_IMAGE_REC_CURTIME_4_3,
eRES_IMAGE_REC_CURTIME_4_4,
eRES_IMAGE_REC_CURTIME_4_5,
eRES_IMAGE_REC_CURTIME_4_6,
eRES_IMAGE_REC_CURTIME_4_7,
eRES_IMAGE_REC_CURTIME_4_8,
eRES_IMAGE_REC_CURTIME_4_9,
eRES_IMAGE_REC_CURTIME_5_0,
eRES_IMAGE_REC_CURTIME_5_1,
eRES_IMAGE_REC_CURTIME_5_2,
eRES_IMAGE_REC_CURTIME_5_3,
eRES_IMAGE_REC_CURTIME_5_4,
eRES_IMAGE_REC_CURTIME_5_5,
eRES_IMAGE_REC_CURTIME_5_6,
eRES_IMAGE_REC_CURTIME_5_7,
eRES_IMAGE_REC_CURTIME_5_8,
eRES_IMAGE_REC_CURTIME_5_9,
eRES_IMAGE_REC_CURTIME_910,
eRES_IMAGE_REC_DOCUMENT,
eRES_IMAGE_REC_DOCUMENT_P,
eRES_IMAGE_REC_SAVE,
eRES_IMAGE_REC_START,
eRES_IMAGE_REC_START_P,
eRES_IMAGE_REC_STOP,
eRES_IMAGE_REC_STOP_P,
eRES_IMAGE_REC_SYSTIME_0_0,
eRES_IMAGE_REC_SYSTIME_0_1,
eRES_IMAGE_REC_SYSTIME_0_2,
eRES_IMAGE_REC_SYSTIME_0_3,
eRES_IMAGE_REC_SYSTIME_0_4,
eRES_IMAGE_REC_SYSTIME_0_5,
eRES_IMAGE_REC_SYSTIME_0_6,
eRES_IMAGE_REC_SYSTIME_0_7,
eRES_IMAGE_REC_SYSTIME_0_8,
eRES_IMAGE_REC_SYSTIME_0_9,
eRES_IMAGE_REC_SYSTIME_1_0,
eRES_IMAGE_REC_SYSTIME_1_1,
eRES_IMAGE_REC_SYSTIME_1_2,
eRES_IMAGE_REC_SYSTIME_1_3,
eRES_IMAGE_REC_SYSTIME_1_4,
eRES_IMAGE_REC_SYSTIME_1_5,
eRES_IMAGE_REC_SYSTIME_1_6,
eRES_IMAGE_REC_SYSTIME_1_7,
eRES_IMAGE_REC_SYSTIME_1_8,
eRES_IMAGE_REC_SYSTIME_1_9,
eRES_IMAGE_REC_SYSTIME_2_0,
eRES_IMAGE_REC_SYSTIME_2_1,
eRES_IMAGE_REC_SYSTIME_2_2,
eRES_IMAGE_REC_SYSTIME_2_3,
eRES_IMAGE_REC_SYSTIME_2_4,
eRES_IMAGE_REC_SYSTIME_2_5,
eRES_IMAGE_REC_SYSTIME_2_6,
eRES_IMAGE_REC_SYSTIME_2_7,
eRES_IMAGE_REC_SYSTIME_2_8,
eRES_IMAGE_REC_SYSTIME_2_9,
eRES_IMAGE_REC_SYSTIME_3_0,
eRES_IMAGE_REC_SYSTIME_3_1,
eRES_IMAGE_REC_SYSTIME_3_2,
eRES_IMAGE_REC_SYSTIME_3_3,
eRES_IMAGE_REC_SYSTIME_3_4,
eRES_IMAGE_REC_SYSTIME_3_5,
eRES_IMAGE_REC_SYSTIME_3_6,
eRES_IMAGE_REC_SYSTIME_3_7,
eRES_IMAGE_REC_SYSTIME_3_8,
eRES_IMAGE_REC_SYSTIME_3_9,
eRES_IMAGE_REC_TOUCH_SAVE,
eRES_IMAGE_REC_TOUCH_SAVE_P,
eRES_IMAGE_RIGHT_PARENTHESIS,
eRES_IMAGE_SBAR,
eRES_IMAGE_SCHEDU1,
eRES_IMAGE_SCHEDU2,
eRES_IMAGE_SET_BK,
eRES_IMAGE_SSLIDER,
eRES_IMAGE_STDB_0_0,
eRES_IMAGE_STDB_0_1,
eRES_IMAGE_STDB_0_2,
eRES_IMAGE_STDB_0_3,
eRES_IMAGE_STDB_0_4,
eRES_IMAGE_STDB_0_5,
eRES_IMAGE_STDB_0_6,
eRES_IMAGE_STDB_0_7,
eRES_IMAGE_STDB_0_8,
eRES_IMAGE_STDB_0_9,
eRES_IMAGE_STDB_1_0,
eRES_IMAGE_STDB_1_1,
eRES_IMAGE_STDB_1_2,
eRES_IMAGE_STDB_1_3,
eRES_IMAGE_STDB_1_4,
eRES_IMAGE_STDB_1_5,
eRES_IMAGE_STDB_1_6,
eRES_IMAGE_STDB_1_7,
eRES_IMAGE_STDB_1_8,
eRES_IMAGE_STDB_1_9,
eRES_IMAGE_STDB_2_0,
eRES_IMAGE_STDB_2_1,
eRES_IMAGE_STDB_2_2,
eRES_IMAGE_STDB_2_3,
eRES_IMAGE_STDB_2_4,
eRES_IMAGE_STDB_2_5,
eRES_IMAGE_STDB_2_6,
eRES_IMAGE_STDB_2_7,
eRES_IMAGE_STDB_2_8,
eRES_IMAGE_STDB_2_9,
eRES_IMAGE_STDB_3_0,
eRES_IMAGE_STDB_3_1,
eRES_IMAGE_STDB_3_2,
eRES_IMAGE_STDB_3_3,
eRES_IMAGE_STDB_3_4,
eRES_IMAGE_STDB_3_5,
eRES_IMAGE_STDB_3_6,
eRES_IMAGE_STDB_3_7,
eRES_IMAGE_STDB_3_8,
eRES_IMAGE_STDB_3_9,
eRES_IMAGE_STDB_BK,
eRES_IMAGE_STDB_SYMBOL,
eRES_IMAGE_STDBTITLEL,
eRES_IMAGE_STDBTITLER,
eRES_IMAGE_SYSCLKBCK,
eRES_IMAGE_SYSTEMB,
eRES_IMAGE_SYSTIME_ADD,
eRES_IMAGE_SYSTIME_ADD_P,
eRES_IMAGE_SYSTIME_EXIT,
eRES_IMAGE_SYSTIME_EXIT_P,
eRES_IMAGE_SYSTIME_SUB,
eRES_IMAGE_SYSTIME_SUB_P,
eRES_IMAGE_SYSTIME_SURE,
eRES_IMAGE_SYSTIME_SURE_P,
eRES_IMAGE_TITLE,
eRES_IMAGE_TITLE_FOCOUS,
eRES_IMAGE_TNUM0,
eRES_IMAGE_TNUM1,
eRES_IMAGE_TNUM2,
eRES_IMAGE_TNUM3,
eRES_IMAGE_TNUM4,
eRES_IMAGE_TNUM5,
eRES_IMAGE_TNUM6,
eRES_IMAGE_TNUM7,
eRES_IMAGE_TNUM8,
eRES_IMAGE_TNUM9,
eRES_IMAGE_TNUM910,
eRES_IMAGE_TOP_CLOCK,
eRES_IMAGE_TOP_EBOOK,
eRES_IMAGE_TOP_FM,
eRES_IMAGE_TOP_GAME,
eRES_IMAGE_TOP_MOVIE,
eRES_IMAGE_TOP_MP3,
eRES_IMAGE_TOP_PHOTO,
eRES_IMAGE_TOP_RECORD,
eRES_IMAGE_TOP_SETUP,
eRES_IMAGE_TOP_TITLE,
eRES_IMAGE_TOP_VOICE,
eRES_IMAGE_VOR128K,
eRES_IMAGE_VOR128K_P,
eRES_IMAGE_VOR32K,
eRES_IMAGE_VOR32K_P,
eRES_IMAGE_WAV128K,
eRES_IMAGE_WAV129K,
eRES_IMAGE_WAV256K,
eRES_IMAGE_WAV256K_P,
eRES_IMAGE_WAV32K,
eRES_IMAGE_WAV32K_P,
eRES_IMAGE_WAV64K,
eRES_IMAGE_WAV64K_P,
eRES_IMAGE_WAV96K,
eRES_IMAGE_WAV96K_P,
eRES_IMAGE_WAVUNKNOWN,
eRES_IMAGE_WAVUNKNOWN_P,
eRES_IMAGE_NUM
}T_RES_IMAGE;
#else
typedef enum {
eRES_IMAGE_AUDABMODEA,
eRES_IMAGE_AUDABMODEAB,
eRES_IMAGE_AUDABMODEB,
eRES_IMAGE_AUDABMODEPLYAUD,
eRES_IMAGE_AUDABMODEPLYAUD1,
eRES_IMAGE_AUDABMODEPLYREC,
eRES_IMAGE_AUDABMODEREC,
eRES_IMAGE_AUDABMODE_,
eRES_IMAGE_AUDBATL0,
eRES_IMAGE_AUDBATL1,
eRES_IMAGE_AUDBATL2,
eRES_IMAGE_AUDBATL3,
eRES_IMAGE_AUDBATL4,
eRES_IMAGE_AUDCONFMENU,
eRES_IMAGE_AUDCONFTITLEICON,
eRES_IMAGE_AUDCYCNORMAL,
eRES_IMAGE_AUDCYCPATHCYC,
eRES_IMAGE_AUDCYCPATHPLAY,
eRES_IMAGE_AUDCYCPREVIEW,
eRES_IMAGE_AUDCYCRANDOM,
eRES_IMAGE_AUDCYCSINGLE,
eRES_IMAGE_AUDCYCTOTAL,
eRES_IMAGE_AUDMUSICICON,
eRES_IMAGE_AUDMUSICLYCICON,
eRES_IMAGE_AUDSPEAKER,
eRES_IMAGE_AUDSTATLYRPLAY,
eRES_IMAGE_AUDSTATPAUSE,
eRES_IMAGE_AUDSTATPLAY,
eRES_IMAGE_AUDSTATSEEKB,
eRES_IMAGE_AUDSTATSEEKF,
eRES_IMAGE_AUDSTATSTOP,
eRES_IMAGE_AUDTONE3D,
eRES_IMAGE_AUDTONECLASSIC,
eRES_IMAGE_AUDTONEDBB,
eRES_IMAGE_AUDTONEJAZZ,
eRES_IMAGE_AUDTONENORMAL,
eRES_IMAGE_AUDTONEPOP,
eRES_IMAGE_AUDTONEROCK,
eRES_IMAGE_AUDTONESOFT,
eRES_IMAGE_AUDTONEWOW,
eRES_IMAGE_AUDTYPEAPE,
eRES_IMAGE_AUDTYPEFLAC,
eRES_IMAGE_AUDTYPEMP3,
eRES_IMAGE_AUDTYPEOGG,
eRES_IMAGE_AUDTYPEWAV,
eRES_IMAGE_AUDTYPEWMA,
eRES_IMAGE_AUDUIJUMPEFF1,
eRES_IMAGE_AUDUIJUMPEFF2,
eRES_IMAGE_AUDUIJUMPEFF3,
eRES_IMAGE_AUDUIJUMPEFF4,
eRES_IMAGE_AUDUIJUMPEFF5,
eRES_IMAGE_AUDVOICEICON,
eRES_IMAGE_AUDTYPEAMR,
eRES_IMAGE_BGLIGHT,
eRES_IMAGE_CHARGERBG,
eRES_IMAGE_CONTRAST,
eRES_IMAGE_CONTRSTW,
eRES_IMAGE_DIRSA,
eRES_IMAGE_FM_PAUSE,
eRES_IMAGE_FM_PLAY,
eRES_IMAGE_FM_AB,
eRES_IMAGE_FM_FB,
eRES_IMAGE_FM_STOP,
eRES_IMAGE_FMSMALL,
eRES_IMAGE_FILEICON,
eRES_IMAGE_FOLDICON,
eRES_IMAGE_FOLDICONCLOSE,
eRES_IMAGE_HINTICON,
eRES_IMAGE_ICONTHROW,
eRES_IMAGE_MEMORY,
eRES_IMAGE_MENUICON,
eRES_IMAGE_MENUDOT,
eRES_IMAGE_MENUDOTA,
eRES_IMAGE_PBKGRD,
eRES_IMAGE_POWERTIME,
eRES_IMAGE_PWRSAVE,
eRES_IMAGE_PWRSLEEP,
eRES_IMAGE_RADIO,
eRES_IMAGE_RADIOCURSOR,
eRES_IMAGE_RADIOPROBODY,
eRES_IMAGE_RADIOPROHEAD_J,
eRES_IMAGE_RADIOPROHEAD_UA,
eRES_IMAGE_RADIOPROTAIL_J,
eRES_IMAGE_RADIOPROTAIL_UA,
eRES_IMAGE_RECMICON,
eRES_IMAGE_RECMP3,
eRES_IMAGE_REPAUTO,
eRES_IMAGE_REPMAN,
eRES_IMAGE_ROOTICON,
eRES_IMAGE_SBKFRM,
eRES_IMAGE_SBKGRD,
eRES_IMAGE_START,
eRES_IMAGE_STDBAUDIOBIG,
eRES_IMAGE_STDBAUDIOSMALL,
eRES_IMAGE_STDBAUDIOSMALLSEL,
eRES_IMAGE_STDBEBOOKBIG,
eRES_IMAGE_STDBEBOOKSMALL,
eRES_IMAGE_STDBRADIOBIG,
eRES_IMAGE_STDBRADIOSMALL,
eRES_IMAGE_STDBRADIOSMALLSEL,
eRES_IMAGE_STDBRECBIG,
eRES_IMAGE_STDBRECSMALL,
eRES_IMAGE_STDBRECSMALLSEL,
eRES_IMAGE_STDBSELECT,
eRES_IMAGE_STDBSYSSETBIG,
eRES_IMAGE_STDBSYSSETSMALL,
eRES_IMAGE_STDBSYSSETSMALLSEL,
eRES_IMAGE_STDBVOICEBIG,
eRES_IMAGE_STDBVOICESMALL,
eRES_IMAGE_STDBVOICESMALLSEL,
eRES_IMAGE_TEXTA,
eRES_IMAGE_TEXTASEL,
eRES_IMAGE_UDISKIDLE,
eRES_IMAGE_UDISKOK,
eRES_IMAGE_ALARM_FOCUS,
eRES_IMAGE_ALARM_ICON,
eRES_IMAGE_AMR5K,
eRES_IMAGE_ARROW,
eRES_IMAGE_ARROW1,
eRES_IMAGE_ARROW2,
eRES_IMAGE_BATVAL0,
eRES_IMAGE_BATVAL1,
eRES_IMAGE_BATVAL2,
eRES_IMAGE_BATVAL3,
eRES_IMAGE_BATVALFULL,
eRES_IMAGE_CDEL_NO,
eRES_IMAGE_CDEL_YES,
eRES_IMAGE_CLOCK1,
eRES_IMAGE_CLOCK2,
eRES_IMAGE_CLOCK3,
eRES_IMAGE_CLOCK4,
eRES_IMAGE_COMPROGBK,
eRES_IMAGE_CONTRSTB,
eRES_IMAGE_DIRS,
eRES_IMAGE_DWNARROW,
eRES_IMAGE_FILES,
eRES_IMAGE_FILESA,
eRES_IMAGE_FMSCALECOM0,
eRES_IMAGE_FMSCALEJAP0,
eRES_IMAGE_MEMBAR,
eRES_IMAGE_MP332K,
eRES_IMAGE_MP340K,
eRES_IMAGE_MRFM,
eRES_IMAGE_POWEROFF,
eRES_IMAGE_POWEROFF01,
eRES_IMAGE_POWEROFF02,
eRES_IMAGE_POWEROFF03,
eRES_IMAGE_POWEROFF04,
eRES_IMAGE_POWEROFF05,
eRES_IMAGE_POWEROFF06,
eRES_IMAGE_POWEROFF07,
eRES_IMAGE_POWEROFF08,
eRES_IMAGE_POWEROFF09,
eRES_IMAGE_POWEROFF10,
eRES_IMAGE_POWEROFF11,
eRES_IMAGE_POWEROFF12,
eRES_IMAGE_POWEROFF13,
eRES_IMAGE_POWEROFF14,
eRES_IMAGE_POWERON01,
eRES_IMAGE_POWERON02,
eRES_IMAGE_POWERON03,
eRES_IMAGE_POWERON04,
eRES_IMAGE_POWERON05,
eRES_IMAGE_POWERON06,
eRES_IMAGE_POWERON07,
eRES_IMAGE_POWERON08,
eRES_IMAGE_POWERON09,
eRES_IMAGE_POWERON10,
eRES_IMAGE_POWERON11,
eRES_IMAGE_POWERON12,
eRES_IMAGE_POWERON13,
eRES_IMAGE_POWERON14,
eRES_IMAGE_POWERON15,
eRES_IMAGE_POWERON16,
eRES_IMAGE_POWERON17,
eRES_IMAGE_POWERTIMEBG,
eRES_IMAGE_RADIO_STEREO,
eRES_IMAGE_READN,
eRES_IMAGE_READNA,
eRES_IMAGE_SBAR,
eRES_IMAGE_SITEM1,
eRES_IMAGE_SITEM2,
eRES_IMAGE_SITEM3,
eRES_IMAGE_SITEM4,
eRES_IMAGE_SITEM5,
eRES_IMAGE_SITEM6,
eRES_IMAGE_SITEM7,
eRES_IMAGE_SITEM8,
eRES_IMAGE_SITEM9,
eRES_IMAGE_SITEMA,
eRES_IMAGE_SITEMB,
eRES_IMAGE_SSLIDER,
eRES_IMAGE_VERSIONICON,
eRES_IMAGE_VOLUMEBK,
eRES_IMAGE_VOR128K,
eRES_IMAGE_VOR32K,
eRES_IMAGE_WAV128K,
eRES_IMAGE_WAV129K,
eRES_IMAGE_WAV256K,
eRES_IMAGE_WAV32K,
eRES_IMAGE_WAV64K,
eRES_IMAGE_WAV96K,
eRES_IMAGE_WAVUNKNOWN,
eRES_IMAGE_NUM
}T_RES_IMAGE;
#endif
#endif
