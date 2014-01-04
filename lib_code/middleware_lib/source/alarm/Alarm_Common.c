/************************************************************************
 * Copyright (c) Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * All rights reserved.
 *
 * @FILENAME Alarm_Common.c
 * @BRIEF Alarm common function
 * @Author：zhao_xiaowei 
 * @Date： 2009-9
 * @Version：
**************************************************************************/
#include "Gbl_Global.h"
#include <string.h>
#include "Alarm_Common.h"
#include "Log_aud_play.h"
#include "Fwl_System.h"
#include "media_lib.h"
#include "log_radio_core.h"

#if(NO_DISPLAY == 0)

#define   DEGUE_OUT            1
typedef enum
{
    VOICE_NULL,
    VOICE_PLAY,
    VOICE_PAUSE,
    VOICE_STOP,
    VOICE_PRE_STOP,
    VOICE_PRE_PAUSE
}T_VOICE_STATE;

typedef struct 
{
    T_BOOL isCyclePlay;
    T_U8 volume;
    T_U8 eq ;
    T_U8 speed;
    T_U16 fileName[MAX_FILE_LEN+1];
    T_U32 fadeInTime;
    T_U32 fadeOutTime;
    T_BOOL isReConfig;
}T_VoiceParam;
typedef struct 
{
    T_AUD_PLAYER* pAudPlayer;
    T_VOICE_STATE playState;
    T_BOOL       isPlayEnd;
    T_BOOL       bFade; // current fade
    T_BOOL       isCyclePlay;//current cycle
    T_BOOL       isSetCpu2x;
    T_BOOL       isPushFreq;
    T_VoiceParam voiceParam;
}T_CklPlayer;

#pragma arm section zidata = "_bootbss_" 
static T_CklPlayer* m_pClkPlayer= AK_NULL;
#pragma arm section zidata 

extern volatile T_U8 send_flag;

/**************************************************************************
* @brief music play freq set
* @author zhao_xiaowei
* @date 2009-9
* @param bPlay AK_TURE ,Push the calculated freq, AK_FALSE Pop Freq
* @return 
***************************************************************************/
T_BOOL AlmClk_PlayerFreqSet(T_BOOL bPush);

T_AUD_PLAYER* AlmClk_PlayerHandlePre(T_VOID)
{
   if(AK_NULL == m_pClkPlayer)
   {
       return AK_NULL;
   }
   else
   {
       if(VOICE_PRE_PAUSE == m_pClkPlayer->playState 
           || VOICE_PRE_STOP == m_pClkPlayer->playState
           || VOICE_PLAY == m_pClkPlayer->playState)
       {
            return m_pClkPlayer->pAudPlayer;
       }
       else
       {
           return AK_NULL;
       }
   }
}

static T_VOID AlmClk_PlayerStopReal(T_VOID)
{
    APlayer_Stop(m_pClkPlayer->pAudPlayer);
    m_pClkPlayer->isPlayEnd= AK_TRUE;
    m_pClkPlayer->playState= VOICE_STOP;
    AlmClk_PlayerFreqSet(AK_FALSE);
}

static T_VOID AlmClk_PlayerPauseReal(T_VOID)
{
    m_pClkPlayer->playState= VOICE_PAUSE;
    m_pClkPlayer->isPlayEnd= AK_TRUE;
    AlmClk_PlayerFreqSet(AK_FALSE);
}

