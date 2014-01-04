/**
 * @file anyka_cpu.h
 * @brief Define the register of ANYKA CHIP
 * Define the register address and bit map for system.
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author zhanggaoxin
 * @date 2012-12-12
 * @version 1.0
 * @note CHIP AK1080L
 * 统一命名风格:寄存器宏名统一以REG_开头，如REG_CLOCK_DIV1
 * 同时寄存器按照模块放置，寄存器的位定义全部放在anyka_bits.h中
 */
#ifndef __ANYKA_CPU_H__
#define __ANYKA_CPU_H__


#include "anyka_bits.h"


/** @defgroup ANYKA_CHIP
 *  @ingroup SPOTLIGHTL PLATFORM
 */
/*@{*/


/** @{@name Base Address Define
 *  The base address of system memory space is define here.
 *  Include memory assignment and module base address define.
 */
/*Memory assignment*/
#define TOPCFG_MODULE_BASE_ADDR         (0x00400000)   // TOP config module

/*L2 totally 160KB, from 0x00000 to 0x27FFF*/
#define L2_START_ADDR                   (0x00800000)   // L2 base address
#define L2_END_ADDR                     (0x00828000)   // L2 end address

/*Module base address of chip config*/
#define GPIO_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x00000000)  // GPIO
#define ADC1_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x00000000)  // ADC1
#define ADCS_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x00090000)  // ADC2&ADC3
#define DACS_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x00080000)  // DACs
#define L2_MODULE_BASE_ADDR             (TOPCFG_MODULE_BASE_ADDR + 0x00010000)  // L2
#define LCD_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x00020000)  // LCD
#define UART1_MODULE_BASE_ADDR          (TOPCFG_MODULE_BASE_ADDR + 0x00036000)  // UART1
#define NFC_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x0004A000)  // NANDFLASH
#define ECC_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x00050000)  // ECC
#define MCI1_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x00060000)  // MMC/SD1
#define MCI2_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x000B0000)  // MMC/SD2
#define USB_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x00070000)  // USB
#define SPI1_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x000A0000)  // SPI1
#define SPI2_MODULE_BASE_ADDR           (TOPCFG_MODULE_BASE_ADDR + 0x000E0000)  // SPI2
#define IMG_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x000C0000)  // Camera
#define PCM_MODULE_BASE_ADDR            (TOPCFG_MODULE_BASE_ADDR + 0x000D0000)  // PCM
#define UART2_MODULE_BASE_ADDR          (TOPCFG_MODULE_BASE_ADDR + 0x000F6000)  // UART2
/** @} */

//IrDA Control reg define
#define REG_IRDA_CFG                    (TOPCFG_MODULE_BASE_ADDR + 0x0200)
#define REG_IRDA_STATUS                 (TOPCFG_MODULE_BASE_ADDR + 0x0208)
#define REG_IRDA_REC                    (TOPCFG_MODULE_BASE_ADDR + 0x020C)



/** @{@name System Control Register define
 */
#define REG_CHIP_ID                     (TOPCFG_MODULE_BASE_ADDR + 0x00)
#define REG_CLOCK_DIV1                  (TOPCFG_MODULE_BASE_ADDR + 0x04)
#define REG_CLOCK_DIV2                  (TOPCFG_MODULE_BASE_ADDR + 0x08)
#define REG_CLOCK_RST_EN                (TOPCFG_MODULE_BASE_ADDR + 0x0C)
#define REG_DACS_HCLK                   (TOPCFG_MODULE_BASE_ADDR + 0x10)
#define REG_IMG_SENSOR_CFG              (TOPCFG_MODULE_BASE_ADDR + 0x14)
#define REG_ADCS_CHANNEL                (TOPCFG_MODULE_BASE_ADDR + 0x28)

#define REG_INT_IRQ                     (TOPCFG_MODULE_BASE_ADDR + 0x34)
#define REG_INT_FIQ                     (TOPCFG_MODULE_BASE_ADDR + 0x38)
#define REG_INT_STA                     (TOPCFG_MODULE_BASE_ADDR + 0xCC)
#define REG_INT_SYSCTL                  (TOPCFG_MODULE_BASE_ADDR + 0x4C)
#define REG_INT_ANALOG                  (TOPCFG_MODULE_BASE_ADDR + 0x98)

