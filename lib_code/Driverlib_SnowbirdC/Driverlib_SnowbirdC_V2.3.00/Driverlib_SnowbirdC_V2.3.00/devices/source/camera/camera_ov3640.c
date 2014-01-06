/**
 * @FILENAME: camera_ov3640.c
 * @BRIEF camera driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2007-11-07
 * @VERSION 1.0
 * @REF
 */ 
#include "anyka_types.h"
#include "drv_api.h"
#include "arch_gpio.h"
#include "arch_timer.h"
#include "camera_ov3640.h"
#include "hal_camera.h"
#include "drv_camera.h"
#include "i2c.h"
#include "arch_init.h"
#include "drv_cfg.h"
 

#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_OV3640 > 0)

#define DEVICE_NAME             "CAMERA:OV3640"

#define CAM_EN_LEVEL           0    
#define CAM_RESET_LEVEL        0

#define CAMERA_SCCB_ADDR        0x78
#define CAMERA_OV3640_ID        0x364c
#define OV3640_CAMERA_MCLK      24
static T_CAMERA_TYPE camera_ov3640_type = CAMERA_3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_ov3640_CurMode = CAMERA_MODE_VGA;

static T_VOID camera_setbit(T_U16 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data3(CAMERA_SCCB_ADDR, reg, &tmp, 1);
    if (value == 1)
        tmp |= 0x1<<bit;
    else
        tmp &= ~(0x1<<bit);
    sccb_write_data3(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}

static T_BOOL camera_set_param(const T_U16 tabParameter[])
{    
    T_U32 i = 0, j = 0;
    T_U8 value;
    printf("OV3640 CameraSetParameter\n");
    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i]))
        {
            break;
        }
        for (j = 0; j < 3; j++)
        {
            if (sccb_write_data3(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1)==AK_TRUE)
            {
                if(sccb_read_data3(CAMERA_SCCB_ADDR, tabParameter[i], &value, 1) 
                            != AK_TRUE)
                {
                    printf("###register read back error!\n");
                }
                if (tabParameter[i+1] != value)
                {
                    printf("reg=0x%x,good=0x%x,bad=0x%x\n", tabParameter[i], tabParameter[i+1], value);
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
        if (j >= 3)
        {
            printf("#3 time!! reg=0x%x,good=0x%x,bad=0x%x\n", tabParameter[i], 
            tabParameter[i+1], value);
            //return AK_FALSE;
        }
        i += 2;            
    }

    return AK_TRUE;
}

static T_VOID camera_setup(const T_U16 tabParameter[])
{
    int i = 0;

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) 
        {
            break;
        }
        else if (DELAY_FLAG == tabParameter[i])
        {
            delay_ms(tabParameter[i + 1]);
        }
        else
        {
            sccb_write_data3(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1);
        }
        i += 2;
    }
}

static T_VOID cam_ov3640_open(T_VOID)
{
	gpio_set_pin_as_gpio(GPIO_CAMERA_CHIP_ENABLE);
    gpio_set_pin_dir(GPIO_CAMERA_CHIP_ENABLE, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);
    delay_ms(10);
	gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, CAM_RESET_LEVEL);
    delay_ms(10);
    gpio_set_pin_level(GPIO_CAMERA_RESET, !CAM_RESET_LEVEL);

    delay_ms(20);
}

