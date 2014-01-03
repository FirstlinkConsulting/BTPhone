/**
 * @FILENAME: remap.h
 * @BRIEF remap virtual memory to physical memory by mmu
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR Justin.Zhao
 * @DATE 2008-1-9
 * @VERSION 1.0
 * @REF
 */

 
#ifndef __REMAP_H__
#define __REMAP_H__

#include "anyka_types.h"



typedef T_BOOL (*REMAP_CALLBACK_FUN_LOAD)( T_pVOID addr, T_U32 size);
typedef T_BOOL (*REMAP_CALLBACK_FUN_FLUSH)( T_pVOID addr, T_U32 size);

typedef struct {
    REMAP_CALLBACK_FUN_LOAD     remapload;                /*load page callback function */
    REMAP_CALLBACK_FUN_FLUSH    remapflush;             /*write back page callback function */
}T_REMAP_CALLBACK;



 /******************************************************************************
 * @NAME    remap_init
 * @BRIEF       init remap kernal, andthen goto the init phase.
            the init phase is different from normal, remap_alloc allots temp buffer after
            address of initphase_bss_end. it needs to call remap_initphase_end function 
            when passed the init phase. 
            
 * @AUTHOR  xuping
 * @DATE    2010-02-24
 * @PARAM:   callbackfun:the call back which used in remap kernal
 *           bFlushEnable: support flush operation or not
             ramSize : remap the size of vir add.
 *           T_VOID
 * @RETURN T_VOID
 *   
 *******************************************************************************/
 T_VOID remap_init(T_REMAP_CALLBACK *callbackfun, T_BOOL bFlushEnable, T_U32 ramSize);

/*@brief remap a virtual address 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 *@author Justin Zhao
 *@date 2008.1.21
 *@input:
 *  T_U32 fault_addr: the virtual memory addr
 *  T_U32 pc_lr: the address of the current pc register
 *@version 1.0
 */
T_VOID do_swap(T_U32 fault_addr, T_U32 pc_lr, T_U32 sp);



/**
 * @brief alloc a virtual page for temp buffer
 * @note must be very careful to use this function, after the buffer is used, free it as quickly as possible
 * @param size, should be a memory page size
 * @retval T_pVOID, the buffer virtual address
 */
T_pVOID remap_alloc(T_U32 size);

/**
 * @brief free the allocated buffer
 * @note this function should be called after remap_alloc()
 * @param ptr, the buffer virtual address to free
 * @retval T_pVOID AK_NULL
 */
T_pVOID remap_free(T_pVOID ptr);

 /******************************************************************************
* @NAME    remap_lock_page
* @BRIEF   lock virtual address in physical page
* @AUTHOR  xuping
* @DATE    2010-02-28;
* @PARAM:   vaddr:the virtual address which will be locked(enlocked)
*           size: size of  be locked(unlocked)
*           lock:AK_TRUE ,lock  AK_FALSE: unlock
* @RETURN:AK_TRUE:success AK_FALSE:fail
*   
*******************************************************************************/
T_BOOL remap_lock_page(T_U32 vaddr, T_U32 size, T_BOOL lock);


  /******************************************************************************
* @NAME    remap_unload_pages
* @BRIEF:  unload the pages between given virtual address
* @AUTHOR  xuping
* @DATE    2010-02-28
* @PARAM:   vaddr:the virtual address
*           size: the memory length
*           binval: AK_TRUE, invalidate the pages AK_FALSE:only unmap pages
* @RETURN:T_VOID
*   
*******************************************************************************/
T_VOID remap_unload_pages(T_U32 vaddr, T_U32 size, T_BOOL binval);

/******************************************************************************
 * @NAME    remap_videomemory_enable
 * @BRIEF:  unload the pages between given virtual address
 * @AUTHOR  xuping
 * @DATE    2010-03-01
 * @PARAM:   benable: AK_TRUE:enable use video memory 
 *                    AK_FALSE:  video memory is forbidden
 * @RETURN:AK_TRUE ,success   AK_FALSE, failture
 *   
 *******************************************************************************/
 //T_BOOL remap_videomemory_enable(T_BOOL benable);

 /******************************************************************************
* @NAME    page_counter_interrupt_handle
* @BRIEF   the interrupt is generate when the value of the COUNTER_REG REG reach 0xffffff
* @AUTHOR  xuping
* @DATE    2010-02-24
* @PARAM 
*           T_VOID
* @RETURN T_VOID
*   
*******************************************************************************/
T_BOOL page_counter_interrupt_handle(T_VOID);


/**
 * @brief check the virtual address is valid or not
 * @param vaddr, the virtual address
 * @retval T_BOOL, if valid return AK_TRUE
 */
T_BOOL remap_isvalid_addr(T_U32 vaddr);




