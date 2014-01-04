/**
 * @file Gbl_Global.h
 * @brief This header file is for global data
 *
 */

#ifndef __GBL_GLOBAL_H__
#define __GBL_GLOBAL_H__

//#include "Gbl_Define.h"
#include <string.h>

#ifdef OS_WIN32
    #pragma warning(disable:4214)
    #pragma warning(disable:4100)
    #pragma warning(disable:4505)
    #pragma warning(disable:4098)

    #include "stdio.h"
    #include "math.h"
    #define _ram
    #define __ram
#else
    
    #define _ram
    #define __ram
#endif


/* lcd type list table
* 0: 128*64 for BW lcd, 128*160 for color lcd
* 1: 128*32 for BW lcd, 172*220 for color lcd
* 2: 96*40 for BW lcd, 128*128 for color lcd
**/
#define LCD_TYPE  0

#if(NO_DISPLAY == 1)

#undef USE_COLOR_LCD
#define USE_COLOR_LCD 0

#undef LCD_HORIZONTAL
#define LCD_HORIZONTAL 0


#undef DEF_RADIO  
#define DEF_RADIO  RADIO_NOFM

#endif

#include "anyka_types.h"
#include "vme.h"
//#include "unicode.h"
#include "Eng_Time.h"
#include "Fwl_detect.h"
#include "Fwl_osFS.h"
#include "log_record.h"
#include "log_radio_core.h"
#include "Gbl_Resource.h"

#define IS_BAT_LOW() ((BATTERY_STAT_LOW_SHUTDOWN == gb.batStat) && (!Fwl_DetectorGetStatus(DEVICE_CHG)))
#define IS_BAT_FULL() (BATTERY_STAT_EXCEEDVOLT == gb.batStat)


//��Ϊ1,����u�̣�A,B�̾��ɼ�����Ϊ0,����u�̣�ֻ��B�̿ɼ���                          
#define     MOUNT_ALL_DRIVER    0   

#if(NO_DISPLAY == 0)
#define USE_ALARM_CLOCK         1//wether use alarm clcok
#define SUPPORT_CALENDAR
#else
#define USE_ALARM_CLOCK     0
#endif

/* define maximum string buffer length (bytes) */
#define EBK_ZONE_MAX_COUNT  850

#define MAX_CLK_COUNT           6

/* define the storage type */
#define SPI_FLASH       0
#define SD_CARD         1
#define NAND_FLASH      2

/*system state*/
#define SYSTEM_STATE_INIT       0
#define SYSTEM_STATE_NORMAL     1
#define SYSTEM_STATE_POWEROFF   2
#define BETWEAEN_WITH(x, LOW, HIGH) { if(x < LOW) {x= LOW ; AK_DEBUG_OUTPUT("Error: %d < LOW:%d\n", x, LOW); }  \
else if( x> HIGH) { x= HIGH; AK_DEBUG_OUTPUT("Error: %d > HIGH:%d\n", HIGH);} }

#define POWEROFF_NORMAL        0
#define POWEROFF_ABNORMAL      1

typedef   struct{
    T_U32   AutoScrollTimer;

    T_U8    AutoScrollSwitch;
    T_U8    AutoTimeInterval;
}AUTOSCROLL;


typedef struct {
    T_RES_LANGUAGE      Lang;                       //current selected language.
    T_TIMER             s_public_timer_id;
    T_U32               TickMax;
    T_U32               TickMin;
    T_U8                batStat;
    T_U8                BgLightTime;
    T_U16               PoffTime;
    T_U16               PoffTimeSleepMode;
    T_U8                Lcd_Lock;
    T_U8                init;
    T_BOOL              usbstate;   /* usb and charger state */
    #ifdef SUPPORT_BLUETOOTH
    T_BOOL              chgStat;    //���״̬�����ڵ���Ƿ����ʱ�ж�
    #endif
    T_BOOL              bFatError;
volatile T_BOOL         power_on;
} T_GLOBAL;


typedef struct {
    T_RES_LANGUAGE      Lang;               // Current system language
    T_U8                LcdContrast;        // contrast of LCD: 0 -- 0x20
    T_U8                BgLightTime;
    T_U8                PowerOffFlag;       // Used to record power off action
    T_RES_STRING        CheckCode;          // Used to validate the config.
    T_U8                ConnectMode;
    T_U16               PoffTime;
    T_U16               PoffTimeSleepMode;
    T_U32               Dummy;
}T_SYSTEM_CFG;


typedef struct {
    T_USTR_FILE         defaultPath;
    T_USTR_FILE         radioDefaultPath;
    T_eREC_MODE         recMode;
    T_eREC_MODE         radioRecMode;
    T_U8                CheckCode;          // Used to validate the config.
    T_BOOL              isVorRec;           // whether is voice control record
    T_U32 Dummy;
}T_RECORD_CFG;

typedef enum
{
    EB_ANSI = 0,
    EB_UTF8,
    EB_UTF16,
    EB_ERROR
}EB_ENCODETYPE;

typedef struct {
    T_U32       CurPageOffset;
    T_U32       ZonOffset[EBK_ZONE_MAX_COUNT];
    T_U16       FilePath[256+4];
    T_U32       filelen;
    EB_ENCODETYPE encode;
    AUTOSCROLL  AutScrSet;
    T_U16       CurZon;
    T_U8        CheckCode;          // Used to validate the config.
    T_U8        Reserved;
}T_EBOOK_CFG;

