/*******************************************************************************
 * @file    Fwl_System.c
 * @brief   this file will constraint the access to the bottom layer system 
 *          function, avoid resource competition. Also, this file os for 
 *          porting to different OS
 * @author  Junhua Zhao
 * @date    2005-5-31
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"

#include "anyka_bsp.h"
#include "mtdlib.h"
#include "Fwl_osMalloc.h"
#include "Fwl_System.h"
#include "Fwl_Timer.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Keypad.h"
#include "Fwl_Detect.h"
#include "Fwl_Console.h"
#include "Fwl_PMU.h"
#include "Fwl_RTC.h"
#include "vme.h"
#include "boot.h"
#include "string.h"
#include "hal_int_msg.h"
#include "gpio_define.h"
#include "arch_interrupt.h"
#include "arch_sys_ctl.h"
#include "arch_analog.h"
#include "arch_timer.h"
#include "arch_gpio.h"
#include "arch_init.h"
#include "Eng_Debug.h"
#include "prog_manager.h"
//#include "Apl_Initialize.h"
#include "Hal_mmu.h"
#include "hal_rtc.h"
#include "hal_keypad.h"
#include "arch_i2c.h"
#include "arch_mmc_sd.h"
#include "Fwl_sd.h"
#include "Fwl_usb_s_state.h"
#include "Gbl_PowerVal.h"
#include "fwl_MicroTask.h"
#include "arch_pmu.h"
#include "hal_keypad.h"
#include "Fwl_Blue.h"
#include "Fwl_LED.h"
#include "Fwl_radio.h"

#ifdef OS_WIN32
#include <windows.h>
#endif


#define BATTERY_VALUE_INVALID       0xFFFF

#define SAVE_VOLTAGE_NUM            4
#define SAVE_VOLTAGE_SHIFT          2

//every group DMA information has 2 parts: viraddr,size
#define DMA_APP_MAX                 5
#define DMA_REG_MAX                 100
#define WATCHDOG_INVALIDATION_ID    (0XFF)
//if can't to feed dog,will reset time is WATCHDOG_FEED_TIME_DEF+WATCHDOG_RESET_TIME
#define WATCHDOG_RESET_TIME         (5 * 1024) //5S
#define WATCHDOG_MICRO_TASK_TIME    (100)  //100*10ms = 1S
#define WATCHDOG_FEED_TIME_DEF      (1) // 5S+1S=6S

#pragma arm section zidata = "_bootbss1_"
static T_U16    VolSample[SAVE_VOLTAGE_NUM];
static T_U8     volIdx;

#ifdef CHECK_STARTUPTICK
T_U32  g_StartupTick;
T_U32  g_ResumeStartTick;
#endif
#pragma arm section zidata

#pragma arm section rodata = "_frequentcode_"
static const T_U8 strVolInfo[]="V:";
static const T_U8 strFreqInfo[]=" F:";
static const T_U8 strMemInfo[]=" M:";
static const T_U8 strSwapCnt[]=" S:";
static const T_U8 BlueA2DPKMsg[]=" K:";
#pragma arm section rodata

static T_U32  curVolCal; //current calculate voltage.
static T_VOID poweroff_handler(T_VOID);
static T_U16  sys_enter_standby(T_U8 bHasVw);
#ifdef DEEP_STANDBY
static T_VOID sys_enter_deep_standby(T_VOID);
#endif

#pragma arm section rwdata = "_cachedata_"
static T_U8 watchDogMicroTaskId = WATCHDOG_INVALIDATION_ID;
static volatile T_U32 watchDogMaxTimeCnt = WATCHDOG_FEED_TIME_DEF;
#pragma arm section rwdata

extern T_BOOL isinBtDev(void);
extern T_VOID analog_init(T_VOID);
extern T_VOID rtc_set_watchdog_time(T_BOOL en, T_U16 unit);
extern T_VOID rtc_watchdog_output(T_VOID);
extern T_VOID rtc_watchdog_feed(T_VOID);

extern T_VOID Fwl_UsbHostMsgDeal(T_BOOL bStatus);
extern T_VOID Fwl_DetectorMsgDeal(T_BOOL bStatus, T_U16 devInfo);
extern T_VOID Fwl_KeypadMsgDeal(T_U8 keyID, T_U16 pressType);
extern T_VOID Fwl_TimerMsgDeal(T_TIMER timerId, T_U32 deley);
extern T_VOID Fwl_RTCMsgDeal(T_VOID);
extern T_VOID Fwl_UsbSlaveMsgDeal(T_BOOL bStatus);
extern T_VOID Fwl_WaveForceOutClose(T_VOID);
extern T_VOID pwm_stop_32k_square(T_VOID);

#pragma arm section code = "_frequentcode_"
static T_VOID watchdog_micro_task_cb(T_VOID)
{
    //AK_PRINTK("wc: ", watchDogMaxTimeCnt, AK_TRUE);
    if (watchDogMaxTimeCnt > 0)
    {
        watchDogMaxTimeCnt--;
        rtc_watchdog_feed();
    }
}
#pragma arm section code

static T_VOID Fwl_RegisterWatchDogTask(T_VOID)
{
	return ;
    //register micro task for feed watch dog
    if (WATCHDOG_INVALIDATION_ID == watchDogMicroTaskId)
    {
        watchDogMicroTaskId = Fwl_MicroTaskRegister(watchdog_micro_task_cb, 
                                                    WATCHDOG_MICRO_TASK_TIME);
        //这里只在开机调，开机时，留长点时间
        watchDogMaxTimeCnt = WATCHDOG_FEED_TIME_DEF << 4;
        Fwl_MicroTaskResume(watchDogMicroTaskId);
    }
}

#pragma arm section code = "_sysinit_"
static T_BOOL Fwl_VolInit(T_VOID)
{
    T_U8 i;
    volIdx = 0;
    for(i=0; i<SAVE_VOLTAGE_NUM; i++)
        VolSample[i] = BATTERY_VALUE_INVALID;
    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_frequentcode_"
static T_BOOL Fwl_VolDet(T_VOID)
{
    T_U32 voltage;
#ifdef OS_ANYKA
    voltage = analog_getvoltage_bat();
#else
    voltage = 400;
#endif
    VolSample[volIdx]  = (T_U16)voltage;
    volIdx = (volIdx+1) % SAVE_VOLTAGE_NUM;
    
    return AK_TRUE;
}

static T_U32 Fwl_VolCal(T_VOID)
{
    T_U8 i;
    T_U32 sum;
    int sumcount;
    T_U32 ad_value;
    T_U32 min_vol, max_vol;
    
    sum = 0;
    sumcount = 0;
    for(i=0; i<SAVE_VOLTAGE_NUM; i++)
    {
        if (VolSample[i] != BATTERY_VALUE_INVALID)
        {
            sum += VolSample[i];
            sumcount++;
        }
    }

    if (sumcount == SAVE_VOLTAGE_NUM)
    {
        min_vol = max_vol = VolSample[0];
        for(i=1; i<SAVE_VOLTAGE_NUM; i++)
        {
            if(VolSample[i] > max_vol)
            {
                max_vol = VolSample[i];
            }
            if(VolSample[i] < min_vol)
            {
                min_vol = VolSample[i];
            }
        }
        ad_value = (sum - max_vol - min_vol) >> (SAVE_VOLTAGE_SHIFT - 1);         //SAVE_VOLTAGE_NUM == 4, >>2 == /4
    }
    else if (sumcount != 0)
    {
        ad_value = sum / sumcount;
    }
    else
    {
#ifdef OS_ANYKA
        ad_value = analog_getvoltage_bat();
#else
        ad_value = 3800;
#endif
    }

    //return (BATTERY_VOL_FORMULA(ad_value));
   return ad_value;
}


/*******************************************************************************
 * @brief   get current bat voltage
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the bat voltage(unit: mv)
*******************************************************************************/
T_U32 Fwl_SysGetBatVolt(T_VOID)
{
    Fwl_VolDet();
    Fwl_VolDet();

    curVolCal = Fwl_VolCal();

    return curVolCal;
}

