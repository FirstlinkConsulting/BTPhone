/*****************************************************************************
 * Copyright (C) Anyka 2005
 *****************************************************************************
 *   Project:
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
*/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "windows.h"
#include "vme.h"
#include "w_winvme.h"
#include "Eng_Debug.h"
#include "Apl_Public.h"
#include "m_state.h"
#include "m_state_api.h"
#include "log_record.h"
#include "Fwl_System.h"
#include "Fwl_System.h"

/**
* types for vme event queue
*/
#ifdef SUPPORT_MUSIC_PLAY
extern T_VOID aud_player_read(T_VOID);
#endif
    
extern T_BOOL AudioRecord_Process(T_VOID);
extern void Blue_Process(void);

typedef struct _vmeT_Event
{
    struct _vmeT_Event * next;
    struct _vmeT_Event * prev;
    T_EVT_CODE          vmeEvtCode;             // event code
    T_EVT_PARAM       * pvmeEvtParam;           // event parameter
} vmeT_Event, *vmeT_pEvent;

typedef struct
{
    CRITICAL_SECTION    CSEventQueue;
    vmeT_pEvent         pFirst;
    vmeT_pEvent         pLast;
} T_EVENT_QUEUE;

static T_EVENT_QUEUE    s_vmeEventQueue;

static T_VOID FreeVMEEvent(vmeT_pEvent pEvent);
static vmeT_pEvent AllocVMEEvent(T_VOID);
static T_VOID FreeVMEEventParam(T_EVT_PARAM *pEventParam);
static T_EVT_PARAM *AllocVMEEventParam(T_VOID);
static T_VOID VME_EvtQueueCreate(T_VOID);
static T_VOID VME_EvtQueueDestroy(T_VOID);
static T_BOOL VME_EvtQueueGet(T_EVT_CODE *pEvtCode, T_EVT_PARAM *pEvtParam, T_BOOL *pbEvtParamExist);
DWORD WINAPI VME_EvtQueueHandleMM(LPVOID);
extern T_VOID Wave_DefaultCB(T_U8 **buf, T_U32 *len);

T_VOID akmain(T_VOID)
{
    VME_EvtQueueCreate();

    // call VME_Main () from userware
    VME_Main();

    VME_EvtQueuePut(VME_EVT_SYSSTART, AK_NULL);
#ifdef SUPPORT_VOICE_TIP
		Voice_InitTip();
#endif

    CreateThread(NULL, 0x100000, VME_EvtQueueHandleMM, NULL, 0, NULL);

    return;
}

T_VOID VME_Terminate(T_VOID)
{
    VME_EvtQueueDestroy();
    winvme_CloesAppl();
}

T_VOID VME_EvtQueuePut(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam)
{
    vmeT_pEvent    pNewEvent;

    EnterCriticalSection (&s_vmeEventQueue.CSEventQueue);

    // get memory for new event
    pNewEvent   =   AllocVMEEvent ();

    // copy event in new memory
    pNewEvent->vmeEvtCode   = EvtCode;
    if (AK_NULL != pEvtParam)
    {
        // get memory for event params
        pNewEvent->pvmeEvtParam = AllocVMEEventParam ();

        // copy event param
        *pNewEvent->pvmeEvtParam    = *pEvtParam;
    }

    // put in vme event queue
    // if first event in queue
    if (AK_NULL == s_vmeEventQueue.pLast)
    {
        s_vmeEventQueue.pLast     = pNewEvent;
        s_vmeEventQueue.pFirst    = pNewEvent;
    }
    else
    {
        // events exist in queue
        pNewEvent->prev             = s_vmeEventQueue.pLast;
        s_vmeEventQueue.pLast->next   = pNewEvent;
        s_vmeEventQueue.pLast         = pNewEvent;
    }

    LeaveCriticalSection (&s_vmeEventQueue.CSEventQueue);
    winvme_ScheduleVMEEngine();
    return;
}

