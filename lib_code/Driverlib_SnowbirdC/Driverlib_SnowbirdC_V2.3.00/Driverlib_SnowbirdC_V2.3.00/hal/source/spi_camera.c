/**
 * @file spi_camera.c
 * @brief spi camera driver source file
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author lu_heshan
 * @date 2013-02-04
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "arch_init.h"
#include "l2.h"
#include "clk.h"
#include "drv_camera.h"
#include "anyka_cpu.h"
#include "spi.h"
#include "drv_cfg.h"
#include "hal_errorstr.h"

#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)

//sensor I2C id
#define CAMERA_SCCB_ADDR        (0x80)

#define  DEVICE_IS_NULL         (0)
#define  DEVICE_IS_INIT         (1 << 0)
#define  DEVICE_IS_SET_INFO     (1 << 1)
#define  DEVICE_IS_FINNISH      (DEVICE_IS_INIT | DEVICE_IS_SET_INFO)
#define  DEVICE_IS_START_SAM    (1 << 2)
#define  DEVICE_IS_STANDBY      (1 << 3)

#pragma arm section code = "_drvbootcode_" 
#define  ADD_SYNC_HEAD   //if need add the sync head code,set the sensor register P2:0x03 bit[5] is 1.
//sensor add anyc flag
#ifdef ADD_SYNC_HEAD
#define  CHECK_SYNC_HEAD(v1, v2, v3) ((0xff == v1) && (0xff == v2) && (0xff == v3))
#define  CHECK_FRAME_START(val)      (0x01 == val)
#define  CHECK_LINE_START(val)       (0x02 == val)
#define  CHECK_LINE_END(val)         (0x40 == val)
#define  CHECK_FRAME_END(val)        (0x00 == val)
#endif
#pragma arm section code

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_CAM;
#pragma arm section rodata

typedef struct
{
    DEVICE_SELECT dev;
    T_U32 buf;
    T_U32 len;
    T_U32 id;
    T_U32 index;
}T_SPI_L2_INFO;

typedef struct 
{
    T_U8 *buf_A;
    T_U8 *buf_B;
    T_U8 *buf_cur;
    T_fCAMCALLBACK sample_cb;
    T_SPI_L2_INFO l2_info;
    T_eSPI_ID spi_id;
    T_U32 width;
    T_U32 hight;
    T_U32 line_index; //current sample line index
    T_U32 line_int_num; //the line number had received the callback will be call
    T_U32 dev_state;
    T_U32 cpu_clk;
    T_U8 asic_index;
    T_BOOL frame_ok;
}T_SPI_CAM_CTL;

#pragma arm section zidata = "_drvbootbss_"
static volatile T_SPI_CAM_CTL spi_cam_ctl;
#pragma arm section zidata

static T_VOID re_sample(T_VOID);
static T_BOOL sample_pixel_valid_bit(T_VOID);


#pragma arm section code = "_drvbootcode_" 

/**
 * @brief spi_int_callback
 * @author Lu_Heshan
 * @date 2013-07-26
 * @param status the spi status
 * @return T_VOID
 * @retval T_VOID
 */
