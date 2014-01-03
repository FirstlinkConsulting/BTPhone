/*******************************************************************************
 * @file    Fwl_Detect.c
 * @brief   This file provides detector APIs: initialization, enable and so on
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @version 1.0
*******************************************************************************/
#include "Fwl_Detect.h"
#include "hal_detector.h"
#include "gpio_define.h"
#include "hal_int_msg.h"
#include "vme.h"
#include "M_event.h"
#include "Eng_Debug.h"
#include "arch_gpio.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Timer.h"
#include "Fwl_PMU.h"
#include "Eng_LedHint.h"
#include "Apl_public.h"
#include "hal_usb_h_disk.h"
#include "hal_usb_s_state.h"
#include "hal_usb_s_state.h"
#include "Fwl_usb_host_mount.h"
#include "Fwl_System.h"
#include "Gbl_powerval.h"
#include "arch_pmu.h"
#include "prog_manager.h"

volatile T_BOOL s_usb_conn_pc = AK_FALSE;

static volatile T_BOOL gs_UsbHostCnntIsOk = AK_FALSE;   //是否有U盘插入标志

#ifdef SUPPORT_USBHOST
static volatile T_BOOL gs_UsbHostInitIsOk = AK_FALSE;   //usb host 初始化标志

extern T_VOID usbbus_proc(T_VOID);


/*******************************************************************************
 * @brief   usb pop in callback
 * @author  mayeyu
 * @date    2011-7-27
 * @param   [in]lun_ready
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_UsbDisk_POP_IN(T_U16 lun_ready)
{
    if (AK_TRUE == gs_UsbHostInitIsOk)
    {
        gs_UsbHostCnntIsOk = AK_TRUE;       //连接成功
    }
}
extern T_VOID usb_switch(T_BOOL bswitch);

/*******************************************************************************
 * @brief   switch usb between UHost and USlave
 * @author  
 * @date    2013-5-20
 * @param   [in]bUhost:AK_TRUE->uhost, AK_FALSE->uslave
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_USBSwitch(T_BOOL bUhost)
{
    usb_switch(bUhost);
    /*gpio_set_pin_dir(GPIO_USB_SWITCH, GPIO_DIR_OUTPUT);
    if (AK_FALSE == bUhost)
    {
        gpio_set_pin_level(GPIO_USB_SWITCH, GPIO_LEVEL_HIGH);
    }
    else
    {
        gpio_set_pin_level(GPIO_USB_SWITCH, GPIO_LEVEL_LOW);
    }*/
}

/*******************************************************************************
 * @brief   enable usb disk function.
 * @author  mayeyu
 * @date    2011-7-27
 * @param   [in]bEnable:
 * @return  T_BOOL:success return AK_TRUE,fail return AK_FALSE
*******************************************************************************/
static T_BOOL Fwl_UsbHostDetectEnable(T_BOOL bEnable)
{
    if (AK_TRUE == bEnable)
    {
        gs_UsbHostInitIsOk = AK_FALSE;
        gs_UsbHostCnntIsOk = AK_FALSE;
        
        //设置U盘插入和拔出回调，注意中断回调未实现。
        udisk_host_set_callback(Fwl_UsbDisk_POP_IN, AK_NULL);
        
        Fwl_USBSwitch(AK_TRUE);
        udisk_host_init(USB_MODE_20);
        gs_UsbHostInitIsOk = AK_TRUE;       //usb host 初始成功

        //progmanage_abort_checkirq(AK_FALSE);
        akerror("udisk_host_init success",0,1);
    }
    else
    {
        gs_UsbHostCnntIsOk = AK_FALSE;
        
        if (AK_TRUE == Fwl_UsbDiskIsMnt())  //拔出U盘，要关USB HOST
        {
            akerror("unmount usb disk!", 0, 1);
            Fwl_UnMountUSBDisk();
        }

        if (AK_TRUE == gs_UsbHostInitIsOk)
        {
            gs_UsbHostInitIsOk = AK_FALSE;
            akerror("close usb host!", 0, 1);
            udisk_host_close();
        }
        Fwl_USBSwitch(AK_FALSE);
        //progmanage_abort_checkirq(AK_TRUE);
        akerror("DisconnectDisk ok", 0, 1);
    }
    
    return AK_TRUE;
}

