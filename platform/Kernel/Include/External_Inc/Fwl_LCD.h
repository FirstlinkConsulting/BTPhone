/**
 * @file Fwl_LCD.h
 * @brief This header file is for display function prototype
 * 
 */
#ifndef __FWL_LCD_H__
#define __FWL_LCD_H__


#include "Gbl_Global.h"


//横向刷屏
#define VIDEO_HORIZONTAL_SHOW       1  

#define DEFAULT_DIRECTION   90

#define USE_LCD_PARALLEL                1

/* visual LCD parameter */
#if (USE_COLOR_LCD)
    #if (LCD_HORIZONTAL)
        #if (LCD_TYPE==0)
        #define MAIN_LCD_WIDTH          160            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         128             ///<length of LCD height
        #endif
        #if (LCD_TYPE==1)
        #define MAIN_LCD_WIDTH          220            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         176             ///<length of LCD height
        #endif
        #if (LCD_TYPE==2)
        #define MAIN_LCD_WIDTH          128            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         128             ///<length of LCD height
        #endif
        #if (LCD_TYPE==3)
        #define MAIN_LCD_WIDTH          320            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         240             ///<length of LCD height
        #endif
    #else
        #if (LCD_TYPE==0)
        #define MAIN_LCD_WIDTH          128            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         160             ///<length of LCD height
        #endif
        #if (LCD_TYPE==1)
        #define MAIN_LCD_WIDTH          176            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         220             ///<length of LCD height
        #endif
        #if (LCD_TYPE==2)
        #define MAIN_LCD_WIDTH          128            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         128             ///<length of LCD height
        #endif
        #if (LCD_TYPE==3)
        #define MAIN_LCD_WIDTH          240            ///<length of LCD width
        #define MAIN_LCD_HEIGHT         320             ///<length of LCD height
        #endif
    #endif
#else
    #if (LCD_TYPE==0)
    #define MAIN_LCD_WIDTH          128            ///<length of LCD width
    #define MAIN_LCD_HEIGHT         64             ///<length of LCD height
    #endif
    #if (LCD_TYPE==1)
    #define MAIN_LCD_WIDTH          128            ///<length of LCD width
    #define MAIN_LCD_HEIGHT         32             ///<length of LCD height
    #endif
    #if (LCD_TYPE==2)
    #define MAIN_LCD_WIDTH          96            ///<length of LCD width
    #define MAIN_LCD_HEIGHT         40            ///<length of LCD height
    #endif
#endif
#define DISPLAY_COMMONBUFFSIZE      2048

typedef T_U16       T_LEN;              /* length type: unsigned short */
typedef T_U16       T_POS;              /* position type: short */

typedef enum
{
    FWLLCD_L2MODE,
    FWLLCD_VIDEOMODE,
    FWLLCD_MODENUM
}T_FWLLCD_MODE; //拍照录像模式选择

#if(NO_DISPLAY == 1)

#define Fwl_DisplayInit()
#define Fwl_LCD_lock(locked);
#define Fwl_SetContrast(contrast)
#define Fwl_Enable_L2Refresh(enable)      
#define Fwl_LCD_on(bDelayEnable)
#define Fwl_LCD_off()
#define Fwl_DisplayBacklightOn()
#define Fwl_FillRect(left, top, width, height, color)
#define Fwl_lcd_Rotate(rotate)
#define Fwl_Lcd_Set_L2_Refresh()
#define Fwl_Lcd_Set_VEDIO_Refresh()
#define Fwl_Lcd_Set_CPU_Refresh()

//#define Fwl_SwitchLcdMode(mode)
#define Fwl_LCDVMCheckRefresh()         AK_TRUE
#define Fwl_LCDVMSetRefresh(left, top, width, height, reversed)

#else

T_VOID Fwl_DisplayInit(T_VOID);
T_VOID Fwl_LCD_lock(T_U8 locked); 
T_VOID Fwl_SetPixel(T_U16 x, T_U16 y, T_U16 color);
//T_VOID Fwl_GetPixel(T_U16 x, T_U16 y);

T_VOID Fwl_RefreshRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content, T_BOOL reversed);
//T_VOID Fwl_FillRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color);
T_U8 Fwl_SetContrast(T_U8 contrast); 
T_BOOL Fwl_Enable_Set_Contrast(T_VOID);

#if (USE_COLOR_LCD)
    #if(VIDEO_HORIZONTAL_SHOW)
        T_VOID Fwl_Lcd_Rotate1(T_BOOL rotate);
        #define Fwl_lcd_Rotate(rotate)           Fwl_Lcd_Rotate1(rotate)
    #else
        #define Fwl_lcd_Rotate(rotate)           //Fwl_Lcd_Rotate1(rotate)
    #endif
#else
#define Fwl_lcd_Rotate(rotate)      
#endif

#ifdef OS_ANYKA
T_VOID Fwl_Enable_L2Refresh1(T_BOOL enable);
#define Fwl_Enable_L2Refresh(enable)        Fwl_Enable_L2Refresh1(enable)
#else
#define Fwl_Enable_L2Refresh(enable)
#endif

T_VOID Fwl_DrawRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color, T_U8 filled);
T_VOID Fwl_FillRect(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U16 color);
T_VOID Fwl_LCD_on(T_BOOL bDelayEnable);
T_VOID Fwl_LCD_off(T_VOID);

T_VOID Fwl_DisplayOn(T_VOID);
T_VOID Fwl_DisplayOff(T_VOID);
T_VOID Fwl_DisplayBacklightOn(T_VOID);
T_VOID Fwl_DisplayBacklightOff(T_VOID);
T_VOID Fwl_Lcd_Set_L2_Refresh(T_VOID);
T_VOID Fwl_Lcd_Set_VEDIO_Refresh(T_VOID);
T_VOID Fwl_Lcd_Set_CPU_Refresh(T_VOID);
T_VOID Fwl_DisplaySleep(T_VOID);

//T_VOID Fwl_SwitchLcdMode(T_FWLLCD_MODE mode);
T_BOOL Fwl_LCDVMCheckRefresh(T_VOID);
T_VOID Fwl_LCDVMSetRefresh(T_U16 left, T_U16 top, T_U32 width, T_U32 height, T_BOOL reversed);

extern T_VOID scaler2_config(T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH);//驱动库提供

#endif  //#if(NO_DISPLAY == 1)


#endif //__FWL_LCD_H__
