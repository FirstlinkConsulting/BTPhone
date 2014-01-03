/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME  s_cam_RawRGB.c
 * @BRIEF  RAW RGB capture application
 * @Author£ºli_shengkai
 * @Date£º 2011-11
 * @Version£º
**************************************************************************/
#include "akdefine.h"
#include "Gbl_Global.h"
#include "Fwl_camera.h"
#include "Fwl_osMalloc.h"
#include "vme.h"
#include "Eng_Debug.h"
#include "log_Raw_RGB.h"
#include "Vme.h"
#include "Eng_AutoOff.h"
#include "Fwl_Timer.h"
#include "Fwl_FreqMgr.h"
#include "m_event_api.h"
#include "fwl_timer.h"

#ifdef OS_ANYKA

#ifdef CAMERA_SUPPORT

#define  CAP_WIDTH      240
#define  CAP_HEIGHT     240
#define  LINE_NUM       (CAP_HEIGHT/2)
#define  BUF_CNT        2
const T_U16 case1_file[] = {'B',':','/','c','a','s','e','1','.','r','a','w','\0'};

typedef struct 
{
    T_U8 *data_buf[BUF_CNT];
    T_U32 sam_status;
    T_U32 line_index;
    T_U32 buf_index;
}T_S_CAM_PARAM;

#pragma arm section zidata = "_drvbootbss_"
static T_S_CAM_PARAM m_s_cam_param;
#pragma arm section zidata

#pragma arm section code = "_drvbootcode_"
static T_VOID cam_int_cb(T_FWL_CAM_INTR status, 
                                     T_U8 *cur_buf, T_U32 line_index)
{
    m_s_cam_param.sam_status = status;
    m_s_cam_param.line_index = line_index;
    if (FWL_CAM_LINE_OK == status)
    {
        m_s_cam_param.data_buf[m_s_cam_param.buf_index & 0x1] = cur_buf;
        m_s_cam_param.buf_index++;
    }
    else
    {
        m_s_cam_param.buf_index = 0;
    }
}
#pragma arm section code

static T_VOID save_img(const T_U16*path, T_U32 data_size, T_U32 test_times)
{
    T_hFILE fd;
    T_U32 start_time;
    T_U32 test_cnt;
    T_U32 tmp;
    T_U32 line;

    test_cnt = test_times;
    start_time = Fwl_GetTickCountMs();
    tmp = start_time;
    m_s_cam_param.line_index = T_U32_MAX;
    m_s_cam_param.sam_status = T_U32_MAX;
    m_s_cam_param.buf_index = 0;
    line = m_s_cam_param.line_index;
    
    while(test_cnt > 0)
    {
        if (line != m_s_cam_param.line_index)
        {
            line = m_s_cam_param.line_index;
        }
        else
        {
            continue;
        }
        
        if (FWL_CAM_FRAME_OK == m_s_cam_param.sam_status)
        {
            AK_DEBUG_OUTPUT("frame time:%d ms\n", Fwl_GetTickCountMs() - tmp);
            tmp = Fwl_GetTickCountMs();

            if (test_cnt == 1) //save the last frame
            {
                AK_DEBUG_OUTPUT("frame rate: %d fps\n", 
                    (1000 * test_times) / (tmp - start_time));
                
                fd = Fwl_FileOpen(path, _FMODE_CREATE, _FMODE_CREATE);
                if (fd == _FOPEN_FAIL)
                {
                    AK_DEBUG_OUTPUT("create raw file failed\n");
                    return;
                }
                
                for (tmp = 0; tmp < BUF_CNT; tmp++)
                {
                    if (data_size == Fwl_FileWrite(fd, (T_pCVOID)m_s_cam_param.data_buf[tmp], data_size))
                    {
                        AK_DEBUG_OUTPUT("s s\n");
                    }
                    else
                    {
                        AK_DEBUG_OUTPUT("s f\n");
                    }
                }
                if (fd != _FOPEN_FAIL)
                {
                    Fwl_FileClose(fd);
                }
            }
            test_cnt--;
        }
        else if (FWL_CAM_LINE_OK == m_s_cam_param.sam_status)
        {
            
        }
        else if (FWL_CAM_FRAME_ERR == m_s_cam_param.sam_status)
        {
             AK_DEBUG_OUTPUT("cap fail\n");
             break;
        }
    }
}

void initcamera_capture(void)
{
    Fwl_FreqPush(FREQ_APP_MAX); //change freq
}

void exitcamera_capture(void)
{  
    if (!Fwl_CameraFree())
    {
        AK_DEBUG_OUTPUT("cam close fail\n");
    }
    Fwl_FreqPop();
}

void paintcamera_capture(void)
{
    
}

unsigned char handlecamera_capture(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{   
    AK_DEBUG_OUTPUT("test case 1: save the last frame\n");
    if (!Fwl_CameraInit())
    {
        AK_DEBUG_OUTPUT("cam init fail\n");
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
        return 0;
    }
    
    if (!Fwl_CameraSetInfo(CAP_WIDTH, CAP_HEIGHT, LINE_NUM, cam_int_cb))
    {
        AK_DEBUG_OUTPUT("set info fail\n");
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
        return 0;
    }
    
    if (!Fwl_CameraCaptureStart())
    {
        AK_DEBUG_OUTPUT("set info fail\n");
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
        return 0;
    }
    save_img(case1_file, CAP_WIDTH * LINE_NUM, 100);
    if (!Fwl_CameraCaptureStop())
    {
        AK_DEBUG_OUTPUT("cam close fail\n");
        m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
        return 0;
    }

    AK_DEBUG_OUTPUT("test finish\n");

    m_triggerEvent(M_EVT_RETURN_ROOT, pEventParm);
    return 0;
}

#else //CAMERA_SUPPORT
void initcamera_capture(void)
{
}
void exitcamera_capture(void)
{ 
}
void paintcamera_capture(void)
{
}
unsigned char handlecamera_capture(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}
#endif //CAMERA_SUPPORT

#else
void initcamera_capture(void)
{
}
void exitcamera_capture(void)
{ 
}
void paintcamera_capture(void)
{
}
unsigned char handlecamera_capture(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}
#endif


