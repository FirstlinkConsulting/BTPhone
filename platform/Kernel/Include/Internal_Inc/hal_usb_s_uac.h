/**
*@file hal_usb_s_uac.h
*@brief provide operations of how to use USB Audio Class device.
*
*This file describe frameworks of USB Audio Class device
*Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
*@author LiuHuadong
*@date 2012-05-4
*@version 1.0
*/

#ifndef _HAL_USB_S_UAC_H_
#define _HAL_USB_S_UAC_H_

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_UAC USB_UAC
 *  @ingroup USB
 */
/*@{*/

//  Feature Unit Control Selectors
#define     FU_CONTROL_UNDEFINED                            0x00
#define     MUTE_CONTROL                                    0x01
#define     VOLUME_CONTROL                                  0x02
#define     BASS_CONTROL                                    0x03
#define     MID_CONTROL                                     0x04
#define     TREBLE_CONTROL                                  0x05
#define     GRAPHIC_EQUALIZER_CONTROL                       0x06
#define     AUTOMATIC_GAIN_CONTROL                          0x07
#define     DELAY_CONTROL                                   0x08
#define     BASS_BOOST_CONTROL                              0x09
#define     LOUDNESS_CONTROL                                0x0a

#define     RECIPIENT_TYPE_ENTITY_FU_ID                     0x00
#define     RECIPIENT_TYPE_ENDPOINT_ID                      0x01

//Endpoint Control Selectors
#define     EP_CONTROL_UNDEFINED                            0x00
#define     SAMPLING_FREQ_CONTROL                           0x01
#define     PITCH_CONTROL                                   0x02

#define     EXTERN_PCM_PLAYER           1
//#define UAC_MIC_FUN                     0x01


#define SET_SENDFLAG(flag, type, val) ((val)? ((flag) |= (1 << (type))) : ((flag) &= ~(1 << (type))))
#define CHK_SENDFLAG(flag, type) ((flag) & (1 << type))

//check l2 send data state.
typedef enum
{
    L2_REQUEST = 0x00,
    DAC_SIGNAL,
    L2_SENDING
}L2_SEND_DATA_STATE;


typedef enum
{
    UAC_CTRL_VOLUME,             ///< uvc device brightness control selector
    UAC_CTRL_MUTE,               ///< uvc device contrast control selector
    UAC_CTRL_LOUNDNESS,          ///< uvc device saturation control selector 
    UAC_CTRL_RESOLUTION ,        ///< uvc device resolution control selector
    UAC_CTRL_NUM                 ///< uvc device control number
}T_eUAC_CONTROL;

typedef enum
{
    UAC_VOLUME_CTRL_INC,
    UAC_VOLUME_CTRL_DEC,
#ifdef UAC_HID_EX    
    UAC_VOLUME_CTRL_MUTE,
    UAC_VOLUME_CTRL_PRE,
    UAC_VOLUME_CTRL_NEXT,    
    UAC_VOLUME_CTRL_PLAY,
    UAC_VOLUME_CTRL_PAUSE,       
#endif    
    UAC_VOLUME_CTRL_RELEASE
}T_eUAC_VOLCTRL;



typedef T_VOID (*T_pUAC_AC_CTRL_CALLBACK)(T_eUAC_CONTROL dwControl,T_U8 type, T_U32 value);
typedef T_VOID (*T_pUAC_PCM_SENT_CALLBACK)(T_U8 *data,T_U32 len);

//#ifdef PCM_PLAYER
typedef T_U32 (*T_pUAC_PCM_SETBUF_CB)(T_U32 *pBuf, T_U32 size);
typedef T_VOID (*T_pUAC_PCM_POST_CB)(T_U32 sampleRate);
typedef T_VOID (*T_pUAC_SET_SR_CB)(T_U32 sampleRate);
//#endif
/** 
 * @brief set UAC callback
 *
 *This function is called by application level to set control callback after uac_init successful.
 * @author LiuHuadong
 * @date 2012-05-10
 * @param  ac_ctrl_callback[in] implement audio control interface controls 
 * @param pcm_sent_callback[in] implement pcm stream sent  controls 
 * @return T_VOID
 */
T_VOID uac_set_callback(T_pUAC_AC_CTRL_CALLBACK ac_ctrl_callback, 
                            T_pUAC_PCM_SENT_CALLBACK pcm_sent_callback);

/**
* @brief Initialize uac descriptor
* @author LiuHuadong
* @date 2012-05-04
* @param mode[in] usb2.0 or usb1.0 
* @param format[in] The UAC frame format
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TRUE means successful
*/
T_BOOL uac_init(T_U32 mode,T_U32 size);

/**
 * @brief  enter uac task,poll uac msg
 *
 * This function is added for  bios version,and must be call after usbdisk_start
 * @author LiuHuadong
 * @date  2012-05-04
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE just received a uac command
 * @retval AK_FALSE currently no uac data
 */
T_BOOL uac_proc(T_VOID);

/**
* @brief    Start UAC
* @author LiuHuadong
* @date 2010-05-04
* @return T_BOOL
*/
T_BOOL uac_start(T_VOID);

/**
* @brief    Stop UAC
* @author LiuHuadong
* @date 2012-05-04
* @return   VOID
*/
T_VOID uac_stop(T_VOID);

/**
* @brief    set UAC volume
* @author LiuHuadong
* @date 2012-05-04
* @return   VOID
*/
T_BOOL uac_volume_set_func(T_eUAC_VOLCTRL type);

/**
* @brief    GET UAC PCM DATA
* @author LiuHuadong
* @date 2012-05-04
* @return   T_BOOL
*/
T_BOOL uac_get_pcm_data(T_pVOID pfilter);

/**
* @brief    GET UAC PCM DATA
* @author LiuHuadong
* @date 2012-05-04
* @return   T_BOOL
*/
T_BOOL uac_send_pcm_data(T_VOID);

/**
* @brief    SENT UAC PCM DATA
* @author LiuHuadong
* @date 2012-05-04
* @return   T_BOOL
*/
T_BOOL uac_send(T_U8 *data_buf, T_U32 length);

T_VOID uac_pcm_set_callback(T_pUAC_PCM_SETBUF_CB fGetBuf_cb, T_pUAC_PCM_POST_CB fPost_cb);
T_VOID uac_sr_set_callback(T_pUAC_SET_SR_CB fSR_set_cb);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif    //_HAL_USB_S_UAC_H_
