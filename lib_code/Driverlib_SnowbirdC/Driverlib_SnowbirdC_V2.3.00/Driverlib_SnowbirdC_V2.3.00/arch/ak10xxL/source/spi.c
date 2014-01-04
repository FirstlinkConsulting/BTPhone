/**
 * @file spi.c
 * @brief SPI driver, define SPI APIs.
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-11-17
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "spi.h"
#include "arch_spi.h"
#include "share_pin.h"
#include "clk.h"
#include "drv_api.h"
#include "arch_init.h"
#include "l2.h"
#include "Interrupt.h"
#include "boot.h"
#include "drv_cfg.h"

#define SPI_USE_DMA             1  ///1 is dma mode, 0 is cpu mode 

#pragma arm section zidata = "_drvbootbss_"
static T_SPI s_tSpi[SPI_NUM];
static f_spi_int_cb pSpi_int_cb;
#pragma arm section zidata

//move share pin protection code to spi flash driver/spi camera driver
//keep these two symbols here as micro define, for future use
#pragma arm section code = "_drvbootcode_"
#if (DRV_SUPPORT_SPI_BOOT > 0)    
extern T_U32 g_clk_map;
#endif
T_VOID spi_set_protect(T_U32 spi_id, T_U8 width)
{
#if (DRV_SUPPORT_SPI_BOOT > 0)
    g_clk_map |= CLK_SPI_EN;
#endif
    if (SPI_ID0 == spi_id)
    {
        //sys_module_enable(eVME_SPI1_CLK, AK_TRUE);
        
        if (SPI_BUS4 == width)
            sys_share_pin_lock(ePIN_AS_SPI1_4LINE); 
        else
            sys_share_pin_lock(ePIN_AS_SPI1);
    }
    else
    {
        //sys_module_enable(eVME_SPI2_CLK, AK_TRUE);
        if (SPI_BUS4 == width)
            sys_share_pin_lock(ePIN_AS_SPI2_4LINE);
        else
            sys_share_pin_lock(ePIN_AS_SPI2);
    }
}

T_VOID spi_set_unprotect(T_U32 spi_id, T_U8 width)
{
    if (SPI_ID0 == spi_id)
    {
        //chip defect: adc2/3 need spi1 clock to work, so cannot disable here
        //sys_module_enable(eVME_SPI1_CLK, AK_FALSE);
        if (SPI_BUS4 == width)
            sys_share_pin_unlock(ePIN_AS_SPI1_4LINE); 
        else
            sys_share_pin_unlock(ePIN_AS_SPI1);
    }
    else
    {
        //sys_module_enable(eVME_SPI2_CLK, AK_FALSE);
        if (SPI_BUS4 == width)
            sys_share_pin_unlock(ePIN_AS_SPI2_4LINE);
        else
            sys_share_pin_unlock(ePIN_AS_SPI2);
    }
#if (DRV_SUPPORT_SPI_BOOT > 0)    
    g_clk_map &= ~CLK_SPI_EN;
#endif
}
#pragma arm section code


static void set_tx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL);
    reg_value &= ~(1<<0);
    reg_value |= (1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = reg_value ;
}

#pragma arm section code = "_drvbootcode_"
static void set_rx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL);
    reg_value |= (1<<0);
    reg_value &= ~(1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = reg_value ;
}
#pragma arm section code

static void force_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL);
    reg_value |= SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = reg_value ;
}

static void unforce_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL);
    reg_value &= ~SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = reg_value ;
}

static T_U8 get_spi_clk_div(T_U32 asic_clk, T_U32 spi_clk)
{
    T_U8 clk_div;

    clk_div = (asic_clk/2 - 1) / spi_clk;
	if(clk_div&&(spi_clk<(60*1000*1000)))
		clk_div--;
    //drv_print("asic clk: ", asic_clk, AK_FALSE);
    //drv_print(", spi clk: ", asic_clk/(2*(1+clk_div)), AK_TRUE);

    return clk_div;
}

T_VOID spi_on_change(T_U32 asic_clk)
{
    T_U32 reg;
    T_U8 div;
    T_U32 i;

    for (i=0; i<SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            //calculate clock
            div = get_spi_clk_div(asic_clk, s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) 
            {
                div = 3;
            }
            //sys_module_enable(SPI_ID0 == i ? eVME_SPI1_CLK : eVME_SPI2_CLK, AK_TRUE);
            reg = (1 << 1) | (s_tSpi[i].ucMode << 2) | (s_tSpi[i].ucRole << 4) | SPI_CTRL_ENA_WORK | (div << 8);
            REG32(s_tSpi[i].ulBaseAddr + oSPI_CTRL) = reg;
            //sys_module_enable(SPI_ID0 == i ? eVME_SPI1_CLK : eVME_SPI2_CLK, AK_FALSE);
        }
    }
}


/**
 * @brief Initialize SPI
 *
 * this func must be called before call any other SPI functions
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param mode[in] spi mode selected 
 * @param role[in] master or slave
 * @param clk_div[in] SPI working frequency = ASICCLK/(2*(clk_div+1)
 * @return T_BOOL
 * @retval AK_TRUE: Successfully initialized SPI.
 * @retval AK_FALSE: Initializing SPI failed.
 */
