/*******************************************************************************
 * @file    rtc_ht1381.c
 * @brief   rtc_ht1381 driver
 * Copyright (C) 2012 Anyka (GuangZhou) Technology Co., Ltd.
 * @author  XP
 * @date    2009-08-25
 * @version 1.0
 * @ref     
*******************************************************************************/
#include "arch_i2c.h"
#include "rtc_device.h"
#include "arch_gpio.h"
#include "arch_init.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_RTC > 0) && (RTC_HT1381 > 0)


#define DEVICE_NAME             "RTC:HT1381"

#define BRUST_WRITE_CMD         0xBE
#define BRUST_READ_CMD          0xBF
#define WRITE_SECOND_CMD        0x80
#define READ_SECOND_CMD         0x81
#define WRITE_PROTECT_CMD       0x8E
#define CLOCK_HALT_CMD          0x80


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   set extern rtc device ready for data transmit
 * @author  ChenWeiwen
 * @date    2008-07-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE or AK FALSE 
 ******************************************************************************/
static T_BOOL rtc_ready(T_VOID)
{
    T_U8 buff[2];
    buff[0] = WRITE_PROTECT_CMD;
    buff[1] = 0x00;
    
    gpio_set_pin_level(GPIO_RTC_RST, 1);
    delay_us(5);
    i2c_write_rtc_data_ex(buff, 2);

    delay_us(5);
    gpio_set_pin_level(GPIO_RTC_RST, 0);
    delay_us(10);
    buff[0] = CLOCK_HALT_CMD;
    gpio_set_pin_level(GPIO_RTC_RST, 1);
    delay_us(10);
    
    i2c_write_rtc_data_ex(buff, 2);

    gpio_set_pin_level(GPIO_RTC_RST, 0);
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   rtc device initial
 * @author  Chenweiwen
 * @date    2009-08-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_init(T_VOID)
{
    drv_print(DEVICE_NAME, 0, AK_TRUE);

    gpio_set_pullup_pulldown(GPIO_RTC_RST, 1);
    gpio_set_pin_dir(GPIO_RTC_RST, 1);
    gpio_set_pin_level(GPIO_RTC_RST, 0);
    gpio_set_pullup_pulldown(GPIO_RTC_DAT, 1);
    gpio_set_pin_dir(GPIO_RTC_DAT, 1);
    gpio_set_pin_level(GPIO_RTC_DAT, 0);
    gpio_set_pullup_pulldown(GPIO_RTC_CLK, 1);
    gpio_set_pin_dir(GPIO_RTC_CLK, 1);
    gpio_set_pin_level(GPIO_RTC_CLK, 0);
    delay_ms(50);

    rtc_ready();
}
#pragma arm section code


/*******************************************************************************
 * @brief   set date to rtc
 * @author  Chenweiwen
 * @date    2009-08-26
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_set_time(T_SYSTIME *date)
{
    T_U8 buff[9];

    buff[0] = BRUST_WRITE_CMD;
    buff[1] = ((((date->second/10)&0x7) << 4) | (date->second%10));
    buff[2] = ((((date->minute/10)&0x7) << 4) | (date->minute%10));
    buff[3] = ((((date->hour/10)&0x3) << 4) | (date->hour%10));
    
    buff[4] = ((((date->day/10)&0x3) << 4) | (date->day%10));
    buff[5] = ((((date->month/10)&0x1) << 4) | (date->month%10));
    if(0 == date->week)
    {
        buff[6] = 7;
    }
    else
    {
        buff[6] = date->week;
    }
    buff[7] = (T_U8)(((date->year/10) << 4) | (date->year%10));
    buff[8] = 0x00;

    gpio_set_pin_level(GPIO_RTC_RST, 1);
    delay_us(100);
    i2c_write_rtc_data_ex(buff, 9);
    delay_us(100);
    gpio_set_pin_level(GPIO_RTC_RST, 0);
}

/*******************************************************************************
 * @brief   get date from rtc
 * @author  Chenweiwen
 * @date    2009-08-26
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_get_time(T_SYSTIME *date)
{
    T_U8 buff[8];
    T_U8 a, b;

    buff[0] = BRUST_READ_CMD;
    delay_us(10);
    gpio_set_pin_level(GPIO_RTC_RST, 1);
    i2c_write_rtc_data_ex(buff, 1);

    i2c_read_rtc_data_ex(buff, 8);
    gpio_set_pin_level(GPIO_RTC_RST, 0);

    a = buff[0]&0x0f;
    b = ((buff[0] >> 4)&0x07)*10;
    date->second = a + b;

    a = buff[1]&0x0f;
    b = ((buff[1] >> 4)&0x07)*10;
    date->minute = a + b;

    a = buff[2]&0x0f;
    b = ((buff[2] >> 4)&0x03)*10;
    date->hour = a + b;

    a = buff[3]&0x0f;
    b = ((buff[3] >> 4)&0x03)*10;
    date->day = a + b;

    a = buff[4]&0x0f;
    b = ((buff[4] >> 4)&0x01)*10;
    date->month = a + b;

    if(7 == buff[5]&0xf)
    {
        date->week = 0;
    }
    else
    {
        date->week = buff[5]&0xf;
    }

    a = buff[6]&0x0f;
    b = ((buff[6] >> 4)&0x0f)*10;
    date->year = (T_U16)((a + b) + 2000);
}


/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  Chenweiwen
 * @date    2009-08-26
 * @param   [in]en: enable or disable
 * @param   [in]type: type of alarm
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL rtc_driver_set_alarm_time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date)
{
    return;
}


#endif  //(DRV_SUPPORT_RTC > 0) && (RTC_HT1381 > 0)

