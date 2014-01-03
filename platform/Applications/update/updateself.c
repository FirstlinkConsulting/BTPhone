#include "anyka_types.h"
//#include "utils.h"
#include "updateself.h" 
#include "burner.h" 
#include "spiburn.h"

#include "eng_debug.h" 

#pragma arm section code = "_bootbss_" 

T_PRODCALLBACK                   gpf = {0};             //burn callback function
#pragma arm section code

#define     UPDATE_HEAD_SIZE        8
#define     VOLUME_LABLE_LEN        12

static const T_U8     m_update_file_head[UPDATE_HEAD_SIZE] = {'A','N','Y','K','A','1','0','6'}; 

T_UPDATE_FILE_HEAD* m_pUFH = AK_NULL;
E_CHIP_TYPE g_chip_type = CHIP_RESERVER;

//#pragma arm section code = "_bootcode1_" 
#pragma arm section code = "_update_"
T_BOOL UnpacketSpiBinFile(T_hFILE hFile)
{
    T_U32 i;
    T_S32 read_len;
    T_U32 info_pos;
    T_FILE_INFO file_info;
    T_BIN_PARAM DownloadBin;

    info_pos = m_pUFH->file_info_offset;

    gpf.fDriver.Printf("PR_U file count: %d, info_pos:%d\r\n", m_pUFH->file_count, info_pos);

    for(i=0; i<m_pUFH->file_count; i++)
    {
        gpf.fMem.MemSet(&DownloadBin, 0, sizeof(T_BIN_PARAM));
        gpf.fFs.FileSeek(hFile, info_pos, FS_SEEK_SET);

        read_len = gpf.fFs.FileRead(hFile, &file_info, sizeof(T_FILE_INFO));

        if(read_len != sizeof(T_FILE_INFO))
        {
            return AK_FALSE;
        }

        DownloadBin.data_length = file_info.file_length;
        DownloadBin.ld_addr= file_info.ld_addr;
        gpf.fMem.MemCpy(DownloadBin.file_name, file_info.file_name, 15);
        DownloadBin.bCompare = file_info.bCompare;
        DownloadBin.bUpdateself = AK_FALSE;

        gpf.fFs.FileSeek(hFile, file_info.file_offset, FS_SEEK_SET);
        
        if(!Burner_WriteSpiBinFile(hFile, &DownloadBin, g_chip_type))
        {
            return AK_FALSE;
        }

        info_pos += sizeof(T_FILE_INFO);
    }

    return Sflash_BurnWriteConfig();
}  
#pragma arm section code


//Caclutate check sum
T_U32 Cal_CheckSum(T_U8* data, T_U32 len)
{
    T_U32 sum = 0;
    T_U32 i;

    for(i = 0; i < len-2;)
    {
        T_U16 xor1 = (T_U16)data[i];
        T_U16 xor2 = (T_U16)data[i+2];
        
        sum += xor1 ^ xor2;
        i +=2;
    }

    return sum;
}

T_BOOL UnpacketHeadInfo(T_hFILE hFile)
{
    T_U32 i;
    T_S32 read_len;

    m_pUFH = (T_UPDATE_FILE_HEAD*)gpf.fMem.RamAlloc(sizeof(T_UPDATE_FILE_HEAD));

    if(AK_NULL == m_pUFH)
    {
        return AK_FALSE;
    }
    
    read_len = gpf.fFs.FileRead(hFile, (T_pVOID)m_pUFH, sizeof(T_UPDATE_FILE_HEAD));    

    if(read_len != sizeof(T_UPDATE_FILE_HEAD))
    {
        return AK_FALSE;
    }

    for(i=0; i<UPDATE_HEAD_SIZE; i++)
    {	
    	//AK_DEBUG_OUTPUT("D[%d]:%c", i, m_pUFH->head_info[i]);
        if(m_pUFH->head_info[i] != m_update_file_head[i])
        {
            break;
        }
    }

    if(i<UPDATE_HEAD_SIZE)
    {
         return AK_FALSE;
    }
    if(m_pUFH->check_sum != Cal_CheckSum((T_U8*)m_pUFH, sizeof(T_UPDATE_FILE_HEAD)-sizeof(m_pUFH->check_sum)))
    {
          return AK_FALSE;
    }
	
    return AK_TRUE;
}