T_VOID AlmClk_PlayerDealDelay(T_VOID)
{
  if(AK_NULL != m_pClkPlayer)
  {
        if(VOICE_PRE_PAUSE == m_pClkPlayer->playState)
        {
            AlmClk_PlayerPauseReal();
#if(DEGUE_OUT)
            AK_DEBUG_OUTPUT("VOICE_PRE_PAUSE Delay\n");
#endif
            
        }
        else if(VOICE_PRE_STOP == m_pClkPlayer->playState)
        {
            AlmClk_PlayerStopReal();
#if(DEGUE_OUT)
            AK_DEBUG_OUTPUT("VOICE_PRE_STOP Delay\n");
#endif
        }
        else if( VOICE_PLAY == m_pClkPlayer->playState)
        {
            if(m_pClkPlayer->isCyclePlay)
            {
                AlmClk_PlayerNext();
#if(DEGUE_OUT)
                AK_DEBUG_OUTPUT("VOICE_PLAY Delay\n");
#endif
            }
            else
            {
                AlmClk_PlayerStopReal();
                AK_DEBUG_OUTPUT("VOICE_PLAY Delay Real");
            }
        }
        else
        {
#if(DEGUE_OUT)
            AK_DEBUG_OUTPUT("DealDelay StateNone\n");
#endif
        }
  }
  else
  {
#if(DEGUE_OUT)
      AK_DEBUG_OUTPUT("DealDelay AK_NULL\n");
#endif
  }
}

T_VOID AlmClk_PlayerHandlePost(T_S32 size , T_AUD_PLAYER* pAudPlayer)
{
	//handle decode result
	if (size < 0)
	{	
//	    if (Fwl_DMA_Ability_IsEnabled(FWL_DMA_DAC))
		if (Pcm_CheckPause())
		{
//			Fwl_DMA_Ability_Enable(FWL_DMA_DAC, AK_FALSE);
			AlmClk_PlayerDealDelay();
		}
		
	}   
}

static T_VOID AlmClk_PlayerInit(T_VOID)
{   
    m_pClkPlayer= (T_CklPlayer* )Fwl_Malloc(sizeof(T_CklPlayer));
    AK_ASSERT_PTR_VOID(m_pClkPlayer, "AlmClk_PlayerInit is AK_NULL");
    
    m_pClkPlayer->pAudPlayer= APlayer_Init(15, NORMAL_SPEED, NORMAL_EQ);
	memset(&m_pClkPlayer->voiceParam, 0, sizeof(m_pClkPlayer->voiceParam));

	MediaLib_SetCPULevel(AK_FALSE);	

    APlayer_RegHDLPre(AlmClk_PlayerHandlePre);
    APlayer_RegHDLPost(AlmClk_PlayerHandlePost);  
    m_pClkPlayer->playState= VOICE_NULL;

    m_pClkPlayer->isPushFreq= AK_FALSE;
    m_pClkPlayer->isSetCpu2x= AK_FALSE;

	/*if(Aud_AudCtrlGetHpState() || IsInRadio())
	{
		m_pClkPlayer->pAudPlayer->flagHDPopen = AK_TRUE;
	}
	else
	{
		m_pClkPlayer->pAudPlayer->flagHDPopen = AK_FALSE;
	}*/
    AK_PRINTK("***AlmClk_PlayerInit", 0, 1);
}

T_BOOL AlmClk_PlayerSet(T_U16* fileName, T_BOOL isCyclePlay, T_U8 volume, T_U8 eq , T_U8 speed,T_U32 fideInTime, T_U32 fideOutTime)
{
    AK_ASSERT_PTR(m_pClkPlayer, "AlmClk_PlayerSet AK_NULL", AK_FALSE);
    if(Utl_UStrCmp(fileName, m_pClkPlayer->voiceParam.fileName) == 0)
    {
        m_pClkPlayer->voiceParam.isReConfig= AK_FALSE;
    }
	else
	{
		 Utl_UStrCpy(m_pClkPlayer->voiceParam.fileName, fileName);
		 m_pClkPlayer->voiceParam.isReConfig= AK_TRUE;
	}

	if(m_pClkPlayer->voiceParam.volume != volume
		|| m_pClkPlayer->voiceParam.eq != eq
		|| m_pClkPlayer->voiceParam.speed != speed)
	{
		m_pClkPlayer->voiceParam.isReConfig= AK_TRUE;
	}
    m_pClkPlayer->voiceParam.isCyclePlay= isCyclePlay;
    m_pClkPlayer->voiceParam.volume= volume;
    m_pClkPlayer->voiceParam.eq= eq;
    m_pClkPlayer->voiceParam.speed= speed;
    m_pClkPlayer->voiceParam.fadeInTime= fideInTime;
    m_pClkPlayer->voiceParam.fadeOutTime= fideOutTime;
    return AK_TRUE;
}

