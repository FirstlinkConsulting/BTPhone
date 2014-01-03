/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_disp_version.c
 * @BRIEF display current verison of software and hardware 
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-21
 * @Version£º
**************************************************************************/
#include "Apl_Public.h"
#include "m_event.h"
#include "Eng_Font.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "Fwl_Keypad.h"
#include "Ctrl_Dialog.h"
#include "Fwl_osFS.h"
#include "Fwl_osMalloc.h"
#include "Eng_Profile.h"
#include "Gbl_Global.h"
#include "M_event_api.h"

#if(NO_DISPLAY == 0)

#define  SYS_VER_DISP_REFRESH   0X00000001

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_VERSION_TITLE_HIGHT    32   //title hight
#define SYS_VERSION_TITLE_WIDTH    128
#define SYS_VERSION_TITLE_LEFT     0
#define SYS_VERSION_TITLE_TOP      0
#endif



typedef struct
{
    CDialogCtrl   dlg; 
    T_U32         refresh;
}T_SET_DISP_VISION;
static T_SET_DISP_VISION* pDispVision = AK_NULL;
extern const T_CONFIG_INFO gb_sysconfig;

void initset_disp_version(void)
{
    T_U8 TempString[MAX_FILE_LEN] = {0};
    T_U16 TempUString[MAX_FILE_LEN] = {0};
    
    pDispVision = (T_SET_DISP_VISION*)Fwl_Malloc(sizeof(T_SET_DISP_VISION));
    if (AK_NULL == pDispVision)
    {
        return;
    }
    pDispVision->refresh = -1;

    
#if (USE_COLOR_LCD)
    Utl_StrCat(TempString, gb_sysconfig.release_time);
    Utl_StrCat(TempString, "\n");
    Utl_StrCat(TempString, gb_sysconfig.fireware_version);
    Utl_StrCat(TempString, "\n");
    Utl_StrCat(TempString, AK_VERSION_HARDWARE);
    Utl_StrCat(TempString, "\n");
    Utl_StrCat(TempString,AK_VERSION_HARDWARE_DATE);
#else
    Utl_StrCpyN(TempString, gb_sysconfig.fireware_version, Utl_StrLen(gb_sysconfig.fireware_version));
    Utl_StrCat(TempString, "\n");
    Utl_StrCat(TempString,AK_VERSION_HARDWARE_DATE);  
    
#endif
    Eng_MultiByteToWideChar(TempString, Utl_StrLen(TempString), TempUString, MAX_FILE_LEN, AK_NULL);
#if (USE_COLOR_LCD)
    //Dialog_Init(&pDispVision->dlg, TempUString, CTRL_DIALOG_RESPONSE_HIDDEN, eRES_STR_RADIO_EMPTY_STR, INVALID_IMAGERES_ID, CTRL_DIALOG_RESPONSE_YES);
    Dialog_InitEx(&pDispVision->dlg, 
                    TempUString, 
                    CTRL_DIALOG_RESPONSE_HIDDEN, 
                    eRES_STR_RADIO_EMPTY_STR, 
                    INVALID_IMAGERES_ID, 
                    CTRL_DIALOG_RESPONSE_YES,
                    SYS_VERSION_TITLE_LEFT,
                    SYS_VERSION_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_VERSION_TITLE_HIGHT);
    #if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pDispVision->dlg.dialog_topbar, eRES_STR_VERSION);
    #endif  
    //Dialog_SetTitle(&pDispVision->dlg,eRES_IMAGE_TOP_SETUP);
#else
    Dialog_Init(&pDispVision->dlg, TempUString, CTRL_DIALOG_RESPONSE_HIDDEN, INVALID_STRINGRES_ID, eRES_IMAGE_VERSIONICON, CTRL_DIALOG_RESPONSE_YES);
    //Eng_ImageResDisp(8, 0, eRES_IMAGE_VERSIONICON, AK_FALSE);
#endif
     
}

void exitset_disp_version(void)
{
    if (AK_NULL != pDispVision)
    {
        if(pDispVision->refresh & SYS_VER_DISP_REFRESH)
        {
            #if (USE_COLOR_LCD)
            #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
            Eng_ImageResDisp(SYS_VERSION_TITLE_LEFT, (T_POS)SYS_VERSION_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
            #endif
            #endif
            pDispVision->refresh &= ~SYS_VER_DISP_REFRESH;
        }
        Dialog_Free(&pDispVision->dlg);
        pDispVision = Fwl_Free(pDispVision);
    }
}

void paintset_disp_version(void)
{
    if (AK_NULL != pDispVision)
    {
        Dialog_Show(&pDispVision->dlg);
        #if (!USE_COLOR_LCD)
        Eng_ImageResDisp(8, 0, eRES_IMAGE_VERSIONICON, AK_FALSE);
        #endif
    }
}

unsigned char handleset_disp_version(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret =0;
    
    ret = Dialog_Handler(&pDispVision->dlg,event, pEventParm);
    
    switch(ret)
    {   //return from current state
        case CTRL_RESPONSE_QUIT:
        case CTRL_DIALOG_RESPONSE_YES: 
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            break;     
        default:    //defalut do nothing
            break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_disp_version(void)
{
}
void paintset_disp_version(void)
{
}
void exitset_disp_version(void)
{
}
unsigned char handleset_disp_version(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_disp_version\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif


