/*******************************************************************************
 * @file    init.c
 * @brief   init module
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  xuchang
 * @date    2007.10.23
 * @version 1.0
 * @ref
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_init.h"
#include "timer.h"
#include "gpio.h"
#include "clk.h"
#include "pmu.h"
#include "hal_detector.h"
#include "analog.h"
#include "drv_cfg.h"
#include "drv_api.h"
#include "arch_uart.h"


#ifdef __GNUC__
extern int __initcall_start, __initcall_end;
static int *initcall_start=&__initcall_start, *initcall_end=&__initcall_end;
#endif
#ifdef __CC_ARM
extern int Image$$init$$Base, Image$$init$$Limit;
static int *initcall_start=&Image$$init$$Base, *initcall_end=&Image$$init$$Limit;
#endif

#pragma arm section zidata = "_drvbootbss_"
static T_DRIVE_INITINFO g_drv_info;
#pragma arm section

static const T_CHR VERSION[] = _DRV_LIB_VER;

#pragma arm section code = "_sysinit_"
void do_initcalls(void)
{
    initcall_t *call;
    int *addr;

    for (addr=initcall_start; addr<initcall_end; addr++) 
    {
        call = (initcall_t *)addr;
        (*call)();
    }
}


/*******************************************************************************
 * @brief   driver library initialization
 *
 * should be called on start-up step, to initial interrupt module 
 * and register hardware as camera, lcd...etc.
 * @author  xuchang
 * @date    2008-01-21
 * @return  T_VOID
*******************************************************************************/
T_VOID drv_init(T_PDRIVE_INITINFO drv_info)
{
    memcpy(&g_drv_info, drv_info, sizeof(g_drv_info));

    clk_set_pll(DEF_PLL_VAL);
#if (CHIP_SEL_10C > 0 && DRV_SIMU_UART == 0)
    uart_init(uiUART2, 115200);
#endif
    pmu_init();
    timer_init();
    gpio_init();
#ifndef BURN_TOOL
    analog_init();
    detector_init();
#endif
    //l2_initial();
#if !(USB_VAR_MALLOC > 0)
    DrvModule_Init();
#endif
    //device module registeration init
    do_initcalls();
    drv_print(VERSION, 0, AK_TRUE);

    //close VDDIO3V3 and VDD1V2 unstabitily detect
    //REG32(REG_PMU_CTRL1) |= (PMU_PD_PWR_VCC | PMU_PD_PWR_VDD);

#if DRV_SUPPORT_CAMERA > 0
#if (CAMERA_GC6113 > 0)
    camera_gc6113_reg();
#endif
#if (CAMERA_GC0308 > 0)
    camera_gc0308_reg();
#endif

#endif
#if (CHIP_SEL_10C > 0)
	//pwm32k 是给RDA芯片用的，如果FM工作不好，用32K晶振试下
    pwm_start_32k_square();
#endif
}
#pragma arm section code

/*******************************************************************************
 * @brief   memory alloc
 *
 * @author  liao_zhijun
 * @date    2010-07-15
 * @param   [in]size size of memory to alloc
 * @return  T_pVOID
*******************************************************************************/
T_pVOID drv_malloc(T_U32 size)
{
    if(AK_NULL == g_drv_info.fRamAlloc)
    {   
        return AK_NULL;
    }

    return g_drv_info.fRamAlloc(size);
}


/*******************************************************************************
 * @brief   memory free
 *
 * @author  liao_zhijun
 * @date    2010-07-15
 * @param   [in]var address of memory to free
 * @return  T_pVOID
*******************************************************************************/
T_pVOID drv_free(T_pVOID var) 
{
    if(AK_NULL == g_drv_info.fRamFree)
        return AK_NULL;

    return g_drv_info.fRamFree(var);
}  


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   print func
 *
 * @author  liao_zhijun
 * @date    2010-07-15
 * @param   [in]s string to print
 * @param   [in]n number to print 
 * @param   [in]newline AK_TRUE: add '\n' to string, AK_FALSE: donot add
 * @return  T_VOID
*******************************************************************************/
T_VOID drv_print(const T_CHR *s, T_U32 n, T_BOOL newline)
{
    if(AK_NULL == g_drv_info.fPrint)
        return;

    g_drv_info.fPrint(s, n, newline);
}


/*******************************************************************************
 * @brief   get chip type
 *
 * @author  liao_zhijun
 * @date    2010-07-15
 * @param   T_VOID
 * @return  E_AKCHIP_TYPE
*******************************************************************************/
E_AKCHIP_TYPE drv_get_chip_type(T_VOID)
{
    return g_drv_info.chip;
}

#pragma arm section code

