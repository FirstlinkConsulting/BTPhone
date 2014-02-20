/**
 * @file    adc.c
 * @brief   the interface for the AD controller
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.20
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_adc.h"
#include "clk.h"
#include "l2.h"


static T_BOOL bspi1_clk_en;


/**
 * @brief   open the AD Controller 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID adc_open(T_VOID)
{
    T_U32 reg_value;

    //L has a bug, spi1 clk must open, when adc2/3 working
    reg_value = REG32(REG_CLOCK_RST_EN);
    bspi1_clk_en = (0 == (reg_value & (1 << eVME_SPI1_CLK)));
    if(AK_FALSE == bspi1_clk_en)
    {        
        REG32(REG_CLOCK_RST_EN) = reg_value & (~(1 << eVME_SPI1_CLK));
    }
    
    sys_module_enable(eVME_ADC_CLK, AK_TRUE);
    sys_module_reset(eVME_ADC_CLK);

    //reset and enable the clk and gate
    reg_value = REG32(REG_CLOCK_DIV2);
    reg_value &= ~(ADCS_RESET_DIS | ADCS_GATE_DIS);
    reg_value |= (0xFF << ADCS_DIV);
    REG32(REG_CLOCK_DIV2) = reg_value;
    reg_value |= (ADCS_RESET_DIS | ADCS_CLK_EN);
    REG32(REG_CLOCK_DIV2) = reg_value;

    //Enable right channel clock of ADC filter
    //Enable left channel clock of ADC filter
    REG32(REG_ADCS_CHANNEL) &= ~(ADCS_RIGHT_CH_DIS | ADCS_LEFT_CH_DIS);

    REG32(REG_MUL_FUNC_CTRL) &= ~(RECEIVER_SLAVE_MODE);

    //Record from stereo channel
    //To use internal ADC
    //To receive the left channel data when the LRCK is low
    //To disable CPU read interrupt
    //To enable L2 mode
    //To enable ADC2 interface
    REG32(REG_ADCS_CTRL) = ADCS_STEREO_REC | ADCS_L2_MODE | ADCS_I_EN;

    if(AK_FALSE == l2_init_device_buf(ADDR_ADC))
    {
        drv_print("adc alloc buf fail", 0, AK_TRUE);
    }
    
#if (DRV_SUPPORT_BLUETOOTH > 0)
    l2_cpu_set_offset(0);
#endif
}


/**
 * @brief   open the AD Controller 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_VOID
 */ 
T_VOID adc_close(T_VOID)
{
    T_U32 reg_value;

    //Disable right channel clock of ADC filter
    //Disable left channel clock of ADC filter
    REG32(REG_ADCS_CHANNEL) |= (ADCS_RIGHT_CH_DIS | ADCS_LEFT_CH_DIS);

    reg_value = REG32(REG_CLOCK_DIV2);
    reg_value &= ~ADCS_CLK_EN;
    reg_value |= ADCS_GATE_DIS;
    REG32(REG_CLOCK_DIV2) = reg_value;

    REG32(REG_ADCS_CTRL) &= ~ADCS_I_EN;

    sys_module_enable(eVME_ADC_CLK, AK_FALSE);

    l2_release_device_buf(ADDR_ADC);

    if(AK_FALSE == bspi1_clk_en)
    {
        REG32(REG_CLOCK_RST_EN) |= (1 << eVME_SPI1_CLK);
    }

    
#if (DRV_SUPPORT_BLUETOOTH > 0)
    l2_cpu_set_offset(0);
#endif
}


static T_U32 get_osr_adcdiv(T_U32 *pmode_48k, T_U32 *padc_clk_div, 
                            T_U32 sample_rate)
{
    T_U32   OSR;
    T_U32   i;
    T_U32   min_dif, div;
    T_U32   aclk, aclk_r, aclk_tmp;
    T_U32   dif_tmp;
    T_U32   fact;
    T_U32   sr;

    aclk_r  = clk_get_aclk();

    if(sample_rate > 32000)
    {
        fact = 10;
    }
    else
    {
        fact = 1;
    }
    sr   = sample_rate / fact; //in case of overflow
    aclk = aclk_r/fact;

    if(24000 < sample_rate)
    {
        *pmode_48k = 1;
        OSR        = 256;
    }
    else
    {
        *pmode_48k = 0;
        OSR        = 512;
    }    
    
    min_dif = ~0;
    for(i=0x100; i > 1; --i)
    {
        aclk_tmp = sr * OSR * i;
        dif_tmp  = (aclk_tmp > aclk) ? (aclk_tmp - aclk):(aclk - aclk_tmp);
        if(dif_tmp < min_dif)
        {
            min_dif = dif_tmp;
            div     = i;
        }
    }

    if(0 != div)
    {
        div = div - 1;
    }

    *padc_clk_div = div;

    sr = aclk_r/(OSR * (div+1));

    drv_print("sr = 0x", sample_rate, AK_FALSE);
    drv_print(", set sr = 0x", sr, AK_TRUE);

    return sr;
}


/**
 * @brief   set the sample rate of the AD controller.
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]sample_rate
 * @param   [in]channel
 * @param   [in]BitsPerSample
 * @return  T_U32 
 * @retval  the real sample rate
 */ 
T_U32 adc_setinfo(T_U32 sample_rate, T_U16 channel, T_U16 BitsPerSample)
{
    T_U32   adc_clk_div, mode_48k;
    T_U32   real_sr;
    T_U32   reg_value;
    T_U32   reg_value1;
    T_U32   reg_value2;
    

    real_sr = get_osr_adcdiv(&mode_48k, &adc_clk_div, sample_rate);
    
    reg_value = REG32(REG_CLOCK_DIV2);
    //reset ADCS
    reg_value &= ~(ADCS_RESET_DIS); 
    REG32(REG_CLOCK_DIV2) = reg_value;

    reg_value &= ~(0xFF << ADCS_DIV);
    reg_value |= (adc_clk_div << ADCS_DIV);
    REG32(REG_CLOCK_DIV2) = reg_value;

    if(mode_48k)
    {
        REG32(REG_ANALOG_CTRL2) |= MODE_48K;
    }
    else
    {
        REG32(REG_ANALOG_CTRL2) &= ~MODE_48K;
    }

    reg_value |= (ADCS_RESET_DIS);
    REG32(REG_CLOCK_DIV2) = reg_value;

    reg_value  = REG32(REG_ADCS_CTRL);
    reg_value1 = REG32(REG_ADCS_CHANNEL);
    reg_value2 = REG32(REG_ANALOG_CTRL3);
    if(2 == channel)    //stereo channel
    {
        reg_value2 &= ~(PD_R_S2D | PD_L_S2D);
        reg_value2 &= ~(PD_R_SDM | PD_L_SDM);
        reg_value2 |= (LIMIT_R_EN | LIMIT_L_EN);
        reg_value1 &= ~(ADCS_RIGHT_CH_DIS);
        
        reg_value |= ADCS_STEREO_REC;
    }
    else                //mono channel, just left channel
    {
        reg_value2 &= ~(PD_L_SDM | PD_L_S2D | LIMIT_R_EN);
        reg_value2 |=  (PD_R_SDM | PD_R_S2D | LIMIT_L_EN);
        
        reg_value1 |= (ADCS_RIGHT_CH_DIS);
        
        reg_value &= ~ADCS_STEREO_REC;
        reg_value |= ADCS_CH_LEFT;
    }   
    REG32(REG_ANALOG_CTRL3) = reg_value2;
    REG32(REG_ADCS_CHANNEL) = reg_value1;
    REG32(REG_ADCS_CTRL)    = reg_value;

    return real_sr;
}

