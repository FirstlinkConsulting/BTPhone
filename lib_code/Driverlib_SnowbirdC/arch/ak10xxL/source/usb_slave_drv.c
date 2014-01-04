/*******************************************************************************
 * @file    usb_slave_drv.c
 * @brief   frameworks of usb driver.
 * This file describe udriver of usb in slave mode.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-26
 * @version 1.0
 * @ref     
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "usb_slave_drv.h"
#include "interrupt.h"
#include "drv_api.h"
#include "usb_common.h"
#include "hal_usb_s_state.h"
#include "hal_usb_s_std.h"
#include "hal_usb_std.h"
#include "l2.h"
#include "clk.h"
#include "arch_init.h"
#include "hal_int_msg.h"
#include "hal_clk.h"
#include "drv_cfg.h"


#define MAX_EP_NUM     7
#define MAX_DMA_SIZE   4096

#if USB_USE_DMA > 0
#define DMA_TRANS_MODE 1
#endif


typedef enum
{
    EP_RX_FINISH = 0,
    EP_RX_RECEIVING,
    EP_TX_FINISH,
    EP_TX_SENDING
}USB_EP_STATE;

typedef struct
{
    T_U32 EP_TX_Count;
    USB_EP_STATE EP_TX_State;
    T_fUSB_TX_FINISH_CALLBACK TX_Finish; 
    T_U8  *EP_TX_Buffer;
    T_U8  L2_Buf_ID;
    T_BOOL bDmaStart;
}USB_EP_TX;

typedef struct
{
    T_U32 EP_RX_Count;
    USB_EP_STATE EP_RX_State;
    T_fUSB_NOTIFY_RX_CALLBACK RX_Notify;
    T_fUSB_RX_FINISH_CALLBACK RX_Finish;
    T_U8  *EP_RX_Buffer;
    T_U8  L2_Buf_ID;
    T_BOOL bDmaStart;
}USB_EP_RX;

typedef union
{
    USB_EP_RX rx;
    USB_EP_TX tx;
}USB_EP;

typedef struct
{
    T_U32 tx_count;
    T_CONTROL_TRANS ctrl_trans;
    T_fUSB_CONTROL_CALLBACK std_req_callback;
    T_fUSB_CONTROL_CALLBACK class_req_callback;
    T_fUSB_CONTROL_CALLBACK vendor_req_callback;
}
T_USB_SLAVE_CTRL;

typedef struct
{
    T_U32 ulInitMode;                           ///<expected mode when init
    T_U32 mode;
    T_U32 state;
    T_U32 usb_max_pack_size;
    T_fUSB_RESET_CALLBACK reset_callback;
    T_fUSB_SUSPEND_CALLBACK suspend_callback;
    T_fUSB_RESUME_CALLBACK resume_callback;
    T_fUSB_CONFIGOK_CALLBACK configok_callback;
    USB_EP ep[6];
    T_BOOL bInit;
    T_BOOL usb_need_zero_packet;
}USB_SLAVE;

#define usb_slave_send_message(status) post_int_message(IM_USLAVE, status, 0)

static T_VOID usb_slave_dma_send_mode0(T_U8 EP_index, T_U32 addr, T_U32 count);

static T_VOID usb_slave_reset(T_VOID);
static T_VOID usb_slave_suspend(T_VOID);
static T_VOID usb_slave_reset_ep(T_U32 EP_index, T_U16 wMaxPacketSize, T_U8 ep_type, T_U8 dma_surport);

static T_U32  usb_slave_get_intr_type(T_U8 *usb_int, T_U16 *usb_ep_int_tx, T_U16 * usb_ep_int_rx);
static T_VOID usb_slave_ep0_rx_handler(T_VOID);
static T_VOID usb_slave_ep0_tx_handler(T_VOID);
static T_VOID usb_slave_common_intr_handler(T_U8 usb_int);

static T_VOID usb_slave_tx_handler(EP_INDEX EP_index);
static T_VOID usb_slave_rx_handler(EP_INDEX EP_index);

static T_VOID usb_slave_write_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 value);
static T_VOID usb_slave_read_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 *value);
static T_VOID usb_slave_read_int_reg(T_U8 *value0, T_U16 *value1, T_U16 *value2);

static T_U32  usb_slave_receive_data(EP_INDEX EP_index);
static T_U32  usb_slave_send_data(EP_INDEX EP_index);

static T_U32  usb_slave_ctrl_in(T_U8 *data, T_U32 len);
static T_U32  usb_slave_ctrl_out(T_U8 *data);
static T_BOOL usb_slave_ctrl_callback(T_U8 req_type);
T_U32 usb_slave_intr_handler_inner(T_VOID);
static T_U32 usb_slave_dma_stop(EP_INDEX EP_index);


#pragma arm section zidata = "_udisk_bss_"

volatile USB_SLAVE usb_slave;
volatile T_USB_SLAVE_CTRL usb_ctrl;
//volatile T_U8 dma_flag=0;

static volatile T_U16 m_ep_status[MAX_EP_NUM]={0};
#pragma arm section zidata

#pragma arm section zidata = "_drvbootbss_"
static T_U32 detect_time;
static T_BOOL usb_connect_pc;
#pragma arm section zidata

#pragma arm section rwdata = "_drvbootcache_"
static volatile T_U8 asic_id = INVALID_ASIC_INDEX;
#pragma arm section rwdata


/*******************************************************************************
 * @brief   initialize usb slave global variables, and set buffer for control tranfer
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]buffer: buffer to be set for control transfer
 * @param   [in]buf_len: buffer length
 * @return  T_BOOL
 * @retval  AK_TRUE init successfully
 * @retval  AK_FALSE init fail
*******************************************************************************/
T_BOOL usb_slave_init(T_U8 *buffer, T_U32 buf_len)
{
    T_U32 buf1_addr,buf2_addr;
    T_U32 len1,len2;
    
    //check param
    if(AK_NULL == buffer || buf_len == 0)
    {
        return AK_FALSE;
    }

    //init global variables
    memset((T_pVOID)&usb_slave, 0, sizeof(usb_slave));
    usb_slave.bInit = AK_TRUE;

    usb_slave.usb_max_pack_size = 64;

    //alloc buffer
    #ifdef DMA_TRANS_MODE
    l2_init_device_buf(ADDR_USB2);
    l2_init_device_buf(ADDR_USB3);
    
    get_device_buf_info(ADDR_USB2, (T_U32 *)&buf1_addr, 
                           (T_U32 *)&len1, (T_U32 *)&(usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID));
    get_device_buf_info(ADDR_USB3, &buf2_addr, 
                           (T_U32 *)&len2, (T_U32 *)&(usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID));    

    drv_print("EP2_INDEX:",usb_slave.ep[EP2_INDEX].tx.L2_Buf_ID,1);
    drv_print("EP3_INDEX:",usb_slave.ep[EP3_INDEX].rx.L2_Buf_ID,1);
    drv_print("EP2_len:",len1,1);
    drv_print("EP3_len:",len2,1);
    #endif

    usb_ctrl.ctrl_trans.buffer = buffer;
    usb_ctrl.ctrl_trans.buf_len = buf_len;

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   free usb slave global variables,L2 buffer
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_free(T_VOID)
{
    memset((T_pVOID)&usb_slave, 0, sizeof(usb_slave));

#ifdef DMA_TRANS_MODE
    l2_release_device_buf(ADDR_USB2);
    l2_release_device_buf(ADDR_USB3);
#endif

    usb_ctrl.ctrl_trans.buffer = AK_NULL;
    usb_ctrl.ctrl_trans.buf_len= 0;
}


/*******************************************************************************
 * @brief   set control transfer call back function
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]type: request type, must be one of (REQUEST_STANDARD, REQUEST_CLASS, REQUEST_VENDOR)
 * @param   [in]callback: callback function
 * @return  T_BOOL
 * @retval  AK_TRUE callback function set successfully
 * @retval  AK_FALSE fail to set callback function
*******************************************************************************/
T_BOOL usb_slave_set_ctrl_callback(T_U8 type, T_fUSB_CONTROL_CALLBACK callback)
{
    //standard request
    if(REQUEST_STANDARD == type)
        usb_ctrl.std_req_callback = callback;

    //class request
    if(REQUEST_CLASS == type)
        usb_ctrl.class_req_callback = callback;

    //vendor request
    if(REQUEST_VENDOR == type)
        usb_ctrl.vendor_req_callback = callback;

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   set usb event(reset, suspend, resume, configok) callback.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]reset_callback: callback function for reset interrupt
 * @param   [in]suspend_callback: callback function for suspend interrupt
 * @param   [in]resume_callback: callback function for resume interrupt
 * @param   [in]configok_callback: callback function for config ok event
 * @return  T_BOOL
 * @retval  AK_TRUE callback function set successfully
 * @retval  AK_FALSE fail to set callback function
*******************************************************************************/
T_BOOL usb_slave_set_callback(T_fUSB_RESET_CALLBACK reset_callback, T_fUSB_SUSPEND_CALLBACK suspend_callback, T_fUSB_RESUME_CALLBACK resume_callback, T_fUSB_CONFIGOK_CALLBACK configok_callback)
{
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.reset_callback = reset_callback;
    usb_slave.suspend_callback = suspend_callback;
    usb_slave.resume_callback = resume_callback;
    usb_slave.configok_callback = configok_callback;

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   Register a callback function to notify tx send data finish.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0??
 * @param   [in]callback_func: T_fUSB_TX_FINISH_CALLBACK can be null
 * @return  T_BOOL
 * @retval  AK_TRUE callback function set successfully
 * @retval  AK_FALSE fail to set callback function
*******************************************************************************/
T_BOOL usb_slave_set_tx_callback(EP_INDEX EP_index, T_fUSB_TX_FINISH_CALLBACK callback_func)
{
    T_U32 tx_index = EP_index;
    
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.ep[tx_index].tx.TX_Finish = callback_func;
    
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   Register a callback function to notify rx receive data finish and rx have data.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param   [in]notify_rx: rx notify callbakc function, can be null
 * @param   [in]rx_finish: rx finish callbakc function, can be null
 * @return  T_BOOL
 * @retval  AK_TRUE callback function set successfully
 * @retval  AK_FALSE fail to set callback function
*******************************************************************************/
T_BOOL usb_slave_set_rx_callback(EP_INDEX EP_index, T_fUSB_NOTIFY_RX_CALLBACK notify_rx, T_fUSB_RX_FINISH_CALLBACK rx_finish)
{
    T_U32 rx_index = EP_index;
    
    if(!usb_slave.bInit)
        return AK_FALSE;

    usb_slave.ep[rx_index].rx.RX_Notify = notify_rx;
    usb_slave.ep[rx_index].rx.RX_Finish = rx_finish;

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   enable usb slave driver.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]mode: T_U32 usb mode
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_device_enable(T_U32 mode)
{
    T_U32 tmp;
    extern  T_U8 g_usbtype;
    
    //check init
    if(!usb_slave.bInit)
    {
        drv_print("usb slave isn't init\n", 0, AK_TRUE);
        return;
    }
    
    //enable clock, USB PLL, USB 2.0
    sys_module_enable(eVME_USB_CLK, AK_TRUE);

    //enable the usb transceiver and suspend enable
    REG32(REG_MUL_FUNC_CTRL) |= (PHY_RST | PHY_SUS);
    REG32(REG_MUL_FUNC_CTRL) &= ~(PHY_RST | PHY_LDO);

    //enable the usb phy
    REG8(USB_REG_CFG) = 0xd7;

    #if CHIP_SEL_10C > 0
    tmp = REG32(USB_REG_CTRL);
    tmp |= (0x1<<18);
    tmp |= (0x1<<21);
    tmp |= (0x1<<22);
    tmp |= (0x7<<24);
    tmp &= ~(0x1<<27); 
    REG32(USB_REG_CTRL) = tmp;
    #endif
    
    //set usb type to slave
    g_usbtype = 0;
    
    //enable usb
    if(USB_MODE_20 == (USB_MODE_20 & mode))
    {
        REG8(USB_REG_POWER) = 0x20;
        usb_slave.ulInitMode = USB_MODE_20;
        //set usb_max_pack_size here  for burn tool
        usb_slave.usb_max_pack_size = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        drv_print("high speed!", 0, 1);
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;
        usb_slave.ulInitMode = USB_MODE_11;
        //set usb_max_pack_size here  for burn tool
        usb_slave.usb_max_pack_size = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        drv_print("full speed!\r\n", 0, 1);
    }

    usb_slave_set_state(USB_CONFIG);

#if (USLAVE_USE_INTR > 0)

    INT_ENABLE(INT_EN_USBMCU);

    #ifdef DMA_TRANS_MODE
    INT_ENABLE(INT_EN_USBDMA);
    #endif

#else

    INT_DISABLE(INT_EN_USBMCU);
    INT_DISABLE(INT_EN_USBDMA);

#endif

    l2_int_ctrl(AK_TRUE); //enable page counter interrupt
}


/*******************************************************************************
 * @brief   disable usb slave driver.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_device_disable(T_VOID)
{
    INT_DISABLE(INT_EN_USBMCU);
    #ifdef DMA_TRANS_MODE
    INT_DISABLE(INT_EN_USBDMA);
    #endif
    //平台要求一直要开着。
    //l2_int_ctrl(AK_FALSE); //disable page counter interrupt

    REG8(USB_REG_POWER) = 0;

    //disable usb transcieve
    REG32(REG_MUL_FUNC_CTRL) &= ~PHY_SUS;

    //close usb clock
    sys_module_enable(eVME_USB_CLK, AK_FALSE);
    sys_module_reset(eVME_USB_CLK);

    usb_slave_set_state(USB_NOTUSE);
}


//******************************************************************************
static T_VOID usb_slave_reset(T_VOID)
{
    T_U32 temp;

    //set address to default addr
    usb_slave_set_address(0);

    //check which speed now on after negotiation
    if ((REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE)
    {
        REG8(USB_REG_POWER) = 0x20;

        usb_slave.mode = USB_MODE_20;
        
        drv_print("reset to high speed!", 0, 1);
    }
    else
    {
        REG8(USB_REG_POWER) = 0x0;

        usb_slave.mode = USB_MODE_11;
        
        drv_print("reset to full speed!", 0, 1);
    }

    //open all common interrupt except sof
    REG8(USB_REG_INTRUSBE) = 0xFF & (~USB_INTR_SOF);      //disable the sof interrupt
}

#ifdef DMA_TRANS_MODE

static T_U32 usb_slave_dma_stop(EP_INDEX EP_index)
{
    if(EP2_INDEX == EP_index)
    {
        if(usb_slave.ep[EP_index].tx.bDmaStart)
        {
            REG8(USB_REG_INDEX) = USB_EP2_INDEX;
            REG8(USB_REG_TXCSR2) = 0;
            REG32(USB_DMA_CNTL_1) = 0;
            REG32(USB_DMA_COUNT_1) = 0;
            usb_slave.ep[EP_index].tx.bDmaStart = AK_FALSE;
            ldma_trans_stop();
        }
    }
    else if(EP3_INDEX == EP_index)
    {
        if(usb_slave.ep[EP_index].rx.bDmaStart)
        {
            REG8(USB_REG_INDEX) = USB_EP3_INDEX;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
            usb_slave.ep[EP_index].rx.bDmaStart = AK_FALSE;
            ldma_trans_stop();
        }
    }

}

#endif


//******************************************************************************
T_VOID usb_slave_set_ep(T_U32 EP_index, T_U8 ep_attribute, T_U8 ep_type, T_U16 wMaxPacketSize)
{
    T_U32 fifo_size;
    T_U8 tmp;

#ifdef DMA_TRANS_MODE
    usb_slave_dma_stop(EP_index);
#endif

    //clear status
    if(USB_EP_IN_TYPE == ep_type)
    {
        usb_slave.ep[EP_index].tx.bDmaStart = AK_FALSE;
        usb_slave.ep[EP_index].tx.EP_TX_Buffer = 0;
        usb_slave.ep[EP_index].tx.EP_TX_Count= 0;
        usb_slave.ep[EP_index].tx.EP_TX_State = EP_TX_FINISH;
        usb_slave.ep[EP_index].tx.L2_Buf_ID = 0;
    }
    else
    {
        usb_slave.ep[EP_index].rx.bDmaStart = AK_FALSE;
        usb_slave.ep[EP_index].rx.EP_RX_Buffer = 0;
        usb_slave.ep[EP_index].rx.EP_RX_Count = 0;
        usb_slave.ep[EP_index].rx.EP_RX_State = EP_RX_FINISH;
        usb_slave.ep[EP_index].rx.L2_Buf_ID = 0;
    }

    //select the ep
    REG8(USB_REG_INDEX) = EP_index;             /* select this EP */

    tmp = REG8(USB_REG_FIFOSIZE);
    fifo_size = 1 << (tmp & 0x0F);

    tmp = REG8(USB_REG_FIFOSIZE);
    fifo_size = 1 << ((tmp & 0xF0) >> 4);

    if (USB_MODE_20 == usb_slave.mode)
    {
        if (EP_ATTRIBUTE_SYNCH == ep_attribute)
            usb_slave.usb_max_pack_size = EP_ISO_HIGHSPEED_MAX_PAK_SIZE;
        else if (EP_ATTRIBUTE_BULK == ep_attribute)
            usb_slave.usb_max_pack_size = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;    
    }
    else
    {
        if (EP_ATTRIBUTE_SYNCH == ep_attribute)
            usb_slave.usb_max_pack_size = EP_ISO_FULLSPEED_MAX_PAK_SIZE;
        else if (EP_ATTRIBUTE_BULK == ep_attribute)
            usb_slave.usb_max_pack_size = EP_BULK_FULLSPEED_MAX_PAK_SIZE;        
    }
    
    //select ep type and max packet size
    switch(ep_attribute)
    {
        case EP_ATTRIBUTE_BULK:
        case EP_ATTRIBUTE_INTR:
        {
            if (ep_type == USB_EP_IN_TYPE)
            {
                REG8(USB_REG_TXCSR2) = USB_TXCSR2_MODE;
                REG16(USB_REG_TXMAXP1) = wMaxPacketSize;
            }
            else if (ep_type == USB_EP_OUT_TYPE)
            {
                REG8(USB_REG_RXCSR2) = 0;
                REG16(USB_REG_RXMAXP1) = wMaxPacketSize;
                REG8(USB_REG_RXCSR1) &=  (~USB_RXCSR1_RXPKTRDY);
            }
        }break;
        case EP_ATTRIBUTE_SYNCH:
        {
            //select ep type and max packet size
            if (ep_type == USB_EP_IN_TYPE)
            {
                REG8(USB_REG_TXCSR2) = (USB_TXCSR2_MODE|0x40);
                REG16(USB_REG_TXMAXP1) = wMaxPacketSize;
            }
            else if (ep_type == USB_EP_OUT_TYPE)
            {
                REG8(USB_REG_RXCSR2) = 0;
                REG8(USB_REG_RXCSR2) = 0x40;
                REG16(USB_REG_RXMAXP1) = wMaxPacketSize;
                REG8(USB_REG_RXCSR1) &=  (~USB_RXCSR1_RXPKTRDY);
            }
        }break;
        default:
            break;
    }
    
    //enable the TX endpoint
    REG8(USB_REG_INTRTX1E) = (USB_EP0_ENABLE | USB_EP2_TX_ENABLE |USB_EP1_TX_ENABLE);

    //enable the RX endpoint
    REG8(USB_REG_INTRRX1E) = (USB_EP3_RX_ENABLE);

    //clear the interrupt
    tmp = REG8(USB_REG_INTRUSB);
    tmp = REG8(USB_REG_INTRTX1);
    tmp = REG8(USB_REG_INTRTX2);
    tmp = REG8(USB_REG_INTRRX1);
    tmp = REG8(USB_REG_INTRRX2);

    //select this EP0
    REG8( USB_REG_INDEX) = USB_EP0_INDEX;

    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
}


//******************************************************************************
static T_VOID usb_slave_suspend(T_VOID)
{
    //device is at full speed when in suspend ,so reinit high speed
    if (USB_MODE_20 == usb_slave.ulInitMode)
    {
        REG8(USB_REG_POWER) = 0x20;
    }
}


#pragma arm section code = "_udisk_rw_"

static T_U32 usb_slave_dma_start(EP_INDEX EP_index)
{
    T_U32 ret,res;
    T_U32 tx_index = EP_index;
    T_U32 id_buf = ADDR_USB2;
    
    //set autoset/DMAReqEnable/DMAReqMode
    REG8(USB_REG_INDEX) = EP_index;
    REG8(USB_REG_TXCSR2) |= (USB_TXCSR2_DMAMODE | USB_TXCSR2_DMAENAB | USB_TXCSR2_AUTOSET);

    ret = usb_slave.ep[tx_index].tx.EP_TX_Count;
    res = (ret / 512);
    ret = res*512;
    if (ret > MAX_DMA_SIZE)
        ret = MAX_DMA_SIZE;
   
    l2_set_status(usb_slave.ep[tx_index].tx.L2_Buf_ID, 0);   
    l2_trans_data_dma(id_buf,(T_U32)usb_slave.ep[tx_index].tx.EP_TX_Buffer, ret);
    
    REG32(USB_DMA_ADDR_1) = 0x72000000;
    REG32(USB_DMA_COUNT_1) = ret;

    //change global variable value    
    usb_slave.ep[tx_index].tx.EP_TX_Count -= ret;
    usb_slave.ep[tx_index].tx.EP_TX_Buffer += ret;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;

    usb_slave.ep[EP2_INDEX].tx.bDmaStart = AK_TRUE;
    
    REG32(USB_DMA_CNTL_1) = (USB_ENABLE_DMA | USB_DIRECTION_TX | USB_DMA_MODE1 | USB_DMA_INT_ENABLE | (EP_index<<4) | USB_DMA_BUS_MODE3);
    return ret;
}


static T_U32 usb_slave_dma_receive(EP_INDEX EP_index)
{
    T_U32 res,ret;
    T_U32 id_buf=ADDR_USB3;
    T_U32 i;
    T_U32 *data;


    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    
    //read data from fifo
    for(i = 0; i < ret; i++)
        usb_slave.ep[EP_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP3);

    if(usb_slave.ep[EP_index].rx.EP_RX_Count <= ret ||
        usb_slave.ep[EP_index].rx.EP_RX_Count <= usb_slave.usb_max_pack_size)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

        usb_slave.ep[EP_index].rx.EP_RX_Count = 0;
        usb_slave.ep[EP_index].rx.EP_RX_State = EP_RX_FINISH;
        usb_slave.ep[EP_index].rx.EP_RX_Buffer += ret;
        return ret;
    }

    usb_slave.ep[EP_index].rx.EP_RX_Count -= ret;
    usb_slave.ep[EP_index].rx.EP_RX_Buffer += ret;
    data = (T_U32 *)(usb_slave.ep[EP_index].rx.EP_RX_Buffer);
    if ((T_U32)data & 0x3)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;
        return ret;
    }    

    #ifdef DMA_TRANS_MODE
    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[EP_index].rx.EP_RX_Count >= 512))
    {
        ret = usb_slave.ep[EP_index].rx.EP_RX_Count;
        res = (ret / 512);
        ret = res*512;
        if (ret > MAX_DMA_SIZE)
            ret = MAX_DMA_SIZE;

        REG8(USB_REG_RXCSR2) |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);
        l2_set_status(usb_slave.ep[EP_index].rx.L2_Buf_ID,0);
        l2_trans_data_dma((T_U32)usb_slave.ep[EP_index].rx.EP_RX_Buffer, id_buf, ret);

        REG32(USB_DMA_ADDR_2) = 0x73000000;
        REG32(USB_DMA_COUNT_2) = ret;

        //usb_slave.ep[EP_index].rx.EP_RX_Count -= ret;
        usb_slave.ep[EP_index].rx.bDmaStart = AK_TRUE;
        usb_slave.ep[EP_index].rx.EP_RX_State = EP_RX_RECEIVING;
        //if (usb_slave.ep[EP_index].rx.EP_RX_Count==0)
        //    usb_slave.ep[EP_index].rx.EP_RX_State = EP_RX_FINISH;
        REG32(USB_DMA_CNTL_2) = (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(EP_index<<4)|USB_DMA_BUS_MODE3);
        //uart_write_chr(0, 'S');
    }
    #endif
    
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}


