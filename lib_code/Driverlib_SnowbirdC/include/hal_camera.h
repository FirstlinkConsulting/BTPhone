/*******************************************************************************
 * @file hal_camera.h
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author lu_heshan
 * @date 2013-01-14
 * @version 1.0
 * @note this an example to use the API of camera driver
*******************************************************************************/
#ifndef __HAL_CAMERA_H__
#define __HAL_CAMERA_H__


#include "anyka_types.h"
#include "arch_spi.h"

/** @defgroup Hal_camera Hardware Abstract Layer of camera
 *    @ingroup Camera
 */
/*@{*/

/******************************************************************************************
 *    the following define the camera device register interface *      
******************************************************************************************/

/** @brief Camera Mode definition
 *
 *  This structure define the mode list of camera
 */
typedef enum
{
    CAMERA_MODE_QSXGA = 0, //2592*1944
    CAMERA_MODE_QXGA,      //2048*1536
    CAMERA_MODE_UXGA ,     //1600*1200
    CAMERA_MODE_SXGA,      //1280*1024
    CAMERA_MODE_XGA,       //1024*768
    CAMERA_MODE_SVGA,      //800*600
    CAMERA_MODE_VGA,       //640*480 
    CAMERA_MODE_QSVGA,     //400*300
    CAMERA_MODE_CIF,       //352*288
    CAMERA_MODE_QVGA,      //320*240
    CAMERA_MODE_QCIF,      //176*144
    CAMERA_MODE_QQVGA,     //160*120
    CAMERA_MODE_PREV,
    CAMERA_MODE_REC,
    CAMERA_MODE_VGA_JPEG,
    CAMERA_MODE_QXGA_JPEG,
    CAMERA_MODE_QSXGA_JPEG,
    CAMERA_MODE_NUM
} T_CAMERA_MODE;


/** @brief Camera Parameter Night Mode definition
 *
 *  This structure define the value of parameter Night Mode
 */
typedef enum
{
    CAMERA_DAY_MODE,
    CAMERA_NIGHT_MODE,
    CAMERA_NIGHT_NUM
}T_NIGHT_MODE;


/** @brief Camera Parameter CCIR601/656 protocol
 *
 *    This structure define the CMOS sensor compatible with CCIR601 or CCIR656 protocol
 */
typedef enum
{
    CAMERA_CCIR_601,
    CAMERA_CCIR_656,
    CAMERA_CCIR_NUM
}T_CAMERA_INTERFACE;

/** @brief Camera Parameter Exposure definition
 *
 *  This structure define the value of parameter Exposure
 */
typedef enum
{
    EXPOSURE_WHOLE = 0,
    EXPOSURE_CENTER,
    EXPOSURE_MIDDLE,
    CAMERA_EXPOSURE_NUM
}T_CAMERA_EXPOSURE;

/** @brief Camera Parameter AWB definition
 *
 *  This structure define the value of parameter AWB
 */
typedef enum
{
    AWB_AUTO = 0,
    AWB_SUNNY,
    AWB_CLOUDY,
    AWB_OFFICE,
    AWB_HOME,
    AWB_NIGHT,
    AWB_NUM
}T_CAMERA_AWB;


/** @brief Camera Parameter Brightness definition
 *
 *  This structure define the value of parameter Brightness
 */
typedef enum
{
    CAMERA_BRIGHTNESS_0 = 0,
    CAMERA_BRIGHTNESS_1,
    CAMERA_BRIGHTNESS_2,
    CAMERA_BRIGHTNESS_3,
    CAMERA_BRIGHTNESS_4,
    CAMERA_BRIGHTNESS_5,
    CAMERA_BRIGHTNESS_6,
    CAMERA_BRIGHTNESS_NUM
}T_CAMERA_BRIGHTNESS;


/** @brief Camera Parameter Contrast definition
 *
 *  This structure define the value of parameter Contrast
 */
typedef enum 
{
    CAMERA_CONTRAST_1 = 0,
    CAMERA_CONTRAST_2,
    CAMERA_CONTRAST_3,
    CAMERA_CONTRAST_4,
    CAMERA_CONTRAST_5,
    CAMERA_CONTRAST_6,
    CAMERA_CONTRAST_7,
    CAMERA_CONTRAST_NUM
}T_CAMERA_CONTRAST;

/** @brief Camera Parameter Saturation definition
 *
 *  This structure define the value of parameter Saturation
 */
typedef enum
{
    CAMERA_SATURATION_1 = 0,
    CAMERA_SATURATION_2,
    CAMERA_SATURATION_3,
    CAMERA_SATURATION_4,
    CAMERA_SATURATION_5,
    CAMERA_SATURATION_NUM
}T_CAMERA_SATURATION;

/** @brief Camera Parameter Sharpness definition
 *
 *  This structure define the value of parameter Sharpness
 */
typedef enum
{
    CAMERA_SHARPNESS_0 = 0,
    CAMERA_SHARPNESS_1,
    CAMERA_SHARPNESS_2,
    CAMERA_SHARPNESS_3,
    CAMERA_SHARPNESS_4,
    CAMERA_SHARPNESS_5,
    CAMERA_SHARPNESS_6,
    CAMERA_SHARPNESS_NUM
}T_CAMERA_SHARPNESS;

