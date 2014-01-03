/*******************************************************************************
 * @file    Fwl_Keypad.c
 * @brief   this file will constraint the access to the bottom layer common 
 *          platform function, avoid resource competition. Also, 
 *          this file os for porting to different OS
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "Gpio_Define.h"
#include "Fwl_Keypad.h"
#include "hal_keypad.h"
#include "hal_int_msg.h"
#include "m_event.h"
#include "Eng_Debug.h"
#include "Fwl_Timer.h"


#define KEYGROUP_NUM                (8)
#define POWERKEY_INDEX              (KEYGROUP_NUM)

#ifdef BOARD_SPT1050L
#define KBKEY1                      (kbFUNC)
#define KBKEY2                      (kbMODE)
#define KBKEY3                      (kbRECORD)
#define KBKEY4                      (kbVOLADD)
#define KBKEY5                      (kbVOLSUB)
#define KBKEY6                      (kbRIGHT)
#define KBKEY7                      (kbOK)
#define KBKEY8                      (kbLEFT)
#endif
#ifdef BOARD_SPT1060L
#define KBKEY1                      (kbVOLSUB)
#define KBKEY2                      (kbVOLADD)
#define KBKEY3                      (kbLEFT)
#define KBKEY4                      (kbRIGHT)
#define KBKEY5                      (kbMODE)
#define KBKEY6                      (kbOK)
#define KBKEY7                      (kbFUNC)
#define KBKEY8                      (kbRECORD)
#endif
#ifdef BOARD_SPT1080L
#define KBKEY1                      (kbRECORD)
#define KBKEY2                      (kbBT)
#define KBKEY3                      (kbRIGHT)
#define KBKEY4                      (kbLEFT)
#define KBKEY5                      (kbFUNC)
#define KBKEY6                      (kbOK)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#endif

#ifdef BOARD_SPT1052C

#if (SYS_KEY_MODE == 4)
#define KBKEY1                      (kbLEFT)
#define KBKEY2                      (kbRIGHT)
#define KBKEY3                      (kbRECORD)
#define KBKEY4                      (kbBT)
#define KBKEY5                      (kbFUNC)
#define KBKEY6                      (kbOK)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#elif (SYS_KEY_MODE == 3)
#define KBKEY1                      (kbLEFT)
#define KBKEY2                      (kbRIGHT)
#define KBKEY3                      (kbBT)
#define KBKEY4                      (kbFUNC)
#define KBKEY5                      (kbOK)
#define KBKEY6                      (kbNULL)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#elif (SYS_KEY_MODE == 2)
#define KBKEY1                      (kbLEFT)
#define KBKEY2                      (kbRIGHT)
#define KBKEY3                      (kbFUNC)
#define KBKEY4                      (kbOK)
#define KBKEY5                      (kbNULL)
#define KBKEY6                      (kbNULL)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#elif (SYS_KEY_MODE == 1)
#define KBKEY1                      (kbLEFT)
#define KBKEY2                      (kbRIGHT)
#define KBKEY3                      (kbBT)
#define KBKEY4                      (kbOK)
#define KBKEY5                      (kbNULL)
#define KBKEY6                      (kbNULL)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#elif (SYS_KEY_MODE == 0)
#define KBKEY1                      (kbLEFT)
#define KBKEY2                      (kbRIGHT)
#define KBKEY3                      (kbOK)
#define KBKEY4                      (kbNULL)
#define KBKEY5                      (kbNULL)
#define KBKEY6                      (kbNULL)
#define KBKEY7                      (kbNULL)
#define KBKEY8                      (kbNULL)
#endif
#endif

#define KBKEY1_VAL                  (ADVAL_KEY1)
#define KBKEY2_VAL                  (ADVAL_KEY2)
#define KBKEY3_VAL                  (ADVAL_KEY3)
#define KBKEY4_VAL                  (ADVAL_KEY4)
#define KBKEY5_VAL                  (ADVAL_KEY5)
#define KBKEY6_VAL                  (ADVAL_KEY6)
#define KBKEY7_VAL                  (ADVAL_KEY7)
#define KBKEY8_VAL                  (ADVAL_KEY8)

typedef struct
{
    T_U8 keyID;
    T_U8 press_type;
    T_U16 pressType;
}T_KEY_MSG;


#pragma arm section rodata = "_bootcode1_"
static const T_KEY_DET_STR m_keypad_ad_matrix[KEYGROUP_NUM] =
{
    {(KBKEY1_VAL - OFFSET), (T_U16)(KBKEY1_VAL + OFFSET), KBKEY1},
    {(KBKEY2_VAL - OFFSET), (T_U16)(KBKEY2_VAL + OFFSET), KBKEY2},
    {(KBKEY3_VAL - OFFSET), (T_U16)(KBKEY3_VAL + OFFSET), KBKEY3},
    {(KBKEY4_VAL - OFFSET), (T_U16)(KBKEY4_VAL + OFFSET), KBKEY4},
    {(KBKEY5_VAL - OFFSET), (T_U16)(KBKEY5_VAL + OFFSET), KBKEY5},
    {(KBKEY6_VAL - OFFSET), (T_U16)(KBKEY6_VAL + OFFSET), KBKEY6},
    {(KBKEY7_VAL - OFFSET), (T_U16)(KBKEY7_VAL + OFFSET), KBKEY7},
    {(KBKEY8_VAL - OFFSET), (T_U16)(KBKEY8_VAL + OFFSET), KBKEY8},
};

static const T_KEYPAD_PARAM keypad_param = {
    ADVAL_MIN,              //从m_keypad_ad_matrix所有Min值中找到最小值
    (ADVAL_MAX+ OFFSET),    //从m_keypad_ad_matrix所有Max值中找到最大值
    m_keypad_ad_matrix,
    ADKEY_NUMBER,
    kbPOWER,         //从s_key_id中找出kbPOWER的下标
};
#pragma arm section rodata

static const T_eKEY_ID s_key_id[KEYGROUP_NUM + 1] = 
{
    KBKEY1,
    KBKEY2,
    KBKEY3,
    KBKEY4,
    KBKEY5,
    KBKEY6,
    KBKEY7,
    KBKEY8,
    kbPOWER,
};

static T_KEY_MSG preKey;   //记录前一个按键状态，用于转换PRESS_LONG消息。  


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   initialize keypad
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadInit(T_VOID)
{
    keypad_init(AK_NULL, &keypad_param);
    preKey.keyID = kbNULL;
    preKey.pressType = 0;
}
#pragma arm section code


/*******************************************************************************
 * @brief   enable keypad scan
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]bEnable: enable or disable
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadEnable(T_BOOL bEnable)
{
    if (AK_FALSE == bEnable)
    {
        keypad_disable_scan();
    }
    else
    {
        keypad_enable_scan();
    }
}
#pragma arm section code = "_frequentcode_"


/*******************************************************************************
 * @brief   scan keypad to get key id
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_eKEY_ID
 * @retval  the key id
*******************************************************************************/
T_eKEY_ID Fwl_KeypadScan(T_VOID)
{
    T_KEY_ID keyIdx; //对平台来说,驱动库返回的并不是真正的KeyId,
                     //只是相应KeyId的索引值,这里需要转换成真正的KeyId

    keyIdx = keypad_scan();
    if (INVALID_KEY_ID == keyIdx)
    {
        return kbNULL;
    }
    else
    {
        return keyIdx;
    }
}

