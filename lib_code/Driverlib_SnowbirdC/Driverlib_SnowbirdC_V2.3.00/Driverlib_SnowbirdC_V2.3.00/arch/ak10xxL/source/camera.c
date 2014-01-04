/*******************************************************************************
 * @file camera.c
 * @brief camera function file
 * This file provides camera APIs: open, capture photo
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2010-12-10
 * @version 1.0
 * @note ref AK37xx technical manual.
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "share_pin.h"
#include "clk.h"
#include "camera.h"
#include "interrupt.h"
#include "arch_init.h"
#include "drv_cfg.h"

#if DRV_SUPPORT_CAMERA > 0


#define CIS_CLK_DISABLE         (1<<9)

#pragma arm section zidata = "_drvbootbss_"
static volatile T_U32 m_CameraCaptureCommand;
static T_fCamera_Interrupt_CALLBACK m_CameraInterruptCallback;
#pragma arm section zidata


/**
 * @brief enable camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID camctrl_enable(T_VOID)
{
    sys_module_enable(eVME_CAMERA_CLK, AK_TRUE);
    sys_share_pin_lock(ePIN_AS_CAMERA);

    //reset camera interface
    sys_module_reset(eVME_CAMERA_CLK);

    //output 26M clock here, if not i2c doesn't work for some sensor, such as ov9650 
    camctrl_open(260);
}


/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID camctrl_disable(T_VOID)
{    
    INT_DISABLE(INT_EN_CAMERA);
    
    //disable CIS clock    
    REG32(REG_IMG_SENSOR_CFG) |= CIS_CLK_DISABLE;

    //disable camera controller clock
    sys_module_enable(eVME_CAMERA_CLK, AK_FALSE);

    sys_share_pin_unlock(ePIN_AS_CAMERA);
}


/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk camera mclk, unit: 100KHz
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL camctrl_open(T_U32 mclk)
{    
    T_U32 clk_168;
    T_U32 div;
    T_U32 reg;
    
    clk_168 = clk_get_clk168m();

    div = clk_168 / (2 * mclk * 100000) - 1;
    if(clk_168 % mclk) div += 1;
    if(div >= 8)
        div = 7;
    //set cis clock div
    reg = REG32(REG_IMG_SENSOR_CFG);
    reg &= ~(0x7 << 0);
    reg |= div & 0x7;
    REG32(REG_IMG_SENSOR_CFG) = reg;

    //enable cis clock
    REG32(REG_IMG_SENSOR_CFG) &= ~CIS_CLK_DISABLE;

    //bit[10]:pclk polarity, 0, negative; 1, positive
    //bit[9]:vihref, 0, active low; 1, active hight
    //bit[8]:vivref, 0, active low; 1, active hight
    REG32(REG_IMG_CFG) |= ((1<<10) | (1<<9));
    REG32(REG_IMG_CFG) &= ~(1<<8);

    return AK_TRUE;
}


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
T_BOOL camctrl_clip(T_U32 clip_hoff, T_U32 clip_voff, T_U32 clip_hsize, T_U32 clip_vsize)
{        
    if((clip_hoff % 2) != 0)
    {
        drv_print("camctrl_clip error", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    //IMG_CLIP_OFFSIZE_REG:bit[31:16]:CLIP_VOFF;bit[15:0],CLIP_HOFF;
    //IMG_CLIP_SIZE_REG:bit[31:16]µÄCLIP_VSIZE;bit[15:0]:CLIP_HSIZE
    REG32(REG_IMG_CLIP_SIZE) = (clip_vsize << CAM_CLIP_VSIZE) | clip_hsize;
    REG32(REG_IMG_CLIP_OFFSIZE) = (clip_voff << CAM_CLIP_VOFFSET) | clip_hoff;
   
    return AK_TRUE;
}

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
                                T_CAMERA_DATA_MODE format)
{   
    T_U32 Val;
    T_U32 line_num;

    Val = REG32(REG_IMG_STATUS);    //clear status

    if ((AK_NULL == buf_A) || (AK_NULL == buf_B))
    {
        return AK_FALSE;
    }
    
    if (CAMERA_DATA_MODE_YUV == format) //YUV420
    {
        REG32(REG_IMG_YADDR_A) = ((T_U32)((T_pYUV_DATA_BUF)buf_A)->buf_y) >> 2;
        REG32(REG_IMG_UADDR_A) = ((T_U32)((T_pYUV_DATA_BUF)buf_A)->buf_u) >> 2;
        REG32(REG_IMG_VADDR_A) = ((T_U32)((T_pYUV_DATA_BUF)buf_A)->buf_v) >> 2;
        REG32(REG_IMG_YADDR_B) = ((T_U32)((T_pYUV_DATA_BUF)buf_B)->buf_y) >> 2;
        REG32(REG_IMG_UADDR_B) = ((T_U32)((T_pYUV_DATA_BUF)buf_B)->buf_u) >> 2;
        REG32(REG_IMG_VADDR_B) = ((T_U32)((T_pYUV_DATA_BUF)buf_B)->buf_v) >> 2;
    }
    else
    {
        REG32(REG_IMG_JPEGADDR_A) = ((T_U32)buf_A) >> 2;
        REG32(REG_IMG_JPEGADDR_B) = ((T_U32)buf_B) >> 2;
    }

    if (CAMERA_DATA_MODE_JPEG == format)
    {
        REG32(REG_IMG_CFG) &= ~CAM_CFG_DATA_YUV;
        line_num = (int_cnt >> 2);//buffer size(unit:word)
    }
    else //yuv/raw/y
    {
        REG32(REG_IMG_CFG) |= CAM_CFG_DATA_YUV;
        line_num = int_cnt;
    }

    REG32(REG_IMG_LINE_CFG) = CAM_LINE_INT_EN | line_num;

    //bit[9:8]:dara format: 00:JPEG;01:YUV420;10:ONLY Y;11:RAW RGB.
    m_CameraCaptureCommand &= ~(0x3 << CAM_CMD_FORMAT);
    m_CameraCaptureCommand |= (format << CAM_CMD_FORMAT);
    //REG32(REG_IMG_CMD) = m_CameraCaptureCommand;
    
    return AK_TRUE;
}

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
                        T_U32 dstHeight, T_BOOL bIsCCIR656)
{
    T_U32 reg_val;
    T_BOOL ret;
    
    REG32(REG_IMG_PIXEL_NUM) = srcWidth * srcHeight;
    
    REG32(REG_IMG_SINFO) = srcWidth | (srcHeight << CAM_SINFO_HEIGHT);
    REG32(REG_IMG_DINFO) = dstWidth | (dstHeight << CAM_DINFO_HEIGHT);

    //bit[12]:0, CCIR 601; 1, CCIR 656. 
    reg_val = REG32(REG_IMG_CFG);
    if (AK_FALSE == bIsCCIR656)
    {
         
        reg_val &= ~CAM_CFG_CCIR656;
    }
    else
    {
        reg_val |= CAM_CFG_CCIR656;
    }
    REG32(REG_IMG_CFG) = reg_val;
    
    //REG_IMG_CMD bit[6]:0,disable clipping; 1, enable
    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
    {
        m_CameraCaptureCommand = 0;
        ret = AK_FALSE;
    }
    else
    {
        m_CameraCaptureCommand = CAM_CMD_CLIP_EN;
        ret = AK_TRUE;
    }

    return ret;
}


/**
 * @brief set interrupt callback function
 * @author xia_wenting  
 * @date 2010-12-01
 * @param
 * @return 
 * @retval 
 */
