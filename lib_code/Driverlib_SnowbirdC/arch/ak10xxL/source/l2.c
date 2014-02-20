/**
 * @file    l2.h
 * @brief   the interface for the l2
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.12.19
 * @version 1.0
 */


#include <string.h>
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "interrupt.h"
#include "drv_api.h"
#include "arch_init.h"
#include "hal_errorstr.h"
#include "drv_cfg.h"

#define L2_SMALL_BUF_SIZE       128
#define L2_BIG_BUF_SIZE         512

#define INVALID_BUF_ID          3


typedef struct
{
    T_U8    device_id;      //device < 16, id < 16
    T_U8    len_4div;
    T_U16   addr_128div;
}T_DEV_BUF_INFO;

#pragma arm section zidata = "_drvbootbss_"
T_U32                   local_l2_4k;
static T_U16            buf_bitmap;
static T_U16            l2_intr_type;
static T_U8     device_buffer_id[DEVICE_NUM];
//static T_DEV_BUF_INFO   dev_buf_info_cache;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_L2;
#pragma arm section rodata



#pragma arm section code = "_drvbootcode_" 



/**
 * @brief       get the address of the buffer specialed by buf_id.
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   buf_id
 * @return      T_U32 
 * @retval      the address of the buffer in L2.
 * @remark      the address is not the absolute address, and is the
 *              relative address in L2.
 *              absolute address = retval + L2_START_ADDR
 */ 
static T_U32 buf_id_to_buf_addr(T_U8 buf_id)
{
    T_U32 addr;

    if(INVALID_BUF_ID >= buf_id)
    {
        addr = BUF_ADDR_BASE + (buf_id << 9);   //device * 512
    }
    else
    {
        buf_id = buf_id - 4;
        addr = BUF_ADDR_BASE + (buf_id << 7);   //device * 128
    }

    return addr;
}


/**
 * @brief       get the config register about the device
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device 
 * @param[in]   preg, pointer to a T_U32 type variable for fetching the 
 *              register address.
 * @param[in]   poffset, pointer to a T_U32 type variable for fetching the 
 *              offset in the register.
 * @return      T_VOID
 */ 
static T_VOID device_map_reg_offset(DEVICE_SELECT device, T_U32 *preg, 
    T_U8 *pbuffer_id_offset, T_U8 *pbuffer_size_offset)
{
    T_U32   buf_cfg_reg;
    T_U8   buffer_id_offset;


    if(device <= ADDR_SPI2_TX)
    {
        buf_cfg_reg = REG_BUFFER_CONFIG_1;
        buffer_id_offset      = device;
    }
    else if(device <= ADDR_SPI2_RX)
    {
        buf_cfg_reg = REG_BUFFER_CONFIG_2;
        buffer_id_offset      = device - ADDR_USB0;
    }
    else
    {
        buf_cfg_reg = REG_BUFFER_CONFIG_3;
        buffer_id_offset      = device - ADDR_PCM_TX;
    }
    
    *preg       = buf_cfg_reg;

    /*uarts only take 4 bits to configure buffer id, 
        don't need to configure buffer size
        */
    switch (device)
    {
        case ADDR_UART1_TX:
        case ADDR_UART1_RX:
            
#if (CHIP_SEL_10C > 0)
        case ADDR_UART2_TX:
        case ADDR_UART2_RX:
#endif
            *pbuffer_id_offset = UART1TX_BUF_SEL 
                + ((device - ADDR_UART1_TX) << 2);
            *pbuffer_size_offset = 0xFF;
            break;
        default:    
            *pbuffer_id_offset    = buffer_id_offset * 5;
            *pbuffer_size_offset  = *pbuffer_id_offset + 4; 
            break;
    }
    
}

/**
 * @brief       get the buffer id of device.
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device
 * @return      T_U32 
 * @retval      the buffer id of the device.
 */ 
static T_U8 get_device_buf_id(DEVICE_SELECT device)
{    
    T_U8    buf_id;

    if((ADDR_MCI1 == device) || (ADDR_MCI2 == device))
    {
        if(((REG32(REG_L2_CONFIG) >> MASS_MEM_SEL) & 0x3) != (device - ADDR_MCI1  + 1))
        {
            return INVALID_BUF_ID;
        }

        device = ADDR_SDMMC;
    }
    
    return device_buffer_id[device];
}