T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk)
{
    T_U32 div;
    T_U32 spi_inf;

    if (spi_id > SPI_ID1 || mode >= SPI_MODE_NUM || role >= SPI_ROLE_NUM)
    {
        drv_print("SPI initialized failed!", 0, AK_TRUE);
        return AK_FALSE;
    }

	sys_module_enable(SPI_ID0 == spi_id ? eVME_SPI1_CLK : eVME_SPI2_CLK, AK_TRUE);
    
    s_tSpi[spi_id].ucRole= role;
    s_tSpi[spi_id].ucMode = mode;
    s_tSpi[spi_id].clock = clk;
    s_tSpi[spi_id].bOpen = AK_TRUE;
    s_tSpi[spi_id].ucBusWidth = SPI_BUS1;

    if(SPI_ID0 == spi_id)
    {
        s_tSpi[spi_id].ulBaseAddr = SPI1_MODULE_BASE_ADDR;
        sys_module_reset(eVME_SPI1_CLK);
        spi_inf = ADDR_SPI1_RX;
    }
    else
    {
        s_tSpi[spi_id].ulBaseAddr = SPI2_MODULE_BASE_ADDR;
        sys_module_reset(eVME_SPI2_CLK);
        spi_inf = ADDR_SPI2_RX;
    }
    
    if (1 == SPI_USE_DMA)
    {
        if (!l2_init_device_buf(spi_inf))
        {
            drv_print("spi m R l2 fail\n", 0,  AK_TRUE);
            return AK_FALSE;
        }
    }

    div = get_spi_clk_div(clk_get_asic(), clk);
    if((SPI_SLAVE == role) && (div < 3)) div = 3;

    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = (1<<1) | (mode<<2) | (role<<4) | SPI_CTRL_ENA_WORK | (div<<8);

    drv_print("SPI initialized ok!", 0, AK_TRUE);

    return AK_TRUE;
}

/**
 * @brief close SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @return T_VOID
 */
T_VOID spi_close(T_eSPI_ID spi_id)
{
    T_U32 i = 0;
    T_U32 spi_inf;
    
    
    if (spi_id > SPI_ID1)
    {
        drv_print("spi_id is invalid, id = ", spi_id, AK_TRUE);
        return;
    }

    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi is not open, id= ", spi_id, AK_TRUE);
        return;
    }

    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) &= ~(1<<6);

    s_tSpi[spi_id].bOpen = AK_FALSE;

    //check whether all spi is closed
    //sysctl_clock(~CLOCK_SPI_ENABLE);  
    if(SPI_ID0 == spi_id)
    {
        sys_module_enable(eVME_SPI1_CLK, AK_FALSE);
        spi_inf = ADDR_SPI1_RX;
    }
    else
    {
        sys_module_enable(eVME_SPI2_CLK, AK_FALSE);
        spi_inf = ADDR_SPI2_RX;
    }

    if (1 == SPI_USE_DMA)
    {
        if (!l2_release_device_buf(spi_inf))
        {
            drv_print("spi free l2 fail", 0, AK_TRUE);
        }
    }
}

/**
 * @brief reset SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @return T_VOID
 */
T_VOID spi_reset(T_VOID)
{
    T_U32 i = 0;
    T_U32 reg;
    T_U8 div;
    
    for (i=0; i<SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            div = get_spi_clk_div(clk_get_asic(), s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) 
            {
                div = 3;
            }
            
            reg = (1<<1) | (s_tSpi[i].ucMode << 2) | (s_tSpi[i].ucRole << 4) | SPI_CTRL_ENA_WORK | (div << 8);
            REG32(s_tSpi[i].ulBaseAddr + oSPI_CTRL) = reg;           
        }
    }
}

/**
 * @brief spi master write
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi write successfully.
 * @retval AK_FALSE: spi write failed.
 */
