
#include "anyka_types.h"
#include "Fwl_osFS.h"
#include "Fwl_Timer.h"
#include "string.h"
#include "Fwl_detect.h"
#include "Fwl_osMalloc.h"
#include "gpio_define.h"
#include "Eng_VoiceTip.h"
#include "Fwl_keypad.h"
#include "Gbl_Global.h"
#include "Apl_public.h"
#include "Fwl_led.h"

#include "BtDev.h"
#include "Eng_BtPlayer.h"

#include "Fwl_Radio.h"
#include "Fwl_waveout.h"

#if SUPPORT_HW_TEST 


#define  TEST_TIMEOUT		15000
#define  TEST_VOLUME        10      //测试时播放的音量
#define  TEST_TIME          10000    //测试时播放的时间

#define HW_TEST_PRINT    AK_DEBUG_OUTPUT
#define HW_TEST_DEBUG(s,n)      akerror(s,n,1)

static T_U8 *default_environment = AK_NULL;

static T_U8 *env_get_addr(T_pVOID buffer, T_U32 index);
static T_U8 env_get_char(T_pVOID buffer, T_U32 index);
static T_U8 *env_get_next_line(T_U8 *curLine);
static T_BOOL env_construct(T_VOID);
static T_U32 env_init_contex(T_pCWSTR path,T_pVOID pContextbuf,T_U32 count);
static T_VOID env_destory(T_VOID);
static T_U8 *env_read_cfg(T_U8 *buffer, T_U8 *var, T_U8 *config, T_U32 bufsz);
static T_BOOL env_check_char_valid(T_pVOID buffer, T_U32 index, const T_U8 *exclude, T_U32 len);
extern void Blue_Process(void);
extern T_VOID initbt_player(T_VOID);
extern T_VOID exitbt_player(T_VOID);
extern T_VOID BtDev_PrintAddr(T_BD_ADDR addr);
extern T_BOOL Profile_WriteData(T_eCFG_ITEM item, const T_pVOID buff);




/*T_U16 RDAFM_GetChipID(void);
T_VOID rda_blue_init(T_VOID);
T_VOID rda_blue_open(T_BOOL isUseCfg, T_pVOID exCfg);
T_BOOL rda_blue_start(T_BOOL isConnectOld);
T_BOOL isRdaBtOn(T_VOID);
T_VOID rda_blue_close(T_VOID);
T_VOID rda_blue_deinit(T_VOID);
*/

T_BOOL Save_Null_Cfg = AK_FALSE;
extern T_BOOL g_test_mode;
extern T_BTDEV_CTRL *gBtDevCtrl;


static T_BOOL StartBT_Test(T_U32 timeout, T_U32 playtime)
{
	T_U32 startTime, CurrentTime, connected, startPlay = 0 ;
	T_BOOL m_playFlg = AK_FALSE ;
	T_BTDEV_CFG * cfg;

	AkDebugOutput("++++ enter Bluetooth test !!!! \n");
	
	g_test_mode = AK_TRUE ;
	
	initbt_player();
	startTime = Fwl_GetTickCountMs() ;
	do
	{
		Blue_Process();
		
		CurrentTime = Fwl_GetTickCountMs() ;
		if((CurrentTime>= startTime)&&((CurrentTime-startTime)> timeout) ||
			((CurrentTime< startTime)&&(((T_U32_MAX-startTime)+CurrentTime)> timeout)))
		{
			break;
		}

		if(m_playFlg == AK_FALSE)
		{
			if(BtPlayer_IsDecing())
			{
				m_playFlg = AK_TRUE ;
				startPlay = Fwl_GetTickCountMs() ;
			}
				
		}
		else
		{
			if((CurrentTime>= startPlay)&&((CurrentTime-startPlay)> playtime) ||
			((CurrentTime< startPlay)&&(((T_U32_MAX-startPlay)+CurrentTime)> playtime)))
			{
				break;
			}
		}
		

	}while(1);

	BA_GetConnectionStatus(gBtDevCtrl->RemoteInfo.Info.BD_ADDR, &connected);
	if(connected)
	{
		BA_Disconnect(gBtDevCtrl->RemoteInfo.Info.BD_ADDR);
			
		while(connected)
		{			
			BA_Process(0);
			BA_GetConnectionStatus(gBtDevCtrl->RemoteInfo.Info.BD_ADDR, &connected);
		}
	}

	exitbt_player();

	g_test_mode = AK_FALSE;
	AkDebugOutput("---- Completed Bluetooth test %s!!!! \n",m_playFlg?"OK":"FAIL");

	//清除测试数据
	cfg = Fwl_Malloc(sizeof(T_BTDEV_CFG));
	memset(cfg, 0, sizeof(T_BTDEV_CFG));
	
    Profile_WriteData(eCFG_BTDEV, cfg);
	cfg = Fwl_Free(cfg);
	return m_playFlg;
	
}

