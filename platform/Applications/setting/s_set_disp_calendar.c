/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_disp_calendar.c
 * @BRIEF display current calendar info 
 * @Author：Yan_Junping
 * @Date：2009-04-29
 * @Version：
**************************************************************************/

#include "m_event.h"
#include "Eng_Font.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "Fwl_Keypad.h"
#include "eng_lunarcalendar.h"
#include "Eng_Graph.h"
#include "Fwl_RTC.h"
#include "Fwl_FreqMgr.h"
#include "M_event_api.h"

#if(NO_DISPLAY == 0)

#ifdef SUPPORT_CALENDAR

#define  CALENDAR_REFRESH_NONE          0X00000000
#define  CALENDAR_REFRESH_DAY           0x00000001
#define  CALENDAR_REFRESH_HOUR          0x00000008
#define  CALENDAR_REFRESH_MINUTE        0x00000010
#define  CALENDAR_REFRESH_ALL           0xFFFFFFFF

#if (USE_COLOR_LCD)
#if (LCD_TYPE == 3)
#if (LCD_HORIZONTAL == 1)
    #define CALENDAR_WEEK_TOP   (64)

    #define CALENDAR_HOUR_LEFT  (136)
    #define CALENDAR_HOUR_TOP   (90)

    #define CALENDAR_MIN_LEFT   (CALENDAR_HOUR_LEFT+30)
    #define CALENDAR_MIN_TOP    (CALENDAR_HOUR_TOP)

    #define CALENDAR_DATE_TOP   (120)
    #define CALENDAR_LUNAR_TOP  (CALENDAR_DATE_TOP+FONT_HEIGHT*2)
#else
    #define CALENDAR_WEEK_TOP   (24)

    #define CALENDAR_HOUR_LEFT  (96)
    #define CALENDAR_HOUR_TOP   (50)

    #define CALENDAR_MIN_LEFT   (126)
    #define CALENDAR_MIN_TOP    (CALENDAR_HOUR_TOP)

    #define CALENDAR_DATE_TOP   (80)
    #define CALENDAR_LUNAR_TOP  (CALENDAR_DATE_TOP+FONT_HEIGHT*2)
#endif
#else
    #define CALENDAR_WEEK_TOP   (24)

    #define CALENDAR_HOUR_LEFT  (40)
    #define CALENDAR_HOUR_TOP   (50)

    #define CALENDAR_MIN_LEFT   (70)
    #define CALENDAR_MIN_TOP    (CALENDAR_HOUR_TOP)

    #define CALENDAR_DATE_TOP   (80)
    #define CALENDAR_LUNAR_TOP  (CALENDAR_DATE_TOP+FONT_HEIGHT*2)
#endif

#define CALENDAR_FONTCLR    (CLR_WHITE)
#define CALENDAR_BACKCLR    (CLR_BLUE)

#else
#define CALENDAR_WEEK_LEFT  (15)
#define CALENDAR_WEEK_TOP   (5)

#define CALENDAR_HOUR_LEFT  (65)
#define CALENDAR_HOUR_TOP   (4)

#define CALENDAR_MIN_LEFT   (90)
#define CALENDAR_MIN_TOP    (CALENDAR_HOUR_TOP)

#define CALENDAR_DATE_TOP   (23)
#define CALENDAR_LUNAR_TOP  (CALENDAR_DATE_TOP+20)

#define CALENDAR_FONTCLR    (CLR_WHITE)
#define CALENDAR_BACKCLR    (CLR_BLACK)
#endif

typedef struct
{
    T_SYSTIME   date;
    T_LUNARCALENDAR lunar;
    T_BOOL  bLunarValid;
    T_U32   refresh;
}T_SET_DISP_CALENDAR;


static T_SET_DISP_CALENDAR* pDispCalendar = AK_NULL;


