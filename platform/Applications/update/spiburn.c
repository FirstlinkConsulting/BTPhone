//#include "burn_result.h"
#include "anyka_types.h"
#include "updateself.h" 
#include "spiburn.h"
#include "fwl_spiflash.h"

typedef struct
{
    T_BOOL bBootData;
    T_U32  EraseBlocks;
    T_U32  BootDataPageIndex;
    T_U32  FileStartPage;
    T_U32  CompareStartPage;
    T_U32  FileCnt;
    T_U32  BootPages;
    T_SPI_FILE_CONFIG*  pFileInfo;
}T_DOWNLOAD_SPIFLASH; 

static T_DOWNLOAD_SPIFLASH         m_download_sflash = {0};

T_SFLASH_PARAM gSPIFlash_base;

/**
 * @BREIF    init spiflash burn
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-12-10
 * @PARAM    [out] chip ID
 * @RETURN   T_U32
 * @retval   BT_SUCCESS :  succeed
 * @retval   BT_FAIL :     fail
 */
T_U32 Sflash_BurnInit(T_U32 *chipID, T_SFLASH_PARAM *spi_info)
{
    m_download_sflash.bBootData = AK_FALSE;
    m_download_sflash.BootDataPageIndex = 0;
    m_download_sflash.CompareStartPage = 0;
    m_download_sflash.pFileInfo = AK_NULL;
    m_download_sflash.FileCnt = 0;
    m_download_sflash.EraseBlocks = 0;
    m_download_sflash.BootPages = SPIBOOT_DATA_START_PAGE;
    m_download_sflash.FileStartPage = m_download_sflash.BootPages + 1;

    gpf.fDriver.Printf("PR_S default boopages:%d, spi_info:%x\r\n", m_download_sflash.BootPages, spi_info);

    gpf.fDriver.Printf("PR_S SPIFlash_Init\r\n");  
    

    if(AK_NULL== spi_info)
    {
        gpf.fDriver.Printf("PR_S SPIFlash_Init fail\r\n");
        return AK_FALSE;
    }

    //init spiflash control
    gpf.fMem.MemCpy(&gSPIFlash_base, spi_info, sizeof(T_SFLASH_PARAM));

    *chipID = gSPIFlash_base.id;

    //malloc space for binfile info
    m_download_sflash.pFileInfo = (T_SPI_FILE_CONFIG*)gpf.fMem.RamAlloc(gSPIFlash_base.page_size);
    if(AK_NULL== m_download_sflash.pFileInfo)
    {
        return AK_FALSE;
    }

    gpf.fMem.MemSet(m_download_sflash.pFileInfo, 0, gSPIFlash_base.page_size);   

    return AK_TRUE;
} 

T_VOID Sflash_ChangeBootLength(T_U32 bootLen)
{
    m_download_sflash.BootPages = (bootLen + gSPIFlash_base.page_size - 1) / gSPIFlash_base.page_size;
    gpf.fDriver.Printf("PR_S change boolen:%d\r\n", bootLen);

    m_download_sflash.FileStartPage = m_download_sflash.BootPages + 1;
}    

T_U32 Sflash_BurnCheckAsa(T_VOID)
{
    return AK_TRUE;
}    


/**
 * @BREIF    start to write boot area 
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-12-10
 * @retval   BT_SUCCESS :  succeed
 * @retval   BT_FAIL :     fail
 */
T_U32 Sflash_BurnBootStart(T_U32 dataLen)
{
    //flag of boot area
    m_download_sflash.bBootData = AK_TRUE;

    if(dataLen > m_download_sflash.BootPages * gSPIFlash_base.page_size)
    {
        gpf.fDriver.Printf("spiboot is too large\n");
        return AK_FALSE;
    }    

    return AK_TRUE;
}


/**
 * @BREIF    get bin information to start to write bin
 * @DATE     2009-09-10
 * @PARAM    [in] bin information
 * @RETURN   T_BOOL
 * @retval   AK_TRUE :  succeed
 * @retval   AK_FALSE : fail
 */
T_U32 Sflash_BurnBinStart(T_BIN_INFO *pBinFile)
{ 
    T_SPI_FILE_CONFIG* pfile_cfg;

    //space to keep bin info
    pfile_cfg = m_download_sflash.pFileInfo + m_download_sflash.FileCnt;
   
    m_download_sflash.bBootData = AK_FALSE;
    m_download_sflash.FileCnt++;

    if(m_download_sflash.FileCnt > FILE_MAX_NUM)
    {
        gpf.fDriver.Printf("PR_S file num excess %d\r\n", FILE_MAX_NUM);
        return AK_FALSE;
    }

    pfile_cfg->file_length = pBinFile->data_length;
    pfile_cfg->ld_addr = pBinFile->ld_addr;
    pfile_cfg->start_page = m_download_sflash.FileStartPage;

    gpf.fDriver.Printf("PR_S map:%d\r\n", pfile_cfg->start_page);
    gpf.fDriver.Printf("PR_S file len:%d\r\n", pfile_cfg->file_length);
    gpf.fDriver.Printf("PR_S ld addr:0x%x\r\n", pfile_cfg->ld_addr);
   
    gpf.fMem.MemCpy(pfile_cfg->file_name, pBinFile->file_name, 16);

    return AK_TRUE;
}

#pragma arm section code = "_update_"

/**
 * @BREIF    write data buffer
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-12-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] data buffer length
 * @RETURN   T_U32 
 * @retval   BT_SUCCESS :  succeed
 * @retval   BT_FAIL :     fail
 */
