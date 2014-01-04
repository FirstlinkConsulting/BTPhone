/*******************************************************************************
 * @file    rtc_s35390a.c
 * @brief   rtc_s35390a driver
 * Copyright (C) 2012 Anyka (GuangZhou) Technology Co., Ltd.
 * @author  XP
 * @date    2009-08-25
 * @version 1.0
 * @ref     
*******************************************************************************/
#include "rtc_device.h"
#include "arch_gpio.h"
#include "interrupt.h"
#include "arch_init.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_RTC > 0) && (RTC_S35390A > 0)


#define DEVICE_NAME          "RTC:S35390A"

#define I2C_READ_FLAG        1
#define I2C_WRITE_FLAG       0

#define RTC_S35390A_ADDR     0x60   //( (0110b) <<4 )
#define RTC_S35390A_ADDR_MIX 0x60
#define RTC_S35390A_ADDR_MAX 0x67

/* S35390A regs number define */
#define REG_STATUS1         0
#define REG_STATUS2         1
#define REG_DATE            2
#define REG_TIME            3
#define REG_ALARM1          4
#define REG_FREQ1           4
#define REG_ALARM2          5
#define REG_FREQ2           5
#define REG_ADJUST          6
#define REG_FREE            7

/* S35390A regs len define */
#define STATUS1_LEN         1
#define STATUS2_LEN         1
#define DATE_LEN            7
#define TIME_LEN            3
#define ALARM1_LEN          3
#define FREQ1_LEN           1
#define ALARM2_LEN          3
#define FREQ2_LEN           5
#define ADJUST_LEN          1
#define FREE_LEN            1
#define RTC_MAX_REG_LEN     7

#define RTC_POC     (1 << 7)
#define RTC_BLD     (1 << 6)
#define RTC_24H     (1 << 1)
#define RTC_INT1    (1 << 4)
#define RTC_INT2    (1 << 5)
#define RTC_RST     (1 << 0)


#define BIT_ALARM1  (1 << 0)
#define BIT_ALARM2  (1 << 1)


/*
** 0x00 means all alarms close,
** 0x01 means alarm1 is open
** 0x02 means alarm2 is open
** 0x03 means alarm1 and alarm2 are open
*/

typedef struct{
    T_U8    year;       /* 0~99, actually yesr is 2000 + 0~99 */
    T_U8    month;      /* 1-12 */
    T_U8    day;        /* 1-31 */
    T_U8    week;       /* 0-6,  0: monday, 6: sunday*/
    T_U8    hour;       /* 0-23 for or 0~11. if status1 reg bit6 equal 0,
                            bit6 is am/pm flag: 0 means am, 1 means pm */
    T_U8    minute;     /* 0-59 */
    T_U8    second;     /* 0-59 */
}T_RTC_TIME, *T_pRTC_TIME;


/*******************************************************************************
 * @brief   change sys time to rtc time(BCD code)
 * @author  xuping
 * @date    2009-08-28
 * @param   [out]rtc: rtc time
 * @param   [in]sys: sys time
 * @return  T_VOID
*******************************************************************************/
static T_VOID rtc_time_sys2rtc(T_RTC_TIME* rtc, const T_SYSTIME* sys)
{
    AK_ASSERT_PTR_VOID(rtc,"sys2RtcTime rtc NULL");
    AK_ASSERT_PTR_VOID(rtc,"sys2RtcTime sys NULL");
    
    rtc->second= ((((sys->second/10)&0x7) << 4) | (sys->second%10));
    rtc->minute= ((((sys->minute/10)&0x7) << 4) | (sys->minute%10));
    rtc->hour= ((((sys->hour/10)&0x3) << 4) | (sys->hour%10));    
    rtc->day = ((((sys->day/10)&0x3) << 4) | (sys->day%10));
    rtc->month= ((((sys->month/10)&0x1) << 4) | (sys->month%10));
    rtc->week = sys->week;
    rtc->year = (T_U8)((((sys->year-2000)/10) << 4) | ((sys->year)%10));  
}


