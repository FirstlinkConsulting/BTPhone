/*******************************************************************************
 * @file    sys_ctl.c
 * @brief   system control function file
 * This file provides system control APIs: initialization, standby, wake up,
 * system control interrupt handler.
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @version 1.0
 * @ref     AK1180 technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "sys_ctl.h"
#include "arch_sys_ctl.h"
#include "arch_gpio.h"
#include "analog.h"
#include "timer.h"
#include "clk.h"
#include "pmu.h"
#include "drv_cfg.h"
#include "hal_int_msg.h"


#define charge_send_message(charge_msg) post_int_message(IM_CHG, 0, charge_msg)

/*------------------System Control Interrupt Service Routine------------------*/
extern T_VOID rtc_alarm_clear_int_sta( T_VOID);
extern T_VOID rtc_interrupt_handler(T_VOID);
extern T_VOID gpio_int_detect_chk(T_U32 gpio_num, T_U8 level);
extern T_VOID IrDA_interrupt_handle(T_VOID);


#pragma arm section code = "_drvbootcode_"
static T_BOOL sysctl_interrupt_handler(T_VOID)
{
    T_U32 reg_status;

    reg_status = REG32(REG_INT_SYSCTL);
    reg_status = (reg_status&0xffff0000) & (reg_status<<16);

    if (0 == reg_status)
    {
        return AK_FALSE;
    }

    if (reg_status & INT_STA_TIMER1)
    {
        timer_interrupt_handler();
    }

    if (reg_status & INT_STA_TIMER2)
    {
        timer2_interrupt_handler();
    }
    

#ifndef BURN_TOOL
#if (CHIP_SEL_10C > 0)
    if (reg_status & INT_STA_IRDA)
    {
        //drv_print(str_irda_int,0,1);
        keypad_scan_start_chk(64,1);
    }
#endif
#if (DRV_SUPPORT_RTC > 0) && (RTC_ANYKA > 0)
    if (reg_status & INT_STA_RTC_ALARM)
    {
        rtc_interrupt_handler();
    }
/*
    if (reg_status & INT_STA_RTC_TIMER)
    {
        rtc_timer_interrupt_handler();
    }
*/
#endif
    if (reg_status & INT_STA_GPIO)
    {
        gpioset_interrupt_handler();
    }
#endif

    return AK_TRUE;
}

