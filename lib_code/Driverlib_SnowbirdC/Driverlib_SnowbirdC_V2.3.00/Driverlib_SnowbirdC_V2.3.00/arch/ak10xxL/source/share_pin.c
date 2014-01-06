/*******************************************************************************
 * @file    share_pin.c
 * @brief   share pin configuration function.
 * Copyright (C) 2012 Anyka (GuangZhou) Technology Co., Ltd.
 * @author  ZGX
 * @date    2012-11-5
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "share_pin.h"
#include "drv_api.h"
#include "arch_interrupt.h"
#include "arch_init.h"
#include "drv_cfg.h"
#include "hal_errorstr.h"


#define L(m)                (1 << m)


#if (CHIP_SEL_10C > 0)

#define NANDFLASH_SHAREBIT  (L(ePIN_AS_NANDFLASH))

#define MCI1_SHAREBIT       (L(ePIN_AS_MCI1)|L(ePIN_AS_LCD))

#define MCI2_1LINE_SHAREBIT  (L(ePIN_AS_MCI2_1LINE)|L(ePIN_AS_MCI2_4LINE)|L(ePIN_AS_LCD)|L(ePIN_AS_UART1_HD))

#define MCI2_4LINE_SHAREBIT  (L(ePIN_AS_MCI2_1LINE)|L(ePIN_AS_MCI2_4LINE)|L(ePIN_AS_LCD)|L(ePIN_AS_UART1_HD))

#define LCD_SHAREBIT        (L(ePIN_AS_LCD)|L(ePIN_AS_MCI1)|L(ePIN_AS_MCI2_1LINE)|L(ePIN_AS_MCI2_4LINE)| \
                            L(ePIN_AS_SPI1)|L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)| \
                            L(ePIN_AS_UART2)|L(ePIN_AS_I2S)|L(ePIN_AS_IRDA))

#define SPI1_SHAREBIT       (L(ePIN_AS_SPI1)|L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_LCD))

#define SPI1_4LINE_SHAREBIT (L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_SPI1)|L(ePIN_AS_LCD)| \
                             L(ePIN_AS_CAMERA))

#define SPI2_SHAREBIT       (L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_CAMERA)| \
                            L(ePIN_AS_I2S)|L(ePIN_AS_LCD)|L(ePIN_AS_UART2))

#define SPI2_4LINE_SHAREBIT (L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_CAMERA)| \
                            L(ePIN_AS_I2S)|L(ePIN_AS_LCD)|L(ePIN_AS_UART2))

#define PCM_SHAREBIT        (L(ePIN_AS_PCM))

#define PWM_SHAREBIT        (L(ePIN_AS_PWM))
#define PWM2_SHAREBIT       (L(ePIN_AS_PWM2)|L(ePIN_AS_UART1))

#define UART1_SHAREBIT      (L(ePIN_AS_UART1)|L(ePIN_AS_UART1_HD)|L(ePIN_AS_PWM2))
#define UART1_HD_SHAREBIT   (L(ePIN_AS_UART1_HD)|L(ePIN_AS_UART1)|L(ePIN_AS_PWM2)| \
                            L(ePIN_AS_MCI2_1LINE)|L(ePIN_AS_MCI2_4LINE))

#define UART2_SHAREBIT      (L(ePIN_AS_UART2)|L(ePIN_AS_LCD)|L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_CAMERA))


#define I2S_SHAREBIT        (L(ePIN_AS_I2S)|L(ePIN_AS_LCD)|L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_CAMERA))

#define CAMERA_SHAREBIT     (L(ePIN_AS_CAMERA)|L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)| \
                            L(ePIN_AS_UART2)|L(ePIN_AS_I2S)|L(ePIN_AS_IRDA))

#define IRDA_SHAREBIT       (L(ePIN_AS_IRDA)|L(ePIN_AS_LCD)|L(ePIN_AS_CAMERA))  

#define GPIO_SHAREBIT       (0)


#else

#define NANDFLASH_SHAREBIT  (L(ePIN_AS_NANDFLASH)|L(ePIN_AS_MCI2)|L(ePIN_AS_MCI1)| \
                             L(ePIN_AS_MCI1_8LINE)|L(ePIN_AS_LCD)| \
                             L(ePIN_AS_SPI1)|L(ePIN_AS_SPI1_4LINE))

#define MCI1_SHAREBIT       (L(ePIN_AS_MCI1)|L(ePIN_AS_MCI1_8LINE)| \
                             L(ePIN_AS_NANDFLASH)|L(ePIN_AS_LCD))
#define MCI1_8LINE_SHAREBIT (L(ePIN_AS_MCI1_8LINE)|L(ePIN_AS_MCI1)| \
                             L(ePIN_AS_NANDFLASH)|L(ePIN_AS_LCD)|L(ePIN_AS_MCI2))

#define MCI2_SHAREBIT       (L(ePIN_AS_MCI2)|L(ePIN_AS_NANDFLASH)|L(ePIN_AS_LCD)| \
                             L(ePIN_AS_MCI1_8LINE)|L(ePIN_AS_UART1_HD))

#define LCD_SHAREBIT        (L(ePIN_AS_LCD)|L(ePIN_AS_SPI1)|L(ePIN_AS_SPI1_4LINE)| \
                             L(ePIN_AS_NANDFLASH)|L(ePIN_AS_MCI2)| \
                             L(ePIN_AS_MCI1)|L(ePIN_AS_MCI1_8LINE))

#define SPI1_SHAREBIT       (L(ePIN_AS_SPI1)|L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_LCD)| \
                             L(ePIN_AS_NANDFLASH))
#define SPI1_4LINE_SHAREBIT (L(ePIN_AS_SPI1_4LINE)|L(ePIN_AS_SPI1)|L(ePIN_AS_LCD)| \
                             L(ePIN_AS_NANDFLASH)|L(ePIN_AS_SPI2_4LINE))

#define SPI2_SHAREBIT       (L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_PCM))
#define SPI2_4LINE_SHAREBIT (L(ePIN_AS_SPI2_4LINE)|L(ePIN_AS_SPI2)|L(ePIN_AS_PCM)| \
                             L(ePIN_AS_SPI1_4LINE))

#define PCM_SHAREBIT        (L(ePIN_AS_PCM)|L(ePIN_AS_SPI2)|L(ePIN_AS_SPI2_4LINE))

#define PWM_SHAREBIT        (L(ePIN_AS_PWM))

#define UART1_SHAREBIT      (L(ePIN_AS_UART1)|L(ePIN_AS_UART1_HD))
#define UART1_HD_SHAREBIT   (L(ePIN_AS_UART1_HD)|L(ePIN_AS_UART1)|L(ePIN_AS_MCI2))

#define I2S_SHAREBIT        (L(ePIN_AS_I2S))

#define CAMERA_SHAREBIT     (L(ePIN_AS_CAMERA))

#define GPIO_SHAREBIT       (0)


#endif


#pragma arm section rwdata = "_drvbootcache_"
static T_eSHARE_PIN_CFG s_cfg_cache = ePIN_AS_GPIO;
#pragma arm section rwdata

#pragma arm section zidata = "_drvbootbss_"
static T_U32 s_share_bit;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_SHARE_PIN;

#if (CHIP_SEL_10C > 0)
static const T_U32 s_share_reg[] = {REG_SHARE_PIN_CTRL, REG_SHARE_PIN_CTRL2};
//base高两位bit[7:6]表示复用引脚相应控制的寄存器，最多可扩展为3位
static const T_SHARE_PIN_GPIO s_share_gpio[] = 
{
    //begin         end             base                num
    {5,             5,              1 |0x00,            2},
    {10,            10,             1 |0x00,            2},
    {43,            43,             3 |0x00,            2},
    {47,            50,             5 |0x00,            3},
    {14,            14,             8 |0x00,            2},
    {51,            52,             8 |0x00,            2},
    {53,            53,             10|0x00,            2},
    {54,            54,             12|0x00,            2},
    {0,             2,              17|0x00,            2},
    {7,             7,              17|0x00,            2},
    {6,             6,              19|0x00,            2},
    {12,            12,             21|0x00,            2},
    {13,            13,             23|0x00,            1},
    {15,            15,             24|0x00,            2},
    {16,            16,             26|0x00,            2},
    {10,            10,             30|0x00,            1},

    {30,            30,             0 |0x40,            3},
    {31,            31,             3 |0x40,            3},
    {32,            32,             6 |0x40,            3},
    {33,            33,             9 |0x40,            2},
    {34,            34,             11|0x40,            2},
    {35,            35,             13|0x40,            2},
    {36,            36,             15|0x40,            2},
    {37,            37,             17|0x40,            2},
    {38,            38,             19|0x40,            3},
    {39,            39,             22|0x40,            3},
    {40,            40,             25|0x40,            2},
    {41,            41,             27|0x40,            2}
};
#else
static const T_SHARE_PIN_GPIO s_share_gpio[] = 
{
    //begin         end             base            num
    {3,             4,              0,              1},
    {5,             5,              1,              2},
    {10,            10,             1,              2},
    {43,            46,             3,              2},
    {47,            50,             5,              3},
    {14,            14,             8,              2},
    {51,            52,             8,              2},
    {53,            53,             10,             2},
    {54,            54,             12,             2},
    {9,             9,              14,             1},
    {55,            55,             14,             1},
    {18,            18,             15,             2},
    {22,            22,             15,             2},
    {0,             2,              17,             2},
    {7,             7,              17,             2},
    {6,             6,              19,             1},
    {11,            11,             19,             1},
    {12,            12,             20,             1},
    {13,            13,             21,             1},
    {15,            15,             22,             2},
    {16,            16,             24,             2},
    {25,            29,             26,             2},
    //当用于SPI CAMERA时，这样设置还有5个GPIO可以用，分别是CIS_DATA[4:0]，对应GPIO30~34
#if ((DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0))
    {35,            41,             28,             1}
#else
    {30,            41,             28,             1}
#endif
};
#endif

#if (CHIP_SEL_10C > 0)

static const T_U32 SHARE_BIT[] = 
{
    NANDFLASH_SHAREBIT,
    MCI1_SHAREBIT,
    MCI2_1LINE_SHAREBIT,
    MCI2_4LINE_SHAREBIT,
    SPI1_SHAREBIT,
    SPI1_4LINE_SHAREBIT,
    SPI2_SHAREBIT,
    SPI2_4LINE_SHAREBIT,
    LCD_SHAREBIT,
    PWM_SHAREBIT,
    PWM2_SHAREBIT,
    UART1_SHAREBIT,
    UART1_HD_SHAREBIT,
    UART2_SHAREBIT,
    CAMERA_SHAREBIT,
    I2S_SHAREBIT,
    PCM_SHAREBIT,
    IRDA_SHAREBIT,
    GPIO_SHAREBIT
};

#else

static const T_U32 SHARE_BIT[] = 
{
    NANDFLASH_SHAREBIT,
    MCI1_SHAREBIT,
    MCI1_8LINE_SHAREBIT,
    SPI1_SHAREBIT,
    SPI1_4LINE_SHAREBIT,
    SPI2_SHAREBIT,
    SPI2_4LINE_SHAREBIT,
    LCD_SHAREBIT,
    PWM_SHAREBIT,
    UART1_SHAREBIT,
    UART1_HD_SHAREBIT,
    CAMERA_SHAREBIT,
    MCI2_SHAREBIT,
    I2S_SHAREBIT,
    PCM_SHAREBIT,
    GPIO_SHAREBIT
};

#endif

#pragma arm section rodata



#pragma arm section code ="_drvbootcode_"
/*******************************************************************************
 * @brief   set share pin group
 * @brief   for gpio share-pin 's different setting
 * @author  zhanggaoxin
 * @date    2012-12-12
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_VOID
*******************************************************************************/

