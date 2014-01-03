/*******************************************************************************
 * @file    Fwl_MicroTask.h
 * @brief   micro task head file
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-11-29
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_MICROTASK_H__
#define __FWL_MICROTASK_H__


typedef T_VOID (*TASK_FUNC)(T_VOID);


/*******************************************************************************
 * @brief   register micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]task: 
 * @param   [in]callInterval: 
 * @return  T_U8
 * @retval  the handle of micro task
*******************************************************************************/
T_U8 Fwl_MicroTaskRegister(TASK_FUNC task,T_U16 callInterval);//CallInterval以10ms为单位


/*******************************************************************************
 * @brief   unregister micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskUnRegister(T_U8 index);


/*******************************************************************************
 * @brief   pause micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskPause(T_U8 index);


/*******************************************************************************
 * @brief   resume micro task 
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]index: task handle
 * @return  T_BOOL
 * @retval  success of fail
*******************************************************************************/
T_BOOL Fwl_MicroTaskResume(T_U8 index);


#endif //__FWL_MICROTASK_H__


