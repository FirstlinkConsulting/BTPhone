/*******************************************************************************
 * @file    Fwl_Timer.c
 * @brief   this file will constraint the access to the bottom layer timer 
 *          avoid resource competition. Also, this file os for porting to 
 *          different OS
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @version 1.0
*******************************************************************************/
#include "Gbl_global.h"
#include "arch_timer.h"
#include "Fwl_Timer.h"
#include "vme.h"
#include "m_event.h"
#include "Eng_debug.h"



#define TIMER_NUM_MAX               6    
#define TIMER_INACTIVE              0       /* inactive(available) */
#define TIMER_ACTIVE                1       /* active(vailable) */
#define TIMER_INTERVAL              10       


typedef struct
{
    T_U8    vtimer_state:7;                 /* TIMER_INACTIVE or TIMER_ACTIVE */
    T_U8    vtimer_loop:1;                  /* loop or not */
    T_U32   vtimer_delay;
    T_U32   vtimer_time;
    T_fVTIMER_CALLBACK  callback_func;  /* callback function only for current timer */
} T_TIMER_DATA;


#pragma arm section zidata = "_bootbss1_"
static volatile T_TIMER_DATA vtimer_data[TIMER_NUM_MAX];
static volatile T_TIMER drvtimer_id = ERROR_TIMER;
#pragma arm section zidata


#pragma arm section code = "_frequentcode_"
/*******************************************************************************
 * @brief   virtual timer callback function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]timer_id: timer id
 * @param   [in]delay: timer delay
 * @return  T_VOID
*******************************************************************************/
T_VOID vtimer_callback_func(T_TIMER timer_id, T_U32 delay)
{
    T_EVT_PARAM pEventParm;

    pEventParm.w.Param1 = timer_id;
    pEventParm.w.Param2 = delay;

    if (timer_id == GetPublicTimerID())
        VME_EvtQueuePut(M_EVT_PUB_TIMER, &pEventParm);
    else
        VME_EvtQueuePut(VME_EVT_TIMER, &pEventParm);
}
#pragma arm section code


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   initialize virtual timer
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_TimerInit(T_VOID)
{
    T_U32 i;

    for (i=0; i<TIMER_NUM_MAX; i++)
    {
        vtimer_data[i].vtimer_state  = TIMER_INACTIVE;
        //vtimer_data[i].callback_func = AK_NULL;
        //vtimer_data[i].vtimer_delay  = 0;
		//vtimer_data[i].vtimer_time = 0;
    }
	drvtimer_id = timer_start(TIMER_INTERVAL, AK_TRUE, AK_NULL);
}
#pragma arm section code


/*******************************************************************************
 * @brief   start timer. When the time reach, the vtimer callback function 
 *          will be called. User must call function vtimer_stop() to free 
 *          the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function Fwl_TimerInit() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]ms: specifies the time-out value, in millisecond. 
 *                  Caution, this value must can be divided by 20.
 * @param   [in]loop: loop or not
 * @param   [in]callback_func: timer callback function. If callback_func is
 *                             not AK_NULL, then this callback function will 
 *                             be called when time reach.
 * @return  T_TIMER
 * @retval  timer handle, user can stop the timer by this handle. 
 *          if equal to ERROR_TIMER: failed
*******************************************************************************/
static T_TIMER vtimer_start(T_U16 ms, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
    T_U32 i;

    if (ms == 0 || callback_func == AK_NULL || drvtimer_id == ERROR_TIMER)
    {
        return ERROR_TIMER;
    }
    
    for (i=0; i<TIMER_NUM_MAX; i++)
    {
        if (vtimer_data[i].vtimer_state == TIMER_INACTIVE)
        {
            break;
        }
    }

    //No free timer
    if (i == TIMER_NUM_MAX)
    {
        AK_PRINTK("Error: no tm!\n",0,1);
        return ERROR_TIMER;
    }

    vtimer_data[i].vtimer_state  = TIMER_ACTIVE;
    vtimer_data[i].vtimer_loop   = loop;
    vtimer_data[i].vtimer_delay  = ms;
    vtimer_data[i].callback_func = callback_func;
	vtimer_data[i].vtimer_time = 0;

    return i;
}


