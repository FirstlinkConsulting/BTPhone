/*******************************************************************************
 * @file    Fwl_MicroTask.c
 * @brief   micro task file
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "Fwl_MicroTask.h"
#include "arch_interrupt.h"
#include "Eng_debug.h"
#include "Arch_Timer.h"


#define  MICRO_TASK_DEBUG    0
#define  MAX_MICRO_TASK      (6)


typedef struct{
    TASK_FUNC TaskCalBak;
    T_U16 CallInterval;
    T_U16 BackUpInterval;
}T_TASK_REF;


#pragma arm section rwdata = "_cachedata_",zidata = "_bootbss_"
T_TASK_REF task_param[MAX_MICRO_TASK];
T_U32 TaskContext[16];
volatile T_U8 TaskMutex = 0;
volatile T_U8 TaskActiveMap = 0;
volatile T_U8 TaskMaskMap = 0xff;
volatile T_U8 TaskRegMap = 0x00;//已注册的任务映射
T_U8 RunTaskCnt = 0;
T_TIMER TaskTimer;
#pragma arm section


extern T_VOID Do_IRQ_Return(T_U32 sp_svc);
//thumb 标志要注意,必须注意保护好SVC下的LR
static T_VOID TaskTimerCallBak(T_TIMER timer_id, T_U32 delay);



/*******************************************************************************
 * @brief   register micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]task: 
 * @param   [in]callInterval: 
 * @return  T_U8
 * @retval  the handle of micro task
*******************************************************************************/
T_U8 Fwl_MicroTaskRegister(TASK_FUNC task,T_U16 CallInterval)//CallInterval以10ms为单位
{
    T_U8 i;

    for(i=0;i<MAX_MICRO_TASK;i++)
    {
        if(task_param[i].TaskCalBak == AK_NULL)
        {
            task_param[i].TaskCalBak = task;
            task_param[i].BackUpInterval = task_param[i].CallInterval = CallInterval;
            RunTaskCnt++;
            TaskRegMap |= (1<<i);//注册记录
            if(RunTaskCnt == 1)
            {
                TaskTimer = timer_start(10, AK_TRUE, TaskTimerCallBak);
                if(TaskTimer == ERROR_TIMER)
                {
                    akerror("task timer start fail",0,1);
                    while(1);
                }
            }
            return i;
        }
    }
    return 0xff;
}


/*******************************************************************************
 * @brief   unregister micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskUnRegister(T_U8 Index)
{
    if(Index >= MAX_MICRO_TASK)
        return AK_FALSE;
    store_all_int();//INTR_DISABLE(INT_ENABLE_TIMER_BIT);
    TaskActiveMap &= ~(1<<Index);
    task_param[Index].TaskCalBak = AK_NULL;
    TaskMaskMap |= (1<<Index);
    TaskRegMap &= ~(1<<Index);
    restore_all_int();//INTR_ENABLE(INT_ENABLE_TIMER_BIT);
    RunTaskCnt--;
    if(RunTaskCnt == 0)
        TaskTimer = timer_stop(TaskTimer);
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   pause micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskPause(T_U8 Index)
{
    if(Index >= MAX_MICRO_TASK)
        return AK_FALSE;
   
    store_all_int();//INTR_DISABLE(INT_ENABLE_TIMER_BIT);
    TaskMaskMap |= (1<<Index);
    restore_all_int();//INTR_ENABLE(INT_ENABLE_TIMER_BIT);
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   resume micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskResume(T_U8 Index)
{
    if(Index >= MAX_MICRO_TASK)
        return AK_FALSE;
    if((TaskRegMap & (1<<Index)) == 0)//未注册任务不做激活
        return AK_FALSE;
    
    store_all_int();//INTR_DISABLE(INT_ENABLE_TIMER_BIT);
    TaskMaskMap &= ~(1<<Index);
    restore_all_int();//INTR_ENABLE(INT_ENABLE_TIMER_BIT);
    return AK_TRUE;
}


#pragma arm section code = "_bootcode1_"
T_BOOL TaskPending(T_VOID)
{
#if(MICRO_TASK_DEBUG)
    T_U8 i;
#endif

    if(TaskActiveMap && (TaskMutex == 0))
    {
#if(MICRO_TASK_DEBUG)
        akerror("TaskPending succedd:",TaskActiveMap,1);
        for(i = 0;i < 14;i++)
        {
            print_x(Data[i]);
            Fwl_ConsoleWriteChr(' ');
        }
        Fwl_ConsoleWriteChr('\n');
#endif
        return AK_TRUE;
    }
    //akerror("TaskPending Failed:",TaskActiveMap,1);
    return AK_FALSE;
}


T_VOID TaskTimerCallBak(T_TIMER timer_id, T_U32 delay)
{
    T_U8 i;

    for (i=0;i<MAX_MICRO_TASK;i++)
    {
        if (task_param[i].TaskCalBak != AK_NULL)
        {
            if ((TaskMaskMap & (1<<i)) == 0)
            {
                task_param[i].CallInterval--;
                if (task_param[i].CallInterval == 0)
                {
                    //akerror("Micro Task Call:",task_param[i].BackUpInterval,1);
                    task_param[i].CallInterval = task_param[i].BackUpInterval;
                    TaskActiveMap |= (1<<i);
                }
            }
        }
    }
}
#pragma arm section code


#pragma arm section code = "_music_playing_"
T_VOID BackGroundTask(T_U32 sp_svc)
{
    T_U8 i;

#if (MICRO_TASK_DEBUG)
    akerror("sp_svc:", sp_svc, 1);

    for (i = 0;i < 16;i++)
    {
        print_x(TaskContext[i]);
        Fwl_ConsoleWriteChr(' ');
    }
    Fwl_ConsoleWriteChr('\n');
#endif

    for (i = 0;i < MAX_MICRO_TASK;i++)
    {
        if (TaskActiveMap & (1<<i))
        {
            TaskActiveMap &= ~(1<<i);
            if (task_param[i].TaskCalBak)
            {
                task_param[i].TaskCalBak();
            }
        }
    }
    Do_IRQ_Return(sp_svc);
}
#pragma arm section code

