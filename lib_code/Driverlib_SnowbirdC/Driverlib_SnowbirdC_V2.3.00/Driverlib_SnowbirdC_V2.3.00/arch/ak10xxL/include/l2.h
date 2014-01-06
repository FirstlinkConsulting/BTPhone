/**
 * @file l2.h
 * @brief Define L2 driver header file
 * This file provides L2 driver APIs: 
 * interrupt handler, and keypad scaning function.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author YiRuoxiang
 * @date 2008-05-09
 * @version 1.0
 * @ref
 */
#ifndef _L2_H_
#define _L2_H_


#include "anyka_types.h"
#include "drv_cfg.h"


#ifdef UNSUPPORT_REMAP

// location of L2's 4K, from 0 to 68, because L2 is 276 KB
#define LOCAT_L2_4K         39

#else

//location of L2's 4K, from 0 to 47, because L2 is 192 KB
extern T_U32   local_l2_4k;
#define LOCAT_L2_4K         local_l2_4k

#endif


//each buffer's address is based on the 4K location
#define BUF_ADDR_BASE       (LOCAT_L2_4K << 12)
#define L2_FIFO_BASE        (L2_START_ADDR + BUF_ADDR_BASE)

//data transfer direction
#define BUF2MEM             0
#define MEM2BUF             1
#define MEM2MEM             2


typedef enum
{
    ADDR_ADC = 0,   //L2 Buffer Configuration Register 1
    ADDR_DAC,
    ADDR_SDMMC,
    ADDR_SPI1_TX,
    ADDR_SPI1_RX,
    ADDR_SPI2_TX,
    
    ADDR_USB0,      //L2 Buffer Configuration Register 2
    ADDR_USB1,
    ADDR_USB2,
    ADDR_USB3,
    ADDR_USB4,
    ADDR_SPI2_RX,

    ADDR_PCM_TX,    //L2 Buffer Configuration Register 3
    ADDR_PCM_RX,
    ADDR_UART1_TX,
    ADDR_UART1_RX,
#if (CHIP_SEL_10C > 0)
    ADDR_UART2_TX,
    ADDR_UART2_RX,
#endif

    ADDR_MCI1,
    ADDR_MCI2,
    
    DEVICE_NUM
}DEVICE_SELECT;



/**
 * @brief       init the L2
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID l2_initial(T_VOID);


/**
 * @brief       get the interrupt type of the ldma
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID 
 * @return      T_U8
 * @retval      the DEVICE_SELECT which cause the interrupt
 */
T_U16 l2_get_intr_type(T_VOID);


/**
 * @brief       set the status of the buffer
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   buf_id
 * @param[in]   buf_status
 *              when (buf_id <= INVALID_BUF_ID), 0<= buf_status <= 8
 *              when (buf_id > INVALID_BUF_ID), 0<= buf_status <= 2
 * @return      T_VOID
 */
T_VOID l2_set_status(T_U8 buf_id, T_U8 buf_status);


/**
 * @brief       get the status of the buffer
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   buf_id 
 * @return      T_U8
 * @retval      the status value of the buffer specialed by buf_id
 */
T_U8 l2_get_status(T_U8 buf_id);


/**
 * @brief       enable or disable the L2 interrupt
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   enable, AK_TRUE, enable; AK_FALSE, disable 
 * @return      T_VOID
 */ 
T_VOID l2_int_ctrl(T_BOOL enable);


/**
 * @brief       disable the ldma interrupt
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID ldma_int_disable(T_VOID);


/**
 * @brief       stop the ldma transfer
 * @author      zhanggaoxin
 * @date        2013.02.25
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID ldma_trans_stop(T_VOID);


/**
 * @brief       alloc a buffer for the device
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 * @remark      should not call the print function when the device is 
 *              ADDR_UART_TX
 */ 
T_BOOL l2_init_device_buf(DEVICE_SELECT device);


/**
 * @brief       free a buffer for the device
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL l2_release_device_buf(DEVICE_SELECT device);


/**
 * @brief       get the buffer address and the length of the device
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device  
 * @param[in]   pbuf, pointer to a T_U32 type variable for fetching the 
 *              buffer address.
 * @param[in]   plen, pointer to a T_U32 type variable for fetching the 
 *              length of the buffer.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE,
 *              means the device buffer is not init.
 */ 
T_BOOL get_device_buf_info(DEVICE_SELECT device, T_U32 *pbuf_addr, 
                           T_U32 *pbuf_len, T_U32 *pbuf_id);


/**
 * @brief       transfer the data along ram and l2 buffer by cpu
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   dest_addr 
 *              the destination address
 * @param[in]   src_addr  
 *              the source address
 * @param[in]   size 
 *              the number of bytes to transfer
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */
T_BOOL l2_trans_data_cpu(T_U32 dest_addr, T_U32 src_addr, T_U32 size, T_U32 tran_offset);


/**
 * @brief       transfer the data along ram and l2 buffer by ldma
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   dest_addr 
 *              the destination address, must 4bytes align
 * @param[in]   src_addr  
 *              the source address, must 4bytes align
 * @param[in]   size 
 *              the number of bytes to transfer. must not more than 4K
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */
T_BOOL l2_trans_data_dma(T_U32 dest_addr, T_U32 src_addr, T_U32 size);


#endif  /* _L2_H_ */