#define REG_POWER_CTRL                  (TOPCFG_MODULE_BASE_ADDR + 0x50)
#define REG_BOOTUP_MODE                 (TOPCFG_MODULE_BASE_ADDR + 0x54)
#define REG_MUL_FUNC_CTRL               (TOPCFG_MODULE_BASE_ADDR + 0x58)
#define REG_SHARE_PIN_CTRL              (TOPCFG_MODULE_BASE_ADDR + 0x74)
#define REG_SHARE_PIN_CTRL2             (TOPCFG_MODULE_BASE_ADDR + 0x108)
#define REG_SHARE_PIN_CTRL3             (TOPCFG_MODULE_BASE_ADDR + 0x10C)

#define REG_PMU_CTRL1                   (TOPCFG_MODULE_BASE_ADDR + 0x8C)
#define REG_PMU_CTRL2                   (TOPCFG_MODULE_BASE_ADDR + 0x90)

#define REG_EFUSE_CTRL                  (TOPCFG_MODULE_BASE_ADDR + 0xA4)
#define REG_EFUSE_READ_DATA1            (TOPCFG_MODULE_BASE_ADDR + 0xA8)
#define REG_EFUSE_READ_DATA2            (TOPCFG_MODULE_BASE_ADDR + 0xAC)
/** @} */


/** @{@name TIMER module register define
 */
#define REG_TIMER1_CTRL                 (TOPCFG_MODULE_BASE_ADDR + 0x18)
#define REG_TIMER2_CTRL                 (TOPCFG_MODULE_BASE_ADDR + 0x1C)
#define REG_TIMER2_CNT                  (TOPCFG_MODULE_BASE_ADDR + 0x104)
/** @} */


/** @{@name RTC module register define
 */
#define REG_RTC_CTRL                    (TOPCFG_MODULE_BASE_ADDR + 0x20)
#define REG_RTC_READ_VAL                (TOPCFG_MODULE_BASE_ADDR + 0x24)
/** @} */


/** @{@name PWM module register define
 */
#define REG_PWM0_CTRL                   (TOPCFG_MODULE_BASE_ADDR + 0x2C)
#define REG_PWM1_CTRL                   (TOPCFG_MODULE_BASE_ADDR + 0x30)
/** @} */


/** @{@name NAND module register define
*/
#define REG_NAND_COMMAND                (NFC_MODULE_BASE_ADDR + 0x00)
#define REG_NAND_DATA1                  (NFC_MODULE_BASE_ADDR + 0x50)
#define REG_NAND_DATA2                  (NFC_MODULE_BASE_ADDR + 0x54)
#define REG_NAND_CMD_LEN                (NFC_MODULE_BASE_ADDR + 0x5C)
#define REG_NAND_DATA_LEN               (NFC_MODULE_BASE_ADDR + 0x60)
#define REG_NAND_RAND_ENC               (NFC_MODULE_BASE_ADDR + 0x68)
#define REG_NAND_RAND_DEC               (NFC_MODULE_BASE_ADDR + 0x6C)
#define REG_NAND_ECC_CTRL               (ECC_MODULE_BASE_ADDR + 0x00)
#define REG_NAND_ECC_MODE               (ECC_MODULE_BASE_ADDR + 0x04)
#define REG_NAND_ECC_RESULT             (ECC_MODULE_BASE_ADDR + 0x08)
/** @} */


/** @{@name LCD module register define
 */
