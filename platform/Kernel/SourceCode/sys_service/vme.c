#include "vme.h"
#include "Fwl_System.h"
#include "m_event.h"
#include "Eng_debug.h"

#ifdef OS_ANYKA

extern T_BOOL AudioRecord_Process(T_VOID);

extern void m_mainloop(vT_EvtCode Event, vT_EvtParam *pParam);

//#ifdef USE_CPU_STANDY
T_VOID cpu_standby(T_VOID);
//#endif


#define MAX_EVENT_NUMBER    16  //should be 2^

/**
* types for vme event queue
*/
struct _evt_data
{
    T_EVT_CODE  event;
    T_EVT_PARAM param;
};

typedef struct
{
    struct _evt_data    d[MAX_EVENT_NUMBER];
    T_U16               head;
    T_U16               tail;
} T_EVENT_QUEUE;
  
#pragma arm section zidata = "_bootbss1_"
static T_EVENT_QUEUE    s_vmeEventQueue;
#pragma arm section zidata

static T_BOOL VME_EvtQueueGet(T_EVT_CODE *pEvtCode, T_EVT_PARAM *pEvtParam);

extern T_VOID aud_player_read(T_VOID);
extern void Blue_Process(void);
extern T_BOOL BtPhone_IsWorking(void);

T_VOID VME_MainLoop(T_VOID)
{
    T_EVT_CODE  EvtCode;
    T_EVT_PARAM EvtParam;
//    T_BOOL      InIdleMode = AK_FALSE;

    while (1)
    {

		Blue_Process();
		
        #ifdef SUPPORT_AUDIO_RECORD
        AudioRecord_Process();
        #endif

        Fwl_SysMsgHandle();
        
#ifdef SUPPORT_MUSIC_PLAY
        aud_player_read();
#endif 

        if (VME_EvtQueueGet(&EvtCode, &EvtParam))
        {
            m_mainloop(EvtCode, &EvtParam);
        }
//#ifdef USE_CPU_STANDY
        else
        {
			if(!BtPhone_IsWorking())
			{
            	cpu_standby();
			}
        }
//#endif

    }
}

static T_BOOL VME_CheckOverflow(T_EVT_CODE EvtCode)
{
    if ((M_EVT_PUB_TIMER == EvtCode) || 
            (M_EVT_USER_KEY == EvtCode) || 
            (M_EVT_AUDIO_CTRL_TIMER == EvtCode))
    {
        if (s_vmeEventQueue.head  <= s_vmeEventQueue.tail)
        {
            if ((s_vmeEventQueue.head + MAX_EVENT_NUMBER -s_vmeEventQueue.tail) <= 3)
            {
                AK_PRINTK("of2: ", EvtCode-VME_EVT_USER, AK_TRUE);
                return AK_TRUE;
            }
        }
        else
        {
            if ((s_vmeEventQueue.head-s_vmeEventQueue.tail) <= 3)
            {
                AK_PRINTK("of3: ", EvtCode-VME_EVT_USER, AK_TRUE);
                return AK_TRUE;
            }
        }
    }
    else
    {
        if (((s_vmeEventQueue.tail + 1) & (MAX_EVENT_NUMBER-1)) == s_vmeEventQueue.head)
        {
            AK_PRINTK("of1: ", EvtCode-VME_EVT_USER, AK_FALSE);
            return AK_TRUE;
        }
    }
    return AK_FALSE;
}

T_VOID VME_EvtQueuePut(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam)
{
    if (VME_CheckOverflow(EvtCode))
        return;

    Fwl_StoreAllInt();
    s_vmeEventQueue.d[s_vmeEventQueue.tail].event = EvtCode;
    if (pEvtParam != AK_NULL)
    {
        s_vmeEventQueue.d[s_vmeEventQueue.tail].param = *pEvtParam;
    }
    else
    {
        s_vmeEventQueue.d[s_vmeEventQueue.tail].param.w.Param1 = 0;
        s_vmeEventQueue.d[s_vmeEventQueue.tail].param.w.Param2 = 0;
    }
    s_vmeEventQueue.tail = (s_vmeEventQueue.tail + 1) & (MAX_EVENT_NUMBER-1);
    Fwl_RestoreAllInt();

    return;
}

