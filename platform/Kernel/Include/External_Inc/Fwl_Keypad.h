/*******************************************************************************
 * @file    Fwl_Keypad.h
 * @brief   the header file of Fwl_Keypad.c
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_KEYPAD_H__
#define __FWL_KEYPAD_H__


#include "anyka_types.h"


#define GET_KEY_ID(pEventParam)      ((pEventParam)->c.Param1)
#define GET_KEY_TYPE(pEventParam)    ((pEventParam)->c.Param2)

//根据红外按键的键值来指定值
typedef enum {
    kbNULL = 0xFF,
    kbPRE = 0x07,       kbSPEED = 0x08,     kbNEXT = 0x09,      kbVOLSUB = 0x0c,    
    kbNEXTFILE = 0x0d,  kbOK = 0x15,        kbVOLADD = 0x16,    kbBACKWARD = 0x18,  
    kbPREFILE = 0x19,   kbRECORD = 0x40,    kbFUNC = 0x43,      kbBT = 0x44,
    kbMUTE = 0x45,      kbEQ = 0x46,        kbPOWER = 0x47,     kbFORWARD = 0x5e,
    kbLEFT,             kbRIGHT,            kbMODE,              kbVOLUME,            
    MAX_KEY_NUM
} T_eKEY_ID;

typedef enum {
    PRESS_SHORT = 0,
    PRESS_LONG,
    PRESS_UP,
    PRESSING,
    PRESS_DOUBLE,
    NO_PRESS,
    PRESS_TYPE_NUM
} T_ePRESS_TYPE;

typedef struct {
    T_eKEY_ID id;
    T_ePRESS_TYPE pressType;
}T_PRESS_KEY;


/*******************************************************************************
 * @brief   initialize keypad
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadInit(T_VOID);


/*******************************************************************************
 * @brief   enable keypad scan
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   [in]bEnable: enable or disable
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_KeypadEnable(T_BOOL bEnable);


/*******************************************************************************
 * @brief   scan keypad to get key id
 * @author  zhanggaoxin
 * @date    2013-03-18
 * @param   T_VOID
 * @return  T_eKEY_ID
 * @retval  the key id
*******************************************************************************/
T_eKEY_ID Fwl_KeypadScan(T_VOID);


#endif //__FWL_KEYPAD_H__

