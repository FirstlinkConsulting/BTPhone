/*******************************************************************************
 * @file    hal_l2.c
 * @brief   the interface for the register callback function when DMA finished.
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.22
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "hal_l2.h"
#include "l2.h"
#include "mmu.h"
#include "arch_init.h"
#include "hal_errorstr.h"
#include "arch_timer.h"

extern T_BOOL page_counter_interrupt_handle(T_VOID);
extern T_U32 MMU_Vaddr2Paddr(T_U32 vaddr);
extern T_VOID MMU_InvalidateIDCache(T_VOID);
static T_VOID l2_cpu_tran_cb(T_TIMER timer_id, T_U32 delay);
static T_U32 l2_cpu_get_buf_data(T_eGB_TYPE type, T_U32 ram_addr, T_U32 len);
static T_U32 get_buf_size(T_eGB_TYPE type, DEVICE_SELECT *device, T_U32 *len);


#pragma arm section zidata = "_drvbootbss_"
static T_U16 func_get_buf[GB_MAX_TYPE];
#if (DRV_SUPPORT_BLUETOOTH > 0)
static T_U16 g_PreReadOff = 0;
static T_U32 bufAddr,bufLen;
#endif
#pragma arm section zidata

#pragma arm section rwdata = "_drvbootcache_"
static T_U8 timeid = ERROR_TIMER;
#pragma arm section rwdata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_HAL_L2;
#pragma arm section rodata


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   DAC_SendPCM
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]pBuf
 * @param   [in]uLen
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
static T_BOOL DAC_SendPCM(T_U8 *pBuf, T_U32 uLen)
{
    if (AK_NULL != pBuf)
    {
        if (!l2_trans_data_dma(ADDR_DAC, (T_U32)pBuf, uLen))
            return AK_FALSE;
    }
    else
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }

    return AK_TRUE;
}

/*******************************************************************************
 * @brief   ADC_RecvPCM
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]pBuf
 * @param   [in]uLen
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
static T_BOOL ADC_RecvPCM(T_U8 *pBuf, T_U32 uLen)
{
    if (AK_NULL != pBuf)
    {
        l2_trans_data_dma((T_U32)pBuf, ADDR_ADC, uLen);
    }
    else
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return AK_FALSE;
    }

    return AK_TRUE;
}

/*******************************************************************************
 * @brief   l2_interrupt_handle
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID l2_interrupt_handle(T_VOID)
{
    T_U8 *pBuf = AK_NULL;
    T_U32 uLen = 0;
    T_U16 Temp;

#ifndef UNSUPPORT_REMAP
    /* check L2 page counter interrupt first */
    if (page_counter_interrupt_handle())
    {
        return;
    }
#endif

    //clear interrupt
    ldma_int_disable();

    Temp = l2_get_intr_type();
    switch (Temp)
    {
    case ADDR_DAC:  //发生DAC L2中断
        ((T_FUNC_GET_BUF)(func_get_buf[GB_DAC] | L2_START_ADDR))(&pBuf, &uLen);
        if ((AK_NULL != pBuf) && (0 != uLen))    //(0 != uLen) ?
        {
            DAC_SendPCM(pBuf, uLen);
        }
        else
        {
            T_U32 buf_addr,buf_len,buf_id;
            T_U32 i = 0;

            get_device_buf_info(ADDR_DAC, &buf_addr, &buf_len, &buf_id);

            //send some a section of 0-samples to driver the dac output level to 1.5v,
            //as there is no fade in/out used in 10L now.
            //may removed in future when better method is found.
            for( i = 0 ; i < buf_len; i+=4)
            {
                REG32(buf_addr+i) = 0;
            }
        }
        break;
    case ADDR_ADC:  //发生ADC L2中断
        ((T_FUNC_GET_BUF)(func_get_buf[GB_ADC] | L2_START_ADDR))(&pBuf, &uLen);
        if ((AK_NULL != pBuf) && (0 != uLen))    //(0 != uLen) ?
        {
            ADC_RecvPCM(pBuf, uLen);
        }
        break;
    default:
        break;
    }
}
#pragma arm section code


/*******************************************************************************
 * @brief   set the callback function which will be called when DAM finish.
 * @author  wangguotian
 * @date    2012.11.21
 * @param   [in]pget_buffer
 *              a pointer to a call back function.
 * @param   [in]type
 * @return  T_BOOL
*******************************************************************************/
T_BOOL l2_get_buffer_register(T_FUNC_GET_BUF pget_buffer, T_eGB_TYPE type)
{
    if ((AK_NULL == pget_buffer) || (type >= GB_MAX_TYPE))
    {
        return AK_FALSE;
    }

    if (AK_FALSE == MMU_IsPremapAddr((T_U32)pget_buffer))
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        while (1);
    }

    func_get_buf[type] = (T_U32)pget_buffer & 0xFFFF;

    return AK_TRUE;
}


