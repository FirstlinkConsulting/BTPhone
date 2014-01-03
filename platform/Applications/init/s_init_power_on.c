#include "Apl_Public.h"
#include "Gbl_ImageRes.h"
#include "Eng_ImageResDisp.h"
#include "Fwl_Timer.h"
#include "Fwl_Keypad.h"
#include "Eng_USB.h"
#include "Eng_AutoOff.h"
#include "Eng_Profile.h"
#include "Fwl_osFS.h"
#include "Gbl_Global.h"
#include "Fwl_osMalloc.h"
#include "Log_aud_control.h"
#include "Alarm_Common.h"
#include "log_radio_core.h"

#if(NO_DISPLAY == 0)
#if (USE_COLOR_LCD)
#define MAX_BOOT_PIC    9
#if (STORAGE_USED != SPI_FLASH)
static const T_RES_IMAGE boot_img[MAX_BOOT_PIC] = {
    eRES_IMAGE_POWERON01, eRES_IMAGE_POWERON02, eRES_IMAGE_POWERON03, eRES_IMAGE_POWERON04,
    eRES_IMAGE_POWERON05, eRES_IMAGE_POWERON06, eRES_IMAGE_POWERON07, eRES_IMAGE_POWERON08,
    eRES_IMAGE_POWERON09
};
#endif
#else
#define MAX_BOOT_PIC    17
static const T_RES_IMAGE boot_img[MAX_BOOT_PIC] = {
    eRES_IMAGE_POWERON01, eRES_IMAGE_POWERON02, eRES_IMAGE_POWERON03, eRES_IMAGE_POWERON04,
    eRES_IMAGE_POWERON05, eRES_IMAGE_POWERON06, eRES_IMAGE_POWERON07, eRES_IMAGE_POWERON08,
    eRES_IMAGE_POWERON09, eRES_IMAGE_POWERON10, eRES_IMAGE_POWERON11, eRES_IMAGE_POWERON12,
    eRES_IMAGE_POWERON13, eRES_IMAGE_POWERON14, eRES_IMAGE_POWERON15, eRES_IMAGE_POWERON16,
    eRES_IMAGE_POWERON17
};
#endif

typedef struct _PowerOn
{
    T_U8            show_step;
    T_TIMER         timer_id;
    T_EVT_CODE      type;
}T_POWERON;

static T_POWERON *pPowerOn = AK_NULL;
#endif

static T_U8 firstly_poweron = AK_TRUE;


#ifdef SUPPORT_SDCARD
extern T_BOOL IsUseSD(T_MEM_DEV_ID index);

static T_BOOL PowerOn_CheckSDisPullOut(T_VOID)
{
    if ((!Fwl_MemDevIsMount(MMC_SD_CARD))&& IsUseSD(MMC_SD_CARD))
    {
        AK_DEBUG_OUTPUT("check sd pull out.");
        VME_EvtQueuePut(M_EVT_RETURN_ROOT, AK_NULL);
        return AK_TRUE;
    }
    return AK_FALSE;
}
#endif

void initinit_power_on(void)
{
    AK_DEBUG_OUTPUT("initinit_power_on\n");
#if(NO_DISPLAY == 0)
    pPowerOn = (T_POWERON*)Fwl_Malloc(sizeof(T_POWERON));
    pPowerOn->show_step = 0;
    pPowerOn->type = M_EVT_1;
    if (firstly_poweron)
        pPowerOn->timer_id = Fwl_TimerStartMilliSecond(150, AK_TRUE);
    else
    {
        if (Fwl_DetectorGetStatus(DEVICE_CHG))
            pPowerOn->timer_id = ERROR_TIMER;
        else
            pPowerOn->timer_id = Fwl_TimerStartMilliSecond(150, AK_TRUE);
    }
#endif
    GblReadCmpSysTime();
}