T_VOID Start_PCTestBT(T_VOID)
{
	StartBT_Test(TEST_TIMEOUT, TEST_TIME);
}

static T_U8 env_get_char(T_pVOID buffer, T_U32 index)
{
    T_U8 ch;

    ch = ((T_U8 *)buffer)[index];
    
    return (ch);
}

static T_U8 *env_get_addr(T_pVOID buffer, T_U32 index)
{
    return (&(((T_U8 *)buffer)[index]));
}

static T_BOOL env_char_in_set(const T_U8 ch, const T_U8 *set, T_U32 len)
{
    T_U32 i = 0;

    for (;i<len;i++)
    {
        if (ch == set[i])
        {
            return AK_TRUE;
        }
    }
    return AK_FALSE;
}

static T_BOOL env_check_char_valid(T_pVOID buffer, T_U32 index, const T_U8 *exclude, T_U32 len)
{
    T_U8 ch = env_get_char(buffer, index);

    return (!env_char_in_set(ch, exclude, len));
}

static T_U8 *env_read_cfg(T_U8 *buffer, T_U8 *var, T_U8 *config, T_U32 bufsz)
{
    T_U32 i, nxt, len, vallen;
    const T_U8 *lval, *rval, *nval;
    const T_U8 excludech[2] = {'\0', '\n'};
    
    if (!config)
    {
        return AK_NULL;
    }
    
    nxt = 0;
    config[0] = '\0';

    if (var)
    {
        len = strlen(var);
    }
    else
    {
        len = 0;
    }
    
    /* now iterate over the variables and select those that match */
    for (i=0; env_check_char_valid(buffer, i, excludech, 1); i=nxt+1) {

        // get formated string    as : xxxx=xxx
        for (nxt=i; env_check_char_valid(buffer, nxt, excludech, 2); ++nxt)
            ;
        
        // get  left string
        lval = (char *)env_get_addr(buffer, i);
        nval = (char *)env_get_addr(buffer, nxt);

        // get right string
        rval = strchr(lval, '=');
        if (rval != AK_NULL) {
            vallen = rval - lval;
            rval++;
        } else
            vallen = nval - lval;

        if (var)
        {
            // match lval as input lval
            if (len > 0 && (vallen < len || memcmp(lval, var, len) != 0))
                continue;
            
            // get right val  length
            vallen = nval - rval;
            nval = rval;
        }
        else
        {
            // redirect to left val
            nval = lval;
        }
        
        // check lval whether valid
        if (0 == vallen)
            continue;
        
        if (vallen >= bufsz)
        {
            vallen = bufsz-1;
        }
        memcpy(config, nval, vallen); config += vallen;
        *config++ = '\0';
        
        return rval;
            
    }

    return AK_NULL;
}


