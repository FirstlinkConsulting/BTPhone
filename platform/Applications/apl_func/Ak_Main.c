/**
 * @file
 * @brief ANYKA software
 * this file is the entry of VME platform, init the hardware
 *
 * @author Pengyu Xue
 * @date    2003-04-18
 * @author
 */
#include "Gbl_Global.h"
#include "Apl_Public.h"
#include "Eng_Profile.h"
#include "Fwl_Timer.h"
#include "Fwl_System.h"
#include "m_state.h"
#include "Eng_Font.h"
#include "Fwl_osFS.h"
#include "Fwl_RTC.h"
#include "Eng_DataConvert.h"
#include "Eng_ImageResDisp.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Keypad.h"
#include "Fwl_Radio.h"
#include "eng_usb.h"
#include "gbl_ImageRes.h"
#include "AlarmClock.h"
#include "Eng_AutoOff.h"
#include "Eng_loadMem.h"
#include "Eng_Debug.h"
#include "Fwl_LCD.h"
#include "Fwl_led.h"
#include "Fwl_Detect.h"
#include "Eng_VoiceTip.h"
#include "sm_port.h"


#define PUB_TIMER_SECONDS       1

#define AK_VERSION_SOFTWARE                 "V1.0.09"
#define AK_VERSION_HARDWARE                 "AK1052C_V1"
#define AK_VERSION_HARDWARE_DATE            "20130524/20130524"
#define AK_VERSION_MEDIA                    MEDIA_LIB_VERSION
#define AK_VERSION_IMAGE                    "Image V0.4.2"

#ifdef OS_WIN32
const T_U16 defsystem[] = {'A',':','/','S','Y','S','T','E','M','\0'};
const T_U16 defaudio[] =  {'A',':','/','A','U','D','I','O','\0'};
const T_U16 defrecord[] = {'A',':','/','R','E','C','O','R','D','\0'};
const T_U16 defebook[] =  {'A',':','/','E','B','O','O','K','\0'};
#if (USE_COLOR_LCD)
const T_U16 defimage[] =  {'A',':','/','I','M','A','G','E','\0'};
const T_U16 defvideo[] =  {'A',':','/','V','I','D','E','O','\0'};
#endif

#ifdef SUPPORT_SDCARD
const T_U16 defaudio_sd[] =  {'B',':','/','A','U','D','I','O','\0'};
const T_U16 defrecord_sd[] = {'B',':','/','R','E','C','O','R','D','\0'};
const T_U16 defebook_sd[] =  {'B',':','/','E','B','O','O','K','\0'};
    #if (USE_COLOR_LCD)
    const T_U16 defimage_sd[] =  {'B',':','/','I','M','A','G','E','\0'};
    const T_U16 defvideo_sd[] =  {'B',':','/','V','I','D','E','O','\0'};
    #endif
#endif

#else   

const T_U16 defsystem[] = {'A',':','/','S','Y','S','T','E','M','\0'};

#if (STORAGE_USED == NAND_FLASH)
    const T_U16 defaudio[] =  {'B',':','/','A','U','D','I','O','\0'};
    const T_U16 defrecord[] = {'B',':','/','R','E','C','O','R','D','\0'};
    const T_U16 defebook[] =  {'B',':','/','E','B','O','O','K','\0'};
    #if (USE_COLOR_LCD)
        const T_U16 defimage[] =  {'B',':','/','I','M','A','G','E','\0'};
        const T_U16 defvideo[] =  {'B',':','/','V','I','D','E','O','\0'};
    #endif
    #ifdef SUPPORT_SDCARD
        const T_U16 defaudio_sd[] =  {'C',':','/','A','U','D','I','O','\0'};
        const T_U16 defrecord_sd[] = {'C',':','/','R','E','C','O','R','D','\0'};
        const T_U16 defebook_sd[] =  {'C',':','/','E','B','O','O','K','\0'};
        #if (USE_COLOR_LCD)
            const T_U16 defimage_sd[] =  {'C',':','/','I','M','A','G','E','\0'};
            const T_U16 defvideo_sd[] =  {'C',':','/','V','I','D','E','O','\0'};
        #endif
    #endif
