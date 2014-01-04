/*******************************************************************************
 * @file    irda.c
 * @brief   irda driver.
 * This file provides UART APIs: UART initialization, write data to UART.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  ChenWeiwen
 * @date    2008-06-07
 * @version 1.0
 * @ref     AK1050C technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_irda.h"
#include "interrupt.h"
#include "share_pin.h"
#include "drv_cfg.h"
#include "boot.h"


#if (CHIP_SEL_10C > 0)

//0200
#define IRDA_ERR_CAST   26
#define IRDA_ATOM_PAT   16
#define IRDA_ADDR_EXT   22
#define IRDA_IN_POLAR   23
#define IRDA_REC_SYNC   21
#define IRDA_RX_ENABLE  31
#define IRDA_DATA_IN    0
//0208
#define IRDA_RX_INTR    4

#define MHZ             1000000
#define IR_CFG          (0x2d7)
#pragma arm section zidata = "_drvbootbss_"
//irDA cb rx DATA
static T_irDA_CALLBACK s_irDA_cb;
#pragma arm section zidata

/*******************************************************************************
 * @brief   IrDA_init
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]irDACB  
 * @return  T_VOID
*******************************************************************************/
T_VOID IrDA_init(T_BOOL signal_reverse,T_BOOL chkbyte_reverse)
{
    T_U32 regvalue;
    
    if(AK_FALSE == sys_share_pin_is_lock(ePIN_AS_IRDA))
    {
        sys_share_pin_lock(ePIN_AS_IRDA);
    }
    
    regvalue = REG32(REG_IRDA_CFG);
    regvalue &= ~((1<<IRDA_ERR_CAST));//输入不取反。
    if(chkbyte_reverse)
        regvalue &=~(1<<IRDA_ADDR_EXT);
    else
        regvalue |= (1<<IRDA_ADDR_EXT);
    if(signal_reverse)
        regvalue |= (1<<IRDA_IN_POLAR);
    else
        regvalue &= ~(1<<IRDA_IN_POLAR);
    
    regvalue |= (1<<IRDA_REC_SYNC);
    regvalue |= (19<<IRDA_ATOM_PAT);
    regvalue |= (1UL<<IRDA_RX_ENABLE);
    regvalue = (regvalue&(0xffff0000))|(IR_CFG);
    REG32(REG_IRDA_CFG) = regvalue;
    delay_ms(1);
    REG32(REG_IRDA_STATUS) |= 1<<IRDA_RX_INTR;
    
}

/*******************************************************************************
 * @brief   IrDA_interrupt_handle
 * @author  Liuhuadong
 * @date    2013.07.19
 * @param   [in]void 
 * @return  T_BOOL
*******************************************************************************/

#pragma arm section code = "_drvbootcode_",rodata="_drvbootcode_"
//const T_CHR  str_irda_warn[]="irda cnt:";
//static const T_CHR irda_ret[] = "ret";
//static const T_CHR irda_id[] = "id";

T_VOID IrDA_ScanSwitch(T_BOOL bEnable)
{
    if (bEnable)
    {
        INT_ENABLE_SCM(INT_EN_IRDA);
    }
    else
    {
        INT_DISABLE_SCM(INT_EN_IRDA);
    }
}

T_BOOL IrDA_GetKey(T_U8 *key)
{
    T_U32 regval;
    T_U8 i;
    T_U32 count;
    T_U32 data;
    T_U8 *ptr = (T_U8*)&data;
    regval = REG32(REG_IRDA_STATUS);
    count = (regval>>8)&0x7;
 
    //drv_print(str_irda_warn, count, 1);
    if(regval&(1<<IRDA_DATA_IN))
    {
        for(i=0;i<count;i++)
        {
            data = REG32(REG_IRDA_REC);       
        }
        *key = ptr[2];
        //drv_print(irda_id, ptr[2], 1);
        return AK_TRUE;
    }
    return AK_FALSE;
}


#pragma arm section


#endif