extern T_U32 gA2DPKTime;

/*******************************************************************************
 * @brief   printf all infomation of the system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysInfoPrint(T_VOID)
{
#ifndef OS_WIN32
    AK_PRINTK(strVolInfo, curVolCal, AK_FALSE);
    AK_PRINTK(strFreqInfo, Fwl_FreqGet(), 0);
    AK_PRINTK(strMemInfo, Fwl_GetUsedMem(), 0);
	AK_PRINTK(BlueA2DPKMsg, gA2DPKTime, 0);
	gA2DPKTime = 0;
    
#endif
#ifdef SWAP_WRITBAK_PRINT
    AK_PRINTK(strSwapCnt, get_the_swap_cnt(), 0);
    clear_the_swap_cnt();
#endif
    Fwl_ConsoleWriteChr('\n');
}
#pragma arm section code


#pragma arm section code = "_bootcode1_"
T_VOID Fwl_StoreAllInt(T_VOID)    
{
    store_all_int();
}
T_VOID Fwl_RestoreAllInt(T_VOID)   
{
    restore_all_int();
}
#pragma arm section code 


/*******************************************************************************
 * @brief   initialize driver
 * @author  zhanggaoxin
 * @date    2013-03-20
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
static T_VOID Fwl_DrvInit(T_VOID)
{
    T_DRIVE_INITINFO drv_info;
    drv_info.chip = CHIP_1080L;
#ifdef OS_ANYKA
    drv_info.fPrint = akerror;
#endif
    drv_info.fRamAlloc = Fwl_Malloc;
    drv_info.fRamFree = Fwl_Free;
#ifdef OS_ANYKA
    drv_init(&drv_info);

    i2c_initial(GPIO_I2C_SCLK, GPIO_I2C_SDA);
    
#endif
   // RTC_Init(AK_NULL);
#ifdef DEEP_STANDBY
    deepstdb_set_callback(poweroff_handler);
#endif
}


#pragma arm section code = "_sysinit_"
/*******************************************************************************
 * @brief   power on to system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOn(T_VOID) 
{   
#ifdef OS_ANYKA
    Fwl_MallocInit();

    Fwl_DrvInit();

    Fwl_RegisterWatchDogTask();
    //Fwl_ConsoleInit();

#ifdef USED_INTERNAL_LDO_MODE
    Fwl_PMU_Init(AK_TRUE);
#else
    Fwl_PMU_Init(AK_FALSE);
#endif

    Fwl_VolInit();

    //progmanage_abort_checkirq(AK_TRUE);
#endif
}

#if (SYS_SWITCH_MODE == 1)
enum
{
	POWER_KEY_INIT,
	POWER_KEY_OPEN,
	POWER_KEY_CLOSE
};
#define POWER_OFF_MAX_TIME	100
T_U8 poweroffTaskID = T_U8_MAX;

#pragma arm section code ="_bootcode1_"
T_U8 poweroff_getstatus(T_VOID)
{
	if(keypad_get_power_status(0xFF))
	{
		return POWER_KEY_CLOSE;
	}
	else
	{
		return POWER_KEY_OPEN;
	}
}
T_BOOL Fwl_ConsoleClose(T_VOID);
T_VOID poweroff_RunCheck(T_VOID)
{
	static T_U8 PowerKeyStatus = POWER_KEY_INIT;
	static T_U32 PressKeySysTime;
    T_BOOL IsPressKey = AK_FALSE;
    T_BOOL charge_status = AK_FALSE;
    T_BOOL charging = AK_TRUE;
    
	if(PowerKeyStatus == POWER_KEY_INIT)
	{
		PowerKeyStatus = POWER_KEY_OPEN;
		PressKeySysTime = Fwl_GetTickCountMs();
	}
	else if(PowerKeyStatus == poweroff_getstatus())
	{
		if(POWER_KEY_CLOSE == PowerKeyStatus)
		{
			if((PressKeySysTime + POWER_OFF_MAX_TIME < Fwl_GetTickCountMs()))
			{
				Fwl_MicroTaskPause(poweroffTaskID);
				Fwl_MicroTaskUnRegister(poweroffTaskID);
			    Fwl_DetectorEnable(DEVICE_USB, AK_FALSE);
			    Fwl_DisableWatchdog();
                Fwl_BlueDeInit();
				Fwl_LEDOff(LED_RED|LED_BLUE);
				Fwl_WaveForceOutClose();
                Fwl_RadioFree();
                pwm_stop_32k_square();
				if(PowerKeyStatus != poweroff_getstatus())
				{
					PowerKeyStatus = poweroff_getstatus();
					PressKeySysTime = Fwl_GetTickCountMs();
					AkDebugOutput("system power restart!");
                    Fwl_ConsoleClose();
	                Fwl_SysReboot();
				}
				else
				{
				    while (LEVEL_HIGH == gpio_get_pin_level(USB_DETECT_GPIO))
				    {
			            //if (kbPOWER == Fwl_KeypadScan())
			            if(gpio_get_pin_level(POWERKEY_GPIO)&&(AK_FALSE == IsPressKey))    //拨到on,如果USB线插入，系统重启
			            {
			                IsPressKey = AK_TRUE;
			                #ifdef OS_ANYKA
			                    Fwl_SysReboot();
			                #endif
			            }
                        else if((AK_FALSE == pmu_charge_status())&&(AK_FALSE == charge_status))
                        {
                            charge_status = AK_TRUE;
                            charging = AK_FALSE;
                            Fwl_LEDOff(LED_BLUE|LED_RED);
                        }
                        
                        if(AK_TRUE == charging)
                        {
                            charging = AK_FALSE;
                            Fwl_LEDOff(LED_BLUE);
                            Fwl_LEDOn(LED_RED);
                        }
				    }
                    
				    rtc_enable_in_pwd(AK_FALSE);
                    pmu_dipswitch_enable(AK_FALSE);
					AkDebugOutput("system power off!");
                    //while(1);
				    soft_power_off();
				}
			}
		}
	}
	else
	{
		PowerKeyStatus = poweroff_getstatus();
		PressKeySysTime = Fwl_GetTickCountMs();
	}
}
#pragma arm section code

T_VOID poweroff_StartCheckThread(T_VOID)
{
	
	poweroffTaskID = Fwl_MicroTaskRegister(poweroff_RunCheck, 1); 
	Fwl_MicroTaskResume(poweroffTaskID);	
}
#endif

static T_VOID poweroff_handler(T_VOID)
{
    T_BOOL IsPressKey = AK_FALSE;
    T_BOOL charge_status = AK_FALSE;
    T_BOOL charging = AK_TRUE;    
    
    Fwl_DetectorEnable(DEVICE_USB, AK_FALSE);
    Fwl_LEDInit();    //LedHint_Init()里面也会调用
    Fwl_DisableWatchdog();

    while (LEVEL_HIGH == gpio_get_pin_level(USB_DETECT_GPIO))
    {
            //if (kbPOWER == Fwl_KeypadScan())
            if(gpio_get_pin_level(POWERKEY_GPIO)&&(AK_FALSE == IsPressKey))    //拨到on,如果USB线插入，系统重启
            {
                IsPressKey = AK_TRUE;
                #ifdef OS_ANYKA
                    Fwl_SysReboot();
                #endif
            }
            else if((AK_FALSE == pmu_charge_status())&&(AK_FALSE == charge_status))
            {
                charge_status = AK_TRUE;
                charging = AK_FALSE;
                Fwl_LEDOff(LED_BLUE|LED_RED);
            }
            
            if(AK_TRUE == charging)
            {
                charging = AK_FALSE;
                Fwl_LEDOff(LED_BLUE);
                Fwl_LEDOn(LED_RED);
            }
    }
    
    //rtc_enable_in_pwd(AK_FALSE);
    pmu_dipswitch_enable(AK_FALSE);
    soft_power_off();
}

/*******************************************************************************
 * @brief   power off to system
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysPowerOff(T_VOID)
{
#ifdef DEEP_STANDBY
    sys_enter_deep_standby();
#else
    poweroff_handler();
#endif
}
#pragma arm section code 


#pragma arm section code ="_bootcode1_"

/*******************************************************************************
 * @brief   deal system message
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SysMsgHandle(T_VOID)
{
    ST_MSG msg;

    while (peek_int_message(&msg))
    {
        switch (msg.msg)
        {
        case IM_KEYINFO:
            Fwl_KeypadMsgDeal(msg.bparam, msg.hwparam);
            break;

        case IM_TIMER:
            Fwl_TimerMsgDeal(msg.bparam, msg.hwparam);
            break;

        case IM_RTC:
            Fwl_RTCMsgDeal();
            break;

        case IM_DEVCHANGE:
            Fwl_DetectorMsgDeal(msg.bparam, msg.hwparam);
            break;

        #ifdef SUPPORT_USBHOST
        case IM_UHOST:
            Fwl_UsbHostMsgDeal(msg.bparam);
            break;
        #endif

        case IM_USLAVE:
            Fwl_UsbSlaveMsgDeal(msg.bparam);
            break;

        default:
            break;
        }
    }
}
#pragma arm section code
#pragma arm section rodata ="_bootcode1_"
const T_CHR test1[]={"wait uart"};
const T_CHR test2[]={"uart recv"};
const T_CHR test3[]={"w1"};
const T_CHR test4[]={"w2"};
#pragma arm section rodata


#pragma arm section code ="_bootcode1_"
/*******************************************************************************
 * @brief   reset the system
 * @author  yangyiming
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
//extern T_VOID uart_wait_recv(T_VOID);

/*#define REG32(_register_)   (*(volatile T_U32 *)(_register_))
#define TOPCFG_MODULE_BASE_ADDR         (0x00400000)
#define REG_SHARE_PIN_CTRL              (TOPCFG_MODULE_BASE_ADDR + 0x74)
#define REG_SHARE_PIN_CTRL2             (TOPCFG_MODULE_BASE_ADDR + 0x108)
#define REG_SHARE_PIN_CTRL3             (TOPCFG_MODULE_BASE_ADDR + 0x10C)
#define REG_CLOCK_RST_EN             (TOPCFG_MODULE_BASE_ADDR + 0x0C)
*/
T_VOID Fwl_SysReboot(T_VOID)
{
    //Soft_Reset();
    void (*f)(void) = 0;

    store_all_int();

    //clk_set_pll(120);      
    MMU_InvalidateIDCache();

    MMU_DisableICache();
    MMU_DisableDCache();      
    MMU_InvalidateTLB();
    MMU_DisableMMU();

    //REG32(REG_CLOCK_RST_EN) = ((REG32(REG_CLOCK_RST_EN) &0xffff)|0xffff0000);
    //REG32(REG_CLOCK_RST_EN) = (REG32(REG_CLOCK_RST_EN) &0xffff);
    (*(volatile T_U32 *)0x0040000C) = (((*(volatile T_U32 *)0x0040000C) &0xffff)|0xffff0000);
    (*(volatile T_U32 *)0x0040000C) = ((*(volatile T_U32 *)0x0040000C) &0xffff);
    
    (*(volatile T_U32 *)0x0041002C) = 0x0;
    f();
}
#pragma arm section code