static T_VOID spi_int_callback(T_U32 status)
{
    T_U32 receive_cnt;
    T_CAMERA_INTR_STATUS sample_status;
    
    if (SPI_TRANFINISH == (status & SPI_TRANFINISH))
    {
        spi_receive_disable(spi_cam_ctl.spi_id);
        
        if (AK_TRUE == sample_pixel_valid_bit())
        {
            sample_status = CAMERA_LINE_OK;
        }
        else //sample err reset sample
        {
            sample_status = CAMERA_FRAME_ERR;
#if 0
            //re_sample();
            
            receive_cnt = spi_cam_ctl.width;
#ifdef ADD_SYNC_HEAD
            receive_cnt += 21;//add bytes for sync line code and frame start code (12 + 9)
#endif
                spi_receive_enable(spi_cam_ctl.spi_id, spi_cam_ctl.l2_info.len, 
                                            receive_cnt);
#endif
        }

        //call back
        if ((0 == (spi_cam_ctl.line_index % spi_cam_ctl.line_int_num)) 
            || (CAMERA_FRAME_ERR == sample_status))
        {
            if (AK_NULL != spi_cam_ctl.sample_cb)
            {
                spi_cam_ctl.sample_cb(sample_status, spi_cam_ctl.buf_cur, 
                                            spi_cam_ctl.line_index);
            }
            //为了与并口那边的回调格式统一,所以FRAME_OK单独
            if (spi_cam_ctl.line_index == spi_cam_ctl.hight)
            {
                if (AK_TRUE == spi_cam_ctl.frame_ok)
                {
                    sample_status = CAMERA_FRAME_OK;
                }
                else
                {
                    sample_status = CAMERA_FRAME_ERR;
                }
                
                if (AK_NULL != spi_cam_ctl.sample_cb)
                {
                    spi_cam_ctl.sample_cb(sample_status, AK_NULL, 
                                            spi_cam_ctl.line_index);
                }
            }
            else
            {
                if (spi_cam_ctl.buf_cur != spi_cam_ctl.buf_A)
                {
                    spi_cam_ctl.buf_cur = spi_cam_ctl.buf_A;
                }
                else
                {
                    spi_cam_ctl.buf_cur = spi_cam_ctl.buf_B;
                }
            }
        }
        
        if (CAMERA_LINE_OK != sample_status)
        {
            spi_cam_ctl.line_index = 0;
            spi_cam_ctl.buf_cur = spi_cam_ctl.buf_A;
            spi_cam_ctl.frame_ok = AK_FALSE;
        }
    }
}
#pragma arm section code


static T_BOOL spi_clk_check(T_VOID)
{
    T_U32 asic;
    T_U32 sensor_mclk;
    T_U32 spi_max_clk;
    T_U32 i;
    T_U8 val;

    sensor_mclk = (DEF_PLL_VAL >> 1) / ((REG32(REG_IMG_SENSOR_CFG) & 0x7) + 1);
    spi_max_clk = (DEF_PLL_VAL >> 1) / 3;
    
    for (i = 1; i <= 0xf; i++)
    {
        if ((sensor_mclk / i) <= spi_max_clk)
        {
            break;
        }
    }
    
    if (i <= 0xf)
    {
        //select P2
        val = 0x02;
        sccb_write_data(CAMERA_SCCB_ADDR, 0xfe, &val, 1);

        //bit[3:0] is 1/(n+1) MCLK
        val = sccb_read_data(CAMERA_SCCB_ADDR, 0x04);
        if ((val & 0xf) != (i - 1))
        {
            val &= ~(0xf);
            val |= (i - 1);
            drv_print("spi clk:", sensor_mclk / i, AK_TRUE);
            drv_print("sensor clk:", sensor_mclk, AK_TRUE);
            return sccb_write_data(CAMERA_SCCB_ADDR, 0x04, &val, 1);
        }
        else
        {
            return AK_TRUE;
        }
    }
    else
    {
        return AK_FALSE;
    }
}


/**
 * @brief check_clk
 * @author Lu_Heshan
 * @date 2013-07-26
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  check_clk successfully.
 * @retval AK_FALSE: check_clk failed.
 */
static T_BOOL check_clk(T_BOOL bEnable)
{
    if (AK_TRUE == bEnable)
    {        
        spi_cam_ctl.cpu_clk = clk_get_cpu();
        
        spi_cam_ctl.asic_index = clk_request_min_asic(DEF_PLL_VAL >> 1);
        clk_set_cpu2x(AK_TRUE);

        if (AK_FALSE == spi_clk_check())
        {
            drv_print("spi chk clk fail", 0, AK_TRUE);
            return AK_FALSE;
        }
    }
    else
    {
        clk_cancel_min_asic(spi_cam_ctl.asic_index);

        clk_set_cpu(spi_cam_ctl.cpu_clk);
    }

    return AK_TRUE;
}

