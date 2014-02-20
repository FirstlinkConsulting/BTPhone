/**
 * @file camera_gc6113.h
 * @brief camera driver file
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2012-12-6
 * @version 1.0
 * @ref
 */

 /*
 要缩短行与行之间的等待时间，可以减小HB，P1：0x01---hb[7:0]，0x0f[3:0]---hb[11:8],但 HB有如下的要求：
 如果SPI速度是ISP速 度的两倍，HB至少设置为240x4，如果SPI速度与ISP速度相等，HB至少要设置为240x8。
 要想缩短行与行之间的等待时间，可以不对SCK（spi）分频，
 对ISP（及MCLK，设置0xfa寄存器）二分频，这样则只 需要大于240x4即可。HB越小帧率也越高

 */
#ifndef __CAMERA_GC6113_H__
#define __CAMERA_GC6113_H__


#include "drv_cfg.h"
#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)


#define DELAY_FLAG        0xfb   // first parameter is 0xfb, then 2nd parameter is delay time count
#define END_FLAG          0xff   // first parameter is 0xff, then parameter table is over 

#define USE_240X240 //if define,use size 240X240,else, 240X320

static const T_U8 INIT_TAB[] = 
{    
    0xfe, 0x80,
    0xfc, 0x12, //clk_en
    0xfa, 0x00, // Cis clock config 1/1; 0x10, Cis clock config 1/2
    
    //////////////////PAGE0 START///////////////////
    /********************************************/
    /******************* analog ********************/
    /********************************************/
    0xfe, 0x00,
#ifdef USE_240X240
    0x09, 0x00, //[8]cis_win_height  248 
    0x0a, 0xf8, //[7:0]cis_win_height
    0x0b, 0x00, //[9:8]cis_win_width 248
    0x0c, 0xf8, //[7:0]cis_win_width
#else
    0x09, 0x01, //[8]cis_win_height  328
    0x0a, 0x48, //[7:0]cis_win_height
    0x0b, 0x00, //[9:8]cis_win_width 248
    0x0c, 0xf8, //[7:0]cis_win_width
#endif
    0x12, 0x2a, //sh_delay
    0x14, 0x10, 
    0x16, 0x06, //[3:0]number of a/d pipe stages
    
    0x1c, 0x49,
    0x1d, 0x95,
    0x20, 0x7f,
    0x21, 0x38,
    0x22, 0x73,
    0x50, 0x14,
    /********************************************/
    /******************* BLK ********************/
    /********************************************/
    0x30, 0xf7,
    0x39, 0x09,  // exp_rate_darkc
    0x3a, 0x18,
    0x3b, 0x20,
    0x3c, 0x22,
    0x3d, 0x22,
    /********************************************/
    /******************* LSC ********************/
    /********************************************/
    0x88, 0x20,
    0x8b, 0x24,
    0x8c, 0x20,
    0x8d, 0x18,

    /********************************************/
    /******************* DNDD ********************/
    /********************************************/
    0x62, 0x02,  // b_base
    0x64, 0x05,

    /********************************************/
    /******************* ASDE********************/
    /********************************************/
    0x69, 0x38,  //[5]auto_disable_dn when b<2, [4] 1->enhance dn when dark, 0->decrease dn when dark,
                      //[3:0]c_slope
    0x6a, 0x88,  // [7:4]b_slope , [3:0]n_slope
    0x6e, 0x50,  // ee1_effect,  ee1_slope
    0x6f, 0x40,  // ee2_effect,  ee2_slope


    /********************************************/
    /******************* EEINTP ********************/
    /********************************************/
    0x72, 0x6c,  


    /********************************************/
    /******************* AWB ********************/
    /********************************************/
    0xfe, 0x01, 
    0x00, 0xf5,     
    0x02, 0x1a, 
    0x03, 0x20, 
    0x04, 0x10, 
    0x05, 0x08,
    0x06, 0x20, 
    0x08, 0x20, 
    0x0a, 0x40, 
    0x11, 0x3f, 
    0x12, 0x72, 
    0x13, 0x11, 
    0x14, 0x42, //awb_r_5_gain
    0x15, 0x43, //awb_b_5_gain
    0x16, 0xc2,     //sinT
    0x17, 0xa6, //cosT

#ifdef USE_240X240
    0xc0, 0x00, //x0
    0xc1, 0x00, //y0
    0xc2, 0x3c, //x1 w = 240 /4
    0xc3, 0x3c, //y1 h = 240 /4
#else    
    0xc0, 0x08, 
    0xc1, 0x06, 
    0xc2, 0x3e, 
    0xc3, 0x4e, 
#endif
    /********************************************/
    /******************* ABS ********************/
    /********************************************/
    0xb0, 0x02,
    0xb1, 0x01,
    0xb2, 0x00,   
    0xb3, 0x10,   //  y strech limit

    /********************************************/
    /******************* YCP ********************/
    /********************************************/
    0xfe, 0x00,
    0xb1, 0x38,
    0xb2, 0x38,

    /*
    0xb3, 0x40,
    0xbe, 0x21,
    0x93, 0x48,
    0x94, 0xf6,
    0x95, 0xf8,
    0x96, 0x00, 
    0x97, 0x50, 
    0x98, 0xfa,
    0x9d, 0x03,
    */
    
    /********************************************/
    /*******************Gamma********************/
    /********************************************/
    0x9F, 0x0E,          
    0xA0, 0x1C,  
    0xA1, 0x34,
    0xA2, 0x48,
    0xA3, 0x5A,
    0xA4, 0x6B,
    0xA5, 0x7B,
    0xA6, 0x95,
    0xA7, 0xAB,
    0xA8, 0xBF,
    0xA9, 0xCE,
    0xAA, 0xD9,
    0xAB, 0xE4,
    0xAC, 0xEC,
    0xAD, 0xF7,
    0xAE, 0xFd,
    0xAF, 0xFF,

    /********************************************/
    /******************* AEC ********************/
    /********************************************/
    0xfe, 0x00,
    0xe5, 0x01,  //AEC enable
    0xfe, 0x01,  
    0x30, 0x0b,
    0x31, 0x10,
    0x32, 0x00,
    0x33, 0x48, // Y target
    0x3b, 0x92,
    0x3c, 0xa5,
    
#ifdef USE_240X240
#if 0//240X240 34M 28--29fps //标准
    0xfe, 0x00,
    0x07, 0x08, //0x07,0x08修改这两个寄存器可以修正边界问题
    0x08, 0x08,
    0x01, 0x97,  //hb[7:0]
    0x02, 0x0C,  //vb[7:0]
    0x0f, 0x07,  //vb[11:8] , hb[11:8] 

    0xfe, 0x01,
    0x42, 0x00,   //step[11:8]
    0x43, 0x14,   //step[7:0]
    0x44, 0x00,   // level_0 
    0x45, 0xA0,  
    0x46, 0x00,   // level_1 
    0x47, 0xA0,
    0x48, 0x00,   // level_2 
    0x49, 0xA0,
    0x4a, 0x00,   // level_3 
    0x4b, 0xA0,   //
#endif
#if 1//240X240 34M 30fps
    0xfe, 0x00,
    0x07, 0x08, //0x07,0x08修改这两个寄存器可以修正边界问题
    0x08, 0x08,
    0x01, 0x80,  //hb[7:0]
    0x02, 0x0C,  //vb[7:0]
    0x0f, 0x07,  //vb[11:8] , hb[11:8] 

    0xfe, 0x01,
    0x42, 0x00,   //step[11:8]
    0x43, 0x12,   //step[7:0]
    0x44, 0x00,   // level_0 
    0x45, 0x90,  
    0x46, 0x00,   // level_1 
    0x47, 0x90,
    0x48, 0x00,   // level_2 
    0x49, 0x90,
    0x4a, 0x00,   // level_3 
    0x4b, 0x90,   //
#endif    

#if 0//240X240 34M 26.6fps //0xfa, 0x11
    0xfe, 0x00,
    0x07, 0x08, //0x07,0x08修改这两个寄存器可以修正边界问题
    0x08, 0x08,
    0x01, 0xC0,  //hb[7:0]
    0x02, 0x0C,  //vb[7:0]
    0x0f, 0x03,  //vb[11:8] , hb[11:8] 

    0xfe, 0x01,
    0x42, 0x00,   //step[11:8]
    0x43, 0x14,   //step[7:0]
    0x44, 0x00,   // level_0 
    0x45, 0xA0,  
    0x46, 0x00,   // level_1 
    0x47, 0xA0,
    0x48, 0x00,   // level_2 
    0x49, 0xA0,
    0x4a, 0x00,   // level_3 
    0x4b, 0xA0,   //
#endif
#else //240x320 48mhz mclk, spi clk 48mhz, 31.2fps
    0xfe, 0x00,
    0x07, 0x08, //0x07,0x08修改这两个寄存器可以修正边界问题
    0x08, 0x08,
    0x01, 0x9E,  //hb[7:0]
    0x02, 0x10,  //vb[7:0]
    0x0f, 0x07,  //vb[11:8] , hb[11:8] 

    0xfe, 0x01,
    0x42, 0x00,   //step[11:8]
    0x43, 0x56,   //step[7:0]
    0x44, 0x01,   // level_0 
    0x45, 0x58,  
    0x46, 0x01,   // level_1 
    0x47, 0x58,
    0x48, 0x01,   // level_2 
    0x49, 0x58,
    0x4a, 0x01,   // level_3 
    0x4b, 0x58,   //
#endif
    0x4c, 0x10,  // exp_level //llhhss 0x10
    0x4d, 0x02,  // min_exp
    0x4e, 0xc0,  //aec post gain limit
    0x4f, 0x40,  //aec pre gain limit
    
    /********************************************/
    /******************* SPI ********************/
    /********************************************/
    0xfe, 0x02,
    0x01, 0x03,
    0x02, 0x80, //0x80->0x02, LSB & Falling edge sample
    0x03, 0x20, //signal wire //0x20 add sync code package //llhhss 

    0x04, 0x00,  // 20  [4] master_outformat
    0x05, 0x00,  
    0x09, 0x00,  
    0x0a, 0x00,  //Data ID, 0x00-YUV422, 0x01-RGB565

#ifdef USE_240X240
    0x0c, 0x00,//width
    0x0b, 0xf0,
    0x0e, 0x00,//height
    0x0d, 0xf0,
#endif
    0x13, 0xf0,

    ///////////////////PAGE2 END///////////////////

    /********************************************/
    /******************* OUT ********************/
    /********************************************/
    0xfe, 0x00,
    0x1f, 0x15,  // bit[7:6] driving SCK current
    0x1d, 0x95,  // bit[3:2] driving SDO_0 current
    0x24, 0x11, //0x18, rawrgb;0x11,only Y
    0x2e, 0xc0,  // 42  close in_buf ;0xc2, //0xc1 is test image llhhss 
    0xf0, 0x19, //0x11, //output_en //0x19,ssn output enable

//subsample 
    0xd4, 0x11,//disable subsample
    
//crop 240X320
    0x46, 0x80, //crop en
    0x47, 0x00, //Out?window?y0[7:0]
    0x48, 0x00, //Out?window?x0[7:0]?
    0x49, 0x01, //Out?window?height[8]
    0x4a, 0x40, //Out?window?height[7:0]
    0x4b, 0x00, //Out?window?width[9:8]
    0x4c, 0xF0, //Out?window?width[7:0]?
    END_FLAG, END_FLAG
};

