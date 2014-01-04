#ifndef __AK_VME_H__
#define __AK_VME_H__

#include "anyka_types.h"

#define VME_EVT_SYSSTART    0x10
#define VME_EVT_SERIAL      0x20
#define VME_EVT_TIMER		0x30
#define VME_EVT_VIDEO       0x40
#define VME_EVT_AUDPLAY     0x50
#define VME_EVT_RECORDCTRL  0x60
#define M_EVT_RADIO_RECORD  0x70
#define M_EVT_ABPLAY_DECERR 	0x80
#define M_EVT_ABPLAY_FILEEND 	0x90
#define M_EVT_ABPLAY_REC 		0x100

#define VME_EVT_USER        0x1000

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
