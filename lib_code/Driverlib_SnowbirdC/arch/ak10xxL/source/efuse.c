/*******************************************************************************
 * @file    efuse.c
 * @brief   vendor id and serial id driver file, provides vendor id and 
 *          series id  APIs: read, burn
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  LiuHuadong
 * @date    2012-11-22
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_efuse.h"
#include "arch_timer.h"
#include "drv_cfg.h"


#if (DRV_SUPPORT_EFUSE > 0)
#pragma arm section code = "_drvbootcode_"
#else   //efuse仅作Vref校准使用
#pragma arm section code = "_drvbootinit_"
#endif
/*******************************************************************************
 * @brief   read vendor id or serial id
 * @author  LiuHuadong
 * @date    2012-03-04
 * @param   [in]id: EFUSE_READ_VENDOR:vendor id or EFUSE_READ_SERIAL:serial id
 * @return  T_U32
 * @retval  the value of vendor id or serial id
*******************************************************************************/ 
T_U32 efuse_read(T_U8 id)
{
    T_U32 efuse_value;
    T_U32 reg_value;


    REG32(REG_ANALOG_CTRL3) &= ~CONNECT_AVCC;   //avcc3.3
    REG32(REG_ANALOG_CTRL4) |= DISCHG_VP;       //vp =0
    delay_us(30000);

    reg_value = REG32(REG_EFUSE_CTRL);
    reg_value &= ~EFUSE_RDY_CFG;                //rdy_cfg
    reg_value |= (0x7f<<EFUSE_CELL_EN);
    reg_value &= ~EFUSE_MODE_SEL;               //read =0
    reg_value &= ~(0x7f<<EFUSE_CELL_RD_CS);     //csmux 1:ram 0:efuse
    REG32(REG_EFUSE_CTRL) = reg_value;

    REG32(REG_EFUSE_CTRL) |= EFUSE_RDY_CFG;     //rdy_cfg
    while (!(REG32(REG_EFUSE_READ_DATA2) & EFUSE_READ_ACK));

    if (EFUSE_READ_VENDOR == id)
    {
        efuse_value  = (REG32(REG_EFUSE_READ_DATA1)&0xffff) >> 2;
    }
    else
    {
        efuse_value  = (REG32(REG_EFUSE_READ_DATA2)&0xfffful) << 16;
        efuse_value |= (REG32(REG_EFUSE_READ_DATA1) >> 16);
    }

    REG32(REG_EFUSE_READ_DATA2) |= EFUSE_READ_ACK;  //write 1 to clear
    REG32(REG_EFUSE_CTRL) &= ~EFUSE_RDY_CFG;;   //rdy_cfg clear

    REG32(REG_EFUSE_CTRL) |= EFUSE_MODE_SEL;    //read =0

    return efuse_value;
}
#pragma arm section code


