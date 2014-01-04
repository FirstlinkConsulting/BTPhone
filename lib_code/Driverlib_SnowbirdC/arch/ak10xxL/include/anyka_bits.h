/**
 * @file anyka_bits.h
 * @brief Define the bits of ANYKA CHIP REGS
 * Define the bit map for anyka chip registers.
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author zhanggaoxin
 * @date 2012-12-12
 * @version 1.0
 * @note CHIP AK1080L
 * @example:
 * //寄存器名（便于使用某寄存器搜索相关位的定义）
 * #define XXX   （1 << m）//只定义一位的时候 （宏名表示此位为1的意义）
 * #define XXX   （n）     //当使用多位的时候
 */
#ifndef __ANYKA_BITS_H__
#define __ANYKA_BITS_H__


/** @{@name System Control Register Bit map
 *  Define system control register bit map here,
 *  include CLOCK control, INTERRUPT control, POWER control
 */

//REG_INT_IRQ
//REG_INT_FIQ
#define INT_EN_UART2                    (1 << 16)
#define INT_EN_SPI2                     (1 << 15)
#define INT_EN_PCM                      (1 << 14)
#define INT_EN_CAMERA                   (1 << 13)
#define INT_EN_MCI2                     (1 << 12)
#define INT_EN_SPI1                     (1 << 11)
#define INT_EN_SYSCTL                   (1 << 10)
#define INT_EN_ADCS                     (1 << 9)
#define INT_EN_DACS                     (1 << 8)
#define INT_EN_USBDMA                   (1 << 7)
#define INT_EN_USBMCU                   (1 << 6)
#define INT_EN_MCI1                     (1 << 5)
#define INT_EN_ECC                      (1 << 4)
#define INT_EN_NAND                     (1 << 3)
#define INT_EN_UART1                    (1 << 2)
#define INT_EN_LCD                      (1 << 1)
#define INT_EN_L2                       (1 << 0)


//REG_INT_STA
#define INT_STA_UART2                   (1 << 16)
#define INT_STA_SPI2                    (1 << 15)
#define INT_STA_PCM                     (1 << 14)
#define INT_STA_CAMERA                  (1 << 13)
#define INT_STA_MCI2                    (1 << 12)
#define INT_STA_SPI1                    (1 << 11)
#define INT_STA_SYSCTL                  (1 << 10)
#define INT_STA_ADCS                    (1 << 9)
#define INT_STA_DACS                    (1 << 8)
#define INT_STA_USBDMA                  (1 << 7)
#define INT_STA_USBMCU                  (1 << 6)
#define INT_STA_MCI1                    (1 << 5)
#define INT_STA_ECC                     (1 << 4)
#define INT_STA_NAND                    (1 << 3)
#define INT_STA_UART1                   (1 << 2)
#define INT_STA_LCD                     (1 << 1)
#define INT_STA_L2                      (1 << 0)


//REG_INT_SYSCTL
#define INT_STA_IRDA                    (1 << 25)
#define INT_STA_RTC_TIMER               (1 << 24)
#define INT_STA_RTC_ALARM               (1 << 23)
#define INT_STA_RTC_READY               (1 << 22)
#define INT_STA_TIMER2                  (1 << 21)
#define INT_STA_GPIO                    (1 << 20)
#define INT_STA_WGPIO                   (1 << 19)
#define INT_STA_ASIC_ADJ                (1 << 18)
#define INT_STA_TIMER1                  (1 << 17)
#define INT_STA_ADC1_COV                (1 << 16)

#define INT_EN_IRDA                     (1 << 9)
#define INT_EN_RTC_TIMER                (1 << 8)
#define INT_EN_RTC_ALARM                (1 << 7)
#define INT_EN_RTC_READY                (1 << 6)
#define INT_EN_TIMER2                   (1 << 5)
#define INT_EN_GPIO                     (1 << 4)
#define INT_EN_WGPIO                    (1 << 3)
#define INT_EN_ASIC_ADJ                 (1 << 2)
#define INT_EN_TIMER1                   (1 << 1)
#define INT_EN_ADC1_COV                 (1 << 0)


//REG_INT_ANALOG
#define ALG_STA_BGR_OK                  (1 << 24)
#define ALG_STA_USB_IN                  23

