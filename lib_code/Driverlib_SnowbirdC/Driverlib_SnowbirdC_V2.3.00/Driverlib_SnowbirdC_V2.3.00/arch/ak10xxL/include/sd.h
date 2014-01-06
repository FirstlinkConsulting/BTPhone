/**@file arch_sd.h
 * @brief provide arch level operations of how to control sd&sdio.
 *
 * This file describe sd&sdio controller driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __ARCH_SD_H
#define __ARCH_SD_H

#include "anyka_types.h"
#include "arch_mmc_sd.h"
#include "hal_common_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

//clock ctrl reg
#define CLK_DIV_L_OFFSET           0
#define CLK_DIV_H_OFFSET           8
#define SD_CLK_ENABLE              (1<<16)
#define PWR_SAVE_ENABLE            (1<<17)
#define FALLING_TRIGGER            (1<<19)
#define SD_INTERFACE_ENABLE        (1<<20)

//cmd reg
#define CMD_INDEX_OFFSET           1
#define CPSM_ENABLE                (1<<0)
#define WAIT_REP_OFFSET            7
#define WAIT_CMD_PEND              (1<<9)
#define RSP_CRC_NO_CHK             (1<<10)
        
//status reg
#define CMD_CRC_FAIL               (1 << 0)
#define DATA_CRC_FAIL              (1 << 1)
#define CMD_TIME_OUT               (1 << 2)
#define DATA_TIME_OUT              (1 << 3)
#define CMD_RESP_END               (1 << 4)
#define CMD_SENT                   (1 << 5)
#define DATA_END                   (1 << 6)
#define DATA_BLOCK_END             (1 << 7)
#define DATA_START_BIT_ERR         (1 << 8)
#define CMD_ACTIVE                 (1 << 9)
#define TX_ACTIVE                  (1 << 10)
#define RX_ACTIVE                  (1 << 11)
#define DATA_BUF_FULL              (1 << 12)
#define DATA_BUF_EMPTY             (1 << 13)
#define DATA_BUF_HALF_FULL         (1 << 14)
#define DATA_BUF_HALF_EMPTY        (1 << 15)

//dma reg
#define BUF_SIZE_OFFSET         (17)
#define DMA_EN                  (1<<16)
#define START_ADDR_OFFSET       (1)
#define START_ADDR_MASK         (0x7fff<<1)
#define BUF_EN                  (1)

//data ctrl reg
#define SD_DATA_CTL_ENABLE              1
#define SD_DATA_CTL_DISABLE             0
#define SD_DATA_CTL_TO_HOST             1
#define SD_DATA_CTL_TO_CARD             0
#define SD_DATA_CTL_BUS_MODE_OFFSET     3
#define SD_DATA_CTL_DIRECTION_OFFSET    1
#define SD_DATA_CTL_BLOCK_LEN_OFFSET    16
#define SD_DAT_MAX_TIMER_V              0x800000

#define SD_CMD(n)                       n       
#define SD_NO_RESPONSE                  0                           
#define SD_SHORT_RESPONSE               1                               
#define SD_LONG_RESPONSE                3
#define SD_NO_ARGUMENT                  0
#define SD_POWER_SAVE_ENABLE            1
#define SD_POWER_SAVE_DISABLE           0 
#define SD_DEFAULT_BLOCK_LEN            512
#define SD_DEFAULT_BLOCK_LEN_BIT        9
#define SDIO_MAX_BLOCK_LEN              2048
#define SD_BUS_WIDTH_1BIT               0
#define SD_BUS_WIDTH_4BIT               2
#define SD_DMA_SIZE_4K                 (4*1024)
#define SD_DMA_SIZE_8K                 (8*1024)
#define INNER_BUF_MODE                  0
#define L2_CPU_MODE                     1
#define L2_DMA_MODE                     2


/**
 * @brief Set the sd interface.
 *
 * Select the sd interface(INTERFACE_MMC1 or INTERFACE_SD2)and select the relevant registers,L2 ,pin.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_MMC1 or INTERFACE_SD2.
 * @return T_VOID
 */
T_VOID set_interface(T_eCARD_INTERFACE cif, T_U8 bus_mode);


/**
 * @brief Set sd card clock.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param sd_clk[in] The main clock for sd card.
 * @param asic_freq[in] The asic clock (unit,hz)
 * @param pwr_save[in] Set this parameter true to enable power save
 * @return T_VOID
 */
T_VOID set_clock(T_U32 sd_clk, T_U32 asic_freq, T_BOOL pwr_save);


/**
 * @brief send sd command.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_BOOL
 * @retval AK_TRUE: CMD sent successfully
 * @retval AK_FALSE: CMD sent failed
 */
T_BOOL send_cmd(T_U8 cmd_index, T_U8 resp, T_U32 arg);

/**
 * @brief Get sd card  short response.
*  Get register response0.
 * Only get register response0 .
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_VOID
 */
T_VOID get_short_resp(T_U32 resp[]);


/**
 * @brief Get sd card  long response.
 *
 *  Get register response0,1,2,3.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The buffer address to save long response
 * @return T_VOID
 */
T_VOID get_long_resp(T_U32 resp[]);

/**
 * @brief SD read or write data use l2 dma or l2 cpu
 *
 * start l2 dma
 * @author Huang Xin
 * @date 2010-07-14
 * @param drv_card[in] the handle of sd card
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_trans_data_dma(T_pSD_DEVICE drv_card, T_U32 ram_addr, T_U32 size, T_U8 dir);

/**
 * @brief SD read data use cpu
 * @author Huang Xin
 * @date 2010-07-14
 * @param drv_card[in] the handle of sd card
 * @param [in] T_U8 buf[] the data address
 * @param [in] T_U32 len transfer bytes
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_rx_data_cpu(T_pSD_DEVICE drv_card, T_U8 buf[], T_U32 len);

/**
 * @brief SD write data use cpu
 * @author Huang Xin
 * @date 2010-07-14
 * @param drv_card[in] the handle of sd card
 * @param [in] T_U8 buf[] the data address
 * @param [in] T_U32 len transfer bytes
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_tx_data_cpu(T_pSD_DEVICE drv_card, T_U8 buf[], T_U32 len);


/**
* @brief Check if sd controller is transferring now
* @author Huang Xin
* @date 2010-07-14
* @return T_BOOL
* @retval  AK_TRUE: sd controller is transferring
* @retval  AK_FALSE: sd controller is not transferring
*/
T_BOOL sd_trans_busy(T_VOID);

/**
 * @brief Get the sd device.
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return T_pVOID
 */
T_pVOID get_sd_device(T_eCARD_INTERFACE cif);

#ifdef __cplusplus
}
#endif


#endif 
  
