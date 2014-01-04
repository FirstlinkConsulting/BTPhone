/************************************************************************
 * Copyright (c) 2011, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name：Eng_loadMem.c
 * Function：This source file is API for load Memory 
 *
 * Author： liangxiong
 * Date：
 * Version：0.0.1
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#include "Eng_loadMem.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"

typedef struct{
	T_U8* header;		 // 加载内存的起始地址
	T_U16 size;          // 加载内存的长度
	T_U16 commBlocks:4;   // bit0-bit3:记录用作公共缓存的512B块数目
	T_U16 memType:1;      // bit4:内存使用类型。1: 系统缓存,0:vbuf缓存
	T_U16 mallocFlag:1;   // bit5:内存申请情况。1: 已申请,0:没申请
	T_U16 commUse:1;      // bit6:公共内存使用。1: 已使用,0:没使用
	T_U16 imageUse:1;     // bit7:图片加载内存使用。1: 已使用,0:没使用
}T_LOADMEM_CONTROL;

#define LOADMEM_MEMTYPE_VBUFF      0
#define LOADMEM_MEMTYPE_SYSTEM     1

#define LOADMEM_MALLOC_NOUGHT      0
#define LOADMEM_MALLOC_DONE        1

#define LOADMEM_COMMEM_NOUSE       0
#define LOADMEM_COMMEM_USED        1

#define LOADMEM_IMAGEMEM_NOUSE      0
#define LOADMEM_IMAGEMEM_USED       1

#define LOADMEM_VBUFF_DISABLE       0
#define LOADMEM_VBUFF_ENABLE        1

#define LOADMEM_ALLSIZE 		4096//4 * 1024   //内存最大长度
#define LOADMEM_COMMONSIZE 		2048//2 * 1024

#define LoadMem_GetMod(var)  ((var) & (0x01ff))  //mode by 512
#define LoadMem_DIV(var)     ((var) >> 9)        //整除512
#define LoadMem_MUL(var)     ((var) << 9)        //512倍数

//#if(NO_DISPLAY == 0)
//内存管理变量
#ifdef OS_ANYKA
#pragma arm section zidata = "_bootbss_"
#endif
T_LOADMEM_CONTROL   g_loadmem;
#ifdef OS_ANYKA
#pragma arm section zidata
#endif


//初始化加载内存管理
T_VOID LoadMem_Init(T_VOID)
{
	g_loadmem.header = AK_NULL;
	g_loadmem.size = 0;
	g_loadmem.commBlocks = 0;
	g_loadmem.memType = LOADMEM_MEMTYPE_VBUFF;
	g_loadmem.mallocFlag = LOADMEM_MALLOC_NOUGHT;
	g_loadmem.commUse = LOADMEM_COMMEM_NOUSE;
	g_loadmem.imageUse = LOADMEM_IMAGEMEM_NOUSE;

	AK_DEBUG_OUTPUT("LoadMem_Init.\r\n");
}


T_U8 *LoadMem_Malloc(T_U32 size)
{
	T_U8* tempHeader = AK_NULL;
	
	//长度小于5KB
	if (size > LOADMEM_ALLSIZE || size == 0)
	{
		AK_DEBUG_OUTPUT("LoadMem_Malloc overtop the limit size.\r\n");
		return tempHeader;
	}

	if ((LOADMEM_COMMEM_NOUSE == g_loadmem.commUse)
		&& (LOADMEM_IMAGEMEM_NOUSE == g_loadmem.imageUse))
	{
		if (LOADMEM_MALLOC_NOUGHT == g_loadmem.mallocFlag)//没申请内存
		{
#ifdef TEMP_NO_FREE_MEMORY
			if (AK_NULL != g_loadmem.header) //头指针不为空
			{
				tempHeader = g_loadmem.header;
			}
			else
			{
				tempHeader = Fwl_DMAMalloc(size);
			}
#else
			//CAMERA缓存申请失败后申请系统缓存
			if (g_loadmem.header == AK_NULL)
			{
				tempHeader = Fwl_DMAMalloc(size);
			}
			else
			{
				tempHeader = g_loadmem.header;
			}
#endif
			if (tempHeader != AK_NULL)
			{
				g_loadmem.header = tempHeader;
				g_loadmem.size = (T_U16)size;
				g_loadmem.commBlocks = 0;
				g_loadmem.memType = LOADMEM_MEMTYPE_SYSTEM;
				g_loadmem.mallocFlag = LOADMEM_MALLOC_DONE;	
			}	

		}
		else
		{
			AK_DEBUG_OUTPUT("error: the loadmemory no free.\r\n");
		}
	}
	else
	{
		AK_DEBUG_OUTPUT("error: the loadmemory is using.\r\n");
	}
	return tempHeader;
}

T_VOID LoadMem_Free(T_VOID)
{
	if ((LOADMEM_COMMEM_NOUSE == g_loadmem.commUse)
		&& (LOADMEM_IMAGEMEM_NOUSE == g_loadmem.imageUse)
		&& (LOADMEM_MALLOC_DONE == g_loadmem.mallocFlag)) //已申请内存
	{
		if (g_loadmem.memType == LOADMEM_MEMTYPE_VBUFF)//申请CAMERA缓存
		{
			g_loadmem.header = AK_NULL;
			g_loadmem.size = 0;
			g_loadmem.commBlocks = 0;
		}
		else //已经使用系统内存
		{
#ifndef  TEMP_NO_FREE_MEMORY   //暂时不释放头指针内存
			//AK_DEBUG_OUTPUT("image:free system momery success.");
			Fwl_DMAFree(g_loadmem.header);
			g_loadmem.header = AK_NULL;
#endif		
			g_loadmem.size = 0;
			g_loadmem.commBlocks = 0;

		}
		g_loadmem.mallocFlag = LOADMEM_MALLOC_NOUGHT;		
	}

}

//获取总长度
T_U16 LoadMem_GetAllSize(T_VOID)
{
	if (LOADMEM_MALLOC_DONE == g_loadmem.mallocFlag)
	{
		return g_loadmem.size;
	}
	return 0;
}

//获取公共内存长度
T_U16 LoadMem_GetCommSize(T_VOID)
{
	if ((LOADMEM_MALLOC_DONE == g_loadmem.mallocFlag)
		&&(LOADMEM_COMMEM_USED == g_loadmem.commUse)
		&& (g_loadmem.commBlocks > 0))
	{
		return LoadMem_MUL(g_loadmem.commBlocks);
	}
	return 0;
}

//最大申请2kB公共内存
//系统自动申请5kB使用
T_U8 *LoadMem_CommBuffMalloc(T_U16 size)
{
	T_U8* tempHeader = AK_NULL;
	
	if (LOADMEM_MALLOC_NOUGHT == g_loadmem.mallocFlag)//没申请内存
	{
		if (size > LOADMEM_COMMONSIZE || size == 0)
		{
			AK_DEBUG_OUTPUT("get common buff more than 2KB\r\n");
			return AK_NULL;
		}
		
		tempHeader = LoadMem_Malloc(LOADMEM_ALLSIZE);
		if (AK_NULL != tempHeader)
		{
			g_loadmem.commBlocks = LoadMem_DIV(size);
			g_loadmem.commUse = LOADMEM_COMMEM_USED;
			return tempHeader;
		}
	}
	AK_DEBUG_OUTPUT("LoadMem_CommBuffMalloc: Memory error.\r\n");
	return tempHeader;
}
T_U8 *LoadMem_GetCommBuff(T_VOID)
{
    if (LOADMEM_MALLOC_NOUGHT == g_loadmem.mallocFlag)//没申请内存
   		return AK_NULL;
	else
		return g_loadmem.header;
}

T_VOID LoadMem_CommBuffFree(T_VOID)
{
	g_loadmem.commUse = LOADMEM_COMMEM_NOUSE;
	g_loadmem.commBlocks = 0;
	LoadMem_Free();
}

//申请图片加载内存
T_U8 *LoadMem_GetRemainBuff(T_U16 * size)
{
	T_U8* tempRemain = AK_NULL;
	T_U16 len = 0;

	if (LOADMEM_MALLOC_DONE == g_loadmem.mallocFlag)//已申请内存
	{   
		if (g_loadmem.imageUse == LOADMEM_IMAGEMEM_NOUSE) 
		{
			len = LoadMem_GetCommSize();
			*size = LoadMem_GetAllSize() - len;
			tempRemain = g_loadmem.header + len;
			g_loadmem.imageUse = LOADMEM_IMAGEMEM_USED;
		}
	}
	else
	{
		tempRemain = LoadMem_Malloc(LOADMEM_ALLSIZE);
		if (AK_NULL != tempRemain)
		{
			*size = LOADMEM_ALLSIZE;
			g_loadmem.imageUse = LOADMEM_IMAGEMEM_USED;
		}
	}
	return tempRemain;
}


T_VOID LoadMem_RemainBuffFree(T_VOID )
{
	g_loadmem.imageUse = LOADMEM_IMAGEMEM_NOUSE;
	LoadMem_Free();
}

//#endif //#if(NO_DISPLAY == 0)


