/*******************************************************************************
 * @file interrupt.h
 * @brief: This file describe how to control the AK3223M interrupt issues.
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
*******************************************************************************/
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


#include "anyka_cpu.h"
#include "arch_interrupt.h"



//Interrupt Mask Register of IRQ
//All the bit is define in anyka_bits.h
#define INT_ENABLE(int_bits)        REG32(REG_INT_IRQ) |= (int_bits)
#define INT_DISABLE(int_bits)       REG32(REG_INT_IRQ) &= ~(int_bits)


//IRQ of system control module->REG_INT_SYSCTL
//All the bit is define in anyka_bits.h
#define INT_ENABLE_SCM(int_bits)    REG32(REG_INT_SYSCTL) |= (int_bits)
#define INT_DISABLE_SCM(int_bits)   REG32(REG_INT_SYSCTL) &= ~(int_bits)


//IRQ of system control module->REG_INT_ANALOG
//All the bit is define in anyka_bits.h
#define INT_ENABLE_ALG(int_bits)    REG32(REG_INT_ANALOG) |= (int_bits)
#define INT_DISABLE_ALG(int_bits)   REG32(REG_INT_ANALOG) &= ~(int_bits)


//Interrupt Mask Register of FIQ
//All the bit is define in anyka_bits.h
#define FIQ_INT_ENABLE(int_bits)    REG32(REG_INT_FIQ) |= (int_bits)
#define FIQ_INT_DISABLE(int_bits)   REG32(REG_INT_FIQ) &= ~(int_bits)



/*******************************************************************************
 * @brief   mask the irq.
 * @author 
 * @date    2006-12-1
 * @return  T_VOID
*******************************************************************************/
T_VOID irq_mask(T_VOID);

/*******************************************************************************
 * @brief   unmask the irq.
 * @author 
 * @date    2006-12-1
 * @return  T_VOID
 ******************************************************************************/
T_VOID irq_unmask(T_VOID);


#endif  //__INTERRUPT_H__

