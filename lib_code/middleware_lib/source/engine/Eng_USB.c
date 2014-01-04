/**
@author liu zhenwu
@date   2006.1.24
@file       Eng_USB.c
@brief  control USB
*/
#include "Apl_Public.h"
#include "Gbl_Global.h"
#include "Eng_Debug.h"
#include "Eng_USB.h"
#include "Eng_Time.h"
#include "Fwl_System.h"
#include "Fwl_Mount.h"
#include "Fwl_Timer.h"
#include "Fwl_Detect.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_usb_s_state.h"
#include "Fwl_usb_s_disk.h"
#include "Fwl_Led.h"
#include "Eng_LedHint.h"
#include "Eng_UStrPublic.h"
#include "Eng_BtCtrl.h"

#if (USE_COLOR_LCD)
#define USB_PIC_POS_X   0
#define USB_PIC_POS_Y   0
#if (LCD_TYPE == 3)
#if (1 == LCD_HORIZONTAL)
#define USB_ICON_POS_X  148
#define USB_ICON_POS_Y  104
#define BAT_ICON_POS_X  248
#define BAT_ICON_POS_Y  190

#else
#define USB_ICON_POS_X  110
#define USB_ICON_POS_Y  150
#define BAT_ICON_POS_X  180
#define BAT_ICON_POS_Y  260
#endif
#else
#define USB_ICON_POS_X  60
#define USB_ICON_POS_Y  75
#define BAT_ICON_POS_X  106
#define BAT_ICON_POS_Y  146
#endif
#else
#define USB_PIC_POS_X   0
#define USB_PIC_POS_Y   16
#define USB_ICON_POS_X  55
#define USB_ICON_POS_Y  26
#define BAT_ICON_POS_X  115
#define BAT_ICON_POS_Y  16
#endif

//按键处理
//#define exitKey             kbOK

#define DELAY_USB_CHECK     20000    //检测usb连接状态循环次数

/* define the usb buffer size */
#define MSC_BUFFER_SIZE             (32*1024)   /* each 16k bytes */
#define BGLIGHT_OFF_TIME            10

/* the name showed on PC */
#define MANUFACTURER_USB            "anyka"
#define PRODUCT_USB                 "usbdisk"

#if(NO_DISPLAY == 0)
const T_RES_IMAGE battery_img[BATTERY_STAT_NOR_FULL] = {
    eRES_IMAGE_BATVAL0, eRES_IMAGE_BATVAL1, eRES_IMAGE_BATVAL2, eRES_IMAGE_BATVAL3,
    eRES_IMAGE_BATVALFULL
};
#endif

static T_VOID USB_LCD_Switch(T_BOOL bTurnon);

/* @desp:  prepare for rom update: erase block 0 and reset
    @author: Liaozhijun
    @date: 2008-7-18
*/
extern T_VOID UpdateRom_Reset(T_VOID);
extern T_PMEDIUM FS_GetFakeMedium(T_U8 DriverID);
extern T_BOOL FS_UnMountMemDev(T_U8 DriverID);
extern T_VOID Start_PCTestBT(T_VOID);

T_VOID update_rom(T_VOID)
{
#ifdef OS_ANYKA
    T_S32 i=0;

    Fwl_FreqPush(FREQ_APP_AUDIO_L3);
    Fwl_Freq_Clr_Add();

    for (i=6; i>=0; i--)
    {
        Fwl_TimerStop(i);
    }
    Fwl_StoreAllInt();
    UpdateRom_Reset();
#endif
}


void usbFlushNandDisk()
{
    T_U8 i;
    T_PMEDIUM medium;
    
    for(i = 0; i < _gNandDriverCnt; i++)
    {
        medium = FS_GetFakeMedium((T_U8)(_gNandStartDriverID + i));
        medium->flush(medium);
    }
}


T_U16 Utl_StrLen(T_pCSTR strMain);
#ifdef SUPPORT_BLUETOOTH
static T_BTDEV_CFG *g_USB_Btdev_cfg;

T_BOOL test_bt_dev = AK_FALSE;
typedef enum _MSC_BO_STAGE
{
    MSC_STAGE_READY = 0,
    MSC_STAGE_COMMAND,
    MSC_STAGE_DATA_IN,
    MSC_STAGE_DATA_OUT,
    MSC_STAGE_STATUS
}T_eMSC_BO_STAGE;

struct{
    T_U8 adapter_addr[6];
    T_U8 dev_addr[6];
    T_BOOL auto_test_flag;
    T_BOOL btestmode;
}T_PASSTHROUGH_INFO;

