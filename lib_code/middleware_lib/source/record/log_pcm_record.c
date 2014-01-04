#ifdef SUPPORT_BLUETOOTH

#include "log_pcm_record.h"
#include "Eng_Debug.h"
#include <string.h>
#include "Fwl_OsMalloc.h"
#include "Eng_BtPhone.h"

/******************************************************************************************
**FileName      :      log_pcm_record.c
**brief         :      NO
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   aijun
**date      :   2013-5-09
**version   :   1.0
*******************************************************************************************/

static volatile T_U16 gRecordWriteSize = 0;
T_U8 *PcmRecordBuf = AK_NULL;
#define PCM_RECORD_BUF_SIZE 256
#pragma arm section code = "_bootcode1_"

T_VOID Record_CBFun(T_U8 **buf, T_U32 *len)
{
	if(gRecordWriteSize)
	{
		gRecordWriteSize = BtPhone_ADReSmaple(gRecordWriteSize, PcmRecordBuf, PCM_RECORD_BUF_SIZE + 64, PcmRecordBuf);
		if(gRecordWriteSize)
		{
			BtPhone_CheckVoice(PcmRecordBuf, gRecordWriteSize, AK_FALSE);
		}
	}
	//从循环buffer获取数据，用于传输
	*buf = PcmRecordBuf;
	gRecordWriteSize = PCM_RECORD_BUF_SIZE;
	*len = PCM_RECORD_BUF_SIZE;
}
#pragma arm section code 


//在open的时候，WAVE_in open
/*********************************************************************
  Function:     Pcm_Record_Open
  Description:      call wave_in open.
  Input:            T_WAVE_IN : src, dst type
  Input:            T_BOOL trans_mode   CPU OR DMA
  Return:           open id
  Author:           AIJUN
  Data:             2013-5-9
**********************************************************************/
T_U8 Pcm_Record_Open(T_WAVE_IN type, T_BOOL trans_mode)
{
    return Fwl_WaveInOpen(type, Record_CBFun, trans_mode);
}

//设置一下标志，准备DAM传输；同时申请循环buffer
/*********************************************************************
  Function:     Pcm_Record_Start
  Description:      call wave_out start and cycbuf_create.
  Input:            id : pcm open id
  Input:            info: DAC set information
  Return:           start succeed or not
  Author:           lishengkai
  Data:            2013-3-6
**********************************************************************/
T_BOOL Pcm_Record_Start(T_U32 id, T_RECORD_INFO *info)
{
	gRecordWriteSize = 0;
	PcmRecordBuf = Fwl_Malloc(PCM_RECORD_BUF_SIZE + 64); //为了做重采样
	memset(PcmRecordBuf, 0, PCM_RECORD_BUF_SIZE + 64);

    if(!Fwl_WaveInStart(id, info->samplerate, info->channel, info->bps))
    {
        akerror("  pcm record start failed! id:", id, 1);
        return AK_FALSE;
    }
	
	Fwl_WaveInSetGain(id, 7);
	

    return AK_TRUE;
}


//停止传输，WAVE_in stop
/*********************************************************************
  Function:         Pcm_Record_Pause
  Description:      call waveout_stop
  Input:            id : pcm open id
  Return:           pause succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Pause(T_U8 id)
{
	return Fwl_WaveInStop(id);
}

/*********************************************************************
  Function:         Pcm_Record_ReStart
  Description:      call WaveiN_Start
  Input:            id : pcm open id  
  Input:            info: DAC set information
  Return:           restart succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_ReStart(T_U8 id, T_RECORD_INFO *info)
{

    if(!Fwl_WaveInStart(id, info->samplerate, info->channel, info->bps))
    {
        akerror("   pcm record restart failed! id:", id, 1);
        return AK_FALSE;
    }
	gRecordWriteSize = 0;
    
    return AK_TRUE;
}


//停止dma传输，同时释放循环buffer
/*********************************************************************
  Function:     Pcm_ReStart
  Description:      call WaveOut_Stop and cycbuf_destory
  Input:            id : pcm open id  
  Return:           stop succeed or not
  Author: 		   AIJUN
  Data:		   2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Stop(T_U8 id)
{
    
    Fwl_WaveInStop(id);

	Fwl_Free(PcmRecordBuf);
	PcmRecordBuf = AK_NULL;

    return AK_TRUE;
}

/*********************************************************************
  Function:     Pcm_Record_Close
  Description:      wave out close.
  Input:            id : the open id
  Return:           close succeed or not
  Author: 		  AIJUN
  Data:			  2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Close(T_U8 id)
{
    return Fwl_WaveInClose(id);
}
#endif