static const T_U8 VGA_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 CIF_MODE_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U8 QVGA_MODE_TAB[] = 
{
#if 0
    0xfe, 0x00,
    0x09, 0x00, //[8]cis_win_height  248 
    0x0a, 0xf8, //[7:0]cis_win_height
    0x0b, 0x00, //[9:8]cis_win_width 248
    0x0c, 0xf8, //[7:0]cis_win_width

    0xfe, 0x01,
    0xc0, 0x00, //x0
    0xc1, 0x00, //y0
    0xc2, 0x3c, //x1 w = 240 /4
    0xc3, 0x3c, //y1 h = 240 /4

    0xfe, 0x02,
    0x0c, 0x00,//width
    0x0b, 0xf0,
    0x0e, 0x00,//height
    0x0d, 0xf0,
    
    0x13, 0xf0, //fifo width 240
#endif
    END_FLAG, END_FLAG
};

static const T_U8 QCIF_MODE_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U8 QQVGA_MODE_TAB[] = 
{
    //120X160 34M 83fps
    0xfe, 0x00,
    0x07, 0x08, //0x07,0x08修改这两个寄存器可以修正边界问题
    0x08, 0x08, //whith
    0x01, 0xC0,  //hb[7:0]
    0x02, 0x0c,  //vb[7:0]
    0x0f, 0x03,  //vb[11:8] , hb[11:8] 
    
    0xfe, 0x01,
    0x42, 0x00,   //step[11:8]
    0x43, 0x14,   //step[7:0]
    0x44, 0x00,   // level_0 
    0x45, 0xA0,  
    0x46, 0x00,   // level_1 
    0x47, 0xA0,
    0x48, 0x00,   // level_2 
    0x49, 0xA0,
    0x4a, 0x00,   // level_3 
    0x4b, 0xA0,   //*/   

    /*0xd4, 0x22,//1/2subsmaple
       0xd5, 0x07,
       0xd6, 0x00,
       0xd7, 0x00,
       0xd8, 0x00,
       0xd9, 0x00,
       0xda, 0x00,
       0xdb, 0x00,
       0xdc, 0x00,
       0xdd, 0x00,*/

    //crop 120X160
    0x46, 0x80, //crop en
    0x47, 0x00, //Out?window?y0[7:0]
    0x48, 0x00, //Out?window?x0[7:0]?
    0x49, 0x00, //Out?window?height[8]
    0x4a, 0xA0, //Out?window?height[7:0]
    0x4b, 0x00, //Out?window?width[9:8]
    0x4c, 0x78, //Out?window?width[7:0]

    0xfe, 0x00,
    0x09, 0x00, //[8]cis_win_height  248 
    0x0a, 0xA8, //[7:0]cis_win_height
    0x0b, 0x00, //[9:8]cis_win_width 248
    0x0c, 0x80, //[7:0]cis_win_width

    0xfe, 0x01,
    0xc0, 0x00, //x0
    0xc1, 0x00, //y0
    0xc2, 0x1E, //x1 w = 120 /4
    0xc3, 0x28, //y1 h = 160 /4

    0xfe, 0x02,
    0x0c, 0x00,//width
    0x0b, 0x78,
    0x0e, 0x00,//height
    0x0d, 0xA0,
    
    0x13, 0x78, //fifo width 120

    END_FLAG, END_FLAG
};