#define REG_LCD_CTRL                    (LCD_MODULE_BASE_ADDR + 0x00)
#define REG_LCD_READBACK                (LCD_MODULE_BASE_ADDR + 0x04)
#define REG_LCD_FRAME_ADDR              (LCD_MODULE_BASE_ADDR + 0x08)
#define REG_LCD_BG_COLOR                (LCD_MODULE_BASE_ADDR + 0x0C)
#define REG_LCD_IMG_SIZE                (LCD_MODULE_BASE_ADDR + 0x10)
#define REG_LCD_CFG                     (LCD_MODULE_BASE_ADDR + 0x14)
#define REG_LCD_OPT                     (LCD_MODULE_BASE_ADDR + 0x18)
#define REG_LCD_STA                     (LCD_MODULE_BASE_ADDR + 0x1C)
#define REG_LCD_INT                     (LCD_MODULE_BASE_ADDR + 0x20)
/** @} */


/** @{@name GPIO module register define
 */
#define REG_GPIO_DIR_1                  (GPIO_MODULE_BASE_ADDR + 0x7C)
#define REG_GPIO_DIR_2                  (GPIO_MODULE_BASE_ADDR + 0x84)
#define REG_GPIO_OUT_1                  (GPIO_MODULE_BASE_ADDR + 0x80)
#define REG_GPIO_OUT_2                  (GPIO_MODULE_BASE_ADDR + 0x88)
#define REG_GPIO_IN_1                   (GPIO_MODULE_BASE_ADDR + 0xBC)
#define REG_GPIO_IN_2                   (GPIO_MODULE_BASE_ADDR + 0xC0)
#define REG_GPIO_INT_EN_1               (GPIO_MODULE_BASE_ADDR + 0xE0)
#define REG_GPIO_INT_EN_2               (GPIO_MODULE_BASE_ADDR + 0xE4)
#define REG_GPIO_INT_POL_1              (GPIO_MODULE_BASE_ADDR + 0xF0)
#define REG_GPIO_INT_POL_2              (GPIO_MODULE_BASE_ADDR + 0xF4)
#define REG_GPIO_PULL_UD_1              (GPIO_MODULE_BASE_ADDR + 0x9C)
#define REG_GPIO_PULL_UD_2              (GPIO_MODULE_BASE_ADDR + 0xA0)

#define REG_WGPIO_POL                   (GPIO_MODULE_BASE_ADDR + 0x3C)
#define REG_WGPIO_CLR                   (GPIO_MODULE_BASE_ADDR + 0x40)
#define REG_WGPIO_EN                    (GPIO_MODULE_BASE_ADDR + 0x44)
#define REG_WGPIO_STA                   (GPIO_MODULE_BASE_ADDR + 0x48)
/** @} */


/** @{@name L2 module register define
 */
#define REG_L2_CONFIG                   (L2_MODULE_BASE_ADDR + 0x00)
#define REG_LDMA_SRC                    (L2_MODULE_BASE_ADDR + 0x04)
#define REG_LDMA_DEST                   (L2_MODULE_BASE_ADDR + 0x08)
#define REG_LDMA_COUNT                  (L2_MODULE_BASE_ADDR + 0x0C)
#define REG_BUFFER_STATUS_1             (L2_MODULE_BASE_ADDR + 0x10)
#define REG_BUFFER_STATUS_2             (L2_MODULE_BASE_ADDR + 0x30)
#define REG_PAGE_DIRTY_FLAG_1           (L2_MODULE_BASE_ADDR + 0x14)
#define REG_PAGE_DIRTY_FLAG_2           (L2_MODULE_BASE_ADDR + 0x18)
#define REG_PAGE_CURRENT_COUNT          (L2_MODULE_BASE_ADDR + 0x1C)
#define REG_BUFFER_CONFIG_1             (L2_MODULE_BASE_ADDR + 0x20)
#define REG_BUFFER_CONFIG_2             (L2_MODULE_BASE_ADDR + 0x24)
#define REG_BUFFER_CONFIG_3             (L2_MODULE_BASE_ADDR + 0x28)
#define REG_BUFFER_ENABLE               (L2_MODULE_BASE_ADDR + 0x2C)
#define REG_PAGE_LIFE_COUNT             (L2_MODULE_BASE_ADDR + 0x120)
/** @} */


/** @{@name UART module register offset define
 */