#pragma arm section code = "_changefreq_"
#ifdef DEEP_STANDBY
extern void deepstdb_enter(void);
/*******************************************************************************
 * @brief   enter deepstandby
 * @author  liangxiong
 * @date    2013-02-25
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
static T_VOID sys_enter_deep_standby(T_VOID)
{   
    ST_MSG       msg;
    T_U8         bk_data[DMA_REG_MAX];
#ifdef CHECK_STARTUPTICK
    T_U32        preResumeTick;
    T_U32        tempTick;
#endif
#if(STORAGE_USED == NAND_FLASH)
T_DMA_INFO   dma_info[DMA_APP_MAX];
T_U32        groups;
#endif
    if (DMA_REG_MAX < get_reg_buf_size())
    {
        AK_PRINTK("Error buf_size.", 0, AK_TRUE);
        while(1);
    }
    Fwl_FreqPush(FREQ_APP_MAX);

    AK_PRINTK("S", 0, AK_FALSE);
    store_all_int();
    
    while(peek_int_message(&msg)); //清空driver message.
    /* save position and size of DMA memory */
#if(STORAGE_USED == NAND_FLASH)
    groups = deepstdb_DMA_save(dma_info, DMA_APP_MAX);
#endif

    AK_PRINTK("T", 0, AK_FALSE);
    
    store_base_reg(bk_data);
    
    AK_PRINTK("A", 0, AK_FALSE);
    /* enter deep standby mode and power off!*/
