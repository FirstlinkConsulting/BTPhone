#ifndef __AK_VME_H__
#define __AK_VME_H__

#include "anyka_types.h"

#define VME_EVT_SYSSTART        0x10
#define VME_EVT_SERIAL          0x20
#define VME_EVT_TIMER           0x30
#define VME_EVT_VIDEO           0x40
#define VME_EVT_AUDPLAY         0x50
#define VME_EVT_RECORDCTRL      0x60
#define M_EVT_RADIO_RECORD      0x70
#define M_EVT_ABPLAY_DECERR     0x80
#define M_EVT_ABPLAY_FILEEND    0x90
#define M_EVT_ABPLAY_REC        0x100

//bt_phone status mechine event
#define VME_EVT_BTPHONE_RING        0x200


//bt_player status mechine event
#define VME_EVT_BTPLAYER_INIT        0x210
#define VME_EVT_BTPLAYER_DEINIT        0x220
#define VME_EVT_CONNECT_SERVICE            0x230 // 指定要连接那个服务的消息
#define VME_EVT_RECONNECT_SERVICE           0x240//自动重练时
#define VME_EVT_CONNECT_A2DP            0x250 // 指定要连接那个服务的消息
#define VME_EVT_CONNECT_HFP            0x260 // 指定要连接那个服务的消息
#define VME_EVT_CONNECT_AVRCP            0x270 // 指定要连接那个服务的消息
#define VME_EVT_CONNECT_FAIL         0x280 // 指定要连接那个服务的消息
#define VME_EVT_BTPLAYER_STOP        0x290
#define VME_EVT_STOP_RING        0x300
#define VME_EVT_TRY_RECONNECT_SERVICE 0x310
#define VME_EVT_ACCEPT_PHONE   0x320
#define VME_EVT_SAVE_DEVINFO 0x330
#define VME_EVT_DISCONNECT 0x340
#define VME_EVT_AVRCP_DISCONNECT 0x350
#define VME_EVT_IPHONE_BAT_CHANGE      0x360
#define VME_EVT_BLUEA_TIMEOUT		0x370


//power change event
#define VME_EVT_POWER_CHANGE    0x400
#define VME_EVT_VOICE_TIP       0x410

#define VME_EVT_MAX_VOLUME      0X500

#define VME_EVT_USER            0x1000

typedef T_U16  T_EVT_CODE;

typedef union
{
    struct
    {
        T_U8 Param1;
        T_U8 Param2;
        T_U8 Param3;
        T_U8 Param4;
        T_U8 Param5;
        T_U8 Param6;
        T_U8 Param7;
        T_U8 Param8;
    } c;

    struct
    {
        T_U16 Param1;
        T_U16 Param2;
        T_U16 Param3;
        T_U16 Param4;
    } s;

    struct
    {
        T_U32 Param1;
        T_U32 Param2;
    } w;

    struct
    {
        T_pVOID pParam1;
        T_pVOID pParam2;
    } p;

} T_EVT_PARAM; ///< Union for event message parameters. 

typedef void (*T_fMAIN_CALLBACK)     (T_EVT_CODE event, T_EVT_PARAM *param);///< Userware main callback

#define vT_EvtCode  T_EVT_CODE
#define vT_EvtParam T_EVT_PARAM
#define vBOOL       T_BOOL
#define vTRUE       AK_TRUE
#define vFALSE      AK_FALSE
typedef signed char vINT8;


T_VOID VME_MainLoop(T_VOID);
T_VOID VME_EvtQueueCreate(T_VOID);
T_VOID VME_Terminate(T_VOID);
T_VOID VME_EvtQueuePut(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam);
T_VOID VME_EvtQueuePutTimer(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam);
T_VOID VME_EvtQueuePutUnique(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam, T_BOOL mustHold);
T_VOID VME_EvtQueueHandle(T_VOID);
T_VOID VME_EvtQueueClearTimerEvent(T_VOID);
T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt);

#endif
