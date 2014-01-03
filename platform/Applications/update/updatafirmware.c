#include "Apl_Public.h"
#include "Gbl_Global.h"
//#include "Fat.h"
//#include "eng_debug.h" 
#include "updateself.h"
#include "fwl_spiflash.h" 
#include "gpio_define.h" 
#include "fwl_osfs.h" 
#include "file.h"
#include "Fwl_osMalloc.h"
#include "VoiceMsg.h"
#include "Eng_VoiceTip.h"
#include "fwl_system.h"

typedef struct BinInfo
{
    T_U32   data_length;
    T_U32   ld_addr;
    T_BOOL  bBackup;
    T_U8    file_name[16];
}T_BIN_INFO;

typedef struct
{
    T_U8  Disk_Name;                                //盘符名
    T_U8  bOpenZone;                                //
    T_U8  ProtectType;                        //        
    T_U8  ZoneType;                                //
    T_U32 Size;
    T_U32 resv[4];
}T_PARTITION_INFO;


typedef T_pVOID (*Prod_RamAlloc)(T_U32 size);
typedef T_pVOID (*Prod_RamFree)(T_pVOID var);
typedef T_pVOID (*Prod_MemSet)(T_pVOID pBuf, T_S32 value, T_U32 count);
typedef T_pVOID (*Prod_MemCpy)(T_pVOID dst, T_pVOID src, T_U32 count);
typedef T_S32   (*Prod_MemCmp)(T_pVOID pbuf1, T_pVOID pbuf2, T_U32 count);
typedef T_VOID (*Prod_Printf)(T_pCSTR s, ...);
typedef T_U32   (*FS_FILE_Read)(T_hFILE hFile, T_pVOID buffer, T_U32 count);
typedef T_S32   (*FS_FILE_Seek)(T_hFILE hFile, T_S32 offset, T_U16 origin);

extern T_BOOL UnpacketSpiFileInit(T_SPIBURNFUNC* fSpi, T_hFILE hFile, T_SFLASH_PARAM *spi_info);
extern T_BOOL UnpacketSpiBootFile(T_hFILE hFile, T_BOOL bCompare);
extern T_BOOL UnpacketSpiBinFile(T_hFILE hFile);
extern T_U8 Fwl_SD1_Read(T_U32 src, T_U8 *databuf, T_U32 size);

//#pragma arm section code = "_bootcode1_" 
#pragma arm section code = "_update_"

char *ak_memcpy(char *dest, const char *src, unsigned int n)
{
	unsigned int i;
	if((AK_NULL==dest)||(AK_NULL==src))
		return AK_NULL;
	for(i=0;i<n;i++)
		{
			dest[i]=src[i];
		}
	return dest;
}

char *ak_memset(char *dest, char ch, unsigned int n)
{
	unsigned int i;
	if(AK_NULL==dest)
		return AK_NULL;
	for(i=0;i<n;i++)
		{
			dest[i]=ch;
		}
	return dest;
}



int ak_memcmp(const char *s1, const char *s2, unsigned int n)
{
	unsigned int i;
	for(i=0;i<n;i++)
		{
			if(s1[i]>s2[i])
				return 1;
			else if(s1[i]<s2[i])
				return -1;			
		}
	return 0;	
}

#pragma arm section code

//#pragma arm section code = "_bootbss_" 

static T_U32 pos=0;
static T_U32 *pFilePosInfo=0;			//sector 地址
static T_U32 ClusterNum=0;				//簇个数
static T_U32 SecPerClus=0;				//每个簇包含的sector个数
static T_U32 filelen=0;

T_U8 *g_pSpiStack   = AK_NULL;
T_U32 g_SpiStackPtr = 0;
//#pragma arm section code



//#pragma arm section code = "_bootcode1_" 
#pragma arm section code = "_update_"

T_U32 upd_fileseek(T_hFILE hFile, T_S32 offset, T_U16 origin)
{	
	 switch (origin)
    {
        case FS_SEEK_SET:
        {
            pos = offset;
        }
        break;

        case FS_SEEK_CUR:
        {
            pos += offset;
        }
        break;

        case FS_SEEK_END:
        {
			pos = filelen;
			pos += offset;//在尾部offset肯定是小于0的
            
        }
        break;

        default:
        {
            pos = offset;
        }
        break;
	 }
	 return pos;
	
}


