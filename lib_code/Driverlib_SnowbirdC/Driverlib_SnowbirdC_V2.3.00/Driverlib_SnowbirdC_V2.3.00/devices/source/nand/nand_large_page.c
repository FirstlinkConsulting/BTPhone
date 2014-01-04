#include "anyka_types.h"
#include "arch_nand.h"
#include "nand_retry.h"
#include "drv_cfg.h"
#include "nand_command.h"

#if (DRV_SUPPORT_NAND > 0)

#if (NAND_SMALL_PAGE < 1)

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
    T_U8   nBlockShift;
    T_U8   nEccType;
    T_BOOL bRandom;
    T_BOOL bRetry;
}T_NAND_PHYSIC_INFO;

static T_BOOL init_device_info_large(T_NAND_PARAM *pParam, T_U8 nChipCnt);

#pragma arm section zidata = "_drvbootbss_"
static T_NAND_DEVICE_INFO   m_DevInfo_Large;
static T_NAND_PHYSIC_INFO   m_PhyInfo_Large;
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

//read operation cycle sequence
static const T_U16 m_aReadOps[] = 
{
    CMD_CYCLE(NFLASH_READ_1),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    CMD_CYCLE(NFLASH_READ_2),
    RB_CYCLE,
    END_CYCLE
};

//erase operation cycle sequence
static const T_U16 m_aEraseOps[] = 
{
    CMD_CYCLE(NFLASH_ERASE_1),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    CMD_CYCLE(NFLASH_ERASE_2),
    RB_CYCLE,
    END_CYCLE
};