T_VOID camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func)
{
    m_CameraInterruptCallback = callback_func;
    
    if (callback_func)
    {
        FIQ_INT_ENABLE(INT_EN_CAMERA);
    }
    else
    {
        FIQ_INT_DISABLE(INT_EN_CAMERA);
    }
}

#pragma arm section code = "_drvbootcode_"
/**
 * @brief read camera controller's register, and check the frame finished or occur errorred
 * @author xia_wenting   
 * @date 2010-12-06
 * @param
 * @return T_BOOL
 * @retval AK_TRUE the frame finished
 * @retval AK_FALSE the frame not finished or occur errorred
 */
T_BOOL camctrl_check_status(T_VOID)
{
    T_U32 value;

    value = REG32(REG_IMG_STATUS);

    if ((value & 0x01) == 0x01)             //capture end
    {
        if ((value & 0x02) == 0x02)         //capture error
        {
            return AK_FALSE;            
        }
        else
        {
            return AK_TRUE;
        }        
    }
    else                                       //frame has not been finished
    {
        return AK_FALSE;
    }
}


/**
 * @brief read camera controller's register, and check line interruption status
 * @author lu_heshan   
 * @date 2012-12-31
 * @param[in\out]:cur_buf, get the cur_buf,A or B.
 * @return T_BOOL
 * @retval AK_TRUE the line interruption
 * @retval AK_FALSE the the line no interruption
 */
T_BOOL camctrl_check_buf_status(T_CAMERA_CUR_BUF *cur_buf)
{
    T_U32 Value;

    Value = REG32(REG_IMG_LINE_CFG);

    if ((Value & CAM_LINE_INT_STA) == CAM_LINE_INT_STA)
    {
        if (AK_NULL != cur_buf)
        {
            *cur_buf = (Value >> 31) && 0x1;
        }
        return AK_TRUE;
    }

    return AK_FALSE;
}
#pragma arm section code


/**
 * @brief start camera controller to capture a frame
 * @author lu_heshan
 * @date 2013-01-07
 * @return T_VOID
 */
T_VOID camctrl_capture_frame(T_VOID)
{
    // set the frist buffer is A buf.
    //REG32(REG_IMG_LINE_CFG) &= ~CAM_LINE_CUR_BUF;
    REG32(REG_IMG_CMD) = m_CameraCaptureCommand;
}


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
T_VOID camctrl_set_auto_mode(T_BOOL bauto)
{
    T_U32 regavl;
    
    //bit[1]:auto_mode:0:every frame capture needs system soft start
                   //1:can capture without soft start,can be stopped by cam_stop
    //bit[0]:the bit will stop image sensor during data capturing.
    regavl = REG32(REG_IMG_CAP_STOP);
    if (bauto)
    {
        REG32(REG_IMG_CAP_STOP) |= CAM_AUTO_MODE;
    }
    else
    {
        if (CAM_AUTO_MODE == (regavl & CAM_AUTO_MODE))
        {
            REG32(REG_IMG_CAP_STOP) |= CAM_STOP_CAP;
        }
        REG32(REG_IMG_CAP_STOP) &= ~CAM_AUTO_MODE;
    }
}

#pragma arm section code = "_drvbootcode_"
/**
 * @brief to get jpeg data length
 * @author lu_heshan
 * @date 2013-01-07
 * @param[in]:T_VOID
 * @return T_U32
 * @return can get the jpeg length when the capture a frame is end.
 */
T_U32 camctrl_get_jpeg_len(T_VOID)
{
    T_U32 ret;

    ret = REG32(REG_IMG_JPEG_LINE_NUM);

    return (ret & 0xFFFF);
}

T_VOID camctrl_interrupt_handler(T_VOID)
{
    if (m_CameraInterruptCallback != AK_NULL)
    {
        m_CameraInterruptCallback();
    }
    else
    {
        camctrl_check_status();
    }
}
#pragma arm section code

#endif  //(DRV_SUPPORT_CAMERA > 0)

