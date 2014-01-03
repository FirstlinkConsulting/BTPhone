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
#include "Fwl_Detect.h"
#include "Eng_Debug.h"
#include "arch_dac.h"
#include "arch_analog.h"
#include "hal_l2.h"
#include "fwl_console.h"
#include "Log_Pcm_Player.h"
#include "Fwl_Timer.h"


typedef struct
{
    T_WAVE_OUT          Wav_IO;
	T_PCM_INFO			pcm_info;
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

#pragma arm section rwdata = "_cachedata_"
volatile T_WAVE_STRUCT cur_wav_info = {AK_FALSE, 0, 0, INVAL_WAVEOUT_ID, AK_NULL};
#pragma arm section rwdata


#define SRC_GAIN(vol)           L2HpVolTab[(vol)][0]
#define DST_GAIN(vol)           L2HpVolTab[(vol)][1]

#define WAVEOUT_FLAG            cur_wav_info.flag
#define COUNT_TIME              2000000
#define WAVE_OUT_TIMEOUT_MS		1000

#define WAVEOUT_SET_SENDFLAG(flag, type, val) ((val)? ((flag) |= (1 << (type))) : ((flag) &= ~(1 << (type))))
#define WAVEOUT_GET_SENDFLAG(flag, type) ((flag) & (1 << type))


#pragma arm section rodata = "_waveout_data_"
const static T_U8 wav_io_info[MAX_WAVOUT_NUM][2] = {
    {SRC_DAC, DST_HP}, 
    {SRC_LINEIN, DST_HP},
    {SRC_MIC, DST_HP}
};

static DA_INFO *da_info_buf[MAX_WAVEOUT_INSTANCE] = {AK_NULL};
#pragma arm section rodata



#pragma arm section code = "_bootcode1_"
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
    if (cur_wav_info.CBufFunc != AK_NULL)
    {
        cur_wav_info.CBufFunc(buf, len);
    }

    if (*buf == AK_NULL || *len == 0)
    {
        WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING, AK_FALSE);
    }
    /*
    if (!WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST))
    {
        *buf = AK_NULL ;
        *len = 0;
    }*/
}
#pragma arm section code


#pragma arm section code = "_frequentcode_" 
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
    return cur_wav_info.bDACOpen;
}
#pragma arm section code


