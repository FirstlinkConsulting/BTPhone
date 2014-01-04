/**
 * @file interrupt.c
 * @brief interrupt function file
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */

#include "anyka_cpu.h"
#include "anyka_types.h"
#include "interrupt.h"
#include "arch_init.h"

#define MAX_LAYER   8
#define INT_ERROR   "Oiq"

#pragma arm section zidata = "_drvbootbss_"
static volatile T_U32 maskreg_stack[MAX_LAYER];
static volatile T_U8 top;
#pragma arm section zidata

T_VOID store_int(T_U32 mask)
{
    T_U32 value;
    
    if (top>=MAX_LAYER)
    {
        drv_print(INT_ERROR, MAX_LAYER, 1);
        while(1);
    }

    value = REG32(REG_INT_IRQ);
    mask &= value;
    REG32(REG_INT_IRQ) = mask;
    maskreg_stack[top] = value;
    top++;
}

T_VOID restore_int(T_VOID)
{
    if (top==0)
    {
        drv_print(INT_ERROR, 0, 1);
        while(1);
    }
    
    top--;
    REG32(REG_INT_IRQ) = maskreg_stack[top];
}


T_VOID store_all_int(T_VOID)
{
    store_int(0);
}

T_VOID restore_all_int(T_VOID)
{
    restore_int();
}