/**
 * @brief       set the buffer id of device.
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device  
 * @param[in]   buf_id 
 * @return      T_VOID 
 */ 
static T_VOID set_device_buf_id(DEVICE_SELECT device, T_U8 buf_id)
{
    T_U32   buf_cfg_reg;
    T_U32   buf_cfg_value;
    T_U8    buffer_id_offset;
    T_U8    buffer_size_offset;
    T_U8    buffer_id_cfg;
    T_BOOL  buffer_big_size;

    if((ADDR_MCI1 == device) || (ADDR_MCI2 == device))
    {
        buf_cfg_value = REG32(REG_L2_CONFIG);
        buf_cfg_value &= ~(3 << MASS_MEM_SEL);
        buf_cfg_value |= (device - ADDR_MCI1 + 1) << MASS_MEM_SEL;
        REG32(REG_L2_CONFIG) = buf_cfg_value;
        device = ADDR_SDMMC;
    }
    
    device_buffer_id[device] = buf_id;
    device_map_reg_offset(device, &buf_cfg_reg, 
        &buffer_id_offset, &buffer_size_offset);
    buf_cfg_value = REG32(buf_cfg_reg);
    if(REG_BUFFER_CONFIG_1 == buf_cfg_reg)//a chip bug
    {
        buf_cfg_value &= ~(0x1F << SPI1RX_BUF_SEL);
        buf_cfg_value |= ((device_buffer_id[ADDR_SPI1_RX] - 4) << SPI1RX_BUF_SEL);
    }
    
    if (buf_id <= INVALID_BUF_ID)
    {
        buffer_id_cfg = buf_id << 2;
        buffer_big_size = AK_TRUE;
    }
    else
    {
        buffer_id_cfg = buf_id - 4;
        buffer_big_size = AK_FALSE;
    }

    if (0xFF != buffer_size_offset)
    {
        if (buffer_big_size)
            buf_cfg_value |= (1 << buffer_size_offset);
        else
            buf_cfg_value &= ~(1 << buffer_size_offset);
    }
    
    buf_cfg_value &= ~(0xF << buffer_id_offset);    
    buf_cfg_value |= (buffer_id_cfg << buffer_id_offset);
    
    REG32(buf_cfg_reg) = buf_cfg_value;
}

/**
 * @brief       get a free buffer, from the buffer bitmap
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   bBigSize, 512 size buffer or 128 size buffer
 * @param[in]   pbuf_id, pointer to a T_U8 type variable for fetching the 
 *              buffer id.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */ 
static T_BOOL get_free_buf(T_BOOL bBigSize, T_U8 *pbuf_id)
{
    T_U16   mask;
    T_U8 big = 0, small = 12;

    do
    {   
        if (bBigSize)// buffer 0~2 occupy 4 bits
        {
            mask = 0xF << (big << 2);            
            big++;
        }
        else  // buffer 15~ 4 occupy 1 bit
        {
            small--;
            mask = 1 << small;
        }

        if (0 == (buf_bitmap & mask))
        {
            buf_bitmap |= mask;
            *pbuf_id = bBigSize ? (big - 1) : (small + 4);
            return AK_TRUE;
        }
        
    } while (0 != small && INVALID_BUF_ID != big);

    return AK_FALSE;
}
    

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
                           T_U32 *pbuf_len, T_U32 *pbuf_id)
{
#if 0
    if((0 != dev_buf_info_cache.len_4div) && (device == (dev_buf_info_cache.device_id >> 4)))
    {
        *pbuf_addr = (dev_buf_info_cache.addr_128div << 7);
        *pbuf_len  = (dev_buf_info_cache.len_4div << 2);
        *pbuf_id   = (dev_buf_info_cache.device_id & 0xF);
        return ;
    }
#endif

    *pbuf_id = get_device_buf_id(device);
    if(INVALID_BUF_ID == *pbuf_id)
    {
        return AK_FALSE;
    }
    if(INVALID_BUF_ID < *pbuf_id)
    {
        *pbuf_len = L2_SMALL_BUF_SIZE;
    }
    else
    {
        *pbuf_len = L2_BIG_BUF_SIZE;
    }

    *pbuf_addr = buf_id_to_buf_addr(*pbuf_id) + L2_START_ADDR;

#if 0
    dev_buf_info_cache.device_id    = (device << 4) | (*pbuf_id);
    dev_buf_info_cache.len_4div     = (*pbuf_len >> 2);
    dev_buf_info_cache.addr_128div  = (*pbuf_addr) >> 7;
#endif

    return AK_TRUE;
}