static T_BOOL analog_interrupt_handler(T_VOID)
{
    T_U32 reg_status;
    T_U32 charge_msg;

    reg_status = REG32(REG_INT_ANALOG);
    reg_status = (reg_status&(0x7f<<7)) & (reg_status<<7);

    if (0 == reg_status)
    {
        return AK_FALSE;
    }

    charge_msg = reg_status & (0x67<<7);
    if (charge_msg != 0)
    {
        charge_send_message(charge_msg);
    }

#if !(HP_MODE_DC > 0)
    if (reg_status & INT_STA_HP)
    {
    #if (DETECT_DEV_MAX >0)
        //假装成GPIO中断，欺骗DETECTOR模块
        gpio_int_detect_chk(HP_DETECT_GPIO, LEVEL_HIGH);
    #endif
    }
#endif
    REG32(REG_INT_ANALOG) |= reg_status;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief   system control interrupt function
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval
*******************************************************************************/
T_VOID system_control_interrupt_handler(T_VOID)
{
    sysctl_interrupt_handler();
    analog_interrupt_handler();
}
#pragma arm section code


#ifndef BURN_TOOL
/*---------------------Standby & Wakeup of System Control --------------------*/
extern T_U8 gpio_wakeup_status(T_VOID);

/*******************************************************************************
 * @brief   voice wakeup config
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]freq:voice wakeup frequency control
 * @param   [in]ref:voice wakeup threshold control
 * @param   [in]time:voice wakeup output debounce time control
 * @return  T_VOID
*******************************************************************************/
static T_VOID voice_wakeup_config(T_U8 freq, T_U8 ref, T_U8 time)
{
    T_U32 vw_cfg = 0x0;

    REG32(REG_ANALOG_CTRL4) &= ~(0xfe000000);
    vw_cfg |= (((freq & 0x3) << 30)|((ref & 0x7) << 27)|((time & 0x3) << 25));
    REG32(REG_ANALOG_CTRL4) |= vw_cfg;
}

/*******************************************************************************
 * @brief   voice wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable voice wakeup.
                        AK_FALSE means disable voice wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID voice_wakeup_enable(T_BOOL enable)
{
    T_U32 reg_value;


    if (AK_FALSE == enable)
    {
        REG32(REG_POWER_CTRL) &= ~WK_VOICE_INT_EN;  //disable voice wakeup interrupt
        REG32(REG_ANALOG_CTRL4) |= PD_VW;           //power down pd_vw
        REG32(REG_ANALOG_CTRL3) &= ~CONNECT_AVCC;   //disconnect AVCC to AVDD_MIC
        REG32(REG_ANALOG_CTRL1) |= PL_VCM2 | PL_VCM3;
    }
    else
    {
        reg_value = REG32(REG_ANALOG_CTRL3);
        reg_value |= CONNECT_AVCC;          //connect AVCC to AVDD_MIC
        reg_value &= ~CONNECT_VCM3;         //disconnect VCM3 to AVDD_MIC
        reg_value &= ~VCM3_SEL_VCM2;        //vcm3 from refrec1.5
        reg_value |= (PD_R_MICP| PD_L_MICP);//power down mic interface
        reg_value &= ~(7 << ADCS_IN_SEL);   //set no input to adcs
        REG32(REG_ANALOG_CTRL3) = reg_value;

        reg_value = REG32(REG_ANALOG_CTRL1);
        reg_value |= (4 << HP_IN);          //set hp from mic
        reg_value |= PRE_EN2;               //HP with 3k to gnd
        reg_value &= ~PL_VCM2;              //no pl_vcm2 with 2k to gnd
        reg_value &= ~PL_VCM3;              //no Pl_vcm3 with 2k to gnd
        reg_value &= ~(0x1F << PTM_DCHG);   //disable discharge for VCM2
        reg_value &= ~PD_BIAS;              //power on pd_bias
        reg_value &= ~PD_VCM2;              //power on vcm2
        reg_value &= ~PD_VCM3;              //power on vcm3
        REG32(REG_ANALOG_CTRL1) = reg_value;

        reg_value = REG32(REG_POWER_CTRL);
        reg_value &= ~WK_VOICE_INT_EN;      //disable voice wakeup interrupt
        reg_value |= WK_VOICE_INT_CLR;      //clear voice wakeup interrupt status
        REG32(REG_POWER_CTRL) = reg_value;
        reg_value &= ~WK_VOICE_INT_CLR;     //set voice wakeup interrupt status
        REG32(REG_POWER_CTRL) = reg_value;

        delay_us(700000);                   //charging time for vcm2,vcm3

        reg_value = REG32(REG_ANALOG_CTRL1);
        reg_value |= PD_BIAS;               //power off pd_bias
        reg_value |= PD_VCM2;               //power off vcm2
        reg_value |= PD_VCM3;               //power off vcm3
        reg_value &= ~(7 << HP_IN);         //set hp mute
        reg_value &= ~PRE_EN2;              //no HP with 3k to gnd
        REG32(REG_ANALOG_CTRL1) = reg_value;

        voice_wakeup_config(3, 7, 0);       //voice valve level.

        REG32(REG_ANALOG_CTRL4) &= ~PD_VW;  //power on pd_vw
        REG32(REG_POWER_CTRL) |= WK_VOICE_INT_EN;   //enable voice wakeup interrupt
    }
}

/*******************************************************************************
 * @brief   usb wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable usb wakeup.
                        AK_FALSE means disable usb wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_wakeup_enable(T_BOOL enable)
{
    T_U32 reg_val;

    reg_val = REG32(REG_POWER_CTRL);
    reg_val |= (WK_USB_INT_CLR);
    REG32(REG_POWER_CTRL) = reg_val;
    reg_val &= ~(WK_USB_INT_CLR);
    if (AK_FALSE == enable)
    {
        reg_val &= ~(WK_USB_INT_EN);
    }
    else
    {
        if (REG32(REG_INT_ANALOG) & (1<<ALG_STA_USB_IN))
        {
            REG32(REG_WGPIO_POL) |= WK_USB_INT_POL;
        }
        else
        {
            REG32(REG_WGPIO_POL) &= ~WK_USB_INT_POL;
        }
        reg_val |= (WK_USB_INT_EN);
    }
    REG32(REG_POWER_CTRL) = reg_val;
}

/*******************************************************************************
 * @brief   onoff wakeup enable
 * @author  zhanggaoxin
 * @date    2012-11-27
 * @param   [in]enable: AK_TRUE means enable onoff wakeup.
                        AK_FALSE means disable onoff wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID powerkey_wakeup_enable(T_BOOL enable)
{
    T_U32 reg_val;

    reg_val = REG32(REG_POWER_CTRL);
    reg_val |= (WK_ONOFF_INT_CLR);
    REG32(REG_POWER_CTRL) = reg_val;
    reg_val &= ~(WK_ONOFF_INT_CLR);
    if (AK_FALSE == enable)
    {
        reg_val &= ~(WK_ONOFF_INT_EN);
    }
    else
    {
        reg_val |= (WK_ONOFF_INT_EN);
    }
    REG32(REG_POWER_CTRL) = reg_val;
}

/*******************************************************************************
 * @brief   analog wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable analog wakeup.
                        AK_FALSE means disable analog wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID analog_wakeup_enable(T_BOOL enable)
{
    T_U32 reg_val;

    reg_val = REG32(REG_POWER_CTRL);

    reg_val |= (WK_AIN0_INT_CLR);
    reg_val |= (WK_AIN1_INT_CLR);
    REG32(REG_POWER_CTRL) = reg_val;

    reg_val &= ~(WK_AIN0_INT_CLR);
    reg_val &= ~(WK_AIN1_INT_CLR);
    REG32(REG_POWER_CTRL) = reg_val;

    if (AK_FALSE == enable)
    {
        reg_val &= ~(WK_AIN0_INT_EN);
        reg_val &= ~(WK_AIN1_INT_EN);
    }
    else
    {
        reg_val |= (WK_AIN0_INT_EN);
        reg_val |= (WK_AIN1_INT_EN);
    }
    REG32(REG_POWER_CTRL) = reg_val;
}

#if (DRV_SUPPORT_RTC > 0) && (RTC_ANYKA > 0)
/*******************************************************************************
 * @brief   rtc alarm wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable RTC alarm wakeup.
                        AK_FALSE means disable RTC alarm wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_alarm_wakeup_enable(T_BOOL enable)
{
    if (AK_FALSE == enable)
    {
        REG32(REG_POWER_CTRL) &= ~(WK_RTC_ALM_INT_EN);
    }
    else
    {
        REG32(REG_POWER_CTRL) |= (WK_RTC_ALM_INT_EN);
    }
}

/*******************************************************************************
 * @brief   rtc timer wakeup enable
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]enable: AK_TRUE means enable RTC timer wakeup.
                        AK_FALSE means disable RTC timer wakeup.
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_timer_wakeup_enable(T_BOOL enable)
{
    if (AK_FALSE == enable)
    {
        REG32(REG_POWER_CTRL) &= ~(WK_RTC_TMR_INT_EN);
    }
    else
    {
        REG32(REG_POWER_CTRL) |= (WK_RTC_TMR_INT_EN);
    }
}
#endif

/*******************************************************************************
 * @brief   enter standby mode. 
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/ 
T_VOID enter_standby(T_VOID)
{
    T_U32 share_reg;
    T_U32 dir_reg1;
    T_U32 dir_reg2;
    T_U32 pud_reg1;
    T_U32 pud_reg2;
    T_U32 adc1_cfg;
    T_U32 pmu_ctrl1;


    share_reg = REG32(REG_SHARE_PIN_CTRL);
    dir_reg1  = REG32(REG_GPIO_DIR_1);
    dir_reg2  = REG32(REG_GPIO_DIR_2);
    pud_reg1  = REG32(REG_GPIO_PULL_UD_1);
    pud_reg2  = REG32(REG_GPIO_PULL_UD_2);
    adc1_cfg  = REG32(REG_ADC1_CFG);
    pmu_ctrl1 = REG32(REG_PMU_CTRL1);

    //clear wakeup status
    REG32(REG_WGPIO_CLR) = 0xffffffff;
    REG32(REG_WGPIO_CLR) = 0x0;

    REG32(REG_SHARE_PIN_CTRL) = 0x0;
    REG32(REG_GPIO_DIR_1)     = 0xffffffff;
    REG32(REG_GPIO_DIR_2)     = 0xffffffff;
    REG32(REG_GPIO_PULL_UD_1) = 0xffffffff;
    REG32(REG_GPIO_PULL_UD_2) = 0xffffffff;
    REG32(REG_ADC1_CFG)       = adc1_cfg & ~VREF_PIN_SEL;
    REG32(REG_PMU_CTRL1)      = pmu_ctrl1 & ~PMU_VREF_SEL;

    REG32(REG_CLOCK_DIV1) |= (CLK_STANDBY_EN | CLK_PLL_EN);

    while (REG32(REG_CLOCK_DIV1)&(CLK_STANDBY_EN));

    //has been wake up
    REG32(REG_PMU_CTRL1)      = pmu_ctrl1;
    REG32(REG_ADC1_CFG)       = adc1_cfg;
    REG32(REG_SHARE_PIN_CTRL) = share_reg;
    REG32(REG_GPIO_DIR_1)     = dir_reg1;
    REG32(REG_GPIO_DIR_2)     = dir_reg2;
    REG32(REG_GPIO_PULL_UD_1) = pud_reg1;
    REG32(REG_GPIO_PULL_UD_2) = pud_reg2;
}

/*******************************************************************************
 * @brief   exit standby mode and return the reason
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_U16 the reason of exitting standby
 * @retval  the low 8-bit of the return value is the wakeup type,
 *          refer to T_WU_TYPE.
 *          if the wakeup type is WU_GPIO, the high 8-bit of the return
 *          value is the gpio number. or else, the high 8-bit is zero.
*******************************************************************************/ 
T_U16 exit_standby(T_VOID)
{
    T_U32 reg_val;
    T_U16 reason;

    reg_val = REG32(REG_POWER_CTRL);
    if ((reg_val & WK_USB_INT_STA) && (reg_val & WK_USB_INT_EN))
    {
        REG32(REG_POWER_CTRL) |= (WK_USB_INT_CLR);
        drv_print("wakeup by usb vbus!", 0, AK_TRUE);
        return WU_USB;
    }

    if ((reg_val & WK_VOICE_INT_STA) && (reg_val & WK_VOICE_INT_EN))
    {
        REG32(REG_POWER_CTRL) |= (WK_VOICE_INT_CLR);
        drv_print("wakeup by voice!", 0, AK_TRUE);
        return WU_VOICE;
    }

    if ((reg_val & WK_ONOFF_INT_STA) && (reg_val & WK_ONOFF_INT_EN))
    {
        REG32(REG_POWER_CTRL) |= WK_ONOFF_INT_CLR;
        drv_print("wakeup by power key!", 0, AK_TRUE);
        return WU_POWERKEY;
    }

    if ((reg_val & WK_AIN0_INT_STA) && (reg_val & WK_AIN0_INT_EN))
    {
        REG32(REG_POWER_CTRL) |= (WK_AIN0_INT_CLR);
        drv_print("wakeup by analog keypad!", 0, AK_TRUE);
        return WU_ANALOG;
    }

    if ((reg_val & WK_AIN1_INT_STA) && (reg_val & WK_AIN1_INT_EN))
    {
        REG32(REG_POWER_CTRL) |= (WK_AIN1_INT_CLR);
        drv_print("wakeup by analog keypad!", 0, AK_TRUE);
        return WU_ANALOG;
    }

    reason = gpio_wakeup_status();
    if (INVALID_GPIO == reason)
    {
        drv_print("wakeup by alarm!", 0, AK_TRUE);
        return WU_ALARM;
    }
    else
    {
        drv_print("wakeup by wakeup gpio, gpio is:", reason, AK_TRUE);
        return WU_GPIO | (reason<<8);
    }
}

/*******************************************************************************
 * @brief   get the size of the basal reg backup buf
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_U32
 * @retval  the size of the basal reg backup buf
*******************************************************************************/
T_U32 get_reg_buf_size(T_VOID)
{
    return sizeof(T_REG_BACKUP);
}

/*******************************************************************************
 * @brief   store basal reg before enter deepstandby 
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [out]reg_buf
 * @return  T_VOID
*******************************************************************************/
T_VOID store_base_reg(T_VOID *reg_buf)
{
    T_REG_BACKUP *backup;

    backup = (T_REG_BACKUP *)reg_buf;
    backup->m_dir1  = REG32(REG_GPIO_DIR_1);
    backup->m_dir2  = REG32(REG_GPIO_DIR_2);
    backup->m_pud1  = REG32(REG_GPIO_PULL_UD_1);
    backup->m_pud2  = REG32(REG_GPIO_PULL_UD_2);
    backup->m_out1  = REG32(REG_GPIO_OUT_1);
    backup->m_out2  = REG32(REG_GPIO_OUT_2);
    backup->m_in1   = REG32(REG_GPIO_IN_1);
    backup->m_in2   = REG32(REG_GPIO_IN_2);
    backup->m_intp1 = REG32(REG_GPIO_INT_POL_1);
    backup->m_intp2 = REG32(REG_GPIO_INT_POL_2);
    backup->m_inte1 = REG32(REG_GPIO_INT_EN_1);
    backup->m_inte2 = REG32(REG_GPIO_INT_EN_2);
    backup->m_sysctl      = REG32(REG_INT_SYSCTL);
    backup->m_share_pin   = REG32(REG_SHARE_PIN_CTRL);
    backup->m_timer2_cnt  = REG32(REG_TIMER2_CNT);
}

/*******************************************************************************
 * @brief   restore basal reg when recover form deepstandby
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   [in]reg_buf
 * @return  T_VOID
*******************************************************************************/
T_VOID restore_base_reg(T_VOID *reg_buf)
{
    T_REG_BACKUP *backup;

    backup = (T_REG_BACKUP *)reg_buf;
    REG32(REG_GPIO_DIR_1)     = backup->m_dir1;
    REG32(REG_GPIO_DIR_2)     = backup->m_dir2;
    REG32(REG_GPIO_PULL_UD_1) = backup->m_pud1;
    REG32(REG_GPIO_PULL_UD_2) = backup->m_pud2;
    REG32(REG_GPIO_OUT_1)     = backup->m_out1;
    REG32(REG_GPIO_OUT_2)     = backup->m_out2;
    REG32(REG_GPIO_IN_1)      = backup->m_in1;
    REG32(REG_GPIO_IN_2)      = backup->m_in2;
    REG32(REG_GPIO_INT_POL_1) = backup->m_intp1;
    REG32(REG_GPIO_INT_POL_2) = backup->m_intp2;
    REG32(REG_GPIO_INT_EN_1)  = backup->m_inte1;
    REG32(REG_GPIO_INT_EN_2)  = backup->m_inte2;
    REG32(REG_INT_SYSCTL)     = backup->m_sysctl;
    REG32(REG_SHARE_PIN_CTRL) = backup->m_share_pin;
    
    timer_restore(backup->m_timer2_cnt);
    clk_set_pll(DEF_PLL_VAL);
    //init function move to prog_manage of platform
    //pmu_init();
    //analog_init();
}

/*******************************************************************************
 * @brief   enable battery power up
 * Function RTC_Init() must be called before call this function
 * 在USB上电开机的情况下，调用soft_power_on()可以打开电池通路。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID soft_power_on(T_VOID)
{
    //只有在USB上电开机的情况下，才去输出一个
    //WatchDog信号，使电池供电；否则会致系统复位
    if (gpio_get_pin_level(USB_DETECT_GPIO))
    {
        rtc_watchdog_output();
        rtc_set_watchdog_time(AK_TRUE, 0);
        delay_us(1000);//等待WatchDog信号的输出，大约1/1024s
        rtc_set_watchdog_time(AK_FALSE, 0);
    }
}

#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   enable battery power down
 * 在仅电池供电的情况下，调用soft_power_off()可以使芯片掉电关机。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID soft_power_off(T_VOID)
{
    REG32(REG_PMU_CTRL1) |= PMU_SW_PD_EN;
}

/*******************************************************************************
 * @brief   enable rtc in power down or not
 * 如果在关机前调用rtc_enable_in_pwd(AK_TRUE)使能RTC，则RTC在关机状态下也能正常
 * 计时工作；如果调用rtc_enable_in_pwd(AK_FALSE)失能RTC，则能节省关机功耗。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   [in]en:enable or disable
 * @return  T_VOID
*******************************************************************************/
T_VOID rtc_enable_in_pwd(T_BOOL en)
{
    T_U32 reg_value;

    reg_value = REG32(REG_PMU_CTRL1);
    if (en)
    {
        reg_value &= ~PMU_PWD_RTC_SET;
        reg_value |= PMU_PWD_RTC_RST;
    }
    else
    {
        reg_value &= ~PMU_PWD_RTC_RST;
        reg_value |= PMU_PWD_RTC_SET;
    }
    REG32(REG_PMU_CTRL1) = reg_value;
}
#pragma arm section code

/*******************************************************************************
 * @brief   cancel hardware power down
 * 长按POWERKEY键，芯片会自动进入掉电关机流程，
 * 在一定时间内调用cancel_power_off可以取消此次关机。
 * @author  zhanggaoxin
 * @date    2013-3-27
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID cancel_power_off(T_VOID)
{
    REG32(REG_PMU_CTRL1) &= ~PMU_HW_PD_GATE_DIS;
    REG32(REG_PMU_CTRL1) |=  PMU_HW_PD_GATE_DIS;
}

/*******************************************************************************
 * @brief   get chip id. 
 * @author  zhanggaoxin
 * @date    2012-12-3
 * @param   T_VOID
 * @return  T_U32
 * @retval  the chip id
*******************************************************************************/ 
T_U32 get_chip_id(T_VOID)
{
    return REG32(REG_CHIP_ID);
}
#endif

