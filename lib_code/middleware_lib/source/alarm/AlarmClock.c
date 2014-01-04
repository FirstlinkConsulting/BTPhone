/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF Alarm Clock logic 
 * @Author：zhao_xiaowei 
 * @Date： 2009-9
 * @Version：
**************************************************************************/
#include "Gbl_Global.h"
#include "Eng_Profile.h"
#include "AlarmClock.h"
#include "Fwl_Timer.h"
#include "Eng_DataConvert.h"
#include "AlarmClock.h"
#include "Alarm_Common.h"
#include "Fwl_System.h"
#include "Fwl_Serial.h"

#if(USE_ALARM_CLOCK)

#define MIN_SPACE_SEC            5
#define DEBUG_OUT                1
#define Print_Time(hintString, time){AK_DEBUG_OUTPUT("\n%s %02d-%02d %02d:%02d:%02d\n", hintString, time.month, time.day, time.hour, time.minute, time.second);}


static T_VOID AlmClk_WriteCofing(T_Clocks* pClocks);
static T_VOID AlmClk_UpdateEnd(T_Clocks* pClocks, T_U8 curIndex);

#if(DEBUG_OUT)
T_VOID AlmClk_Debug(T_VOID);
#endif
#pragma arm section rwdata = "_cachedata_"
static struct {
         T_U32  curIndex: 3;
         T_U32  hour: 5 ;
         T_U32  minute:6;
         T_U32  type: 2;  
         T_U32  recCount:5;
		 T_U32  bRtcInt:1;
         T_U32  reserve: 10;
    }m_alarm={ERROR_INDEX, 0, 0, 0} ;
#pragma arm section rwdata

T_VOID AlmClk_GetRTCtime(T_SYSTIME *pSysTime)
{
#ifdef OS_WIN32
    T_U32 tickCount;
#endif

    AK_ASSERT_PTR_VOID(pSysTime, "AlmClk_GetRTCtime 1 AK_NULL");

#ifdef OS_WIN32
    tickCount = Fwl_GetSecond();
    Utl_Convert_SecondToDate(tickCount, pSysTime);
#else
    //获取系统当前时间，并进行合法性判断
    Fwl_GetRTCtime(pSysTime);
#endif
}

T_U16 AlmClk_GetWeek(const T_SYSTIME* pSysTime)
{
    AK_ASSERT_PTR(pSysTime, "AlmClk_GetWeek 1 AK_NULL",0);
    return WeekDay(pSysTime->year, pSysTime->month, pSysTime->day);
}

T_S32 AlmClk_Compare(const T_SYSTIME* pDesTime, const T_SYSTIME* pSrcTime)
{
    AK_ASSERT_PTR(pDesTime, "AlmClk_Compare 1 AK_NULL", -1);
    AK_ASSERT_PTR(pSrcTime, "AlmClk_Compare 2 AK_NULL", 1);
    if(pDesTime->year == pSrcTime->year)
    {
        if(pDesTime->month == pSrcTime->month)
        {
            if(pDesTime->day == pSrcTime->day)
            {
                if(pDesTime->hour == pSrcTime->hour)
                {
                    return (T_S32)(pDesTime->minute - pSrcTime->minute);
                }
                else
                {
                    return (T_S32)(pDesTime->hour- pSrcTime->hour);
                }
            }
            else
            {
                return (T_S32)(pDesTime->day- pSrcTime->day);
            }
        }
        else
        {
            return (T_S32)(pDesTime->month- pSrcTime->month);
        }
    }
    else
    {
        return (T_S32)(pDesTime->year- pSrcTime->year);
    }
}

T_VOID AlmClk_AddOneDay(T_SYSTIME* pTime)
{
    T_U16 totalMonthDays;

    AK_ASSERT_PTR_VOID(pTime, "pTime is AK_NULL");

    totalMonthDays= MonthDays(pTime->year, pTime->month);
    if(pTime->day == totalMonthDays)
    {
        if(pTime->month == 12)
        {
            pTime->year++;
            pTime->month= 1;
        }
        else
        {
            pTime->month++;
        }
        pTime->day= 1;
    }
    else
    {
        pTime->day++;
    }
}

T_VOID AlmClk_AddOneMinute(T_SYSTIME* pTime)
{
    AK_ASSERT_PTR_VOID(pTime, "pTime is AK_NULL");

    if(pTime->minute == 59)
    {
        if(pTime->hour == 23)
        {
            AlmClk_AddOneDay(pTime);
            pTime->hour= 0;
        }
        else
        {
            pTime->hour++;
        }
       pTime->minute= 0;
    }
    else
    {
        pTime->minute++;
    }
}

