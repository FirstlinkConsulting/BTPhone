#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0

#if (NAND_RR_H27UBG8T2B > 0) || (NAND_RR_H27UCG8T2M > 0)
#define HYNIX_RETRY_TIMES_26NM 6
#define HYNIX_RETRY_REGCNT_26NM 4

//hynix read retrial command
#define NFLASH_RETRY_GET_PARAM_HYNIX    0x37  
#define NFLASH_RETRY_SET1_PARAM_HYNIX   0x36
#define NFLASH_RETRY_SET2_PARAM_HYNIX   0x16

#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_nScale_26nm;  
static T_U8 s_aDefScales[DRV_SUPPORT_NAND][HYNIX_RETRY_REGCNT_26NM];

#if  (NAND_RR_H27UBG8T2B > 0) &&  (NAND_RR_H27UCG8T2M > 0)
static const T_S8 (*s_aOffset)[HYNIX_RETRY_REGCNT_26NM];
static const T_U8* s_aReg_26nm;
#else
    #if NAND_RR_H27UCG8T2M > 0
    #define s_aOffset   s_aOffset_26nm_64gb
    #define s_aReg_26nm      s_aReg_26nm_64gb
    #endif

    #if NAND_RR_H27UBG8T2B > 0
    #define s_aOffset   s_aOffset_26nm_32gb
    #define s_aReg_26nm      s_aReg_26nm_32gb
    #endif

#endif
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 s_aInfo[]= "S ";
#if NAND_RR_H27UCG8T2M > 0
//hynix 26nm 64Gb MLC RawNAND 
static const T_S8 s_aOffset_26nm_64gb[HYNIX_RETRY_TIMES_26NM][HYNIX_RETRY_REGCNT_26NM] =
{
    {0x00, 0x06, 0x0A, 0x06},
    {T_S8_MAX, -0x03, -0x07, -0x08},
    {T_S8_MAX, -0x06, -0x0D, -0x0F},
    {T_S8_MAX, -0x0B, -0x14, -0x17},
    {T_S8_MAX, T_S8_MAX, -0x1A, -0x1E},
    {T_S8_MAX, T_S8_MAX, -0x2A, -0x25}
};

static const T_U8 s_aReg_26nm_64gb[HYNIX_RETRY_REGCNT_26NM] =
{
    0xAC, 0xAD, 0xAE, 0xAF
};
#endif //NAND_RR_H27UCG8T2M > 0

#if NAND_RR_H27UBG8T2B > 0
//hynix 26nm 32Gb MLC RawNAND 
static const T_S8 s_aOffset_26nm_32gb[HYNIX_RETRY_TIMES_26NM][HYNIX_RETRY_REGCNT_26NM] =
{
    {0x00, 0x06, 0x0A, 0x06},
    {T_S8_MAX, -0x03, -0x07, -0x08},
    {T_S8_MAX, -0x06, -0x0D, -0x0F},
    {T_S8_MAX, -0x09, -0x14, -0x17},
    {T_S8_MAX, T_S8_MAX, -0x1A, -0x1E},
    {T_S8_MAX, T_S8_MAX, -0x2A, -0x25}
};

static const T_U8 s_aReg_26nm_32gb[HYNIX_RETRY_REGCNT_26NM] =
{
    0xA7, 0xAD, 0xAE, 0xAF
};
#endif //NAND_RR_H27UBG8T2B > 0

#pragma arm section rodata


/**
 * @brief set retry param of hynix 2x nm level nand.
 * @author Yang Yiming
 * @date 2011-09-16
 * @param[in] Chip which chip to be set.
 * @param[in] l2_buf_id l2 id, use the same l2 buffer as nand_read fun
 * @return  0
 */
#pragma arm section code = "_drvbootcode_" 
static T_VOID hynix_set_scales_26nm(T_U8 nTarget)
{
    T_U8 aRegVal[HYNIX_RETRY_REGCNT_26NM], i;
    T_U8 nOffset;
    
    for (i = 0; i < HYNIX_RETRY_REGCNT_26NM; i++)
    {
        aRegVal[i] = s_aDefScales[nTarget][i];
    
        if (HYNIX_RETRY_TIMES_26NM != s_nScale_26nm)
        {
            nOffset = s_aOffset[s_nScale_26nm][i];

            if (T_S8_MAX == nOffset)
            {
                aRegVal[i] = 0;
            }
            else
            {
                aRegVal[i] += nOffset;
            }
        }
        //else
       // {
         //   s_nScale_26nm = 0;
       // }
    }
    
    if (HYNIX_RETRY_TIMES_26NM == s_nScale_26nm)
     {
         s_nScale_26nm = 0;
     }

    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_SET1_PARAM_HYNIX),
        ADDR_CYCLE(s_aReg_26nm[0]),
        WDATA_CYCLE(aRegVal[0]),
        ADDR_CYCLE(s_aReg_26nm[1]),
        WDATA_CYCLE(aRegVal[1]),
        ADDR_CYCLE(s_aReg_26nm[2]),
        WDATA_CYCLE(aRegVal[2]),
        ADDR_CYCLE(s_aReg_26nm[3]),
        WDATA_CYCLE(aRegVal[3]),
        CMD_CYCLE(NFLASH_RETRY_SET2_PARAM_HYNIX),
        END_CYCLE);
}

