/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME s_set_disp_contrast.c
 * @BRIEF set disp contrast 
 * @Author£ºHuang_ChuSheng
 * @Date£º2008-04-21
 * @Version£º
**************************************************************************/

#include "m_event.h"
#include "Gbl_resource.h"
#include "gbl_global.h"
#include "Eng_Profile.h"
#include "Ctrl_Progress.h"
#include "Gui_Common.h"
#include "M_Event_Api.h"

#if(NO_DISPLAY == 0)

#if (!USE_COLOR_LCD)    //for BW LCD

#else
#define SYS_CONTRAST_SET_TITLE_HIGHT    32   //title hight
#define SYS_CONTRAST_SET_TITLE_WIDTH    128
#define SYS_CONTRAST_SET_TITLE_LEFT     0
#define SYS_CONTRAST_SET_TITLE_TOP      0

#endif


typedef struct _SET_DISP_CONTRAST
{
    T_SYSTEM_CFG  sysCfg;
    CProgressCtrl prgress;
}T_SET_DISP_CONTRAST;

#define LCD_CONTRAST_MIN_VALUE 0

#if(USE_COLOR_LCD)
#define LCD_CONTRAST_MAX_VALUE 10
#else
#define LCD_CONTRAST_MAX_VALUE 16
#endif

static T_SET_DISP_CONTRAST *pSetContrast = AK_NULL;

void initset_disp_contrast(void)
{
    pSetContrast = (T_SET_DISP_CONTRAST*)Fwl_Malloc(sizeof(T_SET_DISP_CONTRAST));
    AK_ASSERT_VAL_VOID(pSetContrast, "alloc T_SET_DISP_CONTRAST structure error!");

    Profile_ReadData(eCFG_SYSTEM, (T_VOID*)&pSetContrast->sysCfg);

    if (/*pSetContrast->sysCfg.LcdContrast<LCD_CONTRAST_MIN_VALUE ||*/ pSetContrast->sysCfg.LcdContrast>LCD_CONTRAST_MAX_VALUE)
    {
        Profile_CheckData(eCFG_SYSTEM, &pSetContrast->sysCfg, AK_FALSE);
    }
#if (USE_COLOR_LCD)
    //Progress_Init(&pSetContrast->prgress, LCD_CONTRAST_MIN_VALUE, LCD_CONTRAST_MAX_VALUE, eRES_STR_PUB_DEL_HINT, 
    //            INVALID_IMAGERES_ID, 20, pSetContrast->sysCfg.LcdContrast, AK_FALSE, INVALID_IMAGERES_ID, INVALID_IMAGERES_ID);
    #if ((1 != LCD_HORIZONTAL)||(3 != LCD_TYPE))
    Eng_ImageResDisp(SYS_CONTRAST_SET_TITLE_LEFT, (T_POS)SYS_CONTRAST_SET_TITLE_TOP, eRES_IMAGE_TOP_SETUP, AK_FALSE);
    #endif
    Progress_InitEx(&pSetContrast->prgress, 
                    LCD_CONTRAST_MIN_VALUE, 
                    LCD_CONTRAST_MAX_VALUE, 
                    eRES_STR_PUB_DEL_HINT, 
                    INVALID_IMAGERES_ID, 
                    (LCD_CONTRAST_MAX_VALUE- LCD_CONTRAST_MIN_VALUE), 
                    pSetContrast->sysCfg.LcdContrast, 
                    AK_FALSE, 
                    INVALID_IMAGERES_ID, 
                    INVALID_IMAGERES_ID,
                    SYS_CONTRAST_SET_TITLE_LEFT,
                    SYS_CONTRAST_SET_TITLE_HIGHT,
                    MAIN_LCD_WIDTH,
                    MAIN_LCD_HEIGHT-SYS_CONTRAST_SET_TITLE_HIGHT);

#if ((1 == LCD_HORIZONTAL)&&(3 == LCD_TYPE))
    TopBar_ConfigTitleSet(&pSetContrast->prgress.pro_topbar, eRES_STR_LCD_CONTRAST);
#endif
    //Progress_SetTitle(&pSetContrast->prgress,GetUITitleResID());
#else
    Progress_Init(&pSetContrast->prgress, LCD_CONTRAST_MIN_VALUE, LCD_CONTRAST_MAX_VALUE, INVALID_STRINGRES_ID, \
                eRES_IMAGE_CONTRSTW, LCD_CONTRAST_MAX_VALUE, pSetContrast->sysCfg.LcdContrast, AK_FALSE, eRES_IMAGE_CONTRSTB, INVALID_IMAGERES_ID);
#endif
}

void exitset_disp_contrast(void)
{
    //Fwl_SetContrast((T_U8)(pSetContrast->sysCfg.LcdContrast));
    pSetContrast = Fwl_Free(pSetContrast);
}

void paintset_disp_contrast(void)
{
    Progress_Show(&pSetContrast->prgress);
}

unsigned char handleset_disp_contrast(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U16 ret;
    T_U8 value;
    
    ret = Progress_Handler(&pSetContrast->prgress, event, pEventParm);

    switch(ret)
    {
        case CTRL_RESPONSE_QUIT:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
        case CTRL_RESPONSE_NONE:
            break;
        case CTRL_PROGRESS_RESPONSE_CHANGED:
            value = (T_U8)Progress_GetPos(&pSetContrast->prgress);
            Fwl_SetContrast((T_U8)value);
            //printf("\n\n\nnew value:%d\t, old value%d \t\n\n\n",value,pSetContrast->sysCfg.LcdContrast);
            if (value != (T_U16)pSetContrast->sysCfg.LcdContrast)
            {
                //Fwl_SetContrast(pSetContrast->sysCfg.LcdContrast);
                pSetContrast->sysCfg.LcdContrast = (T_U8)value;
                Profile_WriteData(eCFG_SYSTEM, (T_VOID*)&pSetContrast->sysCfg);
            }               

            break;
        default:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
    }

    if (event >= M_EVT_Z00_POWEROFF)
        return 1;
    else
        return 0;
}

#else
void initset_disp_contrast(void)
{
}
void paintset_disp_contrast(void)
{
}
void exitset_disp_contrast(void)
{
}
unsigned char handleset_disp_contrast(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    AK_DEBUG_OUTPUT("error state:handleset_disp_contrast\n");
    m_triggerEvent(M_EVT_RETURN_ROOT, NULL);
    return 0;   
}

#endif

