/**
 * @file camera_gc6113.c
 * @brief camera driver file
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2012-11-12
 * @version 1.0
 * @ref
 */ 
#include "anyka_types.h"
#include "drv_api.h"
#include "arch_gpio.h"
#include "arch_timer.h"
#include "camera_gc6113.h"
#include "hal_camera.h"
#include "drv_camera.h"
#include "i2c.h"
#include "arch_init.h"
#include "drv_cfg.h"

#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)
      
#define DEVICE_NAME             "CAMERA:GC6113"

#define CAM_EN_LEVEL             0     
#define CAM_RESET_LEVEL          0
   
#define CAMERA_SCCB_ADDR        0x80   
#define CAMERA_GC6113_ID        0xb8
 
#define GC6113_CAMERA_MCLK      340
      
static T_CAMERA_TYPE camera_gc6113_type = CAMERA_P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_gc6113_CurMode = CAMERA_MODE_VGA;
 

/*
static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
        tmp |= 0x1 << bit;
    else
        tmp &= ~(0x1 << bit);
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}
*/

static T_BOOL camera_set_param(const T_U8 tabParameter[])
{ 
    int i = 0;
    //T_U8 temp_value; 

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
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1);          
            /*if (!((tabParameter[i] == 0x0e) && (tabParameter[i + 1] & 0x02))
                && !((tabParameter[i] == 0x10) && (tabParameter[i + 1] & 0x26))
                && !((tabParameter[i] == 0x14) && (tabParameter[i + 1] & 0x10))
                && !((tabParameter[i] == 0x17) && (tabParameter[i + 1] & 0x01))
                && !((tabParameter[i] == 0x66) && (tabParameter[i + 1] & 0xe8))
                && !((tabParameter[i] == 0x68) && (tabParameter[i + 1] & 0xa2)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%02x write data is 0x%02x, read data is 0x%02x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

                    return AK_FALSE;
                }
            }*/
        }
        
        i += 2;
    }

    return AK_TRUE;
}

static T_VOID camera_setup(const T_U8 tabParameter[])
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
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)&tabParameter[i + 1], 1);
        }
        i += 2;
    }
}
   
static T_VOID cam_gc6113_open(T_VOID)
{                 
    //gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
    //gpio_set_pin_level(GPIO_CAMERA_AVDD, gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));   

    gpio_set_pin_as_gpio(GPIO_CAMERA_CHIP_ENABLE);
    gpio_set_pin_dir(GPIO_CAMERA_CHIP_ENABLE, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);    
    delay_ms(10);

    gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, CAM_RESET_LEVEL);
    delay_ms(20);
    gpio_set_pin_level(GPIO_CAMERA_RESET, !CAM_RESET_LEVEL);
    delay_ms(20); 
}

static T_BOOL cam_gc6113_close(T_VOID)
{    
    T_U8 Reg0xfc = 0x13;

    sccb_write_data(CAMERA_SCCB_ADDR, 0xfc, &Reg0xfc, 1);

    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
//    gpio_set_pin_level(GPIO_CAMERA_AVDD, !gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);
#if 0
    gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);
#endif    
    return AK_TRUE;
}

