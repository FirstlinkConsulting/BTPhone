/**
 * @filename usb_host_drv.c
 * @brief AK880X frameworks of usb driver.
 *
 * This file describe udriver of usb in host mode.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-20
 * @version 1.0
 * @ref
 */

#include "usb_host_drv.h"
#include "hal_usb_h_std.h"
#include "hal_usb_std.h"
#include "interrupt.h"
#include "l2.h"
#include "clk.h"
#include "drv_api.h"
#include "usb.h"
#include "arch_init.h"
#include "drv_cfg.h"

#if DRV_SUPPORT_UHOST > 0


//********************************************************************
/****** DEVCTL bit MASK  *******/
#define M_DEVCTL_SESSION          0x01
#define M_DEVCTL_HR               0x02
#define M_DEVCTL_HM               0X04
#define M_DEVCTL_PDCON            0x08
#define M_DEVCTL_PUCON            0x10
#define M_DEVCTL_LSDEV            0x20
#define M_DEVCTL_FSDEV            0x40
#define M_DEVCTL_BDEVICE          0x80

#define USB_PKTYPE_SETUP          0x1
#define USB_PKTYPE_EP0IN          0x2
#define USB_PKTYPE_EP0OUT         0x3
#define USB_PKTYPE_EP0IN_STAT     0x4
#define USB_PKTYPE_EP0OUT_STAT    0x5
#define USB_PKTYPE_EPIN           0x6
#define USB_PKTYPE_EPOUT          0x7
#define USB_PKTYPE_EPIN_STAT      0x8
#define USB_PKTYPE_EPOUT_STAT     0x9

#define USB_TXCSR1_H_NAKTIMEOUT   0x80

#define UHOST_IN_INDEX            EP3_INDEX
#define UHOST_OUT_INDEX           EP2_INDEX

#if UHOST_USE_INTR > 0
#define UHOST_INTR_MODE
#endif

//********************************************************************

typedef enum
{
    CTRL_STAGE_IDLE = 0,    ///< idle stage
    CTRL_STAGE_SETUP,       ///< setup stage
    CTRL_STAGE_DATA_IN,     ///< data in stage
    CTRL_STAGE_DATA_OUT,    ///< data out stage
    CTRL_STAGE_STATUS       ///< status stage
}
E_CTRL_TRANS_STAGE;

typedef enum
{
    UHOST_TRANS_IDLE = 0,
    UHOST_TRANS_BULK_IN,
    UHOST_TRANS_BULK_OUT
}
E_USB_HOST_TRANS;

typedef struct tagCONTROL_TRANS
{
    E_CTRL_TRANS_STAGE stage;   ///< stage
    T_UsbDevReq dev_req;        ///< request
    T_U8 *buffer;               ///< buffer
    T_U32 buf_len;              ///< buffer length
    T_U32 data_len;             ///< data length
    T_U32 trans_len;            ///< data length
    T_fUHOST_TRANS_CALLBACK cbk_func;
}
T_CONTROL_TRANS;

typedef struct tagBULK_TRANS
{
    T_U8 stage;
    T_U8 *buffer;    
    T_U8  TxL2Bufid; 
    T_U8  RxL2Bufid;
    T_U32 buf_len;
    T_U32 data_len;
    T_U32 trans_len;
    T_BOOL dma_start;
    T_fUHOST_TRANS_CALLBACK cbk_func;
}
T_USB_HOST_TRANS;

typedef struct
{
    T_fUHOST_COMMON_INTR_CALLBACK fcbk_connect;
    T_fUHOST_COMMON_INTR_CALLBACK fcbk_disconnect;
}
T_USB_HOST_COMMON_INTR_CALLBACK;

//to record the device bulk out/in ep index 
T_U8 g_UsbBulkinIndex;
T_U8 g_UsbBulkoutIndex;
//********************************************************************
static T_VOID usb_print_host_info(T_VOID);
// T_BOOL usb_host_intr_handler(T_VOID);
static T_VOID usb_host_read_int_reg(T_U8* usb_int, T_U32* usb_dma_int_tem);
static T_VOID usb_host_read_ep_int_reg(T_U8* usb_ep_int_tx, T_U8* usb_ep_int_rx);
static T_U32 usb_host_data_in(T_U8 EP_index, T_U8 *data);
static T_U32 usb_host_data_out(T_U8 EP_index, T_U8 *data, T_U32 count);

static T_U8 usb_bulk_send_id;
static T_U8 usb_bulk_receive_id;

T_U32 usb_mode = USB_MODE_20;

static volatile T_U32 m_max_ep0_size = 64;

static volatile T_CONTROL_TRANS m_uhost_ctrl = {0};
static volatile T_USB_HOST_TRANS m_uhost_trans = {0};

#ifdef UHOST_INTR_MODE
#pragma arm section zidata = "_drvbootbss_"
#endif
static T_USB_HOST_COMMON_INTR_CALLBACK m_uhost_cbk = {0};
volatile T_BOOL state_connect = AK_FALSE;
#ifdef UHOST_INTR_MODE
#pragma arm section zidata
#endif

//********************************************************************
#if CHIP_SEL_10C > 0
T_VOID usb_switch(T_BOOL bswitch)
{
    T_U32 tmp;
    if(AK_TRUE == bswitch)
    {
        tmp = REG32(USB_REG_CTRL);
        tmp |= (0x01<<27);
        REG32(USB_REG_CTRL) = tmp;
    }
    else
    {
        tmp = REG32(USB_REG_CTRL);
        tmp &= ~(0x1<<27); 
        REG32(USB_REG_CTRL) = tmp;
    }

}
#endif
T_VOID host_connect_intr_mute(T_BOOL bMute)
{
    if(bMute)
    {
        REG8(USB_REG_INTRUSBE) &= ~(USB_INTR_CONNECT);
    }
    else
    {
        REG8(USB_REG_INTRUSBE) |= USB_INTR_CONNECT;
    }
}

