/**
 * @file Apl_Public.h
 * @brief This file is for include some common used header files
 * @author: Justin.zhao
 */

#ifndef __APL_PUBLIC_H__
#define __APL_PUBLIC_H__

#include "vme.h"
#include "Gbl_Global.h"
#ifdef WIN32
#include <stdlib.h>
#include <string.h>
#endif

#include "code_page.h"
#include "Eng_Graph.h"
#include "Eng_String.h"
#include "Gbl_Resource.h"
#include "Eng_Debug.h"

extern const T_U16 defaudio[];
extern const T_U16 defrecord[];
extern const T_U16 defrecord_sd[];

#if (USE_COLOR_LCD)
#define BOOT_PIC_POS_Y      0
#define POFF_PIC_POS_Y      0
#define SYS_BCKLIGHT_TIME   9
#define SYS_POFF_TIME       30
#define SYS_LCD_CONTRAST	6
#else
#define BOOT_PIC_POS_Y       0
#define POFF_PIC_POS_Y      16
#define SYS_BCKLIGHT_TIME   9
#define SYS_POFF_TIME       30
#define SYS_LCD_CONTRAST	10
#endif

#define SetMaxDate(date)    do{\
    date.year  = 2098;\
    date.month = 12;\
    date.day   = 31;\
    date.hour  = 23;\
    date.minute= 59;\
    date.second= 59;\
}while(0);

#define SetMinDate(date)    do{\
    date.year  = 2000;\
    date.month = 1;\
    date.day   = 1;\
    date.hour  = 0;\
    date.minute= 0;\
    date.second= 0;\
}while(0);

#define SetDefDate(date)    do{\
    date.year  = 2009;\
    date.month = 1;\
    date.day   = 1;\
    date.hour  = 0;\
    date.minute= 0;\
    date.second= 0;\
}while(0);

typedef enum
{
    LED_DEFAULT,              
    LED_NORMAL,
    LED_BT_RECONNECT,
    LED_BT_CONNECTED,
    LED_BT_CALLED,
    LED_NORMAL_CHARGE,
    LED_BT_RECONNECT_CHARGE,
    LED_BT_CONNECTED_CHARGE,
    LED_BT_CALLED_CHARGE,
    LED_LOWBAT, 
    LED_FULLBAT, 
    LED_USB,
    LED_START,
    LED_UPDATE,
    LED_OFF, 
	LED_BRIGHT_NUMS
}T_LED_BRIGHT_TYPE;


#define USERDATA_ON_DISK    0   //Ϊ1��user data �����ڴ���(nand or sd)�����򱣴���spi flash

#if(STORAGE_USED != SPI_FLASH)  //���ʹ�õĲ���spi flash����user dataǿ�ƴ����nand����sd��
#undef USERDATA_ON_DISK
#define USERDATA_ON_DISK    1
#endif

typedef struct tagSystemParam
{
    T_U32   contrast_degree:5;      //�Աȶ�
    T_U32   background_time:5;      //������ʱ��
    T_U32   close_time:6;           //�ػ�ʱ��
    T_U32   sleep_time:7;           //˯��ʱ��
    T_U32   capacitor:4;            //����
    T_U32   FMType:4;               //FM����
    T_U32   FM_Auto:1;              //FM�Զ�ѡ̨,1-�У�0-��
    T_U32   LCD_Driver:4;           //LCD����
    T_U32   ebook:1;                //EBOOK��1-�У�0-��
    T_U32   game:1;                 //GAME��1-�У�0-��
    T_U32   Game_Sugar:1;           //�߲��ǿ�,1-�У�0-��
    T_U32   Game_Tetris:1;          //����˹����,1-�У�0-��
    T_U32   Game3:1;                //��Ϸ����
    T_U32   Game4:1;                //��Ϸ����
    T_U32   Game5:1;                //��Ϸ����
    T_U32   powerdown_voltage:5;    //�͵�ػ���ѹ
    T_U32   record:1;               //����2
    T_U32   Reserve3:15;            //����3
    T_U8    norm_voltage;           //��׼��ѹֵ
    T_U8    norm_offset;            //��׼ƫ��ֵ
}T_SYSTEM_PARAM;

typedef struct tagLanguageSet
{
    T_U32 Language_default:5;
    T_U32 Chinese_simply:1;
    T_U32 English:1;
    T_U32 Chinese_tradition:1;
    T_U32 Chinese_big5:1;
    T_U32 Japanese:1;
    T_U32 Korean:1;
    T_U32 French:1;
    T_U32 Genman:1;
    T_U32 Italy:1;
    T_U32 Netherlands:1;
    T_U32 Poutugal:1;
    T_U32 Spain:1;
    T_U32 Swendish:1;
    T_U32 Czech:1;
    T_U32 Danish:1;
    T_U32 Polish:1;
    T_U32 Russian:1;
    T_U32 Turkish:1;
    T_U32 Hebrew:1;
    T_U32 Thai:1;
}T_LANGUAGE_SET;

typedef struct tagSystemKey
{
    T_U32   key_number:4;
    T_U32   key1:4;
    T_U32   key2:4;
    T_U32   key3:4;
    T_U32   key4:4;
    T_U32   key5:4;
    T_U32   key6:4;
    T_U32   key7:4;
    T_U32   key8:4;
    T_U32   key9:4;
    T_U32   key10:4;
    T_U32   key11:4;
    T_U32   key12:4;
    T_U32   key13:4;
    T_U32   key14:4;
    T_U32   key15:4;
}T_SYSTEM_KEY;

typedef struct tagConfigInfo
{
    T_U8    fireware_version[10];       //�̼��汾
    T_U8    release_date[20];           //��������
    T_U8    release_time[10];           //����ʱ��
    T_U8    device_id[10];              //�豸ID
    T_U8    product_id[10];             //��ƷID
    T_SYSTEM_PARAM  system_param;       //��������--8���ֽ�
    T_SYSTEM_KEY    system_key;         //��������--8���ֽ�
    T_LANGUAGE_SET  language_set;       //��������--4���ֽ�
    T_U32   check_sum;
}T_CONFIG_INFO;

T_VOID  InitVariable(T_VOID);
T_VOID  GetDefUserdata(T_U16 offset, T_VOID *pBuff);
T_VOID  GblReadCmpSysTime(T_VOID);
T_VOID  GblSaveSystime(T_VOID);

T_CODE_PAGE GetLangCodePage(T_RES_LANGUAGE lang);
T_VOID PublicTimerStart(T_VOID);
T_VOID PublicTimerStop(T_VOID);
T_VOID VME_Reset(T_VOID);
T_VOID VME_Main(T_VOID);

#endif
