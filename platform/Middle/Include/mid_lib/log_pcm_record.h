/******************************************************************************************
**FileName      :      log_pcm_player.h
**brief         :      NO
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi Yao
**date      :   2013-3-19
**version   :   1.0
*******************************************************************************************/
#ifndef _LOG_PCM_RECORD_H
#define _LOG_PCM_RECORD_H

#include "Fwl_WaveIn.h"
#include "Eng_CycBuf.h"


typedef struct
{
    T_U32 samplerate;
    T_U32 channel;
    T_U32 bps;
    T_U32 bufsize;
}T_RECORD_INFO;

/*********************************************************************
  Function:         Pcm_Record_READ
  Description:      read data from the cycbuf
  Input:            buf: the data buffer
  Input:            T_U32 size: the size you want to read
  Return:           SUCCEED:read size   FAILED:-1
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_S32 Pcm_Record_Read(T_U32 id, T_U8* buf, T_U32 size);

//在open的时候，WAVE_in open
/*********************************************************************
  Function:     Pcm_Record_Open
  Description:      call wave_in open.
  Input:            T_WAVE_IN : src, dst type
  Input:              T_BOOL trans_mode dam or cpu
  Return:           open id
  Author:           AIJUN
  Data:             2013-5-9
**********************************************************************/
T_U8 Pcm_Record_Open(T_WAVE_IN type, T_BOOL trans_mode);

//设置一下标志，准备AD传输；同时申请循环buffer
/*********************************************************************
  Function:         Pcm_Record_Start
  Description:      call wave_IN start and cycbuf_create.
  Input:            id : pcm open id
  Input:            info: ADC set information
  Return:           start succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Start(T_U32 id, T_RECORD_INFO *info);


//停止dma传输，WAVE_OUT stop
/*********************************************************************
  Function:         Pcm_Record_Pause
  Description:      call waveout_stop
  Input:            id : pcm open id
  Return:           pause succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Pause(T_U8 id);

/*********************************************************************
  Function:         Pcm_Record_ReStart
  Description:      call WaveiN_Start
  Input:            id : pcm open id  
  Input:            info: DAC set information
  Return:           restart succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_ReStart(T_U8 id, T_RECORD_INFO *info);

//停止dma传输，同时释放循环buffer
/*********************************************************************
  Function:         Pcm_Record_Stop
  Description:      call WaveOut_Stop and cycbuf_destory
  Input:            id : pcm open id  
  Return:           stop succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Stop(T_U8 id);

/*********************************************************************
  Function:         Pcm_Record_Close
  Description:      wave out close.
  Input:            id : the open id
  Return:           close succeed or not
  Author:           AIJUN
  Data:             2013-05-09
**********************************************************************/
T_BOOL Pcm_Record_Close(T_U8 id);

#endif