/**
 * @brief   enable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  mode [in] T_U32  full speed or high speed
 * @return  T_VOID
 */
T_VOID usb_host_device_enable(T_U32 mode)
{
    T_U8 usb_ep_int_tx,usb_ep_int_rx;
    T_U8 usb_int;
    T_U32 usb_dma_int_tem,tmp;
    extern  T_U8 g_usbtype;

    //enable usb clock
    sys_module_enable(eVME_USB_CLK, AK_TRUE);
    sys_module_reset(eVME_USB_CLK);

    //clear usb interrupt status
    usb_host_read_ep_int_reg(&usb_ep_int_tx, &usb_ep_int_rx);
    usb_host_read_int_reg(&usb_int, &usb_dma_int_tem);

    #if CHIP_SEL_10C > 0
    tmp = REG32(USB_REG_CTRL);
    tmp &= ~(0x01<<27);
    tmp &= ~(0x7<<24);
    tmp |= (0x1<<18)|(0x7<<24)|(0x01<<27);
    REG32(USB_REG_CTRL) = tmp;
    REG32(USB_REG_CTRL) &=~(0x7<<24);
    #endif

    //enable the otg controller and reset the otg phy
    REG32(REG_MUL_FUNC_CTRL) |= (PHY_RST | PHY_SUS);
    REG32(REG_MUL_FUNC_CTRL) &= ~(PHY_RST | PHY_LDO);

    //start session
    REG8(USB_REG_DEVCTL) |= 0x1;
    drv_print("USB_REG_DEVCTL0 = ", REG8(USB_REG_DEVCTL), AK_TRUE);

    //set host phy
    REG8(USB_REG_CFG) = 0x5f;

    //select mode
    if(mode == USB_MODE_11) //full speed
    {
        REG8(USB_REG_POWER) = 0x0;
        usb_mode = USB_MODE_11;
    }
    else                    //high speed
    {
        REG8(USB_REG_POWER) = 0x21;
        usb_mode = USB_MODE_20;
    }

    //disable the sof interrupt
    REG8(USB_REG_INTRUSBE) = 0xFF & (~(USB_INTR_SOF|USB_INTR_CONNECT));

    //???
    delay_us(200000);

    //close test mode
    REG8(USB_REG_TESEMODE) = 0x0;

    usb_print_host_info();

    REG8(USB_REG_INDEX) = USB_EP0_INDEX;

    //set the nak count to max
    REG8(USB_REG_NAKLIMIT0) = 16;

    //disable the ping protocol
    REG8(USB_REG_CSR02) = 8;


    REG8(USB_REG_INDEX) = USB_EP3_INDEX;
    REG8(USB_REG_RXCSR1)=0;
    REG8(USB_REG_INDEX) = USB_EP2_INDEX;
    REG8(USB_REG_TXCSR1)=0;
    REG8(USB_REG_INDEX) = USB_EP0_INDEX;
    //set usb type to host
    g_usbtype = 1;

    INT_ENABLE(INT_EN_USBMCU);

    #ifndef UHOST_INTR_MODE
    REG8(USB_REG_INTRRX1E)=0;
    REG8(USB_REG_INTRTX1E)=0;
    #endif
    
    //init global variable
    m_uhost_ctrl.stage = CTRL_STAGE_IDLE;
    m_uhost_trans.stage = UHOST_TRANS_IDLE;

    m_max_ep0_size = 64;
}

/**
 * @brief   switch usb host speed.
 *
 * @author  
 * @date    2012-10-30
 * @param  mode [in] T_U32  full speed or high speed
 * @return  T_VOID
 */
T_VOID usb_host_speed_sw(T_U32 mode)
{
    //select mode
    if(mode == USB_MODE_11) //full speed
    {
        REG8(USB_REG_POWER) = 0x0;
        usb_mode = USB_MODE_11;
    }
    else                    //high speed
    {
        REG8(USB_REG_POWER) = 0x21;
        usb_mode = USB_MODE_20;
    }
}
//********************************************************************
/**
 * @brief   disable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_device_disable(T_VOID)
{
    T_U32 tmp;

    #ifdef UHOST_INTR_MODE
    //disable usb irq
    INT_DISABLE(INT_EN_USBMCU);
    #endif

    REG8(USB_REG_POWER) = 0;

    //disable usb transcieve
    REG32(REG_MUL_FUNC_CTRL) &= ~PHY_SUS;

    //close usb clock
    sys_module_enable(eVME_USB_CLK, AK_FALSE);
    sys_module_reset(eVME_USB_CLK);

    memset(&m_uhost_ctrl, 0, sizeof(T_CONTROL_TRANS));
    memset(&m_uhost_trans, 0, sizeof(T_USB_HOST_TRANS));
    memset(&m_uhost_cbk, 0, sizeof(T_USB_HOST_COMMON_INTR_CALLBACK));
}

/**
 * @brief   reset data toggle.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_clear_data_toggle(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;
    if (UHOST_IN_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
    }
    else if (UHOST_OUT_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;
    }
    else
    {
        return;
    }
}
T_VOID usb_host_flush_fifo(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;
  
    if (UHOST_IN_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_FLUSHFIFO;
    }
    else if (UHOST_OUT_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_FLUSHFIFO|USB_TXCSR1_TXPKTRDY;
    }
    else
    {
        return;
    }
    m_uhost_trans.stage = UHOST_TRANS_IDLE;
}

/**
 * @brief   set callback func for common interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param intr_type interrupt type
 * @param callback callback function
 * @return  T_VOID
 */
