#include "anyka_types.h"
#include "arch_nand.h"
#include "drv_cfg.h"
#include "nand_retry.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0
//samsung 21nm 64gb A-Die/32gb B-Die  read retry
#if NAND_RR_SAMSUNG_21NM > 0

#define MAX_RR_CHIPCNT_21NM 1
#define SAMSUNG_RETRY_TIMES_21NM 15
#define SAMSUNG_RETRY_REGCNT_21NM 4
#define NFLASH_RETRY_SET_PARAM_SAMSUNG   0XA1

#pragma arm section zidata = "_drvbootbss_"
static T_U8 s_RetryTimes;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 s_Reg[SAMSUNG_RETRY_REGCNT_21NM] = 
{
    0XA7, 0XA4, 0XA5, 0XA6
};
static const T_U8 s_Samsung_Offset[SAMSUNG_RETRY_TIMES_21NM][SAMSUNG_RETRY_REGCNT_21NM] = 
{
    {0X00, 0X00, 0X00, 0X00},
    {0X05, 0X0A, 0X00, 0X00},
    {0X28, 0X00, 0XEC, 0XD8},
    {0XED, 0XF5, 0XED, 0XE6},
    {0X0A, 0X0F, 0X05, 0X00},
    {0X0F, 0X0A, 0XFB, 0XEC},
    {0XE8, 0XEF, 0XE8, 0XDC},
    {0XF1, 0XFB, 0XFE, 0XF0},
    {0X0A, 0X00, 0XFB, 0XEC},
    {0XD0, 0XE2, 0XD0, 0XC2},
    {0X14, 0X0F, 0XFB, 0XEC},
    {0XE8, 0XFB, 0XE8, 0XDC},
    {0X1E, 0X14, 0XFB, 0XEC},
    {0XFB, 0XFF, 0XFB, 0XF8},
    {0X07, 0X0C, 0X02, 0X00},
};
#pragma arm section rodata 

#pragma arm section code = "_drvbootcode_" 

/**
 * @brief set retry param of SAMSUNG 2Y nm level nand.
 * @author Chen YongPing
 * @date 2013-02-19
  */
static T_VOID samsung_set_scales_21nm(T_U8 nTarget)
{
    if (SAMSUNG_RETRY_TIMES_21NM == s_RetryTimes)
        s_RetryTimes = 0;
    
    nfc_cycle(CMD_CYCLE(NFLASH_RETRY_SET_PARAM_SAMSUNG),
        ADDR_CYCLE(s_Reg[0]),
        WDATA_CYCLE(s_Samsung_Offset[s_RetryTimes][0]),
        ADDR_CYCLE(s_Reg[1]),
        WDATA_CYCLE(s_Samsung_Offset[s_RetryTimes][1]),
        ADDR_CYCLE(s_Reg[2]),
        WDATA_CYCLE(s_Samsung_Offset[s_RetryTimes][2]),
        ADDR_CYCLE(s_Reg[3]),
        WDATA_CYCLE(s_Samsung_Offset[s_RetryTimes][3]),
        DELAY_CYCLE(40),  //delay > 300ns
        END_CYCLE);
}

/**
* @brief        modify read retry register 
* @author Chen YongPing
* @date 2013-01-24
* @param[in] nTarget where ecc error occured
* @return      T_VOID
*/
T_VOID samsung_modify_scales_21nm(T_U8 nTarget)
{
    s_RetryTimes++;
    samsung_set_scales_21nm(nTarget);
}

/**
 * @brief   revert read retry register 
 * @author Chen YongPing
 * @date 2013-01-24
 * @param[in] nTarget  who just modified read retry registers
 * @return  T_VOID
 */
T_VOID samsung_revert_scales_21nm(T_U8 nTarget)
{
    s_RetryTimes = 0;
    samsung_set_scales_21nm(nTarget);
}
#pragma arm section code

#endif//NAND_RR_SAMSUNG_21NM
#endif//NAND_SUPPORT_RR
#endif//DRV_SUPPORT_NAND

