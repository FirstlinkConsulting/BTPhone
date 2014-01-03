/**
 * @filename fwl_nandflash.h
 * @brief: AK3224M frameworks of nandflash driver.
 *
 * This file describe frameworks of nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-03
 * @version 1.0
 * @ref
 */

#ifndef __FWL_NANDFLASH_H__
#define __FWL_NANDFLASH_H__

#include "anyka_types.h" 
#include "prog_manager.h"
#include "nandflash.h"
#include "fha.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REMAP_SUCCESS
#define REMAP_SUCCESS 0
#endif

#ifndef REMAP_FAIL
#define REMAP_FAIL 1
#endif

#ifndef Fwl_nand_eraseblock
#define Fwl_nand_eraseblock nand_eraseblock
#endif

#ifndef Fwl_nand_remap_write
#define Fwl_nand_remap_write nand_remap_write
#endif

#ifndef Fwl_nand_remap_read
#define Fwl_nand_remap_read nand_remap_read
#endif


T_U32 Fwl_nand_remap_read(T_U32 nAbsPage, T_U8 *pMain,  T_U8 *pAdd, T_BOOL  bBin);
T_U32 Fwl_nand_remap_write(T_U32 nAbsPage, T_U8 *pMain,  T_U32 nAdd, T_BOOL  bBin);
T_U32 Fwl_nand_eraseblock(T_U32 nChip, T_U32 nAbsPage);

T_U32 Fwl_Nand_GetTotalBlk(T_PNANDFLASH hNF_Info);
T_U32 Fwl_Nand_BytesPerSector(T_PNANDFLASH hNF_Info);
T_U32 Fwl_Nand_PagePerBlock(T_PNANDFLASH hNF_Info);

T_U8 Fwl_Get_Nand_FsSize(T_VOID);

/** @defgroup Fwl_NandFlash Framework NandFlash Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************

/**
 * @brief   write file system infomation to nandflash without ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] sector which sector will be written.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_WriteSpare(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block, T_U32 sector, T_U8* spare, T_U32 spare_len);

/**
 * @brief   read 1 sector data from nandflash with ECC(large page).
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] data buffer for read sector, should be 512 bytes.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block, T_U32 sector, T_U8 data[], T_U8* spare, T_U32 spare_len);
/**
 * @brief   read file system infomation from nandflash without ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_ReadSpare(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block, T_U32 sector, T_U8* spare, T_U32 spare_len);

/** 
 * @brief   erase 1 block of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] block which block whill be erased.
 * @return  T_U32
 */
E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block);

/**
 * @brief   initialization of nandflash frameworks.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_PNANDFLASH
 */
T_PNANDFLASH  Nand_Init_MTD(T_VOID);

/**
 * @brief   initialization of badBlock buf.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_BOOL
 */

T_BOOL Fwl_Nand_BadBlBufInit(T_VOID);
/**
 * @brief   free bad block buf.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_VOID
 */

T_VOID Fwl_Nand_BadBlBufFree(T_VOID);


/**
 * @brief   check bad blocks of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  T_BOOL
 */
T_BOOL Nand_IsBadBlock(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block );

/**
 * @brief   Set bad block of nandflash.
 *
 * @author  LiaoZhijun
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  T_VOID
 */
T_VOID Nand_SetBadBlock(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block );

/**
 * @brief   write 1 sector data to nandflash with ECC(large page).
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] sector which sector will be written.
 * @param   [in] data buffer for write sector, should be 512 bytes.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_WriteSector_Large(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block,
                                        T_U32 sector, T_U8 data[], T_U8 *spare, T_U32 spare_len);
//********************************************************************
/*@}*/

/**
 * @brief   read 512 byte from nanflash
 *
 * @author  xuping
 * @date    2007-04-08
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] offset offset of in the page.
 * @param   [in] data buffer for read sector, should be 512 bytes.
 * @param   [in] filepath of the data(just for win32)
 * @param   return:> 0 read ok.
 */
T_S8 Nand_ReadSector_512Byte(T_U32 block,T_U32 sector, T_U32 offset,T_U8 data[],T_U8* filepath);

/**
 * @brief   read any byte from nanflash
 *
 * @author  liangxiong
 * @date    2011-02-17
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] offset offset of in the page.
 * @param   [in] data buffer for read sector, should be 512 bytes.
 * @param   [in] filepath of the data(just for win32)
 * @param   [in] size of the data(just for win32)
 * @param   return:> 0 read ok.
 */

T_S8 Fwl_Nand_ReadSector_512Byte(T_U32 nBlock,T_U32 nPage, T_U32 nSector,T_U8 *pMain,T_U8* filepath);
T_S8 Fwl_Nand_ReadSector_AnyByte(T_U32 nBlock,T_U32 nPage, T_U32 nSector,T_U8 *pMain,T_U8* filepath,T_U16 nSize);

T_BOOL Nand_WriteBootSector(T_U32 nAbsPage, const T_U8 *pMain);
T_VOID Fwl_Nand_GetNandInfo(T_U8* pagebit, T_U8* blockbit, T_BOOL bRes);
T_U32 FHA_Nand_ReadPage(T_U32 nChip, T_U32 nAbsPage, T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType);
T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nAbsPage, const T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType);
T_U32 FHA_Nand_EraseBlock(T_U32 chip, T_U32 nAbsPage);
T_BOOL FHA_Nand_ReadBytes(T_U32 nChip, T_U32 nAbsPage, T_U32 nColumn, T_U8 *pBuf, T_U32 nBufLen);
T_VOID Nand_Init_Remap(T_VOID * pNandParam);

#ifdef __cplusplus
}
#endif

#endif