static T_U32 usb_slave_receive_data(EP_INDEX EP_index)
{
    T_U16 ret, i;
    T_U32 rx_index = EP_index;
    T_U32 *data = (T_U32 *)usb_slave.ep[rx_index].rx.EP_RX_Buffer;


    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);

    /*if((ret == EP_BULK_HIGHSPEED_MAX_PAK_SIZE) && (((T_U32)data&0x3) == 0))
    {
        for(i = 0; i < ret/4; i++)
        {
            data[i] = REG32(USB_FIFO_EP3);
        }
    }
    else*/
    {
        for(i = 0; i < ret; i++)
        {
            usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP3);
        }
    }
        
    //change global variable status
    if(usb_slave.ep[rx_index].rx.EP_RX_Count > ret && usb_slave.ep[rx_index].rx.EP_RX_Count > usb_slave.usb_max_pack_size)
        usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    else
    {
        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;
    }

    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;

    //clear RXPKTRDY
    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
}


static T_U32 usb_slave_send_data(EP_INDEX EP_index)
{
    T_U32 count, i;
    T_U8 flag;
    T_U32 regaddr = USB_FIFO_EP2;
    T_U32 tx_index = EP_index;
    T_U32 *data = (T_U32 *)usb_slave.ep[tx_index].tx.EP_TX_Buffer;


    count = usb_slave.ep[tx_index].tx.EP_TX_Count;
    data = (T_U32 *)(usb_slave.ep[tx_index].tx.EP_TX_Buffer);
    flag = 1;
    if ((T_U32)data & 0x3)
    {
        flag = 0;
    }    
    
    #ifdef DMA_TRANS_MODE
    if (flag&&(usb_slave.mode == USB_MODE_20) && (usb_slave.ep[tx_index].tx.EP_TX_Count > usb_slave.usb_max_pack_size) )
    {
        count = usb_slave_dma_start(tx_index);
    }
    else
    #endif
    {
        if (count > usb_slave.usb_max_pack_size)
        {
            count = usb_slave.usb_max_pack_size;
        }

        if (EP2_INDEX == EP_index)
            regaddr = USB_FIFO_EP2;
        else if (EP1_INDEX == EP_index)
            regaddr = USB_FIFO_EP1;

        if((EP_BULK_HIGHSPEED_MAX_PAK_SIZE == count) && (((T_U32)data&0x3) == 0))
        {
            for(i = 0; i < count/4; i++)
            {
                REG32(regaddr) = data[i];
            }
        }
        else
        {
            for(i = 0; i < count; i++)
            {
                REG8(regaddr) = usb_slave.ep[tx_index].tx.EP_TX_Buffer[i];
            }
        }
        //change global variable value
        usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;
        usb_slave.ep[tx_index].tx.EP_TX_Count = usb_slave.ep[tx_index].tx.EP_TX_Count - count;
        usb_slave.ep[tx_index].tx.EP_TX_Buffer += count;
        
        REG8(USB_REG_INDEX) = EP_index;
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_TXPKTRDY;
    }

    return count;
}


