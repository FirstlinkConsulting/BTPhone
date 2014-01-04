#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_gpio.h"
#include "arch_nand.h"
#include "arch_sys_ctl.h"
#include "l2.h"
#include "clk.h"
#include "share_pin.h"
#include "drv_cfg.h"
#include "nand_command.h"

#if DRV_SUPPORT_NAND > 0

// NF controller
#define NFC_CMD_REG         0x0044A000
#define NFC_DAT_REG1        0x0044A050
#define NFC_DAT_REG2        0x0044A054
#define NFC_CTRL_REG        0x0044A058
#define NFC_CMD_LEN_REG     0x0044A05C
#define NFC_DAT_LEN_REG     0x0044A060
#define NFC_RAND_ENC_REG    0x0044A068
#define NFC_RAND_DEC_REG    0x0044A06C

/* command register*/
#define NFC_CMD_REG_RES_BIT         (1UL << 31)   //this bit is reserved in cmd register
#define NFC_CMD_REG_INFO_POS        11          //bit 11 - bit 22
#define NFC_CMD_REG_DELAY_BIT       (1 << 10)   //wait delay time enable bit
#define NFC_CMD_REG_WAIT_JUMP_BIT   (1 << 9)    //wait R/b enable bit
#define NFC_CMD_REG_DAT_BIT         (1 << 8)
#define NFC_CMD_REG_STF_EN_BIT      (1 << 7)
#define NFC_CMD_REG_CMD_BIT         (1 << 6)
#define NFC_CMD_REG_WE_BIT          (1 << 5)
#define NFC_CMD_REG_RE_BIT          (1 << 4)
#define NFC_CMD_REG_CNT_EN_BIT      (1 << 3)
#define NFC_CMD_REG_CLE_BIT         (1 << 2)
#define NFC_CMD_REG_ALE_BIT         (1 << 1)
#define NFC_CMD_REG_LAST_BIT        (1 << 0)    // last command's bit0 set to 1


#define NFC_CMD_REG_CMD_CONF        (NFC_CMD_REG_CLE_BIT | NFC_CMD_REG_WE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_ADD_CONF        (NFC_CMD_REG_ALE_BIT | NFC_CMD_REG_WE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_REG_IN_CONF    (NFC_CMD_REG_CNT_EN_BIT | NFC_CMD_REG_RE_BIT | NFC_CMD_REG_CMD_BIT)
#define NFC_CMD_REG_WDATA_CONF      (NFC_CMD_REG_WE_BIT | NFC_CMD_REG_DAT_BIT | NFC_CMD_REG_CNT_EN_BIT)

#define NFC_REG_DAT_IN_LEN          8
#define NFC_RAND_EN_BIT     (1 << 16)

/* contorl/status register */
#define NFC_CTRL_REG_CMD_DONE_BIT    (1UL << 31)
#define NFC_CTRL_REG_CMD_VALID_BIT   (1 << 30)
#define NFC_CTRL_REG_STA_CLR_BIT     (1 << 14)
#define NFC_CTRL_REG_CE_SEL_POS     (10)
#define NFC_CTRL_REG_CE3_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 3))
#define NFC_CTRL_REG_CE2_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 2))
#define NFC_CTRL_REG_CE1_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS + 1))
#define NFC_CTRL_REG_CE0_SEL_BIT     (1 << (NFC_CTRL_REG_CE_SEL_POS))
#define NFC_CTRL_REG_CE_SAVE_BIT     (1 << 9)
#define NFC_CTRL_REG_STF_INFO_POS    (1)         //bit1 -bit 8
#define NFC_CTRL_REG_POWER_SAVE_BIT  (1 << 0)
#define NFC_CTRL_REG_CHIP_CONF(chip) ((0x01 << (chip)) << NFC_CTRL_REG_CE_SEL_POS)


/* ECC control register */
#define ECC_CTRL_REG1    0x0045A000
#define ECC_CTRL_REG2    0x0045A004
#define ECC_CTRL_REG3    0x0045A008


#define ECC_CTRL_REG1_DEC_INTR_EN_BIT     (1 << 30)
#define ECC_CTRL_REG1_ENC_INTR_EN_BIT     (1 << 29)
#define ECC_CTRL_REG1_END_INTR_EN_BIT     (1 << 28)
#define ECC_CTRL_REG1_RESULT_NO_OK_BIT    (1 << 27)
#define ECC_CTRL_REG1_NO_ERR_BIT          (1 << 26)
#define ECC_CTRL_REG1_DEC_RDY_BIT         (1 << 25)
#define ECC_CTRL_REG1_ENC_RDY_BIT         (1 << 24)
//#define ECC_CTRL_REG1_MODE_POS            (21)
#define ECC_CTRL_REG1_NFC_EN_BIT          (1 << 20)
//#define ECC_CTRL_REG1_BYTE_CFG_POS        (7)         
#define ECC_CTRL_REG1_END_BIT             (1 << 6)
#define ECC_CTRL_REG1_BIGEN_BIT           (1 << 5)
#define ECC_CTRL_REG1_ADDR_CLR_BIT        (1 << 4)
#define ECC_CTRL_REG1_START_BIT           (1 << 3)
#define ECC_CTRL_REG1_DIR_POS             (2)
#define ECC_CTRL_REG1_DEC_EN_BIT          (1 << 1)
#define ECC_CTRL_REG1_ENC_EN_BIT          (1 << 0)
#define ECC_CTRL_REG1_NO_DEC_ENC          (~0x3)
#define ECC_CTRL_REG1_DIR_WRITE           (1)
#define ECC_CTRL_REG1_DIR_READ            (0)
#define ECC_CTRL_REG1_WRITE_BIT           (ECC_CTRL_REG1_DIR_WRITE << ECC_CTRL_REG1_DIR_POS)
#define ECC_CTRL_REG1_READ_BIT            (ECC_CTRL_REG1_DIR_READ << ECC_CTRL_REG1_DIR_POS)

#define ECC_CTRL_REG2_MODE_POS           (25) //bit25 ~27
#define ECC_CTRL_REG2_SECTCNT_POS        (13) //bit13 ~18
#define ECC_CTRL_REG2_SECTSIZE_POS       (0) //bit0 ~12

#define ECC_CTRL_REG3_GOODCNT_POS      (25) //bit25~30
#define ECC_CTRL_REG3_MAXFLIP_POS      (18) //bit18~24
#define ECC_CTRL_REG3_BUFFER_POS       (0) //bit0~17

