/*******************************************************************************
 * @file hal_camera.c
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting ZGX
 * @date 2012-11-07
 * @version 1.0
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "share_pin.h"
#include "clk.h"
#include "camera.h"
#include "drv_camera.h"
#include "interrupt.h"
#include "hal_probe.h"
#include "drv_module.h"
#include "arch_init.h"
#include "drv_cfg.h"
#include "hal_errorstr.h"

#if DRV_SUPPORT_CAMERA > 0

#if CAMERA_GC6113 > 0
#define USE_SPI_CAMERA
#endif

//max 640x480; min:4x16/8x8/16x4
#define CHECK_SIZE_VALID(w,h)       ((((w) <= 640) && ((h) <= 480))\
                                     || (((w) >= 4) && ((h) >= 16))\
                                     || (((w) >= 8) && ((h) >= 8))\
                                     || (((w) >= 16) && ((h) >= 4)))

#define VGA_WIDTH                   640
#define VGA_HEIGHT                  480

typedef struct 
{
    T_fCAMCALLBACK DataNotifyCallbackFunc;    
    T_CAMERA_FUNCTION_HANDLER *pCameraHandler; 
    T_CAMERA_DATA_MODE cam_data_mode;
    T_U32   *jpeg_len;           //save jpeg lenght
    T_U8    *buf_A;
    T_U8    *buf_B;
    T_U32   line_index;
    T_U32   line_cnt;
    T_U16   drv_output_width;    //output width of camera driver
    T_U16   drv_output_height;   //output height of camera driver
    T_U16   sensor_output_width; //output width of camera sensor, maybe include window function(done by sensor itself or AK chip)
    T_U16   sensor_output_height;//output height of camera sensor, maybe include window function(done by sensor itself or AK chip)
    T_BOOL  bIsCCIR656;
    T_BOOL  open_flg;            //if ture,camera is open.
}T_HAL_PARAM;


#pragma arm section zidata = "_drvbootbss_"
static volatile T_HAL_PARAM m_hal_param;
T_U32 gpio_camera_enable;
T_U32 gpio_camera_reset;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_CAM;
#pragma arm section rodata

static T_VOID cam_interrupt_callback(T_VOID);

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting
 * @date 2010-12-06
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
static T_BOOL cam_init(T_VOID)
{
    T_U32 i = 0;
    T_BOOL camera_open_flag = AK_FALSE;
  
    m_hal_param.pCameraHandler = cam_probe();
    if (m_hal_param.pCameraHandler != AK_NULL)
    {        
        for (i = 0; i < 3; i++)
        {
            if (m_hal_param.pCameraHandler->cam_init_func() != AK_FALSE)
            {
                camera_open_flag = AK_TRUE;
                break;
            }
            delay_us(100000);
        }

        if (AK_FALSE == camera_open_flag)
        {
            m_hal_param.pCameraHandler->cam_close_func();
            m_hal_param.pCameraHandler = AK_NULL;
        }
    }
    
    return camera_open_flag;
}

#pragma arm section code = "_drvbootcode_"
static T_VOID cam_interrupt_callback(T_VOID)
{    
    T_CAMERA_CUR_BUF cur_buf = CAMERA_CUR_NULL;
    T_U8 *pBuf;
    
    if (AK_TRUE == camctrl_check_buf_status(&cur_buf))
    {
        if(m_hal_param.DataNotifyCallbackFunc != AK_NULL)
        {
            if (CAMERA_CUR_BUFA == cur_buf)
            {
                pBuf = m_hal_param.buf_A;
            }
            else
            {
                pBuf = m_hal_param.buf_B;
            }
            
            m_hal_param.line_index += m_hal_param.line_cnt;
            
            m_hal_param.DataNotifyCallbackFunc(CAMERA_LINE_OK, pBuf, 
                                                    m_hal_param.line_index);
        }
        
        return;
    }
    
    if (AK_TRUE == camctrl_check_status())    
    {
        if (CAMERA_DATA_MODE_JPEG == m_hal_param.cam_data_mode)
        {
            if (AK_NULL != m_hal_param.jpeg_len)
            {
                *m_hal_param.jpeg_len = camctrl_get_jpeg_len();
            }
        }
        
        if(m_hal_param.DataNotifyCallbackFunc != AK_NULL)
        {
            m_hal_param.DataNotifyCallbackFunc(CAMERA_FRAME_OK, AK_NULL, 
                                                    m_hal_param.line_index);
        }
    }
    else //accept a error frame 
    {
        drv_print(err_str, __LINE__, AK_TRUE);

        if(m_hal_param.DataNotifyCallbackFunc != AK_NULL)
        {
            m_hal_param.DataNotifyCallbackFunc(CAMERA_FRAME_ERR, AK_NULL, 
                                                    m_hal_param.line_index);
        }
    }
}
#pragma arm section code

/**
 * @brief set camera window
 * @author  lu_heshan
 * @date 2013-01-06
 * @param[in] srcWidth window or clip width
 * @param[in] srcHeight window or clip height
 * @return T_BOOL
 * @retval  AK_TRUE if success
 * @retval AK_FALSE if fail
 */
