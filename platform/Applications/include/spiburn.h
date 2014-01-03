#ifndef _SFLASH_BURN_H_
#define _SFLASH_BURN_H_

#include "burner.h"
#include "fwl_spiflash.h"

#define  SPIBOOT_DATA_START_PAGE        257  //数据起始页，这一页存文件信息 

#define     FILE_MAX_NUM    9

typedef struct
{
    T_U32 file_length;
    T_U32 ld_addr;
    T_U32 start_page;
    T_U32 backup_page;        //backup data start page
    T_U8  file_name[16];
}
T_SPI_FILE_CONFIG;

T_U32 Sflash_BurnInit(T_U32 *chipID, T_SFLASH_PARAM *spi_info);
T_U32 Sflash_BurnCheckAsa(T_VOID);
T_U32 Sflash_BurnBootStart(T_U32 dataLen);
T_U32 Sflash_BurnBinStart(T_BIN_INFO *pBinFile);
T_U32 Sflash_BurnCompareData(T_U8* pData_src, T_U8* pBuf, T_U32 data_len);
T_U32 Sflash_BurnWriteData(T_U8* pData, T_U32 data_len);
T_U32 Sflash_BurnWriteConfig(T_VOID);
T_U32 Sflash_BurnWritePage(T_U8 *buf, T_U32 startPage, T_U32 PageCnt);


#endif