/** @brief Camera Parameter Mirror definition
 *
 *  This structure define the value of parameter Mirror
 */
typedef enum
{
    CAMERA_MIRROR_V = 0,
    CAMERA_MIRROR_H,
    CAMERA_MIRROR_NORMAL,
    CAMERA_MIRROR_FLIP,
    CAMERA_MIRROR_NUM
}T_CAMERA_MIRROR;

/** @brief Camera Parameter Effect definition
 *
 *  This structure define the value of parameter Effect
 */
typedef enum
{
    CAMERA_EFFECT_NORMAL = 0,
    CAMERA_EFFECT_SEPIA,
    CAMERA_EFFECT_ANTIQUE,
    CAMERA_EFFECT_BLUE,
    CAMERA_EFFECT_GREEN,
    CAMERA_EFFECT_RED,
    CAMERA_EFFECT_NEGATIVE,
    CAMERA_EFFECT_BW,
    CAMERA_EFFECT_BWN,    
    CAMERA_EFFECT_AQUA,    // PO1200 additional mode add by Liub 20060918
    CAMERA_EFFECT_COOL,
    CAMERA_EFFECT_WARM,
    CAMERA_EFFECT_NUM
}T_CAMERA_EFFECT;

/** @brief Camera type definition
 *
 *  This structure define the type of camera
 */
typedef enum
{
    CAMERA_P3M  = 0x00000001,
    CAMERA_1P3M = 0x00000002,
    CAMERA_2M   = 0x00000004,
    CAMERA_3M   = 0x00000008,
    CAMERA_4M   = 0x00000010,
    CAMERA_5M   = 0x00000020,
    CAMERA_ZOOM = 0x00000040
}T_CAMERA_TYPE;


/** @brief Camera Parameter Feature definition
 *
 *  This structure define the feature list of camera
 */
typedef enum {
    CAM_FEATURE_NIGHT_MODE = 0,
    CAM_FEATURE_EXPOSURE,
    CAM_FEATURE_AWB,
    CAM_FEATURE_BRIGHTNESS,
    CAM_FEATURE_CONTRAST,
    CAM_FEATURE_SATURATION,
    CAM_FEATURE_SHARPNESS,
    CAM_FEATURE_MIRROR,
    CAM_FEATURE_EFFECT,
    CAM_FEATURE_NUM
}T_CAMERA_FEATURE;


/** @brief Camera Capture definition
 *
 *  This structure define the buffer status
 */
typedef enum
{
    CAMERA_LINE_OK = 0,
    CAMERA_FRAME_OK,
    CAMERA_FRAME_ERR
}T_CAMERA_INTR_STATUS;

#define SPI_CAM_SPIIF        SPI_ID1

typedef T_VOID (*T_fCAMCALLBACK)(T_CAMERA_INTR_STATUS intr_status, 
                                       T_U8 *cur_buf, T_U32 line_index);
typedef struct 
{
    T_U8 *buf_y;
    T_U8 *buf_u;
    T_U8 *buf_v;
}T_YUV_DATA_BUF, *T_pYUV_DATA_BUF;

/**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author lu_heshan
 * @date 2013-04-13
 * @param[in] enable_pin: gpio for camera enable pin
 * @param[in] reset_pin: gpio for camera reset pin
 * @return T_BOOL
 * @retval if successed, return camera handler
 * @retval if failed AK_NULL
 */
T_BOOL cam_open(T_U32 enable_pin, T_U32 reset_pin);

/**
 * @brief close camera 
 * @author lu_heshan  
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL cam_close(T_VOID);

/**
 * @brief set an image  info, just for only Y data format.
 * @author luheshan 
 * @date 2013-01-07
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] dst_bufA      the ping-pang buffer A to save the image data 
 * @param[in] dst_bufB      the ping-pang buffer B to save the image data 
 * @param[in] line_cnt       the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_OK, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if fail
 */
T_BOOL cam_set_info(T_U32 dstWidth, T_U32 dstHeight, T_U8 *dst_bufA, T_U8 *dst_bufB,
                               T_U32 line_cnt, T_fCAMCALLBACK callback_func);

/**
 * @brief start camera controller to capture a frame
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL cam_start_capture(T_VOID);


/**
 * @brief cam_stop_capture
 * @author Lu_Heshan
 * @date 2013-04-02
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi stop capture failed, else spi camera stop capture successfully
 */
T_BOOL cam_stop_capture(T_VOID);

/**
 * @brief set camera feature 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] feature_type camera feature type,such as night mode,AWB,contrast..etc.
 * @param[in] feature_setting feature setting
 * @return T_VOID
 */
T_VOID cam_set_feature(T_CAMERA_FEATURE feature_type, T_U8 feature_setting);

/**
 * @brief Set camera frame rate manually
 * @author xia_wenting  
 * @date 2011-01-05
 * @param[in] framerate camera output framerate
 * @return T_U32
 * @retval 0 if error mode 
 * @retval 1 if success
 */
T_U32 cam_set_framerate(float framerate);

/*@}*/
#endif

