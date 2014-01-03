/**
* @FILENAME nandflash.c
* @BRIEF    并口NAND FLASH控制器驱
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR   dengjian
* @MODIFY   zhaojiahuan   chenyanyan
* @DATE     2007-1-10
* @VERSION  1.0
* @REF      Please refer to…
* @NOTE     1.只支持目前市面上大量生产的samsung 和hynix的驱动（这样运行速度会快一些
*        果要支持比较特殊 的nandflash驱动，需要调整程序和宏定义
*       2. 多片选择的相关细节目前没有在驱动中体现，大多数都是针对单片flash编写。
*/
#ifdef OS_ANYKA

#include "anyka_types.h"
#include "prog_manager.h"

#if (STORAGE_USED == NAND_FLASH)//STORGE_MEDIUM is NAND FLASH
#include "mtdlib.h"
#include "Fwl_nandflash.h"
#include "nand_list.h"
#include "arch_nand.h"
#include "string.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "remap.h"
#include "mmu.h"
#include "arch_interrupt.h"
#include "utils.h"
#include "asa.h"
#include "hal_mmu.h"
#include <stdio.h>
#include "Fwl_System.h"

#if (1 == USE_NAND_TYPE)// small page nand
    #define NAND_SUPPORT_SMALL_PAGE
#else
    #define NAND_SUPPORT_LARGE_PAGE
#endif

#ifdef NAND_SUPPORT_SMALL_PAGE
#define OOB_BUF_LEN NAND_MAX_ADD_LEN * 4
#else
#define OOB_BUF_LEN NAND_MAX_ADD_LEN 
#endif
#define ALL_SECTION 0xFF
#define ASA_ADD_LEN 4

#define SPOT_PAGE_LIMIT 4096


typedef enum
{
    AREA_P0 = 0,
    AREA_BOOT,
    AREA_FSA,
    AREA_ASA,
 //   AREA_BIN,
    AREA_CNT
}E_AREA;
#pragma arm section zidata = "_bootbss1_"
T_U32 g_nPageSize;
T_U32 g_nPagePerBlock;
T_U32 g_nPagePerBlock_asa;
T_U32 g_nBlockPerChip;
T_U8  g_nChipCnt;


#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
static T_BOOL bSmall;
static T_U8 nRwPage;
static T_U8 nEBlock;
#else
#ifdef NAND_SUPPORT_SMALL_PAGE
static T_BOOL bSmall ;
static T_U8 nRwPage;
#define nEBlock 8
#endif

#ifdef NAND_SUPPORT_LARGE_PAGE
#define bSmall AK_FALSE
#define nRwPage 1
#define nEBlock 1
#endif

#endif
T_NAND_DEVICE_INFO *m_pDevice;
static T_U8 *m_pBuf_BadBlk = AK_NULL;
static T_U8 m_buf_stat = 0;
static T_BOOL m_bEnhanceSLC;
static T_NAND_ECC_CTRL *m_apEcc[AREA_CNT];
static T_NAND_ECC_CTRL  m_ASAEcc;
#pragma arm section zidata
#pragma arm section rodata = "_drvbootconst_"
static const T_U8 m_aEraseErr[] = "NF_Erf ";
static const T_U8 m_aChipID[] = "NF";
static const T_U8 m_aSLC[] = "SLC";
static const T_U8 m_aRWErr[]="NF_RWf ";
#pragma arm section rodata

static T_NANDFLASH m_NandMTD;

#pragma arm section code = "_drvbootcode_"

static T_VOID  config_device_data(T_NAND_DATA *pData, T_U8 *pMain, T_U8 *pAdd,T_U8 nPageCnt, T_U8 nSectCnt, E_AREA eArea)
{
    T_U32 nMaxSectCnt;
    
    pData->pEccCtrl = m_pDevice->ppEccCtrl[eArea];
    nMaxSectCnt = pData->pEccCtrl->nMainSectCnt;
    pData->nSectCnt = nMaxSectCnt < nSectCnt ? nMaxSectCnt : nSectCnt;
    pData->nPageCnt = nPageCnt;
    pData->pMain  = pMain;
    pData->pAdd  = pAdd;
}

static T_VOID  copy_add(T_U8 *pDest, T_U8 *pSrc, T_U8 nAddLen)
{
    if ((AK_NULL != pSrc) && (AK_NULL != pDest))
    {
//        while (nAddLen--)
        {
        memcpy(pDest, pSrc, nAddLen);
  //          *pDest++ = *pSrc++;
        }
    }
}

