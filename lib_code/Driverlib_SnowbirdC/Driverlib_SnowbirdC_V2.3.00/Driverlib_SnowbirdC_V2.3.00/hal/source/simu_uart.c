/*******************************************************************************
 * @file    simu_uart.c
 * @brief   Using GPIO to achieve the serial transmit function, used for system debugging
 * Copyright (C) 2010 Anyka (GuangZhou) MicroElectronics Technology Co., Ltd.
 * @author  liuhuadong
 * @date    2013.5.5
 * @version 1.0
 * @ref     AK10xx technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "gpio.h"
#include "sys_ctl.h"
#include "anyka_cpu.h"
#include "drv_cfg.h"


#if DRV_SIMU_UART > 0


#define TIMER_COUNT                 0x3ffffff

#pragma arm section zidata = "_cachedata_"
T_U32 gCurTimer2Cnt;
static T_U8 gpio_pin;
#pragma arm section zidata
#pragma arm section rwdata = "_fixedrwdata_"
volatile T_U8 DelayCnt = (226);
#pragma arm section rwdata

#pragma arm section code ="_drvbootinit_"
//#pragma arm section code ="_drv_simu_uart_"

T_VOID Simu_UART_Init(T_U8 gpio_num)
{
    //确保定时器是开着的。
    timer2_enable();
	
    // set direction: output 
    //gpio_set_pin_as_gpio(gpio_num);
    //设置SHAPEPIN ，这边后面要完善。
	REG32(REG_SHARE_PIN_CTRL2) &= ~0x07;

    gpio_set_pullup_pulldown(gpio_num, 1);
    gpio_set_pin_dir(gpio_num, 1);
	 gpio_set_pin_level(gpio_num, 1);
    gpio_pin = gpio_num;
}

#pragma arm section code
#pragma arm section code ="_drv_simu_uart_"

/*
* Function : Simu_UART_SendByte
* Brief : send one byte data using simu_UART
*/


T_U32 timer2_get_diff(T_U32 *cur_time,T_U32 last_time)
{
    T_U32 cur_cnt;

    cur_cnt = REG32(REG_TIMER2_CNT) & 0x3ffffff;
    if(cur_time)
        *cur_time = cur_cnt;
    return ((cur_cnt <= last_time)?(last_time-cur_cnt):(TIMER_COUNT-cur_cnt+last_time));
}

T_U32 Simu_Uart_Delay(T_U32 last_time)
{   
    while(timer2_get_diff(AK_NULL,last_time) < DelayCnt);
    if(last_time >= DelayCnt)
        return last_time - DelayCnt;
    else
        return (0x3ffffff - (DelayCnt-last_time));
}

T_VOID Simu_UART_OnChange(T_U8 delay)
{
    DelayCnt = delay;
}

T_U8 Simu_UART_GetDelay(T_VOID)
{
    return DelayCnt;
}

T_VOID Simu_UART_SendByte(T_U8 chr)
{
    T_U8 tmp_Char = chr;
    T_U8 count = 0;
    T_U32 last_time;

    if(DelayCnt == 0)
        return ;

    //发送起始位
    gpio_set_pin_level(gpio_pin, 0);

    //延时一个位时间
    //#ifdef DEBUG_TRACE
    timer2_get_diff(&last_time,0);
    last_time = Simu_Uart_Delay(last_time);
    //#endif
    
    for (count=0;count<9;count++)
    {
        if (count<8)
        {   
            if ((tmp_Char&0x01))
            {
                //send 1
                gpio_set_pin_level(gpio_pin, 1);

            }
            else
            { 
                //send 0
                gpio_set_pin_level(gpio_pin, 0);
            }
            tmp_Char >>=1;
        }
        else
        { 
            //发送停止位
            gpio_set_pin_level(gpio_pin, 1);            
        }
        
        //延时一个位时间
        last_time = Simu_Uart_Delay(last_time);
    }
    //使用此语句会导致有频繁中断的应用出现异常
}


/*
* Function: Simu_UART_SendStr
* Brief: send string using simu_UART
* Parameter: pStr: pointer of a string
*/
T_VOID Simu_UART_SendDat(T_U8  *pStr, T_U32 len)
{ 
    T_U8 *pChr = pStr;
    T_U32 i;
    
    if (AK_NULL == pChr)
        return;
    
    for (i=0;i<len;i++)
    {
        Simu_UART_SendByte((T_U8)(*pChr));
        pChr++; 
    }
}

T_VOID Simu_UART_SendStr(T_U8  *pStr)
{ 
    T_U8 *pChr = pStr;
    if (AK_NULL == pChr)
        return;
    
    while(*pChr)
    {
        Simu_UART_SendByte((T_U8)(*pChr));
        pChr++; 
    }
}

#pragma arm section code
#endif