/**
 * @brief reserve the consecutive physic pages for DMA or other purpose
 * @param vaddr, the virtual address
 * @param size, the memory length to reserve
 * @param resv, resv or free
 * @retval if success, return AK_TRUE
 */
T_BOOL remap_resv_page(T_U32 vaddr, T_U32 size, T_BOOL resv);
T_BOOL remap_isreserved_page(T_U32 vaddr);

  /******************************************************************************
* @NAME    remap_unloadpage
* @BRIEF:  unload the vir pages,:only unmap pages
* @AUTHOR  xuping
* @DATE    2010-02-28
* @PARAM:   vir_addr:the virtual address
* @RETURN:T_VOID
*   
*******************************************************************************/
T_VOID remap_unloadpage(T_U32 vir_addr);
  
/******************************************************************************
* @NAME  remap_get_vaddrindex
* @BRIEF       get index of the ram page with vaddr. 
* @AUTHOR  liangxiong
* @DATE  2012-05-23
* @PARAM       vaddr: the virtual address.(accord 4K page)
* @RETURN the index of ram page(except the fixed 40K memory)
*   
*******************************************************************************/
T_S32 remap_get_vaddrindex(T_U32 vaddr);

/******************************************************************************
 * @NAME    remap_get_phy2virTab
 * @BRIEF   Get the physical addr to virtual for other files. 
 * @AUTHOR  lisichun
 * @DATE    2012-09-12
 * @PARAM   index: the index of the ram page.(accord 4K page)
 * @RETURN  the index of physical addr to virtual
 *   
 *******************************************************************************/
T_U32 remap_get_phy2virTab(T_U32 index);

/******************************************************************************
 * @NAME    remap_set_phy2virTab
 * @BRIEF   set the virtual to the index of physical addr to  for other files. 
 * @AUTHOR  lisichun
 * @DATE    2012-09-12
 * @PARAM   index: the index of the ram page.
            vaddr: virtual address
 * @RETURN  NONE
 *   
 *******************************************************************************/
void remap_set_phy2virTab(T_U32 index,T_U32 vaddr);

/******************************************************************************
  * @NAME    remap_clr_pg_resv
  * @BRIEF   clear the reserve physic page bit.
  * @AUTHOR  lisichun
  * @DATE    2012-09-12
  * @PARAM   index: the index of reserve physic page bitmap.
  * @RETURN  NONE
  *   
  *******************************************************************************/
void remap_clr_pg_resv(T_U32 index);

/******************************************************************************
 * @NAME    remap_set_pg_resv
 * @BRIEF   set the reserve physic page bit.
 * @AUTHOR  lisichun
 * @DATE    2012-09-12
 * @PARAM   index: the index of reserve physic page bitmap.
 * @RETURN  NONE
 *   
 *******************************************************************************/
void remap_set_pg_resv(T_U32 index);

/******************************************************************************
  * @NAME    remap_page_is_resv
  * @BRIEF   check the reserve physic page bit.
  * @AUTHOR  lisichun
  * @DATE    2012-09-12
  * @PARAM   index: the index of reserve physic page bitmap.
  * @RETURN  NONE
  *   
  *******************************************************************************/
T_U8 remap_page_is_resv(T_U32 index);

/******************************************************************************
  * @NAME    remap_data_notin_remap
  * @BRIEF   check the data is in or not in the  T_REMAP remap;
  * @AUTHOR  lisichun
  * @DATE    2012-09-12
  * @PARAM   pi: the address of the data.
  * @RETURN  if the data is in in the  T_REMAP remap,return AK_TRUE
  *          otherwise return AK_FALSE.
  *******************************************************************************/
T_BOOL remap_data_notin_remap(T_U32 *pi);

/******************************************************************************
  * @NAME   remap_page_is_dirtyone
  * @BRIEF   check the index of page is dirty or not.
  * @AUTHOR  lisichun
  * @DATE    2012-09-12
  * @PARAM   index: the index of  physic page.
  * @RETURN  the state flag of the index of page.
  *   
  *******************************************************************************/
T_U32 remap_page_is_dirtyone(T_U32 index); 

/******************************************************************************
  * @NAME   remap_set_page_cleanone
  * @BRIEF   clean the index of page.
  * @AUTHOR  lisichun
  * @DATE    2012-09-12
  * @PARAM   index: the index of  physic page.
  * @RETURN  NONE
  *   
  *******************************************************************************/
void remap_set_page_cleanone(T_U32 index);

/******************************************************************************
  * @NAME    remap_initphase_end
  * @BRIEF   it note passed the init phase of program. remap can work. 
  * @AUTHOR  liangxiong
  * @DATE    2013-03-08
  * @PARAM   NONE
  * @RETURN  NONE
  *   
  *******************************************************************************/
T_VOID remap_initphase_end(T_VOID);


#endif