T_U32 remap_read_write(T_U32 nAbsPage, T_U8 *pMain,  T_U8 *pAdd, T_BOOL  bWrite)
{
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[OOB_BUF_LEN];
    
#ifdef NAND_SUPPORT_LARGE_PAGE
    T_U8 nPage;
#endif

#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        nAbsPage <<= 2;
    } 
#endif
#ifdef NAND_SUPPORT_LARGE_PAGE
    if (m_bEnhanceSLC)
    {
        m_pDevice->FunSet.ctrl(OP_GET_LOWER_PAGE, nAbsPage % g_nPagePerBlock_asa, &nPage);
        nAbsPage = ((nAbsPage / g_nPagePerBlock_asa) * g_nPagePerBlock) + nPage;
    }
#endif

    Addr.nTargetAddr = 0;
    Addr.nSectAddr = 0;
    Addr.nLogAbsPageAddr = nAbsPage;

    config_device_data(&Data, pMain, \
         aAdd, nRwPage, \
        ALL_SECTION, AREA_ASA);

    if (bWrite)
    {
        copy_add(aAdd, pAdd, ASA_ADD_LEN);
        nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);
    }
    else
    {
        nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
        copy_add(pAdd, aAdd, ASA_ADD_LEN);
    }

    #if 0
    {
        T_U32 i;
        akerror("R ", nAbsPage, 1);
        
        akerror("", 0, bWrite);
        for (i = 0; i < 512; i++)
        {
            if ( 0 == (i & 0xf))
                akerror("", 0, 1);
            akerror(" ", *((T_U32 *)pMain + i) + 1 , 0);
                
        }
        
        akerror("", 0, 1);
        akerror("A ", aAdd[0], 1);
        
    }
    #endif

    if (nDevRet & NAND_FAIL_MASK)
    {
       akerror(m_aRWErr, bWrite + 1, AK_TRUE);
       return REMAP_FAIL;
    }

    return REMAP_SUCCESS;
}

T_U32 nand_remap_read(T_U32 nAbsPage, T_U8 *pMain,  T_U8 *pAdd, T_BOOL  bBin)
{
    return remap_read_write(nAbsPage, pMain, pAdd, AK_FALSE);
}

T_U32 nand_remap_write(T_U32 nAbsPage, T_U8 *pMain,  T_U32 nAdd, T_BOOL  bBin)
{
    return remap_read_write(nAbsPage, pMain, &nAdd, AK_TRUE);
}

T_U32 nand_eraseblock(T_U32 nChip, T_U32 nAbsPage)
{
    T_U32 nRet = REMAP_SUCCESS;
    T_U32 nDevRet;
    T_NAND_ADDR Addr;
 
#ifdef NAND_SUPPORT_SMALL_PAGE
 if (bSmall)
 {
     nAbsPage <<= 2;
 }
#endif
#ifdef NAND_SUPPORT_LARGE_PAGE

    if (m_bEnhanceSLC)
    {
      nAbsPage = ((nAbsPage / g_nPagePerBlock_asa )* g_nPagePerBlock);
    }
#endif

    Addr.nTargetAddr = 0;
    Addr.nSectAddr = 0;
    Addr.nLogAbsPageAddr = nAbsPage;

    nDevRet = m_pDevice->FunSet.erase(m_pDevice, &Addr, nEBlock);

    if (NAND_FAIL_MASK & nDevRet)
    {
        akerror(m_aEraseErr, nAbsPage, AK_TRUE);
        nRet = REMAP_FAIL;    
    }

    return nRet;
}

#pragma arm section code

#pragma arm section code = "_nand_read_write_"

static T_VOID  config_device_addr(T_NAND_ADDR *pAddr, T_U16 nTarget, T_U32 nBlock, T_U32 nPage, T_U16 nSect)
{
    pAddr->nTargetAddr = nTarget;
#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
            8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
            changed by Burntool, and we have to recovery it here for "nand_get_device".
            */
        nBlock <<= 3;
        nPage <<= 2;
    }
#endif
    pAddr->nLogAbsPageAddr = nPage + (nBlock << m_pDevice->LogInfo.nBlockShift);
    pAddr->nSectAddr = nSect;
}

