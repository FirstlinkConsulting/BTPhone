/*******************************************************************************
 * @file    Fwl_Detect.h
 * @brief   detect function header file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_DETECT_H__
#define __FWL_DETECT_H__


#include "anyka_types.h"
#include "Fwl_Mount.h"


typedef enum
{
    DEVICE_USB   = (1 << 0),    //PC设备
    DEVICE_CHG   = (1 << 1),    //充电线
    DEVICE_SD    = (1 << 2),    //SD卡
    DEVICE_HP    = (1 << 4),    //耳机
    DEVICE_LINEIN = (1 << 5),   //LINE IN线
    DEVICE_UHOST = (1 << 6),    //U盘
} T_eDEVICE_ID;


//usb cable status
#define USB_CHECKING                3   //checking usb cable connect pc or charge
#define USB_CNNT_CHG                2   //usb cable connect pc
#define USB_CNNT_PC                 1   //usb cable connect charge
#define USB_CNNT_NOT                0   //usb cable don't connect

#define USB_DETECT_LCD_LOCK         0xFE

#define SD_DET_PUSH_IN              1
#define SD_DET_PULL_OUT             0



/*******************************************************************************
 * @brief   initialize detector
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]mode: 
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_DetectorInit(T_U8 mode);


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
T_BOOL Fwl_DetectorEnable(T_eDEVICE_ID devId, T_BOOL bEnable);


/*******************************************************************************
 * @brief   get the status of the specified device
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]devId: ID of the device detected
 * @return  T_BOOL
 * @retval  connect or not
*******************************************************************************/
T_BOOL Fwl_DetectorGetStatus(T_eDEVICE_ID devId);


/*******************************************************************************
 * @brief   set LoudSpeaker connect
 * @author  
 * @date    
 * @param   [in]bIsConnect: connect or not
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SpkConnectSet(T_BOOL bIsConnect);


#endif //__FWL_DETECT_H__
