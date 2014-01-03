#ifndef __CBUTTON_CTRL_H__
#define __CBUTTON_CTRL_H__

#include "Ctrl_Public.h"
#include "Fwl_Timer.h"

T_VOID  Display_Button(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U16 stringID, T_U8 bFocus);
T_VOID  Display_Title(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID, T_U16 imgID, T_U16 icon_l, T_U16 icon_r);
T_VOID  Display_NavgateBar(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID);


/* 
 @  Scroll Text Rect Control
*/
typedef struct tagCTxtScrollCtrl
{
    T_U16   string[128];
    T_U16   curTicks;
    T_U16   sumTicks;
    T_TIMER timer;

    T_U32    codepage;
    T_U16    x;
    T_U16    y;
    T_U16    width;
    T_U16    color;
    T_BG_PIC Gg;
    T_U16    s_val;

}CTxtScrollCtrl;

T_BOOL  TxtScroll_Init(CTxtScrollCtrl *pTxtScroll, T_U16 msDelay);
T_VOID  TxtScroll_Free(CTxtScrollCtrl *pTxtScroll);
T_VOID  TxtScrool_ReSetString(CTxtScrollCtrl *pTxtScroll, T_U32 string, T_BOOL bStrFromRes, 
                              T_U16 s_val, T_U32 codepage, T_U32 x, T_U32 y, T_U16 Width,T_U16 color, T_BG_PIC *pGg);
T_VOID  TxtScroll_Pause(CTxtScrollCtrl *pTxtScroll);

T_VOID  TxtScroll_Show(CTxtScrollCtrl *pTxtScroll);
//T_VOID    TxtScroll_Hndler(T_TIMER timerId, T_U32 delay);


#endif


