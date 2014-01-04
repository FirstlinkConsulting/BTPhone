/*******************************************************************************
 * @file    boot.h
 * @brief   Boot up code
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date    2006-01-16
 * @version 1.0
*******************************************************************************/
#ifndef __BOOT_H__
#define __BOOT_H__


#include "drv_cfg.h"


/**CPU work mode */
#define ANYKA_CPU_Mode_USR        0x10
#define ANYKA_CPU_Mode_FIQ        0x11
#define ANYKA_CPU_Mode_IRQ        0x12
#define ANYKA_CPU_Mode_SVC        0x13
#define ANYKA_CPU_Mode_ABT        0x17
#define ANYKA_CPU_Mode_UNDEF      0x1B    
#define ANYKA_CPU_Mode_SYS        0x1F        
#define ANYKA_CPU_I_Bit           0x80
#define ANYKA_CPU_F_Bit           0x40
#define ANYKA_CPU_T_Bit           0x20
#define ANYKA_CPU_N_Bit           (1UL<<31)
#define ANYKA_CPU_Z_Bit           (1<<30)
#define ANYKA_CPU_C_Bit           (1<<29)
#define ANYKA_CPU_V_Bit           (1<<28)


#define   __ENABLE_L2_INT__
//#define   __ENABLE_LCD_INT__
#if DRV_SUPPORT_SPI_BOOT > 0
#define   __ENABLE_UART_INT__
#endif
//#define   __ENABLE_NANDFLASH_INT__
//#define   __ENABLE_ECC_INT__
//#define   __ENABLE_SD_MMC_INT__
#define   __ENABLE_USBMCU_INT__
#define   __ENABLE_USBDMA_INT__
#define   __ENABLE_SYSCTRL_INT__
//#define   __ENABLE_SPI1_INT__
#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)
#define   __ENABLE_SPI2_INT__
#endif

#if DRV_SUPPORT_CAMERA > 0
#define   __ENABLE_CAMERA_INT__
#endif


#define PREFECTH_ERR    1
#define DATA_ABORT_ERR  2
#if (CHIP_SEL_10C > 0)
#define UMASS_BOOT_ADDR 0xfcc
#else
#define UMASS_BOOT_ADDR 0xb0
#endif

#endif


