/*******************************************************************************
 * @file    Fwl_WaveOut.c
 * @brief   this file provides WaveOut APIs: 
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#include "Fwl_WaveOut.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "arch_dac.h"
#include "arch_analog.h"
#include "hal_l2.h"
#include "w_audioplay.h"



typedef struct
{
    T_WAVE_OUT          Wav_IO;
    WAVEOUT_WRITE_CB    Func;
}DA_INFO, *P_DA_INFO;

typedef enum
{
    DMA_REQUEST = 0x00,
    DMA_SENDING
}WAVEOUT_SEND_DATA_STATE;

typedef struct {
    T_BOOL              bDACOpen;
    T_U8                HPOpenNum;
    volatile T_U8       flag;
    T_U8                cur_id;
    WAVEOUT_WRITE_CB    CBufFunc;
}T_WAVE_STRUCT;


static const T_U8 L2HpVolTab[L2HP_MAX_VOLUME + 1][2]={
    {0, WHPGAIN_LEVEL0},  //这个设置无效
    {0, WHPGAIN_LEVEL3},  
    {1, WHPGAIN_LEVEL5}, 
    {3, WHPGAIN_LEVEL4}, 
    {4, WHPGAIN_LEVEL4},  
    {5, WHPGAIN_LEVEL4}, 
    {5, WHPGAIN_LEVEL5}, 
    {6, WHPGAIN_LEVEL4},
    {7, WHPGAIN_LEVEL4},  
    {8, WHPGAIN_LEVEL3},  
    {7, WHPGAIN_LEVEL5}, 
    {8, WHPGAIN_LEVEL4},  
    {9, WHPGAIN_LEVEL3}, 
    {8, WHPGAIN_LEVEL5}, 
    {9, WHPGAIN_LEVEL4}, 
    {10, WHPGAIN_LEVEL3},  
    {9, WHPGAIN_LEVEL5} ,
    {10, WHPGAIN_LEVEL4}, 
    {10, WHPGAIN_LEVEL5}, 
    {11, WHPGAIN_LEVEL4}, 
    {11, WHPGAIN_LEVEL5},  
    {12, WHPGAIN_LEVEL4},  
    {13, WHPGAIN_LEVEL3}, 
    {12, WHPGAIN_LEVEL5}, 
    {13, WHPGAIN_LEVEL4},  
    {14, WHPGAIN_LEVEL3}, 
    {13, WHPGAIN_LEVEL5}, 
    {14, WHPGAIN_LEVEL4}, 
    {15, WHPGAIN_LEVEL3}, 
    {14, WHPGAIN_LEVEL5}, 
    {15, WHPGAIN_LEVEL4}, 
    {15, WHPGAIN_LEVEL5}
};

volatile T_WAVE_STRUCT cur_wav_info = {AK_FALSE, 0, 0, INVAL_WAVEOUT_ID, AK_NULL};

#define SRC_GAIN(vol)           L2HpVolTab[(vol)][0]
#define DST_GAIN(vol)           L2HpVolTab[(vol)][1]

#define WAVEOUT_FLAG            cur_wav_info.flag
#define COUNT_TIME              500000

#define WAVEOUT_SET_SENDFLAG(flag, type, val) ((val)? ((flag) |= (1 << (type))) : ((flag) &= ~(1 << (type))))
#define WAVEOUT_GET_SENDFLAG(flag, type) ((flag) & (1 << type))

#pragma arm section rodata = "_waveout_data_"

const static T_U8 wav_io_info[MAX_WAVOUT_NUM][2] = {
    {SRC_DAC, DST_HP}, 
    {SRC_LINEIN, DST_HP},
    {SRC_MIC, DST_HP}
};

static DA_INFO *da_info_buf[MAX_WAVEOUT_INSTANCE] = {AK_NULL};
static T_BOOL bStartFlag = AK_FALSE;
#pragma arm section rodata
extern T_VOID Pcm_CBFun(T_U8 **buf, T_U32 *len);

/*******************************************************************************
 * @brief   dma default callback
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]buf: dma data buffer
 * @param   [in]len: length of this data buffer
 * @return  T_VOID
*******************************************************************************/
T_VOID Wave_DefaultCB(T_U8 **buf, T_U32 *len)
{    
	T_U32   datalen = 0;
    T_U8*   pDMABuf = AK_NULL;

	if (bStartFlag)
	{
		Pcm_CBFun(&pDMABuf, &datalen);

		if ((AK_NULL != pDMABuf) && (datalen > 0))
	    {
	        Audio_WINWrite((T_U16 *)pDMABuf, datalen>>1);
	    }
		if(len)
			*len = datalen;
	}
}


/*******************************************************************************
 * @brief   check wave out dac is open or not
 * @author  lishengkai
 * @date    2013-2-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  whether dac open or not
*******************************************************************************/
T_BOOL Fwl_WaveOutDACIsOpen(T_VOID)
{
    //return cur_wav_info.bDACOpen;
	T_BOOL ret = AK_FALSE;
    return ret;
}


/*******************************************************************************
 * @brief   open wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]wavIO: src, dst type
 * @param   [in]func: l2 interrupt callback function
 * @return  T_U8
 * @retval  the open id
*******************************************************************************/
T_U8 Fwl_WaveOutOpen(T_WAVE_OUT Wav_IO, WAVEOUT_WRITE_CB Func)
{    
    return 1;
}


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
T_BOOL Fwl_WaveOutStart(T_U32 id, T_U32 SampleRate, T_U32 Channel, T_U32 BitsPerSample)
{
    T_BOOL ret = AK_TRUE;
	bStartFlag = AK_TRUE;
    return ret;
}


/*******************************************************************************
 * @brief   check wave out pause or not
 * @author  lishengkai
 * @date    2013-2-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  whether pause or not
*******************************************************************************/
T_BOOL Fwl_WaveOutCheckPause(T_VOID)
{
    T_BOOL ret = AK_FALSE;
    return ret;
}


/*******************************************************************************
 * @brief   ensure wave out l2 interrupt had start
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutEnsureSending(T_U8 id)
{
    T_BOOL ret = AK_FALSE;
    return ret;
}


/*******************************************************************************
 * @brief   stop wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutStop(T_U8 id)
{
    T_BOOL ret = AK_FALSE;
	bStartFlag = AK_FALSE;
    return ret;
}


/*******************************************************************************
 * @brief   close wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutClose(T_U8 id)
{
    T_BOOL ret = AK_FALSE;
    return ret;
}
/*******************************************************************************
 * @brief   set wave out gain level
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: 
 * @param   [in]gain: 
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveOutSetGain(T_U8 id, T_U8 gain)
{
    T_BOOL ret = AK_FALSE;
    return ret;
}


/*******************************************************************************
 * @brief   get da real sample rate
 * @author  aijun
 * @date    2013-2-19
 * @param  none
 * @param   none
 * @return  T_U32
 * @retval  real sample rate
*******************************************************************************/
T_U32 WaveOutRealSample = 0;
T_U32 Fwl_GetWaveOutRealSample(T_VOID)
{
	return WaveOutRealSample;
}

