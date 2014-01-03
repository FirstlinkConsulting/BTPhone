/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name：Fwl_osMalloc.c
 * Function：This header file is API for Memory Library
 *
 * Author：ZhangMuJun
 * Date：
 * Version：2.0.1
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#include <string.h>
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "remap.h"
#include "akdefine.h"
#include "Fwl_nandflash.h"
#include "Eng_mmem.h"
#include "prog_manager.h"

//#define MALLOC_CHECK //用于调试内存泄漏的宏,只用于测非DMA MALLOC

#define MMEM_HEAP_SIZE      ((128-20)*1024)//600k  //aligned by 16Byte

#define DMA_MALLOC_UNIT     4096    ////申请的DMA内存需4K对齐
#define GARBAGE_DATA        0xC3                          //垃圾数据


#ifdef OS_ANYKA
#pragma arm section zidata = "_rambuf_"
T_U8 gb_RAMBuffer[MMEM_HEAP_SIZE];
#pragma arm section zidata
#else
T_U8 gb_RAMBuffer[MMEM_HEAP_SIZE+PAGE_SZ];
#endif

#pragma arm section zidata = "_memstruct_"
static GMEMSTRUCT mem_struct = {0};
#pragma arm section zidata
void MMU_InvalidatePage(unsigned long addrPage, unsigned long szPage);

char MMU_IsValidPage(unsigned long addr);

T_BOOL MMU_IsReservedPage(unsigned long addr);

extern char MMU_AssignResvPage(unsigned long addr, unsigned long size, char bLock);

const GMEMCB MemCb = 
{
	&mem_struct,
	MMU_InvalidatePage,
	MMU_IsValidPage,
	MMU_IsReservedPage,
	MMU_AssignResvPage,
	AkDebugOutput,
};

#ifdef OS_ANYKA
//extern T_NANDFLASH nf_info; //T_NANDFLASH:80B
//extern T_U8 *gs_nandbuffer; //for nand read-write buffer
#endif

#ifdef MALLOC_CHECK
typedef struct {
    T_U32 ptr;
    T_U32 size;
}T_MALLOC_RECORD;
typedef struct{
    T_MALLOC_RECORD *Rec;
    T_U32 count;
}T_REC_ARRAY;
#pragma arm section zidata = "_bootbss_"
T_REC_ARRAY malloc_rec;
#pragma arm section zidata
#endif
#define MALLOC_Ram_Initial_Rom_OFF	(0x4EB4)
#define MALLOC_Ram_Info_Rom_OFF	(0x5650)	
#define MALLOC_Ram_Alloc_Rom_OFF	(0x53FC)	
#define MALLOC_Ram_Free_Rom_OFF	(0x5648)	
#define MALLOC_Ram_AllocEx_Rom_OFF	(0x5684)	
#define MALLOC_Ram_FreeEx_Rom_OFF	(0x568C)	
#define MALLOC_Ram_GetMaxAllocSize_Rom_OFF	(0x5694)
#define MALLOC_Ram_GetVersion_Rom_OFF	(0x56FC)	

const T_U32 mem_rom_add[] ={MALLOC_Ram_Initial_Rom_OFF, //Ram_Initial_Rom
							MALLOC_Ram_Info_Rom_OFF, //Ram_Info_Rom
							MALLOC_Ram_Alloc_Rom_OFF, //Ram_Alloc_Rom
							MALLOC_Ram_Free_Rom_OFF, //Ram_Free_Rom
							MALLOC_Ram_AllocEx_Rom_OFF, //Ram_AllocEx_Rom
							MALLOC_Ram_FreeEx_Rom_OFF, //Ram_FreeEx_Rom
							MALLOC_Ram_GetMaxAllocSize_Rom_OFF, //Ram_GetMaxAllocSize_Rom
							MALLOC_Ram_GetVersion_Rom_OFF	//Ram_GetVersion_Rom
							}; 


T_BOOL	Ram_Initial(T_U8 *addr, T_U32 size)
{
	_fRam_Initial pFuncRom = (_fRam_Initial)mem_rom_add[0];//Ram_Initial_Rom;
	
	return pFuncRom(&MemCb, addr, size);
}

T_VOID  Ram_Info(unsigned short *cntUsed, unsigned long *szUsed)
{
	_fRam_Info pFuncRom = (_fRam_Info)mem_rom_add[1];//Ram_Info_Rom;

	pFuncRom(&MemCb, cntUsed, szUsed);
}

