/**
 * @file    arch_interrupt.h
 * @brief   the interface for the control of interrupt
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */

#ifndef _ARCH_INTERRUPT_H_
#define _ARCH_INTERRUPT_H_

#include "anyka_types.h"

/*
 * @brief: Store the current interrupt mask register value, then mask all interrupt.
 * @author 
 * @date 2006-01-16
 * @return VOID
 */
T_VOID store_all_int(T_VOID);

/*
 * @brief: Recover the previous interrupt mask register value.
 * @author 
 * @date 2006-01-16
 * @return VOID
 */
T_VOID restore_all_int(T_VOID);

T_VOID enable_fiq_int(T_U32 int_bits);

T_VOID disable_fiq_int(T_U32 int_bits);


/*
 * @brief: make sure the stack has 128 bytes free in the physical memory
 * @author 
 * @date 2006-01-16
 * @return VOID
 */
T_VOID check_stack(T_VOID);

/*
 * @brief: make sure the page within p in the physical memory
 * @author 
 * @date 2006-01-16
 * @return VOID
 */
T_VOID check_ptr(T_U8 *p);

#endif //_ARCH_INTERRUPT_H_


