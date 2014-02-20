/**
 * @file arch_init.h
 * @brief list driver library initialize operations
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2008-02-1
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */
#ifndef __ARCH_INIT_H__
#define __ARCH_INIT_H__


#include "anyka_types.h"


/** @defgroup Init Init group
 *  @ingroup Drv_Lib
 */
/*@{*/

/*
 * Used for initialization calls..
 */
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);


#ifdef __GNUC__
#define __initcall(fn) \
initcall_t __initcall_##fn \
__attribute__((__section__(".initcall"))) = fn;
#endif

#ifdef __CC_ARM
#define __initcall(fn) \
initcall_t __initcall_##fn  = fn;
#endif

/**
 *  module init
 */
#define module_init(x)  __initcall(x)

/**
 *  memory alloc callback handler
 */
typedef T_pVOID (*T_RamAlloc)(T_U32 size);

/**
 *  memory free callback handler
 */
typedef T_pVOID (*T_RamFree)(T_pVOID var);

/**
 *  print callback handler
 */
typedef T_pVOID (*T_Print)(const T_CHR *s, T_U32 n, T_BOOL newline);


/** @brief chip name
 * define all chip supported
 */
typedef enum
{
    CHIP_1080L = 0x1080,
    CHIP_1060L = 0x1060,
    CHIP_1050L = 0x1050,
    CHIP_1053L = 0x1053,

    CHIP_RESERVE = 0xffff      ///< reserve
} E_AKCHIP_TYPE;

/** @brief driver init info
 * define chip type and memory callback 
 */
typedef struct tag_DRIVE_INITINFO
{
    E_AKCHIP_TYPE chip;     ///< chip type

    T_Print fPrint;         ///< print callback
    
    T_RamAlloc fRamAlloc;   ///< memory alloc callback function
    T_RamFree  fRamFree;    ///< memory free callback function
} T_DRIVE_INITINFO, *T_PDRIVE_INITINFO;

/**
 * @brief driver library initialization
 *
 * should be called on start-up step, to initial interrupt module and register hardware as camera, lcd...etc.
 * @author xuchang
 * @date 2008-01-21
 * @return void
 */
void drv_init(T_PDRIVE_INITINFO drv_info);

/**
 * @brief memory alloc
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param size T_U32: size of memory to alloc
 * @return void *
 */
T_pVOID drv_malloc(T_U32 size);


/**
 * @brief memory free
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param var T_pVOID: address of memory to free
 * @return void *
 */
T_pVOID drv_free(T_pVOID var);

/**
 * @brief get chip type
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @return T_VOID
 */
E_AKCHIP_TYPE drv_get_chip_type(T_VOID);

/**
 * @brief print func
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param s string to print
 * @param n number to print 
 * @param newline AK_TRUE: add '\n' to string, AK_FALSE: donot add
 * @return void *
 */
T_VOID drv_print(const T_CHR *s, T_U32 n, T_BOOL newline);


/*@}*/
#endif //__ARCH_INIT_H__

