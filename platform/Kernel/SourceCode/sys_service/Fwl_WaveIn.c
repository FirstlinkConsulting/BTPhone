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


#pragma arm section rwdata = "_cachedata_"
volatile T_WAVEIN_STRUCT cur_wavin_info = {AK_FALSE, 0, INVAL_WAVEIN_ID, AK_NULL};
#pragma arm section rwdata


#define WAVEIN_FLAG             cur_wavin_info.flag
#define COUNT_TIME              500000
#define DEFAULT_INTERN          5
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
T_BOOL dma_trans_mode = AK_FALSE;

#pragma arm section code = "_bootcode1_"
/*******************************************************************************
 * @brief   dma default callback
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]buf: dma data buffer
 * @param   [in]len: length of this data buffer
 * @return  T_VOID
*******************************************************************************/
T_VOID WaveIn_DefaultCB(T_U8 **buf, T_U32 *len)
{
    if (cur_wavin_info.CBufFunc != AK_NULL)
    {
        cur_wavin_info.CBufFunc(buf, len);
    }
    if(dma_trans_mode == AK_TRUE)
    {
	    if (*buf == AK_NULL || *len == 0)
	    {
	        WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING, AK_FALSE);
	    }

	    if (!WAVEIN_GET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING))
	    {
	        *buf = AK_NULL ;
	        *len = 0;
	    }
    }
}
#pragma arm section code