static T_VOID Calendar_DispString(T_S16 *pString, T_U16 fontColor, T_U16 backColor, T_U16 startY)
{
    T_U16 len = 0;
    T_U16 i = 0;
    T_U16 line = 0;
   
    len= Utl_UStrLen(pString);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Fwl_FillRect(0, (T_U8)startY, MAIN_LCD_WIDTH, FONT_HEIGHT, backColor);  
    #endif
    while(len- i> 0)
    {
        T_U16 width, drawWidth;
        
        width= GetStringDispWidth(CP_UNICODE, (T_U8* )(pString+ i), 0);
        drawWidth= width> MAIN_LCD_WIDTH? 0: (MAIN_LCD_WIDTH- width)>> 1;
        i += DispStringInWidth(CP_UNICODE, (T_POS)(drawWidth), (T_POS)(startY+ line* FONT_HEIGHT), (T_U8* )(pString+ i),
            MAIN_LCD_WIDTH, fontColor)>>1;
        if(pString[i] == '\n')
            i++;
        line++;
    }   
}

static T_VOID Calendar_DispSolarDate(T_VOID)
{
    T_S16 pBuffer[50];
        
    memset((T_S8*)pBuffer, 0, 50*sizeof(T_S16));
    Utl_UItoa(pDispCalendar->date.year, pBuffer, 10);
    Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer) , GetResStringPtr(eRES_STR_LUNAR_YEAR));
    Utl_UItoa(pDispCalendar->date.month, pBuffer + Utl_UStrLen(pBuffer), 10); 
    
    if ((gb.Lang == eRES_LANG_CHINESE_SIMPLE) 
        || (gb.Lang == eRES_LANG_CHINESE_TRADITION)
        || (gb.Lang == eRES_LANG_CHINESE_BIG5)) 
    {
        Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer) , GetResStringPtr(eRES_STR_LUNAR_MONTH));
    }
    else
    {
        pBuffer[Utl_UStrLen(pBuffer)] = UNICODE_DOT;
    }
    
    Utl_UItoa(pDispCalendar->date.day, pBuffer + Utl_UStrLen(pBuffer), 10); 
    Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer) , GetResStringPtr(eRES_STR_DAY));
   
    Calendar_DispString(pBuffer, CALENDAR_FONTCLR, CALENDAR_BACKCLR, CALENDAR_DATE_TOP);
}

static T_VOID Calendar_DispLunarDate(T_VOID)
{
    T_S16 pBuffer[50];
    T_U16 iMonth,iDay;


    if (!pDispCalendar->bLunarValid)
    {
        return;
    }
    
    memset((T_S8*)pBuffer, 0, 50*sizeof(T_S16));
    iMonth = pDispCalendar->lunar.cMonth;
    iDay = pDispCalendar->lunar.cDay;
    //农历:
    Utl_UStrCpy(pBuffer, GetResStringPtr(eRES_STR_LUNAR));
    
    //月
    if(!pDispCalendar->lunar.isLeap && iMonth ==1 )
    {
        Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_ONE));
    }
    else
    {   
        if(pDispCalendar->lunar.cMonth <= 10)
        {
            if (iMonth == 1)
            {
                Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_ZHENG));
            }
            else
            {
                  Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_TWO + (iMonth -2)));
            }
        }
        else if (iMonth == 11)
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_NOV));
        }
        else
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_DEC));
        }     
    }
    
    Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer) , GetResStringPtr(eRES_STR_LUNAR_MONTH));


    //日
    if(iDay != 20 && iDay !=30)
    {
        if (iDay <= 10)
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_CHU));
        }
        else if (iDay < 20)
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_TEN));
        }
        else if (iDay < 30)
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_NIAN));
        }
        else
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_THREE));
        }

        Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_ONE + ((iDay-1)%10)));
    }
    else
    {
        if (iDay == 20)
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_NIANSHI));
        }
        else
        {
            Utl_UStrCpy(pBuffer+Utl_UStrLen(pBuffer), GetResStringPtr(eRES_STR_LUNAR_SANSHI));
        }
    }

    Calendar_DispString(pBuffer, CALENDAR_FONTCLR, CALENDAR_BACKCLR, CALENDAR_LUNAR_TOP);
}

