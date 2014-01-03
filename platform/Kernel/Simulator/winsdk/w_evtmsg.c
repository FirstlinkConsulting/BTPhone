#ifdef OS_WIN32

#include "anyka_types.h"
#include "w_evtmsg.h"


#define MSG_QUEUE_LEN   8       //must 2 ^ n


typedef struct
{
    ST_MSG  msg_queue[MSG_QUEUE_LEN];
    T_U16   front;
    T_U16   rear;
}T_INT_MSG_QUEUE;


static T_INT_MSG_QUEUE  queue;
static const T_U8 err_str[] = "I ";

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
T_BOOL post_int_message(ST_MSG *pstmsg)
{
    T_U16 rear;
    T_U16 front;

    rear = (queue.rear + 1)&(MSG_QUEUE_LEN-1);
    
    if(rear == queue.front)
    {
        return AK_FALSE;
    }

    if(IM_TIMER == pstmsg->msg)
    {
        front = queue.front;

        while(front != queue.rear)
        {
            if(*(T_U32 *)(&queue.msg_queue[front]) == *(T_U32 *)pstmsg)
            {
                return AK_TRUE;
            }

            front = (front + 1) & (MSG_QUEUE_LEN-1);
        }
    }

    queue.msg_queue[queue.rear] = *pstmsg;

    queue.rear = rear;

    return AK_TRUE;
}

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



#endif
