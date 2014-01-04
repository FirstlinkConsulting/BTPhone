/*******************************************************************************
 * @file    Fwl_WaveIn.c
 * @brief   this file provides WaveIn APIs: 
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_WAVEIN_H__
#define __FWL_WAVEIN_H__


#include "anyka_types.h"


typedef enum 
{
    MIC_ADC,
    LINEIN_ADC,
    DAC_ADC,
    MAX_WAVIN_NUM,
}T_WAVE_IN;


typedef T_VOID (*WAVEIN_WRITE_CB)(T_U8 **buf, T_U32 *len); 


#define INVAL_WAVEIN_ID     0xff
#define MAX_MIC_GAIN        10
#define MAX_LINEIN_GAIN     15



/*******************************************************************************
 * @brief   open wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]wavIO: src, dst type
 * @param   [in]trans_mode: AK_FALSE :CPU OR AK_TRUE:DMA  
 * @param   [in]func: l2 interrupt callback function
 * @return  T_U8
 * @retval  the open id
*******************************************************************************/
T_U8 Fwl_WaveInOpen(T_WAVE_IN Wav_IO, WAVEIN_WRITE_CB Func, T_U8 trans_mode);


/*******************************************************************************
 * @brief   start wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]sampleRate: the SampleRate adc need to setted
 * @param   [in]channel: the Channel adc need to setted 
 * @param   [in]bitsPerSample: the BitsPerSample adc need to setted
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInStart(T_U32 id, T_U32 SampleRate, T_U32 Channel, T_U32 BitsPerSample);


/*******************************************************************************
 * @brief   ensure wave in l2 interrupt had start
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInEnsureSending(T_U8 id);


/*******************************************************************************
 * @brief   stop wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInStop(T_U8 id);


/*******************************************************************************
 * @brief   close wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInClose(T_U8 id);


/*******************************************************************************
 * @brief   set wave in gain
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]gain: the gain level
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInSetGain(T_U8 id, T_U8 gain);

/*******************************************************************************
 * @brief   get ad real sample rate
 * @author  aijun
 * @date    2013-2-19
 * @param  none
 * @param   none
 * @return  T_U32
 * @retval  real sample rate
*******************************************************************************/

T_U32 Fwl_GetWaveInRealSample(T_VOID);

#endif //__FWL_WAVEIN_H__