#else
#if (STORAGE_USED == SD_CARD)
    #if (NO_DISPLAY == 1)
        const T_U16 defaudio_sd[] =  {'A',':','/','A','U','D','I','O','\0'};
        const T_U16 defrecord_sd[] = {'A',':','/','R','E','C','O','R','D','\0'};
    #endif
#else //spi
    #if (NO_DISPLAY == 1)
        const T_U16 defaudio_sd[] =  {'A',':','/','A','U','D','I','O','\0'};
        const T_U16 defrecord_sd[] = {'A',':','/','R','E','C','O','R','D','\0'};
    #endif
#endif
    #ifdef SUPPORT_MUSIC_PLAY
        const T_U16 defaudio[] =  {'A',':','/','A','U','D','I','O','\0'};
    #endif
    #ifdef SUPPORT_AUDIO_RECORD 
        const T_U16 defrecord[] = {'A',':','/','R','E','C','O','R','D','\0'};
    #endif
#endif

#endif


#ifdef IC_PACKAGE_BUCK
#define PD_VOL                  25
#define AVDD                    33
#define AVDD_OFFSET             0
#else
#define PD_VOL                  0
#define AVDD                    15
#define AVDD_OFFSET             0
#endif

#pragma arm section rodata = "_sys_config_"
const T_CONFIG_INFO gb_sysconfig = {AK_VERSION_SOFTWARE, __DATE__, __TIME__, 
    {'M', 'P', '4', '\0'}, 
    {'M', 'P', '4', 'P', 'l', 'a', 'y', 'e', 'r','\0'},

#if (USE_COLOR_LCD)
    {6, 9, 30, 0, 0/*CAP_100*/, DEF_RADIO, 1, 0, 1, 1, 1, 1, 0, 0, 0, PD_VOL, 1, 0, AVDD, AVDD_OFFSET},
#else
    {10, 9, 0, 0, 0/*CAP_100*/, DEF_RADIO, 1, 0, 1, 1, 1, 1, 0, 0, 0,PD_VOL, 1, 0, AVDD, AVDD_OFFSET},
#endif
    {9, kbNULL, kbNULL, kbNULL, kbNULL, kbNULL, kbOK, kbRIGHT, kbMODE, kbLEFT, kbVOLADD, kbNULL, kbVOLSUB, kbNULL, kbNULL, kbNULL},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    0x5a5a5a5a};
#pragma arm section

#pragma arm section zidata = "_bootbss1_"
T_GLOBAL    gb;     //global temp variable
#pragma arm section zidata
#pragma arm section zidata = "_imageHeader_"
T_U8    g_ImageResBuf[eRES_IMAGE_NUM*sizeof(T_IMAGERES_HEADER)] = {0}; 
#pragma arm section zidata

#ifdef CHECK_STARTUPTICK
extern  T_U32 g_StartupTick;
#endif

#if (USE_COLOR_LCD)
#if (LCD_TYPE == 3)
#if (1 == LCD_HORIZONTAL)
#define START_PIC_X     139
#define START_PIC_Y     83
#else
#define START_PIC_X     100
#define START_PIC_Y     120
#endif
#else
#define START_PIC_X     ((GRAPH_WIDTH-21)>>1)
#define START_PIC_Y     ((GRAPH_HEIGHT-37)>>1)
#endif

#else
#define START_PIC_X     ((GRAPH_WIDTH-55)>>1)
#define START_PIC_Y     (((GRAPH_HEIGHT-32)>>1) & ~0X7)
#endif


extern T_VOID stdb_reset_return_stack(T_VOID);