#define ECC_RESULT_MAXFLIP  ((REG32(ECC_CTRL_REG3) >> ECC_CTRL_REG3_MAXFLIP_POS) & 0x7F)

#define ECC_CHECK_NO_ERRBIT        0
#define ECC_CHECK_SOME_ERRBIT      1
#define ECC_CHECK_ERROR            2
#define ECC_CHECK_ALL_FF           3


typedef enum
{
    ECC_MODE_4BITS = 0, //  4 bits ecc requirement per section
    ECC_MODE_8BITS,     //  8 bits ecc requirement per section
    ECC_MODE_12BITS,    //  12 bits ecc requirement per section
    ECC_MODE_24BITS,    //  24 bits ecc requirement per section
    ECC_MODE_40BITS,    //  40 bits ecc requirement per section    
    ECC_MODE_44BITS,    //  44 bits ecc requirement per section
    ECC_MODE_60BITS,    //  60 bits ecc requirement per section
    ECC_MODE_72BITS,    //  72 bits ecc requirement per section    
    ECC_NONE
}E_ECC_MODE;


#define ECC_PARITY_LEN_MODE0    7       //error correct code length under  4 bit ecc
#define ECC_PARITY_LEN_MODE1    14      //error correct code length under  8 bit ecc
#define ECC_PARITY_LEN_MODE2    21      //error correct code length under  12 bit ecc
#define ECC_PARITY_LEN_MODE3    42      //error correct code length under  24 bit ecc
#define ECC_PARITY_LEN_MODE4    70      //error correct code length under  40 bit ecc
#define ECC_PARITY_LEN_MODE5    77      //error correct code length under  44 bit ecc
#define ECC_PARITY_LEN_MODE6    105     //error correct code length under  60 bit ecc
#define ECC_PARITY_LEN_MODE7    126     //error correct code length under  72 bit ecc
#define ECC_PARITY_LEN_MODE_NONE    0     //error correct code length under  72 bit ecc

#define NAND_SMALL_PAGE_PHY_SIZE    528
#define RANDOM_SEED_CNT 128
#define PAGE0_SIZE   (NAND_SMALL_PAGE_PHY_SIZE - ECC_PARITY_LEN_MODE7)
#define PAGE0_ECC    ECC_MODE_72BITS
#define BOOTSECT_SIZE    512
#define BOOTPAGE_LIMIT   7168
//bit4 表示ECC类型，0为4 bit/512B，1为8 bit/512B，2为12 bit/512B，3为16 bit/512B，4为24 bit/1024B，5为32 bit/1024B，6为40 bit/1024B，7为44 bit/1024B，8为60 bit/1024B，9为72 bit/1024B
#define Type2Mode(EccType) (ECC_TYPE_24BITS > EccType ? EccType : (ECC_TYPE_24BITS == EccType) ? ECC_MODE_24BITS : EccType - 2)

#define clear_interface_status()     do \
{   \
    REG32(NFC_CTRL_REG) |= NFC_CTRL_REG_STA_CLR_BIT; \
    REG32(NFC_CTRL_REG) &= ~NFC_CTRL_REG_STA_CLR_BIT; \
    REG32(NFC_CTRL_REG) = 0;    \
}while(0)


typedef enum
{
    AREA_P0,
    AREA_BOOT,
    AREA_USER,
    AREA_CNT
}E_NAND_AREA;


extern T_U32    g_clk_map;
#pragma arm section zidata = "_drvbootbss_"

//**********************static variable definition**********************//
static T_U32    s_nNfcRet;
static T_U32    s_nDataLen;
static T_U32    s_nRandConfig;
static T_U8     s_bWrite;
static T_U8     s_nMaxFlipbit;
static T_U8     s_nGoodSectCnt;
static T_U8     s_nPhyPos[DRV_SUPPORT_NAND];
static T_U8     s_nTrc;//nand orignal trc, unit ns
static T_U8     s_nDelay;//board trc delay
static T_BOOL   s_bRandomizer;
static T_U8     s_nActChip;

#if NAND_SMALL_PAGE > 0
static T_U8 s_aPageBuf[NAND_SMALL_PAGE_PHY_SIZE];
#endif
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_U8 s_aNfcTimeOut[] = NAND_NFC_TIMEOUT;
static const T_U8 s_aEccTimeOut[] = NAND_ECC_TIMEOUT;
static const T_U8 s_aDataLenInfo[] = NAND_DATA_LEN_INFO;
static const T_U8 s_aRandomizerInfo[] = NAND_RANDOMIZER_INFO;

/*several fixed values for data length register , 
   value(bit[19~16]) + 1 ,  the asic cycle num of a RE# / WE# signal
   value (bit[15~12]) + 1, the cycle at whose beginning the WE# falls
   value (bit[11~8]) + 1, the cycle at whose beginning the WE# rises
   value (bit[7~4]) + 1, the cycle at whose beginning the RE# falls
   value( bit[3~0]) + 1, the cycle at whose beginning the RE# rises
*/
static const T_U32 s_aDataLen[] =
{
    0x10101,0x20101,0x30202,0x40303,
    0x50303,0x60404,0x70505,0x81616,
    0x92727,0xa2828,0xb2828,0xc2929,
    0xd3a3a,0xe3b3b,0xf5c5c
};

static const T_U8 s_aEccParityLen[] =
{
    ECC_PARITY_LEN_MODE0,
    ECC_PARITY_LEN_MODE1,
    ECC_PARITY_LEN_MODE2,
    ECC_PARITY_LEN_MODE3,
    ECC_PARITY_LEN_MODE4,
    ECC_PARITY_LEN_MODE5,
    ECC_PARITY_LEN_MODE6,
    ECC_PARITY_LEN_MODE7,
    ECC_PARITY_LEN_MODE_NONE
};

