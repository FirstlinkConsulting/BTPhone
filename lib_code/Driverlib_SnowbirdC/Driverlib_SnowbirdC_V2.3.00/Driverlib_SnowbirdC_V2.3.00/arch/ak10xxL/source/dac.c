/**
 * @file    dac.c
 * @brief   the interface for the DA controller
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_dac.h"
#include "clk.h"
#include "arch_init.h"
#include "l2.h"


#define MAX_DACCLK 25000000


/**
 * @brief   open the DA Controller 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID dac_open(T_VOID)
{
    T_U32 reg_value;

    sys_module_enable(eVME_DAC_CLK, AK_TRUE);
    sys_module_reset(eVME_DAC_CLK);

    //reset and enable the clk and gate
    reg_value = REG32(REG_CLOCK_DIV2);
    reg_value &= ~((DACS_RESET_DIS)|(DACS_GATE_DIS));
    reg_value |= (0xFF << DACS_DIV);
    REG32(REG_CLOCK_DIV2) = reg_value;
    reg_value |= (DACS_RESET_DIS) | DACS_CLK_EN; 
    REG32(REG_CLOCK_DIV2) = reg_value;
    
    REG32(REG_ANALOG_CTRL2) |= DACS_EN;

    //enable I2S internal mode
    reg_value = REG32(REG_MUL_FUNC_CTRL);
    reg_value |= I2S_INTERNAL_MODE;
    reg_value &= ~(TRANSMITTER_SLAVE_MODE);
    REG32(REG_MUL_FUNC_CTRL) = reg_value;
    
    //I2S WORDLEN=16, LEFT POL IS HIGH,
    REG32(REG_I2S_CFG) = (15 << DACS_WORD_LEN) | DACS_LEFT_POL_HIGH;
    

#ifndef USE_NOMAL_DATA_FORMAT
    REG32(REG_DACS_CFG) = DACS_DATA_FORMAT | DACS_L2_MODE | DACS_I_EN;
#else
    REG32(REG_DACS_CFG) = DACS_L2_MODE | DACS_I_EN;
#endif

    if(AK_FALSE == l2_init_device_buf(ADDR_DAC))
    {
        drv_print("dac alloc buf fail", 0, AK_TRUE);
    }
    //in analog ,pls power on HPVCM
}


/**
 * @brief   open the DA Controller 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID dac_close(T_VOID)
{   
    T_U32   reg_value;
    
    REG32(REG_ANALOG_CTRL2) &= ~DACS_EN;
    //disable DAC interface
    REG32(REG_DACS_CFG) &= ~DACS_I_EN;  
    //disable DAC clk
    reg_value = REG32(REG_CLOCK_DIV2);    
    reg_value &= ~DACS_CLK_EN;   
    //inhibit dac clk
    reg_value |= (DACS_GATE_DIS); 
    REG32(REG_CLOCK_DIV2) = reg_value;
    //disable dac high speed clk
    REG32(REG_DACS_HCLK) |= (DACS_HCLK_DIS);    
    sys_module_enable(eVME_DAC_CLK, AK_FALSE);

    l2_release_device_buf(ADDR_DAC);
    //in analog ,pls power off HPVCM
}


static T_U32 get_osr_dacdiv(T_U32 *pOSR, T_U32 *pOSRindex, 
                            T_U32 *pdac_clk_div, T_U32 sample_rate)
{    
    const T_U16 OSR_tbl[] =  {256, 272, 264, 248, 240, 136, 128, 120};
    
    T_U32   i,j;
    T_U32   min_dif,div,OSRindex;
    T_U32   aclk,aclk_r,aclk_tmp;
    T_U32   dif_tmp;
    T_U32   div_min;
    T_U32   fact;
    T_U32   sr;

    aclk_r = clk_get_aclk();

    div_min = aclk_r / MAX_DACCLK;

    if(sample_rate > 60000)
    {
        fact = 10;        
    }
    else
    {
        fact = 1;
    }
    
    sr = sample_rate / fact; //in case of overflow
    aclk = aclk_r / fact;

    min_dif = ~0;
    for(j=0; j < sizeof(OSR_tbl)/sizeof(OSR_tbl[0]); ++j)
    {
        for(i=0x100; i > div_min; --i)
        {
            aclk_tmp = sr * OSR_tbl[j] *i;
            dif_tmp  = (aclk_tmp > aclk) ? (aclk_tmp - aclk):(aclk - aclk_tmp);
            if(dif_tmp < min_dif)
            {
                min_dif  = dif_tmp;
                div      = i;
                OSRindex = j;
            }
        }
    }

    if(0 != div)
    {
        div = div - 1;
    }

    *pdac_clk_div   = div ;
    *pOSR           = OSR_tbl[OSRindex];
    *pOSRindex      = OSRindex;
    
    sr = aclk_r/(OSR_tbl[OSRindex]*(div+1)); 
    drv_print("sr = 0x", sample_rate, AK_FALSE);
    drv_print(", set sr = 0x", sr, AK_TRUE);   

    return sr;
}




static T_VOID set_hclk(T_U32 OSR, T_U32 dac_clk_div)
{
    T_U32   reg_value;
    T_U32   div = 0;

    div = OSR*(dac_clk_div+1)/2.0/750;
    if (div >= 1)
    {
        div = div -1;
    }

    reg_value = REG32(REG_DACS_HCLK);

    reg_value &= ~(DACS_HCLK_DIS); 
    reg_value &= ~(0xFF << DACS_HCLK_DIV); 
    reg_value |= (div << DACS_HCLK_DIV);
    reg_value |= DACS_HCLK_VLD;
    REG32(REG_DACS_HCLK) = reg_value;

    //wait until changing hclk is finished, DAC_HCLK_VLD self-clear
    while((REG32(REG_DACS_HCLK) & (DACS_HCLK_VLD))!= 0);
}


/**
 * @brief   set the sample rate of the DA controller.
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]sample_rate
 * @param   [in]channel
 * @param   [in]BitsPerSample
 * @return  T_U32
 * @retval  the real sample rate
 */ 
T_U32 dac_setinfo(T_U32 sample_rate, T_U16 channel, T_U16 BitsPerSample)
{
    T_U32   OSR, index, dac_clk_div;
    T_U32   real_sr;
    T_U32   reg_value;

    //add dac_open here to deal with continuing background noise, 
    //the reason remains unknown
    dac_open();

    real_sr = get_osr_dacdiv(&OSR, &index, &dac_clk_div, sample_rate);

    REG32(REG_CLOCK_DIV2) &= ~(DACS_RESET_DIS); //reset DAC

    set_hclk(OSR, dac_clk_div);

    reg_value = REG32(REG_CLOCK_DIV2);
    reg_value &= ~(0xFF << DACS_DIV);
    reg_value |= ((dac_clk_div) << DACS_DIV);
    REG32(REG_CLOCK_DIV2) = reg_value;

    reg_value = REG32(REG_ANALOG_CTRL2);
    reg_value &= ~(0x7 << DACS_OSR);
    reg_value |= ((index & 0x7) << DACS_OSR);
    REG32(REG_ANALOG_CTRL2) = reg_value;

    REG32(REG_CLOCK_DIV2) |= (DACS_CLK_EN); //enable DAC clk

    //REG32(REG_DACS_CFG) |= (DACS_DATA_FORMAT); //set memory saving format
    REG32(REG_CLOCK_DIV2) |= (DACS_RESET_DIS); //do not reset DAC

    REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
    REG32(REG_ANALOG_CTRL1) &= ~DACS_RST_M_V;

    return real_sr;
}