/**
 * @brief       enable or disable the L2 interrupt
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   enable, AK_TRUE, enable; AK_FALSE, disable 
 * @return      T_VOID
 */ 
T_VOID l2_int_ctrl(T_BOOL enable)
{
    if (enable)
        INT_ENABLE(INT_EN_L2);
    else
        INT_DISABLE(INT_EN_L2);
}

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
T_VOID l2_set_status(T_U8 buf_id, T_U8 buf_status)
{
    T_U32 reg_value;

    irq_mask();
    reg_value = REG32(REG_L2_CONFIG);
    reg_value &= ~((0x0F << CPU_BUF_SEL) | (0xf << CPU_BUF_NUM));
    reg_value |= (buf_id << CPU_BUF_SEL) | CPU_SEL_EN | 
                 ((buf_status & 0x0F) << CPU_BUF_NUM);
    REG32(REG_L2_CONFIG) = reg_value;

    reg_value &= ~CPU_SEL_EN;

    REG32(REG_L2_CONFIG) = reg_value;
    irq_unmask();
}

/**
 * @brief       get the status of the buffer
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   buf_id 
 * @return      T_U8
 * @retval      the status value of the buffer specialed by buf_id
 */
T_U8 l2_get_status(T_U8 buf_id)
{
    if(buf_id <= INVALID_BUF_ID)
    {
        return (REG32(REG_BUFFER_STATUS_1) >> (buf_id << 2)) & 0x0F;
    }
    else
    {
        buf_id = buf_id - 4;
        return (REG32(REG_BUFFER_STATUS_2) >> (buf_id << 1)) & 0x03;
    }
}


/**
 * @brief       get the interrupt type of the ldma
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID
 * @return      T_U8
 * @retval      the DEVICE_SELECT which cause the interrupt
 */
T_U16 l2_get_intr_type(T_VOID)
{
    return l2_intr_type;
}


static T_BOOL wait_buf_not_full(T_U8 buf_id, T_U8 buf_blocks)
{
    T_U32           uCount;

    uCount = 0;
    while(l2_get_status(buf_id) == buf_blocks)
    {
        if(uCount++ > 200000)// 2s
        {
            drv_print(err_str, __LINE__,AK_TRUE);
            return AK_FALSE;
        }
        delay_us(10);
    }

    return AK_TRUE;
}

