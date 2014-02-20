#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0

#if (NAND_RR_H27UCG8T2A > 0) || (NAND_RR_H27UCG8T2C > 0)
#define HYNIX_RETRY_TIMES_20NM 8
#define HYNIX_RETRY_REGCNT_20NM 8
//hynix read retrial command
#define NFLASH_RETRY_GET_PARAM_HYNIX    0x37  
#define NFLASH_RETRY_SET1_PARAM_HYNIX   0x36
#define NFLASH_RETRY_SET2_PARAM_HYNIX   0x16

#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_nScale;  
static T_U8 s_a3RegVal[DRV_SUPPORT_NAND][HYNIX_RETRY_TIMES_20NM][HYNIX_RETRY_REGCNT_20NM];

static const T_U8* s_aReg;

#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 s_aScaleInfo[]= "S ";

#if (NAND_RR_H27UCG8T2A > 0)
//hynix 20nm 64Gb MLC RawNAND A die
static const T_U8 s_aReg_20nm_Adie[HYNIX_RETRY_REGCNT_20NM] =
{
    0xCC, 0xBF, 0xAA, 0xAB,
    0xCD, 0xAD, 0xAE, 0xAF
};
//pre cycle sequence to get read-retry table
static const T_U16 s_20nm_64gb_AdieOps[] = 
{
    CMD_CYCLE(0x36),
    ADDR_CYCLE(0xFF),
    WDATA_CYCLE(0x40),
    ADDR_CYCLE(0xCC),
    WDATA_CYCLE(0x4D),
    CMD_CYCLE(0x16),
    CMD_CYCLE(0x17),
    CMD_CYCLE(0x04),
    CMD_CYCLE(0x19),
    CMD_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x02),
    ADDR_CYCLE(0x00),
    CMD_CYCLE(0x30),
    RB_CYCLE,
    END_CYCLE
};
#endif

#if  0//NAND_RR_H27UCG8T2C>0
//hynix 20nm 64Gb MLC RawNAND C die
static const T_U8 s_aReg_20nm_64gb_Cdie[HYNIX_RETRY_REGCNT_20NM] =
{
    0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7
};
static const T_U16 s_20nm_64gb_CdieOps[] = 
{
    CMD_CYCLE(0x36),
    ADDR_CYCLE(0xAE),
    WDATA_CYCLE(0x00),
    ADDR_CYCLE(0xB0),
    WDATA_CYCLE(0x4D),
    CMD_CYCLE(0x16),
    CMD_CYCLE(0x17),
    CMD_CYCLE(0x04),
    CMD_CYCLE(0x19),
    CMD_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x02),
    ADDR_CYCLE(0x00),
    CMD_CYCLE(0x30),
    RB_CYCLE,
    END_CYCLE
};
#endif
#if  NAND_RR_H27UBG8T2C>0
//hynix 20nm 32Gb MLC RawNAND C die
static const T_U8 s_aReg_20nm_32gb_Cdie[HYNIX_RETRY_REGCNT_20NM] =
{
    0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7
};
static const T_U16 s_20nm_32gb_CdieOps[] = 
{
    CMD_CYCLE(0x36),
    ADDR_CYCLE(0xAE),
    WDATA_CYCLE(0x00),
    ADDR_CYCLE(0xB0),
    WDATA_CYCLE(0x4D),
    CMD_CYCLE(0x16),
    CMD_CYCLE(0x17),
    CMD_CYCLE(0x04),
    CMD_CYCLE(0x19),
    CMD_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x00),
    ADDR_CYCLE(0x02),
    ADDR_CYCLE(0x00),
    CMD_CYCLE(0x30),
    RB_CYCLE,
    END_CYCLE
};
#endif

#pragma arm section rodata

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief set retry param of hynix 2x nm level nand.
 * @author Yang Yiming
 * @date 2011-09-16
 * @param[in] Chip which chip to be set.
 * @param[in] l2_buf_id l2 id, use the same l2 buffer as nand_read fun
 * @return  0
 */