/*******************************************************************************
 * @brief   read usb data with end point.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @param   [out]pBuf: usb data buffer.
 * @param   [in]count: count to be read
 * @return  T_U32
 * @retval  data out count
*******************************************************************************/
T_U32 usb_slave_data_out(EP_INDEX EP_index, T_VOID *pBuf, T_U32 count)
{
    T_U32 rx_index = EP_index;
    T_U32 ret = 0, res, i,id_buf=ADDR_USB3;
    T_U32 *data;


    if(EP0_INDEX == EP_index)
    {
        //akprintf(C1, M_DRVSYS, "usb_slave_data_out: error ep number: %d\n", EP_index);
        return 0;
    }

    if(usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_RECEIVING)
    {
        //akprintf(C1, M_DRVSYS, "usb_slave_data_out: still receiving\n");
        return 0;
    }
    
    usb_slave.ep[rx_index].rx.EP_RX_Buffer = pBuf;
    usb_slave.ep[rx_index].rx.EP_RX_Count = count;
    usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_RECEIVING;

    //return usb_slave_dma_receive(EP_index);

    #if 1
    //read the count of receive data
    REG8(USB_REG_INDEX) = EP_index;
    ret = REG16(USB_REG_RXCOUNT1);
    //read data from fifo
    for(i = 0; i < ret; i++)
        usb_slave.ep[rx_index].rx.EP_RX_Buffer[i] = REG8(USB_FIFO_EP3);

    if(usb_slave.ep[rx_index].rx.EP_RX_Count <= ret ||
        usb_slave.ep[rx_index].rx.EP_RX_Count <= usb_slave.usb_max_pack_size)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

        usb_slave.ep[rx_index].rx.EP_RX_Count = 0;
        usb_slave.ep[rx_index].rx.EP_RX_State = EP_RX_FINISH;

        if(usb_slave.ep[rx_index].rx.RX_Finish != AK_NULL)
            usb_slave.ep[rx_index].rx.RX_Finish(); 

        return ret;
    }
    
    usb_slave.ep[rx_index].rx.EP_RX_Count -= ret;
    usb_slave.ep[rx_index].rx.EP_RX_Buffer += ret;
    data = (T_U32 *)(usb_slave.ep[rx_index].rx.EP_RX_Buffer);
    if ((T_U32)data&0x3)
    {
        REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;
        return ret;
    }
    
    #ifdef DMA_TRANS_MODE
    if((usb_slave.mode == USB_MODE_20) && (usb_slave.ep[rx_index].rx.EP_RX_Count > 512))
    {
        ret = usb_slave.ep[EP_index].rx.EP_RX_Count;
        res = (ret / 512);
        ret = res*512;
        if (ret > MAX_DMA_SIZE)
            ret = MAX_DMA_SIZE;

        REG8(USB_REG_RXCSR2) |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);
        l2_set_status(usb_slave.ep[EP_index].rx.L2_Buf_ID,0);
        l2_trans_data_dma((T_U32)usb_slave.ep[rx_index].rx.EP_RX_Buffer, id_buf, ret);

        REG32(USB_DMA_ADDR_2) = 0x73000000;
        REG32(USB_DMA_COUNT_2) = ret;

        usb_slave.ep[rx_index].rx.bDmaStart = AK_TRUE;
            
        REG32(USB_DMA_CNTL_2) = (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(EP_index<<4)|USB_DMA_BUS_MODE3);
    }
    #endif

    REG8(USB_REG_RXCSR1) &= ~USB_RXCSR1_RXPKTRDY;

    return ret;
    #endif
}


