#include "anyka_types.h"
#include "arch_nand.h"
#include "nand_retry.h"
#include "drv_cfg.h"
#include "nand_command.h"

#if (DRV_SUPPORT_NAND > 0)

#if (NAND_LARGE_PAGE < 1)

#define NAND_MAX_RETRY_TIME  15
#define NAND_WARN_RETRY_TIME    8

//when flip bits are more than the following defines, be careful of the data
#define WEAK_DANGER_BIT_NUM_TYPE0 3  //4 bit nand ecc's weak danger flip bit number among 512 Bytes
#define WEAK_DANGER_BIT_NUM_TYPE1 5  //8 bit nand ecc's weak danger flip bit number among 512 Bytes

//when flip bits are more than the following defines, don't use current block any more
#define STRONG_DANGER_BIT_NUM_TYPE0 4  //4 bit nand ecc's strong danger flip bit number among 512 Bytes
#define STRONG_DANGER_BIT_NUM_TYPE1 7  //8 bit nand ecc's strong danger flip bit number among 512 Bytes

typedef struct NAND_PHYSIC_INFO
{
    T_U32  nPagesPerLun;
    T_U32  nLun1StartPage;
    T_U16  nPagePerBlock;
    T_U16  nBytePerPage;
    T_U8   nBlockShift;
    T_U8   nEccType;
    T_U8   nRowCycle;
}T_NAND_PHYSIC_INFO;

#pragma arm section zidata = "_drvbootbss_"
static T_NAND_DEVICE_INFO   m_DevInfo_Small;
static T_NAND_PHYSIC_INFO   m_PhyInfo_Small;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 m_aReReadInfo[] = NAND_REREAD_INFO;
static const T_U8 m_aReReadInfo1[] = NAND_REREAD_INFO1;
static const T_U8 m_aStatusErr[] = NAND_STATUS_ERROR;
static const T_U8 m_aReadFailErr[] = NAND_READ_FAIL;

//the experiential value for strong danger in each ecc type 
static const T_U8 m_aStrongDangerBits[] =
{
    STRONG_DANGER_BIT_NUM_TYPE0,
    STRONG_DANGER_BIT_NUM_TYPE1,
};

//the experiential value for weak danger in each ecc type 
static const T_U8 m_aWeakDangerBits[] =
{
    WEAK_DANGER_BIT_NUM_TYPE0,
    WEAK_DANGER_BIT_NUM_TYPE1,
};

#pragma arm section rodata
#pragma arm section code = "_drvbootcode_" 

/**
 * @brief        read nandflash status register
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_U8
 * @retval      the status value
 */
T_U8 wait_ready_status_small(T_VOID )
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
 * @brief        issue erase cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nRowAddr the absolute row address of target block
 * @return      T_BOOL
 */