static T_VOID ak_mkdir(T_VOID)
{
    #if(USERDATA_ON_DISK)
    if(AK_FALSE == Fwl_FsMkDir(defsystem))
    {
        AK_DEBUG_OUTPUT("it fail to create system folder!\n");
    }
    #endif
    
    #ifdef SUPPORT_MUSIC_PLAY
    if(AK_FALSE == Fwl_FsMkDir(defaudio))
    {
        AK_DEBUG_OUTPUT("it fail to create audio folder!\n");
    }
    #endif

    #ifdef SUPPORT_AUDIO_RECORD
    if(AK_FALSE == Fwl_FsMkDir(defrecord))
    {
        AK_DEBUG_OUTPUT("it fail to create record folder!\n");
    }
    #endif

    #if(NO_DISPLAY == 0)
    if(AK_FALSE == Fwl_FsMkDir(defebook))
    {
        AK_DEBUG_OUTPUT("it fail to create ebook folder!\n");
    } 
    #endif
    

#if (USE_COLOR_LCD)
    Fwl_FsMkDir(defimage);
    Fwl_FsMkDir(defvideo);
#endif 

#ifdef OS_WIN32                 //非默认的存储介质不应生成系统文件夹，所以用OS_WIN32隔开（有必要也可以去掉）
#if(STORAGE_USED == NAND_FLASH) //非nand版本默认会在第一个盘创建文件，这里多此一举
    #ifdef SUPPORT_SDCARD
        Fwl_FsMkDir(defaudio_sd);
        Fwl_FsMkDir(defrecord_sd);
        Fwl_FsMkDir(defebook_sd);
        #if (USE_COLOR_LCD)
            Fwl_FsMkDir(defimage_sd);
            Fwl_FsMkDir(defvideo_sd);
        #endif
    #endif
#endif
#endif
}

T_VOID VME_Main(T_VOID)
{
    T_SYSTEM_CFG syscfg;
    T_SYSTIME  date;

    InitVariable();
	#ifdef OS_WIN32
	Fwl_ConsoleInit();
	#endif
    AK_DEBUG_OUTPUT("VME_Main\n");
    Fwl_FreqMgrInit();

    Fwl_DetectorInit(DEVICE_SD | DEVICE_USB | DEVICE_HP | DEVICE_LINEIN | DEVICE_UHOST);

    Eng_SetPowerDownVol(gb_sysconfig.system_param.powerdown_voltage);
    
    Fwl_TimerInit();
#if (USE_ALARM_CLOCK)
    Fwl_RTCInit(AlmClk_InterruptCallBack);
#else
    Fwl_RTCInit(AK_NULL);
#endif
    Fwl_KeypadInit();

#ifdef OS_ANYKA
    LoadMem_Init();
#endif
    
#if(NO_DISPLAY == 0)
    Fwl_DisplayInit();
    Fwl_SetContrast((T_U8)(SYS_LCD_CONTRAST)); // set contrast
    //初始化加载内存管理

    if (!Eng_ImageResInit(g_ImageResBuf,eRES_IMAGE_NUM))
    {
        AK_DEBUG_OUTPUT("image res init  error!\n");
        while(1);
    }
    Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
    Eng_ImageResDisp((T_POS)START_PIC_X, (T_POS)START_PIC_Y, eRES_IMAGE_START, AK_FALSE);
    Fwl_LCD_on(AK_TRUE);
#endif

    if (Fwl_FsInit() != 0)
    {
        AK_DEBUG_OUTPUT("fs init error!\n");
        while(1);
    }

    AK_DEBUG_OUTPUT("MiddleWare Lib version: %s!\n", (char *)MiddleWareLib_GetVersion());

    //load the config data 
    Profile_ReadData(eCFG_SYSTEM, (T_U8 *)&syscfg);
    
    if(POWEROFF_ABNORMAL == syscfg.PowerOffFlag)    //非正常掉电需要调用FS_ChkDsk
    {
        AK_DEBUG_OUTPUT("Fwl_FsChkDsk\n");
        //Fwl_FsChkDsk();
    }
    else    //正常掉电，置标志
    {
        syscfg.PowerOffFlag = POWEROFF_ABNORMAL;
        Profile_WriteData(eCFG_SYSTEM, (T_U8 *)&syscfg);
        // FlushUserdata();
    }
    
    Fwl_SetContrast((T_U8)(syscfg.LcdContrast)); // set contrast
    gb.Lang = syscfg.Lang;
    gb.PoffTime = syscfg.PoffTime;
    gb.BgLightTime = syscfg.BgLightTime;
    gb.PoffTimeSleepMode = syscfg.PoffTimeSleepMode;
    SetMinDate(date);
    gb.TickMin= Utl_Convert_DateToSecond(&date);
    SetMaxDate(date);
    gb.TickMax= Utl_Convert_DateToSecond(&date);

    if (!FontLib_Init())//init font disp
    {
        AK_DEBUG_OUTPUT("font init error!\n");
        while(1);       
    }
    #if(NO_DISPLAY == 0)
    if (!CodePage_init(GetLangCodePage(gb.Lang)))
    {
        AK_DEBUG_OUTPUT("font init codepage error!\n");
    }
	#else

	#ifdef SUPPORT_UNIC
    if (!CodePage_init(CP_936))
    {
        AK_DEBUG_OUTPUT("no display font init codepage error!\n");
    }
	#endif
    
    #endif
    
    //must be after the codepade init
    //create the default directory
    ak_mkdir();
  
#if(USE_ALARM_CLOCK)
    GblReadCmpSysTime();
    AlmClk_Init();
#endif
    GblReadCmpSysTime();
    m_initStateHandler();
    AK_DEBUG_OUTPUT("SMCORE lib version: %s!\n", SM_GetVersion());

    stdb_reset_return_stack();

    gb.power_on = AK_TRUE;

    AK_DEBUG_OUTPUT("Init OK!\n"); 
}