/*******************************************************************************
 * @brief   deal uhost connect message
 * @author  liuhuadong
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_UsbHostMsgDeal(T_BOOL bStatus)
{
    T_U32 cnt = 0;
    T_EVT_PARAM pEventParm;

    if (bStatus)
    {
        Fwl_FreqPush(FREQ_APP_USB_MUSIC);
        while (AK_FALSE == gs_UsbHostCnntIsOk)  //判断USBHOST初始是否成功
        {
            usbbus_proc();                  //探测是否有U盘插入
                                            //只有加了这个处理函数才能进行U盘枚举
            Fwl_DelayUs(1000);
            cnt++;
            if (cnt > 5)                    //对U盘最多进行5次探测
            {
                AK_DEBUG_OUTPUT("no detect udisk\n");
                Fwl_FreqPop();
                return;
            }
            akerror(".", 0, 0);
        }
        AK_DEBUG_OUTPUT("detect udisk ok\n");
        Fwl_FreqPop();
    }
    else
    {
        Fwl_UsbHostDetectEnable(AK_FALSE);
        Fwl_UsbHostDetectEnable(AK_TRUE);
    }

    pEventParm.c.Param1 = 0;
    pEventParm.c.Param2 = bStatus;
    VME_EvtQueuePut(M_EVT_USBHOSTDISK, &pEventParm);
}
#endif

/*******************************************************************************
 * @brief   initialize detector
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]mode: 
 * @return  T_BOOL
 * @retval  success or fail
*******************************************************************************/
T_BOOL Fwl_DetectorInit(T_U8 mode)
{
    T_BOOL ret = AK_FALSE;
    
    detector_init();

    if ((DEVICE_USB & mode) || (DEVICE_CHG & mode))
    {
        detector_register_gpio(DEVICE_USB, GPIO_USB_DET, LEVEL_HIGH, AK_TRUE, 0);
        detector_set_callback(DEVICE_USB, AK_NULL);
        ret = AK_TRUE;
    }

#ifdef OS_ANYKA
#ifdef SUPPORT_SDCARD
    if (DEVICE_SD & mode)
    {
        detector_register_gpio(DEVICE_SD, GPIO_SD_DET, LEVEL_LOW, AK_TRUE, 0);
        detector_set_callback(DEVICE_SD, AK_NULL);
		gpio_set_pullup_pulldown(GPIO_SD_DET, 1);
        ret = AK_TRUE;
    }
#endif
#endif

    if (DEVICE_HP & mode)
    {
        detector_register_gpio(DEVICE_HP, GPIO_HP_DET, LEVEL_LOW, AK_TRUE, 0);
        detector_set_callback(DEVICE_HP, AK_NULL);
        gpio_set_pullup_pulldown(GPIO_HP_DET, AK_TRUE);
        ret = AK_TRUE;
    }

#if SUPPORT_LINEIN_SPK
    if (DEVICE_LINEIN & mode)
    {
        detector_register_gpio(DEVICE_LINEIN, GPIO_LINEIN_DET, LEVEL_LOW, AK_TRUE, 0);
        detector_set_callback(DEVICE_LINEIN, AK_NULL);
        Fwl_DetectorEnable(DEVICE_LINEIN, AK_TRUE);
        gpio_set_pullup_pulldown(GPIO_LINEIN_DET, AK_TRUE);
        ret = AK_TRUE;
    }
#endif

#ifdef SUPPORT_USBHOST
    if (DEVICE_UHOST & mode)
    {
        Fwl_UsbHostDetectEnable(AK_TRUE);
        ret = AK_TRUE;
    }
#endif

    return ret;
}

#pragma arm section code = "_wave_out_"

/*******************************************************************************
 * @brief   enable or disable the detector of the specified device
 * @author  wangguotian
 * @date    2012-03-09
 * @param   [in]devId: ID of the device detected
 * @param   [in]bEnable: enable or disable
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 * @remark  It's not suggested to disable the detector,if the detector
 *          is disable, the connecting state of the device can't be 
 *          informed the user.
*******************************************************************************/
T_BOOL Fwl_DetectorEnable(T_eDEVICE_ID devId, T_BOOL bEnable)
{
    T_BOOL ret = AK_FALSE;

    if (DEVICE_UHOST == devId)
    {
        #ifdef SUPPORT_USBHOST
        ret = Fwl_UsbHostDetectEnable(bEnable);
        #endif
    }
    else
    {
        if (DEVICE_CHG == devId)
        {
            devId = DEVICE_USB;
        }

        ret = detector_enable(devId, bEnable);
    }

    return ret;
}
#pragma arm section code


#pragma arm section code = "_frequentcode_"
/*******************************************************************************
 * @brief   get the status of the specified device
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]devId: ID of the device detected
 * @return  T_BOOL
 * @retval  connect or not
*******************************************************************************/
T_BOOL Fwl_DetectorGetStatus(T_eDEVICE_ID devId)
{
    T_BOOL       bStatus = AK_FALSE;
    T_eDEVICE_ID devRealId;

    if (DEVICE_UHOST == devId)
    {
        return gs_UsbHostCnntIsOk;
    }

    if (DEVICE_CHG == devId)
    {
        devRealId = DEVICE_USB;
    }
    else
    {
        devRealId = devId;
    }

    if (!detector_get_state(devRealId, &bStatus))
    {
        return AK_FALSE;
    }

    if ((DEVICE_USB == devId) && (AK_TRUE == bStatus))
    {
        return s_usb_conn_pc;//return usb_detect();
    }
    return bStatus;
}
#pragma arm section code
#pragma arm section code = "_bootcode1_"