static T_BOOL wait_buf_not_empty(T_U8 buf_id)
{
    T_U32           uCount;

    uCount = 0;
    while(l2_get_status(buf_id) == 0)
    {
        if(uCount++ > 200000)// 2s
        {
            drv_print(err_str, __LINE__,AK_TRUE);
            return AK_FALSE;
        }
        delay_us(10);
    }

    return AK_TRUE;
}


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
 * @return      T_U8
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */
T_BOOL l2_trans_data_cpu(T_U32 dest_addr, T_U32 src_addr, T_U32 size, T_U32 tran_offset)
{
    T_U32           ram_addr;
    T_U32           ram_len;
    DEVICE_SELECT   device;
    T_BOOL          bram2dev;
    T_BOOL          bret;

    T_U32           buf_addr;
    T_U32           buf_len;
    T_U32           buf_id;

    T_U32           ram_blocks;
    T_U32           buf_blocks;
    T_U32           tmp;
    T_U32           i,j;



    if(src_addr < DEVICE_NUM)
    {
        ram_addr    = dest_addr;
        device      = src_addr;
        bram2dev    = AK_FALSE;
    }
    else
    {
        ram_addr    = src_addr;
        device      = dest_addr;
        bram2dev    = AK_TRUE;
    }

    
    get_device_buf_info(device, &buf_addr, &buf_len, &buf_id);

    ram_len    = size;
    ram_blocks = ram_len >> 6;
    buf_blocks = buf_len >> 6;

    buf_addr += tran_offset;
    j = 0;
    
    for(i=0; i<ram_blocks; ++i)
    {
        if(bram2dev)
        {
            if(AK_FALSE == wait_buf_not_full(buf_id, buf_blocks))
            {
                return AK_FALSE;
            }
            
            memcpy((T_VOID *)(buf_addr+j), (T_VOID *)ram_addr, 60);
            memcpy(&tmp, (T_VOID *)(ram_addr + 60), 4);
            //memcpy((T_VOID *)(buf_addr+j+60), &tmp, 4);
            *(T_U32 *)(buf_addr+j+60) = tmp;
            ram_addr += 64;
            j += 64;
            if(j >= buf_len)
            {
                j = 0;
            }
        }
        else
        {
            if(AK_FALSE == wait_buf_not_empty(buf_id))
            {
                return AK_FALSE;
            }
            
            memcpy((T_VOID *)(ram_addr), (T_VOID *)(buf_addr+j), 60);
            memcpy(&tmp, (T_VOID *)(buf_addr+j+60), 4);
            memcpy((T_VOID *)(ram_addr + 60), &tmp, 4);
            ram_addr += 64;
            j += 64;
            if(j >= buf_len)
            {
                j = 0;
            }
        }
    }


    ram_len = ram_len & (64 - 1);
    if(ram_len)
    {
        if(bram2dev)
        {
            if(AK_FALSE == wait_buf_not_full(buf_id, buf_blocks))
            {
                return AK_FALSE;
            }

            if(ram_len > 60)
            {
                i = 60;
                ram_len -= 60;
            }
            else
            {
                i = ram_len;
                ram_len = 0;
            }
            
            memcpy((T_VOID *)(buf_addr+j), (T_VOID *)(ram_addr), i);
            if(ram_len)
            {
                memcpy(&tmp, (T_VOID *)(ram_addr + i), ram_len);
            }
            //memcpy((T_VOID *)(buf_addr+j+60), &tmp, 4);
            *(T_U32 *)(buf_addr+j+60) = tmp;
        }
        else
        {
            i = ram_len & (~3);
            if(i)
            {
                memcpy((T_VOID *)(ram_addr), (T_VOID *)(buf_addr+j), i);
            }
            
            memcpy(&tmp, (T_VOID *)(buf_addr+j+i), 4);
            ram_len = ram_len & 3;
            memcpy((T_VOID *)(ram_addr+i), &tmp, ram_len);
        }
    }

    return AK_TRUE;
}


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
T_BOOL l2_init_device_buf(DEVICE_SELECT device)
{
    T_U8    buf_id;
    T_BOOL  bBigSize;
    T_U32   reg_value;


    buf_id = get_device_buf_id(device);
    if(INVALID_BUF_ID != buf_id)
        return AK_TRUE;

    switch(device)
    {
        case ADDR_UART1_TX:
        case ADDR_UART1_RX:
#if (CHIP_SEL_10C > 0)
        case ADDR_UART2_TX:
        case ADDR_UART2_RX:
#endif            
        case ADDR_SPI1_RX:
        //因为SPI CAMERA一般是用SPI2，且必须是512字节
#if (!((DRV_SUPPORT_CAMERA > 0) && (CAMERA_GC6113 > 0)))
        case ADDR_SPI2_RX:
#endif

            bBigSize = AK_FALSE;
            break;
        default:
            bBigSize = AK_TRUE;
            break;
    }

    if(AK_FALSE == get_free_buf(bBigSize, &buf_id))
    {
        if(AK_FALSE == ((AK_TRUE == bBigSize) && get_free_buf(AK_FALSE, &buf_id)))
        {  
            drv_print(err_str, __LINE__, AK_TRUE);
            return AK_FALSE;
        }
    }

    set_device_buf_id(device, buf_id);
    l2_set_status(buf_id, 0);
    REG32(REG_BUFFER_ENABLE) |= (1 << buf_id);

    return AK_TRUE;
}

