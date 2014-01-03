/**@file arch_nand.h
 * @brief AK322x nand controller
 *
 * This file describe how to control the AK322x nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan, chenyanyan
 * @date    2007-1-10
 * @version 1.0
 */
#ifndef __ARCH_NAND_H__
#define __ARCH_NAND_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "anyka_types.h"
#include "nand_list.h"

#define NAND_DEVICE_NUM 2

#define NFC_SUPPORT_CHIPNUM        4

#define NAND_MAX_ADD_LEN    8

typedef enum
{
    COMMAND_C = 1,
    ADDRESS_C,
    READYB_C,
    DELAY_C,
    NULL_C,
    END_C,
    WDATA_C,
    ARRAY_C
}NAND_CYLCE_TYPE;

#define NAND_CYCLE_FLAG_POS        (8)
#define NAND_CYCLE_FLAG_MASK       (0xFF << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_VALUE_MASK      (0xFF)
#define GET_NAND_CYCLY_TYPE(n)      ((n >> NAND_CYCLE_FLAG_POS) & 0xFF)
#define GET_NAND_CYCLY_VALUE(n)     (n & NAND_CYCLE_VALUE_MASK)
#define NAND_CYCLE_CMD_FLAG         (COMMAND_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_ADDR_FLAG       (ADDRESS_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_RB_FLAG           (READYB_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_DELAY_FLAG     (DELAY_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_NULL_FLAG        (NULL_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_END_FLAG          (END_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_WDATA_FLAG       (WDATA_C << NAND_CYCLE_FLAG_POS)
#define NAND_CYCLE_ARR_FLAG         (ARRAY_C << NAND_CYCLE_FLAG_POS)

#define CMD_CYCLE(x)            (NAND_CYCLE_CMD_FLAG | x)
#define ADDR_CYCLE(x)           (NAND_CYCLE_ADDR_FLAG | x)
#define RB_CYCLE                   NAND_CYCLE_RB_FLAG
#define DELAY_CYCLE(x)          (NAND_CYCLE_DELAY_FLAG | x)
#define NULL_CYCLE              NAND_CYCLE_NULL_FLAG
#define END_CYCLE               NAND_CYCLE_END_FLAG
#define WDATA_CYCLE(x)          (NAND_CYCLE_WDATA_FLAG | x)

#define NAND_FAIL_STATUS          (1UL << 31)
#define NAND_FAIL_NFC_TIMEOUT     (1 << 30)
#define NAND_FAIL_PARAM           (1 << 29)
#define NAND_FAIL_ECC             (1 << 28)
#define NAND_WARN_ONCE_FAIL       (1 << 27)
#define NAND_WARN_STRONG_DANGER   (1 << 26)
#define NAND_WARN_WEAK_DANGER     (1 << 25)
#define NAND_GOODPAGE_POS         (16)
#define NAND_MAXFLIP_POS          (8)
#define NAND_GOODSECT_POS         (0)
#define NAND_FAIL_MASK            (NAND_FAIL_STATUS |NAND_FAIL_NFC_TIMEOUT | NAND_FAIL_PARAM | NAND_FAIL_ECC)
#define NAND_WARN_MASK            (NAND_WARN_ONCE_FAIL | NAND_WARN_STRONG_DANGER | NAND_WARN_WEAK_DANGER)
#define NAND_FLIPBIT_MASK         0xFF

#define SET_GOODSECT_CNT(nRet,nSet)     (nRet |= (nSet << NAND_GOODSECT_POS))

#define GET_GOODSECT_CNT(nRet)          ((nRet >> NAND_GOODSECT_POS) & 0xFF)

#define SET_MAXFLIP_CNT(nRet,nFlip)      (nRet |= (nFlip << NAND_MAXFLIP_POS))

#define GET_MAXFLIP_CNT(nRet)           ((nRet >> NAND_MAXFLIP_POS) & 0xFF)

#define SET_GOODPAGE_CNT(nRet,nPage)    (nRet |= (nPage << NAND_GOODPAGE_POS))

typedef T_U32 T_NAND_RET;

