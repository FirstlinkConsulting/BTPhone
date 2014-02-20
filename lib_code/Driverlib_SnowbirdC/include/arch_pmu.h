/*******************************************************************************
 * @file arch_pmu.h
 * @brief pmu function header file
 * This file provides pmu APIs: initialization
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author zhanggaoxin
 * @date 2012-12-25
 * @version 1.0
 * @ref AK1080L technical manual.
*******************************************************************************/
#ifndef __ARCH_PMU_H__
#define __ARCH_PMU_H__


#include "anyka_types.h"


#define PMU_OVER_TEMP       (1<<7)  //over temperature of charge
#define PMU_CHG             (1<<8)  //charge start or stop
#define PMU_OVER_CURR       (1<<9)  //over current of charge
#define PMU_TKL_TMOUT       (1<<12) //trickle charge timeout
#define PMU_NML_TMOUT       (1<<13) //normal charge timeout


typedef enum
{
    CORE12_DCDC,
    CORE12_LDO
} CORE12_TYPE;

typedef enum
{
    CORE12_09V = 0,
    CORE12_10V,
    CORE12_11V,
    CORE12_115V,
    CORE12_12V,
    CORE12_125V,
    CORE12_13V,
    CORE12_135V
} CORE12_VOLTAGE;//supply power for AK1180 core(device)

typedef enum
{
    LDO33_27V = 0,
    LDO33_28V,
    LDO33_29V,
    LDO33_30V,
    LDO33_31V,
    LDO33_32V,
    LDO33_33V,
    LDO33_36V
} LDO33_VOLTAGE;//supply power for I/Os

typedef enum
{
    VREF_LPBGR = 0,
    VREF_BGR
} VREF_SELECT;

typedef enum
{
    CHG_100MA = 0,
    CHG_150MA,
    CHG_200MA,
    CHG_300MA,
    CHG_350MA,
    CHG_400MA,
    CHG_450MA,
    CHG_500MA
} CHARGE_CURRENT;

typedef enum
{
    CHG_3H = 0,
    CHG_3H5,
    CHG_4H,
    CHG_5H
} CHARGE_TIME;

typedef enum
{
    TKL_2V92 = 0,
    TKL_2V97,
    TKL_3V00,
    TKL_3V04
} TRICKLE_VOLTAGE;

typedef enum
{
    TKL_0H5 = 0,
    TKL_1H
} TRICKLE_TIME;

typedef enum
{
    TER_150MV = 0,
    TER_200MV,
    TER_250MV,
    TER_300MV
} TERMINATE_VOLTAGE;


/*******************************************************************************
 * @brief:  enable slide switch or not
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]enable AK_TRUE:use slide switch, AK_FALSE:use tact switch
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_dipswitch_enable(T_BOOL enable);


/*******************************************************************************
 * @brief:  set CORE12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]type: CORE12_DCDC or CORE12_LDO
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_core12_sel(CORE12_TYPE type, CORE12_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  set DCDC12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_dcdc12_sel(CORE12_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  set LDO12 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 0.9V-1.35V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_ldo12_sel(CORE12_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  set LDO33 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 2.9V-3.3V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_ldo33_sel(LDO33_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  set BOOST33 output voltage
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage: 2.9V-3.3V
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_boost33_sel(LDO33_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  select intel vref 1.5
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]select can be VREF_LPBGR or VREF_BGR
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_vref15_sel(VREF_SELECT select);


/*******************************************************************************
 * @brief:  configurate trickle charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]time
 * @param:  [in]voltage
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_trickle(TRICKLE_TIME time, TRICKLE_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  configurate normal charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]time
 * @param:  [in]current
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_normal(CHARGE_TIME time, CHARGE_CURRENT current);


/*******************************************************************************
 * @brief:  configurate terminate charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  [in]voltage
 * @return: T_BOOL
*******************************************************************************/
T_BOOL pmu_charge_terminate(TERMINATE_VOLTAGE voltage);


/*******************************************************************************
 * @brief:  enable charger
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_BOOL AK_TRUE:enable charge, AK_FALSE:disable charge
 * @return: T_VOID
*******************************************************************************/
T_VOID pmu_charge_enable(T_BOOL enable);


/*******************************************************************************
 * @brief:  get charger status
 * @author: zhanggaoxin
 * @date:   2012-12-25
 * @param:  T_VOID
 * @return: T_BOOL
 * @retval: AK_TRUE :in charge status
 * @retval: AK_FALSE:not in charge status
*******************************************************************************/
T_BOOL pmu_charge_status(T_VOID);


#endif

