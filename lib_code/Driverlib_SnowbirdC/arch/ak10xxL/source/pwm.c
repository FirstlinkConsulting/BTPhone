/*******************************************************************************
 * @FILENAME: pwm.c
 * @BRIEF pwm driver file
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @AUTHOR zhanggaoxin
 * @DATE 2012-12-14
 * @VERSION 1.0
 * @REF
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "arch_pwm.h"
#include "share_pin.h"
#include "drv_cfg.h"


#define PWM_MAX_DUTY_CYCLE  (100)


/*******************************************************************************
 * @brief  set duty cycle of pwm
 * @author zhanggaoxin
 * @date   2012-12-18
 * @param  [in] pwm_id pwm id, can be uiPWM0, uiPWM1.
 * @param  [in] pwm_freq frequency of pwm, must in [92HZ--6MHZ]
 * @param  [in] duty_cycle duty_cycle of pwm
 * @return T_BOOL
 * @retval AK_TRUE set successfully
 * @retval AK_FALSE set unsuccessfully
*******************************************************************************/
T_BOOL pwm_set_duty_cycle(T_ePWM_ID pwm_id, T_U32 pwm_freq, T_U16 duty_cycle)
{
    T_U32 freq_div;
    T_U32 high_level_clk, low_level_clk;
    T_U32 temp_freq;

#if CHIP_SEL_10C > 0
    temp_freq = 26000000;
#else
    temp_freq = 12000000;
#endif

    if ((pwm_id >= MAX_PWM_NUM) || (pwm_id < uiPWM0))
    {
        return AK_FALSE;
    }
    if ((92 >= pwm_freq) || (6000000 < pwm_freq) )
    {
        return AK_FALSE;
    }
    if (duty_cycle > PWM_MAX_DUTY_CYCLE)
    {
        return AK_FALSE;
    }

    if (0 == duty_cycle)
    {
        high_level_clk = 0x00;
        low_level_clk  = 0xffff;
    }
    else if (PWM_MAX_DUTY_CYCLE == duty_cycle)
    {
        high_level_clk = 0xffff;
        low_level_clk  = 0x00;
    }
    else
    {
        freq_div = temp_freq / pwm_freq;

        high_level_clk = freq_div * duty_cycle / PWM_MAX_DUTY_CYCLE;
        if (high_level_clk > 65535)
        {
            return AK_FALSE;
        }

        low_level_clk  = freq_div - high_level_clk;
        if (low_level_clk > 65535)
        {
            return AK_FALSE;
        }
    }

#if (CHIP_SEL_10C > 0)
    if (uiPWM0 == pwm_id)
    {
        if(AK_FALSE == sys_share_pin_is_lock(ePIN_AS_PWM))
        {
            sys_share_pin_lock(ePIN_AS_PWM);
        }
        if (0 == duty_cycle)
        {
            REG32(REG_PWM0_CTRL) = 0;
        }
        else
        {
            REG32(REG_PWM0_CTRL) = (high_level_clk << 17) | low_level_clk | (1<<16);
        }
    }
    else
    {
        if(AK_FALSE == sys_share_pin_is_lock(ePIN_AS_PWM2))
        {
            sys_share_pin_lock(ePIN_AS_PWM2);
        }
        REG32(REG_PWM1_CTRL) = (high_level_clk << 16) | low_level_clk;
    }
#else
    if(AK_FALSE == sys_share_pin_is_lock(ePIN_AS_PWM))
    {
        sys_share_pin_lock(ePIN_AS_PWM);
    }

    if (uiPWM0 == pwm_id)
    {
        REG32(REG_PWM0_CTRL) = (high_level_clk << 16) | low_level_clk;
    }
    else
    {
        REG32(REG_PWM1_CTRL) = (high_level_clk << 16) | low_level_clk;
    }
#endif

    return AK_TRUE;
}

/*******************************************************************************
 * @brief  enable square wave of chip
 * @author zhanggaoxin
 * @date   2012-12-18
 * @param  [in] eable or diable
 * @return T_VOID
*******************************************************************************/
#if CHIP_SEL_10C > 0
T_VOID pwm_set_square_for_chip(T_BOOL enable)
{    
    if (enable)
    {
        REG32(REG_SHARE_PIN_CTRL3) |= (1<<0); //select square wave
        REG32(REG_GPIO_DIR_1) &= ~(1<<3); //select output
    }
    else
    {
        REG32(REG_SHARE_PIN_CTRL3) &= ~(1<<0); //select square wave
        REG32(REG_GPIO_DIR_1) |= (1<<3); //select output
    }
}

/*******************************************************************************
 * @brief  generate 32.768kHZ square
 * @author zhanggaoxin
 * @date   2012-12-18
 * @param  [in] eable or diable
 * @return T_VOID
*******************************************************************************/
T_VOID pwm_start_32k_square(T_VOID)
{
	pwm_set_duty_cycle(0, 32768, 50); 
	pwm_set_square_for_chip(AK_TRUE);
}

/*******************************************************************************
 * @brief  stop generating 32.768kHZ square,pwm output low
 * @author zhanggaoxin
 * @date   2012-12-18
 * @param  [in] eable or diable
 * @return T_VOID
*******************************************************************************/
T_VOID pwm_stop_32k_square(T_VOID)
{
	pwm_set_duty_cycle(0, 32768, 0); 
	pwm_set_square_for_chip(AK_FALSE);
}

#endif