#if (CHIP_SEL_10C > 0)

static T_VOID sys_share_pin_cfg(T_eSHARE_PIN_CFG pin_cfg)
{
    T_U32 reg1, reg2;
    T_U32 temp1, temp2;

    if (s_cfg_cache == pin_cfg)
    {
        return;
    }

    s_cfg_cache = pin_cfg;

    reg1 = REG32(REG_SHARE_PIN_CTRL);
    reg2 = REG32(REG_SHARE_PIN_CTRL2);

    temp1 = reg1;
    temp2 = reg2;
    
    switch (pin_cfg)
    {
        case ePIN_AS_MCI1:
            temp1 &= ~((3<<3) | (3<<1));
            temp1 |=  ((1<<3) | (2<<1) | (1<<30));
            break;

        case ePIN_AS_MCI2_1LINE:
            temp1 &= ~((3<<26) | (3<<24) | (7<<5));
            temp1 |=  ((2<<26) | (2<<24) | (5<<5));
            break;

        case ePIN_AS_MCI2_4LINE:
            temp1 &= ~((3<<26) | (3<<24) | (7<<5));
            temp1 |=  ((2<<26) | (2<<24) | (3<<5));
            break;

        case ePIN_AS_LCD:
            //temp &= ~((3<<12) | (3<<10) | (7<<5) | (3<<3));;
            //temp |=  ((1<<14) | (3<<12) | (2<<10) | (4<<5) | (3<<3));
            break;

        case ePIN_AS_SPI1:
            temp1 &= ~((3<<12) | (3<<8));
            temp1 |=  ((2<<12) | (2<<8));
            break;

        case ePIN_AS_SPI1_4LINE:
            temp1 &= ~((3<<12) | (3<<8));
            temp1 |=  ((2<<12) | (2<<8));
            temp2 &= ~((3<<17) | (3<<15));
            temp2 |= ((2<<17) | (2<<15));
            break;

        case ePIN_AS_SPI2:
            temp2 &= ~((7<<22) | (3<<9) | (7<<3) | (7<<0));
            temp2 |=  ((2<<22) | (2<<9) | (2<<3) | (2<<0));
            break;

        case ePIN_AS_SPI2_4LINE:
            temp2 &= ~((7<<22) | (7<<19) | (3<<9) | (7<<6) | (7<<3) | (7<<0));
            temp2 |=  ((2<<22) | (2<<19) | (2<<9) | (2<<6) | (2<<3) | (2<<0));
            break;

        case ePIN_AS_CAMERA:
            temp2 &= ~(0x1FFFFFFF<<0);
            temp2 |= ((1<<27) | (1<<25) | (1<<22) | (1<<19) | (1<<17) | 
                (1<<15) | (1<<13) | (1<<11) | (1<<9) | (1<<6) | (1<<3) | (1<<0));
            break;

        case ePIN_AS_I2S:
            //temp &= ~(1<<27);
            //temp |=  (1<<26);
            break;

        case ePIN_AS_PCM:
            temp1 |= (3<<17);
            break;
            
        case ePIN_AS_PWM:
            temp1 &= ~(3<<19);
            temp1 |= (0x02<<19);
            break;

        case ePIN_AS_PWM2:
            temp1 &= ~(3<<21);
            temp1 |= (3<<21);
            break;

        case ePIN_AS_UART1:
            temp1 &= ~((3<<21));
            temp1 |= ((1<<23) | (1<<21));
            break;

        case ePIN_AS_UART1_HD:
            temp1 &= ~((3<<26) | (3<<24) | (3<<21));
            temp1 |=  ((1<<26) | (1<<24) | (1<<23) | (1<<21));
            break;

        case ePIN_AS_UART2:
            temp2 &= ~((7<<3) | (7<<0));
            temp2 |= ((3<<3) | (3<<0));
            break;
            
        case ePIN_AS_IRDA:
            temp2 &= ~(3<<11);
            temp2 |= (2<<11);
            
        default:
            break;
    }

    if(reg1 != temp1)
    {
        REG32(REG_SHARE_PIN_CTRL) = temp1;
    }
    if(reg2 != temp2)
    {
        REG32(REG_SHARE_PIN_CTRL2) = temp2;
    }
}