static E_AREA  convert_area(T_U32 nAbsPage,T_U32 eDataType)
{
    E_AREA eArea;

    switch (eDataType)
    {
        case FHA_DATA_BOOT:
        {
           if (0 == nAbsPage)
            eArea = AREA_P0;
           else
            eArea = AREA_BOOT;
        }
        break;
        case FHA_DATA_FS:
        {
            eArea = AREA_FSA;
        }
        break;
        case FHA_DATA_ASA:
        case FHA_DATA_BIN:
        {
            eArea = AREA_ASA;
        }
        break;
    }

    return eArea;
}

E_NANDERRORCODE Erase_NBlock(T_U32 nChip, T_U32 nBlock, T_U32 nBlockCnt)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_ADDR Addr;
    T_BOOL bLocked;
    
    config_device_addr(&Addr, nChip, nBlock, 0, 0);
    
    bLocked = lock_valid_addr(0, 0, AK_TRUE,  bLocked);
    nDevRet = m_pDevice->FunSet.erase(m_pDevice, &Addr, nBlockCnt);
    lock_valid_addr(0, 0, AK_FALSE,  bLocked);

    if (NAND_FAIL_MASK & nDevRet)
    {
        akerror("NF_ERASE:", nBlock, AK_TRUE);
        eRet = NF_FAIL;    
    }
    
    return eRet;
}
#pragma arm section code


T_U32 Fwl_Nand_BytesPerSector(T_PNANDFLASH hNF_Info)
{
   return g_nPageSize;
}

T_U32 Fwl_Nand_PagePerBlock(T_PNANDFLASH hNF_Info)
{
    return g_nPagePerBlock;
}

T_U8 Fwl_Get_Nand_FsSize(T_VOID)
{
    return NAND_MAX_ADD_LEN;
}
T_U32 Fwl_Nand_GetTotalBlk(T_PNANDFLASH hNF_Info)
{
    if(AK_NULL == m_pDevice)
    {
        return AK_NULL;
    }

    return g_nBlockPerChip * g_nChipCnt;
}

T_VOID Fwl_Nand_GetNandInfo(T_U8* pagebit, T_U8* blockbit, T_BOOL bRes)
{
    T_U32 tempsize = 0;
    #define RES_PAGE_USING_LIMIT      (4096)

    //nandflash的pagesize与blocksize为2^n
    tempsize = g_nPageSize;
    
    //增加对于8K以及8Kpage以上的nand的支持,8k以及8k以上nand资源文件只使用其前4k
    if(bRes && (RES_PAGE_USING_LIMIT < tempsize))
    {
        tempsize = RES_PAGE_USING_LIMIT;
    }
    *pagebit = bitnum(tempsize);
    
    tempsize = tempsize * g_nPagePerBlock;
    *blockbit = bitnum(tempsize);  
}

#pragma arm section code = "_nand_write_"

static E_NANDERRORCODE Write_Page(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{  
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[OOB_BUF_LEN];
    T_BOOL bLocked;
    
    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip) \
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        AK_DEBUG_OUTPUT("Writing Error Arg: Chip %ld, Block %ld, Page %ld .\n", nChip, nBlock, nPage);
        return NF_FAIL;
    }
    
    config_device_addr(&Addr, nChip, nBlock, nPage, 0);
    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : aAdd, nRwPage, 
        ALL_SECTION, eArea);
    copy_add(aAdd, pAdd, nAddLen);
        
    bLocked = lock_valid_addr((T_U32)pMain, g_nPageSize, AK_TRUE,  bLocked);

    if ((0 == nBlock) && (0 == nPage))
    {
        nfc_config_randomize(0, 0, AK_TRUE, AK_TRUE);
    }
    
    nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);

    if ((0 == nBlock) && (0 == nPage))
    {
        nfc_config_randomize(0, 0, AK_FALSE, AK_TRUE);
    }
    
    lock_valid_addr((T_U32)pMain, g_nPageSize, AK_FALSE,  bLocked);

    if (nDevRet & NAND_FAIL_MASK)
    {
       // AK_DEBUG_OUTPUT("%s C%d B%d P%d\n", __func__, nChip, nBlock, nPage);
       // AK_DEBUG_OUTPUT("fail!\n");
        eRet = NF_FAIL;    
    }


    return eRet;

}
#pragma arm section code