T_VOID VME_EvtQueuePutTimer(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam)
{
    vmeT_pEvent    temp;
    BOOL            exist;

    exist = FALSE;
    EnterCriticalSection (&s_vmeEventQueue.CSEventQueue);

    temp = s_vmeEventQueue.pFirst;
    while (temp != AK_NULL)
    {
        if ((temp->vmeEvtCode == EvtCode) && \
                (temp->pvmeEvtParam->w.Param1 == pEvtParam->w.Param1))
        {
            exist = TRUE;
            break;
        }
        temp = temp->next;
    }
    LeaveCriticalSection (&s_vmeEventQueue.CSEventQueue);

    if (exist)
    {
        AK_DEBUG_OUTPUT("--------WARNING: VME queue lose timer: %d!\r\n", pEvtParam->w.Param1);
        return;
    }

    VME_EvtQueuePut(EvtCode, pEvtParam);
}

T_VOID VME_EvtQueuePutUnique(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam, T_BOOL mustHold)
{
    vmeT_pEvent    temp;
    BOOL            exist;

    exist = FALSE;
    EnterCriticalSection (&s_vmeEventQueue.CSEventQueue);

    temp = s_vmeEventQueue.pFirst;
    while (temp != AK_NULL)
    {
        if( temp->vmeEvtCode == EvtCode )
        {
            exist = TRUE;
            break;
        }
        temp = temp->next;
    }
    LeaveCriticalSection (&s_vmeEventQueue.CSEventQueue);

    if( exist )
    {
        return;
    }
    VME_EvtQueuePut(EvtCode, pEvtParam);
}

DWORD WINAPI VME_EvtQueueHandleMM(LPVOID lp)
{
    VME_EvtQueueHandle();
    return 0;
}

T_VOID VME_EvtQueueHandle(T_VOID)
{
    T_EVT_CODE  EvtCode;
    T_EVT_PARAM EvtParam;
    T_BOOL      EvtParamExist;

    while (1)
    {
		Blue_Process();

        #ifdef SUPPORT_MUSIC_PLAY
		aud_player_read();
		Wave_DefaultCB(AK_NULL, AK_NULL);
        #endif

        #ifdef SUPPORT_AUDIO_RECORD
        AudioRecord_Process();
        #endif
        
        Fwl_SysMsgHandle();

        if (VME_EvtQueueGet (&EvtCode, &EvtParam, &EvtParamExist))
        {
            m_mainloop(EvtCode, &EvtParam);
        }
        else
        {
            Sleep(0);
        }
    }
}

/*****************************************************************************
 * static functions
 *****************************************************************************
*/
static T_VOID FreeVMEEvent(vmeT_pEvent pEvent)
{
    assert (pEvent != AK_NULL);

    free (pEvent);
    pEvent  =   AK_NULL;

    return;
}

static vmeT_pEvent AllocVMEEvent(T_VOID)
{
    vmeT_pEvent pEvent  =   AK_NULL;

    pEvent  = malloc (sizeof (vmeT_Event));

    assert (pEvent != AK_NULL);

    pEvent->next            = AK_NULL;
    pEvent->prev            = AK_NULL;
    pEvent->vmeEvtCode      = 0;
    pEvent->pvmeEvtParam    = AK_NULL;

    return pEvent;
}

static T_VOID FreeVMEEventParam(T_EVT_PARAM *pEventParam)
{
    assert (pEventParam != AK_NULL);

    free (pEventParam);

    pEventParam = AK_NULL;

    return;
}

static T_EVT_PARAM *AllocVMEEventParam(T_VOID)
{
    T_EVT_PARAM *pEventParam;

    pEventParam = malloc (sizeof (T_EVT_PARAM));

    assert (pEventParam != AK_NULL);

    return pEventParam;
}

static T_VOID VME_EvtQueueCreate(T_VOID)
{
    s_vmeEventQueue.pFirst    =   AK_NULL;
    s_vmeEventQueue.pLast     =   AK_NULL;
    InitializeCriticalSection (&s_vmeEventQueue.CSEventQueue);
}

static T_VOID VME_EvtQueueDestroy(T_VOID)
{
    vmeT_pEvent pEvent;

    // if exists events in queue, then free memory for these
    while (AK_NULL != s_vmeEventQueue.pFirst)
    {
        pEvent                  = s_vmeEventQueue.pFirst;
        s_vmeEventQueue.pFirst    = s_vmeEventQueue.pFirst->next;

        // if exists param for event
        if (AK_NULL != pEvent->pvmeEvtParam)
        {
            FreeVMEEventParam (pEvent->pvmeEvtParam);
        }
        FreeVMEEvent (pEvent);
    }

    DeleteCriticalSection (&s_vmeEventQueue.CSEventQueue);
    return;
}

