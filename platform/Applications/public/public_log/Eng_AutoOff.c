/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME Eng_AutoOff.c
 * @BRIEF auto power off and auto background light off
 * @Author：Huang_ChuSheng
 * @Date：2008-04-30
 * @Version：
**************************************************************************/
#include "anyka_types.h"
#include "gbl_global.h"
#include "eng_autooff.h"
#include "Fwl_FreqMgr.h"
#include "log_aud_control.h"
#include "log_record.h"
#include "Eng_Profile.h"
#include "Fwl_System.h"
#include "Alarm_Common.h"
#include "Eng_Debug.h"
#include "Fwl_lcd.h"

#pragma arm section zidata = "_bootbss_"
/* use the highest bit for a enable flag of auto power off
 * 1 for enable , 0 for disable;
**/
T_U16 gb_AutoOffCount = 0;
T_U16 gb_AutoPOCountSleep =0;


/* use the highest two bit for a flag, so gb_BgLightOffCoun must be less than 64
 * 1 for enable , 0 for disable;
**/
T_U8 gb_BgLightOffCount = 0; 
T_U8 gb_TimeOutCoutn = 0;
T_U32 powerdown_vol  = 0;
#pragma arm section zidata

#define AD_SAMPLE_OFFSET    3//BAT的检测跟万用表测得的值有0.03V的误差
void bglight_off(void)
{
    Fwl_LCD_off();
    if (!IsInVoicePlay())//if in voice play it do nothing
    {
        if(!Aud_AudCtrlFreqSet())   
        {
             if(AudioRecord_GetRecState() != eSTAT_REC_STOP)
             {
                ;
             }
            else
            {
//                Fwl_FreqPush(FREQ_APP_SCR_SAVE);
            }
        }
    }
}
#pragma arm section code = "_sysinit_"
T_VOID Eng_SetPowerDownVol(T_U32 vol)
{
    powerdown_vol = (vol-AD_SAMPLE_OFFSET)*10;
    akerror("powerdown_vol:",powerdown_vol,1);
}
#pragma arm section code


#pragma arm section code = "_frequentcode_"
T_VOID bglighton_freq_set(T_VOID)
{
    if(!IsInVoicePlay())//if in voice play it do nothing
    {
        if (!Aud_AudCtrlFreqSet())  
        {
            if(AudioRecord_GetRecState() != eSTAT_REC_STOP)
            {
                ;
            }
            else
            {
//              Fwl_FreqPop();
            }
        }
    }
}

void bglight_on(T_BOOL bDelayEnable)
{
    AutoBgLightOffSet(gb.BgLightTime);
    AutoPowerOffCountSet(gb.PoffTime);
    Fwl_LCD_on(bDelayEnable);
    bglighton_freq_set();
}


T_BOOL bglight_state_off(T_VOID)
{
    if (gb_BgLightOffCount == MASK_BIT_7)
        return AK_TRUE;
    else
        return AK_FALSE;
}

/************************************************************************
 * @BRIEF decrease power off time per second
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-04-30
 * @PARAM T_VOID
 * @RETURN T_BOOL 
 * @RETVAL If send power off event, return true;
 **************************************************************************/
T_VOID AutoPowerOffCountDecrease(T_VOID)
{
    T_BOOL  bPoweroff = AK_FALSE;
    //AK_DEBUG_OUTPUT("gb.PoffTime %d, gb_AutoOffCount=%x\n", gb.PoffTime, gb_AutoOffCount);
    //IF gb.PoffTime=0 or in save mode and playing music, disable to auto power off.
 //   if((0 != gb.PoffTime)||(0 != gb.PoffTimeSleepMode))
    {
        if(0 != gb.PoffTime)
        {
            if((gb_AutoOffCount&MASK_BIT_15)!=0&&AK_TRUE == Aud_AudCtrlAvaPowerOff())   
            {
                if ((gb_AutoOffCount&(~MASK_BIT_15)) >= 1)
                {
                    gb_AutoOffCount--;
                    if(MASK_BIT_15 == gb_AutoOffCount)
                        bPoweroff = AK_TRUE;                    
                }

            }
        }

        if(0 != gb.PoffTimeSleepMode)
        {
            if (gb_AutoPOCountSleep >= 1)
            {
                gb_AutoPOCountSleep--;
                if(0 == gb_AutoPOCountSleep)
                    bPoweroff = AK_TRUE;
            }
        }

    }

/*
    if ((0 == gb.PoffTime) || (gb.PoffTime <= 60 && (AK_FALSE == Aud_AudCtrlAvaPowerOff())))
    {   
        return AK_FALSE;
    }
    
    if ((gb_AutoOffCount & MASK_BIT_15) > 0)
    {
        if ((gb_AutoOffCount&(~MASK_BIT_15)) > 1)
        {
            gb_AutoOffCount--;
        }
        else
        {
            gb_AutoOffCount = MASK_BIT_15;
        }
*/

    if(bPoweroff)
    {   // call function to show auto power off 
        T_EVT_PARAM pEventParm;
   
        pEventParm.w.Param1 =   EVENT_TYPE_SWITCHOFF_AUTO;
        VME_EvtQueuePut(M_EVT_TIMEOUT, &pEventParm);
        AK_DEBUG_OUTPUT("auto power off %d\n", gb_AutoOffCount);
        VME_EvtQueuePut(M_EVT_Z00_POWEROFF, &pEventParm);
        Fwl_LCD_lock(AK_TRUE);
        gb_AutoPOCountSleep = 0;
     }

    return;
}

