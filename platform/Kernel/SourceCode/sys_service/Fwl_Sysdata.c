#include "Prog_manager.h"
#include "Eng_debug.h"
#include "Gbl_global.h"
#include "Fwl_osMalloc.h"

#if (STORAGE_USED == SPI_FLASH)

#include "Fwl_spiflash.h"

T_U8 *SysDataBuf = AK_NULL;
T_U8 SysDataOpenTime = 0;

/******************************************************************************
* @NAME    Fwl_Sysdata_Open 
* @BRIEF   Open System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM   name: System File Name
*          bAlign: Page Alignment  
*          
* @RETURN  < 0 Is Failure; Others Is System File Handle
*   
*******************************************************************************/
T_S32 Fwl_Sysdata_Open(T_S8 *name, T_BOOL bAlign)
{
#ifdef OS_ANYKA
	T_U16 startPage = 0;
    
    if (0 == progmanage_get_maplist(name, &startPage, 10))    //获取资源文件起始page
    {
    	AK_DEBUG_OUTPUT("get startPage Error\n");
        return -1;
    }
	
	if(AK_NULL == SysDataBuf)
	{
		SysDataBuf = Fwl_Malloc(SPIFLASH_PAGE_SIZE);
	}
	SysDataOpenTime ++;

    if (bAlign)  //page是否需要以block对齐
        return ALIGN(startPage, SPIFLASH_PAGE_PER_BLOCK);
	return startPage;
#else
	return Fwl_FileOpen(name, _FMODE_READ, _FMODE_READ);
#endif
}

/******************************************************************************
* @NAME    Fwl_Sysdata_Close 
* @BRIEF   Close System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM   name: System File Handle
*          
* @RETURN  AK_FALSE Is Failure; AK_TRUE Is Success
*   
*******************************************************************************/
T_BOOL Fwl_Sysdata_Close(T_S32 handle)
{
#ifdef OS_WIN32
	return Fwl_FileClose(handle);
#endif
	SysDataOpenTime --;
	if(0 == SysDataOpenTime)
	{
		Fwl_Free(SysDataBuf);
		SysDataBuf = AK_NULL;
	}

	return AK_TRUE;
}


/******************************************************************************
* @NAME    Fwl_Sysdata_Read 
* @BRIEF   Read System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM handle: System File Handle
*		offset:	Offset Related Start page
*		len:		Read Data Length	
*		pBuf:	Read Data From  Storage Medium
*          
* @RETURN  0 Is Failure; Others Is Read Data Length
*   
*******************************************************************************/
T_U32 Fwl_Sysdata_Read(T_S32 handle, T_U32 offset, T_U32 len, T_U8 *pBuf)
{
#ifdef OS_ANYKA
	T_U32 pageCnt;
	T_U32 misAlignBytes;
	T_U32 i;
	T_U8 *pOutBuf = pBuf;

	if (handle < 0)
		return 0;
	handle += ALIGN_DOWN(offset, SPIFLASH_PAGE_SIZE) / SPIFLASH_PAGE_SIZE;    //偏移位置所在的page
    pageCnt = (ALIGN(offset+len, SPIFLASH_PAGE_SIZE) - ALIGN_DOWN(offset, SPIFLASH_PAGE_SIZE)) / SPIFLASH_PAGE_SIZE;    //需要读取的page数

    for (i = 0; i < pageCnt; i++)
    {      
        if (!Fwl_spiflash_read(handle+i, SysDataBuf, 1))
        {
        	AK_DEBUG_OUTPUT("READ SPI Error\n");
            return 0;
        }      
        
        if (0 == i)  //首页
        {
            //需要读取的偏移位置不对齐的字节数
            misAlignBytes = SPIFLASH_PAGE_SIZE - offset % SPIFLASH_PAGE_SIZE;    
            if (misAlignBytes > len)   //如果不对齐字节数大于要读取的长度
            {
                misAlignBytes = len;
            }
			
            memcpy(pOutBuf, SysDataBuf + offset%SPIFLASH_PAGE_SIZE, misAlignBytes);
            pOutBuf += misAlignBytes;
        }
        else if ((pageCnt - 1) == i) //末页
        {
            memcpy(pOutBuf, SysDataBuf, pBuf + len - pOutBuf);
            pOutBuf = pBuf + len;
        }
        else
        {   
            memcpy(pOutBuf, SysDataBuf, SPIFLASH_PAGE_SIZE);
            pOutBuf += SPIFLASH_PAGE_SIZE;
        }
    }
	

    return len;
#else
 	Fwl_FileSeek(handle, offset, FS_SEEK_SET);
	return Fwl_FileRead(handle, pBuf, len);
#endif
}


/******************************************************************************
* @NAME    Fwl_Sysdata_Write 
* @BRIEF   Write Data to System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM handle: System File Handle
*		offset: Position of the Data Will be Writed(Must be 0).
* 		buff:	the Data Will Be Writed 
*		size:	Data Length
*          
* @RETURN  0 Is Failure; Others Is Writed Data Length
*   
*******************************************************************************/
T_U32 Fwl_Sysdata_Write(T_S32 handle, T_U32 offset, T_pVOID buff, T_U16 size)
{
    T_U32 userDataSize;

	// Must block Alignment
	if (offset > 0)
		return 0;
    userDataSize = ALIGN(size, SPIFLASH_PAGE_SIZE);   //按页对齐

    if (!Fwl_spiflash_erase(handle/SPIFLASH_PAGE_PER_BLOCK))   //擦除页
    {
        AK_DEBUG_OUTPUT("spi erase false:%d", handle/SPIFLASH_PAGE_PER_BLOCK);
        return 0;
    }
    
    if (!Fwl_spiflash_write(handle, buff, userDataSize/SPIFLASH_PAGE_SIZE))
    {  
        AK_DEBUG_OUTPUT("spi write false:%d", userDataSize/SPIFLASH_PAGE_SIZE);
        return 0;
    }

    return size;
}

#endif 	// End of #if (STORAGE_USED == SPI_FLASH)