T_U32 Sflash_BurnWriteData(T_U8* pData, T_U32 data_len)
{
    T_U32 page_cnt;
    T_U32* pStartPage;

     //page of starting to write 
    pStartPage = m_download_sflash.bBootData ? &(m_download_sflash.BootDataPageIndex)
                : &(m_download_sflash.FileStartPage);

  //  gpf.fDriver.Printf("PR_S start page:%d\r\n", *pStartPage);

    page_cnt = (data_len + gSPIFlash_base.page_size - 1) / gSPIFlash_base.page_size;

    if(!Sflash_BurnWritePage(pData, *pStartPage, page_cnt))
    {
		gpf.fDriver.Printf("Sflash_BurnWriteData: write fail\r\n");
        return AK_FALSE;
    }
    else
    {
		m_download_sflash.CompareStartPage = *pStartPage;
        *pStartPage += page_cnt;
    }  

    return AK_TRUE;
}

/**
 * @BREIF    read data to compare
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-12-10
 * @PARAM    [in] data buffer
 * @PARAM    [in] data buffer length
 * @RETURN   T_U32 
 * @retval   BT_SUCCESS :  succeed
 * @retval   BT_FAIL :     fail
 */
T_U32 Sflash_BurnCompareData(T_U8* pData_src, T_U8* pBuf, T_U32 data_len)
{
    T_U32 page_cnt;
	T_U32 i;
	
    page_cnt = (data_len + gSPIFlash_base.page_size - 1) / gSPIFlash_base.page_size;

    if(AK_NULL == pBuf || AK_NULL == pData_src)
    {
        return AK_FALSE;
    }
    gpf.fDriver.Printf("*");

    //gpf.fDriver.Printf("PR_S compare start page:%d\r\n", m_download_sflash.CompareStartPage);

    if(!Fwl_spiflash_read(m_download_sflash.CompareStartPage, pBuf, page_cnt))
    {
        gpf.fDriver.Printf("PR_S compare fail at page:%d\r\n", m_download_sflash.CompareStartPage);
        return AK_FALSE;
    }   

    if(0 != gpf.fMem.MemCmp(pData_src, pBuf, data_len))
    for (i=0; i<data_len; i++)
    {
		if (pData_src[i] != pBuf[i])
		{
			gpf.fDriver.Printf("PR_S compare i:%d, Src:%02x, Dst:%02x\r\n", i, pData_src[i], pBuf[i]);
			gpf.fDriver.Printf("PR_S compare fail\r\n");
			return AK_FALSE;
		}
    }

	
    return AK_TRUE;
}

/**
 * @BREIF    read bin info
 * @AUTHOR   Jiang Dihui
 * @DATE     2009-12-10
 * @RETURN   T_U32 
 * @retval   BT_SUCCESS :  succeed
 * @retval   BT_FAIL :     fail
 */
T_U32 Sflash_BurnWriteConfig(T_VOID)
{
    //T_U32 i;
    T_U8 *pBuf = AK_NULL;
 
    if(m_download_sflash.FileCnt>0)
    {
        pBuf = gpf.pComBuf;//gpf.fMem.RamAlloc(gSPIFlash_base->page_size);

        if (AK_NULL == pBuf)
        {
            return AK_FALSE;
        }
        
        gpf.fDriver.Printf("PR_S write fileinfo, count:%d\r\n", m_download_sflash.FileCnt);
        gpf.fMem.MemCpy(pBuf, &m_download_sflash.FileCnt, 4);
        gpf.fMem.MemCpy(pBuf+4, m_download_sflash.pFileInfo, gSPIFlash_base.page_size - 4);
        if(!Sflash_BurnWritePage(pBuf, m_download_sflash.BootPages, 1))
        {
            gpf.fDriver.Printf("PR_S write fileinfo fail\r\n");
            return AK_FALSE;
        }

    }
        
    gpf.fDriver.Printf("PR_S write config success\n");

    return AK_TRUE;
}

T_U32 Sflash_BurnWritePage(T_U8* buf, T_U32 startPage, T_U32 PageCnt)
{
#if 0
    T_U32 i; 
    T_U32 write_block_index;

    write_block_index = (startPage + PageCnt) / gSPIFlash_base.erase_size;

    if (m_download_sflash.EraseBlocks <= write_block_index)
    {
        for (i=m_download_sflash.EraseBlocks; i<=write_block_index; i++)
        {
            gpf.fDriver.Printf("PR_S erase:%d\r\n", i);
            if (!spi_flash_erase(i * (gSPIFlash_base.erase_size / gSPIFlash_base.page_size)))
            {
                gpf.fDriver.Printf("PR_S erase fail at block:%d\r\n", i);
                return AK_FALSE;
            } 
        } 

        m_download_sflash.EraseBlocks = write_block_index + 1;
    }
    
    if(!spi_flash_write(startPage, buf, PageCnt))
    {
        gpf.fDriver.Printf("PR_S write fail at page:%d\r\n", startPage);
        return AK_FALSE;
    }

    return AK_TRUE;
#endif
	
 	T_U32 i; 
    T_U32 write_block_index;

    write_block_index = (startPage + PageCnt) * gSPIFlash_base.page_size / gSPIFlash_base.erase_size;

    if (m_download_sflash.EraseBlocks <= write_block_index)
    {
        for (i=m_download_sflash.EraseBlocks; i<=write_block_index; i++)
        {
            gpf.fDriver.Printf("PR_S erase:%d\r\n", i);
            if (!Fwl_spiflash_erase(i))
            {
                gpf.fDriver.Printf("PR_S erase fail at block:%d\r\n", i);
                return AK_FALSE;
            } 
        } 

        m_download_sflash.EraseBlocks = write_block_index + 1;
    }

    if(!Fwl_spiflash_write(startPage, buf, PageCnt))
    {
        gpf.fDriver.Printf("PR_S write fail at page:%d\r\n", startPage);
        return AK_FALSE;
    }

    return AK_TRUE;



}
#pragma arm section code



