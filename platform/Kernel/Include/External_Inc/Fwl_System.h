/*******************************************************************************
 * @file    Fwl_System.h
 * @brief   This header file is for display function prototype
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_SYSTEM_H__
#define __FWL_SYSTEM_H__


#include "Fwl_Console.h"
#include "anyka_types.h"


// switch off event type
#define EVENT_TYPE_SWITCHOFF_KEY    0xFF00
#define EVENT_TYPE_SWITCHOFF_BATLOW 0xFF01
#define EVENT_TYPE_SWITCHOFF_AUTO   0xFF02
#define EVENT_TYPE_SWITCHOFF_MANUAL 0xFF03

#define EVENT_RETURN_SWITCHOFF      0xFFFF
#define EVENT_TYPE_TIMEOUT_NORMAL   0xFF04

#ifdef OS_WIN32
T_VOID  Fwl_Beep(T_U32 dwFreq, T_U32 dwDuration);
#endif


/*******************************************************************************
 * @brief   make system enters the standby state
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]wakeupMode: 
 * @return  T_U16
 * @retval  the reason of system waked
*******************************************************************************/
T_U16 Fwl_SysSleep(T_U8 bHasVw);



/*******************************************************************************
 * @brief   power on system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOn(T_VOID);


/*******************************************************************************
 * @brief   power off system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOff(T_VOID);


/*******************************************************************************
 * @brief   deal system message
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysMsgHandle(T_VOID);


/*******************************************************************************
 * @brief   reset the system
 * @author  yangyiming
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysReboot(T_VOID);


/*******************************************************************************
 * @brief   printf all infomation of the system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysInfoPrint(T_VOID);


/*******************************************************************************
 * @brief   get current bat voltage
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the bat voltage(unit: mv)
*******************************************************************************/
T_U32 Fwl_SysGetBatVolt(T_VOID);


/*********************************************************************
  Function:         Fwl_StoreAllInt
  Description:      store and mask all interrupt.
  Input:            void.
  Return:           void.
**********************************************************************/
T_VOID Fwl_StoreAllInt(T_VOID);


/*********************************************************************
  Function:         Fwl_RestoreAllInt
  Description:      restore masked interrupt.
  Input:            void.
  Return:           void.
**********************************************************************/
T_VOID Fwl_RestoreAllInt(T_VOID);

/*******************************************************************************
 * @brief   if charger is connect,do nothing,else disconnect,check bat voltage
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_CheckPowerStatus(T_VOID);


/*******************************************************************************
 * @brief   enable watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_EnableWatchdog(T_VOID);

/*******************************************************************************
 * @brief   disenable watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_DisableWatchdog(T_VOID);

/*******************************************************************************
 * @brief   feed the watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FeedWatchdog(T_VOID);

/*******************************************************************************
 * @brief   set the watchdog die time to long time
 * @author  luheshan
 * @date    2013-06-28
 * @param[in]   time: watchdog die time is time*1S + 5S
                               if time==0;will disable long time watch,and dog die time is 6S
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SetLongWatchdog(T_U32 time);

#endif //__FWL_SYSTEM_H__
