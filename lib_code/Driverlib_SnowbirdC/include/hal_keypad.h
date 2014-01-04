/*******************************************************************************
 * @file    hal_keypad.h
 * @brief   keypad module, for keypad operations
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012.11.24
 * @version 1.0
*******************************************************************************/
#ifndef __HAL_KEYPAD_H__
#define __HAL_KEYPAD_H__


#include "anyka_types.h"
#include "drv_cfg.h"


#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus



#define INVALID_KEY_ID              (0xff)  ///< invalid key ID

//长按键按下的时间长
#define LONG_PRESS_TIME             (1000)  ///< 1000ms

//主动检查键值TIMER的时间间隔
#define KEY_SCAN_TIME               (20)    ///< 20  ms
#define KEY_LOOP_TIME               (10)    ///< 10  ms
#if (CHIP_SEL_10C > 0)
#define KEY_LONG_TIME               (110)   ///< 200 ms
#else
#define KEY_LONG_TIME               (200)   ///< 200 ms
#endif



typedef T_U8            T_KEY_ID;

/**
 * @brief keypad param define
 */
typedef struct
{
    T_U16               m_min;              ///< AD按键变化的最小值
    T_U16               m_max;              ///< AD按键变化的最大值
#if (CHIP_SEL_10C > 0) 
    T_U8                key_id;
#endif
} T_KEY_DET_STR;

typedef struct
{
    T_U16               m_ad_val_min;       ///< 有效按键的最小值
    T_U16               m_ad_val_max;       ///< 有效按键的最大值
    T_KEY_DET_STR*      m_adkey_val_array;  ///< 指向AD按键的映身表
    T_U8                m_adkey_max_num;    ///< AD按键的数量
    T_U8                m_powerkey_value;   ///< 电源键的键值
} T_KEYPAD_PARAM;

/**
 * @brief key status define
 */
typedef enum
{
    eKEYDOWN,                         ///< key press down
    eKEYPRESS,                        ///< key press down and hold 
    eKEYUP,                           ///< key up
    eKEYSTATUS_NUM                    ///< num of key status
} T_eKEYPAD_STATUS;

/**
 * @brief key value struct
 */
typedef struct
{
    T_KEY_ID          m_key_id;       ///< key ID 
    T_BOOL            m_long_press;   ///< AK_TRUE:long press, AK_FALSE:short press
    T_eKEYPAD_STATUS  m_status;       ///< key status: down/hold/up
} T_KEYPAD;



typedef T_VOID (*T_fKEYPAD_CALLBACK)(const T_KEYPAD *keypad); 



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
T_BOOL keypad_init(T_fKEYPAD_CALLBACK callback, T_KEYPAD_PARAM *param);


/*******************************************************************************
 * @brief   sacn keypad to get key id  
 * Function keypad_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2013-03-10
 * @param   T_VOID
 * @return  T_KEY_ID
 * @retval  the key id
*******************************************************************************/
T_KEY_ID keypad_scan(T_VOID);


/*******************************************************************************
 * @brief   enable keypad scan
 * Function keypad_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID keypad_enable_scan(T_VOID);


/*******************************************************************************
 * @brief   disable keypad scan
 * Function keypad_init() must be called before call this function
 * @author  zhanggaoxin
 * @date    2012-11-26
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID keypad_disable_scan(T_VOID);


/*******************************************************************************
 * @brief   get power key status
 * Function keypad_init() must be called before call this function
 * @author  ai_jun
 * @date    2013-8-2
 * @param   [in]new_status:set new power status
 * @return  T_U8
 * @retval  the power status
*******************************************************************************/
T_U8 keypad_get_power_status(T_U8 new_status);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus


#endif // #ifndef __HAL_KEYPAD_H__