static T_BOOL env_construct(T_VOID)
{
	const T_U16 testbtPath[] = {'A',':','/','T','e','s','t','B','T','.','D','A','T','\0'};	
    T_U32 len = 4096;

    if (AK_NULL != default_environment)
    {
        akerror("env has created", 0, 1);
        return AK_TRUE;
    }
    
	//if there is one sd in slot
	if (AK_TRUE == Fwl_DetectorGetStatus(DEVICE_SD))  
    {
		default_environment = Fwl_DMAMalloc(len);
    }   
    else
    {
		akerror("No SD card, Boot Normally", 0, 1);
    }
    
    if (AK_NULL != default_environment)
    {
        if (env_init_contex(testbtPath, default_environment, len) > 2)
        {
            return AK_TRUE;
        }
    }
    
    env_destory();

    return AK_FALSE;
}

static T_VOID env_destory(T_VOID)
{
    if (AK_NULL != default_environment)
    {
		Fwl_DMAFree(default_environment);
        default_environment = AK_NULL;
    }
}

static T_U8 *env_get_next_line(T_U8 *curLine)
{
    T_U8 *cur, *next; 

    if (AK_NULL == curLine)
    {
        cur = next = default_environment;
    }
    else
    {
        cur = next = curLine;
    }
    
    while (next != AK_NULL)
    {   
        cur = next;
        next = strchr(cur, '\n');
        
        if (next)
        {
            next += 1;
        }

        // if not first
        if (AK_NULL != curLine)
        {
            cur = next;
        }
        
        if (cur)
        {
            //skip "comment" line.
            if((strstr(cur,"//")) || (strchr(cur,'#'))  || (strchr(cur,';')))
            {
                continue;
            }
        }
        
        return cur;
    }

    return AK_NULL;
    
}

static T_U32 env_init_contex(T_pCWSTR path,T_pVOID pContextbuf,T_U32 count)
{
	T_hFILE fHandle = FS_INVALID_HANDLE;
	T_U32 ByteCnt = 0;

	fHandle = Fwl_FileOpen(path, _FMODE_READ, _FMODE_READ);

	if(FS_INVALID_HANDLE != fHandle)	//读到文件.
	{	
		Fwl_FileSeek(fHandle, 0L, FS_SEEK_SET);

		/* Read data back from file: */
		ByteCnt = Fwl_FileRead(fHandle,pContextbuf,count);
	}
	else
	{		
		akerror("open file fail\n", 0, 1);
	}
	Fwl_FileClose(fHandle);
	return ByteCnt;
}


static T_U8 env_get_asic_val(T_U8 ch)
{
    if ((ch>='0')&&(ch<='9'))
    {
        return (ch - '0');
    }
    else if ((ch>='a')&&(ch<='Z'))
    {
        return (ch - 'a') + 10;
    }
    else if ((ch>='A')&&(ch<='Z'))
    {
        return (ch - 'A') + 10;
    }

    return 0;
}

/*
static T_U32 env_strm_revert(T_U8 *str, T_U32 len)
{
    T_U32 i=0, cnt=0;
    T_U8 ch;

    if (len < 2)
    {
        return 0;
    }
    
    cnt = len >> 1;
    len -= 1;
    for (;i<cnt;i++)
    {
        ch = str[i];
        str[i] = str[len - i];
        str[len - i] = ch;
    }
}
*/
static T_BOOL env_str2byte(T_U8 *str, T_U8 *byte, T_U32 len)
{
    T_U32 strLen = strlen(str);
    T_U32 i=0, j=0;

    while ((i<strLen)&&(j<len))
    {
        byte[j] = env_get_asic_val(str[i+1]) & 0xF;
        byte[j] |= (env_get_asic_val(str[i]) << 4);

        j++;
        i+= 2;
    }

	return AK_TRUE;
}


//==========================================================

static T_BOOL SD_Test(void)
{	
	T_BOOL ret = AK_FALSE;

	akerror("SD Cart Test\r\n\n", 0, 1);
	//add SD test case here:
	
	ret = AK_TRUE;
	return ret;
	
}


