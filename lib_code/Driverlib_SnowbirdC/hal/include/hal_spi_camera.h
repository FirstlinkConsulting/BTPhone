/**
 * @file hal_spi_camera.h
 * @brief spi camera driver interface file.
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author lu_heshan
 * @date 2013-02-04
 * @version 1.0
 */
#ifndef _HAL_SPI_CAMERA_H_
#define _HAL_SPI_CAMERA_H_


#include "anyka_types.h"
#include "arch_spi.h"

/**
 * @brief spi camera open, must be call first
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param spi_id[in] spi interface selected
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera init successfully.
 * @retval AK_FALSE: spi camera init failed.
 */
T_BOOL spi_cam_open(T_eSPI_ID spi_id);


/**
 * @brief spi_cam_set_info,must be call second
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param[in] width[in] the window width
 * @param[in] height[in] the window height
 * @param[in] dst_bufA      the ping-pang buffer A to save the image data 
 * @param[in] dst_bufB      the ping-pang buffer B to save the image data 
 * @param[in] line_int_num the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_OK, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL spi_cam_set_info(T_U32 width, T_U32 height, T_U8 *dst_bufA, T_U8 *dst_bufB,
                               T_U32 line_int_num, T_fCAMCALLBACK callback_func);

/**
 * @brief spi_cam_start_capture
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi capture failed, else spi camera capture successfully
 */
T_BOOL spi_cam_start_capture(T_VOID);

/**
 * @brief spi_cam_stop_capture
 * @author Lu_Heshan
 * @date 2013-04-02
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi stop capture failed, else spi camera stop capture successfully
 */
T_BOOL spi_cam_stop_capture(T_VOID);

/**
 * @brief spi_cam_close,close camera and spi, free buffer.
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param T_VOID
 * @return T_BOOL
 * @retval
 */
T_BOOL spi_cam_close(T_VOID);
#endif