/************************************************************************
 * @BRIEF decrease background light off time per second
 * @AUTHOR Huang_ChuSheng
 * @DATE  2008-05-03
 * @PARAM T_VOID
 * @RETURN T_BOOL 
 * @RETVAL If send background light off event, return true;
 **************************************************************************/
T_BOOL AutoBgLightOffCountDecrease(T_VOID)
{
    gb_TimeOutCoutn++;
    if (gb_TimeOutCoutn>6)
    {
        T_EVT_PARAM EventParm;
   
        EventParm.w.Param1 =   EVENT_TYPE_TIMEOUT_NORMAL;

        gb_TimeOutCoutn = 0;
        VME_EvtQueuePut(M_EVT_TIMEOUT, &EventParm);
    }
    //AK_DEBUG_OUTPUT("gb.BgLightTime %d\n", gb.BgLightTime);
    if (0 == gb.BgLightTime)    /* disable */
    {   
        return AK_FALSE;
    }

    if(!Aud_AudCtrlAvaBLOff())
    {
        return AK_FALSE;
    }

    if ((gb_BgLightOffCount & MASK_BIT_7) > 0)
    {
        if ((gb_BgLightOffCount & (~MASK_BIT_7)) > 1)
        {
            gb_BgLightOffCount--;
        }
        else
        {
            gb_BgLightOffCount &= 0xc0;
        }
      
        //AK_DEBUG_OUTPUT("gb_BgLightOffCount: %d\n", gb_BgLightOffCount&0x3f);
        if (gb_BgLightOffCount == 0xc0) // background light off
        {
            //VME_EvtQueuePut(M_EVT_Z01_BGLIGHTOFF, AK_NULL);            
            gb_BgLightOffCount = MASK_BIT_7;
            bglight_off();
      
            return AK_TRUE;
        }
    }

    return AK_FALSE;
}

T_U8 Eng_VolDetect(T_U8 *vol)
{
#ifdef OS_ANYKA
    T_U32 volt = 0;

    volt = Fwl_SysGetBatVolt();
    {      
        if(volt<(BATTERY_VALUE_SDOWN))
        {
        	*vol = 0;
            return BATTERY_STAT_LOW_SHUTDOWN;   
        }
		
		*vol = (volt - BATTERY_VALUE_SDOWN)*10/(BATTERY_VALUE_CHARGE-BATTERY_VALUE_SDOWN);
		if(*vol >= 10)
		{
			*vol = 9;
		}
		
		if(volt<BATTERY_VALUE_WARN)
            return BATTERY_STAT_LOW_WARN;
        else if(volt<BATTERY_VALUE_NOR_L1)
            return BATTERY_STAT_NOR_L0;
        else if(volt<BATTERY_VALUE_NOR_L2)
            return BATTERY_STAT_NOR_L1;
        else if(volt<BATTERY_VALUE_NOR_L3)
            return BATTERY_STAT_NOR_L2;
        else if(volt<BATTERY_VALUE_MAX)
            return BATTERY_STAT_NOR_L3;
        else if(volt<BATTERY_VALUE_CHARGE)
            return BATTERY_STAT_NOR_FULL;
        else
            return BATTERY_STAT_EXCEEDVOLT;
    }
#else
		*vol = BATTERY_VALUE_CHARGE;
        return BATTERY_STAT_EXCEEDVOLT;
#endif
}

T_VOID AutoPOffCountSetSleep(T_U16 count,T_BOOL bSleep)
{   
    if ((!bSleep) && (count != 0) && (count <= 60) && ((MASK_BIT_15&gb_AutoOffCount)!=0))
    {
        gb_AutoOffCount = (T_U16) (MASK_BIT_15|count);
    }
      
    if ((bSleep) && (count != 0) && (count <= 120*60))
    {
        gb_AutoPOCountSleep = count;
    }
}

T_VOID AutoBgLightOffSet(T_U8 count)
{
    gb_BgLightOffCount &= MASK_BIT_7;
    gb_BgLightOffCount |= count;
    gb_BgLightOffCount |= 0x40;
    gb_TimeOutCoutn = 0;
}

#pragma arm section code 

