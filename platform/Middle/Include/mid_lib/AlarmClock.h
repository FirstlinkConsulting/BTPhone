/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author£ºzhao_xiaowei
 * @Date£º 2009-8-19
 * @Version£º
**************************************************************************/
#ifndef  __ALARMCLOCK_H__
#define  __ALARMCLOCK_H__

#include "anyka_types.h"
#include "Eng_Time.h"
#include "Gbl_Global.h"

#if(USE_ALARM_CLOCK)
#include "log_file_com.h"

#define ERROR_INDEX             MAX_CLK_COUNT
#define WEEKDAYS_BITMAP         0x3E

#define DEF_SLEEPDUARATION      2 //5
#define DEF_CLOCKCOUNT          3 //5
#define DEF_CLOCKDUARATION      1 // 3
#define DEF_CLOCKVOLUME         DEF_VOLUME
#define DEF_WEEKMAP             0x7f
#define DEF_ISENABLE            AK_TRUE
#define DEF_CLOCKMUSICNAME      "A:/clock.wav"

#define DIF_RANGE   2

#define AlmClk_DisableRTC()

#define GET_SLEEPDUARATION(pClocks, index)          (pClocks->sysClock[index].clockSet.sleepDuaration)
#define GET_CLOCKCOUNT(pClocks, index)              (pClocks->sysClock[index].clockSet.clockCount)
#define GET_CLOCKDUARATION(pClocks, index)          (pClocks->sysClock[index].clockSet.clockDuaration)
#define GET_CLOCKVOLUME(pClocks, index)             (pClocks->sysClock[index].clockSet.clockVolume)
#define GET_ISENABLE(pClocks, index)                (pClocks->sysClock[index].clockSet.isEnable)
#define GET_RESPONSETYPE(pClocks, index)            (pClocks->sysClock[index].clockSet.responseType)
#define GET_CLOCKMUSICNAME(pClocks, index)          (pClocks->sysClock[index].clockSet.clockMusicName)
#define GET_CLOCKTYPE(pClocks, index)               (pClocks->sysClock[index].clockSet.clockType)
#define GET_CLOCKTIME(pClocks, index)               (pClocks->sysClock[index].clockSet.clockTime)
#define GET_WEEKMAP(pClocks, index)                 (pClocks->sysClock[index].clockSet.weekMap)
#define GET_RECCOUNT(pClocks, index)                (pClocks->sysClock[index].clockState.recCount)
#define GET_LASTCLOCKTIME(pClocks, index)           (pClocks->sysClock[index].clockState.lastClockTime)

#define GET_CURINDEX(pClocks)                       (pClocks->clockGblState.curClock)
#define GET_CLOCKSET(pClocks, index)                (pClocks->sysClock[index].clockSet)

#define IS_SAME_TIME(time1, time2)  (time1.hour == time2.hour && time1.minute == time2.minute)
#define IS_SAME_DAY(time1, time2) (time1.year == time2.year && time1.month == time2.month && time1.day == time2.day)
#define MIN_VALID_CLOCKTIME(time){Fwl_GetRTCtime(&time); AlmClk_AddOneMinute(&time);}

#define RESET_CLOCKSTATE(pClocks, index, clockSet) { pClocks->sysClock[index].clockState.recCount= 0;\
            pClocks->sysClock[index].clockState.lastClockTime= clockSet.clockTime;\
            }

#define ERROR_YEAR  (END_YEAR+ 1)
#define CHK_INDEX_STATE(index)   (index < MAX_CLK_COUNT)
typedef T_VOID (*F_ALARM_CLOCK_INTRRUPT)(T_VOID);

typedef struct  {
    T_U8 index;
    T_ClockSet clockSet;
}T_AlarmClockParam;

typedef struct  {
    T_U16 musicName[MAX_FILE_LEN+ 1];
    T_U8 volume;
    T_U8 lastMinute;
}T_AlarmPlayParam;

typedef enum
{
    CLOCK_PAUSE,
    CLOCK_STOP
}T_eClockEnd;

