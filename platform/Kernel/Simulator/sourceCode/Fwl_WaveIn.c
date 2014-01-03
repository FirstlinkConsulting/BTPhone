/*******************************************************************************
 * @file    Fwl_WaveIn.c
 * @brief   this file provides WaveIn APIs: 
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liangxiong
 * @date    2012-11-20
 * @version 1.0
*******************************************************************************/
#include "Fwl_WaveIn.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "arch_dac.h"
#include "arch_adc.h"
#include "arch_analog.h"
#include "hal_l2.h"
#include <stdio.h>
#include <string.h>
#include "Fwl_Timer.h"

extern T_VOID Record_CBFun(T_U8 **buf, T_U32 *len);


typedef struct {
    T_WAVE_IN           Wav_IO;
    WAVEIN_WRITE_CB     Func;
}AD_INFO, *P_AD_INFO;

typedef enum {
    WAVIN_REQUEST = 0x00,
    WAVIN_SENDING
}WAVEIN_SEND_DATA_STATE;

typedef struct {
    T_BOOL          bADCOpen;
    T_U8            flag;
    T_U8            cur_id;
    WAVEIN_WRITE_CB CBufFunc;
}T_WAVEIN_STRUCT;


volatile T_WAVEIN_STRUCT cur_wavin_info = {AK_FALSE, 0, INVAL_WAVEIN_ID, AK_NULL};


#define WAVEIN_FLAG             cur_wavin_info.flag
#define COUNT_TIME              500000

#define MAX_WAVEIN_INSTANCE     5
#define WAVEIN_SET_SENDFLAG(flag, type, val) ((val)? ((flag) |= (1 << (type))) : ((flag) &= ~(1 << (type))))
#define WAVEIN_GET_SENDFLAG(flag, type) ((flag) & (1 << type))


const T_U8 wavin_io_info[MAX_WAVIN_NUM][2] = {{SRC_MIC, DST_ADC}, {SRC_LINEIN, DST_ADC},{SRC_DAC, DST_ADC}};
/*
T_U8 wav_io_info[MAX_WAVOUT_NUM][2] = {{SRC_DAC, DST_HP}, {SRC_DAC, DST_LINEOUT}, {SRC_DAC, DST_ADC}\
                        ,{SRC_LINEIN, DST_HP}, {SRC_LINEIN, DST_LINEOUT}, {SRC_LINEIN, DST_ADC}\
                        ,{SRC_MIC, DST_HP}, {SRC_MIC, DST_LINEOUT}, {SRC_MIC, DST_ADC}};
*/

AD_INFO *ad_info_buf[MAX_WAVEIN_INSTANCE] = {AK_NULL};


/*******************************************************************************
 * @brief   dma default callback
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]buf: dma data buffer
 * @param   [in]len: length of this data buffer
 * @return  T_VOID
*******************************************************************************/
FILE* hPhonefile = 0;
T_TIMER recTimer = ERROR_TIMER;
T_VOID WaveIn_DefaultCB(T_U8 **buf, T_U32 *len)
{   
	T_U32   data_len;
    T_U8*   pDMABuf = AK_NULL;
	T_U8 data[320];
	
	*len = 160;
	Record_CBFun(&pDMABuf, len);

	if ((AK_NULL != pDMABuf) && (*len > 0))
    {
        if(hPhonefile)
		{
			data_len = fread(data,1,(*len)<<1,hPhonefile);
			Pcm_Format(pDMABuf, data,data_len>>2);
		}
    }

}
T_VOID WaveIn_RecCB(T_TIMER timer_id, T_U32 delay)
{
	T_U8 *buf;
	T_U32 len;

	WaveIn_DefaultCB(&buf, &len);
}

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
extern void Fwl_GetCurrentDirectory(char *CurProjPath);
T_U8 Fwl_WaveInOpen(T_WAVE_IN Wav_IO, WAVEIN_WRITE_CB Func, T_U8 trans_mode)
{    
	char CurProjPath[256] = {0};
	Fwl_GetCurrentDirectory(CurProjPath);
	strcat(CurProjPath,"\\PhoneData.pcm");
	
	if(AK_NULL == hPhonefile)
	{
		hPhonefile = fopen(CurProjPath,"rb");
	}
	recTimer = Fwl_TimerStart(10,AK_TRUE,WaveIn_RecCB);

    return 1;
}

T_U32 WaveINRealSample = 0;

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
T_BOOL Fwl_WaveInStart(T_U32 id, T_U32 SampleRate, T_U32 Channel, T_U32 BitsPerSample)
{
    T_BOOL ret = AK_TRUE;
	
	WaveINRealSample = SampleRate;
    return ret;
}

/*******************************************************************************
 * @brief   get ad real sample rate
 * @author  aijun
 * @date    2013-2-19
 * @param  none
 * @param   none
 * @return  T_U32
 * @retval  real sample rate
*******************************************************************************/

T_U32 Fwl_GetWaveInRealSample()
{
	return WaveINRealSample;
}


/*******************************************************************************
 * @brief   ensure wave in l2 interrupt had start
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInEnsureSending(T_U8 id)
{   
    T_BOOL ret = AK_TRUE;
    return ret;
}


/*******************************************************************************
 * @brief   pause wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
static T_BOOL WaveIn_Pause(T_U8 id)
{
    T_BOOL ret = AK_TRUE;
    return ret;
}


/*******************************************************************************
 * @brief   stop wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInStop(T_U8 id)
{
    T_BOOL ret = AK_TRUE;
    return ret;
}


/*******************************************************************************
 * @brief   close wave in
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInClose(T_U8 id)
{
    T_BOOL ret = AK_TRUE;
	if(AK_NULL != hPhonefile)
	{
		fclose(hPhonefile);
	}
	hPhonefile = AK_NULL;
	if(ERROR_TIMER != recTimer)
	{
		Fwl_TimerStop(recTimer);
	}
	recTimer = ERROR_TIMER;
    return ret;
}


/*******************************************************************************
 * @brief   set wave in gain
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]gain: the gain level
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_WaveInSetGain(T_U8 id, T_U8 gain)
{
    T_BOOL ret = AK_TRUE;
    return ret;
}




