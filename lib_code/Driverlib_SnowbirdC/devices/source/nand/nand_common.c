#include "anyka_types.h"
#include "arch_nand.h"
#include "nand_retry.h"
#include "drv_cfg.h"
#include "nand_command.h"

#if (DRV_SUPPORT_NAND > 0)

#if ((NAND_LARGE_PAGE > 0) && (NAND_SMALL_PAGE > 0))

#define NAND_MAX_RETRY_TIME  15
#define NAND_WARN_RETRY_TIME    8

//when flip bits are more than the following defines, be careful of the data
#define WEAK_DANGER_BIT_NUM_TYPE0 3  //4 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE1 5  //8 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE2 8  //12 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE3 12  //16 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE4 18  //24 bit nand ecc's weak danger flip bit number among 1024 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE5 26  //32 bit nand ecc's weak danger flip bit number among 1024 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE6 34  //40 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE7 38  //44 bit nand ecc's weak danger flip bit number among 1024 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE8 46  //60 bit nand ecc's weak danger flip bit number among 1024 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE9 66  //72 bit nand ecc's weak danger flip bit number among 1024 Bytes

//when flip bits are more than the following defines, don't use current block any more
#define STRONG_DANGER_BIT_NUM_TYPE0 4  //4 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE1 7  //8 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE2 10  //12 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE3 14  //16 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE4 22  //24 bit nand ecc's strong danger flip bit number among 1024 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE5 30  //32 bit nand ecc's strong danger flip bit number among 1024 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE6 36  //40 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE7 40  //44 bit nand ecc's strong danger flip bit number among 1024 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE8 56  //60 bit nand ecc's strong danger flip bit number among 1024 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE9 68  //72 bit nand ecc's strong danger flip bit number among 1024 Bytes

typedef struct NAND_PHYSIC_INFO
{
    T_U32  nPagesPerLun;
    T_U32  nLun1StartPage;
    T_U16  nPagePerBlock;
    T_U16  nBytePerPage;
    T_U8    nBlockShift;
    T_U8    nEccType;
    T_U8    nColCycle;
    T_U8    nRowCycle;
    T_BOOL bRandom;

#if NAND_SUPPORT_RR>0
    T_BOOL bRetry;
#endif   

}T_NAND_PHYSIC_INFO;

#pragma arm section zidata = "_drvbootbss_"
static T_NAND_DEVICE_INFO   m_DevInfo;
static T_NAND_PHYSIC_INFO   m_PhyInfo;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 m_aReReadInfo[] = NAND_REREAD_INFO;
static const T_U8 m_aReReadInfo1[] = NAND_REREAD_INFO1;
static const T_U8 m_aReReadInfo2[] = NAND_REREAD_INFO2;
static const T_U8 m_aStatusErr[] = NAND_STATUS_ERROR;
static const T_U8 m_aReadFailErr[] = NAND_READ_FAIL;

//the experiential value for strong danger in each ecc type 
static const T_U8 m_aStrongDangerBits[] =
{
    STRONG_DANGER_BIT_NUM_TYPE0,
    STRONG_DANGER_BIT_NUM_TYPE1,
    STRONG_DANGER_BIT_NUM_TYPE2,
    STRONG_DANGER_BIT_NUM_TYPE3,
    STRONG_DANGER_BIT_NUM_TYPE4,
    STRONG_DANGER_BIT_NUM_TYPE5,
    STRONG_DANGER_BIT_NUM_TYPE6,
    STRONG_DANGER_BIT_NUM_TYPE7,
    STRONG_DANGER_BIT_NUM_TYPE8,
    STRONG_DANGER_BIT_NUM_TYPE9
};

