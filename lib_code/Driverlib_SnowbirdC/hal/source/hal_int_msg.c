/**
 * @file    hal_int_msg.c
 * @brief   define the interrupt message and the interface 
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
#include "anyka_types.h"
#include "hal_int_msg.h"
#include "arch_init.h"
#include "hal_errorstr.h"
#include "drv_cfg.h"


#if (CHIP_SEL_10C > 0)
#define MSG_QUEUE_LEN   32      //must 2 ^ n
#else
#define MSG_QUEUE_LEN   8       //must 2 ^ n
#endif


typedef struct
{
    ST_MSG  msg_queue[MSG_QUEUE_LEN];
    T_U16   front;
    T_U16   rear;
}T_INT_MSG_QUEUE;


#pragma arm section zidata = "_drvbootbss_"
static T_INT_MSG_QUEUE  queue;
#pragma arm section zidata

#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_INT_MSG;
#pragma arm section rodata


#pragma arm section code = "_drvbootcode_" 
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
T_BOOL post_int_message(T_U8 msg, T_U8 bparam, T_U16 hwparam)
{
    ST_MSG stmsg;
    T_U16  rear;
    T_U16  front;

    stmsg.msg = msg;
    stmsg.bparam = bparam;
    stmsg.hwparam = hwparam;

    if ((IM_TIMER == stmsg.msg) || (IM_CHG == stmsg.msg))
    {
        front = queue.front;

        while(front != queue.rear)
        {
            if(*(T_U32 *)(&queue.msg_queue[front]) == *(T_U32 *)&stmsg)
            {
                return AK_TRUE;
            }

            front = (front + 1) & (MSG_QUEUE_LEN-1);
        }
    }

    rear = (queue.rear + 1)&(MSG_QUEUE_LEN-1);

    if(rear == queue.front)
    {
        drv_print(err_str, stmsg.msg, AK_TRUE);
        return AK_FALSE;
    }

    queue.msg_queue[queue.rear] = stmsg;

    queue.rear = rear;

    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_drvfrequent_"
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
T_BOOL peek_int_message(ST_MSG *pstmsg)
{
    if(queue.front == queue.rear)
    {
        return AK_FALSE;
    }

    *pstmsg = queue.msg_queue[queue.front];

    queue.front = (queue.front + 1)&(MSG_QUEUE_LEN-1);

    return AK_TRUE;
}
#pragma arm section code


