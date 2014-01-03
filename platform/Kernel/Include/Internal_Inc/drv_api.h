/**@file drv_api.h
 * @brief driver library interface, define driver APIs.
 * This file provides all the driver APIs needed by upper layer.
 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2010-5-31
 * @version 1.0
 */
#ifndef __DRV_API_H__
#define __DRV_API_H__


/** @defgroup API API group
 *    @ingroup Drv_Lib
 */
/*@{*/


/**
    ������汾����˵����
    1����С���޸�ʱ����fix bug,���ƹ��ܵȣ��ύʱ�޸�С�汾��
    2�����нӿڱ䶯�������¹���ʱ���ύʱ�޸��а汾��
    3�����ܹ��е�����֧���µ�оƬʱ���޸����汾��
*/
#ifndef _DRV_LIB_VER
#define _DRV_LIB_VER    "DrvLib V2.0.01"
#endif

#include "anyka_types.h"

#include "arch_adc.h"
#include "arch_analog.h"
#include "arch_dac.h"
#include "arch_gpio.h"
#include "arch_i2c.h"
#include "arch_init.h"
#include "arch_lcd.h"
#include "arch_mmc_sd.h"
#include "arch_nand.h"
#include "arch_pmu.h"
#include "arch_pwm.h"
#include "arch_spi.h"
#include "arch_sys_ctl.h"
#include "arch_timer.h"
#include "arch_uart.h"

#include "hal_camera.h"
#include "hal_clk.h"
#include "hal_detector.h"
#include "hal_int_msg.h"
#include "hal_keypad.h"
#include "hal_l2.h"
#include "hal_rtc.h"
#include "hal_spiflash.h"
#include "hal_usb_h_disk.h"
#include "hal_usb_s_state.h"
#include "hal_usb_s_disk.h"
#include "hal_usb_s_uvc.h"
#include "hal_usb_s_uac.h"



/*@}*/


#endif   //__DRV_API_H__