T_VOID hynix_set_scales_20nm(T_U8 nTarget)
{
    if (HYNIX_RETRY_TIMES_20NM == s_nScale)
        s_nScale = 0;
    
    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_SET1_PARAM_HYNIX),\
        ADDR_CYCLE(s_aReg[0]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][0]),\
        ADDR_CYCLE(s_aReg[1]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][1]),\
        ADDR_CYCLE(s_aReg[2]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][2]),\
        ADDR_CYCLE(s_aReg[3]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][3]),\
        ADDR_CYCLE(s_aReg[4]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][4]),\
        ADDR_CYCLE(s_aReg[5]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][5]),\
        ADDR_CYCLE(s_aReg[6]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][6]),\
        ADDR_CYCLE(s_aReg[7]),\
        WDATA_CYCLE(s_a3RegVal[nTarget][s_nScale][7]),\
        CMD_CYCLE(NFLASH_RETRY_SET2_PARAM_HYNIX),
        END_CYCLE);
    
}

 /**
 * @brief        modify read retry register 
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTarget where ecc error occured
 * @return      T_VOID
 */
T_VOID hynix_modify_scales_20nm(T_U8 nTarget)
{
    //shift to the next scale
    s_nScale++;
    hynix_set_scales_20nm(nTarget);
}

/**
 * @brief   revert read retry register 
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTargetCnt the target amount in a device
 * @return  T_VOID
 */
T_VOID hynix_revert_scales_20nm(T_U8 nTarget)
{
    s_nScale = 0;
    hynix_set_scales_20nm(nTarget);
    //shift to the 3rd scale, every read-retry operation succeeds 
    //in the 3rd scale
    s_nScale = 2;
}
#pragma arm section code
#pragma arm section code = "_drvbootinit_"

/**
 * @brief    match the read-retry register and offset table
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] nID the lower 1~4th id
 * @return      T_BOOL
 */
T_VOID hynix_retry_init_20nm(T_U8 nChipCnt, T_U32 nID[2])
{
    T_U8 i, nTarget;
    T_U8 *pRegVal;
    T_U8  nTmp;
    T_BOOL   bAcpt;
    T_U8  nSet = 0;
    T_U16 const *pOps = AK_NULL;    
    s_nScale = 0;
        
#if (NAND_RR_H27UCG8T2A > 0 ) 
    if (IDL_Hynix_20nm_64gb_Adie == nID[0] )
    {
        s_aReg = s_aReg_20nm_Adie;
        pOps = s_20nm_64gb_AdieOps;
    }else
#endif
    
#if 0//((NAND_RR_H27UCG8T2C > 0))
    if (IDL_Hynix_20nm_64gb_Cdie == nID[0] && IDL_Hynix_20nm_64gb_Cdie_HI == nID[1])
    {
        s_aReg = s_aReg_20nm_64gb_Cdie;
        pOps = s_20nm_64gb_CdieOps;
    }else
#endif    
    
#if (NAND_RR_H27UBG8T2C > 0)
    if (IDL_Hynix_20nm_32gb_Cdie == nID[0])
    {
        s_aReg = s_aReg_20nm_32gb_Cdie;
        pOps = s_20nm_32gb_CdieOps;
    }else
#endif 
    {
        //default
    }

    for (nTarget = 0; nTarget < nChipCnt; nTarget++)
    {
        bAcpt = AK_FALSE;
        pRegVal = (T_U8 *)&s_a3RegVal[nTarget][0][0];
        
        nfc_select(nTarget, AK_TRUE);
        //pre command sequence
        nfc_cycle(NAND_CYCLE_ARR_FLAG, pOps);
        //discard the first two byte 
        nfc_read(pRegVal, AK_NULL, 2, AK_NULL);

        for (nSet = 0; (nSet < 7) && (AK_FALSE == bAcpt); nSet++)//There's 8 sets of RR Table in OTP area
        {        
            bAcpt = AK_TRUE;
            nfc_read(pRegVal, AK_NULL,\
                HYNIX_RETRY_REGCNT_20NM * HYNIX_RETRY_TIMES_20NM, AK_NULL);

            for (i = 0; i < HYNIX_RETRY_REGCNT_20NM * HYNIX_RETRY_TIMES_20NM; i++)
            {   
                nfc_read(&nTmp, AK_NULL, 1, AK_NULL);
                akerror(s_aScaleInfo, pRegVal[i], 0);
                //check the read-retry scale table with their reverse
                if (pRegVal[i] !=  ((~nTmp) & 0xFF))
                    bAcpt = AK_FALSE;
            }
        }
        //post command sequence
        nfc_cycle(CMD_CYCLE(0xFF), RB_CYCLE, END_CYCLE);
        nfc_cycle(CMD_CYCLE(0x38), RB_CYCLE, END_CYCLE);
        nfc_select(nTarget, AK_FALSE);
       // if (AK_FALSE == bAcpt)
       // {
        //    akerror("Init RR Failed", 0, AK_TRUE);
      //  }
    }
}
#pragma arm section code 

