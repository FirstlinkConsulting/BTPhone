/**@file hal_sd.h
 * @brief provide hal level operations of how to control sd.
 *
 * This file describe sd hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_SD_H
#define __HAL_SD_H

#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

//sd card status
#define SD_CURRENT_STATE_OFFSET     9
#define SD_CURRENT_STATE_MASK       (0xF<<9)
#define SD_CURRENT_STATE_IDLE       0
#define SD_CURRENT_STATE_READY      1 
#define SD_CURRENT_STATE_IDENT      2
#define SD_CURRENT_STATE_STBY       3
#define SD_CURRENT_STATE_TRAN       4
#define SD_CURRENT_STATE_DATA       5
#define SD_CURRENT_STATE_RCV        6
#define SD_CURRENT_STATE_PRG        7
#define SD_CURRENT_STATE_DIS        8
#define SD_CURRENT_STATE_IO_MODE    15
//sd dma operation block length
#define SD_DMA_BLOCK_64K            (64*2)
#define SD_DMA_BLOCK_32K            (32*2)
#define SD_DMA_BLOCK_8K             (8*2)
#define SD_DMA_BLOCK_2K             (2*2)
#define SD_DMA_BLOCK_4K             (4*2)

#define SD_HIGH_SPEED_MODE          1
#define SD_DEFAULT_SPEED_MODE       0
#define SD_MMC_INVALID_SPEC_VERSION      0xff
#define SD_FUNC_SUPPORTED_GROUP1(status)    ((status[51]<<8) | status[50])  
#define SD_FUNC_SUPPORTED_GROUP2(status)    ((status[53]<<8) | status[52])  
#define SD_FUNC_SUPPORTED_GROUP3(status)    ((status[55]<<8) | status[54])  
#define SD_FUNC_SUPPORTED_GROUP4(status)    ((status[57]<<8) | status[56])  
#define SD_FUNC_SUPPORTED_GROUP5(status)    ((status[59]<<8) | status[58]) 
#define SD_FUNC_SUPPORTED_GROUP6(status)    ((status[61]<<8) | status[60])  
#define SD_FUNC_SWITCHED_GROUP1(status)     (status[47]&0x0f)
#define SD_FUNC_SWITCHED_GROUP2(status)     ((status[47]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP3(status)     (status[48]&0x0f)
#define SD_FUNC_SWITCHED_GROUP4(status)     ((status[48]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP5(status)     (status[49]&0x0f)
#define SD_FUNC_SWITCHED_GROUP6(status)     ((status[49]>>4)&0x0f)

#define MMC_SPEC_VERSION(csd)            stuff_bits((T_U16 *)csd,122,4)
#define MMC_C_SIZE(csd)                  stuff_bits((T_U16 *)csd,62,12)
#define MMC4_CARD_TYPE(extcsd)           (extcsd[49]&0xff)
#define MMC4_SECTOR_CNT(extcsd)          (extcsd[53])
#define MMC4_POWER_CLASS(extcsd)         (extcsd[50])

typedef enum
{
    SD_DATA_MODE_SINGLE,                    ///< read or write single block
    SD_DATA_MODE_MULTI                      ///< read or wirte multiply block
}
T_eCARD_DATA_MODE;

typedef enum _SD_STATUS
{
    SD_GET_OCR_VALID,                       ///<get ocr valid
    SD_GET_OCR_FAIL,                        ///<get ocr fial
    SD_GET_OCR_INVALID,                     ///<get ocr invalid
    SD_NEGO_SUCCESS,                        ///< sd nego voltage success
    SD_NEGO_FAIL,                           ///< sd nego voltage fail
    SD_NEGO_TIMEOUT                         ///< sd nego voltage timeout
}T_eSD_STATUS;

/**
 * @brief Init the mem partion 
 * Called when init card 
 * @param drv_card[in] the handle of sd card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
T_U8 init_mem(T_pSD_DEVICE drv_card);

/**
 * @brief Switch or expand memory card functions.
 *
 * This function is supported after sd version 1.10
 * @author Huang Xin
 * @date 2010-07-14
 * @param mode[in] mode
 * @param group[in] group
 * @param value[in] value
 * @param resp[in] response
 * @return T_BOOL
 * @retval  AK_TRUE: CMD sent successfully
 * @retval  AK_FALSE: CMD sent failed
 */
T_BOOL sd_mode_switch(T_U32 mode, T_U32 group, T_U8 value, T_U32 *resp);


#ifdef __cplusplus
}
#endif


#endif 
  