/*******************************************************************************
 * @brief   stop timer
 * Function Fwl_TimerInit() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]timer_id: timer ID
 * @return  T_TIMER
 * @retval  timer handle
*******************************************************************************/
static T_TIMER vtimer_stop(T_TIMER timer_id)
{
    if (timer_id >= TIMER_NUM_MAX)
        return ERROR_TIMER;

    if (vtimer_data[timer_id].vtimer_state == TIMER_INACTIVE)
        return ERROR_TIMER;

    vtimer_data[timer_id].vtimer_state = TIMER_INACTIVE;

    return ERROR_TIMER;
}


/*******************************************************************************
 * @brief   start timer. When the time reach, the vtimer callback function 
 *          will be called. User must call function Fwl_TimerStop() to free 
 *          the timer, in spite of loop is AK_TRUE or AK_FALSE.
 * Function Fwl_TimerInit() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]ms: specifies the time-out value, in millisecond. 
 *                  Caution, this value must can be divided by 20.
 * @param   [in]loop: loop or not
 * @param   [in]callback_func: timer callback function. If callback_func is
 *                             not AK_NULL, then this callback function will 
 *                             be called when time reach.
 * @return  T_TIMER
 * @retval  timer handle, user can stop the timer by this handle. 
 *          if equal to ERROR_TIMER: failed
*******************************************************************************/
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


/*******************************************************************************
 * @brief   stop timer
 * Function Fwl_TimerInit() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]timerHandle: the timer handle
 * @return  T_TIMER
 * @retval  timer handle
*******************************************************************************/
T_TIMER Fwl_TimerStop(T_TIMER timerHandle)                      
{
    return vtimer_stop(timerHandle);
}


#pragma arm section code = "_bootcode1_"
/*******************************************************************************
 * @brief   get ms tick
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_U32
 * @retval  ms tick
 * @note    大约每隔50天会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 Fwl_GetTickCountMs(T_VOID)
{
    return get_tick_count_ms();
}


/*******************************************************************************
 * @brief   get us tick
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_U32
 * @retval  us tick
 * @note    大约每隔一个多小时会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 Fwl_GetTickCountUs(T_VOID)
{
    return get_tick_count_us();
}


/*******************************************************************************
 * @brief   delay microsecond function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]us: delay microsecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_DelayUs(T_U32 us)
{
    delay_us(us);
}

T_VOID Fwl_DelayMs(T_U32 ms)
{
    delay_ms(ms);
}

/*******************************************************************************
 * @brief   deal timer message
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]timerId: the timer id
 * @param   [in]deley: the delay time
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_TimerMsgDeal(T_TIMER timerId, T_U32 deley)
{
    T_U8 vtimer_id;

    for (vtimer_id=0; vtimer_id<TIMER_NUM_MAX; vtimer_id++)
    {
        if (TIMER_ACTIVE == vtimer_data[vtimer_id].vtimer_state)
        {
        	vtimer_data[vtimer_id].vtimer_time += TIMER_INTERVAL;
        	if(vtimer_data[vtimer_id].vtimer_time >= vtimer_data[vtimer_id].vtimer_delay)
        	{
	        	if(vtimer_data[vtimer_id].vtimer_loop == AK_FALSE)
	    		{
	    			vtimer_data[vtimer_id].vtimer_state = TIMER_INACTIVE;
	    		}
				else
				{
					vtimer_data[vtimer_id].vtimer_time = 0;
				}
            	vtimer_data[vtimer_id].callback_func(vtimer_id, vtimer_data[vtimer_id].vtimer_delay);
        	}
        }
    }
}
#pragma arm section code