/**
 * @brief pause
 * @author Lu_Heshan
 * @date 2013-07-26
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera pause successfully.
 * @retval AK_FALSE: spi camera pause failed.
 */
static T_BOOL pause(T_VOID)
{
    T_U8 ret = 0;
    T_U8 val = 0;
    
    //select P2
    val = 0x02;
    if (!sccb_write_data(CAMERA_SCCB_ADDR, 0xfe, &val, 1))
    {
        ret |= (1 << 0);
    }
    
    val = sccb_read_data(CAMERA_SCCB_ADDR, 0x04);

    val |= (0x3 << 6);
    if (!sccb_write_data(CAMERA_SCCB_ADDR, 0x04, &val, 1))
    {
        ret |= (1 << 1);
    }

    if (0 == ret)
    {
        return AK_TRUE;
    }
    else
    {
        drv_print("pause fail:", ret, AK_TRUE);
        return AK_FALSE;
    }
}

/**
 * @brief resume
 * @author Lu_Heshan
 * @date 2013-07-26
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera resume successfully.
 * @retval AK_FALSE: spi camera resume failed.
 */
static T_BOOL resume(T_VOID)
{
    T_U8 ret = 0;
    T_U8 val = 0;

    //select P2
    val = 0x02;
    if (!sccb_write_data(CAMERA_SCCB_ADDR, 0xfe, &val, 1))
    {
        ret |= (1 << 0);
    }

    //read p2:0x04
    val = sccb_read_data(CAMERA_SCCB_ADDR, 0x04);

    //resume
    val &= ~(0x3 << 6);
    if (!sccb_write_data(CAMERA_SCCB_ADDR, 0x04, &val, 1))
    {
        ret |= (1 << 2);
    }
    
    if (0 == ret)
    {
        delay_us(100);
        return AK_TRUE;
    }
    else
    {
        drv_print("re_pause fail:", ret, AK_TRUE);
        return AK_FALSE;
    }
}

/* 
static T_VOID re_sample(T_VOID)
{
    T_U8 ret = 0;
    T_U8 val = 0;
        val = 0x70; //resample
    if (!sccb_write_data(CAMERA_SCCB_ADDR, 0xfe, &val, 1))
    {
        ret |= 1;
    }
}
*/

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief sample_pixel_valid_bit, to check data
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera check successfully.
 * @retval AK_FALSE: spi camera check failed.
 */
