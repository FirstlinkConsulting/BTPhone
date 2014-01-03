/**
 * @file    arch_analog.h
 * @brief   the interface for the control of mic/hp/linein and so on
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
 */
 
#ifndef _ARCH_ANALOG_H_
#define _ARCH_ANALOG_H_


#include "anyka_types.h"


typedef enum 
{
    SRC_DAC = 0,        ///< Ñ¡ÔñDACÊäÈëÒôÆµ
    SRC_LINEIN,         ///< Ñ¡ÔñLINEINÊäÈëÒôÆµ
    SRC_MIC,            ///< Ñ¡ÔñMICÊäÈëÒôÆµ
    SRC_NUL
}ANALOG_SIGNAL_SRC;


typedef enum 
{
    DST_ADC = 0,        ///< Ñ¡ÔñADCÊä³öÒôÆµ
    DST_HP,             ///< Ñ¡ÔñHPÊä³öÒôÆµ
    DST_LINEOUT,        ///< Ñ¡ÔñLINEOUTÊä³öÒôÆµ
    DST_NUL
}ANALOG_SIGNAL_DST;


typedef enum 
{
    SIGNAL_CONNECT = 0,
    SIGNAL_DISCONNECT
}ANALOG_SIGNAL_STATE;



/**
 * @brief       connect or disconnect the signal between source and destination. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   analog_src
 *                  the source type of the signal.
 * @param[in]   analog_dst
 *                  the destinationtype of the signal.
 * @param[in]   state
 *                  connect or disconnect
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setsignal(ANALOG_SIGNAL_SRC analog_src, 
                        ANALOG_SIGNAL_DST analog_dst, 
                        ANALOG_SIGNAL_STATE state);


/**
 * @brief       connect or disconnect the signal between source and destination. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   analog_src
 *                  the source type of the signal.
 * @param[in]   analog_dst
 *                  the destinationtype of the signal.
 * @param[in]   state
 *                  connect or disconnect
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setconnect(ANALOG_SIGNAL_SRC analog_src, 
                         ANALOG_SIGNAL_DST analog_dst, 
                         ANALOG_SIGNAL_STATE state);


/**
 * @brief       get the state of  the signal between source and destination. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   analog_src
 *                  the source type of the signal.
 * @param[in]   analog_dst
 *                  the destinationtype of the signal.
 * @param[out]  state
 *                  connect or disconnect
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_getconnect(ANALOG_SIGNAL_SRC analog_src, 
                         ANALOG_SIGNAL_DST analog_dst, 
                         ANALOG_SIGNAL_STATE *state);


/**
 * @brief       set the gain of the headphone. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   gain
 *                  the gain of the headphone. the range of the gain is 0~5
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setgain_hp(T_U8 gain);


/**
 * @brief       set the gain of the linein. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   gain
 *                  the gain of the linein. the range of the gain is 0~15
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setgain_linein(T_U8 gain);


/**
 * @brief   set the gain of the mic. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]gain
 *              the gain of the mic. the range of the gain is 0~10
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setgain_mic(T_U8 gain);


/**
 * @brief       get the voltage of the battery. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_U32
 * @retval      the voltage of the battery, unit: mv
 */ 
T_U32 analog_getvoltage_bat(T_VOID);


/**
 * @brief       get the voltage of the analog keypad. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_U32
 * @retval      the voltage of the analog keypad, unit: mv
 */ 
T_U32 analog_getvoltage_keypad(T_VOID);


/**
 * @brief       get the voltage of detection. 
 * @author      wangguotian
 * @date        2012.11.19
 * @param[in]   T_VOID
 * @return      T_U32
 * @retval      the voltage of the detection
 */
T_U32 analog_getvoltage_detection(T_VOID);


#endif //_ARCH_ANALOG_H_