#define ALG_STA_NML_TO                  (1 << 22)
#define ALG_STA_TKL_TO                  (1 << 21)
#define ALG_STA_HP                      (1 << 20)
#define ALG_STA_HP_OCP                  (1 << 19)
#define ALG_STA_CHG_OC                  (1 << 18)
#define ALG_STA_CHG                     (1 << 17)
#define ALG_STA_CHG_OT                  (1 << 16)

#define INT_STA_NML_TO                  (1 << 13)
#define INT_STA_TKL_TO                  (1 << 12)
#define INT_STA_HP                      (1 << 11)
#define INT_STA_HP_OCP                  (1 << 10)
#define INT_STA_CHG_OC                  (1 << 9)
#define INT_STA_CHG                     (1 << 8)
#define INT_STA_CHG_OT                  (1 << 7)

#define INT_EN_NML_TO                   (1 << 6)
#define INT_EN_TKL_TO                   (1 << 5)
#define INT_EN_HP                       (1 << 4)
#define INT_EN_HP_OCP                   (1 << 3)
#define INT_EN_CHG_OC                   (1 << 2)
#define INT_EN_CHG                      (1 << 1)
#define INT_EN_CHG_OT                   (1 << 0)


//REG_CLOCK_DIV1
//This register is mainly used to define PLL CLK, CPU CLK and ASIC CLK
#define CLK_ACLK_EN                     (1UL << 31)
#define CLK_ACLK_DIV                    (27)        //[30:27]
#define CLK_CLK168M_EN                  (1 << 26)
#define CLK_MCLK_DIV                    (22)        //[25:22]
#define CLK_MCLK_EN                     (1 << 21)
#define CLK_CLK168M_DIV                 (17)        //[20:17]
#define CLK_CLK168M_BP                  (1 << 16)
#define CLK_CPU2X_EN                    (1 << 15)
#define CLK_ASIC_EN                     (1 << 14)
#define CLK_STANDBY_EN                  (1 << 13)
#define CLK_PLL_EN                      (1 << 12)
#define CLK_ASIC_DIV                    (6)         //[8:6]
#define CLK_PLL_SEL                     (0)         //[5:0]


//REG_CLOCK_DIV2
//This register defines ADC CLK1, ADC CLK2, DAC CLK and I2S MCLK
#define ADC1_DIV_H                      (30)        //[31:30]
#define I2S_MCLK_SEL                    (27)        //[28:27]
#define DACS_GATE_DIS                   (1 << 26)
#define ADCS_GATE_DIS                   (1 << 25)
#define DACS_RESET_DIS                  (1 << 24)
#define ADCS_RESET_DIS                  (1 << 23)
#define ADC1_RESET_DIS                  (1 << 22)
#define DACS_CLK_EN                     (1 << 21)
#define DACS_DIV                        (13)        //[20:13]
#define ADCS_CLK_EN                     (1 << 12)
#define ADCS_DIV                        (4)         //[11:4]
#define ADC1_CLK_EN                     (1 << 3)
#define ADC1_DIV_L                      (0)         //[2:0]


//REG_DACS_HCLK
#define DACS_HCLK_DIS                   (1 << 9)
#define DACS_HCLK_VLD                   (1 << 8)
#define DACS_HCLK_DIV                   (0)         //[7:0]


//REG_IMG_SENSOR_CFG
#define CIS_CLK_DIS                     (1 << 9)
#define CIS_CLK_DIV                     (0)         //[2:0]


//REG_ADCS_CHANNEL
#define ADCS_RIGHT_CH_DIS               (1 << 1)
#define ADCS_LEFT_CH_DIS                (1 << 0)


//REG_POWER_CTRL
#define WK_USB_INT_EN                   (1 << 27)
#define WK_USB_INT_STA                  (1 << 26)
#define WK_USB_INT_CLR                  (1 << 25)
#define WK_ONOFF_INT_EN                 (1 << 24)
#define WK_ONOFF_INT_STA                (1 << 23)
#define WK_ONOFF_INT_CLR                (1 << 22)
#define CTRL_SIG_PUPD_EN                (1 << 21)
#define DATA_HBUS_PUPD_EN               (1 << 20)
#define DATA_LBUS_PUPD_EN               (1 << 19)
#define WK_VOICE_INT_EN                 (1 << 17)
#define WK_VOICE_INT_STA                (1 << 16)
#define WK_VOICE_INT_CLR                (1 << 15)
#define WK_AIN1_INT_EN                  (1 << 14)
#define WK_AIN1_INT_STA                 (1 << 13)
#define WK_AIN1_INT_CLR                 (1 << 12)
#define WK_AIN0_INT_EN                  (1 << 11)
#define WK_AIN0_INT_STA                 (1 << 10)
#define WK_AIN0_INT_CLR                 (1 << 9)
#define WK_RTC_TMR_INT_EN               (1 << 5)
#define WK_RTC_ALM_INT_EN               (1 << 4)
#define BUCK12_SWITCH                   (1 << 3)
#define USB_DP_PU_EN                    (1 << 2)
#define USB_LINE_STATE                  (0)         //[1:0]