#if (DRV_SUPPORT_BLUETOOTH > 0)
T_BOOL l2_trans_cpu_for_bt(T_U32 dest_addr, T_U32 src_addr, T_U32 size, T_U32 tran_offset)
{
    T_U32           ram_addr;
    T_U32           ram_len;
    DEVICE_SELECT   device;
    T_BOOL          bram2dev;
    T_BOOL          bret;

    T_U32           buf_addr;
    T_U32           buf_len;
    T_U32           buf_id;

    T_U32           ram_blocks;
    T_U32           buf_blocks;
    T_U32           tmp;
    T_U32           i,j,k;



    if(src_addr < DEVICE_NUM)
    {
        ram_addr    = dest_addr;
        device      = src_addr;
        bram2dev    = AK_FALSE;
    }
    else
    {
        ram_addr    = src_addr;
        device      = dest_addr;
        bram2dev    = AK_TRUE;
    }

    
    get_device_buf_info(device, &buf_addr, &buf_len, &buf_id);

    ram_len    = size;
    ram_blocks = ram_len >> 6;
    buf_blocks = buf_len >> 6;

    buf_addr += tran_offset;
    j = 0;

    if ((ram_addr&3)||(size&3))
    {
        drv_print(err_str, __LINE__,AK_TRUE);
        while(1);
    }

    //if size if not multipy of 64, or large than remain space in buffer
    //stop here
    if((ram_len&0x3f) || (tran_offset+size > buf_len))
    {
        drv_print(err_str, __LINE__,AK_TRUE);
        while(1);
    }
    
    for(i=0; i<ram_blocks; ++i)
    {
        if(AK_FALSE == wait_buf_not_empty(buf_id))
        {
            drv_print(err_str, __LINE__,AK_TRUE);
            while(1);
        }

        //we just get right channel adc data, for spec use in bt
        for (k = 0; k < 64/4; k++)
        {
            tmp = REG32(buf_addr + k*4);
            REG16(ram_addr + k*2) = (T_U16)(tmp&0xffff);
        }
        
        ram_addr += 32;
        buf_addr += 64;

    }


    return AK_TRUE;
}
#endif

static T_BOOL check_dma_address(T_U32 vaddr, T_U32 size)
{
    T_BOOL ret = AK_TRUE;
    
    if ((vaddr & 0x3) || (size > 0x1000))
    {
        ret = AK_FALSE;
    }

    if ((0 == MMU_Vaddr2Paddr(vaddr))
        ||((MMU_Vaddr2Paddr(vaddr) + size - 4) 
        != MMU_Vaddr2Paddr(vaddr + size - 4)))
    {
        drv_print(err_str, __LINE__, AK_FALSE);
        ret = AK_FALSE;
    }
    
    return ret;
}

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
T_BOOL l2_trans_data_dma(T_U32 dest_addr, T_U32 src_addr, T_U32 size)
{
    T_U32   buf_addr;
    T_U32   buf_len;
    T_U32   buf_id;
    T_U32   tmp;
    DEVICE_SELECT device;
    
    tmp = 0;
    while (REG32(REG_LDMA_COUNT) & LDMA_OPT_EN)
    {        
        if(tmp++ > 100000)
        {
            drv_print(err_str, __LINE__,AK_TRUE);
            return AK_FALSE;
        }
        delay_us(10);
    }
        
    if (dest_addr > DEVICE_NUM)
    {
        tmp = dest_addr;
        device = src_addr;
    }
    else
    {
        tmp = src_addr;
        device = dest_addr;
    }
        
    if (AK_FALSE == check_dma_address(tmp, size))
    {
        drv_print(err_str, __LINE__, AK_FALSE);
        drv_print(AK_NULL, tmp, AK_TRUE);
        return AK_FALSE;
    }
    
    if (AK_FALSE == get_device_buf_info(device, &buf_addr, &buf_len, &buf_id))
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }
    
    buf_len = buf_len >> 2;
    buf_addr = buf_addr - L2_START_ADDR;
    l2_intr_type = device;
    tmp = MMU_Vaddr2Paddr(tmp);
    MMU_InvalidateDCache();
    
    if (dest_addr > DEVICE_NUM)
    {
        REG32(REG_LDMA_SRC) = buf_addr | WRAP_EN_SRC | (buf_len << WRAP_SIZE_SRC);
        REG32(REG_LDMA_DEST) = tmp & ((1 << 18) - 1);
    }
    else
    {
        REG32(REG_LDMA_SRC) = tmp & ((1 << 18) - 1);
        REG32(REG_LDMA_DEST) = buf_addr | WRAP_EN_DEST | (buf_len << WRAP_SIZE_DEST);
    }

    REG32(REG_LDMA_COUNT) = (size >> 2) | LDMA_OPT_EN;
    REG32(REG_L2_CONFIG) |= LDMA_VLD_INT_EN;

    return AK_TRUE;
}


