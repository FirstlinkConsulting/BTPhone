

	AREA MMULIB, CODE, READONLY	
	
	EXPORT MMU_Reg6
MMU_Reg6
    mrc  p15,0,r0,c6,c0,0
    bx lr 
           
	

;void MMU_EnableICache(void)
	EXPORT MMU_EnableICache
MMU_EnableICache
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1<<12) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_DisableICache(void)
	EXPORT MMU_DisableICache
MMU_DisableICache       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1<<12) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_EnableDCache(void)
	EXPORT MMU_EnableDCache
MMU_EnableDCache
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1<<2) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_DisableDCache(void)
	EXPORT MMU_DisableDCache
MMU_DisableDCache       
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1<<2) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_EnableMMU(void)
	EXPORT MMU_EnableMMU
MMU_EnableMMU
   mrc  p15,0,r0,c1,c0,0
   orr  r0,r0,# (1)    
   orr  r0,r0,# (1 << 9) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_DisableMMU(void)
	EXPORT MMU_DisableMMU
MMU_DisableMMU
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,# (1) 
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;=========================
; Set TTBase
;=========================
;void MMU_SetTTBase(int base)
	EXPORT MMU_SetTTBase
MMU_SetTTBase
    ;ro=TTBase
    mcr  p15,0,r0,c2,c0,0
    bx lr 

;=========================
; Set Domain
;=========================
;void MMU_SetDomain(int domain)
	EXPORT MMU_SetDomain
MMU_SetDomain
    ;ro=domain
    mcr  p15,0,r0,c3,c0,0
    bx lr 

;=========================
; ICache/DCache functions
;=========================
;void MMU_InvalidateIDCache(void)
	EXPORT MMU_InvalidateIDCache
MMU_InvalidateIDCache
    mcr  p15,0,r0,c7,c7,0
    bx lr 


;void MMU_InvalidateICache(void)
	EXPORT MMU_InvalidateICache
MMU_InvalidateICache
    mcr  p15,0,r0,c7,c5,0
    bx lr 

;void MMU_Clean_Invalidate_Dcache()
   EXPORT  MMU_Clean_Invalidate_Dcache
MMU_Clean_Invalidate_Dcache
   mrc  p15, 0, r15, c7, c14, 3
   bne  MMU_Clean_Invalidate_Dcache
   bx lr 


;===============
; TLB functions
;===============
;voic MMU_InvalidateTLB(void)
	EXPORT MMU_InvalidateTLB
MMU_InvalidateTLB
    mcr  p15,0,r0,c8,c7,0
    bx lr 

;============
; Process ID
;============
;void MMU_SetProcessId(U32 pid)
	EXPORT MMU_SetProcessId
MMU_SetProcessId
    ;r0= pid
    mcr  p15,0,r0,c13,c0,0
    bx lr 

;void MMU_CleanSR(void)
   EXPORT MMU_CleanSR
MMU_CleanSR      
   mrc  p15,0,r0,c1,c0,0
   bic  r0,r0,#(3<<8)
   mcr  p15,0,r0,c1,c0,0
   bx lr 

;void MMU_DrainWriteBuffer()
;  EXPORT  MMU_DrainWriteBuffer
;MMU_DrainWriteBuffer
;  mcr  p15, 0, r0, c7, c10, 4 
;  bx lr 

	END	