static T_BOOL BtDev_HwTest(T_U8 *tips, T_U32 timeout, T_U32 playtime)
{
	T_U8 cfg_Dongle[80];
	T_U8 cfg_Local[80];
	T_BD_ADDR m_localAddr ;
	T_BD_ADDR m_dongleAddr ;
	T_BTDEV_CFG *cfg;
	T_U8 name[] = "ak_test";
	
	if((env_read_cfg(env_get_next_line(AK_NULL), "DONGLE_BTDEV_ADDR", cfg_Dongle, sizeof(cfg_Dongle)))
		&&(env_read_cfg(env_get_next_line(AK_NULL), "LOCAL_BTDEV_ADDR", cfg_Local, sizeof(cfg_Local))))
	{
		env_str2byte(cfg_Dongle, m_dongleAddr, sizeof(m_dongleAddr));
		env_str2byte(cfg_Local, m_localAddr, sizeof(m_localAddr));
	
		cfg = Fwl_Malloc(sizeof(T_BTDEV_CFG));
		
		if(AK_NULL == cfg)
		{
			return AK_FALSE;
		}

		memset(cfg, 0, sizeof(T_BTDEV_CFG));
		memcpy(cfg->localInfo.info.BD_ADDR, m_localAddr, sizeof(T_BD_ADDR));
		memcpy(cfg->pairedList[0].BD_ADDR, m_dongleAddr, sizeof(T_BD_ADDR));
		memcpy(cfg->localInfo.info.name,name,sizeof(name));
		cfg->localInfo.info.classofDevice = 0X240404;
		BtDev_PrintAddr(cfg->pairedList[0].BD_ADDR);
		cfg->a2dpCurVol = 1024;
		Profile_WriteData(eCFG_BTDEV, cfg);
		cfg = Fwl_Free(cfg);
		
		return StartBT_Test(timeout, playtime);
	}

    return AK_TRUE;
}

extern T_U8 RDA5802_ValidStop(T_U16 freq);
extern T_VOID RDA5802_SetFreq(T_U32 curFreq );
//extern T_S32 rdafm_poweron_start(void);