T_pVOID	Ram_Alloc(T_U32 size)
{
	_fRam_Alloc pFuncRom = (_fRam_Alloc)mem_rom_add[2];//Ram_Alloc_Rom;
	return pFuncRom(&MemCb, size);
}

T_pVOID Ram_Free(T_pVOID var)
{
	_fRam_Free pFuncRom = (_fRam_Free)mem_rom_add[3];//Ram_Free_Rom;

	return pFuncRom(&MemCb, var);
}

T_pVOID	Ram_AllocEx(T_U32 size)
{
	_fRam_Alloc pFuncRom = (_fRam_Alloc)mem_rom_add[4];//Ram_AllocEx_Rom;

	return pFuncRom(&MemCb, size);
}

T_pVOID Ram_FreeEx(T_pVOID var)
{
	_fRam_Free pFuncRom = (_fRam_Free)mem_rom_add[5];//Ram_FreeEx_Rom;

	return pFuncRom(&MemCb, var);
}


T_U32 Ram_GetMaxAllocSize(T_VOID)
{
	_fRam_GetMaxAllocSize pFuncRom = (_fRam_GetMaxAllocSize)mem_rom_add[6];//Ram_GetMaxAllocSize_Rom;

	return pFuncRom(&MemCb);
}

T_pSTR Ram_GetVersion(T_VOID)
{
	_fRam_GetVersion pFuncRom = (_fRam_GetVersion)mem_rom_add[7];//Ram_GetVersion_Rom;

	return pFuncRom();
}


#pragma arm section code = "_sysinit_"
T_VOID Fwl_MallocInit(T_VOID)
{
    T_U8 *p = AK_NULL;

#ifdef OS_ANYKA
    p = (T_U8 *)gb_RAMBuffer;
#else
    p = gb_RAMBuffer + PAGE_SZ - ((T_U32)gb_RAMBuffer & (PAGE_SZ-1));
#endif

    AK_DEBUG_OUTPUT("mem lib version: %s!\n", Ram_GetVersion());

    if (Ram_Initial(p, MMEM_HEAP_SIZE) == AK_FALSE)
    {
        AK_DEBUG_OUTPUT("Fwl_MallocInit failed!!\n");
        while(1);
    }
#ifdef OS_ANYKA    
    //alloc comm buffer
    #if (STORAGE_USED == NAND_FLASH)
    //gs_nandbuffer = Fwl_Malloc(Fwl_Nand_BytesPerSector(0));
    #endif
#endif    

#ifdef MALLOC_CHECK
    malloc_rec.count = 0;
    malloc_rec.Rec = (T_MALLOC_RECORD*)Fwl_Malloc(sizeof(T_MALLOC_RECORD)*501);
    if(malloc_rec.Rec == AK_NULL)
        AK_DEBUG_OUTPUT("Check Malloc Failed\n");
    else
        memset(malloc_rec.Rec,0,sizeof(T_MALLOC_RECORD)*501);
#endif
}
#pragma arm section code
#ifdef MALLOC_CHECK
T_BOOL AddNode2Record(T_U32 ptr,T_U32 size)
{
    T_U16 i;
    if((malloc_rec.Rec == AK_NULL)|| (malloc_rec.count >= 501))
    {
        AK_DEBUG_OUTPUT("Check Malloc Failed\n");
        return AK_FALSE;
    }
    for(i=0 ; i<501 ; i++)
    {
        if(malloc_rec.Rec[i].ptr == 0)
        {
            malloc_rec.Rec[i].ptr = ptr;
            malloc_rec.Rec[i].size = size;
            malloc_rec.count++;
            break;
        }
    }
}
T_VOID DelNodeFromRecord(T_U32 ptr)
{
    T_U16 i;
    if((malloc_rec.Rec == AK_NULL)|| (malloc_rec.count == 0))
    {
        return ;
    }
    for(i=0 ; i<501 ; i++)
    {
        if(malloc_rec.Rec[i].ptr == ptr)
        {
            malloc_rec.Rec[i].ptr = 0;
            malloc_rec.Rec[i].size = 0;
            malloc_rec.count--;
            break;
        }
    }
}
T_VOID PrintChekMaloc(T_VOID)
{
    T_U16 i;
    T_U16 tmp;
    T_U32 totalsize = 0;
    if(malloc_rec.count == 0)
        return;
    AK_DEBUG_OUTPUT("Check count:%d ########\n",malloc_rec.count);
    tmp = malloc_rec.count;
    for(i=0 ; i<501 ; i++)
    {
        if(malloc_rec.Rec[i].ptr != 0)
        {
            totalsize += malloc_rec.Rec[i].size;
            AK_DEBUG_OUTPUT("ptr:%x, size:%d\n",malloc_rec.Rec[i].ptr,malloc_rec.Rec[i].size);
            tmp--;
            if(tmp == 0)
                break;
        }
    }
    AK_DEBUG_OUTPUT("NoFree size:%d,use total size:%d\n",totalsize,(totalsize + sizeof(T_MALLOC_RECORD)*501));
}
T_pVOID TestRam_Free(T_pVOID var)
{
    DelNodeFromRecord((T_U32)var);
    return Ram_Free(var);
}
#endif
void MMU_InvalidatePage(unsigned long addrPage, unsigned long szPage)
{
#ifdef OS_ANYKA
    remap_unload_pages(addrPage,szPage, AK_TRUE);
    progmanage_free_mapinfo(addrPage, szPage);
#endif
}