void exitinit_power_on(void)
{
#if(NO_DISPLAY == 0)
    if (pPowerOn->timer_id!=ERROR_TIMER)
        Fwl_TimerStop(pPowerOn->timer_id);
    pPowerOn = Fwl_Free(pPowerOn);
    pPowerOn = AK_NULL;

    AutoBgLightOffSet(gb.BgLightTime);
    AutoPowerOffCountSet(gb.PoffTime);
    AutoPOffCountSetSleep(gb.PoffTimeSleepMode,AK_TRUE);

#endif
    gb.init = SYSTEM_STATE_NORMAL;

    if (gb.usbstate)
    {
        VME_EvtQueuePut(M_EVT_USB_IN, AK_NULL);
        gb.usbstate = AK_FALSE;
    }

    firstly_poweron = AK_FALSE;
}

void paintinit_power_on(void)
{
}

#if (NO_DISPLAY == 0)
unsigned char handleinit_power_on(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_PRESS_KEY keyPad;
    T_SYSTEM_CFG syscfg;

    switch (event)
    {
    case M_EVT_ALARM_PLAY:
    case M_EVT_RESTART:
        AK_DEBUG_OUTPUT("M_EVT_RESTART:%x\n", event);
        pPowerOn->type = event;
        Fwl_DisplayInit();
        Profile_ReadData(eCFG_SYSTEM, (T_U8 *)&syscfg);
        Fwl_SetContrast((T_U8)(syscfg.LcdContrast)); // set contrast  
        AK_DEBUG_OUTPUT("power on check PowerOffFlag %d\n", syscfg.PowerOffFlag);
        if(POWEROFF_ABNORMAL != syscfg.PowerOffFlag)
        {
            syscfg.PowerOffFlag = POWEROFF_ABNORMAL;
            Profile_WriteData(eCFG_SYSTEM, &syscfg);
            // FlushUserdata();
        }
      
        Fwl_FillRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT, CLR_BLACK);
        Fwl_DelayUs(150000);
        
        if(M_EVT_ALARM_PLAY == event)
        {
            T_EVT_PARAM evtParam;

            evtParam.w.Param1 = M_EVT_RESTART;
            VME_EvtQueuePut(M_EVT_RESTART, &evtParam);
            VME_EvtQueuePut(M_EVT_Z01_MUSIC_PLAY,pEventParm);
        }

#ifdef SUPPORT_SDCARD
        #ifdef OS_ANYKA
        {
            if(Fwl_DetectorGetStatus(DEVICE_SD))
            {
                Fwl_MemDevUnMount(MMC_SD_CARD);
                Fwl_MemDevMount(MMC_SD_CARD);
            }
            else
            {
                Fwl_MemDevUnMount(MMC_SD_CARD);
            }
        }
        #endif
#endif
        break;
    case VME_EVT_TIMER:
        if (pPowerOn && pPowerOn->timer_id == (T_TIMER)pEventParm->w.Param1)
        {
        #if (STORAGE_USED == SPI_FLASH)
            pPowerOn->show_step = MAX_BOOT_PIC;
        #else
            if (pPowerOn->show_step < MAX_BOOT_PIC)
                Eng_ImageResDisp(0, BOOT_PIC_POS_Y, boot_img[pPowerOn->show_step], AK_FALSE);
        #endif

            if (pPowerOn->show_step >= MAX_BOOT_PIC)
            {
                if (M_EVT_RESTART == pPowerOn->type)
                {
                #ifdef SUPPORT_SDCARD
                    if (PowerOn_CheckSDisPullOut())
                    {
                        return 0;
                    }
                #endif

                    pEventParm->w.Param1 = M_EVT_RESTART;

                    VME_EvtQueuePutUnique(M_EVT_RESTART, pEventParm, AK_FALSE);
                }
                else if (firstly_poweron)
                {
                    VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
                    firstly_poweron = AK_FALSE;
                }
            }
            pPowerOn->show_step++;
        }
        break;
    case M_EVT_USER_KEY: 
        keyPad.id = pEventParm->c.Param1;
        keyPad.pressType = pEventParm->c.Param2;

        if (PRESS_SHORT == keyPad.pressType && kbOK == keyPad.id)
        {
            if (firstly_poweron)
            {
            //开机过程不对OK键进行处理
            /*
                         if (USB_GetCnctState() == USB_CABLE_NOT_CONNECT)
                         {
                                USB_SetCnctState(usb_charger_detect_init());
                
                                if(USB_GetCnctState() != USB_CABLE_NOT_CONNECT)
                             {
                                    Fwl_LCD_lock(AK_TRUE);
                                }
                            }
                            VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
                           firstly_poweron = AK_FALSE;
                     */
            }
            else if (M_EVT_RESTART == pPowerOn->type)
            {
                #ifdef SUPPORT_SDCARD
                if (PowerOn_CheckSDisPullOut())
                {
                    return 0;
                }
                #endif
                pEventParm->w.Param1 = M_EVT_RESTART;
                VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
            }
            else
            {
                VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
            }
        }
        break;
    case M_EVT_Z01_MUSIC_PLAY:
        if (M_EVT_RESTART == pPowerOn->type)
          {
                #ifdef SUPPORT_SDCARD
                if (PowerOn_CheckSDisPullOut())
                {
                    return 0;
                }
                #endif

                if (!USB_Init())
                {
                    if (IsAudplayer() || IsInRadio() || IsInVoicePlay())
                    {
                        AK_DEBUG_OUTPUT("*enter audio no usb\n"); 
                        //Aud_PlayerOpenDA();
                        Aud_AudCtrlSetStatJump(JUMP_SWITCH_OFF);
                    }
                    Fwl_LCD_lock(AK_FALSE);
                    VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
                    VME_EvtQueuePut(M_EVT_Z01_MUSIC_PLAY, pEventParm);
                }
            }
            else
            {
                VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
                VME_EvtQueuePut(M_EVT_Z01_MUSIC_PLAY, pEventParm);
            }
       break;
    default:
        if (pPowerOn->timer_id == ERROR_TIMER)
        {
            if (M_EVT_RESTART == pPowerOn->type)
            {
                #ifdef SUPPORT_SDCARD
                if (PowerOn_CheckSDisPullOut())
                {
                    return 0;
                }
                #endif

                if (!USB_Init())
                {
                    if (IsAudplayer() || IsInRadio() || IsInVoicePlay())
                    {
                        AK_DEBUG_OUTPUT("*enter audio no usb\n"); 
                        //Aud_PlayerOpenDA();
                        Aud_AudCtrlSetStatJump(JUMP_SWITCH_OFF);
                    }
                    Fwl_LCD_lock(AK_FALSE);
                    VME_EvtQueuePut(M_EVT_RESTART, pEventParm);
                }
            }
            else
            {
                VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
            }
        }
        break;
    }

    return 0;
}
#else
unsigned char handleinit_power_on(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    switch (event)
    {     
    case M_EVT_RESTART:
        AK_DEBUG_OUTPUT("M_EVT_RESTART:%x\n", event);
    #ifdef SUPPORT_SDCARD
    #ifdef OS_ANYKA
        {
            Fwl_MemDevUnMount(MMC_SD_CARD);
            if(Fwl_DetectorGetStatus(DEVICE_SD))
            {
                Fwl_MemDevMount(MMC_SD_CARD);
            }
        }
        if (PowerOn_CheckSDisPullOut())
        {
            return 0;
        }
    #endif
    #endif
        pEventParm->w.Param1 = M_EVT_RESTART;
        VME_EvtQueuePutUnique(M_EVT_RESTART, pEventParm, AK_FALSE);
        break;
        
    case M_EVT_1:           
        if (firstly_poweron)
        {
            VME_EvtQueuePut(M_EVT_NEXT, pEventParm);
            firstly_poweron = AK_FALSE;
        }
        break;
            
    default:
        break;
    }
    return 0;
}
#endif
