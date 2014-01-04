/**
 * @FILENAME: led_ctrl.h
 * @BRIEF usbdisk state machine
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR wzh
 * @DATE 2012-1-4
 * @VERSION 1.0
 * @REF
 */

#ifndef _LED_CTRL_H_
#define _LED_CTRL_H_

#include "anyka_types.h"

#ifdef SUPPORT_LEDHINT


// ����������ʹ�õĺ�
#define UData(Data)	    ((T_U32) (Data))
#define Fld(Size, Shft)	(((Size) << 16) + (Shft))
#define FShft(Field)	((Field) & 0x0000FFFF)
#define FInsrt(Value, Field) (UData (Value) << FShft (Field))

#define SEQ_EVERY_SHFT_AREA  (2)		//ÿ��λ�洢һ��״̬��Ϣ���ܹ�����ʾ16��Ԫ��

#define LED_STATE_SEQ(n) Fld(SEQ_EVERY_SHFT_AREA, (n) * SEQ_EVERY_SHFT_AREA)


typedef struct
{
    T_U16               time_interval;    // һ�λص�ʱ��������λ10����
    T_U8                prioty;           // ģʽ���ȼ���
    
    volatile T_U8       led_delay_len;    // ������е���ʾ����ʱ��ʱ�䣬��λΪһ�λص����
    volatile T_U8       led_seq_len;      // ����(led_state_seq)�ĳ���
    volatile T_U32      led_state_seq;    // led��ʾ���������������У�ÿһԪ��(2bit����ʾ��Ҫ��˸��LED)��ʾ��ʱ��Ϊһ�λص����
}T_LED_CTRL;

typedef enum
{
	LED_SHOW_OFF = 0,
	LED_SHOW_RED = 1,
	LED_SHOW_BLUE = 2,
	LED_SHOW_DOUBLE = 3
}T_LED_SHOW_STATE;

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
T_BOOL LedHint_Init(T_U8 MaxMode, T_U8 Led_num);

 
/**
 * @brief Free led module
 * @author hyl
 * @date 2013-5-14
 * @param T_VOID
 * @return T_VOID
 */
T_VOID LedHint_Free(T_VOID);
 
/**
 * @brief Add led indicate mode
 * @author hyl
 * @date 2013-5-14
 * @param led_mode: mode id
 * @param ledcfg: control blink ways
 * @return T_VOID
 */
T_BOOL LedHint_Add(T_U8 led_mode, T_LED_CTRL* ledcfg);
 
/**
 * @brief execute led mode
 * @author hyl
 * @date 2013-5-14
 * @param T_VOID
 * @return T_VOID
 */
T_BOOL LedHint_Exec(T_U8 MODE);
 
/**
 * @brief stop led indicate mode
 * @author hyl
 * @date 2013-5-14
 * @param T_VOID
 * @return T_VOID
 */
T_VOID LedHint_Stop(T_U8 MODE);


#endif
#endif
