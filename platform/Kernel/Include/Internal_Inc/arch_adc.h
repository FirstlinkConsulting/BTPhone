/**
 * @file    arch_adc.h
 * @brief   the interface for the AD controller
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
 
#ifndef _ARCH_ADC_
#define _ARCH_ADC_

#include "anyka_types.h"



/**
 * @brief       open the AD Controller 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID adc_open(T_VOID);


/**
 * @brief       open the AD Controller 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID adc_close(T_VOID);

/**
 * @brief       set the sample rate of the AD controller.
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   sample_rate                
 * @param[in]   channel
 * @param[in]   BitsPerSample
 * @return      T_U32 
 * @retval      the real sample rate
 */ 
T_U32 adc_setinfo(T_U32 sample_rate, T_U16 channel, T_U16 BitsPerSample);

#endif //_ARCH_ADC_