static T_BOOL VME_EvtQueueGet(T_EVT_CODE *pEvtCode, T_EVT_PARAM *pEvtParam, T_BOOL *pbEvtParamExist)
{

    T_BOOL bRet = AK_FALSE;

    if (s_vmeEventQueue.CSEventQueue.DebugInfo == AK_NULL)  /* function VME_QueueDestroy() has been called, add by ZouMai */
        return AK_FALSE;

    EnterCriticalSection (&s_vmeEventQueue.CSEventQueue);

    assert (pEvtCode != AK_NULL);
    assert (pEvtParam != AK_NULL);
    assert (pbEvtParamExist != AK_NULL);

    *pbEvtParamExist = AK_FALSE;

    // exists event in queue ?
    if (AK_NULL != s_vmeEventQueue.pFirst)
    {
        // copy event in outbuffers and free memory from queued event
        *pEvtCode    = s_vmeEventQueue.pFirst->vmeEvtCode;

        // if exists param for event
        if (AK_NULL != s_vmeEventQueue.pFirst->pvmeEvtParam)
        {
            *pEvtParam          = *s_vmeEventQueue.pFirst->pvmeEvtParam;
            *pbEvtParamExist    = AK_TRUE;

            // free memory for event param
            FreeVMEEventParam (s_vmeEventQueue.pFirst->pvmeEvtParam);
        }

        // is this element the latter?
        if (AK_NULL == s_vmeEventQueue.pFirst->next)
        {
            FreeVMEEvent (s_vmeEventQueue.pFirst);
            s_vmeEventQueue.pFirst    = AK_NULL;
            s_vmeEventQueue.pLast     = AK_NULL;
        }
        else
        {
            s_vmeEventQueue.pFirst    =   s_vmeEventQueue.pFirst->next;
            FreeVMEEvent (s_vmeEventQueue.pFirst->prev);
        }

        bRet = AK_TRUE;
    }

    LeaveCriticalSection (&s_vmeEventQueue.CSEventQueue);
    return bRet;
}

T_VOID VME_EvtQueueClear(T_VOID)
{
    T_EVT_CODE  EvtCode;
    T_EVT_PARAM EvtParam;
    T_BOOL      EvtParamExist;

    while (VME_EvtQueueGet(&EvtCode, &EvtParam, &EvtParamExist));
}

T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt)
{//只清timer消息
    T_EVT_CODE  EvtCode;
    T_EVT_CODE userKey= 0;
    T_EVT_PARAM evtParam;

    EnterCriticalSection (&s_vmeEventQueue.CSEventQueue);
    
    while(1)
    {
        if(s_vmeEventQueue.pFirst == s_vmeEventQueue.pLast)
            break;
        
        EvtCode = s_vmeEventQueue.pFirst->vmeEvtCode;

        if(EvtCode==M_EVT_PUB_TIMER || EvtCode==VME_EVT_TIMER)
        {
            if(clearEvt != 0 && EvtCode != clearEvt)
            {
                break;
            }
        }
        else if(M_EVT_USER_KEY == EvtCode)
        {
            userKey = M_EVT_USER_KEY;            
            evtParam = *s_vmeEventQueue.pFirst->pvmeEvtParam;
        }
        else
        {
            break;
        }

        s_vmeEventQueue.pFirst = s_vmeEventQueue.pFirst->next;
        //prevert overflow
        Fwl_ConsoleWriteChr('#');
        //AK_PRINTK("#:", EvtCode, 1);
    }
    
    if(userKey != 0)
    {              
        s_vmeEventQueue.pFirst= s_vmeEventQueue.pFirst->next;
        s_vmeEventQueue.pFirst->vmeEvtCode = userKey;
        s_vmeEventQueue.pFirst->pvmeEvtParam = &evtParam;        
    }

    LeaveCriticalSection (&s_vmeEventQueue.CSEventQueue);
}

T_VOID VME_EvtQueueClearTimerEvent(T_VOID)
{
    VME_EvtQueueClearTimerEventEx(0);
}