/*
here are 128 randomize seeds suggested in Samsung randomzier note.
they will be provided to the AK Randomizer to generate random pattern 
separately for 128 pages in a block. when the page amount is over 128, 
the (128 + n)th page share the same seed with nth page.
*/
static const T_U16 s_aSeed[RANDOM_SEED_CNT] = 
{
    0x576A, 0x05E8, 0x629D, 0x45A3, 0x649C, 0x4BF0, 0x2342, 0x272E, 0x7358, 0x4FF3, 0x73EC, 0x5F70, 0x7A60, 0x1AD8, 0x3472, 0x3612,
    0x224F, 0x0454, 0x030E, 0x70A5, 0x7809, 0x2521, 0x48F4, 0x5A2D, 0x492A, 0x043D, 0x7F61, 0x3969, 0x517A, 0x3B42, 0x769D, 0x0647, 
    0x7E2A, 0x1383, 0x49D9, 0x07B8, 0x2578, 0x4EEC, 0x4423, 0x352F, 0x5B22, 0x72B9, 0x367B, 0x24B6, 0x7E8E, 0x2318, 0x6BD0, 0x5519, 
    0x1783, 0x18A7, 0x7B6E, 0x7602, 0x4B7F, 0x3648, 0x2C53, 0x6B99, 0x0C23, 0x67CF, 0x7E0E, 0x4D8C, 0x5079, 0x209D, 0x244A, 0x747B, 
    0x350B, 0x0E4D, 0x7004, 0x6AC3, 0x7F3E, 0x21F5, 0x7A15, 0x2379, 0x1517, 0x1ABA, 0x4E77, 0x15A1, 0x04FA, 0x2D61, 0x253A, 0x1302, 
    0x1F63, 0x5AB3, 0x049A, 0x5AE8, 0x1CD7, 0x4A00, 0x30C8, 0x3247, 0x729C, 0x5034, 0x2B0E, 0x57F2, 0x00E4, 0x575B, 0x6192, 0x38F8, 
    0x2F6A, 0x0C14, 0x45FC, 0x41DF, 0x38DA, 0x7AE1, 0x7322, 0x62DF, 0x5E39, 0x0E64, 0x6D85, 0x5951, 0x5937, 0x6281, 0x33A1, 0x6A32,
    0x3A5A, 0x2BAC, 0x743A, 0x5E74, 0x3B2E, 0x7EC7, 0x4FD2, 0x5D28, 0x751F, 0x3EF8, 0x39B1, 0x4E49, 0x746B, 0x6EF6, 0x44BE, 0x6DB7
};

static const T_NAND_ECC_CTRL   s_aAk10LEccCtrl[AREA_CNT] =
{
    {
        PAGE0_SIZE, //s_aAk10LEccCtrl[AREA_P0].nMainLen according to rom code
        0,          //s_aAk10LEccCtrl[AREA_P0].nSectOffset never be used
        PAGE0_ECC,  //s_aAk10LEccCtrl[AREA_P0].nMainEcc according to rom code
        0,          //s_aAk10LEccCtrl[AREA_P0].nAddEcc according to rom code
        0,          //s_aAk10LEccCtrl[AREA_P0].nAddLen according to rom code
        AK_TRUE,    //s_aAk10LEccCtrl[AREA_P0].bSeperated
        1,          //s_aAk10LEccCtrl[AREA_P0].nMainSectCnt according to rom code
    },
    {
        BOOTSECT_SIZE,//pEccCtrl[AREA_BOOT].nMainLenaccording to the rom code 
        0,//pEccCtrl[AREA_BOOT].nSectOffset  according to the rom code 
        0,//just a initial value
        0,//pEccCtrl[AREA_BOOT].nAddEcc  according to the rom code 
        0,//pEccCtrl[AREA_BOOT].nAddLen according to the rom code 
        AK_TRUE,//pEccCtrl[AREA_BOOT].bSeperated 
        0,//
    }
};    

/*
 cycle sequence to change column address to 0
*/
static const T_U16 m_aChgClmnOps[] =
{
    CMD_CYCLE(NFLASH_READ_CHGC_1),
    ADDR_CYCLE(0),
    ADDR_CYCLE(0),
    CMD_CYCLE(NFLASH_READ_CHGC_2),
    END_CYCLE
};

#pragma arm section rodata

/*adjust the data length with the asic frequency and tRC provided*/
static T_U32 datalen_adjust(T_U32 freq, T_U32 trc)
{
    T_U16 data_opt_len;
    T_U16 freq_temp = freq / 1000000;

    if (0 == freq_temp)//for freq <  1M
    {
        freq_temp = 1;
    }
    
    data_opt_len = trc * freq_temp / 1000;
    
    if (0 == (trc * freq_temp) % 1000)
    {
        data_opt_len--;
    }
    
    if (data_opt_len > (sizeof(s_aDataLen) / sizeof(s_aDataLen[0])) - 1)
    {
        data_opt_len = (sizeof(s_aDataLen) / sizeof(s_aDataLen[0])) - 1;
    }

    return s_aDataLen[data_opt_len];
}

/**
 * @brief       start cycle sequence configured
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_VOID
 */
static T_VOID cmd_start(T_VOID)
{
    volatile T_U32 ctl_reg_val = REG32(NFC_CTRL_REG);

    ctl_reg_val |= (NFC_CTRL_REG_CMD_VALID_BIT | NFC_CTRL_REG_CE_SAVE_BIT);
    
    if (INVALID_GPIO  == s_nPhyPos[s_nActChip])
    { 
        ctl_reg_val |= NFC_CTRL_REG_CHIP_CONF(s_nActChip);
    }
    else
    {//already be low when selected
       // gpio_set_pin_level(s_nPhyPos[s_nActChip], GPIO_LEVEL_LOW);//enable CE
    }
    
    REG32(NFC_CTRL_REG) = ctl_reg_val;
}

/**
 * @brief       wait for cycle sequence finished
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_BOOL
 * @retval      AK_FALSE, timout; AK_TRUE, success
 */
static T_BOOL cmd_end(T_VOID)
{
    T_U32 trigger = 0xFFFFFF;

    while (0 != trigger--)
    {
        if (0 != (REG32(NFC_CTRL_REG) & NFC_CTRL_REG_CMD_DONE_BIT)) 
        {
            return AK_TRUE;
        }
    }
    
    akerror(s_aNfcTimeOut, 0, AK_TRUE);
    sys_module_reset(eVME_NANDFLASH_CLK);
    REG32(NFC_CMD_LEN_REG) = 0xf5ad1; 
    REG32(NFC_DAT_LEN_REG) = s_nDataLen;
    return AK_FALSE;
}
#pragma arm section code = "_drvbootinit_"

