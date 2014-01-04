/**@file hal_common_sd.h
 * @brief provide common operations of sd and sdio.
 *
 * This file describe sd common driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_COMMON_SD_H__
#define __HAL_COMMON_SD_H__

#include "anyka_types.h"
#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SD_IDENTIFICATION_MODE_CLK  (350*1000)              ///<350k
#define SD_TRANSFER_MODE_CLK        (20*1000*1000)          ///<20M
#define HS_TRANSFER_MODE_CLK1       (30*1000*1000)          ///<30M
#define HS_TRANSFER_MODE_CLK2       (26*1000*1000)          ///<26M
#define SD_HCS                      (1<<30)
#define SD_STATUS_POWERUP           (1UL<<31)
#define SD_CCS                      (1<<30)
#define SD_OCR_MASK                 (0xffffffff)
#define SD_DEFAULT_VOLTAGE          (0x00FF8000)
#define ERROR_INVALID_RCA           T_U32_MAX

/**
*@brief card type define
*/

typedef enum _CARD_TYPE
{
    CARD_UNUSABLE=0,                            ///< unusable card
    CARD_MMC,                                   ///< mmc card
    CARD_SD,                                    ///< sd card, mem only
    CARD_SDIO,                                  ///< sdio card, io only
    CARD_COMBO                                  ///< combo card,mem and sdio
}T_eCARD_TYPE;

typedef enum _SD_INIT_STATUS
{
    SD_INIT_VALID,                       ///<SD uninit or init failed
    SD_INIT_MMC,                        ///<MMC init successful
    SD_INIT_SD							///<SD init successful
}T_eSD_INIT_STATUS;



/**
*@brief Common sd device define,contain some attributes of sd sdio and mmc
*/
typedef struct _SD_DEVICE
{
    T_eCARD_INTERFACE enmInterface;              ///< The interface used by sd/mmc card
    T_eCARD_TYPE enmCardType;                    ///< Card type is got when init finish
    T_eBUS_MODE enmBusMode;                      ///< the bus mode used by sd/mmc card
    T_BOOL  bInitMemSuccess;                     ///< Init mem success flag
    T_BOOL  bHighCapacity;                       ///< High capacity flag
    T_U32   ulCapacity;                          ///< The capacity of sd/mmc card,number of blocks
    T_U32   ulDataBlockLen;                      ///< The current block length of sd card
    T_U32   ulRCA;                               ///< Card address
    T_U32   ulMaxReadBlockLen;                   ///< This param is got from csd
    T_U8    ucSpecVersion;                       ///< MMC or sd card spec version 
    T_U32   ulCSD[4];                            ///< CSD of sd/mmc card
    T_BOOL  bSpeedmode;                          ///high speed timing interface:1;default:0
} T_SD_DEVICE,*T_pSD_DEVICE;

/**
*@brief Define some status used when init card
*/
typedef enum _COMMON_SD_STATUS
{
    COMMON_SD_SKIP_INIT_IO,                     ///< Not need to init io
    COMMON_SD_INIT_IO_FAIL,                     ///< init io fail
    COMMON_SD_INIT_IO_SUCCESS,                  ///< init io successful
    COMMON_SD_INIT_IO_ERROR,                    ///< init io error
    COMMON_SD_SKIP_INIT_MEM,                    ///< not need to init mem
    COMMON_SD_INIT_MEM_FAIL,                    ///< init mem fail
    COMMON_SD_INIT_MEM_SUCCESS,                 ///< init mem successful
    COMMON_SD_INIT_MEM_ERROR,                   ///< init mem error
    COMMON_SD_INIT_FAIL                         ///< init card fail
}T_eCOMMON_SD_STATUS;


/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE init_card(T_pSD_DEVICE drv_card);

/**
 * @brief Slect or reject a mmc or sd card.
 *
 * Send CMD7 to select a sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param rca[in] The selected card relative address 
 * @return T_BOOL
 * @retval AK_TRUE Select successful
 * @retval AK_FALSE Select failed
 */
T_BOOL select_card(T_U32 rca);

#ifdef __cplusplus
}
#endif


#endif    //__HAL_COMMON_SD_H__


