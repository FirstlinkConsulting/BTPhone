#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0
#if NAND_RR_TOSHIBA_19NM > 0
#define TOSHIBA_RETRY_REGCNT_19NM 4
#define TOSHIBA_RETRY_TIMES_19NM  6

#define NFLASH_RETRY_PRE1_PARAM_TOSHIBA 0x5C
#define NFLASH_RETRY_PRE2_PARAM_TOSHIBA 0xC5
#define NFLASH_RETRY_SET_PARAM_TOSHIBA  0x55
#define NFLASH_RETRY_RESET_PARAM_TOSHIBA 0xFF
#define NFLASH_RETRY_END1_PARAM_TOSHIBA 0x26
#define NFLASH_RETRY_END2_PARAM_TOSHIBA 0x5D
#pragma arm section zidata = "_drvbootbss_"

static T_U8 s_nTSBScale_19nm = 0;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"

//value for register 
static const T_U8 s_aRegVal_19nm[TOSHIBA_RETRY_TIMES_19NM][TOSHIBA_RETRY_REGCNT_19NM] =
{
    {0x00, 0x00, 0x00, 0x00},
    {0x04, 0x04, 0x04, 0x04},
    {0x7c, 0x7c, 0x7c, 0x7c},
    {0x78, 0x78, 0x78, 0x78},
    {0x74, 0x74, 0x74, 0x74},
    {0x08, 0x08, 0x08, 0x08}
};

//read-retry register
static const T_U8 s_aReg_19nm[TOSHIBA_RETRY_REGCNT_19NM] = 
{
    0x04, 0x05, 0x06, 0x07
};
#pragma arm section rodata 

#pragma arm section code = "_drvbootcode_"

/**
 * @brief set retry param of toshiba retry nand..
 * @author chenyongping
 * @date 2013-01-16
 * @param[in] Chip which chip to be set.
 * @return  T_VOID
 */
T_VOID toshiba_modify_scales_19nm(T_U8 nTarget)
{
    if(TOSHIBA_RETRY_TIMES_19NM == s_nTSBScale_19nm++)
    {
        s_nTSBScale_19nm = 0;
    }

    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_PRE1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_PRE2_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg_19nm[0]),
        WDATA_CYCLE(s_aRegVal_19nm[s_nTSBScale_19nm][0]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg_19nm[1]),
        WDATA_CYCLE(s_aRegVal_19nm[s_nTSBScale_19nm][1]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg_19nm[2]),
        WDATA_CYCLE(s_aRegVal_19nm[s_nTSBScale_19nm][2]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(s_aReg_19nm[3]),
        WDATA_CYCLE(s_aRegVal_19nm[s_nTSBScale_19nm][3]),
        CMD_CYCLE(NFLASH_RETRY_END1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_END2_PARAM_TOSHIBA),
        END_CYCLE);
}

/**
 * @brief   revert read retry register 
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTarget  who just modified read retry registers
 * @return  T_VOID
 */
T_VOID toshiba_revert_scales_19nm(T_U8 nTarget)
{
    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_RESET_PARAM_TOSHIBA), RB_CYCLE, END_CYCLE);
}
#pragma arm section code
#endif//NAND_RR_TOSHIBA_19NM
#endif//NAND_SUPPORT_RR
#endif//DRV_SUPPORT_NAND
