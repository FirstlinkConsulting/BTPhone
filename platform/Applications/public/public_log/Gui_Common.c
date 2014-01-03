/**
 * @file Gui_Common.c
 * @brief ANYKA software
 * this file will provide gui painting function .
 *
 * @author XuPing
 * @date    2008-1013
 * @version 1.0
 */
#include "akdefine.h"
#include "Gbl_Define.h"
#include "Gbl_Global.h"
#include "Eng_USB.h"
#include "Eng_Font.h"
#include "Fwl_osFS.h"
#include "Fwl_System.h"
#include "Fwl_RTC.h"
#include "Ctrl_Public.h"
#include "Eng_ImageResDisp.h"
#include "log_aud_control.h"
#include "log_radio_core.h"
#include "Gui_Common.h"
#include "Eng_AutoOff.h"
#include "Alarm_Common.h"

#if(NO_DISPLAY == 0)

extern  T_BOOL IsAudplayer(T_VOID);
extern  T_BOOL IsInEbk(T_VOID);
extern  T_BOOL IsInSysSet(T_VOID);
extern  T_BOOL IsInImage(T_VOID);
extern  T_BOOL Record_IsNULL(T_VOID);
extern  T_BOOL IsInAlarmSet(T_VOID);

T_BOOL Gui_InitListMenuEx(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID)
{
    T_BOOL ret;
    
    AK_ASSERT_PTR(pListMenu, "Gui_InitListMenu pListMenu is AK_NULL", AK_FALSE);

    #if(USE_COLOR_LCD)
    ret= ListMenu_InitEx(pListMenu,
        titleID,
        menuBegID, 
        menuEndID,
        eRES_IMAGE_MENUICON,
        0,
        TITLE_HEIGHT,
        MAIN_LCD_WIDTH,
        MAIN_LCD_HEIGHT- TITLE_HEIGHT
        );
#else
    ret= ListMenu_Init(pListMenu,
        titleID,
        menuBegID,
        menuEndID,
        eRES_IMAGE_MENUICON);
#endif

    return ret;
}

T_BOOL  Gui_InitListMenu(CListMenuCtrl *pListMenu, T_U16 titleID, T_U16 menuBegID, T_U16 menuEndID, T_BOOL isShowEndMemu)
{
    T_BOOL ret;
    

    ret=  Gui_InitListMenuEx(pListMenu, titleID, menuBegID, menuEndID);
    
    if(isShowEndMemu)
    {
         ListMenu_SetStyle(pListMenu, CTRL_LISTMENU_STYLE_QUIT_DISPSHOW| CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW );
    }
    else
    {
         ListMenu_SetStyle(pListMenu, CTRL_LISTMENU_STYLE_QUIT_DISPNONE | CTRL_LISTMENU_STYLE_SCROLLVER_DISPSHOW );
    }
    return ret;
}

T_BOOL  Gui_InitDialog(CDialogCtrl *pDialog, T_U16* hintString, T_U16 titleID, T_U16 modeType, T_U16 defaultMode)
{
    AK_ASSERT_PTR(pDialog, "Gui_ShowDialog 1", AK_FALSE);

#if(USE_COLOR_LCD)
    return Dialog_InitEx(pDialog, hintString, modeType, titleID, ICON_HINT_ENTRYPTR, defaultMode,
        0, TITLE_HEIGHT, MAIN_LCD_WIDTH, (T_LEN)(MAIN_LCD_HEIGHT- TITLE_HEIGHT) );
#else
    return Dialog_Init(pDialog, hintString, modeType, titleID, ICON_HINT_ENTRYPTR, defaultMode);
#endif
}

T_U16 Gui_GetResString(T_U16* ucBuffer, T_RES_STRING resStringID, T_U16 bufferLen)
{
    T_U16 count;
    
    count= GetResString(resStringID, ucBuffer, bufferLen);
    ucBuffer[count]= 0;
    return count;
}