extern T_BOOL g_test_mode;
T_U32 usb_scsi_ext_proc(T_U8 cmd,T_U8 *param,T_U32 szParam,T_U8 Stage)
{
    T_BOOL result = AK_TRUE;
    
    //#define MASS_SCSI_CMD_UPDATE_BT_NAME        (0x01)
    #define MASS_SCSI_CMD_UPDATE_TEST_INFO      (0x02)
    #define MASS_SCSI_CMD_CLEAN_FLAG            (0x03)
    #define MASS_SCSI_CMD_GET_BT_INFO           (0x04)
    
    switch(cmd)
    {
        case MASS_SCSI_CMD_UPDATE_TEST_INFO:
            if(g_USB_Btdev_cfg == AK_NULL)
            {
                akerror("Err:g_USB_Btdev_cfg",0,1);
                return ;
            }
            if(Stage == MSC_STAGE_COMMAND)
            {
                if((param[0]==0xff)&&(param[1]==0xff)&&(param[2]==0xff)&&
                    (param[3]==0xff)&&(param[4]==0xff)&&(param[5]==0xff))
                {
                    akerror("invalid dst Adapter addr",0,1);
                }
                else
                {
                    //更新蓝牙适配器地址
                    akerror("update adapter addr",0,1);
                    memcpy(&(g_USB_Btdev_cfg->pairedList[0].BD_ADDR),&param[0],6);
                }
                if((param[6]==0xff)&&(param[7]==0xff)&&(param[8]==0xff)&&
                    (param[9]==0xff)&&(param[10]==0xff)&&(param[11]==0xff))
                {
                    akerror("invalid device addr",0,1);
                }
                else
                {
                    //更新蓝牙设备地址
                    akerror("update device addr",0,1);
                    memcpy(&(g_USB_Btdev_cfg->localInfo.info.BD_ADDR),&param[6],6);
                }
                if(param[12]==0xff)
                {
                    //auto disconnect usb disk 
                    akerror("auto disconnect usb disk",0,1);
                    test_bt_dev = AK_TRUE;
                }
                else
                {
                    akerror("do not disconnect usb disk",0,1);
                }
                g_test_mode = param[13];
            }
            else if(Stage == MSC_STAGE_DATA_OUT)
            {
                akerror("update device name",0,1);
                memset(&(g_USB_Btdev_cfg->localInfo.info.name),0,32);
                memcpy(&(g_USB_Btdev_cfg->localInfo.info.name),&param[0],szParam);
            }
            else if((Stage == MSC_STAGE_STATUS))
            {
            }
            break;    
        case MASS_SCSI_CMD_GET_BT_INFO:
            //akerror("update get bt info",Stage,1);
            param[0]=0x2b;
            param[1]=0x4a;
            param[2]=0x7f;
            param[3]=0x1E;
            memcpy(&param[4],&(g_USB_Btdev_cfg->localInfo.info.BD_ADDR),6);
            memcpy(&param[10],(g_USB_Btdev_cfg->localInfo.info.name),Utl_StrLen(g_USB_Btdev_cfg->localInfo.info.name));
            szParam = 10+Utl_StrLen(g_USB_Btdev_cfg->localInfo.info.name);
            break;
        default:
            result = AK_FALSE;
            break;
    }
    return szParam;
}
#endif
T_BOOL usbdisk_main(T_VOID)
{
#ifdef OS_ANYKA
    T_U32 tick, timeout = 0;
    T_U32 transfer_tick;
    T_BOOL refresh=AK_TRUE;
    T_U8 state, cur_state, bglight = 0;
    T_U32 time_start = 0, time_end = 0;
    T_U32 time_diff = 0;
#ifdef SUPPORT_SDCARD
    T_U8 sd_statue;
    T_U8 i;
    T_PMEDIUM medium;

    
    
    Fwl_DelayUs(500000);

    sd_statue = Fwl_DetectorGetStatus(DEVICE_SD);
#endif
    
    Fwl_UsbDiskInit(USB_MODE_20);//only support CPU
#ifdef SUPPORT_BLUETOOTH    
    usb_set_ExtCmdCallbak(usb_scsi_ext_proc); 
    test_bt_dev = AK_FALSE;
    g_USB_Btdev_cfg = (T_BTDEV_CFG*)Fwl_Malloc(sizeof(T_BTDEV_CFG));
    Profile_ReadData(eCFG_BTDEV,g_USB_Btdev_cfg);
#endif    
    Fwl_UsbDiskSetStrDesc(STR_MANUFACTURER_EX,MANUFACTURER_USB);
    Fwl_UsbDiskSetStrDesc(STR_PRODUCT_EX,PRODUCT_USB);

#if (STORAGE_USED == NAND_FLASH)
    Fwl_UsbDiskAddLun(NANDFLASH_DISK);
#endif

#if (MOUNT_ALL_DRIVER)
    Fwl_UsbDiskAddLun(NANDRESERVE_ZONE);
#endif

#ifdef SUPPORT_SDCARD
    Fwl_UsbDiskAddLun(SDCARD_DISK);
#endif

    Fwl_Freq_Add2Calc(FREQ_VAL_1);

    //Fwl_StoreAllInt();//关闭其它中断
    //Fwl_KeypadEnable(AK_FALSE);
    Fwl_DetectorEnable(DEVICE_SD, AK_FALSE);
    Fwl_DetectorEnable(DEVICE_LINEIN, AK_FALSE);
    PublicTimerStop();

    AK_DEBUG_OUTPUT("start usb disk\r\n");

    //PublicTimerStop();
    //start udisk
    /*initialize usb read mode, because of usb read capacity is virtual, 
            but fs read capacity is true*/
    if(Fwl_UsbDiskStart())
    {
        cur_state = Fwl_UsbSlaveGetState();
        
        tick = 0;
        transfer_tick = 180000;

#if(NO_DISPLAY == 0)
#if (STORAGE_USED == SPI_FLASH)
        Eng_ImageResDisp(USB_PIC_POS_X, USB_PIC_POS_X, eRES_IMAGE_UDISKIDLE, AK_FALSE);
#else
        Eng_ImageResDisp(USB_PIC_POS_X, USB_PIC_POS_X, eRES_IMAGE_UDISKOK, AK_FALSE);
#endif
        Fwl_DisplayBacklightOn();
#endif

        time_start = Fwl_GetTickCountMs();

        while(1)
        {
            tick++;
            transfer_tick++;
            if(transfer_tick >= 0xefffffff)
            {
                transfer_tick = 0xefffffff;
            }

            if (Fwl_UsbDiskProc())
            {
                time_start = Fwl_GetTickCountMs();
                //uart_write_chr('&');
                #ifdef  SUPPORT_LEDHINT
                #ifndef  SUPPORT_RADIO_RDA5876
                Fwl_LEDOn(LED_BLUE);
                #endif
                #endif
            }
            #ifdef  SUPPORT_LEDHINT
            #ifndef  SUPPORT_RADIO_RDA5876
            if (tick%5 == 0)
            {
                 Fwl_LEDOff(LED_RED|LED_BLUE);
            }
            #endif
            #endif
            time_end = Fwl_GetTickCountMs();
            if (time_end >= time_start)
            {
                time_diff = time_end - time_start;
            }
            else
            {
                time_diff = T_U32_MAX - time_start + time_end;
            }
            if (time_diff > DELAY_USB_CHECK)
            {
                AK_PRINTK("Over DELAY_USB_CHECK!", 0, 1);
                USB_LCD_Switch(AK_TRUE);
                bglight = 0;
                #ifndef SUPPORT_BLUETOOTH    //暂时按照超时也不退出操作
                    break;
                #endif
            }

            if(!Fwl_DetectorGetStatus(DEVICE_USB))
            {
                break;
            }

            state = Fwl_UsbSlaveGetState();

#ifdef SUPPORT_BLUETOOTH
            if((test_bt_dev== AK_TRUE)||USB_SUSPEND == state || USB_UPDATE == state || USB_NOTUSE == state)
#else
			if(USB_SUSPEND == state || USB_UPDATE == state || USB_NOTUSE == state)
#endif
			{
                AK_PRINTK("usb out!\n",0, AK_TRUE);
                break;
            }
            if(11200 == transfer_tick) //200*56
            {
                AK_PRINTK("flush!\n",0, AK_TRUE);
                Fwl_StoreAllInt();
                /*2009-7-30 12:01 modified by lu for usb speed*/
                usbFlushCacheForTranSpd();
            #if (STORAGE_USED == NAND_FLASH)
                usbFlushNandDisk();
            #endif
                Fwl_RestoreAllInt();
            }
            
#ifdef SUPPORT_SDCARD
            if(g_sd_usb_rw_info.bSD_USB_RW_ERR)
            {
                akerror("usb sd err", 0, 1);
                g_sd_usb_rw_info.bSD_USB_RW_ERR = AK_FALSE;
                if ((g_sd_usb_rw_info.SD_RW_ERR_CNT > 0) 
                    && (g_sd_usb_rw_info.SD_RW_ERR_CNT < SD_USB_RW_ERR_LIMIT))
                {
                    Fwl_MemDevUnMount(MMC_SD_CARD);
                    Fwl_UsbDiskChgLun(MMC_SD_CARD);
                    
                    if(SD_DET_PUSH_IN == sd_statue)
                    {
                        sd_statue = SD_DET_PULL_OUT;
                    }
                    else
                    {
                        sd_statue = SD_DET_PUSH_IN;
                    }
                }
            }
            
            if(sd_statue != Fwl_DetectorGetStatus(DEVICE_SD))
            {
                akerror("sd state change", 0, 1);
                Fwl_DelayUs(100000);//延时消抖
                if(sd_statue != Fwl_DetectorGetStatus(DEVICE_SD))
                {
                    sd_statue = Fwl_DetectorGetStatus(DEVICE_SD);
                    if(SD_DET_PUSH_IN == sd_statue)
                    {
                        Fwl_DelayUs(500000);//to meet SD card spec
                        if (Fwl_MemDevMount(MMC_SD_CARD))
                        {
                            USB_LCD_Switch(AK_TRUE);
                            Fwl_UsbDiskChgLun(MMC_SD_CARD);
                        }
                    }
                    else
                    {
                        if (Fwl_MemDevUnMount(MMC_SD_CARD))
                        {
                            Fwl_UsbDiskChgLun(MMC_SD_CARD);
                        }
                        USB_LCD_Switch(AK_TRUE);
                    }
                    bglight = 0;
                }
            }
#endif

            if (timeout++ != 0)
            {
                if (timeout > 1000000)                    
                {
                    timeout = 0;    //reset timeout
                    if (USB_CONFIG == state)
                    {
                        AK_PRINTK("usb timeout out!\n",0, AK_TRUE);
                        if (bglight > BGLIGHT_OFF_TIME)
                            USB_LCD_Switch(AK_TRUE);
                        break;
                    }
                }
            }


            if (cur_state != state)
            {
                cur_state = state;
                transfer_tick = 0;
                refresh = AK_TRUE;
            }
            
#if(NO_DISPLAY == 0)
            // show pictrue.
            if (refresh && (bglight < BGLIGHT_OFF_TIME))
            {
                if (USB_BULKIN == state)
                {
                    //data in
                    USB_ShowStatus(UDISK_DATAIN);
                }
                else if (USB_BULKOUT == state)
                {
                    //data out
                    USB_ShowStatus(UDISK_DATAOUT);
                }
                else
                {
                    //idle
                    USB_ShowStatus(UDISK_IDLE);
                }
                refresh = AK_FALSE;
            }

            if (tick%50000 == 0)
            {
                if (bglight < BGLIGHT_OFF_TIME)
                {
                    USB_ShowBat();
                    bglight++;
                }
                else if (BGLIGHT_OFF_TIME == bglight)
                {
                    if (gb.BgLightTime != 0)
                    {
                        USB_LCD_Switch(AK_FALSE);
                        bglight++;
                    }
                    else
                    {
                        bglight = 0;
                    }
                }
            }
#endif

            if (Fwl_UsbSlaveIsExit())
            {
                break;
            }
        }
    }

    
    //flush immediately when exit USB
    //2009-7-30 12:01 modified by lu for usb speed,
    usbFlushCacheForTranSpd();
#if (STORAGE_USED == NAND_FLASH)
    usbFlushNandDisk();
#endif
    
    Fwl_UsbDiskStop();

    // update rom mode, 
    if(USB_UPDATE == state)
    {
        update_rom();
        while(1);
    }
#ifdef SUPPORT_BLUETOOTH     
    Profile_WriteData(eCFG_BTDEV,g_USB_Btdev_cfg);
    Fwl_Free(g_USB_Btdev_cfg);
    g_USB_Btdev_cfg = AK_NULL;
#endif    
#ifdef SUPPORT_SDCARD
    {
        //must judge whether is Sd card exist
        if(Fwl_MemDevIsMount(MMC_SD_CARD))
        {
            T_U8 DrvCnt = 0;
            T_U16 driverPath[4] = {0};
            
            Utl_UStrCpyN(driverPath, Fwl_MemDevGetPath(MMC_SD_CARD, &DrvCnt), 2);
                
            if(DrvCnt != 0)
            {
                for(i = 0; i < DrvCnt; i++)
                {
                    medium = FS_GetFakeMedium( (driverPath[0] - 'A') + i);
                    medium->flush(medium);
                }
            }
        }
    }
#endif
    Fwl_Freq_Clr_Add();

    //Fwl_RestoreAllInt();
    //Fwl_KeypadEnable(AK_TRUE);
    Fwl_DetectorEnable(DEVICE_SD, AK_TRUE);
    Fwl_DetectorEnable(DEVICE_LINEIN, AK_TRUE);
    Fwl_DetectorEnable(DEVICE_UHOST, AK_TRUE);
    PublicTimerStart();
    
    AK_DEBUG_OUTPUT("exit usb disk\n"); 

    //取空外部事件队列防止从U盘退出时会响应之前未处理的事件
    //修改上一版本因为调VME_EvtQueueClearAll()导致段超的问题
    VME_EvtQueueCreate();
    usbFreeCacheForTranSpd();
#endif
#ifdef SUPPORT_BLUETOOTH
	if(test_bt_dev)
	{
#if SUPPORT_HW_TEST 
		Start_PCTestBT();
#endif
		while(1);
	}
#endif
    return AK_TRUE;
}