/**
 * @brief       initial nand flash controller
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pCePos the GPIOs connected to nandflash #Ce, 0xFF for default pin 
 * @param       [in]nCeCnt the GPIOs amount to be set, not more than 4 
 * @return      T_VOID
 */
T_VOID  nfc_init(T_U8 *pCePos, T_U8 nCeCnt)
{
    T_U8 i;

    for (i = 0; (i < nCeCnt) && (i < DRV_SUPPORT_NAND);)
    {
        s_nPhyPos[i++] = *pCePos++;
    }
    
    s_nDataLen = 0xF5C5C;
    s_nTrc  = 80;//nand orignal trc, unit ns
    s_nDelay = 0;//board trc delay
    s_nActChip = 0xFF;
    s_bRandomizer = AK_FALSE;
    sys_module_enable(eVME_NANDFLASH_CLK, AK_TRUE);
    REG32(NFC_CMD_LEN_REG) = 0xf5ad1;    //cmd timing, it'll never changed
    REG32(NFC_DAT_LEN_REG) = 0xf5c5c;   //will be changed according to tRC
    REG32(NFC_RAND_ENC_REG) = ~(NFC_RAND_EN_BIT);
    REG32(NFC_RAND_DEC_REG) = ~(NFC_RAND_EN_BIT);
    sys_module_enable(eVME_NANDFLASH_CLK, AK_FALSE);
       
}


T_BOOL nfc_init_randomizer(T_U32 nRandPageSize)
{
    //just to notify that randomizer will be applied
    akerror(s_aRandomizerInfo, 0, AK_TRUE);
    return AK_TRUE;
}
#pragma arm section code

T_VOID nfc_config_randomize(T_U16 nPageAddr, T_U16 nColumnAddr, T_BOOL bEnable, T_U8 bWrite)
{
    s_bRandomizer  = bEnable;
    s_nRandConfig = s_aSeed[nPageAddr % RANDOM_SEED_CNT] | NFC_RAND_EN_BIT;
}

/**
 * @brief       select/deselect a nandflash target
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nTarget target nandflash
 * @param       [in]bSelect AK_TRUE to select, AK_FALSE to deselect
 * @return      T_BOOL
 */
T_BOOL nfc_select(T_U8 nTarget, T_BOOL bSelect)
{
    //only when additional #ce is needed, this block is essential
    if (s_nPhyPos[nTarget] < INVALID_GPIO)
    {
        gpio_set_pin_dir(s_nPhyPos[nTarget], GPIO_DIR_OUTPUT);
        gpio_set_pin_level(s_nPhyPos[nTarget], bSelect ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH);
    }

    if (bSelect)
    {
        g_clk_map |= CLK_NFC_EN;
        store_int(INT_EN_L2);
        sys_module_enable(eVME_NANDFLASH_CLK, AK_TRUE);
        sys_share_pin_lock(ePIN_AS_NANDFLASH);
        s_nActChip = nTarget;
    }
    else
    {
        s_nActChip = 0xFF;
        sys_module_enable(eVME_NANDFLASH_CLK, AK_FALSE);
        sys_share_pin_unlock(ePIN_AS_NANDFLASH);
        restore_int();
        g_clk_map &= ~CLK_NFC_EN;
    }
    
    return AK_TRUE;
}

/**
 * @brief       issue nand cycles including command cycle, address cycle, RB cycle etc.
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nCmdSeq the cycle sequence
 * @return      T_BOOL
 */
T_BOOL nfc_cycle(T_U32 nCmdSeq,...)
{
    volatile T_U32 *pCmdReg = (T_U32 *)NFC_CMD_REG;
    T_VOID *pArg = (T_VOID *)&nCmdSeq;
    T_U32   nCycleVal;
    T_U16   nCycle;
    T_U8    nCycleType;
    T_BOOL  bArr = AK_FALSE;
    T_U8    nDataCnt = 0;
    T_U8    aWdata[10];
    
    clear_interface_status();

    //this block is added in purpose of saving device code text compiled.
    //course pushing every parameter in stack take more instruction than 
    //just pushing an array and a flag in.
    if (nCmdSeq == NAND_CYCLE_ARR_FLAG)
    {
        pArg = (T_VOID *)((T_U32)pArg + 4);
        pArg = (T_VOID *)(*(T_U32 *)pArg);
        bArr = AK_TRUE;
    }

    while (1)
    {
        nCycle = *(T_U16 *)pArg;
        
        if (AK_TRUE == bArr)
        {
            pArg = (T_VOID *)((T_U32)pArg + 2);
        }
        else
        {
            pArg = (T_VOID *)((T_U32)pArg + 4);
        }
        
        nCycleType = GET_NAND_CYCLY_TYPE(nCycle);
        nCycleVal = (nCycle & NAND_CYCLE_VALUE_MASK) << NFC_CMD_REG_INFO_POS;

        switch (nCycleType)
        {
            case COMMAND_C://command cycle
            {
                nCycleVal |= NFC_CMD_REG_CMD_CONF;
            }
            break;
            case ADDRESS_C://address cycle
            {
                nCycleVal |= NFC_CMD_REG_ADD_CONF;
            }
            break;
            case DELAY_C://delay several k asic. will it be used?
            {
                 nCycleVal |= NFC_CMD_REG_DELAY_BIT;
            }
            break;
            case WDATA_C://output data cycle
            {
                nCycleVal = (0 << NFC_CMD_REG_INFO_POS) | NFC_CMD_REG_WDATA_CONF;
                aWdata[nDataCnt] = nCycle & NAND_CYCLE_VALUE_MASK;//get the data value
                nDataCnt++;
            }
            break;
            case READYB_C://wait for the RB signal
            {
                nCycleVal = NFC_CMD_REG_WAIT_JUMP_BIT;
            }
            break;
            case NULL_C://do nothing
            {
                continue;
            }
            break;
            case END_C://ending flag , way to exit
            {
                pCmdReg[-1] |=  NFC_CMD_REG_LAST_BIT;
                
                if (nDataCnt > 0)//we use ecc sub module to deliver the data
                {
                    //config and enable the ECC sub-Module
                    REG32(ECC_CTRL_REG1) = ECC_CTRL_REG1_WRITE_BIT | ECC_CTRL_REG1_NFC_EN_BIT | ECC_CTRL_REG1_ADDR_CLR_BIT | ECC_CTRL_REG1_RESULT_NO_OK_BIT | ECC_CTRL_REG1_NO_ERR_BIT | ECC_CTRL_REG1_END_BIT;
                    REG32(ECC_CTRL_REG2) = (nDataCnt << ECC_CTRL_REG2_SECTSIZE_POS) | (1 << ECC_CTRL_REG2_SECTCNT_POS);
                    REG32(ECC_CTRL_REG3) = MMU_Vaddr2Paddr((T_U32)aWdata) & 0x3FFFF;
                    REG32(ECC_CTRL_REG1) = ECC_CTRL_REG1_WRITE_BIT | ECC_CTRL_REG1_NFC_EN_BIT | ECC_CTRL_REG1_ADDR_CLR_BIT | ECC_CTRL_REG1_RESULT_NO_OK_BIT | ECC_CTRL_REG1_NO_ERR_BIT | ECC_CTRL_REG1_END_BIT | ECC_CTRL_REG1_START_BIT;
                } 

                cmd_start();
                
                return cmd_end();
            }

        }
        
        *pCmdReg = nCycleVal;
        pCmdReg++;
    }

}
#pragma arm section code = "_drvbootinit_"

