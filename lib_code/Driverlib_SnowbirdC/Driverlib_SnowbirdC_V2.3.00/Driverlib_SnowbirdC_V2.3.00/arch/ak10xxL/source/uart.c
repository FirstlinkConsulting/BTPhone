/*******************************************************************************
 * @file    uart.c
 * @brief   UART driver, define UARTs APIs.
 * This file provides UART APIs: UART initialization, write data to UART.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  ChenWeiwen
 * @date    2008-06-07
 * @version 1.0
 * @ref     AK1080L technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_uart.h"
#include "interrupt.h"
#include "share_pin.h"
#include "clk.h"
#include "l2.h"
#include "hal_errorstr.h"
#include "drv_cfg.h"
#include "boot.h"
#include "arch_timer.h"

#define BAUD_REG(asic_clk, baud_rate)   ((asic_clk / baud_rate) - 1)

#define UART1_INIT                      (1 << 0)
#define UART2_INIT                      (1 << 1)

#define UART_RX_SIZE_MASK                    (0x1F<<13)
#define UART_RX_WORD_CNT(regval)            ((((regval)>>13)&0x1F))
#define UART_RX_BYTE_CNT(regval)            ((regval>>23)&0x03)
#define UART_RX_UNALIGNED_SIZE(word, byte)  ((((word)-1)<<2)+(byte))
#define UART_RX_ALIGNED_SIZE(word)          ((word)<<2)


#pragma arm section zidata = "_drvbootbss_"
static T_U8             s_uart_initmap;

//UART l2 buf offset
static volatile T_U32   Pre_Uart_off;
static T_eUART_BAUDRATE s_uart_baudrate[MAX_UART_NUM];
static T_fUART_CALLBACK s_uart_callback[MAX_UART_NUM];
#ifdef __ENABLE_UART_INT__
static T_U32 rx_addr;
static volatile T_U8 uart_tmr_int_flag = 0;
static volatile T_U32 uart_rx_cur_cnt=0;
static T_TIMER m_timer_id=ERROR_TIMER;
#endif
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_UART;
#pragma arm section rodata

T_VOID uart_rx_timer_lookup(T_TIMER timer_id, T_U32 delay);

//此处使用SPIBOOT来做判断是一个临时做法
//等切换到10C芯片上后要改用10C来做判断
//#if CHIP_SEL_10C > 0
#if (DRV_SUPPORT_SPI_BOOT > 0) ///////?????????????
#pragma arm section code = "_drvbootcode_"
#else
#pragma arm section code = "_drvbootinit_" 
#endif
/*******************************************************************************
 * @brief   init the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]baud_rate
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE; 
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
T_BOOL uart_init(T_eUART_ID id, T_eUART_BAUDRATE baud_rate)
{
    T_U32               asic_clk;
    T_U32               uart_reg_base;
    DEVICE_SELECT       rx_l2_select, tx_l2_select;
    T_eSHARE_PIN_CFG    pin_cfg;


    if (uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        rx_l2_select  = ADDR_UART1_RX;
        tx_l2_select  = ADDR_UART1_TX;
        pin_cfg       = ePIN_AS_UART1;
        s_uart_initmap |= UART1_INIT;
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        rx_l2_select  = ADDR_UART2_RX;
        tx_l2_select  = ADDR_UART2_TX;
        pin_cfg       = ePIN_AS_UART2;
        s_uart_initmap |= UART2_INIT;
    }
#endif
    else
    {
        return AK_FALSE;
    }

    asic_clk = clk_get_asic();
    s_uart_baudrate[id] = baud_rate;

    if(AK_FALSE == sys_share_pin_is_lock(pin_cfg))
    {
        sys_share_pin_lock(pin_cfg);
    }
	REG32(0x00400108) &=(~(7<<3));

    if ((UART1_INIT | UART2_INIT) != s_uart_initmap)
    {
        sys_module_enable(eVME_UART_CLK, AK_TRUE);
        sys_module_reset(eVME_UART_CLK);
    }

	Pre_Uart_off = 0;

    REG32(uart_reg_base + oUART_CFG1) = (TX_STA_CLR | RX_STA_CLR |
                             UART_INTERFACE_EN | BAUD_RATE_ADJ_EN |
                             BAUD_REG(asic_clk, baud_rate));



    //alloc tx l2 fifo 
    if(AK_FALSE == l2_init_device_buf(tx_l2_select))
    {
        return 0;
    }

    #ifndef __ENABLE_UART_INT__
	if (uiUART1==id)
	{
	    if(AK_FALSE == l2_init_device_buf(rx_l2_select))
	    {
	        l2_release_device_buf(tx_l2_select);
	        return AK_FALSE;
	    }
	}
#endif

    return AK_TRUE;
}
#pragma arm section code


/*******************************************************************************
 * @brief   close the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE; 
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
#pragma arm section code = "_drvfrequent_"
T_BOOL uart_close(T_eUART_ID id)
{
    T_U32               uart_reg_base;
    DEVICE_SELECT       tx_l2_select;
    DEVICE_SELECT       rx_l2_select;
    T_eSHARE_PIN_CFG    pin_cfg;


    if (uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        tx_l2_select  = ADDR_UART1_TX;
        rx_l2_select  = ADDR_UART1_RX;
        pin_cfg       = ePIN_AS_UART1;
        s_uart_initmap &= ~UART1_INIT;
        #ifdef __ENABLE_UART_INT__
        if (s_uart_callback[uiUART1])
        {
            timer_stop(m_timer_id);
            m_timer_id = ERROR_TIMER;
            s_uart_callback[uiUART1] = AK_NULL;
        }
        #endif //__ENABLE_UART_INT__
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        tx_l2_select  = ADDR_UART2_TX;
        rx_l2_select  = ADDR_UART2_RX;
        pin_cfg       = ePIN_AS_UART2;
        s_uart_initmap &= ~UART2_INIT;
        #ifdef __ENABLE_UART_INT__
        if (s_uart_callback[uiUART2])
        {
            timer_stop(m_timer_id);
            m_timer_id = ERROR_TIMER;
            s_uart_callback[uiUART2] = AK_NULL;
        }
        #endif //__ENABLE_UART_INT__
    }
#endif
    else
    {
        return AK_FALSE;
    }

    s_uart_baudrate[id] = 0;

    REG32(uart_reg_base+oUART_CFG1) &= ~(UART_INTERFACE_EN);
	sys_share_pin_unlock(pin_cfg);
	
    if (0 == s_uart_initmap)
    {
        sys_module_enable(eVME_UART_CLK, AK_FALSE);
        //sys_share_pin_unlock(pin_cfg);
    }
    l2_release_device_buf(tx_l2_select);
    l2_release_device_buf(rx_l2_select);
	Pre_Uart_off = 0;

    return AK_TRUE;
}
#pragma arm section code


#ifdef __ENABLE_UART_INT__
/*******************************************************************************
 * @brief   uart_set_callback
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]id  
 * @param   [in]callback func
 * @return  T_void
*******************************************************************************/
#pragma arm section code = "_drvfrequent_"
T_VOID uart_set_callback(T_eUART_ID id, T_fUART_CALLBACK callback_func)
{
    T_U32               len,tmp;
    T_U32               intr_en_bit;
    T_U32               uart_reg_base;
    DEVICE_SELECT       rx_l2_select;


    if (uiUART1 == id)
    {
        rx_l2_select  = ADDR_UART1_RX;
        if (AK_NULL == callback_func)
        {
            INT_DISABLE(INT_EN_UART1);
            if (s_uart_callback[uiUART1])
            {
                timer_stop(m_timer_id);
                m_timer_id = ERROR_TIMER;
            }
            s_uart_callback[uiUART1] = AK_NULL;
            l2_release_device_buf(rx_l2_select);
            return;
        }
        else
        {
            intr_en_bit   = INT_EN_UART1;
            uart_reg_base = UART1_MODULE_BASE_ADDR;
        }
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        rx_l2_select  = ADDR_UART2_RX;
        if (AK_NULL == callback_func)
        {
            INT_DISABLE(INT_EN_UART2);
            if (s_uart_callback[uiUART2])
            {
                timer_stop(m_timer_id);
                m_timer_id = ERROR_TIMER;
            }            
            s_uart_callback[uiUART2] = AK_NULL;
            l2_release_device_buf(rx_l2_select);
            return;
        }
        else
        {
            intr_en_bit   = INT_EN_UART2;
            uart_reg_base = UART2_MODULE_BASE_ADDR;
        }
    }
#endif
    else
    {
        return;
    }

    //alloc l2 buffer
    if(AK_FALSE == l2_init_device_buf(rx_l2_select))
    {
        return;
    }
    
    REG32(uart_reg_base+oUART_CFG1) |= RX_ADR_CLR;

    /* open timeout function */
    REG32(uart_reg_base+oUART_CFG1) |= (RX_TIMEOUT_EN);

    /* for uart receive data */
    REG32(uart_reg_base+oUART_CFG2) |= (TIMEOUT_INT_EN | RX_TH_INT_EN | RX_BUF_FULL_INT_EN|RX_ERROR_INT_EN);
    /* RX_TH_CLR must set to 1, then set to 0,or the threshold_int won't occur */
    REG32(uart_reg_base+oUART_BUF_TRSHLD) &= ~0x1f;
    REG32(uart_reg_base+oUART_BUF_TRSHLD) |= (TX_TH_CLR|RX_TH_CLR|7);//(cfg+1)*4byte Receive Interrupt
    REG32(uart_reg_base+oUART_BUF_TRSHLD) &= ~(RX_TH_CLR);
    get_device_buf_info(rx_l2_select, &rx_addr, &len, &tmp);

    s_uart_callback[id] = callback_func;
    //INT_ENABLE(intr_en_bit);
	Pre_Uart_off = 0;
    FIQ_INT_ENABLE(intr_en_bit);
	uart_tmr_int_flag = 0;
    uart_rx_cur_cnt = 0;

    if (ERROR_TIMER == m_timer_id)
        m_timer_id = timer_start(5, AK_TRUE, uart_rx_timer_lookup);
}
#pragma arm section code
#endif


