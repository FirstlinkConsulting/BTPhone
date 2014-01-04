/*******************************************************************************
 * @file TIMER.h
 * @brief the bits define of TIMER register
 * This file is bits define of TIMER register
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author zhanggaoxin
 * @date 2012-11-26
 * @version 1.0
 * @ref AK1080L programme manual.
*******************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__


//Three Timer Register

//REG_TIMER1_CTRL(0x0400, 0018)
//REG_TIMER2_CTRL(0x0400, 001C)
#define TIMER_STATUS_BIT           (1 << 29)  //0: Hasn't generated an interrupt
                                              //1: Has generated an interrupt.
                                              //Note: cleared after being read.
#define TIMER_CLEAR_BIT            (1 << 28)  //1: To clear timer pulse.
#define TIMER_LOAD_BIT             (1 << 27)  //1: To load new count value.
#define TIMER_ENABLE_BIT           (1 << 26)  //0: To disable the Timer
                                              //1: To enable the Timer.

//[25:0]: The value will be decremented every 12M crystal clock cycle once
//        the Timer is enabled. An interrupt is generated when it reaches
//        zero, and bit[29] is set to 1 an the same time.


//REG_TIMER2_CNT(0x0400, 0104)
//This register holds the real time value of Timer2
//[25:0]: The real time value of Timer2.


/*******************************************************************************
 * @brief   init timer. Mainly to start timer2, open timer interrupt.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_init(T_VOID);


/*******************************************************************************
 * @brief   restore timer module when from deep standby
 * @author  Yang Yiming
 * @date    2013-08-12
 * @param   [in]timer2_cnt: the timer count before power off
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_restore(T_U32 timer2_cnt);


/*******************************************************************************
 * @brief   Timer interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer_interrupt_handler(T_VOID);


/*******************************************************************************
 * @brief   Timer2 interrupt handler
 * If chip detect that timer2 counter reach 0, this function will be called.
 * @author  zhanggaoxin
 * @date    2012-12-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID timer2_interrupt_handler(T_VOID);


#endif