static T_BOOL cam_set_window(T_U32 dstWidth, T_U32 dstHeight)
{
    T_U32 clip_hoff;
    T_U32 clip_voff;
    T_U32 strWidth;
    T_U32 strHeight;
    T_BOOL clip_en;
    
    if (!CHECK_SIZE_VALID(dstWidth, dstHeight))
    {
        drv_print("CHECK_SIZE_VALID fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }

    if ((dstWidth > m_hal_param.sensor_output_width) 
        || (dstHeight > m_hal_param.sensor_output_height))
    {
        drv_print("dstWidth or dstHeight is err\n", 0, AK_FALSE);
        return AK_FALSE;
    }

    strWidth = m_hal_param.sensor_output_width;
    strHeight = m_hal_param.sensor_output_height;
    m_hal_param.drv_output_width = dstWidth;
    m_hal_param.drv_output_height = dstHeight;

    //set info
    clip_en = camctrl_set_info(strWidth, strHeight, dstWidth, dstHeight, 
                               m_hal_param.bIsCCIR656);

    if (AK_TRUE == clip_en)
    {   
        clip_hoff = (strWidth - dstWidth) / 2;
        clip_voff = (strHeight - dstHeight) / 2;

        return camctrl_clip(clip_hoff, clip_voff, dstWidth, dstHeight);
    }

    return AK_TRUE;
}

/**
 * @brief Set camera night mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] mode night mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_night_mode(T_NIGHT_MODE mode)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) && 
        (AK_NULL != m_hal_param.pCameraHandler->cam_set_night_mode_func))
    {
        m_hal_param.pCameraHandler->cam_set_night_mode_func(mode);
        return;
    }
    drv_print("cam_set_night_mode is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_exposure(T_CAMERA_EXPOSURE exposure)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_exposure_func))
    {
        m_hal_param.pCameraHandler->cam_set_exposure_func(exposure);
        return;
    }
    drv_print("cam_set_exposure is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_AWB(T_CAMERA_AWB awb)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_AWB_func))
    {
        m_hal_param.pCameraHandler->cam_set_AWB_func(awb);
        return;
    }
    drv_print("cam_set_AWB is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera brightness level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_brightness_func))
    {
        m_hal_param.pCameraHandler->cam_set_brightness_func(brightness);
        return;
    }
    drv_print("cam_set_brightness is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera contrast level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_contrast(T_CAMERA_CONTRAST contrast)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_contrast_func))
    {
        m_hal_param.pCameraHandler->cam_set_contrast_func(contrast);
        return;
    }
    drv_print("cam_set_contrast is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera saturation level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_saturation(T_CAMERA_SATURATION saturation)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_saturation_func))
    {
        m_hal_param.pCameraHandler->cam_set_saturation_func(saturation);
        return;
    }
    drv_print("cam_set_saturation is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera sharpness level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_sharpness_func))
    {
        m_hal_param.pCameraHandler->cam_set_sharpness_func(sharpness);
        return;
    }
    drv_print("cam_set_sharpness is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_mirror(T_CAMERA_MIRROR mirror)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_mirror_func))
    {
        m_hal_param.pCameraHandler->cam_set_mirror_func(mirror);
        return;
    }
    drv_print("cam_set_mirror is not supported", 0, AK_TRUE);
}

/**
 * @brief Set camera effect mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_set_effect(T_CAMERA_EFFECT effect)
{
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_effect_func))
    {
        m_hal_param.pCameraHandler->cam_set_effect_func(effect);
        return;
    }
    drv_print("cam_set_effect is not supported", 0, AK_TRUE);
}


/**
 * @brief open camera, should be done the after reset camera to initialize 
 * @author xia_wenting
 * @date 2010-12-06
 * @param[in] enable_pin: gpio for camera enable pin
 * @param[in] reset_pin: gpio for camera reset pin
 * @return T_BOOL
 * @retval if successed, return camera handler
 * @retval if failed AK_NULL
 */
T_BOOL cam_open(T_U32 enable_pin, T_U32 reset_pin)
{
    T_BOOL ret;
    
    gpio_camera_enable = enable_pin;
    gpio_camera_reset = reset_pin;
    
    //enable camera controller
    camctrl_enable();

#ifndef USE_SPI_CAMERA
    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_NUM;
#endif

    //init sensor
    if (AK_TRUE == cam_init())
    {
        //open camera controller
        ret = camctrl_open(m_hal_param.pCameraHandler->cam_mclk);      
    }
    else
    {
        drv_print("camera open fail", 0, AK_TRUE);
        camctrl_disable();
        ret =  AK_FALSE;
    }

    if (AK_TRUE == ret)
    {
#ifdef USE_SPI_CAMERA
        if (AK_FALSE == spi_cam_open(SPI_CAM_SPIIF))
        {
            camctrl_disable();
            return AK_FALSE;
        }
#endif
        m_hal_param.open_flg = AK_TRUE;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

/**
 * @brief close camera 
 * @author lu_heshan  
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL cam_close(T_VOID)
{
    T_BOOL ret = AK_TRUE;
    
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_close_func))
    {
        m_hal_param.pCameraHandler->cam_close_func();
        m_hal_param.pCameraHandler = AK_NULL;
    }

#ifdef USE_SPI_CAMERA
    ret = spi_cam_close();
#endif
    camctrl_disable();

    m_hal_param.open_flg = AK_FALSE;

    return ret;
}

#if 0
/**
 * @brief switch camera reg to preview mode
 * @author  luheshan
 * @date 2013-01-07
 * @param[in] srcWidth sensor out width
 * @param[in] srcHeight sensor out height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{        
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if (!CHECK_SIZE_VALID(srcWidth, srcHeight))
    {
        drv_print("check size fail", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_to_prev_func))
    {                
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;
        
        camctrl_set_auto_mode(AK_TRUE);
        
        return m_hal_param.pCameraHandler->cam_set_to_prev_func(srcWidth, srcHeight);
    }
    
    return AK_FALSE;    
}

/**
 * @brief switch from capture mode to capture mode
 * @author  luheshan
 * @date 2013-01-07
 * @param[in] srcWidth sensor out width
 * @param[in] srcHeight sensor out height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{        
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if (!CHECK_SIZE_VALID(srcWidth, srcHeight))
    {
        drv_print("check size fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_to_cap_func))
    {
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;
        
        camctrl_set_auto_mode(AK_FALSE);

        return m_hal_param.pCameraHandler->cam_set_to_cap_func(srcWidth, srcHeight);
    }

    return AK_FALSE;
}

/**
 * @brief switch camera to record mode
 * @author  luheshan
 * @date 2013-01-07
 * @param[in] srcWidth sensor out width
 * @param[in] srcHeight sensor out height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{        
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if (!CHECK_SIZE_VALID(srcWidth, srcHeight))
    {
        drv_print("check size fail", 0, AK_TRUE);
        return AK_FALSE;
    }
        
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_to_record_func))
    {
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;

        camctrl_set_auto_mode(AK_TRUE);
           
        return m_hal_param.pCameraHandler->cam_set_to_record_func(srcWidth, srcHeight);
    }

    return AK_FALSE;    
}


/**
 * @brief capture an image in RAW data format
 * @author luheshan 
 * @date 2013-01-07
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] dst_bufA      the ping-pang buffer A to save the image data 
 * @param[in] dst_bufB      the ping-pang buffer B to save the image data
 * @param[in] line_num      the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_INTR, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if fail
 */
T_BOOL cam_capture_RAW(T_U32 dstWidth, T_U32 dstHeight, T_U8 *dst_bufA, 
                              T_U8 *dst_bufB, T_U32 line_num, T_fCAMCALLBACK callback_func)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if ((AK_NULL == dst_bufA) || (AK_NULL == dst_bufB) 
        || (0 != ((T_U32)dst_bufA & 0x3)) || (0 != ((T_U32)dst_bufB & 0x3)))
    {
        drv_print("camera chk buf fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (AK_FALSE == cam_set_window(dstWidth, dstHeight))
    {
        drv_print("RAW cam_set_window fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }
    
    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_RAW;
    m_hal_param.DataNotifyCallbackFunc = callback_func;
    
    if (AK_NULL != m_hal_param.pCameraHandler->cam_set_RawRGB_func)
    {
        m_hal_param.pCameraHandler->cam_set_RawRGB_func(m_hal_param.sensor_output_width, 
                                                        m_hal_param.sensor_output_height);
    }
    
    return camctrl_capture_data(dst_bufA, dst_bufB, line_num, CAMERA_DATA_MODE_RAW);
}


/**
 * @brief capture an image in only Y data format
 * @author luheshan 
 * @date 2013-01-07
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] dst_bufA      the ping-pang buffer A to save the image data 
 * @param[in] dst_bufB      the ping-pang buffer B to save the image data 
 * @param[in] line_num      the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_INTR, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if fail
 */
T_BOOL cam_capture_only_Y(T_U32 dstWidth, T_U32 dstHeight, T_U8 *dst_bufA, T_U8 *dst_bufB,
                               T_U32 line_num, T_fCAMCALLBACK callback_func)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }

    if ((AK_NULL == dst_bufA) || (AK_NULL == dst_bufB) 
        || (0 != ((T_U32)dst_bufA & 0x3)) || (0 != ((T_U32)dst_bufB & 0x3)))
    {
        drv_print("camera chk buf fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (AK_FALSE == cam_set_window(dstWidth, dstHeight))
    {
        drv_print("only Y cam_set_window fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }

    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_Y;
    m_hal_param.DataNotifyCallbackFunc = callback_func;

    return camctrl_capture_data(dst_bufA, dst_bufB, line_num, CAMERA_DATA_MODE_Y);
}


/**
 * @brief capture an image in only Y data format
 * @author luheshan 
 * @date 2013-01-07
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] buf_A          the ping-pang buffer A to save the image data of Y,U,V
 * @param[in] buf_B          the ping-pang buffer B to save the image data of Y,U,V
 * @param[in] line_num      the line number of line interruption
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_INTR, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end.
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if fail
 */
T_BOOL cam_capture_YUV(T_U32 dstWidth, T_U32 dstHeight, T_fCAMCALLBACK callback_func,
                                T_pYUV_DATA_BUF buf_A, T_pYUV_DATA_BUF buf_B, T_U32 line_num)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }

    if ((AK_NULL == buf_A) || (AK_NULL == buf_B) \
        || (AK_NULL == buf_A->buf_y) || (AK_NULL == buf_A->buf_u) 
        || (AK_NULL == buf_A->buf_v) || (AK_NULL == buf_B->buf_y) 
        || (AK_NULL == buf_B->buf_u) || (AK_NULL == buf_B->buf_v)
        || (0 != ((T_U32)buf_A->buf_y & 0x3)) || (0 != ((T_U32)buf_A->buf_u & 0x3))
        || (0 != ((T_U32)buf_A->buf_v & 0x3)) || (0 != ((T_U32)buf_B->buf_y & 0x3))
        || (0 != ((T_U32)buf_B->buf_u & 0x3)) || (0 != ((T_U32)buf_B->buf_v & 0x3)))
    {
        drv_print("YUV check buf fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }

    if (AK_FALSE == cam_set_window(dstWidth, dstHeight))
    {
        drv_print("YUV cam_set_window fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }
    
    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_YUV;
    m_hal_param.DataNotifyCallbackFunc = callback_func;

    return camctrl_capture_data(buf_A, buf_B, line_num, CAMERA_DATA_MODE_YUV);
}


/**
 * @brief capture an image in JPEG data format
 * @author luheshan 
 * @date 2013-01-07
 * @param[in] dstWidth      the window width
 * @param[in] dstHeight     the window height
 * @param[in] dst_bufA      the ping-pang buffer A to save the image data 
 * @param[in] dst_bufB      the ping-pang buffer B to save the image data  
 * @param[in] buf_size       the ping-pang one buffer size (unit:byte)
 * @param[in/out] JPEGlength  the JPEG image length
 * @param[in] callback_func: the call back function will get the ping-pang buffer status:
 * @                        when the status is CAMERA_LINE_INTR, can get the data from ping-pang buffer;
 * @                        when the status is CAMERA_FRAME_OK, a frame is end, get the remnant data.
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if fail
 */
T_BOOL cam_capture_JPEG(T_U32 dstWidth, T_U32 dstHeight, T_U8 *dst_bufA, T_U8 *dst_bufB, 
                              T_U32 buf_size, T_U32 *JPEGlength, T_fCAMCALLBACK callback_func)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if ((AK_NULL == dst_bufA) || (AK_NULL == dst_bufB) 
        || (0 != ((T_U32)dst_bufA & 0x3)) || (0 != ((T_U32)dst_bufB & 0x3)))
    {
        drv_print("camera chk buf fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    
    if (AK_FALSE == cam_set_window(dstWidth, dstHeight))
    {
        drv_print("JPEG cam_set_window fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }

    m_hal_param.jpeg_len = JPEGlength;
    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_JPEG;
    m_hal_param.DataNotifyCallbackFunc = callback_func;
    
    return camctrl_capture_data(dst_bufA, dst_bufB, buf_size, CAMERA_DATA_MODE_JPEG);
}

#endif
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
                               T_U32 line_cnt, T_fCAMCALLBACK callback_func)
{
    T_U8 *bufA;
    T_U8 *bufB;
    
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    if ((AK_NULL == dst_bufA) || (AK_NULL == dst_bufB) 
        || (0 != ((T_U32)dst_bufA & 0x3)) || (0 != ((T_U32)dst_bufB & 0x3)))
    {
        drv_print("camera chk buf fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (0 != (dstHeight % line_cnt))
    {
        drv_print("line_cnt no suspensory", 0, AK_TRUE);
        return AK_FALSE;
    }
    
#ifndef USE_SPI_CAMERA
    if (!CHECK_SIZE_VALID(dstWidth, dstHeight))
    {
        drv_print("check size fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_to_record_func))
    {
        if (!m_hal_param.pCameraHandler->cam_set_to_record_func(dstWidth, dstHeight))
        {
            drv_print("set sendor size fail\n", 0, AK_FALSE);
            return AK_FALSE;
        }
    }

    m_hal_param.sensor_output_width = dstWidth;
    m_hal_param.sensor_output_height = dstHeight;
    if (AK_FALSE == cam_set_window(dstWidth, dstHeight))
    {
        drv_print("cam_set_window fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }
    
    bufA = (T_U8*)MMU_Vaddr2Paddr((T_U32)dst_bufA);
    bufB = (T_U8*)MMU_Vaddr2Paddr((T_U32)dst_bufB);
    if (!camctrl_capture_data((T_pVOID)bufA, (T_pVOID)bufB, line_cnt, CAMERA_DATA_MODE_Y))
    {
        drv_print("camctrl_capture_data fail\n", 0, AK_FALSE);
        return AK_FALSE;
    }
    
    m_hal_param.cam_data_mode = CAMERA_DATA_MODE_Y;
    m_hal_param.DataNotifyCallbackFunc = callback_func;
    m_hal_param.buf_A = dst_bufA;
    m_hal_param.buf_B = dst_bufB;
    m_hal_param.line_cnt = line_cnt;
#else
    spi_cam_set_info(dstWidth, dstHeight, dst_bufA, dst_bufB, line_cnt, callback_func);
#endif
    return AK_TRUE;
}

/**
 * @brief start camera controller to capture a frame
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL cam_start_capture(T_VOID)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
#ifndef USE_SPI_CAMERA
    m_hal_param.line_index = 0;
    //set interrupt callback
    camctrl_set_interrupt_callback(cam_interrupt_callback);
    camctrl_set_auto_mode(AK_TRUE);
    camctrl_capture_frame();
#else
    if (AK_FALSE == spi_cam_start_capture())
    {
        return AK_FALSE;
    }
#endif
    return AK_TRUE;
}

/**
 * @brief cam_stop_capture
 * @author Lu_Heshan
 * @date 2013-04-02
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi stop capture failed, else spi camera stop capture successfully
 */
T_BOOL cam_stop_capture(T_VOID)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return AK_FALSE;
    }
#ifndef USE_SPI_CAMERA
    camctrl_set_auto_mode(AK_FALSE);
    camctrl_set_interrupt_callback(AK_NULL);
#else
    if (AK_FALSE == spi_cam_stop_capture())
    {
        return AK_FALSE;
    }
#endif
    return AK_TRUE;
}


/**
 * @brief set camera feature 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] feature_type camera feature type,such as night mode,AWB,contrast..etc.
 * @param[in] feature_setting feature setting
 * @return T_VOID
 */
T_VOID cam_set_feature(T_CAMERA_FEATURE feature_type, T_U8 feature_setting)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return;
    }
    
    switch (feature_type)
    {
        case CAM_FEATURE_NIGHT_MODE:
            cam_set_night_mode(feature_setting);
            break;

        case CAM_FEATURE_EXPOSURE:
            cam_set_exposure(feature_setting);
            break;

        case CAM_FEATURE_AWB:
            cam_set_AWB(feature_setting);
            break;

        case CAM_FEATURE_BRIGHTNESS:
            cam_set_brightness(feature_setting);
            break;

        case CAM_FEATURE_CONTRAST:
            cam_set_contrast(feature_setting);
            break;
            
        case CAM_FEATURE_SATURATION:
            cam_set_saturation(feature_setting);
            break;  
            
        case CAM_FEATURE_SHARPNESS:
            cam_set_sharpness(feature_setting);
            break;
            
        case CAM_FEATURE_MIRROR:
            cam_set_mirror(feature_setting);
            break;
            
        case CAM_FEATURE_EFFECT:
            cam_set_effect(feature_setting);
            break;
            
        default:
            drv_print("error camera feature", 0, AK_TRUE);
            break;
    }   
}

/**
 * @brief Set camera frame rate manually
 * @author xia_wenting  
 * @date 2011-01-05
 * @param[in] framerate camera output framerate
 * @return T_U32
 * @retval 0 if error mode 
 * @retval 1 if success
 */
T_U32 cam_set_framerate(float framerate)
{
    if (AK_FALSE == m_hal_param.open_flg)
    {
        drv_print("camera is no open", 0, AK_TRUE);
        return 0;
    }
    
    if ((AK_NULL != m_hal_param.pCameraHandler) 
        && (AK_NULL != m_hal_param.pCameraHandler->cam_set_framerate_func))
    {
        return m_hal_param.pCameraHandler->cam_set_framerate_func(framerate);      
    }
    drv_print("cam_set_framerate is not supported", 0, AK_TRUE);
    return 0;
}
#endif //DRV_SUPPORT_CAMERA