/**
 * @brief           get ecc strategy from nand controller
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [out]ppEccCtrl the ecc strategy
 * @param       [in]nPageSize the physical page size of nandflash
 * @param       [in]nEccType ecc requirement 
 * @return      T_VOID
 */
T_VOID nfc_get_ecc(T_NAND_ECC_CTRL **ppEccCtrl, T_U32 nPageSize, T_U8 nEccType)
{
    T_U16 nEccSectLen;
    E_ECC_MODE eEccMode;
    T_U16 nPageLimit;
    T_U8 i;
    T_NAND_ECC_CTRL *pEccCtrl = (T_NAND_ECC_CTRL *)s_aAk10LEccCtrl;
#if 0
    s_aAk11EccCtrl[AREA_P0].nMainLen = PAGE0_SIZE;//according to rom code
    s_aAk11EccCtrl[AREA_P0].nSectOffset = 0;//never be used
    s_aAk11EccCtrl[AREA_P0].nMainEcc = PAGE0_ECC;//according to rom code
    s_aAk11EccCtrl[AREA_P0].nAddEcc   = 0;//according to rom code
    s_aAk11EccCtrl[AREA_P0].nAddLen = 0;//according to rom code
    s_aAk11EccCtrl[AREA_P0].bSeperated = AK_TRUE;//
    s_aAk11EccCtrl[AREA_P0].nMainSectCnt = 1;////according to rom code
#endif
    eEccMode = Type2Mode(nEccType);//conver the ecctype in  nand param to eccmode in ak chip 
    nPageLimit = nPageSize > BOOTPAGE_LIMIT ? BOOTPAGE_LIMIT : nPageSize;//according to the rom code 
    //pEccCtrl[AREA_BOOT].nMainLen = BOOTSECT_SIZE;//according to the rom code 
    //pEccCtrl[AREA_BOOT].nSectOffset = 0;//according to the rom code 
    //an ugly sentence for Micro 29F32G08CBACA
    pEccCtrl[AREA_BOOT].nMainEcc = ((ECC_TYPE_24BITS == nEccType) && (4096 == nPageSize)) ? ECC_TYPE_12BITS : eEccMode;
    //pEccCtrl[AREA_BOOT].nAddEcc   = 0;//according to the rom code 
    //pEccCtrl[AREA_BOOT].nAddLen = 0;//according to the rom code 
    //pEccCtrl[AREA_BOOT].bSeperated = AK_TRUE;//
    pEccCtrl[AREA_BOOT].nMainSectCnt = nPageLimit / BOOTSECT_SIZE;

    nEccSectLen = nEccType > ECC_TYPE_16BITS ? 1024 : 512;
    pEccCtrl[AREA_USER].nMainLen = nEccSectLen;
    pEccCtrl[AREA_USER].nSectOffset = (512 == nPageSize) ? 0 : nEccSectLen + s_aEccParityLen[eEccMode];
    pEccCtrl[AREA_USER].nMainEcc  = eEccMode;
    pEccCtrl[AREA_USER].nAddEcc   = ECC_MODE_4BITS;
    pEccCtrl[AREA_USER].nAddLen = NAND_MAX_ADD_LEN;
    //only small page nand store the additional data together with main data
    pEccCtrl[AREA_USER].bSeperated = (512 == nPageSize) ? AK_FALSE : AK_TRUE;
    pEccCtrl[AREA_USER].nMainSectCnt = nPageSize / nEccSectLen;

    for (i = 0; i < AREA_CNT; i++)
    {
        *ppEccCtrl++ = pEccCtrl++;
    }
    
}


/**
 * @brief       change the tRC manually
 *                  if the board signal environment was quite good, 
 *                  try to decrease the tRC to improve performance.
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nTrc the time for read/write cycle
 * @param       [in]nDelay for some reason, user can increase the tRC by nDelay with nTrc equals zero
 * @return      T_VOID
 */
T_VOID nfc_configtRC(T_U8 nTrc, T_U8 nDelay)
{
    if (nTrc)
    {
        s_nTrc = nTrc;
    }

    s_nDelay = nDelay;

    nfc_timing_adjust(clk_get_asic());
}

#pragma arm section code

/**
 * @brief       interface provided for clock module to adjust 
    nand timing with a new asic
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]nAsic the new asic, unit Hz
 * @return      T_VOID
 */
T_VOID  nfc_timing_adjust(T_U32 nAsic)
{
     s_nDataLen = datalen_adjust(nAsic, s_nTrc + s_nDelay);
     
     sys_module_enable(eVME_NANDFLASH_CLK, AK_TRUE);
     REG32(NFC_DAT_LEN_REG) = s_nDataLen;//change the timing to speed up 
     sys_module_enable(eVME_NANDFLASH_CLK, AK_FALSE);
     akerror(s_aDataLenInfo, s_nDataLen, AK_TRUE);
}

/**
 * @brief       read data through register port, one of the two 
    ports to transfer data. the other one is l2 port
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pBuf
 * @param       [in]nBufLen
 * @return      T_U32
 */
