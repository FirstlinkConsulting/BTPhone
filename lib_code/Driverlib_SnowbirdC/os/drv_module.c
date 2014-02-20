/**
 * @file drv_module.c
 * @brief provide functions of os related function
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-07-24
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_module.h"
#include "arch_init.h"
#include "drv_cfg.h"

//max num of driver module
#define MAX_DRV_MODULE 32

#define MAX_MODULE_MEG 2
//message param size
#define MESSAGE_PARAM_SIZE 3

#define MAX_MESSAGE_LEN 10

//meeage map struct
typedef struct tagT_MESSAGE_MAP
{
    T_U16 msg;
    T_U8 wr;
    T_U8 rd;
    T_U32 param[MESSAGE_PARAM_SIZE*MAX_MESSAGE_LEN];
    T_fDRV_CALLBACK fCbk;

    volatile T_BOOL flag[MAX_MESSAGE_LEN];
    struct tagT_MESSAGE_MAP *next;
}
T_MESSAGE_MAP;

//event group struct
typedef struct
{
    T_U32 events;
}
T_EVENT_GROUP;

//driver module struct
typedef struct
{
    T_EVENT_GROUP eGroup;
    T_MESSAGE_MAP *msgmap_head;
}
T_DRIVER_MODULE;

/*事件标识定义*/
typedef T_U32  T_SYS_EVTID;

/*消息机构体定义*/
typedef struct
{
    /*为了兼MMI，下面两项跟以前是一致的，需要赋值*/
    T_SYS_EVTID event;   /*事件标识*/
    T_U32 param[MESSAGE_PARAM_SIZE];/*事件参数*/

}T_SYS_MAILBOX;
/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


static T_VOID DrvModule_Free(E_DRV_MODULE module);

//global variable for driver module

#pragma arm section zidata = "_udisk_bss_" 

volatile T_DRIVER_MODULE m_drv_module[DRV_MODULE_UNDIFINE];
#if !(USB_VAR_MALLOC > 0)
T_MESSAGE_MAP m_msg_map[DRV_MODULE_UNDIFINE][MAX_MODULE_MEG];
#else
T_MESSAGE_MAP *pMsg_map = AK_NULL;//[DRV_MODULE_UNDIFINE][MAX_MODULE_MEG];
#endif

#pragma arm section zidata

/**
* @brief init driver module
* @author liao_zhijun
* @data 2010-06-18
* @return T_BOOL
* @retval AK_TRUE init success
* @retval AK_FALSE init fail
*/
T_BOOL DrvModule_Init()
{
    T_U32 i;

    for(i = 0; i < DRV_MODULE_UNDIFINE; i++)
    {
        m_drv_module[i].eGroup.events = 0;

        m_drv_module[i].msgmap_head = AK_NULL;
    }

#if USB_VAR_MALLOC > 0
    pMsg_map = (T_MESSAGE_MAP *)drv_malloc(sizeof(T_MESSAGE_MAP)*DRV_MODULE_UNDIFINE*MAX_MODULE_MEG);
    if (AK_NULL == pMsg_map)
        drv_print("alloc failed:", 0, AK_TRUE);
    memset((T_U8 *)pMsg_map,0,sizeof(T_MESSAGE_MAP)*DRV_MODULE_UNDIFINE*MAX_MODULE_MEG);
#endif

    return AK_TRUE;
}

/**
* @brief create task for giving module
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @return T_BOOL
* @retval AK_TRUE create task success
* @retval AK_FALSE fail to create task
*/
T_BOOL DrvModule_Create_Task(E_DRV_MODULE module)
{
    //do nothing if no AKOS defined
    return AK_TRUE;
}

/**
* @brief terminate task for giving module
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @return T_VOID
*/
T_VOID DrvModule_Terminate_Task(E_DRV_MODULE module)
{
    //release all resource
    DrvModule_Free(module);

}

