#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0

extern T_VOID hynix_modify_scales_26nm(T_U8 nTarget);
extern T_VOID hynix_revert_scales_26nm(T_U8 nTarget);
extern T_VOID hynix_modify_scales_20nm(T_U8 nTarget);
extern T_VOID hynix_revert_scales_20nm(T_U8 nTarget);
extern T_VOID toshiba_modify_scales_24nm(T_U8 nTarget);
extern T_VOID toshiba_revert_scales_24nm(T_U8 nTarget);
extern T_VOID hynix_enhanced_slc_program_26nm(T_U8 nTarget, T_BOOL b_enable);
extern T_VOID hynix_enhanced_slc_program_20nm(T_U8 nTarget, T_BOOL b_enable);
extern T_VOID toshiba_modify_scales_19nm(T_U8 nTarget);
extern T_VOID toshiba_revert_scales_19nm(T_U8 nTarget);
extern T_VOID samsung_modify_scales_21nm(T_U8 nTarget);
extern T_VOID samsung_revert_scales_21nm(T_U8 nTarget);

#pragma arm section zidata = "_drvbootbss_"
static T_VOID (*p_modify)(T_U8 nTarget);
static T_VOID (*p_revert)(T_U8 nTarget);
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 s_aRR[] = NAND_RR_INFO;
#pragma arm section rodata

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief        modify read retry register 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTarget where ecc error occured
 * @return      T_VOID
 */
T_VOID modify_scales(T_U8 nTarget)
{
    if (AK_NULL != p_modify)
        p_modify(nTarget);
}

/**
 * @brief        revert read retry registers 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTarget who just modified read retry registers
 * @return      T_VOID
 */
T_VOID revert_scales(T_U8 nTarget)
{
    if (AK_NULL != p_modify)
        p_revert(nTarget);
}
#pragma arm section code
#pragma arm section code = "_drvbootinit_"

/**
 * @brief        match the read-retry method
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] nID the lower 1~4th id
 * @return      T_BOOL
 */
T_VOID retry_init(T_U8 nTargetCnt, T_U32 nID[2])
{
    akerror(s_aRR, nTargetCnt, AK_TRUE);

    #if (NAND_RR_H27UBG8T2B > 0) || (NAND_RR_H27UCG8T2M > 0)    
    if(IDL_Hynix_26nm_64gb == nID[0] ||IDL_Hynix_26nm_32gb == nID[0])
    {
        p_modify = hynix_modify_scales_26nm;
        p_revert = hynix_revert_scales_26nm;
        hynix_retry_init_26nm(nTargetCnt, nID);
    }else
    #endif
    #if (NAND_RR_H27UCG8T2A > 0) || (NAND_RR_H27UCG8T2C > 0) ||(NAND_RR_H27UBG8T2C > 0)     
    //(IDL_Hynix_20nm_64gb_Cdie == nID[0]  && IDL_Hynix_20nm_64gb_Cdie_HI==nID[1])||
    if(IDL_Hynix_20nm_64gb_Adie == nID[0]  || IDL_Hynix_20nm_32gb_Cdie == nID[0])
    {
        p_modify = hynix_modify_scales_20nm;
        p_revert = hynix_revert_scales_20nm;
        hynix_retry_init_20nm(nTargetCnt, nID);
    }else
    #endif
    #if NAND_RR_TOSHIBA_24NM > 0
    if(IDL_Toshiba_24nm_32gb == nID[0] ||IDL_Toshiba_24nm_64gb == nID[0])
    {
        p_modify = toshiba_modify_scales_24nm;
        p_revert = toshiba_revert_scales_24nm;
    }else
    #endif
    #if NAND_RR_TOSHIBA_19NM > 0
    if(IDL_Toshiba_19nm_64gb_DC == nID[0] ||
        IDL_Toshiba_19nm_128gb_DC == nID[0]||
        IDL_Toshiba_19nm_64gb_DD == nID[0] ||
        IDL_Toshiba_19nm_128gb_DD == nID[0] )
    { 
        p_modify = toshiba_modify_scales_19nm;
        p_revert = toshiba_revert_scales_19nm;
    }else
    #endif
    #if NAND_RR_SAMSUNG_21NM > 0
    if(IDL_Samsung_21nm == nID[0])
    {
        p_modify = samsung_modify_scales_21nm;
        p_revert = samsung_revert_scales_21nm;
    }else
    #endif
    {
        akerror(s_aRR, nID[1], AK_TRUE);
        while(1); //for your own good.
    }
}
#pragma arm section code

