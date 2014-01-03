/**
 * @file
 * @brief ANYKA software
 * this file will constraint the access to the bottom layer serial function,
 * avoid resource competition. Also, this file os for porting to 
 * different OS
 *
 * @author 
 * @date 2001-4-20
 * @version 1.0
 */
 
#include "arch_uart.h"
#include "vme.h"
#include "Fwl_Console.h"
#include "gpio_define.h"

#define PRINTF_USB_GPIO        30 //GPIO number

//#define GPIO_SIMU_UART

#pragma arm section code = "_bootcode1_"
/*********************************************************************
  Function:         Fwl_UartInit
  Description:      initialize uart.
  Input:            T_VOID.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_ConsoleInit(T_VOID)
{    
#ifdef GPIO_SIMU_UART
    Simu_UART_Init(PRINTF_USB_GPIO);  
#else    
    uart_init(CONSOLE_UART_ID, UART_BAUD_115200);
#endif
    return AK_TRUE;
}
T_BOOL Fwl_ConsoleClose(T_VOID)
{    
#ifdef GPIO_SIMU_UART
    //Simu_UART_Init(PRINTF_USB_GPIO);  
#else    
    uart_close(CONSOLE_UART_ID);
#endif
    return AK_TRUE;
}

//#pragma arm section code

//#pragma arm section code = "_bootcode1_" //??
/*********************************************************************
  Function:         Fwl_ConsoleWriteChr
  Description:      print the number in the hex form.
  Input:            t : number.
  Return:           T_VOID. 
  Author:           
  Data:             2012-11-20
**********************************************************************/
T_VOID Fwl_ConsoleWriteChr(T_U8 chr)
{
#ifdef GPIO_SIMU_UART
    Simu_UART_SendByte(chr);    
#else    
    uart_write_chr(CONSOLE_UART_ID, chr);
#endif

}

T_BOOL Fwl_ConsoleWriteStr(T_U8 *str)
{
#ifdef GPIO_SIMU_UART
    Simu_UART_SendStr(str);
#else    
    uart_write_str(CONSOLE_UART_ID, str);
#endif
    return AK_TRUE;
}

static T_VOID print_x(T_U32 t)
{
    T_S32 i, k=7;
    T_S8 buf[8];

    for (i=0; i<8; i++)
        buf[i] = '0';

    for(;;)
    {
        if (t > 15)
            i = t % 16;
        else
            i = t;

        if (i < 10)
            buf[k--] = i + '0';
        else if(i < 16)
           buf[k--] = i - 10 +'a';

        if (t < 15)
            break;
        t >>= 4;
    }

    for(i =0 ;i < 8 ; i++)
        Fwl_ConsoleWriteChr(buf[i]);
}

T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline)
{

#ifdef GPIO_SIMU_UART
    Simu_UART_SendStr(s);
#else
    uart_write_str(CONSOLE_UART_ID, s);        
#endif

    if (n)
        print_x(n);
        
    if (newline)
    {
    #ifdef GPIO_SIMU_UART
        Simu_UART_SendByte('\n');
    #else
        Fwl_ConsoleWriteChr('\n');
    #endif
    }        
}

#pragma arm section code



