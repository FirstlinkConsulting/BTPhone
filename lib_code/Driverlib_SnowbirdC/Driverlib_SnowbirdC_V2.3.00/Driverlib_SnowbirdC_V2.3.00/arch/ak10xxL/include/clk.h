/*******************************************************************************
 * @FILENAME: clk.h
 * @BRIEF clk driver head file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR zhanggaoxin
 * @DATE 2012-11-20
 * @VERSION 1.0
 * @REF
*******************************************************************************/
#ifndef __CLK_H__
#define __CLK_H__


#include "anyka_types.h"
#include "drv_cfg.h"


#define SleepMode_enable    0x7F
#define SleepMode_disable   0x77


#define CLK_PLL_MIN         180
#define CLK_PLL_MAX         432


#if (CHIP_SEL_10C > 0)
#define DEF_PLL_VAL         228000000
#define MAX_ASIC_VAL        114000000
#else
#define DEF_PLL_VAL         232000000
#define MAX_ASIC_VAL        116000000
#endif
#define MAX_ACLK_VAL        200000000


#define CLK_NFC_EN          (1 << 0)
#define CLK_MCI_EN          (1 << 1)
#define CLK_SPI_EN          (1 << 2)


typedef enum
{
    eVME_LCD_CLK = 0,
    eVME_MCI1_CLK,
    eVME_NANDFLASH_CLK,
    eVME_UART_CLK,
    eVME_USB_CLK,
    eVME_DAC_CLK,
    eVME_ADC_CLK,
    eVME_MEMORY_ARM_CLK,
    eVME_SPI1_CLK,
    eVME_MCI2_CLK,
    eVME_CAMERA_CLK,
    eVME_PCM_CLK,
    eVME_SPI2_CLK,
    eVME_SleepMode      //< it is the middle state between enable and disable
} vT_ModuleList;


/*******************************************************************************
 * @brief   set PLL frequency value.
 * main clock is controlled by pll register.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   pll_value [in] pll frequency value(Hz)
 * @return  T_BOOL
 * @retval  AK_TRUE  set pll frequency successful
 * @retval  AK_FALSE set pll frequency unsuccessful
*******************************************************************************/
T_BOOL clk_set_pll(T_U32 pll_value);


/*******************************************************************************
 * @brief   get current system pll.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   T_VOID
 * @return  T_U32
 * @retval  pll frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_pll(T_VOID);


/*******************************************************************************
 * @brief   get current system clk168M.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   T_VOID
 * @return  T_U32
 * @retval  clk168M frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_clk168m(T_VOID);


/*******************************************************************************
 * @brief   get current aclk.
 * @author  wangguotian
 * @date    2012-12-21
 * @param   T_VOID
 * @return  T_U32
 * @retval  aclk frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_aclk(T_VOID);


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
T_BOOL clk_adjust(T_U32 freq, T_U32 bottom_asic);


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
T_BOOL clk_set_asic(T_U32 mclk_div, T_U32 asic_div);


/*******************************************************************************
 * @brief   get the asic freq.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   void
 * @return  T_U32
 * @retval  asic frequency value(Hz)
*******************************************************************************/
T_U32 clk_get_asic(T_VOID);


/*******************************************************************************
 * @brief   set cpu frequency twice of asic frequency or not
 * this function just set cpu_clk = PLL1_clk or set cpu_clk = asic_clk
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]set_value  set twice or not
 * @return  T_VOID
*******************************************************************************/
T_VOID clk_set_cpu2x(T_BOOL set_value);


/*******************************************************************************
 * @brief   judge whether cpu frequency is twice of asic frequency or not
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @return  T_BOOL
 * @retval  AK_TRUE cpu frequency is twice of asic frequency
 * @retval  AK_FALSE cpu frequency is not twice of asic frequency
*******************************************************************************/
T_BOOL clk_get_cpu2x(T_VOID);


/*******************************************************************************
 * @brief   reset different module in ASIC
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]vT_ModuleList MoudleId
 * @return  T_BOOL
 * @retval
*******************************************************************************/
T_BOOL sys_module_reset(vT_ModuleList MoudleId);


/*******************************************************************************
 * @brief   enable or disable module clock.
 * @author  zhanggaoxin
 * @date    2012-12-13
 * @param   [in]vT_ModuleList MoudleId
 * @param   [in]T_BOOL EnableFlag
 * @return  T_BOOL
 * @retval
*******************************************************************************/
T_BOOL sys_module_enable(vT_ModuleList MoudleId, T_BOOL EnableFlag);


#endif

