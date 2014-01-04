/**
 * @file Eng_Profile.c
 * @brief User Profile file Read/Write
 *
 * @author xie_wenzhong
 * @date 2013-05-16
 * @version 1.0
 */

#include "Apl_Public.h"
#include "Eng_Profile.h"
#include "Fwl_osFS.h"
#include "Eng_DataConvert.h"
#include "Fwl_FreqMgr.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "log_aud_play.h"
#include "Gbl_Global.h"
#include "Fwl_Sysdata.h"
#include "Eng_BtCtrl.h"

#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD )
const T_U16 profile[] = {'A',':','/','S','Y','S','T','E','M','/',
                    'p','r','o','f','i','l','e','.','d','a','t','\0'};
#elif(STORAGE_USED == SPI_FLASH)
const T_S8 profile[] = "PROFILE";
#endif

extern const T_CONFIG_INFO gb_sysconfig;
extern const T_U16 defsystem[];

#define SYS_DEFVOLUME   (15)
#define FM_DEFVOLUME    (5)

#define GET_CHECKCODE_SYSCFG(pSysCfg, Check) do { \
        (Check)  = (T_U8)((T_U8)((pSysCfg)->Lang) ^ (T_U8)((pSysCfg)->LcdContrast));     \
        (Check) ^= (T_U8)((T_U8)((pSysCfg)->PoffTime) ^ (T_U8)((pSysCfg)->BgLightTime)); \
        (Check) ^= (T_U8)((T_U8)((pSysCfg)->PowerOffFlag) ^ (T_U8)((pSysCfg)->ConnectMode)); \
    } while (0)
    
#define GET_CHECKCODE_SYSTMCFG(pSysCfg, Check){ \
    (Check)= (T_U8)((T_U8)(pSysCfg->systime>>24) ^ (T_U8)(pSysCfg->systime<<8>>24)); \
    (Check)= (T_U8)((T_U8)(pSysCfg->systime<<16>>24) ^ (T_U8)(pSysCfg->systime<<24>>24)); \
}  

#define GET_CHECKCODE_RECCFG(pRecCfg, Check) do { \
        (Check) = (T_U8)((T_U8)((pRecCfg)->recMode) ^ (T_U8)((pRecCfg)->radioRecMode)); \
    } while (0)

#define GET_CHECKCODE_RADCFG(pRadCfg, Check) do { \
        (Check)  = (T_U8)((T_U8)((pRadCfg)->CurFreq) ^ (T_U8)((pRadCfg)->SavedFreq));\
        (Check) ^= (T_U8)((T_U8)((pRadCfg)->RadioArea) ^ (T_U8)((pRadCfg)->Volume) ^(T_U8)((pRadCfg)->prePos)); \
    } while (0)

#define GET_CHECKCODE_AUDCFG(pAudCfg, Check) do { \
        (Check)  = (T_U8)((T_U8)((pAudCfg)->abMode) ^ (T_U8)((pAudCfg)->speed)); \
        (Check) ^= (T_U8)((T_U8)((pAudCfg)->volume) ^ (T_U8)((pAudCfg)->abSpac));\
    } while (0)
    
#define GET_CHECKCODE_BTDEVCFG(pBtCfg, Check) do { \
			(Check)  = (T_U8)((T_U8)((pBtCfg)->localInfo.info.BD_ADDR[1]) ^ (T_U8)((pBtCfg)->localInfo.info.BD_ADDR[3])); \
			(Check) ^= (T_U8)((T_U8)((pBtCfg)->localInfo.info.BD_ADDR[5]) ^ (T_U8)((pBtCfg)->pairedList[0].classofDevice));\
		} while (0)