T_VOID usb_host_set_common_intr_callback(E_USB_HOST_COMMON_INTR intr_type, T_fUHOST_COMMON_INTR_CALLBACK callback)
{
    if(USB_HOST_CONNECT == intr_type)
    {
        m_uhost_cbk.fcbk_connect = callback;
    }
    else if(USB_HOST_DISCONNECT == intr_type)
    {
        m_uhost_cbk.fcbk_disconnect = callback;
    }
}

/**
 * @brief   set callback func for transfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param ctrl_cbk callback function for control transfer
 * @param trans_cbk callback function for other transfer
 * @return  T_VOID
 */
T_VOID usb_host_set_trans_callback(T_fUHOST_TRANS_CALLBACK ctrl_cbk, T_fUHOST_TRANS_CALLBACK trans_cbk)
{
    m_uhost_ctrl.cbk_func = ctrl_cbk;
    m_uhost_trans.cbk_func = trans_cbk;
}

//********************************************************************
static T_VOID usb_print_host_info(T_VOID)
{
    T_U8  devctl_v;

    devctl_v = REG8(USB_REG_DEVCTL);
    
    drv_print("USB_REG_DEVCTL:", devctl_v,1);
    if(0 != (devctl_v & M_DEVCTL_BDEVICE))
    {
        drv_print("operate as B device!", 0, AK_TRUE);
    }
    else
    {
        drv_print("operate as A device!", 0, AK_TRUE);
    }
}
#pragma arm section code = "_usb_host_"

static T_VOID usb_host_request_data(T_U8 EP_index)
{
    T_U32 pkt_num,byte_num,usb_bulk_in_maxsize;
  
    REG8(USB_REG_INDEX) = EP_index; 
    usb_bulk_in_maxsize = REG16(USB_REG_RXMAXP1);
    #if 0
    //usb dma
    if (m_uhost_trans.data_len >= usb_bulk_in_maxsize && m_uhost_trans.trans_len == 0)
    {
        byte_num = m_uhost_trans.data_len;
        pkt_num = byte_num / usb_bulk_in_maxsize;
        byte_num -= (byte_num % usb_bulk_in_maxsize);

        REG8(USB_REG_RXCSR2) |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_AUTOREQ | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);
        REG16(USB_REG_REQPKTCNT3) = pkt_num;
        
        m_uhost_trans.dma_start = AK_TRUE;
    }
    #endif
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_H_REQPKT;
}

static T_VOID usb_host_irq_ctrl(T_BOOL enable)
{
    #ifdef UHOST_INTR_MODE
    if(enable)
    {
        INT_ENABLE(INT_EN_USBMCU|INT_EN_USBDMA);
    }
    else
    {
        INT_DISABLE(INT_EN_USBMCU|INT_EN_USBDMA);
    }
    #endif
}
#pragma arm section code

T_VOID usb_host_set_max_ep0_size(T_U32 size)
{
    if(size > 64)
        m_max_ep0_size = 64;
    else 
        m_max_ep0_size = size;
}

//********************************************************************
#pragma arm section code = "_usb_host_"

static T_U32 usb_host_data_out(T_U8 EP_index, T_U8 *data, T_U32 count)
{
    T_U32 i, j;
    T_U32 send_count= 0;
    T_U32 usb_bulk_out_maxsize;
    T_U32 *buf=AK_NULL;
    //check EP_index
    if(EP0_INDEX == EP_index || EP1_INDEX == EP_index)
    {
        drv_print("usb_host_data_out: error ep number:", EP_index, AK_TRUE);
        return 0;
    }
    REG8(USB_REG_INDEX) = EP_index;
    send_count = count;
    usb_bulk_out_maxsize = REG16(USB_REG_TXMAXP0);
    buf = (T_U32 *)data;
    
    if ((512 == send_count)&&((T_U32)buf&0x3) == 0)
    {
        for (i = 0; i < send_count/4; i++)
        {
            REG32(USB_FIFO_EP0 + (EP_index << 2)) = buf[i];
        }
    }
    else
    {
        for (i = 0; i < send_count; i++)
        {
            REG8(USB_FIFO_EP0 + (EP_index << 2)) = *(data + i);
        }
    }
    REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;

#if 0
    if (count >= usb_bulk_out_maxsize)
    {
        send_count -= (count % usb_bulk_out_maxsize);
    }

    for (i = 0; i < send_count; i++)
    {
        REG8(USB_FIFO_EP0 + (EP_index << 2)) = *(data + i);
    }
    REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;
#endif

    return send_count;
}

//********************************************************************
static T_U32 usb_host_data_in(T_U8 EP_index, T_U8 *data)
{
    T_U32 i,fifo_count = 0;
    T_U32 *buf = (T_U32 *)data;


    fifo_count = REG16(USB_REG_RXCOUNT1);
    //unload data from usb fifo
    if ((512 == fifo_count)&&((T_U32)data&0x3) == 0)
    {
        for (i = 0; i < fifo_count/4; i++)
        {
            buf[i] = REG32(USB_FIFO_EP0 + (EP_index << 2));
        }
    }
    else
    {
        for (i = 0; i < fifo_count; i++)
        {
            *(data+i) = REG8(USB_FIFO_EP0 + (EP_index << 2));
        }
    }
    //clear RXPKTRDY
    REG8(USB_REG_RXCSR1) &= (~USB_RXCSR1_RXPKTRDY);
    //drv_print("rx data finish\n",0,1);

    return fifo_count;
}
#ifndef UHOST_INTR_MODE
#pragma arm section code = "_drvbootcode_"
#endif
//********************************************************************
static T_VOID usb_host_read_int_reg(T_U8* usb_int, T_U32* usb_dma_int_tem)
{
    *usb_int = REG8(USB_REG_INTRUSB);
    //*usb_dma_int_tem = REG32(USB_DMA_INTR);
}
#ifndef UHOST_INTR_MODE
#pragma arm section code
#endif