//REG_MUL_FUNC_CTRL
#define FADE_OUT_FINISH                 (1 << 27)
#define RECEIVER_SLAVE_MODE             (1 << 26)
#define TRANSMITTER_SLAVE_MODE          (1 << 25)
#define I2S_INTERNAL_MODE               (1 << 24)
#define FADED_FACTOR                    (19)        //[23:19]
#define FADE_OUT_EN                     (1 << 18)
#define PHY_SUS                         (1 << 2)
#define PHY_LDO                         (1 << 1)
#define PHY_RST                         (1 << 0)


//REG_PMU_CTRL1
#define PMU_LDO12_SW                    (1UL << 31)
#define PMU_LPVREF_BUF_DIS              (1 << 30)
#define PMU_VREF_SEL                    (1 << 29)
#define PMU_PD_CHG_OCP                  (1 << 28)
#define PMU_CHG_SD_SEL                  (26)        //[27:26]
#define PMU_TKL_TIME_SEL                (1 << 25)
#define PMU_TKL_VOLT_SEL                (23)        //[24:23]
#define PMU_NML_TIME_SEL                (21)        //[22:21]
#define PMU_CHG_CURR_SEL                (18)        //[20:18]
#define PMU_PD_CHG                      (1 << 17)
#define PMU_LDO12_EN                    (1 << 16)
#define PMU_LDO12_SEL_EX                (1 << 15)
#define PMU_LDO12_SEL                   (12)        //[14:12]
#define PMU_PWD_RTC_RST                 (1 << 11)
#define PMU_PWD_RTC_SET                 (1 << 10)
#define PMU_ON_TRIG_SEL                 (1 << 9)
#define PMU_LDO33_SEL                   (6)         //[8:6]
#define PMU_PD_PWR_VDD                  (1 << 5)
#define PMU_PD_PWR_VCC                  (1 << 4)
#define PMU_PD_BUCK12                   (1 << 3)
#define PMU_HW_PD_TIME_SEL              (1 << 2)
#define PMU_HW_PD_GATE_DIS              (1 << 1)
#define PMU_SW_PD_EN                    (1 << 0)


//REG_PMU_CTRL2
#define PMU_PD_NMOS                     (1 << 19)
#define PMU_BUCK12_SEL                  (16)        //[18:16]
#define PMU_BOOST33_2X                  (1 << 6)
#define PMU_BOOST33_SEL                 (0)         //[2:0]
/** @} */


//REG_EFUSE_CTRL
#define EFUSE_RDY_CFG                   (1 << 27)
#define EFUSE_CELL_RST                  (19)        //[24:19]
#define EFUSE_CELL_RD_CS                (12)        //[17:12]
#define EFUSE_MODE_SEL                  (1 << 10)
#define EFUSE_CELL_EN                   (3)         //[8:3]
#define EFUSE_CELL_BIT                  (0)         //[2:0]


//REG_EFUSE_READ_DATA1
#define EFUSE_CELL4_DATA                (16)        //[31:24]
#define EFUSE_CELL3_DATA                (16)        //[23:16]
#define EFUSE_CELL2_DATA                (8)         //[15:8]
#define EFUSE_CELL1_DATA                (0)         //[7:0]


//REG_EFUSE_READ_DATA2
#define EFUSE_READ_ACK                  (1 << 26)
#define EFUSE_ANYKA_LOCK                (1 << 25)
#define EFUSE_GBL_LOCK                  (1 << 24)
#define EFUSE_CELL7_DATA                (16)        //[23:16]
#define EFUSE_CELL6_DATA                (8)         //[15:8]
#define EFUSE_CELL5_DATA                (0)         //[7:0]



