/*******************************************************************************
 * @file    Fwl_Timer.c
 * @brief   this file will constraint the access to the bottom layer timer 
 *          avoid resource competition. Also, this file os for porting to 
 *          different OS
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @version 1.0
*******************************************************************************/
#include "w_vtimer.h"
#include "w_evtmsg.h"
#include "Gbl_global.h"
#include "Fwl_Timer.h"
#include "vme.h"
#include "m_event.h"
#include "Eng_debug.h"
#include "windows.h"

T_VOID vtimer_callback_func(T_TIMER timer_id, T_U32 delay)
{
    ST_MSG   msg;

	msg.msg     = IM_TIMER;
    msg.bparam  = (T_U8)timer_id;
    msg.hwparam = (T_U16)delay;

    post_int_message(&msg);    
}

T_VOID Fwl_TimerInit(T_VOID)
{
    vtimer_init();
}

T_TIMER Fwl_TimerStart(T_U16 milliSeconds, T_BOOL loop, T_fVTIMER_CALLBACK callback)    
{
    return vtimer_start(milliSeconds, loop, callback);
}


T_TIMER Fwl_TimerStartSecond(T_U8 seconds, T_BOOL loop)
{
    return vtimer_start((T_U16)(seconds*1000), loop, vtimer_callback_func);
}


T_TIMER Fwl_TimerStartMilliSecond(T_U16 milliSeconds, T_BOOL loop)
{
    return vtimer_start(milliSeconds, loop, vtimer_callback_func);
}

T_TIMER Fwl_TimerStop(T_TIMER timerHandle)                      
{
    return vtimer_stop(timerHandle);
}

T_U32 Fwl_GetTickCountMs(T_VOID)
{
    return (GetPubTimerCnt() + 1)*1000;
}



T_U32 Fwl_GetTickCountUs(T_VOID)
{
    return 0;
}


T_VOID Fwl_DelayMs(T_U32 ms)
{ 
	Sleep(ms);

}

T_VOID Fwl_DelayUs(T_U32 us)
{
    Sleep(us/1000);
}


T_VOID Fwl_TimerMsgDeal(T_TIMER timer_id, T_U32 delay)
{    
    T_EVT_PARAM pEventParm;

    pEventParm.w.Param1 = timer_id;
    pEventParm.w.Param2 = delay;

    if (timer_id == GetPublicTimerID())
        VME_EvtQueuePut(M_EVT_PUB_TIMER, &pEventParm);
    else
        VME_EvtQueuePut(VME_EVT_TIMER, &pEventParm);
}


