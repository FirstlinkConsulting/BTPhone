/************************************************************************
* Copyright (c) 2011, Anyka Co., Ltd. 
* All rights reserved.  
*  
* File Name：prog_manager.h
* Function：offer the program bin file manage interfence
*
* Author：xuping
* Date：2011-03-02
* Version：0.1.0
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
#ifndef _PROG_MANAGER_H_
#define _PROG_MANAGER_H_

#include "anyka_types.h"


#define SPI_FLASH       0
#define SD_CARD         1
#define NAND_FLASH      2

#if (STORAGE_USED == NAND_FLASH)
#define NAND_PARAM_ADDR       0x0080003c
#endif


#if (STORAGE_USED == SPI_FLASH)
//#define SFLASH_FILE_CONFIG_OFFSET   (257 + DEEPSTDB_PAGE_NUM)//reserved DEEPSTDB_BOLCK_NUM blocks to deep standby.
#define SFLASH_FILE_CONFIG_OFFSET   257
#define SFLASH_PARAM_ADDR   0x00800040
 
 typedef struct
{
    T_U32 file_length;
    T_U32 ld_addr;
    T_U32 start_page;
    T_U32 backup_page;        //backup data start page
    T_U8  file_name[16];
}T_SPI_FILE_CONFIG;

 
#endif

#if (STORAGE_USED == SD_CARD)
#define SD_STRUCT_INFO_SECTOR     2
typedef struct
{
    T_U32 file_length;          //file data length
    T_U32 ld_addr;              //link address
    T_U32 start_sector;         //origin data start sector
    T_U32 backup_sector;        //backup data start sector
    T_U8  file_name[16];        //name
}T_SD_FILE_INFO;

typedef struct
{
    T_U32 file_cnt;             //count of bin file
    T_U32 data_start;           //reserve zone start sector, bin data start sector if resv_size is zero
    T_U32 resv_size;            //size of reserve zone
    T_U32 bin_info_pos;         //start sector of bin file info
    T_U32 bin_end_pos;          //end sector of bin file data 
}T_SD_STRUCT_INFO;   
#endif

#ifdef DEEP_STANDBY
 typedef struct
 {
     T_U32 vaddr;
     T_U32 size;
 }T_DMA_INFO;
 
 typedef T_VOID (*T_fPOWEROFF_CALLBACK)(T_VOID);
 
#endif


T_VOID progmanage_StorageGetInfo(T_U8* pagebit, T_U8* blockbit);

T_S8 progmanage_LoadStorage512Byte(T_U32 block,T_U32 sector, T_U32 offset,T_U8 data[],T_U8* filepath);

T_S8 progmanage_LoadStorageMulti512Byte(T_U32 block,T_U32 sector, T_U32 offset,T_U8 data[],T_U8* filepath,T_U16 size);
 /******************************************************************************
* @NAME    progmanage_get_maplist 
* @BRIEF   get the bin file map info and check data is valid 
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   strName:bin file name
*          mapData:the buf to store the map info 
*          length: buf lenght
* @RETURN  bin file length
*   
*******************************************************************************/
T_U32 progmanage_get_maplist (T_S8 strName[], T_U16 *mapData, T_U32 Length);

/******************************************************************************
* @NAME    progmanage_recover_block 
* @BRIEF   rcover the block from backup block
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   strName:bin name
*          block: block index
           pBuf: buffer of operation. The size is must multiple of 4kB.
* @RETURN  AK_TRUE: success   AK_FALSE:failure
*   
*******************************************************************************/
T_BOOL progmanage_recover_block (T_S8 strName[], T_U32 block, T_U8 * pBuf);

/******************************************************************************
* @NAME    progmanage_init 
* @BRIEF   init the program bin managent
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   T_VOID
* @RETURN  T_VOID
*   
*******************************************************************************/
T_VOID progmanage_init(T_VOID);


/******************************************************************************
* @NAME    progmanage_free_mapinfo
* @BRIEF   free vaddr2store info
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   vaddr:start vaddr
*          size:free addr size
* @RETURN  bin file length
*   
*******************************************************************************/
T_U32 progmanage_free_mapinfo(T_U32 vaddr, T_U32 size);


T_VOID exception_handler(T_U32 pc_lr, T_U32 type, T_U32 sp, T_U32 cur_lr);

T_VOID progmanage_set_protect(T_BOOL protect);

/**
 * @brief check the page can write or not
 * @param T_U32 pageno, the page number to check
 * @retval T_BOOL, if writeable, return AK_TRUE, else dead
 */
T_BOOL progmanage_check_page_writeable(T_U32 chip, T_U32 pageno);

