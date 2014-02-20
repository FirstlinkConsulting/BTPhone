/**
 * @file spi.h
 * @brief: frameworks of spi driver.
 *
 * This file describe driver of spi.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang xin
 * @date    2010-11-17
 * @version 1.0
 */

#ifndef __SPI_H__
#define __SPI_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//status register
#define SPI_TXBUF_EMPTY                 (1<<0)
#define SPI_TXBUF_FULL                  (1<<1)
#define SPI_TXBUF_HALFEMPTY             (1<<2)
#define SPI_TXBUF_UNDERRUN              (1<<3)
#define SPI_RXBUF_EMPTY                 (1<<4)
#define SPI_RXBUF_FULL                  (1<<5)
#define SPI_RXBUF_HALFFULL              (1<<6)
#define SPI_RXBUF_OVERRUN               (1<<7)
#define SPI_TRANFINISH                  (1<<8)
#define SPI_MASTER_TRANING              (1<<9)
#define SPI_SLAVER_TIMEOUT              (1<<10)

//enable interrupt
#define SPI_TXBUF_EMPTY_ENA             (1<<0)
#define SPI_TXBUF_FULL_ENA              (1<<1)
#define SPI_TXBUF_HALFEMPTY_ENA         (1<<2)
#define SPI_TXBUF_UNDERRUN_ENA          (1<<3)
#define SPI_RXBUF_EMPTY_ENA             (1<<4)
#define SPI_RXBUF_FULL_ENA              (1<<5)
#define SPI_RXBUF_HALFEMPTY_ENA         (1<<6)
#define SPI_RXBUF_OVERRUN_ENA           (1<<7)
#define SPI_TRANFINISH_ENA              (1<<8)
#define SPI_MASTER_TRANING_ENA          (1<<9)
#define SPI_SLAVER_TIMEOUT_ENA          (1<<10)

//cs and output 0
#define SPI_CTRL_FORCE_CS               (1<<5)
#define SPI_CTRL_ENA_WORK               (1<<6)

#define SP1_CS                          (76)
#define SP1_CLK                         (77)
#define SP1_DOUT                        (78)
#define SP1_DIN                         (79)
                                        
#define SP2_CS                          (80)
#define SP2_CLK                         (81)
#define SP2_DOUT                        (82)
#define SP2_DIN                         (83)

#define SPI_DMA_SIZE_12K                (12 * 1024 )

typedef struct {
    T_BOOL  bOpen;
    T_U8    ucRole;
    T_U8    ucMode;       //phase mode
    T_U8    ucBusWidth;   //bus width for AK1080L
    T_U32   clock;        //clock div
    T_U32   ulBaseAddr;   //base addr
}T_SPI;

T_VOID spi_set_protect(T_U32 spi_id, T_U8 width);

T_VOID spi_set_unprotect(T_U32 spi_id, T_U8 width);

T_BOOL spi_reset_ctl(T_U32 spi_id);

T_BOOL spi_close_ctl(T_U32 spi_id);

#ifdef __cplusplus
}
#endif

#endif
