/*******************************************************************************
 * @file    keypad.c
 * @brief   keypad function file, provide keypad APIs: init, scan...
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-01-25
 * @version 1.0
 * @ref     1080L programer guide
*******************************************************************************/
#include "anyka_types.h"
#include "hal_keypad.h"
#include "arch_timer.h"
#include "arch_gpio.h"
#include "hal_int_msg.h"
#include "hal_errorstr.h"
#include "drv_cfg.h"



#if (DRV_SUPPORT_KEYPAD > 0)


typedef struct
{
    T_BOOL              m_init;
    T_KEY_ID            m_prev_key_id;
    T_U32               m_press_time;
    T_TIMER             m_timer_id;
} T_KEYPAD_CONTROL;


typedef struct
{
    T_KEYPAD_PARAM*          m_param;
    T_KEYPAD_CONTROL         m_control;
    T_fKEYPAD_CALLBACK       m_callback;
} T_HARDWARE_KEYPAD;


#pragma arm section zidata = "_drvbootbss_"
static T_HARDWARE_KEYPAD s_keypad;
volatile T_U8 PowerKeyStatus = 0;
#pragma arm section zidata


#pragma arm section rodata = "_drvbootconst_"
static const T_CHR err_str[] = ERROR_KEYPAD;
#pragma arm section rodata


static T_VOID keypad_timer_start(T_U16 ms, T_BOOL loop, T_fTIMER_CALLBACK cb);
static T_VOID keypad_timer_stop(T_VOID);
static T_VOID keypad_send_key(T_KEY_ID key_id, T_BOOL long_press, T_eKEYPAD_STATUS status);
static T_VOID keypad_scan_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_steady_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_state_timer(T_TIMER timer_id, T_U32 delay);

/*******************************************************************************
 * @brief   initialize keypad
 * If callback function is not equal AK_NULL, the callback function...
 * ...will be called by keypad interrupt
 * If callback function is equal AK_NULL, the key id will be sent by message
 * Function keypad_init() must be called before call any other keypad functions
 * @author  zhanggaoxin
 * @date    2012-12-28
 * @param   [in]callback: keypad callback function
 * @param   [in]param : keypad param
 * @return  T_BOOL
 * @retval  AK_TRUE:init successfully; AK_FALSE:init failed.
*******************************************************************************/
T_BOOL keypad_init(T_fKEYPAD_CALLBACK callback, T_KEYPAD_PARAM *param)
{
    //memset(&s_keypad, 0, sizeof(s_keypad));

    if ((AK_NULL == param) || (AK_NULL == param->m_adkey_val_array))
    {
        drv_print("keypad init error!", 0, AK_TRUE);
        s_keypad.m_control.m_init = AK_FALSE;
        return AK_FALSE;
    }
#if (CHIP_SEL_10C > 0)
    IrDA_init();
#endif

    s_keypad.m_callback = callback;
    s_keypad.m_param = param;
    s_keypad.m_control.m_timer_id = ERROR_TIMER;
    s_keypad.m_control.m_prev_key_id = INVALID_KEY_ID;
    s_keypad.m_control.m_press_time = 0;
    s_keypad.m_control.m_init = AK_TRUE;

    keypad_enable_scan();
#if (CHIP_SEL_10C > 0)
    IrDA_ScanSwitch(AK_TRUE);
#endif

    return AK_TRUE;
}

#pragma arm section code = "_drvbootcode_"
#if (CHIP_SEL_10C > 0)&&(SWITCH_MODE_SLIDE > 0)
/*******************************************************************************
 * @brief   get power key status
 * Function keypad_init() must be called before call this function
 * @author  ai_jun
 * @date    2013-8-2
 * @param   [in]new_status:set new power status
 * @return  T_U8
 * @retval  the power status
*******************************************************************************/
T_U8 keypad_get_power_status(T_U8 new_status)
{
    if(T_U8_MAX != new_status)
    {
        PowerKeyStatus = new_status;
    }
    return PowerKeyStatus;
}
#endif

