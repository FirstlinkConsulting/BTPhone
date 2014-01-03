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
#include "windows.h"



/*********************************************************************
  Function:         Fwl_ConsoleInit
  Description:      initialize uart.
  Input:            T_VOID.
  Return:           true = initialization sucess; 
                    false = initialization error. 
  Author:           
  Data:             2012-11-20
**********************************************************************/
T_BOOL Fwl_ConsoleInit(T_VOID)
{    
#ifdef DEBUG_SAVE_SBCDATA
	Debug_InitSaveSBC();
	Debug_InitSavePCM();

#endif
	init_printf();

  //  uart_init(uiUART1, UART_BAUD_115200);

	return AK_TRUE;
}

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
	printf("%c", chr);
//    uart_write_chr(uiUART1, chr);
}

T_BOOL Fwl_ConsoleWriteStr(T_U8 *str)
{
	printf("%s", str);
 //   uart_write_str(uiUART1, str);  
    return AK_TRUE;
}

static T_VOID print_x(T_U32 t)
{

	printf("%08x", t);

}


T_VOID Fwl_ConsoleWriteHex(T_U8 *s, T_U32 n, T_BOOL newline)
{
	
	printf("%s%08x", s,n); 
	if(newline)
	{
		printf("\n");
	}
}

T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline)
{

	Fwl_ConsoleWriteHex(s, n, newline);
}