/**
* @brief release all resource for giving module
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @return T_VOID
*/
static T_VOID DrvModule_Free(E_DRV_MODULE module)
{
    T_S32 status;
    T_MESSAGE_MAP *pNext, *pTmp;
    
    //free all hisr
    m_drv_module[module].msgmap_head = AK_NULL;
#if USB_VAR_MALLOC > 0
    if (AK_NULL != pMsg_map)
        drv_free(pMsg_map);
#endif
}

/**
* @brief map callback function for message
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the giving message
* @param callback [in]:  callback function to be mapped
* @return T_BOOL
* @retval AK_TRUE map message success
* @retval AK_FALSE fail to map message 
*/
T_BOOL DrvModule_Map_Message(E_DRV_MODULE module, T_U32 msg, T_fDRV_CALLBACK callback)
{
    T_MESSAGE_MAP *pMsgMap = AK_NULL;
    T_MESSAGE_MAP *pNext = AK_NULL;
    T_U8 i = 0;
    
    //check if message already exit or not
    pNext = m_drv_module[module].msgmap_head;
    while(pNext != AK_NULL)
    {
        if(pNext->msg == msg)
        {
            pNext->fCbk = callback;
            return AK_TRUE;
        }
        i++;
        pNext = pNext->next;
    }

    //alloc memory
    if (i < MAX_MODULE_MEG)
    {
        #if !(USB_VAR_MALLOC > 0)
        pMsgMap = &m_msg_map[module][i];
        #else
        pMsgMap = pMsg_map+module*i;
        #endif
    }
    else 
    {
        return AK_FALSE;
    }   
    
    //save msg and callback
    pMsgMap->msg = msg;
    pMsgMap->wr = 0;
    pMsgMap->rd = 0;
    pMsgMap->fCbk = callback;
    pMsgMap->next = AK_NULL;
    for (i=0;i<MAX_MESSAGE_LEN;i++)
    {
        pMsgMap->flag[i] = AK_FALSE;
    }
    
    //add to map list
    if(AK_NULL == m_drv_module[module].msgmap_head)
    {
        m_drv_module[module].msgmap_head = pMsgMap;
    }
    else
    {
        pNext = m_drv_module[module].msgmap_head;
        while(pNext->next != AK_NULL)
        {
            pNext = pNext->next;
        }

        pNext->next = pMsgMap;
    }

    return AK_TRUE;
}

#pragma arm section code = "_udisk_rw_"

/**
* @brief  send message to giving module
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @param param [in]:  param for the message, its size is 12 bytes
* @return T_BOOL
* @retval AK_TRUE send message success
* @retval AK_FALSE fail to send message 
*/
T_BOOL DrvModule_Send_Message(E_DRV_MODULE module, T_U32 msg, T_U32 *param)
{
    T_MESSAGE_MAP *pNext;
    
    pNext = m_drv_module[module].msgmap_head;

    //loop to find the mapping hisr or callback function
    while(pNext != AK_NULL)
    {
        if(pNext->msg == msg)
        {
            if (pNext->flag[pNext->wr] == AK_TRUE)
            {
                if (pNext->wr == pNext->rd)//judge whether FULL Queue.                
                {
                    drv_print("ov", 0, 1);
                    return AK_FALSE;
                }

                pNext->wr++;
                if (pNext->wr>=MAX_MESSAGE_LEN)
                {
                    pNext->wr = 0;                                     
                }
                continue;
            }        
            
            //save param
            if (param != AK_NULL)
                memcpy(pNext->param+(pNext->wr*(MESSAGE_PARAM_SIZE)), param, MESSAGE_PARAM_SIZE*4);

            pNext->flag[pNext->wr] = AK_TRUE;

            //指向下一个队列。
            pNext->wr++;
            if (pNext->wr>=MAX_MESSAGE_LEN)
            {
                pNext->wr = 0;                                     
            }

            //if AKOS not used, we call callback function the directly
            if(module == DRV_MODULE_UVC)
            {
                pNext->fCbk(param, MESSAGE_PARAM_SIZE*4);
            }
            
            return AK_TRUE;
        }

        pNext = pNext->next;
    }

    return AK_TRUE;
}