T_VOID AlmClk_AddMinutes(T_SYSTIME* pTime, T_U32 minutes)
{
    T_U32 i;

     AK_ASSERT_PTR_VOID(pTime, "pTime is AK_NULL");

     for(i= 0; i< minutes; i++)
     {
         AlmClk_AddOneMinute(pTime);
     }     
}

T_VOID AlmClk_CombinDateTime(T_SYSTIME* desTime, const T_SYSTIME*  pDate, const T_SYSTIME *pTime)
{
    AK_ASSERT_PTR_VOID(desTime, "desTime is AK_NULL");
    AK_ASSERT_PTR_VOID(pDate, "AlmClk_CombinDateTime 1 is AK_NULL");
    AK_ASSERT_PTR_VOID(pTime, "AlmClk_CombinDateTime 2 is AK_NULL");

    *desTime= *pDate;
    desTime->hour= pTime->hour;
    desTime->minute= pTime->minute;
    desTime->second= pTime->second;
}

/**************************************************************************
* @brief start the next clock
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Init()
{
    T_Clocks* pClocks= AK_NULL; 
    T_U8 i;
    pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
    AlmClk_ReadConfig(pClocks);
    
    for(i= 0; i< MAX_CLK_COUNT && NO_CLOCK != GET_CLOCKTYPE(pClocks, i); i++)
    {
        AK_DEBUG_OUTPUT("Alarm_Init idx:%d, recCnt:%d\n",i, GET_RECCOUNT(pClocks, i));
        GET_RECCOUNT(pClocks, i)= 0;
    }
    AlmClk_StartNext(pClocks);
    
    AlmClk_WriteCofing(pClocks);  
    pClocks= (T_Clocks*) Fwl_Free(pClocks);
}


/**************************************************************************
* @brief get the weekday of the onlyone clock's next start clockTime; 
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_U16 AlmClk_GetOnlyOnceClockWeek(const T_SYSTIME* pClockTime)
{
    T_SYSTIME tCur;
    T_SYSTIME clockTime;
    AK_ASSERT_PTR(pClockTime, "AlmClk_GetOnlyOnceClockWeek 1 AK_NULL", 0);
    clockTime= *pClockTime;
    AlmClk_GetRTCtime(&tCur);
    if(AlmClk_Compare(&clockTime, &tCur) < 0)
    {
        AlmClk_AddOneDay(&clockTime);
    }
    return AlmClk_GetWeek(&clockTime);
}

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
T_S8 AlmClk_Validate(const T_ClockSet* pClockSet, T_U8 exceptIndex)
{
    T_U8 i;
    T_S8 valid= 0;
    T_U8 week;
    T_U8 desWeekMap;
    T_Clocks* pClocks;

    AK_ASSERT_PTR(pClockSet, "AlmClk_Validate 1 AK_NULL", MAX_CLK_COUNT);
    pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
    AK_ASSERT_PTR(pClocks, "AlmClk_Validate pClocks is AK_NULL", MAX_CLK_COUNT);
    week= (T_U8)AlmClk_GetOnlyOnceClockWeek(&pClockSet->clockTime);
    desWeekMap= (T_U8)(pClockSet->weekMap &0x7f);

    AlmClk_ReadConfig(pClocks);
    if(ONLYONCE == pClockSet->responseType)
    {
       desWeekMap= (T_U8)(1<< week);
    }
    for(i= 0; (i< MAX_CLK_COUNT) && GET_CLOCKTYPE(pClocks, i) != NO_CLOCK ; i++)
    {
        if(i == exceptIndex)
        {
            continue;
        }
        if( IS_SAME_TIME(pClockSet->clockTime, GET_CLOCKTIME(pClocks, i)))
        {
         
           T_U8 srcWeekMap= (T_U8)(GET_WEEKMAP(pClocks, i)& 0x7f);
           if(ONLYONCE == GET_RESPONSETYPE(pClocks,  i))
           {
               srcWeekMap= (T_U8)(1<< week);
           }

           if(desWeekMap & srcWeekMap)
           {
               valid= -1;
               break;
           }
           
        }
    }
    
    pClocks= (T_Clocks*) Fwl_Free(pClocks);
    if(valid != -1 )
        valid= i;
    return valid;
}

/**************************************************************************
* @brief when user set clockType responseType and clockTime ,want to save 
* then Set the Other infomation to default
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_SetDefaultClockSet(T_ClockSet* pClockSet)
{
   AK_ASSERT_PTR_VOID(pClockSet, "AlmClk_GetDefaultClockSet pClockSet is AK_NULL");

   pClockSet->sleepDuaration= DEF_SLEEPDUARATION;
   if(POWEROFF_CLOCK == pClockSet->clockType)
   {
       pClockSet->clockCount= 1;
   }
   else
   {
       pClockSet->clockCount= DEF_CLOCKCOUNT;
       ToWideChar(pClockSet->clockMusicName, DEF_CLOCKMUSICNAME);
   }
   pClockSet->clockDuaration= DEF_CLOCKDUARATION;
   pClockSet->clockVolume= DEF_CLOCKVOLUME;
   pClockSet->isEnable= AK_TRUE;

   if(WEEKDAYS == pClockSet->responseType)
   {
       pClockSet->weekMap= WEEKDAYS_BITMAP;
   }
   else
   {
       pClockSet->weekMap=0x7f;
   }    
}

/**************************************************************************
* @brief set the clockSet to the index of clock Array
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Set(T_U8 index, const T_ClockSet* pClockSet)
{
    T_Clocks* pClocks;
    
    AK_ASSERT_PTR_VOID(pClockSet, "AlmClk_Set is AK_NULL");
    AK_ASSERT_VAL_VOID(CHK_INDEX_STATE(index), "AlmClk_Set index is invalid");

    pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_Set pClocks is AK_NULL");
    AlmClk_ReadConfig(pClocks);
  
    pClocks->sysClock[index].clockSet= *pClockSet;
    pClocks->sysClock[index].clockSet.clockTime.second= 0;// clear second 
    RESET_CLOCKSTATE(pClocks, index, (*pClockSet));

    AlmClk_WriteCofing(pClocks);  
    pClocks= (T_Clocks*) Fwl_Free(pClocks);   

}

T_VOID AlmClk_SetParam(const T_AlarmClockParam* pAlarmParam)
{
    AK_ASSERT_PTR_VOID(pAlarmParam, "AlmClk_SetParam Ak_NULL");  

    if(CHK_INDEX_STATE(pAlarmParam->index))
    {
        AlmClk_Set(pAlarmParam->index, &pAlarmParam->clockSet);
    }
    
}
/**************************************************************************
* @brief get the clockset of the index clock
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Get(T_U8 index, T_ClockSet* pClockSet)
{
    T_Clocks* pClocks;

    AK_ASSERT_VAL_VOID(CHK_INDEX_STATE(index), "AlmClk_Get index is invalid");
    AK_ASSERT_VAL_VOID(pClockSet, "AlmClk_Get pClockSet is AK_NULL");
    pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_get pClocks is AK_NULL");

    AlmClk_ReadConfig(pClocks);
    *pClockSet= pClocks->sysClock[index].clockSet;
    pClocks= (T_Clocks*) Fwl_Free(pClocks);
}

/**************************************************************************
* @brief add clock to alarm clock array
* @author zhao_xiaowei
* @date 2009-8
* @param clockSet :the alarm clock time to set 
* @return return the empty index of the clock arry, 0 to MAX_CLK_COUNT-1 is 
* is successfull, >=MAX_CLK_COUNT is full, -1 has the clock time conflicts
***************************************************************************/
T_S8 AlmClk_Add(const T_ClockSet* pClockSet)
{
    T_S8 state;
    AK_ASSERT_PTR(pClockSet, "AlmClk_Add is AK_NULL", -1);
    state= AlmClk_Validate(pClockSet, (T_U8)-1);
    if(CHK_INDEX_STATE(state))
    {
        AlmClk_Set((T_U8)state, pClockSet);
    }
    
    return state;
}