#pragma arm section code = "_nand_read_"
static E_NANDERRORCODE Read_Page(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[OOB_BUF_LEN];
    T_BOOL bLocked;
    
    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip) \
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        AK_DEBUG_OUTPUT("Reading Error Arg: Chip %ld, Block %ld, Page %ld .\n", nChip, nBlock, nPage);
        return NF_FAIL;
    }
    
    config_device_addr(&Addr, nChip, nBlock, nPage, 0);
    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : aAdd, nRwPage, \
        ALL_SECTION, eArea);

    bLocked = lock_valid_addr((T_U32)pMain, g_nPageSize, AK_TRUE,  bLocked);
    
    if ((0 == nBlock) && (0 == nPage))
    {
        nfc_config_randomize(0, 0, AK_TRUE, AK_FALSE);
    }
    
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    
    if ((0 == nBlock) && (0 == nPage))
    {
        nfc_config_randomize(0, 0, AK_FALSE, AK_FALSE);
    }
    
    lock_valid_addr((T_U32)pMain, g_nPageSize, AK_FALSE,  bLocked);

    copy_add(pAdd, aAdd, nAddLen);
    
    if (nDevRet & NAND_FAIL_MASK)
    {
        //AK_DEBUG_OUTPUT("RS B %d P %d Sect %d Cnt %d Add %d Fail!\n",
       //     nBlock, nPage, Addr.nSectAddr, Data.nSectCnt, nAddLen);
        AK_DEBUG_OUTPUT("read Fail\n");

        eRet = NF_FAIL;
    }
    else if (nDevRet & NAND_WARN_STRONG_DANGER)
    {
       // AK_DEBUG_OUTPUT("RS B %d P %d Sect %d Cnt %d Add %d Strong Danger!\n",
       //     nBlock, nPage, Addr.nSectAddr, Data.nSectCnt, nAddLen);
        eRet = NF_STRONG_DANGER;
       
       AK_DEBUG_OUTPUT("strong danger\n");
    }
    else if (nDevRet & NAND_WARN_WEAK_DANGER)
    {
        AK_DEBUG_OUTPUT("weak danger\n");
        eRet = NF_WEAK_DANGER;
    }
    
    return eRet;
}
#pragma arm section code

#pragma arm section code = "_nand_read_write_"
T_BOOL Nand_IsBadBlock(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block )
{

    T_U32 blk_per_chip, phyBlk;
    T_U8 pData[4] = {0};
    T_PNANDFLASH pNF_Info = (T_PNANDFLASH)hNF_Info;

    blk_per_chip = pNF_Info->BlockPerPlane*pNF_Info->PlanePerChip;
    phyBlk = chip*blk_per_chip + block;

    if(1 == m_buf_stat && m_pBuf_BadBlk != AK_NULL)
    {
        asa_get_bad_block(0, m_pBuf_BadBlk, pNF_Info->BlockPerPlane*pNF_Info->PlaneCnt);
        m_buf_stat = 2;
    }

    if(m_buf_stat > 1  && m_pBuf_BadBlk != AK_NULL)
    {
        T_U32 byte_loc, byte_offset;

        byte_loc = phyBlk / 8;
        byte_offset = 7 - phyBlk % 8;

        if(m_pBuf_BadBlk[byte_loc] & (1 << byte_offset))
        {
            return AK_TRUE;
        }
    }
    else
    {
        asa_get_bad_block(chip*blk_per_chip+block, pData, 1);

        if ((pData[0] & (1 << 7)) != 0)
        {
            return AK_TRUE;
        }
    }
    return AK_FALSE;
}

T_VOID Nand_SetBadBlock(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block )
{

    T_U32 blk_per_chip;
    T_U32 byte_loc, byte_offset, phyblock;
    T_PNANDFLASH pNF_Info = (T_PNANDFLASH)hNF_Info;

    blk_per_chip = pNF_Info->BlockPerPlane*pNF_Info->PlanePerChip;
    phyblock = chip * blk_per_chip + block;

    asa_set_bad_block(phyblock);

    if(AK_NULL != m_pBuf_BadBlk)
    {
        byte_loc = phyblock / 8;
        byte_offset = 7 - phyblock % 8;
        m_pBuf_BadBlk[byte_loc] |= 1 << byte_offset;
    }
}

E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH hNF_Info, T_U32 nChip, T_U32 nBlock)
{
    return Erase_NBlock(nChip, nBlock, nEBlock);
}

E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd, T_U32 nAddLen)
{
    return Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
}

E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen)
{
    return Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
}