/*********************************************
* @brief show usb status
*
* @author Justin.Zhao
*
* @date 2008-06-04
* @return : no return
* @retval
*******************************************************/
T_VOID USB_ShowStatus(T_UDISK_STATUS status)
{  
#if(NO_DISPLAY == 0)
    switch(status)
    {
    case UDISK_CONNECT:
        Eng_ImageResDisp(USB_PIC_POS_X, USB_PIC_POS_X, eRES_IMAGE_UDISKIDLE, AK_FALSE);
        break;
    case UDISK_DATAIN:
        //Eng_ImageResDisp(0, USB_PIC_POS_Y, eRES_IMAGE_UDISKIN, AK_FALSE);
        Eng_ImageResDisp(USB_ICON_POS_X, USB_ICON_POS_Y, eRES_IMAGE_ARROW1, AK_FALSE);
        break;
    case UDISK_DATAOUT:
        //Eng_ImageResDisp(0, USB_PIC_POS_Y, eRES_IMAGE_UDISKOUT, AK_FALSE);
        Eng_ImageResDisp(USB_ICON_POS_X, USB_ICON_POS_Y, eRES_IMAGE_ARROW, AK_FALSE);
        break;
    case UDISK_IDLE:
        Eng_ImageResDisp(USB_ICON_POS_X, USB_ICON_POS_Y, eRES_IMAGE_ARROW2, AK_FALSE);
        break;
    default:
        Eng_ImageResDisp(USB_ICON_POS_X, USB_ICON_POS_Y, eRES_IMAGE_ARROW2, AK_FALSE);
        break;
    }
#endif
}


