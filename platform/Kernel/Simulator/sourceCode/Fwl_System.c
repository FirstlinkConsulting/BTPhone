/*******************************************************************************
 * @file    Fwl_System.c
 * @brief   this file will constraint the access to the bottom layer system 
 *          function, avoid resource competition. Also, this file os for 
 *          porting to different OS
 * @author  Junhua Zhao
 * @date    2005-5-31
 * @version 1.0
*******************************************************************************/
#ifdef OS_WIN32
#include <windows.h>
#endif
#include "anyka_types.h"
#include "w_evtmsg.h"
#include "eng_debug.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Fwl_RTC.h"
#include "Fwl_Detect.h"


static const    T_U8 strVolInfo[]="V:";
static const    T_U8 strFreqInfo[]=" F:";
static const    T_U8 strSwapCnt[]=" S:";
static const    T_U8 strWritbakCnt[]=" W:";
extern unsigned int SM_GetCurrentSM(void);

extern T_VOID Fwl_DetectorMsgDeal(T_BOOL bStatus, T_U16 devInfo);
extern T_VOID Fwl_KeypadMsgDeal(T_U8 keyID, T_U16 pressType);
extern T_VOID Fwl_TimerMsgDeal(T_TIMER timerId, T_U32 deley);
extern T_VOID Fwl_RTCMsgDeal(T_VOID);

T_BOOL Fwl_VolInit(T_VOID)
{
    return AK_TRUE;
}
T_VOID sys_enter_standby(T_VOID)
{
}

/*******************************************************************************
 * @brief   get current bat voltage
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the bat voltage(unit: mv)
*******************************************************************************/
T_U32 Fwl_SysGetBatVolt(T_VOID)
{
    return 1;
}


/*******************************************************************************
 * @brief   printf all infomation of the system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysInfoPrint(T_VOID)
{
	AK_DEBUG_OUTPUT("state=%d\n", SM_GetCurrentSM());
}
extern 	CRITICAL_SECTION cs;
T_VOID Fwl_StoreAllInt(T_VOID)    
{
	EnterCriticalSection(&cs);
}
T_VOID Fwl_RestoreAllInt(T_VOID)   
{    
	LeaveCriticalSection(&cs);

}


/*******************************************************************************
 * @brief   power on to system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOn(T_VOID) 
{   

}


/*******************************************************************************
 * @brief   power off system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOff(T_VOID)
{

}


/*******************************************************************************
 * @brief   deal system message
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysMsgHandle(T_VOID)
{
    ST_MSG msg;

    while (peek_int_message(&msg))
    {
        switch (msg.msg)
        {
        case IM_KEYINFO:
            Fwl_KeypadMsgDeal(msg.bparam, msg.hwparam);
            break;

        case IM_TIMER:
            Fwl_TimerMsgDeal(msg.bparam, msg.hwparam);
            break;

        case IM_RTC:
            Fwl_RTCMsgDeal();
            break;

        case IM_DEVCHANGE:
            Fwl_DetectorMsgDeal(msg.bparam, msg.hwparam);
            break;

        default:
            break;
        }
    }
}



T_VOID Fwl_SysReboot(T_U32 RunAddress)
{   
}


/**
 * @brief let the application sleep for a short time.
 * 
 * @author Pengyu Xue
 * @date 2001-06-18
 * @param T_S16 sleep sleep duration.
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_Sleep(T_U32 delayTime)
{
    Sleep(delayTime);
}

/* other operation */
/**
 * @brief Let speaker ring.
 * 
 * @author Pengyu Xue
 * @date 2001-06-18
 * @param T_U16 dwFreq Sound frequent.
 * @param  T_U16 dwDuration Sound duration.
 * @return T_VOID
 * @retval void
 */
T_VOID   Fwl_Beep(T_U32 dwFreq, T_U32 dwDuration)
{
#ifdef OS_WIN32
    Beep(dwFreq, dwDuration);
#else
#endif
}


/*******************************************************************************
 * @brief   make system enters the standby state
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]wakeupMode: 
 * @return  T_U8
 * @retval  the reason of system waked
*******************************************************************************/
T_VOID Fwl_SysSleep(T_U8 bHasVw)
{   
}


/*******************************************************************************
 * @brief   feed the watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FeedWatchdog(T_VOID)
{

}

/*******************************************************************************
 * @brief   set the watchdog die time to long time
 * @author  luheshan
 * @date    2013-06-28
 * @param[in]   time: watchdog die time is time*1S + 5S
                               if time==0;will disable long time watch,and dog die time is 6S
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SetLongWatchdog(T_U32 time)
{    
}


/* end of files */