#pragma arm section code = "_changefreq_"
/*******************************************************************************
 * @brief   change uart when baud_rate or asic_clk changed
 * @author  ChenWeiwen
 * @date    2008-06-07
 * @param   [in]asic_clk: system clock
 * @return  T_VOID
*******************************************************************************/
T_VOID uart_on_change(T_U32 asic_clk)
{
    if (s_uart_initmap & UART1_INIT)
    {
        REG32(UART1_MODULE_BASE_ADDR+oUART_CFG1) = (TX_STA_CLR | RX_TIMEOUT_EN |
                         UART_INTERFACE_EN | BAUD_RATE_ADJ_EN |
                         BAUD_REG(asic_clk, s_uart_baudrate[uiUART1]));
    }
#if (CHIP_SEL_10C > 0)
    if (s_uart_initmap & UART2_INIT)
    {
        REG32(UART2_MODULE_BASE_ADDR+oUART_CFG1) = (TX_STA_CLR | RX_TIMEOUT_EN |
                         UART_INTERFACE_EN | BAUD_RATE_ADJ_EN |
                         BAUD_REG(asic_clk, s_uart_baudrate[uiUART2]));
    }
#endif
    
}
#pragma arm section code


#pragma arm section code = "_drvbootcode_" 
/*******************************************************************************
 * @brief   send data to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]data, pointer to a T_U8 type buffer.
 * @param   [in]byte_nbr, the length of the buffer.
 * @return  T_U32
 * @retval  the number of the bytes sended to uart, if 0, mean the 
 *          ADDR_UART_TX buffer is no init.
*******************************************************************************/
static T_U32 uart_write_cpu(T_eUART_ID id, const T_U8 *data, T_U32 byte_nbr)
{
    T_U32               value;
    T_U32               buf_addr;
    T_U32               buf_len;
    T_U32               buf_id;
    T_U32               uCount;
    T_U32               left;
    T_U32               uart_reg_base;
    DEVICE_SELECT       tx_l2_select;


    if(uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        tx_l2_select  = ADDR_UART1_TX;
    }
#if (CHIP_SEL_10C > 0)
    else if(uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        tx_l2_select  = ADDR_UART2_TX;
    }
#endif
    else
    {
        return 0;
    }


    if(AK_FALSE == get_device_buf_info(tx_l2_select, &buf_addr, 
        &buf_len, &buf_id))
    {
        return 0;
    }

    left = byte_nbr; //use for tmp
    
    while(left)
    {
        while(1)
        {
            value = REG32(uart_reg_base+oUART_CFG2);
            if(value & TX_END)
            {
                break;
            }
        }
        
        if(left > buf_len)
        {
            uCount = buf_len;
        }
        else
        {
            uCount = left;
        }

#if 0//(CHIP_SEL_10C > 0)
        //do a trick, can't send 64 mutiple size
        if ((uCount & 0x3f) == 0 )
            uCount -= 4;
#endif

        left = left - uCount;

        l2_set_status(buf_id, 0);
        
#if 0//(CHIP_SEL_10C > 0)
        if ((uCount & 0x3f) == 0 && uiUART2 == id)
            *((T_U32 *)(buf_addr+0x3c)) = 0;
#endif        

        l2_trans_data_cpu(tx_l2_select, (T_U32)data, uCount, 0);
        data = data + uCount;
#if (CHIP_SEL_10C > 0)
			//if ((uCount & 0x3f) == 0)
			//	*((T_U32 *)(buf_addr+0x3c)) = 0;
			l2_set_status(buf_id, 3);
#endif

        REG32(uart_reg_base+oUART_CFG1) |= TX_STA_CLR;            //clear tx status

        value = REG32(uart_reg_base+oUART_CFG2);
        value &= ~(0xFFF << TX_BYT_CNT);
        value |= ((uCount << TX_BYT_CNT) | TX_BYT_CNT_VLD);
		// lixm 2013-7-16, don't clear rx status
		// cdh:value &= 0xbffffffb;
		value &= 0xbffffffb;

        REG32(uart_reg_base+oUART_CFG2) = value;

    }
    
    return byte_nbr;
}


