#ifndef _LEDHINT_CFG_H
#define _LEDHINT_CFG_H

#include "Eng_LedHint.h"

#ifdef  SUPPORT_LEDHINT

const T_LED_CTRL LedMode_Default = {0, 0, 0, 0, 0};

const T_LED_CTRL LedMode_Normal = {50, 0, 4, 2,
                            FInsrt(LED_SHOW_BLUE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_BT_Reconnect = {20, 0, 0, 2,
                            FInsrt(LED_SHOW_BLUE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_BT_Connected = {50, 0, 10, 2,
                            FInsrt(LED_SHOW_BLUE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_BT_Called = {25, 0, 8, 4,
                            FInsrt(LED_SHOW_BLUE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))
                            | FInsrt(LED_SHOW_BLUE, LED_STATE_SEQ(2))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(3))};


const T_LED_CTRL LedMode_Normal_Charge = {50, 2, 4, 2,
                            FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_BT_Reconnect_Charge = {20, 2, 0, 2,
                            FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

#pragma arm section rodata = "_SYS_BLUE_A2DP_CODE_"

const T_LED_CTRL LedMode_BT_Connected_Charge = {50, 2, 10, 2,
                            FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_BT_Called_Charge = {25, 2, 8, 4,
                            FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))
                            | FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(2))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(3))};
#pragma arm section rodata 

const T_LED_CTRL LedMode_LowBat = {25, 1, 8, 4,
                            FInsrt(LED_SHOW_RED, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(1))
                            | FInsrt(LED_SHOW_RED, LED_STATE_SEQ(2))
                            | FInsrt(LED_SHOW_OFF, LED_STATE_SEQ(3))};

const T_LED_CTRL LedMode_FullBat = {100, 4, 0, 2,
                            FInsrt(LED_SHOW_RED, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_RED, LED_STATE_SEQ(1))};
//{0, 5, 0, 0, 0};
const T_LED_CTRL LedMode_USB = {100, 3, 0, 2,
                            FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_DOUBLE, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_Start = {100, 0, 0, 2,
                            FInsrt(LED_SHOW_RED, LED_STATE_SEQ(0))
                            | FInsrt(LED_SHOW_RED, LED_STATE_SEQ(1))};

const T_LED_CTRL LedMode_OFF = {0, 5, 0, 0, 0};

#endif
#endif
