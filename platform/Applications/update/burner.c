#include "anyka_types.h"
#include "burner.h"
#include "spiburn.h"

#define  FILE_READ_BUF_SIZE     (256)//must larger than nand_page_size

typedef enum
{
    DATA_TYPE_BIN_WRITE = 0,        //bin data
    DATA_TYPE_BIN_CMP,              //bin data compare
    DATA_TYPE_BOOT_WRITE,           //boot data
    DATA_TYPE_BOOT_CMP,             //boot data compare
    DATA_TYPE_IMG_WRITE,            //img data
    DATA_TYPE_IMG_CMP,              //img data compare
    DATA_TYPE_UDISK_WRITE,          //udisk file data 
    DATA_TYPE_UDISK_CMP,            //udisk file data compare 
}E_BURN_DATA_TYPE;

static T_U32 OperateDataAssist(T_hFILE hFile, T_U32 length, E_BURN_DATA_TYPE dataType, T_BOOL data_cmp, E_CHIP_TYPE chip_type);
static T_U32 WriteBootDataFirstBuffer(T_hFILE hFile, T_U8* pBuf, T_U32 BufLen, T_U32 dwSize, E_CHIP_TYPE chip_type);
static T_VOID config_spiboot_snowbird(T_U8 *sflashboot, T_U32 dwSize, E_CHIP_TYPE chip_type);
static T_VOID  ConfigSpibootParam(T_U8 *buf, E_CHIP_TYPE ChipType);
extern T_SFLASH_PARAM gSPIFlash_base;


#pragma arm section code = "_update_"

static T_VOID config_spiboot_snowbird(T_U8 *sflashboot, T_U32 dwSize, E_CHIP_TYPE chip_type)
{
	if(AK_NULL == sflashboot)
	{
		return;
	}

	*(T_U8 *)(sflashboot + 0x25) = 'N';	
	*(T_U8 *)(sflashboot + 0x26) = 'N';
	*(T_U8 *)(sflashboot + 0x27) = 'C';
	*(T_U32 *)(sflashboot + 0x28) = dwSize;  //data size
	*(T_U32 *)(sflashboot + 0x30) = 0x800000; 
}

static T_VOID  ConfigSpibootParam(T_U8 *buf, E_CHIP_TYPE ChipType)
{
	T_U32 offset = 0;
	T_U32 i;

	for (i=0; i<FILE_READ_BUF_SIZE-sizeof(T_SFLASH_PARAM); i++)
	{
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3]))
		{
			offset = i;
			break;
		}
	}
	
	#if 0
	akerror("ConfigSpibootParam: ",i,1);
	{
	akerror("clock: ", gSPIFlash_base.clock, 1);
	akerror("erase_size: ", gSPIFlash_base.erase_size, 1);
	akerror("flag: ", gSPIFlash_base.flag, 1);
	akerror("id: ", gSPIFlash_base.id, 1);
	akerror("page_size: ", gSPIFlash_base.page_size, 1);
	akerror("program_size: ", gSPIFlash_base.program_size, 1);
	akerror("protect_mask: ", gSPIFlash_base.protect_mask, 1);
	akerror("total_size: ", gSPIFlash_base.total_size, 1);		
	}
	#endif
	
	if (i < FILE_READ_BUF_SIZE-sizeof(T_SFLASH_PARAM))
	{
		gpf.fMem.MemCpy(buf + offset + 4, &gSPIFlash_base, sizeof(T_SFLASH_PARAM));
	}
}

static T_U32 WriteBootDataFirstBuffer(T_hFILE hFile, T_U8* pBuf, T_U32 BufLen, T_U32 dwSize, E_CHIP_TYPE chip_type)
{
    T_S32 read_len;
    T_U32 read_to_len;

	if (BufLen < 32)
	{
        gpf.fDriver.Printf("SPI_UPDATE_LIB: BufLen(%d) < 32 \r\n", BufLen);
        return AK_FALSE;
	}
	
    read_to_len = BufLen;
    read_len    = gpf.fFs.FileRead(hFile, pBuf, read_to_len);         

    if(read_len!=read_to_len)
    {
        gpf.fDriver.Printf("SPI_UPDATE_LIB: read boot file fail:%d,%d\r\n", read_to_len, read_len);
        return AK_FALSE;
    }
	
	config_spiboot_snowbird(pBuf, dwSize, chip_type);

	ConfigSpibootParam(pBuf, chip_type);
	
    return AK_TRUE;

}