#endif //NAND_SUPPORT_RR

#if ENHANCED_SLC_PROGRAM > 0
#pragma arm section zidata = "_drvbootbss_"
static T_VOID (*p_slc_program)(T_U8 nTarget, T_BOOL bEnable);
static T_U8 nMakerId;
#pragma arm section zidata

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief        enable/disable the enhance slc mode if it existed
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTarget
 * @param[in] bEnable , disable or enable
 * @return      T_BOOL
 */
T_VOID enhance_slc_program(T_U8 nTarget, T_BOOL bEnable)
{
    if (AK_NULL != p_slc_program)
        p_slc_program(nTarget, bEnable);
}

/**
 * @brief        lower page method for Samsung, Toshiba, Microm and Hynix
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nSlcPage, the nSlcpPage'th lower page in a block
 * @param[out] pTruePage , the true page address in a block
 * @return      T_BOOL
 */
T_VOID enhance_slc_get_lower_page(T_U8 nSlcPage, T_U8 *pTruePage)
{
    T_U16 nTruePage;

    nTruePage = nSlcPage;
    
    switch (nMakerId)
    {
#if (NAND_ENHANCED_SLC_SAMSUNG_21NM > 0) || (NAND_ENHANCED_SLC_TOSHIBA_24NM > 0)|| (NAND_ENHANCED_SLC_TOSHIBA_19NM > 0) || (NAND_ENHANCED_SLC_TOSHIBA_1YNM > 0)
        case 0x98:
        case 0xEC:
        if (nSlcPage > 1)
        {
             nTruePage = (nSlcPage - 2) * 2 + 3;
        }
        break;
        #endif
#if (NAND_ENHANCED_SLC_H27UBG8T2B > 0) || (NAND_ENHANCED_SLC_H27UCG8T2M > 0) || (NAND_ENHANCED_SLC_H27UCG8T2A > 0) || (NAND_ENHANCED_SLC_H27UCG8T2C > 0)
        case 0xAD:
        if (nSlcPage > 125)
        {
             nTruePage = (nSlcPage * 2) - 2 - (nSlcPage & 0x1);
        }
        else if(nSlcPage > 1)
        {
             nTruePage = (((nSlcPage / 2) - 1) * 4) + (nSlcPage & 0x1) + 2;
        }
        break;
#endif
        
#if (NAND_ENHANCED_SLC_MICRON_20NM > 0)
        case 0x2C:
        //we just use the 128 of 132 pages lower-page in micron mlc nandflash
        if (nSlcPage > 3)
        {
            nTruePage = nSlcPage & 0x1;
            nTruePage += ((nSlcPage >> 1) - 1) * 4;
        }
        break;
#endif
        default:
        break;
    }
    
    *pTruePage = nTruePage;
}

#pragma arm section code
#pragma arm section code = "_drvbootinit_"

/**
 * @brief        match the enhance-slc mode triggle function
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in] nTargetCnt the target amount in a device
 * @param[in] nID the lower 1~4th id
 * @return      T_VOID
 */
T_VOID enhance_slc_init(T_U8 nTargetCnt, T_U32 nID[2])
{
    #if (NAND_ENHANCED_SLC_H27UBG8T2B > 0) || (NAND_ENHANCED_SLC_H27UCG8T2M > 0)
    if(IDL_Hynix_26nm_64gb == nID[0] ||IDL_Hynix_26nm_32gb == nID[0])
        {
            p_slc_program = hynix_enhanced_slc_program_26nm;
            hynix_enhanced_init_26nm(nTargetCnt, nID);
        }else
    #endif
    #if (NAND_ENHANCED_SLC_H27UCG8T2A > 0) || (NAND_ENHANCED_SLC_H27UCG8T2C > 0) || (NAND_ENHANCED_SLC_H27UBG8T2C > 0)
    //(IDL_Hynix_20nm_64gb_Cdie == nID[0]  && IDL_Hynix_20nm_64gb_Cdie_HI==nID[1])||
     if(IDL_Hynix_20nm_64gb_Adie == nID[0] || IDL_Hynix_20nm_32gb_Cdie == nID[0])
    {
        p_slc_program = hynix_enhanced_slc_program_20nm;
        hynix_enhanced_init_20nm(nTargetCnt, nID);
    }else
    #endif
    {
        //default:
    }
    nMakerId = nID[0] & 0xFF;
}

#pragma arm section code 

#endif //ENHANCED_SLC_PROGRAM > 0
#endif //DRV_SUPPORT_NAND
