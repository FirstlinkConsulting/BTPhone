/**
 * @file    hal_int_msg.h
 * @brief   define the interrupt message and the interface 
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
 
#ifndef _HAL_INT_MSG_H_
#define _HAL_INT_MSG_H_


#include "anyka_types.h"


/*the message when device change ,which registerd by detector*/
#define IM_DEVCHANGE    1   //hwparam : high 8bit, indicate other infor, 
                            //          when for usb detect, if connect to the pc,
                            //          the value is 1,when connect to the charger,
                            //          the value is 0.
                            //          low 8bit indicate device id
                            //bparam  : state, AK_TRUE means connect, 
                            //                 AK_FALSE, means disconnect

/*the message when key is pressed down*/
#define IM_KEYINFO      2   //hwparam : high 8bit indicate longpress or not
                            //          low 8bit indicate key status,can be down/press/up
                            //bparam  : key id(详见hal_keypad.h中结构体T_KEYPAD定义)

#define IM_TIMER        3   //bparam  : timer_id, hwparam : delay_ms

#define IM_RTC          4   //bparam  : 0, hwparam : 0

#define IM_CHG          5   //bparm   : 0, hwparam : (详见arch_pmu.h中定义)

#define IM_UHOST        6   //bparam  : state, AK_TRUE means connect, 
                            //                 AK_FALSE, means disconnect

#define IM_USLAVE       7   //bparam  : status,AK_TRUE means connect, 
                            //                 AK_FALSE, means disconnect


/**
 * @brief 
 */
typedef struct
{
    T_U8    msg;        //Specifies the message identifier
    T_U8    bparam;     //Specifies additional information about the message. 
                        //The exact meaning depends on the value of the msg member
    T_U16   hwparam;    //Specifies additional information about the message. 
                        //The exact meaning depends on the value of the msg member
}ST_MSG;



/**
 * @brief       post the interrupt message 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[out]  pstmsg
 *                  Pointer to an ST_MSG structure that receives message information
 * @return      T_BOOL
 * @retval      If message queue is not full, the return value is AK_TRUE;
 *              If message queue if full, the return value is AK_FALSE.
 *
 * @remark      The queue will be just hold one copy of the same IM_TIMER
 */ 
T_BOOL post_int_message(ST_MSG *pstmsg);


/**
 * @brief       peek the interrupt message 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[out]  pstmsg
 *                  Pointer to an ST_MSG structure that receives message information
 * @return      T_BOOL
 * @retval      If a message is available, the return value is AK_TRUE;
 *              If no messages are available, the return value is AK_FALSE.
 */ 
T_BOOL  peek_int_message(ST_MSG *pstmsg);

#endif //_HAL_INT_MSG_H_