/*******************************************************************************
 * @brief   set LoudSpeaker connect
 * @author  
 * @date    
 * @param   [in]bIsConnect: connect or not
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SpkConnectSet(T_BOOL bIsConnect)
{
    if (bIsConnect)
    {
        gpio_set_pin_dir(GPIO_SPEAKER_CONTROL, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_SPEAKER_CONTROL, LEVEL_HIGH);
    }
    else
    {
        gpio_set_pin_dir(GPIO_SPEAKER_CONTROL, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(GPIO_SPEAKER_CONTROL, LEVEL_LOW);
    }
}
#pragma arm section code

/*******************************************************************************
 * @brief   deal USB detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @param   [in]isCntPC: if connect, PC or Adapter
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_USBMsgDeal(T_BOOL bStatus)
{
    if (bStatus)
    {
    #ifdef OS_ANYKA
        Fwl_PMU_USBPushIn();
        /*#ifdef SUPPORT_LEDHINT
        LedHint_Exec(LED_NORMAL_CHARGE);
        #endif*/
    #endif
        if (AK_TRUE == gs_UsbHostCnntIsOk)
        {
            s_usb_conn_pc = AK_FALSE;
        }
        else
        {
            Fwl_DetectorEnable(DEVICE_UHOST, AK_FALSE);
            usb_slave_detect_start(AK_NULL);
        }

        //充电器插入，打开充电功能
        pmu_charge_enable(AK_TRUE);
        
        //disable watch dog
        Fwl_DisableWatchdog();

        AK_DEBUG_OUTPUT("USB CABLE IN\n");
    }
    else
    {
    #ifdef OS_ANYKA
        Fwl_PMU_USBPullOut();
        /*#ifdef SUPPORT_LEDHINT
        LedHint_Stop(LED_NORMAL_CHARGE);
        #endif*/
    #endif
        s_usb_conn_pc = AK_FALSE;
        VME_EvtQueuePut(M_EVT_USB_OUT, AK_NULL);

        //充电器拔出，关闭充电功能
        pmu_charge_enable(AK_FALSE);
        //enable watch dog
        Fwl_EnableWatchdog();

        AK_DEBUG_OUTPUT("USB CABLE OUT\n");
    }
}


/*******************************************************************************
 * @brief   deal SDMMC detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @param   [in]bSDMMC1: SDMMC1 or SDMMC2
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_SDMsgDeal(T_BOOL bStatus)
{
    T_EVT_PARAM pEventParm;
    
    pEventParm.c.Param1 = MMC_SD_CARD;
    pEventParm.c.Param2 = bStatus;
    VME_EvtQueuePut(M_EVT_SDCARD, &pEventParm);
    if (bStatus)
        AK_DEBUG_OUTPUT("SD IN\n");
    else
        AK_DEBUG_OUTPUT("SD OUT\n");
}


/*******************************************************************************
 * @brief   deal headphone detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_HPMsgDeal(T_BOOL bStatus)
{
    if (bStatus)
    {
        if (Fwl_DetectorGetStatus(DEVICE_HP))
        {
            AK_DEBUG_OUTPUT("HP IN\n");
            Fwl_SpkConnectSet(AK_FALSE);
        }
    }
    else
    {
        if (!Fwl_DetectorGetStatus(DEVICE_HP))
        {
            AK_DEBUG_OUTPUT("HP OUT\n");
            Fwl_SpkConnectSet(AK_TRUE);
        }
    }
}

/*******************************************************************************
 * @brief   deal line in detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_LINEINMsgDeal(T_BOOL bStatus)
{
    if (bStatus)
    {
        if (Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            VME_EvtQueuePut(M_EVT_LINE_IN, AK_NULL);
            AK_DEBUG_OUTPUT("LINE_IN IN\n");
        }
    }
    else
    {
        if (!Fwl_DetectorGetStatus(DEVICE_LINEIN))
        {
            VME_EvtQueuePut(M_EVT_LINE_IN, AK_NULL);
            AK_DEBUG_OUTPUT("LINE_IN OUT\n");
        }
    }
}

/*******************************************************************************
 * @brief   deal device detect message
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   [in]bStatus: connect or not
 * @param   [in]devInfo: devId and other information
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_DetectorMsgDeal(T_BOOL bStatus, T_U16 devInfo)
{
    switch (devInfo)
    {
    case DEVICE_USB:
        Fwl_USBMsgDeal(bStatus);
        break;

    case DEVICE_HP:
        Fwl_HPMsgDeal(bStatus);
        break;

    case DEVICE_LINEIN:
        Fwl_LINEINMsgDeal(bStatus);
        break;

    case DEVICE_SD:
        Fwl_SDMsgDeal(bStatus);
        break;

    default:
        break;
    }
}