static const T_U8 PREV_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};


static const T_U8 RECORD_MODE_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

/****************   Camera Exposure Table   ****************/
static const T_U8 EXPOSURE_WHOLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EXPOSURE_CENTER_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EXPOSURE_MIDDLE_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Brightness Table   ****************/
static const T_U8 BRIGHTNESS_0_TAB[] = 
{
    //0xb5, 0xd0,
    //0xd3, 0x40,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_1_TAB[] = 
{
    //0xb5, 0xe0,
    //0xd3, 0x48,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_2_TAB[] = 
{
    //0xb5, 0xf0,
    //0xd3, 0x50,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_3_TAB[] = 
{
    //0xb5, 0x00,
    //0xd3, 0x58,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_4_TAB[] = 
{
    //0xb5, 0x10,
    //0xd3, 0x60,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_5_TAB[] = 
{
    //0xb5, 0x20,
    //0xd3, 0x68,
    END_FLAG, END_FLAG
};

static const T_U8 BRIGHTNESS_6_TAB[] = 
{
    //0xb5, 0x30,
    //0xd3, 0x70,
    END_FLAG, END_FLAG
};

/****************   Camera Contrast Table   ****************/
static const T_U8 CONTRAST_1_TAB[] = 
{
    //0xb3, 0x40,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_2_TAB[] = 
{
    //0xb3, 0x50,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_3_TAB[] = 
{
    //0xb3, 0x60,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_4_TAB[] = 
{
    //0xb3, 0x70,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_5_TAB[] = 
{
    //0xb3, 0x80,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_6_TAB[] = 
{
    //0xb3, 0x90,
    END_FLAG, END_FLAG
};

static const T_U8 CONTRAST_7_TAB[] = 
{
    //0xb3, 0xa0,
    END_FLAG, END_FLAG
};

/****************   Camera Saturation Table   ****************/
static const T_U8 SATURATION_1_TAB[] = 
{
    //0xb1, 0x40,
    //0xb2, 0x40,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_2_TAB[] = 
{
    //0xb1, 0x48,
    //0xb2, 0x48,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_3_TAB[] = 
{
    //0xb1, 0x50,
    //0xb2, 0x50,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_4_TAB[] = 
{
    //0xb1, 0x58,
    //0xb2, 0x58,
    END_FLAG, END_FLAG
};

static const T_U8 SATURATION_5_TAB[] = 
{
    //0xb1, 0x60,
    //0xb2, 0x60,
    END_FLAG, END_FLAG
};

/****************   Camera Sharpness Table   ****************/
static const T_U8 SHARPNESS_0_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_1_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_2_TAB[] = 
{ 
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_3_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_4_TAB[] = 
{  
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_5_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 SHARPNESS_6_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera AWB Table   ****************/
static const T_U8 AWB_AUTO_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_SUNNY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_CLOUDY_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_OFFICE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_HOME_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 AWB_NIGHT_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Effect Table   ****************/
static const T_U8 EFFECT_NORMAL_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_SEPIA_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_ANTIQUE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_BLUISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_GREENISH_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_NEGATIVE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_BW_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 EFFECT_BWN_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera night/day mode   ****************/
static const T_U8 DAY_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 NIGHT_MODE_TAB[] = 
{
    END_FLAG, END_FLAG
};

/****************   Camera Mirror Table   ****************/
static const T_U8 MIRROR_V_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 MIRROR_H_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 MIRROR_NORMAL_TAB[] = 
{
    END_FLAG, END_FLAG
};

static const T_U8 MIRROR_FLIP_TAB[] = 
{
    END_FLAG, END_FLAG
};
#endif
#endif
