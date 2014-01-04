/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºFwl_osMalloc.h
 * Function£ºThis header file is API for Memory Library
 *
 * Author£º
 * Date£º
 * Version£º
**************************************************************************/
#ifndef __FWL_OS_MALLOC_H__
#define __FWL_OS_MALLOC_H__


#include "anyka_types.h"


T_U32   Fwl_GetMaxAllocSize(T_VOID);

T_VOID  Fwl_MallocInit(T_VOID);

T_pVOID Fwl_Malloc(unsigned long sz);

T_pVOID Fwl_Free(T_pVOID var);

T_pVOID Fwl_DMAMalloc(T_U32 size);

T_pVOID Fwl_DMAFree(T_pVOID var);

T_pVOID Fwl_SysRsvMalloc(T_VOID);

T_U32 Fwl_GetUsedMem(T_VOID);

T_BOOL Fwl_SysMallocFlag(T_VOID);

#ifdef MALLOC_CHECK
T_VOID PrintChekMaloc(T_VOID);
#endif


#endif //__FWL_MALLOC_H__

