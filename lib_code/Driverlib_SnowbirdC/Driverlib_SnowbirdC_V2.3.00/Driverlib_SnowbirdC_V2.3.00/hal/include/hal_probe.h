 /**
 * @file hal_probe.h
 * @brief device probe head file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author guoshaofeng 
 * @date 2010-12-07
 * @version 1.0
 * @ref
 */
#ifndef __HAL_PROBE_H__
#define __HAL_PROBE_H__

#include "drv_camera.h"
#include "lcd_device.h"
/**
 * @brief camera probe pointer
 * @author xia_wenting
 * @date 2010-12-07
 * @param
 * @return T_CAMERA_FUNCTION_HANDLER camera device pointer
 * @retval
 */
T_CAMERA_FUNCTION_HANDLER *cam_probe(T_VOID);


#endif  //__HAL_PROBE_H__