/**************************************************************************
* @brief set the clockType  of  index clock to NO_CLOCK
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_Remove(T_U8 index)
{
    T_U32 i;
    T_Clocks* pClocks;

    AK_ASSERT_VAL_VOID(CHK_INDEX_STATE(index), "AlmClk_Remove index is invalid");
    pClocks=  (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_Remove pClocks is AK_NULL");
    AlmClk_ReadConfig(pClocks);
    for(i= index; i< MAX_CLK_COUNT-1 && NO_CLOCK != GET_CLOCKTYPE(pClocks, i+ 1); i++)
    {
        pClocks->sysClock[i]= pClocks->sysClock[i+ 1];
    }
    GET_CLOCKTYPE(pClocks, i)= NO_CLOCK;

    if(GET_CURINDEX(pClocks) == index)
    {
        GET_CURINDEX(pClocks)= ERROR_INDEX; // if the delete index clock is the curindex Clock, then set to errror index
        AlmClk_StartNext(pClocks);
    }
    AlmClk_WriteCofing(pClocks);  
    pClocks= (T_Clocks*) Fwl_Free(pClocks);   
}

/**************************************************************************
* @brief Update the clockSet of index clock 
* eg update clockTime enableState and so on
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_S8 AlmClk_Update(T_U8 index, const T_ClockSet* pClockSet)
{
    T_S8 state;
     AK_ASSERT_PTR(pClockSet, "AlmClk_Update is AK_NULL", -1);
     AK_ASSERT_VAL(CHK_INDEX_STATE(index), "AlmClk_Update index is invalid", index);
     state= AlmClk_Validate(pClockSet, index);
     if(state != -1)//does not conflicts
     {
         AlmClk_Set(index, pClockSet);
     }
     return state;     
}


/**************************************************************************
* @brief get the most recent valid  time of the index alarm clock 
* 
* @author zhao_xiaowei
* @date 2009-8
* @param begTime , the alarm clock must more than or equal begTime
* @return if the clock Time is invalid then return AK_FALSE ,else AK_TRUE
***************************************************************************/
static T_BOOL AlmClk_GetNextTime(T_Clocks* pClocks, T_U8 index, const T_SYSTIME* pBegTime, T_SYSTIME* pStartTime)
{
    T_BOOL state= AK_TRUE;
    T_U16 clockWeek;
    T_SYSTIME clockDate;
    T_SYSTIME clockTime;
    T_SYSTIME clockDateTime;

    AK_ASSERT_VAL(pClocks, "pClocks is AK_NULL", AK_FALSE)
    AK_ASSERT_PTR(pStartTime, "pStartTime is AK_NULL", AK_FALSE);
    AK_ASSERT_VAL(CHK_INDEX_STATE(index), "AlmClk_GetNextTime index is Invalid", AK_FALSE);

    AlmClk_GetRTCtime(&clockDate);    
    clockTime= GET_CLOCKTIME(pClocks, index);

	if( (GET_WEEKMAP(pClocks, index) & 0x7f) == 0 )
	{
		AK_PRINTK("***Invlaide clock:", index, 1);
		return AK_FALSE;
	}
	else
	{
	    while(1)
	    {
	         AlmClk_CombinDateTime(&clockDateTime, &clockDate, &clockTime);
	         clockWeek= AlmClk_GetWeek(&clockDateTime);
	         if((1<< clockWeek) & GET_WEEKMAP(pClocks, index))
	         {
	             if(AlmClk_Compare(&clockDateTime, pBegTime)>= 0)
	             {
	                 break;
	             }
	         }
	         AlmClk_AddOneDay(&clockDate);
	    }       
	        
	        *pStartTime= clockDateTime;
	    return state;
	}
}

