/*******************************************************************************
 * @file    Eng_Debug.h
 * @brief   This header file is for debug & trace function prototype
 * @author  
 * @date    
 * @version 1.0
*******************************************************************************/
#ifndef __ENG_DEBUG_H__
#define __ENG_DEBUG_H__


#include "anyka_types.h"
#include "Fwl_Console.h"


T_VOID akerror(T_U8 *s, T_U32 n, T_BOOL newline);

/**
 * @brief call this function to get version str
 * @param[in] void
 * @return T_pSTR
 */
T_pSTR MiddleWareLib_GetVersion(T_VOID);

#ifdef DEBUG     //调试版本才允许打印
    #ifndef DEBUG_USE_ASSERT
        #define DEBUG_USE_ASSERT        /* Use assert */
    #endif
#endif

#ifndef DEBUG_TRACE_FUNCTIONE
//  #define DEBUG_TRACE_FUNCTIONE   /* funciton trace */
#endif

#ifndef DEBUG_TRACE_SERIAL
//  #define DEBUG_TRACE_SERIAL      /* serial trace */
#endif

#ifndef DEBUG_TRACE_LINE
//  #define  DEBUG_TRACE_LINE   /* trace line */
#endif

#ifdef OS_WIN32
    #ifndef DEBUG_OUTPUT_TO_WINDOW  /* DEBUG_OUTPUT_TO_SERIAL, DEBUG_OUTPUT_TO_FILE, DEBUG_OUTPUT_TO_WINDOW */
        #define DEBUG_OUTPUT_TO_WINDOW
    #endif
#else
    #ifdef DEBUG   //调试版本才使用断言
        #ifndef DEBUG_OUTPUT_TO_SERIAL  /* DEBUG_OUTPUT_TO_SERIAL */
            #define DEBUG_OUTPUT_TO_SERIAL
        #endif
    #endif
#endif

/********************************************************************
*                                                                   *
*                           DEBUG use assert                        *
*                                                                   *
********************************************************************/
#ifdef DEBUG_USE_ASSERT
    // data point check, not only for NULL, but also for thoses points that are 
    // out of range. We will define different data point ranges for different platforms 
    T_BOOL AkAssertCheckPointer(T_pCVOID ptr);
    // deal with failure condition
    T_VOID AkAssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line);

    #ifndef __FILE__
        #define __FILE__    ""
    #endif
    #ifndef __LINE__
        #define __LINE__    0
    #endif

    #define AK_ASSERT_VAL(_bool_, _msg_, _retval_)  if (!(_bool_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
    #define AK_ASSERT_VAL_VOID(_bool_, _msg_)       if (!(_bool_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }
    #define AK_ASSERT_PTR(_ptr_, _msg_, _retval_)   if (!AkAssertCheckPointer(_ptr_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
    #define AK_ASSERT_PTR_VOID(_ptr_, _msg_)        if (!AkAssertCheckPointer(_ptr_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }
#else       /* not define DEBUG_USE_ASSERT */
    #define AK_ASSERT_VAL(_bool_, _msg_, _retval_)  AK_EMPTY
    #define AK_ASSERT_VAL_VOID(_bool_, _msg_)       AK_EMPTY
    #define AK_ASSERT_PTR(_ptr_, _msg_, _retval_)   AK_EMPTY
    #define AK_ASSERT_PTR_VOID(_ptr_, _msg_)        AK_EMPTY
#endif   // end of DEBUG_USE_ASSERT

/********************************************************************
*                                                                   *
*                           DEBUG trace function                    *
*                                                                   *
********************************************************************/
#ifdef DEBUG_TRACE_FUNCTIONE
    #define AK_FUNCTION_ENTER(_func_)               AK_DEBUG_OUTPUT("Enter function: %s \r\n", _func_)
    #define AK_FUNCTION_LEAVE(_func_)               AK_DEBUG_OUTPUT("Leave function: %s \r\n", _func_)
    #define AK_FUNCTION_RET_STR(_func_, _retval_)   AK_DEBUG_OUTPUT("Leave function: %s and return %s \r\n", _func_, _retval_)
    #define AK_FUNCTION_RET_INT(_func_, _retval_)   AK_DEBUG_OUTPUT("Leave function: %s and return %d \r\n", _func_, _retval_)
#else    // not define DEBUG_TRACE_FUNCTIONE
    #define AK_FUNCTION_ENTER(_func_)               AK_EMPTY
    #define AK_FUNCTION_LEAVE(_func_)               AK_EMPTY
    #define AK_FUNCTION_RET_STR(_func_, _retval_)   AK_EMPTY
    #define AK_FUNCTION_RET_INT(_func_, _retval_)   AK_EMPTY
#endif    // end of define DEBUG_TRACE_FUNCTIONE

