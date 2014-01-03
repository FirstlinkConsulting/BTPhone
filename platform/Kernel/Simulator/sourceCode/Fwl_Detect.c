/*******************************************************************************
 * @file    Fwl_Detect.c
 * @brief   This file provides detector APIs: initialization, enable and so on
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @version 1.0
*******************************************************************************/
#include "fwl_detect.h"
#include "hal_detector.h"
#include "gpio_define.h"
#include "hal_int_msg.h"
#include "vme.h"
#include "M_event.h"
#include "Eng_Debug.h"
#include "arch_gpio.h"
#include "Fwl_Timer.h"
#include "Fwl_PMU.h"



/*******************************************************************************
 * @brief   initialize detector
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]mode: 
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_DetectorInit(T_U8 mode)
{
    T_BOOL ret = AK_FALSE;
    
    return ret;
}


/*******************************************************************************
 * @brief   enable or disable the detector of the specified device
 * @author  wangguotian
 * @date    2012-03-09
 * @param   [in]devId: ID of the device detected
 * @param   [in]bEnable: enable or disable
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 * @remark  It's not suggested to disable the detector,if the detector
 *          is disable, the connecting state of the device can't be 
 *          informed the user.
*******************************************************************************/
T_BOOL Fwl_DetectorEnable(T_eDEVICE_ID devId, T_BOOL bEnable)
{
    T_BOOL ret = AK_FALSE;

    return ret;
}


/*******************************************************************************
 * @brief   get the status of the specified device
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]devId: ID of the device detected
 * @param   [out]bStatus: connect or not
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_DetectorGetStatus(T_eDEVICE_ID devId)
{
    //win32 don't support detector
    return AK_FALSE;
}

T_VOID Fwl_USBMsgDeal(T_BOOL bStatus)
{

}

T_VOID Fwl_SDMsgDeal(T_BOOL bStatus)
{
    T_EVT_PARAM pEventParm;
    
    pEventParm.c.Param1 = MMC_SD_CARD;
    pEventParm.c.Param2 = bStatus;
    VME_EvtQueuePut(M_EVT_SDCARD, &pEventParm);
}

T_VOID Fwl_SpkConnectSet(T_BOOL bIsConnect)
{

}

T_VOID Fwl_HPMsgDeal(T_BOOL bStatus)
{
    if (bStatus)
    {
        if (Fwl_DetectorGetStatus(DEVICE_HP))
        {
            AK_DEBUG_OUTPUT("HP IN\n");
        }
    }
    else
    {
        if (!Fwl_DetectorGetStatus(DEVICE_HP))
        {
            AK_DEBUG_OUTPUT("HP OUT\n");
        }
    }
}

T_VOID Fwl_LINEINMsgDeal(T_BOOL bStatus)
{
    if (bStatus)
    {
        if (Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            AK_DEBUG_OUTPUT("LINE_IN IN\n");
        }
    }
    else
    {
        if (!Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            AK_DEBUG_OUTPUT("LINE_IN OUT\n");
        }
    }
}

/*******************************************************************************
 * @brief   deal device detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @param   [in]devInfo: devId and other information
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_DetectorMsgDeal(T_BOOL bStatus, T_U16 devInfo)
{
    switch (devInfo)
    {
    case DEVICE_USB:
        Fwl_USBMsgDeal(bStatus);
        break;

    case DEVICE_HP:
        Fwl_HPMsgDeal(bStatus);
        break;

    case DEVICE_LINEIN:
        Fwl_LINEINMsgDeal(bStatus);
        break;

    case DEVICE_SD:
        Fwl_SDMsgDeal(bStatus);
        break;

    default:
        break;
    }
}