/*******************************************************************************
 * @brief   write usb data with end point.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @param   [in]data: usb data buffer.
 * @param   [in]count: count to be send.
 * @return  T_U32
 * @retval  data in count
*******************************************************************************/
T_U32 usb_slave_data_in(EP_INDEX EP_index, T_U8 *data, T_U32 count)
{
    T_U32 tx_index = EP_index;
    T_U32 ret = 0;
    T_U32 *dat;
    
    //check EP_index
    if(EP0_INDEX == EP_index)
    {
        drv_print("usb_slave_data_in: error ep number:", EP_index, AK_TRUE);
        return 0;        
    }

    //check status
    if(usb_slave.ep[tx_index].tx.EP_TX_State == EP_TX_SENDING)
    {
        drv_print("usb_slave_data_in: still sending\n", 0, AK_FALSE);
        return 0;
    }

    usb_slave.ep[tx_index].tx.EP_TX_Buffer = data;
    usb_slave.ep[tx_index].tx.EP_TX_Count = count;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_SENDING;

    usb_slave_send_data(EP_index);

    return ret;
}


/*******************************************************************************
 * @brief   set usb slave stage.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]state
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_set_state(T_U8 stage)
{
    if((stage == USB_OK) && (usb_slave.state == USB_CONFIG))
    {
        usb_slave.state = stage;

        if(usb_slave.configok_callback != AK_NULL)
            usb_slave.configok_callback();
    }

    usb_slave.state = stage;
}


/*******************************************************************************
 * @brief   get usb slave stage.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   T_VOID
 * @return  T_U8
 * @retval  
*******************************************************************************/
T_U8 usb_slave_get_state(T_VOID)
{
    return usb_slave.state;
}
#pragma arm section code


