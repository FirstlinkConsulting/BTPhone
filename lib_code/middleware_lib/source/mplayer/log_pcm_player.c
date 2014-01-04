/******************************************************************************************
**FileName      :      log_pcm_player.c
**brief         :      NO
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi Yao
**date      :   2013-3-19
**version   :   1.0
*******************************************************************************************/

#include "log_pcm_player.h"
#include "Eng_Debug.h"
#include <string.h>
#ifdef SUPPORT_BLUETOOTH
#include "Fwl_OsMalloc.h"
#include "Eng_BtPhone.h"
#endif

#define     BUFCOUNT        MAX_WAVEOUT_INSTANCE
#ifdef OS_ANYKA
#define     DMA_PCM_SIZE    1024
#define     DMA_ONE_CH_PCM_SIZE    256
#else
#define     DMA_PCM_SIZE    4096
#define     DMA_ONE_CH_PCM_SIZE    2048
#endif
#pragma arm section rwdata = "_cachedata_"
static T_BOOL gDAC_first_frame_flag = AK_TRUE;
#ifdef SUPPORT_BLUETOOTH
T_BOOL PcmChannel = AK_TRUE;
#endif
#pragma arm section rwdata

#pragma arm section zidata = "_bootbss_"
static volatile T_U32 dma_size = 0;
static T_S16 gpre_save_DAC_data = 0;
#pragma arm section zidata 

T_U32 Pcm_BufSize[BUFCOUNT] = {0};
#pragma arm section code = "_bootcode1_"

T_VOID Pcm_Fix_phase(T_S16* data, T_U32 len)
{
	int i;
	T_S16 last_data;
	
	if(AK_TRUE == gDAC_first_frame_flag)
	{
       gDAC_first_frame_flag = AK_FALSE;
	   gpre_save_DAC_data = 2 * data[0] - data[2];
	}
	
	last_data = data[len-2];
	for (i=len-4; i>=0; i-=2)
	{ 
	  data[i+2] = data[i];
	}
	data[0] = gpre_save_DAC_data;
	gpre_save_DAC_data = last_data;
}

T_VOID Pcm_CBFun(T_U8 **buf, T_U32 *len)
{
    cycbuf_read_updateflag(CYC_DA_BUF_ID, dma_size);
	#ifdef SUPPORT_BLUETOOTH
	*len = PcmChannel? DMA_PCM_SIZE:DMA_ONE_CH_PCM_SIZE;
	#else
	*len = DMA_PCM_SIZE;
	#endif
    //从循环buffer获取数据，用于dma传输
    
    dma_size = cycbuf_getdatabuf(CYC_DA_BUF_ID, buf, *len);
	#ifdef SUPPORT_BLUETOOTH
	if(dma_size < *len)
		dma_size = 0;
	#endif
    *len = dma_size;
    if(!(dma_size))
    {
		gDAC_first_frame_flag = AK_TRUE;
        *buf = AK_NULL;
        *len = 0;
        return;
    } 
	#ifdef SUPPORT_BLUETOOTH
	{
		T_U8 *SysRsvBuf = *buf; 
		
		if(!PcmChannel)
		{
			T_U16 i, j, *SrcPtr, *DestPtr;

			BtPhone_CheckVoice(*buf, *len, AK_TRUE);
			SysRsvBuf = Fwl_SysRsvMalloc();
			SrcPtr = (T_U16 *)*buf;
			DestPtr = (T_U16 *)SysRsvBuf;
			for(i = 0, j = 0; i < (*len>>1); i++)
			{
				DestPtr[j++] = SrcPtr[i];
				DestPtr[j++] = SrcPtr[i];
			}
			*len <<= 1;
		}

		*buf = SysRsvBuf;
	}
	#endif
	Pcm_Fix_phase(*buf, *len >> 1);
}
#pragma arm section code 