E_NANDERRORCODE Nand_ReadFlag(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pAdd, T_U32 nAddLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[NAND_MAX_ADD_LEN];

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip) \
       || (nPage >= g_nPagePerBlock) || (AK_NULL == pAdd))
    {
        AK_DEBUG_OUTPUT("Reading Error Arg: Chip %ld, Block %ld, Page %ld .\n", nChip, nBlock, nPage);
        return NF_FAIL;
    }
    
    config_device_data(&Data, AK_NULL, aAdd, 1, 0, AREA_FSA);
    config_device_addr(&Addr, nChip, nBlock, nPage, bSmall ? 0 : Data.pEccCtrl->nMainSectCnt);
    check_stack();
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    copy_add(pAdd, aAdd, nAddLen);
    
    if (NAND_FAIL_MASK & nDevRet)
    {
        AK_DEBUG_OUTPUT("Read Flag Fail: Chip %ld, Block %ld Page %ld\n", nChip, nBlock, nPage);
        eRet = NF_FAIL;    
    }
    
    return eRet;
}

#pragma arm section code

E_NANDERRORCODE Nand_WriteFlag(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block, T_U32 sector, T_U8 *spare, T_U32 spare_len)
{
    return NF_SUCCESS;
}

E_NANDERRORCODE Nand_MutiWriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    return NF_FAIL;
}
E_NANDERRORCODE Nand_MutiReadSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    return NF_FAIL;
}

E_NANDERRORCODE Nand_MultiCopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    return NF_FAIL;
}

E_NANDERRORCODE Nand_MultiEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block)
{
    return NF_FAIL;
}
                              
#pragma arm section code = "_nand_write_"

E_NANDERRORCODE Nand_WriteSector_Exnftl(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nPlaneCnt, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, T_U32 nPageCnt)
{
   E_NANDERRORCODE ret;
   T_U32 i;

   for(i = 0; i < nPageCnt; i++)
   {
       ret = Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, AREA_FSA);

       if(NF_FAIL == ret)
       {
           goto L_EXIT;
       }
       
       nPage++;
       pMain += g_nPageSize;
       pAdd += nAddLen;
   }

L_EXIT:
   return  ret;
}

#pragma arm section code
#pragma arm section code = "_nand_read_write_"
E_NANDERRORCODE Nand_EraseBlock_Exnftl(T_PNANDFLASH nand, T_U32 nChip, T_U32 plane_num, T_U32 nBlock)
{
    return Erase_NBlock(nChip, nBlock, nEBlock);
}
#pragma arm section

#pragma arm section code = "_nand_read_"

E_NANDERRORCODE Nand_ReadSector_Exnftl(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nPlaneCnt, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, T_U32 nPageCnt)
{
    E_NANDERRORCODE ret;
    T_U32 i;

    for(i = 0; i < nPageCnt; i++)
    {
        ret = Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, AREA_FSA);

        if(NF_FAIL == ret)
        {
           goto L_EXIT;
        }

        nPage++;
        pMain += g_nPageSize;
        pAdd += nAddLen;
    }
    
L_EXIT:
    return  ret;
}

#pragma arm section
#pragma arm section code = "_update_"
#if (UPDATA_USED == 1)

#ifdef NAND_SUPPORT_SMALL_PAGE
typedef E_NANDERRORCODE (*pRW)(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea);

static T_U32 Small_Page0(T_U8 *pMain, T_BOOL bWrite)
{
    
    E_NANDERRORCODE nRet = NF_FAIL;
    pRW pRwFun;
    
    bSmall = AK_FALSE;

    if (bWrite)
    {
        pRwFun = Write_Page;
    }
    else
    {
        pRwFun = Read_Page;
    }
    
    nRwPage = 1;
    if (NF_FAIL == pRwFun(0, 0, 0, pMain, AK_NULL, 0, AREA_P0))
    {
        goto L_EXIT;
    }
    
    
    nRwPage = 3;
    nRet = pRwFun(0, 0, 1, pMain + 512, AK_NULL, 0, AREA_BOOT);
L_EXIT:
    nRwPage = 4;

    bSmall = AK_TRUE;

    if (NF_FAIL == nRet)
        return FHA_FAIL;
    
    return FHA_SUCCESS;
}
#endif
E_NANDERRORCODE Nand_ReadBytes(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U16 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_BOOL bLocked;
    
    config_device_addr(&Addr, nChip, nBlock, nPage, nColumn);
    Data.pEccCtrl = AK_NULL;
    Data.nSectCnt = nBufLen;
    Data.nPageCnt = 1;
    Data.pMain  = pBuf;
    Data.pAdd  = AK_NULL;
    
    bLocked = lock_valid_addr((T_U32)pBuf, nBufLen, AK_TRUE,  bLocked);
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    lock_valid_addr((T_U32)pBuf, nBufLen, AK_FALSE,  bLocked);

    
    if (NAND_FAIL_MASK & nDevRet)
    {
        AK_DEBUG_OUTPUT("fail!\n");
        eRet= NF_FAIL;
    }
    
     
    return eRet;
}