static T_BOOL FM_Test(T_VOID)
{
	T_U32  i = 875;
	T_U32  Valid_Freq = 875*100000;
    T_U32  appoint_freq[10] = {0};
	T_U8   status= AK_FALSE;
    T_BOOL auto_search = AK_FALSE;
    T_U8   cfg_auto_search[40] = {0};
    T_U8   cfg_freq[40] = {0};
    T_U8   appoint_num[40] = {0};
    T_U8   freq[4] = {0};
    T_U8   channel_num = 0;
    T_U8   j = 0;

    T_U8 fm_freq[9]= {"FM_FREQ1"};
    
	if(AK_FALSE == Fwl_RadioInit())
    {
		HW_TEST_PRINT("Radio Init Error!\r\n");
        return AK_FALSE;
    }

	if(env_read_cfg(env_get_next_line(AK_NULL), "AUTO_SEARCH", cfg_auto_search, sizeof(cfg_auto_search)))
	{
        if (strchr(cfg_auto_search, 'Y')||strchr(cfg_auto_search, 'y'))
        {
            auto_search = AK_TRUE;
        }
        else
        {
            if(env_read_cfg(env_get_next_line(AK_NULL), "FM_CHANNEL_NUM", appoint_num, sizeof(appoint_num)))
            {
                channel_num =env_get_asic_val(appoint_num[0]);
                HW_TEST_PRINT("FM_CHANNEL_NUM:%d\r\n",channel_num);
            }
            for(j=0;j<channel_num;j++)
            {
                fm_freq[7]=j+49;
                HW_TEST_PRINT("%s  ",fm_freq);
                if(env_read_cfg(env_get_next_line(AK_NULL), fm_freq, cfg_freq, sizeof(cfg_freq)))
                {
                    freq[0]= env_get_asic_val(cfg_freq[0]);
                    freq[1]= env_get_asic_val(cfg_freq[1]);
                    freq[2]= env_get_asic_val(cfg_freq[2]);
                    freq[3]= env_get_asic_val(cfg_freq[3]);
                    if(freq[0] > 1)
                    {
                        appoint_freq[j] = freq[0]*100+freq[1]*10+freq[2];
                    }
                    else
                    {
                        appoint_freq[j] = freq[0]*1000+freq[1]*100+freq[2]*10+freq[3];
                    }
                      
                    
                    if(RDA5802_ValidStop(appoint_freq[j]))
                    {
                        HW_TEST_PRINT("Appoint frep:%d is vailid!\r\n",appoint_freq[j]);
                    }
                    else
                    {
                        HW_TEST_PRINT("Appoint frep:%d is invailid!\r\n",appoint_freq[j]);
                        return AK_FALSE;
                    }
                }            
            }
            
        }
    }
	
	if(AK_TRUE == auto_search)
	{
		while(1)
        {
            status = RDA5802_ValidStop(i);

            if (AK_TRUE == status)
            {
                {
                    HW_TEST_PRINT("vaild frep:%d\r\n",i);
                    Valid_Freq = i*100000;
                    break;
                }
            }
            
            i++;   
            
            if (1080 < i)
			{
                i = 875;
                RDA5802_SetFreq(i);
				HW_TEST_PRINT("No valid Freq!\r\n");
				break;
			}
		}
        
		HW_TEST_PRINT("Cur frep:%d\r\n",i);
		
		if ((875*100000 >= Valid_Freq)||(1080*100000 <= Valid_Freq))//设置频点是否合理
		{
			HW_TEST_PRINT("set freq error!\r\n");
			return AK_FALSE;
		}
		
		//RDA5802_SetFreq(Valid_Freq/100000);
	}
	else
	{
        if ((875 >= appoint_freq[channel_num-1])||(1080 <= appoint_freq[channel_num-1]))//设置频点是否合理
		{
			HW_TEST_PRINT("appoint freq error!%d\r\n",appoint_freq[channel_num-1]);
			return AK_FALSE;
		}
        
		//RDA5802_SetFreq(appoint_freq[channel_num-1]);
        //HW_TEST_PRINT("Appoint frep:%d\r\n",appoint_freq[channel_num-1]);
	}

	Fwl_RadioSetVolume(TEST_VOLUME);
	Fwl_DelayMs(TEST_TIME);
	
	Fwl_RadioFree();
	
	HW_TEST_PRINT("FM Test ok!\r\n\n");
	return AK_TRUE;
}

static T_BOOL LINEIN_Test(void)
{
    T_U8 wave_id = 0xff;

    wave_id = Fwl_WaveOutOpen(LINEIN_HP, AK_NULL);
    
    if (INVAL_WAVEOUT_ID == wave_id)
    {
        return AK_FALSE;
    }
    Fwl_WaveOutSetGain(wave_id, TEST_VOLUME);
    
    Fwl_DelayMs(TEST_TIME);

    Fwl_WaveOutClose(wave_id);
    
    HW_TEST_PRINT("LINEIN Test ok!\r\n\n");
    return AK_TRUE;
}

static T_BOOL REC_Test(void)
{
    T_U8 wave_id = 0xff;
    
    wave_id = Fwl_WaveOutOpen(MIC_HP, AK_NULL);
    
    if (INVAL_WAVEOUT_ID == wave_id)
    {
        return AK_FALSE;
    }
    Fwl_WaveOutSetGain(wave_id, TEST_VOLUME);
    
    Fwl_DelayMs(TEST_TIME);

    Fwl_WaveOutClose(wave_id);
    
    HW_TEST_PRINT("REC Test ok!\r\n\n");
    return AK_TRUE;
}