static T_BOOL AlmClk_PlayerParam(T_VoiceParam *pVoiceParam)
{
    m_pClkPlayer->bFade= (T_BOOL)(pVoiceParam->fadeInTime != 0 
        || pVoiceParam->fadeOutTime != 0);
    if(!APlayer_Open(m_pClkPlayer->pAudPlayer, pVoiceParam->fileName
        , MEDIALIB_MEDIA_UNKNOWN, 
        m_pClkPlayer->bFade, pVoiceParam->fadeInTime,
        pVoiceParam->fadeOutTime,(T_WSOLA_ARITHMATIC)AUD_SPEED_ARITHMETIC))
    {
        return AK_FALSE;
    }

    m_pClkPlayer->playState= VOICE_PLAY;//first set the state to VOICE_PLAY to set freq
    //Aud_PlayerOpenHP(m_pClkPlayer->pAudPlayer);
    AlmClk_PlayerFreqSet(AK_TRUE);
    m_pClkPlayer->isCyclePlay= pVoiceParam->isCyclePlay;

        
    m_pClkPlayer->isPlayEnd= AK_FALSE;
    return APlayer_Play(m_pClkPlayer->pAudPlayer);
}

extern T_VOID ClkPlay_DealError(T_VOID);
extern T_VOID ClkPlay_TriggerSDOut(T_VOID);
T_BOOL AlmClk_PlayerNext(T_VOID)
{
	T_BOOL state;
    AK_ASSERT_PTR(m_pClkPlayer, "AlmClk_PlayerNext AK_NULL", AK_FALSE);
    if(m_pClkPlayer->playState != VOICE_NULL && m_pClkPlayer->playState != VOICE_STOP)
    {
        APlayer_Stop(m_pClkPlayer->pAudPlayer);
    }

	state= AlmClk_PlayerParam(&m_pClkPlayer->voiceParam);
    if(!state)
    {
   #if(USE_ALARM_CLOCK)        
        {
            T_BOOL flag = AK_FALSE;
            if(!Fwl_MemDevIsMount(MMC_SD_CARD))
            {
                ClkPlay_TriggerSDOut();
                flag = AK_TRUE;
            }
            if(AK_FALSE == flag)
            {
                ClkPlay_DealError();
            }
        }
    #endif
    }
	return state;
}



T_BOOL AlmClk_PlayerStart(T_U16* fileName, T_BOOL isCyclePlay, T_U8 volume, T_U8 eq , T_U8 speed,T_U32 fideInTime, T_U32 fideOutTime)
{
    if(AK_NULL == m_pClkPlayer)
    {
        AlmClk_PlayerInit();
    }
    AlmClk_PlayerSet(fileName, isCyclePlay, volume, eq, speed, fideInTime, fideOutTime);
    return AlmClk_PlayerParam(&m_pClkPlayer->voiceParam);
}


T_VOID AlmClk_PlayerPause(T_VOID)
{
    AK_ASSERT_PTR_VOID(m_pClkPlayer, "AlmClk_PlayerPause AK_NULL");

    if(VOICE_PLAY == m_pClkPlayer->playState)
    {
        APlayer_Pause(m_pClkPlayer->pAudPlayer);
        if(m_pClkPlayer->bFade)
        {
            m_pClkPlayer->playState= VOICE_PRE_PAUSE;
            m_pClkPlayer->isPlayEnd= AK_FALSE;
        }
        else
        {
            AlmClk_PlayerPauseReal();
        }
    }

}