//the experiential value for weak danger in each ecc type 
static const T_U8 m_aWeakDangerBits[] =
{
    WEAK_DANGER_BIT_NUM_TYPE0,
    WEAK_DANGER_BIT_NUM_TYPE1,
    WEAK_DANGER_BIT_NUM_TYPE2,
    WEAK_DANGER_BIT_NUM_TYPE3,
    WEAK_DANGER_BIT_NUM_TYPE4,
    WEAK_DANGER_BIT_NUM_TYPE5,
    WEAK_DANGER_BIT_NUM_TYPE6,
    WEAK_DANGER_BIT_NUM_TYPE7,
    WEAK_DANGER_BIT_NUM_TYPE8,
    WEAK_DANGER_BIT_NUM_TYPE9
};
#pragma arm section rodata

/**
 * @brief        read nandflash status register
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_U8
 * @retval      the status value
 */
static T_U8 wait_ready_status(T_VOID )
{
    T_U16   i;
    T_U32   status;
    
    for (i = 0; i < T_U16_MAX; i++ )
    {
        nfc_cycle(CMD_CYCLE(NFLASH_STATUS), DELAY_CYCLE(0), END_CYCLE);
        nfc_read((T_U8 *)&status, AK_NULL, 1, AK_NULL);

        if (0xFF == status)//when got 0xFF ,the datalen might be too short to saffice operation timing
        {
            status = 0;           
            break;
        }
        
        if (0 == (status & NFLASH_STATUS_NWP_BIT))
        {
            break;
        }
        else if (0 != (status & NFLASH_STATUS_READY_BIT))
        {
            break;
        }

       status = 0;
    } 

    return status;
}

/**
 * @brief        get column address for specified sector
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   nSectAddr,  sector address
 * @param[in]   pEccCtrl, ecc strategy for this page
 * @return      T_U16
 * @retval      column address
 */
static T_U16 get_column_addr(T_U16 nSectAddr, T_NAND_ECC_CTRL * pEccCtrl)
{
    T_U16 nColumn;
    T_U16 section_offset;
    T_U8 i;
    
    if (AK_NULL == pEccCtrl)
    {
        nColumn = nSectAddr;
    }
    else
    {
        nColumn = 0;
        section_offset = pEccCtrl->nSectOffset;
        for (i = 0; i < nSectAddr; i++)
        {
            nColumn += section_offset;
        }
    }

    return nColumn;
}



static T_VOID op_reset(T_VOID)
{
    //we don't wait R/b signal course may not there be a nandflash conected . just to delay enough time,
    nfc_cycle(CMD_CYCLE(NFLASH_RESET), DELAY_CYCLE(240), END_CYCLE);
}

/**
 * @brief        issue erase cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nRowAddr the absolute row address of target block
 * @return      T_BOOL
 */
static T_BOOL op_erase(T_U32 nRowAddr, T_NAND_PHYSIC_INFO *pPhyInfo, T_BOOL bRb)
{
    return nfc_cycle(CMD_CYCLE(NFLASH_ERASE_1),\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        pPhyInfo->nRowCycle > 1 ? ADDR_CYCLE((nRowAddr >> 8) & 0xFF) : NULL_CYCLE,\
        pPhyInfo->nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        CMD_CYCLE(NFLASH_ERASE_2),\
        bRb ? RB_CYCLE : NULL_CYCLE,\
        END_CYCLE);
}

/**
 * @brief        issue read cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nColumnAddr the coumn address of target column
 * @param[in] nRowAddr the absolute row address of target page
 * @return      T_BOOL
 */