/*
================================================================================
================================================================================
*/
/** @{@name Analog module Register Bit map
 *  Define analog module register bit map here,
 *  include ADC1, ADC2&ADC3, DAC, I2S
 */

//REG_ANALOG_CTRL1
#define DISCHG_HP                       (29)        //[31:29]
#define PD_HP_CTRL                      (25)        //[28:25]
#define HP_GAIN                         (20)        //[24:20]
#define PD_HPMID                        (1 << 19)
#define PRE_EN2                         (1 << 18)
#define PD_PMOS                         (1 << 17)
#define PD_NMOS                         (1 << 16)
#define HP_IN                           (13)        //[15:13]
#define DACS_RST_M_V                    (1 << 12)
#define PD_OP                           (1 << 11)
#define PD_CK                           (1 << 10)
#define PTM_DCHG                        (5)         //[9:5]
#define PL_VCM3                         (1 << 4)
#define PD_VCM3                         (1 << 3)
#define PL_VCM2                         (1 << 2)
#define PD_VCM2                         (1 << 1)
#define PD_BIAS                         (1 << 0)


//REG_ANALOG_CTRL2
#define DACS_OSR                        (14)        //[16:14]
#define DACS_EN                         (1 << 13)
#define MODE_48K                        (1 << 12)
#define ADC1_COV_EN                     (1 << 10)


//REG_ANALOG_CTRL3
#define ADCS_IN_SEL                     (28)        //[30:28]
#define LIMIT_R_EN                      (1 << 27)
#define LIMIT_L_EN                      (1 << 26)
#define PD_R_S2D                        (1 << 25)
#define PD_L_S2D                        (1 << 24)
#define PD_R_SDM                        (1 << 23)
#define PD_L_SDM                        (1 << 22)
#define PTM_CHG                         (17)        //[21:17]
#define LINE_GAIN                       (13)        //[16:13]
#define MIC_GAIN                        (10)        //[12:10]
#define MIC_GAIN_2X                     (1 << 9)
#define CONNECT_VCM3                    (1 << 8)
#define CONNECT_AVCC                    (1 << 7)
#define VCM3_SEL_VCM2                   (1 << 6)
#define PD_R_MICP                       (1 << 4)
#define PD_L_MICP                       (1 << 2)
#define PD_R_LINEIN                     (1 << 1)
#define PD_L_LINEIN                     (1 << 0)


//REG_ANALOG_CTRL4
#define VW_FREQ_SEL                     (30)        //[31:30]
#define VW_REF_SEL                      (27)        //[29:27]
#define VW_DTIME_SEL                    (25)        //[26:25]
#define PD_VW                           (1 << 24)
#define DISCHG_VP                       (1 << 23)
#define ANTI_POP_EN                     (1 << 14)
#define VCM2_BIAS_NOT_DB                (1 << 13)
#define MIC_VCM2_CNCT                   (1 << 12)
#define HP_OCP_DIS                      (1 << 11)
#define HP_PLUGIN_DET_EN                (1 << 10)


//REG_ADC1_STA
#define ADC1_COV_STATE                  (1 << 30)
#define AIN1_VALUE                      (20)        //[29:10]
#define BAT_VALUE                       (10)        //[19:10]
#define AIN0_VALUE                      (0)         //[9:0]


//REG_ADC1_CFG
#define ADC1_RESET                      (1UL << 31)
#define USB_REF_SEL                     (28)        //[30:28]
#define USB_REF_DIS                     (1 << 27)
#define USB_TRIM_EN                     (1 << 26)
#define VREF_PIN_SEL                    (1 << 18)
#define VREF1V5_BGR                     (1 << 17)
#define ADC1_VREF_BGR                   (1 << 8)
#define AIN1_CHANNEL                    (1 << 7)
#define BAT_CHANNEL                     (1 << 6)
#define AIN0_CHANNEL                    (1 << 5)
#define AIN1_WAKEUP_EN                  (1 << 4)
#define AIN0_WAKEUP_EN                  (1 << 3)
#define BAT_DIV_2                       (1 << 2)
#define BAT_DIV_EN                      (1 << 1)
#define ADC1_PD                         (1 << 0)