/*******************************************************************************
 * @brief   change rtc time(BCD code) to sys time
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]rtc: rtc time
 * @param   [out]sys: sys time
 * @return  T_VOID
*******************************************************************************/
static T_VOID rtc_time_rtc2sys(T_SYSTIME* sys, const T_RTC_TIME* rtc)
{
    T_U8 a, b;

    AK_ASSERT_PTR_VOID(rtc,"rtc2sysTime rtc NULL");
    AK_ASSERT_PTR_VOID(rtc,"rtc2sysTime sys NULL");

    a = (rtc->second)&0x0f;
    b = ((rtc->second >> 4)&0x07)*10;
    sys->second = a + b;

    a = rtc->minute &0x0f;
    b = ((rtc->minute >> 4)&0x07)*10;
    sys->minute = a + b;

    a = rtc->hour&0x0f;
    b = ((rtc->hour >> 4)&0x03)*10;
    sys->hour = a + b;

    a = rtc->day&0x0f;
    b = ((rtc->day >> 4)&0x03)*10;
    sys->day = a + b;

    a = rtc->month&0x0f;
    b = ((rtc->month >> 4)&0x01)*10;
    sys->month = a + b;

    sys->week = rtc->week&0x3;

    a = rtc->year&0x0f;
    b = ((rtc->year >> 4)&0x0f)*10;
    sys->year = (T_U16)((a + b) + 2000);
}


/*******************************************************************************
 * @brief   chang byte end to front
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]src: drc byte
 * @return  T_U8
*******************************************************************************/
static T_U8 rtc_byte_flip(T_U8 src)
{
    T_U8 tmp = 0;
    T_U32 i;

    //字节颠倒
    for (i=0; i<8; i++)
    {
        if (src & 0x01)
        {
            tmp |= (0x01 << (7 -i));
        }
        src >>= 1;
    }
    return tmp;
}


/*******************************************************************************
 * @brief   write RTC register
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]reg_num: the num of reg
 * @param   [in]reg_data: the data writed to the reg
 * @param   [in]reg_len : date len
 * @return  T_U8
 * @retval  > 0 success and = 0 fail
*******************************************************************************/
static T_U32 rtc_write_reg(T_U8 reg_num, T_U8 *reg_data, T_U8 reg_len)
{
    T_U8 dab;
    T_U8 ret;

    //构建指令地址
    dab = RTC_S35390A_ADDR | (reg_num << 1);

    for(ret = 0; ret < reg_len; ret++)
    {
        reg_data[ret] = rtc_byte_flip(reg_data[ret]);
    }
    store_all_int();
    ret = i2c_write_data(dab, reg_data, reg_len);
    restore_all_int();
    return ret;
}


/*******************************************************************************
 * @brief   read RTC register
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]reg_num: the num of reg
 * @param   [out]reg_data: the data read to the reg
 * @param   [in]reg_len : date len
 * @return  T_U8
 * @retval  > 0 success and = 0 fail
*******************************************************************************/
static T_U32 rtc_read_reg(T_U8 reg_num, T_U8 *reg_data, T_U8 reg_len)
{
    T_U8 dab;
    T_U8 ret;
    T_U8 rst;

    dab = RTC_S35390A_ADDR | (reg_num << 1) | I2C_READ_FLAG;

    store_all_int();
    rst = i2c_read_data(dab, reg_data, reg_len);
    restore_all_int();

    for(ret = 0; ret < reg_len; ret++)
    {
        reg_data[ret] = rtc_byte_flip(reg_data[ret]);
    }   

    return rst;
}


/*******************************************************************************
 * @brief   open rtc alarm
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]alarm: type of alarm 
 * @return  T_VOID
*******************************************************************************/
static T_BOOL rtc_open_alarm(RTC_ALARM_TYPE alarm)
{
    T_U8 status1;
    T_U8 status2;

    rtc_read_reg(REG_STATUS1, &status1, STATUS1_LEN );
    rtc_read_reg(REG_STATUS2, &status2, STATUS2_LEN );

    switch (alarm)
    {
        case RTC_ALARM1:
            status2 |= 0x04;    //alarm 中断
            status1 |= RTC_INT1;
            break;

        case RTC_ALARM2:
            status2 |= 0x40;    //alarm 中断
            status1 |= RTC_INT2;
            break;

        default:
            return AK_FALSE;
            break;
    }

    //设置报警中断输出pin
    rtc_write_reg(REG_STATUS1, &status1, STATUS1_LEN);
    rtc_write_reg(REG_STATUS2, &status2, STATUS2_LEN);

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   clear rtc alarm time
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]alarm: type of alarm 
 * @return  T_S32
 * @retvak  -1 fail
 *
 * @notes   must call after alarm interrupt to clear alarm 