/*********************************************************************
  Function:     Pcm_write
  Description:      write data into the cycbuf
  Input:            buf: the data buffer
  Input:            T_U32 size: the size you want to write
  Return:           SUCCEED:write size   FAILED:-1
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
#pragma arm section code = "_audioplayer_resident_"

T_S32 Pcm_Write(T_U8* buf, T_U32 size)
{
    T_U8 *writebuf;
    T_U32 retsize, temp = size;


    if (cycbuf_getblanksize(CYC_DA_BUF_ID) < size)
    {
        return -1;
    }

    
    while(temp)
    {
        retsize = cycbuf_getwritebuf(CYC_DA_BUF_ID, &writebuf, temp);
        memcpy(writebuf, buf, retsize);
        cycbuf_write_updateflag(CYC_DA_BUF_ID, retsize);
        buf += retsize;
        temp -= retsize;
    }

    return size;
}

/*********************************************************************
  Function:     Pcm_GetFreeSize
  Description:      get the free size of the cycbuf
  Return:           the free size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 Pcm_GetFreeSize(T_VOID)
{
	return cycbuf_getblanksize(CYC_DA_BUF_ID);
}

/*********************************************************************
  Function:     Pcm_GetPcmSize
  Description:      get the pcm data size of the cycbuf
  Return:           the pcm data size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 Pcm_GetPcmSize(T_VOID)
{
	return cycbuf_getdatasize(CYC_DA_BUF_ID);
}

#pragma arm section code
#pragma arm section code = "_audioplayer_open_code_"

//在open的时候，WAVE_OUT open
/*********************************************************************
  Function:     Pcm_Open
  Description:      call wave_out open.
  Input:            T_WAVE_OUT : src, dst type
  Input:            WAVEOUT_WRITE_CB: l2 interrupt callback function
  Return:           open id
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_U8 Pcm_Open(T_VOID)
{
    return Fwl_WaveOutOpen(DAC_HP, Pcm_CBFun);
}

//设置一下标志，准备DAM传输；同时申请循环buffer
/*********************************************************************
  Function:     Pcm_Start
  Description:      call wave_out start and cycbuf_create.
  Input:            id : pcm open id
  Input:            info: DAC set information
  Return:           start succeed or not
  Author:           lishengkai
  Data:            2013-3-6
**********************************************************************/
T_BOOL Pcm_Start(T_U32 id, T_PCM_INFO *info)
{
    dma_size = 0;

    if(!Fwl_WaveOutStart(id, info->samplerate, info->channel, info->bps))
    {
        akerror("   pcm start failed!", 0, 1);
        return AK_FALSE;
    }
#ifdef SUPPORT_BLUETOOTH
    PcmChannel = (info->channel==2);
#endif
    if (info->bufsize)
    {
        akerror("  create size:", info->bufsize, 1);
        Pcm_BufSize[id] = info->bufsize;
        cycbuf_create(CYC_DA_BUF_ID, info->bufsize);
    }
    else
    {
        akerror("  Pcm buffer set err:", Pcm_BufSize[id], 1);
    }

    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_music_playing_"
//开始DAM传输
/*********************************************************************
  Function:     Pcm_Playing
  Description:      call WaveOut_Ensure_Sending
  Input:            id : pcm open id
  Return:           start succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Playing(T_U8 id)
{
    return Fwl_WaveOutEnsureSending(id);
}
#pragma arm section code

//停止dma传输，WAVE_OUT stop
/*********************************************************************
  Function:     Pcm_Pause
  Description:      call waveout_stop
  Input:            id : pcm open id
  Return:           pause succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Pause(T_U8 id)
{
    if(!Pcm_CheckPause())
    {
    	#ifdef SUPPORT_BLUETOOTH
		dma_size = 0;
		cycbuf_clear(CYC_DA_BUF_ID);
		#else
        Fwl_WaveOutEnsureSending(id);
        while(cycbuf_getdatasize(CYC_DA_BUF_ID));
		#endif
        return Fwl_WaveOutStop(id);
    }

    return AK_TRUE;
}

/*********************************************************************
  Function:     Pcm_ReStart
  Description:      call WaveOut_Start
  Input:            id : pcm open id  
  Input:            info: DAC set information
  Return:           restart succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_ReStart(T_U8 id, T_PCM_INFO *info)
{
    dma_size = 0;

#ifdef SUPPORT_BLUETOOTH
        PcmChannel = (info->channel==2);
#endif

    if(!Fwl_WaveOutStart(id, info->samplerate, info->channel, info->bps))
    {
        akerror("   pcm start failed!", 0, 1);
        return AK_FALSE;
    }

    if (info->bufsize > 0)
    {
        if (Pcm_BufSize[id] != info->bufsize)
        {            
            cycbuf_destory(CYC_DA_BUF_ID, Pcm_BufSize[id]);
            cycbuf_create(CYC_DA_BUF_ID, info->bufsize);
            Pcm_BufSize[id] = info->bufsize;
        }        
    }
    
    return AK_TRUE;
}


//停止dma传输，同时释放循环buffer
/*********************************************************************
  Function:     Pcm_ReStart
  Description:      call WaveOut_Stop and cycbuf_destory
  Input:            id : pcm open id  
  Return:           stop succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Stop(T_U8 id)
{
    if (!Pcm_BufSize[id])
    {
        return AK_FALSE;
    }
	#ifdef SUPPORT_BLUETOOTH
	dma_size = 0;
    cycbuf_clear(CYC_DA_BUF_ID);
	#endif
    if(!Pcm_CheckPause())
    {
    	Fwl_WaveOutStop(id);
    }

    cycbuf_destory(CYC_DA_BUF_ID, Pcm_BufSize[id]);

    Pcm_BufSize[id] = 0;

    return AK_TRUE;
}

/*********************************************************************
  Function:     Pcm_Close
  Description:      wave out close.
  Input:            id : the open id
  Return:           close succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Close(T_U8 id)
{
    return Fwl_WaveOutClose(id);
}


/*********************************************************************
  Function:         Pcm_CheckPause
  Description:      wave out check pause or not.
  Input:            T_VOID
  Return:           whether pause or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_CheckPause(T_VOID)
{
    return Fwl_WaveOutCheckPause();
}

/*********************************************************************
  Function:     Pcm_DiscardWaveData
  Description:      discard wave data.
  Input:            id : pcm open id
  Input:            size : the size of expecting to discard (multiple of 1Kbyte).
  Return:           info >0: discarded data size; 
                    info <0: discards false.
  Author:           liangxiong
  Data:             2013-4-13
**********************************************************************/
T_S32 Pcm_DiscardWaveData(T_U32 id, T_U32 size)
{
	T_U8 *buf;
	T_U32 tempsize = 0;
	T_U32 discsize = size;

    if (0 == dma_size)
	{
		while (0 < discsize)
		{
			tempsize = cycbuf_getdatabuf(CYC_DA_BUF_ID, &buf, discsize);
			if (0 == tempsize)
			{
				break;
			}
			else if (discsize >= tempsize)
			{
				discsize -= tempsize;
				cycbuf_read_updateflag(CYC_DA_BUF_ID, tempsize);
			}
			else
			{
				break;
			}
        }
        return (size - discsize);
	}
	return -1;
}


