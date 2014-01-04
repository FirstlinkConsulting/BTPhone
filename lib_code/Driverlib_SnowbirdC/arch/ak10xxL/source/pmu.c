/*******************************************************************************
 * @file    pmu.c
 * @brief   this file provides pmu APIs: initialization
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-12-25
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_pmu.h"
#include "arch_gpio.h"
#include "arch_init.h"
#include "arch_efuse.h"
#include "arch_sys_ctl.h"
#include "hal_int_msg.h"
#include "pmu.h"
#include "drv_cfg.h"

#pragma arm section code ="_drvbootinit_"
/*******************************************************************************
 * @brief:  pmu initial
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_VOID
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_init(T_VOID)
{
    T_U32 reg_val;

    //BGR Trimming
    efuse_read(EFUSE_READ_VENDOR);

    reg_val = REG32(REG_PMU_CTRL1);
    reg_val |= PMU_PD_CHG;
    reg_val |= PMU_PD_CHG_OCP;
#if (SWITCH_MODE_SLIDE == 1)
    //芯片选择拨码开关方式，即电平触发
    reg_val |= PMU_ON_TRIG_SEL;
#endif
    REG32(REG_PMU_CTRL1) = reg_val;

    reg_val = REG32(REG_INT_ANALOG);
    reg_val |= INT_EN_CHG_OT;
    reg_val |= INT_EN_CHG;
    reg_val |= INT_EN_CHG_OC;
    reg_val |= INT_EN_TKL_TO;
    reg_val |= INT_EN_NML_TO;
    REG32(REG_INT_ANALOG) |= reg_val;

    REG32(REG_POWER_CTRL) |= BUCK12_SWITCH;
}
#pragma arm section code

/*******************************************************************************
 * @brief:  enable slide switch or not
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]enable AK_TRUE:use slide switch, AK_FALSE:use tact switch
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_dipswitch_enable(T_BOOL enable)
{
    T_U32 reg_val;

    reg_val = REG32(REG_PMU_CTRL1);
    if (AK_FALSE == enable)
    {
        reg_val &= ~PMU_ON_TRIG_SEL;
    }
    else
    {
        reg_val |= PMU_ON_TRIG_SEL;
    }
    REG32(REG_PMU_CTRL1) = reg_val;
}

/*******************************************************************************
 * @brief:  set DCDC12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_dcdc12_sel(CORE12_VOLTAGE voltage)
{
    T_U32 reg_val = 0;

    if (gpio_get_pin_level(USB_DETECT_GPIO))
    {
        //有USB插入的情况下，不要打开DCDC供电方式
        drv_print("cannt set DCDC mode at present!", 0, AK_TRUE);
        return AK_FALSE;
    }
    //设置DCDC模式的时候将LDO设置低一点
    reg_val = REG32(REG_PMU_CTRL1);
    reg_val &= ~(7 << PMU_LDO12_SEL);
    reg_val |= ((voltage - 1) << PMU_LDO12_SEL);
    REG32(REG_PMU_CTRL1) = reg_val;

    reg_val = REG32(REG_PMU_CTRL2);
    reg_val &= ~(7 << PMU_BUCK12_SEL);
    reg_val |= (voltage << PMU_BUCK12_SEL);
    REG32(REG_PMU_CTRL2) = reg_val;

    REG32(REG_PMU_CTRL1) &= ~(PMU_LDO12_EN | PMU_PD_BUCK12 | PMU_LDO12_SW);

    return AK_TRUE;
}

#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief:  set LDO12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_ldo12_sel(CORE12_VOLTAGE voltage)
{
    T_U32 reg_val = 0;

    reg_val = REG32(REG_PMU_CTRL1);
    reg_val &= ~(7 << PMU_LDO12_SEL);
    reg_val |= (voltage << PMU_LDO12_SEL);
    reg_val |= (PMU_LDO12_EN | PMU_PD_BUCK12 | PMU_LDO12_SW);
    REG32(REG_PMU_CTRL1) = reg_val;

    return AK_TRUE;
}
#pragma arm section code

/*******************************************************************************
 * @brief:  set CORE12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]type: CORE12_DCDC or CORE12_LDO
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_core12_sel(CORE12_TYPE type, CORE12_VOLTAGE voltage)
{
    T_BOOL ret = AK_FALSE;

    //CORE12_09V 工作会死机不给设置
    if ((CORE12_10V > voltage) || (CORE12_135V < voltage))
    {
        return AK_FALSE;
    }

    if (CHIP_1053L == drv_get_chip_type())
    {
        type = CORE12_LDO;
    }

    switch (type)
    {
    case CORE12_DCDC:
        ret = pmu_dcdc12_sel(voltage);
        break;

    case CORE12_LDO:
        ret = pmu_ldo12_sel(voltage);
        break;

    default:
        break;
    }

    return ret;
}

/*******************************************************************************
 * @brief:  set LDO33 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 2.9V-3.3V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_ldo33_sel(LDO33_VOLTAGE voltage)
{
    T_U32 reg_val = 0;

    if ((LDO33_27V > voltage) || (LDO33_36V < voltage))
    {
        return AK_FALSE;
    }

    reg_val = REG32(REG_PMU_CTRL1);
    reg_val &= ~(7 << PMU_LDO33_SEL);
    reg_val |= (voltage << PMU_LDO33_SEL);
    REG32(REG_PMU_CTRL1) = reg_val;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief:  set BOOST33 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 2.9V-3.3V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_boost33_sel(LDO33_VOLTAGE voltage)
{
    T_U32 reg_val = 0;

    if ((LDO33_27V > voltage) || (LDO33_36V < voltage))
    {
        return AK_FALSE;
    }

    reg_val = REG32(REG_PMU_CTRL2);
    reg_val |= PMU_BOOST33_2X;
    reg_val &= ~(7 << PMU_BOOST33_SEL);
    reg_val |= (voltage << PMU_BOOST33_SEL);
    REG32(REG_PMU_CTRL2) = reg_val;

    return AK_TRUE;
}

#pragma arm section code ="_drvbootinit_"
/*******************************************************************************
 * @brief:  select intel vref 1.5
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]select: can be VREF_LPBGR or VREF_BGR
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_vref15_sel(VREF_SELECT select)
{
    if (VREF_BGR == select)
    {
        REG32(REG_ADC1_CFG)  |= VREF1V5_BGR;
        REG32(REG_PMU_CTRL1) |= PMU_VREF_SEL;
    }
    else
    {
        REG32(REG_PMU_CTRL1) &= ~PMU_VREF_SEL;
    }

    return AK_TRUE;
}
#pragma arm section code

/*******************************************************************************
 * @brief:  configurate trickle charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]time
 * @param:  [in]voltage
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_trickle(TRICKLE_TIME time, TRICKLE_VOLTAGE voltage)
{
    T_U32 reg_val;

    reg_val = REG32(REG_PMU_CTRL1);
    if (TKL_0H5 == time)
    {
        reg_val &= ~PMU_TKL_TIME_SEL;
    }
    else
    {
        reg_val |= PMU_TKL_TIME_SEL;
    }

    reg_val &= ~(3 << PMU_TKL_VOLT_SEL);
    reg_val |= (voltage & 3) << PMU_TKL_VOLT_SEL;

    REG32(REG_PMU_CTRL1) = reg_val;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief:  configurate normal charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]time
 * @param:  [in]current
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_normal(CHARGE_TIME time, CHARGE_CURRENT current)
{
    T_U32 reg_val;

    reg_val = REG32(REG_PMU_CTRL1);
    reg_val &= ~((3 << PMU_NML_TIME_SEL) | (7 << PMU_CHG_CURR_SEL));
    reg_val |= (time & 3) << PMU_NML_TIME_SEL;
    reg_val |= (current & 7) << PMU_CHG_CURR_SEL;
    REG32(REG_PMU_CTRL1) = reg_val;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief:  configurate terminate charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_terminate(TERMINATE_VOLTAGE voltage)
{
    T_U32 reg_val;

    reg_val = REG32(REG_PMU_CTRL1);
    reg_val &= ~(3 << PMU_CHG_SD_SEL);
    reg_val |= (voltage & 3) << PMU_CHG_SD_SEL;
    REG32(REG_PMU_CTRL1) = reg_val;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief:  enable charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_BOOL AK_TRUE:enable charge, AK_FALSE:disable charge
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_charge_enable(T_BOOL enable)
{
    T_U32 reg_val;

    reg_val = REG32(REG_PMU_CTRL1);
    if (AK_FALSE == enable)
    {
        reg_val |= PMU_PD_CHG;
        //reg_val |= PMU_PD_CHG_OCP;
    }
    else
    {
        reg_val &= ~PMU_PD_CHG;
        //reg_val &= ~PMU_PD_CHG_OCP;
    }
    REG32(REG_PMU_CTRL1) = reg_val;
}

/*******************************************************************************
 * @brief:  get charger status
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_VOID
 * @return: T_BOOL
 * @retval: AK_TRUE :in charge status
 * @retval: AK_FALSE:not in charge status
*******************************************************************************/
T_BOOL pmu_charge_status(T_VOID)
{
    return (REG32(REG_INT_ANALOG) & ALG_STA_CHG) ? AK_TRUE : AK_FALSE;
}


