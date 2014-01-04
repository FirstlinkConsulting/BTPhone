/**
 * @file mmu.c
 * @brief mmu function file, provide drivers of MMU module
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */

#include "anyka_types.h"
#include "anyka_cpu.h"
#include "mmu.h"
#include "hal_mmu.h"
#include "hal_errorstr.h"


extern T_VOID set_l2_peripheral_buf(T_U32 addr);



#pragma arm section zidata = "_drvbootbss_", rwdata = "_fixedrwdata_"
static T_U32    TT_base;
static T_U32    Page_base;
static T_U32    uRam_size;
static T_U32    premap_end = L2_END_ADDR;
#pragma arm section

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_MMU;
#pragma arm section rodata

#pragma arm section code = "_drvbootinit_"

T_VOID MMU_Init_SetMTT(T_U32 vaddrStart,T_U32 vaddrEnd,
                       T_U32 paddrStart,T_U32 attr)
{
    T_U32 *pTT;
    T_U32 i, nSec;

    pTT = (T_U32 *)TT_base + (vaddrStart>>20);  /* poT_U32er arith! */
    nSec = (vaddrEnd>>20) - (vaddrStart>>20);
    for (i=0; i<=nSec; i++)
    {
        *pTT++ = attr | (((paddrStart>>20)+i)<<20);
    }
}


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
    T_U32 mmuTT_base, T_U32 mmuPage_base)
{
    //========================== IMPORTANT NOTE =========================
    //The current stack and code area can't be re-mapped in this routine.
    //If you want memory map mapped freely, your own sophisticated MMU
    //initialization code is needed.
    //===================================================================

    if((peripheral_buf_base & 0xFFF) || (mmuTT_base & 0x3FFF) || 
        (mmuPage_base & 0x3FF))
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }
    if((L2_START_ADDR > peripheral_buf_base) || 
        (L2_START_ADDR > mmuTT_base) || 
        (L2_START_ADDR > mmuPage_base))
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }
    //need the check the overflow ?


    TT_base     = mmuTT_base;
    Page_base   = mmuPage_base;
    uRam_size   = ram_size;
    premap_end   = L2_START_ADDR+premap_size;

    set_l2_peripheral_buf(peripheral_buf_base);

    //rom code space
    MMU_Init_SetMTT(0x00000000, TOPCFG_MODULE_BASE_ADDR, 
                    0x00000000, RW_CNB);
    //io register space
    MMU_Init_SetMTT(TOPCFG_MODULE_BASE_ADDR, L2_START_ADDR,
                    TOPCFG_MODULE_BASE_ADDR, RW_NCNB);


    //set the second coarse page table
    MMU_MapPage(L2_START_ADDR,L2_START_ADDR+premap_size,L2_START_ADDR,RW_SMALL);

    //set MMU table as non-cache
    MMU_MapPage(Page_base & (~(VPAGE_SIZE-1)), 
                (Page_base & (~(VPAGE_SIZE-1))) + VPAGE_SIZE,
                Page_base & (~(VPAGE_SIZE-1)), RW_NCNB_SMALL);

    //set L2 buffer as non-cache
    MMU_MapPage(peripheral_buf_base & (~(VPAGE_SIZE-1)),
                (peripheral_buf_base & (~(VPAGE_SIZE-1))) + VPAGE_SIZE,
                peripheral_buf_base & (~(VPAGE_SIZE-1)), RW_NCNB_SMALL);


    MMU_MapPage(L2_START_ADDR+premap_size, L2_START_ADDR+ram_size,\
                L2_START_ADDR+premap_size, FB_SMALL);

    MMU_MapPage(L2_START_ADDR+160*1024, L2_START_ADDR+160*1024+4*1024, 
                L2_START_ADDR+160*1024, RW_SMALL);


    MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR);
    //DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)
    MMU_SetProcessId(0x0);
    MMU_CleanSR();

    MMU_SetTTBase(TT_base);
    //MMU_EnableAlignFault();

    return AK_TRUE;
}
#pragma arm section code


T_VOID MMU_Enable(T_VOID)
{
    MMU_EnableMMU();
    MMU_InvalidateTLB();
    MMU_EnableICache();
    MMU_EnableDCache();
}
#if 0
T_VOID MMU_CleanCache()
{
    T_U32 i,j;
    //clean the D cache
    //If write-back is used,the DCache should be cleaned.
    for(i=0;i<64;i++)
        for(j=0;j<8;j++)
            MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
}
#endif

T_VOID MMU_InvalidateDCache()
{
    MMU_Clean_Invalidate_Dcache();
}

T_VOID MMU_MapPageEx(T_U32 vaddrStart, T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr)
{
    T_U32 *pTT, *pPT;
    T_U32 addr;

    //aligned the boundary of address
    vaddrStart = vaddrStart & (~(VPAGE_SIZE-1));
    vaddrEnd = vaddrEnd & (~(VPAGE_SIZE-1));
    paddrStart = paddrStart & (~(VPAGE_SIZE-1));

    for(addr=vaddrStart; addr<vaddrEnd; addr+=VPAGE_SIZE, paddrStart+=VPAGE_SIZE)
    {
        pTT = (T_U32 *)TT_base + (addr>>20);
        pPT = (T_U32 *)Page_base + ((addr - L2_START_ADDR)>>12);

        *pTT = ((T_U32)pPT & (~((1<<10)-1))) | DEF_PAGE;
        *pPT = (paddrStart & (~((1<<12)-1))) | attr;
    }
}

T_VOID MMU_MapPage(T_U32 vaddrStart,T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr)
{
    MMU_MapPageEx(vaddrStart, vaddrEnd, paddrStart, attr);

    /* refresh TLB and IDCache */
    //MMU_InvalidateDCache();
    MMU_InvalidateTLB();
}

/*******************************************************************************
 * @NAME    MMU_GetPT2Paddr
 * @BRIEF   get content of the item in mmu page table. 
 * @AUTHOR  liangxiong
 * @DATE    2012-05-23
 * @PARAM   vaddr: the virtual address.(accord 4K page)
 * @RETURN  get content of the item in mmu page table.
*******************************************************************************/
T_U32 MMU_GetPT2Paddr(T_U32 vaddr)
{
    T_U32 *pPT = 0;

#ifndef UNSUPPORT_REMAP
    if (vaddr >= L2_START_ADDR && vaddr < (L2_START_ADDR+uRam_size))
    {
        pPT = (T_U32 *)Page_base + ((vaddr - L2_START_ADDR)>>12);
        return (*pPT);
    }
#endif
    return 0;
}

T_U32 MMU_Vaddr2Paddr(T_U32 vaddr)
{
    T_U32 *pPT;
    T_U32 paddr;

#ifndef UNSUPPORT_REMAP
    if (vaddr >= L2_START_ADDR && vaddr < (L2_START_ADDR+uRam_size))
    {
        pPT = (T_U32 *)Page_base + ((vaddr - L2_START_ADDR)>>12);
        if (MMU_CheckMaping(*pPT)) //check mapping or not
            paddr = (((*pPT)&(~0xFFF)) | ((T_U32)vaddr&0xFFF));
        else
            paddr = 0;
    }
    else
        paddr = vaddr;
#else
    paddr = vaddr;
#endif

    return paddr;
}

T_BOOL MMU_IsPremapAddr(T_U32 addr)
{
    return (addr < premap_end);
}

