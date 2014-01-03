/** @file anyka_bsp.h
 * @brief BSP(board support packet) file
 *
 * User must define the physical info of the board here. 
 * such as  FLASH/RAM/STACK etc
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */
#ifndef _ANYKA_BSP_H_
#define _ANYKA_BSP_H_

#define USED_MMU                1

/* Memory start address */
#define RAM_BASE_ADDR           0x00800000  // RAM start address

/** @{@name RAM Define 
 *  Define the RAM size of the system
 */
#define RAM_SIZE                0x200000    //2M

#define PAGE_BITNUM             12
#define PT_ALIGN_BITNUM         10

/* physical memory size 
 */
#define NORMAL_PHYSICAL_SIZE    (160*1024)
#define TT_PHYSICAL_SIZE        (512)
#define MAX_PHYSICAL_SIZE       (NORMAL_PHYSICAL_SIZE + TT_PHYSICAL_SIZE) //(160.5*1024) 

/* fix memory size 
 */
#define PRE_MAPSIZE             (32*1024) 
#define REMAP_MEM_PSIZE         (NORMAL_PHYSICAL_SIZE - PRE_MAPSIZE)

/* no counter memory size 
 */
#define NO_COUNTER_SIZE         (32*1024)

/* address start
 */
#define SIZE_ALIGN(size, mode_bitnum)  (((((size)+(1<<(mode_bitnum))-1)>>(mode_bitnum)))<<(mode_bitnum))
#define REMAP_PADDR_START       (RAM_BASE_ADDR + PRE_MAPSIZE)
#define REMAP_VADDR_START       (RAM_BASE_ADDR + SIZE_ALIGN(MAX_PHYSICAL_SIZE, PAGE_BITNUM))

/** @} */

/* Memory distribution for use remap
 * |---------------------------------- RAM_SIZE
 * |            svc stack
 * |            virtual page area
 * |---------------------------------- 0x29000
 * |            can not use
 * |---------------------------------- 0x28200
 * |            irq stack  (472 B)
 * |---------------------------------- 0x28020
 * |            mmu TT     (40 B)
 * |---------------------------------- 0x28000
 * |            physical page area for remap
 * |---------------------------------- 0x8000
 * |            mmu pT     (2K B)
 * |---------------------------------- 0x7800
 * |            fiq stack  (512 B)
 * |---------------------------------- 0x7600
 * |            l2 fifo    (1.5 kB)
 * |---------------------------------- 0x7000
 * |            abort stack(1 kB)
 * |---------------------------------- 0x6c00
 * |            code + bss
 * |---------------------------------- 0x0000
*/

/* define the MMU size and start address */
#define _MMU_TT_SIZE            (0x28)     //10*4= 40B 
#define _MMU_PT_SIZE            (0x800)    //2k page table

/* define the FIFO address */
#define L2_FIFO_ADDR            (RAM_BASE_ADDR + 0x7000)
#define L2_FIFO_SIZE            (0x600)    //(1.5*1024)

/* MUST BE 16k aligned */
#define _MMUTT_PADDRESS         (RAM_BASE_ADDR + NORMAL_PHYSICAL_SIZE)  // 
#define _MMUPT_PADDRESS         (SIZE_ALIGN(L2_FIFO_ADDR+L2_FIFO_SIZE, PT_ALIGN_BITNUM))    

/* each stack size and start address */
#define SVC_MODE_STACK          (RAM_BASE_ADDR + RAM_SIZE)      //2M offset
#define IRQ_MODE_STACK          (RAM_BASE_ADDR + MAX_PHYSICAL_SIZE)   //160.5k offset
#define FIQ_MODE_STACK          (L2_FIFO_ADDR + L2_FIFO_SIZE + 0x200)
#define ABORT_MODE_STACK        (L2_FIFO_ADDR)//(_MMUPT_PADDRESS)               //offset

#define IRQ_MODE_STACK_SIZE     (TT_PHYSICAL_SIZE - _MMU_TT_SIZE)     
#define ABORT_MODE_STACK_SIZE   (1024)//((1<<PAGE_BITNUM) - _MMU_PT_SIZE - L2_FIFO_SIZE)     

#define ABORT_MODE_STACK_END    (ABORT_MODE_STACK - ABORT_MODE_STACK_SIZE)

#define MMU_TT_start            (_MMUTT_PADDRESS)
#define MMU_PT_end              (_MMUPT_PADDRESS + _MMU_PT_SIZE)

#define resident_data_end       (RAM_BASE_ADDR + PRE_MAPSIZE)

#define PHY_SVC_MODE_STACK      (RAM_BASE_ADDR + NORMAL_PHYSICAL_SIZE)
#define PHY_SVC_MODE_STACK_SIZE (1<<PAGE_BITNUM)

#define PROG_RESUME_STACK       (_MMUPT_PADDRESS) //(ABORT_MODE_STACK - ABORT_MODE_STACK_SIZE) 

/** @} */

/*@}*/


#endif  // #ifndef _ANYKA_BSP_H_