/** play config parameters */
typedef struct {
    T_U16   musicClassIdx;  /* current class idx        */
    T_U16   musicCurIdx;    /* music current idx        */
    T_U32   musicTolIdx;    /* music total idx          */
    T_U32   curTimeMedia;   /* current time of media    */
    T_U8    toneMode;       /* music tone and repeat mode*/
    T_U8    repMode;        /* repeat mode              */
    T_U8    abMode;         /* A B mode                 */
    T_U8    speed;          /* play speed               */
    T_U8    volume;         /* play volume              */
    T_U8    abSpac;         /* space time for AB mode   */
    T_U8    abRep;          /* repeat time for AB mode  */
    T_U8    cycMode;        /* cycric play mode         */
    T_U8    listUpdated;    /* need to update list      */
    T_U8    abAutoRepMode;  /* AB auto repeat mode      */
    T_U8    CheckCode;      // Used to validate the config.
    T_U8    Reserved;
    T_U8    ListStyle;
    T_U8	seeklenId;
} T_AUDIO_CFG, T_VIDIO_CFG;

 

#if (USE_COLOR_LCD)
typedef struct
{
    //nes global var
    T_U16   file[MAX_FILE_LEN];
    T_BOOL  isFileChange;
    T_U8    CheckCode;
    T_U32   Dummy;
}T_GAMENES_CFG;

#endif
typedef struct{
    T_U32   systime;
    T_U8         CheckCode;
}T_SYSTIME_CFG;

#if(USE_ALARM_CLOCK)
typedef enum
{   
    ONLYONCE,
    EVERYDAY,
    WEEKDAYS
    //TIMER     //��ʱ��
}T_eResponseType;

typedef enum
{ 
    NO_CLOCK,           //������
    NORMAL_CLOCK ,      //��ͨ����
    POWEROFF_CLOCK      //��ʱ�ػ�
}T_eClockType;

typedef struct
{
     struct ClockSet
    {
        T_U8        sleepDuaration; //̰˯ʱ�䣬�Է����㣬��ʱ�ػ���������
        T_U8        clockCount;     //�����������ʱ�ػ��㶨Ϊ1
        T_U8        clockDuaration; //һ�����������ʱ�䣬����Ƕ�ʱ�ػ������Ǹ��û���ʱ��ʱ�䣬����������ʱ�����û�û��ȡ���ػ����ػ����Զ��ػ��㶨Ϊ5����
        T_U8        clockVolume;    //��������
        T_U8        weekMap;        //���ڵ��λ���ÿ�����壬weekMapΪ0xff,���ڹ��������壬��������ӦδΪ1������Ϊ0
        T_U8        isEnable;       //�Ƿ�û�б�����

        T_eResponseType  responseType;// ������Ӧ��ʽ
        T_eClockType  clockType;    //��������
        T_SYSTIME   clockTime;

        T_U16  clockMusicName[MAX_FILE_LEN+1];  //������������֧��ϵͳ���ŵ����ָ�ʽ
    } clockSet;

    struct ClockState
    {
        T_U8        recCount;//�������������ʱ�ػ���������
        T_SYSTIME   lastClockTime;  //��һ������ʱ��    
    }clockState;
}T_ClockInfo;

typedef struct ClockState T_ClockState;
typedef struct ClockSet   T_ClockSet;

typedef struct
{
   T_U8       curClock;//��ǰ������±�
}T_ClockGblState;


typedef struct
{
T_U8             checkCode;
T_ClockGblState  clockGblState;
T_ClockInfo sysClock[MAX_CLK_COUNT];// ensure the vlalid clock is Serial
T_U32 Dummy;
}T_Clocks, T_ALARMCLOCK_CFG;
#endif


typedef enum
{
	eCFG_SYSTEM = 0,
	eCFG_SYSTIME,
	eCFG_REC,
	eCFG_AUDIO,
	eCFG_SDAUDIO,
	eCFG_USBAUDIO,
	eCFG_RADIO,
	eCFG_VOICE,
	eCFG_SDVOICE,
	#ifdef SUPPORT_BLUETOOTH
	eCFG_BTDEV,
	#endif
	// ... ...
	eCFG_NUM,
}T_eCFG_ITEM;


#if (USE_COLOR_LCD)
#define COMMON_BUFFER_SIZE      1024  + FONT_BYTE_COUNT
#else
#define COMMON_BUFFER_SIZE      1024 
#endif
extern T_GLOBAL gb;

#define ALIGN(addr, align)          (((addr)+((align)-1))&~((align)-1)) 
#define ALIGN_DOWN(addr, align)     ((addr)&~((align)-1))   


#define SCREEN_STATE_ON         0
#define SCREEN_STATE_OFF        1



#define RETURN_FORM_ABPALYER      0x1231

#define GetCurLangCodePage()    (GetLangCodePage(gb.Lang))
#define GetResStringPtr(ResID)  ((T_U16 *)gb_StrResource[gb.Lang][ResID])
#define GetResStrNum()          (eRES_STR_NUM)
#define GetResStrMaxLen()       (MAX_RESSTRING_LEN)
#define Gbl_GetCommBuff()       ((T_U8*)g_CommBuff)
#define Gbl_GetLcdType()        (LCD_TYPE)
T_TIMER GetPublicTimerID(T_VOID);
//#define GetGblInitStat()      (gb.init)
#define Gbl_GetRecordSta()      ((T_BOOL)gb_sysconfig.system_param.record)


#endif/* GBL_GLOBAL_H */