#define oUART_CFG1                      (0x00)
#define oUART_CFG2                      (0x04)
#define oUART_DATA_CFG                  (0x08)
#define oUART_BUF_TRSHLD                (0x0C)


/** @{@name SD/MMC module register offset define
 */
#define oSD_CLK_CTRL                    (0x04)
#define oSD_ARG                         (0x08)
#define oSD_CMD                         (0x0C)
#define oSD_RESP_CMD                    (0x10)
#define oSD_RESP0                       (0x14)
#define oSD_RESP1                       (0x18)
#define oSD_RESP2                       (0x1C)
#define oSD_RESP3                       (0x20)
#define oSD_DATA_TMR                    (0x24)
#define oSD_DATA_LEN                    (0x28)
#define oSD_DATA_CTRL                   (0x2C)
#define oSD_DATA_CNT                    (0x30)
#define oSD_INT_STA                     (0x34)
#define oSD_INT_EN                      (0x38)
#define oSD_DMA_MODE                    (0x3C)
#define oSD_CPU_MODE                    (0x40)
/** @} */


/** @{@name SPI module register offset define
 */
#define oSPI_CTRL                       (0x00)
#define oSPI_STA                        (0x04)
#define oSPI_INT_EN                     (0x08)
#define oSPI_DATA_CNT                   (0x0c)
#define oSPI_TX_EXBUF                   (0x10)
#define oSPI_RX_EXBUF                   (0x14)
#define oSPI_TX_INBUF                   (0x18)
#define oSPI_RX_INBUF                   (0x1c)
#define oSPI_RX_TO                      (0x20)
#define oSPI_TX_DLY                     (0x24)
/** @} */


/** @{@name USB module register define
 */
#define USB_DMA_ADDR_2                  (USB_MODULE_BASE_ADDR + 0x0218)
#define USB_DMA_CNTL_1                  (USB_MODULE_BASE_ADDR + 0x0204)
#define USB_DMA_CNTL_2                  (USB_MODULE_BASE_ADDR + 0x0214)

#define USB_REG_FADDR                   (USB_MODULE_BASE_ADDR + 0x0000)
#define USB_REG_POWER                   (USB_MODULE_BASE_ADDR + 0x0001)
#define USB_REG_INTRTX1                 (USB_MODULE_BASE_ADDR + 0x0002)
#define USB_REG_INTRTX2                 (USB_MODULE_BASE_ADDR + 0x0003)
#define USB_REG_INTRRX1                 (USB_MODULE_BASE_ADDR + 0x0004)
#define USB_REG_INTRRX2                 (USB_MODULE_BASE_ADDR + 0x0005)
#define USB_REG_INTRTX1E                (USB_MODULE_BASE_ADDR + 0x0006)
#define USB_REG_INTRTX2E                (USB_MODULE_BASE_ADDR + 0x0007)
#define USB_REG_INTRRX1E                (USB_MODULE_BASE_ADDR + 0x0008)
#define USB_REG_INTRRX2E                (USB_MODULE_BASE_ADDR + 0x0009)
#define USB_REG_INTRUSB                 (USB_MODULE_BASE_ADDR + 0x000A)
#define USB_REG_INTRUSBE                (USB_MODULE_BASE_ADDR + 0x000B)
#define USB_REG_FRAME1                  (USB_MODULE_BASE_ADDR + 0x000C)
#define USB_REG_FRAME2                  (USB_MODULE_BASE_ADDR + 0x000D)
#define USB_REG_INDEX                   (USB_MODULE_BASE_ADDR + 0x000E)
#define USB_REG_TESEMODE                (USB_MODULE_BASE_ADDR + 0x000F)

#define USB_REG_DEVCTL                  (USB_MODULE_BASE_ADDR + 0x0060)
#define USB_REG_CFG                     (TOPCFG_MODULE_BASE_ADDR + 0x0060)
#define USB_REG_CTRL                    (TOPCFG_MODULE_BASE_ADDR + 0x010C)

