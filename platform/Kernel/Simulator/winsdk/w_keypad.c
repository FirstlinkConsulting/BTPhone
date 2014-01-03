/**
 * @file keypad.c
 * @brief Define keypad driver.
 * This file provides keypad driver APIs: initialization, enable, disable, keypad
 * interrupt handler, and keypad scaning function.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-21
 * @version 1.0
 * @ref
 */


//#include "sys_ctl.h"
#include "Gbl_Global.h"
#include "w_vtimer.h"
#include "w_evtmsg.h"
#include "w_keypad.h"
#include "Fwl_Keypad.h"
#include "hal_keypad.h"

static T_U32 PressDelay[MAX_KEY_NUM];

#define MAX_PHY_KEY_NUM         8

#if (USE_COLOR_LCD)
static T_WIN_KEY s_keypad_rect[MAX_PHY_KEY_NUM] = {
#if (LCD_TYPE==0)
#if (LCD_HORIZONTAL == 1)
{  225, 124,  17,  24, kbLEFT}, //Left Arrow
{  242, 124,  17,  24, kbRIGHT},    //Right Arrow
{  16, 140,  27,  27, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  21, 159,  13,  13, kbPOWER},    //power,on\off
{  13, 40,  33,  15, kbRECORD},    //record
{  13, 94,  33,  15, kbVOLUME}, //volume
{  225, 62,  34,  21, kbOK},  //ok
};
#else
{  193, 124,  17,  24, kbLEFT}, //Left Arrow
{  210, 124,  17,  24, kbRIGHT},    //Right Arrow
{  16, 140,  27,  27, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  21, 159,  13,  13, kbPOWER},    //power,on\off
{  13, 40,  33,  15, kbRECORD},    //record
{  13, 94,  33,  15, kbVOLUME}, //volume
{  193, 62,  34,  21, kbOK},  //ok
};
#endif
#endif
#if (LCD_TYPE==1)
{  241, 138,  17,  24, kbLEFT}, //Left Arrow
{  258, 138,  17,  24, kbRIGHT},    //Right Arrow
{  15, 182,  27,  27, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  19, 216,  13,  13, kbPOWER},    //power,on\off
{  13, 50,  33,  15, kbRECORD},    //record
{  13, 117,  33,  15, kbVOLUME}, //volume
{  241, 71,  34,  21, kbOK},  //ok
};
#endif
#if (LCD_TYPE==2)
{  193, 97,  17,  24, kbLEFT}, //Left Arrow
{  210, 97,  17,  24, kbRIGHT},    //Right Arrow
{  18, 103,  27,  27, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  19, 132,  11,  11, kbPOWER},    //power,on\off
{  16, 30,  33,  15, kbRECORD},    //record
{  16, 67,  33,  15, kbVOLUME}, //volume
{  194, 51,  34,  21, kbOK},  //ok
};
#endif
#if (LCD_TYPE==3)
#if (LCD_HORIZONTAL == 1)
{  483, 192,  34,  28, kbLEFT}, //Left Arrow
{  522, 192,  34,  28, kbRIGHT},    //Right Arrow
{  42, 215,  57,  34, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  21, 159,  13,  13, kbPOWER},    //power,on\off
{  40, 62,  76,  24, kbRECORD},    //record
{  38, 142,  75, 22, kbVOLUME}, //volume
{  488, 100,  72, 28, kbOK},  //ok
};
#else
{  361, 251,  30,  50, kbLEFT}, //Left Arrow
{  391, 251,  30,  50, kbRIGHT},    //Right Arrow
{  29, 280,  50,  50, kbMODE},   //play\stop
{  0,   0,   1,   1, kbNULL},   //NULL
{  21, 159,  13,  13, kbPOWER},    //power,on\off
{  25, 79,  66,  30, kbRECORD},    //record
{  26, 190,  66,  30, kbVOLUME}, //volume
{  360, 125,  66,  40, kbOK},  //ok
};
#endif
#endif
#else
static T_WIN_KEY s_keypad_rect[MAX_PHY_KEY_NUM] = {
{   27,  52,  43,  23, kbLEFT},   //Left Arrow
{  125,  52,  43,  23, kbRIGHT},  //Right Arrow
{   27,  22,  43,  23, kbFUNC},   //func(kbFUNC)
{   76,  22,  43,  23, kbMODE},   //mode(kbMODE)
{  175,  22,  43,  23, kbVOLADD}, //vol+(kbPOWER)
{  125,  22,  43,  23, kbRECORD}, //rec
{  175,  52,  43,  23, kbVOLSUB}, //vol-(kbVOLUME)
{   76,  52,  43,  23, kbOK},     //ok
};
#endif


static T_U16        s_cur_mouse_x = 0;
static T_U16        s_cur_mouse_y = 0;
static T_eKEY_ID    s_prev_key_id = kbNULL;         /* previous key ID */
static T_U32        s_cur_key_delay = 0;            /* current key delay time */
static T_U8         s_sent_key_num = 0;             /* key number which be has been sent */
static T_TIMER      s_key_timer = ERROR_TIMER;
static T_TIMER      s_intervaltimer_id = ERROR_TIMER;
static T_VOID keypad_timer(T_TIMER timer_id, T_U32 delay);
static T_VOID keypad_intervaltimer(T_TIMER timer_id, T_U32 delay);

/**
 * @brief Initialize keypad
 * If pointer callback_func is not equal AK_NULL, the keypad interrupt will be enabled
 * Function keypad_init() must be called before call any other keypad functions
 * @author ZouMai
 * @date 2004-09-21
 * @param T_fKEYPAD_CALLBACK callback_func: Keypad callback function
 * @return T_VOID
 * @retval
 */