#pragma arm section code = "_wave_in_"
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
T_U8 Fwl_WaveInOpen(T_WAVE_IN Wav_IO, WAVEIN_WRITE_CB Func, T_U8 trans_mode)
{
    T_U8 i;
    T_U32 wav_in, wav_out;

    if ((Wav_IO >= MAX_WAVIN_NUM)||(AK_NULL == Func))
        return INVAL_WAVEIN_ID;

    //find a valided id
    for(i=0; i<MAX_WAVEIN_INSTANCE; i++)
    {
        if(AK_NULL == ad_info_buf[i])
            break;
    }

    if (i >= MAX_WAVEIN_INSTANCE)
        return INVAL_WAVEIN_ID;

    ad_info_buf[i] = (AD_INFO*)Fwl_Malloc(sizeof(AD_INFO));

    if (AK_NULL == ad_info_buf[i])
    {
        akerror("wav instance malloc false!", 0, 1);
        return INVAL_WAVEIN_ID;
    }

    ad_info_buf[i]->Wav_IO = Wav_IO;
    ad_info_buf[i]->Func = Func;

    //open adc
    if (!cur_wavin_info.bADCOpen)
    {
        adc_open();
        cur_wavin_info.bADCOpen = AK_TRUE;
        akerror("wavin adc open!", 0, 1);
    }

    dma_trans_mode = trans_mode;
    //set signal connect
    wav_in = wavin_io_info[ad_info_buf[i]->Wav_IO][0];
    wav_out = wavin_io_info[ad_info_buf[i]->Wav_IO][1];
    if (!analog_setsignal(wav_in, wav_out, SIGNAL_CONNECT))
    {
        akerror("wavin set signal failed!", 0, 1);
    }
    akerror("wavin open id:", i, 1);
    
    return i;
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

T_U32 WaveINRealSample = 0;
T_U32 Fwl_GetWaveInRealSample()
{
	return WaveINRealSample;
}

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
    T_U32 ret;
    T_U32 wav_in, wav_out;
    T_U32 mode;

    if ((id >=MAX_WAVEIN_INSTANCE) || (AK_NULL == ad_info_buf[id]))
        return AK_FALSE;

    wav_in = wavin_io_info[ad_info_buf[id]->Wav_IO][0];
    wav_out = wavin_io_info[ad_info_buf[id]->Wav_IO][1];

    ret = adc_setinfo(SampleRate, Channel, BitsPerSample);
	WaveINRealSample = ret;
    AkDebugOutput("wavin real sample rate:%d, ch:%d, bps:%d\n", ret, Channel, BitsPerSample);

    WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_REQUEST, AK_TRUE);
    WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING, dma_trans_mode);

    cur_wavin_info.CBufFunc = ad_info_buf[id]->Func;
    l2_get_buffer_register(WaveIn_DefaultCB, GB_ADC);

    if (!analog_setconnect(wav_in, wav_out, SIGNAL_CONNECT))
    {
        akerror("wavin connect failed!", 0, 1);
        return AK_FALSE;
    }

    cur_wavin_info.cur_id = id;
    mode = dma_trans_mode?L2_TRANS_DMA:L2_TRANS_CPU;

    return l2_transfer_start(GB_ADC, mode, DEFAULT_INTERN);
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
    T_U32 mode;
    
    if ((id >= MAX_WAVEIN_INSTANCE )||(cur_wavin_info.cur_id != id))
    {
        akerror("wavin cur_wavin_info.cur_id != id!", 0, 1);
        return AK_FALSE;
    }
    // 判断是否使能
    if (WAVEIN_GET_SENDFLAG(WAVEIN_FLAG, WAVIN_REQUEST) && !WAVEIN_GET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING))
    {
        WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING, AK_TRUE);
        mode = L2_TRANS_CPU;
        if (AK_TRUE == dma_trans_mode)
            mode = L2_TRANS_DMA;
        if (l2_transfer_start(GB_ADC, mode, DEFAULT_INTERN))
        {
            return AK_TRUE;
        }
        else
        {
            WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING, AK_FALSE);
        }
    }
    return AK_FALSE;
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
    T_U32 count = 0;
    T_U32 wav_in, wav_out;

    if((id >= MAX_WAVEIN_INSTANCE ) || (cur_wavin_info.cur_id != id))
    {
        akerror("wavin invalied id!", 0, 1);
        return AK_FALSE;
    }

    wav_in = wavin_io_info[ad_info_buf[id]->Wav_IO][0];
    wav_out = wavin_io_info[ad_info_buf[id]->Wav_IO][1];

    cur_wavin_info.CBufFunc = AK_NULL;
    if (WAVEIN_GET_SENDFLAG(WAVEIN_FLAG, WAVIN_REQUEST))
    {
        while (WAVEIN_GET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING))
        {
            count++;
            if (count > COUNT_TIME)
            {
                akerror("wavin Sending overtime!", 0, 1);
                l2_transfer_stop(GB_ADC);//等待中断超时,强制关闭LDMA传输.
                WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_SENDING, AK_FALSE);
                break;
            }
        }
    }
    WAVEIN_SET_SENDFLAG(WAVEIN_FLAG, WAVIN_REQUEST, AK_FALSE);

    if (!analog_setconnect(wav_in, wav_out, SIGNAL_DISCONNECT))
    {
        akerror("wavin disconnect failed!", 0, 1);
        return AK_FALSE;
    }

    cur_wavin_info.cur_id = INVAL_WAVEIN_ID;

    //对于CPU工作模式，需要主动停止内部timer
    if (dma_trans_mode == AK_FALSE)
        l2_transfer_stop(GB_ADC);

    return AK_TRUE;
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
    T_U32 wav_in, wav_out, i;

    if((id >= MAX_WAVEIN_INSTANCE ) || (AK_NULL == ad_info_buf[id]))
    {
        akerror("wavin close invalied id!", 0, 1);
        return AK_FALSE;
    }

    for (i=0; i<MAX_WAVEIN_INSTANCE; i++)
    {
        if ((i != id)&&(ad_info_buf[i]->Wav_IO == ad_info_buf[id]->Wav_IO))
        {
            ret = AK_FALSE;
            break;
        }
    }

    if (ret)
    {
        wav_in = wavin_io_info[ad_info_buf[id]->Wav_IO][0];
        wav_out = wavin_io_info[ad_info_buf[id]->Wav_IO][1];
        analog_setconnect(wav_in, wav_out, SIGNAL_DISCONNECT);
        if (!analog_setsignal(wav_in, wav_out, SIGNAL_DISCONNECT))
        {
            akerror("wavin set signal failed!", 0, 1);
        }
        
        akerror("wavin set signal disconnect!", 0, 1);
    }

    ad_info_buf[id]->Wav_IO = MAX_WAVIN_NUM;
    ad_info_buf[id]->Func = AK_NULL;
    ad_info_buf[id] = Fwl_Free(ad_info_buf[id]);

    for(i=0; i<MAX_WAVEIN_INSTANCE; i++)
    {
        if(ad_info_buf[i])
            break;
    }

    if (i >= MAX_WAVEIN_INSTANCE)
    {
        adc_close();
        cur_wavin_info.bADCOpen = AK_FALSE;
        akerror("wavin adc close!", 0, 1);
    }

    return AK_TRUE;
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
    T_BOOL ret = AK_FALSE;

    if((id >= MAX_WAVEIN_INSTANCE ) || (AK_NULL == ad_info_buf[id]))
    {
        akerror("wavin set gain invalied id!", 0, 1);
        return ret;
    }

    if (MIC_ADC == ad_info_buf[id]->Wav_IO)
    {
        gain = (gain > MAX_MIC_GAIN) ? MAX_MIC_GAIN : gain;
        ret = analog_setgain_mic(gain);  //set mic gain(0~10)
    }
    else if (LINEIN_ADC == ad_info_buf[id]->Wav_IO)
    {
        gain = (gain > MAX_LINEIN_GAIN) ? MAX_LINEIN_GAIN : gain;
        ret = analog_setgain_linein(gain); //set linein gain(0~15)
    }
    AK_DEBUG_OUTPUT("wavin set gain: gain=%d, ret=%d!\n", gain, ret);

    return ret;
}
#pragma arm section code