#if (SYS_KEY_MODE != 4)
#pragma arm section rwdata = "_cachedata_"
static T_TIMER key_timer_id = ERROR_TIMER;
#pragma arm section rwdata

static T_U8 last_key = 0xff;
static T_U8 last_type = 0xff;
static T_BOOL long_press = AK_FALSE;

#pragma arm section code

T_VOID Post_PreKeyEvent(T_TIMER timer_id, T_U32 delay)
{
    T_EVT_PARAM         pEventParm;
    pEventParm.c.Param1 = last_key;
    pEventParm.c.Param2 = last_type;
    
    Fwl_TimerStop(key_timer_id);
    key_timer_id = ERROR_TIMER;
    VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, long_press >> 8);
}

T_VOID Post_KeyEvent(T_TIMER timer_id, T_U32 delay)
{
    T_EVT_PARAM         pEventParm;
    pEventParm.c.Param1 = preKey.keyID;
    pEventParm.c.Param2 = preKey.press_type;
    
    Fwl_TimerStop(key_timer_id);
    key_timer_id = ERROR_TIMER;
    VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, (preKey.pressType) >> 8);
}
#endif
extern T_U32 ExitStandbyTime;

/*******************************************************************************
 * @brief   deal keypad message
 * @author  songmengxing
 * @date    2012-11-28
 * @param   [in]keyIdx: key index
 * @param   [in]pressType: high 8bit indicate long or short
 *                         low 8bit indicate status: up/press/down
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadMsgDeal(T_U8 keyIdx, T_U16 pressType)
{
    T_EVT_PARAM         pEventParm;
    T_BOOL              longPress = pressType >> 8;
    T_eKEYPAD_STATUS    status = (T_U8)pressType;

	if(T_U32_MAX != ExitStandbyTime)
	{
		if(ExitStandbyTime + 1500 > Fwl_GetTickCountMs())
		{
			if (eKEYUP == status)
			{
				ExitStandbyTime = T_U32_MAX;
			}
			return ;
		}
		else
		{
			ExitStandbyTime = T_U32_MAX;
		}
	}
    pEventParm.c.Param1 = keyIdx;
    if (longPress)  //long
    {
        if ((eKEYPRESS == status) //转换驱动库第一次eKEYPRESS消息为PRESS_LONG
            && ((keyIdx != preKey.keyID) 
                || (preKey.pressType != pressType)))
        {
            pEventParm.c.Param2 = PRESS_LONG;
        }
        else if (eKEYUP == status)
        {
            pEventParm.c.Param2 = PRESS_UP;
        }
        else 
        {
            pEventParm.c.Param2 = PRESSING;
        }
    }
    else    //short
    {
        if(eKEYDOWN == status)
        {
            if(kbPOWER == pEventParm.c.Param1 )
            {
                pEventParm.c.Param2 = 0xff;       //插USB线开机后，拨向ON
            }
		}
        else
        {
            pEventParm.c.Param2 = PRESS_SHORT;    //拨向OFF   
        }
    }

    preKey.keyID = keyIdx;
    preKey.pressType = pressType;

    preKey.press_type = pEventParm.c.Param2;

    #if (SYS_KEY_MODE != 4)
    if(PRESS_SHORT == pEventParm.c.Param2)
    {   
        if(ERROR_TIMER != key_timer_id)
        {
            if(last_key == pEventParm.c.Param1)
            {
                Fwl_TimerStop(key_timer_id);
                key_timer_id = ERROR_TIMER;
                
                pEventParm.c.Param2 = PRESS_DOUBLE;
                AK_DEBUG_OUTPUT("press double!\n");
                
                #if ((SYS_KEY_MODE == 0)||(SYS_KEY_MODE == 1))    //三键(四键模式二)模式:kbOK双击=kbBT
                if(kbOK == pEventParm.c.Param1)
                {
                    pEventParm.c.Param1=kbBT;
                    pEventParm.c.Param2 = PRESS_SHORT;
                }
                #endif
                
                VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, longPress);
            }
            else
            {
                //AK_DEBUG_OUTPUT("press two different key quickly and shortly!\n");
                Post_PreKeyEvent(0, 0);    //执行上一次按键消息
                key_timer_id = Fwl_TimerStart(500,AK_FALSE,Post_KeyEvent);
            }
        }
        else
        {
            key_timer_id = Fwl_TimerStart(500,AK_FALSE,Post_KeyEvent);
        }
    }
    else
    {
        if(ERROR_TIMER != key_timer_id)
        {
            //AK_DEBUG_OUTPUT("press two different key quickly ,fisrt shotly then longly!\n");
            Post_PreKeyEvent(0, 0);   //执行上一次短按消息
        }
        
        #if ((SYS_KEY_MODE == 0)||(SYS_KEY_MODE == 2))    //三键(四键模式一)模式:kbOK长按=kbFUNC短按
        if ((kbOK == pEventParm.c.Param1)&&(PRESS_LONG == pEventParm.c.Param2))
        {
            pEventParm.c.Param1= kbFUNC;
            pEventParm.c.Param2 = PRESS_SHORT;
        }
        #endif
        
        VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, longPress);
    }

    //保存上一次按键信息
    last_key = pEventParm.c.Param1;
    last_type = pEventParm.c.Param2;
    long_press = longPress;
    #else
    /*if (kbFUNC== pEventParm.c.Param1
        && (PRESS_LONG == pEventParm.c.Param2
            || PRESSING == pEventParm.c.Param2))
    {
        pEventParm.c.Param1 = kbPOWER;
    }*/
    
    AK_DEBUG_OUTPUT("K_ID:%d, K_Type:%d\n",pEventParm.c.Param1,pEventParm.c.Param2);
    VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, longPress);
    #endif
}


//the end of files