T_BOOL Gui_InitProgress(CProgressCtrl *pProgress, T_S16 minValue, T_S16 maxValue, T_U16 titleStringID, T_FLOAT curValue,  T_RES_IMAGE bkImg)
{
#if(USE_COLOR_LCD)
    return Progress_InitEx(pProgress, minValue, maxValue, titleStringID, INVALID_IMAGERES_ID, 
        (T_U16)(maxValue- minValue), curValue, AK_FALSE, INVALID_IMAGERES_ID, bkImg, 
        0, TITLE_HEIGHT, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT- TITLE_HEIGHT);
#else
    return Progress_Init(pProgress, minValue, maxValue, titleStringID, INVALID_IMAGERES_ID, 
        (T_U16)(maxValue- minValue), curValue, AK_FALSE, INVALID_IMAGERES_ID, bkImg);
#endif
}
/**
 * @brief   display batter info 
 * @author  XuPing
 * @date    2008-10-13
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/
/**************************************************************************
* @brief 
* 
* @author zhao_xiaowei
* @date 2008
* @param 
* @param 
* @return 
***************************************************************************/
T_VOID Gui_DispResHint(T_U16 ResID, T_U16 fontColor, T_U16 backColor, T_S16 startY)
{
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL)) 

    T_U32 x, y;
    T_RES_IMAGE ImgID;

    if(eRES_STR_AUDIO_HINT_WAITING == ResID)
    {
        ImgID = eRES_IMAGE_CTRLBKG;
    }
    else
    {
        ImgID = eRES_IMAGE_IMG_ERROR;
    }
        
    x = GetStringDispWidth(CP_UNICODE, CONVERT2STR(ResID), 0);
    x = (AUD_LCD_WIDTH - x) / 2;
    y = (AUD_LCD_HIGH- FONT_HEIGHT) / 2;

    if (eRES_STR_AUDIO_HINT_NOMUSIC != ResID)
        Eng_ImageResDisp(0, 0, ImgID, AK_FALSE);       
    DispStringInWidth(CP_UNICODE, x, y, CONVERT2STR(ResID), MAIN_LCD_WIDTH, fontColor);
    
#else

    T_U16 lcdBuf[LCD_BUF_LEN+ 1];
    T_U16 count= GetResString(ResID, lcdBuf, LCD_BUF_LEN);
    lcdBuf[count]= 0;
    Gui_DispHint(lcdBuf, fontColor, backColor, startY);
    
#endif  
}

T_VOID Gui_FillRect(T_U16 x, T_U16 y, T_U16 width, T_U16 height, T_U16 backColor)
{
#if(USE_COLOR_LCD && LCD_TYPE == 3)
    T_U16 midHeight;
    midHeight= height>>1;
    Fwl_FillRect(x ,y, width, midHeight, backColor);
    Fwl_FillRect(0,(T_U16)(midHeight+y),width, (T_U16)(height- midHeight),backColor);
#else
    Fwl_FillRect(x, y, width, height, backColor);
#endif
}

/**************************************************************************
* @brief 
* 
* @author zhao_xiaowei
* @date 2008
* @param 
* @param 
* @return 
***************************************************************************/
T_VOID Gui_DispHint(T_U16* pString, T_U16 fontColor, T_U16 backColor, T_S16 startY)
{
    T_U32 len;
    T_U32 i;
    T_U32 line = 0;
    AK_ASSERT_PTR_VOID(pString, "pString is Empty");

    len= Utl_UStrLen(pString);
    if(startY < 0)
    {
        T_U16 totalLine;
        T_U16 leave;
        i= 0;
        while(len- i> 0)
        {
            i += (T_U16)GetStringDispWidth(CP_UNICODE, (T_U8* )(pString+ i), MAIN_LCD_WIDTH)>> 1;
            if(pString[i] == '\n')
                i += 1;
            line++;
        }
        totalLine= MAIN_LCD_HEIGHT/ FONT_HEIGHT;
        leave= MAIN_LCD_HEIGHT% FONT_HEIGHT;
        if(line >= totalLine)
        {
            startY= 0;
        }
        else
        {
            startY= (T_S16) (((((totalLine- line+ 1)>>1)) * FONT_HEIGHT )+ leave);
        }
    }
     
    i= 0;
    line= 0;

    Gui_FillRect(0, 0, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, backColor);
    while(len- i> 0)
    {
        T_U32 width, drawWidth;
   
        width= GetStringDispWidth(CP_UNICODE, (T_U8* )(pString+ i), 0);
        drawWidth= width> MAIN_LCD_WIDTH? 0: (MAIN_LCD_WIDTH- width)>> 1;
        i += DispStringInWidth(CP_UNICODE, (T_POS)(drawWidth), (T_POS)(startY+ line* FONT_HEIGHT), (T_U8* )(pString+ i),
            MAIN_LCD_WIDTH, fontColor)>>1;
        if(pString[i] == '\n')
            i++;
        line++;
    }   

     
}

