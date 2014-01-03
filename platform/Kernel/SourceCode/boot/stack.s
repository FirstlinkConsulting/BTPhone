;/**
; * @file stack.s
; * @brief stack function
; * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
; * @author Liangxiong
; * @date 2012-07-7
; * @version 1.0
; * @note 
; */
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#include "anyka_bsp.h"
#include "boot.h"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    AREA _bootbss1_, READWRITE
    EXPORT	IrqStackTopLimit     ; variable prototype (T_U32 *)
    EXPORT	AbortStackTopLimit   ; variable prototype (T_U32 *)
IrqStackTopLimit	DCD   0
AbortStackTopLimit	DCD   0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    AREA _bootcode1_, CODE, READONLY
    CODE32
    IMPORT	reset_handler_loop
    IMPORT	akerror

    EXPORT set_each_mode_defstack
;T_VOID set_each_mode_defstack(T_VOID)
set_each_mode_defstack
    ;Change processor mode to FIQ Mode, and initialize the stack pointer
    mov     r0, #ANYKA_CPU_Mode_FIQ
    msr     CPSR_c, r0
    ldr     r1, =FIQ_MODE_STACK
    mov     sp, r1

    ;Change processor mode to IRQ Mode, and initialize the stack pointer
    mov     r0, #ANYKA_CPU_Mode_IRQ
    msr     CPSR_c, r0
    ldr     r1, =IRQ_MODE_STACK
    mov     sp, r1

    ;Change processor mode to ABORT Mode, and initialize the stack pointer
    mov     r0, #ANYKA_CPU_Mode_ABT
    msr     CPSR_c, r0
    ldr     r1, =ABORT_MODE_STACK
    mov     sp, r1
    
    ;Change processor mode to User Mode, and initialize the stack pointer
    mov     r0, #ANYKA_CPU_Mode_SVC
    msr     CPSR_c, r0
    ldr     r1, =SVC_MODE_STACK
    mov     sp, r1

    ldr     r1, =STACK_CHECKFLAGE           ; get the stackflag
    ldr     r2, [r1] 
    ldr     r1, =IRQ_MODE_STACK
    ldr     r0, =IRQ_MODE_STACK_SIZE
    sub     r0, r1, r0
    add     r0, r0, #4                      
    ldr     r1, =IrqStackTopLimit           
    str     r0, [r1]                        ; saved the ptr of IrqStack TopLimit
    str     r2, [r0]                        ; stored the stackflag in space via ptr.
    
    ldr     r1, =ABORT_MODE_STACK
    ldr     r0, =ABORT_MODE_STACK_SIZE
    sub     r0, r1, r0
    add     r0, r0, #4
    ldr     r1, =AbortStackTopLimit         
    str     r0, [r1]                       ; saved the ptr of AbortStack TopLimit
    str     r2, [r0]                       ; stored the stackflag in space via ptr.
    bx      r14

	EXPORT check_irqstack
;T_VOID check_irqstack(T_U32 flag)
check_irqstack
    ldr 	r1, =IrqStackTopLimit
    b		check_stackflag
	
	EXPORT check_abortstack
;T_VOID check_abortstack(T_U32 flag)
check_abortstack
    ldr 	r1, =AbortStackTopLimit 	   
check_stackflag
    ldr	        r3, [r1]						; ptr of StackTopLimit
    ldr	        r1, [r3]						; content from ptr of StackTopLimit
    ldr	        r3, =STACK_CHECKFLAGE			; check the stackflag
    ldr	        r2, [r3]
    cmp 	r1, r2
    bxeq 	r14
    mov 	r1, r0							; discover the error of stackflag 
    ldr 	r0, =ABORT_STRING
    mov 	r2,#1
    bl		akerror
    b		reset_handler_loop

    EXPORT get_sp
;T_U32 get_sp(T_VOID)
get_sp
    mov     r0, sp
    bx	    r14
		
	EXPORT get_pc
;T_U32 get_sp(T_VOID)
get_pc
    mov 	r0, pc
    bx		r14

    EXPORT set_stack
;T_VOID set_stack(T_U32)
set_stack
    mov     sp, r0  
    bx	    lr 
	
    EXPORT	STACK_CHECKFLAGE
STACK_CHECKFLAGE    DCD 0x76543210
ABORT_STRING        DCB "ABTOVER", 0x0
IRQ_STRING          DCB "IRQOVER", 0x0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    AREA _bootcode1_, CODE, READONLY
    CODE32
	
    EXPORT set_irq_stack
;T_VOID set_irq_stack(T_U32 stack_addr, T_U32 stackLen)
set_irq_stack
    stmfd	sp!, {r2-r3, r14}
    mrs 	r2, CPSR
    mov 	r3, #ANYKA_CPU_Mode_IRQ
    orr 	r3, r3, #0xc0                     ; disable I,F bit
    msr 	CPSR_c, r3                        ; change the mode
    mov 	sp, r0                            ; change the stackTop
    sub 	r0, r0, r1
    add 	r0, r0, #4
    ldr 	r1, =STACK_CHECKFLAGE             ; get the stackflag
    ldr 	r3, [r1]
    str 	r3, [r0]			  
    ldr 	r1, =IrqStackTopLimit             ; saved the IrqStackTopLimit
    str 	r0, [r1]
    msr 	CPSR_c, r2
    ldmfd	sp!, {r2-r3, r14}
    bx		r14 

#ifdef SYS_USE_DEEP_STANDBY
		
	AREA _DEEPSETABT_SETSTACK_, CODE, READONLY
	CODE32
		
;set abort stack
;void deepstdb_set_ABTStack(void)
	EXPORT deepstdb_set_ABTStack
deepstdb_set_ABTStack
    ;Change processor mode to ABORT Mode, and initialize the stack pointer
    mov 	r0, #ANYKA_CPU_Mode_ABT
    msr 	CPSR_c, r0
    ldr 	r1, =ABORT_MODE_STACK
    mov 	sp, r1
	
    ldr     r1, =STACK_CHECKFLAGE           ; get the stackflag
    ldr     r2, [r1] 
    
    ldr     r1, =ABORT_MODE_STACK
    ldr     r0, =ABORT_MODE_STACK_SIZE
    sub     r0, r1, r0
    add     r0, r0, #4
    ldr     r1, =AbortStackTopLimit         
    str     r0, [r1]                       ; saved the ptr of AbortStack TopLimit
    str     r2, [r0]  

    ;Change processor mode to User Mode, and go back
    mov 	r0, #ANYKA_CPU_Mode_SVC
    msr 	CPSR_c, r0

    bx    lr
		
#endif //#ifdef SYS_USE_DEEP_STANDBY

    END



