/*******************************************************************************
 * @file    drv_cfg.h
 * @brief   driver configuration header file
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  ZGX
 * @date    2012-11-6
 * @version 1.0
*******************************************************************************/
#ifndef __DRV_CFG_H__
#define __DRV_CFG_H__


#define DRV_SUPPORT_LCD           SUPPORT_LCD  /* --- Whether support LCD --- */
#if DRV_SUPPORT_LCD > 0
    #define LCD_MAX_SUPPORT       SUPPORT_LCD_NUM
    #define LCD_R61580            SUPPORT_LCD_R61580
#endif


#define DRV_SUPPORT_CAMERA        SUPPORT_CAMERA  /*- Whether support camera -*/
#if DRV_SUPPORT_CAMERA > 0
    #define CAMERA_MAX_SUPPORT    SUPPORT_CAMERA_NUM
    #define CAMERA_GC0308         SUPPORT_CAMERA_GC0308
    #define CAMERA_OV2643         SUPPORT_CAMERA_OV2643
    #define CAMERA_OV9650         SUPPORT_CAMERA_OV9650
    #define CAMERA_OV3640         SUPPORT_CAMERA_OV3640
    #define CAMERA_GC6113         SUPPORT_CAMERA_GC6113
#endif


#define DRV_SUPPORT_RTC           SUPPORT_RTC  /*---- Whether support rtc ----*/
#if DRV_SUPPORT_RTC > 0
    #define RTC_ANYKA             SUPPORT_RTC_ANYKA
    #define RTC_S35390A           SUPPORT_RTC_S35390A
    #define RTC_HT1381            SUPPORT_RTC_HT1381
#endif


#define DRV_SUPPORT_RADIO         SUPPORT_RADIO  /*-- Whether support radio --*/
#if DRV_SUPPORT_RADIO > 0
    #define RADIO_RDA5807P        SUPPORT_RADIO_RDA5807P
    #define RADIO_RDA5876         SUPPORT_RADIO_RDA5876
    #define RADIO_RDA5876P        SUPPORT_RADIO_RDA5876P    
    #define RADIO_TEA5767         SUPPORT_RADIO_TEA5767
#endif

#define DRV_SUPPORT_BLUETOOTH     SUPPORT_BLUETOOTH  /*-- Whether support radio --*/
#if DRV_SUPPORT_BLUETOOTH > 0
    #define BT_RDA5876            SUPPORT_BLUETOOTH_RDA5876
    #define BT_RDA5876p           SUPPORT_BLUETOOTH_RDA5876p
#endif


#define DRV_SUPPORT_I2S           SUPPORT_I2S /*Whether support external codec*/
#if DRV_SUPPORT_I2S > 0
    #define I2S_CS42L52           SUPPORT_I2S_CS42L52
#endif


#define DRV_SUPPORT_KEYPAD        SUPPORT_KEYPAD  /*- Whether support keypad -*/


#define DRV_SUPPORT_EFUSE         SUPPORT_EFUSE  /*-- Whether support efuse --*/


#define DRV_SUPPORT_I2C           SUPPORT_I2C  /*---- Whether support i2c ----*/


#define DRV_SUPPORT_SPI_BOOT      SUPPORT_SPI_BOOT


#define USB_VAR_MALLOC            SUPPORT_USB_MALLOC

#define USB_USE_DMA               SUPPORT_USB_DMA

#define DRV_SUPPORT_UHOST         SUPPORT_UHOST

#define UHOST_USE_INTR            SUPPORT_UHOST_INTR
#define USLAVE_USE_INTR           SUPPORT_USLAVE_INTR

#if SUPPORT_CHIP_TYPE == 2
#define CHIP_SEL_10C              1
#else
#define CHIP_SEL_10C              0
#endif

#define DRV_SIMU_UART             SUPPORT_SIMU_UART


#define HP_MODE_DC                SUPPORT_HP_DC


#define SWITCH_MODE_SLIDE         SUPPORT_SLIDE_SWITCH


#define DETECT_DEV_MAX            SUPPORT_DETECT_DEV_NUM
#define DETECT_MODE_ADC           SUPPORT_DETECT_MODE_ADC


#define DRV_SUPPORT_NAND          SUPPORT_NAND
#if DRV_SUPPORT_NAND > 0
#define NAND_LARGE_PAGE           SUPPORT_NAND_LARGE_PAGE
#define NAND_SMALL_PAGE           SUPPORT_NAND_SMALL_PAGE

#define NAND_SUPPORT_RR           SUPPORT_NAND_RR
#if  NAND_SUPPORT_RR > 0
#define NAND_RR_H27UBG8T2B        SUPPORT_NAND_RR_H27UBG8T2B
#define NAND_RR_H27UBG8T2C        SUPPORT_NAND_RR_H27UBG8T2C
#define NAND_RR_H27UCG8T2M        SUPPORT_NAND_RR_H27UCG8T2M
#define NAND_RR_H27UCG8T2A        SUPPORT_NAND_RR_H27UCG8T2A
#define NAND_RR_H27UCG8T2C        SUPPORT_NAND_RR_H27UCG8T2C
#define NAND_RR_TOSHIBA_24NM      SUPPORT_NAND_RR_TOSHIBA_24NM
#define NAND_RR_TOSHIBA_19NM      SUPPORT_NAND_RR_TOSHIBA_19NM
#define NAND_RR_TOSHIBA_1YNM      SUPPORT_NAND_RR_TOSHIBA_1YNM
#define NAND_RR_SAMSUNG_21NM      SUPPORT_NAND_RR_SAMSUNG_21NM  
#endif

#define ENHANCED_SLC_PROGRAM      SUPPORT_NAND_ENHANCED_SLC
#if  ENHANCED_SLC_PROGRAM > 0
#define NAND_ENHANCED_SLC_H27UBG8T2B        SUPPORT_NAND_ENHANCED_SLC_H27UBG8T2B
#define NAND_ENHANCED_SLC_H27UBG8T2C        SUPPORT_NAND_ENHANCED_SLC_H27UBG8T2C
#define NAND_ENHANCED_SLC_H27UCG8T2M        SUPPORT_NAND_ENHANCED_SLC_H27UCG8T2M
#define NAND_ENHANCED_SLC_H27UCG8T2A        SUPPORT_NAND_ENHANCED_SLC_H27UCG8T2A
#define NAND_ENHANCED_SLC_H27UCG8T2C        SUPPORT_NAND_ENHANCED_SLC_H27UCG8T2C
#define NAND_ENHANCED_SLC_TOSHIBA_24NM      SUPPORT_NAND_ENHANCED_SLC_TOSHIBA_24NM
#define NAND_ENHANCED_SLC_TOSHIBA_19NM      SUPPORT_NAND_ENHANCED_SLC_TOSHIBA_19NM
#define NAND_ENHANCED_SLC_TOSHIBA_1YNM      SUPPORT_NAND_ENHANCED_SLC_TOSHIBA_1YNM
#define NAND_ENHANCED_SLC_MICRON_20NM       SUPPORT_NAND_ENHANCED_SLC_MICRON_20NM
#define NAND_ENHANCED_SLC_SAMSUNG_21NM      SUPPORT_NAND_ENHANCED_SLC_SAMSUNG_21NM
#endif
#endif


#endif


/*---------------------------------end of file--------------------------------*/


