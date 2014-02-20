/*******************************************************************************
 * @file    rtc.c
 * @brief   the driver of rtc module
 * Copyright (C) 2013 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "rtc_device.h"
#include "interrupt.h"
#include "arch_init.h"
#include "arch_timer.h"
#include "hal_rtc.h"
#include "drv_cfg.h"


#if (DRV_SUPPORT_RTC > 0) && (RTC_ANYKA > 0)


#define DEVICE_NAME     "RTC:ANYKA"


#define DATE_P1(date)   (date->second&0x3f | ((date->minute&0x3f) << 6))
#define DATE_P2(date)   (date->hour&0x1f | ((date->day&0x1f) << 5) |\
                        ((date->week&0x07) << 10))
#define DATE_P3(date)   (date->month&0x0f | (((date->year - 2000)&0x7f) << 4))


#pragma arm section zidata = "_drvbootbss_"
static volatile T_BOOL osc_revising;
#pragma arm section zidata


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   write rtc interal register 
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]reg: one of 7 RTC register
 * @param   [in]data: the data which will write into register
 * @return  T_VOID
*******************************************************************************/
static T_VOID rtc_write_reg(T_U32 reg, T_U32 data)
{
    T_U32 reg_value;

    while (!(REG32(REG_INT_SYSCTL)&INT_STA_RTC_READY));

    reg_value = REG32(REG_RTC_CTRL);
    reg_value &= ~(0x3fffff);
    reg_value |= (RTC_WR_RD_EN | RTC_WR_START | reg | (data&0x3fff));
    REG32(REG_RTC_CTRL) = reg_value;

    while (!(REG32(REG_INT_SYSCTL)&INT_STA_RTC_READY));

    if (AK_FALSE == osc_revising)
    {
        REG32(REG_RTC_CTRL) &= ~RTC_WR_RD_EN;
    }
}


/*******************************************************************************
 * @brief   read rtc interal register 
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]reg: one of 7 RTC register
 * @return  T_U32
 * @retval  the register data
*******************************************************************************/
static T_U32 rtc_read_reg(T_U32 reg)
{
    T_U32 reg_value;

    while (!(REG32(REG_INT_SYSCTL)&INT_STA_RTC_READY));

    reg_value = REG32(REG_RTC_CTRL);
    reg_value &= ~(0xff << 14);
    reg_value |= (RTC_WR_RD_EN | RTC_RD_START | reg);
    REG32(REG_RTC_CTRL) = reg_value;

    while (!(REG32(REG_INT_SYSCTL)&INT_STA_RTC_READY));

    if (AK_FALSE == osc_revising)
    {
        REG32(REG_RTC_CTRL) &= ~RTC_WR_RD_EN;
    }

    return REG32(REG_RTC_READ_VAL)&0x3fff;
}


static T_VOID osc_revise_finish(T_TIMER timer_id, T_U32 delay)
{
    timer_stop(timer_id);

    REG32(REG_RTC_CTRL) &= ~RTC_WR_RD_EN;
    osc_revising = AK_FALSE;
}
#pragma arm section code


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   revise internal rtc osc
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_osc_revise(T_VOID)
{
    while ((REG32(REG_RTC_READ_VAL)&RTC_OSC_REVISE));

    osc_revising = AK_TRUE;
    REG32(REG_RTC_CTRL) |= RTC_WR_RD_EN;
    REG32(REG_RTC_READ_VAL) |= RTC_OSC_REVISE;

    while ((REG32(REG_RTC_READ_VAL)&RTC_OSC_REVISE));

    timer_start(2000, AK_FALSE, osc_revise_finish);
}