T_VOID USB_ShowBat(T_VOID)
{
#if(NO_DISPLAY == 0)
    static T_U8 show_index = 0;
	T_U8 Grade;
    
    if (BATTERY_STAT_EXCEEDVOLT != Eng_VolDetect(&Grade))
    {
        show_index++;
        if (BATTERY_STAT_NOR_FULL == show_index)
            show_index = 0;
        Eng_ImageResDisp(BAT_ICON_POS_X, BAT_ICON_POS_Y, battery_img[show_index], AK_FALSE);
    }
    else
    {
        Eng_ImageResDisp(BAT_ICON_POS_X, BAT_ICON_POS_Y, battery_img[BATTERY_STAT_NOR_FULL-1], AK_FALSE);
    }
#endif
}

/*********************************************
* @brief USB_LCD_Switch
*
* @author xuping
*
* @date 2009-12-14
* @param T_BOOL bTurnon:   AK_TRUE:turn on LCD  
*                          AK_FALSE:Turn off LCD
* @return : no return
* @retval
*******************************************************/
static T_VOID USB_LCD_Switch(T_BOOL bTurnon)
{
#if(NO_DISPLAY == 0)
    if(bTurnon)
    {
        Fwl_LCD_on(AK_TRUE);
    }
    else
    {
        Fwl_LCD_off();
    }
#endif
}

T_VOID usb_using_exit_handle(T_VOID)
{
#ifdef OS_ANYKA
    usbFlushNandDisk();
#ifdef SUPPORT_SDCARD
    {
        Fwl_MemDevUnMount(MMC_SD_CARD);
        if(SD_DET_PUSH_IN == Fwl_DetectorGetStatus(DEVICE_SD))
        {
            Fwl_MemDevMount(MMC_SD_CARD);
        }
    }
#endif

#endif //if OS_ANYKA
}
//the end of file