#ifdef OS_ANYKA
extern T_BOOL asa_get_Serialno(T_U8* pData);

T_VOID PrintFirmWareInfo(T_VOID)
{
    //version
    AK_DEBUG_OUTPUT("\r\n######FirmWare information#########\r\n");
    AK_DEBUG_OUTPUT("##Sw Version: %s\r\n",gb_sysconfig.fireware_version);
    AK_DEBUG_OUTPUT("##Build Date: %s\r\n",gb_sysconfig.release_date);
    AK_DEBUG_OUTPUT("##Build Time: %s\r\n",gb_sysconfig.release_time);
#ifdef USE_CHIP_TYPE
    AK_DEBUG_OUTPUT("##Chip Type : AK%d\r\n", USE_CHIP_TYPE);
#endif
#ifdef USED_INTERNAL_LDO_MODE
    AK_DEBUG_OUTPUT("##Power mode: internal LDO\r\n");
#else
    AK_DEBUG_OUTPUT("##Power mode: internal DCDC\r\n");
#endif
    AK_DEBUG_OUTPUT("###################################\r\n\r\n");
}

T_VOID PrintSerialNo()
{
#if(STORAGE_USED == NAND_FLASH)
    T_U8 serialNo[8];
    memset(serialNo, 0, 8);
    if(asa_get_Serialno(serialNo))
    {
        T_U8 i;
        AK_DEBUG_OUTPUT("Serial No:");
        for(i= 0; i< 8; i++)
        {
            AK_DEBUG_OUTPUT("%d", serialNo[i]);
        }
        AK_DEBUG_OUTPUT("\n");
    }
    else
    {
        AK_DEBUG_OUTPUT("***GetSerialNo Failed!\n");
    }
#endif
}

extern T_BOOL anyka_spi_update_self(T_BOOL DelUpdateFlag);
extern T_VOID HWTest(T_VOID);


T_VOID AK_Main(T_VOID)
{
    Fwl_SysPowerOn();

    VME_EvtQueueCreate();
    PrintFirmWareInfo();
    VME_Main();
    PrintSerialNo();
    
    Fwl_LEDOn(LED_RED);//开机红灯常亮
#ifdef SUPPORT_VOICE_TIP
    Voice_InitTip();
#endif

#if SUPPORT_HW_TEST
        //rda_blue_test();
        HWTest();
#endif

VME_EvtQueuePut(VME_EVT_SYSSTART, AK_NULL);

#ifdef CHECK_STARTUPTICK
    g_StartupTick += Fwl_GetTickCountUs();
    AK_DEBUG_OUTPUT("StartupTick first success: 0x%x us.\n", g_StartupTick);
#endif
#ifdef SUPPORT_UPDATA
    //SD卡自升级功能
    anyka_spi_update_self(AK_TRUE);
#endif
#if (SYS_SWITCH_MODE == 1)
	poweroff_StartCheckThread();
#endif
    VME_MainLoop();
}
#endif

T_VOID PublicTimerStart(T_VOID)
{
    PublicTimerStop();
    gb.s_public_timer_id = Fwl_TimerStartSecond(PUB_TIMER_SECONDS, AK_TRUE);
}

