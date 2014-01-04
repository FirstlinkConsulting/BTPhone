/*******************************************************************************
 * @file    timer.c
 * @brief   hardware timer function header file
 * the Timer0: millisecond timer, the Timer1: microsecond timer.
 * This file provides hardware timer APIs: initialization, start timer,
 * stop timer and timer interrupt handler.
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "timer.h"
#include "arch_timer.h"
#include "interrupt.h"
#include "arch_init.h"
#include "hal_int_msg.h"
#include "hal_errorstr.h"
#include "mmu.h"
#include "drv_cfg.h"


//max timer counter value
#define TIMER_MAX_COUNT             0x3ffffff

//max timer count for timer1
#define TIMER_NUM_MAX               8

#define PHYSICAL_TIMER              5   /* interrupt every 5 millisecond */
#define PHYSICAL_TIMER2             2   /* interrupt every 2S*/

//Timer status
#define TIMER_INACTIVE              0   /* inactive(available) */
#define TIMER_ACTIVE                1   /* active */
#define TIMER_TIMEOUT               2   /* time out */

#if CHIP_SEL_10C > 0
#define CRYSTAL_CLOCK               (26*1000*1000) //10C crystal is 26Mhz
#else
#define CRYSTAL_CLOCK               (12*1000*1000)
#endif

#define CLK_PER_MS                  (CRYSTAL_CLOCK/1000)
#define CLK_PER_US                  (CRYSTAL_CLOCK/1000000)

#define CLK_TIMER2                  (CRYSTAL_CLOCK*PHYSICAL_TIMER2)
#define CLK_TRSHLD                  (CLK_TIMER2 - 1200000)

#define timer_send_message(timer_id, delay) post_int_message(IM_TIMER, timer_id, delay)

#if 0
typedef enum {
    uiTIMER0 = 0,                       /* interrupt every 5 millisecond */
    uiTIMER1,
    uiTIMER_INVALID
}T_TIMER_ID;
#endif

typedef struct
{
    T_U8    state;         /* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
    T_U8    loop;          /* loop or not */
    T_U16   total_delay;   /* total delay, microseconds */
    T_U16   cur_delay;     /* current delay, microseconds */
    T_U16   callback_func; /* callback function only for current timer */
} T_TIMER_DATA;

#pragma arm section zidata = "_drvbootbss_"
static volatile T_TIMER_DATA timer_data[TIMER_NUM_MAX];
static volatile T_U32 m_second_cnt;
T_U32 g_pc_lr;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_TIMER_COUNTER;
#pragma arm section rodata



#pragma arm section code = "_drvbootinit_"
//临时添加的接口，用于平台尚未初始化驱动库的时候调用
//开启tick count计数功能，方便平台检测开机时间
T_VOID tick_start(T_VOID)
{
    m_second_cnt = 0;
    //REG32(REG_TIMER2_CTRL) = TIMER_CLEAR_BIT;
    REG32(REG_TIMER2_CTRL) = CLK_TIMER2;  //5s timer
    REG32(REG_TIMER2_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);
}
#pragma arm section code