char MMU_IsValidPage(unsigned long addr)
{
#ifdef OS_ANYKA
    return remap_isvalid_addr(addr);
#else
    return 1;
#endif
}

T_BOOL MMU_IsReservedPage(unsigned long addr)
{
#ifdef OS_ANYKA
    return remap_isreserved_page(addr);
#else
    return AK_FALSE;
#endif
}

extern char MMU_AssignResvPage(unsigned long addr, unsigned long size, char bLock)
{
#ifdef OS_ANYKA
    return remap_resv_page(addr, size, bLock);
#else
    return 1;
#endif
}

T_pVOID Fwl_DMAMalloc(T_U32 size)
{
    T_U32 realSize;
    T_U32 value;

    value = size % DMA_MALLOC_UNIT;

    if ( value == 0)
    {
        realSize = size;
    }
    else
    {
        realSize = size - value + DMA_MALLOC_UNIT;
    }

    //AK_DEBUG_OUTPUT("\nsize=%d, realsize=%d\n", size,realSize);

    return Ram_AllocEx(realSize);
}


T_pVOID Fwl_DMAFree(T_pVOID var)
{
    return Ram_FreeEx(var);
}

T_U32 Fwl_GetMaxAllocSize(T_VOID)
{
    return Ram_GetMaxAllocSize();
}
#pragma arm section rwdata = "_cachedata_"
T_U8 gSysRsvMalloc[512] = {1};
static volatile T_BOOL SysMallocFlag = AK_FALSE;
#pragma arm section rwdata

#pragma arm section code = "_bootcode1_"
T_pVOID Fwl_SysRsvMalloc(T_VOID)
{
	return (T_pVOID)gSysRsvMalloc;
}
T_BOOL Fwl_SysMallocFlag(T_VOID)
{
	return SysMallocFlag;
}
#pragma arm section code

#pragma arm section code = "_frequentcode_"
T_pVOID Fwl_Malloc(unsigned long sz)
{
    T_pVOID ptr;

	if(SysMallocFlag)
	{
		AkDebugOutput("*******Fwl_Malloc:the ram lib error at system call");
	}
	SysMallocFlag = AK_TRUE;
#ifdef DEBUG
    ptr = Ram_Alloc(sz);
    
    if(AK_NULL == ptr)
    {
        unsigned short cnt;
        unsigned long  used;
        Ram_Info(&cnt, &used);
        AK_DEBUG_OUTPUT("\n alloc null sz=%ld cnt=%d used=%ld\r\n", sz, cnt, used);
    }
    else
    {
#ifdef MALLOC_CHECK
        akerror("Malloc ptr:",ptr,1);
        AddNode2Record(ptr,sz);
#endif
        memset(ptr, GARBAGE_DATA,sz);
    }
#else
	ptr = Ram_Alloc(sz);
#endif
    SysMallocFlag = AK_FALSE;
    return ptr;
}

T_pVOID Fwl_Free(T_pVOID var)
{	
	if(SysMallocFlag)
	{
		AkDebugOutput("*******Fwl_Free:the ram lib error at system call");
	}
    SysMallocFlag = AK_TRUE;
#ifdef MALLOC_CHECK
     TestRam_Free(var);
#else
     Ram_Free(var);
#endif
    SysMallocFlag = AK_FALSE;
	return AK_NULL;
}

T_U32 Fwl_GetUsedMem(T_VOID)
{
    T_U32 cnt, sz;
    
    Ram_Info(&cnt, &sz);
    return sz;
}

#pragma arm section code