//REG_ADCS_CTRL
#define ADCS_CH_LEFT                    (1 << 15)
#define ADCS_STEREO_REC                 (1 << 14)
#define I2S_RECEIVE_EN                  (1 << 4)
#define ADCS_LEFT_POL_HIGH              (1 << 3)
#define CPU_RD_INT_EN                   (1 << 2)
#define ADCS_L2_MODE                    (1 << 1)
#define ADCS_I_EN                       (1 << 0)


//REG_DACS_CFG
#define DACS_DATA_FORMAT                (1 << 4)
#define CPU_WR_INT_EN                   (1 << 3)
#define DACS_MUTE_EN                    (1 << 2)
#define DACS_L2_MODE                    (1 << 1)
#define DACS_I_EN                       (1 << 0)


//REG_I2S_CFG
#define DACS_I2S_LRCK_H                 (1 << 6)
#define DACS_LEFT_POL_HIGH              (1 << 5)
#define DACS_WORD_LEN                   (0)         //[4:0]
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of L2 module register
 */

//REG_L2_CONFIG
#define FIX_MADDR_H                     (20)        //[25:20]
#define CURRENT_CNT_INT_EN              (1 << 17)
#define LDMA_VLD_INT_EN                 (1 << 16)
#define CPU_BUF_NUM                     (12)        //[15:12]
#define CPU_SEL_EN                      (1 << 11)
#define CPU_BUF_SEL                     (7)         //[10:7]
#define AHB_FLAG_EN                     (1 << 5)
#define MASS_MEM_SEL                    (3)         //[4:3]


//REG_LDMA_SRC
#define WRAP_SIZE_SRC                   (20)        //[29:20]
#define WRAP_EN_SRC                     (1 << 19)
#define LDMA_ADDR_SRC                   (0)         //[17:0]


//REG_LDMA_DEST
#define WRAP_SIZE_DEST                  (20)        //[29:20]
#define WRAP_EN_DEST                    (1 << 19)
#define LDMA_ADDR_DEST                  (0)         //[17:0]


//REG_LDMA_COUNT
#define LDMA_OPT_EN                     (1 << 13)
#define LDMA_CNT                        (0)         //[10:0]


//REG_BUFFER_STATUS_1
#define BUF3_STA                        (12)        //[15:12]
#define BUF2_STA                        (8)         //[11:8]
#define BUF1_STA                        (4)         //[7:4]
#define BUF0_STA                        (0)         //[0:3]


//REG_BUFFER_STATUS_2
#define BUF15_STA                       (22)        //[23:22]
#define BUF14_STA                       (20)        //[21:20]
#define BUF13_STA                       (18)        //[18:19]
#define BUF12_STA                       (16)        //[16:17]
#define BUF11_STA                       (14)        //[15:14]
#define BUF10_STA                       (12)        //[13:12]
#define BUF9_STA                        (10)        //[11:10]
#define BUF8_STA                        (8)         //[9:8]
#define BUF7_STA                        (6)         //[7:6]
#define BUF6_STA                        (4)         //[5:4]
#define BUF5_STA                        (2)         //[3:2]
#define BUF4_STA                        (0)         //[1:0]


//REG_PAGE_DIRTY_FLAG_1


//REG_PAGE_DIRTY_FLAG_2


//REG_PAGE_CURRENT_COUNT
#define CNT_OPT_EN                      (1 << 30)
#define CURRENT_PAGE_H                  (24)        //[29:24]
#define CURRENT_CNT_INDEX               (0)         //[23:0]


//REG_BUFFER_CONFIG_1
#define SPI2TX_BUF_SIZE_512             (1 << 29)
#define SPI2TX_BUF_SEL                  (25)        //[28:25]
#define SPI1RX_BUF_SIZE_512             (1 << 24)
#define SPI1RX_BUF_SEL                  (20)        //[23:20]
#define SPI1TX_BUF_SIZE_512             (1 << 29)
#define SPI1TX_BUF_SEL                  (15)        //[18:15]
#define MASS_BUF_SIZE_512               (1 << 14)
#define MASS_BUF_SEL                    (10)        //[13:10]
#define DAC_BUF_SIZE_512                (1 << 9)
#define DAC_BUF_SEL                     (5)         //[8:5]
#define ADC_BUF_SIZE_512                (1 << 4)
#define ADC_BUF_SEL                     (0)         //[3:0]