#pragma arm section code = "_wave_out_"
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
    T_U8 i;
    T_U32 wav_in, wav_out;

    if (Wav_IO >= MAX_WAVOUT_NUM)
        return INVAL_WAVEOUT_ID;

    //find a valied id
    for(i=0; i<MAX_WAVEOUT_INSTANCE; i++)
    {
        if(AK_NULL == da_info_buf[i])
            break;
    }

    if (i >= MAX_WAVEOUT_INSTANCE)
        return INVAL_WAVEOUT_ID;

    da_info_buf[i] = (DA_INFO*)Fwl_Malloc(sizeof(DA_INFO));

    if (AK_NULL == da_info_buf[i])
    {
        akerror("wav instance malloc false!", 0, 1);
        return INVAL_WAVEOUT_ID;
    }

    da_info_buf[i]->Wav_IO = Wav_IO;
    da_info_buf[i]->Func = Func;

    wav_in = wav_io_info[da_info_buf[i]->Wav_IO][0];
    wav_out = wav_io_info[da_info_buf[i]->Wav_IO][1];

    //open da
    if (SRC_DAC == wav_in)
    {
        if (!cur_wav_info.bDACOpen)
        {
            dac_open();
            akerror("wav dac open!", 0, 1);
            cur_wav_info.bDACOpen = AK_TRUE;
        }
    }

    //set signal connect
    if (!analog_setsignal(wav_in, wav_out, SIGNAL_CONNECT))
    {
        akerror("wav set signal failed!", 0, 1);
    }

    if (DST_HP == wav_out)
    {
        if (0 == cur_wav_info.HPOpenNum)
        {
            analog_setgain_hp(5);
            Fwl_DetectorEnable(DEVICE_HP, AK_TRUE);
        }
        if (!Fwl_DetectorGetStatus(DEVICE_HP))
        {
            Fwl_SpkConnectSet(AK_TRUE);
        }
        cur_wav_info.HPOpenNum |= (1 << i);
    }

    akerror("wavout open id:", i, 1);

    return i;
}
T_U32 WaveOutRealSample = 0;
/*******************************************************************************
 * @brief   get da real sample rate
 * @author  aijun
 * @date    2013-2-19
 * @param  none
 * @param   none
 * @return  T_U32
 * @retval  real sample rate
*******************************************************************************/
T_U32 Fwl_GetWaveOutRealSample(T_VOID)
{
	return WaveOutRealSample;
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
    T_U32 wav_in, wav_out;

    if ((id >=MAX_WAVEOUT_INSTANCE) || (AK_NULL == da_info_buf[id]))
    {
        akerror("wavout invalided id!", 3, 1);
        return AK_FALSE;
    }

    //set signal connect
    wav_in = wav_io_info[da_info_buf[id]->Wav_IO][0];
    wav_out = wav_io_info[da_info_buf[id]->Wav_IO][1];
	da_info_buf[id]->pcm_info.samplerate= SampleRate;
	da_info_buf[id]->pcm_info.channel= Channel;
	da_info_buf[id]->pcm_info.bps= BitsPerSample;
	
    if (SRC_DAC == wav_in)
    {

        WaveOutRealSample = dac_setinfo(SampleRate, Channel, BitsPerSample);
        AK_DEBUG_OUTPUT("wav da set info, sp:%d, ch:%d, bps:%d\n", SampleRate, Channel, BitsPerSample);

        if(!Fwl_WaveOutDACIsOpen())
        {
            akerror("wav dac not opened!", 0, 1);
            return AK_FALSE;
        }

        if (AK_NULL == da_info_buf[id]->Func)
        {
            akerror("wavout invalided Func!", 0, 1);
            return AK_FALSE;
        }

        WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST, AK_TRUE);

        cur_wav_info.CBufFunc = da_info_buf[id]->Func;
        l2_get_buffer_register(Wave_DefaultCB, GB_DAC);
    }

    if (!analog_setconnect(wav_in, wav_out, SIGNAL_CONNECT))
    {
        akerror("wavout connect failed!", 0, 1);
        return AK_FALSE;
    }

    cur_wav_info.cur_id = id;
	if (!Fwl_DetectorGetStatus(DEVICE_HP))
	{
    	Fwl_SpkConnectSet(AK_TRUE);
	}
    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/*******************************************************************************
 * @brief   mute wave out
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @param   [in]isMute: mute or not
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
static T_BOOL Fwl_WaveOutMute(T_U8 id, T_BOOL isMute)
{
    T_BOOL ret = AK_FALSE;
    ANALOG_SIGNAL_SRC src;
    ANALOG_SIGNAL_DST dst;
    ANALOG_SIGNAL_STATE state;

    if((id >= MAX_WAVEOUT_INSTANCE ) || (AK_NULL == da_info_buf[id]))
    {
        akerror("wavout set gain invalied id!", 0, 1);
        return ret;
    }

	if(!Fwl_DetectorGetStatus(DEVICE_HP))
	{	
		Fwl_SpkConnectSet(!isMute);
	}
	
    switch (da_info_buf[id]->Wav_IO)
    {
    case DAC_HP:
        src = SRC_DAC;
        dst = DST_HP;
        break;
    case MIC_HP:
        src = SRC_MIC;
        dst = DST_HP;
        break;
    case LINEIN_HP:
        src = SRC_LINEIN;
        dst = DST_HP;
        break;
    default:
        return ret;
    }

    state = isMute ? SIGNAL_DISCONNECT : SIGNAL_CONNECT;
    ret = analog_setconnect(src, dst, state);
    return ret;
}
#pragma arm section code


#pragma arm section code = "_music_playing_"
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
    if ((!WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST))&&(!WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING)))
        return AK_TRUE;
    return AK_FALSE;
}
#pragma arm section code

T_U32 gA2DPKTime = 0;
#pragma arm section code = "_music_playing_"
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
    if ((id >=MAX_WAVEOUT_INSTANCE))
    {
        akerror("wavout cur_wav_info.cur_id != id!", cur_wav_info.cur_id, 1);
        return AK_FALSE;
    }

	if(cur_wav_info.cur_id != id)
	{
        akerror("wavout reset resample rate!", cur_wav_info.cur_id, 1);
		Fwl_WaveOutStart(id, da_info_buf[id]->pcm_info.samplerate, da_info_buf[id]->pcm_info.channel, da_info_buf[id]->pcm_info.bps);
	}
	
    if (SRC_DAC == wav_io_info[da_info_buf[id]->Wav_IO][0])
    {
        // 判断是否使能
        if (WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST) && !WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING))
        {
            WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING, AK_TRUE);
            if (l2_transfer_start(GB_DAC, L2_TRANS_DMA, 0))
            {
				gA2DPKTime ++;
                return AK_TRUE;
            } 
            else
            {
                WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING, AK_FALSE);
            }
        }
    }
    return AK_FALSE;
}
#pragma arm section code


#pragma arm section code = "_wave_out_"
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
 //   T_U32 count = 0;
    T_U32 wav_in, wav_out, time;

    if ((id >=MAX_WAVEOUT_INSTANCE) || (cur_wav_info.cur_id != id))
    {
        akerror("wavout invalided id!", cur_wav_info.cur_id, 1);
        return AK_FALSE;
    }

    wav_in = wav_io_info[da_info_buf[id]->Wav_IO][0];
    wav_out = wav_io_info[da_info_buf[id]->Wav_IO][1];

    if (SRC_DAC == wav_in)
    {
        if (WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST))
        {
        	time = Fwl_GetTickCountMs();
            while (WAVEOUT_GET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING))
            {
               // count++;
                if (Fwl_GetTickCountMs() > time + WAVE_OUT_TIMEOUT_MS)
                {
                    akerror("wav Sending overtime!", 0, 1);
                    l2_transfer_stop(GB_DAC);//等待中断超时,强制关闭LDMA传输.
                    WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_SENDING, AK_FALSE);
                    break;
                }
            }
        }
        WAVEOUT_SET_SENDFLAG(WAVEOUT_FLAG, DMA_REQUEST, AK_FALSE);
    }

    if (!analog_setconnect(wav_in, wav_out, SIGNAL_DISCONNECT))
    {
        akerror("wavout disconnect failed!", 0, 1);
        return AK_FALSE;
    }

    cur_wav_info.cur_id = INVAL_WAVEOUT_ID;
	if (!Fwl_DetectorGetStatus(DEVICE_HP))
	{
    	Fwl_SpkConnectSet(AK_FALSE);
	}
    return AK_TRUE;
}

/*******************************************************************************
 * @brief   close wave out forcely.
 * @author  lishengkai
 * @date    2013-2-19
 * @param   [in]id: the open id
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_VOID Fwl_WaveForceOutClose(T_VOID)
{
    T_U32 wav_in, wav_out;
	
	if(INVAL_WAVEOUT_ID != cur_wav_info.cur_id)
	{
	    wav_in = wav_io_info[da_info_buf[cur_wav_info.cur_id]->Wav_IO][0];
	    wav_out = wav_io_info[da_info_buf[cur_wav_info.cur_id]->Wav_IO][1];
		
		l2_transfer_stop(GB_DAC);
		analog_setconnect(wav_in, wav_out, SIGNAL_DISCONNECT);
		analog_setsignal(wav_in, wav_out, SIGNAL_DISCONNECT);
		dac_close();
	}
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
    T_BOOL ret = AK_TRUE;
    T_U8 i;
    T_U32 wav_in, wav_out;

    if ((id >=MAX_WAVEOUT_INSTANCE) || (AK_NULL == da_info_buf[id]))
    {
        akerror("wavout invalided id!", 1, 1);
        return AK_FALSE;
    }

    wav_in = wav_io_info[da_info_buf[id]->Wav_IO][0];
    wav_out = wav_io_info[da_info_buf[id]->Wav_IO][1];

    for (i=0; i<MAX_WAVEOUT_INSTANCE; i++)
    {
        if ((i != id)&&(da_info_buf[i]->Wav_IO == da_info_buf[id]->Wav_IO))
        {
            ret = AK_FALSE;
            break;
        }
    }
	
	cur_wav_info.HPOpenNum &= ~(1 << id);
    if (ret)
    {
        if (DST_HP == wav_out)
        {
            if (0 == cur_wav_info.HPOpenNum)
            {
                Fwl_DetectorEnable(DEVICE_HP, AK_FALSE);
                Fwl_SpkConnectSet(AK_FALSE);
            }
        }
        analog_setconnect(wav_in, wav_out, SIGNAL_DISCONNECT);
        analog_setsignal(wav_in, wav_out, SIGNAL_DISCONNECT);
    }
    
    da_info_buf[id]->Wav_IO = MAX_WAVOUT_NUM;
    da_info_buf[id]->Func = AK_NULL;
    da_info_buf[id] = Fwl_Free(da_info_buf[id]);

    if (SRC_DAC == wav_in)
    {
        for (i=0; i<MAX_WAVEOUT_INSTANCE; i++)
        {
            if ((AK_NULL != da_info_buf[i]) 
                && (SRC_DAC == wav_io_info[da_info_buf[i]->Wav_IO][0]))
            {
                break;
            }
        }
        
        if (i >= MAX_WAVEOUT_INSTANCE)
        {
            dac_close();
            cur_wav_info.bDACOpen = AK_FALSE;
            akerror("wavout dac close!", 0, 1);
        }
    }

    return AK_TRUE;
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

    if((id >= MAX_WAVEOUT_INSTANCE ) || (AK_NULL == da_info_buf[id]))
    {
        akerror("wavout set gain invalied id!", 0, 1);
        return ret;
    }

    if (0 == gain)
    {
        ret = Fwl_WaveOutMute(id, AK_TRUE);
        return ret;
    }
    else
    {
        ret = Fwl_WaveOutMute(id, AK_FALSE);
    }

    switch (da_info_buf[id]->Wav_IO)
    {
    case DAC_HP:
        analog_setgain_hp(DST_GAIN(gain));     //set hp gain(0~5)
        break;

    case LINEIN_HP:
        analog_setgain_linein(SRC_GAIN(gain)); //set linein gain(0~15)
        analog_setgain_hp(DST_GAIN(gain));     //set hp gain(0~5)
        break;

    case MIC_HP:
        analog_setgain_mic(SRC_GAIN(gain));    //set mic gain(0~10)
        analog_setgain_hp(DST_GAIN(gain));     //set hp gain(0~5)
        break;

    default:
        ret = AK_FALSE;
        break;
    }
    //AK_DEBUG_OUTPUT("wavout set gain: gain=%d, ret=%d!\n", gain, ret);

    return ret;
}

T_BOOL Fwl_WaveOutSetSignal(T_WAVE_OUT wave_id, T_BOOL isSet)
{
    T_BOOL ret = AK_FALSE;
    ANALOG_SIGNAL_SRC src;
    ANALOG_SIGNAL_DST dst;
    ANALOG_SIGNAL_STATE state;

    switch (wave_id)
    {
    case DAC_HP:
        src = SRC_DAC;
        dst = DST_HP;
        break;
    case MIC_HP:
        src = SRC_MIC;
        dst = DST_HP;
        break;
    case LINEIN_HP:
        src = SRC_LINEIN;
        dst = DST_HP;
        break;
    default:
        return ret;
    }

    state = isSet ? SIGNAL_CONNECT : SIGNAL_DISCONNECT;
    ret = analog_setsignal(src, dst, state);
    return ret;
}


#pragma arm section code