static T_U32 reg_read(T_U8 *pBuf, T_U8 nBufLen)
{
    T_U8 i;
    T_U32 nRet = 0;
    
    clear_interface_status();
    REG32(NFC_CMD_REG) = ((nBufLen - 1) << NFC_CMD_REG_INFO_POS) | NFC_CMD_REG_REG_IN_CONF | NFC_CMD_REG_LAST_BIT;
    cmd_start();

    if (AK_FALSE == cmd_end())
        return NAND_FAIL_NFC_TIMEOUT;
    
    for (i = 0; i < nBufLen; i++)
    {
        if (i < 4)
        {
            pBuf[i] = (T_U8)(0xFF & (REG32(NFC_DAT_REG1) >> (i << 3))) ;
            //akerror("Reg_\n",pBuf[i],1);
        }
        else
        {
            pBuf[i] = (T_U8)(0xFF & (REG32(NFC_DAT_REG2) >> ((i - 4) << 3))) ;
        }
    }

    return nRet;
}

/**
 * @brief       wait for ecc submodule finish transferring data
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_BOOL
 */
static T_BOOL __inline wait_ecc_complete()
{
    T_U32 nTrigger = 0xFFFFFF;//protect the fun from being trapped
    T_U32 nExpectbit;
    volatile T_U32 nRegTmp = REG32(ECC_CTRL_REG1);
    
    if (ECC_CTRL_REG1_DEC_EN_BIT == (nRegTmp & 0x3))//ECC module in decode mode
    {
        nExpectbit = ECC_CTRL_REG1_DEC_RDY_BIT;
    }
    else if (ECC_CTRL_REG1_ENC_EN_BIT == (nRegTmp & 0x3))//ECC module in encode mode
    {
        nExpectbit = ECC_CTRL_REG1_ENC_RDY_BIT;
    }
    else//ECC module in bypass mode
    {
        nExpectbit = ECC_CTRL_REG1_END_BIT;
    }
    
    while (0 != nTrigger--)
    {
       nRegTmp = REG32(ECC_CTRL_REG1);
       
        if (0 != (nRegTmp & nExpectbit)) 
        {
            return AK_TRUE;
        }
    }
    
    akerror(s_aEccTimeOut, REG32(ECC_CTRL_REG1), AK_TRUE);
    return AK_FALSE;
} 

/**
 * @brief       wait for ecc submodule finish transferring data
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_U8
 * @retval      ECC_CHECK_NO_ERRBIT,  not a single bit flips
 *                  ECC_CHECK_ERROR,  too many flips flip to correct
 *                  ECC_CHECK_SOME_ERRBIT,  some bits flip but can be corrected
 */
static T_U8  __inline check_ecc_result(T_VOID)
{
    T_U8 ret = ECC_CHECK_NO_ERRBIT;
    T_U32 reg_val = REG32(ECC_CTRL_REG1);
    
    if (ECC_CTRL_REG1_NO_ERR_BIT == (reg_val & ECC_CTRL_REG1_NO_ERR_BIT))
    {
    }
    else if (ECC_CTRL_REG1_RESULT_NO_OK_BIT == (reg_val & ECC_CTRL_REG1_RESULT_NO_OK_BIT))
    {
        ret = ECC_CHECK_ERROR;
    }
    else
    {
        ret = ECC_CHECK_SOME_ERRBIT;
    }

    return ret;
}

/**
 * @brief       judge whether a page is clean, hasn't been programmed 
    after erased
 * @author      Yang Yiming
 * @date        2012-12-25
 * @return      T_BOOL
 */
static  T_BOOL  is_all_FF(T_U8 *pBuf, T_U32 nBuflen)
{
    T_U32 j;
    T_U8 unmatch_byte;
    T_U8 aBuf[24];
    unmatch_byte = 0;
    
#if NAND_LARGE_PAGE > 0
    if (s_bRandomizer)//special op to identify clean page,  try to fix me.
    {
        //issue "Change READ column " cmd ,to change the column to 0x00 0x00.
        nfc_cycle(NAND_CYCLE_ARR_FLAG, m_aChgClmnOps);
        /*when buflen << 8, nfc transfer data through the register port, 
                through which data won't be randomized.*/
        reg_read(aBuf, 8);
        reg_read(aBuf + 8, 8);
        reg_read(aBuf + 16, 8);
        nBuflen = 24;
        pBuf = aBuf;
    }
#endif
    for (j = 0; j < nBuflen; j++)
    {
        if (0xff != *(pBuf + j))
        {
            if (unmatch_byte++ > 3)// "3" just a experiential value
            {
                return AK_FALSE;
            }
        }
    }
    
    return AK_TRUE;
}

/**
 * @brief       configure the controller to generate data cycles 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   eMode, the ecc mode of data to be transferred
 * @param[in]   nSectLen, the valid data length for ecc section,  parity data excluded
 * @param[in]   nSectCnt, the sector amount
 * @return      T_VOID
 */
static T_VOID __inline config_data_cycle(E_ECC_MODE eMode, 
    T_U16 nSectLen, T_U16 nSectCnt)
{
    T_U32 nRegVal;
    T_U32 nCycleCnt;
    T_U32 *pReg = (T_U32 *)NFC_CMD_REG;
    
    clear_interface_status();//clear all status of the nfc , including the statemachine 
    
    if (s_bWrite)
    {
        nRegVal = (NFC_CMD_REG_WE_BIT | NFC_CMD_REG_CNT_EN_BIT | NFC_CMD_REG_DAT_BIT);
    }
    else
    {
        nRegVal = (NFC_CMD_REG_RE_BIT | NFC_CMD_REG_CNT_EN_BIT | NFC_CMD_REG_DAT_BIT);
    }
    
    //the ECC parity data must be considered, and the partity data size for bypass mode is 0 byte
    nCycleCnt = nSectLen + s_aEccParityLen[eMode];
    
    nRegVal |= ((nCycleCnt - 1) << NFC_CMD_REG_INFO_POS);

    while (0 < nSectCnt--)//loop to config the data cycle for every section
    {
        *pReg++ = nRegVal;
    }

    pReg[-1] |= NFC_CMD_REG_LAST_BIT;
    
    cmd_start();
}

/**
 * @brief       read / write data from /to nand bus 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   pBuf, the data buffer
 * @param[in]   eMode, the ecc mode of data to be transferred
 * @param[in]   nSectLen, the valid data length for ecc section,  parity data excluded
 * @param[in]   nSectCnt, the sector amount
 * @return      T_VOID
 */