#endif//NAND_RR_H27UCG8T2A > 0
#endif//NAND_SUPPORT_RR > 0
#endif//DRV_SUPPORT_NAND > 0


/*******************************************************************************
* @以下是hynix 20nm nandflash enhanced 功能
* @author   Chen YongPing
* @date     2013-01-24
*******************************************************************************/
#if DRV_SUPPORT_NAND > 0
#if ENHANCED_SLC_PROGRAM > 0

#if (NAND_ENHANCED_SLC_H27UCG8T2A > 0) || (NAND_ENHANCED_SLC_H27UCG8T2C > 0)
#define HYNIX_ENHANCED_REGCNT 4
#define NFLASH_ENHANCED_GET_PARAM_HYNIX  0x37
#define NFLASH_ENHANCED_SET1_PARAM_HYNIX 0x36
#define NFLASH_ENHANCED_SET2_PARAM_HYNIX 0x16
#define NFLASH_ENHANCED_PARAM_OFFSET    0x0A//offset of enhance slc program enable  param
#define NFLASH_ENHANCED_NO_OFFSET    0x00//offset = '0' disable  enhance


#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_aDefVal_Enhanced[DRV_SUPPORT_NAND][HYNIX_ENHANCED_REGCNT];

#if (NAND_ENHANCED_SLC_H27UCG8T2A > 0) && (NAND_ENHANCED_SLC_H27UCG8T2C > 0)
    static const T_U8* s_aReg_20nm_Enhanced;
#else
    #if NAND_ENHANCED_SLC_H27UCG8T2A > 0
    #define s_aReg_20nm_Enhanced      s_aReg_Enhanced_Adie
    #endif

    #if NAND_ENHANCED_SLC_H27UCG8T2C > 0
    #define s_aReg_20nm_Enhanced      s_aReg_Enhanced_Cdie
    #endif
#endif
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
#if (NAND_ENHANCED_SLC_H27UCG8T2A > 0)
//hynix 20nm 64Gb MLC RawNAND A die
static const T_U8 s_aReg_Enhanced_Adie[HYNIX_ENHANCED_REGCNT] =
{
    0xB0, 0XB1, 0XA0, 0XA1
};
#endif

#if  NAND_ENHANCED_SLC_H27UCG8T2C>0
//hynix 20nm 64Gb MLC RawNAND C die
static const T_U8 s_aReg_Enhanced_Cdie[HYNIX_ENHANCED_REGCNT] =
{
    0xA0, 0XA1, 0XA7, 0XA8
};
#endif
#pragma arm section rodata
#pragma arm section code = "_update_"

/**
 * @brief  enter enhance-slc mode
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] b_enable disable or enable
 * @return  T_VOID
 */