/********************************************************************
*                                                                   *
*                           DEBUG trace serial                      *
*                                                                   *
********************************************************************/
#ifdef DEBUG_TRACE_SERIAL
    #define AK_SERIAL_RECEIVE(_sid_, _msg_, _size_) AK_DEBUG_OUTPUT("Read data from CHANNEL %d(%d): %s\r\n", _sid_, _size_, _msg_)
    #define AK_SERIAL_SEND(_sid_, _msg_, _size_)    AK_DEBUG_OUTPUT("Write data to  CHANNEL %d(%d): %s\r\n", _sid_, _size_, _msg_)
#else    // not define DEBUG_TRACE_SERIAL
    #define AK_SERIAL_RECEIVE(_sid_, _msg_, _size_) AK_EMPTY
    #define AK_SERIAL_SEND(_sid_, _msg_, _size_)    AK_EMPTY
#endif    // end of define DEBUG_TRACE_SERIAL

/********************************************************************
*                                                                   *
*                           DEBUG output function                   *
*                                                                   *
********************************************************************/
#ifdef DEBUG_OUTPUT_TO_SERIAL
    T_VOID AkDebugOutputSerialInit(T_VOID);
    T_VOID AkDebugOutputSerialEnd(T_VOID);
    T_VOID AkDebugOutputSerialS(T_pCSTR message);
    T_VOID AkDebugOutput(T_pCSTR s, ...);

    #define AK_DEBUG_OUTPUT_INIT()                  
    #define AK_DEBUG_OUTPUT_END()                   
    #define AK_DEBUG_OUTPUT_S(_msg_)                AkDebugOutputSerialS(_msg_)
    #define AK_DEBUG_OUTPUT                         AkDebugOutput
    /* for kernel debuging,the function is always in ram*/
    #define AK_PRINTK(_str_,_hvalue_,_bnewline_)    akerror(_str_,_hvalue_,_bnewline_)   //only debug version output debug info
    
#else   // not define DEBUG_OUTPUT_TO_SERIAL
    #ifdef DEBUG_OUTPUT_TO_FILE
        T_VOID AkDebugOutputFileInit(T_VOID);
        T_VOID AkDebugOutputFileEnd(T_VOID);
        T_VOID AkDebugOutputFileS(T_pCSTR message);
        T_VOID AkDebugOutput(T_pCSTR s, ...);

        #define AK_DEBUG_OUTPUT_INIT()              AkDebugOutputFileInit()
        #define AK_DEBUG_OUTPUT_END()               AkDebugOutputFileEnd()
        #define AK_DEBUG_OUTPUT_S(_msg_)            AkDebugOutputFileS(_msg_)
        #define AK_DEBUG_OUTPUT                     AkDebugOutput
    #else   // not define DEBUG_OUTPUT_TO_SERIAL and not define DEBUG_OUTPUT_TO_FILE
        #ifdef DEBUG_OUTPUT_TO_WINDOW
            T_VOID AkDebugOutputWindowInit(T_VOID);
            T_VOID AkDebugOutputWindowEnd(T_VOID);
            T_VOID AkDebugOutputWindowS(T_pCSTR message);
            T_VOID AkDebugOutput(T_pCSTR s, ...);

            #define AK_DEBUG_OUTPUT_INIT()                              AkDebugOutputWindowInit()
            #define AK_DEBUG_OUTPUT_END()                               AkDebugOutputWindowEnd()
            #define AK_DEBUG_OUTPUT_S(_msg_)                            AkDebugOutputWindowS(_msg_)
            #define AK_DEBUG_OUTPUT                                     AkDebugOutput
            #define AK_PRINTK(_str_,_hvalue_,_bnewline_)                {\
                                                                            AkDebugOutput(_str_);\
                                                                            AkDebugOutput("0x%x",_hvalue_);\
                                                                            if(_bnewline_)\
                                                                            {\
                                                                                AkDebugOutput("\r\n");\
                                                                            }\
                                                                        }
        #else   // not define DEBUG_OUTPUT_TO_SERIAL and not define DEBUG_OUTPUT_TO_FILE and not define DEBUG_OUTPUT_TO_WINDOW
            T_VOID AkDebugOutput(T_pCSTR s, ...);

            #define AK_DEBUG_OUTPUT_INIT()                                  AK_EMPTY
            #define AK_DEBUG_OUTPUT_END()                                   AK_EMPTY
            #define AK_DEBUG_OUTPUT_S(_msg_)                                AK_EMPTY
            #define AK_DEBUG_OUTPUT(...)                                    AK_EMPTY
            #define AK_PRINTK(_str_,_hvalue_,_bnewline_)                    AK_EMPTY
        #endif
    #endif
#endif
#endif   // end of _AKUTLDEBUG_H_