static T_BOOL cam_ov3640_close(T_VOID)
{    
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

	//IIC需要用回宏
	gpio_set_pin_dir(28, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(28, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(0, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(0, GPIO_LEVEL_LOW);
    
    return AK_TRUE;
}

T_U32 cam_ov3640_read_id(T_VOID)
{
    T_U8 tem_value = 0;
    T_U32 id = 0;
    
    sccb_init(28, 0); 
    
    sccb_read_data3(CAMERA_SCCB_ADDR, 0x300a, &tem_value, 1);
     id = tem_value;    
     sccb_read_data3(CAMERA_SCCB_ADDR, 0x300b, &tem_value, 1);
    id = id << 8;
    id |= tem_value;    
    id &= 0xffff;
    printf("i2c addr 0x%x, cam_ov3640_read_id = %x\r\n", CAMERA_SCCB_ADDR, id);
    return id;         
}

 /**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting 
 * @date 2010-12-01
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
static T_BOOL cam_ov3640_init(T_VOID)
{
    if (!camera_set_param(INIT_TAB))
    {
        return AK_FALSE;
    }
    else
    {        
        night_mode = CAMERA_DAY_MODE;
        return AK_TRUE;
    }        
}

/**
 * @brief Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_mode(T_CAMERA_MODE mode)
{
    s_ov3640_CurMode = mode;
    switch(mode)
    {
        case CAMERA_MODE_QXGA:              
            camera_setup(QXGA_MODE_TAB);
            break;                          
        case CAMERA_MODE_UXGA:
            camera_setup(UXGA_MODE_TAB);
            break;
        case CAMERA_MODE_SXGA:              
            camera_setup(SXGA_MODE_TAB);
            break;
        case CAMERA_MODE_XGA:               
            camera_setup(XGA_MODE_TAB);     
            break;                          
        case CAMERA_MODE_SVGA:
            camera_setup(SVGA_MODE_TAB);
            break;            
        case CAMERA_MODE_VGA:
            camera_setup(VGA_MODE_TAB);
            break;
        case CAMERA_MODE_CIF:
            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:
            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:
            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_PREV:              //preview mode
            camera_setup(PREV_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_REC:              //record mode
            camera_setup(RECORD_MODE_TAB);
            break;
        case CAMERA_MODE_VGA_JPEG:
            camera_setup(JPEG_TAB);
            break;
        case CAMERA_MODE_QXGA_JPEG:
            camera_setup(QXGA_MODE_TAB);
            camera_setup(JPEG_TAB);
            break;
        default:
            s_ov3640_CurMode = CAMERA_MODE_VGA;
          
            break;
    }
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_exposure(T_CAMERA_EXPOSURE exposure)
{
#if 0
	switch(exposure)
    {
        case EXPOSURE_WHOLE:
            camera_setup(EXPOSURE_WHOLE_TAB);
            break;
        case EXPOSURE_CENTER:
            camera_setup(EXPOSURE_CENTER_TAB);
            break;
        case EXPOSURE_MIDDLE:
            camera_setup(EXPOSURE_MIDDLE_TAB);
            break;
        default:
            break;
    }
#endif	
}

/**
 * @brief Set camera brightness level 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    switch(brightness)
    {
        case CAMERA_BRIGHTNESS_0:
            camera_setup(BRIGHTNESS_0_TAB);
            break;
        case CAMERA_BRIGHTNESS_1:
            camera_setup(BRIGHTNESS_1_TAB);
            break;
        case CAMERA_BRIGHTNESS_2:
            camera_setup(BRIGHTNESS_2_TAB);
            break;
        case CAMERA_BRIGHTNESS_3:
            camera_setup(BRIGHTNESS_3_TAB);
            break;
        case CAMERA_BRIGHTNESS_4:
            camera_setup(BRIGHTNESS_4_TAB);
            break;
        case CAMERA_BRIGHTNESS_5:
            camera_setup(BRIGHTNESS_5_TAB);
            break;
        case CAMERA_BRIGHTNESS_6:
            camera_setup(BRIGHTNESS_6_TAB);
            break;
        default:
            break;
    }
}

/**
 * @brief Set camera contrast level 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_contrast(T_CAMERA_CONTRAST contrast)
{
    switch(contrast)
    {
        case CAMERA_CONTRAST_1:
            camera_setup(CONTRAST_1_TAB);
            break;
        case CAMERA_CONTRAST_2:
            camera_setup(CONTRAST_2_TAB);
            break;
        case CAMERA_CONTRAST_3:
            camera_setup(CONTRAST_3_TAB);
            break;
        case CAMERA_CONTRAST_4:
            camera_setup(CONTRAST_4_TAB);
            break;
        case CAMERA_CONTRAST_5:
            camera_setup(CONTRAST_5_TAB);
            break;
        case CAMERA_CONTRAST_6:
            camera_setup(CONTRAST_6_TAB);
            break;
        case CAMERA_CONTRAST_7:
            camera_setup(CONTRAST_7_TAB);
            break;
        default:
            break;
    }
}

/**
 * @brief Set camera saturation level 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_saturation(T_CAMERA_SATURATION saturation)
{
    switch(saturation)
    {
        case CAMERA_SATURATION_1:
            camera_setup(SATURATION_1_TAB);
            break;
        case CAMERA_SATURATION_2:
            camera_setup(SATURATION_2_TAB);
            break;
        case CAMERA_SATURATION_3:
            camera_setup(SATURATION_3_TAB);
            break;
        case CAMERA_SATURATION_4:
            camera_setup(SATURATION_4_TAB);
            break;
        case CAMERA_SATURATION_5:
            camera_setup(SATURATION_5_TAB);
            break;
        default:
            break;
    }
}

/**
 * @brief Set camera sharpness level 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    switch(sharpness)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(SHARPNESS_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(SHARPNESS_1_TAB);
            break;
        case CAMERA_SHARPNESS_2:
            camera_setup(SHARPNESS_2_TAB);
            break;
        case CAMERA_SHARPNESS_3:
            camera_setup(SHARPNESS_3_TAB);
            break;
        case CAMERA_SHARPNESS_4:
            camera_setup(SHARPNESS_4_TAB);
            break;
        case CAMERA_SHARPNESS_5:
            camera_setup(SHARPNESS_5_TAB);
            break;
        case CAMERA_SHARPNESS_6:
            camera_setup(SHARPNESS_6_TAB);
            break;
        default:
            break;
    }
}

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_AWB(T_CAMERA_AWB awb)
{
    switch(awb)
    {
        case AWB_AUTO:
            camera_setup(AWB_AUTO_TAB);
            break;
        case AWB_SUNNY:
            camera_setup(AWB_SUNNY_TAB);
            break;
        case AWB_CLOUDY:
            camera_setup(AWB_CLOUDY_TAB);
            break;
        case AWB_OFFICE:
            camera_setup(AWB_OFFICE_TAB);
            break;
        case AWB_HOME:
            camera_setup(AWB_HOME_TAB);
            break;
        case AWB_NIGHT:
            camera_setup(AWB_NIGHT_TAB);
            break;
        default:
            break;            
    }
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            camera_setup(MIRROR_V_TAB);
            break;    
        case CAMERA_MIRROR_H:
            camera_setup(MIRROR_H_TAB);
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setup(MIRROR_NORMAL_TAB);
            break;    
        case CAMERA_MIRROR_FLIP:
            camera_setup(MIRROR_FLIP_TAB);
            break;                    
        default:
            break;
    }
}

/**
 * @brief Set camera effect mode 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov3640_set_effect(T_CAMERA_EFFECT effect)
{
    switch(effect)
    {
        case CAMERA_EFFECT_NORMAL:
            camera_setup(EFFECT_NORMAL_TAB);
            break;
        case CAMERA_EFFECT_SEPIA:
            camera_setup(EFFECT_SEPIA_TAB);
            break;
        case CAMERA_EFFECT_ANTIQUE:
            camera_setup(EFFECT_ANTIQUE_TAB);
            break;
        case CAMERA_EFFECT_BLUE:
            camera_setup(EFFECT_BLUISH_TAB);
            break;            
        case CAMERA_EFFECT_GREEN:
            camera_setup(EFFECT_GREENISH_TAB);
            break;
        case CAMERA_EFFECT_RED:
            camera_setup(EFFECT_REDDISH_TAB);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            camera_setup(EFFECT_NEGATIVE_TAB);
            break;    
        case CAMERA_EFFECT_BW:
            camera_setup(EFFECT_BW_TAB);
            break;
        case CAMERA_EFFECT_BWN:
            camera_setup(EFFECT_BWN_TAB);
            break;    
        default:
            break;
    }
}

 /**
 * @brief set camera window
 * @author  
 * @date 2010-07-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
static T_S32 cam_ov3640_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    return 1;
}

static T_VOID cam_ov3640_set_night_mode(T_NIGHT_MODE mode)
{
    switch(mode)
    {
        case CAMERA_DAY_MODE:
            camera_setup(DAY_MODE_TAB);
            night_mode = CAMERA_DAY_MODE;
            break;
        case CAMERA_NIGHT_MODE:
            camera_setup(NIGHT_MODE_TAB);
            night_mode = CAMERA_NIGHT_MODE;
            break;
        default:
            break;
    }
}

static T_VOID start_preview(T_CAMERA_MODE Cammode)
{
    //Change to preview mode
    cam_ov3640_set_mode(Cammode);
}

static T_VOID start_capture(T_CAMERA_MODE Cammode)
{
    cam_ov3640_set_mode(Cammode);
}

static T_VOID start_record(T_CAMERA_MODE Cammode)
{
    //Change to record mode
    cam_ov3640_set_mode(Cammode); 
}

static T_BOOL cam_ov3640_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{
    T_CAMERA_MODE Cammode;

    if ((srcWidth <= 160) && (srcHeight <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcWidth <= 176) && (srcHeight <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((srcWidth <= 320) && (srcHeight <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcWidth <= 352) && (srcHeight <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcWidth <= 640) && (srcHeight <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else if ((srcWidth <= 1280) && (srcHeight <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else if ((srcWidth <= 1600) && (srcHeight <= 1200))
    {
        Cammode = CAMERA_MODE_UXGA;
    }
    else if ((srcWidth == 2048) && (srcHeight == 1536))
    {
        Cammode = CAMERA_MODE_QXGA_JPEG;
    }
    else
    {
        return AK_FALSE;
    }
    start_capture(Cammode);    
    cam_ov3640_set_digital_zoom(srcWidth, srcHeight);
    delay_ms(300);
    return AK_TRUE;
}

static T_BOOL cam_ov3640_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{
    start_preview(CAMERA_MODE_PREV);
    cam_ov3640_set_digital_zoom(srcWidth, srcHeight);
    delay_ms(300);
    return AK_TRUE;
}

static T_BOOL cam_ov3640_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{    
    start_record(CAMERA_MODE_REC);
    cam_ov3640_set_digital_zoom(srcWidth, srcHeight);
    delay_ms(300);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov3640_get_type(T_VOID)
{
    return camera_ov3640_type;
}

static T_CAMERA_FUNCTION_HANDLER ov3640_function_handler = 
{
    OV3640_CAMERA_MCLK,
    cam_ov3640_open,
    cam_ov3640_close,
    cam_ov3640_read_id,
    cam_ov3640_init,
    cam_ov3640_set_mode,
    cam_ov3640_set_exposure,
    cam_ov3640_set_brightness,
    cam_ov3640_set_contrast,
    cam_ov3640_set_saturation,
    cam_ov3640_set_sharpness,
    cam_ov3640_set_AWB,
    cam_ov3640_set_mirror,
    cam_ov3640_set_effect,
    cam_ov3640_set_digital_zoom,
    cam_ov3640_set_night_mode,
    AK_NULL,
    cam_ov3640_set_to_cap,
    cam_ov3640_set_to_prev,
    cam_ov3640_set_to_record,
    cam_ov3640_get_type
};

int camera_ov3640_reg(void)
{
    camera_reg_dev(CAMERA_OV3640_ID, &ov3640_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov3640_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