/**
 * @brief       disable the ldma interrupt
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID ldma_int_disable(T_VOID)
{
    REG32(REG_L2_CONFIG) &= ~LDMA_VLD_INT_EN;
}


#pragma arm section code

#pragma arm section code = "_drvbootinit_"
T_VOID set_l2_peripheral_buf(T_U32 addr)
{
    local_l2_4k = (addr - L2_START_ADDR) >> 12;
    l2_initial();
}


/**
 * @brief       init the L2
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID l2_initial(T_VOID)
{
    T_U8 i;
    
    REG32(REG_L2_CONFIG) = CURRENT_CNT_INT_EN | (LOCAT_L2_4K << FIX_MADDR_H) \
                    | (0 << MASS_MEM_SEL) | AHB_FLAG_EN;

    buf_bitmap      = 0;
    l2_intr_type    = DEVICE_NUM;

    for (i = 0; i < DEVICE_NUM; i++)
        device_buffer_id[i] = INVALID_BUF_ID;

    //all device select the INVALID_BUF_ID which is 3
    REG32(REG_BUFFER_CONFIG_1) = ~0x0;
    REG32(REG_BUFFER_CONFIG_2) = ~0x0;
    REG32(REG_BUFFER_CONFIG_3) = ~0x0;

    //disable all the buffer
    REG32(REG_BUFFER_ENABLE)   = 0x0;

    l2_int_ctrl(AK_TRUE);
}

#pragma arm section code

/**
 * @brief       set a buffer to be free, in the buffer bitmap
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   buf_id 
 * @return      T_VOID
 */ 
//no so safely
static T_VOID set_free_buf(T_U8 buf_id)
{
    if(INVALID_BUF_ID == buf_id)
        return;

    if(INVALID_BUF_ID > buf_id)
        buf_bitmap &= ~(0xF << (buf_id<<2));
    else
        buf_bitmap &= ~(1 << (buf_id - 4));
}


/**
 * @brief       free a buffer for the device
 * @author      wangguotian
 * @date        2012.12.19
 * @param[in]   device
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE; 
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL l2_release_device_buf(DEVICE_SELECT device)
{
    T_U8    buf_id;

    buf_id = get_device_buf_id(device);
    if(INVALID_BUF_ID == buf_id)
    {
        return AK_FALSE;
    }

    REG32(REG_BUFFER_ENABLE) &= ~(1 << buf_id);

    set_device_buf_id(device, INVALID_BUF_ID);

    set_free_buf(buf_id);

    return AK_TRUE;
}


/**
 * @brief       stop the ldma transfer
 * @author      zhanggaoxin
 * @date        2013.02.25
 * @param[in]   T_VOID
 * @return      T_VOID
 */ 
T_VOID ldma_trans_stop(T_VOID)
{
    T_U32 t1, t2, t;
    T_U8 dir = 0, buf_id;

    //exit when dma is finished
    if(!(REG32(REG_LDMA_COUNT) & LDMA_OPT_EN))
        return;

    //get dir
    if(REG32(REG_LDMA_SRC) & WRAP_EN_SRC)
    {
        dir = 1;
    }
    else if(REG32(REG_LDMA_DEST) & WRAP_EN_DEST)
    {
        dir = 0;
    }
    else
    {
        //exit when no device in dma, no need to stop
        return;
    }

    //get id
    buf_id = get_device_buf_id(l2_intr_type);
    if(INVALID_BUF_ID == buf_id)
    {
        return;
    }
    
    //set status to stop dma
    t1 = get_tick_count_ms();
    while(REG32(REG_LDMA_COUNT) & LDMA_OPT_EN)
    {
        if(dir)     // from device
        {
            if(buf_id < INVALID_BUF_ID)
                l2_set_status(buf_id, 8);
            else
                l2_set_status(buf_id, 2);
        }
        else        // to device
        {
            l2_set_status(buf_id, 0);
        }

        //timeout is 1s
        t2 = get_tick_count_ms();
        t = (t2>=t1) ? (t2-t1) : t2;
        if(t > 1000)
        {
            drv_print(err_str, __LINE__, AK_TRUE);
            return;
        }
    }    
}


/* end of file */
