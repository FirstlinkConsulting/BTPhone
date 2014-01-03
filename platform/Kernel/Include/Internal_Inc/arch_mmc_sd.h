/**
 * @file arch_mmc_sd.h
 * @brief list SD card operation interfaces.
 *
 * This file define and provides functions of SD card
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author HuangXin
 * @date 2010-06-17
 * @version 2.0 for AK322x
 */
#ifndef __ARCH_MMC_SD_H
#define __ARCH_MMC_SD_H


#include "anyka_types.h"


/** @defgroup MMC_SD_SDIO MMC_SD_SDIO
 *    @ingroup Drv_Lib
 */
/*@{*/

typedef T_VOID * T_pCARD_HANDLE;

typedef enum
{
    INTERFACE_MMC1 = 0,
    INTERFACE_SD2,
    INTERFACE_NOT_SD
} T_eCARD_INTERFACE;

typedef enum
{
    USE_ONE_BUS,
    USE_FOUR_BUS,
    USE_EIGHT_BUS
} T_eBUS_MODE;


/**
* @brief initial mmc sd or comob card
* @author Huang Xin
* @date 2010-06-17
* @param cif[in] card interface selected
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sd_initial(T_eCARD_INTERFACE cif, T_eBUS_MODE bus_mode);

/**
 * @brief read data from sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_src[in] source block to read
 * @param databuf[out] data buffer to read
 * @param block_count[in] size of blocks to be readed
 * @return T_BOOL
 * @retval  AK_TRUE: read successfully
 * @retval  AK_FALSE: read failed
 */
T_BOOL sd_read_block(T_pCARD_HANDLE handle, T_U32 block_src, T_U8 *databuf, T_U32 block_count);

/**
 * @brief write data to sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_dest[in] destation block to write
 * @param databuf[in] data buffer to write
 * @param block_count[in] size of blocks to be written
 * @return T_BOOL
 * @retval  AK_TRUE:write successfully
 * @retval  AK_FALSE: write failed
 */
T_BOOL sd_write_block(T_pCARD_HANDLE handle, T_U32 block_dest, const T_U8 *databuf, T_U32 block_count);

/**
 * @brief Close sd controller
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @return T_VOID
 */
T_VOID sd_free(T_pCARD_HANDLE handle);

/**
 * @brief get sd card information
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param total_block[out] current sd's total block number
 * @param block_size[out] current sd's block size
 * @a block = 512 bytes
 * @return T_VOID
 */
T_VOID sd_get_info(T_pCARD_HANDLE handle, T_U32 *total_block, T_U32 *block_size);

/*@}*/


#endif //__ARCH_MMC_SD_H  

