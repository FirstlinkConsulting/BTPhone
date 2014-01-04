/**
 * @file Ctrl_LedConfig.c
 * @brief common led config
 * @copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * 
 * @author 
 * @date  
 * @version 1,0 
 */

#include "Anyka_types.h"
#include "Eng_LedHint.h"
#include "Eng_Debug.h"
#include "Fwl_osMalloc.h"
#include "Fwl_MicroTask.h"
#include "Fwl_LED.h"
#include "Fwl_Timer.h"
#include <string.h>

#ifdef SUPPORT_LEDHINT


#define     LED_ERROR_ID     0xff


#define LED_CTRL(index) led_parm.pLed[index]

typedef struct
{
    T_U8                MaxMode;            //模式的最大总数
    T_U8                LedNum;            //闪烁LED灯的数量
    T_U8                ModeNum;            //模式实际注册总数
    T_U8                LedTaskId;          //微任务返回ID
    T_U8                CurLedMode;         //当前模式下标号
    volatile T_U32      LiveMode;           //活动的LED模式标志，1bit代表一种模式
    
    T_U32               led_seq_cnt;        //计算当前执行到的LED序列位置
    T_U32               led_delay_cnt;      //计算当前执行到的delay位置
    T_LED_CTRL          **pLed;             //各模式LED控制配置的指针数组
}T_LED_PARM;


static T_LED_PARM led_parm = {0};

static T_U8 LedHint_MaxPrioMode(T_VOID)
{
    T_S8 i;
    T_U8 mode;

    mode = 0;
    
    //先找到一个激活的模式
    for(i=led_parm.ModeNum; i>=0; --i)
    {
        if (led_parm.LiveMode & (0x1 << i))
        {
            mode = i;
            break;
        }
        else if(0 == i)
        {
            AK_DEBUG_OUTPUT("@@@ No Live mode !!\n");
            return 0xff;
        }
    }

    //再比较筛选出优先级最高的模式
    for(i=led_parm.ModeNum; i>0; --i)
    {
        if (led_parm.LiveMode & (0x1 << i))
        {
            if (led_parm.pLed[mode]->prioty
                < led_parm.pLed[i]->prioty)
            {
                mode = i;
            }
        }
    }

    AK_DEBUG_OUTPUT("Max prioty Mode:%d\n", mode);
    return mode;
}
#pragma arm section code = "_frequentcode_"
static T_VOID led_ctrl_task(T_VOID)
{
    T_U32 led_seq_len   = 0;
    T_U32 led_state_seq = 0;
    T_U32 led_delay_len = 0;

    T_U8  led_mask = 0;

    led_seq_len   = led_parm.pLed[led_parm.CurLedMode]->led_seq_len;
    led_state_seq = led_parm.pLed[led_parm.CurLedMode]->led_state_seq;
    led_delay_len = led_parm.pLed[led_parm.CurLedMode]->led_delay_len;

    // 显示序列中的元素
    if (led_parm.led_seq_cnt < led_seq_len)
	{
#if 0	
        switch((led_state_seq >> (led_parm.LedNum * led_parm.led_seq_cnt)) & 0x03)
    	{
    		case LED_SHOW_OFF:
    			Fwl_LEDOff(LED_SHOW_DOUBLE);
    			break;
    		case LED_SHOW_RED:
    			Fwl_LEDOn(LED_SHOW_RED);
                Fwl_LEDOff(LED_SHOW_BLUE);
    			break;
    		case LED_SHOW_BLUE:
    			Fwl_LEDOn(LED_SHOW_BLUE);
                Fwl_LEDOff(LED_SHOW_RED);
    			break;
            case LED_SHOW_DOUBLE:
    			Fwl_LEDOn(LED_SHOW_DOUBLE);
    			break;
    		default:
    			break;
    	}
#endif
        led_mask = ((led_state_seq >> (led_parm.LedNum * led_parm.led_seq_cnt)) 
                        << (32-led_parm.LedNum) >> (32-led_parm.LedNum));
        Fwl_LEDOn(led_mask);
        Fwl_LEDOff(~led_mask);

    	led_parm.led_seq_cnt++;
    }
    else
    {
        // 是否需要延时
        if (led_parm.led_delay_cnt < led_delay_len)
        {
            led_parm.led_delay_cnt++;
        }
        else
        {
            // 两个计数器清0，重复显示序列
            led_parm.led_seq_cnt = 0;
            led_parm.led_delay_cnt = 0;

            // 有显示序列，马上显示
            if (led_seq_len != 0)
            {
                // 非递归，显示一次即退出
                led_ctrl_task();
            }
        }
    }
}
#pragma arm section code

/**
* @brief init led_parm, malloc led array by MaxMode
*
* @author 
* @date 2013-05-10
*
* @param in T_U8 MaxMode: max mode num
*           T_U8 Led_num: blink led num
*
* @return T_BOOL
* @retval 
*/
T_BOOL LedHint_Init(T_U8 MaxMode, T_U8 Led_num)
{
    if (32 < MaxMode)
    {
        AK_DEBUG_OUTPUT("LedHint_Init ModeNum:%d, bigger than 32!\n", MaxMode);
        return AK_FALSE;
    }
    
    led_parm.MaxMode       = MaxMode;
    led_parm.LedNum        = Led_num;
    led_parm.ModeNum       = 0;
    led_parm.CurLedMode    = 0;
    led_parm.LedTaskId     = LED_ERROR_ID;
    led_parm.LiveMode      = 0;
    led_parm.led_seq_cnt   = 0;
    led_parm.led_delay_cnt = 0;

    if (AK_NULL != led_parm.pLed)
    {
        AK_DEBUG_OUTPUT("LedCfg_Init pLed is not null!\n");
        led_parm.pLed = Fwl_Free(led_parm.pLed);
    }

    led_parm.pLed = (T_U32*)Fwl_Malloc(MaxMode * sizeof(T_U32));
    AK_ASSERT_PTR(led_parm.pLed, "LedCfg_Init pLed", AK_FALSE);
    memset(led_parm.pLed, 0, MaxMode * sizeof(T_U32));

    Fwl_LEDInit();

    return AK_TRUE;
}


