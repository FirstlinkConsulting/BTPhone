/******************************************************************************************
**FileName      :      log_pcm_player.h
**brief         :      NO
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi Yao
**date      :   2013-3-19
**version   :   1.0
*******************************************************************************************/
#ifndef _LOG_PCM_PLAYER_H
#define _LOG_PCM_PLAYER_H

#include "Fwl_WaveOut.h"
#include "Eng_CycBuf.h"


typedef struct
{
    T_U32 samplerate;
    T_U32 channel;
    T_U32 bps;
    T_U32 bufsize;
}T_PCM_INFO;


#define     INVAL_PCMOPEN_ID        INVAL_WAVEOUT_ID

/*********************************************************************
  Function:         Pcm_write
  Description:      write data into the cycbuf
  Input:            buf: the data buffer
  Input:            T_U32 size: the size you want to write
  Return:           SUCCEED:write size   FAILED:-1
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_S32 Pcm_Write(T_U8* buf, T_U32 size);

//在open的时候，WAVE_OUT open
/*********************************************************************
  Function:         Pcm_Open
  Description:      call wave_out open.
  Input:            T_WAVE_OUT : src, dst type
  Input:            WAVEOUT_WRITE_CB: l2 interrupt callback function
  Return:           open id
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_U8   Pcm_Open(T_VOID);

//设置一下标志，准备DAM传输；同时申请循环buffer
/*********************************************************************
  Function:         Pcm_Start
  Description:      call wave_out start and cycbuf_create.
  Input:            id : pcm open id
  Input:            info: DAC set information
  Return:           start succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Start(T_U32 id, T_PCM_INFO *info);

//开始DAM传输
/*********************************************************************
  Function:         Pcm_Playing
  Description:      call WaveOut_Ensure_Sending
  Input:            id : pcm open id
  Return:           start succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Playing(T_U8 id);

//停止dma传输，WAVE_OUT stop
/*********************************************************************
  Function:         Pcm_Pause
  Description:      call waveout_stop
  Input:            id : pcm open id
  Return:           pause succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Pause(T_U8 id);

/*********************************************************************
  Function:         Pcm_ReStart
  Description:      call WaveOut_Start
  Input:            id : pcm open id  
  Input:            info: DAC set information
  Return:           restart succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_ReStart(T_U8 id, T_PCM_INFO *info);

//停止dma传输，同时释放循环buffer
/*********************************************************************
  Function:         Pcm_ReStart
  Description:      call WaveOut_Stop and cycbuf_destory
  Input:            id : pcm open id  
  Return:           stop succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Stop(T_U8 id);

/*********************************************************************
  Function:         Pcm_Close
  Description:      wave out close.
  Input:            id : the open id
  Return:           close succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_Close(T_U8 id);

/*********************************************************************
  Function:         Pcm_CheckPause
  Description:      wave out check pause or not.
  Input:            T_VOID
  Return:           whether pause or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_BOOL Pcm_CheckPause(T_VOID);

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
T_S32 Pcm_DiscardWaveData(T_U32 id, T_U32 size);


/*********************************************************************
  Function:     Pcm_GetFreeSize
  Description:      get the free size of the cycbuf
  Return:           the free size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 Pcm_GetFreeSize(T_VOID);


/*********************************************************************
  Function:     Pcm_GetPcmSize
  Description:      get the pcm data size of the cycbuf
  Return:           the pcm data size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 Pcm_GetPcmSize(T_VOID);


#endif