T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 status;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi_master_write(): SPI not initialized!", 0, AK_TRUE);
        return AK_FALSE;
    }

    //prepare spi
    force_cs(spi_id);
    set_tx(spi_id);

    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_TX_EXBUF) = 0;
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

    for(i = 0; i < count;)
    {
        temp = *(volatile T_U32 *)(T_VOID*)(buf + i);
        i += 4;

        while(!(REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_TXBUF_HALFEMPTY));

        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_TX_INBUF) = temp;
    }

    //wait finish status
    while((REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_TRANFINISH) != SPI_TRANFINISH );

    //pull up CS
    if (bReleaseCS)
    {
        for (i = 0; i < 30; i++);
        unforce_cs(spi_id);
    }

    return ret;
}

/**
 * @brief spi master read
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi read successfully.
 * @retval AK_FALSE: spi read failed.
 */
T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi_master_read(): SPI not initialized!\n", 0,  AK_TRUE);
        return AK_FALSE;
    }

    //prepare spi read
    force_cs(spi_id);
    set_rx(spi_id);

    if (SPI_USE_DMA && (count >= 256) && ((count & (64-1)) == 0) && (((T_U32)buf & 3) == 0))
    {
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = ((1 << 0) | (1 << 16));
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

        if (!l2_trans_data_cpu((T_U32)buf, ADDR_SPI1_RX, count, 0))
        {
            drv_print("dma trans fail\n", 0,  AK_TRUE);
            ret = AK_FALSE;
        }
    }
    else
    {
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

        for(i = 0; i < count;)
        {
            if((i + 4) > count)
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_TRANFINISH));
            }
            else
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_RXBUF_HALFFULL));
            }

            temp = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_INBUF);

            *(volatile T_U32 *)(T_VOID*)(buf + i) = temp;
            i += 4;
        }
    }

    //pull up CS
    if (bReleaseCS)
    {
        for(i = 0; i < 30; i++);
        unforce_cs(spi_id);
    }

    return ret;
}

/**
 * @brief spi_data_mode - select spi data mode, set GPIO pin #SPI_D2 #SPI_D3 
 * @author LuHeshan
 * @date 2012-12-24
 * @param spi_id: spi id
 * @param data_mode: 1-2-4wire
 * @return T_BOOL
 * @version 
 */
T_BOOL spi_data_mode(T_eSPI_ID spi_id, T_eSPI_BUS data_mode)
{
    T_U32 reg_value;

    if ((!s_tSpi[spi_id].bOpen) || (SPI_BUS_NUM <= data_mode))
    {
        drv_print("spi_data_mode(): spi no open or param fail!\n", 0,  AK_TRUE);
        return AK_FALSE;
    }

    s_tSpi[spi_id].ucBusWidth = data_mode;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL);
    reg_value &= ~(0x3 << 16);
    reg_value |= (data_mode << 16);
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = reg_value;

    return AK_TRUE;
}

//以下几个函数只在spi camera的功能打开时才调用到
#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)
#pragma arm section code = "_drvbootcode_"
/**
 * @brief spi_receive_enable - start to receive data to L2 buffer when call this function.
 * @ spi_receive_enable and spi_receive_disable must be called one by one.
 * @author LuHeshan
 * @date 2013-02-07
 * @param spi_id[in]: spi id
 * @param L2_len[in]: l2 buffer len
 * @param count[in] the read data count(bytes),then count number must be n*64.
 * @return T_BOOL
 * @version 
 */
T_BOOL spi_receive_enable(T_eSPI_ID spi_id, T_U32 l2_len, T_U32 count)
{       
    //prepare spi read
    set_rx(spi_id);

    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = ((1 << 0) | (1 << 16) 
                                                        | ((l2_len >> 2) << 17));
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

    return AK_TRUE;
}

/**
 * @brief spi_receive_disable - stop to receive data.
 * @ spi_receive_enable and spi_receive_disable must be called one by one.
 * @author LuHeshan
 * @date 2013-02-07
 * @param spi_id[in]: spi id
 * @return T_VOID
 * @version 
 */
T_VOID spi_receive_disable(T_eSPI_ID spi_id)
{
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = 0;
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = 0;
}
#pragma arm section code


/**
 * @brief spi_int_enable
 * @author LuHeshan
 * @date 2013-04-01
 * @param spi_id[in]: spi id
 * @param int_mask[in]: interrupt bit map
 * @param pCb[in] when the interrupt bit is set,will call this function.
 * @return T_VOID
 * @version 
 */