static T_U32 cam_gc6113_read_id(T_VOID)
{
    T_U32 id = 0;
    T_U8 Reg0xfe = 0x00;
    T_U8 Reg0xfc = 0x12;
    T_U32 cnt = 0,i;  
    
    //power down disabled by sensor internal register
    sccb_write_data(CAMERA_SCCB_ADDR, 0xfc, &Reg0xfc, 1);
    delay_ms(20);   
    sccb_write_data(CAMERA_SCCB_ADDR, 0xfe, &Reg0xfe, 1);
    
    id = sccb_read_data(CAMERA_SCCB_ADDR, 0x00);
    
    drv_print("cam_gc6113_read_id = ", id, AK_TRUE);
    
    return id;
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting 
 * @date 2011-01-11
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_gc6113_init() 
{
    //drv_print(DEVICE_NAME, 0, AK_TRUE);

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
 * @date 2011-01-11
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_mode(T_CAMERA_MODE mode)
{
    s_gc6113_CurMode = mode;
    switch(mode)
    {
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
        case CAMERA_MODE_QQVGA:
            camera_setup(QQVGA_MODE_TAB);
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

            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        default:
            s_gc6113_CurMode = CAMERA_MODE_VGA;
            //akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
            break;
        }
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_exposure(T_CAMERA_EXPOSURE exposure)
{
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
            //akprintf(C1, M_DRVSYS, "set exposure parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera brightness level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
            //akprintf(C1, M_DRVSYS, "set brightness parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera contrast level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_contrast(T_CAMERA_CONTRAST contrast)
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
            //akprintf(C1, M_DRVSYS, "set contrast parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera saturation level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_saturation(T_CAMERA_SATURATION saturation)
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
            //akprintf(C1, M_DRVSYS, "set saturation parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera sharpness level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
            //akprintf(C1, M_DRVSYS, "set sharpness parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_AWB(T_CAMERA_AWB awb)
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
            //akprintf(C1, M_DRVSYS, "set AWB mode parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting 
 * @date 2011-03-21
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_mirror(T_CAMERA_MIRROR mirror)
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
            //akprintf(C1, M_DRVSYS, "camera gc6113 set mirror parameter error!\n");
            break;    
    }
}


/**
 * @brief Set camera effect mode 
 * @author xia_wenting 
 * @date 2011-03-21
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc6113_set_effect(T_CAMERA_EFFECT effect)
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
            //akprintf(C1, M_DRVSYS, "set camer effect parameter error!\n");
            break;
    }
}

/**
 * @brief set camera window
 * @author xia_wenting  
 * @date 2011-03-22
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if failed
 */
static T_S32 cam_gc6113_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    return 1;
}

static T_VOID cam_gc6113_set_night_mode(T_NIGHT_MODE mode)
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
            //akprintf(C1, M_DRVSYS, "set night mode parameter error!\n");
            break;
    }
}

static T_BOOL cam_gc6113_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{    
    T_CAMERA_MODE Cammode;

    if ((srcHeight <= 160) && (srcWidth <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcHeight <= 176) && (srcWidth <= 144))
    {
        Cammode = CAMERA_MODE_QCIF; 
    }
    else if ((srcHeight <= 320) && (srcWidth <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcHeight <= 352) && (srcWidth <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcHeight <= 640) && (srcWidth <= 480))
    { 
        Cammode = CAMERA_MODE_VGA;
    }
    else
    {
        //akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
        return AK_FALSE;
    }
    
    cam_gc6113_set_mode(Cammode);
    cam_gc6113_set_digital_zoom(srcWidth, srcHeight);    
    delay_ms(200);
    return AK_TRUE;
}

static T_BOOL cam_gc6113_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_gc6113_set_mode(CAMERA_MODE_PREV);    
    cam_gc6113_set_digital_zoom(srcWidth, srcHeight);
    delay_ms(200);
    return AK_TRUE;
}

static T_BOOL cam_gc6113_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{
    T_CAMERA_MODE Cammode;

    if ((srcHeight <= 160) && (srcWidth <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcHeight <= 176) && (srcWidth <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((srcHeight <= 320) && (srcWidth <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcHeight <= 352) && (srcWidth <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcHeight <= 640) && (srcWidth <= 480))
    {
        Cammode = CAMERA_MODE_REC;
    }
    else 
    {
        //akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
        return AK_FALSE;
    }

    cam_gc6113_set_mode(Cammode);
    cam_gc6113_set_digital_zoom(srcWidth, srcHeight);    
    delay_ms(200);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_gc6113_get_type(T_VOID)
{
    return camera_gc6113_type;
} 

static T_CAMERA_FUNCTION_HANDLER gc6113_function_handler = 
{
    GC6113_CAMERA_MCLK,
    cam_gc6113_open,
    cam_gc6113_close,
    cam_gc6113_read_id,
    cam_gc6113_init,
    cam_gc6113_set_mode,
    cam_gc6113_set_exposure,
    cam_gc6113_set_brightness,
    cam_gc6113_set_contrast,
    cam_gc6113_set_saturation,
    cam_gc6113_set_sharpness,
    cam_gc6113_set_AWB,
    cam_gc6113_set_mirror,
    cam_gc6113_set_effect,
    cam_gc6113_set_digital_zoom,
    cam_gc6113_set_night_mode,
    AK_NULL,
    cam_gc6113_set_to_cap,
    cam_gc6113_set_to_prev,
    cam_gc6113_set_to_record,
    cam_gc6113_get_type
};

int camera_gc6113_reg(void)
{
    camera_reg_dev(CAMERA_GC6113_ID, &gc6113_function_handler);
    return 0;
}
#endif