static T_BOOL sample_pixel_valid_bit(T_VOID)
{
    T_U8  buf[64];
    T_U32 receive_cnt;
    T_U32 offset;
    T_U32 tmp;

    offset = 0;
    receive_cnt = spi_cam_ctl.width;
#ifdef ADD_SYNC_HEAD
    if ((spi_cam_ctl.hight - 1) == spi_cam_ctl.line_index)
    {
        receive_cnt += 21; //to receive the frist line
    }
    else if ((spi_cam_ctl.hight - 2) == spi_cam_ctl.line_index)
    {
        receive_cnt += 16;//to receive  last line
    }
    else
    {
        receive_cnt += 12;//add bytes for sync line code
    }
#endif

    memcpy((T_VOID *)((T_U32)buf), (T_VOID *)(spi_cam_ctl.l2_info.buf), 64);

#ifdef ADD_SYNC_HEAD
    //check sync info code
    if (0 == spi_cam_ctl.line_index)
    {
        //frame start
        if (CHECK_SYNC_HEAD(buf[0],buf[1],buf[2]) && CHECK_FRAME_START(buf[3]))
        {
            offset = 9; // skip the 9 bytes sync code
        }
        else
        {
            drv_print(err_str, __LINE__, AK_TRUE);
            return AK_FALSE;
        }
    }

    //line start
    if (CHECK_SYNC_HEAD(buf[offset],buf[offset+1],buf[offset+2])
        && CHECK_LINE_START(buf[offset+3]))
    {
        spi_cam_ctl.line_index++;

        //check line number
        if (spi_cam_ctl.line_index != (buf[offset+4] | (buf[offset+5] << 8)))
        {
            drv_print(err_str, __LINE__, AK_TRUE);
            return AK_FALSE;
        }

        offset += 12;//skip the 12 bytes sync code of line start and line end
    }
    else
    {        
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }
#else
    spi_cam_ctl.line_index++;
#endif

    //检测完后,马上启动接收下一行
    spi_receive_enable(spi_cam_ctl.spi_id, spi_cam_ctl.l2_info.len, 
                   receive_cnt);

    //copy data
    tmp = (spi_cam_ctl.line_index - 1) % spi_cam_ctl.line_int_num;
    memcpy((T_VOID *)((T_U32)spi_cam_ctl.buf_cur + tmp * spi_cam_ctl.width),
        (T_VOID *)(T_U32)(buf + offset), 64 - offset );

    memcpy((T_VOID *)((T_U32)spi_cam_ctl.buf_cur + tmp * spi_cam_ctl.width
                                                    + 64 - offset),
        (T_VOID *)(spi_cam_ctl.l2_info.buf + 64), spi_cam_ctl.width - (64 - offset));

#ifdef ADD_SYNC_HEAD     
    if (spi_cam_ctl.hight == spi_cam_ctl.line_index)
    {
        memcpy((T_VOID *)((T_U32)buf), 
            (T_VOID *)(spi_cam_ctl.l2_info.buf + spi_cam_ctl.width + 12), 4);
        //frame end
        if (!(CHECK_SYNC_HEAD(buf[0], buf[1],buf[2]) && CHECK_FRAME_END(buf[3])))
        {
            drv_print(err_str, __LINE__, AK_TRUE);
            spi_cam_ctl.frame_ok = AK_FALSE;
        }
        else
        {
            spi_cam_ctl.frame_ok = AK_TRUE;
        }
    }
#endif
 
    return AK_TRUE;
}
#pragma arm section code

/**
 * @brief spi camera open, must be call first
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param spi_id[in] spi interface selected
 * @return T_BOOL
 * @retval AK_TRUE:  spi camera init successfully.
 * @retval AK_FALSE: spi camera init failed.
 */