/**************************************************************************
* @brief Get the Alarm clock ,the most recent valid time of which is the least
* WR_CLOCKS
* @author zhao_xiaowei
* @date 2009-8
* @param pStartTime[out], the time of next clock when it will start
* @return The index of the alarm clock,which value from 0 to MAX_CLK_COUNT-1 is sucessful,
>= MAX_CLK_COUNT if failed
***************************************************************************/
static T_U8 AlmClk_GetNextClock(T_Clocks *pClocks, T_SYSTIME* pStartTime)
{
    T_SYSTIME tCmp;
    T_SYSTIME tCur;
    T_U8 index= MAX_CLK_COUNT;
    T_U8 i;

    AK_ASSERT_PTR(pStartTime, "pStartTime is AK_NULL", MAX_CLK_COUNT);
    AK_ASSERT_PTR(pClocks, "AlmClk_GetNextClock pClocks is AK_NULL", MAX_CLK_COUNT);
    
    tCmp.year= ERROR_YEAR;
    AlmClk_GetRTCtime(&tCur);
    Print_Time("Get_NextClock cur time", tCur);
    AlmClk_AddOneMinute(&tCur);//and 1 minute
    /*
    if(tCur.second > MIN_SPACE_SEC)
    {
        AlmClk_AddOneMinute(&tCur);//and 1 minute
    }
    */
    for(i= 0; i< MAX_CLK_COUNT; i++)
    {
        if(GET_CLOCKTYPE(pClocks, i) != NO_CLOCK && GET_ISENABLE(pClocks, i))
        {
            AK_DEBUG_OUTPUT("index:%d, recCount:%d",i,GET_RECCOUNT(pClocks, i));
            if(0 == GET_RECCOUNT(pClocks, i))// it has not started
            {
				if(!AlmClk_GetNextTime(pClocks, i, &tCur, pStartTime))// compute the clock time
				{
					continue;
				}
            }
            else
            {
                if(GET_RECCOUNT(pClocks, i) < GET_CLOCKCOUNT(pClocks, i))
                {
                    //space is sleepDuaration and ClockDuaration
                    T_U8 period= (T_U8)(GET_SLEEPDUARATION(pClocks, i)+ GET_CLOCKDUARATION(pClocks, i));
                    *pStartTime= GET_LASTCLOCKTIME(pClocks, i); 
                    Print_Time("last clock Time", GET_LASTCLOCKTIME(pClocks, i));
                    //first and SleepDuaration
                    AlmClk_AddMinutes(pStartTime, GET_SLEEPDUARATION(pClocks, i));
                    while(AlmClk_Compare(pStartTime, &tCur) < 0) // then andd period calculated above
                    {
                        AlmClk_AddMinutes(pStartTime, period);
                    }
                }
            }
            
            if(AlmClk_Compare(pStartTime, &tCmp)< 0) // record the min one
            {
                tCmp= *pStartTime;
                index= i;
            }
        }
    }
#if DEBUG_OUT
    AK_DEBUG_OUTPUT("start clock index:%d, type: %d\n", index, GET_RESPONSETYPE(pClocks, index));
#endif
    
    *pStartTime= tCmp;
    return index;
}