#if (DRV_SIMU_UART > 0)
#pragma arm section code ="_drv_simu_uart_"
#else
#pragma arm section code = "_sysinit_"
#endif
/*******************************************************************************
 * @brief   init timer. Mainly to start timer2, open timer interrupt.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_init(T_VOID)
{
    T_U32 i;

    for (i=0; i<TIMER_NUM_MAX; i++)
    {
        timer_data[i].state = TIMER_INACTIVE;
    }

    REG32(REG_TIMER1_CTRL) = (CLK_PER_MS * PHYSICAL_TIMER);
    REG32(REG_TIMER1_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);
    REG32(REG_TIMER2_CTRL) = CLK_TIMER2;  //5s timer
    REG32(REG_TIMER2_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);

    //enable timer interrupt
    INT_ENABLE(INT_EN_SYSCTL);
    INT_ENABLE_SCM(INT_EN_TIMER1);
    INT_ENABLE_SCM(INT_EN_TIMER2);
}
#pragma arm section code

/*******************************************************************************
 * @brief   restore timer module when from deep standby
 * @author  Yang Yiming
 * @date    2013-08-12
 * @param   [in]timer2_cnt: the timer count before power off
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_restore(T_U32 timer2_cnt)
{
    REG32(REG_TIMER1_CTRL)  = (CLK_PER_MS * PHYSICAL_TIMER);
    REG32(REG_TIMER1_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);
    //retore current timer count
    REG32(REG_TIMER2_CTRL)  = timer2_cnt;
    REG32(REG_TIMER2_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);
    //prepare the next timer count
    REG32(REG_TIMER2_CTRL)  = (CLK_TIMER2 | TIMER_ENABLE_BIT);
}

#if (DRV_SIMU_UART > 0)
#pragma arm section code ="_drvbootinit_"
/*******************************************************************************
 * @brief   init timer. Mainly to start timer2, open timer interrupt.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer2_enable(T_VOID)
{

    //REG32(REG_TIMER2_CTRL) = TIMER_CLEAR_BIT;
    REG32(REG_TIMER2_CTRL) = CLK_TIMER2;  //5s timer
    REG32(REG_TIMER2_CTRL) |= (TIMER_LOAD_BIT | TIMER_ENABLE_BIT);
    INT_ENABLE_SCM(INT_EN_TIMER2);
}
#pragma arm section code
#endif

#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   Start timer
 * When the time reach, the timer callback function will be called. 
 * User must call function timer_stop() to free the timer ID,... 
 * ...in spite of loop is AK_TRUE or AK_FALSE.
 * Function timer_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]ms: specifies the time-out value, in millisecond.
 *                  Caution, this value must can be divided by 5.
 * @param   [in]loop: loop or not
 * @param   [in]callback_func: timer callback function. If callback_func is...
 *                             ...not AK_NULL, then this callback function...
 *                             ...will be called when time reach.
 * @return  T_TIMER: timer ID, user can stop the timer by this ID
 * @retval  ERROR_TIMER: failed
*******************************************************************************/
T_TIMER timer_start(T_U16 ms, T_BOOL loop, T_fTIMER_CALLBACK callback_func)
{
    T_U32 i;

    if (AK_FALSE == MMU_IsPremapAddr((T_U32)callback_func))
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        while (1);
    }

    irq_mask();

    for (i=0; i<TIMER_NUM_MAX; i++)
    {
        if (timer_data[i].state == TIMER_INACTIVE)
        {
            break;
        }
    }

    //No free timer
    if (i == TIMER_NUM_MAX)
    {
        irq_unmask();
        drv_print(err_str, __LINE__, AK_TRUE);
        return ERROR_TIMER;
    }

    timer_data[i].state = TIMER_ACTIVE;
    timer_data[i].loop = loop;
    timer_data[i].total_delay = ms;
    timer_data[i].cur_delay = 0;
    timer_data[i].callback_func = (T_U32)callback_func & 0xFFFF;

    irq_unmask();

    return i;
}

/*******************************************************************************
 * @brief   Stop timer
 * Function timer_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]timer_id: Timer ID
 * @return  T_TIMER
 * @retval  ERROR_TIMER:stop successfully.
*******************************************************************************/
T_TIMER timer_stop(T_TIMER timer_id)
{
    if (timer_id >= TIMER_NUM_MAX)
        return ERROR_TIMER;

    if (timer_data[timer_id].state == TIMER_INACTIVE)
        return ERROR_TIMER;

    irq_mask();

    timer_data[timer_id].state = TIMER_INACTIVE;

    irq_unmask();

    return ERROR_TIMER;
}