#define USB_EP0_TX_COUNT                (USB_MODULE_BASE_ADDR + 0x0330)
#define USB_EP1_TX_COUNT                (USB_MODULE_BASE_ADDR + 0x0334)
#define USB_EP2_TX_COUNT                (USB_MODULE_BASE_ADDR + 0x0338)
#define USB_FORBID_WRITE_REG            (USB_MODULE_BASE_ADDR + 0x033C)
#define USB_START_PRE_READ_REG          (USB_MODULE_BASE_ADDR + 0x0340)
#define USB_BUF_ADDR_CHG_REG            (USB_MODULE_BASE_ADDR + 0x0344)
#define USB_FS_SPEED_REG                (USB_MODULE_BASE_ADDR + 0x0348)

#define USB_REG_TXMAXP0                 (USB_MODULE_BASE_ADDR + 0x0010)
#define USB_REG_TXMAXP1                 (USB_MODULE_BASE_ADDR + 0x0010)
#define USB_REG_CSR0                    (USB_MODULE_BASE_ADDR + 0x0012)
#define USB_REG_TXCSR1                  (USB_MODULE_BASE_ADDR + 0x0012)
#define USB_REG_CSR02                   (USB_MODULE_BASE_ADDR + 0x0013)
#define USB_REG_TXCSR2                  (USB_MODULE_BASE_ADDR + 0x0013)
#define USB_REG_RXMAXP1                 (USB_MODULE_BASE_ADDR + 0x0014)
#define USB_REG_RXMAXP2                 (USB_MODULE_BASE_ADDR + 0x0015)
#define USB_REG_RXCSR1                  (USB_MODULE_BASE_ADDR + 0x0016)
#define USB_REG_RXCSR2                  (USB_MODULE_BASE_ADDR + 0x0017)
#define USB_REG_COUNT0                  (USB_MODULE_BASE_ADDR + 0x0018)
#define USB_REG_RXCOUNT1                (USB_MODULE_BASE_ADDR + 0x0018)
#define USB_REG_RXCOUNT2                (USB_MODULE_BASE_ADDR + 0x0019)

#define USB_REG_TXTYPE                  (USB_MODULE_BASE_ADDR + 0x001A)
#define USB_REG_NAKLIMIT0               (USB_MODULE_BASE_ADDR + 0x001B)
#define USB_REG_RXTYPE                  (USB_MODULE_BASE_ADDR + 0x001C)

#define USB_REG_REQPKTCNT1              (USB_MODULE_BASE_ADDR + 0x0304)
#define USB_REG_REQPKTCNT2              (USB_MODULE_BASE_ADDR + 0x0308)
#define USB_REG_REQPKTCNT3              (USB_MODULE_BASE_ADDR + 0x030C)

#define USB_REG_CFG_INFO                (USB_MODULE_BASE_ADDR + 0x001F)

#define USB_REG_FIFOSIZE                (USB_MODULE_BASE_ADDR + 0x001F)

#define USB_FIFO_EP0                    (USB_MODULE_BASE_ADDR + 0x0020)
#define USB_FIFO_EP1                    (USB_MODULE_BASE_ADDR + 0x0024)
#define USB_FIFO_EP2                    (USB_MODULE_BASE_ADDR + 0x0028)
#define USB_FIFO_EP3                    (USB_MODULE_BASE_ADDR + 0x002C)
/** @} */


/** @{@name ANALOG module register define
 */
#define REG_ANALOG_CTRL1                (ADC1_MODULE_BASE_ADDR + 0x5C)
#define REG_ANALOG_CTRL2                (ADC1_MODULE_BASE_ADDR + 0x64)
#define REG_ANALOG_CTRL3                (ADC1_MODULE_BASE_ADDR + 0x68)
#define REG_ANALOG_CTRL4                (ADC1_MODULE_BASE_ADDR + 0x6C)
#define REG_ANALOG_CTRL5                (ADC1_MODULE_BASE_ADDR + 0x78)
#define REG_ADC1_STA                    (ADC1_MODULE_BASE_ADDR + 0x70)
#define REG_ADC1_CFG                    (ADC1_MODULE_BASE_ADDR + 0x94)