#else

static T_VOID sys_share_pin_cfg(T_eSHARE_PIN_CFG pin_cfg)
{
    T_U32 temp;

    if (s_cfg_cache == pin_cfg)
    {
        return;
    }

    s_cfg_cache = pin_cfg;

    temp = REG32(REG_SHARE_PIN_CTRL);
    switch (pin_cfg)
    {
    case ePIN_AS_MCI1:
        temp &= ~((3<<3) | (3<<1));
        temp |=  ((2<<3) | (2<<1));
        break;

    case ePIN_AS_MCI1_8LINE:
        temp &= ~((7<<5) | (3<<3) | (3<<1));
        temp |=  ((2<<5) | (2<<3) | (2<<1));
        break;

    case ePIN_AS_MCI2:
        temp &= ~((3<<24) | (3<<22) | (7<<5));
        temp |=  ((2<<24) | (2<<22) | (3<<5));
        break;

    case ePIN_AS_NANDFLASH:
#if (DRV_SUPPORT_NAND > 2)
        temp &= ~(3<<1);
        temp |=  (1<<1);
#endif
        temp &= ~((3<<12) | (3<<10) | (3<<8) | (7<<5) | (3<<3));
        temp |=  ((1<<12) | (1<<10) | (1<<8) | (1<<5) | (1<<3) | (1<<0));
        break;

    case ePIN_AS_LCD:
        temp &= ~((3<<12) | (3<<10) | (7<<5) | (3<<3));;
        temp |=  ((1<<14) | (3<<12) | (2<<10) | (4<<5) | (3<<3));
        break;

    case ePIN_AS_SPI1:
        temp &= ~((3<<12) | (3<<8));
        temp |=  ((2<<12) | (2<<8));
        break;

    case ePIN_AS_SPI1_4LINE:
        temp &= ~((3<<15) | (3<<12) | (3<<8));
        temp |=  ((1<<15) | (2<<12) | (2<<8));
        break;

    case ePIN_AS_SPI2:
        temp &= ~(3<<17);
        temp |=  (2<<17);
        break;

    case ePIN_AS_SPI2_4LINE:
        temp &= ~((3<<17) | (3<<15));
        temp |=  ((2<<17) | (2<<15));
        break;

    case ePIN_AS_CAMERA:
        //当用于SPI CAMERA时，这样设置还有5个GPIO可以用，分别是CIS_DATA[4:0]，对应GPIO30~34
#if ((DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0))
        REG32(REG_IMG_DATA_MODE) |= 0x7;
#endif
        temp |= (1<<28);
        break;

    case ePIN_AS_I2S:
        temp &= ~(1<<27);
        temp |=  (1<<26);
        break;

    case ePIN_AS_PCM:
        temp |= (3<<17);
        break;

    case ePIN_AS_PWM:
        temp |= (1<<19);
        break;

    case ePIN_AS_UART1:
        temp |= ((1<<21) | (1<<20));
        break;

    case ePIN_AS_UART1_HD:
        temp &= ~((3<<24) | (3<<22));
        temp |=  ((1<<24) | (1<<22) | (1<<21) | (1<<20));
        break;

    default:
        break;
    }
    REG32(REG_SHARE_PIN_CTRL) = temp;
}

