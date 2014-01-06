/**
 * @file    int.s
 * @brief   the entry of the interrupt and the exception
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.20
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "drv_cfg.h"
#include "boot.h"


#ifndef UNSUPPORT_REMAP
    IMPORT  BackGroundTask
    IMPORT  TaskPending
    IMPORT  TaskContext
    IMPORT  TaskMutex

    EXPORT  Do_IRQ_Return
#endif

    IMPORT  exception_handler
    IMPORT  check_abortstack
    IMPORT  check_irqstack

    IMPORT  MMU_InvalidateIDCache
    IMPORT  MMU_DisableICache
    IMPORT  MMU_DisableDCache
    IMPORT  MMU_DisableMMU
    IMPORT  MMU_InvalidateTLB

    EXPORT  prefetch_handler
    EXPORT  abort_handler
    EXPORT  irq_handler
    EXPORT  fiq_handler

    EXPORT  cpu_standby
    EXPORT  get_user_lr

    AREA _drvbootbss_, READWRITE
    EXPORT  g_clk_map
    EXPORT  irq_mask_l2     ; variable prototype (T_U32 *)
g_clk_map   DCD   0
irq_mask_l2 DCD   0

    AREA BOOT, CODE, READONLY
    CODE32

#ifndef UNSUPPORT_REMAP
Do_IRQ_Return
    mov     sp,r0
    mrs     r0,cpsr
    orr     r0,r0,#(ANYKA_CPU_I_Bit|ANYKA_CPU_F_Bit);disable FIQ and IRQ
    msr     cpsr_c,r0;
    ldr     r0,=TaskMutex
    mov     r1,#0
    strb    r1,[r0,#0]
    ldr     r7,=TaskContext
    ldr     r0,[r7],#4
    msr     spsr_cxsf,r0
    ldr     lr,[r7],#4
    add     r7,r7,#28
    ldmia   r7!, {r0-r6}
    stmfd   sp!, {r0-r6}
    sub     r7,r7,#56
    ldmia   r7!, {r0-r6}
    stmfd   sp!, {r0-r6}
    ldmfd   sp!, {r0-r12, pc}^
#endif
;;;;;;;;;;;;;;;;;;;;;   Entry of FIQ   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
fiq_handler
    sub     lr,lr,#4
    stmfd   sp!, {r0-r7, lr}               ;backup the context

    ;load the status register of interrupt
    ldr     r0, =REG_INT_STA
    ldr     r9, [r0]
    ldr     r0, =REG_INT_FIQ
    ldr     r8, [r0]

    ;IMPORT  g_pc_lr
    ;ldr     r1, =g_pc_lr
    ;mov     r0, lr
    ;str     r0, [r1]
    
    ;MVN     r8, r8
    AND     r9, r9, r8

fiq_handler_check_status                    ;loop to check all bits of the status register
    #ifdef    __ENABLE_L2_INT__
    IMPORT  l2_interrupt_handle
    tst     r9, #INT_STA_L2
    blne    l2_interrupt_handle
    #endif 

    #ifdef    __ENABLE_SPI2_INT__
    IMPORT  spictrl2_interrupt_handler
    tst     r9, #INT_STA_SPI2
    blne    spictrl2_interrupt_handler
    #endif
    
    #ifdef    __ENABLE_UART_INT__
    IMPORT  uart1_interrupt_handler
    tst     r9, #INT_STA_UART1
    blne    uart1_interrupt_handler
    #endif

#if (CHIP_SEL_10C > 0)
    #ifdef    __ENABLE_UART_INT__
    IMPORT  uart2_interrupt_handler
    tst     r9, #INT_STA_UART2
    blne    uart2_interrupt_handler
    #endif
#endif    

    #ifdef    __ENABLE_CAMERA_INT__
    IMPORT  camctrl_interrupt_handler
    tst     r9, #INT_STA_CAMERA
    blne    camctrl_interrupt_handler
    #endif

fiq_handler_exit                        ;Exit point of FIQ
    ldmfd   sp!,{r0-r7,pc}^

;;;;;;;;;;;;;;;;;;;;;   Entry of IRQ   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
irq_handler
    sub     lr,lr,#4
    stmfd   sp!, {r0-r12, lr}               ;backup the context

    ;load the status register of interrupt
    ldr     r0, =REG_INT_STA
    ldr     r9, [r0]
    ldr     r0, =REG_INT_IRQ
    ldr     r8, [r0]

    IMPORT  g_pc_lr
    ldr     r1, =g_pc_lr
    mov     r0, lr
    str     r0, [r1]
    
    ;MVN     r8, r8
    AND     r9, r9, r8

irq_handler_check_status                    ;loop to check all bits of the status register
    #ifdef    __ENABLE_SYSCTRL_INT__
    IMPORT  system_control_interrupt_handler
    tst     r9, #INT_STA_SYSCTL
    blne    system_control_interrupt_handler
    #endif

    #ifdef    __ENABLE_L2_INT__
    IMPORT  l2_interrupt_handle
    tst     r9, #INT_STA_L2
    blne    l2_interrupt_handle
    #endif 

    #ifdef    __ENABLE_USBMCU_INT__
    IMPORT  usb_intr_handler
    tst     r9, #INT_STA_USBMCU
    blne    usb_intr_handler
    #endif
    
    #ifdef    __ENABLE_USBDMA_INT__
    IMPORT  usb_intr_handler
    tst     r9, #INT_STA_USBDMA
    blne    usb_intr_handler
    #endif
    
    #ifdef    __ENABLE_UART_INT__
    IMPORT  uart1_interrupt_handler
    tst     r9, #INT_STA_UART1
    blne    uart1_interrupt_handler
    #endif

#if (CHIP_SEL_10C > 0)
    #ifdef    __ENABLE_UART_INT__
    IMPORT  uart2_interrupt_handler
    tst     r9, #INT_STA_UART2
    blne    uart2_interrupt_handler
    #endif
#endif

    #ifdef    __ENABLE_CAMERA_INT__
    IMPORT  camctrl_interrupt_handler
    tst     r9, #INT_STA_CAMERA
    blne    camctrl_interrupt_handler
    #endif 
    
    #ifdef    __ENABLE_SPI1_INT__
    IMPORT  spictrl1_interrupt_handler
    tst     r9, #INT_STA_SPI1
    blne    spictrl1_interrupt_handler
    #endif
    
    #ifdef    __ENABLE_SPI2_INT__
    IMPORT  spictrl2_interrupt_handler
    tst     r9, #INT_STA_SPI2
    blne    spictrl2_interrupt_handler
    #endif

#ifndef UNSUPPORT_REMAP
    mrs     r0, spsr
    and     r0, r0, #0x1f
    cmp     r0, #ANYKA_CPU_Mode_ABT
    beq     irq_handler_exit

    ;check if still under sd or nand operation
    ldr     r0,=g_clk_map
    ldr     r1, [r0,#0]
    cmp     r1, #0
    bne     irq_handler_exit

    ;mov     r0,sp
    bl      TaskPending
    cmp     r0,#0
    beq     irq_handler_exit
    ldr     r0,=TaskMutex
    mov     r1,#1
    strb    r1,[r0,#0]
    ldr     r7,=TaskContext
    mrs     r0,spsr
    str     r0,[r7],#4
    bic     r0,r0,#(0x1f|ANYKA_CPU_I_Bit|ANYKA_CPU_F_Bit)
    bic     r0,r0,#(0xf<<28) ;clear condition flag
    orr     r0,r0,#(ANYKA_CPU_T_Bit|ANYKA_CPU_Mode_SVC)
    msr     spsr_cxsf,r0
    mov     r2, #(ANYKA_CPU_Mode_SVC|ANYKA_CPU_I_Bit|ANYKA_CPU_F_Bit);disable IRQ and FIQ,enter svc mode
    msr     CPSR_c, r2;change mode to svc
    mov     r0,lr
    mov     r9,sp
    mov     r2, #(ANYKA_CPU_Mode_IRQ|ANYKA_CPU_I_Bit|ANYKA_CPU_F_Bit);disable IRQ and FIQ,enter fiq mode
    msr     CPSR_c, r2;change mode to irq
    
    str     r0,[r7],#4
    mov     r8,sp
    ldmia   r8!, {r0-r6}
    stmia   r7!, {r0-r6}    ;save r0-r6
    ldmia   r8!, {r0-r6}
    stmia   r7!, {r0-r6}    ;save r7-r12,lr
    ldr     r0, =BackGroundTask
    str     r0,[r8,#-4]
    str     r9,[sp,#0];//param 1
;    str     r8,[sp,#4];//param 2
    bl      check_irqstack 

#endif  //UNSUPPORT_REMAP

irq_handler_exit                        ;Exit point of IRQ
    ldmfd   sp!,{r0-r12,pc}^
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of Prefech handler  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
prefetch_handler
    stmdb   sp!, {r0-r12, lr}           ;backup the context 

    mrs     r0, spsr
    mrs     r1, cpsr
    orr     r0, r0, #0xc0               ;disable I,F bit
    bic     r0, r0, #0x20               ;clear T bit
    msr     cpsr_cxsf, r0

    MOV     r3, lr
    mov     r2, sp
    msr     cpsr_cxsf, r1

#if ENABLE_IRQ_IN_ABORT > 0
    mrs     r0, spsr
    and     r0, r0, #0x1f
    cmp     r0, #ANYKA_CPU_Mode_IRQ
    beq     prefetch_do_exception

    ;mask all irq except l2
    ldr     r4,=REG_INT_IRQ
    ldr     r6, [r4, #0]
    ldr     r5,=irq_mask_l2
    str     r6, [r5]
    mov     r6, #0x1
    str     r6, [r4]

    ;enable irq
    bic     r1, r1, #0x80
    msr     cpsr_cxsf, r1
#endif

prefetch_do_exception
    subs    r0, lr, #4
    mov     r1, #PREFECTH_ERR
    bl      exception_handler

    bl      check_abortstack            ;check the stackflag of abort mode

#if ENABLE_IRQ_IN_ABORT > 0
    mrs     r0, spsr
    and     r0, r0, #0x1f
    cmp     r0, #ANYKA_CPU_Mode_IRQ
    beq     prefetch_exit

    ;disable irq
    mrs     r1, cpsr
    orr     r1, r1, #0x80
    msr     cpsr_cxsf, r1

    ;restore all irq mask
    ldr     r5,=irq_mask_l2
    ldr     r6, [r5, #0]
    ldr     r4,=REG_INT_IRQ
    str     r6, [r4]
#endif

prefetch_exit
    ldmia   sp!, {r0-r12, lr}       ;restore the context
    subs    pc, lr, #4              ;return to the interrupt point

;;;;;;;;;;;;;;;;;;;;;   Entry of Data abort handler  ;;;;;;;;;;;;;;;;;;;;;;;;;
abort_handler
    stmdb   sp!, {r0-r12, lr}       ;backup the context

    mrs     r0, spsr
    mrs     r1, cpsr
    orr     r0, r0, #0xc0           ;disable I,F bit
    bic     r0, r0, #0x20           ;clear T bit
    msr     cpsr_cxsf, r0

    MOV     r3, lr 
    mov     r2, sp
    msr     cpsr_cxsf, r1

#if ENABLE_IRQ_IN_ABORT > 0
    mrs     r0, spsr
    and     r0, r0, #0x1f
    cmp     r0, #ANYKA_CPU_Mode_IRQ
    beq     abort_do_exception

    ;mask all irq except l2
    ldr     r4,=REG_INT_IRQ
    ldr     r6, [r4, #0]
    ldr     r5,=irq_mask_l2
    str     r6, [r5]
    mov     r6, #0x1
    str     r6, [r4]

    ;enable irq
    bic     r1, r1, #0x80
    msr     cpsr_cxsf, r1 
#endif

abort_do_exception
    subs    r0, lr, #8
    mov     r1, #DATA_ABORT_ERR
    bl      exception_handler

    bl      check_abortstack        ;check the stackflag of abort mode

#if ENABLE_IRQ_IN_ABORT > 0
    mrs     r0, spsr
    and     r0, r0, #0x1f
    cmp     r0, #ANYKA_CPU_Mode_IRQ
    beq     abort_exit

    ;disable irq
    mrs     r1, cpsr
    orr     r1, r1, #0x80
    msr     cpsr_cxsf, r1 

    ;restore all irq mask
    ldr     r5,=irq_mask_l2
    ldr     r6, [r5, #0]
    ldr     r4,=REG_INT_IRQ
    str     r6, [r4]
#endif
    
abort_exit
    ldmia   sp!, {r0-r12, lr}       ;restore the context
    subs    pc, lr, #8              ;return to the interrupt point

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT irq_mask
irq_mask
    stmdb   sp!, {r0}
    mrs     r0, cpsr
    orr     r0, r0, #ANYKA_CPU_I_Bit
    msr     cpsr_cxsf, r0
    ldmia   sp!, {r0}
    bx      lr

    EXPORT irq_unmask
irq_unmask
    stmdb   sp!, {r0, r1}
    mrs     r0, cpsr
    and     r1, r0, #0x1f
    cmp     r1, #ANYKA_CPU_Mode_IRQ
    bicne   r0, r0, #ANYKA_CPU_I_Bit
    msr     cpsr_cxsf, r0
    ldmia   sp!, {r0, r1}
    bx      lr

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


    EXPORT fake
;T_U32 fake(T_U32 *seed, T_U32 range)
fake
    ldr     r2, [r0, #0]
    ldr     r3, value
    mla     r3,r2,r3,r3     ;r3 = r2*r3+r3
    str     r3, [r0, #0]
    mov     r2, r1          ;the range of the number
    umull   r2, r0, r3, r2
    bx      lr
value DCD 0x91e6d6a5

    EXPORT Soft_Reset
;T_VOID Soft_Reset(T_VOID)
Soft_Reset
;disable mmu
    blx     MMU_InvalidateIDCache
    blx     MMU_DisableICache
    blx     MMU_DisableDCache
    blx     MMU_DisableMMU
    blx     MMU_InvalidateTLB
    
;set (BOOT0)gpio6 , (BOOT1)gpio12 input
    mov     r0,#0x0
    ldr     r1,=REG_SHARE_PIN_CTRL
    str     r0,[r1]
    mov     r0,#0x1040
    ldr     r1,=REG_GPIO_DIR_1
    str     r0,[r1]
    
;disable all buffer
    mov     r0,#0
    ldr     r1,=REG_BUFFER_ENABLE
    str     r0,[r1]
    
    ldr     pc, =0x00000000
    
    EXPORT UpdateRom_Reset
;T_VOID UpdateRom_Reset(T_VOID)
UpdateRom_Reset
;disable mmu
    blx     MMU_InvalidateIDCache
    blx     MMU_DisableICache
    blx     MMU_DisableDCache
    blx     MMU_DisableMMU
    blx     MMU_InvalidateTLB

;enable the clock:[2]: NAND [3]: UART [4]: USB [7]: System [8]: SPI1 [13]: VBUF1 [14]: VBUF2 [15]: SPI2
    ;ldr     r1,=0x00001E63
    ldr     r1,=0x0000fe63    ;enable the clock: [2]: NAND  [3]: UART  [4]: USB  [7]: System  [10]: Ram
    ldr     r0,=REG_CLOCK_RST_EN
    str     r1,[r0]

;change to Supervisor Mode & inital stack for all program
    ldr     r0,=0x13
    msr     cpsr_cxsf, r0
    ldr     r1,=0x00826f00
    mov     sp, r1 

;init l2
    ldr     r0,=REG_L2_CONFIG
    ldr     r1,=(0x27<<20)
    orr     r1, r1, #0x20
    str     r1,[r0]
    
;disable all buffer
    mov     r1,#0
    ldr     r2,=REG_BUFFER_ENABLE
    str     r1,[r2]

;set boot mode as USB mass storage boot
    ldr     pc, =UMASS_BOOT_ADDR

;T_VOID cpu_standby(T_VOID);
cpu_standby
    str     r0,[sp,#-4]
    mov     r0,#0
    mcr     p15, 0, r0, c7, c0, 4           ;@ Wait for interrupt
    ldr     r0,[sp,#-4]
    bx lr
    
;T_VOID get_user_lr(T_VOID);
get_user_lr
    stmfd   r13!, {r1-r2, r14}
    mrs     r0, CPSR
    mov     r1, r0
    bic     r0, r0, #0x1f
    orr     r0, r0, #(ANYKA_CPU_Mode_SVC)
    msr     CPSR_c, r0
    MOV     r0, lr
    msr     CPSR_c, r1
    ldmfd   r13!, {r1-r2, pc}


    EXPORT check_ptr
;T_VOID check_ptr(T_U8 *p)
check_ptr
    LDRB    r1,[r0,#0]
    bx      r14

    EXPORT check_stack
;T_VOID check_stack(T_VOID)
check_stack
    stmfd   r13!, {r0-r1, r14}
    mov     r0, sp
    sub     r0, r0, #256
    LDRB    r1,[r0,#0]
    ldmfd   r13!, {r0-r1, pc}

    END

