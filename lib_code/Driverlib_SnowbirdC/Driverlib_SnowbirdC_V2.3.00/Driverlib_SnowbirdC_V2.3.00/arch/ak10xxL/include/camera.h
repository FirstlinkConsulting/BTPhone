/*******************************************************************************
 * @file camera.h
 * @brief Define structures and functions of camera driver
 * This file provide APIs of Camera, such as open, close, capture image. etc.
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2010-12-10
 * @version 1.0
*******************************************************************************/
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "hal_camera.h"

typedef T_VOID (*T_fCamera_Interrupt_CALLBACK)(T_VOID);

typedef enum
{
    CAMERA_CUR_BUFA = 0,
    CAMERA_CUR_BUFB,
    CAMERA_CUR_NULL
}T_CAMERA_CUR_BUF;

//dara format: 00:JPEG;01:YUV420;10:ONLY Y;11:RAW RGB.
typedef enum
{
    CAMERA_DATA_MODE_JPEG = 0,
    CAMERA_DATA_MODE_YUV,
    CAMERA_DATA_MODE_Y,
    CAMERA_DATA_MODE_RAW,
    CAMERA_DATA_MODE_NUM
 }T_CAMERA_DATA_MODE;
 
 /**
  * @brief enable camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return T_VOID
  */
 T_VOID camctrl_enable(T_VOID);
 
 /**
  * @brief close camera 
  * @author xia_wenting  
  * @date 2010-12-06
  * @return T_VOID
  */
 T_VOID camctrl_disable(T_VOID);
 
 /**
  * @brief open camera, should be done the after reset camera to  initialize 
  * @author xia_wenting  
  * @date 2010-12-06
  * @param[in] mclk camera mclk, unit: 100KHz
  * @return T_BOOL
  * @retval AK_TRUE if successed
  * @retval AK_FALSE if failed
  */
 T_BOOL camctrl_open(T_U32 mclk);
 
 /**
  * @brief Set clip windows
  * @author xia_wenting
  * @date 2010-12-16
  * @param[in] clip_hoff clip horizontal offsize
  * @param[in] clip_voff clip vertical offsize
  * @param[in] clip_hsize clip width
  * @param[in] clip_vsize clip height
  * @return T_BOOL
  * @retval AK_FLASE if error mode 
  * @retval AK_TURE  if success
  */
 T_BOOL camctrl_clip(T_U32 clip_hoff, T_U32 clip_voff, T_U32 clip_hsize, T_U32 clip_vsize);
 
 /**
  * @brief capture an image in YUV/only Y/RAW/JPEG data format
  * @author luheshan
  * @date 2013-01-07
  * @param[in] buf_A          the ping-pang buffer A to save the image data
  * @param[in] buf_B          the ping-pang buffer B to save the image data
  * @param[in] int_cnt        the line number of  a frame,if data format is JPEG,int_cnt is the buf size(byte)
  * @param[in] format         data format.
  * @return T_BOOL
  * @retval AK_TRUE if success
  * @retval AK_FALSE if buffer is null
  */
 T_BOOL camctrl_capture_data(T_pVOID buf_A, T_pVOID buf_B, T_U32 int_cnt, 
                                T_CAMERA_DATA_MODE format);
 
 /**
  * @brief set camera controller's register,is source and dest size
  * @author luheshan
  * @date 2013-01-07
  * @param[in] srcWidth  source width, output width of camera sensor
  * @param[in] srcHeight source height, output height of camera sensor
  * @param[in] dstWidth  desination width, the actual width of image in buffer 
  * @param[in] dstHeight desination height, the actual height of image in buffer 
  * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
  * @return T_BOOL,if need clip,will return AK_TRUE.
  */
 T_BOOL camctrl_set_info(T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, 
                         T_U32 dstHeight, T_BOOL bIsCCIR656);
 
 
 /**
  * @brief set interrupt callback function
  * @author xia_wenting  
  * @date 2010-12-01
  * @param
  * @return 
  * @retval 
  */
 T_VOID camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func);
 
 /**
  * @brief read camera controller's register, and check the frame finished or occur errorred
  * @author xia_wenting   
  * @date 2010-12-06
  * @param
  * @return T_BOOL
  * @retval AK_TRUE the frame finished
  * @retval AK_FALSE the frame not finished or occur errorred
  */
 T_BOOL camctrl_check_status(T_VOID);
 
 /**
  * @brief read camera controller's register, and check line interruption status
  * @author lu_heshan   
  * @date 2012-12-31
  * @param[in\out]:cur_buf, get the cur_buf,A or B.
  * @return T_BOOL
  * @retval AK_TRUE the line interruption
  * @retval AK_FALSE the the line no interruption
  */
 T_BOOL camctrl_check_buf_status(T_CAMERA_CUR_BUF *cur_buf); 
 
 /**
  * @brief start camera controller to capture a frame
  * @author lu_heshan
  * @date 2013-01-07
  * @return T_VOID
  */
 T_VOID camctrl_capture_frame(T_VOID);
 
 /**
  * @brief to set auto mode to capture a frame
  * @       auto_mode:flase:every frame capture needs system soft start.
  * @                       ture:can capture without soft start.
  * @author lu_heshan
  * @date 2013-01-07
  * @param[in]:bauto, if ture,enable auto mede; if flase, stop auto mode.
  * @return T_BOOL
  * @return T_VOID
  */
 T_VOID camctrl_set_auto_mode(T_BOOL bauto);
 
 /**
  * @brief to get jpeg data length
  * @author lu_heshan
  * @date 2013-01-07
  * @param[in]:T_VOID
  * @return T_U32
  * @return can get the jpeg length when the capture a frame is end.
  */
 T_U32 camctrl_get_jpeg_len(T_VOID);
#endif //__CAMERA_H__