//REG_BUFFER_CONFIG_2
#define SPI2RX_BUF_SIZE_512             (1 << 29)
#define SPI2RX_BUF_SEL                  (25)        //[28:25]
#define USB4_BUF_SIZE_512               (1 << 24)
#define USB4_BUF_SEL                    (20)        //[23:20]
#define USB3_BUF_SIZE_512               (1 << 19)
#define USB3_BUF_SEL                    (15)        //[18:15]
#define USB2_BUF_SIZE_512               (1 << 14)
#define USB2_BUF_SEL                    (10)        //[13:10]
#define USB1_BUF_SIZE_512               (1 << 9)
#define USB1_BUF_SEL                    (5)         //[8:5]
#define USB0_BUF_SIZE_512               (1 << 4)
#define USB0_BUF_SLE                    (0)         //[3:0]


//REG_BUFFER_CONFIG_3
#define UART2RX_BUF_SEL                 (22)        //[25:22]
#define UART2TX_BUF_SEL                 (18)        //[21:18]
#define UART1RX_BUF_SEL                 (14)        //[17:14]
#define UART1TX_BUF_SEL                 (10)        //[13:10]
#define PCMRX_BUF_SIZE_512              (1 << 9)
#define PCMRX_BUF_SEL                   (5)         //[8:5]
#define PCMTX_BUF_SIZE_512              (1 << 4)
#define PCMTX_BUF_SEL                   (0)         //[3:0]


//REG_BUFFER_ENABLE
#define BUF15_EN                        (1 << 15)
#define BUF14_EN                        (1 << 14)
#define BUF13_EN                        (1 << 13)
#define BUF12_EN                        (1 << 12)
#define BUF11_EN                        (1 << 11)
#define BUF10_EN                        (1 << 10)
#define BUF9_EN                         (1 << 9)
#define BUF8_EN                         (1 << 8)
#define BUF7_EN                         (1 << 7)
#define BUF6_EN                         (1 << 6)
#define BUF5_EN                         (1 << 5)
#define BUF4_EN                         (1 << 4)
#define BUF3_EN                         (1 << 3)
#define BUF2_EN                         (1 << 2)
#define BUF1_EN                         (1 << 1)
#define BUF0_EN                         (1 << 0)


//REG_PAGE_LIFE_COUNT
#define CURRENT_CNT                     (0)         //[23:0]
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of UART module register
 */

//REG_UART_CONFIG_1
#define TX_ADR_CLR                      (1UL << 31)
#define RX_ADR_CLR                      (1 << 30)
#define RX_STA_CLR                      (1 << 29)
#define TX_STA_CLR                      (1 << 28)
#define ENDIAN_SEL_BIG                  (1 << 27)
#define PARITY_EN                       (1 << 26)
#define PARITY_SEL_EVEN                 (1 << 25)
#define RX_TIMEOUT_EN                   (1 << 23)
#define BAUD_RATE_ADJ_EN                (1 << 22)
#define UART_INTERFACE_EN               (1 << 21)
#define URD_SEL_INVERSELY               (1 << 17)
#define UTD_SEL_INVERSELY               (1 << 16)
#define BAUD_RATE_DIV                   (0)         //[15:0]


//REG_UART_CONFIG_2
#define TX_TH_INT_STA                   (1 << 31)
#define RX_TH_INT_STA                   (1 << 30)
#define TX_TH_INT_EN                    (1 << 29)
#define RX_TH_INT_EN                    (1 << 28)
#define TX_END_INT_EN                   (1 << 27)
#define RX_BUF_FULL_INT_EN              (1 << 26)
#define TX_BUF_EMP_INT_EN               (1 << 24)
#define RX_ERROR_INT_EN                 (1 << 23)
#define TIMEOUT_INT_EN                  (1 << 22)
#define RX_READY_INT_EN                 (1 << 21)
#define TX_END                          (1 << 19)
#define RX_TIMEROUT_STA                 (1 << 18)
#define RX_READY                        (1 << 17)   //?????
#define TX_BYT_CNT_VLD                  (1 << 16)
#define TX_BYT_CNT                      (4)         //[15:4]
#define RX_ERROR                        (1 << 3)
#define RX_TIMEROUT                     (1 << 2)
#define RX_BUF_FULL                     (1 << 1)
#define TX_FIFO_EMPTY                   (1 << 0)


//REG_UART_DATA_CONFIG
#define BYT_LEFT                        (23)        //[24:23]
#define TX_ADDR                         (18)        //[22:18]
#define RX_ADDR                         (13)        //[17:13]
#define TX_BYT_SUM                      (0)         //[12:0]