#endif

/*******************************************************************************
 * @brief   set share pin as gpio
 * @brief   for gpio share-pin 's different setting
 * @author  zhanggaoxin
 * @date    2012-12-12
 * @param   [in]pin gpio pin num
 * @return  T_VOID
*******************************************************************************/
T_VOID gpio_set_pin_as_gpio(T_U32 pin)
{
    T_U32 i;
    T_U32 base, num;
#if (CHIP_SEL_10C > 0)
    T_U32 reg;
#endif

    s_cfg_cache = ePIN_AS_GPIO;
    //loop to find the correct bits to clr in share pin control register
    for (i=0; i<sizeof(s_share_gpio)/sizeof(s_share_gpio[0]); i++)
    {
        if ((pin >= s_share_gpio[i].m_gpio_begin)
             && (pin <= s_share_gpio[i].m_gpio_end))
        {
            base = s_share_gpio[i].m_bits_base & 0x1f;
            num  = s_share_gpio[i].m_bits_num;
        #if (CHIP_SEL_10C > 0)
            reg  = s_share_reg[s_share_gpio[i].m_bits_base >> 6];
            REG32(reg) &= ~(((1<<num)-1) << base);
        #else
            
            REG32(REG_SHARE_PIN_CTRL) &= ~(((1<<num)-1) << base);
        #endif
        }
    }
}