/**
* @brief free led array, clean led_parm
*
* @author 
* @date 2013-04-10
*
* @param T_VOID
*
* @return T_VOID
* @retval 
*/
T_VOID LedHint_Free(T_VOID)
{
    if (AK_NULL != led_parm.pLed)
    {
        led_parm.pLed = Fwl_Free(led_parm.pLed);
    }

    led_parm.MaxMode       = 0;
    led_parm.LedNum        = 0;
    led_parm.ModeNum       = 0;
    led_parm.CurLedMode    = 0;
    led_parm.LedTaskId     = LED_ERROR_ID;
    led_parm.LiveMode      = 0;
    led_parm.led_seq_cnt   = 0;
    led_parm.led_delay_cnt = 0;
}


/**
 * @brief Add led indicate mode
 * @author hyl
 * @date 2013-5-14
 * @param led_mode: mode id
 * @param ledcfg: control blink ways
 * @return T_VOID
 */
T_BOOL LedHint_Add(T_U8 led_mode, T_LED_CTRL *ledctrl)
{
    T_U8 index = 0xff;

    if (AK_NULL == led_parm.pLed)
    {
        AK_DEBUG_OUTPUT("LedCfg_AddLed pLed is null, please call LedCfg_Init first!\n");
        return AK_FALSE;
    }

    if (led_parm.ModeNum == led_parm.MaxMode)
    {
        AK_DEBUG_OUTPUT("LedCfg_AddLed led array is full, can't add one more!\n");
        return AK_FALSE;
    }

    if (led_parm.MaxMode < led_mode)
    {
        AK_DEBUG_OUTPUT("LedCfg_AddLed ModeId is bigger than maxnum!\n");
        return AK_FALSE;
    }

    index = led_mode;

    led_parm.pLed[index] = ledctrl;

    led_parm.ModeNum++;

    return AK_TRUE;
}


/**
 * @brief execute led mode
 * @author hyl
 * @date 2013-5-14
 * @param T_VOID
 * @return T_VOID
 */
T_BOOL LedHint_Exec(T_U8 Ledmode)
{
    T_U8    MaxMode;
    
    if ((AK_NULL == led_parm.pLed)
        || (AK_NULL == led_parm.pLed[Ledmode])
        || (0 == led_parm.ModeNum)
        || (Ledmode >= led_parm.MaxMode))
    {
        AK_DEBUG_OUTPUT("## LedHint_Exec error\n");
        return AK_FALSE;
    }

    led_parm.LiveMode |= (0x1 << Ledmode);

    //判断是否当前模式的优先级更高。
    if (led_parm.pLed[Ledmode]->prioty
        < led_parm.pLed[led_parm.CurLedMode]->prioty)
    {
        AK_DEBUG_OUTPUT("Current LedMode prioty is higher!!\n");
        return AK_FALSE;
    }

    //判断是否有优先级更高模式要执行
    MaxMode = LedHint_MaxPrioMode();
    if ((0xff != MaxMode)
        && (led_parm.pLed[Ledmode]->prioty
            < led_parm.pLed[MaxMode]->prioty))
    {
        led_parm.CurLedMode = MaxMode;
    }
    else
    {
        led_parm.CurLedMode = Ledmode;
    }
    
    if (LED_ERROR_ID != led_parm.LedTaskId)
    {
        Fwl_MicroTaskUnRegister(led_parm.LedTaskId);
        led_parm.LedTaskId = LED_ERROR_ID;
        Fwl_LEDOff(LED_SHOW_DOUBLE);
    }

    if (0 != led_parm.pLed[led_parm.CurLedMode]->time_interval)
    {
        led_parm.LedTaskId = Fwl_MicroTaskRegister(led_ctrl_task, led_parm.pLed[led_parm.CurLedMode]->time_interval);
        Fwl_MicroTaskResume(led_parm.LedTaskId);
    }
    else
    {
        Fwl_LEDOff(LED_SHOW_DOUBLE);
    }

    return AK_TRUE;
}


/**
 * @brief stop led indicate mode
 * @author hyl
 * @date 2013-5-14
 * @param T_VOID
 * @return T_VOID
 */
T_VOID LedHint_Stop(T_U8 Ledmode)
{
    Fwl_MicroTaskUnRegister(led_parm.LedTaskId);
    Fwl_LEDOff(LED_BLUE|LED_RED);
    led_parm.LedTaskId = LED_ERROR_ID;
    #ifdef SUPPORT_BLUETOOTH
    if(1 == Ledmode)
    {
        if(led_parm.LiveMode & (0x11<<9))    //低电、充满有效
        {
            led_parm.LiveMode &= ~(0x1 << Ledmode);
        }
        else
        {
            led_parm.LiveMode = 2;
        }
    }
    else
	#endif
    {
        led_parm.LiveMode &= ~(0x1 << Ledmode);
    }
    led_parm.CurLedMode = LedHint_MaxPrioMode();
    LedHint_Exec(led_parm.CurLedMode);
}

/**
 * @BRIEF	stop led already or not 
 * @AUTHOR	
 * @DATE	2012-12-20
 * @PARAM	T_VOID
 * @RETURN	AK_TURE: is stop, AK_FALSE: isn't stop
 */
T_BOOL LedHint_Task_Isstop(T_VOID)
{
    return (led_parm.LedTaskId == LED_ERROR_ID);
}


#endif 