static const T_U16 gCfgItemSize[eCFG_NUM] = {
	sizeof(T_SYSTEM_CFG),
	sizeof(T_SYSTIME_CFG),
	sizeof(T_RECORD_CFG),
	sizeof(T_AUDIO_CFG),
	sizeof(T_AUDIO_CFG),
	sizeof(T_AUDIO_CFG),
	sizeof(T_RADIO_CFG),
	sizeof(T_AUDIO_CFG),
	sizeof(T_AUDIO_CFG),
	#ifdef SUPPORT_BLUETOOTH
	sizeof(T_BTDEV_CFG),
	#endif
	// ... ...
};
T_VOID getCheckCode(T_eCFG_ITEM item,T_pVOID buf)
{
	switch (item)
	{
	case eCFG_SYSTEM:
		{
			T_SYSTEM_CFG * data = (T_SYSTEM_CFG *)buf;
			GET_CHECKCODE_SYSCFG(data, data->CheckCode);
		}
		break;
		
	case eCFG_SYSTIME:
		{
			T_SYSTIME_CFG * data = (T_SYSTIME_CFG *)buf;
			GET_CHECKCODE_SYSTMCFG(data, data->CheckCode);		
		}
		break;
		
	case eCFG_REC:
		{
			T_RECORD_CFG * data = (T_RECORD_CFG *)buf;
			GET_CHECKCODE_RECCFG(data, data->CheckCode);
		}
		break;
		
	case eCFG_AUDIO:
	case eCFG_SDAUDIO:
	case eCFG_USBAUDIO:	
	case eCFG_SDVOICE:
	case eCFG_VOICE:
		{
			T_AUDIO_CFG * data = (T_AUDIO_CFG *)buf;
			GET_CHECKCODE_AUDCFG(data, data->CheckCode);
		}
		break;
		
	case eCFG_RADIO:
		{
			T_RADIO_CFG * data = (T_RADIO_CFG *)buf;
			GET_CHECKCODE_RADCFG(data, data->CheckCode);
		}
		break;
#ifdef SUPPORT_BLUETOOTH
	case eCFG_BTDEV:
		{
			T_BTDEV_CFG * data = (T_BTDEV_CFG *)buf;
			GET_CHECKCODE_BTDEVCFG(data, data->CheckCode);
		}
		break;
#endif
	default:
		break;
	}
}

static T_U16 getOffset(T_eCFG_ITEM item)
{
	T_U16 offset = 0;
	T_U16 i;

	for (i = 0; i < item; ++i)
		offset += gCfgItemSize[i];

	return offset;
}

#define getItemSize(item) gCfgItemSize[item]

#if (STORAGE_USED == SPI_FLASH)

static T_U8* gCfgUserData[eCFG_NUM];

static T_pVOID Profile_initData(T_VOID)
{
	T_U16 i;
	
	gCfgUserData[0] = Fwl_Malloc(getOffset(eCFG_NUM));
	if (AK_NULL == gCfgUserData[0])
		return AK_NULL;
	
	for (i = 1; i < eCFG_NUM; ++i)
	{
		gCfgUserData[i] = gCfgUserData[0] + getOffset(i);
	}

	return gCfgUserData[0];
}

static T_VOID Profile_freeData(T_VOID)
{
	gCfgUserData[0] = Fwl_Free(gCfgUserData[0]);
	memset(gCfgUserData, 0, sizeof(gCfgUserData));
}
#endif	// End of #if (STORAGE_USED == SPI_FLASH)