static T_U32 usb_slave_ctrl_in(T_U8 *data, T_U32 len)
{
    T_U32 i;

    REG8(USB_REG_INDEX) = EP0_INDEX;

    if(0 == len)
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);
        return 0;
    }

    if(len > EP0_MAX_PAK_SIZE)
        len = EP0_MAX_PAK_SIZE;

    //write fifo
    for(i = 0; i < len; i++)
    {
        REG8(USB_FIFO_EP0) = data[i];
    }

    //set TXPKTRDY, if last packet set data end
    if(len < EP0_MAX_PAK_SIZE)
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY | USB_CSR0_P_DATAEND);
    }
    else
    {
        REG8(USB_REG_CSR0) |= (USB_CSR0_TXPKTRDY);
    }

    return len;
}

static T_U32 usb_slave_ctrl_out(T_U8 *data)
{
    T_U32 ret, i;

    //get rx data count
    REG8(USB_REG_INDEX) = EP0_INDEX;
    ret = REG16(USB_REG_RXCOUNT1);

    //read data from usb fifo
    for(i = 0; i < ret; i++)
    {
        data[i] = REG8(USB_FIFO_EP0);
    }

    return ret;
}

static T_BOOL usb_slave_ctrl_callback(T_U8 req_type)
{
    T_BOOL ret;
    T_CONTROL_TRANS *pTrans = (T_CONTROL_TRANS *)&usb_ctrl.ctrl_trans;
    
    if((REQUEST_STANDARD == req_type) && usb_ctrl.std_req_callback)
    {
        ret = usb_ctrl.std_req_callback(pTrans);
    }
    else if((REQUEST_CLASS == req_type) && usb_ctrl.class_req_callback)
    {
        ret = usb_ctrl.class_req_callback(pTrans);
    }
    else if((REQUEST_VENDOR == req_type) && usb_ctrl.vendor_req_callback)
    {
        ret = usb_ctrl.vendor_req_callback(pTrans);
    }

    if(!ret)
    {
        usb_slave_ep_stall(EP0_INDEX);
    }

    return ret;
}


/*******************************************************************************
 * @brief   write usb address.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]address: usb device address.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_set_address(T_U8 address)
{
    REG8(USB_REG_FADDR) = address;
}


/*******************************************************************************
 * @brief   set ep status.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @param   [in]bStall: stall or not.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_set_ep_status(T_U8 EP_Index, T_BOOL bStall)
{
    if(EP_Index >= MAX_EP_NUM)
        return;

    if(bStall)
        m_ep_status[EP_Index] = 1;
    else
        m_ep_status[EP_Index] = 0;
}


/*******************************************************************************
 * @brief   get ep status.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @return  T_VOID
*******************************************************************************/
T_U16 usb_slave_get_ep_status(T_U8 EP_Index)
{
    if(EP_Index >= MAX_EP_NUM)
        return 0;

    return m_ep_status[EP_Index];
}


/*******************************************************************************
 * @brief   stall ep
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_ep_stall(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(USB_EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_CSR0_P_SENDSTALL;
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, AK_TRUE);
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_P_SENDSTALL;
        usb_slave_set_ep_status(EP_index, AK_TRUE);
    }
}


/*******************************************************************************
 * @brief   clear stall
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_ep_clr_stall(T_U8 EP_index)
{
    REG8(USB_REG_INDEX) = EP_index;

    if(EP0_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &= (~USB_CSR0_P_SENTSTALL);
    }
    else if(EP2_INDEX == EP_index)
    {
        REG8(USB_REG_TXCSR1) &=(~(USB_TXCSR1_P_SENDSTALL|USB_TXCSR1_P_SENTSTALL));
        REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;
    }
    else if(EP3_INDEX == EP_index)
    {
        REG8(USB_REG_RXCSR1) &= (~(USB_RXCSR1_P_SENDSTALL | USB_RXCSR1_P_SENTSTALL));
        REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
    }
}

T_VOID usb_slave_clr_toggle(T_VOID)
{
    REG8(USB_REG_INDEX) = USB_EP2_INDEX;
    REG8(USB_REG_TXCSR1) |= USB_TXCSR1_CLRDATATOG;

    REG8(USB_REG_INDEX) = USB_EP3_INDEX;
    REG8(USB_REG_RXCSR1) |= USB_RXCSR1_CLRDATATOG;
}


#pragma arm section code = "_udisk_rw_"
//******************************************************************************
static T_VOID usb_slave_write_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 value)
{
    REG8(USB_REG_INDEX) = EP_index;
    REG16(reg) = value;
}
//******************************************************************************
static T_VOID usb_slave_read_ep_reg(T_U8 EP_index, T_U32 reg, T_U16 *value)
{
    REG8(USB_REG_INDEX) = EP_index;
    *value = REG16(reg);
}
//******************************************************************************
static T_VOID usb_slave_read_int_reg(T_U8 *value0, T_U16 *value1, T_U16 *value2)
{
    *value0 = REG8(USB_REG_INTRUSB);
    *value1 = REG16(USB_REG_INTRTX1);
    *value2 = REG16(USB_REG_INTRRX1);
}
//******************************************************************************

/*******************************************************************************
 * @brief   read data count of usb end point.
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   [in]EP_index: usb end point.
 * @param   [out]cnt: cnt data count.
 * @return  T_VOID
*******************************************************************************/
T_VOID  usb_slave_read_ep_cnt(T_U8 EP_index, T_U32 *cnt)
{
    REG8(USB_REG_INDEX) = EP_index;
    *cnt = REG16(USB_REG_RXCOUNT1);
}
#pragma arm section code