static T_BOOL  USBHOST_Test(void)
{
#ifdef SUPPORT_USBHOST
    Fwl_DetectorEnable(DEVICE_UHOST, AK_TRUE);
#endif
	HW_TEST_PRINT("USBHOST Test ok!\r\n\n");
    return AK_TRUE;
}

static T_BOOL Gpio_Test(void)
{
	T_BOOL ret = AK_FALSE;

	HW_TEST_PRINT("GPIO connect Test\r\n\n");
	//add Gpio test Case here:

	ret = AK_TRUE;
	return ret;
}


static T_BOOL doTestAsCfg(T_S8 *s)
{
	const char *KeyStr_GPIO    = "BT_SPEKER_GPIO";
	const char *KeyStr_UART    = "BT_SPEKER_BT";
	const char *KeyStr_SD 	   = "BT_SPEKER_SD";
	const char *KeyStr_FM 	   = "BT_SPEKER_FM";
    const char *KeyStr_LINEIN  = "BT_SPEKER_LINEIN";
    const char *KeyStr_REC     = "BT_SPEKER_REC";
    const char *KeyStr_USBHOST = "BT_SPEKER_USBHOST";
    
	T_BOOL TestResult = AK_TRUE;
				
	if(strstr(s, KeyStr_GPIO))
	{
        TestResult = Gpio_Test();
	}
#ifdef SUPPORT_BLUETOOTH
	else if(strstr(s, KeyStr_UART))
	{
        TestResult = BtDev_HwTest("BtDevTest", TEST_TIMEOUT, TEST_TIME);
	}
#endif
	else if(strstr(s,KeyStr_SD))
	{
        TestResult = SD_Test();
	}
	else if(strstr(s,KeyStr_FM))
	{
        TestResult = FM_Test();
	}
    else if(strstr(s,KeyStr_LINEIN))
	{
        TestResult = LINEIN_Test();
	}
    else if(strstr(s,KeyStr_REC))
    {
        TestResult = REC_Test();
    }
    else if(strstr(s,KeyStr_USBHOST))
    {
        TestResult = USBHOST_Test();
    }

	return TestResult;
}




/***********************************************************************************
**name:     	HWTest()    
**brief: 	      hardware Test using in factory
**param:	void
**return:	void  
**author:	huangxiaoqing
**date:		2012-05-09
***********************************************************************************/
void HWTest(void)
{
    T_U8 item[40] = {0};
    T_U8 cfg[40] = {0};
    T_U8 *pcur = AK_NULL;
	const char *Str_FM = "BT_SPEKER_FM";
    
	akerror("Enter Hardware test \r\n", 0, 1);

	if(!env_construct())   
	{
	    return;
	}

    pcur = AK_NULL;
    
    while(AK_NULL != (pcur = env_get_next_line(pcur)))
    {
        if (!env_read_cfg(pcur, AK_NULL, item, sizeof(item)))
        {
            continue;
        }
        
        env_read_cfg(pcur, item, cfg, sizeof(cfg));
        akerror(item, 0, 0);
        akerror(":", 0, 0);
        akerror(cfg, 0, 1);
        
        if(strstr(item,Str_FM))
	    {
            pcur = env_get_next_line(pcur);
            pcur = env_get_next_line(pcur);
	    }
        
        if (strchr(cfg, 'Y')||strchr(cfg, 'y'))
        {
           if (!doTestAsCfg(item))
           {
				AkDebugOutput("All Test fail!\n");
                while(1)
                {					
	                Fwl_LEDOff(LED_BLUE);
	                Fwl_LEDOn(LED_RED); //测试失败
                }
           }
        }
     }
    
    env_destory();
	AkDebugOutput("All Test Pass!\n");
	while(1)
	{
	    Fwl_LEDOff(LED_RED);
	    Fwl_LEDOn(LED_BLUE);
	}
}
#endif 