/**
* @brief  check if message come or not, if come, call the call back func
*            mainly used in non-os situation
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @param msg [in]: the message to be send
* @return T_BOOL
* @retval AK_TRUE send message success
* @retval AK_FALSE fail to send message 
*/
T_BOOL DrvModule_Retrieve_Message(E_DRV_MODULE module, T_U32 msg)
{
    T_MESSAGE_MAP *pNext;
    T_BOOL ret = AK_FALSE;
    T_U8 tmp;
    
    pNext = m_drv_module[module].msgmap_head;

    //loop to find the mapping hisr or callback function
    while(pNext != AK_NULL)
    {

        if(pNext->msg == msg)
        {
            tmp = pNext->wr;
            
            while(pNext->rd!=tmp)
            {
                if (pNext->flag[pNext->rd] && pNext->fCbk)
                {                                                
                    pNext->fCbk(pNext->param+(pNext->rd*MESSAGE_PARAM_SIZE), MESSAGE_PARAM_SIZE*4);
                    pNext->flag[pNext->rd] = AK_FALSE;
 
                }
                pNext->rd++;
                if (pNext->rd >= MAX_MESSAGE_LEN)
                {
                    pNext->rd = 0;
                }                
                ret = AK_TRUE;
            }

        }

        pNext = pNext->next;
    }

    return ret;
}

T_BOOL DrvModule_ClrEvent(E_DRV_MODULE module, T_U32 event)
{
    //if akos not defined, just set bits in event
    m_drv_module[module].eGroup.events &= ~event;

    return AK_TRUE;
}

/**
* @brief  send event for giving module
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be send
* @return T_BOOL
* @retval AK_TRUE set event success
* @retval AK_FALSE fail to set event
*/
//#pragma arm section code = "_usb_host_"
T_BOOL DrvModule_SetEvent(E_DRV_MODULE module, T_U32 event)
{
    //if akos not defined, just set bits in event
    m_drv_module[module].eGroup.events |= event;

    return AK_TRUE;
}



/**
* @brief  wait for sepcifical event
* @author liao_zhijun
* @data 2010-06-18
* @param module [in]: the giving module
* @param event [in]: the event to be wait for
* @param timeout [in]: timeout value, if the event still not come after the giving time, a timeout will be returned 
* @return T_S32
* @retval DRV_MODULE_SUCCESS wait event success
* @retval DRV_MODULE_TIMEOUT wait event timeout
* @retval DRV_MODULE_ERROR wait event error
*/
extern volatile T_BOOL state_connect;
T_S32 DrvModule_WaitEvent(E_DRV_MODULE module, T_U32 event, T_U32 timeout)
{
    T_U32 tick;
    T_BOOL bTimeout = AK_TRUE;
    
    //if no akos defined, we use get_tick_count to check if timeout or not 
    tick = 0;//get_tick_count();
    do
    {
        #if DRV_SUPPORT_UHOST > 0
        #if UHOST_USE_INTR == 0
        if (DRV_MODULE_USB_BUS == module)
        {
            if (state_connect == AK_FALSE)
                break;
            usb_host_poll_status();
        }
        #endif
        #endif //#if DRV_SUPPORT_UHOST > 0
        
        #if !(USLAVE_USE_INTR > 0)
        if (DRV_MODULE_USB_DISK == module)
        {
            if (LEVEL_LOW == gpio_get_pin_level(USB_DETECT_GPIO))
                break;
            usb_slave_poll_status();
        }
        #endif
        
        //check event bits in event group
        if (m_drv_module[module].eGroup.events & event)
        {
            bTimeout = AK_FALSE;
            break;
        }
    }
    while (tick++ < timeout*200000);//(get_tick_count()-tick) < (timeout<<3));

    //clear events bits
    m_drv_module[module].eGroup.events &= ~(event);

    if (bTimeout)
    {
        if (tick >= timeout*200000)
            return DRV_MODULE_TIMEOUT;
        else
            return DRV_MODULE_ERROR;
    }
    else
    {
        return DRV_MODULE_SUCCESS;
    }
}
#pragma arm section code 