#ifdef DEEP_STANDBY
    deepstdb_enter();
#endif
    AK_PRINTK("N", 0, AK_FALSE);

#ifdef CHECK_STARTUPTICK
    if (g_ResumeStartTick > Fwl_GetTickCountUs())
    {
        preResumeTick = 0xffffffff + (g_ResumeStartTick - Fwl_GetTickCountUs());
    }
    else
    {
        preResumeTick = Fwl_GetTickCountUs() - g_ResumeStartTick;
    }
#endif

    //初始化一些必要的硬件。
    restore_base_reg(bk_data);
    AK_PRINTK("B", 0, AK_FALSE);

#ifdef CHECK_STARTUPTICK
    tempTick = Fwl_GetTickCountUs();
#endif

#ifdef SUPPORT_SDCARD
    //sd_card_init();
#endif
    
#if(NO_DISPLAY == 0)
    Fwl_DisplayInit(); 
#endif

    /* resume application of DMA memory before entering deep standby mode */
#if(STORAGE_USED == NAND_FLASH)
    if (groups)
        deepstdb_DMA_resume(dma_info, groups);
#endif
    restore_all_int();

    
#ifdef USED_INTERNAL_LDO_MODE
    Fwl_PMU_Init(AK_TRUE);
#else
    Fwl_PMU_Init(AK_FALSE);