/*******************************************************************************
 * @brief   start the l2 transter.
 * @author  wangguotian
 * @date    2012.11.21
 * @param   [in]type
 * @param   [in]tran_mode
 * @param   [in]interval
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
*******************************************************************************/
T_BOOL l2_transfer_start(T_eGB_TYPE type, T_eMode_TYPE tran_mode, T_U8 interval)
{
    T_U8 *pBuf = AK_NULL;
    T_U32 uLen = 0;
    DEVICE_SELECT device;
    T_U32 len;

    if ((type >= GB_MAX_TYPE) || (0 == func_get_buf[type]))
    {
        return AK_FALSE;
    }

    ((T_FUNC_GET_BUF)(func_get_buf[type] | L2_START_ADDR))(&pBuf, &uLen);
    if ((AK_NULL != pBuf) && (0 != uLen))  //(0 != uLen) ?
    {
        switch (type)
        {
        case GB_DAC:
            if (DAC_SendPCM(pBuf, uLen))
            {
                return AK_TRUE;
            }
            break;
        case GB_ADC:
            #if (DRV_SUPPORT_BLUETOOTH > 0)
            if (L2_TRANS_CPU == tran_mode)
            {
                T_U32 buf_addr,buf_len,buf_id;

                if (ERROR_TIMER != timeid)
                {
                    timer_stop(timeid);
                    timeid = ERROR_TIMER;
                }
                bufAddr = (T_U32)pBuf;
                bufLen = (uLen<<1);
                l2_cpu_get_buf_data(GB_ADC, bufAddr, bufLen);
                timeid = timer_start(interval, 1, l2_cpu_tran_cb);
                return AK_TRUE;
            }
            else
            #endif
            {
                if (ADC_RecvPCM(pBuf, uLen))
                {
                    return AK_TRUE;
                }
            }
            break;
        default:
            return AK_FALSE;
            break;
        }
    }
    else
    {
        if (L2_TRANS_CPU == tran_mode)
        {
            timeid = timer_stop(timeid);
        }
    }

    return AK_FALSE;
}

#if (DRV_SUPPORT_BLUETOOTH > 0)
#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   l2_cpu_tran_cb
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]timer_id
 * @param   [in]delay
 * @return  T_void
*******************************************************************************/
static T_VOID l2_cpu_tran_cb(T_TIMER timer_id, T_U32 delay)
{
    T_U32 size;
    T_U8 *pBuf = AK_NULL;
    T_U32 uLen = 0;

    if (timeid == ERROR_TIMER)
        return;

    size = l2_cpu_get_buf_data(GB_ADC, bufAddr, bufLen);

    if (size == bufLen)
    {
        ((T_FUNC_GET_BUF)(func_get_buf[GB_ADC] | L2_START_ADDR))(&pBuf, &uLen);
        if (uLen)
        {
            bufAddr = (T_U32)pBuf;
            bufLen = (uLen<<1);
        }
    }
    else if (size < bufLen)
    {
        bufAddr += (size>>1);
        bufLen -= size;
    }
    else
    {
        drv_print(err_str, __LINE__, AK_TRUE);
    }
}

/*******************************************************************************
 * @brief   get_buf_size
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]type
 * @param   [in]device
 * @param   [in]len
 * @return  T_u32
*******************************************************************************/
static T_U32 get_buf_size(T_eGB_TYPE type, DEVICE_SELECT *device, T_U32 *len)
{
    T_U32 buf_addr;
    T_U32 buf_len;
    T_U32 buf_id;

    switch (type)
    {
    case GB_DAC:
        *device = ADDR_DAC;
        break;
    case GB_ADC:
        *device = ADDR_ADC;
        break;
    default:
        break;
    }
    get_device_buf_info(*device, &buf_addr, &buf_len, &buf_id);
    *len = buf_len;

    return (l2_get_status(buf_id) << 6);
}

/*******************************************************************************
 * @brief   l2_cpu_get_buf_data
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]type
 * @param   [in]ram addr
 * @param   [in]len
 * @return  T_u32
*******************************************************************************/
static T_U32 l2_cpu_get_buf_data(T_eGB_TYPE type, T_U32 ram_addr, T_U32 len)
{
    T_U32 size;
    T_U32 buflen;
    T_U32 tmp;
    T_U32 copytmp,i;
    DEVICE_SELECT device;

    size = get_buf_size(type, &device, &buflen);

    if(0 == size || (buflen != 512) ) return 0;

    if (len <= size)
    {
        size = len;
    }

    if ((g_PreReadOff + size) > buflen)
    {
        tmp = buflen - g_PreReadOff;
        l2_trans_cpu_for_bt(device, ram_addr, tmp, g_PreReadOff);
        l2_trans_cpu_for_bt(device, ram_addr + tmp/2, size - tmp, 0);//tmp/2, size - tmp, 0);
    }
    else
    {
        l2_trans_cpu_for_bt(device, ram_addr, size, g_PreReadOff);
    }

    g_PreReadOff = (g_PreReadOff + size) & (buflen-1);
    return size;
}
#pragma arm section code

T_VOID l2_cpu_set_offset(T_U32 tmp)
{
    g_PreReadOff = tmp;
}

#endif

/*******************************************************************************
 * @brief   stop the l2 transter.
 * @author  zhanggaoxin
 * @date    2013.02.25
 * @param   [in]type
 * @return  T_VOID
*******************************************************************************/
T_VOID l2_transfer_stop(T_eGB_TYPE type)
{
    switch (type)
    {
    case GB_ADC:
        if (timeid != ERROR_TIMER)
        {
             timer_stop(timeid);
             timeid = ERROR_TIMER;
             break;
        }
    case GB_DAC:
        ldma_int_disable();
        ldma_trans_stop();
        break;

    default:
        break;
    }
}