static T_VOID Calendar_DispWeek(T_VOID)
{
    T_U16 pBuffer[50];

    if (!pDispCalendar->bLunarValid)
    {
        return;
    }
     
    memset((T_S8*)pBuffer, 0, 50*sizeof(T_S16));
   
    switch(pDispCalendar->lunar.weekDay) 
    {
    case 0:
        Utl_UStrCat( pBuffer, GetResStringPtr(eRES_STR_WEEK_DAY));
        break;
    case 1:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_ONE));
        break;
    case 2:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_TWO));
        break;
    case 3:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_THREE));
        break;
    case 4:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_FOUR));
        break;
    case 5:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_FIVE));
        break;
    case 6:
        Utl_UStrCat( pBuffer, GetResStringPtr((T_U32)eRES_STR_WEEK_SIX));
        break;
    default:
        break;
    }

#if (USE_COLOR_LCD)
    Calendar_DispString(pBuffer, CALENDAR_FONTCLR, CALENDAR_BACKCLR, CALENDAR_WEEK_TOP);
#else
    DispStringInWidth(CP_UNICODE, (T_POS)CALENDAR_WEEK_LEFT, (T_POS)CALENDAR_WEEK_TOP, CONVERT2STR(pBuffer),\
        GetStringDispWidth(CP_UNICODE, CONVERT2STR(pBuffer), 0), CALENDAR_FONTCLR);      
#endif
}

static T_VOID Calendar_DispHour(T_VOID)
{
    T_U8 pBuffer[16];
    T_U16 dispWidth;
    
    memset(pBuffer, 0, 16);

    if(pDispCalendar->date.hour < 10)
    {
        pBuffer[0] = '0';
        Utl_Itoa(pDispCalendar->date.hour,&pBuffer[1]);
    }
    else
    {
        Utl_Itoa(pDispCalendar->date.hour,pBuffer); 
    }

    pBuffer[Utl_StrLen(pBuffer)] = ' ';
    pBuffer[Utl_StrLen(pBuffer)] = ':';
    pBuffer[Utl_StrLen(pBuffer)] = ' ';
    dispWidth = GetStringDispWidth(CP_UNICODE, pBuffer, 0);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))        
    Fwl_FillRect(CALENDAR_HOUR_LEFT, CALENDAR_HOUR_TOP, (T_LEN)dispWidth, FONT_HEIGHT, CALENDAR_BACKCLR);
    #endif
    DispStringInWidth(CP_936,CALENDAR_HOUR_LEFT,CALENDAR_HOUR_TOP,(T_U8 *)pBuffer,dispWidth,CALENDAR_FONTCLR);
}

static T_VOID Calendar_DispMinute(T_VOID)
{
    T_U8 pBuffer[16];
    T_U16 dispWidth;
    
    memset(pBuffer, 0, 16);
    
    if(pDispCalendar->date.minute < 10)
    {
        pBuffer[0] = '0';
        Utl_Itoa(pDispCalendar->date.minute,&pBuffer[1]);
    }
    else
    {
        Utl_Itoa(pDispCalendar->date.minute,pBuffer);   
    }
    pBuffer[Utl_StrLen(pBuffer)] = ' ';

    dispWidth = GetStringDispWidth(CP_UNICODE, pBuffer, 0);
#if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Fwl_FillRect(CALENDAR_MIN_LEFT, CALENDAR_MIN_TOP, (T_LEN)dispWidth, FONT_HEIGHT, CALENDAR_BACKCLR);
#endif
    DispStringInWidth(CP_936,CALENDAR_MIN_LEFT,CALENDAR_MIN_TOP,(T_U8 *)pBuffer, dispWidth,CALENDAR_FONTCLR);

}


