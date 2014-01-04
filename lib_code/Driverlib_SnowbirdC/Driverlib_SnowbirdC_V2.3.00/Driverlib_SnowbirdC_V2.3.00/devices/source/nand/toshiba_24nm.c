#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0
#if NAND_RR_TOSHIBA_24NM > 0
#define TOSHIBA_RETRY_REGCNT_24NM 4
#define TOSHIBA_RETRY_TIMES_24NM 5

#define NFLASH_RETRY_PRE1_PARAM_TOSHIBA   0x5C
#define NFLASH_RETRY_PRE2_PARAM_TOSHIBA   0xC5
#define NFLASH_RETRY_SET_PARAM_TOSHIBA    0x55
#define NFLASH_RETRY_RESET_PARAM_TOSHIBA  0xFF
#define NFLASH_RETRY_END1_PARAM_TOSHIBA    0x26
#define NFLASH_RETRY_END2_PARAM_TOSHIBA    0x5D
#pragma arm section zidata = "_drvbootbss_"

static T_U8 s_nTSBScale = 0;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"

//toshiba 24nm 64Gb MLC RawNAND 
const T_U8 s_aRegVal[TOSHIBA_RETRY_TIMES_24NM][TOSHIBA_RETRY_REGCNT_24NM] =
{
    {0x04, 0x04, 0x04, 0x04},
    {0x7c, 0x7c, 0x7c, 0x7c},
    {0x78, 0x78, 0x78, 0x78},
    {0x74, 0x74, 0x74, 0x74},
    {0x08, 0x08, 0x08, 0x08}
};

const T_U8 s_aReg[TOSHIBA_RETRY_REGCNT_24NM] =
{
    0x04, 0x05, 0x06, 0x07
};

#pragma arm section rodata

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief set retry param of toshiba retry nand..
 * @author yangyiming
 * @date 2012-03-30
 * @param[in] Chip which chip to be set.
 * @return  T_VOID
 */
T_VOID toshiba_modify_scales_24nm(T_U8 nTarget)
{

    if (TOSHIBA_RETRY_TIMES_24NM == s_nTSBScale++) //read fail,CMD FF to reset
    {    
        s_nTSBScale = 0;
    }

    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_PRE1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_PRE2_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg[0]),
        WDATA_CYCLE(s_aRegVal[s_nTSBScale][0]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg[1]),
        WDATA_CYCLE(s_aRegVal[s_nTSBScale][1]),         
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg[2]),
        WDATA_CYCLE(s_aRegVal[s_nTSBScale][2]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg[3]),
        WDATA_CYCLE(s_aRegVal[s_nTSBScale][3]),
        CMD_CYCLE(NFLASH_RETRY_END1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_END2_PARAM_TOSHIBA),
        END_CYCLE);
}


 /**
 * @brief   revert read retry register 
 * @author yangyiming
 * @date 2012-03-30
 * @param[in] nTarget  who just modified read retry registers
 * @return  T_VOID
 */
T_VOID toshiba_revert_scales_24nm(T_U8 nTarget)
{
    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_RESET_PARAM_TOSHIBA), RB_CYCLE, END_CYCLE);
}

#pragma arm section code

#endif//NAND_RR_TOSHIBA_24NM
#endif//NAND_SUPPORT_RR
#endif//DRV_SUPPORT_NAND