static T_VOID  read_write_sections(T_U8  *pBuf, E_ECC_MODE eEccMode,
    T_U16 nValidLen, T_U16 nSectCnt)
{
    T_U32 nCtrlRegVal = ECC_CTRL_REG1_NFC_EN_BIT | ECC_CTRL_REG1_ADDR_CLR_BIT
                     | ECC_CTRL_REG1_RESULT_NO_OK_BIT | ECC_CTRL_REG1_NO_ERR_BIT
                     | ECC_CTRL_REG1_END_BIT;
    T_U32 nModeRegVal = 0;
    T_U32 nL2Addr;
    T_U32 nL2AddrEnd;
    T_U8 *pL2Buf = AK_NULL;
    T_U16 nBytePerTime;
    T_U8 nMaxFlip = 0;
    T_U8 nSectCntPerTime;
    T_U8 nGoodSect = 0;
    T_U8 nTmp;
    T_U8 bBuf;
    volatile T_U8 nToDirty;
    
    if (0 == nValidLen || 0 == nSectCnt)
    {
        s_nNfcRet = NAND_FAIL_PARAM;
        return;
    }

    config_data_cycle(eEccMode, nValidLen, nSectCnt);

    if (s_bWrite)//caculate the value for ECC CTRL REG1,
    {
        nCtrlRegVal |= (ECC_CTRL_REG1_ENC_EN_BIT | ECC_CTRL_REG1_WRITE_BIT);
    }
    else
    {
        nCtrlRegVal |= (ECC_CTRL_REG1_DEC_EN_BIT | ECC_CTRL_REG1_READ_BIT);
    }

    if (ECC_NONE == eEccMode)//clear the decode/encode bit in bypass mode
    {
        nCtrlRegVal &= ECC_CTRL_REG1_NO_DEC_ENC;
    }
    
    if (s_bRandomizer)
    {
        /*when Randomizer needed, 
            we config the random seed per ecc section, expect the boot area
            , and ecc sections of the boot area are always less than 1K*/
        if (nValidLen >= 1024)
        {
            nSectCntPerTime = 1;
        }
        else
        {
            nSectCntPerTime = nSectCnt;
        }
    }
    else
    {
        /* to keep the data size less than 4097 &  guarantee 
            that every transfermission get a same size*/
        if (nSectCnt & 0x1)
        {
            nSectCntPerTime = 1;
        }
        else if (nSectCnt & 0x2)
        {
            nSectCntPerTime = 2;
        }
        else
        {
            nSectCntPerTime = 4;
        }
    }

    nBytePerTime  = nValidLen * nSectCntPerTime;

    //caculate the value for ECC CTRL REG2
    nModeRegVal  = (nValidLen << ECC_CTRL_REG2_SECTSIZE_POS);    
    nModeRegVal |= (nSectCntPerTime << ECC_CTRL_REG2_SECTCNT_POS);
    nModeRegVal |= (eEccMode << ECC_CTRL_REG2_MODE_POS);

    while (nSectCnt > nGoodSect)
    {
        nL2Addr = MMU_Vaddr2Paddr((T_U32)pBuf);//convert the Virtual Address to L2 Physic Address
        nL2AddrEnd = MMU_Vaddr2Paddr((pBuf + nBytePerTime - 1));
        if ((0 == nL2Addr)|| (0 == nL2AddrEnd))//the buffer must be mapped in Physic Address
        {
            s_nNfcRet = NAND_FAIL_PARAM;
            goto L_EXIT;
        }

        if (s_bRandomizer)//config the Randomizer if nessary
        {
            REG32(NFC_RAND_ENC_REG) = s_nRandConfig;
            REG32(NFC_RAND_DEC_REG) = s_nRandConfig;
        }

        //config and enable the ECC sub-Module
        REG32(ECC_CTRL_REG1) = nCtrlRegVal;
        REG32(ECC_CTRL_REG2) = nModeRegVal;
        
        /*Using the DMA to transfer data have to guarantee that
                the Physical Memory of the Virtual Memory is continuous.
                If not, we remap_alloc a temporary buffer
              */
#ifndef UNSUPPORT_REMAP
        if ((nL2Addr + nBytePerTime - 1) != nL2AddrEnd) 
        {
            bBuf = AK_TRUE;
            
            if (AK_NULL == pL2Buf)
            { 
                // 4 k ought to be enough for every area,  nBytePerTime < 4 k
                pL2Buf = (T_U8 *)remap_alloc(4096); 
                if (AK_NULL == pL2Buf)
                {
                    s_nNfcRet = NAND_FAIL_PARAM;
                    goto L_EXIT;
                }
            }

            if (s_bWrite)//Writing operation, copy data from pBuf to temporary buff
            {
                memcpy(pL2Buf, pBuf, nBytePerTime);
            }

            REG32(ECC_CTRL_REG3) =  MMU_Vaddr2Paddr(pL2Buf) & 0x3FFFF;
        }
        else
#endif
        {
            bBuf = AK_FALSE;
            REG32(ECC_CTRL_REG3) = nL2Addr & 0x3FFFF;
        }

        //config and enable the ECC sub-Module
        REG32(ECC_CTRL_REG1) = (nCtrlRegVal | ECC_CTRL_REG1_START_BIT);

        if (AK_FALSE == wait_ecc_complete())// wait for transmission finished
        {
            s_nNfcRet = NAND_FAIL_ECC;
            goto L_EXIT;
        }

        //must invalidate cache here or strange data will get in the next transmission
        MMU_InvalidateDCache();

        if (s_bWrite)
        {
            nGoodSect += nSectCntPerTime;//whatever, the data's out
        }
        else
        {
            switch (check_ecc_result())
            {   
                case ECC_CHECK_ERROR:
                    nTmp = (REG32(ECC_CTRL_REG3) >> ECC_CTRL_REG3_GOODCNT_POS) & 0x3F;
                    nGoodSect += nTmp;
                    //judge whether a clean page read
                    if ((0 == nGoodSect) 
                        && (AK_TRUE == 
                            is_all_FF((T_U8 *) bBuf ? pL2Buf: pBuf, nValidLen)))
                    {
                        /*return 0xFF for all section, when reading the clean page */
                        nMaxFlip = 0;
                        nGoodSect = nSectCnt;
                        akmemset((T_U32 *)pBuf, 0xFFFFFFFF, nSectCnt * nValidLen >> 2);
                    }
                    else
                    {
                        s_nNfcRet = NAND_FAIL_ECC; 
                    }
                    
                    goto L_EXIT;
                case ECC_CHECK_SOME_ERRBIT:
                    nTmp = ECC_RESULT_MAXFLIP;//get the max flipbit num when fipbits found
                    nMaxFlip = nMaxFlip > nTmp ? nMaxFlip : nTmp;
                case ECC_CHECK_NO_ERRBIT:
                    nGoodSect += nSectCntPerTime;
                    break;
            }
        }
        
        //Reading operation, copy data from temporary buff to pBuf
        if (AK_FALSE == s_bWrite)
        {
            if (AK_TRUE  == bBuf)
            {
                memcpy(pBuf, pL2Buf, nBytePerTime);
            }
            else
            {
                //transferring data by DMA will not get the "Dirty flag" be set, 
                //to write the first byte and last byte through cpu
                nToDirty = pBuf[0];
                pBuf[0] = nToDirty;
                nToDirty = pBuf[nBytePerTime - 1];
                pBuf[nBytePerTime - 1] = nToDirty;
            }
        }
                
        pBuf += nBytePerTime;
    }

L_EXIT:
    
    if (s_bRandomizer)//disable the Randomizer
    {
        REG32(NFC_RAND_ENC_REG) =  ~(NFC_RAND_EN_BIT);
        REG32(NFC_RAND_DEC_REG) =  ~(NFC_RAND_EN_BIT);
    }
    
#ifndef UNSUPPORT_REMAP
    if (AK_NULL != pL2Buf)
    {
        remap_free(pL2Buf);
    }
    
#endif

    s_nMaxFlipbit  =  s_nMaxFlipbit > nMaxFlip ? s_nMaxFlipbit : nMaxFlip;
    s_nGoodSectCnt += nGoodSect;
}