T_U32 upd_fileread(T_hFILE hFile, T_pVOID buffer, T_U32 count)
{
	T_U32 i;
    T_U8 buf[512];        /*因为不知道簇多大所有定义512*/
	T_U32 total_copy_cnt = 0;   /*用于记载拷贝的数量*/
	T_U32 cluster_index = 0;  /*获取落在那个cluster中*/
	T_U32 SecSize = 512;      /*SD的扇区都是512*/
	T_U32 ClusSize = SecPerClus * SecSize;     /*SD的簇的大小*/
	T_U32 SecOffsetInCluster;/*获取扇区在cluster中偏移*/
	T_U32 ByteOffsetInCluster;/*获取字节在cluster中偏移*/
	T_U32 ByteOffsetInSec;    /*获取字节在sector中偏移*/
	T_U32 copy_size;
	T_U32 can_copy_size_in_buf;
	T_U32 need_copy_size = count;

    //AK_DEBUG_OUTPUT("pos: %d, cluster_index:%d, need_copy_size:%d\r\n", pos, total_copy_cnt, need_copy_size);

	while(total_copy_cnt < need_copy_size)
	{
		cluster_index       = pos / ClusSize;
		ByteOffsetInCluster = pos % ClusSize;
		SecOffsetInCluster  = ByteOffsetInCluster / SecSize;
		ByteOffsetInSec     = ByteOffsetInCluster % SecSize;

		copy_size = ClusSize - ByteOffsetInCluster;
		//AK_DEBUG_OUTPUT("pos: %d, cluster_index:%d, SecOffsetInCluster:%d\r\n", pos, cluster_index, SecOffsetInCluster);
		for (i=SecOffsetInCluster; i<SecPerClus; i++)
		{
			/*读写的单位是sector的*/
			//sd_read(pFilePosInfo[cluster_index] + i,buf, 1);//10blue单T卡
            Fwl_SD1_Read(pFilePosInfo[cluster_index] + i, buf, 1);//11blue双T卡
			//AK_DEBUG_OUTPUT("sd_read sec: %x\r\n", pFilePosInfo[cluster_index] + i);
			//AK_DEBUG_OUTPUT("%02x, %02x, %02x, %02x, \r\n", buf[0], buf[1], buf[2], buf[3]);
			if (ByteOffsetInSec != 0)
			{
				can_copy_size_in_buf = SecSize - ByteOffsetInSec;
			}
			else
			{
				can_copy_size_in_buf = SecSize;
			}
			
			if (need_copy_size > can_copy_size_in_buf)
			{
				copy_size = can_copy_size_in_buf;
			}
			else
			{
				/*因为can_copy_size_in_buf最大是一个can_copy_size_in_buf,
				所以need_copy_size都不足够一个SecSize的大小了*/
				copy_size = need_copy_size;
			}

//			AK_DEBUG_OUTPUT("total_copy_cnt: %d, copy_size:%d\r\n", total_copy_cnt, copy_size);
			ak_memcpy((char*)buffer + total_copy_cnt, &buf[ByteOffsetInSec], copy_size);
			ByteOffsetInSec = 0;//后面都修正成这个扇区的开始的
			
			pos += copy_size;
			total_copy_cnt += copy_size;
			need_copy_size -= copy_size;
			if (total_copy_cnt == count)
			{
				//已经拷贝完毕
				return count;
			}

		}
	}	

	return count;
}






//-----------------------------------------------------------------------



#define SPI_UPDATE_SELF_BUF_SIZE 2048

T_pVOID SPI_RamAlloc(T_U32 size);
T_pVOID SPI_RamFree(T_pVOID var);

T_pVOID SPI_RamAlloc(T_U32 size)
{
    T_pVOID RetPtr = AK_NULL;
    
    if (AK_NULL == g_pSpiStack ||
        0 == size 
        || ((g_SpiStackPtr + size) > SPI_UPDATE_SELF_BUF_SIZE))
    {
        return AK_NULL;
    }
    
    RetPtr = (T_pVOID)(g_pSpiStack + g_SpiStackPtr);
    
    g_SpiStackPtr += size;
    
    return RetPtr;
}

T_pVOID SPI_RamFree(T_pVOID var)
{
    return AK_NULL;
}