/*******************************************************************************
 * @brief   set usb controller to enter test mode
 * @author  liaozhijun
 * @date    2010-06-30
 * @param   testmode [in] T_U8 test mode, it can be one of the following value: 
 *
 *          Test_J              0x1
 *
 *          Test_K              0x2
 *
 *          Test_SE0_NAK        0x3
 *
 *          Test_Packet         0x4
 *
 *          Test_Force_Enable   0x5
 *
 * @return  T_VOID
*******************************************************************************/
T_VOID usb_slave_enter_testmode(T_U8 testmode)
{
    const unsigned char test_packet_data[64] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
    0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0x7E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };   //MENTOR DESIGNE    

    switch(testmode)
    {
    case Test_SE0_NAK:
        REG8(USB_REG_TESEMODE) = (1 << 0);
        break;

    case Test_J:
        REG8(USB_REG_TESEMODE) = (1 << 1);
        break;

    case Test_K:
        REG8(USB_REG_TESEMODE) = (1 << 2);
        break;

    case Test_Packet:
        {
            T_U32 i;

            // write data to usb fifo
            for (i = 0; i < 53; i++)
            {
                REG8(USB_FIFO_EP0) = test_packet_data[i];  
            }

            REG8(USB_REG_TESEMODE) = (1 << 3);

            REG8(USB_REG_CSR0) = USB_CSR0_TXPKTRDY;
            
            for (i = 0; i < 3; i++);
                break;
        }
        break;

    default:
        break;
    }
}


#pragma arm section code = "_udisk_rw_"
T_VOID  usb_slave_start_send(EP_INDEX EP_index)
{
    T_U32 tx_index = EP_index;

    usb_slave.ep[tx_index].tx.EP_TX_Count = 0;
    usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;
    usb_slave.usb_need_zero_packet = AK_FALSE;
}

T_U32 usb_slave_get_intr_type(T_U8 *usb_int, T_U16 *usb_ep_int_tx, T_U16 * usb_ep_int_rx)
{
    T_U16 usb_ep_csr;
    T_U32 tmp;
    T_U32 intr_ep = EP_UNKNOWN;
    T_U32 usb_dma_int;

    if(USB_NOTUSE == usb_slave.state)
    {
        return EP_UNKNOWN;
    }

    usb_dma_int = REG32(USB_DMA_INTR);
    
    if((usb_dma_int & DMA_CHANNEL1_INT) == DMA_CHANNEL1_INT)
    {
        //l2dma_wait_status();
        REG8(USB_REG_INDEX) = USB_EP2_INDEX;
        REG8(USB_REG_TXCSR2) = USB_TXCSR_MODE1;
        REG32(USB_DMA_CNTL_1) = 0;

        return EP2_DMA_INTR;
    }

    if((usb_dma_int & DMA_CHANNEL2_INT) == DMA_CHANNEL2_INT)
    {        
        //l2dma_wait_status();
        REG8(USB_REG_INDEX) = USB_EP3_INDEX;
        REG8(USB_REG_RXCSR2) = 0;
        REG32(USB_DMA_CNTL_2) = 0;

        return EP3_DMA_INTR;
    }    
    usb_slave_read_int_reg(usb_int, usb_ep_int_tx, usb_ep_int_rx);

    //common interrupt
    if (0 != (*usb_int & USB_INTR_RESET) ||
        0 != (*usb_int & USB_INTR_SUSPEND) ||
        0 != (*usb_int & USB_INTR_RESUME))
    {
        return USB_INTR;
    }

    //EP0 INTR
    if(0 != (*usb_ep_int_tx & USB_INTR_EP0))
    {
         intr_ep |= EP0_INTR;
    }
    
    //receive EP1 INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP3))
    {
        usb_slave_read_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, &usb_ep_csr);

        if (0 != (usb_ep_csr & USB_RXCSR1_RXPKTRDY))
        {
            intr_ep |= EP3_INTR;
        }
    
        //clear over run
        if (0 != (usb_ep_csr & USB_RXCSR1_P_OVERRUN))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_OVERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_RXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP3_INDEX, USB_REG_RXCSR1, ((~USB_RXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }
    
    //EP2 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP2))
    {
        usb_slave_read_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY))
        {
            intr_ep |= EP2_INTR;
        }

        //clear underrun
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP2_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }

    //EP1 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP1))
    {
        usb_slave_read_ep_reg(USB_EP1_INDEX, USB_REG_TXCSR1, &usb_ep_csr);

        if (0 == (usb_ep_csr & USB_TXCSR1_TXPKTRDY))
        {
            intr_ep |= EP1_INTR;
        }

        //clear underrun
        if (0 != (usb_ep_csr & USB_TXCSR1_P_UNDERRUN))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_UNDERRUN) & usb_ep_csr));
        }

        //clear stall
        if (0 != (usb_ep_csr & USB_TXCSR1_P_SENTSTALL))
        {
            usb_slave_write_ep_reg(USB_EP1_INDEX, USB_REG_TXCSR1, ((~USB_TXCSR1_P_SENTSTALL) & usb_ep_csr));
        }
    }

    //EP1 tx INT
    if(0 != (*usb_ep_int_tx & USB_INTR_EP3))
    {
        drv_print("EP3 TX INT!\n", 0, 1);
        return EP_UNKNOWN;
    }

    //EP2 rx INT
    if(0 != (*usb_ep_int_rx & USB_INTR_EP2))
    {
        drv_print("EP2 RX INT!\n", 0, 1);
        return EP_UNKNOWN;
    }

    return intr_ep;
}
#pragma arm section code


static T_VOID usb_slave_common_intr_handler(T_U8 usb_int)
{
    //RESET
    if(0 != (usb_int & USB_INTR_RESET))
    {
        //prepare to receive the enumeration
        usb_slave_reset();

        if(usb_slave.reset_callback != AK_NULL)
            usb_slave.reset_callback(usb_slave.mode);

        drv_print("R", 0, 1);
        
        return;
    }
    //SUSPEND
    else if(0 != (usb_int & USB_INTR_SUSPEND))
    {
        usb_slave_suspend();
        //enter the suspend mode
        if(USB_OK == usb_slave.state
        || USB_BULKIN == usb_slave.state
        || USB_BULKOUT == usb_slave.state)
        {
            drv_print("suspend in config,usb done\n", 0, 1);

            if(usb_slave.suspend_callback != AK_NULL)
                usb_slave.suspend_callback();

            usb_slave_set_state(USB_SUSPEND);
        }
        
        return;
    }
    //RESUME
    else if(0 != (usb_int & USB_INTR_RESUME))
    {
        usb_slave_set_state(USB_OK);

        if(usb_slave.resume_callback != AK_NULL)
            usb_slave.resume_callback();
        
        drv_print("RESUME\n", 0, 1);
        
        return;
    }
}

