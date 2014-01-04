#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0

#if NAND_RR_TOSHIBA_1YNM > 0
#define NFLASH_RETRY_PRE1_PARAM_TOSHIBA 0x5C
#define NFLASH_RETRY_PRE2_PARAM_TOSHIBA 0xC5
#define NFLASH_RETRY_SET_PARAM_TOSHIBA  0x55
#define NFLASH_RETRY_RESET_PARAM_TOSHIBA 0xFF
#define NFLASH_RETRY_END1_PARAM_TOSHIBA 0x26
#define NFLASH_RETRY_END2_PARAM_TOSHIBA 0x5D

#define TOSHIBA_RETRY_REGCNT_1YNM 5
#define TOSHIBA_RETRY_TIMES_1YNM  7
/*
addtional command 0x0d and data 0x00 compared to1ynm
7th cycle has an extra command 0xb3 without data  
*/
#define TOSHIBA_RETRY_7TH_EXTRA_CMD 0xb3
#define TOSHIBA_RETRY_REVERT_INDEX 7    //  8th cycle

#pragma arm section rodata = "_drvbootconst_"

//read-retry table
static const T_U8 s_aRegVal_1ynm[TOSHIBA_RETRY_TIMES_1YNM][TOSHIBA_RETRY_REGCNT_1YNM] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x04, 0x04, 0x04, 0x04, 0x00},
    {0x7c, 0x7c, 0x7c, 0x7c, 0x00},
    {0x78, 0x78, 0x78, 0x78, 0x00},
    {0x74, 0x74, 0x74, 0x74, 0x00},
    {0x08, 0x08, 0x08, 0x08, 0x00}
};
#define TOSHIBA_1YNM_REVERT_DATA 0
#define TOSHIBA_1YNM_REG0 0X04
#define TOSHIBA_1YNM_REG1 0X05
#define TOSHIBA_1YNM_REG2 0X06
#define TOSHIBA_1YNM_REG3 0X07
#define TOSHIBA_1YNM_REG4 0X0D

/*static const T_U8 s_aReg_1ynm[TOSHIBA_RETRY_REGCNT_1YNM] = 
{
    0x04, 0x05, 0x06, 0x07, 0x0d
};*/
//cycle sequence to revert scales and turn to normal-read mode
static const T_U16 s_aRevert[]={
    CMD_CYCLE(NFLASH_RETRY_PRE1_PARAM_TOSHIBA),
    CMD_CYCLE(NFLASH_RETRY_PRE2_PARAM_TOSHIBA),
    CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
    ADDR_CYCLE(TOSHIBA_1YNM_REG0),
    WDATA_CYCLE(TOSHIBA_1YNM_REVERT_DATA),
    CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
    ADDR_CYCLE(TOSHIBA_1YNM_REG1),
    WDATA_CYCLE(TOSHIBA_1YNM_REVERT_DATA),
    CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
    ADDR_CYCLE(TOSHIBA_1YNM_REG2),
    WDATA_CYCLE(TOSHIBA_1YNM_REVERT_DATA),
    CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
    ADDR_CYCLE(TOSHIBA_1YNM_REG3),
    WDATA_CYCLE(TOSHIBA_1YNM_REVERT_DATA),
    CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
    ADDR_CYCLE(TOSHIBA_1YNM_REG4),
    WDATA_CYCLE(TOSHIBA_1YNM_REVERT_DATA),
    CMD_CYCLE(NFLASH_RETRY_RESET_PARAM_TOSHIBA), 
    RB_CYCLE, END_CYCLE
};
#pragma arm section rodata 

#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_nTSBScale_1ynm = 0;
#pragma arm section zidata

#pragma arm section code = "_drvbootcode_"

/**
 * @brief set retry param of toshiba retry nand..
 * @author lihongwu
 * @date 2013-08-06
 * @param[in] Chip which chip to be set.
 * @return  T_VOID
 */
T_VOID toshiba_modify_scales_1ynm(T_U8 nTarget)
{
    if(TOSHIBA_RETRY_TIMES_1YNM-1 == s_nTSBScale_1ynm++)
    {
        s_nTSBScale_1ynm = 0;
    }
    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_PRE1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_PRE2_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(TOSHIBA_1YNM_REG0),
        WDATA_CYCLE(s_aRegVal_1ynm[s_nTSBScale_1ynm][0]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(TOSHIBA_1YNM_REG1),
        WDATA_CYCLE(s_aRegVal_1ynm[s_nTSBScale_1ynm][1]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(TOSHIBA_1YNM_REG2),
        WDATA_CYCLE(s_aRegVal_1ynm[s_nTSBScale_1ynm][2]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(TOSHIBA_1YNM_REG3),
        WDATA_CYCLE(s_aRegVal_1ynm[s_nTSBScale_1ynm][3]),
        CMD_CYCLE(NFLASH_RETRY_SET_PARAM_TOSHIBA),
        ADDR_CYCLE(TOSHIBA_1YNM_REG4),
        WDATA_CYCLE(s_aRegVal_1ynm[s_nTSBScale_1ynm][4]),
        CMD_CYCLE(NFLASH_RETRY_END1_PARAM_TOSHIBA),
        CMD_CYCLE(NFLASH_RETRY_END2_PARAM_TOSHIBA),
        (s_nTSBScale_1ynm ==6?CMD_CYCLE(TOSHIBA_RETRY_7TH_EXTRA_CMD):END_CYCLE),
        END_CYCLE);

}

/**
 * @brief revert scales
 * @author lihongwu
 * @date 2013-08-06
 * @param[in] Chip which chip to be set.
 * @return  T_VOID
 */
T_VOID toshiba_revert_scales_1ynm(T_U8 nTarget)
{
    nfc_cycle(NAND_CYCLE_ARR_FLAG, s_aRevert);
}
#pragma arm section code

#endif//NAND_RR_TOSHIBA_1YNM

#endif//NAND_SUPPORT_RR
#endif//DRV_SUPPORT_NAND

