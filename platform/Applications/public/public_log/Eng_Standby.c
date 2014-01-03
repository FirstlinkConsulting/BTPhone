/**
 * @file 
 * @brief Standby function
 * 
 * @author 
 * @date 2013-02-28
 * @version 1.0
 */

#include "Eng_Standby.h"
#include "Eng_AutoOff.h"
#include "anyka_types.h"
#include "gbl_global.h"
#include "eng_debug.h"

#pragma arm section rwdata = "_cachedata_"
/*
    发待机事件的条件之一为gb_StandByCount==0，因此初值不能为0，
    否则可能在重设gb_StandByCount前就发了M_EVT_PUB_STANDBY .
*/
static volatile T_U32           gb_StandByCount = STANDBY_TIME;
static T_BOOL                   gb_StandByFlag = AK_FALSE;
static T_U16                    nCalledCount = 0;
//static T_U16                  StandByTM = 20;

#pragma arm section rwdata


#pragma arm section code = "_frequentcode_"
/**
 * @brief Reset Standby count down
 * 
 * @author @b 
 * @date 2013-3-1
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID UserCountDownReset(T_VOID)
{
    StandbyCountSet(STANDBY_TIME);
    //AutoBgLightOffSet(gb.BgLightTime);
    //AutoPowerOffCountSet(gb.PoffTime);

    return;
}

/**
 * @brief Set standby count down
 * 
 * @author 
 * @date 2013-3-1
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID StandbyCountSet(T_U32 count)
{
    if (gb_StandByCount == STANDBY_DISABLE_ID)     /* disable */
        return;

    gb_StandByCount = count;
    gb_StandByFlag = AK_FALSE;

    return;
}

/**
 * @brief Send event call function standby 
 * 
 * @author 
 * @date 2013-3-1
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID CallStandbyEvent(T_U32 StdbyParam)
{
    T_EVT_PARAM EventParm;
    if (0 != StdbyParam)
    {
        EventParm.w.Param1 = StdbyParam;
        VME_EvtQueuePut(M_EVT_Z02_STANDBY, &EventParm);
    }
    else
    {
        VME_EvtQueuePut(M_EVT_Z02_STANDBY, AK_NULL);
    }
    
    gb_StandByFlag  = AK_TRUE;
    gb_StandByCount = 0;
    
    return;
}

/**
 * @brief Decrease standby count
 * 
 * @author 
 * @date 2013-3-1
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL StandbyCountDecrease(T_U32 millisecond)
{
    T_U32 second = 0;
    static T_U32 num = 0;

    second = millisecond / 1000;
    
    if (gb_StandByCount == STANDBY_DISABLE_ID)     /* disable */
        return AK_FALSE;

    /*if (StandByTM == 0)    //always off 
        return AK_FALSE;*/


    if (gb_StandByCount > second)
        gb_StandByCount -= second;
    else
        gb_StandByCount = 0;
        
    if (++num >= PRINT_TIMES_SEC)
    {
        AK_DEBUG_OUTPUT("sec = %d, cnt = %d, SF = %d\n", second, gb_StandByCount, gb_StandByFlag);
        num = 0;
    }
    
    if (!gb_StandByFlag && gb_StandByCount == 0)    /* Start screen saver */
    {
        /* call function standby */
        CallStandbyEvent(0);
        return AK_TRUE;
    }

    return AK_FALSE;
}

/**
 * @brief Judge standby is ture on or not
 * 
 * @author 
 * @date 2013-3-1
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL StandbyIsOn(T_VOID)
{
    if (gb_StandByCount == STANDBY_DISABLE_ID)
        return AK_FALSE;

    return AK_TRUE;
}
#pragma arm section code 


T_VOID StandbyDisable(T_VOID)
{
    gb_StandByCount = STANDBY_DISABLE_ID;
    nCalledCount++;
}

T_VOID StandbyEnable(T_VOID)
{
    if (nCalledCount>0)
    {
        nCalledCount--;
    }
    else
    {
        return;
    }
    
    if (nCalledCount == 0)
    {
        gb_StandByCount = STANDBY_TIME;
	    gb_StandByFlag  = AK_FALSE;
    }
		
}


