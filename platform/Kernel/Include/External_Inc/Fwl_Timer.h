/*******************************************************************************
 * @file    Fwl_Timer.h
 * @brief   This header file is for OS related function prototype
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_TIMER_H__
#define __FWL_TIMER_H__


#include "anyka_types.h"


#define GET_TIMER_ID(pEventParam)           ((pEventParam)->w.Param1)
#define GET_TIMER_DELAY(pEventParam)        ((pEventParam)->w.Param2)


/*******************************************************************************
 * @brief   initialize timer
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_TimerInit(T_VOID);


/*******************************************************************************
 * @brief   start timer. When the time reach, the vtimer callback function 
 *          will be called. User must call function Fwl_TimerStop() to free 
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
T_TIMER Fwl_TimerStart(T_U16 milliSeconds, T_BOOL loop, T_fVTIMER_CALLBACK callback);


T_TIMER Fwl_TimerStartSecond(T_U8 seconds, T_BOOL loop);
T_TIMER Fwl_TimerStartMilliSecond(T_U16 milliSeconds, T_BOOL loop);


/*******************************************************************************
 * @brief   stop timer
 * Function Fwl_TimerInit() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]timerHandle: the timer handle
 * @return  T_TIMER
 * @retval  timer handle
*******************************************************************************/
T_TIMER Fwl_TimerStop(T_TIMER timerHandle);


/*******************************************************************************
 * @brief   get ms tick
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_U32
 * @retval  ms tick
 * @note    大约每隔50天会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 Fwl_GetTickCountMs(T_VOID);


/*******************************************************************************
 * @brief   get us tick
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_U32
 * @retval  us tick
 * @note    大约每隔一个多小时会有一次T_U32溢出反转，所以调用者需对其做相应处理
*******************************************************************************/
T_U32 Fwl_GetTickCountUs(T_VOID);


/*******************************************************************************
 * @brief   delay microsecond function
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]us: delay microsecond time
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_DelayUs(T_U32 us);

T_VOID Fwl_DelayMs(T_U32 ms);

#endif //__FWL_TIMER_H__