E_NANDERRORCODE Nand_WriteBytes(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U16 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_BOOL bLocked;
    
    config_device_addr(&Addr, nChip, nBlock, nPage, nColumn);
    Data.pEccCtrl = AK_NULL;
    Data.nSectCnt = nBufLen;
    Data.nPageCnt = 1;
    Data.pMain  = pBuf;
    Data.pAdd  = AK_NULL;
    
    bLocked = lock_valid_addr((T_U32)pBuf, nBufLen, AK_TRUE,  bLocked);
    nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);
    lock_valid_addr((T_U32)pBuf, nBufLen, AK_FALSE,  bLocked);

    if (NAND_FAIL_MASK & nDevRet)
    {
        AK_DEBUG_OUTPUT("faile!\n");
        eRet = NF_FAIL;    
    }
    return eRet;

}


T_BOOL FHA_Nand_ReadBytes(T_U32 nChip, T_U32 nAbsPage, T_U32 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = FHA_SUCCESS;
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);

    if (NF_FAIL ==  Nand_ReadBytes(AK_NULL, nChip, nBlock, nPage, nColumn, pBuf, nBufLen))
        eRet= FHA_FAIL;
     
    return eRet;
}


T_U32 FHA_Nand_EraseBlock(T_U32 chip, T_U32 nAbsPage)
{
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
#ifdef NAND_SUPPORT_SMALL_PAGE
    
    if (bSmall)
    {
        nBlock >>= 1;
    }
#endif

    if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
    }
    
    if (NF_FAIL == Erase_NBlock(chip, nBlock, nEBlock))
        return FHA_FAIL;
    else
        return FHA_SUCCESS;

}

T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nAbsPage, const T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType)
{
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);
    E_AREA eArea = convert_area(nAbsPage, eDataType);
    E_NANDERRORCODE nRet;
    T_BOOL bElcMode;
    
#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        if (0 == nAbsPage)
        {
            return Small_Page0(pMain, AK_TRUE);
        }
        
        if (nBlock & 0x1)
            nPage += 32;

        nBlock >>= 1;
    }
#endif

    if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
        m_pDevice->FunSet.ctrl(OP_GET_LOWER_PAGE, nAbsPage % g_nPagePerBlock_asa, &nPage);
        bElcMode = AK_TRUE;
        m_pDevice->FunSet.ctrl(OP_ENABLE_SLC, nChip, (T_VOID *)&bElcMode);
    }

    nRet = Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, eArea);
    if (AK_TRUE == m_bEnhanceSLC)
    {
        bElcMode = AK_FALSE;
        m_pDevice->FunSet.ctrl(OP_ENABLE_SLC, nChip, (T_VOID *)&bElcMode);
    }

    if (NF_FAIL == nRet)
        return FHA_FAIL;
    else
        return FHA_SUCCESS;

}

T_U32 FHA_Nand_ReadPage(T_U32 nChip, T_U32 nAbsPage, T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType)
{ 
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);
    E_AREA eArea = convert_area(nAbsPage, eDataType);
    E_NANDERRORCODE nRet;

#ifdef NAND_SUPPORT_SMALL_PAGE

    if (bSmall)
    {
        if (0 == nAbsPage)
        {
            return Small_Page0(pMain, AK_FALSE);
        }
        
        if (nBlock & 0x1)
            nPage += 32;

        nBlock >>= 1;
    }
#endif
if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
        m_pDevice->FunSet.ctrl(OP_GET_LOWER_PAGE, nAbsPage % g_nPagePerBlock_asa, &nPage);
    }
    
    nRet = Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, eArea);
    if (NF_FAIL == nRet)
        return FHA_FAIL;
    else
        return FHA_SUCCESS;
}


#endif

#pragma arm section code 