static T_BOOL op_erase_small(T_U32 nRowAddr)
{
    return nfc_cycle(CMD_CYCLE(NFLASH_ERASE_1),\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        ADDR_CYCLE((nRowAddr >> 8) & 0xFF),\
        m_PhyInfo_Small.nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        CMD_CYCLE(NFLASH_ERASE_2),\
        RB_CYCLE,\
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
static T_BOOL  op_read_small(T_U16 nColumnAddr, T_U32 nRowAddr)
{
    T_BOOL   bRet = AK_FALSE;
    T_U8    nCmdValue = NFLASH_READ_1;

    //to read large page or the small page when ColumnAddr < 256
    if (nColumnAddr >= 512)
    {
        nCmdValue = NFLASH_READ22;
    }
    else   if (nColumnAddr >= 256)
    {
        nCmdValue = NFLASH_READ1_HALF;
    }

    bRet = nfc_cycle(CMD_CYCLE(nCmdValue),\
        ADDR_CYCLE(nColumnAddr & 0xFF),\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        ADDR_CYCLE((nRowAddr >> 8) & 0xFF),\
        m_PhyInfo_Small.nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        RB_CYCLE,
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
static T_BOOL  op_prog1_small(T_U16 nColumnAddr, T_U32 nRowAddr)
{
    return nfc_cycle(CMD_CYCLE(NFLASH_PROG_1),\
        ADDR_CYCLE(nColumnAddr & 0xFF),\
        ADDR_CYCLE(nRowAddr & 0xFF),\
        ADDR_CYCLE((nRowAddr >> 8) & 0xFF),\
        m_PhyInfo_Small.nRowCycle > 2 ? ADDR_CYCLE((nRowAddr >> 16) & 0xFF) : NULL_CYCLE,\
        END_CYCLE);
}

/**
 * @brief        issue 0x10 command cycle 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] bRb whether waitting for Ready signal
 * @return      T_BOOL
 */
static T_BOOL  op_prog2_small(T_BOOL bRb)
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
static T_BOOL op_result_small(T_VOID)
{
    T_U8 status;
    
    status =  wait_ready_status_small();

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
static T_BOOL init_device_info_small(T_NAND_PARAM *pParam, T_U8 nChipCnt)
{
    T_U8    i, j = 0;
    T_U32   nFlag;
    
    if (0 == nChipCnt)
    {
        return AK_FALSE;//no NANDFLASH
    }
    
    nFlag = pParam->flag;
    m_DevInfo_Small.nID[0] = pParam->chip_id;
    m_DevInfo_Small.nID[1] = (0 != (nFlag&FLAG_HIGH_ID))?pParam->cmd_len:0;
    m_DevInfo_Small.nChipCnt = nChipCnt;
    m_DevInfo_Small.pPhyInfo = &m_PhyInfo_Small;
        
    nfc_configtRC(pParam->data_len / 4200, 0);
    m_PhyInfo_Small.nBytePerPage = pParam->page_size;
    m_PhyInfo_Small.nPagePerBlock = pParam->page_per_blk;
    m_PhyInfo_Small.nPagesPerLun = pParam->group_blk_num * pParam->page_per_blk;
    m_PhyInfo_Small.nLun1StartPage= (0 != (nFlag & FLAG_LUN_GAP) ? m_PhyInfo_Small.nPagesPerLun * 2 : m_PhyInfo_Small.nPagesPerLun);
    
    i = 0;
    while ((1 << ++i) < pParam->page_per_blk);
    m_PhyInfo_Small.nBlockShift = i;
    m_PhyInfo_Small.nRowCycle = pParam->row_cycle;
    //m_PhyInfo_Small.nOobSize = pParam->spare_size + (T_U16)(nFlag & FLAG_SPARE_MASK);//bit 8~9
    m_PhyInfo_Small.nEccType = ECC_TYPE(nFlag);
    

    //no merge
   // m_PhyInfo_Small.nMergeMode = MERGE_NONE;

    m_DevInfo_Small.LogInfo.nLogicBPP = pParam->page_size;
    m_DevInfo_Small.LogInfo.nLogicPPB = pParam->page_per_blk;;
    m_DevInfo_Small.LogInfo.nLogicBPC = pParam->blk_num;
    m_DevInfo_Small.LogInfo.nBlockShift = m_PhyInfo_Small.nBlockShift;

    return AK_TRUE;
}

/**
 * @brief        the other operations entry except read, program and erase
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_BOOL control_small(E_DEV_OP eOp, T_U8 nArgc, T_VOID *pArgv)
{
    T_BOOL bRet = AK_TRUE;
    
    switch (eOp)
    {
        case OP_INIT:
            {
                bRet = init_device_info_small((T_NAND_PARAM *)pArgv, nArgc);
                break;
            }
        case OP_GET_ECC:
            {
                nfc_get_ecc((T_NAND_ECC_CTRL **)pArgv, m_PhyInfo_Small.nBytePerPage, m_PhyInfo_Small.nEccType);
                break;
            }
#if 0
        case OP_CHECK_BAD:
            {
                
                break;
            }
#endif
    }
    
    return bRet;
}
/**
 * @brief        erase small pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  eraseN_small(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_U16 nBlockCnt)
{
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U32 nRet = 0;
    T_U8 i;
    
   nfc_select(pAddr->nTargetAddr, AK_TRUE);
#if 0
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Small.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Small.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Small.nLun1StartPage;
    }
#endif 
    for (i = 0; i < nBlockCnt; i++)
    {
        if (AK_FALSE == op_erase_small(nRow))
        {
            nRet |= NAND_FAIL_NFC_TIMEOUT;
            break; 
        }

        if (AK_FALSE == op_result_small())
        {
            nRet |= NAND_FAIL_STATUS;
            break;
        }
                
        nRow += m_PhyInfo_Small.nPagePerBlock;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);
    return nRet;
}

/**
 * @brief        program small pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  progN_small(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8 * pMain, *pAdd;
    T_U32 nRet = 0;
    T_U8 nGoodPage;
#if 0
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Small.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Small.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Small.nLun1StartPage;
    }
#endif

    pMain = pData->pMain;
    pAdd = pData->pAdd;

    nfc_select(pAddr->nTargetAddr, AK_TRUE);
    
    for (nGoodPage = 0; nGoodPage < pData->nPageCnt; nGoodPage++)
    {
        op_prog1_small(pAddr->nSectAddr, nRow);
        nRet = nfc_write(pMain, pAdd, pData->nSectCnt, pEccCtrl);

        if (0 != (nRet & NAND_FAIL_MASK))
        {
            break;
        }

        op_prog2_small(AK_TRUE);

        if (AK_FALSE == op_result_small())
        {
            nRet |= NAND_FAIL_STATUS;
            goto L_Exit;
        }
        
        if (AK_NULL != pEccCtrl)
        {
            pMain += pEccCtrl->nMainLen;
            pAdd  += pEccCtrl->nAddLen;
            nRow++;
        }
        
   } 
    
L_Exit:    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);
    SET_GOODPAGE_CNT(nRet, nGoodPage);
    
    return nRet;
}

/**
 * @brief           read small pagesize Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_NAND_RET  readN_small(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8 nGoodPage, nMaxFlipBits = 0;
    T_U8 * pMain, *pAdd;
    T_U8 nRetryTimes = 0;
    T_U32 nRet = 0;
    T_U32 nRetTmp;
    
    pMain = pData->pMain;
    pAdd   = pData->pAdd;
    #if 0
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Small.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Small.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Small.nLun1StartPage;
    }
    #endif
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);

    for (nGoodPage = 0; nGoodPage < pData->nPageCnt; nGoodPage++)
    {
L_Retry:        
    if (AK_FALSE == op_read_small(pAddr->nSectAddr, nRow))
        {
            nRet |= NAND_FAIL_NFC_TIMEOUT;
            goto L_EXIT;
        }
        
        nRetTmp = nfc_read(pMain, pAdd, pData->nSectCnt,  pEccCtrl);

        if (AK_NULL != pEccCtrl)//ecc applyed
        {                
            if (0 != (nRetTmp & NAND_FAIL_MASK))//fail some where
            {
                nRetryTimes++;
                nRet |= NAND_WARN_ONCE_FAIL;
                
                if (nRetryTimes > NAND_MAX_RETRY_TIME)
                {
                    nRet |= (nRetTmp & NAND_FAIL_MASK);
                    akerror(m_aReadFailErr,0 ,1);
                    goto L_EXIT;
                }
                else if (nRetryTimes > NAND_WARN_RETRY_TIME)
                {
                    akerror(m_aReReadInfo, nRetTmp , AK_FALSE);
                    akerror(m_aReReadInfo1, nRow, AK_TRUE); 
                }
                
                goto L_Retry;
            }
            
            if (AK_NULL != pMain)
            {
                pMain += pEccCtrl->nMainLen;
            }
            
            nRow++;
            pAdd += pEccCtrl->nAddLen;
            nMaxFlipBits = nMaxFlipBits >  GET_MAXFLIP_CNT(nRetTmp) ? nMaxFlipBits :  GET_MAXFLIP_CNT(nRetTmp);
        }
    }
L_EXIT:

    nfc_select((T_U8)pAddr->nTargetAddr, AK_FALSE);

    if (nMaxFlipBits < m_aWeakDangerBits[m_PhyInfo_Small.nEccType])
    {
        //data in security
    }
    else if (nMaxFlipBits < m_aStrongDangerBits[m_PhyInfo_Small.nEccType])
    {
        nRet |= NAND_WARN_WEAK_DANGER;//data in weak danger
    }
    else
    {
        nRet |= NAND_WARN_STRONG_DANGER;//data in strong danger
    }
    
    SET_GOODPAGE_CNT(nRet, nGoodPage);
    return nRet;
}

 /**
  * @brief           register small pagesize Nandflash
  * @author      Yang Yiming
  * @date        2012-12-25
  * @return      int
  */
 int reg_small_nand(void)
{
    //just init the funcs for common device
    m_DevInfo_Small.FunSet.ctrl = control_small;
    m_DevInfo_Small.FunSet.erase = eraseN_small;
    m_DevInfo_Small.FunSet.program = progN_small;
    m_DevInfo_Small.FunSet.read = readN_small;
    
    nand_reg_device((T_NAND_DEVICE_INFO *)&m_DevInfo_Small);
    return 0;
}
#pragma arm section code

#endif
#endif //DRV_SUPPORT_NAND > 0