*******************************************************************************/
static T_VOID rtc_clear_alarm(RTC_ALARM_TYPE alarm)
{
    T_U8 status;


    switch (alarm)
    {
        case RTC_ALARM1:
            rtc_read_reg(REG_STATUS2, &status, STATUS1_LEN);
            status &= 0xf0;
            break;

        case RTC_ALARM2:
            rtc_read_reg(REG_STATUS2, &status, STATUS2_LEN);
            status &= 0x0f;
            break;

        default:
            return;
            break;
    }

    rtc_write_reg(REG_STATUS2, &status, STATUS2_LEN);
}


/*******************************************************************************
 * @brief   rtc device initial
 * @author  xuping
 * @date    2009-08-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_init(T_VOID)
{
    T_U8 buf[1] = {0};

    drv_print(DEVICE_NAME, 0, AK_TRUE);

    buf[0] = RTC_RST;
    /* init rtc module by set status1 reg's bit0 */
    rtc_write_reg(REG_STATUS1, buf, 1);

    delay_ms(10);
    rtc_read_reg(REG_STATUS1, buf, 1);

    buf[0] |= (RTC_24H | RTC_INT1);//初始化为24小时， INT1 pin报警
    rtc_write_reg(REG_STATUS1, buf, 1);
    drv_print("*** rtc init", 0, 1);
}


/*******************************************************************************
 * @brief   get date from rtc
 * @author  xuping
 * @date    2009-08-26
 * @param   [out]date: the buffer of a structural which include 
 *                     second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_get_time(T_SYSTIME *date)
{
    T_U8 *buf;
    T_RTC_TIME rtime;

    AK_ASSERT_PTR_VOID(date,"RTC READ ptr NULL");

    buf = (T_U8 *)&rtime;
    rtc_read_reg(REG_DATE, buf, DATE_LEN);

    rtc_time_rtc2sys(date, &rtime);
}


/*******************************************************************************
 * @brief   set date to rtc
 * @author  xuping
 * @date    2009-08-26
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_driver_set_time(T_SYSTIME *date)
{
    T_U8 *buf;
    T_RTC_TIME rtime;

    AK_ASSERT_PTR_VOID(date,"RTC Write ptr NULL"); 

    rtc_time_sys2rtc(&rtime, date);
    if (date->hour > 12)
    {
        rtime.hour |= (1 << 6);//PM
    } 
    buf = (T_U8 *)&rtime;
    rtc_write_reg(REG_DATE, buf, DATE_LEN); 
}


/*******************************************************************************
 * @brief   set the time of common alarm, minimum unit is 1 S
 * @author  xuping
 * @date    2009-08-28
 * @param   [in]en: enable or disable
 * @param   [in]type: type of alarm
 * @param   [in]date: the buffer of a structural which include 
 *                    second, minute, hour, date......
 * @return  T_BOOL
*******************************************************************************/
T_BOOL rtc_driver_set_alarm_time(T_BOOL en, RTC_ALARM_TYPE type, T_SYSTIME *date)
{
    T_U8 reg_num;
    T_U8 buf[ALARM1_LEN];

    if(en)
    {
        if (!rtc_open_alarm(type))
        {
                drv_print("open alarm fail", AK_FALSE, AK_TRUE);
                return AK_FALSE;
        }
        gpio_int_polarity(GPIO_RTC_WAKEUP, LEVEL_LOW);
        gpio_int_enable(GPIO_RTC_WAKEUP, AK_TRUE);

        switch (type)
        {
            case RTC_ALARM1:
                reg_num = REG_ALARM1;
                break;

            case RTC_ALARM2:
                reg_num = REG_ALARM2;
                break;

            default:
                return AK_FALSE;
        }
        buf[0] = date->week;  //星期设置无效
        buf[1] = ((((date->hour/10)&0x3) << 4) | (date->hour%10) | (1 << 7));//24小时
        if (date->hour > 12 )
        {
            buf[1] |= (1 << 6);//PM
        }
        buf[2] = ((((date->minute/10)&0x7) << 4) | (date->minute%10) | (1 << 7));

        rtc_write_reg(reg_num, buf, ALARM1_LEN);
    }
    else
    {
        gpio_int_enable(GPIO_RTC_WAKEUP, 0);
        rtc_clear_alarm(type);
    }

    return AK_TRUE;
}


#endif  //(DRV_SUPPORT_RTC > 0) && (RTC_S35390A > 0)