typedef enum
{
    CLOCK_SUCCESS,
    CLOCK_SAME,
    CLOCK_FULL
}T_eClockAddState;

typedef enum
{
    DEAL_POWEROFF,
    DEAL_ALARMPLAY
}T_eAlarmDealType;
/**************************************************************************
* @brief calculate the next clock ,and start alarm clock
* thif fuction is called in the VME_Main
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Init(T_VOID);
/**************************************************************************
* @brief add clock to alarm clock array
* @author zhao_xiaowei
* @date 2009-8
* @param clockSet :the alarm clock time to set 
* @return return the empty index of the clock arry, 0 to MAX_CLK_COUNT-1 is 
* is successfull, >=MAX_CLK_COUNT is failed
***************************************************************************/
T_S8 AlmClk_Add(const T_ClockSet *pClockSet);

/**************************************************************************
* @brief set the clockType  of  index clock to NO_CLOCK
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Remove(T_U8 index);

/**************************************************************************
* @brief Update the clockSet of index clock 
* eg update clockTime enableState and so on
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_S8 AlmClk_Update(T_U8 index, const T_ClockSet* pClockSet);


/**************************************************************************
* @brief calcuate the next clock ,when it eixstes ,start the clock
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_StartNext(T_Clocks *pClock);


/**************************************************************************
* @brief thic function is called by prehandle function, when the type of the
* started clock is normal clock , it sends a play clock music message, while it's
* poweroff clock, it sends a poweroff message
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_ProcessComingMsg(T_BOOL isBatLow);


/**************************************************************************
* @brief the alarm clcok RTC interrupt callback function
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_InterruptCallBack(T_VOID);


/**************************************************************************
* @brief get the check code 
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_U8 AlmClk_GetCheckCode(T_ALARMCLOCK_CFG* pClockCfg);

/**************************************************************************
* @read clocks info from sysconfig file
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_ReadConfig(T_Clocks* pClocks);

/**************************************************************************
* @brief when user set clockType responseType and clockTime ,want to save 
* then Set the Other infomation to default
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_SetDefaultClockSet(T_ClockSet* pClockSet);

/**************************************************************************
* @brief get the clockset of the index clock
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Get(T_U8 index, T_ClockSet* pClockSet);

/**************************************************************************
* @brief valid the clockset ,check whethe the clocks has the same clocktime
* @or the clock array is fall
* @just Read
* @author zhao_xiaowei
* @date 2009-8
* @param clockSet
* @return return the empty index of the clock arry, 0 to MAX_CLK_COUNT-1 is 
* is successfull, >=MAX_CLK_COUNT is full, -1 has the clock time conflicts
***************************************************************************/
T_S8 AlmClk_Validate(const T_ClockSet* pClockSet, T_U8 exceptIndex);

/**************************************************************************
* @brief set the clockSet to the index of clock Array
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Set(T_U8 index, const T_ClockSet* pClockSet);

/**************************************************************************
* @brief update the pAlarmParam infos
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_SetParam(const T_AlarmClockParam* pAlarmParam);

/**************************************************************************
* @brief when clock play end ,update the play state , and start next clock
* @author zhao_xiaowei
* @date 2009-8
* @param endType ,ClOCK_PAUSE: just stop current alarm playing
* @CLOCK_STOP to cancel the current clock
* @return 
***************************************************************************/
T_VOID AlmClk_DealPlayEnd(T_Clocks* pClocks, T_eClockEnd endType);

/**************************************************************************
* @brief convert clockSet to T_AlarmPlayParam 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_GetAlarmPlayParam(T_AlarmPlayParam* pPlayParam, const T_ClockSet* pClockSet);


T_VOID AlmClk_DealAlarm(T_eAlarmDealType eAlarmDealType,T_AlarmPlayParam* pAlarmPlayParam);
T_BOOL  AlmClk_IsComing(T_VOID);
#endif // end of USE_ALARM_CLOCK
#endif