T_BOOL spi_cam_open(T_eSPI_ID spi_id)
{
    T_BOOL ret;
    
    if (DEVICE_IS_NULL != spi_cam_ctl.dev_state)
    {
        drv_print("spi cam cap had init:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_TRUE;
    }

    // spi initial slave mode
    if(!spi_init(spi_id, SPI_MODE0, SPI_SLAVE, 15*1000*1000))
    {
        drv_print("spi cam init fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (SPI_ID0 == spi_id)
    {
        spi_cam_ctl.l2_info.dev = ADDR_SPI1_RX;
    }
    else
    {
        spi_cam_ctl.l2_info.dev = ADDR_SPI2_RX;
    }

    //check L2 buffer size
    ret = get_device_buf_info(spi_cam_ctl.l2_info.dev, 
        (T_U32 *)&(spi_cam_ctl.l2_info.buf), (T_U32 *)&(spi_cam_ctl.l2_info.len), 
        (T_U32 *)&(spi_cam_ctl.l2_info.id));
    if ((AK_FALSE == ret) || (512 != spi_cam_ctl.l2_info.len))
    {
        drv_print("spi cam get/check l2 info fail", 0, AK_TRUE);
        spi_close(spi_id);
        return AK_FALSE;
    }

    pause();
    spi_close_ctl(spi_cam_ctl.spi_id);
    
    spi_cam_ctl.spi_id = spi_id;
    spi_cam_ctl.dev_state = DEVICE_IS_INIT;
    spi_cam_ctl.dev_state &= ~DEVICE_IS_START_SAM; //spi camera has no start to sample.
    spi_cam_ctl.dev_state &= ~DEVICE_IS_STANDBY; //spi camera disanble standby.

    return AK_TRUE;
}


/**
 * @brief spi_cam_set_info,must be call second
 * @author Lu_Heshan
 * @date 2013-4-13
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
                               T_U32 line_int_num, T_fCAMCALLBACK callback_func)
{    
    if (!(((240 == width) && (240 == height)) 
          || ((240 == width) && (320 == height))))
    {
        drv_print("image size no suspensory", 0, AK_TRUE);
        return AK_FALSE;
    }

    if ((AK_NULL == dst_bufA) || (AK_NULL == dst_bufB))
    {
        drv_print("buf is null", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (0 != (height % line_int_num))
    {
        drv_print("line_int_num no suspensory", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (AK_NULL == callback_func)
    {
        drv_print("call back func is null", 0, AK_TRUE);
        return AK_FALSE;
    }
    
    spi_cam_ctl.width = width;
    spi_cam_ctl.hight = height;
    spi_cam_ctl.buf_A = dst_bufA;
    spi_cam_ctl.buf_B = dst_bufB;
    spi_cam_ctl.line_int_num = line_int_num;
    spi_cam_ctl.sample_cb = callback_func;
    
    spi_cam_ctl.dev_state |= DEVICE_IS_SET_INFO;
    
    return AK_TRUE;
}


/**
 * @brief spi_cam_start_capture
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi capture failed, else spi camera capture successfully
 */ 
T_BOOL spi_cam_start_capture(T_VOID)
{   
    T_U32 receive_cnt;
    
    if ((DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & spi_cam_ctl.dev_state))
        || (DEVICE_IS_START_SAM == (DEVICE_IS_START_SAM & spi_cam_ctl.dev_state))
        || (DEVICE_IS_STANDBY == (DEVICE_IS_STANDBY & spi_cam_ctl.dev_state)))
    {
        drv_print("spi cam can't to capture:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_FALSE;
    }

    //open the Infrared light 
    pwm_set_duty_cycle(0, 2000, 0); //TP4
    pwm_set_duty_cycle(1, 2000, 0); //TP3

    //request clock
    check_clk(AK_TRUE);
    
    // must be to do
    store_all_int();

    //must be reset
    spi_reset_ctl(spi_cam_ctl.spi_id);

    if (!resume())
    {
        restore_all_int();
        return AK_FALSE;
    }
    
    l2_set_status(spi_cam_ctl.l2_info.id, 0);

    spi_cam_ctl.line_index = 0;
    spi_cam_ctl.buf_cur = spi_cam_ctl.buf_A;
    spi_cam_ctl.frame_ok = AK_FALSE;
    receive_cnt = spi_cam_ctl.width;
#ifdef ADD_SYNC_HEAD
    receive_cnt += 21;//add bytes for sync line code and frame start code (12 + 9)
#endif

    spi_set_protect(SPI_CAM_SPIIF, SPI_BUS1);
    spi_receive_enable(spi_cam_ctl.spi_id, spi_cam_ctl.l2_info.len, 
                                receive_cnt);    
    
    spi_int_enable(spi_cam_ctl.spi_id, SPI_TRANFINISH_ENA, spi_int_callback);

    restore_all_int();
    
    spi_cam_ctl.dev_state |= DEVICE_IS_START_SAM;
    
    return AK_TRUE;
}

/**
 * @brief spi_cam_stop_capture
 * @author Lu_Heshan
 * @date 2013-04-02
 * @param dstBuf[in]  T_VOID
 * @return T_BOOL
 * @retval FLASE: spi stop capture failed, else spi camera stop capture successfully
 */
T_BOOL spi_cam_stop_capture(T_VOID)
{
    T_U32 uCount;
    T_U8 val;
    
    if ((DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & spi_cam_ctl.dev_state))
        || (DEVICE_IS_START_SAM != (DEVICE_IS_START_SAM & spi_cam_ctl.dev_state))
        || (DEVICE_IS_STANDBY == (DEVICE_IS_STANDBY & spi_cam_ctl.dev_state)))
    {
        drv_print("spi cam can't to stop capture:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_FALSE;
    }

    pause();
    
    spi_receive_disable(spi_cam_ctl.spi_id);
    spi_set_unprotect(SPI_CAM_SPIIF, SPI_BUS1);
    
    spi_int_enable(spi_cam_ctl.spi_id, SPI_TRANFINISH_ENA, AK_NULL);

    //free clock
    check_clk(AK_FALSE);

    //close the Infrared light 
    pwm_set_duty_cycle(0, 2000, 100); //TP4
    pwm_set_duty_cycle(1, 2000, 100); //TP3

    spi_close_ctl(spi_cam_ctl.spi_id);
    
    spi_cam_ctl.dev_state &= ~DEVICE_IS_START_SAM;
    
    return AK_TRUE;
}

/**
 * @brief spi_cam_enable_standby
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param T_VOID
 * @return T_BOOL
 * @retval
 */
T_BOOL spi_cam_enable_standby(T_VOID)
{
    T_U8 val = 0;

    if ((DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & spi_cam_ctl.dev_state))
        || (DEVICE_IS_START_SAM == (DEVICE_IS_START_SAM & spi_cam_ctl.dev_state)))
    {
        drv_print("spi cam can't to standby:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_FALSE;
    }

    //no need set standby again
    if (DEVICE_IS_STANDBY == (DEVICE_IS_STANDBY & spi_cam_ctl.dev_state))
    {
        return AK_TRUE;
    }
    
    val = 0x00; //disable spi out
    sccb_write_data(CAMERA_SCCB_ADDR, 0xf0, &val, 1);

    val = 0x01; //enable standby
    sccb_write_data(CAMERA_SCCB_ADDR, 0xfc, &val, 1);

    spi_cam_ctl.dev_state |= DEVICE_IS_STANDBY;
    
    return AK_TRUE;
}

#if 0

T_BOOL spi_cam_disable_standby(T_VOID)
{
    T_U8 val = 0;

    if (DEVICE_IS_FINNISH != (DEVICE_IS_FINNISH & spi_cam_ctl.dev_state))
    {
        drv_print("spi cam is no init:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_FALSE;
    }

    //no need exit standby again
    if (DEVICE_IS_STANDBY != (DEVICE_IS_STANDBY & spi_cam_ctl.dev_state))
    {
        return AK_TRUE;
    }

    val = 0x12; //exit standby
    sccb_write_data(CAMERA_SCCB_ADDR, 0xfc, &val, 1);
    
    if (AK_FALSE == spi_cam_ctl.cam_handler->cam_init_func())
    {
        drv_print("spi cam reinit fail", 0, AK_TRUE);
        return AK_FALSE;
    }

    spi_cam_ctl.dev_state &= ~DEVICE_IS_STANDBY;

    return AK_TRUE;
}
#endif
/**
 * @brief spi_cam_close,close camera and spi, free buffer.
 * @author Lu_Heshan
 * @date 2013-02-04
 * @param T_VOID
 * @return T_BOOL
 * @retval
 */
T_BOOL spi_cam_close(T_VOID)
{
    if (DEVICE_IS_INIT != (DEVICE_IS_INIT & spi_cam_ctl.dev_state))
    {
        drv_print("spi cam cap no open:", spi_cam_ctl.dev_state, AK_TRUE);
        return AK_FALSE;
    }
    if (DEVICE_IS_START_SAM == (DEVICE_IS_START_SAM & spi_cam_ctl.dev_state))
    {
        if (AK_FALSE == spi_cam_stop_capture())
        {
            return AK_FALSE;
        }
    }

    //sensor can be powerdown
    if (DEVICE_IS_STANDBY != (DEVICE_IS_STANDBY & spi_cam_ctl.dev_state))
    {
        if (AK_FALSE == spi_cam_enable_standby())
        {
            return AK_FALSE;
        }
    }

    spi_close(spi_cam_ctl.spi_id);

    spi_cam_ctl.dev_state = DEVICE_IS_NULL;

    return AK_TRUE;
}
#endif