/*******************************************************************************
 * @brief   Timer interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_interrupt_handler(T_VOID)
{
    T_U32 i;

    REG32(REG_TIMER1_CTRL) |= TIMER_CLEAR_BIT;

    for (i=0; i<TIMER_NUM_MAX; i++)
    {
        if (TIMER_ACTIVE != timer_data[i].state)
        {
            continue;
        }

        timer_data[i].cur_delay += PHYSICAL_TIMER;
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

            if (timer_data[i].callback_func)
            {
                ((T_fTIMER_CALLBACK)\
                (timer_data[i].callback_func | L2_START_ADDR))\
                (i, timer_data[i].total_delay);
            }
            else
            {
                timer_send_message(i, timer_data[i].total_delay);
            }
        }
    }
}

/*******************************************************************************
 * @brief   Timer2 interrupt handler
 * If chip detect that timer2 counter reach 0, this function will be called.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer2_interrupt_handler(T_VOID)
{
    REG32(REG_TIMER2_CTRL) |= TIMER_CLEAR_BIT;
    m_second_cnt += PHYSICAL_TIMER2;
	//drv_print(err_str, g_pc_lr, 1);
}

/*******************************************************************************
 * @brief   get tick count from hardware timer. 
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [out]second: the time(unit:5s)
 * @return  T_U32
 * @retval  the tick count
*******************************************************************************/
static T_U32 get_tick_count(T_U32 *second)
{
    T_U32 cur_tick = 0;
    T_U32 threshold = CLK_TIMER2 - 1200000;

    *second = m_second_cnt;
    cur_tick = REG32(REG_TIMER2_CNT) & TIMER_MAX_COUNT;
    //当获取到tick大于设定的阀值时，表示Timer2可能在上述两条语句之间
    //产生了一次中断，此时需要重新读取全局变量。
    if (cur_tick > threshold)
    {
        *second = m_second_cnt;
    }

    return (CLK_TIMER2-cur_tick);
}

/*******************************************************************************
 * @brief   get us level tick count from hardware timer. 
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_U32
 * @retval  the us level tick count
 * @note    大约每隔一个多小时会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 get_tick_count_us(T_VOID)
{
    T_U32 second;
    T_U32 tick;
    T_U32 cur_us = 0;

    tick = get_tick_count(&second);
    cur_us = (second * 1000000) + (tick / CLK_PER_US);

    return cur_us;
}

/*******************************************************************************
 * @brief   get ms level tick count from hardware timer. 
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_U32
 * @retval  the ms level tick count
 * @note    大约每隔50天会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 get_tick_count_ms(T_VOID)
{
    T_U32 second;
    T_U32 tick;
    T_U32 cur_ms = 0;

    tick = get_tick_count(&second);
    cur_ms = (second * 1000) + (tick / CLK_PER_MS);

    return cur_ms;
}

/*******************************************************************************
 * @brief   delay precise microsecond function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]us: delay microsecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID delay_us(T_U32 us)
{
    if (REG32(REG_TIMER2_CTRL) & TIMER_ENABLE_BIT)
    {
        T_U32 second1, second2;
        T_U32 tick1, tick2;
        T_U32 tickdiff;

        tickdiff = us * CLK_PER_US;
        tick1 = get_tick_count(&second1);
        while (1)
        {
            tick2 = get_tick_count(&second2);

            if (((second2 - second1)*CRYSTAL_CLOCK + tick2 - tick1) >= tickdiff)
            {
                return;
            }
        }
    }
    else
    {
        //当CPU频率不小于116M的时候是准确的
        //用于Timer2尚未使能的系统启动阶段，此时CPU为116M
        T_U32 tmp;
        T_U32 i, sum;
        
        sum = (us * 10) / 6;
        for (i=0; i<=sum; i++)
        {
            tmp = REG32(REG_CHIP_ID);
        }
    }
}

/*******************************************************************************
 * @brief   delay precise millisecond function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]ms: delay millisecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID delay_ms(T_U32 ms)
{
    delay_us(ms * 1000);
}
#pragma arm section code


/* end of file */