T_VOID AlmClk_PlayerResume(T_VOID)
{
     AK_ASSERT_PTR_VOID(m_pClkPlayer, "AlmClk_PlayerResume AK_NULL");

     if(VOICE_PAUSE == m_pClkPlayer->playState || VOICE_PRE_PAUSE == m_pClkPlayer->playState)
     {
           m_pClkPlayer->playState= VOICE_PLAY;
           m_pClkPlayer->isPlayEnd= AK_FALSE;
           AlmClk_PlayerFreqSet(AK_TRUE);
           APlayer_Resume(m_pClkPlayer->pAudPlayer);
     }
}

T_BOOL AlmClk_IsRealPlaying(T_VOID)
{
    if(AK_NULL == m_pClkPlayer)
    {
        return AK_FALSE;
    }
    else
    {
        return (T_BOOL)(VOICE_PRE_PAUSE == m_pClkPlayer->playState 
           || VOICE_PRE_STOP == m_pClkPlayer->playState
           || VOICE_PLAY == m_pClkPlayer->playState
            );
    }
}

T_VOID AlmClk_SetCpu2xManual(T_BOOL bSet)
{

	if(m_pClkPlayer != AK_NULL)
	{
		if(!m_pClkPlayer->isSetCpu2x)
		{
			if (bSet)
				Fwl_Freq_Add2Calc(FREQ_VAL_1);
			else
				Fwl_Freq_Clr_Add();
		}
	}

}

T_BOOL AlmClk_PlayerFreqSet(T_BOOL bPush)
{
    if(AK_NULL == m_pClkPlayer)
    {
        AK_PRINTK("Alm Clk not init", 0, 1);
        return AK_FALSE;
    }
    if(bPush)
    {
        T_FREQ_APP freq= FREQ_APP_DEFAULT;

		if(m_pClkPlayer->isSetCpu2x)
		{
			AK_PRINTK("***AlmClk has Set Cpu 2X", 0, 1);
			Fwl_Freq_Clr_Add();
		}

		m_pClkPlayer->isSetCpu2x= AK_FALSE;
		
        if(m_pClkPlayer->isPushFreq)
        {
            AK_PRINTK("***AlmClk has Pushed Freq", 0 ,1);
            Fwl_FreqPop();
        }
   
         m_pClkPlayer->isPushFreq= AK_TRUE;
 
 
		switch(m_pClkPlayer->pAudPlayer->audType)
	    {
	        case _SD_MEDIA_TYPE_MP3:
			case _SD_MEDIA_TYPE_ADPCM_FLASH:
			case _SD_MEDIA_TYPE_ADPCM_IMA:
			case _SD_MEDIA_TYPE_ADPCM_MS:
			case _SD_MEDIA_TYPE_PCM:
	            break;
			case _SD_MEDIA_TYPE_WMA:
				 m_pClkPlayer->isSetCpu2x= AK_TRUE;
				 break;
	        default:
				 m_pClkPlayer->isSetCpu2x= AK_TRUE;
				 freq= FREQ_APP_AUDIO_L5;
	            break;
	    }
        Fwl_FreqPush(freq);

        if(m_pClkPlayer->isSetCpu2x)
        {
            #ifdef OS_ANYKA
            AK_PRINTK("AlmClk setCpu2X", 0, 1);
            #endif
            Fwl_Freq_Add2Calc(FREQ_VAL_1);
        }
    }
    else
    {
        if(m_pClkPlayer->isSetCpu2x)
        {
            #ifdef OS_ANYKA
             AK_PRINTK("AlmClk Re setCpu2X", 0, 1);
            #endif
            Fwl_Freq_Clr_Add();
            m_pClkPlayer->isSetCpu2x= AK_FALSE;
        }

        if(m_pClkPlayer->isPushFreq)
        {
            m_pClkPlayer->isPushFreq= AK_FALSE;
            AK_PRINTK("***alarm freq pop", 0, 1);
            Fwl_FreqPop();
        }
        else
        {
            AK_PRINTK("***AlmClk has Freq pop", 0,1);
        }
        
    }
    return AK_TRUE;
}

