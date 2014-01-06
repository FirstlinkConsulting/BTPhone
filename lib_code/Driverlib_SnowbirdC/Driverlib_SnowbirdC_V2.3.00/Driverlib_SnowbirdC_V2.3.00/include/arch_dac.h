/**
 * @file    arch_dac.h
 * @brief   the interface for the DA controller
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
 
#ifndef _ARCH_DAC_
#define _ARCH_DAC_

#include "anyka_types.h"



/**
 * @brief       open the DA Controller 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID dac_open(T_VOID);


/**
 * @brief       open the DA Controller 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID dac_close(T_VOID);

/**
 * @brief       set the sample rate of the DA controller.
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   sample_rate                
 * @param[in]   channel
 * @param[in]   BitsPerSample
 * @return      T_U32 
 * @retval      the real sample rate
 */ 
T_U32 dac_setinfo(T_U32 sample_rate, T_U16 channel, T_U16 BitsPerSample);

#endif //_ARCH_DAC_

