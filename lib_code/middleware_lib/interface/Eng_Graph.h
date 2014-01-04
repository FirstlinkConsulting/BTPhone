/**
 * @file Eng_Graph.h
 * @brief This header file is for display function prototype
 * 
 */

#ifndef __ENG_GRAPH_H__
#define __ENG_GRAPH_H__

#include "anyka_types.h"
#include "Gbl_global.h"
#include "Eng_Time.h"

#define RGB_COLOR(r,g,b)    ((T_U16)(((r & 0xF8)<<8) | ((g & 0xFC)<<3) | ((b & 0xF8)>>3)))

#define CLR_BLACK       RGB_COLOR(0,0,0)
#define CLR_WHITE       RGB_COLOR(255,255,255)

#if (NO_DISPLAY == 0)
#define GRAPH_WIDTH     g_Graph.LCDWIDTH
#define GRAPH_HEIGHT    g_Graph.LCDHEIGHT
#else
#define GRAPH_WIDTH     0
#define GRAPH_HEIGHT    0
#endif

#if (USE_COLOR_LCD)
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#define CLR_CUSTOM      RGB_COLOR(120,120,120)
#else
#define CLR_CUSTOM      CLR_WHITE
#endif

#define CLR_RED         RGB_COLOR(255,0,0)
#define CLR_GREEN       RGB_COLOR(0,255,0)
#define CLR_BLUE        RGB_COLOR(0,0,255)
#define CLR_OKBG        RGB_COLOR(255, 255, 0)
#endif

typedef struct {
    T_POS       x;
    T_POS       y;
} T_POINT, *T_pPOINT;

typedef struct {
    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height; 
} T_RECT, *T_pRECT;

typedef struct
{
    T_LEN               LCDWIDTH;
    T_LEN               LCDHEIGHT;
} T_GRAPH;

extern T_GRAPH g_Graph;

T_BOOL Eng_PtInRect(const T_RECT *rect, T_POINT *pt);

#endif
