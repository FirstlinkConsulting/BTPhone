/*******************************************************************************
 * @file    Fwl_WaveOut.c
 * @brief   this file provides WaveOut APIs: 
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_WAVEOUT_H__
#define __FWL_WAVEOUT_H__


#include "anyka_types.h"


typedef enum 
{
    DAC_HP,
    LINEIN_HP,
    MIC_HP,
    MAX_WAVOUT_NUM,
}T_WAVE_OUT;

typedef enum
{
    WHPGAIN_LEVEL0, //weakly Gain
    WHPGAIN_LEVEL1, 
    WHPGAIN_LEVEL2, 
    WHPGAIN_LEVEL3, 
    WHPGAIN_LEVEL4, 
    WHPGAIN_LEVEL5, //Stronger Gain
    WHPGAIN_NUM
}WHP_GAIN_LEVEL;

typedef T_VOID (*WAVEOUT_WRITE_CB)(T_U8 **buf, T_U32 *len); 


#define L2HP_MAX_VOLUME         31
#define INVAL_WAVEOUT_ID        0xff
#define MAX_WAVEOUT_INSTANCE    5



/*******************************************************************************
 * @brief   open wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]wavIO: src, dst type
 * @param   [in]func: l2 interrupt callback function
 * @return  T_U8
 * @retval  the open id
*******************************************************************************/
T_U8 Fwl_WaveOutOpen(T_WAVE_OUT Wav_IO, WAVEOUT_WRITE_CB Func);


/*******************************************************************************
 * @brief   start wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]sampleRate: the SampleRate adc need to setted
 * @param   [in]channel: the Channel adc need to setted 
 * @param   [in]bitsPerSample: the BitsPerSample adc need to setted
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutStart(T_U32 id, T_U32 SampleRate, T_U32 Channel, T_U32 BitsPerSample);


/*******************************************************************************
 * @brief   ensure wave out l2 interrupt had start
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutEnsureSending(T_U8 id);


/*******************************************************************************
 * @brief   stop wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutStop(T_U8 id);


/*******************************************************************************
 * @brief   close wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutClose(T_U8 id);


/*******************************************************************************
 * @brief   check wave out pause or not
 * @author  lishengkai
 * @date    2013-2-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  whether pause or not
*******************************************************************************/
T_BOOL Fwl_WaveOutCheckPause(T_VOID);


/*******************************************************************************
 * @brief   check wave out dac is open or not
 * @author  lishengkai
 * @date    2013-2-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  whether dac open or not
*******************************************************************************/
T_BOOL Fwl_WaveOutDACIsOpen(T_VOID);


/*******************************************************************************
 * @brief   set wave out gain
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]gain: the gain level
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutSetGain(T_U8 id, T_U8 gain);

/*******************************************************************************
 * @brief   get da real sample rate
 * @author  aijun
 * @date    2013-2-19
 * @param  none
 * @param   none
 * @return  T_U32
 * @retval  real sample rate
*******************************************************************************/
T_U32 Fwl_GetWaveOutRealSample(T_VOID);

/*******************************************************************************
 * @brief   open or close wave out 
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]isSet: 1 for open, 0 for close
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/

T_BOOL Fwl_WaveOutSetSignal(T_WAVE_OUT wave_id, T_BOOL isSet);

#endif //__FWL_WAVEOUT_H__
