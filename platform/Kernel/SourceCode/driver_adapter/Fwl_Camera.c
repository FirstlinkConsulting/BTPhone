 /**************************************************************************
 * Copyrights (C) 2002, ANYKA software Inc
 * All rights reserced.
 *
 * File name: Fwl_Camera.c
 * Function: This file will constraint the access to the bottom layer file
 system, avoid resource competition. Also, this file os for
 porting to different OS
 *
 * Author: Zou Mai
 * Date: 2001-06-14
 * Version: 1.0
***************************************************************************/
#include <stdio.h>
#include "Fwl_Camera.h"
#include "hal_camera.h"
#include "Arch_gpio.h"
#include "gpio_define.h"
#include "fwl_osMalloc.h"

#ifdef CAMERA_SUPPORT

#define  DEVICE_IS_NULL         (0)
#define  DEVICE_IS_INIT         (1 << 0)
#define  DEVICE_IS_SET_INFO     (1 << 1)
#define  DEVICE_IS_FINNISH      (DEVICE_IS_INIT | DEVICE_IS_SET_INFO)
#define  DEVICE_IS_START_SAM    (1 << 2)

typedef struct 
{
    T_U8 *data_buf_A;
    T_U8 *data_buf_B;
    T_U32 dev_status;
}T_FWL_CAM_PARAM;

static T_FWL_CAM_PARAM m_fwl_cam_param = {AK_NULL, AK_NULL, DEVICE_IS_NULL};

/*
 * @brief  the function init camera
 * @author luheshan
 * @date 2013-4-13
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL Fwl_CameraInit(T_VOID)
{
#ifdef OS_ANYKA
    if (DEVICE_IS_NULL != m_fwl_cam_param.dev_status)
    {
        return AK_FALSE;
    }
    /* init camera */
    if (!cam_open(GPIO_CAMERA_CHIP_ENABLE, GPIO_CAMERA_RESET))  
    {
        return AK_FALSE;
    }
    m_fwl_cam_param.dev_status = DEVICE_IS_INIT;
    return AK_TRUE;
#else
    return AK_FALSE;
#endif // OS_ANYKA
}

/*
 * @brief  the function close camera
 * @author luheshan
 * @date 2013-4-13
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera set info successfully.
 * @retval AK_FALSE: spi camera set info failed.
 */
T_BOOL Fwl_CameraFree(T_VOID)
{
#ifdef OS_ANYKA
    T_BOOL ret;

    if (DEVICE_IS_INIT != (m_fwl_cam_param.dev_status & DEVICE_IS_INIT))
    {
        return AK_FALSE;
    }
    
    ret = cam_close();
    
    if (AK_NULL != m_fwl_cam_param.data_buf_A)
    {
        Fwl_DMAFree(m_fwl_cam_param.data_buf_A);
        m_fwl_cam_param.data_buf_A = AK_NULL;
    }
    if (AK_NULL != m_fwl_cam_param.data_buf_B)
    {
        Fwl_DMAFree(m_fwl_cam_param.data_buf_B);
        m_fwl_cam_param.data_buf_B = AK_NULL;
    }

    m_fwl_cam_param.dev_status = DEVICE_IS_NULL;
    
    return ret;
#else
    return AK_FALSE;
#endif
}

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
                                T_U32 line_cnt, T_FWL_CAMCB callback_func)
{
    if (DEVICE_IS_INIT != m_fwl_cam_param.dev_status)
    {
        return AK_FALSE;
    }
    
    if (AK_NULL == callback_func)
    {
        return AK_FALSE;
    }
    
    m_fwl_cam_param.data_buf_A = (T_U8 *)Fwl_DMAMalloc(dstWidth * line_cnt);
    if (AK_NULL == m_fwl_cam_param.data_buf_A)
    {
        return AK_FALSE;
    }
    m_fwl_cam_param.data_buf_B = (T_U8 *)Fwl_DMAMalloc(dstWidth * line_cnt);
    if (AK_NULL == m_fwl_cam_param.data_buf_B)
    {
        Fwl_DMAFree(m_fwl_cam_param.data_buf_A);
        return AK_FALSE;
    }

    if (cam_set_info(dstWidth, dstHeight, m_fwl_cam_param.data_buf_A, 
                          m_fwl_cam_param.data_buf_B, line_cnt, 
                          (T_fCAMCALLBACK)callback_func))
    {
        m_fwl_cam_param.dev_status |= DEVICE_IS_SET_INFO;
        return AK_TRUE;
    }
    else
    {
        Fwl_DMAFree(m_fwl_cam_param.data_buf_A);
        Fwl_DMAFree(m_fwl_cam_param.data_buf_B);
        m_fwl_cam_param.data_buf_A = AK_NULL;
        m_fwl_cam_param.data_buf_B = AK_NULL;
        return AK_FALSE;
    }
}

/**
 * @brief start camera controller to capture
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL Fwl_CameraCaptureStart(T_VOID)
{
    if ((DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & m_fwl_cam_param.dev_status))
        || (DEVICE_IS_START_SAM == (DEVICE_IS_START_SAM & m_fwl_cam_param.dev_status)))
    {
        return AK_FALSE;
    }
        
    if (!cam_start_capture())
    {
        return AK_FALSE;
    }

    m_fwl_cam_param.dev_status |= DEVICE_IS_START_SAM;
    return AK_TRUE;
}

/**
 * @brief stop camera controller to capture
 * @author luheshan 
 * @date 2013-04-13
 * @return T_BOOL
 */
T_BOOL Fwl_CameraCaptureStop(T_VOID)
{
    if ((DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & m_fwl_cam_param.dev_status))
        || (DEVICE_IS_START_SAM != (DEVICE_IS_START_SAM & m_fwl_cam_param.dev_status)))
    {
        return AK_FALSE;
    }
        
    if (!cam_stop_capture())
    {
        return AK_FALSE;
    }

    m_fwl_cam_param.dev_status &= ~DEVICE_IS_START_SAM;
    return AK_TRUE;
}

#endif


