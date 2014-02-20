/**
 * @file arch_spi.h
 * @brief SPI driver header file.
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI.
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-11-17
 * @version 1.0
 */
#ifndef __ARCH_SPI_H__
#define __ARCH_SPI_H__


#include "anyka_types.h"

/**
 * @brief SPI port define
 *    define port name with port number
 */
typedef enum
{
    SPI_ID0 = 0,
    SPI_ID1 = 1,
    SPI_NUM
}T_eSPI_ID;

typedef enum
{
    SPI_MODE0 = 0,///<CPHA = 0, CPOL = 0
    SPI_MODE1,    ///<CPHA = 0, CPOL = 1
    SPI_MODE2,    ///<CPHA = 1, CPOL = 0
    SPI_MODE3,    ///<CPHA = 1, CPOL = 1
    SPI_MODE_NUM
}T_eSPI_MODE;

typedef enum
{
    SPI_SLAVE = 0,
    SPI_MASTER,
    SPI_ROLE_NUM
}T_eSPI_ROLE;

typedef enum
{
    SPI_BUS1 = 0,
    SPI_BUS2,
    SPI_BUS4,
    SPI_BUS_NUM
}T_eSPI_BUS;

typedef T_VOID (*f_spi_int_cb)(T_U32 status);

/**
 * @brief init spi function
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] mode the spi mode, refer to T_eSPI_MODE
 * @param[in] role the spi role, refer to T_eSPI_ROLE
 * @param[in] clk the spi working clock(unit:Hz)
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk);

/**
 * @brief disable spi function
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @return T_VOID
 */
T_VOID spi_close(T_eSPI_ID spi_id);

/**
 * @brief reset SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @return T_VOID
 */
T_VOID spi_reset(T_VOID);

/**
 * @brief master write to spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after write data release CS signal or not
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);

/**
 * @brief master read data from spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after read data, release CS signal or not
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);

/**
 * @brief spi_data_mode - select spi data mode, set GPIO pin #SPI_D2 #SPI_D3 
 * @author LuHeshan
 * @date 2012-12-24
 * @param[in] spi_id: spi id
 * @param[in]  data_mode: 1-2-4wire
 * @return success to return AK_TRUE
 * @version 
 */
T_BOOL spi_data_mode(T_eSPI_ID spi_id, T_eSPI_BUS data_mode);


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
T_BOOL spi_receive_enable(T_eSPI_ID spi_id, T_U32 l2_len, T_U32 count);

/**
 * @brief spi_receive_disable - stop to receive data.
 * @ spi_receive_enable and spi_receive_disable must be called one by one.
 * @author LuHeshan
 * @date 2013-02-07
 * @param spi_id[in]: spi id
 * @return T_VOID
 * @version 
 */
T_VOID spi_receive_disable(T_eSPI_ID spi_id);

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
T_VOID spi_int_enable(T_eSPI_ID spi_id, T_U32 int_mask, f_spi_int_cb pCb);

#endif