T_VOID RTC_SetALarm(T_SYSTIME* pClockTime)
{    
    AK_ASSERT_PTR_VOID(pClockTime, "RTC_SetALarm AK_NULL");
    #ifdef OS_ANYKA
    Fwl_SetRTC_AlarmTime(AK_TRUE, (RTC_ALARM_TYPE)0, pClockTime);
    #endif
}

static T_VOID AlmClk_GetInfo(T_Clocks* pClocks, T_SYSTIME *pStartTime)
{
    m_alarm.curIndex= GET_CURINDEX(pClocks);
    if(m_alarm.curIndex != ERROR_INDEX)
    {
        m_alarm.hour= pStartTime->hour;
        m_alarm.minute= pStartTime->minute;
        m_alarm.type= GET_CLOCKTYPE(pClocks, m_alarm.curIndex);
        m_alarm.recCount= GET_RECCOUNT(pClocks, m_alarm.curIndex);
        m_alarm.bRtcInt= AK_FALSE;
    }
}   

/**************************************************************************
* @brief find the next clock ,and start
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_StartNext(T_Clocks* pClocks)
{
    T_SYSTIME startTime;
    T_U8 index;
    T_BOOL isToReLoad= AK_FALSE;
    
    if(AK_NULL == pClocks)
    {
		if(m_alarm.bRtcInt)
		{
#ifdef OS_ANYKA
			AK_PRINTK("***AlmClk_StartNext has Start", 0, 1);
#endif
			return ; // if has start ,then return for Manual start Alarm
		}
        isToReLoad= AK_TRUE;
#ifdef OS_ANYKA
        AK_PRINTK("***start next reload file", 0, 1);
#endif
        pClocks= (T_Clocks*)Fwl_Malloc(sizeof(T_Clocks));
        AlmClk_ReadConfig(pClocks);
    }
 
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_StartNext is AK_NULL");    
    index= AlmClk_GetNextClock(pClocks, &startTime);
    if(CHK_INDEX_STATE(index))
    {
        T_SYSTIME curTime;
        T_U32    curSecond;
        T_U32    startSecord;
        //start rtc interrupt
#ifdef OS_ANYKA
        AK_PRINTK("start next recCount:", GET_RECCOUNT(pClocks, index), 0);
        AK_PRINTK("  clkCount:", GET_CLOCKCOUNT(pClocks, index), 1);
#endif
        Print_Time("Start Next Time", startTime);
        GET_CURINDEX(pClocks)= index;

        //LASTCLOCTIME save the Start Clock time
        // when is normal clock, wen play end ,it will update to the current playing end time
        GET_LASTCLOCKTIME(pClocks, index)= startTime;

        Fwl_GetRTCtime(&curTime);
        curSecond= Utl_Convert_DateToSecond(&curTime);
        startSecord= Utl_Convert_DateToSecond(&startTime);
        if(startSecord <= curSecond)
        {
            Print_Time("*** Error curTim:", curTime);
            Print_Time("startTime:", startTime);
            startSecord= curSecond+ MIN_SPACE_SEC;
            Utl_Convert_SecondToDate(startSecord, &startTime);
            GET_LASTCLOCKTIME(pClocks, index)= startTime;
            Print_Time("Re startTime:", startTime);
        } 
        RTC_SetALarm(&startTime);// set alarm interrupt   
        
    }
    else
    {
        GET_CURINDEX(pClocks)= ERROR_INDEX; // set no clock 
        AK_DEBUG_OUTPUT("Start No Clock\n");
    }

    AlmClk_GetInfo(pClocks, &startTime);
    
    if(isToReLoad)
    {
        AlmClk_WriteCofing(pClocks);
        pClocks= (T_Clocks*)Fwl_Free(pClocks);
    }
}

T_VOID AlmClk_DealAlarm(T_eAlarmDealType eAlarmDealType, T_AlarmPlayParam* pAlarmPlayParam)
{
	if(CHK_INDEX_STATE(m_alarm.curIndex) && m_alarm.bRtcInt)
    {
		T_Clocks* pClocks= AK_NULL;
		pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
        m_alarm.bRtcInt= AK_FALSE;
		AK_ASSERT_PTR_VOID(pClocks, "AlmClk_DealAlarm pClocks is AK_NULL");
		AlmClk_ReadConfig(pClocks);
   
  
	   // we suppose it's a right Alarm comming, so the recCount add 1
		 GET_RECCOUNT(pClocks, GET_CURINDEX(pClocks)) = (T_U8)m_alarm.recCount;
		if(m_alarm.curIndex != GET_CURINDEX(pClocks))
		{
			AK_PRINTK("**m_alarm.curIndex != GET_CURINDEX(pClocks)", 0, 1);
		}
		AK_DEBUG_OUTPUT("***Deal Alarm index: %d Time:%d:%d type:%d\n",m_alarm.curIndex, m_alarm.hour, m_alarm.minute, m_alarm.type);
		if(DEAL_POWEROFF == eAlarmDealType)
		{
			if(POWEROFF_CLOCK == m_alarm.type)
			{
				AlmClk_UpdateEnd(pClocks, GET_CURINDEX(pClocks));
				AlmClk_StartNext(pClocks);// start the next clock
			}
			else if(NORMAL_CLOCK == m_alarm.type)
			{
				AlmClk_DealPlayEnd(pClocks, CLOCK_PAUSE);//deal end and start the next 
			}
		}
		else if(DEAL_ALARMPLAY == eAlarmDealType)
		{
			AlmClk_GetAlarmPlayParam(pAlarmPlayParam, &GET_CLOCKSET(pClocks, GET_CURINDEX(pClocks)));
		}

		AlmClk_WriteCofing(pClocks);  
		pClocks= (T_Clocks*)Fwl_Free(pClocks);
	 }
}

T_VOID AlmClk_GetAlarmPlayParam(T_AlarmPlayParam* pPlayParam, const T_ClockSet* pClockSet)
{ 
    AK_ASSERT_PTR_VOID(pPlayParam, "AlmClk_GetAlarmPlayParam AK_NULL");
    AK_ASSERT_PTR_VOID(pClockSet, "AlmClk_GetAlarmPlayParam AK_NULL2");
  
    Utl_UStrCpy(pPlayParam->musicName, pClockSet->clockMusicName);
    pPlayParam->lastMinute= pClockSet->clockDuaration;
    pPlayParam->volume= pClockSet->clockVolume;
}

/**************************************************************************
* @brief put powerof event
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
static __inline T_VOID AlmClk_PowerOff(T_VOID)
{
    T_EVT_PARAM evtParam;
    Fwl_LCD_lock(AK_TRUE);
    evtParam.w.Param1= EVENT_TYPE_SWITCHOFF_MANUAL;
    VME_EvtQueuePut(M_EVT_TIMEOUT, &evtParam);
    VME_EvtQueuePut(M_EVT_Z00_POWEROFF, &evtParam);
    AK_DEBUG_OUTPUT("AlmClk_PowerOff\n");
}

/**************************************************************************
* @brief acording response type to reset reccount
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
static T_VOID AlmClk_UpdateEnd(T_Clocks* pClocks, T_U8 curIndex)
{
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_UpdateEnd AK_NULL");
    AK_ASSERT_VAL_VOID(CHK_INDEX_STATE(curIndex), "AlmClk_UpdateEnd index inValid");

    if(GET_RECCOUNT(pClocks, curIndex) == GET_CLOCKCOUNT(pClocks, curIndex))
    {
        AK_DEBUG_OUTPUT("rec clockCount type:%d\n", GET_RESPONSETYPE(pClocks, curIndex));
        if(WEEKDAYS == GET_RESPONSETYPE(pClocks, curIndex)
            || EVERYDAY == GET_RESPONSETYPE(pClocks, curIndex))
        {
            GET_RECCOUNT(pClocks, curIndex)= 0;  //update recCount,ONLYONCE unchange
        }
        else if(ONLYONCE == GET_RESPONSETYPE(pClocks, curIndex))
        {
            GET_ISENABLE(pClocks, curIndex)= AK_FALSE;//如果是单次闹铃，到了闹铃次数则，设Enble为AK_FALSE
        }
    }
}

T_BOOL  AlmClk_IsComing(T_VOID)
{
    return (T_BOOL)m_alarm.bRtcInt;
}

/**************************************************************************
* @brief thic function is called by prehandle function, when the type of the
* started clock is normal clock , it sends a play clock music message, while it's
* poweroff clock, it sends a poweroff message
* WR_CLOCK
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_ProcessComingMsg(T_BOOL isBatLow)
{
    T_SYSTIME  cur;
    
    AlmClk_GetRTCtime(&cur);
    Print_Time("Process coming:", cur);
    AK_DEBUG_OUTPUT("***ProcessComingMsg index:%d recCount:%d Time:%d:%d type:%d\n", 
                m_alarm.curIndex,m_alarm.recCount, m_alarm.hour, m_alarm.minute, m_alarm.type);
    if(m_alarm.curIndex >= 0 && m_alarm.curIndex <= MAX_CLK_COUNT- 1)
    {
         T_U32 setTime= m_alarm.hour * 60 +  m_alarm.minute;
         T_U32 curTime= (T_U32)cur.hour* 60 + cur.minute;
         T_U32 dif;

         m_alarm.bRtcInt= AK_TRUE;
         if(setTime > curTime)
         {
            dif= setTime- curTime;
         }
         else
         {
            dif= curTime- setTime;
         }
         AK_PRINTK("**dif:", dif, 1);
         if(dif <= DIF_RANGE)//|tCur- tLastClockTime|< DIF_RANGE
        {
            m_alarm.recCount ++;// increament recCount;
            if( NORMAL_CLOCK == m_alarm.type)
            {
                if(isBatLow)// if the bat is low ,then power off system
                {
                    if(SYSTEM_STATE_POWEROFF != gb.init)   // if it's not power off                   
                    {
                        AlmClk_PowerOff();
                    }
                }
                else
                {
                    Fwl_LCD_lock(AK_TRUE);
                    VME_EvtQueuePut(M_EVT_TIMEOUT, AK_NULL);
                    VME_EvtQueuePut(M_EVT_Z01_MUSIC_PLAY, AK_NULL); //set play clock music msg
                    AK_PRINTK("***Send Alarm Play",0, 1);
                }
            }
            else if (POWEROFF_CLOCK == m_alarm.type) 
            {
                if(SYSTEM_STATE_POWEROFF != gb.init)   // if it's not power off                   
                {
                   AlmClk_PowerOff();
                }
            }
            AK_PRINTK("real clk is comming!", 0, 1);
        }
        else
        {
             AlmClk_DealAlarm(DEAL_POWEROFF, AK_NULL);
#ifdef OS_ANYKA
            AK_PRINTK("errr time is coming idx:", m_alarm.curIndex, 1);
#endif
   
        }
    }
}


/**************************************************************************
* @brief when the clock music plays end ,call this function to update infomation
* @of the clocks, calculate the next clock, and judge whether power off system
* WR_CLOCK
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_DealPlayEnd(T_Clocks *pClocks, T_eClockEnd clockEnd)
{
    T_SYSTIME begTime;
    T_SYSTIME endTime;
    T_SYSTIME startTime;
    T_U8 curIndex;
    T_U8 i;
    T_BOOL isToReLoad= AK_FALSE;
    T_BOOL isPowerOff= AK_FALSE;

    AlmClk_GetRTCtime(&endTime); //record end time
 
#ifdef OS_ANYKA
    Print_Time("DealPlayEnd time", endTime);
#endif
    if(AK_NULL == pClocks)
    {
        isToReLoad= AK_TRUE;
        pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
        AlmClk_ReadConfig(pClocks);
    }
    
    AK_ASSERT_PTR_VOID(pClocks, "AlmClk_Set pClocks is AK_NULL");

   
    curIndex=  GET_CURINDEX(pClocks);
    if( curIndex != ERROR_INDEX)
    {
        begTime= GET_LASTCLOCKTIME(pClocks, curIndex);
        GET_LASTCLOCKTIME(pClocks, curIndex)= endTime;
        if(CLOCK_STOP == clockEnd)
        {
            GET_RECCOUNT(pClocks, curIndex)= GET_CLOCKCOUNT(pClocks, curIndex);
            AK_PRINTK("**End today clock", 0, 1);
            //to end the current clock
        }
        else
        {
            AK_PRINTK("**pause cur clock", 0, 1);
        }
        AlmClk_UpdateEnd(pClocks, curIndex);       

        for(i= 0; i< MAX_CLK_COUNT && GET_CLOCKTYPE(pClocks, i) != NO_CLOCK; i++)
        {
            if(curIndex != i && GET_RECCOUNT(pClocks, i) < GET_CLOCKCOUNT(pClocks, i))// if the clock is not end ,then update the recCount
            {
				if(!AlmClk_GetNextTime(pClocks, i, &begTime, &startTime))
				{
					continue;
				}
                if(AlmClk_Compare(&startTime, &endTime) <= 0)
                {
                    if(POWEROFF_CLOCK == GET_CLOCKTYPE(pClocks, i) && GET_ISENABLE(pClocks, i))//if it's power off,then set the symbol
                    {
                         isPowerOff= AK_TRUE;
                    }
                    GET_RECCOUNT(pClocks, i) += 1;
                    AK_DEBUG_OUTPUT("clockIndex:%d, curIndexClk:%d add 1 to %d\n", curIndex, i, GET_RECCOUNT(pClocks, i));
                    GET_LASTCLOCKTIME(pClocks, i)= endTime;

                    AlmClk_UpdateEnd(pClocks, i);
                }
            }
        }// maybe must to change to update between endTime and startTime not equal

        GET_CURINDEX(pClocks)= ERROR_INDEX;//when ended ,then set the current clock to invalid
    }

    if(isPowerOff)
    {
        AlmClk_PowerOff();
    }
    
    AlmClk_StartNext(pClocks);

    if(isToReLoad)
    {
        AlmClk_WriteCofing(pClocks);  
        pClocks= (T_Clocks*)Fwl_Free(pClocks);
    }
    //else if you want to save pClocks you must Write back by you self
}

/**************************************************************************
* @read clocks info from sysconfig file
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_ReadConfig(T_Clocks* pClocks)
{
    AK_ASSERT_PTR_VOID(pClocks, "pClocks is AK_NULL");
    Profile_ReadData(CFG_CLOCK_OFFSET, pClocks);
}

#if(DEBUG_OUT)
T_VOID AlmClk_Debug(T_VOID)
{
	T_U32 i;
	T_Clocks* pClocks= AK_NULL;
	pClocks= (T_Clocks* )Fwl_Malloc(sizeof(T_Clocks));
	AK_ASSERT_PTR_VOID(pClocks, "AlmClk_DealAlarm pClocks is AK_NULL");
	AlmClk_ReadConfig(pClocks);

    AK_DEBUG_OUTPUT("curIndex:%d recCnt:%d bInt:%d h:%d m:%d\n", m_alarm.curIndex,m_alarm.recCount, m_alarm.bRtcInt, m_alarm.hour, m_alarm.minute);
	
    AK_DEBUG_OUTPUT("File CurIndex:%d\n", GET_CURINDEX(pClocks));
    for(i= 0; i < MAX_CLK_COUNT; i++)
	{
		AK_DEBUG_OUTPUT("index:%d, type:%d recCount:%d dua:%d stm:%d\n", i, GET_CLOCKTYPE(pClocks, i), GET_RECCOUNT(pClocks, i), GET_CLOCKDUARATION(pClocks, i), GET_SLEEPDUARATION(pClocks, i));
        Print_Time("clockTime:", GET_CLOCKTIME(pClocks, i));
		Print_Time("lastClockTime:", GET_LASTCLOCKTIME(pClocks, i));
		Printf_UC(GET_CLOCKMUSICNAME(pClocks, i));
		AK_DEBUG_OUTPUT("***********************************************\n");
	}

	pClocks= (T_Clocks*)Fwl_Free(pClocks);
}
#endif

/**************************************************************************
* @write clocks to sysconfig file
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_VOID AlmClk_WriteCofing(T_Clocks* pClocks)
{
    AK_ASSERT_PTR_VOID(pClocks, "pClocks is AK_NULL");
    Profile_WriteData(CFG_CLOCK_OFFSET, pClocks);
}

/**************************************************************************
* @brief the alarm clcok RTC interrupt callback function
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
#pragma arm section code = "_bootcode1_"
T_VOID AlmClk_InterruptCallBack(T_VOID)
{
   VME_EvtQueuePut(M_EVT_ALARM_CLOCK, AK_NULL);
   Fwl_ConsoleWriteChr('~');
}
#pragma arm section code 

/**************************************************************************
* @brief get the check code 
* 
* @author zhao_xiaowei
* @date 2009-8
* @param 
* @return 
***************************************************************************/
T_U8 AlmClk_GetCheckCode(T_ALARMCLOCK_CFG* pClockCfg)
{
    T_U8* pByte= (T_U8* )pClockCfg;
    T_U8 checkCode= 0;
    T_U32 size= sizeof(T_ALARMCLOCK_CFG);
    T_U32 step= size>>3;
    T_U32 i;
    
    AK_ASSERT_PTR(pClockCfg, "AlmClk_GetCheckCode AK_NULL", 0);
    for(i= 1; i< 8; i++)
    {
        checkCode = (T_U8)(checkCode ^pByte[i* step]);
    }
    return checkCode;
}
#endif