static T_BOOL  op_read(T_U16 nColumnAddr, T_U32 nRowAddr, T_NAND_PHYSIC_INFO *pPhyInfo, T_BOOL bRb)
{
    T_BOOL   bRet = AK_FALSE;
    T_U8    nCmdValue = NFLASH_READ_1;

    //to read large page or the small page when ColumnAddr < 256
    if (512 == pPhyInfo->nBytePerPage)
    {
        if (nColumnAddr >= 512)
        {
            nCmdValue = NFLASH_READ22;
        }
        else   if (nColumnAddr >= 256)
        {
            nCmdValue = NFLASH_READ1_HALF;
        }
    }

    bRet = nfc_cycle(CMD_CYCLE(nCmdValue),\
        ADDR_CYCLE(nColumnAddr & 0xFF),\
        pPhyInfo->nColCycle > 1 ? ADDR_CYCLE((nColumnAddr >> 8) & 0xFF) : NULL_CYCLE,\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        pPhyInfo->nRowCycle > 1 ? ADDR_CYCLE((nRowAddr >> 8) & 0xFF) : NULL_CYCLE,\
        pPhyInfo->nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        512 == pPhyInfo->nBytePerPage ? NULL_CYCLE : CMD_CYCLE(NFLASH_READ_2),\
        bRb ? RB_CYCLE : NULL_CYCLE,
        END_CYCLE);

    return bRet;
}
/**
 * @brief        issue program cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nColumnAddr the coumn address of target column
 * @param[in] nRowAddr the absolute row address of target page
 * @return      T_BOOL
 */
static T_BOOL  op_prog1(T_U16 nColumnAddr, T_U32 nRowAddr, T_NAND_PHYSIC_INFO *pPhyInfo)
{
    return nfc_cycle(CMD_CYCLE(NFLASH_PROG_1),\
        ADDR_CYCLE(nColumnAddr & 0xFF),\
        pPhyInfo->nColCycle > 1 ? ADDR_CYCLE((nColumnAddr >> 8) & 0xFF) : NULL_CYCLE,\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        pPhyInfo->nRowCycle > 1 ? ADDR_CYCLE((nRowAddr >> 8) & 0xFF) : NULL_CYCLE,\
        pPhyInfo->nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        END_CYCLE);
}

/**
 * @brief        issue 0x10 command cycle 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] bRb whether waitting for Ready signal
 * @return      T_BOOL
 */
static T_BOOL  op_prog2(T_BOOL bRb)
{
    return nfc_cycle(CMD_CYCLE(NFLASH_PROG_2),\
        bRb ? RB_CYCLE : NULL_CYCLE,\
        END_CYCLE);
}

/**
 * @brief        check operation result
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_BOOL
 */
static T_BOOL op_result(T_VOID)
{
    T_U8 status;
    
    status =  wait_ready_status();

    if (0 == status)
    {
        akerror(m_aStatusErr, 0, AK_TRUE);
    }
        
    if (NFLASH_STATUS_FAIL_BIT0 == (status & NFLASH_STATUS_FAIL_BIT0))
    {
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }
}

/**
 * @brief           initial Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   pParam, nand infomation struct
 * @param[in]   nChipCnt, chip count detected.
 * @return      T_BOOL
 * @retval  AK_TRUE, success; AK_FALSE, fail
 */