//program operation cycle sequence
static const T_U16 m_aProgOps[] =
{
    CMD_CYCLE(NFLASH_PROG_1),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    END_CYCLE
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
static T_U8 wait_ready_status_large(T_VOID )
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
 * @brief        modify the row cycles in operation cycle sequence 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   nRow, absolute row address
 * @param[in\out]   pCycle, operation cycles array
 * @return      T_VOID
 */
static T_VOID config_row(T_U32 nRow, T_U8 *pCycle)
{
    pCycle[0] = nRow & 0xFF;
    pCycle[2] = (nRow >> 8)& 0xFF;
    pCycle[4] = (nRow >> 16) & 0xFF;
}

/**
 * @brief        modify the address cycles in operation cycle sequence 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   nColumn,  colum address
 * @param[in]   nRow, absolute row address
 * @param[in\out]   pCycle, operation cycles array
 * @return      T_VOID
 */
static T_VOID config_column_row(T_U16 nColumn, T_U32 nRow, T_U8 *pCycle)
{    
    pCycle[0] = nColumn & 0xFF;
    pCycle[2] = (nColumn >> 8) & 0xFF;
    pCycle[4] = nRow & 0xFF;
    pCycle[6] = (nRow >> 8)& 0xFF;
    pCycle[8] = (nRow >> 16) & 0xFF;
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
static T_U16 get_column_addr_large(T_U16 nSectAddr,
    T_NAND_ECC_CTRL * pEccCtrl)
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

/**
 * @brief        issue erase cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nRowAddr the absolute row address of target block
 * @return      T_BOOL
 */
static T_BOOL op_erase_Large(T_U32 nRowAddr)
{
    config_row(nRowAddr, (T_U8 *)&m_aEraseOps[1]);
    return nfc_cycle(NAND_CYCLE_ARR_FLAG, m_aEraseOps);
}

/**
 * @brief        issue read cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nColumnAddr the coumn address of target column
 * @param[in] nRowAddr the absolute row address of target page
 * @return      T_BOOL
 */
static T_BOOL  op_read_Large(T_U16 nColumnAddr, T_U32 nRowAddr)
{
    config_column_row(nColumnAddr, nRowAddr, (T_U8 *)&m_aReadOps[1]);
    return nfc_cycle(NAND_CYCLE_ARR_FLAG, m_aReadOps);
}

/**
 * @brief        issue program cycle
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nColumnAddr the coumn address of target column
 * @param[in] nRowAddr the absolute row address of target page
 * @return      T_BOOL
 */
static T_BOOL  op_prog1_Large(T_U16 nColumnAddr, T_U32 nRowAddr)
{
    config_column_row(nColumnAddr, nRowAddr, (T_U8 *)&m_aProgOps[1]);
    return nfc_cycle(NAND_CYCLE_ARR_FLAG, m_aProgOps);
}

/**
 * @brief        issue 0x10 command cycle 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] bRb whether waitting for Ready signal
 * @return      T_BOOL
 */
static T_BOOL  op_prog2_Large(T_BOOL bRb)
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
static T_BOOL op_result_Large(T_VOID)
{
    T_U8 status;
    
    status =  wait_ready_status_large();

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
 * @brief        the other operations entry except read, program and erase
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] pDevice device information structure of target device
 * @param[in] pAddr address information structure 
 * @param[in/out] pData data-related information
 * @return      T_NAND_RET
 */
static T_BOOL control_large(E_DEV_OP eOp, T_U8 nArgc, T_VOID *pArgv)
{
    T_BOOL bRet = AK_TRUE;
    
    switch (eOp)
    {
        case OP_INIT:
            {
                bRet = init_device_info_large((T_NAND_PARAM *)pArgv, nArgc);
                break;
            }
        case OP_GET_ECC:
            {
                nfc_get_ecc((T_NAND_ECC_CTRL **)pArgv, m_PhyInfo_Large.nBytePerPage, m_PhyInfo_Large.nEccType);
                break;
            }
    #if ENHANCED_SLC_PROGRAM > 0
            case OP_ENABLE_SLC:
            {
                enhance_slc_program(nArgc, *(T_BOOL *)pArgv);
                break;
            }
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
static T_NAND_RET  erase_large(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_U16 nBlockCnt)
{
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U32 nRet = 0;
    
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Large.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Large.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Large.nLun1StartPage;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);
    
    if (AK_FALSE ==  op_erase_Large(nRow))
    {
        nRet |= NAND_FAIL_NFC_TIMEOUT;
    }

    if (AK_FALSE == op_result_Large())
    {
        nRet |= NAND_FAIL_STATUS;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);
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
static T_NAND_RET  prog_large(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8 *pMain = pData->pMain;
    T_U32 nRow = pAddr->nLogAbsPageAddr;
    T_U32 nRet;
    T_U16   nColumn;

    nColumn = pAddr->nSectAddr; //writes bytes
    
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Large.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Large.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Large.nLun1StartPage;
    }
    
    nfc_select(pAddr->nTargetAddr, AK_TRUE);

    if (AK_FALSE == op_prog1_Large(nColumn, nRow))
	{
        nRet |= NAND_FAIL_NFC_TIMEOUT;
        goto L_Exit;
    }	

    if ((AK_TRUE == m_PhyInfo_Large.bRandom) && (AK_NULL != pEccCtrl))
    {
        nfc_config_randomize(nRow, nColumn, AK_TRUE,  AK_TRUE);
    }
    
    nRet = nfc_write(pData->pMain, pData->pAdd, pData->nSectCnt,  pEccCtrl);

    if ((AK_TRUE == m_PhyInfo_Large.bRandom) && (AK_NULL != pEccCtrl))
    {
        nfc_config_randomize(nRow, nColumn, AK_FALSE,  AK_TRUE);
    }

    if (0 != (nRet & NAND_FAIL_MASK))
    {
        goto L_Exit;
    }
    
    if(AK_FALSE == op_prog2_Large(AK_TRUE))
	{
        nRet |= NAND_FAIL_NFC_TIMEOUT;
        goto L_Exit;
    }

    if (AK_FALSE == op_result_Large())
    {
        nRet |= NAND_FAIL_STATUS;
        goto L_Exit;
    }
    else
    {
        SET_GOODPAGE_CNT(nRet, 1);
    }
    
L_Exit:    
    nfc_select(pAddr->nTargetAddr, AK_FALSE);
    
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
static T_NAND_RET  read_large(T_NAND_DEVICE_INFO *pDevice, 
    T_NAND_ADDR *pAddr, T_NAND_DATA *pData)
{
    T_NAND_ECC_CTRL *pEccCtrl = pData->pEccCtrl;
    T_U8    *pMain;
    T_U32   nRow = pAddr->nLogAbsPageAddr;
    T_U32   nRet = 0;
    T_U32   nRetTmp;
    T_U16   nColumn;
    T_U16   nSectCnt;
    T_U16   nSectAddr;
    T_U8    nGoodSect = 0 , j = 0;
    T_U8    nRetryTimes = 0, nMaxFlipBits = 0;
    
    //get  the NAND physic address according to the logic address
    if (nRow > m_PhyInfo_Large.nPagesPerLun)
    {
        nRow -= m_PhyInfo_Large.nPagesPerLun;
        nRow = nRow + m_PhyInfo_Large.nLun1StartPage;
    }
    
    pMain = pData->pMain;
    nSectCnt = pData->nSectCnt;
    nSectAddr = pAddr->nSectAddr;
    nfc_select(pAddr->nTargetAddr, AK_TRUE);

L_Retry: 
    
    nColumn = get_column_addr_large(nSectAddr, pEccCtrl);
    
    if (AK_FALSE == op_read_Large(nColumn, nRow))
    {
        nRet |= NAND_FAIL_NFC_TIMEOUT;
        goto L_EXIT;
    }

    if ((AK_TRUE == m_PhyInfo_Large.bRandom) && (AK_NULL != pEccCtrl))
    {
        nfc_config_randomize(nRow, nColumn, AK_TRUE,  AK_FALSE);
    }
    
    nRetTmp = nfc_read(pMain, pData->pAdd,  nSectCnt,  pEccCtrl);
        
    if ((AK_TRUE == m_PhyInfo_Large.bRandom) && (AK_NULL != pEccCtrl))
    {
        nfc_config_randomize(nRow, nColumn, AK_FALSE,  AK_FALSE);
    }

    if (AK_NULL != pEccCtrl)//ecc applyed
    {
        if (0 != (nRetTmp & NAND_FAIL_MASK))//fail some where
        {
            if (AK_NULL != pMain)
            {
                nGoodSect += GET_GOODSECT_CNT(nRetTmp);
                nSectAddr = pAddr->nSectAddr + nGoodSect;
                nSectCnt = pData->nSectCnt - nGoodSect;
                
                while ((j < nGoodSect) & (j < pEccCtrl->nMainSectCnt))
                {
                    j++;
                    pMain += pEccCtrl->nMainLen;
                }
            }
            
            nRet |= NAND_WARN_ONCE_FAIL;
            nRetryTimes++;
            if (nRetryTimes > NAND_MAX_RETRY_TIME)
            {
                nRet |= (nRetTmp & NAND_FAIL_MASK);
                akerror(m_aReadFailErr,0 ,1);
                goto L_EXIT;
            }
            else if (nRetryTimes > NAND_WARN_RETRY_TIME)
            {
                akerror(m_aReReadInfo, nRetTmp, AK_FALSE);
                akerror(m_aReReadInfo1, nRow, AK_FALSE); 
                akerror(m_aReReadInfo2, nColumn, AK_TRUE);
            }
#if NAND_SUPPORT_RR > 0
            if (m_PhyInfo_Large.bRetry)
            {
                modify_scales(pAddr->nTargetAddr);
            }
#endif
            goto L_Retry;
        }
        else
        {
            nMaxFlipBits = nMaxFlipBits >  GET_MAXFLIP_CNT(nRetTmp) ? nMaxFlipBits :  GET_MAXFLIP_CNT(nRetTmp);
        }
        
        SET_GOODPAGE_CNT(nRet, 1);
    }
    
L_EXIT:

#if NAND_SUPPORT_RR>0
    if (m_PhyInfo_Large.bRetry && (0 != (nRet & NAND_WARN_ONCE_FAIL)))
    {
        revert_scales(pAddr->nTargetAddr);
    }
#endif
    
    nfc_select((T_U8)pAddr->nTargetAddr, AK_FALSE);

    if (nMaxFlipBits < m_aWeakDangerBits[m_PhyInfo_Large.nEccType] || m_PhyInfo_Large.bRetry)
    {
        //data in security
    }
    else if (nMaxFlipBits < m_aStrongDangerBits[m_PhyInfo_Large.nEccType])
    {
        nRet |= NAND_WARN_WEAK_DANGER;//data in weak danger
    }
    else
    {
        nRet |= NAND_WARN_STRONG_DANGER;//data in strong danger
    }
    
    SET_GOODSECT_CNT(nRet, nGoodSect);
    return nRet;
}

#pragma arm section code

#pragma arm section code = "_drvbootinit_"

/**
 * @brief           initial Nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   pParam, nand infomation struct
 * @param[in]   nChipCnt, chip count detected.
 * @return      T_BOOL
 * @retval  AK_TRUE, success; AK_FALSE, fail
 */
static T_BOOL init_device_info_large(T_NAND_PARAM *pParam, T_U8 nChipCnt)
{
    T_U32   nFlag;
    T_U8  i, j = 0;

    if (0 == nChipCnt)
    {
        return AK_FALSE;//no NANDFLASH
    }

    nFlag = pParam->flag;
    m_DevInfo_Large.nID[0] = pParam->chip_id;
    m_DevInfo_Large.nID[1] = (0 != (nFlag&FLAG_HIGH_ID)) ?pParam->cmd_len:0;
    m_DevInfo_Large.nChipCnt = nChipCnt;
    m_DevInfo_Large.pPhyInfo = &m_PhyInfo_Large;

    //using data_len to calculate a approximation of tRC
    nfc_configtRC(pParam->data_len / 4200, 0);
    m_PhyInfo_Large.nBytePerPage = pParam->page_size;
    m_PhyInfo_Large.nPagePerBlock = pParam->page_per_blk;
    m_PhyInfo_Large.nPagesPerLun = pParam->group_blk_num * pParam->page_per_blk;

    //calculate bockshift
    i = 0;
    while ((1 << ++i) < pParam->page_per_blk);
    m_PhyInfo_Large.nBlockShift = i;
    
    m_PhyInfo_Large.nEccType = ECC_TYPE(nFlag);
    //analyze  whether there's gap between lun0 and lun1
    m_PhyInfo_Large.nLun1StartPage= (0 != (nFlag & FLAG_LUN_GAP) ? 
        m_PhyInfo_Large.nPagesPerLun * 2 : m_PhyInfo_Large.nPagesPerLun);
    //analyze randomizer flag
    m_PhyInfo_Large.bRandom = (0 != (nFlag & FLAG_RANDOMIZER)) ? 
        AK_TRUE : AK_FALSE;

    if (m_PhyInfo_Large.bRandom)
    {
        nfc_init_randomizer(m_PhyInfo_Large.nBytePerPage);
    }
 
#if NAND_SUPPORT_RR>0
    //analyze read retry flag
    m_PhyInfo_Large.bRetry = (0 != (nFlag & FLAG_READ_RETRY)) ? AK_TRUE : AK_FALSE;;

    if (m_PhyInfo_Large.bRetry)
    {
        retry_init(nChipCnt, m_DevInfo_Large.nID);
    }
#endif
#if ENHANCED_SLC_PROGRAM > 0
    //analyze enhance slc flag
    if (0 != (nFlag & FLAG_ENHANCE_SLC))
    {
        enhance_slc_init(nChipCnt,  m_DevInfo_Large.nID);
    }
#endif
 
    //no merge
    // m_PhyInfo_Large.nMergeMode = MERGE_NONE;
 
    m_DevInfo_Large.LogInfo.nLogicBPP = pParam->page_size;
    m_DevInfo_Large.LogInfo.nLogicPPB = pParam->page_per_blk;;
    m_DevInfo_Large.LogInfo.nLogicBPC = pParam->blk_num;
    m_DevInfo_Large.LogInfo.nBlockShift = m_PhyInfo_Large.nBlockShift;
 
    return AK_TRUE;
}

 /**
  * @brief           register large pagesize Nandflash
  * @author      Yang Yiming
  * @date        2012-12-25
  * @return      int
  */
 int reg_large_nand(void)
{
    //just init the funcs for common device
    m_DevInfo_Large.FunSet.ctrl = control_large;
    m_DevInfo_Large.FunSet.erase = erase_large;
    m_DevInfo_Large.FunSet.program = prog_large;
    m_DevInfo_Large.FunSet.read = read_large;
    nand_reg_device((T_NAND_DEVICE_INFO *)&m_DevInfo_Large);
    return 0;
}
#pragma arm section code

#endif
#endif //DRV_SUPPORT_NAND > 0

