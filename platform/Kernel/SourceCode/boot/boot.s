;/**
; * @file boot.s
; * @brief boot code
; * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
; * @author Miaobaoli
; * @date 2005-07-13
; * @version 1.0
; * @note ref Janus II.pdf, AK36XXM technical manual.
; */

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#include "anyka_bsp.h"
#include "boot.h"

    IMPORT  AK_Main
    IMPORT  exception_handler
    IMPORT  progmanage_mmu_init
    IMPORT  progmanage_init
    IMPORT  akerror
    IMPORT |Image$$rodata$$Base|
    IMPORT |Image$$rodata$$Limit|
    IMPORT |Image$$ER_ZI$$ZI$$Base|   ;; MUST bss$$ZI, not bss
    IMPORT |Image$$ER_ZI$$ZI$$Limit|  ;; MUST bss$$ZI, not bss
    IMPORT |Image$$data$$Limit|

    IMPORT  set_each_mode_defstack
    IMPORT  check_abortstack
    IMPORT  check_irqstack
    IMPORT  BackGroundTask
    IMPORT  TaskPending
    IMPORT  TaskContext
    IMPORT  TaskMutex
    IMPORT  prefetch_handler
    IMPORT  abort_handler
    IMPORT  irq_handler
    IMPORT  fiq_handler
    IMPORT  MMU_Enable
    IMPORT  progmanage_resume
    IMPORT  Fwl_CheckPowerStatus
    
    AREA BOOT, CODE, READONLY
    CODE32
    ENTRY

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Exception vectors   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    EXPORT START
START
    b   reset_handler       ;Start point: Jump to first instruction: reset_handler
    b   undefined_handler   ;Exception(Undefined Instruction): Dead Loop
swi_handler
    mov pc, lr				;Software Interrupt: Jump to entry of SWI	
	b	prefetch_handler	;Exception(Instruction Prefetch Failure): Dead Loop
	b	abort_handler		;Exception(Data Abort): Dead Loop
    nop                     ;Reserved
    b   irq_handler         ;Interrupt Request: Jump to entry of IRQ
    b   fiq_handler           ;Fast Interrupt Request: Reserved

    EXPORT bootdata
bootdata

password        DCB "SUPERNNC"
page_size       DCB 4
page_num        DCB 25
cmd_cycle       DCB 2
col_addr        DCB 2
row_addr        DCB 3
cmd_unlock      DCB 0x00
cmd_confirm     DCB 0x30
reserved        DCB 0x0
cmd_timing      DCD 0x62451
data_timing     DCD 0x61313
rst_wait_time   DCW 0x0005
rd_wait_time    DCW 0x005

#if (STORAGE_USED == 2)
chip_id         DCD 0x9551d3ec
bytes_per_page  DCW 2048
page_per_block  DCW 64
chip_blk_num    DCW 8192
group_blk_num   DCW 4096
plane_blk_num   DCW 2048
spare_size      DCB 64
col_cycle       DCB 2
col_mask        DCB 0xf
row_cycle       DCB 3
row_mask        DCB 11
cust_param      DCB 0x1
nand_flag       DCD 0xb0000001
cmd_len         DCD 0xf5ad1
data_len        DCD 0x40203
desp_str        DCB "Samsung K9K8G08U0M              "
retry_scales    DCD 0xAAAAAAAA
#elif (STORAGE_USED == 0)
sflash_start    DCB "SPIP"
chip_id			DCD 0x1540c8
total_size		DCD 4*1024*1024
pag_size		DCD 256
prg_size		DCD 1
era_size		DCD 64*1024
clock			DCD 10*1000*1000
flag			DCB 0X5
pro_mask		DCB 0X2C
res1			DCB 0X0
res2			DCB 0X0
des_str			DCB "SST25VF032B                     "
#endif
GPIO_LEVEL      DCD 0x04000088
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT  main                    ;this function will be called at the end of the function: __main
main
    ldr     pc, =AK_Main             ;entry user function: akmain()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT __user_initial_stackheap
__user_initial_stackheap
    LDR   r0,=|Image$$ER_ZI$$ZI$$Base|
    ;LDR   r2,=|Image$$ER_ZI$$ZI$$Limit|

    bx lr
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of Reset   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
reset_handler
    ;disable mmu
    mrc  p15,0,r0,c1,c0,0
    bic  r0,r0,# (1) 
    mcr  p15,0,r0,c1,c0,0
    ldr r0,GPIO_LEVEL
    ldr r2,[r1]
    MOV r1,#(1<<16)
    orr r2,r2,r1
    str r2,[r2]
    ;clear bss
    ldr  r0, =|Image$$data$$Limit|
    ldr  r2, =0x0
    ldr  r3, =(RAM_BASE_ADDR + PRE_MAPSIZE)
    
clear_bss_loop
    cmp  r0, r3
    strcc r2, [r0], #4
    bcc  clear_bss_loop

    ;do a trick here, borrow IRQ stack
    mov     r0, #ANYKA_CPU_Mode_SVC
    msr     CPSR_c, r0
    ldr     r1, =PHY_SVC_MODE_STACK
    mov     sp, r1
    
    bl      progmanage_mmu_init    ;Init MMU
    bl      progmanage_init  ;Init remap module
    ldr     r1, =PROG_RESUME_STACK
    mov     sp, r1  
    bl      MMU_Enable    
    bl      set_each_mode_defstack  ; set Default stack
    bl      Fwl_CheckPowerStatus
    bl      progmanage_resume
    bl      set_each_mode_defstack  ; set Default stack

    ;now start ads main function: __main(). User fuction akmain() will be called through this function.
    ldr     pc, =AK_Main             ;entry user function: akmain()

    EXPORT reset_handler_loop
;end of user function, enter dead loop
reset_handler_loop
    b       reset_handler_loop
    
    EXPORT get_spsr
;T_U32 get_spsr(T_VOID)
get_spsr
    mrs     r0, spsr
    bx      lr 

undefined_handler
    ldr     r13, =IRQ_MODE_STACK
    ldr     r0,=UDF_STRING    
    mov     r1,lr
    mov     r2,#1
    bl      akerror
    subs    pc, lr, #4
UDF_STRING  DCB "undf lr:",0x0

    END

