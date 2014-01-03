/************************************************ 
  NAME    : MMU.H
  DESC    :
  Revision: 02.28.2002 ver 0.0
 ************************************************/


#ifndef __MMU_H__
#define __MMU_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "anyka_types.h"



#define CB              (3<<2)          //cache_on, write_back
#define CNB             (2<<2)          //cache_on, write_through 
#define NCB             (1<<2)          //cache_off,WR_BUF on
#define NCNB            (0<<2)          //cache_off,WR_BUF off

//macro of section or page table descrption
#define DESC_PAGE       (0x1|(1<<4))    //description of coarse page
#define DESC_SEC        (0x2|(1<<4))    //description of section

#define AP_RW           (3<<10)         //supervisor=RW, user=RW
#define AP_RO           (2<<10)         //supervisor=RW, user=RO

#define DOMAIN_FAULT    (0x0)           //no access
#define DOMAIN_CHK      (0x1)
#define DOMAIN_NOTCHK   (0x3)

#define DOMAIN0_ATTR    (DOMAIN_CHK<<0)
#define DOMAIN1_ATTR    (DOMAIN_FAULT<<2)

#define DOMAIN0         (0x0<<5)        //DOMAIN0_ATTR->DOMAIN_CHK
#define DOMAIN1         (0x1<<5)        //DOMAIN1_ATTR->DOMAIN_FAULT

#define RW_CB           (AP_RW|DOMAIN0|CB|DESC_SEC)
#define RW_CNB          (AP_RW|DOMAIN0|CNB|DESC_SEC)
#define RW_NCNB         (AP_RW|DOMAIN0|NCNB|DESC_SEC)
#define RW_FAULT        (DOMAIN1|NCNB|DESC_SEC)         //supervisor=forbid, user=forbid
#define RW_FB           (DOMAIN1|NCNB|DESC_SEC)         //supervisor=forbid, user=forbid
#define DEF_PAGE        (DOMAIN0|DESC_PAGE)             //define a coarse page

//macro of page description
#define DESC_UNMAP      (0x0)
#define DESC_SMALL      (0x2)                           //description of a small page
#define DESC_LARGE      (0x1)                           //description of a large page
#define PAGE_AP_RW      ((3<<4)|(3<<6)|(3<<8)|(3<<10))  //ap0,ap1,ap2,ap3 all are rw
#define PAGE_AP_RO      ((0<<4)|(0<<6)|(0<<8)|(0<<10))  //ap0,ap1,ap2,ap3 all are ro when S=0  R=1

#define RO_SMALL        (PAGE_AP_RO|CNB|DESC_SMALL)     //supervisor=RW, user=RW
#define RW_SMALL        (PAGE_AP_RW|CNB|DESC_SMALL)     //supervisor=RW, user=RW
#define RW_NCNB_SMALL   (PAGE_AP_RW|NCNB|DESC_SMALL)    //supervisor=RW, user=RW
#define FB_SMALL        (DESC_UNMAP)//(CNB|DESC_SMALL)   //unmap second level descriptor

#define VPAGE_SIZE      (4096)
#define VPAGE_BITS      (12)
#define SPARE_SIZE      (512)
#define SPARE_BITS      (9)

//check the data is mapping or not.
#define MMU_CheckMaping(ptdata)    (((ptdata)&(0x3)) ? AK_TRUE : AK_FALSE)


T_VOID MMU_SetMTT(T_U32 vaddrStart,T_U32 vaddrEnd,T_U32 paddrStart,T_U32 attr);
T_VOID MMU_CleanCache(T_VOID);
T_VOID MMU_Clean_Invalidate_Dcache(T_VOID);

//T_VOID ChangeRomCacheStatus(T_U32 attr);

T_VOID MMU_SetTTBase(int base);
T_VOID MMU_SetDomain(int domain);
T_VOID MMU_SetProcessId(T_U32 pid);
T_VOID MMU_CleanSR(T_VOID);

T_BOOL MMU_IsPremapAddr(T_U32 addr);


#ifdef __cplusplus
}
#endif


#endif /*__MMU_H__*/