static T_VOID usb_slave_ep0_rx_handler(T_VOID)
{
    T_U32 data_len;
    T_U8 req_type;
    T_U8 data_dir;
    T_UsbDevReq *pDevReq = (T_UsbDevReq *)&usb_ctrl.ctrl_trans.dev_req;
    T_U32 i;
    T_U32 intr_ep;
    
    //stage: idle, setup packet comes
    if(CTRL_STAGE_IDLE == usb_ctrl.ctrl_trans.stage)
    {
        //receive data
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer);
        if(data_len != SETUP_PKT_SIZE)
        {
            drv_print("error setup packet size %d\r\n", data_len, AK_TRUE);
            return;
        }

        memcpy(&usb_ctrl.ctrl_trans.dev_req, usb_ctrl.ctrl_trans.buffer, SETUP_PKT_SIZE);

        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_SETUP;
        usb_ctrl.tx_count = 0;
        usb_ctrl.ctrl_trans.data_len = 0;

        //analysis bmRequest Type
        req_type = (pDevReq->bmRequestType >> 5) & 0x3;
        data_dir = pDevReq->bmRequestType >> 7;

        if(0 == pDevReq->wLength)
        {
            //callback
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            //no data stage
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;

            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }
        else
        {
            //callback
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
            
            if(!data_dir) //data out
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_OUT;
            }
            else         //data in
            {
                usb_ctrl.ctrl_trans.stage = CTRL_STAGE_DATA_IN;
               //start send
                data_len = usb_ctrl.ctrl_trans.data_len;
                if(data_len > EP0_MAX_PAK_SIZE)
                {
                    data_len = EP0_MAX_PAK_SIZE;
                }
                else if(data_len < EP0_MAX_PAK_SIZE)
                {
                    usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
                }

                usb_ctrl.tx_count = data_len;
                usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer, data_len);

                if(data_len < EP0_MAX_PAK_SIZE)
                {
                    for(i = 0; i<1000000; i++)
                    {
                        intr_ep = usb_slave_intr_handler_inner();
                        if((intr_ep & EP0_INTR) == EP0_INTR)
                        {
                            //drv_print("", '&', AK_TRUE);
                            return;
                        }
                    }
                } 
            }
        }

        return;
    }

    if(CTRL_STAGE_DATA_OUT == usb_ctrl.ctrl_trans.stage)
    {
        data_len = usb_slave_ctrl_out(usb_ctrl.ctrl_trans.buffer + usb_ctrl.ctrl_trans.data_len);
        usb_ctrl.ctrl_trans.data_len += data_len;

        if(data_len < EP0_MAX_PAK_SIZE || usb_ctrl.ctrl_trans.data_len > usb_ctrl.ctrl_trans.dev_req.wLength)
        {
            //callback
            req_type = (pDevReq->bmRequestType >> 5) & 0x3;
            if(!usb_slave_ctrl_callback(req_type))
            {
                return;
            }

            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;

            //last packet
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY | USB_CSR0_P_DATAEND;

            for(i = 0; i<1000000; i++)
            {
                intr_ep = usb_slave_intr_handler_inner();
                if((intr_ep & EP0_INTR) == EP0_INTR)
                {
                    //drv_print("", '!', AK_TRUE);
                    return;
                }
            }
        }
        else
        {
            REG8(USB_REG_INDEX) = EP0_INDEX;
            REG8(USB_REG_CSR0) |= USB_CSR0_P_SVDRXPKTRDY;
        }
    }
}

static T_VOID usb_slave_ep0_tx_handler(T_VOID)
{
    T_U8 req_type;
    T_U32 data_trans;

    req_type = (usb_ctrl.ctrl_trans.dev_req.bmRequestType >> 5) & 0x3;

    if(CTRL_STAGE_DATA_IN == usb_ctrl.ctrl_trans.stage)
    {
        data_trans = usb_ctrl.ctrl_trans.data_len - usb_ctrl.tx_count;
        if(data_trans > EP0_MAX_PAK_SIZE)
        {
            data_trans = EP0_MAX_PAK_SIZE;
        } 
        else if(data_trans < EP0_MAX_PAK_SIZE)
        {
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_STATUS;
        }

        //send data
        usb_slave_ctrl_in(usb_ctrl.ctrl_trans.buffer + usb_ctrl.tx_count, data_trans);
        usb_ctrl.tx_count += data_trans;

        return;
    }

    if(CTRL_STAGE_STATUS == usb_ctrl.ctrl_trans.stage)
    {
        usb_slave_ctrl_callback(req_type);

        usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
    }
}


T_VOID usb_slave_ep0_intr_handler(T_U16 usb_ep_int_tx)
{
    T_U16 usb_ep_csr;
    T_BOOL bError = AK_FALSE;
    
    //because EP0's all interrupt is in USB_REG_INTRTX1
    if(0 != (usb_ep_int_tx & USB_INTR_EP0))
    {
        usb_slave_read_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, &usb_ep_csr);

        //setup end
        if (0 != (usb_ep_csr & USB_CSR0_P_SETUPEND))
        {
            usb_slave_write_ep_reg(USB_EP0_INDEX, USB_REG_CSR0, USB_CSR0_P_SVDSETUPEND);
            bError = AK_TRUE;

            //back to idle stage after steup end
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE;
        }
        //stall
        else if(0 != (usb_ep_csr & USB_CSR0_P_SENTSTALL))
        {
            //clear stall
            usb_slave_ep_clr_stall(EP0_INDEX);
            bError = AK_TRUE;

            //back to idle stage after stall
            usb_ctrl.ctrl_trans.stage = CTRL_STAGE_IDLE; 
        }
        //rec pkt
        if (0 != (usb_ep_csr & USB_CSR0_RXPKTRDY))
        {
            usb_slave_ep0_rx_handler();
        }
        //send pkt or status end
        else if(!bError)
        {
            usb_slave_ep0_tx_handler();
        }
    }
}


#pragma arm section code = "_udisk_rw_"
static T_VOID usb_slave_tx_handler(EP_INDEX EP_index)
{
    T_U32 tx_index = EP_index;

    if (usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_SENDING)
    {
        return;
    }

    if (usb_slave.ep[tx_index].tx.EP_TX_Count == 0)
    {
        if (usb_slave.ep[tx_index].tx.EP_TX_State != EP_TX_FINISH)
        {
            usb_slave.ep[tx_index].tx.EP_TX_State = EP_TX_FINISH;

            if (usb_slave.ep[tx_index].tx.TX_Finish != AK_NULL)
                usb_slave.ep[tx_index].tx.TX_Finish();
        }
        return;
    }

    usb_slave_send_data(EP_index);
}