T_BOOL anyka_spi_update_self(T_BOOL DelUpdateFlag)
{
  //T_U32 *pFilePosInfo = AK_NULL;
  //T_U32 ClusterNum = 0; 
  //T_U32 SecPerClus = 0;
    T_SPIBURNFUNC spi_BurnFunc;    
	T_U32* pdateGetInfo;
	T_U32 file = AK_NULL;	 
	const T_U16 path[]={'A',':','/','u','p','d','a','t','e','.','U','P','D',0};
	T_SFLASH_PARAM *pParam;

	pParam = (T_SFLASH_PARAM*)(0x00800040);

    AK_DEBUG_OUTPUT("test_spi_update_self start\n");
    
	file = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);
    if (!File_Exist(file))
    {
        /* Whatever mode you open the file, it fails when it doesn't exist. */
		AK_DEBUG_OUTPUT("test_spi_update_self: open upd file error\n");
        File_Close(file);
        return AK_FALSE;
    }
    else
    {
        #ifdef SUPPORT_VOICE_TIP
            Voice_PlayTip(eBTPLY_SOUND_TYPE_UPDATE, AK_NULL);
            Voice_WaitTip();
        #endif
    }
    //gpio_set_pin_dir(GPIO_POWER_CTRL, 1);
    //gpio_set_pin_level(GPIO_POWER_CTRL, 1);
        
    if (AK_NULL == g_pSpiStack)
    {
        g_SpiStackPtr = 0;		
        g_pSpiStack = (T_U8 *)Fwl_DMAMalloc(SPI_UPDATE_SELF_BUF_SIZE);//徐申实现 平台的molloc
        if(g_pSpiStack==AK_NULL)
        	{
        		akerror("test_spi_update_self: malloc fail",SPI_UPDATE_SELF_BUF_SIZE,1);
				return AK_FALSE;
        	}
    }  

	filelen = File_GetLength(file, AK_NULL);
    pdateGetInfo = File_GetfileclusterInfo_ToUpdate(file,&ClusterNum, &SecPerClus);

	/*因为簇链已经得到，可以删除，文件删除不会干掉真正的内容的*/
#if 1	
	if(DelUpdateFlag && (!Fwl_FileDeleteHandle(file)))
	{
        akerror("test_spi_update_self: File_DeleteFile fail",0,1);
		Fwl_DMAFree(g_pSpiStack);
		File_Close(file);
		return AK_FALSE;	
	}
#endif

	File_Close(file);
	
    if (AK_NULL == pdateGetInfo)
    {
        //fwl_closefile关闭文件句柄
        //File_Close(file);
        akerror("test_spi_update_self: get pdateGetInfo fail",0,1);
		Fwl_DMAFree(g_pSpiStack);
		return AK_FALSE;
    }
    pFilePosInfo = pdateGetInfo;
	
    spi_BurnFunc.RamAlloc = SPI_RamAlloc;
    spi_BurnFunc.RamFree  = SPI_RamFree;
    spi_BurnFunc.MemSet   = ak_memset;
    spi_BurnFunc.MemCpy   = ak_memcpy;
    spi_BurnFunc.MemCmp   = ak_memcmp;
    spi_BurnFunc.Printf   = AkDebugOutput;//AK_DEBUG_OUTPUT;//
    spi_BurnFunc.fSeek    =upd_fileseek;///Fwl_FileSeek;//
    spi_BurnFunc.fRead    =upd_fileread;///Fwl_FileRead;//
	
	{
		AK_DEBUG_OUTPUT("clock: 0x%x \r\n", pParam->clock);
		AK_DEBUG_OUTPUT("erase_size: 0x%x \r\n", pParam->erase_size);
		AK_DEBUG_OUTPUT("flag: 0x%x \r\n", pParam->flag);
		AK_DEBUG_OUTPUT("id: 0x%04x \r\n", pParam->id);
		AK_DEBUG_OUTPUT("page_size: 0x%x \r\n", pParam->page_size);
		AK_DEBUG_OUTPUT("program_size: 0x%x \r\n", pParam->program_size);
		AK_DEBUG_OUTPUT("protect_mask: 0x%x \r\n", pParam->protect_mask);
		AK_DEBUG_OUTPUT("total_size: 0x%x \r\n", pParam->total_size);
	}
	
    if (!UnpacketSpiFileInit(&spi_BurnFunc, file, pParam))
    {
        //fwl_closefile关闭文件句柄
		akerror("test_spi_update_self: UnpacketSpiFileInit fail",0,1);         
		Fwl_DMAFree(g_pSpiStack);
        return AK_FALSE;
    }
    if (!UnpacketSpiBinFile(file))
    {
        //fwl_closefile关闭文件句柄
      //   File_Close(file);
        akerror("UnpacketSpiFileInit: UnpacketSpiBinFile fail",0,1);
		Fwl_DMAFree(g_pSpiStack);   
        return AK_FALSE; 
    }
    //后面的是重新读取数据比较是否正确, 需求很大的内存8k，所以不比较
    if (!UnpacketSpiBootFile(file, AK_TRUE))
    {
        //fwl_closefile关闭文件句柄
     //    File_Close(file);    
        akerror("UnpacketSpiFileInit: UnpacketSpiBootFile fail",0,1);
		Fwl_DMAFree(g_pSpiStack);
        return AK_FALSE;
    }
    
    //不需要释放了，重新启动会重新初始化内存的
    {
        akerror("UnpacketSpiFileInit: SPI update successed!",0,1);
	}   
	/*测试中sys_reset回进入massboot，所以目前进入sys_reset();*/
	Fwl_SysReboot();
	while(1);
    return AK_TRUE;
}
#pragma arm section code