//REG_UART_TXRX_BUF_THRESHOLD
#define TX_TH_CNT                       (17)        //[21:17]
#define RX_TH_CNT                       (12)        //[16:12]
#define TX_TH_CLR                       (1 << 11)
#define TX_TH_CFG                       (6)         //[10:6]
#define RX_TH_CLR                       (1 << 5)
#define RX_TH_CFG                       (0)         //[4:0]
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of GPIO module register
 */

//REG_GPIO_PULL_UD_1
#define ONOFF_STA                       (1 << 20)
#define AIN1_STA                        (1 << 21)
#define AIN0_STA                        (1 << 24)


//REG_WGPIO_POL
#define WK_USB_INT_POL                  (1 << 26)
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of RTC module register
 */

//REG_RTC_CTRL
#define RTC_WR_RD_EN                    (1 << 25)
#define RTC_EN                          (1 << 24)

#define RTC_WR_START                    ((1 << 21) | (2 << 18))
#define RTC_RD_START                    ((1 << 21) | (2 << 18) | (1 << 17))

#define RTC_REG0                        (0 << 14)
#define RTC_REG1                        (1 << 14)
#define RTC_REG2                        (2 << 14)
#define RTC_REG3                        (3 << 14)
#define RTC_REG4                        (4 << 14)
#define RTC_REG5                        (5 << 14)
#define RTC_REG6                        (6 << 14)
#define RTC_REG7                        (7 << 14)


//REG_RTC_READ_VAL
#define RTC_OSC_REVISE                  (1UL << 31)


//RTC_REG3, RTC_REG4, RTC_REG5
#define RTC_ALARM_EN                    (1 << 13)


//RTC_REG6(seven regiseters)
#define RTC_TIMER_EN                    (1 << 13)
#define RTC_WATCHDOG_EN                 (1 << 13)


//RTC_REG7
#define RTC_WATCHDOG_RD                 (0 << 10)
#define RTC_TIMER_WR                    (0 << 10)
#define RTC_WATCHDOG_WR                 (1 << 10)
#define RTC_TIMER_RD                    (1 << 10)
#define RTC_OSC_SOURCE                  (2 << 10)
#define RTC_SAVE1                       (3 << 10)
#define RTC_SAVE2                       (4 << 10)
#define RTC_SAVE3                       (5 << 10)
#define RTC_SAVE4                       (6 << 10)

#define RTC_WATCHDOG_FED                (1 << 6)
#define RTC_WATCHDOG_OUT                (1 << 5)
#define RTC_REAL_TIME_RD                (1 << 4)
#define RTC_REAL_TIME_WR                (1 << 3)
#define RTC_ALARM_OUT                   (1 << 2)
#define RTC_TIMER_STA_CLR               (1 << 1)
#define RTC_ALARM_STA_CLR               (1 << 0)
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of CAMERA module register
 */

//REG_IMG_CLIP_SIZE
#define CAM_CLIP_VSIZE                  (16)


//REG_IMG_CLIP_OFFSIZE
#define CAM_CLIP_VOFFSET                (16)


//REG_IMG_SINFO
#define CAM_SINFO_HEIGHT                (16)


//REG_IMG_DINFO
#define CAM_DINFO_HEIGHT                (16)


//REG_IMG_CFG
#define CAM_CFG_CCIR656                 (1 << 12)
#define CAM_CFG_PCLK_pos                (1 << 10)
#define CAM_CFG_HREF_pos                (1 << 9)
#define CAM_CFG_VREF_pos                (1 << 8)
#define CAM_CFG_DATA_YUV                (1 << 5)


//REG_IMG_LINE_CFG
#define CAM_LINE_CUR_BUF                (1UL << 31)
#define CAM_LINE_INT_STA                (1 << 17)
#define CAM_LINE_INT_EN                 (1 << 16)


//REG_IMG_CMD
#define CAM_CMD_FORMAT                  (8)
#define CAM_CMD_CLIP_EN                 (1 << 6)


//REG_IMG_CAP_STOP
#define CAM_AUTO_MODE                   (1 << 1)
#define CAM_STOP_CAP                    (1 << 0)
/** @} */



/*
================================================================================
================================================================================
*/
/** @{@bit of xxxx module register
 */



/** @} */

#endif  //__ANYKA_BITS_H__