static  T_VOID  DispCalendarPaint(T_VOID)
{


    if (CALENDAR_REFRESH_NONE != pDispCalendar->refresh)
    {
#ifdef OS_WIN32
        Utl_Convert_SecondToDate(Fwl_GetSecond(), &pDispCalendar->date);
#else
        //获取系统当前时间，并进行合法性判断
        Fwl_GetRTCtime(&pDispCalendar->date);
#endif
    }

    if ((pDispCalendar->refresh == CALENDAR_REFRESH_ALL)
        ||(pDispCalendar->refresh & CALENDAR_REFRESH_DAY == CALENDAR_REFRESH_DAY))
    {
        Fwl_FreqPush(FREQ_APP_MAX);
        pDispCalendar->bLunarValid = Eng_LunarCalendarCovert(pDispCalendar->date, &pDispCalendar->lunar);

        if (pDispCalendar->refresh == CALENDAR_REFRESH_ALL)
        {
        #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CALENDAR_BACKCLR);
        #else
            Eng_ImageResDisp(0,0,eRES_IMAGE_CTRLBKG,AK_FALSE);
        #endif
        }
        
        Calendar_DispWeek();
        Calendar_DispSolarDate();
        Calendar_DispLunarDate();
        Fwl_FreqPop();
    }

    //dispaly hour string
    if(pDispCalendar->refresh & CALENDAR_REFRESH_HOUR)
    {
        Calendar_DispHour();
    }

    //display minute string
    if(pDispCalendar->refresh & CALENDAR_REFRESH_MINUTE)
    {
        Calendar_DispMinute();
    }

    pDispCalendar->refresh = CALENDAR_REFRESH_NONE;
}

void initset_disp_calendar(void)
{
     
    pDispCalendar = (T_SET_DISP_CALENDAR*)Fwl_Malloc(sizeof(T_SET_DISP_CALENDAR));
    AK_ASSERT_PTR_VOID(pDispCalendar, "pDispCalendar alloc error\n");
    memset(pDispCalendar, 0, sizeof(T_SET_DISP_CALENDAR));
    pDispCalendar->bLunarValid = AK_FALSE;


#ifdef OS_WIN32
    Utl_Convert_SecondToDate(Fwl_GetSecond(), &pDispCalendar->date);
#else
    //获取系统当前时间，并进行合法性判断
    Fwl_GetRTCtime(&pDispCalendar->date);
#endif
    pDispCalendar->refresh = CALENDAR_REFRESH_ALL;
    
}

void exitset_disp_calendar(void)
{
    pDispCalendar = Fwl_Free(pDispCalendar);
}

void paintset_disp_calendar(void)
{
    DispCalendarPaint();
}

unsigned char handleset_disp_calendar(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    switch( event )
    {
    case M_EVT_CALENDAR:
        break;

    case M_EVT_PUB_TIMER: 
          {   
              T_SYSTIME  date;

#ifdef OS_WIN32
              Utl_Convert_SecondToDate(Fwl_GetSecond(), &date);
#else
              Fwl_GetRTCtime(&date);
#endif
        
              if (pDispCalendar->date.minute != date.minute)
              {
                  pDispCalendar->refresh |= CALENDAR_REFRESH_MINUTE;
              }
               
              if(pDispCalendar->date.hour != date.hour)
              {
                  pDispCalendar->refresh |= CALENDAR_REFRESH_HOUR;
              }

              if(pDispCalendar->date.day != date.day)
              {
                  pDispCalendar->refresh |= CALENDAR_REFRESH_DAY;
              }        
          }         
          break;  
    case M_EVT_USER_KEY:
         if ((kbOK == pEventParm->c.Param1) 
             ||(kbMODE == pEventParm->c.Param1))
         {
             m_triggerEvent(M_EVT_EXIT, pEventParm);
         }
        return 0;
        
    default:
          break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_disp_calendar(void)
{
}


void paintset_disp_calendar(void)
{

}

void exitset_disp_calendar(void)
{

}

unsigned char handleset_disp_calendar(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}

#endif

#else   //#if(NO_DISPLAY == 0)
void initset_disp_calendar(void)
{
}
void paintset_disp_calendar(void)
{
}
void exitset_disp_calendar(void)
{
}
unsigned char handleset_disp_calendar(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_disp_calendar\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

