/*******************************************************************************
 * @file    arch_sys_ctl.h
 * @brief   
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.22
 * @version 1.0
*******************************************************************************/
#ifndef __ARCH_SYS_CTL_H__
#define __ARCH_SYS_CTL_H__


#include "anyka_types.h"


typedef enum
{
    WU_GPIO     = (1<<0),
    WU_ALARM    = (1<<1),
    WU_USB      = (1<<2),
    WU_VOICE    = (1<<3),
    WU_ANALOG   = (1<<4),
    WU_POWERKEY = (1<<5)
} T_WU_TYPE;

/*******************************************************************************
 * @brief   Soft_Reset
 * @author  yangyiming
 * @date    2013-03-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Soft_Reset(T_VOID);

/*******************************************************************************
 * @brief   UpdateRom_Reset ,provide for Burntool to implement Udisk Burning
 * @author  yangyiming
 * @date    2013-03-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID UpdateRom_Reset(T_VOID);

/*******************************************************************************
 * @brief   voice wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable voice wakeup.
                        AK_FALSE means disable voice wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID voice_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   usb wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable usb wakeup.
                        AK_FALSE means disable usb wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   onoff wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable onoff wakeup.
                        AK_FALSE means disable onoff wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID powerkey_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   analog wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable analog wakeup.
                        AK_FALSE means disable analog wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID analog_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   rtc alarm wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable RTC alarm wakeup.
                        AK_FALSE means disable RTC alarm wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_alarm_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   rtc timer wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable RTC timer wakeup.
                        AK_FALSE means disable RTC timer wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_timer_wakeup_enable(T_BOOL enable);

/*******************************************************************************
 * @brief   enter standby mode. 
 * @author  wangguotian
 * @date    2012.11.21
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/ 
T_VOID enter_standby(T_VOID);

/*******************************************************************************
 * @brief   exit standby mode and return the reason
 * @author  wangguotian
 * @date    2012.11.21
 * @param   T_VOID
 * @return  T_U16 the reason of exitting standby
 * @retval  the low 8-bit of the return value is the wakeup type,
 *          refer to T_WU_TYPE.
 *          if the wakeup type is WU_GPIO, the high 8-bit of the return
 *          value is the gpio number. or else, the high 8-bit is zero.
*******************************************************************************/ 
T_U16 exit_standby(T_VOID);

/*******************************************************************************
 * @brief   get the size of the basal reg backup buf
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_U32
 * @retval  the size of the basal reg backup buf
*******************************************************************************/
T_U32 get_reg_buf_size(T_VOID);

/*******************************************************************************
 * @brief   store basal reg before enter deepstandby 
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [out]reg_buf
 * @return  T_VOID
*******************************************************************************/
T_VOID store_base_reg(T_VOID *reg_buf);

/*******************************************************************************
 * @brief   restore basal reg when recover form deepstandby
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]reg_buf
 * @return  T_VOID
*******************************************************************************/
T_VOID restore_base_reg(T_VOID *reg_buf);

/*******************************************************************************
 * @brief   enable battery power up
 * Function RTC_Init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID soft_power_on(T_VOID);

/*******************************************************************************
 * @brief   enable battery power down
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID soft_power_off(T_VOID);

/*******************************************************************************
 * @brief   cancel hardware power down
 * 长按POWERKEY键，芯片会自动进入掉电关机流程，
 * 在一定时间内调用cancel_power_off可以取消此次关机。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID cancel_power_off(T_VOID);

/*******************************************************************************
 * @brief   enable rtc in power down or not
 * 如果在关机前调用rtc_enable_in_pwd(AK_TRUE)使能RTC，则RTC在关机状态下也能正常
 * 计时工作；如果调用rtc_enable_in_pwd(AK_FALSE)失能RTC，则能节省关机功耗。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   [in]en:enable or disable
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_enable_in_pwd(T_BOOL en);

/*******************************************************************************
 * @brief   get chip id.
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_U32 the chip id
*******************************************************************************/ 
T_U32 get_chip_id(T_VOID);


#endif  //__ARCH_SYS_CTL_H__