/*******************************************************************************
 * @brief   send one char to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]chr
 * @return  T_BOOL
 * @retval  0, fail, or else succeed.
*******************************************************************************/
T_BOOL uart_write_chr(T_eUART_ID id, T_U8 chr)
{
    if (s_uart_callback[id] == AK_NULL)
    {
        return uart_write_cpu(id, &chr, 1);
    }
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   send string to uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id
 * @param   [in]str
 * @return  T_U32
 * @retval  the number of the bytes sended to uart, if 0, mean the 
 *          ADDR_UART_TX buffer is no init.
*******************************************************************************/
T_U32 uart_write_dat(T_eUART_ID id, T_U8 *str, T_U32 len)
{
    if (AK_NULL == str)
    {
        return 0;
    }

    return uart_write_cpu(id, str, len);
}

/*******************************************************************************
 * @brief   uart_get_write_status
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]id
 * @return  T_bool
*******************************************************************************/
T_BOOL uart_get_write_status(T_eUART_ID id)
{
    T_U32 value;
    T_U32 uart_reg_base;
    
    if(uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
    }
#if (CHIP_SEL_10C > 0)
    else if(uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
    }
#endif
    else
    {
        return AK_FALSE;
    }    
    
    value = REG32(uart_reg_base+oUART_CFG2);
    if(value & TX_END)
    {
        return AK_TRUE;
    }
    
    return AK_FALSE;
}

/*******************************************************************************
 * @brief   uart_write_str
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]id
 * @param   [in]STR_ADDR 
 * @return  T_U32
*******************************************************************************/
T_U32 uart_write_str(T_eUART_ID id, T_U8 *str)
{
    T_U32 len  = 0;
    T_U8  *s   = str;


    if (AK_NULL == str)
    {
        return 0;
    }

    while (*s++)
    {
        ++len;
    }

    return uart_write_cpu(id, str, len);
}

/*******************************************************************************
 * @brief   uart_set_fiq_int
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]id
 * @param   [in]STATUS 
 * @return  T_VOID
*******************************************************************************/
T_VOID uart_set_fiq_int(T_eUART_ID id, T_BOOL status)
{
    T_U32 int_bit;

    if (uiUART1 == id)
    {
        int_bit = INT_EN_UART1;
    }
    else
    {
        int_bit = INT_EN_UART2;
    }
    
    if (status)
	{	    
    	FIQ_INT_ENABLE(int_bit);
    	restore_int();
	}
    else
	{
	    store_all_int();
    	FIQ_INT_DISABLE(int_bit);
	}
}

#pragma arm section code


/*******************************************************************************
 * @brief   receive one char from the uart
 * @author  wangguotian
 * @date    2012.12.19
 * @param   [in]id  
 * @return  T_U8
 * @retval  the char received from the uart.
*******************************************************************************/
#pragma arm section code = "_drvfrequent_" 
T_U8 uart_read_chr(T_eUART_ID id)
{
    T_U32               chr;
    T_U32               buf_addr;
    T_U32               buf_len;
    T_U32               buf_id;
    T_U32               uart_reg_base;
    DEVICE_SELECT       rx_l2_select;


    if(uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        rx_l2_select  = ADDR_UART1_RX;
    }
#if (CHIP_SEL_10C > 0)
    else if(uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        rx_l2_select  = ADDR_UART2_RX;
    }
#endif
    else
    {
        return 0;
    }

    REG32(uart_reg_base+oUART_CFG1) |= RX_STA_CLR;            //clear rx status
    REG32(uart_reg_base+oUART_CFG1) |= RX_TIMEOUT_EN;

    while(!(REG32(uart_reg_base+oUART_CFG2)&RX_TIMEROUT))
    {}

    REG32(uart_reg_base+oUART_CFG2) |= RX_TIMEROUT;

    l2_trans_data_cpu((T_U32)&chr, rx_l2_select, 1, 0);
    

    REG32(uart_reg_base+oUART_CFG1) &= ~RX_TIMEOUT_EN;

    return (T_U8)chr;
}
#pragma arm section code


#ifdef __ENABLE_UART_INT__
/*******************************************************************************
 * @brief   uart_reset
 * @author  Liuhuadong
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]baud_rate  
 * @return  T_U8
*******************************************************************************/
#pragma arm section code = "_drvfrequent_" 
T_BOOL uart_reset(T_eUART_ID id, T_eUART_BAUDRATE baud_rate)
{
    T_U32               asic_clk;
    T_U32               uart_reg_base;
    T_eSHARE_PIN_CFG    pin_cfg;


    if (uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        pin_cfg       = ePIN_AS_UART1;
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        pin_cfg       = ePIN_AS_UART2;
    }
#endif
    else
    {
        return AK_FALSE;
    }

    asic_clk = clk_get_asic();
    s_uart_baudrate[id] = baud_rate;

    if (AK_FALSE == sys_share_pin_is_lock(pin_cfg))
    {
        sys_share_pin_lock(pin_cfg);
    }

    REG32(uart_reg_base+oUART_CFG1) = (TX_STA_CLR | RX_STA_CLR | RX_TIMEOUT_EN \
                             | UART_INTERFACE_EN | BAUD_RATE_ADJ_EN \
                             | BAUD_REG(asic_clk, baud_rate));  /* Set baudrate */

    //for uart read data
    REG32(uart_reg_base+oUART_CFG2) |= (TIMEOUT_INT_EN | RX_TH_INT_EN | RX_BUF_FULL_INT_EN);


	Pre_Uart_off = 0;

    return AK_TRUE;
}
#pragma arm section code

#pragma arm section code = "_drvbootcode_" 
/*******************************************************************************
 * @brief   uart_intrrx_data
 * @author  Liuhuadong
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]data
 * @param   [in]len 
 * @return  T_void
*******************************************************************************/
static __inline T_VOID uart_intrrx_data(T_eUART_ID id, T_U8 *data, T_U32 len)
{
    if (s_uart_callback[id] != AK_NULL)
    {
        (*s_uart_callback[id])(data, len);
    }
}

/*******************************************************************************
 * @brief   uart_rx_get_data
 * @author  Liuhuadong
 * @date    2012.12.19
 * @param   [in]id  
 * @param   [in]status
 * @return  T_U32
*******************************************************************************/
static T_U32 uart_rx_get_data(T_eUART_ID id, T_U32 status)
{
    T_U32 reg_value;
    T_U32 ReReadFlag, size = 0, word, pos, off;
    T_U32 SeconddataCfg;
    T_U32 uart_reg_base;


    if (uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
    }
#endif
    else
    {
        return 0;
    }
    /* interrupt handle of rx timeout or  threshold */
    //if((status & RX_TIMEROUT) || (status & RX_TH_INT_STA))
    {
        reg_value = REG32(uart_reg_base+oUART_DATA_CFG);

        if (status & RX_TIMEROUT)
        {
			if(0 == ((reg_value>>13)&0x1F))
			{
            	size = ((reg_value>>23) & 0x03);
				
			}
			else
			{
            	size = ((((reg_value>>13)&0x1F)-1) << 2) + ((reg_value>>23) & 0x03);
				// lixm 2013-7-16, check timeout addr
				
			}
        }
        else
        {
            size = ((reg_value>>13)&0x1F) << 2;
        }
        
    	word = UART_RX_WORD_CNT(reg_value);
		off = (Pre_Uart_off <<2);
		
		if(word > Pre_Uart_off)
		{
			uart_intrrx_data(id, (T_U8 *)(rx_addr + off), size - off);
		}		
		else if(word < Pre_Uart_off)
		{
		    if ((((reg_value>>13)&0x1F) == 0) && (status & RX_TIMEROUT))
		    {
    			uart_intrrx_data(id, (T_U8 *)(rx_addr + off), 124 - off+size);
			}
			else
			{
                uart_intrrx_data(id, (T_U8 *)(rx_addr + off), 128 - off);
    			uart_intrrx_data(id, (T_U8 *)(rx_addr), size);
			}
			
		}
		else if (uiUART1 == id)
		{
            T_U32 addr, buf_len, buf_id;

            get_device_buf_info(ADDR_UART1_RX, &addr, &buf_len, &buf_id);
		    if(l2_get_status(buf_id) == 2)
		    {
		        l2_set_status(buf_id, 0);
		    }
		}
		Pre_Uart_off = word;

		/* clear rx_adr if rx_adr not update */
        //REG32(uart_reg_base+oUART_CFG1) |= RX_ADR_CLR; //clear RX_adr, data will be saved from start
	}
	return 1;
}

//!!!!!!!!中断处理函数是常驻内存的，需要尽量精简!!!!!!!!
/*******************************************************************************
 * @brief   UART read interrupt handler
 * Function uart_init() must be called before call this function
 * @author  ChenWeiwen
 * @date    2008-06-07
 * @param   T_VOID
*******************************************************************************/
T_VOID uart_interrupt_handler(T_eUART_ID id)
{
    T_U32 status, reg_value;
    T_U32 ReReadFlag, size = 0, word, pos;
    T_U32 SeconddataCfg;
    T_U32 uart_reg_base;


    if (uart_tmr_int_flag)
    {
        uart_tmr_int_flag = 0;
    }
	
    if (uiUART1 == id)
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
    }
#if (CHIP_SEL_10C > 0)
    else if (uiUART2 == id)
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
    }