T_VOID AlmClk_PlayerStop(T_BOOL isForceStop)
{
     AK_ASSERT_PTR_VOID(m_pClkPlayer, "AlmClk_PlayerStop AK_NULL");
     if(VOICE_PLAY == m_pClkPlayer->playState)
     {
         APlayer_Pause(m_pClkPlayer->pAudPlayer);
         if(!isForceStop && m_pClkPlayer->bFade)
         {
             // its state is pause
             m_pClkPlayer->isPlayEnd= AK_FALSE;  //if delay then wait voice stop
             m_pClkPlayer->playState= VOICE_PRE_STOP;
#if DEGUE_OUT
             AK_DEBUG_OUTPUT("AlmClk_PlayerStop PreStop\n");
#endif
         }
         else
         {
            AlmClk_PlayerStopReal();
         }                      
     }
     else if(VOICE_PAUSE == m_pClkPlayer->playState)
     {
         APlayer_Stop(m_pClkPlayer->pAudPlayer); //if it's pause ,just stop,because pause state has pop stack ,so not repop
         m_pClkPlayer->playState= VOICE_STOP;
          m_pClkPlayer->isPlayEnd= AK_TRUE;
     }
     else if(VOICE_PRE_PAUSE == m_pClkPlayer->playState)
     {
         m_pClkPlayer->playState= VOICE_PRE_STOP;
     }
}


T_VOID AlmClk_PlayerDestroy(T_VOID)
{
    if(m_pClkPlayer != AK_NULL)
    {
        if(m_pClkPlayer->playState != VOICE_STOP && m_pClkPlayer->playState != VOICE_NULL)//if it has stoped
        {
            APlayer_Stop(m_pClkPlayer->pAudPlayer);// force stop ,not first pause and wait the voice play end
            
            if(VOICE_PAUSE != m_pClkPlayer->playState)
                AlmClk_PlayerFreqSet(AK_FALSE);
            m_pClkPlayer->playState= VOICE_NULL;
           AK_DEBUG_OUTPUT("AlmClk_PlayerDestroy state:%d\n", m_pClkPlayer->playState);
        }
		
		Pcm_Pause(m_pClkPlayer->pAudPlayer->pcm_id);
        APlayer_Destroy(m_pClkPlayer->pAudPlayer);
        //关闭DA之前需要等待L2数据传输完毕
		//while( CHK_SENDFLAG(send_flag, L2_SENDING));
		
//	    if (Fwl_DMA_Ability_IsEnabled(FWL_DMA_DAC))
		{
//			Fwl_DMA_Ability_Enable(FWL_DMA_DAC, AK_FALSE);
		}

        m_pClkPlayer= (T_CklPlayer*)Fwl_Free(m_pClkPlayer);
    }
    
}


T_BOOL AlmClk_IsPlayerStopped(T_VOID)
{
    if(AK_NULL == m_pClkPlayer)
    {
        return AK_TRUE;
    }
    else
    {
        return m_pClkPlayer->isPlayEnd;
    }
}

#pragma arm section code = "_frequentcode_"
T_BOOL IsInVoicePlay(T_VOID)
{
    return (T_BOOL)(m_pClkPlayer != AK_NULL);
}
#pragma arm section code 

#ifdef SUPPORT_SDCARD
T_MEM_DEV_ID VoiceGetCurDriver(T_VOID)
{
    if( m_pClkPlayer != AK_NULL)
    {
        return Fwl_MemDevGetDriver(m_pClkPlayer->voiceParam.fileName[0]);
    }
	return SYSTEM_STORAGE;
}
#endif

#endif //#if(NO_DISPLAY == 0)

	
