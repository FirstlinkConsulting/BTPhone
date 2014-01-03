;/**
; * @file sys_deep_stdby.s
; * @brief deep standby asm function
; * Copyright (C) 2009 Anyka (GuangZhou) Technology Co., Ltd.
; * @author
; * @date 2009-07-03
; * @version 1.0
; * @note 
; */
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#include "anyka_bsp.h"
#include "boot.h"

#ifdef SYS_USE_DEEP_STANDBY

    IMPORT m_ulDeepSP
    IMPORT deepstdb_unmap
    IMPORT deepstdb_hang
    IMPORT MMU_InvalidateTLB
    IMPORT MMU_InvalidateIDCache
    IMPORT set_irq_stack
    
    AREA DEEPSTDB, CODE, READONLY


;Change processor mode to User Mode, and change svc stack 
;void deepstdb_set_SVCStack(T_U32 stack_addr)
    EXPORT deepstdb_set_SVCStack
deepstdb_set_SVCStack
    mov     sp, r0
    
    bx lr 
    
;save register named r0~r15,pc,sp, change svc stack, and clear cache, and...
;void deepstdb_enter(void)
    EXPORT deepstdb_enter
deepstdb_enter
    stmfd   sp!,{r0-r12,lr} ; push lr & register file
    mrs     r4,CPSR
    stmfd   sp!,{r4}        ; push current cpsr
    mrs     r4,SPSR
    stmfd   sp!,{r4}        ; push current spsr
    
    ldr     r5, =m_ulDeepSP ; save sp
    mov     r0, sp  
    str     r0, [r5]
    
    ldr     r0, =IRQ_MODE_STACK ;
    bl      deepstdb_set_SVCStack
    
    ldr     r0, =0
    bl      MMU_InvalidateTLB
    bl      MMU_InvalidateIDCache
    ;mcr     p15,0,r0,c7,c5,6    ;clear jump target cache
    ;mcr     p15,0,r0,c7,c7,0
    
    ;cancel mapping of the 128KB RAM, write them to nand auto
    bl      deepstdb_unmap

    ;store fixed datas of the 32KB RAM, write them to nand auto
    ;record nand position of the fixed datas and the flag of deep standby to nand
    bl      deepstdb_hang

    ;dead loop, or waiting one keyboard is pressed, just test!
deepstdb_enter_loop
    b       deepstdb_enter_loop


;point sp to the place before deep standby, and get value of spsr¡¢cpsr¡¢r0-r12¡¢pc from stack
;void deepstdb_resume(void)
    EXPORT deepstdb_resume
deepstdb_resume
    ldr     r0, =IRQ_MODE_STACK
    ldr     r1, =IRQ_MODE_STACK_SIZE
    bl      set_irq_stack

    ;Change processor mode to User Mode, and resume the stack pointer
    mov     r0, #ANYKA_CPU_Mode_SVC
    msr     CPSR_c, r0
    ldr     r6,=m_ulDeepSP
    ldr     sp,[r6]         ; get sp
    
    ldmfd   sp!,{r4}        ; pop spsr??
    msr     SPSR_cxsf,r4
    ldmfd   sp!,{r4}        ; pop cpsr
    msr     CPSR_cxsf,r4
    ldmfd   sp!,{r0-r12,pc} ; pop r0-r12, lr

#endif //endif SYS_USE_DEEP_STANDBY
    END