T_S8 Fwl_Nand_ReadSector_AnyByte(T_U32 nBlock,T_U32 nPage, T_U32 nSector,T_U8 *pMain,T_U8* filepath,T_U16 nSize)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 nSectorCnt = nSize / 512;
    T_BOOL bLocked;
    
    //AK_DEBUG_OUTPUT("%s  B%d P%d S%d Cnt%d\n", __func__,  nBlock, nPage, nSector, nSectorCnt);

    config_device_addr(&Addr, 0, nBlock, nPage, nSector);
    config_device_data(&Data, pMain, AK_NULL, 1, nSectorCnt, AREA_ASA);

    bLocked = lock_valid_addr((T_U32)pMain, nSize, AK_TRUE,  bLocked);
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    lock_valid_addr((T_U32)pMain, nSize, AK_FALSE,  bLocked);



    if (nDevRet & NAND_FAIL_MASK)
    {
        AK_DEBUG_OUTPUT("fail!\n");
        eRet = NF_FAIL;
    }

    return eRet;
}

T_S8 Fwl_Nand_ReadSector_512Byte(T_U32 nBlock,T_U32 nPage, T_U32 nSector,T_U8 *pMain,T_U8* filepath)
{
    return Fwl_Nand_ReadSector_AnyByte(nBlock, nPage, nSector, pMain,filepath, 512);
}

T_PNANDFLASH Nand_Init_MTD(T_VOID)
{
    T_PNANDFLASH pNandMtdInfo = &m_NandMTD;

    if (AK_NULL == m_pDevice)
    {
        AK_DEBUG_OUTPUT("No Nand Device!\n");
        pNandMtdInfo = AK_NULL;
    }
    
    pNandMtdInfo->ChipCharacterBits = 0x1;//step = 1
    pNandMtdInfo->NandType = 0;//NANDFLASH_TYPE_SAMSUNG useless
    pNandMtdInfo->BytesPerSector = m_pDevice->LogInfo.nLogicBPP;                            
    pNandMtdInfo->BlockPerPlane = m_pDevice->LogInfo.nLogicBPC;
    pNandMtdInfo->PlanePerChip = 1;
    pNandMtdInfo->PlaneCnt = g_nChipCnt;
    pNandMtdInfo->PagePerBlock =  m_pDevice->LogInfo.nLogicPPB;
    pNandMtdInfo->SectorPerPage = 1;   
    pNandMtdInfo->WriteSector = Nand_WriteSector;
    pNandMtdInfo->ReadSector = Nand_ReadSector;
    pNandMtdInfo->ReadFlag = Nand_ReadFlag;
    pNandMtdInfo->WriteFlag = Nand_WriteFlag;
    pNandMtdInfo->CopyBack = AK_NULL;

    pNandMtdInfo->EraseBlock = Nand_EraseBlock;
    pNandMtdInfo->IsBadBlock = Nand_IsBadBlock;
    pNandMtdInfo->SetBadBlock  = Nand_SetBadBlock;

    pNandMtdInfo->MultiEraseBlock = Nand_MultiEraseBlock;
    pNandMtdInfo->MultiCopyBack = Nand_MultiCopyBack;
    pNandMtdInfo->MultiWrite = Nand_MutiWriteSector;
    pNandMtdInfo->MultiRead = Nand_MutiReadSector;

    pNandMtdInfo->ExEraseBlock = Nand_EraseBlock_Exnftl;
    pNandMtdInfo->ExRead = Nand_ReadSector_Exnftl;
    pNandMtdInfo->ExReadFlag = Nand_ReadFlag;
    pNandMtdInfo->ExWrite = Nand_WriteSector_Exnftl;
    pNandMtdInfo->ExIsBadBlock = Nand_IsBadBlock;    
    pNandMtdInfo->ExSetBadBlock = Nand_SetBadBlock;

#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        pNandMtdInfo->BytesPerSector = 2048;
        pNandMtdInfo->PagePerBlock = 64;
        pNandMtdInfo->BlockPerPlane >>= 3;
    }
#endif

    return pNandMtdInfo;

}
/**
 * @brief   initialization of badBlock buf.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_BOOL
 */

T_BOOL Fwl_Nand_BadBlBufInit(T_VOID)
{
    m_pBuf_BadBlk = (T_U8*)Fwl_Malloc(((g_nBlockPerChip * g_nChipCnt) >> 3) + 1);
    if (AK_NULL != m_pBuf_BadBlk)
    {
        m_buf_stat = 1;
        return AK_TRUE;
    }
    AK_DEBUG_OUTPUT("malloc error\n");
    return AK_FALSE;
}
/**
 * @brief   free bad block buf.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_VOID
 */

T_VOID Fwl_Nand_BadBlBufFree(T_VOID)
{
    if (AK_NULL != m_pBuf_BadBlk)
    {
        Fwl_Free(m_pBuf_BadBlk);        
        m_pBuf_BadBlk = AK_NULL;
    }
    m_buf_stat = 0;
}


