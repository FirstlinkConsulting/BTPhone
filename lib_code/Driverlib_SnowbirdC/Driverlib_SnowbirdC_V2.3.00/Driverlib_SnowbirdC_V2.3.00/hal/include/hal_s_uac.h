/**
*@file hal_usb_s_uac.h
*@brief provide operations of how to use USB audio Class device.
*
*This file describe frameworks of USB audio Class device
*Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
*@author LiuHuadong
*@date 2012-05-4
*@version 1.0
*/

#ifndef _HAL_S_UAC_H_
#define _HAL_S_UAC_H_

#include "hal_uac.h"
#include "hal_usb_s_uac.h"

#ifdef __cplusplus
extern "C" {
#endif



#define     UAC_SAMFREQ                 44100  
#define     UAC_BUF_RESEVERED           (1024*4)
#define     UAC_SEND_BUF_PER            (512*4)
#define     UAC_DEV_MSG                 1
#define     UAC_BUFFER_NUM              2


#define     LOBYTE(w)                   ((T_U8)(w))
#define     HIBYTE(w)                   ((T_U8)(((T_U16)(w) >> 8) & 0xFF))

#define     UVC_CTRL_PARAM_GRP(index,value)   (((index)<<16) | (value))      
#define     UAC_SUPPORTED_CTRL_NUM          0xb  

//  Audio Class-Specific Request Codes
#define     RC_UNDEFINED                                    0x00
#define     SET_CUR                                         0x01
#define     GET_CUR                                         0x81
#define     GET_MIN                                         0x82
#define     GET_MAX                                         0x83
#define     GET_RES                                         0x84
#define     RESERVED                                        0x0a

#ifdef UAC_HID_EX
#define DEC_BUTTON      0x04
#define INC_BUTTON      0x02
#define MUTE_BUTTON     0x01

#define NEXT_BUTTON      0x08
#define PRE_BUTTON       0x10
#define PLAY_BUTTON      0x01
#define PAUSE_BUTTON     0x04
#else
#define DEC_BUTTON      0x02
#define INC_BUTTON      0x01

#endif

#define RELEASE_BUTTON  0



#ifdef UAC_MIC_FUN    
#define NUM_INTERFACE            0x04                            // bNumInterfaces (3)
#define INTERFACE_HEAD_LEN       (sizeof(T_USB_AUDIO_HEAD_DESC))
#else
#define NUM_INTERFACE            0x03
#define INTERFACE_HEAD_LEN       (sizeof(T_USB_AUDIO_HEAD_DESC)-1)

#endif

#ifdef UAC_MIC_FUN
// Feature Unit ID
#define SPEAKER_FU_ID           0x05

// Input Termintal ID
#define SPEAKER_IT_ID           0x04

// Output Termintal ID
#define SPEAKER_OT_ID           0x06

// ENDPOINT Termintal ID
#define SPEAKER_ENDPOINT_ID     0x03

// Interface ID
#define SPEAKER_INTERFACE_ID    0x02


// Feature Unit ID
#define MIC_FU_ID           0x02

// Input Termintal ID
#define MIC_IT_ID           0x01

// Output Termintal ID
#define MIC_OT_ID           0x03

// ENDPOINT Termintal ID
#define MIC_ENDPOINT_ID     0x82

// Interface ID
#define MIC_INTERFACE_ID    0x01
#else

#define SPEAKER_FU_ID           0x0D

// Input Termintal ID
#define SPEAKER_IT_ID           0x01

// Output Termintal ID
#define SPEAKER_OT_ID           0x03

// ENDPOINT Termintal ID
#define SPEAKER_ENDPOINT_ID     0x03

// Interface ID
#define SPEAKER_INTERFACE_ID    0x01

#endif

typedef struct _BUFFER_TRANS
{
    T_BOOL bFull;
    T_U8 *pBuffer;
}T_BUFFER_TRANS, *T_pBUFFER_TRANS;

typedef enum
{
    CTRL_CUR,
    CTRL_MIN,
    CTRL_MAX,
    CTRL_RES    
}T_CTRL_ID;

typedef struct _UAC_PIPE
{
 T_U32              len;
 T_U32              wr;
 T_U32              rd;
 T_U8               *buf;
}T_UAC_PIPE;

typedef struct _UAC_RESAMPLE
{
 T_U32              len;
 T_U32              wr;
 T_U32              rd;
 T_U8               *buf;
}T_UAC_RESAMPLE;

typedef enum 
{
    MSG_AC_CTRL,   
    MSG_AS_CTRL,
    MSG_PCM_SENT
}T_eUVC_DEV_MSG_ID;

typedef enum
{
    SAMPLE_SLOW,
    SAMPLE_FAST,
    SAMPLE_EQUA  
}T_SYNCH_STATE;

typedef struct _UAC_DEV_MSG
{
    T_U8           ucMsgId;
    T_U8           ucParam1;
    T_U8           ucParam2;
    T_U32          ulParam3;
    T_U32          ulParam4;
}T_UAC_DEV_MSG,*T_pUAC_DEV_MSG;

typedef struct _UAC_CONTROL_SETTING
{
    T_U32   ulCur;
    T_U32   ulMin;
    T_U32   ulMax;
    T_U32   ulRes;
    T_U32   ulLen;

}T_UAC_CONTROL_SETTING,*T_pUAC_CONTROL_SETTING;

typedef struct _UAC_DEV
{
    T_U32           ulMode;
    T_UAC_DEV_MSG   tMsg;   
#ifdef EXTERN_PCM_PLAYER
	T_pUAC_PCM_SETBUF_CB fPCM_SETBUF_CB;
	T_pUAC_PCM_POST_CB fPCM_POST_CB;
	T_U8			*mFrameBuf;

	T_pUAC_SET_SR_CB fSR_SET_CB;

#else
	T_UAC_PIPE      tTrans;
	T_pUAC_PCM_SENT_CALLBACK fPCMSentCallBack;
#endif
    T_UAC_CONTROL_SETTING tCtrlSetting[UAC_SUPPORTED_CTRL_NUM];
    T_pUAC_AC_CTRL_CALLBACK fAcCtrlCallBack;
    
}T_UAC_DEV, *T_pUAC_DEV;

/**
 * @brief Define uac controls implemented
 */


#ifdef __cplusplus
}
#endif

#endif