/**************************************************************************
* @brief 
* 
* @author zhao_xiaowei
* @date 2008
* @param 
* @param 
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/
#if(USE_COLOR_LCD)
T_BOOL Gui_DispTitle(T_RES_STRING stringID, T_U16 fontColor, T_RES_IMAGE backImgID, T_U16 titleHight)
{
    T_BG_PIC backPic;
    T_U32 x, y;
    T_U16 resLen;
 
    if((T_U16)stringID == INVALID_STRINGRES_ID || (T_U16)backImgID == INVALID_IMAGERES_ID)
    {
        return AK_FALSE;
    }
    resLen= GetStringDispWidth(CP_UNICODE, (T_U8 *)stringID, 0);
    if(resLen>= MAIN_LCD_WIDTH)
    {
        x= 0;
    }
    else
    {
        x= (MAIN_LCD_WIDTH- resLen)>>1;
    }

    if(titleHight< FONT_HEIGHT)
    {
        y= 0;
    }
    else
    {
        y= (titleHight- FONT_HEIGHT)>>1;
    }

    backPic.hOffset= (T_POS)x;
    backPic.vOffset= (T_POS)y;
    backPic.resId= (T_U16)backImgID;
    DispStringOnPic(CP_UNICODE, x, y, (T_U8 *)stringID, MAIN_LCD_WIDTH, fontColor, &backPic);
    return AK_TRUE;
}
#endif

#pragma arm section code = "_audioplayer_"

#if (3 == LCD_TYPE && 1 == LCD_HORIZONTAL)

T_VOID Gui_DispBackgroundPlay(T_POS x, T_POS y ,T_RES_IMAGE imgID)
{

}

#if (STORAGE_USED == SPI_FLASH)//spi版本资源图片剪裁，时间图片暂用一套
T_VOID Gui_DispTime(T_POS x, T_POS y, const T_RES_IMAGE dataBuf[10],T_RES_IMAGE ColonID)
{
    T_SYSTIME  date;
    
    #ifdef OS_WIN32
    T_U32 tick;       
    tick = Fwl_GetSecond();
    Utl_Convert_SecondToDate(tick, &date);
    #else
    Fwl_GetRTCtime(&date);
    #endif

    Eng_ImageResDisp(x, y, dataBuf[date.hour/10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[date.hour/10]);    
    Eng_ImageResDisp(x, y, dataBuf[date.hour%10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[date.hour/10]);
    
    Eng_ImageResDisp(x, y, ColonID, AK_FALSE);
    x += Eng_GetResImageWidth(ColonID);
    
    Eng_ImageResDisp(x, y, dataBuf[date.minute/10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[date.hour/10]);
    
    Eng_ImageResDisp(x, y, dataBuf[date.minute%10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[date.hour/10]);

}
#else
T_VOID Gui_DispTime(T_POS x, T_POS y, const T_RES_IMAGE dataBuf[4][10],T_RES_IMAGE ColonID)
{
    T_SYSTIME  date;
    
    #ifdef OS_WIN32
    T_U32 tick;       
    tick = Fwl_GetSecond();
    Utl_Convert_SecondToDate(tick, &date);
    #else
    Fwl_GetRTCtime(&date);
    #endif

    Eng_ImageResDisp(x, y, dataBuf[0][date.hour/10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[0][date.hour/10]);    
    Eng_ImageResDisp(x, y, dataBuf[1][date.hour%10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[0][date.hour/10]);
    
    Eng_ImageResDisp(x, y, ColonID, AK_FALSE);
    x += Eng_GetResImageWidth(ColonID);
    
    Eng_ImageResDisp(x, y, dataBuf[2][date.minute/10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[0][date.hour/10]);
    
    Eng_ImageResDisp(x, y, dataBuf[3][date.minute%10], AK_FALSE);
    x += Eng_GetResImageWidth(dataBuf[0][date.hour/10]);

}
#endif
#endif

T_BOOL Gui_DispBat(T_POS x, T_POS y)
{
#if (!((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL)))

    static T_U32 disBatLev = 0;
    T_U32 tempBatLev = 0;

#ifdef OS_ANYKA
    if (Fwl_DetectorGetStatus(DEVICE_CHG) 
        && (BATTERY_STAT_EXCEEDVOLT != gb.batStat))
    {
        disBatLev = ((disBatLev+1) % BATTERY_STAT_NOR_FULL);
        tempBatLev = disBatLev+1;
    }
    else
#endif
    {
        tempBatLev = gb.batStat;
    }
    //battery level
    switch(tempBatLev)
    {
        case BATTERY_STAT_LOW_WARN:
        case BATTERY_STAT_NOR_L0:
        case BATTERY_STAT_LOW_SHUTDOWN:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL0, AK_FALSE);           //eRES_IMAGE_AUDBATL0
           break;
        case BATTERY_STAT_NOR_L1:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL1, AK_FALSE);           //eRES_IMAGE_AUDBATL1
           break;
        case BATTERY_STAT_NOR_L2:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL2, AK_FALSE);           //eRES_IMAGE_AUDBATL2
           break;
        case BATTERY_STAT_NOR_L3:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL3, AK_FALSE);           // eRES_IMAGE_AUDBATL3
           break;
        case BATTERY_STAT_NOR_FULL:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL4, AK_FALSE);           //eRES_IMAGE_AUDBATL4
           break;
        default:
           Eng_ImageResDisp(x, y, eRES_IMAGE_AUDBATL4, AK_FALSE);           //eRES_IMAGE_AUDBATL4
           break;
    }
    
#endif

    return AK_TRUE;
}
#pragma arm section code

/**
 * @brief   Gui_SetClrTitle
 * @author  Set UI title of color LCD
 * @date    2008-10-13
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:  ok
            AK_FALSE: failed
 **/

