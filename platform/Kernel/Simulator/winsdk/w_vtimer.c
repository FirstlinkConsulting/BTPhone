/**
 * @file vtimer.c
 * @brief Virtual timer function header file
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-22
 * @version 1.0
 * @ref AK3210 technical manual.
 */


#include "w_vtimer.h"
#include "Eng_Time.h"
#include "w_winvme.h"
#include "wtypes.h"
#include "winbase.h"
#include "Fwl_RTC.h"

#define TIMER_NUM_MAX               8    /* 0~3 as second timer, 4~15 as millisecond timer.*/
#define TIMER_MSTIMER_START_ID      5

#define PHYSICAL_TIMER              10    /* interrupt every 10 millisecond */

#define TIMER_INACTIVE              0       /* inactive(available) */
#define TIMER_ACTIVE                1       /* active */
#define TIMER_TIMEOUT               2       /* time out */

typedef struct
{
    T_U8    state:7;            /* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
    T_U8    loop:1;             /* loop or not */
    T_U8    id;                 /* hardware timer id */
    T_U16   total_delay;        /* total delay, microseconds */
    T_U16   cur_delay;          /* current delay, microseconds */
    T_fVTIMER_CALLBACK  callback_func;  /* callback function only for current timer */
} T_TIMER_DATA;

static T_TIMER_DATA timer_data[TIMER_NUM_MAX];
static T_U32 s_millisecond_enable = 0;

/**
 * @brief: Init RTC module
 * @author YiRuoxiang
 * @date 2006-04-03
 * @param T_fRTC_CALLBACK callback_func: RTC callback function
 * @return T_VOID
 * @retval
 */
T_VOID RTC_Init(T_fRTC_CALLBACK pFuntion)
{
}

T_BOOL RTC_SetTime(T_SYSTIME *date)
{
    return AK_TRUE;
}

T_BOOL RTC_GetTime(T_SYSTIME *date)
{
    return AK_TRUE;
}


/**
 * @brief Set true RTC value of 32k clock domain
 * @author YiRuoxiang
 * @date 2006-02-16
 * @param T_U32 rtc_val: value will be set to RTC_LOAD_REG
 * @return T_VOID
 */
 /*
T_VOID RTC_SetCount(T_U32 rtc_val)
{
}
*/
/**
 * @brief Get true RTC tick count
 * @author YiRuoxiang
 * @date 2006-02-14
 * @param T_VOID
 * @return T_U32: tick count
 */
T_U32 RTC_GetSecond(T_VOID)
{
    SYSTEMTIME w_systime;
    T_SYSTIME ak_time;
    T_U32 seconds;

    GetLocalTime(&w_systime);
    ak_time.year = w_systime.wYear;
    ak_time.month = (T_U8)w_systime.wMonth;
    ak_time.day = (T_U8)w_systime.wDay;
    ak_time.hour = (T_U8)w_systime.wHour;
    ak_time.minute = (T_U8)w_systime.wMinute;
    ak_time.second = (T_U8)w_systime.wSecond;

    seconds = Utl_Convert_DateToSecond(&ak_time);
    return seconds;
}

/**
 * @brief: Init timer. Mainly to open timer interrupt.
 * @author ZouMai
 * @date 2004-09-22
 * @param T_fTIMER_CALLBACK callback_func: Timer callback function
 * @param T_U32 sys_clk: system clock
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_init(T_VOID)
{
    T_U32   i;

    for ( i=0; i<TIMER_NUM_MAX; i++ )
    {
        timer_data[i].id = 0xff;
        timer_data[i].state = TIMER_INACTIVE;
        timer_data[i].callback_func = AK_NULL;
    }
    /* start physical timer */
    winvme_StartTimer(PHYSICAL_TIMER, 0);

    return;
}


/**
 * @brief Start timer
 * When the time reach, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_U32 milli_sec: Specifies the time-out value, in millisecond. Caution, this value must can be divided by 10.
 * @param T_BOOL loop: loop or not
 * @param T_fVTIMER_CALLBACK callback_func: Timer callback function. If callback_func is
 *              not AK_NULL, then this callback function will be called when time reach.
 * @return T_TIMER: timer ID, user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
T_TIMER vtimer_start(T_U16 tick, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
    T_U32   i=TIMER_NUM_MAX,j,cnt=0;

    if (tick == 0 || callback_func == AK_NULL) 
        return ERROR_TIMER;
        
    for ( j=0; j<TIMER_NUM_MAX; j++ )
    {
        if (timer_data[j].state == TIMER_INACTIVE)
        {
            i = j;
        }
        else if (timer_data[j].state == TIMER_ACTIVE)
        {
            if (timer_data[j].id == 0)
                cnt++;
        }
    }

    //No free timer
    if( i == TIMER_NUM_MAX )
    {
        return ERROR_TIMER;
    }
    
    timer_data[i].state = TIMER_ACTIVE;
    timer_data[i].loop = loop;
    timer_data[i].id = 0;
    timer_data[i].total_delay = tick / PHYSICAL_TIMER;
    timer_data[i].cur_delay = 0;
    timer_data[i].callback_func = callback_func;

    
    return i;
}


/**
 * @brief Stop timer
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_TIMER timer_id: Timer ID
 * @return T_VOID
 * @retval
 */
T_TIMER vtimer_stop(T_TIMER timer_id)
{
    T_U32 i,cnt=0;
    T_U8 id;

    if(timer_id >= TIMER_NUM_MAX)
        return ERROR_TIMER;
    
    if (timer_data[timer_id].state == TIMER_INACTIVE)
        return ERROR_TIMER;
    
    timer_data[timer_id].state = TIMER_INACTIVE;
    id = timer_data[timer_id].id;
    timer_data[timer_id].id = 0xff;
    
    for ( i=0; i<TIMER_NUM_MAX; i++ )
    {
        if (timer_data[i].state == TIMER_ACTIVE)
        {
            if (timer_data[i].id == id)
                cnt++;
        }
    }
        
    return ERROR_TIMER;    
}

/**
 * @brief Timer interrupt handler for WIN32
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: timer interrupt occur
 */
T_BOOL vtimer_interrupt_handler_WIN32(T_TIMER timer_id)
{
    T_U16   i;

    for (i = 0; i < TIMER_NUM_MAX; i++)
    {
        if (timer_data[i].state == TIMER_ACTIVE && timer_data[i].id == timer_id)
        {
            timer_data[i].cur_delay++;
            if (timer_data[i].cur_delay >= timer_data[i].total_delay)
            {
                if (timer_data[i].loop)
                {
                    timer_data[i].cur_delay = 0;
                }
                else
                {
                    //非循环timer，在这里置为TIMER_INACTIVE，调用者不用做释放动作，可以重新分配
                    timer_data[i].state = TIMER_INACTIVE;
                }
                timer_data[i].callback_func(i, timer_data[i].total_delay);
            }
        }
    }

    return AK_TRUE;
}

/**
 * @brief Get tick count
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_VOID
 * @return T_U32: tick count
 * @retval
 */
T_U32 get_tick_count(T_VOID)
{
    return 0;
}

/**
 * @brief delay precise millisecond function
 * delay the time
 * @author YiRuoxiang
 * @date 2006-03-10
 * @param T_U32 ms: delay millisecond time, not more 10000(10 seconds)
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_delayms(T_U32 ms)
{
    return;
}

/* end of file */