/******************************************************************************
* @NAME    Profile_CheckData 
* @BRIEF   Check User Profile Data
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the Bufffer Will Be Checked  
*          
* @RETURN  AK_TRUE Is OK; AK_FALSE Is ERROR and than Will Load Default Data
*   
*******************************************************************************/
T_BOOL Profile_CheckData(T_eCFG_ITEM item, T_pVOID buff, T_BOOL ldDefault)
{
	T_U8 Check;

	switch (item)
	{
	case eCFG_SYSTEM:
		{
	        T_SYSTEM_CFG *pSysCfg = (T_SYSTEM_CFG*)buff;
	        GET_CHECKCODE_SYSCFG(pSysCfg, Check);
			
	        if (ldDefault || Check != pSysCfg->CheckCode)
	        {
	            pSysCfg->Lang        = (T_RES_LANGUAGE)gb_sysconfig.language_set.Language_default;
	            pSysCfg->LcdContrast = (T_U8)gb_sysconfig.system_param.contrast_degree;
	            pSysCfg->PoffTime    = (T_U8)gb_sysconfig.system_param.close_time;
	            pSysCfg->PoffTimeSleepMode = (T_U16)gb_sysconfig.system_param.sleep_time* 60;
	            pSysCfg->BgLightTime = (T_U8)gb_sysconfig.system_param.background_time;
	            pSysCfg->ConnectMode = eRES_STR_MULTI_DRIVE;
	            pSysCfg->PowerOffFlag = POWEROFF_NORMAL;
				
	            GET_CHECKCODE_SYSCFG(pSysCfg, pSysCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default SYS Data\n");
				return AK_FALSE;
	        }

			return AK_TRUE;
	    }
		break;
		
	case eCFG_SYSTIME:
		{
			T_SYSTIME date;
            T_SYSTIME_CFG* pSystimeCfg = (T_SYSTIME_CFG*)buff; 

			GET_CHECKCODE_SYSTMCFG(pSystimeCfg, Check);
			
            if (ldDefault || Check != pSystimeCfg->CheckCode)
            {
	            SetDefDate(date);
	            pSystimeCfg->systime = Utl_Convert_DateToSecond(&date);
	            // default time: 2009-01-01 00:00:00
	            //pSystimeCfg->systime = 0x368d6180U;
	            
	            GET_CHECKCODE_SYSTMCFG(pSystimeCfg, pSystimeCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default SYSTIME Data\n");
				return AK_FALSE;
            }

			return AK_TRUE;
        }		
		break;
		
	case eCFG_REC:
		{
            T_RECORD_CFG *pRecCfg = (T_RECORD_CFG*)buff;
			GET_CHECKCODE_RECCFG(pRecCfg, Check);
			
            if (ldDefault || Check != pRecCfg->CheckCode)
            {
	            pRecCfg->recMode      = eREC_MODE_ADPCM8K_2;
	            pRecCfg->radioRecMode = eREC_MODE_MP3HI;
	            pRecCfg->isVorRec     = AK_FALSE;
				
	            GET_CHECKCODE_RECCFG(pRecCfg, pRecCfg->CheckCode);
				
	            Utl_UStrCpyN(pRecCfg->defaultPath, defrecord, (T_U32)(MAX_FILE_LEN - 1));
	            Utl_UStrCpyN(pRecCfg->radioDefaultPath, defrecord, (T_U32)(MAX_FILE_LEN - 1));
				// AK_DEBUG_OUTPUT("Load Default REC Data\n");
				return AK_FALSE;
            }

			return AK_TRUE;
        }		
		break;
		
	case eCFG_AUDIO:
		{			
			T_AUDIO_CFG *pAudCfg = (T_AUDIO_CFG*)buff;
			GET_CHECKCODE_AUDCFG(pAudCfg, Check);
			
            if (ldDefault || Check != pAudCfg->CheckCode)
            {
				pAudCfg->abMode 	   = 0;
				pAudCfg->musicCurIdx   = 0;
				pAudCfg->musicClassIdx = 0;
				pAudCfg->musicTolIdx   = 0;
				pAudCfg->curTimeMedia  = 0;
				pAudCfg->repMode	   = 0;
				pAudCfg->speed		   = AUD_PLAY_NORMAL_SPEED;
				pAudCfg->toneMode	   = 0;
				pAudCfg->volume 	   = SYS_DEFVOLUME;
				pAudCfg->abRep		   = 5;
				pAudCfg->abSpac 	   = 2;
				pAudCfg->cycMode	   = AUD_CYC_MODE_TOTAL;
				pAudCfg->listUpdated   = 0;
				pAudCfg->abAutoRepMode = 0;
				pAudCfg->ListStyle	   = eAUD_MUSIC;
				pAudCfg->seeklenId	   = 0;
				
				GET_CHECKCODE_AUDCFG(pAudCfg, pAudCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default AUDIO Data\n");
				return AK_FALSE;
            }

			return AK_TRUE;
		}		
		break;
		
	case eCFG_SDAUDIO:
		{			
			T_AUDIO_CFG *pAudCfg = (T_AUDIO_CFG*)buff;
			GET_CHECKCODE_AUDCFG(pAudCfg, Check);
			
            if (ldDefault || Check != pAudCfg->CheckCode)
            {
				pAudCfg->abMode 	   = 0;
				pAudCfg->musicCurIdx   = 0;
				pAudCfg->musicClassIdx = 0;
				pAudCfg->musicTolIdx   = 0;
				pAudCfg->curTimeMedia  = 0;
				pAudCfg->repMode	   = 0;
				pAudCfg->speed		   = AUD_PLAY_NORMAL_SPEED;
				pAudCfg->toneMode	   = 0;
				pAudCfg->volume 	   = SYS_DEFVOLUME;
				pAudCfg->abRep		   = 5;
				pAudCfg->abSpac 	   = 2;
				pAudCfg->cycMode	   = AUD_CYC_MODE_TOTAL;
				pAudCfg->listUpdated   = 0;
				pAudCfg->abAutoRepMode = 0;
				pAudCfg->ListStyle	   = eAUD_SDMUSIC;
				pAudCfg->seeklenId	   = 0;
				
				GET_CHECKCODE_AUDCFG(pAudCfg, pAudCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default SDAUDIO Data\n");
				return AK_FALSE;
            }

			return AK_TRUE;
		}		
		break;
		
	case eCFG_USBAUDIO:
		{			
			T_AUDIO_CFG *pAudCfg = (T_AUDIO_CFG*)buff;
			GET_CHECKCODE_AUDCFG(pAudCfg, Check);
			
            if (ldDefault || Check != pAudCfg->CheckCode)
            {
				pAudCfg->abMode 	   = 0;
				pAudCfg->musicCurIdx   = 0;
				pAudCfg->musicClassIdx = 0;
				pAudCfg->musicTolIdx   = 0;
				pAudCfg->curTimeMedia  = 0;
				pAudCfg->repMode	   = 0;
				pAudCfg->speed		   = AUD_PLAY_NORMAL_SPEED;
				pAudCfg->toneMode	   = 0;
				pAudCfg->volume 	   = SYS_DEFVOLUME;
				pAudCfg->abRep		   = 5;
				pAudCfg->abSpac 	   = 2;
				pAudCfg->cycMode	   = AUD_CYC_MODE_TOTAL;
				pAudCfg->listUpdated   = 0;
				pAudCfg->abAutoRepMode = 0;
				pAudCfg->ListStyle	   = eAUD_USBMUSIC;
				pAudCfg->seeklenId	   = 0;
				
				GET_CHECKCODE_AUDCFG(pAudCfg, pAudCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default USBAUDIO Data\n");
				return AK_FALSE;
            }

			return AK_TRUE;
		}	
		break;
		
	case eCFG_RADIO:
		{
            T_U16 temp   = 0;
            T_RADIO_CFG *pRadCfg = (T_RADIO_CFG*)buff;
			GET_CHECKCODE_RADCFG(pRadCfg, Check);
			
            if (ldDefault || Check != pRadCfg->CheckCode)
            {
	            pRadCfg->CurFreq   = 87000000;
	            pRadCfg->SavedFreq = 76000000; 
	            pRadCfg->RadioArea = RADIO_EUROPE;
	            pRadCfg->Volume    = FM_DEFVOLUME;
	            pRadCfg->prePos    = (T_U8)(-1);
				
	            GET_CHECKCODE_RADCFG(pRadCfg, pRadCfg->CheckCode);
#ifdef SUPPORT_RADIO_RDA5876
	            for(temp = 0; temp < 20; temp++)
#else
                for(temp = 0; temp < 40; temp++)
#endif
	            {
	                pRadCfg->ChannelList[temp] = 0;
	            }
				
				// AK_DEBUG_OUTPUT("Load Default RADIO Data\n");
				
				return AK_FALSE;
            }

			return AK_TRUE;
        }
		break;
		
	case eCFG_VOICE:
		{			
			T_AUDIO_CFG *pAudCfg = (T_AUDIO_CFG*)buff;
			GET_CHECKCODE_AUDCFG(pAudCfg, Check);
			
            if (ldDefault || Check != pAudCfg->CheckCode)
            {
				pAudCfg->abMode 	   = 0;
				pAudCfg->musicCurIdx   = 0;
				pAudCfg->musicClassIdx = 0;
				pAudCfg->musicTolIdx   = 0;
				pAudCfg->curTimeMedia  = 0;
				pAudCfg->repMode	   = 0;
				pAudCfg->speed		   = AUD_PLAY_NORMAL_SPEED;
				pAudCfg->toneMode	   = 0;
				pAudCfg->volume 	   = SYS_DEFVOLUME;
				pAudCfg->abRep		   = 5;
				pAudCfg->abSpac 	   = 2;
				pAudCfg->cycMode	   = AUD_CYC_MODE_TOTAL;
				pAudCfg->listUpdated   = 0;
				pAudCfg->abAutoRepMode = 0;
				pAudCfg->ListStyle	   = eAUD_VOICE;
				pAudCfg->seeklenId	   = 0;
				
				GET_CHECKCODE_AUDCFG(pAudCfg, pAudCfg->CheckCode);
				
				// AK_DEBUG_OUTPUT("Load Default VOICE Data\n");
				
				return AK_FALSE;
            }

			return AK_TRUE;
		}	
		break;
		
	case eCFG_SDVOICE:
		{			
			T_AUDIO_CFG *pAudCfg = (T_AUDIO_CFG*)buff;
			GET_CHECKCODE_AUDCFG(pAudCfg, Check);
			
            if (ldDefault || Check != pAudCfg->CheckCode)
            {
				pAudCfg->abMode 	   = 0;
				pAudCfg->musicCurIdx   = 0;
				pAudCfg->musicClassIdx = 0;
				pAudCfg->musicTolIdx   = 0;
				pAudCfg->curTimeMedia  = 0;
				pAudCfg->repMode	   = 0;
				pAudCfg->speed		   = AUD_PLAY_NORMAL_SPEED;
				pAudCfg->toneMode	   = 0;
				pAudCfg->volume 	   = SYS_DEFVOLUME;
				pAudCfg->abRep		   = 5;
				pAudCfg->abSpac 	   = 2;
				pAudCfg->cycMode	   = AUD_CYC_MODE_TOTAL;
				pAudCfg->listUpdated   = 0;
				pAudCfg->abAutoRepMode = 0;
				pAudCfg->ListStyle	   = eAUD_SDVOICE;
				pAudCfg->seeklenId	   = 0;
				
				GET_CHECKCODE_AUDCFG(pAudCfg, pAudCfg->CheckCode);
				
				// AK_DEBUG_OUTPUT("Load Default SDVOICE Data\n");
				
				return AK_FALSE;
            }

			return AK_TRUE;
		}	
		break;
#ifdef SUPPORT_BLUETOOTH
	case eCFG_BTDEV:
		{			
			T_BTDEV_CFG *pBtCfg = (T_BTDEV_CFG*)buff;
			GET_CHECKCODE_BTDEVCFG(pBtCfg, Check);
			
            if (ldDefault || Check != pBtCfg->CheckCode)
            {
				memset(pBtCfg,0,sizeof(T_BTDEV_CFG));
				pBtCfg->a2dpCurVol = 1024;
				GET_CHECKCODE_BTDEVCFG(pBtCfg, pBtCfg->CheckCode);
				// AK_DEBUG_OUTPUT("Load Default BTDEV Data\n");
				
				return AK_FALSE;
            }

			return AK_TRUE;
		}		
		break;
#endif
	default:
		AK_DEBUG_OUTPUT("Check Data ERROR!!!\n");
		break;
	}

	return AK_FALSE;
}

#if(STORAGE_USED != SPI_FLASH)
/******************************************************************************
* @NAME    Profile_RdData_disk 
* @BRIEF   Read User Data From Profile File 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   offset: Offset of The Profile File
*                size: Read Buffer Size
*                buff: the buf Is Read From the Profile File 
*          
* @RETURN Read Successfully Buffer SIZE
*   
*******************************************************************************/
T_U16 Profile_RdData_disk(T_U16 offset, T_U16 size, T_pVOID buff)
{
	T_hFILE fd;
 
    fd = Fwl_FileOpen(profile, _FMODE_READ, _FMODE_READ);
    if (fd == _FOPEN_FAIL)
    {
    	AK_DEBUG_OUTPUT("rdData_disk, Open profile file Error\n");
        return 0;
    }

    Fwl_FileSeek(fd, offset, FS_SEEK_SET);
    size = Fwl_FileRead(fd, buff, size);
	Fwl_FileClose (fd);

	return size;
}

/******************************************************************************
* @NAME    Profile_WrData_disk 
* @BRIEF   Write User Data to File System 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   offset: Offset of The File
*                size: Write Buffer Size
*                buff: the buf Is Writed to Profile File 
*          
* @RETURN Write Successfully Buffer SIZE
*   
*******************************************************************************/
T_U16 Profile_WrData_disk(T_U16 offset, T_U16 size, T_pVOID buff)
{
	T_hFILE fd;

	fd = Fwl_FileOpen(profile, _FMODE_OVERLAY, _FMODE_OVERLAY);
    if (fd == _FOPEN_FAIL)
    {
    	T_U16 item;
        //make dir
        Fwl_FsMkDir(defsystem);
		
		// AK_DEBUG_OUTPUT("wrData_disk, Create profile File\n");
        fd = Fwl_FileOpen(profile, _FMODE_CREATE, _FMODE_CREATE);
        if (fd == _FOPEN_FAIL)
        {
        	AK_DEBUG_OUTPUT("wrData_disk, Open profile file Error\n");
            return 0;
        }
		
		for (item = 0; item < eCFG_NUM; ++item)
		{
			T_pDATA data = Fwl_Malloc(getItemSize(item));
			if (AK_NULL == data)
			{
				Fwl_FileClose(fd);
				AK_DEBUG_OUTPUT("Profile_WriteData, Malloc Error\n");
				return 0;
			}
			
			Profile_CheckData(item, data, AK_TRUE);
			Fwl_FileWrite(fd, data, getItemSize(item));
			Fwl_Free(data);
		}
    }

    Fwl_FileSeek(fd, offset, FS_SEEK_SET);
    size = Fwl_FileWrite(fd, buff, size);
	Fwl_FileClose(fd);

	return size;
}
#endif


/******************************************************************************
* @NAME    Profile_ReadData 
* @BRIEF   Read User Data From Profile file 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the buf Is Read From the Profile File 
*          
* @RETURN  AK_TRUE Is Success or AK_FALSE Is Failure
*   
*******************************************************************************/
T_BOOL Profile_ReadData(T_eCFG_ITEM item, T_pVOID buff)
{
	T_U16 len;

#if(STORAGE_USED == SPI_FLASH)
	T_S32 handle;

	handle = Fwl_Sysdata_Open(profile, AK_TRUE);
	len = Fwl_Sysdata_Read(handle, getOffset(item), getItemSize(item), buff);
	Fwl_Sysdata_Close(handle);
#else
	len = Profile_RdData_disk(getOffset(item), getItemSize(item), buff);
#endif

	if (Profile_CheckData(item, buff, AK_FALSE))
		return AK_TRUE;
	
	// AK_DEBUG_OUTPUT("Write Back ... ...\n");
	// Read Data Error, Write Back
	Profile_WriteData(item, buff);

	return AK_FALSE;
}

/******************************************************************************
* @NAME    Profile_WriteData 
* @BRIEF   Write User Data to Profile file 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the buf to Write the Profile File 
*          
* @RETURN  AK_TRUE Is Success or AK_FALSE Is Failure
*   
*******************************************************************************/
T_BOOL Profile_WriteData(T_eCFG_ITEM item, const T_pVOID buff)
{
#if(STORAGE_USED == SPI_FLASH)

	T_U16 size;
	T_S32 handle;

	if (!Profile_initData())
	{
		AK_DEBUG_OUTPUT("Init SPI Userdata Failure\n");
		return AK_FALSE;
	}
	
	handle = Fwl_Sysdata_Open(profile, AK_TRUE);
	
	Fwl_Sysdata_Read(handle, 0, getOffset(eCFG_NUM), gCfgUserData[0]);
	
	// Update Data Item
	getCheckCode(item,buff);//重新更新chechcode
	memcpy(gCfgUserData[item], buff, getItemSize(item));
	size = Fwl_Sysdata_Write(handle, 0, gCfgUserData[0], getOffset(eCFG_NUM));

	Fwl_Sysdata_Close(handle);
	Profile_freeData();

	// AK_DEBUG_OUTPUT("Write %d Bytes\n", size);

#else
	getCheckCode(item,buff);//重新更新chechcode
	Profile_WrData_disk(getOffset(item), getItemSize(item), buff);
#endif

	return AK_TRUE;
}



