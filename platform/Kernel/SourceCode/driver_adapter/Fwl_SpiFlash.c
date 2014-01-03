#ifdef OS_ANYKA

#include "anyka_bsp.h"
#include "hal_spiflash.h"
#include "fwl_spiflash.h"
#include "eng_debug.h"
#include "fha.h"
#include "Gbl_Global.h"

typedef enum tag_SPIFlashErrorCode
{
    SF_SUCCESS        =    ((T_U16)1),
    SF_FAIL           =    ((T_U16)0),           //FOR DEBUG/
}E_SPIFLASHERRORCODE; 
#if 0

typedef struct SPIFlash T_SPIFLASH;
typedef struct SPIFlash* T_PSPIFLASH;


typedef E_SPIFLASHERRORCODE  (*fSPIFlash_WritePage)(T_PSPIFLASH spiFlash, T_U32 page, const T_U8 data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_ReadPage)(T_PSPIFLASH spiFlash, T_U32 page, T_U8 data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_EraseBlock)(T_PSPIFLASH spiFlash, T_U32 block);


struct SPIFlash
{
    T_U32 total_page;
    T_U32 page_size;
    T_U32 PagesPerBlock;
    fSPIFlash_WritePage WritePage;
    fSPIFlash_ReadPage ReadPage;
    fSPIFlash_EraseBlock EraseBlock;
};
#endif

T_SFLASH_PARAM g_SPIFlashParam;

T_BOOL Fwl_spiflash_init(SFlash_ID sflash_id, T_eSFLASH_BUS bus_width)
{
    return spi_flash_init(sflash_id, bus_width);
}
T_VOID Fwl_spiflash_set(pSFlash_PARAM sflash_param)
{
    spi_flash_set_param(sflash_param);
}
T_U32 Fwl_spiflash_getid(T_VOID)
{
    return spi_flash_getid();
}
T_BOOL Fwl_spiflash_write(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    return spi_flash_write(page, buf, page_cnt);
}
T_BOOL Fwl_spiflash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    return spi_flash_read(page, buf, page_cnt);
}
T_BOOL Fwl_spiflash_erase(T_U32 sector)
{
    return spi_flash_erase(sector);
}

T_U32 Fwl_spiflash_blocknum(T_VOID)
{
 return(g_SPIFlashParam.total_size/g_SPIFlashParam.erase_size);
}

T_U32 sflash_remap_read(T_U32 rowAddr,  T_U8 *data,  T_U32 *spare, T_BOOL  bBin)
{
    return Fwl_spiflash_read(rowAddr, data, 16);
}
T_BOOL sflash_remap_init(T_pVOID pParam)
{
    T_SFLASH_PARAM *pPara;
    T_BOOL bRet;
    
    bRet = spi_flash_init(SPI_ID0, SPI_BUS4);
    if(AK_FALSE == bRet)
    {   
        return bRet;
    }

    pPara = (T_SFLASH_PARAM*)(pParam);

    g_SPIFlashParam.id         = pPara->id;
    g_SPIFlashParam.total_size = pPara->total_size;
    g_SPIFlashParam.page_size = pPara->page_size;
    g_SPIFlashParam.erase_size = pPara->erase_size;
    g_SPIFlashParam.program_size = pPara->program_size;
    g_SPIFlashParam.flag = pPara->flag;
    g_SPIFlashParam.clock = pPara->clock;
    g_SPIFlashParam.protect_mask = pPara->protect_mask;

    akerror("sflash param id:", g_SPIFlashParam.id, 1);
    akerror("total_size:", g_SPIFlashParam.total_size, 1);
    akerror("page size:", g_SPIFlashParam.page_size, 1);
    akerror("erase size:", g_SPIFlashParam.erase_size, 1);
    akerror("progarm size", g_SPIFlashParam.program_size, 1);
    akerror("flag:", g_SPIFlashParam.flag, 1);
    akerror("clock:", g_SPIFlashParam.clock, 1);
    akerror("pro_mask:", g_SPIFlashParam.protect_mask, 1);

    spi_flash_set_param(&g_SPIFlashParam);


    //memcpy(&g_SPIFlashParam, &param, sizeof(T_SFLASH_PARAM));
    return bRet;
}