#endif

    Fwl_FreqPop();

    AK_PRINTK("Y", 0, AK_TRUE);

    Fwl_RegisterWatchDogTask();

#ifdef CHECK_STARTUPTICK
    if (tempTick > Fwl_GetTickCountUs())
    {
        preResumeTick += 0xffffffff + (tempTick - Fwl_GetTickCountUs());
    }
    else
    {
        preResumeTick += Fwl_GetTickCountUs() - tempTick;
    }
    g_StartupTick += preResumeTick;
    AK_DEBUG_OUTPUT("StartupTick deepstdb success: 0x%x us.\n", g_StartupTick);
#endif
}
#endif

T_U32 ExitStandbyTime = T_U32_MAX;
static T_U16 sys_enter_standby(T_U8 bHasVw)
{
    T_U16 wakeupType, backup = 0;

    AK_PRINTK("z1",0,AK_TRUE);
    store_all_int();
    analog_wakeup_enable(AK_TRUE);
    usb_wakeup_enable(AK_TRUE);
    powerkey_wakeup_enable(AK_TRUE);
	if(1 == gpio_get_pin_level(GPIO_SD_DET))
	{
		backup = 1;
		gpio_wakeup_polarity(GPIO_SD_DET, 0);
		gpio_wakeup_enable(GPIO_SD_DET,AK_TRUE);
	}
	
    if (bHasVw)
    {
        voice_wakeup_enable(AK_TRUE);
    }

#ifdef SUPPORT_SDCARD
    Fwl_SD_Free();
#endif
    
    AK_PRINTK("z2", 0, AK_TRUE);
   // pmu_dipswitch_enable(AK_TRUE);
    enter_standby();
    //pmu_dipswitch_enable(AK_FALSE);

    AK_PRINTK("z3", 0, AK_TRUE);
    wakeupType = exit_standby();

    analog_wakeup_enable(AK_FALSE);
    usb_wakeup_enable(AK_FALSE);
	if(backup)
	{
	    powerkey_wakeup_enable(AK_FALSE);
		gpio_wakeup_enable(GPIO_SD_DET,AK_FALSE);
	}
    if(AK_TRUE == bHasVw)
    {
        voice_wakeup_enable(AK_FALSE);
    }
    
    AK_PRINTK("z4:", wakeupType, AK_TRUE);
    restore_all_int();
   	ExitStandbyTime = Fwl_GetTickCountMs();
    return wakeupType;
}
#pragma arm section