#if(NO_DISPLAY == 0)
#pragma arm section code = "_lyricdisplay_"
T_CODE_PAGE GetLangCodePage(T_RES_LANGUAGE lang)
{
    T_CODE_PAGE     code_page;
    
    switch (lang)
    {
    case eRES_LANG_CHINESE_SIMPLE:
    case eRES_LANG_CHINESE_TRADITION:
    case eRES_LANG_ENGLISH:
        code_page = CP_936;
        break;
    case eRES_LANG_CHINESE_BIG5:
        code_page = CP_950;
        break;
    case eRES_LANG_JAPANESE:
        code_page = CP_932;
        break;
    case eRES_LANG_KOREAN:
        code_page = CP_949;
        break;
    case eRES_LANG_FRENCH:
    case eRES_LANG_GERMAN:
    case eRES_LANG_ITALY:
    case eRES_LANG_NETHERLANDS:
        code_page = CP_28591;
        break;
    case eRES_LANG_PORTUGAL:
        code_page = CP_860;
        break;
    case eRES_LANG_SPAIN:
        code_page = CP_850;
        break;
    case eRES_LANG_SWEDISH:
    case eRES_LANG_CZECH:
    case eRES_LANG_DANISH:
    case eRES_LANG_POLISH:
        code_page = CP_28592;
        break;
    case eRES_LANG_RUSSIAN:
        code_page = CP_866;
        break;
    case eRES_LANG_TURKISH:
        code_page = CP_857;
        break;
    case eRES_LANG_HEBREW: 
        code_page = CP_1255;
        break;
    case eRES_LANG_THAI:
        code_page = CP_874;
        break;      
    default:
        code_page = CP_936;
        break;
    }
    return code_page;
}
#pragma arm section code
#endif
T_VOID PublicTimerStop(T_VOID)
{
    if (gb.s_public_timer_id != ERROR_TIMER)
    {
        Fwl_TimerStop(gb.s_public_timer_id);
        gb.s_public_timer_id = ERROR_TIMER;
    }
}

/**
 * @brief Initialize global variable
 *
 * @autor @b    Baoli.Miao
 *  Initialize SIM card phonebook storage.
 *  Init phonebook map table and group map table.
 * @author Justin.Zhao
 * @date 2008-04-14
 * @param
 * @return T_VOID
 * @retval
 */
#pragma arm section code = "_sysinit_"
T_VOID InitVariable(T_VOID)
{
    gb.init = SYSTEM_STATE_INIT;
    gb.usbstate = AK_FALSE;
    gb.bFatError = AK_FALSE;
    gb.s_public_timer_id = ERROR_TIMER;

    return;
}
#pragma arm section code


T_VOID GblReadCmpSysTime()
{
    T_SYSTIME_CFG systimeCfg;
    T_U32   rtcTime;
    T_SYSTIME systime;
    
    rtcTime= Fwl_GetSecond();

    Profile_ReadData(eCFG_SYSTIME, &systimeCfg);
    if(systimeCfg.systime > rtcTime)
    {
        Utl_Convert_SecondToDate(systimeCfg.systime, &systime);
        Fwl_SetRTCtime(&systime);
    }
}

T_VOID GblSaveSystime()
{
    T_SYSTIME_CFG systimeCfg;
    
    systimeCfg.systime= Fwl_GetSecond();

    Profile_WriteData(eCFG_SYSTIME, &systimeCfg);
}

#pragma arm section code = "_bootcode1_"
/******************************************************************************
 * @NAME    Gbl_GetCurLang
 * @BRIEF   get current lang
 * @AUTHOR  xuping
 * @DATE    2008-04-18
 * @PARAM 
 *          T_VOID
 * @RETURN codepage
******************************************************************************/ 
T_TIMER GetPublicTimerID(T_VOID)
{
    return gb.s_public_timer_id;
}

#pragma arm section code 


/* 修改卷标参考代码.
static T_BOOL SetFSVolume(T_pCSTR volume)
{
   T_PFILE file = AK_NULL;
 
    file = File_OpenAsc(AK_NULL, volume, FILE_MODE_VOLUME);
 
   if (AK_NULL == file)
   {
        return AK_FALSE;
   }
 
   File_Close(file);
   return AK_TRUE;
}

//修改卷标的调用格式.
SetFSVolume("B:/EMERSON MP3");
*/