T_U32 sflash_get_pagesize(T_VOID)
{
    return g_SPIFlashParam.page_size;
}

T_U32 sflash_get_erasesize(T_VOID)
{
    return g_SPIFlashParam.erase_size;
}

T_U32 sflash_get_blockpgs(T_VOID)
{
    return (g_SPIFlashParam.erase_size / g_SPIFlashParam.page_size);
}

T_U32 sflash_get_deepstandby_pages(T_VOID)
{
#ifdef    DEEP_STANDBY
    return CALC_SPI_PAGES(SPIFLASH_PAGE_SIZE, SPIFLASH_BOLCK_SIZE, REMAP_MEM_PSIZE);
#else
    return 0;
#endif
}

#pragma arm section code = "_update_"

#if ((UPDATA_USED == 1) && (STORAGE_USED == SPI_FLASH))

/**
 * @brief:      FHA Erase spi flash
 * @author:   ChenWeiwen
 * @date:      2008-06-06
 * @param:   nChip:chip
 * @param:   nPage:page
 * @return:    T_U32:Erase fail or success
 * @retval
 */
T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage)
{
    T_U32 nBlock = 0;
    T_U32 total_page = 0;   //need erase page number
    T_U32 PagesPerBlock = 0;//page  number of  Each pages

    total_page = g_SPIFlashParam.total_size / g_SPIFlashParam.page_size;
    PagesPerBlock = g_SPIFlashParam.erase_size / g_SPIFlashParam.page_size;

    nBlock = nPage / PagesPerBlock;
    
    if ((nBlock + 1) * PagesPerBlock > total_page)
    {
        return SF_FAIL;
    } 

    if (!spi_flash_erase(nBlock))//erase block
    {
        return FHA_FAIL;
    }   
        
    return FHA_SUCCESS;
}

/**
 * @brief:      FHA read spi flash
 * @author:   ChenWeiwen
 * @date:      2008-06-06
 * @param:   nChip:chip
 * @param:   nPage:page
 * @param:   pData:data 
 * @param:   nDataLen:data length
 * @return:   T_U32:fact read data length
 */

T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{ 
    T_U32 i = 0;
    T_U32 total_page = 0;   //need read page number
    T_U32 PagesPerBlock = 0;//page  number of  Each pages

    total_page = g_SPIFlashParam.total_size / g_SPIFlashParam.page_size;
    PagesPerBlock = g_SPIFlashParam.erase_size / g_SPIFlashParam.page_size;

    if ((nPage + nDataLen) >= total_page)
    {
        return SF_FAIL;
    } 

    
    for (i = 0; i < nDataLen; i++)
    {
        if (!spi_flash_read(nPage + i, pData + i * g_SPIFlashParam.page_size, 1))//read data from spi falsh 
        {
            return FHA_FAIL;
        }
    }

    return FHA_SUCCESS;
}

/**
 * @brief:      FHA write spi flash
 * @author:   ChenWeiwen
 * @date:      2008-06-06
 * @param:   nChip:chip
 * @param:   nPage:page
 * @param:   pData:data 
 * @param:   nDataLen:data length
 * @return:    T_U32:fact write data length
 */
T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    T_U32 total_page = 0;   //need write page number

    total_page = g_SPIFlashParam.total_size / g_SPIFlashParam.page_size;

    if ((nPage + nDataLen) >= total_page)
    {
        return SF_FAIL;
    } 
        
    if (!spi_flash_write(nPage, pData, nDataLen))//write data to spi falsh 
    {
        return FHA_FAIL;
    }    
   
    return FHA_SUCCESS;
}

#endif//#if ((UPDATA_USED == 1) && (STORAGE_USED == SPI_FLASH))
#pragma arm section code

#endif

