/** @file
 * @brief Boot up code
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */


#ifndef __BOOT_H__
#define __BOOT_H__




/** @defgroup BootUp  
 *  @ingroup M3PLATFORM
 */
/*@{*/

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
#define ANYKA_CPU_N_Bit         (1UL<<31)
#define ANYKA_CPU_Z_Bit         (1<<30)
#define ANYKA_CPU_C_Bit         (1<<29)
#define ANYKA_CPU_V_Bit         (1<<28)
#define   __ENABLE_L2_INT__
//#define   __ENABLE_LCD_INT__
#define   __ENABLE_UART_INT__
//#define   __ENABLE_NANDFLASH_INT__
//#define   __ENABLE_ECC_INT__
//#define   __ENABLE_SD_MMC_INT__
#define   __ENABLE_USBMCU_INT__
//#define   __ENABLE_USBDMA_INT__
//#define   __ENABLE_DAC_INT__
//#define   __ENABLE_ADC23_INT__
#define   __ENABLE_SYSCTRL_INT__
#define   __ENABLE_CAMERA_INT__
#ifdef CAMERA_SUPPORT
#define   __ENABLE_VBUF1_INT__
#endif
#define   __ENABLE_RAWRGB_INT__

#define PREFECTH_ERR    1
#define DATA_ABORT_ERR  2


#ifdef DEEP_STANDBY
#define SYS_USE_DEEP_STANDBY
#else

#endif

/*@}*/

#endif


