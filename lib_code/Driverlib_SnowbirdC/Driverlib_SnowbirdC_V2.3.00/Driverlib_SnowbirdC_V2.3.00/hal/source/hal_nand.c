#include <string.h>
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_command.h"

#if DRV_SUPPORT_NAND > 0

#define NAND_ID_LEN 8

#ifdef BURN_TOOL
static T_NAND_DEVICE_INFO   *NAND_INFO_TABLE[NAND_DEVICE_NUM];
#else
#pragma arm section zidata = "_drvbootbss_"
static T_NAND_DEVICE_INFO   *m_pNand_Dev;
#pragma arm section zidata

#endif
#pragma arm section code = "_drvbootinit_"
static __inline T_VOID reg_nand()
{
#if ((NAND_LARGE_PAGE > 0) && (NAND_SMALL_PAGE > 0))
        //register a common device here
        reg_common_nand();
#else
    #if (NAND_LARGE_PAGE > 0)
            reg_large_nand();
    #endif
    #if  (NAND_SMALL_PAGE > 0)
            reg_small_nand();
    #endif
#endif
}

/**
 * @brief   set gpios as nand Ce pin, and get the chip id and chip count detected
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [out]pChipID the ID of nandflash detected
 * @param       [out]pChipCnt the amount of nandflash detected
 * @param       [in]pCePos the GPIOs connected to nandflash Ce, 0xFF for default pin 
 * @param       [in]nCeCnt the GPIOs amount to be set, not more than 4
 * @return      T_VOID
 */
T_VOID nand_setCe_getID(T_U32 pChipID[2], T_U32 *pChipCnt, T_U8 *pCePos, T_U32 nCeCnt)
{
    T_U32 i;
    T_U32 aID[NAND_ID_LEN / 4];

    nfc_init(pCePos, nCeCnt);

    for (i = 0; i < nCeCnt; i++)
    {
        nfc_select(i, AK_TRUE);
                
        nfc_cycle(CMD_CYCLE(NFLASH_RESET), DELAY_CYCLE(240), END_CYCLE);
        nfc_cycle(CMD_CYCLE(NFLASH_ID), ADDR_CYCLE(0), DELAY_CYCLE(8), END_CYCLE);        
        nfc_read((T_U8*)aID, AK_NULL, NAND_ID_LEN, AK_NULL);
        
        nfc_select(i, AK_FALSE);
        
        if (0xFFFFFFFF == aID[0])
        {
            break;
        }
        
        pChipID[0] =  aID[0];
        pChipID[1] =  aID[1]&0xffff;
    }
    
    *pChipCnt = i;
  
}

/**
 * @brief       initial the nandflash with pNandParam and get T_NAND_DEVICE_INFO
 *                  function nand_setCe_getID  should be called before calling this
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pNandParam a anyka nandflash param structure 
 * @param       [in]nCeCnt the chipcnt to be initialed, not more than 4
 * @return      T_NAND_DEVICE_INFO
 */
T_NAND_DEVICE_INFO* nand_get_device(T_NAND_PARAM *pNandParam, T_U32 nChipCnt)
{
    T_NAND_DEVICE_INFO * pDevInfo;

    reg_nand();

#ifdef BURN_TOOL
{
    T_U32 i;

    pDevInfo =  NAND_INFO_TABLE[0];//the default device

    for (i = 1; i < NAND_DEVICE_NUM; i++) //try to mach the id first
    {
        if (NAND_INFO_TABLE[i]->nID[0] == pNandParam->chip_id)
        {            
            if((0 != (pNandParam->flag&FLAG_HIGH_ID)) &&
                NAND_INFO_TABLE[i]->nID[1] ==pNandParam->cmd_len)
            {
                pDevInfo =  NAND_INFO_TABLE[i];
                break;
            }
        }
    }
}
#else

    pDevInfo =  m_pNand_Dev;
#endif

    if (pDevInfo->FunSet.ctrl(OP_INIT, nChipCnt, (T_VOID *)pNandParam))
    {
        return pDevInfo;
    }
    else
    {
        return AK_NULL;
    }
}

/**
 * @brief       register a nand device
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pDevice a structure including the chip ID etc.
 * @return      T_BOOL
 */
T_BOOL nand_reg_device(T_NAND_DEVICE_INFO *pDevice)
{

#ifdef BURN_TOOL

    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < NAND_DEVICE_NUM; i++)
    {
        if (AK_NULL == NAND_INFO_TABLE[i])
        {
            NAND_INFO_TABLE[i] = pDevice;
            ret = AK_TRUE;
            break;
        }
    }/////////////
#else
    m_pNand_Dev = pDevice;
#endif
    //printf("Register Device %d\n", i);
    return AK_TRUE;
}
#pragma arm section code

#endif