//********************************************************************
static T_VOID usb_host_read_ep_int_reg(T_U8* usb_ep_int_tx, T_U8* usb_ep_int_rx)
{
    *usb_ep_int_tx = REG8(USB_REG_INTRTX1);
    *usb_ep_int_rx = REG8(USB_REG_INTRRX1);
}
#pragma arm section code 

//********************************************************************
/**
 * @brief   set faddr to the new address send to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param address  [in]  usb device address.
 * @return  T_VOID
 */
T_VOID usb_host_set_address(T_U8 address)
{
    REG8(USB_REG_FADDR) = address;
}


/**
 * @brief   config usb host endpoint through device ep descriptor.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  ep [in]  ep description.
 * @return  T_VOID
 */
T_VOID usb_host_set_ep(T_USB_ENDPOINT_DESCRIPTOR ep)
{
    T_U8 ep_num; 

    ep_num = ep.bEndpointAddress & 0xF;
    if (ENDPOINT_TYPE_BULK == ep.bmAttributes)
    {
        if(ep.bEndpointAddress & ENDPOINT_DIR_IN)
        {
            g_UsbBulkinIndex = ep_num;
            REG8(USB_REG_INDEX) = USB_HOST_IN_INDEX;
            //set ep type, ep num, max packet size
            REG8(USB_REG_RXTYPE) = ((ep.bmAttributes << 4) | ep_num);
            REG16(USB_REG_RXMAXP1) = ep.wMaxPacketSize;

            //clear toggle
            REG8(USB_REG_RXCSR1) = USB_RXCSR1_CLRDATATOG;

            //open interrupt
            #ifdef UHOST_INTR_MODE
            REG8(USB_REG_INTRRX1E) |= (1 << USB_HOST_IN_INDEX);
            #endif
        }
        else
        {
            g_UsbBulkoutIndex = ep_num;
            REG8(USB_REG_INDEX) = USB_HOST_OUT_INDEX;
            //set ep type, ep num, max packet size
            REG8(USB_REG_TXTYPE) = ((ep.bmAttributes << 4) | ep_num);
            REG16(USB_REG_TXMAXP0) = ep.wMaxPacketSize;

            //clear toggle, set to tx mode
            REG8(USB_REG_TXCSR1) = USB_TXCSR1_CLRDATATOG;
            REG8(USB_REG_TXCSR2) = USB_TXCSR2_MODE;
            
            //open interrupt
            #ifdef UHOST_INTR_MODE
            REG8(USB_REG_INTRTX1E) |= (1 << USB_HOST_OUT_INDEX);
            #endif
        }
    }
}


/**
 * @brief   open or close sof interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  enable [in]  open sof interrupt or not.
 * @return  T_VOID
 */
T_VOID usb_host_sof_intr(T_BOOL enable)
{
    if(enable)
        REG8(USB_REG_INTRUSBE) |= USB_INTR_SOF_ENA;
    else
        REG8(USB_REG_INTRUSBE) &= ~USB_INTR_SOF_ENA;
}

//********************************************************************
/**
 * @brief   send reset signal to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_reset(T_VOID)
{
    REG8(USB_REG_POWER) |= USB_POWER_RESET;
    delay_us(20000);
    REG8(USB_REG_POWER) &= (~USB_POWER_RESET);
}


/**
   @brief   sent suspend signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_suspend(T_VOID)
{
    REG8(USB_REG_POWER) |= (USB_POWER_SUSPENDM|USB_POWER_ENSUSPEND);
}

/**
   @brief   sent resume signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_resume(T_VOID)
{
    REG8(USB_REG_POWER) &= ~(USB_POWER_SUSPENDM|USB_POWER_ENSUSPEND);
    REG8(USB_REG_POWER) |= USB_POWER_RESUME;
    delay_us(20000);
    REG8(USB_REG_POWER) &= (~USB_POWER_RESUME);
}

static T_U32 usb_host_ctrl_in(T_U8 *data)
{
    T_U32 fifo_count;
    T_U32 i, j;
    T_U32 tmp_val;

    //read data count
    fifo_count = REG8(USB_REG_COUNT0);
    if(0 == fifo_count)
        return 0;

    //receive data from l2
    for (i = 0; i < fifo_count; i++)
    {
        *(data + i) = REG8(USB_FIFO_EP0);
    }
    //clear RXPKTRDY
    REG8(USB_REG_CSR0) &= (~USB_CSR0_RXPKTRDY);
    
    return fifo_count;
}

//send one packet
static T_U32 usb_host_ctrl_out(T_U8 *data, T_U8 len, T_BOOL bSetup)
{
    T_U32 i;
    T_U32 count;

    //set ep index
    REG8(USB_REG_INDEX) = EP0_INDEX;
    
    //clear csr
    REG8(USB_REG_CSR0) = 0;

    count = len;
    if(count == 0)          //zero packet
    {
        REG32(USB_EP0_TX_COUNT) = count;
        REG8(USB_REG_CSR0) = USB_CSR0_TXPKTRDY;
        return 0;
    }
    else if(count > m_max_ep0_size)
    {
        count = m_max_ep0_size;
    }
    //load data to ep0 fifo
    for( i = 0; i < count; i++ )
    {
        REG8( USB_FIFO_EP0 ) = data[i];
    }   
    //SET THE TX COUNT and START PRE READ
    REG32(USB_EP0_TX_COUNT) = count;

    if(bSetup)
        REG8(USB_REG_CSR0) |= USB_CSR0_H_SETUPPKT | USB_CSR0_TXPKTRDY;
    else
        REG8(USB_REG_CSR0) |= USB_CSR0_TXPKTRDY;

    return count;
}

/**
   @brief   start control tranfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param dev_req [in] device request
 * @param data [in/out] data buffer
 * @param len [in] buffer length
 * @return  T_BOOL
 */
