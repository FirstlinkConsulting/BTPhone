/**
 * @file    hal_mmu.h
 * @brief   the interface for the control of mmu onchip
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.29
 * @version 1.0
 */

#ifndef _HAL_MMU_H_
#define _HAL_MMU_H_

#include "anyka_types.h"

/**
* @brief       set up the section and the page table,and then enable the mmu 
*               and the I/D cache. the L2 peripheral buffer will also be set.
* @author      wangguotian
* @date        2012.12.3
* @param[in]   ram_size
*                  the memory ram size in the virtual space¡£
* @param[in]   premap_size
*                  the size of the ram which will not be remap.
* @param[in]   peripheral_buf_base
*                  the L2 peripheral buffer start address
* @param[in]   mmuTT_base
*                  the start address of the translation table
* @param[in]   mmuPage_base
*                  the start address of the page table
* @return      T_BOOL
* @retval      If the function succeeds, the return value is AK_TRUE;
*              If the function fails, the return value is AK_FALSE.
*/
T_BOOL MMU_Init(T_U32 ram_size, T_U32 premap_size, T_U32 peripheral_buf_base, 
    T_U32 mmuTT_base, T_U32 mmuPage_base);
    
T_VOID MMU_Enable(T_VOID);

T_VOID MMU_EnableICache(T_VOID);

T_VOID MMU_DisableICache(T_VOID);

T_VOID MMU_EnableDCache(T_VOID);

T_VOID MMU_DisableDCache(T_VOID);

T_VOID MMU_EnableMMU(T_VOID);

T_VOID MMU_DisableMMU(T_VOID);

T_VOID MMU_InvalidateIDCache(T_VOID);

T_VOID MMU_InvalidateICache(T_VOID);

T_VOID MMU_InvalidateDCache(T_VOID);

T_VOID MMU_InvalidateTLB(T_VOID);


 /******************************************************************************
* @NAME    MMU_GetPT2Paddr
* @BRIEF   get content of the item in mmu page table. 
* @AUTHOR  liangxiong
* @DATE    2012-05-23
* @PARAM   vaddr: the virtual address.(accord 4K page)
* @RETURN  get content of the item in mmu page table.
*	
*******************************************************************************/
T_U32 MMU_GetPT2Paddr(T_U32 vaddr);



T_U32 MMU_Vaddr2Paddr(T_U32 vaddr);

T_VOID MMU_MapPageEx(T_U32 vaddrStart, T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr);


T_VOID MMU_MapPage(T_U32 vaddrStart,T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr);



#endif //_HAL_MMU_H_
