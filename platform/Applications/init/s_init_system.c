
#include "Apl_Public.h"
#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Fwl_System.h"
#include "LedHint_cfg.h"
#include "Eng_LedHint.h"
#include "M_event.h"
#include "M_event_api.h"

#if(NO_DISPLAY == 1)        //无屏版本
#if((STORAGE_USED == NAND_FLASH) || (UPDATA_USED == 1))
#ifdef SUPPORT_SDCARD       //只有在支持SD卡的情况下，才允许自升级
#define UPDATE_KEY  kbOK
#endif
#endif
#endif

static T_BOOL LedHint_Initialize(T_VOID)
{
#ifdef SUPPORT_LEDHINT
    if (LedHint_Init(LED_BRIGHT_NUMS, 2))
    {
        LedHint_Add(LED_DEFAULT, &LedMode_Default);
        LedHint_Add(LED_NORMAL, &LedMode_Normal);
        LedHint_Add(LED_BT_RECONNECT, &LedMode_BT_Reconnect);
        LedHint_Add(LED_BT_CONNECTED, &LedMode_BT_Connected);
        LedHint_Add(LED_BT_CALLED, &LedMode_BT_Called);
        LedHint_Add(LED_NORMAL_CHARGE, &LedMode_Normal_Charge);
        LedHint_Add(LED_BT_RECONNECT_CHARGE, &LedMode_BT_Reconnect_Charge);
        LedHint_Add(LED_BT_CONNECTED_CHARGE, &LedMode_BT_Connected_Charge);
        LedHint_Add(LED_BT_CALLED_CHARGE, &LedMode_BT_Called_Charge);
        LedHint_Add(LED_LOWBAT, &LedMode_LowBat);
        LedHint_Add(LED_FULLBAT, &LedMode_FullBat);
        LedHint_Add(LED_USB, &LedMode_USB);
        LedHint_Add(LED_START,&LedMode_Start);
        LedHint_Add(LED_OFF, &LedMode_OFF);
    }
    else
    {
        AK_DEBUG_OUTPUT("LedHint_Init Fail!!!\n");
        return AK_FALSE;
    }  
    LedHint_Exec(LED_NORMAL);//系统启动后，至少由一个状态在里面
#endif

   return AK_TRUE;
}

void initinit_system(void)
{
    LedHint_Initialize();
}

void exitinit_system(void)
{
    PublicTimerStart();
}

void paintinit_system(void)
{
}

unsigned char handleinit_system(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    #if(NO_DISPLAY == 1)        //无屏版本
    #if((STORAGE_USED == NAND_FLASH) || (UPDATA_USED == 1))
    #ifdef SUPPORT_SDCARD       //只有在支持SD卡的情况下，才允许自升级
    T_BOOL isUpdata = AK_FALSE;
   
    if(M_EVT_EXIT == event)
    {
        //back from update module, no updatepackage or updatepackage not match
        isUpdata = AK_FALSE;    
    }
    /*else if(UPDATE_KEY == Fwl_KeypadScan())
    {
        Fwl_DelayUs(20000); //wait and reconfirm
        if(UPDATE_KEY == Fwl_KeypadScan())
        {
            isUpdata = AK_TRUE;

            //go to update module
            m_triggerEvent(M_EVT_UPDATA, pEventParm);
        }
    }*/

    if (AK_FALSE == isUpdata)
    #endif //SUPPORT_SDCARD
    #endif
    #endif //NO_DISPLAY

    {
        //power on system
        m_triggerEvent(M_EVT_1, pEventParm);
    }
        
    return 0;
}


/* end of files */