/**
 * @brief        modify read retry register 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTarget where ecc error occured
 * @return      T_VOID
 */
T_VOID hynix_modify_scales_26nm(T_U8 nTarget)
{
    hynix_set_scales_26nm(nTarget);
    s_nScale_26nm++;
}

/**
 * @brief        revert read retry registers 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTarget who just modified read retry registers
 * @return      T_VOID
 */
T_VOID hynix_revert_scales_26nm(T_U8 nTarget)
{
    s_nScale_26nm = HYNIX_RETRY_TIMES_26NM;
    hynix_set_scales_26nm(nTarget);
}
#pragma arm section code
#pragma arm section code = "_drvbootinit_"

/**
 * @brief        match the read-retry register and offset table
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] nID the lower 1~4th id
 * @return      T_BOOL
 */
T_VOID hynix_retry_init_26nm(T_U8 nChipCnt, T_U32 nID[2])
{
    T_U8 i, nTarget;

    s_nScale_26nm = 0;
    
#if  (NAND_RR_H27UBG8T2B > 0) &&  (NAND_RR_H27UCG8T2M > 0)
    if (IDL_Hynix_26nm_64gb == nID[0])
    {
        s_aReg_26nm = s_aReg_26nm_64gb;
        s_aOffset = s_aOffset_26nm_64gb;
    }
    else// if (IDL_Hynix_26nm_32gb == nID)
    {
        s_aReg_26nm = s_aReg_26nm_32gb;
        s_aOffset = s_aOffset_26nm_32gb;
    }
        
#endif

    for (nTarget = 0; nTarget < nChipCnt; nTarget++)
    {
        nfc_select(nTarget, AK_TRUE);
        
        nfc_cycle(CMD_CYCLE(NFLASH_RETRY_GET_PARAM_HYNIX), END_CYCLE);

        for (i = 0; i < HYNIX_RETRY_REGCNT_26NM; i++)
        { 
            nfc_cycle(ADDR_CYCLE(s_aReg_26nm[i]), DELAY_CYCLE(1), END_CYCLE);
            
            nfc_read(&s_aDefScales[nTarget][i], AK_NULL, 1, AK_NULL);
            
            akerror(s_aInfo, s_aDefScales[nTarget][i], 1);
        }
        nfc_select(nTarget, AK_FALSE);
    }
}
#pragma arm section code
#endif//(NAND_RR_H27UBG8T2B > 0) || (NAND_RR_H27UCG8T2M > 0)
#endif//NAND_SUPPORT_RR > 0
#endif//DRV_SUPPORT_NAND > 0


/*******************************************************************************
 * @以下是hynix 26nm nandflash enhanced 功能
 * @author  Chen YongPing
 * @date    2013-01-28
*******************************************************************************/
#if DRV_SUPPORT_NAND > 0
#if ENHANCED_SLC_PROGRAM > 0

#if (NAND_ENHANCED_SLC_H27UBG8T2B > 0) || (NAND_ENHANCED_SLC_H27UCG8T2M > 0)
#define HYNIX_ENHANCED_REGCNT_26NM 5
#define NFLASH_ENHANCED_GET_PARAM_HYNIX_26NM   0x37
#define NFLASH_ENHANCED_SET1_PARAM_HYNIX_26NM 0x36
#define NFLASH_ENHANCED_SET2_PARAM_HYNIX_26NM 0x16
#define NFLASH_ENHANCED_PARAM_OFFSET1_26NM    0x26//offset of enhance slc program enable  param
#define NFLASH_ENHANCED_PARAM_OFFSET2_26NM    0x01//offset of enhance slc program enable  param
#define NFLASH_ENHANCED_PARAM_OFFSET3_26NM    0x25 
#define NFLASH_ENHANCED_NO_OFFSET_26NM        0x00//offset = '0' disable  enhance


#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_aDefVal_Enhanced_26nm[DRV_SUPPORT_NAND][HYNIX_ENHANCED_REGCNT_26NM];

#if (NAND_ENHANCED_SLC_H27UBG8T2B > 0) && (NAND_ENHANCED_SLC_H27UCG8T2M > 0)
    static const T_U8* s_aReg_26nm_Enhanced;
    static T_U8  s_Param;
#else
    #if NAND_ENHANCED_SLC_H27UBG8T2B > 0
    #define s_aReg_26nm_Enhanced      s_aReg_Enhanced_B
    #define s_Param                   NFLASH_ENHANCED_PARAM_OFFSET1_26NM  
    #endif

    #if NAND_ENHANCED_SLC_H27UCG8T2M > 0
    #define s_aReg_26nm_Enhanced      s_aReg_Enhanced_M
    #define s_Param                   NFLASH_ENHANCED_PARAM_OFFSET3_26NM 
    #endif