static T_BOOL init_device_info(T_NAND_PARAM *pParam, T_U8 nChipCnt)
{
    T_U8    i, j = 0;
    T_U32   nFlag;
    
    if (0 == nChipCnt)
    {
        return AK_FALSE;//no NANDFLASH
    }

    //while (nChipCnt > (1 << j++));
    nFlag = pParam->flag;
    m_DevInfo.nID[0] = pParam->chip_id;
    m_DevInfo.nID[1] = (0 != (nFlag&FLAG_HIGH_ID)) ?pParam->cmd_len:0;
    m_DevInfo.nChipCnt = nChipCnt;
    m_DevInfo.pPhyInfo = &m_PhyInfo;
        
  //  m_PhyInfo.nTargetShift = j;
//    m_PhyInfo.nTrc  = pParam->data_len / 4200;

    nfc_configtRC(pParam->data_len / 4200, 0);
    m_PhyInfo.nBytePerPage = pParam->page_size;
    m_PhyInfo.nPagePerBlock = pParam->page_per_blk;
    m_PhyInfo.nPagesPerLun = pParam->group_blk_num * pParam->page_per_blk;
//    m_PhyInfo.nBlockPerTarget = pParam->blk_num;
    m_PhyInfo.nLun1StartPage= (0 != (nFlag & FLAG_LUN_GAP) ? m_PhyInfo.nPagesPerLun * 2 : m_PhyInfo.nPagesPerLun);
    
    i = 0;
    while ((1 << ++i) < pParam->page_per_blk);
    m_PhyInfo.nBlockShift = i;
    m_PhyInfo.nColCycle = pParam->col_cycle;
    m_PhyInfo.nRowCycle = pParam->row_cycle;
    //m_PhyInfo.nOobSize = pParam->spare_size + (T_U16)(nFlag & FLAG_SPARE_MASK);//bit 8~9
    m_PhyInfo.nEccType = ECC_TYPE(nFlag);
    
    m_PhyInfo.bRandom = (0 != (nFlag & FLAG_RANDOMIZER)) ? AK_TRUE : AK_FALSE;

    if (m_PhyInfo.bRandom)
    {
        nfc_init_randomizer(m_PhyInfo.nBytePerPage);
    }

#if NAND_SUPPORT_RR>0
   m_PhyInfo.bRetry = (0 != (nFlag & FLAG_READ_RETRY)) ? AK_TRUE : AK_FALSE;
    
    if (m_PhyInfo.bRetry)
    {
        retry_init(nChipCnt, m_DevInfo.nID);
    }
#endif
#if ENHANCED_SLC_PROGRAM > 0
    if (0 != (nFlag & FLAG_ENHANCE_SLC))
    {
        enhance_slc_init(nChipCnt,  m_DevInfo.nID);
    }
#endif
    //no merge
   // m_PhyInfo.nMergeMode = MERGE_NONE;

    m_DevInfo.LogInfo.nLogicBPP = pParam->page_size;
    m_DevInfo.LogInfo.nLogicPPB = pParam->page_per_blk;
    m_DevInfo.LogInfo.nLogicBPC = pParam->blk_num;
    m_DevInfo.LogInfo.nBlockShift = m_PhyInfo.nBlockShift;

    return AK_TRUE;
}

static T_BOOL def_control(E_DEV_OP eOp, T_U8 nArgc, T_VOID *pArgv)
{
    T_BOOL bRet = AK_TRUE;
    
    switch (eOp)
    {
        case OP_INIT:
            {
                bRet = init_device_info((T_NAND_PARAM *)pArgv, nArgc);
                break;
            }
        case OP_GET_ECC:
            {
                nfc_get_ecc((T_NAND_ECC_CTRL **)pArgv, m_PhyInfo.nBytePerPage, m_PhyInfo.nEccType);
                break;
            }
        
#if ENHANCED_SLC_PROGRAM > 0
        #ifdef BURN_TOOL
        case OP_ENABLE_SLC:
            {
                enhance_slc_program(nArgc, *(T_BOOL *)pArgv);
                break;
            }
        #endif
        case OP_GET_LOWER_PAGE:
            {
                enhance_slc_get_lower_page(nArgc, (T_U8 *)pArgv);
                break;
            }
#endif
#if 0
        case OP_CHECK_BAD:
            {
                
                break;
            }
#endif
        default:
            break;
    }
    
    return bRet;
}

