/*******************************************************************************
 * @FILENAME: clk.c
 * @BRIEF clk driver file
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @AUTHOR zhanggaoxin
 * @DATE 2012-12-13
 * @VERSION 1.0
 * @REF
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "interrupt.h"
#include "clk.h"
#include "arch_init.h"
#include "drv_cfg.h"
#include "usb_bus_drv.h"


extern T_VOID uart_on_change(T_U32 asic_clk);
extern T_VOID spi_on_change(T_U32 asic_clk);
extern T_VOID nfc_timing_adjust(T_U32 asic_clk);
extern T_VOID sd_on_change(T_U32 asic_clk);

#if DRV_SIMU_UART > 0

//list
static const T_U8 simu_sys_clk[]={116,0};

static const T_U8 simu_uart_delay[][2]=
{
    {0x60,0x64},//116000000
    {0x60,0x64},//58000000
    {0x5d,0x64},//38666666
    {0x5c,0x64},//29000000
    {0x60,0x64},//96000000,
    {0x61,0x64}//114000000
};
#endif

/*******************************************************************************
 * @brief   set PLL frequency value.
 * main clock is controlled by pll register.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]clk_168 pll frequency value(Hz)
 * @return  T_BOOL
 * @retval  AK_TRUE  set pll frequency successful
 * @retval  AK_FALSE set pll frequency unsuccessful
*******************************************************************************/
T_BOOL clk_set_pll(T_U32 clk_168)
{
    T_U32 reg;
    T_U32 pll_sel = 0, aclk_div = 0;
    T_U32 cur_asic, cur_clk_168;
    T_U32 asic_div;

    clk_168 = (clk_168 / 1000000) & ~(0x3);
    //check param, set pll bounds to (180, 432)
    if (clk_168 > CLK_PLL_MAX || clk_168 < CLK_PLL_MIN)
    {
        return AK_FALSE;
    }

    cur_clk_168 = clk_get_clk168m();
    

    if ((cur_clk_168 == clk_168 * 1000000) && (clk_get_aclk() <= MAX_ACLK_VAL))
    {
        return AK_TRUE;
    }

    pll_sel = (clk_168 - CLK_PLL_MIN) >> 2;
    clk_168 = clk_168 * 1000000;
    while ((aclk_div + 1) * MAX_ACLK_VAL < clk_168)
    {
        aclk_div++;
    }
    
    check_stack();
    store_all_int();

    //freq low to high, adjust asic module timing first
    if (cur_clk_168 < clk_168)
    {
        cur_asic = clk_get_asic();
        asic_div = cur_clk_168 / cur_asic;
        
        spi_on_change(clk_168 / asic_div);
 #if DRV_SUPPORT_NAND > 0
        nfc_timing_adjust(clk_168 / asic_div);
 #endif
        sd_on_change(clk_168 / asic_div);
    }

    reg = REG32(REG_CLOCK_DIV1);
    
    reg &= ~(0x3f << CLK_PLL_SEL);    //reg clear
    reg |= (pll_sel << CLK_PLL_SEL);  //set pll div
    reg &= ~(0xf << CLK_ACLK_DIV);
    reg |= (aclk_div << CLK_ACLK_DIV);
    reg &= ~(0xf << CLK_CLK168M_DIV); //set clk168m = pll
    reg |= CLK_PLL_EN;
    
    REG32(REG_CLOCK_DIV1) = reg;

    while (REG32(REG_CLOCK_DIV1) & CLK_PLL_EN);

    //need to adjust asic module, as asic freqency was also changed
    cur_asic = clk_get_asic();
    uart_on_change(cur_asic);
    //freq high to low, adjust asic module timing later
    if (cur_clk_168 > clk_168)
    {   
        spi_on_change(cur_asic);
#if DRV_SUPPORT_NAND > 0
        nfc_timing_adjust(cur_asic);
#endif
        sd_on_change(cur_asic);
    }

    restore_all_int();

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   get current system pll.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   T_VOID
 * @return  T_U32
 * @retval  pll frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_pll(T_VOID)
{
    T_U32 reg_val, pll_val;

    reg_val = REG32(REG_CLOCK_DIV1);
    pll_val = (((reg_val>>CLK_PLL_SEL) & 0x3f) << 2) + CLK_PLL_MIN;

    return (pll_val*1000000);
}


/*******************************************************************************
 * @brief   get current system clk168M.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   T_VOID
 * @return  T_U32
 * @retval  clk168M frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_clk168m(T_VOID)
{
    T_U32 reg_val, pll_val;

    reg_val = REG32(REG_CLOCK_DIV1);
    pll_val = (((reg_val>>CLK_PLL_SEL) & 0x3f) << 2) + CLK_PLL_MIN;

    return ((pll_val*1000000) / (((reg_val >> CLK_CLK168M_DIV) & 0xf) + 1));
}


/*******************************************************************************
 * @brief   get current system aclk.
 * @author  wangguotian
 * @date    2012-12-21
 * @param   T_VOID
 * @return  T_U32
 * @retval  aclk frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_aclk(T_VOID)
{
    T_U32 reg_val, pll_val;

    reg_val = REG32(REG_CLOCK_DIV1);
    pll_val = (((reg_val>>CLK_PLL_SEL) & 0x3f) << 2) + CLK_PLL_MIN;

    return ((pll_val*1000000) / (((reg_val >> CLK_ACLK_DIV) & 0xf) + 1));
}


#pragma arm section code = "_changefreq_"
/*******************************************************************************
 * @brief   get asic parameters.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]freq_div
 * @param   [in/out]mclk_div
 * @param   [in/out]asic_div
 * @return  T_BOOL
 * @retval  
*******************************************************************************/
static T_BOOL get_asic_param(T_U32 freq_div, T_U32 *mclk_div, T_U32 *asic_div)
{
    if ((AK_NULL == mclk_div) || (AK_NULL == asic_div))
    {
        return AK_FALSE;
    }

    if ((*asic_div == 1) || (*asic_div == 0))
    {
        for (*mclk_div+=1; *mclk_div<16; (*mclk_div)++)
        {
            if (freq_div < ((*mclk_div + 1) << 1))
            {
                break;
            }
        }
        if (*mclk_div > 15)
        {
            *asic_div = 2;
        }
    }

    if (*asic_div > 1)
    {
repeat: for (*mclk_div=8; *mclk_div<16; (*mclk_div)++)
        {
            if (freq_div < ((*mclk_div + 1) << *asic_div))
            {
                break;
            }
        }
        if (*mclk_div > 15)
        {
            *asic_div += 1;
            if (*asic_div > 7)
            {
                return AK_FALSE;
            }
            goto repeat;
        }
    }

    if ((*asic_div > 1) && (*mclk_div == 8))
    {
        *asic_div -= 1;
        *mclk_div = 15;
    }
    else
    {
        *mclk_div -= 1;
    }

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   unified function interface provided for adjust cpu clk
 * @author  zhanggaoxin
 * @date    2012-12-14
 * @param   [in]freq cpu frequency value to set(Hz)
 * @param   [in]bottom_asic the min asic can be set(Hz)
 * @return  T_BOOL
 * @retval  AK_TRUE  adjust successfully
 * @retval  AK_FALSE adjust failed
*******************************************************************************/
T_BOOL clk_adjust(T_U32 freq, T_U32 bottom_asic)
{
    T_U32 mclk_div = 0, asic_div = 1;
    T_U32 cur_pll = clk_get_pll();
    T_U32 freq_div = cur_pll / freq;
    T_BOOL cpu2x_flag = AK_FALSE;
    
#if DRV_SUPPORT_UHOST > 0
#if UHOST_USE_INTR == 0
    extern T_U32 connect_flag;


    if (USB_CONNECT_OK == connect_flag)
    {
        if (freq > clk_get_asic())
        {
            if (AK_FALSE == clk_get_cpu2x())
            {
                clk_set_cpu2x(AK_TRUE);
            }
        }
        else if (freq < clk_get_asic())
        {
            if (AK_TRUE == clk_get_cpu2x())
            {
                clk_set_cpu2x(AK_FALSE);
            }
        }

        return AK_TRUE;
    }
#endif
#endif//#if DRV_SUPPORT_UHOST > 0

    if ((freq_div < 16) && ((cur_pll/freq_div) >= (2*bottom_asic)) 
        && ((freq_div & 1) == 1))
    {
        mclk_div = freq_div - 1;
        cpu2x_flag = AK_TRUE;
    }
    else
    {
        while (1)       //seek the divider of the max asic value can be set
        {
            if (((cur_pll/(mclk_div+1))>>1) <= MAX_ASIC_VAL)
            {
                break;
            }
            mclk_div++;
        }
        if (AK_FALSE == get_asic_param(freq_div, &mclk_div, &asic_div))
        {
            return AK_FALSE;
        }
    }

    if (AK_TRUE == cpu2x_flag)
    {
        if (AK_FALSE == clk_get_cpu2x())
        {
            clk_set_cpu2x(AK_TRUE);
        }
    }
    else
    {
        if (AK_TRUE == clk_get_cpu2x())
        {
            clk_set_cpu2x(AK_FALSE);
        }
    }
    if (((cur_pll/(mclk_div + 1)) >> asic_div) == clk_get_asic())
    {
        return AK_TRUE;
    }
    else
    {
        return clk_set_asic(mclk_div, asic_div);
    }
}


/*******************************************************************************
 * @brief   set asic frequency.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]mclk_div
 * @param   [in]asic_div 
 * @return  T_BOOL
 * @retval  AK_TRUE  set asic frequency successful
 * @retval  AK_FALSE set asic frequency unsuccessful
*******************************************************************************/
T_BOOL clk_set_asic(T_U32 mclk_div, T_U32 asic_div)
{
    T_U32 reg;
    T_U32 cur_pll, cur_asic;
    T_U32 freq;
    T_U8 i;
    
    cur_pll  = clk_get_pll();
    cur_asic = clk_get_asic();
    freq     = (cur_pll/(mclk_div + 1)) >> asic_div;

    check_stack();
    store_all_int();
    //freq low to high, adjust asic module timing first
    if (cur_asic < freq)
    {        

    #if DRV_SIMU_UART > 0
        for (i=0;i<sizeof(simu_sys_clk);i++)
        {
            if (simu_sys_clk[i]==freq)
                Simu_UART_OnChange(simu_uart_delay[i][clk_get_cpu2x()]);    
        }
    #endif
    
        spi_on_change(freq);
#if DRV_SUPPORT_NAND > 0
        nfc_timing_adjust(freq);
#endif
        sd_on_change(freq);
    }

    reg = REG32(REG_CLOCK_DIV1);
    reg &= ~(0xf << CLK_MCLK_DIV);
    reg &= ~((0xf << CLK_CLK168M_DIV) | (0x07 << CLK_ASIC_DIV));
    reg |= ((mclk_div<<CLK_CLK168M_DIV) | CLK_CLK168M_EN
            | CLK_ASIC_EN | (asic_div<<CLK_ASIC_DIV));
    REG32(REG_CLOCK_DIV1) = reg;

    while ((REG32(REG_CLOCK_DIV1) & CLK_CLK168M_EN) 
            || (REG32(REG_CLOCK_DIV1) & CLK_ASIC_EN));

    freq = clk_get_asic();
    //adjust asic module clock
    uart_on_change(freq);
    //freq high to low, adjust asic module timing later
    if (cur_asic > freq)
    {

    #if DRV_SIMU_UART > 0
            for (i=0;i<sizeof(simu_sys_clk);i++)
            {
                if (simu_sys_clk[i]==freq)
                    Simu_UART_OnChange(simu_uart_delay[i][clk_get_cpu2x()]);    
            }
    #endif

        spi_on_change(freq);
#if DRV_SUPPORT_NAND > 0
        nfc_timing_adjust(freq);
#endif
        sd_on_change(freq);
    }
    restore_all_int();

    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   get the asic freq.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   T_VOID
 * @return  T_U32
 * @retval  asic frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_asic(T_VOID)
{
    T_U32 reg_val, mclk_div, asic_div;
    T_U32 clk168, mclk_tmp;
    T_U32 pll_val, clk168_div;

    reg_val = REG32(REG_CLOCK_DIV1);

    pll_val = ((reg_val & 0x3f) << 2) + CLK_PLL_MIN;
    clk168_div = (reg_val >> CLK_CLK168M_DIV) & 0xf;

    clk168  = (pll_val*1000000) / (clk168_div+1);

    mclk_div = (reg_val >> CLK_MCLK_DIV) & 0xf;
    asic_div = (reg_val >> CLK_ASIC_DIV) & 0x7;

    if(0 == asic_div) asic_div = 1;

    mclk_tmp = clk168 / (mclk_div+1);

    return (mclk_tmp>>asic_div);
}
#pragma arm section code


#pragma arm section code = "_changefreq_"
/*******************************************************************************
 * @brief   set cpu frequency twice of asic frequency or not
 * this function just set cpu_clk = PLL1_clk or set cpu_clk = asic_clk
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]set_value  set twice or not
 * @return  T_VOID
*******************************************************************************/
T_VOID clk_set_cpu2x(T_BOOL set_value)
{
    store_all_int();
    if (set_value)
    {
        REG32(REG_CLOCK_DIV1) |= CLK_CPU2X_EN;
    }
    else
    {
        REG32(REG_CLOCK_DIV1) &= ~CLK_CPU2X_EN;
    }
    restore_all_int();
}
#pragma arm section code


/*******************************************************************************
 * @brief   judge whether cpu frequency is twice of asic frequency or not
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @return  T_BOOL
 * @retval  AK_TRUE cpu frequency is twice of asic frequency
 * @retval  AK_FALSE cpu frequency is not twice of asic frequency
*******************************************************************************/
T_BOOL clk_get_cpu2x(T_VOID)
{   
    return (REG32(REG_CLOCK_DIV1) & CLK_CPU2X_EN) ? (AK_TRUE) : (AK_FALSE);
}


#pragma arm section code ="_drvbootcode_"
/*******************************************************************************
 * @brief   reset different module in ASIC
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]vT_ModuleList MoudleId
 * @return  T_BOOL
 * @retval
*******************************************************************************/
T_BOOL sys_module_reset(vT_ModuleList MoudleId)
{
    if ((MoudleId >= eVME_SleepMode) || (MoudleId < eVME_LCD_CLK))
    {
        return AK_FALSE;
    }

    irq_mask();
    REG32(REG_CLOCK_RST_EN) |= (1 << MoudleId) << 16;
    REG32(REG_CLOCK_RST_EN) &= ~((1 << MoudleId) << 16);
    irq_unmask();

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   enable or disable module clock.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]vT_ModuleList MoudleId
 * @param   [in]T_BOOL EnableFlag
 * @return  T_BOOL
 * @retval
*******************************************************************************/
T_BOOL sys_module_enable(vT_ModuleList MoudleId, T_BOOL EnableFlag)
{
    if ((MoudleId > eVME_SleepMode) || (MoudleId < eVME_LCD_CLK))
    {
        return AK_FALSE;
    }

    if (MoudleId == eVME_SleepMode)
    {
        if (EnableFlag == AK_TRUE)
        {
            REG32(REG_CLOCK_RST_EN) = SleepMode_enable;
        }
        else
        {
            REG32(REG_CLOCK_RST_EN) = SleepMode_disable;
        }
    }
    else
    {
        if (EnableFlag)
        {
            REG32(REG_CLOCK_RST_EN) &= ~(1 << MoudleId);
        }
        else
        {
            REG32(REG_CLOCK_RST_EN) |= (1 << MoudleId);            
        }
    }

    return AK_TRUE;
}
#pragma arm section code