T_VOID w_keypad_init()
{
    int iKeypadIndex;
    T_U8    i;
    for(i = 0 ; i < MAX_KEY_NUM ; i++)
    {
        PressDelay[i] = 0;
    }

    // correct RECT Structure
    for (iKeypadIndex = 0; iKeypadIndex < MAX_PHY_KEY_NUM; iKeypadIndex++)
    {
        s_keypad_rect[iKeypadIndex].right   +=   s_keypad_rect[iKeypadIndex].left;
        s_keypad_rect[iKeypadIndex].bottom  +=   s_keypad_rect[iKeypadIndex].top;
    }

    return;
}

static T_VOID keypad_callback_func(T_eKEY_ID key_id, T_ePRESS_TYPE status)
{
    ST_MSG   msg;
	T_U8 long_press = 0; //TBD

	if (status == PRESS_LONG)
		long_press = 1;
    msg.msg     = IM_KEYINFO;
    msg.bparam  = key_id;
    msg.hwparam = (long_press<<8) | (T_U8)(status);
    post_int_message(&msg);
}

/**
 * @brief Scan keypad
 * Function keypad_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-21
 * @param T_VOID
 * @return T_eKEY_ID: The pressed key's ID
 * @retval
 */
T_eKEY_ID w_keypad_scan(T_VOID)
{
    T_eKEY_ID   keyID = kbNULL;
    T_U16       i;

    for (i = 0; i < MAX_PHY_KEY_NUM; i++)
    {
        if (s_cur_mouse_x >= s_keypad_rect[i].left &&
            s_cur_mouse_x < s_keypad_rect[i].right &&
            s_cur_mouse_y >= s_keypad_rect[i].top &&
            s_cur_mouse_y < s_keypad_rect[i].bottom)
        {
            keyID = s_keypad_rect[i].key_id;
            break;
        }
    }

    return keyID;
}

/**
 * @brief Keypad interrupt handler for WIN32
 * If chip detect that KEYPAD GPIO interrupt, this function will be called.
 * Function keypad_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-21
 * @param T_U8 MouseState: 0: mouse down, 1: mouse move, 2: mouse up
 * @return T_BOOL
 * @retval AK_TRUE: keypad interrupt arrive
 */
T_BOOL keypad_interrupt_handler_WIN32(T_U8 MouseState, T_U16 x, T_U16 y)
{
    T_PRESS_KEY key;

    s_cur_mouse_x = x;
    s_cur_mouse_y = y;
    //keypad was pressed
    if( MouseState == 1)
    {
        s_prev_key_id = w_keypad_scan();  //record the first key_id
        if (s_prev_key_id != kbNULL)
        {
            s_key_timer = vtimer_start(DEFAULT_LONG_KEY_DELAY, AK_FALSE, keypad_timer); //use keypad_timer to verify the key function
            s_sent_key_num = 0;
        }
        else
            return AK_FALSE;
    }
    else if (MouseState == 0)   //up
    {
        //stop all timer
        if (s_key_timer != ERROR_TIMER)
            s_key_timer = vtimer_stop( s_key_timer );
        if (s_intervaltimer_id != ERROR_TIMER)
        {
            s_intervaltimer_id = vtimer_stop( s_intervaltimer_id );
        }

        //call keypad callback function when the key is not a kbNULL one
        if (s_prev_key_id != kbNULL)
        {
            if( !s_sent_key_num ) //if the key is a short one, m_sent_key_num = 0,sent a short key
            {
                key.pressType = eKEYDOWN;
            }
            else                 //if the key is a up one after long, sent a up key
            {
                key.pressType = eKEYUP;
            }
            key.id = s_prev_key_id;
            keypad_callback_func(key.id, key.pressType);
        }

        s_sent_key_num =0;
        s_prev_key_id = kbNULL;
        s_key_timer = ERROR_TIMER;
        s_intervaltimer_id = ERROR_TIMER;
    }
    return AK_TRUE;
}


/**
 * @brief Keypad timer
 * Function keypad_init() must be called before call this function
 * @author YiRuoxiang
 * @date 2006-01-13
 * @param T_TIMER timer_id: timer ID
 * @param T_U32 delay: timer delay
 * @return T_VOID
 * @retval
 */
static T_VOID keypad_timer(T_TIMER timer_id, T_U32 delay)
{
    T_PRESS_KEY key;

    
    key.id = s_prev_key_id;
    s_sent_key_num = 1;

    key.pressType = eKEYPRESS;
    keypad_callback_func(key.id, key.pressType);

    if (s_key_timer != ERROR_TIMER)
    s_key_timer = vtimer_stop( s_key_timer );

    if((kbLEFT == s_prev_key_id) || (kbRIGHT == s_prev_key_id))
    {
        s_intervaltimer_id = vtimer_start(DEFAULT_INTERVAL_KEY_DELAY, AK_TRUE, keypad_intervaltimer);
    }

}

/**
 * @brief Keypad interval timer
 * Function keypad_init() must be called before call this function
 * @author YiRuoxiang
 * @date 2006-02-10
 * @param T_TIMER timer_id: timer ID
 * @param T_U32 delay: timer delay
 * @return T_VOID
 */
static T_VOID keypad_intervaltimer(T_TIMER timer_id, T_U32 delay)
{
    if (s_prev_key_id != kbNULL)
        keypad_callback_func(s_prev_key_id, PRESSING);
}


/* end of file */

