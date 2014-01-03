/************************************************************************************
* Copyright(c) 2006 Anyka.com
* All rights reserved.
*
* File	:	mmem.h
* Brief :	heap memory allocator for mini system based on MMU
*           
*			1. 内存以16对齐
*			2. 堆大小不大于1.5M
*			3. 总内存个数不大于503个
* 
* Author  : 
* Modify  : 
* Data    : 
*************************************************************************************/
#ifndef __MEM_API_H__
#define __MEM_API_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "anyka_types.h"

#define MEMLIBVERSION "V0.0.7"

#ifndef DEBUG_MEM_BITMAP
	//#define DEBUG_MEM_BITMAP //初始化[0xCF]/分配[0xCC]/释放[0xFF]/失效[0xFC]/锁存[0xEC]/解锁[0xCE]内存时自动设定值
#endif

#define PAGE_SZ			4096// 1 << PAGE_ALIGN
#define BLOCK_SZ		16  // 1 << BLOCK_ALIGN
#define PAGE_ALIGN		12// 4096 = 1 << 12
#define BLOCK_ALIGN		4 //16 = 1 << 4
#define PAGE_BLOCKSZ	256 //PAGE_SZ>>BLOCK_ALIGN
#define ALIGN_By_BASE(var, base)	(((T_U32)(var)+(base)-1) &~ ((base)-1)) //Note base = 2^n

/*
 * NOTE : can't modify random
*/
#define MAX_BUBBLE_CNT			503
#define MAX_BUBBLE_EXTRA_CNT	7

//#define MMEM_HEAP_ADDR			gs_RAMBuffer //aligned by 4K
//#define MMEM_HEAP_SIZE			1310720//1572864	  //aligned by 16Byte


typedef union tagBUBInfo
{
	struct
	{
		unsigned short	bUsing : 1;//tag memory block is used or not
		unsigned short  bLarge : 1;//tag memory block size is larger than 256K or not
		unsigned short	sz     : 14;//large memory<size=2^10, id:4>; small memory<size=2^14>
	}bit;

	unsigned short		val;
}BUBInfo;

typedef struct  
{
	unsigned long  use : 22;	//amount of using memory total size
	unsigned long  cnt : 10;	//amount of using memory total number
	unsigned long  last_addr;    //bubble offset
	unsigned short last_alloc;   //bubble id

	unsigned char  extra_bitmap; //large memory bitmap
	unsigned char  extra[MAX_BUBBLE_EXTRA_CNT];//aux to record large memory block size 
	
	BUBInfo		   small[MAX_BUBBLE_CNT];//memory block 

}GMEMInfo;//1024Byte

typedef struct  
{
	GMEMInfo	mem_info;
	T_U32		mmem_heap_addr;	//aligned by 4K
	T_U32		mmem_heap_size;	//aligned by 16Byte

}GMEMSTRUCT;


typedef void (*_fMMU_InvalidatePage)(unsigned long addrPage, unsigned long szPage);
typedef char (*_fMMU_IsValidPage)(unsigned long addr);
typedef T_BOOL (*_fMMU_IsReservedPage)(unsigned long addr);
typedef char (*_fMMU_AssignResvPage)(unsigned long addr, unsigned long size, char bLock);
typedef void (*_fAkDebugOutput)(T_pCSTR s, ...);



typedef struct  
{
	GMEMSTRUCT				*pMemStruct;
	_fMMU_InvalidatePage 	MMU_InvalidatePage;
	_fMMU_IsValidPage		MMU_IsValidPage;
	_fMMU_IsReservedPage	MMU_IsReservedPage;
	_fMMU_AssignResvPage	MMU_AssignResvPage;
	_fAkDebugOutput			AkDebugOutput;
}GMEMCB;


typedef T_BOOL (*_fRam_Initial)(const GMEMCB *pMemCb, T_U8 *addr, T_U32 size);
typedef T_VOID (*_fRam_Info)(const GMEMCB *pMemCb, unsigned short *cntUsed, unsigned long *szUsed);
typedef T_pVOID (*_fRam_Alloc)(const GMEMCB *pMemCb, T_U32 size);
typedef T_pVOID (*_fRam_Free)(const GMEMCB *pMemCb, T_pVOID var);
typedef T_pVOID (*_fRam_AllocEx)(const GMEMCB *pMemCb, T_U32 size);
typedef T_pVOID (*_fRam_FreeEx)(const GMEMCB *pMemCb, T_pVOID var);
typedef T_U32 (*_fRam_GetMaxAllocSize)(const GMEMCB *pMemCb);
typedef T_BOOL (*_fRam_DbgEnum)(const GMEMCB *pMemCb, T_U16 LLD, T_BOOL bQueryStatus);
typedef T_pSTR (*_fRam_GetVersion)(T_VOID);


T_BOOL	Ram_Initial_Rom(const GMEMCB *pMemCb, T_U8 *addr, T_U32 size);
T_VOID	Ram_Info_Rom(const GMEMCB *pMemCb, unsigned short *cntUsed, unsigned long *szUsed);

//high freq func
T_pVOID	Ram_Alloc_Rom(const GMEMCB *pMemCb, T_U32 size);
T_pVOID Ram_Free_Rom(const GMEMCB *pMemCb, T_pVOID var);

//dma memory
T_pVOID Ram_AllocEx_Rom(const GMEMCB *pMemCb, T_U32 size);
T_pVOID Ram_FreeEx_Rom(const GMEMCB *pMemCb, T_pVOID var);

T_U32 Ram_GetMaxAllocSize_Rom(const GMEMCB *pMemCb);
#ifdef DEBUG_MEM_BITMAP
extern T_BOOL Ram_DbgEnum_Rom(const GMEMCB *pMemCb, T_U16 LLD, T_BOOL bQueryStatus);
#endif

/**
 * @brief call this function to get version str
 * @param[in] void
 * @return T_pSTR
 */
T_pSTR Ram_GetVersion_Rom(T_VOID);

#ifdef __cplusplus
}
#endif


#endif