#pragma arm section code = "_audioplayer_menu_"
T_VOID  Gui_SetClrTitle(T_POS x, T_POS y, T_LEN width, T_LEN height, T_U32 stringID, T_U16 imgID)
{
#if (USE_COLOR_LCD)
#if (!(3 == LCD_TYPE && 1 == LCD_HORIZONTAL))

    T_U32  i, j;
    T_BG_PIC pic;
    T_U16    img = imgID;

    //draw background
    if(INVALID_IMAGERES_ID == img)
    {
        img = TITLE_BKG_IMAGEPTR;
    }
    
    Eng_ImageResDisp(x, y, img, AK_FALSE);

    //draw string
    if(INVALID_STRINGRES_ID != stringID)
    {
        j = (T_U8)GetStringDispWidth(CP_UNICODE, CONVERT2STR(stringID), 0);
        i = x + ((j >= width) ? 0: ((width-j)>>1)); 
        j = y + ((height > FONT_HEIGHT) ? ((height - FONT_HEIGHT)>>1):0);

        pic.hOffset = (T_POS)i;
        pic.vOffset = (T_POS)(j);
        pic.resId   = img;  
        DispStringOnPic(CP_UNICODE, (T_POS)i, (T_POS)j, CONVERT2STR(stringID), (T_U16)(CTRL_WND_WIDTH - i), CLR_WHITE, &pic);
    }
#endif 
#endif
}