T_VOID Prod_CallbackInit(T_PRODCALLBACK* fProd, T_U32 fsChipID)
{
    gpf.fMem.RamAlloc = fProd->fMem.RamAlloc;
    gpf.fMem.RamFree = fProd->fMem.RamFree;
    gpf.fMem.RamReAlloc = fProd->fMem.RamReAlloc;
    gpf.fMem.MemCpy = fProd->fMem.MemCpy;
    gpf.fMem.MemSet= fProd->fMem.MemSet;
    gpf.fMem.MemCmp = fProd->fMem.MemCmp;
    gpf.fMem.MemMov = fProd->fMem.MemMov;

    gpf.fDriver.Printf = fProd->fDriver.Printf;
    gpf.fDriver.EraseBlock = fProd->fDriver.EraseBlock;
    gpf.fDriver.ReadPage = fProd->fDriver.ReadPage;
    gpf.fDriver.WritePage = fProd->fDriver.WritePage;
    gpf.fDriver.ReadBootPage = fProd->fDriver.ReadBootPage;
    gpf.fDriver.WriteBootPage = fProd->fDriver.WriteBootPage;
    gpf.fDriver.ReadASAPage = fProd->fDriver.ReadASAPage;
    gpf.fDriver.WriteASAPage = fProd->fDriver.WriteASAPage;
    gpf.fDriver.ReadBytes = fProd->fDriver.ReadBytes;

    gpf.fFs.FileRead = fProd->fFs.FileRead;
    gpf.fFs.FileSeek = fProd->fFs.FileSeek;

  	g_chip_type = fsChipID;
	
 //   gpf.fDriver.Printf("\r\nProdLib version:%d.%d.%d_%s\r\n", PROD_MAIN_VERSION, PROD_SUB_VERSION, PROD_SUB0_VERSION, PROD_SVN);
}


T_BOOL UnpacketSpiFileInit(T_SPIBURNFUNC* fSpi, T_hFILE hFile, T_SFLASH_PARAM *spi_info)
{
    T_U32 chipID;
    T_PRODCALLBACK callback;

    callback.fDriver.Printf = fSpi->Printf;
    callback.fFs.FileSeek   = fSpi->fSeek;
    callback.fFs.FileRead   = fSpi->fRead;
    callback.fMem.RamAlloc  = fSpi->RamAlloc;
    callback.fMem.RamFree   = fSpi->RamFree;
    callback.fMem.MemCpy    = fSpi->MemCpy;
    callback.fMem.MemSet    = fSpi->MemSet;
    callback.fMem.MemCmp    = fSpi->MemCmp;
	
	akerror("UnpacketSpiFileInit: start",0,1);	
    Prod_CallbackInit(&callback, CHIP_10XXC);

	gpf.pComBuf = gpf.fMem.RamAlloc(SPI_UPDATE_SELF_MAX_STACK);
	if (AK_NULL == gpf.pComBuf)
	{	
		akerror("UnpacketSpiFileInit: malloc fail",0,1);	
        return AK_FALSE;
	}
	// 获取解压包的文件是否正确
    if(!UnpacketHeadInfo(hFile))
    {
		akerror("UnpacketSpiFileInit: UnpacketHeadInfo fail",0,1);	
        return AK_FALSE;
    }

	// 初始化SPI的参数是否正确
    if(!Sflash_BurnInit(&chipID, spi_info))
    {
		akerror("UnpacketSpiFileInit: Sflash_BurnInit fail",0,1);	
        return AK_FALSE;
    }
 
    return AK_TRUE;
}    


#pragma arm section code = "_update_"

T_BOOL UnpacketSpiBootFile(T_hFILE hFile, T_BOOL bCompare)
{
    T_BOOT_PARAM BootParam;
    BootParam.chip_type = CHIP_10XXC;
    BootParam.nRamRegNum = 0;

    gpf.fFs.FileSeek(hFile, m_pUFH->bios_offset, FS_SEEK_SET);

    if(!Burner_WriteSpiBootFile(hFile, m_pUFH->bios_size, bCompare, &BootParam, g_chip_type))
    {
        return AK_FALSE;
    }
    
    return AK_TRUE;
}
#pragma arm section code