T_BOOL usb_host_ctrl_tranfer(T_UsbDevReq dev_req, T_U8 *data, T_U32 len)
{
    //check if current stage is idle or not
    if(m_uhost_ctrl.stage != CTRL_STAGE_IDLE)
    {
        return AK_FALSE;
    }

    //save param into m_uhost_ctrl
    memcpy(&m_uhost_ctrl.dev_req, &dev_req, sizeof(dev_req));

    m_uhost_ctrl.buffer = data;
    m_uhost_ctrl.buf_len = len;
    m_uhost_ctrl.data_len = len;
    m_uhost_ctrl.trans_len = 0;

    //send setup packet
    m_uhost_ctrl.stage = CTRL_STAGE_SETUP;
    usb_host_ctrl_out((T_U8 *)&m_uhost_ctrl.dev_req, sizeof(T_UsbDevReq), AK_TRUE);

    return AK_TRUE;
}

/**
   * @brief   start bulk in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  EP_index [in]  usb end point.
   * @param  data [out]  usb data buffer.
   * @param  len [in]  length
   * @return  T_U32 acctual read bytes
 */
 #pragma arm section code = "_usb_host_"
T_BOOL usb_host_bulk_in(T_U8 *data, T_U32 len)
{
    //return if not in idle stage
    if(m_uhost_trans.stage != UHOST_TRANS_IDLE)
    {
        return AK_FALSE;
    }

    usb_host_irq_ctrl(AK_FALSE);

    //set global variable
    m_uhost_trans.buffer = data;
    m_uhost_trans.data_len = len;
    m_uhost_trans.trans_len = 0;
    m_uhost_trans.stage = UHOST_TRANS_BULK_IN;

    //request data packet
    usb_host_request_data(UHOST_IN_INDEX);
    
    usb_host_irq_ctrl(AK_TRUE);

    return AK_TRUE;
}