static T_VOID usb_slave_rx_handler(EP_INDEX EP_index)
{
    T_U32 i;
    T_U32 *p32;
    T_U16 ret;
    T_U8 *p8;

    T_U32 rx_index = EP_index;

    if (usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
    {
        if (usb_slave.ep[rx_index].rx.RX_Notify != AK_NULL)
            usb_slave.ep[rx_index].rx.RX_Notify();
    }
    else
    {
        #ifdef DMA_TRANS_MODE
            usb_slave_dma_receive(rx_index); 
        #else    
            usb_slave_receive_data(rx_index);
        #endif
        
        if (usb_slave.ep[rx_index].rx.EP_RX_State == EP_RX_FINISH)
        {
            if (usb_slave.ep[rx_index].rx.RX_Finish != AK_NULL)
                usb_slave.ep[rx_index].rx.RX_Finish();
        }
    }
}

T_VOID usb_slave_poll_status(T_VOID)
{
    usb_slave_intr_handler_inner();
}


T_VOID usb_slave_intr_handler(T_VOID)
{
    usb_slave_intr_handler_inner();
}

T_U32 usb_slave_intr_handler_inner(T_VOID)
{
    T_U8 usb_int;
    T_U16 usb_ep_csr;
    T_U32 tmp, usb_dma_int;
    T_U16 usb_ep_int_tx;
    T_U16 usb_ep_int_rx;
    T_U32 intr_ep = EP_UNKNOWN;
    T_S32 status;

    intr_ep = usb_slave_get_intr_type(&usb_int, &usb_ep_int_tx, &usb_ep_int_rx);

    if((intr_ep & USB_INTR) == USB_INTR)
    {
        usb_slave_common_intr_handler(usb_int);
    }
    
    if((intr_ep & EP0_INTR) == EP0_INTR)
    {
        usb_slave_ep0_intr_handler(usb_ep_int_tx);
    }
    
    if((intr_ep & EP1_INTR) == EP1_INTR)
    {
        usb_slave_tx_handler(EP1_INDEX);
    }
    
    if((intr_ep & EP2_INTR) == EP2_INTR)
    {
        usb_slave_tx_handler(EP2_INDEX);
    }

    if((intr_ep & EP2_DMA_INTR) == EP2_DMA_INTR)
    {
        usb_slave.ep[EP2_INDEX].tx.bDmaStart = AK_FALSE;
        usb_slave_tx_handler(EP2_INDEX);
    }

    #ifdef DMA_TRANS_MODE
    if ((intr_ep & EP3_DMA_INTR) == EP3_DMA_INTR)
    {
        usb_slave.ep[EP3_INDEX].rx.bDmaStart = AK_FALSE;
        tmp = REG32(USB_DMA_ADDR_2) - 0x73000000;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += tmp;
        usb_slave.ep[EP3_INDEX].rx.EP_RX_Count -= tmp;

        if (0 == usb_slave.ep[EP3_INDEX].rx.EP_RX_Count)
        {
            usb_slave.ep[EP3_INDEX].rx.EP_RX_State = EP_RX_FINISH;
            if (usb_slave.ep[EP3_INDEX].rx.RX_Finish != AK_NULL)
                usb_slave.ep[EP3_INDEX].rx.RX_Finish();
        }

        REG8(USB_REG_INDEX) = USB_EP3_INDEX;
        REG8(USB_REG_RXCSR2) = 0;
        REG32(USB_DMA_CNTL_2) = 0;
        REG32(USB_DMA_COUNT_2) = 0;
    }
    #endif

    if ((intr_ep & EP3_INTR) == EP3_INTR)
    {
    #ifdef DMA_TRANS_MODE
        if (usb_slave.ep[EP3_INDEX].rx.bDmaStart)
        {
            usb_slave.ep[EP3_INDEX].rx.EP_RX_Buffer += REG32(USB_DMA_ADDR_2) - 0x73000000;       
            usb_slave.ep[EP3_INDEX].rx.EP_RX_Count -= REG32(USB_DMA_ADDR_2) - 0x73000000;
            usb_slave.ep[EP3_INDEX].rx.bDmaStart = AK_FALSE;
            REG8(USB_REG_RXCSR2) = 0;
            REG32(USB_DMA_CNTL_2) = 0;
            REG32(USB_DMA_COUNT_2) = 0;
        }
    #endif
        usb_slave_rx_handler(EP3_INDEX);
    }

    return intr_ep;
}
#pragma arm section code




/*******************************************************************************
 * @brief   get usb cable connect to pc or not
 * @author  
 * @date    
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE connect to pc
 * @retval  AK_FALSE not connect to pc
*******************************************************************************/
T_BOOL usb_slave_detect(T_VOID)
{
    T_BOOL stat = AK_FALSE;
    T_U32 tbegin, tend;

    usb_slave_detect_init();

    tbegin = get_tick_count_ms();

    while(1)
    {
        if(usb_slave_detect_status())
        {
            stat = AK_TRUE;
            break;
        }

        tend = get_tick_count_ms();
        if(tend < tbegin)
        {
            tbegin = 0;
        }
        else if((tend - tbegin) > 1000)
        {
            break;
        }
    }

    usb_slave_detect_exit();

    return(stat);
}


#pragma arm section code = "_drvbootcode_"
static T_VOID usb_slave_get_status(T_TIMER timer_id, T_U32 delay)
{
    T_BOOL usb_stat = AK_FALSE;

    usb_stat = usb_slave_detect_status();
    detect_time += delay;

    if ((AK_TRUE == usb_stat) || (detect_time >= 1000))
    {
        timer_stop(timer_id);
        usb_slave_detect_exit();
        usb_slave_send_message(usb_stat);
    }
}
#pragma arm section code


T_VOID usb_slave_detect_start(T_pVOID callback)
{
    detect_time = 0;
    usb_slave_detect_init();

    timer_start(50, AK_TRUE, usb_slave_get_status);
}


T_VOID usb_slave_detect_init(T_VOID)
{
    T_U32 tmp;

    if (INVALID_ASIC_INDEX != asic_id)
    {
        return;
    }
    asic_id = clk_request_min_asic(29000000);
    usb_connect_pc = AK_FALSE;

    //enable clock, USB PLL, USB 2.0
    sys_module_enable(eVME_USB_CLK, AK_TRUE);

    //enable the usb transceiver and suspend enable
    REG32(REG_MUL_FUNC_CTRL) |= (PHY_RST | PHY_SUS);
    REG32(REG_MUL_FUNC_CTRL) &= ~(PHY_RST | PHY_LDO);

    //enable the usb phy
    REG8(USB_REG_CFG) = 0xd7;

    //set to high speed
    REG8(USB_REG_POWER) = 0x20;
}


#pragma arm section code = "_drvbootcode_"
T_BOOL usb_slave_detect_status(T_VOID)
{
    T_U8 usb_int;

    if (INVALID_ASIC_INDEX == asic_id)
    {
        return usb_connect_pc;
    }

    usb_int = REG8(USB_REG_INTRUSB);
    if (((usb_int & USB_INTR_SOF) == USB_INTR_SOF)
        || (AK_TRUE == usb_connect_pc))
    {
        usb_connect_pc = AK_TRUE;
        return AK_TRUE;
    }

    return AK_FALSE;
}


T_VOID usb_slave_detect_exit(T_VOID)
{
    if (INVALID_ASIC_INDEX == asic_id)
    {
        return;
    }

    REG8(USB_REG_POWER) = 0;

    //disable usb transcieve
    REG32(REG_MUL_FUNC_CTRL) &= ~PHY_SUS;

    //close usb clock
    sys_module_enable(eVME_USB_CLK, AK_FALSE);
    sys_module_reset(eVME_USB_CLK);

    clk_cancel_min_asic(asic_id);
    asic_id = INVALID_ASIC_INDEX;
}
#pragma arm section code


T_VOID usb_slave_suspend_controller(T_BOOL flag)
{
    if(AK_FALSE == flag)
    {
        REG32(REG_MUL_FUNC_CTRL) |= ((1<<2) | (1<<4));
    }
    else
    {
        REG32(REG_MUL_FUNC_CTRL) &= ~((1<<2) | (1<<4));//REG_MUL_FUNC_CTRL中的第4位是保留的
    }
}


#ifdef BURN_TOOL
T_U32 usb_slave_get_mode(T_VOID)
{
    //check which speed now on
    if((REG8(USB_REG_POWER) & USB_POWER_HSMODE) == USB_POWER_HSMODE)
    {
        drv_print("high speed now", 0, 1);
        return USB_MODE_20;
    }
    else
    {
        drv_print("full speed now", 0, 1);
        return USB_MODE_11;
    }
}
#endif