/*******************************************************************************
 * @brief   enable keypad scan
 * Function keypad_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID keypad_enable_scan(T_VOID)
{
    if (AK_FALSE == s_keypad.m_control.m_init)
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return;
    }

    keypad_timer_stop();
#if (CHIP_SEL_10C > 0)
    IrDA_ScanSwitch(AK_TRUE);
#endif
    gpio_int_polarity(GPIO_AIN0_INT, LEVEL_HIGH);
    gpio_int_polarity(POWERKEY_GPIO, LEVEL_HIGH);

    gpio_int_enable(GPIO_AIN0_INT, AK_TRUE);
    gpio_int_enable(POWERKEY_GPIO, AK_TRUE);
}

/*******************************************************************************
 * @brief   disable keypad scan
 * Function keypad_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID keypad_disable_scan(T_VOID)
{
    if (AK_FALSE == s_keypad.m_control.m_init)
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return;
    }
#if (CHIP_SEL_10C > 0)
    IrDA_ScanSwitch(AK_FALSE);
#endif
    keypad_timer_stop();
    gpio_int_enable(GPIO_AIN0_INT, AK_FALSE);
    gpio_int_enable(POWERKEY_GPIO, AK_FALSE);
}


static T_VOID keypad_timer_start(T_U16 ms, T_BOOL loop, T_fTIMER_CALLBACK cb)
{
    if(ERROR_TIMER != s_keypad.m_control.m_timer_id)
    {
        timer_stop(s_keypad.m_control.m_timer_id);
        s_keypad.m_control.m_timer_id = ERROR_TIMER;
    }

    s_keypad.m_control.m_timer_id = timer_start(ms, loop, cb);

    if(ERROR_TIMER == s_keypad.m_control.m_timer_id)
    {
        drv_print(err_str, __LINE__, AK_TRUE);
    }
}


static T_VOID keypad_timer_stop(T_VOID)
{
    if(ERROR_TIMER != s_keypad.m_control.m_timer_id)
    {
        timer_stop(s_keypad.m_control.m_timer_id);
        s_keypad.m_control.m_timer_id = ERROR_TIMER;
    }
}

/*******************************************************************************
 * @brief   to send key ID by callback func or message
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [in]key_id, key number
 * @param   [in]long_press, true is long key
 * @param   [in]status, key press is status
 * @return  T_VOID
*******************************************************************************/
static T_VOID keypad_send_key(T_KEY_ID key_id, T_BOOL long_press, T_eKEYPAD_STATUS status)
{
    if (AK_NULL == s_keypad.m_callback)
    {
        post_int_message(IM_KEYINFO, key_id, (T_U16)((long_press<<8) | (T_U8)(status)));
    }
    else
    {
        T_KEYPAD key;

        key.m_key_id     = key_id;
        key.m_long_press = long_press;
        key.m_status     = status;
        s_keypad.m_callback(&key);
    }
}

/*******************************************************************************
 * @brief   to get the current key ID
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [out]key_id, will return the key ID
 * @return  T_BOOL true is OK.
 * @retval  AK_TRUE is OK, AK_FALSE is failed
*******************************************************************************/
static T_BOOL keypad_get_cur_key_id(T_KEY_ID *key_id)
{
    T_KEYPAD_PARAM *keypad_param = s_keypad.m_param;
    T_U32 index;
    T_U16 ad_val = 0;

    if (AK_NULL == key_id)
    {
        return AK_FALSE;
    }
#if (CHIP_SEL_10C > 0)
    if(IrDA_GetKey(key_id))
        return AK_TRUE;
#endif

#if !(CHIP_SEL_10C > 0)||!(SWITCH_MODE_SLIDE > 0)
    if (LEVEL_HIGH == gpio_get_pin_level(POWERKEY_GPIO))
    {
        *key_id = keypad_param->m_powerkey_value;
        return AK_TRUE;
    }
#endif

    ad_val = (T_U16)analog_getvoltage_keypad();
    //drv_print(AK_NULL, ad_val, 1);
    if ((ad_val < keypad_param->m_ad_val_min) || (ad_val > keypad_param->m_ad_val_max))
    {
        return AK_FALSE;
    }
    for (index=0; index<keypad_param->m_adkey_max_num; ++index)
    {
        if ((ad_val >= keypad_param->m_adkey_val_array[index].m_min) \
            && (ad_val <= keypad_param->m_adkey_val_array[index].m_max)) 
        {
        #if (CHIP_SEL_10C > 0)
            *key_id = keypad_param->m_adkey_val_array[index].key_id;
        #else     
            *key_id = index;
        #endif
            return AK_TRUE;
        }
    }

    return AK_FALSE;
}

/*******************************************************************************
 * @brief   keypad callback function
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [in]pin: key gpio pin ID.
 * @param   [in]polarity: 1 means active high interrupt. 0 means active low interrupt.
 * @return  T_VOID
*******************************************************************************/
T_VOID keypad_scan_start_chk(T_U32 pin, T_U8 polarity)
{
    if (AK_FALSE == s_keypad.m_control.m_init)
    {
        drv_print(err_str, __LINE__, AK_TRUE);
        return;
    }

#if !(CHIP_SEL_10C > 0)||!(SWITCH_MODE_SLIDE > 0)
    if (((GPIO_AIN0_INT == pin) && (LEVEL_HIGH == polarity)) 
        || ((POWERKEY_GPIO == pin) && (LEVEL_HIGH == polarity)))
#else
    if (POWERKEY_GPIO == pin)
    {
        if (LEVEL_LOW == polarity)
        {
            PowerKeyStatus = 1;
        }
        else
        {
            PowerKeyStatus = 0;
        }
    }
    else if ((GPIO_AIN0_INT == pin) && (LEVEL_HIGH == polarity))    
#endif
    {
        keypad_disable_scan();

        s_keypad.m_control.m_prev_key_id = INVALID_KEY_ID;
        s_keypad.m_control.m_press_time  = KEY_SCAN_TIME;
        s_keypad.m_control.m_timer_id    = ERROR_TIMER;

        keypad_timer_start(KEY_SCAN_TIME, AK_FALSE, keypad_scan_timer);
    }
#if (CHIP_SEL_10C > 0)
    else if(64 == pin)
    {
        keypad_disable_scan();
        IrDA_GetKey(&(s_keypad.m_control.m_prev_key_id));
        s_keypad.m_control.m_press_time  = KEY_SCAN_TIME;
        keypad_timer_start(KEY_LONG_TIME, AK_TRUE, keypad_state_timer);
    }
#endif
}