/**
 * @brief        erase large pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  def_eraseN(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_U16 nBlockCnt)
{
    T_NAND_PHYSIC_INFO *pPhyInfo  = pDevice->pPhyInfo;
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U32 nRet = 0;
    T_U16 i;

    //get  the NAND physic address according to the logic address
    if (nRow > pPhyInfo->nPagesPerLun)
    {
        nRow -= pPhyInfo->nPagesPerLun;
        nRow = nRow + pPhyInfo->nLun1StartPage;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);
    
    for (i = 0; i < nBlockCnt; i++)
    {
        if (AK_FALSE ==  op_erase(nRow, pPhyInfo, AK_TRUE))
        {
            nRet |= NAND_FAIL_NFC_TIMEOUT;
            break; 
        }

        if (AK_FALSE == op_result())
        {
            nRet |= NAND_FAIL_STATUS;
            break;
        }
                
        nRow += pPhyInfo->nPagePerBlock;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);
    SET_GOODPAGE_CNT(nRet, i);
    return nRet;
}

/**
 * @brief        program large pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  def_progN(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_NAND_PHYSIC_INFO *pPhyInfo  = pDevice->pPhyInfo;
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8 * pMain, *pAdd;
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U32 nRet = 0;
    T_U16   nColumn;
    T_U16   nSectCnt;
    T_U8 i;
    T_U8 j = 0;
    T_U8 nGoodPage = 0;
    
    //get  the NAND physic address according to the logic address
    if (nRow > pPhyInfo->nPagesPerLun)
    {
        nRow -= pPhyInfo->nPagesPerLun;
        nRow = nRow + pPhyInfo->nLun1StartPage;
    }
    
    pMain = pData->pMain;
    pAdd   = pData->pAdd;
    nSectCnt = pData->nSectCnt;
    nColumn = pAddr->nSectAddr; //writes bytes
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);
    
    for (i = 0; i < pData->nPageCnt; i++)
    {
        op_prog1(nColumn, nRow, pPhyInfo);
        
        if ((AK_TRUE == pPhyInfo->bRandom) && (AK_NULL != pEccCtrl))
        {
            nfc_config_randomize(nRow, nColumn, AK_TRUE,  AK_TRUE);
        }
        
        nRet = nfc_write(pMain, pAdd, nSectCnt,  pEccCtrl);

        if ((AK_TRUE == pPhyInfo->bRandom) && (AK_NULL != pEccCtrl))
        {
            nfc_config_randomize(nRow, nColumn, AK_FALSE,  AK_TRUE);
        }
        if (0 != (nRet & NAND_FAIL_MASK))
        {
            break;
        }

        op_prog2(AK_TRUE);

        if (AK_FALSE == op_result())
        {
            nRet |= NAND_FAIL_STATUS;
            goto L_Exit;
        }
        
        if (AK_NULL != pEccCtrl)
        {   
            for (j = 0; j < nSectCnt; j++)
            {
                pMain += pEccCtrl->nMainLen;
            }
  
            pAdd += pEccCtrl->nAddLen;
            nRow++;
        }
        
        nGoodPage++;
   } 
L_Exit:    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);

    SET_GOODPAGE_CNT(nRet, nGoodPage);
    
    return nRet;
}

/**
 * @brief           read large pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  def_readN(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_NAND_PHYSIC_INFO *pPhyInfo  = pDevice->pPhyInfo;
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8 * pMain, *pAdd;
    T_U32 nRet = 0;
    T_U32 nRetTmp;
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U16 nSectAddr;
    T_U16 nColumn;
    T_U16 nSectCnt;
    T_U8 nGoodSect, j = 0;
    T_U8 nGoodPage, nTmp, nMaxFlipBits = 0;
    T_U8 nRetryTimes = 0;
    
    //get  the NAND physic address according to the logic address
    if (nRow > pPhyInfo->nPagesPerLun)
    {
        nRow -= pPhyInfo->nPagesPerLun;
        nRow = nRow + pPhyInfo->nLun1StartPage;
    }
    
    pMain = pData->pMain;
    pAdd   = pData->pAdd;
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);

    for (nGoodPage = 0; nGoodPage < pData->nPageCnt; nGoodPage++)
    {
        nSectAddr = pAddr->nSectAddr;
        nSectCnt = pData->nSectCnt;
        j = 0;
        nGoodSect = 0;
        
        nColumn = get_column_addr(nSectAddr, pEccCtrl);
L_Retry:        
        if (AK_FALSE == op_read(nColumn, nRow, pPhyInfo, AK_TRUE))
        {
            nRet |= NAND_FAIL_NFC_TIMEOUT;
            goto L_EXIT;
        }


        if ((AK_TRUE == pPhyInfo->bRandom) && (AK_NULL != pEccCtrl))
        {
            nfc_config_randomize(nRow, nColumn, AK_TRUE,  AK_FALSE);
        }
        
        nRetTmp = nfc_read(pMain, pAdd,  nSectCnt,  pEccCtrl);

        if ((AK_TRUE == pPhyInfo->bRandom) && (AK_NULL != pEccCtrl))
        {
            nfc_config_randomize(nRow, nColumn, AK_FALSE,  AK_FALSE);
        }

        if (AK_NULL != pEccCtrl)//ecc applyed
        {
            if (AK_NULL != pMain)
            {
                nGoodSect += GET_GOODSECT_CNT(nRetTmp);// the good sectcnt  including the additional data section
                nSectAddr = pAddr->nSectAddr + nGoodSect;
                nSectCnt = pData->nSectCnt - nGoodSect;
                nColumn = get_column_addr(nSectAddr, pEccCtrl);

                while ((j < nGoodSect) & (j < pEccCtrl->nMainSectCnt))
                {
                    j++;
                    pMain += pEccCtrl->nMainLen;
                }
            }
                
            if (0 != (nRetTmp & NAND_FAIL_MASK))//fail some where
            {
                nRetryTimes++;
                nRet |= NAND_WARN_ONCE_FAIL;
                
                if (nRetryTimes > NAND_MAX_RETRY_TIME)
                {
                    nRet |= (nRetTmp & NAND_FAIL_MASK);
                    akerror(m_aReadFailErr,0 ,AK_TRUE);
                    goto L_EXIT;
                }
                
                if (nRetryTimes > NAND_WARN_RETRY_TIME)
                {
                    akerror(m_aReReadInfo, nRetTmp, AK_FALSE);
                    akerror(m_aReReadInfo1, nRow, AK_FALSE); 
                    akerror(m_aReReadInfo2, nColumn, AK_TRUE);
                }
#if NAND_SUPPORT_RR>0
                if (pPhyInfo->bRetry)
                {
                    modify_scales(pAddr->nTargetAddr);
                }
#endif
                goto L_Retry;
            }

            nMaxFlipBits = nMaxFlipBits >  GET_MAXFLIP_CNT(nRetTmp) ? nMaxFlipBits :  GET_MAXFLIP_CNT(nRetTmp);
            pAdd += pEccCtrl->nAddLen;
            nRow++;
        }
    }
L_EXIT:

#if NAND_SUPPORT_RR>0

    if (pPhyInfo->bRetry && (0 != (nRet & NAND_WARN_ONCE_FAIL)))
    {
        revert_scales(pAddr->nTargetAddr);
    }
#endif
    
    nfc_select((T_U8)pAddr->nTargetAddr, AK_FALSE);

    if (nMaxFlipBits < m_aWeakDangerBits[pPhyInfo->nEccType])
    {
        //data in security
    }
    else if (nMaxFlipBits < m_aStrongDangerBits[pPhyInfo->nEccType])
    {
        nRet |= NAND_WARN_WEAK_DANGER;//data in weak danger
    }
    else
    {
        nRet |= NAND_WARN_STRONG_DANGER;//data in strong danger
    }
    
    SET_GOODPAGE_CNT(nRet, nGoodPage);
    SET_GOODSECT_CNT(nRet, nGoodSect);
    return nRet;
}

 /**
  * @brief           register Nandflash
  * @author      Yang Yiming
  * @date        2012-12-25
  * @return      int
  */
int reg_common_nand(void)
{
    //just init the funcs for common device
    m_DevInfo.FunSet.ctrl= def_control;
    m_DevInfo.FunSet.erase= def_eraseN;
    m_DevInfo.FunSet.program= def_progN;
    m_DevInfo.FunSet.read= def_readN;
    
    nand_reg_device((T_NAND_DEVICE_INFO *)&m_DevInfo);
    return 0;
}
 
#endif // ((NAND_LARGE_PAGE > 1) && (NAND_SMALL_PAGE > 1))
#endif //DRV_SUPPORT_NAND > 0

