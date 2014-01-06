/**
 * @file i2s_device.h
 * @brief I2S device header file
 * This file provides all the APIs of I2S device, such as initial I2S, turn on/off I2S.
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ChenWeiwen
 * @date 2009-11-02
 * @version 1.0
 * @ref AK1080 technical manual.
 */
#ifndef __I2S_DEVICE_H__
#define __I2S_DEVICE_H__


#include "anyka_types.h"
#include "i2s.h"
#include "arch_i2c.h"


T_VOID I2S_Device_Init (T_VOID);


T_VOID I2S_Device_Open (T_VOID);


T_VOID I2S_Device_Close (T_VOID);


T_VOID I2S_Device_SelectInput(AUDIO_AD_STATE ad_sta);


/*********************************************************************
  Function:     I2S
  Description:      
  Input:            
  Return:           
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_U32 I2S_Device_SetInfo(ADDA_INFO *info);


/*********************************************************************
  Function:     I2S
  Description:      
  Input:            
  Return:           
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Device_SetHeadPhone(HP_STATE hp_sta);


T_VOID I2S_Device_SetDACGain (HP_GAIN_LEVEL gain);


/*********************************************************************
  Function:     I2S driver
  Description:  set the codec adc gain
  Input:        ad_sta: linein or mic
                gain: HP_GAIN_LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Device_SetADCGain (AUDIO_AD_STATE ad_sta, T_U8 gain);

#endif