/**
 * @brief       read / write data from /to nand bus 
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param[in]   pMain, the main data buffer
 * @param[in]   pAdd, the additional data buffer
 * @param[in]   nSectCnt, the sector amount 
 * @param[in]   pEccCtrl, struct contain ecc control information
 * @return      T_VOID
 */
static T_VOID read_write_page(T_U8  *pMain, T_U8 *pAdd, 
    T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl)
{
    T_U16 nBytesOneTime;
    s_nNfcRet = 0;
    s_nGoodSectCnt = 0;
    s_nMaxFlipbit = 0;

    if (AK_NULL == pEccCtrl)//bypass ECC mode
    {
        while (nSectCnt)
        {
            //transfer 2k bytes data per time
            nBytesOneTime = nSectCnt > 2048 ? 2048 : nSectCnt;
            nSectCnt -= nBytesOneTime;
            read_write_sections(pMain, ECC_NONE, nBytesOneTime, 1);
            pMain += nBytesOneTime;
        }
    }
    else
    {
#if NAND_LARGE_PAGE > 0
        if (pEccCtrl->bSeperated)
        {
            if ((AK_NULL != pMain) && (0 != nSectCnt))
            {
                read_write_sections(pMain, pEccCtrl->nMainEcc,
                    pEccCtrl->nMainLen, nSectCnt);
                if (NAND_FAIL_MASK & s_nNfcRet)
                {
                    goto L_EXIT;
                }
            }

            if ((AK_NULL != pAdd) && (0 != pEccCtrl->nAddLen))
            {
                read_write_sections(pAdd, pEccCtrl->nAddEcc, pEccCtrl->nAddLen, 1);
            }
        }
        else
#endif
        {
#if NAND_SMALL_PAGE > 0
            /* course the ecc section transmission not be divided into two steps in 
            chip L  , we must copy the main data and additonal to a continous buffer*/
            if (AK_TRUE == s_bWrite)
            {
                if (AK_NULL != pMain)//copy the main data
                {
                    memcpy(s_aPageBuf, pMain, pEccCtrl->nMainLen);
                }
                
                //copy the additional data
                if (AK_NULL != pAdd && (0 != pEccCtrl->nAddLen))
                {
                    memcpy(s_aPageBuf + pEccCtrl->nMainLen, pAdd, pEccCtrl->nAddLen);
                }
            }
            
            read_write_sections(s_aPageBuf, pEccCtrl->nMainEcc, 
                pEccCtrl->nMainLen + pEccCtrl->nAddLen, 1);

            if (AK_FALSE == s_bWrite)
            {
                if (AK_NULL != pMain)//copy the main data
                {
                    memcpy(pMain, s_aPageBuf, pEccCtrl->nMainLen);
                }
                
                if ((AK_NULL != pAdd) && (0 != pEccCtrl->nAddLen))//copy the additional data
                {
                    memcpy(pAdd, s_aPageBuf + pEccCtrl->nMainLen, pEccCtrl->nAddLen);
                }
            }
#endif
        }
    }
    
L_EXIT:
    SET_GOODSECT_CNT(s_nNfcRet, s_nGoodSectCnt);
    SET_MAXFLIP_CNT(s_nNfcRet, s_nMaxFlipbit);
}

/**
 * @brief           transfer data out to nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pMain the main data buffer
 * @param       [in]pAdd the additonal data buffer
 * @param       [in]nSectCnt the main data section count
 * @param       [in]pEccCtrl ecc strategy got from nfc_get_ecc
 * @return      T_NAND_RET
 */
T_U32 nfc_write(T_U8  *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl)
{
    s_bWrite = AK_TRUE;
    read_write_page(pMain, pAdd, nSectCnt, pEccCtrl);
    return s_nNfcRet;
}

/**
 * @brief           transfer data in from nandflash
 * @author      Yang Yiming
 * @date        2012-12-25
 * @param       [in]pMain the main data buffer
 * @param       [in]pAdd the additonal data buffer
 * @param       [in]nSectCnt the main data section count
 * @param       [in]pEccCtrl ecc strategy got from nfc_get_ecc
 * @return      T_NAND_RET
 */
T_U32 nfc_read(T_U8  *pMain, T_U8 *pAdd, T_U16 nSectCnt, T_NAND_ECC_CTRL *pEccCtrl)
{   
    if ((AK_NULL == pEccCtrl) && (NFC_REG_DAT_IN_LEN >= nSectCnt))
    {
        return reg_read(pMain, nSectCnt);
    }
    
    s_bWrite = AK_FALSE;  
    read_write_page(pMain, pAdd, nSectCnt, pEccCtrl);
    return s_nNfcRet;
}

#endif