T_VOID spi_int_enable(T_eSPI_ID spi_id, T_U32 int_mask, f_spi_int_cb pCb)
{
    pSpi_int_cb = pCb;
        
    if (AK_NULL != pCb)
    {        
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_INT_EN) |= int_mask;
        
        if (SPI_ID0 == spi_id)
        {
            INT_ENABLE(INT_EN_SPI1);
        }
        else if (SPI_ID1 == spi_id)
        {
            FIQ_INT_ENABLE(INT_EN_SPI2);
        }
    }
    else
    {
        if (SPI_ID0 == spi_id)
        {
            INT_DISABLE(INT_EN_SPI1);
        }
        else if (SPI_ID1 == spi_id)
        {
            FIQ_INT_DISABLE(INT_EN_SPI2);
        }
                
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_INT_EN) &= ~(int_mask);
    }
}

T_BOOL spi_reset_ctl(T_U32 spi_id)
{
    T_U32 div;
    
    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi is not open, id= ", spi_id, AK_TRUE);
        return AK_FALSE;
    }
        
    sys_module_enable(SPI_ID0 == spi_id ? eVME_SPI1_CLK : eVME_SPI2_CLK, AK_TRUE);

    sys_module_reset(SPI_ID0 == spi_id ? eVME_SPI1_CLK : eVME_SPI2_CLK);

    div = get_spi_clk_div(clk_get_asic(), s_tSpi[spi_id].clock);
    if((SPI_SLAVE == s_tSpi[spi_id].ucRole) && (div < 3)) 
        div = 3;
    
    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) = (1<<1) 
                                                 | (s_tSpi[spi_id].ucMode<<2) 
                                                 | (s_tSpi[spi_id].ucRole<<4) 
                                                 | SPI_CTRL_ENA_WORK 
                                                 | (div<<8);
    return AK_TRUE;
}

T_BOOL spi_close_ctl(T_U32 spi_id)
{
    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi is not open, id= ", spi_id, AK_TRUE);
        return AK_FALSE;
    }

    REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_CTRL) &= ~(1<<6);

    sys_module_enable(SPI_ID0 == spi_id ? eVME_SPI1_CLK : eVME_SPI2_CLK, AK_FALSE);

    return AK_TRUE;
}

#pragma arm section code = "_drvbootcode_" 
#ifdef    __ENABLE_SPI1_INT__
T_VOID spictrl1_interrupt_handler(T_VOID)
{
    T_U32 status;
    
    if (AK_NULL != pSpi_int_cb)
    {
        status = REG32(SPI1_MODULE_BASE_ADDR + oSPI_STA);
        
        pSpi_int_cb(status);
    }
}
#endif

#ifdef    __ENABLE_SPI2_INT__
T_VOID spictrl2_interrupt_handler(T_VOID)
{
    T_U32 status;
    
    if (AK_NULL != pSpi_int_cb)
    {
        status = REG32(SPI2_MODULE_BASE_ADDR + oSPI_STA);
        
        pSpi_int_cb(status);
    }
}
#endif
#pragma arm section code

#endif //#if (DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)

#if 0
/**
 * @brief spi slave read
 *
 * this func must be called in spi  slave mode
 * @author Lu_Heshan
 * @date 2013-02-01
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @return T_BOOL
 * @retval AK_TRUE:  spi read successfully.
 * @retval AK_FALSE: spi read failed.
 */
T_BOOL spi_slave_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count)
{
    T_U32 spi_inf;
    T_U32 i,temp;
    T_BOOL ret = AK_TRUE;

    if (!s_tSpi[spi_id].bOpen)
    {
        drv_print("spi_slave_read(): SPI not initialized!\n", 0,  AK_TRUE);
        return AK_FALSE;
    }

    if (SPI_ID0 == spi_id)
    {
        spi_inf = ADDR_SPI1_RX;
    }
    else
    {
        spi_inf = ADDR_SPI2_RX;
    }

    set_protect(spi_id);

    //prepare spi read
    set_rx(spi_id);

    if (SPI_USE_DMA && (count >= 256) && ((count & (64-1)) == 0) && (((T_U32)buf & 3) == 0))
    {        
        //initial slaver time out max time cycle
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_TO) = 0xffff;
        
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = ((1 << 0) | (1 << 16));
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

        if (!l2_trans_data_cpu((T_U32)buf, spi_inf, count))
        {
            drv_print("dma trans fail\n", 0,  AK_TRUE);
            ret = AK_FALSE;
        }
    }
    else
    {
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_DATA_CNT) = count;

        for(i = 0; i < count;)
        {
            if((i + 4) > count)
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_TRANFINISH));
            }
            else
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_STA) & SPI_RXBUF_HALFFULL));
            }

            temp = REG32(s_tSpi[spi_id].ulBaseAddr + oSPI_RX_INBUF);

            *(volatile T_U32 *)(T_VOID*)(buf + i) = temp;
            i += 4;
        }
    }

    set_unprotect(spi_id);
    
    return ret;

}
#endif