/*******************************************************************************
 * @brief   let speaker ring.
 * @author  Pengyu Xue
 * @date    2001-06-18
 * @param   [in]dwFreq Sound frequent.
 * @param   [in]dwDuration Sound duration.
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_Beep(T_U32 dwFreq, T_U32 dwDuration)
{
#ifdef OS_WIN32
    Beep(dwFreq, dwDuration);
#endif
}


/*******************************************************************************
 * @brief   make system enters the standby state
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]wakeupMode: 
 * @return  T_U16
 * @retval  the reason of system waked
*******************************************************************************/
T_U16 Fwl_SysSleep(T_U8 bHasVw)
{
    T_U16 ret;

    Fwl_DisableWatchdog();
    pmu_vref15_sel(VREF_LPBGR);
    
    ret = sys_enter_standby(bHasVw);

    pmu_vref15_sel(VREF_BGR);
    Fwl_EnableWatchdog();
    
    return ret;
}

/*******************************************************************************
 * @brief   if charger is connect,do nothing,else disconnect,check bat voltage
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_CheckPowerStatus(T_VOID)
{
    T_U32 voltage = 0;
    T_U32 vol_max = 0;
    T_U32 vol_min = T_U32_MAX;
    T_U32 i,tmp, j;

    //在关机时，保存了GPIO，中断等寄存器的状态；长按POWER没有触发中断
    //开机时，如果不关闭硬件自动关机，会有问题；
    //cancel_power_off();
	
  //  rtc_enable_in_pwd(AK_TRUE);

 //   pmu_vref15_sel(VREF_BGR);

    RTC_Init(AK_NULL);
    //watch dog 在启动时，必须得先关一下清0
	rtc_set_watchdog_time(AK_TRUE, 0);     
	rtc_set_watchdog_time(AK_FALSE, 0); 
    
    //charger is disconnect
    Fwl_SpkConnectSet(AK_FALSE);

    if (LEVEL_HIGH != gpio_get_pin_level(USB_DETECT_GPIO))
    {
    	delay_ms(100);//电池启动时，SPK 脚有120 ms的高电平,且设了GPIO 输出低电平也不管用
		for(j = 0, voltage = 0; j < 3; j ++)
		{
	        for (i = 0; i < 6; i++)
	        {
	            tmp = analog_getvoltage_bat();
	            voltage += tmp;
	            if (tmp < vol_min)
	            {
	                vol_min = tmp;
	            }
	            if (tmp > vol_max)
	            {
	                vol_max = tmp;
	            }
	        }

	        voltage = (voltage - vol_min - vol_max) >> 2;
			
			if (BATTERY_VALUE_SDOWN <= voltage) //3.45V
			{
				break;
			}
		}
 
	 if (BATTERY_VALUE_SDOWN > voltage) //3.45V
	 {
		 AK_PRINTK("cv: ", voltage, AK_TRUE);
		 rtc_enable_in_pwd(AK_FALSE);
		 soft_power_off();
	 }
 }
 else
 {
	 //usb in,but,power is disconnect;
	 if(LEVEL_LOW == gpio_get_pin_level(POWERKEY_GPIO)) //如果power按键拨到off，直接关机
	 {
		 poweroff_handler();
	 }
	 else
	 {
		 soft_power_on();
	 }
 }
 
 
    Fwl_EnableWatchdog();
}

/*******************************************************************************
 * @brief   enable watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_EnableWatchdog(T_VOID)
{
	return; 
    rtc_watchdog_output();
    rtc_set_watchdog_time(AK_TRUE, WATCHDOG_RESET_TIME); //5S

    watchDogMaxTimeCnt = WATCHDOG_FEED_TIME_DEF;
    Fwl_MicroTaskResume(watchDogMicroTaskId);
}

/*******************************************************************************
 * @brief   disenable watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return   T_VOID
 * @retval  
*******************************************************************************/
T_VOID Fwl_DisableWatchdog(T_VOID)
{
	return; 
    rtc_set_watchdog_time(AK_FALSE, 0);

    Fwl_MicroTaskPause(watchDogMicroTaskId);
}

#pragma arm section code = "_frequentcode_"
/*******************************************************************************
 * @brief   feed the watchdog
 * @author  luheshan
 * @date    2013-06-19
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_FeedWatchdog(T_VOID)
{
    if (WATCHDOG_FEED_TIME_DEF > watchDogMaxTimeCnt)
    {
        watchDogMaxTimeCnt = WATCHDOG_FEED_TIME_DEF;
    }
    
    //rtc_watchdog_feed();
}
#pragma arm section code

/*******************************************************************************
 * @brief   set the watchdog die time to long time
 * @author  luheshan
 * @date    2013-06-28
 * @param[in]   time: watchdog die time is time*1S + 5S
                               if time==0;will disable long time watch,and dog die time is 6S
 * @return  T_VOID
*******************************************************************************/
T_VOID Fwl_SetLongWatchdog(T_U32 time)
{    
    if (time > 0)
    {
        watchDogMaxTimeCnt = time;
    }
    else
    {
        watchDogMaxTimeCnt = WATCHDOG_FEED_TIME_DEF;
    }
}

/* end of files */