#define REG_ADCS_CTRL                   (ADCS_MODULE_BASE_ADDR + 0x00)
#define REG_ADCS_RX_DATA1               (ADCS_MODULE_BASE_ADDR + 0x04)
#define REG_ADCS_RX_DATA2               (ADCS_MODULE_BASE_ADDR + 0x08)

#define REG_DACS_CFG                    (DACS_MODULE_BASE_ADDR + 0x00)
#define REG_I2S_CFG                     (DACS_MODULE_BASE_ADDR + 0x04)
#define REG_DACS_DATA                   (DACS_MODULE_BASE_ADDR + 0x08)
/** @} */


/** @{@name Camera module register define
 */
#define REG_IMG_CMD                     (IMG_MODULE_BASE_ADDR + 0x00)
#define REG_IMG_SINFO                   (IMG_MODULE_BASE_ADDR + 0x04)
#define REG_IMG_DINFO                   (IMG_MODULE_BASE_ADDR + 0x08)
#define REG_IMG_PIXEL_NUM               (IMG_MODULE_BASE_ADDR + 0x10)
#define REG_IMG_YADDR_A                 (IMG_MODULE_BASE_ADDR + 0x18)
#define REG_IMG_UADDR_A                 (IMG_MODULE_BASE_ADDR + 0x1c)
#define REG_IMG_VADDR_A                 (IMG_MODULE_BASE_ADDR + 0x20)
#define REG_IMG_JPEGADDR_A              (IMG_MODULE_BASE_ADDR + 0x24)
#define REG_IMG_YADDR_B                 (IMG_MODULE_BASE_ADDR + 0x28)
#define REG_IMG_UADDR_B                 (IMG_MODULE_BASE_ADDR + 0x2c)
#define REG_IMG_VADDR_B                 (IMG_MODULE_BASE_ADDR + 0x30)
#define REG_IMG_JPEGADDR_B              (IMG_MODULE_BASE_ADDR + 0x34)
#define REG_IMG_CLIP_OFFSIZE            (IMG_MODULE_BASE_ADDR + 0x38)
#define REG_IMG_CLIP_SIZE               (IMG_MODULE_BASE_ADDR + 0x3c)
#define REG_IMG_CFG                     (IMG_MODULE_BASE_ADDR + 0x40)
#define REG_IMG_CAP_STOP                (IMG_MODULE_BASE_ADDR + 0x4c)
#define REG_IMG_LINE_CFG                (IMG_MODULE_BASE_ADDR + 0x50)
#define REG_IMG_STATUS                  (IMG_MODULE_BASE_ADDR + 0x60)
#define REG_IMG_JPEG_LINE_NUM           (IMG_MODULE_BASE_ADDR + 0x80)
#define REG_IMG_DATA_MODE               (TOPCFG_MODULE_BASE_ADDR + 0xF8)
/** @} */


/** @{@name Register Operation Define
 *  Define the macro for read/write register and memory
 */
/* ------ Macro definition for reading/writing data from/to register ------ */
#define HAL_READ_UINT8( _register_, _value_ )       ((_value_) = *((volatile T_U8 *)(_register_)))
#define HAL_WRITE_UINT8( _register_, _value_ )      (*((volatile T_U8 *)(_register_)) = (_value_))
#define HAL_READ_UINT16( _register_, _value_ )      ((_value_) = *((volatile T_U16 *)(_register_)))
#define HAL_WRITE_UINT16( _register_, _value_ )     (*((volatile T_U16 *)(_register_)) = (_value_))
#define HAL_READ_UINT32( _register_, _value_ )      ((_value_) = *((volatile T_U32 *)(_register_)))
#define HAL_WRITE_UINT32( _register_, _value_ )     (*((volatile T_U32 *)(_register_)) = (_value_))

#define REG8(_register_)    (*(volatile T_U8  *)(_register_))
#define REG16(_register_)   (*(volatile T_U16 *)(_register_))
#define REG32(_register_)   (*(volatile T_U32 *)(_register_))
/** @} */


/*@}*/

#endif  // __ANYKA_REGS_H__