/*******************************************************************************
 * @brief   rtc device initial
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_init(T_VOID)
{
    T_U32 reg_value;

    #define EXTERNAL_OSC           0
    #define INTERNAL_OSC           1

    drv_print(DEVICE_NAME, 0, AK_TRUE);

    REG32(REG_RTC_CTRL) |= RTC_EN;

    //select rtc oscillation source selection register
    reg_value = rtc_read_reg(RTC_REG7);
    reg_value &= ~(0xf << 10);
    reg_value |= RTC_OSC_SOURCE;
    rtc_write_reg(RTC_REG7, reg_value);

    //select the internal 32K crystal
    rtc_write_reg(RTC_REG6, INTERNAL_OSC);

    rtc_osc_revise();
}
#pragma arm section code


/*******************************************************************************
 * @brief   set date to rtc
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_set_time(T_SYSTIME *date)
{
    //set the second and minute
    rtc_write_reg(RTC_REG0, DATE_P1(date));
    //set the hour, day and week
    rtc_write_reg(RTC_REG1, DATE_P2(date));
    //set the month and year
    rtc_write_reg(RTC_REG2, DATE_P3(date));

    //write 3 reg data into the real time counter
    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_REAL_TIME_WR);

    //wait for write finish
    while (rtc_read_reg(RTC_REG7)&RTC_REAL_TIME_WR);
}


/*******************************************************************************
 * @brief   get date from rtc
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_get_time(T_SYSTIME *date)
{
    T_U32 reg_value = 0;


    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_REAL_TIME_RD);

    reg_value = rtc_read_reg(RTC_REG0);
    date->second = reg_value&0x3f;
    date->minute = (reg_value >> 6)&0x3f;

    reg_value = rtc_read_reg(RTC_REG1);
    date->hour = reg_value&0x1f;
    date->day = (reg_value >> 5)&0x1f;
    date->week = (reg_value >> 10)&0x07;

    reg_value = rtc_read_reg(RTC_REG2);
    date->month = reg_value&0x0f;
    date->year = ((reg_value >> 4)&0x7f) + 2000;
}


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   clear RTC common interrupt status
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_alarm_clear_int_sta( T_VOID)
{
    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_ALARM_STA_CLR);
}
#pragma arm section code


//普通定时中断/唤醒
/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]en: enable or disable
 * @param   [in]type
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL rtc_driver_set_alarm_time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date)
{
    if(en)
    {
        //set the alarm second and minute
        rtc_write_reg(RTC_REG3, DATE_P1(date) | RTC_ALARM_EN);
        //set the alarm hour, day and week
        rtc_write_reg(RTC_REG4, DATE_P2(date) | RTC_ALARM_EN);
        //set the alarm month and year
        rtc_write_reg(RTC_REG5, DATE_P3(date) | RTC_ALARM_EN);

        rtc_alarm_clear_int_sta();
        INT_ENABLE_SCM(INT_EN_RTC_ALARM);
    }
    else
    {
        //clear enable flag
        rtc_write_reg(RTC_REG3, 0);
        rtc_write_reg(RTC_REG4, 0);
        rtc_write_reg(RTC_REG5, 0);

        INT_DISABLE_SCM(INT_EN_RTC_ALARM);
    }

    return AK_TRUE;
}


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   clear rtc timer interrupt status
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_timer_clear_int_sta(T_VOID)
{
    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_TIMER_STA_CLR);
}


/*******************************************************************************
 * @brief   rtc periodicity interrupt handler
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_timer_interrupt_handler(T_VOID)
{
    //add by user
    rtc_timer_clear_int_sta();
}
#pragma arm section code


//周期性定时中断/唤醒
/*******************************************************************************
 * @brief   set the time of timer, minimum unit is 1/1024 S
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]en: enable or disable
 * @param   [in]unit: 1/1024 S
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_set_timer_time(T_BOOL en, T_U16 unit)
{
    T_U32 reg_value;

    //select rtc timer operation
    reg_value = rtc_read_reg(RTC_REG7);
    reg_value &= ~(0xf << 10);
    reg_value |= RTC_TIMER_WR;   //snowbirdL chip bug
    rtc_write_reg(RTC_REG7, reg_value);

    if(en)
    {
        rtc_write_reg(RTC_REG6, unit | RTC_TIMER_EN);

        rtc_timer_clear_int_sta();
        INT_ENABLE_SCM(INT_EN_RTC_TIMER);
    }
    else
    {
        rtc_write_reg(RTC_REG6, 0);
        INT_DISABLE_SCM(INT_EN_RTC_TIMER);
    }
}


/*******************************************************************************
 * @brief   feed the watchdog
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_watchdog_feed(T_VOID)
{
    //set RTC_REG7 bit6
    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_WATCHDOG_FED);
}


/*******************************************************************************
 * @brief   request to output the watchdog signal off the chip
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_watchdog_output(T_VOID)
{
    //enable signal output
    rtc_write_reg(RTC_REG7, rtc_read_reg(RTC_REG7)|RTC_WATCHDOG_OUT);
}


/*******************************************************************************
 * @brief   set the time of watchdog, minimum unit is 1/1024 S
 * @author  zhanggaoxin
 * @date    2013-01-08
 * @param   [in]en: enable or disable
 * @param   [in]unit: 1/1024 S
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_set_watchdog_time(T_BOOL en, T_U16 unit)
{
    T_U32 reg_value;

    //select watchdog operation
    reg_value = rtc_read_reg(RTC_REG7);
    reg_value &= ~(0xf << 10);
    reg_value |= RTC_WATCHDOG_WR;   //snowbirdL chip bug
    rtc_write_reg(RTC_REG7, reg_value);

    if(en)
    {
        rtc_write_reg(RTC_REG6, unit | RTC_WATCHDOG_EN);
    }
    else
    {
        rtc_write_reg(RTC_REG6, 0);
    }
}


#endif  //(DRV_SUPPORT_RTC > 0) && (RTC_ANYKA > 0)