static T_VOID hynix_enhanced_set_param_20nm(T_U8 nChip,  T_U8 val)
{
    nfc_cycle(CMD_CYCLE(NFLASH_ENHANCED_SET1_PARAM_HYNIX),\
        ADDR_CYCLE(s_aReg_20nm_Enhanced[0]),\
        WDATA_CYCLE(s_aDefVal_Enhanced[nChip][0] + val),\
        ADDR_CYCLE(s_aReg_20nm_Enhanced[1]),\
        WDATA_CYCLE(s_aDefVal_Enhanced[nChip][1] + val),\
        ADDR_CYCLE(s_aReg_20nm_Enhanced[2]),\
        WDATA_CYCLE(s_aDefVal_Enhanced[nChip][2] + val),\
        ADDR_CYCLE(s_aReg_20nm_Enhanced[3]),\
        WDATA_CYCLE(s_aDefVal_Enhanced[nChip][3] + val),\
        CMD_CYCLE(NFLASH_ENHANCED_SET2_PARAM_HYNIX),\
        END_CYCLE);
}

/**
 * @brief  enhanced slc trigger function
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] b_enable disable or enable
 * @return  T_VOID
 */
T_VOID hynix_enhanced_slc_program_20nm(T_U8 nTarget, T_BOOL b_enable)
{
    nfc_select(nTarget, AK_TRUE);

    if (b_enable)
    {
        //enter enhanced-slc mode
        hynix_enhanced_set_param_20nm(nTarget, NFLASH_ENHANCED_PARAM_OFFSET);
    }
    else
    {
        //exit enhanced-slc mode
        hynix_enhanced_set_param_20nm(nTarget, NFLASH_ENHANCED_NO_OFFSET);
        nfc_cycle(CMD_CYCLE(0x00),\
            ADDR_CYCLE(0),\
            ADDR_CYCLE(0),\
            ADDR_CYCLE(0),\
            ADDR_CYCLE(0),\
            ADDR_CYCLE(0),\
            CMD_CYCLE(0x30),\
            RB_CYCLE,
            END_CYCLE);
    }
    
    nfc_select(nTarget, AK_FALSE);
}
#pragma arm section code

#pragma arm section code = "_drvbootinit_"

/**
 * @brief get enhanced param of hynix 2y nm level nand.
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nChipCnt the amount of chip to be initialed.
 * @param[in] nID the ID of chip to be initialed.
 * @return  void
 */
T_VOID hynix_enhanced_init_20nm(T_U8 nChipCnt, T_U32 nID[2])
{
    T_U8 i, nTarget;

#if (NAND_ENHANCED_SLC_H27UCG8T2A > 0)
    if (IDL_Hynix_20nm_64gb_Adie == nID[0] )
    {
        s_aReg_20nm_Enhanced = s_aReg_Enhanced_Adie;
    }else
#endif

#if 0//(NAND_ENHANCED_SLC_H27UCG8T2C > 0)
    if (IDL_Hynix_20nm_64gb_Cdie == nID[0] && IDL_Hynix_20nm_64gb_Cdie_HI == nID[1] )
    {
        s_aReg_20nm_Enhanced = s_aReg_Enhanced_Cdie;
    }else
#endif

#if (NAND_ENHANCED_SLC_H27UBG8T2C > 0)
    if (IDL_Hynix_20nm_32gb_Cdie == nID[0])
    {
        s_aReg_20nm_Enhanced = s_aReg_Enhanced_Cdie;
    }else
#endif
    {
        //default
    }
    for (nTarget = 0; nTarget < nChipCnt; nTarget++)
    {
        nfc_select(nTarget, AK_TRUE);
        nfc_cycle(CMD_CYCLE(NFLASH_ENHANCED_GET_PARAM_HYNIX), END_CYCLE);

        for (i = 0; i < HYNIX_ENHANCED_REGCNT; i++)
        { 
            nfc_cycle(ADDR_CYCLE(s_aReg_20nm_Enhanced[i]), DELAY_CYCLE(1), END_CYCLE);
            
            nfc_read(&s_aDefVal_Enhanced[nTarget][i], AK_NULL, 1, AK_NULL);
        }
        
        nfc_select(nTarget, AK_FALSE);
    }
}


#pragma arm section code
#endif //(NAND_ENHANCED_SLC_H27UCG8T2A > 0) || (NAND_ENHANCED_SLC_H27UCG8T2C > 0)
#endif //ENHANCED_SLC_PROGRAM > 0
#endif //DRV_SUPPORT_NAND > 0