/**
 * @brief get the location of a specificated block of the file reserved
 * @param strName, the given file name
 * @param index, the block index
 * @retval T_U32, the block location. if error, return 0
 */
T_U32 progmanage_get_backup_block(T_S8 strName[], T_U32 index);

/**
 * @brief when abort exception happens to check irq mode or not.
 * @param T_U32 ischeck, set to check irq or not.
 * @retval void.
 */
T_VOID progmanage_abort_checkirq(T_BOOL ischeck);

/**
 * @brief if it checks irq mode at abort or not.
 * @param void.
 * @retval T_BOOL true ischeck, flase is not.
 */
T_BOOL progmanage_is_checkirq(T_VOID);

#if (STORAGE_USED == SPI_FLASH)
/******************************************************************************
* @NAME    spiflash_file_read
* @BRIEF   read the resource file from spi flash
* @AUTHOR  luqizhou
* @DATE    2011-07-15
* @PARAM   name:file name
*          offset:read offset
*          pBuf:pointer of the buffer
*          len:read bytes
*          bAlign:the page of the file needed to be aligned by block or not
* @RETURN  T_BOOL
*   
*******************************************************************************/
T_BOOL spiflash_file_read(T_S8 *name, T_U32 offset, T_U8 *pBuf, T_U32 len, T_BOOL bAlign);
#endif

#if (STORAGE_USED == SD_CARD)
/******************************************************************************
* @NAME    SDCARD_CFGFILE_RW
* @BRIEF   read the resource file from spi flash
* @AUTHOR  ZHS
* @DATE    2011-10-18
* @PARAM   start_sec:the first Sector of Config Bin Store in SD Card 
*          offset:File offset
*          pBuf:pointer of the buffer
*          len:read bytes
*          bRead:to read or write File
* @RETURN  T_BOOL
*
*******************************************************************************/
T_BOOL SDCARD_CFGFILE_RW(T_U32 start_sec, T_U32 FileOffset, T_U8 *pBuf,T_U32 len, T_BOOL bRead);
#endif
#ifdef DEEP_STANDBY
/******************************************************************************
 * @brief   释放非常驻236K的映射关系，将dirty的数据回写到nandflash上
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_VOID
 * @retval  T_VOID
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform.     
 ******************************************************************************/

T_VOID deepstdb_unmap(T_VOID);

/******************************************************************************
 * @brief   select standby block, save fixed data after calculating sum, then shut down GPIO
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_VOID
 * @retval  T_VOID
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform     
 ******************************************************************************/

T_VOID deepstdb_hang(T_VOID);

/******************************************************************************
 * @brief   save position and size of DMA memory
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [out] T_DMA_INFO dma_info[]: save position and size of DMA memory
            [in] T_U32 groups: max groups of DMA app
 * @return  T_U32: using DMA groups actually
 ******************************************************************************/

T_U32 deepstdb_DMA_save(T_DMA_INFO dma_info[], T_U32 groups);

/******************************************************************************
 * @brief   resume application of DMA memory
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in] T_DMA_INFO dma_info[]: position and size of DMA memory
            [in] T_U32 groups: using groups of DMA app
 * @return  T_VOID
******************************************************************************/

T_VOID deepstdb_DMA_resume(T_DMA_INFO dma_info[], T_U32 groups);

/******************************************************************************
 * @brief   the callback function of deep standby.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_fPOWEROFF_CALLBACK callback_func:the address of the callback function.
 * @return  NONE
 ******************************************************************************/

T_VOID deepstdb_set_callback(T_fPOWEROFF_CALLBACK callback_func);

/******************************************************************************
 * @brief check deep standby flag, load saved data, and resume cpu status.
          finally, go back to the original place.
 * @author  luojianhua
 * @date    2009-08-25
 * @param T_VOID
 * @retval T_VOID
******************************************************************************/

T_VOID deepstdb_handler(T_VOID);


#endif

#ifdef SWAP_WRITBAK_PRINT
/************************************************************************
 * @BRIEF get current swap count
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL current swap count
 **************************************************************************/
T_U32 get_the_swap_cnt(T_VOID);

/************************************************************************
 * @BRIEF get current write back count
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL current write back count
 **************************************************************************/
T_U32 get_the_writbak_cnt(T_VOID);

/************************************************************************
 * @BRIEF clear the swap count
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL T_VOID
 **************************************************************************/
T_VOID clear_the_swap_cnt(T_VOID);

/************************************************************************
 * @BRIEF clear the write back count
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL T_VOID
 **************************************************************************/
T_VOID clear_the_writbak_cnt(T_VOID);

#endif

#endif