/*******************************************************************************
 * @brief   to check key state timer, keypad interrupt will enable this timer
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [in]timer_id,
 * @param   [in]delay
 * @return  T_VOID
*******************************************************************************/
static T_VOID keypad_scan_timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &s_keypad.m_control;
    T_KEY_ID cur_id = INVALID_KEY_ID;
    T_BOOL ret = AK_FALSE;

    ret = keypad_get_cur_key_id(&cur_id);

    //key id is change, the key is down
    if ((cur_id != INVALID_KEY_ID) && ret)
    {
        control->m_prev_key_id = cur_id;

        keypad_timer_start(KEY_LOOP_TIME, AK_FALSE, keypad_steady_timer);
    }
    else
    {
        keypad_enable_scan();
    }
}

/*******************************************************************************
 * @brief   this function can prevent trembling signal
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [in]timer_id,
 * @param   [in]delay
 * @return  T_VOID
*******************************************************************************/
static T_VOID keypad_steady_timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &s_keypad.m_control;
    T_KEY_ID cur_id = INVALID_KEY_ID;
    T_BOOL ret = AK_FALSE;

    ret = keypad_get_cur_key_id(&cur_id);

    if (control->m_prev_key_id != cur_id)
    {
        if (AK_FALSE == ret)
        {
            keypad_enable_scan();
        }
        else
        {
            control->m_prev_key_id = cur_id;
            
            keypad_timer_start(KEY_LOOP_TIME, AK_FALSE, keypad_steady_timer);
        }
    }
    else
    {
        //keypad_send_key(control->m_prev_key_id, 0, eKEYDOWN);//send key down
        control->m_press_time += delay;

        keypad_timer_start(KEY_LONG_TIME, AK_TRUE, keypad_state_timer);
    }
}

/*******************************************************************************
 * @brief   to check long key is up delay timer 
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   [in]timer_id,
 * @param   [in]delay
 * @return  T_VOID
*******************************************************************************/
static T_VOID keypad_state_timer(T_TIMER timer_id, T_U32 delay)
{
    T_KEYPAD_CONTROL *control = &s_keypad.m_control;
    T_KEY_ID cur_id = INVALID_KEY_ID;
    T_BOOL ret = AK_FALSE;

    ret = keypad_get_cur_key_id(&cur_id);

    if (control->m_prev_key_id != cur_id) //key is up or new key is down
    {
        if (control->m_press_time >= LONG_PRESS_TIME)
        {
            keypad_send_key(control->m_prev_key_id, 1, eKEYUP);//send long key UP
        }
        else
        {
            keypad_send_key(control->m_prev_key_id, 0, eKEYUP);//send short key UP
        }

        if (AK_FALSE == ret)    //key is up
        {
            keypad_enable_scan();
        }
        else    //new key is down
        {
            control->m_prev_key_id = cur_id;
            control->m_press_time = delay;

            keypad_timer_start(KEY_LOOP_TIME, AK_FALSE, keypad_steady_timer);
        }
    }
    else
    {
        control->m_press_time += delay;
        if (control->m_press_time >= LONG_PRESS_TIME)
        {
            keypad_send_key(control->m_prev_key_id, 1, eKEYPRESS);//send long key PRESS
        }
    }
}
#pragma arm section code

#pragma arm section code = "_drvfrequent_"
/*******************************************************************************
 * @brief   sacn keypad to get key id  
 * @author  zhanggaoxin
 * @date    2013-03-10
 * @param   T_VOID
 * @return  T_KEY_ID
 * @retval  the key id
*******************************************************************************/
T_KEY_ID keypad_scan(T_VOID)
{
    T_KEY_ID pre_id = INVALID_KEY_ID;
    T_KEY_ID cur_id = INVALID_KEY_ID;

    while (keypad_get_cur_key_id(&cur_id))
    {
        pre_id = cur_id;
        //#if (CHIP_SEL_10C > 0)
        //delay_ms(120);
        //#else
        delay_ms(10);
        //#endif
        if (keypad_get_cur_key_id(&cur_id))
        {
            if (pre_id == cur_id)
            {
                return cur_id;
            }
        }
        else
        {
            break;
        }
    }
    return INVALID_KEY_ID;
}

#pragma arm section code

#endif

