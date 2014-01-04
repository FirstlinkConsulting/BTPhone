/**
 * @file Eng_Debug.c
 * @brief ANYKA software
 * 
 * @author pyxue, ZouMai
 * @date 2001-09-21
 * @version 1.0
 */
#include <string.h>
#include "Eng_Debug.h"
#include "Fwl_System.h"
#include "Fwl_Serial.h"

#ifdef OS_WIN32
#include "w_winvme.h"
#include <stdio.h>
#endif

#define MIDDLEWARELIB_VERSION     "V1.0.08"

#define KEY_BACKSPACE  0x08
#define KEY_ENTER      0x0d
#define KEY_ESC        0x1b

#define CR          0x0D
#define LF          0x0A

/**
 * @brief call this function to get version str
 * @param[in] void
 * @return T_pSTR
 */
T_pSTR MiddleWareLib_GetVersion(T_VOID)
{
	return MIDDLEWARELIB_VERSION;
}

/********************************************************************
*                                                                   *
*                           DEBUG use assert                        *
*                                                                   *
********************************************************************/
#ifdef DEBUG_USE_ASSERT

#ifdef OS_WIN32
    #define MIN_RAM_ADDR    0x00001000  // The first address of RAM
    #define MAX_RAM_ADDR    0x2fffffff  // the end of RAM
#else
    #define MIN_RAM_ADDR    0x00800000	//(RAM_BASE_ADDR)  // The first address of RAM
    #define MAX_RAM_ADDR    0x00A00000	//(RAM_BASE_ADDR + RAM_SIZE)  // the end of RAM 8M
#endif



/**
 * @brief Check if the pointer is out of range
 * 
 * @author @b ZouMai
 * 
 * @author pyxue
 * @date 2001-09-21
 * @param T_pVOID ptr
 * @return T_BOOL
 * @retval AK_TRUE: legal
 * @retval AK_FALSE: illegal
 */
T_BOOL AkAssertCheckPointer(T_pCVOID ptr)
{
    if (((T_U32)ptr >= MIN_RAM_ADDR) && ((T_U32)ptr <= MAX_RAM_ADDR) )
    {
        return AK_TRUE;
    }
    else
    {
        AK_DEBUG_OUTPUT("assert!  ptr:0x%x!\n", ptr);
        return AK_FALSE;
    }
}

/**
 * @brief Assert fail process in WIN32 OS
 * 
 * @author @b ZouMai
 * 
 * @author pyxue
 * @date 2002-12-27
 * @param T_pSTR message: user message
 * @param T_pSTR filename: file name of caller
 * @param T_U32 line: line ID of caller
 * @return T_VOID
 * @retval 
 */
T_VOID AkAssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line)
{
#ifdef DEBUG_TRACE_LINE
    AK_DEBUG_OUTPUT("message: %s, file name: %s, line: %d\r\n", message, filename, line);
#else
    AK_DEBUG_OUTPUT("message: %s\r\n", message);
#endif
#ifdef OS_WIN32
    Fwl_Beep(2000, 10);
    Fwl_Beep(1000, 10);
#endif
    return;
}

#endif  /* not define DEBUG_USE_ASSERT */

/********************************************************************
*                                                                   *
*                           DEBUG output function                   *
*                                                                   *
********************************************************************/
#include <stdarg.h>

extern int vsprintf( char *buffer, const char *format, va_list argptr ); 

//extern int vsprintf(char * /*s*/, const char * /*format*/, __va_list /*arg*/);;

T_VOID AkDebugOutput(T_pCSTR s, ...)
{
    T_S8 printf_buf[640] = {0};
    va_list ap;

    va_start(ap, s);
    vsprintf(printf_buf, s, ap);
    va_end(ap);

    //if (printf_buf[strlen(printf_buf)-1]!='\n')
		//strcat(printf_buf, "\r\n");

	AK_DEBUG_OUTPUT_S(printf_buf);

    return;
}

#ifdef DEBUG_OUTPUT_TO_SERIAL
T_VOID AkDebugOutputSerialS(T_pCSTR message)
{
    Fwl_ConsoleWriteStr((T_pSTR)message);
}
#endif

#ifdef DEBUG_OUTPUT_TO_FILE
#define DEBUG_OUTPUT_FILE   "Temp/Trace.txt"

T_VOID AkDebugOutputFileInit(T_VOID)
{
    FILE *tracefile;

    tracefile = fopen(DEBUG_OUTPUT_FILE, "w");
    if (tracefile == AK_NULL)
        return;

    fclose(tracefile);
}

T_VOID AkDebugOutputFileEnd(T_VOID)
{
}

T_VOID AkDebugOutputFileS(T_pCSTR message)
{
    FILE *tracefile;

    tracefile = fopen(DEBUG_OUTPUT_FILE, "a+");
    if (tracefile == AK_NULL)
        return;

    fseek(tracefile, 0, SEEK_END);
    while ((*message!= '\0'))
        fputc((T_S16)*(message++), tracefile);
    fclose(tracefile);
}

#endif

#ifdef DEBUG_OUTPUT_TO_WINDOW

T_VOID AkDebugOutputWindowInit(T_VOID)
{
    printf("*** This is userware ***\r\nVersion 1.0\r\nTrace window is active!\r\n");
    return;
}

T_VOID AkDebugOutputWindowEnd(T_VOID)
{
    printf("*** End of userware ***\r\n");
    return;
}

T_VOID AkDebugOutputWindowS(T_pCSTR message)
{
    printf((char *)message);
}

#endif





//the end of files
