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
#include "Gpio_define.h"
#include "Fwl_Keypad.h"
#include "hal_keypad.h"
#include "hal_int_msg.h"
#include "m_event.h"
#include "w_keypad.h"

T_VOID Fwl_KeypadInit(T_VOID)
{
    w_keypad_init();
}

/*******************************************************************************
 * @brief   enable keypad scan
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]bEnable: enable or disable
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadEnable(T_BOOL bEnable)
{    
 
}


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
    return w_keypad_scan();
}



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

    pEventParm.c.Param1 = keyIdx;
    
    if (longPress)  //long
    {
        if (eKEYPRESS == status)
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
        pEventParm.c.Param2 = PRESS_SHORT;
    }
        
    VME_EvtQueuePutUnique(M_EVT_USER_KEY, &pEventParm, longPress);
}



//the end of files