//loop to write or compare data buffer
static T_U32 OperateDataAssist(T_hFILE hFile, T_U32 length, E_BURN_DATA_TYPE dataType, T_BOOL data_cmp, E_CHIP_TYPE chip_type)
{
    T_S32 read_len;
    T_U32 i;
    T_BOOL bFirst = AK_TRUE;
    T_U8* pTmpBuffer = AK_NULL;
    T_U8* pTmpBuffer_cmp = AK_NULL;
    T_U32 ret;
    T_U32 read_to_len;
    T_U32 loop;

    if(0 == length)
    {
        return AK_FALSE;
    }

	pTmpBuffer = gpf.pComBuf;
	pTmpBuffer_cmp =pTmpBuffer + FILE_READ_BUF_SIZE;

    gpf.fMem.MemSet(pTmpBuffer, 0, FILE_READ_BUF_SIZE);

    loop = (length - 1) / FILE_READ_BUF_SIZE + 1;
    
    for(i=0; i<loop; i++)
    {
        if(i < (loop -1))
        {
            read_to_len = FILE_READ_BUF_SIZE;
        }
        else
        {
            read_to_len = length - i * FILE_READ_BUF_SIZE;
        }
        
        //Must to config boot param if write boot data firstly
        if(((DATA_TYPE_BOOT_WRITE == dataType) ||(DATA_TYPE_BOOT_CMP == dataType))
            && bFirst)
        {
            ret = WriteBootDataFirstBuffer(hFile, pTmpBuffer, read_to_len, length, chip_type);

            if(!ret)
            {
                return ret;
            }
                        
            bFirst = AK_FALSE;
        }
        else
        {
            read_len =  gpf.fFs.FileRead(hFile, pTmpBuffer, read_to_len);    

            if(read_len!=read_to_len)
            {
                gpf.fDriver.Printf("PL_BR read file fail\r\n");
                return AK_FALSE;
            }
        }

		if (!Sflash_BurnWriteData(pTmpBuffer, read_to_len))
        {
			return AK_FALSE;
        }

		if (data_cmp)
		{
			if (!Sflash_BurnCompareData(pTmpBuffer, pTmpBuffer_cmp, read_to_len))
			{
				return AK_FALSE;
			}
		}
    }
     
    return AK_TRUE;
}

#pragma arm section code
#pragma arm section code = "_update_"

T_U32 Burner_WriteSpiBinFile(T_hFILE hFile, T_BIN_PARAM*  binParam, E_CHIP_TYPE chip_type)
{
    T_BIN_INFO binInfo;

    if(FS_INVALID_HANDLE == hFile || AK_NULL == binParam)
    {
        return AK_FALSE;
    }
  
    binInfo.bBackup     = AK_FALSE;   //default backup
    binInfo.data_length = binParam->data_length;
    binInfo.ld_addr     = binParam->ld_addr;
    gpf.fMem.MemCpy(&binInfo.file_name, binParam->file_name, 15);

    //start bin burn
    if (!Sflash_BurnBinStart(&binInfo))
    {
		 return AK_FALSE;
    }
	

    //write bin file data
    if (!OperateDataAssist(hFile, binInfo.data_length, DATA_TYPE_BIN_WRITE, binParam->bCompare, chip_type))
    {
		return AK_FALSE;
    }

    return AK_TRUE;
}

T_U32 Burner_WriteSpiBootFile(T_hFILE hFile, T_U32 dataLen, T_BOOL bCompare, T_BOOT_PARAM* pBootParam, E_CHIP_TYPE chip_type)
{
    T_U32 data_real_len;
        
    if(FS_INVALID_HANDLE == hFile || AK_NULL == pBootParam)
    {
        return AK_FALSE;
    }

    gpf.fDriver.Printf("PR_BR write boot\r\n");
    
    data_real_len = dataLen;

    if (!Sflash_BurnBootStart(data_real_len))
    {
		return AK_FALSE;
    }
	
    if (!OperateDataAssist(hFile, data_real_len, DATA_TYPE_BOOT_WRITE, bCompare, chip_type))
    {
		return AK_FALSE;
    }

    return AK_TRUE;
}
#pragma arm section code






