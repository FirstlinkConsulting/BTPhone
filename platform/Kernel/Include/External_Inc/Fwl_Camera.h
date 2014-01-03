/**
 * @file Fwl_Camera.h
 * @brief This header file is for camera function prototype
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */
#ifndef __FWL_CAMERA_H__
#define __FWL_CAMERA_H__


#include "akdefine.h"
#include "anyka_types.h"

#ifdef CAMERA_SUPPORT

typedef enum
{
    FWL_CAM_LINE_OK = 0,
    FWL_CAM_FRAME_OK,
    FWL_CAM_FRAME_ERR
}T_FWL_CAM_INTR;

typedef T_VOID (*T_FWL_CAMCB)(T_FWL_CAM_INTR status, 
                                     T_U8 *cur_buf, T_U32 line_index);

/*
 * @brief  the function init camera
 * @author luheshan
 * @date 2013-4-13
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL Fwl_CameraInit(T_VOID);

/*
 * @brief  the function close camera
 * @author luheshan
 * @date 2013-4-13
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL Fwl_CameraFree(T_VOID);

/*
 * @brief  the function close camera
 * @author luheshan
 * @date 2013-4-13
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] line_cnt       the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_OK, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL Fwl_CameraSetInfo(T_U32 dstWidth, T_U32 dstHeight, 
                                T_U32 line_cnt, T_FWL_CAMCB callback_func);

/**
 * @brief start camera controller to capture
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL Fwl_CameraCaptureStart(T_VOID);

/**
 * @brief stop camera controller to capture
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL Fwl_CameraCaptureStop(T_VOID);

#endif

#endif //__FWL_CAMERA_H__