#define NAND_RR_INFO        "NF_RR"
#define NAND_RANDOMIZER_INFO "NF_RAND"
#define NAND_DATA_LEN_INFO  "NF_DL:"
#define NAND_REREAD_INFO    "NF_R F: "
#define NAND_REREAD_INFO1   " R:"
#define NAND_REREAD_INFO2   " C:"
#define NAND_STATUS_ERROR   "NF_S:"
#define NAND_READ_FAIL      "NF_F"
#define NAND_NFC_TIMEOUT    "NF_CT!"
#define NAND_ECC_TIMEOUT    "NF_ET!"
#define NAND_ECC_ERROR      "NF_EE:"

typedef enum
{
    OP_INIT,
    OP_GET_ECC,
    OP_ENABLE_SLC,
    OP_GET_LOWER_PAGE,
    OP_CHECK_BAD
}E_DEV_OP;

typedef struct NAND_ECC_CTL
{
    T_U16 nMainLen;
    T_U16 nSectOffset;
    T_U8   nMainEcc;
    T_U8   nAddEcc;
    T_U8   nAddLen;//fixed 8 bytes?
    T_BOOL   bSeperated;
    T_U8    nMainSectCnt;
}T_NAND_ECC_CTRL;

typedef struct NAND_ADDR
{
    T_U16 nSectAddr;
    T_U16 nTargetAddr;
    T_U32 nLogAbsPageAddr;
}T_NAND_ADDR;

typedef struct NAND_DATA
{
    T_U16    nSectCnt;//read bytes biaoshi ×Ö½ÚÊý
    T_U16    nPageCnt;
    T_U8    *pMain;
    T_U8    *pAdd;
    T_NAND_ECC_CTRL *pEccCtrl;
}T_NAND_DATA;

typedef struct  NAND_DEVICE_INFO T_NAND_DEVICE_INFO;
typedef T_BOOL (*f_device_ctrl)(E_DEV_OP eOp, T_U8 nArgc, T_VOID * pArgv);
typedef T_NAND_RET  (*f_eraseN)(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_U16 nBlockCnt);
typedef T_NAND_RET  (*f_progN)(T_NAND_DEVICE_INFO *pDevice, T_NAND_ADDR *pAddr, T_NAND_DATA *pData);
typedef T_NAND_RET  (*f_readN)(T_NAND_DEVICE_INFO * pDevice, T_NAND_ADDR * pAddr, T_NAND_DATA * pData);

typedef struct NAND_FUNCTIONS
{
    f_device_ctrl  ctrl ;
    f_eraseN erase;
    f_progN program;
    f_readN read;
}T_NAND_FUNCTIONS;

typedef struct NAND_LOGIC_INFO
{
    T_U16   nLogicBPP;
    T_U16   nLogicPPB;
    T_U16   nLogicBPC;
    T_U8    nBlockShift;
    T_U8    nPageShift;
}T_NAND_LOGIC_INFO;

struct NAND_DEVICE_INFO
{
    T_U32   nID1234;
    T_U8    nChipCnt;
    T_NAND_FUNCTIONS FunSet;
    T_NAND_LOGIC_INFO LogInfo;
    T_NAND_ECC_CTRL   **ppEccCtrl;    
    T_VOID  *pPhyInfo;
};

T_VOID nand_setCe_getID(T_U32 *pChipID, T_U32 *pChipCnt, T_U8 *pCePos, T_U32 nCeCnt);

T_NAND_DEVICE_INFO* nand_get_device(T_NAND_PARAM *pNandParam, T_U32 nChipCnt);

T_BOOL nand_reg_device(T_NAND_DEVICE_INFO *pDevice);

T_VOID  nfc_init(T_U8 *pCePos, T_U8 nCeCnt);

T_BOOL nfc_init_randomizer(T_U32 nRandPageSize);

T_VOID nfc_config_randomize(T_U16 nPageAddr, T_U16 nColumnAddr, T_BOOL bEnable, T_U8 bWrite);

T_BOOL nfc_select(T_U8 nTarget, T_BOOL bSelect);

T_VOID nfc_configtRC(T_U8 nTrc, T_U8 nDelay);

T_VOID  nfc_timing_adjust(T_U32 nAsic);

T_BOOL nfc_cycle(T_U32 nCmdSeq,...);

T_BOOL nfc_waitstatus(T_U8 nStatuscmd, T_U8 nExpectbits);

T_VOID nfc_get_ecc(T_NAND_ECC_CTRL **ppEccCtrl, T_U32 nPageSize, T_U8 nEccType);

T_U32 nfc_write(T_U8  *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl);

T_U32 nfc_read(T_U8  *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif //__ARCH_NAND_H__