#endif
    else
    {
        return;
    }

    /* get status of uart interrupt bit */
    status = REG32(uart_reg_base+oUART_CFG2);
    /* interrupt handle of rx buffer full */
    if (status & (RX_BUF_FULL | RX_READY))
    {
        REG32(uart_reg_base+oUART_CFG2) = (status | RX_BUF_FULL);   //clear interrupt flag
    }
	
    if (status & RX_ERROR)
    {
        REG32(uart_reg_base+oUART_CFG2) = (status | RX_ERROR);   //clear interrupt flag
    }
    /* 如果接收到数据不是4 的倍数会产生timeout 中断*/
    if (status & RX_TIMEROUT)
    {
        REG32(uart_reg_base+oUART_CFG2) = (status | RX_TIMEROUT);//clear timeout bit
    }

    /* 如果接收到数据是4 的倍数会产生rx_threshold 中断*/
    if (status & RX_TH_INT_STA)
    {
        REG32(uart_reg_base+oUART_CFG2) = (status | RX_TH_INT_STA);//clear TH_INT bit
    }
	uart_rx_get_data(id, status);
    
}

/*******************************************************************************
 * @brief   uart_rx_timer_lookup
 * Function Timer lookup whether uart buf have data. 
 * @author  ChenWeiwen
 * @date    2013-07-19
 * @param   T_VOID
*******************************************************************************/
T_VOID uart_rx_timer_lookup(T_TIMER timer_id, T_U32 delay)
{
    T_U32 uart_reg_base;
    T_U32 uart_port=MAX_UART_NUM, uart_int;

    //only allowed one interrupt UART!
    if (s_uart_callback[uiUART1])
    {
        uart_reg_base = UART1_MODULE_BASE_ADDR;
        uart_int = INT_EN_UART1;
        uart_port = uiUART1;
    }
#if (CHIP_SEL_10C > 0)
    else if (s_uart_callback[uiUART2])
    {
        uart_reg_base = UART2_MODULE_BASE_ADDR;
        uart_int = INT_EN_UART2;
        uart_port = uiUART2;
    }
#endif
    else
    {
        return;
    }

    if (MAX_UART_NUM != uart_port)
    {
        T_U32 datacnt = UART_RX_WORD_CNT(REG32(uart_reg_base+oUART_DATA_CFG));
        
        if ((Pre_Uart_off == datacnt) || (datacnt != uart_rx_cur_cnt))
        {
            uart_tmr_int_flag = 0;
            uart_rx_cur_cnt = datacnt;
            return;
        }
        
        if (2 == ++uart_tmr_int_flag)
        {        	        
            FIQ_INT_DISABLE(uart_int);
            uart_rx_get_data(uart_port, REG32(uart_reg_base+oUART_CFG2));
            FIQ_INT_ENABLE(uart_int);
            uart_tmr_int_flag = 0;
            uart_rx_cur_cnt = 0;
        }
    }
}


T_VOID uart1_interrupt_handler(T_VOID)
{
    uart_interrupt_handler(uiUART1);
}


#if (CHIP_SEL_10C > 0)
T_VOID uart2_interrupt_handler(T_VOID)
{
    
    uart_interrupt_handler(uiUART2);
}
#endif
#pragma arm section code



#endif  //__ENABLE_UART_INT__


