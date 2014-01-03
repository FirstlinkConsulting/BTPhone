/*******************************************************************************
 * @file    arch_timer.h
 * @brief   timer function header file
 * This file provides timer APIs: start timer, stop timer and get tick count
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#ifndef __ARCH_TIMER_H__
#define __ARCH_TIMER_H__


#include "anyka_types.h"


#define ERROR_TIMER         0xFF

/* Define timer callback function type */
typedef T_VOID (*T_fTIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);



/*******************************************************************************
 * @brief   start timer
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
 * @return  T_TIMER
 * @retval  timer ID, user can stop the timer by this ID. ERROR_TIMER: failed
*******************************************************************************/
T_TIMER timer_start(T_U16 ms, T_BOOL loop, T_fTIMER_CALLBACK callback_func);


/*******************************************************************************
 * @brief   stop timer
 * Function timer_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]timer_id: Timer ID
 * @return  T_TIMER
 * @retval  ERROR_TIMER:stop successfully.
*******************************************************************************/
T_TIMER timer_stop(T_TIMER timer_id);


/*******************************************************************************
 * @brief   get us level tick count from hardware timer.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_U32
 * @retval  the us level tick count
 * @note    大约每隔一个多小时会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 get_tick_count_us(T_VOID);


/*******************************************************************************
 * @brief   get ms level tick count from hardware timer.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_U32
 * @retval  the ms level tick count
 * @note    大约每隔50天会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 get_tick_count_ms(T_VOID);


/*******************************************************************************
 * @brief   delay precise millisecond function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]ms: delay millisecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID delay_ms(T_U32 ms);


/*******************************************************************************
 * @brief   delay precise microsecond function
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   [in]us: delay microsecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID delay_us(T_U32 us);


#endif //__ARCH_TIMER_H__