static show_share_pin_error(T_eSHARE_PIN_CFG to_share, T_U32 share_value)
{
    drv_print(err_str, __LINE__, AK_TRUE);
    drv_print(AK_NULL, to_share, AK_TRUE);
    drv_print(AK_NULL, share_value, AK_TRUE);
    while(1);
}


/*******************************************************************************
 * @brief   set share pin group
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_VOID
 * @remark  pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_VOID sys_share_pin_lock(T_eSHARE_PIN_CFG pin_cfg)
{
    T_U16 tmp;

    tmp = s_share_bit & SHARE_BIT[pin_cfg];
    if (tmp)
    {
        show_share_pin_error(pin_cfg, tmp);
    }

    s_share_bit = s_share_bit | L(pin_cfg);

    sys_share_pin_cfg(pin_cfg);
}


/*******************************************************************************
 * @brief   release share pin group
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_VOID
 * @remark  pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_VOID sys_share_pin_unlock(T_eSHARE_PIN_CFG pin_cfg)
{
    s_share_bit = s_share_bit & (~(L(pin_cfg)));
}


/*******************************************************************************
 * @brief   check share pin group has been set
 * @brief   for gpio share-pin 's different setting
 * @author  wangguotian
 * @date    2012-12-11
 * @param   [in]pin_cfg different setting of share-pin
 * @return  T_BOOL
 * @retval  AK_TRUE: set, AK_FALSE, not set
 * @remark  this function is not recommended to use.
 *          pin_cfg must be a T_eSHARE_PIN_CFG, and and can't be > 15,
 *          and the function not check this value if in the valid range
*******************************************************************************/
T_BOOL sys_share_pin_is_lock(T_eSHARE_PIN_CFG pin_cfg)
{
    T_BOOL bLock;

    bLock = (s_share_bit & L(pin_cfg)) ? AK_TRUE : AK_FALSE;
    return bLock;
}
#pragma arm section code


/* end of file */