/**************************************************************************
* @BRIEF     Default pEvtParam compare function.
* @AUTHOR 
* @DATE     2013-2-04
* @PARAM   pEvtParam1 [in]  The first pEvtParam item to be compared.
* @PARAM   pEvtParam2 [in]  The second pEvtParam item to be compared.
* @RETURN  T_BOOL  The result of the comparison.
* @RETVAL  AK_TRUE       The two items are same.
* @RETVAL  AK_FALSE      The two items are diffrent.
***************************************************************************/
static T_BOOL AK_EvtParamCmp(T_EVT_PARAM pEvtParam1, T_EVT_PARAM pEvtParam2)
{
    if (pEvtParam1.w.Param1 == pEvtParam2.w.Param1
        && pEvtParam1.w.Param2 == pEvtParam2.w.Param2)
        return AK_TRUE;
    else
        return AK_FALSE;
}

T_VOID VME_EvtQueuePutUnique(T_EVT_CODE EvtCode, T_EVT_PARAM *pEvtParam, T_BOOL mustHold)
{
    int i;

    for (i = s_vmeEventQueue.head; (i & (MAX_EVENT_NUMBER-1)) != s_vmeEventQueue.tail; i++)
    {
        if (s_vmeEventQueue.d[i & (MAX_EVENT_NUMBER-1)].event == EvtCode)
        {
            if (mustHold == AK_TRUE)
            {
                if (pEvtParam != AK_NULL
                    && AK_EvtParamCmp(s_vmeEventQueue.d[i & (MAX_EVENT_NUMBER-1)].param 
                                    , *pEvtParam))
                {
                    s_vmeEventQueue.d[i & (MAX_EVENT_NUMBER-1)].param = *pEvtParam;
                }
                else//not equal to send event
                {
                    VME_EvtQueuePut(EvtCode, pEvtParam);
                }
            }
            return;
        }
    }

    VME_EvtQueuePut(EvtCode, pEvtParam);
}

#pragma arm section code = "_sysinit_"
T_VOID VME_EvtQueueCreate(T_VOID)
{
    s_vmeEventQueue.head = 0;
    s_vmeEventQueue.tail = 0;
}
#pragma arm section code
#pragma arm section code = "_bootcode1_"

static T_BOOL VME_EvtQueueGet(T_EVT_CODE *pEvtCode, T_EVT_PARAM *pEvtParam)
{
    if (s_vmeEventQueue.head == s_vmeEventQueue.tail)
        return AK_FALSE;

    *pEvtCode = s_vmeEventQueue.d[s_vmeEventQueue.head].event;
    *pEvtParam = s_vmeEventQueue.d[s_vmeEventQueue.head].param;

    s_vmeEventQueue.head = (s_vmeEventQueue.head + 1) & (MAX_EVENT_NUMBER-1);

    return AK_TRUE;
}
#pragma arm section code 

#pragma arm section code = "_audioplayer_"
T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt)
{//只清timer消息
    T_EVT_CODE  EvtCode;
    T_EVT_CODE userKey= 0;
    T_EVT_PARAM evtParam;
    
    while(1)
    {
        if(s_vmeEventQueue.head == s_vmeEventQueue.tail)
            break;

        EvtCode = s_vmeEventQueue.d[s_vmeEventQueue.head].event;
        if(EvtCode==M_EVT_PUB_TIMER || EvtCode==VME_EVT_TIMER)
        {
            if(clearEvt != 0 && EvtCode != clearEvt)
            {
                break;
            }
        }
        else if(M_EVT_USER_KEY == EvtCode)
        {
            userKey= M_EVT_USER_KEY;
            evtParam= s_vmeEventQueue.d[s_vmeEventQueue.head].param;
        }
        else
        {
            break;
        }
        s_vmeEventQueue.head = (s_vmeEventQueue.head + 1) & (MAX_EVENT_NUMBER-1);
        //prevert overflow
        Fwl_ConsoleWriteChr('#');
        //AK_PRINTK("#:", EvtCode, 1);
    }
    
    if(userKey != 0)
    {
        Fwl_StoreAllInt();
        //when head is 0, then 0+16-1= 15
        s_vmeEventQueue.head= (s_vmeEventQueue.head + MAX_EVENT_NUMBER- 1) & (MAX_EVENT_NUMBER-1);
        s_vmeEventQueue.d[s_vmeEventQueue.head].event= userKey;
        s_vmeEventQueue.d[s_vmeEventQueue.head].param= evtParam;
        Fwl_RestoreAllInt();
    }
}

T_VOID VME_EvtQueueClearTimerEvent(T_VOID)
{
    VME_EvtQueueClearTimerEventEx(0);
}

#pragma arm section code

#endif

#ifdef OS_WIN32
T_VOID VME_EvtQueueClearTimerEvent(T_VOID){}
T_VOID VME_EvtQueueClearTimerEventEx(T_EVT_CODE clearEvt){}
#endif