#if (USE_COLOR_LCD)
T_U16 GetUITitleResID(T_VOID)//T_ePlayingType type ,T_AUD_LISTSTLE style)
{
    T_U16   ResID = INVALID_IMAGERES_ID;
#if (STORAGE_USED != SPI_FLASH)//以后nand版本要改成topbar，spi版本不需要，屏蔽

    if(IsAudplayer())
    {
        switch(Aud_AudGetListStyle())    
        {
            case eAUD_MUSIC:
                ResID = eRES_IMAGE_TOP_MP3;
                break;
            case eAUD_VOICE:
                ResID = eRES_IMAGE_TOP_VOICE;
                break;
            default:
                ResID = INVALID_IMAGERES_ID;
                break;
        }
    }

    
    if(INVALID_IMAGERES_ID == ResID)
    {
        if(!Record_IsNULL())
            ResID = eRES_IMAGE_TOP_RECORD;
        else if(IsInRadio())
            ResID = eRES_IMAGE_TOP_FM;
        else if(IsInEbk())
            ResID = eRES_IMAGE_TOP_EBOOK;
#if(USE_ALARM_CLOCK)
        else if(IsInAlarmSet())
            ResID= IMAGE_ALARM_SET;
#endif
        else if(IsInSysSet())
            ResID = eRES_IMAGE_TOP_SETUP;
        else if(IsInImage())
            ResID = eRES_IMAGE_TOP_PHOTO;
        else
            ResID= eRES_IMAGE_PUB_TOPBAR_BK;
    }

 /*   if(INVALID_IMAGERES_ID == ResID)
    {
        switch(type)
        {
            case eRecord:
                ResID = eRES_IMAGE_TOP_RECORD;
                break;      
            case eRadio:
                ResID = eRES_IMAGE_TOP_FM;
                break;  
            case eEBook:
                ResID = eRES_IMAGE_TOP_EBOOK;
                break;
#if(USE_ALARM_CLOCK)                
            case eAlarmSet:
                ResID= IMAGE_ALARM_SET;
                break;
#endif              
            case eSysSet:
                ResID = eRES_IMAGE_TOP_SETUP;
                break;  
            case eImage:
                ResID = eRES_IMAGE_TOP_PHOTO;
                break;
            case eGame:
                ResID = eRES_IMAGE_TOP_GAME;
                break;
            case eCustomAdd:
                ResID= eRES_IMAGE_TOP_MP3;
                break;  
            default:
                ResID= eRES_IMAGE_PUB_TOPBAR_BK;
                break;
        }
    }*/
#endif
    return ResID;
}
#endif
#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
T_U16 GetUITitleStrID(T_VOID)//T_ePlayingType type ,T_AUD_LISTSTLE style)
{
    T_U16   ResID = INVALID_STRINGRES_ID;

    if(IsAudplayer())
    {
        switch(Aud_AudGetListStyle())    
        {
            case eAUD_MUSIC:
                ResID = eRES_STR_AUD_PLAY;
                break;
            case eAUD_VOICE:
                ResID = eRES_STR_VOICE_PLAYER;
                break;
            default:
                ResID = INVALID_STRINGRES_ID;
                break;
        }
    }

    
    if(INVALID_STRINGRES_ID == ResID)
    {
        if(!Record_IsNULL())
            ResID = eRES_STR_AUDIO_RECORD;
        else if(IsInRadio())
            ResID = eRES_STR_FM_PLAYER;
        else if(IsInEbk())
            ResID = eRES_STR_MAIN_EBOOK;
#if(USE_ALARM_CLOCK)
        else if(IsInAlarmSet())
            ResID = eRES_STR_ALARM_SETTING;
#endif
        else if(IsInSysSet())
            ResID = eRES_STR_SYS_SETTING;
        else if(IsInImage())
            ResID = eRES_STR_IMAGE_VIEWER;
        else
            ResID = eRES_STR_INVALID0;
    }
AK_DEBUG_OUTPUT("================\n");
AK_DEBUG_OUTPUT("strID:%d\n",ResID);
    return ResID;
}
#endif
#pragma arm section code

#endif