#endif
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
#if NAND_ENHANCED_SLC_H27UBG8T2B > 0
//hynix 26nm 32Gb MLC RawNAND 
static const T_U8 s_aReg_Enhanced_B[HYNIX_ENHANCED_REGCNT_26NM] =
{
    0xA0, 0XA1, 0XB0, 0XB1, 0XC9
};
#endif

#if  NAND_ENHANCED_SLC_H27UCG8T2M > 0
//hynix 26nm 64Gb MLC RawNAND 
static const T_U8 s_aReg_Enhanced_M[HYNIX_ENHANCED_REGCNT_26NM] =
{
    0xA4, 0XA5, 0XB0, 0XB1, 0XC9
};
#endif
#pragma arm section rodata

#pragma arm section code = "_update_"
static T_VOID hynix_enhanced_set_param_26nm(T_U8 nChip,  T_U8 val, T_U8 offset)
{
    nfc_cycle(CMD_CYCLE(NFLASH_ENHANCED_SET1_PARAM_HYNIX_26NM),\
        ADDR_CYCLE(s_aReg_26nm_Enhanced[0]),\
        WDATA_CYCLE(s_aDefVal_Enhanced_26nm[nChip][0] + val),\
        ADDR_CYCLE(s_aReg_26nm_Enhanced[1]),\
        WDATA_CYCLE(s_aDefVal_Enhanced_26nm[nChip][1] + val),\
        ADDR_CYCLE(s_aReg_26nm_Enhanced[2]),\
        WDATA_CYCLE(s_aDefVal_Enhanced_26nm[nChip][2] + val),\
        ADDR_CYCLE(s_aReg_26nm_Enhanced[3]),\
        WDATA_CYCLE(s_aDefVal_Enhanced_26nm[nChip][3] + val),\
        ADDR_CYCLE(s_aReg_26nm_Enhanced[4]),\
        WDATA_CYCLE(s_aDefVal_Enhanced_26nm[nChip][4] + offset),\
        CMD_CYCLE(NFLASH_ENHANCED_SET2_PARAM_HYNIX_26NM),\
        END_CYCLE);
}

/**
 * @brief         the enhance-slc mode triggle function
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] b_enable disable or enable
 * @return      T_VOID
 */
T_VOID hynix_enhanced_slc_program_26nm(T_U8 nTarget, T_BOOL b_enable )
{
    nfc_select(nTarget, AK_TRUE);
    
    if (b_enable)
    {
        hynix_enhanced_set_param_26nm(nTarget, s_Param, NFLASH_ENHANCED_PARAM_OFFSET2_26NM); 
    }
    else
    {
        hynix_enhanced_set_param_26nm(nTarget, NFLASH_ENHANCED_NO_OFFSET_26NM, NFLASH_ENHANCED_NO_OFFSET_26NM); 
    }
          
    nfc_select(nTarget, AK_FALSE);
}
#pragma arm section code

#pragma arm section code = "_drvbootinit_"

/**
 * @brief        match the enhance-slc mode register
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] nID the lower 1~4th id
 * @return      T_VOID
 */
T_VOID hynix_enhanced_init_26nm(T_U8 nChipCnt, T_U32 nID[2])
{
    T_U8 i, nTarget;

#if (NAND_ENHANCED_SLC_H27UBG8T2B > 0) && (NAND_ENHANCED_SLC_H27UCG8T2M > 0)
    if (IDL_Hynix_26nm_32gb == nID[0])
    {
        s_aReg_26nm_Enhanced = s_aReg_Enhanced_B;
        s_Param = NFLASH_ENHANCED_PARAM_OFFSET1_26NM;
    }
    else
    {
        s_aReg_26nm_Enhanced = s_aReg_Enhanced_M;
        s_Param = NFLASH_ENHANCED_PARAM_OFFSET3_26NM;
    }
#endif
    for (nTarget = 0; nTarget < nChipCnt; nTarget++)
    {
        nfc_select(nTarget, AK_TRUE);
        nfc_cycle(CMD_CYCLE(NFLASH_ENHANCED_GET_PARAM_HYNIX_26NM), END_CYCLE);

        for (i = 0; i < HYNIX_ENHANCED_REGCNT_26NM; i++)
        { 
            nfc_cycle(ADDR_CYCLE(s_aReg_26nm_Enhanced[i]), DELAY_CYCLE(1), END_CYCLE);
            
            nfc_read(&s_aDefVal_Enhanced_26nm[nTarget][i], AK_NULL, 1, AK_NULL);
        }
        
        nfc_select(nTarget, AK_FALSE);
    }
}

#pragma arm section code
#endif
#endif
#endif