#pragma arm section code
#pragma arm section code = "_drvbootinit_"

T_VOID Nand_Init_Remap(T_VOID * pNandParam)
{
    T_U32 ChipID, ChipCnt;
    T_U32 nGpioPos;
    T_NAND_PARAM NandParam;
#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
    bSmall = AK_FALSE;
    nRwPage = 1;
    nEBlock = 1;
#else
    #ifdef NAND_SUPPORT_SMALL_PAGE
    bSmall = AK_TRUE ;
    nRwPage = 4;
    #endif

#endif
    nGpioPos = 0xFFFFFFFF;
    
    nand_setCe_getID(&ChipID, &ChipCnt, &nGpioPos, NFC_SUPPORT_CHIPNUM);

    akerror(m_aChipID, ChipID, AK_FALSE);
    akerror(AK_NULL, ChipCnt, AK_TRUE);
    memcpy(&NandParam, pNandParam, sizeof(T_NAND_PARAM));

#ifdef NAND_SUPPORT_SMALL_PAGE

    if (1 == NandParam.col_cycle)    
    {    
        /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
            8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
            changed by Burntool, and we have to recovery it here for "nand_get_device".
            */
        
#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
        bSmall = AK_TRUE;
        nRwPage = 4;
        nEBlock = 8;
#endif
        NandParam.page_per_blk = 32;
        NandParam.page_size = 512;
        NandParam.blk_num <<= 3;
        NandParam.plane_blk_num = NandParam.blk_num;
        NandParam.group_blk_num = NandParam.blk_num;
    }
#endif
    
    m_pDevice = nand_get_device(&NandParam, ChipCnt);

    if (AK_NULL == m_pDevice)
    {
    }
    else
    {   
        m_pDevice->FunSet.ctrl(OP_GET_ECC, 0, m_apEcc);
        m_pDevice->ppEccCtrl = m_apEcc;
        m_apEcc[AREA_ASA]= &m_ASAEcc;
        //  m_apEcc[AREA_ASA] = m_apEcc[AREA_FSA];
        m_ASAEcc.bSeperated = m_apEcc[AREA_FSA]->bSeperated;
        m_ASAEcc.nAddEcc = m_apEcc[AREA_FSA]->nAddEcc;
        m_ASAEcc.nAddLen = m_apEcc[AREA_FSA]->nAddLen;
        m_ASAEcc.nMainEcc = m_apEcc[AREA_FSA]->nMainEcc;
        m_ASAEcc.nMainLen = m_apEcc[AREA_FSA]->nMainLen; //512;
        m_ASAEcc.nSectOffset = bSmall ? 0 : m_apEcc[AREA_FSA]->nSectOffset;;
       // m_ASAEcc.nMainSectCnt = (m_pDevice->LogInfo.nLogicBPP > SPOT_PAGE_LIMIT ? SPOT_PAGE_LIMIT : m_pDevice->LogInfo.nLogicBPP) / m_apEcc[AREA_FSA]->nMainLen;
        m_ASAEcc.nMainSectCnt = m_pDevice->LogInfo.nLogicBPP > SPOT_PAGE_LIMIT ? 4 : m_apEcc[AREA_FSA]->nMainSectCnt;
        g_nChipCnt = ChipCnt;
        g_nPageSize = m_pDevice->LogInfo.nLogicBPP;                            
        g_nBlockPerChip = m_pDevice->LogInfo.nLogicBPC;
        g_nPagePerBlock = m_pDevice->LogInfo.nLogicPPB;

#ifdef NAND_SUPPORT_SMALL_PAGE

        if (1 == NandParam.col_cycle)    
        {    
            /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
                8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
                changed by Burntool, and we have to recovery it here for "nand_get_device".
                */
            g_nPageSize = 2048;                            
            g_nBlockPerChip = m_pDevice->LogInfo.nLogicBPC >> 3;
            g_nPagePerBlock = 64;
        }
#endif
        g_nPagePerBlock_asa = g_nPagePerBlock;

        if (0 != (FLAG_ENHANCE_SLC & NandParam.flag))
        {
            g_nPagePerBlock_asa >>= 1;
            m_bEnhanceSLC = AK_TRUE;
            akerror(m_aSLC, g_nPagePerBlock_asa, AK_TRUE);
        }
    }
    
}
#pragma arm section code
#endif//(STORAGE_USED == NAND_FLASH)//STORGE_MEDIUM is NAND FLASH

#endif