/**
   * @brief   start bulk out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  EP_index [in]  usb end point.
   * @param  data [in]  usb data buffer.
   * @param  len [in] len length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_bulk_out(T_U8 *data, T_U32 len)
{
    T_U32 trans_len = 0;
    T_U32 temp = 0;
    
    //return if not in idle stage
    if(m_uhost_trans.stage != UHOST_TRANS_IDLE)
    {
        return AK_FALSE;
    }

    usb_host_irq_ctrl(AK_FALSE);

    //set global variable
    m_uhost_trans.buffer = data;
    m_uhost_trans.data_len = len;
    m_uhost_trans.trans_len = 0;
    m_uhost_trans.stage = UHOST_TRANS_BULK_OUT;

    REG8(USB_REG_INDEX) = USB_HOST_OUT_INDEX;
    temp = REG16(USB_REG_TXMAXP0);
    
    if (temp < len)
        len = temp;

    trans_len = usb_host_data_out(UHOST_OUT_INDEX, data, len);

    m_uhost_trans.trans_len += trans_len;

    usb_host_irq_ctrl(AK_TRUE);

    return AK_TRUE;
}
#pragma arm section code

//********************************************************************
T_VOID usb_set_mode(T_U32 mode)
{
    REG8(USB_REG_POWER) = mode;
    
    delay_us(50000);
    
    REG8(USB_REG_POWER) |= 0x08;
    delay_us(30000);
    REG8(USB_REG_POWER) &= ~0x08;
}

//********************************************************************
#ifdef UHOST_INTR_MODE
#pragma arm section code = "_usb_host_"
#else
#pragma arm section code = "_drvbootcode_"
#endif
static T_VOID usb_host_common_intr_handler(T_U8 usb_int)
{
    if(0 != (usb_int & USB_INTR_CONNECT))                   //connect
    {
        state_connect = AK_TRUE;
        if(m_uhost_cbk.fcbk_connect != AK_NULL)
        {
            m_uhost_cbk.fcbk_connect();
        }
        return;
    }

    if(0 != (usb_int & USB_INTR_SUSPEND))                   //suspend
    {
        return;
    }

    if(0 != (usb_int & USB_INTR_RESUME))                    //resume
    {
        return;
    }

    if(0 != (usb_int & USB_INTR_DISCONNECT))                //disconnect
    {
        state_connect = AK_FALSE;
        #ifdef UHOST_INTR_MODE
        INT_DISABLE(INT_EN_USBMCU);
        #endif
        m_uhost_ctrl.stage = CTRL_STAGE_IDLE;
        m_uhost_trans.stage = UHOST_TRANS_IDLE;

        if(m_uhost_cbk.fcbk_disconnect != AK_NULL)
        {
            m_uhost_cbk.fcbk_disconnect();
        }
    }
    
    if(0 != (usb_int & USB_INTR_SOF))
    {
        return;
    }
}

#pragma arm section code

static T_VOID usb_host_ep0_rx_handler()
{
    T_U32 trans_len;
    
    switch(m_uhost_ctrl.stage)
    {
    case CTRL_STAGE_DATA_IN:
        drv_print("ctrl stage data in", 0, AK_TRUE);
        trans_len = usb_host_ctrl_in(m_uhost_ctrl.buffer + m_uhost_ctrl.trans_len);
        m_uhost_ctrl.trans_len += trans_len;

        if((trans_len < m_max_ep0_size) || (m_uhost_ctrl.trans_len >= m_uhost_ctrl.dev_req.wLength))
        {
            //last packet, set to status stage
            REG8(USB_REG_CSR0) |= USB_CSR0_TXPKTRDY | USB_CSR0_H_STATUSPKT;
            m_uhost_ctrl.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            //continue request packet
            REG8(USB_REG_CSR0) |= USB_CSR0_H_REQPKT;
        }

        break;

    case CTRL_STAGE_STATUS:
        //transaction finish
        REG8(USB_REG_CSR0) &= (~USB_CSR0_RXPKTRDY);

        m_uhost_ctrl.stage = CTRL_STAGE_IDLE;

        if(m_uhost_ctrl.cbk_func != AK_NULL)
            m_uhost_ctrl.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_ctrl.trans_len);

        break;

    default:
        //must be error in this case
        drv_print("error stage in ep0 rx:", m_uhost_ctrl.stage, AK_TRUE);
        break;
    }
}

static T_VOID usb_host_ep0_tx_handler()
{
    T_U32 trans_len, data_len;

    switch(m_uhost_ctrl.stage)
    {
    case CTRL_STAGE_SETUP:
        if(0 == m_uhost_ctrl.dev_req.wLength)               //no data stage
        {
            REG8(USB_REG_CSR0) |= USB_CSR0_H_REQPKT | USB_CSR0_H_STATUSPKT;
            m_uhost_ctrl.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            if(m_uhost_ctrl.dev_req.bmRequestType & (1<<7)) //data in
            {
                //request packet
                REG8(USB_REG_CSR0) = USB_CSR0_H_REQPKT;

                //set stage to data in
                m_uhost_ctrl.stage = CTRL_STAGE_DATA_IN;
            }
            else                                            //data out
            {
                //start send
                data_len = m_uhost_ctrl.data_len;
                trans_len = usb_host_ctrl_out(m_uhost_ctrl.buffer, data_len, AK_FALSE);

                m_uhost_ctrl.trans_len += trans_len;

                //set stage to data out
                m_uhost_ctrl.stage = CTRL_STAGE_DATA_OUT;
            }
        }
        break;

    case CTRL_STAGE_DATA_OUT:
        if(m_uhost_ctrl.trans_len < m_uhost_ctrl.data_len)
        {
            data_len = m_uhost_ctrl.data_len - m_uhost_ctrl.trans_len;
            trans_len = usb_host_ctrl_out(m_uhost_ctrl.buffer+m_uhost_ctrl.trans_len, data_len, AK_FALSE);

            m_uhost_ctrl.trans_len += trans_len;
        }
        else
        {
            //data out finish, enter status stage
            REG8(USB_REG_CSR0) |= USB_CSR0_H_REQPKT | USB_CSR0_H_STATUSPKT;
            m_uhost_ctrl.stage = CTRL_STAGE_STATUS;
        }
        break;

    case CTRL_STAGE_STATUS:
        //transaction finish
        m_uhost_ctrl.stage = CTRL_STAGE_IDLE;

        if(m_uhost_ctrl.cbk_func != AK_NULL)
            m_uhost_ctrl.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_ctrl.trans_len);
        break;

    default:
        //must be error in this case
        drv_print("error stage in ep0 tx:", m_uhost_ctrl.stage, AK_TRUE);
        break;
    }
}

static T_VOID usb_host_ep0_intr_handler()
{
    T_U8 usb_ep_csr;

    //read control and status reg0
    REG8(USB_REG_INDEX) = EP0_INDEX;
    usb_ep_csr = REG8(USB_REG_CSR0);

    if(usb_ep_csr & USB_CSR0_H_RXSTALL)                 //stall
    {
        //clear stall
        REG8(USB_REG_CSR0) &= ~USB_CSR0_H_RXSTALL;

        goto CTRL_TRANS_ERROR;
    }

    if(usb_ep_csr & USB_CSR0_H_ERROR)                   //error
    {
        //clear error
        REG8(USB_REG_CSR0) &= ~USB_CSR0_H_ERROR;

        goto CTRL_TRANS_ERROR;
    }

    if(usb_ep_csr & USB_CSR0_H_NAKTIMEOUT)             //nak timeout
    {
        //clear nak timeout
        REG8(USB_REG_CSR0) &= ~USB_CSR0_H_NAKTIMEOUT;

        goto CTRL_TRANS_ERROR;
    }

    if(usb_ep_csr & USB_CSR0_RXPKTRDY)                  //a packet is received
    {
        usb_host_ep0_rx_handler();
    }
    else if(!(usb_ep_csr & USB_CSR0_TXPKTRDY))
    {
        usb_host_ep0_tx_handler();
    }
    else
    {
        drv_print("undefined ep0 interrupt:", usb_ep_csr, AK_TRUE);
    }

    return;

CTRL_TRANS_ERROR:

    m_uhost_ctrl.stage = CTRL_STAGE_IDLE;

    if(m_uhost_ctrl.cbk_func != AK_NULL)
    {
        m_uhost_ctrl.cbk_func(USB_HOST_TRANS_ERROR, m_uhost_ctrl.data_len);
    }
}

#pragma arm section code = "_usb_host_"

static T_VOID usb_host_tx_handler()
{
    T_U8 usb_ep_csr;
    T_U32 trans_len = 0;
    T_U32 temp = 0;
    T_U32 value = 0;
    
    REG8(USB_REG_INDEX) = EP2_INDEX;
    usb_ep_csr = REG8(USB_REG_TXCSR1);
    temp = REG16(USB_REG_TXMAXP0);
    
    if (0 != (usb_ep_csr & USB_TXCSR1_H_RXSTALL))           //stall
    {
        //clear stall
        REG8(USB_REG_TXCSR1) &= (~USB_TXCSR1_H_RXSTALL);

        goto USB_TX_ERROR;
    }

    if(0 != (usb_ep_csr & USB_TXCSR1_H_ERROR))              //error
    {
        //clear error
        REG8(USB_REG_TXCSR1) &= (~USB_TXCSR1_H_ERROR);

        goto USB_TX_ERROR;
    }
    
    if(0 != (usb_ep_csr & USB_TXCSR1_H_NAKTIMEOUT))         //nak timeout
    {
        //clear nak timeout
        REG8(USB_REG_TXCSR1) &= (~USB_TXCSR1_H_NAKTIMEOUT);

        goto USB_TX_ERROR;
    }
    
    if(0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY))             //one packet send successfully
    {
        if(m_uhost_trans.stage == UHOST_TRANS_BULK_OUT)
        {
            if(m_uhost_trans.trans_len < m_uhost_trans.data_len)
            {
                value = m_uhost_trans.data_len - m_uhost_trans.trans_len;
                if (value > temp)
                    value = temp;
                trans_len = usb_host_data_out(UHOST_OUT_INDEX, m_uhost_trans.buffer + m_uhost_trans.trans_len, value);
                 
                m_uhost_trans.trans_len += trans_len;
            }
            else
            {
                //trans finish
                m_uhost_trans.stage = UHOST_TRANS_IDLE;

                if(m_uhost_trans.cbk_func != AK_NULL)
                {
                    m_uhost_trans.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_trans.trans_len);
                }
            }
        }
    }

    return;

USB_TX_ERROR:

    m_uhost_trans.stage = UHOST_TRANS_IDLE;

    if(m_uhost_trans.cbk_func != AK_NULL)
    {
        m_uhost_trans.cbk_func(USB_HOST_TRANS_ERROR, m_uhost_trans.trans_len);
    }
}


static T_VOID usb_host_rx_handler()
{
    T_U8 usb_ep_csr;
    T_U32 trans_len = 0;
    
    REG8(USB_REG_INDEX) = EP3_INDEX;
    usb_ep_csr = REG16(USB_REG_RXCSR1);
    
    if (0 != (usb_ep_csr & USB_RXCSR1_H_RXSTALL))       //stall
    {
        //clear stall
        //akprintf(C1, M_DRVSYS, "S");
        REG8(USB_REG_RXCSR1) &= (~USB_REG_RXCSR1_RXSTALL);

        goto USB_RX_ERROR;
    }

    if(0 != (usb_ep_csr & USB_RXCSR1_H_ERROR))         //error
    {
        //clear error
        //akprintf(C1, M_DRVSYS, "e");
        REG8(USB_REG_RXCSR1) &= (~USB_RXCSR1_H_ERROR);

        goto USB_RX_ERROR;
    }

    if(0 != (usb_ep_csr & USB_RXCSR1_NAKTIMEOUT))      //nak timeout
    {
        //clear nak timeout
        //akprintf(C1, M_DRVSYS, "t");
        REG8(USB_REG_RXCSR1) &= (~USB_RXCSR1_NAKTIMEOUT);

        goto USB_RX_ERROR;
    }

    if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))       //a packet is received
    {
        if(m_uhost_trans.stage == UHOST_TRANS_BULK_IN)
        {
            #if 0
            //a short pkt is reveived while usb dma is working
            if (m_uhost_trans.dma_start)
            {
                m_uhost_trans.trans_len += REG32(USB_DMA_ADDR_2) - 0x71000000;
            }
            #endif
            trans_len = usb_host_data_in(UHOST_IN_INDEX, m_uhost_trans.buffer + m_uhost_trans.trans_len);
            m_uhost_trans.trans_len += trans_len;

            //trans finish
            if (trans_len < REG16(USB_REG_RXMAXP1) ||
               m_uhost_trans.trans_len >=  m_uhost_trans.data_len)
            {
                m_uhost_trans.stage = UHOST_TRANS_IDLE;
                //drv_print("rx trans finish\n",0,1);
                //call back
                if(m_uhost_trans.cbk_func != AK_NULL)
                {
                    m_uhost_trans.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_trans.trans_len);
                }
            }
            else
            {
                //drv_print("rx data again\n",0,1);
                usb_host_request_data(EP3_INDEX);
            }
        }
        else
        {
            //drv_print("rx data discard\n",0,1);
            //discard this packet
            //l2_clr_status(usb_bulk_receive_id);
            REG8(USB_REG_RXCSR1) &= (~USB_RXCSR1_RXPKTRDY);
        }
    }

    return;

USB_RX_ERROR:
    m_uhost_trans.stage = UHOST_TRANS_IDLE;

    if(m_uhost_trans.cbk_func != AK_NULL)
    {
        m_uhost_trans.cbk_func(USB_HOST_TRANS_ERROR, m_uhost_trans.trans_len);
    }
}
#pragma arm section code 

static T_VOID usb_host_dma_handler(T_U32 dma_int)
{
    T_U32 i,short_pkt_len;
    
    if( (dma_int & DMA_CHANNEL1_INT)  == DMA_CHANNEL1_INT)
    {
        REG8(USB_REG_INDEX) = USB_EP2_INDEX;
        REG8(USB_REG_TXCSR2) = USB_TXCSR_MODE1;
        REG32(USB_DMA_CNTL_1) = 0;
        //l2_combuf_wait_dma_finish(usb_bulk_send_id);
        
        short_pkt_len = m_uhost_trans.data_len - m_uhost_trans.trans_len;
        
        if (0 != short_pkt_len)
        {
            for (i = 0; i < short_pkt_len; i++)
            {
                REG8(USB_FIFO_EP0 + (USB_EP2_INDEX << 2)) = *(m_uhost_trans.buffer +  m_uhost_trans.trans_len + i);
            }
            //set tx count
            REG32(USB_EP2_TX_COUNT) = short_pkt_len;
            //set TXPKTRDY
            REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;
        }
        else
        {
            //trans finish
            m_uhost_trans.stage = UHOST_TRANS_IDLE;

            if(m_uhost_trans.cbk_func != AK_NULL)
            {
                m_uhost_trans.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_trans.trans_len);
            }
        }
    }

    if((dma_int & DMA_CHANNEL2_INT) == DMA_CHANNEL2_INT)
    {        
        REG8(USB_REG_INDEX) = USB_EP3_INDEX;
        REG8(USB_REG_RXCSR2) = 0;
        REG32(USB_DMA_CNTL_2) = 0;
        //l2_combuf_wait_dma_finish(usb_bulk_receive_id);
        m_uhost_trans.trans_len += REG32(USB_DMA_ADDR_2) - 0x71000000;
        m_uhost_trans.dma_start = AK_FALSE;
        if (m_uhost_trans.trans_len < m_uhost_trans.data_len)
        {
            //REG8(USB_REG_RXCSR1) |= USB_RXCSR1_H_REQPKT;
        }
        else
        {
            m_uhost_trans.stage = UHOST_TRANS_IDLE;
            //call back
            if(m_uhost_trans.cbk_func != AK_NULL)
            {
                m_uhost_trans.cbk_func(USB_HOST_TRANS_COMPLETE, m_uhost_trans.trans_len);
            }
        }
    }
}

//******************************************************************************

#ifdef UHOST_INTR_MODE
#pragma arm section code = "_usb_host_"
#else
#pragma arm section code = "_drvbootcode_"
#endif
T_VOID usb_host_intr_handler(T_VOID)
{
    T_U8 usb_int;
    T_U8 usb_ep_int_tx;
    T_U8 usb_ep_int_rx;
    T_U32 usb_dma_int;
    T_BOOL usb_other_interrupt = AK_TRUE;


    usb_host_read_int_reg(&usb_int, &usb_dma_int);

    if(0 != usb_int)
    {
        usb_host_common_intr_handler(usb_int);
        usb_other_interrupt = AK_FALSE;
    }
    
#ifdef UHOST_INTR_MODE
    usb_host_read_ep_int_reg(&usb_ep_int_tx, &usb_ep_int_rx);

    if(USB_INTR_EP0 == (usb_ep_int_tx & USB_INTR_EP0))
    {
        usb_host_ep0_intr_handler();
        usb_other_interrupt = AK_FALSE;
    }
    
    if(USB_INTR_EP1 == (usb_ep_int_tx & USB_INTR_EP1))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_rx_handler();
    }
    
    if(USB_INTR_EP3 == (usb_ep_int_rx & USB_INTR_EP3))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_rx_handler();
    }
    
    if(USB_INTR_EP2 == (usb_ep_int_tx & USB_INTR_EP2))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_tx_handler();
    }
    #if 0
    if(usb_other_interrupt)
    {
        drv_print("usb_int = %x\n", usb_int,1);
        drv_print("usb_t_int = %x\n", usb_ep_int_tx,1);
        drv_print("usb_r_int = %x\n\n", usb_ep_int_rx,1);
        drv_print("not designed !\n" ,0,1);
    }
    #endif
#endif
}
#pragma arm section code 


#ifndef UHOST_INTR_MODE
#pragma arm section code = "_usb_host_"
T_VOID usb_host_poll_status(T_VOID)
{
    T_U8 usb_ep_int_tx;
    T_U8 usb_ep_int_rx;
    T_U32 usb_dma_int;

    T_BOOL usb_other_interrupt = AK_TRUE;

    usb_host_read_ep_int_reg(&usb_ep_int_tx, &usb_ep_int_rx);
    if(USB_INTR_EP0 == (usb_ep_int_tx & USB_INTR_EP0))
    {
        usb_host_ep0_intr_handler();
        usb_other_interrupt = AK_FALSE;
    }
    
    if(USB_INTR_EP1 == (usb_ep_int_tx & USB_INTR_EP1))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_rx_handler();
    }
    
    if(USB_INTR_EP3 == (usb_ep_int_rx & USB_INTR_EP3))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_rx_handler();
    }
    
    if(USB_INTR_EP2 == (usb_ep_int_tx & USB_INTR_EP2))
    {
        usb_other_interrupt = AK_FALSE;
        usb_host_tx_handler();
    }
}
#pragma arm section code 
#endif


//******************************************************************************
#endif// #if DRV_SUPPORT_UHOST > 0


