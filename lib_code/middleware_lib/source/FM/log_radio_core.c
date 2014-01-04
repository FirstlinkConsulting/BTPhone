/******************************************************************************************
**FileName  	:      log__ebkcore.c
**brief        	:      read book data and display on screen
**Copyright 	:      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author 		:	Hongshi Yao
**date		      : 	2008-04-21
**version 	      :	0.0.1
*******************************************************************************************/
#include "Gbl_Global.h"
#include "Eng_Profile.h"
#include "Fwl_Radio.h"
#include "log_radio_core.h"
#include "Fwl_Timer.h"
#include "eng_debug.h"
//#include "Eng_AutoOff.h"
#include "Fwl_osMalloc.h"
#include "stdlib.h"
#include "Fwl_System.h"
#include "Fwl_WaveOut.h"
#ifdef SUPPORT_RADIO

#ifdef OS_WIN32
#include <math.h>
#endif

#define RADIO_STOP_LEVEL            1

#define  WATCHDOG_SET_LONG_TIME  115 // 115+5S = 120
#define  WATCHDOG_DISABLE_LONG_TIME  0 // 1+5S = 6S

extern const T_RADIO_AREA_PARM   RadioAreaParam[MAX_RADIO_AREA];
T_RADIO_PLAYER_PARM * pRadioParam = AK_NULL;

#if (USE_COLOR_LCD)
#define SEARCH_INTERVAL            100    //radio search inteval time
#else
#define SEARCH_INTERVAL            20 
#endif

#ifdef SUPPORT_RADIO_RDA5876
/***********************************************************************************
**brief: 	initialize radiopalyer default param and hardware
**param:	pRParam :radio struct  pointer (ref: Fwl_pfRadio.h)
**return:	AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
***********************************************************************************/
extern T_BOOL auto_search_flag;
T_BOOL Radio_PlayerInit(T_RADIO_PLAYER_PARM * pRParam)
{
	T_RADIO_CFG	    radioparam;
	T_U32			i;

    //get radio param struct pointer
    if(AK_NULL == pRParam)
        return AK_FALSE;
    else    
        pRadioParam = pRParam;

    //如果初始化失败
    if(AK_FALSE == Fwl_RadioInit())
    {
        return AK_FALSE;
    }
    
	pRadioParam->channelList = (T_U32 *)Fwl_Malloc(RADIO_VALID_FREQ_COUNT*sizeof(T_U32));
	if (pRadioParam->channelList == AK_NULL)
	{
		akerror("#FM->:radio channelList malloc faile!!!", 0, 1);
	}
	memset(pRadioParam->channelList, 0, sizeof(T_U32)*RADIO_VALID_FREQ_COUNT);
    //AK_DEBUG_OUTPUT(" CurFreq:%d,CheckCode:%x\n",radioparam.CurFreq,radioparam.CheckCode);
    //read saved config data
    Profile_ReadData(eCFG_RADIO, &radioparam);
    //AK_DEBUG_OUTPUT(" CurFreq:%d,CheckCode:%x\n",radioparam.CurFreq,radioparam.CheckCode);
	if((radioparam.CurFreq >= 87500000) 
		&& (radioparam.CurFreq <= 108000000)
		&&(radioparam.RadioArea<= RADIO_AMERICA)
		&&(radioparam.RadioArea>= RADIO_EUROPE))
	{   //如果保存的信息是合法值就采用保存信息	
		pRadioParam->CurFreq    = radioparam.CurFreq;
		pRadioParam->RadioArea 	= radioparam.RadioArea;
        pRadioParam->Volume		= radioparam.Volume;	
        if(pRadioParam->Volume > 31 )//MAX_VOLUME
            pRadioParam->Volume     = 31;

		if (pRadioParam->channelList)
		{
			for (i = 0; i < RADIO_VALID_FREQ_COUNT; i++)
				pRadioParam->channelList[i] = radioparam.ChannelList[i];
		}
	}
	else
	{   //保存信息不合法，采用默认值
		//  set para used default 
		pRadioParam->CurFreq		= RadioAreaParam[0].FreqMin;
		pRadioParam->RadioArea		= RADIO_EUROPE;
		pRadioParam->Volume		    = DEF_VOLUME;
	}

	//other param use default setup
	pRadioParam->FreqMax            = RadioAreaParam[pRadioParam->RadioArea].FreqMax;
	pRadioParam->FreqMin            = RadioAreaParam[pRadioParam->RadioArea].FreqMin;
	pRadioParam->RadioState		    = RADIO_PLAY;	

	pRadioParam->bStereo            = AK_FALSE;
	pRadioParam->bHWER		        = AK_FALSE;

    if(0 != pRadioParam->channelList[0])
    {
        auto_search_flag = AK_FALSE;
    }
    else
    {
        auto_search_flag = AK_TRUE;
    }
    
    /*
    //start initialize hardware
    if (!RadioSwitch(SWITCH_ON))
    { //initialize radio hardware, if failed do following
        pRadioParam->RadioState   = RADIO_STOP;
        pRadioParam->bHWER        = AK_TRUE;
        AK_DEBUG_OUTPUT("#FM->:init failed!!!\n");
    }
	else
	{
        //AK_DEBUG_OUTPUT("RadioSwitch Suc!\n");
    	pRadioParam->RadioState   = RADIO_PLAY;
        //akerror("CheckCode",radioparam.CheckCode,1);
		if (radioparam.CheckCode == 0x3a)
		{
			auto_search_flag = AK_TRUE;
		}
        //Play_FM_Fre(pRadioParam->CurFreq);
        //Fwl_RadioSetVolume(pRadioParam->Volume); 
        //RadioSwitch(SWITCH_ON);
	}
	
    if(pRadioParam->bHWER)
        return AK_FALSE;
    else
        return AK_TRUE;
    */
        return AK_TRUE;  
}

T_BOOL	Radio_Switch(SWITCH switch_state)
{	
	T_RADIO_PARM_EX   RadioParam;
	
	if (AK_NULL == pRadioParam)
	{
		AK_DEBUG_OUTPUT("#FM->:RadioSetFreq(): pRadioParam pointer is NULL");
		return AK_FALSE;
	}

	//init limit 
	if(RADIO_JAPAN == pRadioParam->RadioArea)
	    RadioParam.BandLimit = AK_TRUE;
	else
		RadioParam.BandLimit = AK_FALSE;
	RadioParam.Freq = pRadioParam->CurFreq;	

	//set search mod
	RadioParam.SearchFlag = AK_FALSE;
	RadioParam.SearchDir = AK_FALSE;
	RadioParam.MonoStereo = AK_FALSE;
	RadioParam.StopLevel = RADIO_STOP_LEVEL;

	// turn on or trun off radioplayer
	if (switch_state == SWITCH_OFF)  //(Fwl_GetAudioVolume() == 0) ||(OnAndOff == AK_FALSE)
        RadioParam.MuteFlag = AK_TRUE;
    else
	    RadioParam.MuteFlag = AK_FALSE;
	//set param to hardware 
	if (Fwl_RadioSetParam(&RadioParam))
	{
		return AK_TRUE;
    }
    else
		return AK_FALSE;

}
static __inline T_U32 Radio_ValidFreq(T_U32 freq)
{
	if (freq < 87500000 || freq > 108000000)
		return 0;
	else
		return freq;
}

T_BOOL Radio_AutoSearch(T_VOID)
{
	T_U8 ret = 0;
	T_U32 i;
	akerror("radio auto search begain!", 0, 1);
	if (pRadioParam->channelList)
	{
		memset(pRadioParam->channelList, 0, RADIO_VALID_FREQ_COUNT*sizeof(T_U32));
		ret = Fwl_Radio_AutoSearch(pRadioParam->channelList);
	}
	else
	{
		akerror("#FM->:no chan list", 0, 1);
		return ret;
	}
	
	for (i = 0; i < RADIO_VALID_FREQ_COUNT; i++)
	{
		AK_DEBUG_OUTPUT("#FM->:I:%d, F:%d\n", i, pRadioParam->channelList[i]);
	}
    if(AK_FALSE== pRadioParam->channelList[0])
    {
        pRadioParam->channelList[0]= 87500000;
    }
	if (Radio_ValidFreq(pRadioParam->channelList[0]))
	{
		pRadioParam->CurFreq = pRadioParam->channelList[0];
		//RadioSwitch(SWITCH_ON);
		//Play_FM_Fre(pRadioParam->CurFreq);
	}
	
	return ret;
}



T_BOOL Radio_JumpChannel(T_U8 dir)
{
	T_U32 preChan = 0xff;
	T_U32 i;

	if (pRadioParam == AK_NULL || pRadioParam->channelList == AK_NULL)
	{
		akerror("#FM->:channel list null", 0, 1);
		return 0;
	}

	for ( i = 0; i < RADIO_VALID_FREQ_COUNT; i++)
	{
		if (pRadioParam->CurFreq == pRadioParam->channelList[i])
			break;
		if (Radio_ValidFreq(pRadioParam->channelList[i]))
			preChan = pRadioParam->channelList[i];
	}

	if (i == RADIO_VALID_FREQ_COUNT)
	{
		if (preChan != 0xff)
		{
			i = RADIO_VALID_FREQ_COUNT-1;
			dir = 1;
		}
		else
			return 0;
	}

	if (dir)
	{//next
		i++;
		i = i%RADIO_VALID_FREQ_COUNT;
		if (Radio_ValidFreq(pRadioParam->channelList[i]))
		{
			pRadioParam->CurFreq = pRadioParam->channelList[i];
		    AK_DEBUG_OUTPUT("#FM->:next chan i:%d", i);
        }
		else
		{
			pRadioParam->CurFreq = pRadioParam->channelList[0];
            i = 0;
            AK_DEBUG_OUTPUT("#FM->:next chan i:%d", i);
		}
		
	}
	else
	{//pre
		if (i != 0)
		{
			pRadioParam->CurFreq = preChan;
		}
		else
		{
			for (i = RADIO_VALID_FREQ_COUNT; i != 0; i--)
			{
				if (Radio_ValidFreq(pRadioParam->channelList[i-1]))
				{
					pRadioParam->CurFreq = pRadioParam->channelList[i-1];
					break;
				}
			}
		}
		AK_DEBUG_OUTPUT("#FM->:pre chan i:%d", i);
	}
	
	AK_DEBUG_OUTPUT(", freq:%d\n", pRadioParam->CurFreq);

	//RadioSwitch(SWITCH_ON);
	//Play_FM_Fre(pRadioParam->CurFreq);
    
	return 1;
}

/***********************************************************************************
**name:     	RadioConserveData()    
**brief: 	conserve radio parameters current ,for next running
**param:	NO	
**return:	NO
**author:	YaoHongshi
**date:		2008-04-09
***********************************************************************************/
T_VOID	Radio_ConserveData(T_VOID)
{
	T_U32	i;
	T_RADIO_CFG	radioparam;

    if(AK_NULL == pRadioParam)
        return;

    //get frequency and area information
	radioparam.CurFreq 		= pRadioParam->CurFreq;
	radioparam.RadioArea	= pRadioParam->RadioArea;
    radioparam.Volume       = pRadioParam->Volume;

	if (pRadioParam->channelList)
	{
		for (i = 0; i < RADIO_VALID_FREQ_COUNT; i++)
			radioparam.ChannelList[i] = pRadioParam->channelList[i];
	}
    Profile_WriteData(eCFG_RADIO, &radioparam);
}

/*******************************************************************************
**brief     :   set radio mute
**param     :   NO    
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_SetMute(T_VOID)
{
    Fwl_RadioSetVolume(0);

    Fwl_DelayUs(100);
}


/***********************************************************************************
**brief:    free radioplayer
**param: NO		
**return: NO		
***********************************************************************************/
T_VOID Radio_PlayerFree(T_VOID)
{
    if(AK_NULL == pRadioParam)
        return;

	Radio_ConserveData();

	if (pRadioParam->channelList)
		pRadioParam->channelList = Fwl_Free(pRadioParam->channelList);

	Fwl_RadioFree();
}


/***********************************************************************************
**brief: 		set radio volume
**param:		search direction	
**return:		
**author:	       YaoHongshi
**date:		2008-04-09
***********************************************************************************/
T_BOOL Radio_SetVolume(T_VOID)
{   
	Fwl_RadioSetVolume(pRadioParam->Volume);
	akerror("#FM->:radio V:", pRadioParam->Volume, 1);
    return AK_TRUE;
}

/*******************************************************************************
**brief     :   get radio volume
**param     :   NO
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_U8 Radio_GetVolume(T_VOID)
{
    return pRadioParam->Volume;
}

/***********************************************************************************
**brief     :   set radio volume add
**param     :   search direction    
**return    :   
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL Radio_AddVolume(T_VOID)
{   
    if (pRadioParam->Volume < L2HP_MAX_VOLUME)
    {
       pRadioParam->Volume++;
       Radio_SetVolume();
    }

    Fwl_DelayUs(100);
    return AK_TRUE;
}

/***********************************************************************************
**brief     :   set radio volume sub
**param     :   search direction    
**return    :   
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL Radio_SubVolume(T_VOID)
{   
    if (pRadioParam->Volume > 0)
    {
        pRadioParam->Volume--;
        Radio_SetVolume();
    }

    Fwl_DelayUs(100);
    return AK_TRUE;
}

#else
/*******************************************************************************
**name      :   radioplayerinit()    
**brief     :   initialize radiopalyer default param and hardware
**param     :   pRParam :radio struct  pointer (ref: Fwl_pfRadio.h)
**return    :   AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
**author    :   YaoHongshi
**date      :   2008-04-09 ->(2008-08-21M)
*******************************************************************************/
T_BOOL  Radio_InitPlayer(T_RADIO_PLAYER_PARM * pRParam)
{
    T_U32           temp;
    T_RADIO_CFG     radioparam;

    //get radio param struct pointer
    if(AK_NULL == pRParam)
        return AK_FALSE;
    else    
        pRadioParam = pRParam;

    //Fm device initialize
    if (AK_FALSE == Fwl_RadioInit())
    {
        pRadioParam = AK_NULL;
        return AK_FALSE;
    }

    pRadioParam->channelAll = (T_U32 *)Fwl_Malloc(RADIO_STATION_NUM*2*sizeof(T_U32));
    //AK_ASSERT_PTR_VOID(pRadioParam->channelAll, "malloc memory error in initradio_player\n")

    //read saved config data
    Profile_ReadData(eCFG_RADIO, &radioparam);

    if((radioparam.CurFreq >= 76000000) 
        && (radioparam.CurFreq <= 108000000)
        &&(radioparam.RadioArea<= RADIO_AMERICA)
        &&(radioparam.RadioArea>= RADIO_EUROPE))
    {   //如果保存的信息是合法值就采用保存信息  
        pRadioParam->CurFreq    = radioparam.CurFreq;
        pRadioParam->SavedFreq  = radioparam.SavedFreq;
        pRadioParam->RadioArea  = radioparam.RadioArea;
        pRadioParam->Volume     = radioparam.Volume;    //Fwl_GetAudioVolume();
        if(pRadioParam->Volume > L2HP_MAX_VOLUME )
            pRadioParam->Volume     = DEF_VOLUME;

        for(temp = 0; temp < (RADIO_STATION_NUM*2); temp++)
            pRadioParam->channelAll[temp] = radioparam.ChannelList[temp];
                
        if(pRadioParam->RadioArea == RADIO_JAPAN)
            pRadioParam->CurList = pRadioParam->channelAll + RADIO_STATION_NUM;
        else
            pRadioParam->CurList = pRadioParam->channelAll;
    }else
    {   //保存信息不合法，采用默认值
        //  set para used default 
        pRadioParam->CurFreq        = RadioAreaParam[0].FreqMin;
        pRadioParam->RadioArea      = RADIO_EUROPE;
        pRadioParam->Volume         = 5;
        pRadioParam->CurList        = pRadioParam->channelAll;

        for(temp = 0; temp < (RADIO_STATION_NUM*2); temp++)
            pRadioParam->channelAll[temp] = RADIO_FREQ_NULL;
    }


    //other param use default setup
    pRadioParam->FreqMax            = RadioAreaParam[pRadioParam->RadioArea].FreqMax;
    pRadioParam->FreqMin            = RadioAreaParam[pRadioParam->RadioArea].FreqMin;
    pRadioParam->RadioState         = RADIO_PLAY;
    pRadioParam->ch_pos             = (T_U8)INVALID;
    pRadioParam->preChPos           = radioparam.prePos;
    pRadioParam->bStereo            = AK_FALSE;
    pRadioParam->bHWER              = AK_FALSE;
    pRadioParam->Stimer             = ERROR_TIMER;

    if (RADIO_FREQ_NULL == pRadioParam->CurList[0])
    {
        Radio_StartAutoSearch(RADIO_SEARCH_ALL, AK_TRUE);
        return AK_TRUE;
    }

    //start initialize hardware
    if (!Radio_Switch(SWITCH_ON))
    { //initialize radio hardware, if failed do following
        pRadioParam->RadioState     = RADIO_STOP;
        AK_DEBUG_OUTPUT("init failed!!!\n");
        return AK_FALSE;
    }
    else
    {
        pRadioParam->RadioState     = RADIO_PLAY;
        return AK_TRUE;
    }
}


/*******************************************************************************
**name      :   Radio_Switch()    
**brief     :   start radioplayer ,on current frequency
**param     :   ONAndOff :  1   turn on radio
**                          0   turn off radio  
**return    :   AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
**author    :   YaoHongshi
**date      :   2008-04-09->(2008-08-21)
*******************************************************************************/
T_BOOL Radio_Switch(SWITCH switch_state)
{
    T_RADIO_PARM_EX RadioParam;
    T_BOOL ret;
    
    if (AK_NULL == pRadioParam)
    {
        AK_DEBUG_OUTPUT("RadioSetFreq(): pRadioParam pointer is NULL");
        return AK_FALSE;
    }

    //init limit 
    if(RADIO_JAPAN == pRadioParam->RadioArea)
        RadioParam.BandLimit = AK_TRUE;
    else
        RadioParam.BandLimit = AK_FALSE;
    RadioParam.Freq = pRadioParam->CurFreq; 

    //set search mod
    RadioParam.SearchFlag = AK_FALSE;
    RadioParam.SearchDir  = AK_FALSE;
    RadioParam.MonoStereo = AK_FALSE;
    RadioParam.StopLevel  = RADIO_STOP_LEVEL;
    RadioParam.Volume     = pRadioParam->Volume;

    pRadioParam->bStereo = RadioSteCheck(&RadioParam);
    pRadioParam->ch_pos  = RadioGetChPos(pRadioParam->CurFreq, RADIO_STATION_NUM);

    //turn on or trun off radioplayer
    if (switch_state == SWITCH_OFF)  //(Fwl_GetAudioVolume() == 0) ||(OnAndOff == AK_FALSE)
    {
        RadioParam.MuteFlag = AK_TRUE;
        RadioSetMute();
        ret = Fwl_RadioSetParam(&RadioParam);
    }
    else
    {
        RadioParam.MuteFlag = AK_FALSE;
        ret = Fwl_RadioSetParam(&RadioParam);
        RadioSetVolume();
    }

    return ret;
}


/*******************************************************************************
**name      :   Radio_JumpBandWidth()    
**brief     :   jump to next station with adding or  
**param     :   ONAndOff :  1  forward
**                          0  back 
**return    :   NO
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_JumpBandWidth( T_S16 dir)
{
    if(AK_NULL ==pRadioParam)
        return;

    if(dir < 0)
    {   //jump back a bandwidth
        dir = 0 - dir;
        pRadioParam->CurFreq = pRadioParam->CurFreq - (T_U32)(BAND_WIDTH*dir);
    }
    else
    {   // jump forward a bandwidth
        
        pRadioParam->CurFreq = pRadioParam->CurFreq + (T_U32)(BAND_WIDTH*dir);
    }
    if(pRadioParam->CurFreq < pRadioParam->FreqMin)
        pRadioParam->CurFreq = pRadioParam->FreqMax;
    else if(pRadioParam->CurFreq > pRadioParam->FreqMax)
        pRadioParam->CurFreq = pRadioParam->FreqMin;
    AK_DEBUG_OUTPUT("FreqMin:%d curfreq:%d  FreqMax:%d\n",pRadioParam->FreqMin,pRadioParam->CurFreq,pRadioParam->FreqMax);
    Radio_Switch(SWITCH_ON);    //restart radioplayer
    pRadioParam->RadioState   = RADIO_PLAY;
}


/*******************************************************************************
**name      :   Radio_SetSearchFreq()   
**brief     :   set search frequency
**param     :   search direction    
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
//change by lsk 2011-5-26
*******************************************************************************/
T_BOOL Radio_SetSearchFreq(T_BOOL bSerchfirst)
{
    T_RADIO_PARM_EX Param;

    if(AK_NULL == pRadioParam)
        return AK_FALSE;


    if(RADIO_JAPAN == pRadioParam->RadioArea)
        Param.BandLimit = AK_TRUE;
    else
        Param.BandLimit = AK_FALSE;
    
    if (RADIO_SEARCH_PREV == pRadioParam->RadioState)
    {
        if(pRadioParam->CurFreq <= pRadioParam->FreqMin)
            pRadioParam->CurFreq = pRadioParam->FreqMax;
        else
            pRadioParam->CurFreq -= BAND_WIDTH;
        Param.SearchDir = AK_FALSE;
    }
    else
    {
        if(pRadioParam->CurFreq >= pRadioParam->FreqMax)
            pRadioParam->CurFreq = pRadioParam->FreqMin;
        else
            pRadioParam->CurFreq += BAND_WIDTH;
        Param.SearchDir = AK_TRUE;
    }
    if (bSerchfirst)
    {
        pRadioParam->CurFreq = pRadioParam->FreqMin;
    }
    
    Param.Volume        = pRadioParam->Volume;
    Param.Freq          = pRadioParam->CurFreq;
    Param.MonoStereo    = AK_FALSE;
    Param.StopLevel     = RADIO_STOP_LEVEL;
    Param.SearchFlag    = AK_FALSE;
    
    pRadioParam->ch_pos  = RadioGetChPos(pRadioParam->CurFreq,RADIO_STATION_NUM);
    pRadioParam->bStereo = RadioSteCheck(&Param);

    Param.MuteFlag = AK_FALSE;
    Param.SearchFlag = AK_TRUE;

    if (AK_FALSE == Fwl_RadioSetParam(&Param))
    {
        AK_DEBUG_OUTPUT("radio set parm fail\n");
        return AK_FALSE;
    }

    Fwl_DelayUs(10000);
    return AK_TRUE;
}


/*******************************************************************************
**name      :   Radio_StartAutoSearch()   
**brief     :   start auto rearch stations
**param     :   dir radio auto search direction   
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_StartAutoSearch(T_RADIO_STATE SearchType ,T_BOOL bSerchFirst)
{
    if(AK_NULL == pRadioParam)
        return;
    
    //set radio state
    switch(SearchType)
    {
    case RADIO_SEARCH_PREV:
        pRadioParam->RadioState = RADIO_SEARCH_PREV;
        break;

    case RADIO_SEARCH_NEXT:
        pRadioParam->RadioState = RADIO_SEARCH_NEXT;
        break;

    case RADIO_SEARCH_ALL:
        pRadioParam->RadioState = RADIO_SEARCH_ALL; 
        break;

    default:
        return;
        break;
    }

    RadioSetMute();
    //set search start frequency
    Radio_SetSearchFreq(bSerchFirst);

    //stop timer
    if (ERROR_TIMER != pRadioParam->Stimer)
    {
        Fwl_TimerStop(pRadioParam->Stimer);
        pRadioParam->Stimer = ERROR_TIMER;
    }

    //start timer
    pRadioParam->Stimer = Fwl_TimerStartMilliSecond(SEARCH_INTERVAL, AK_TRUE);
    if (ERROR_TIMER != pRadioParam->Stimer)
    {
        //set watch dog time to 2 minute
        Fwl_SetLongWatchdog(WATCHDOG_SET_LONG_TIME);
    }
}


/*******************************************************************************
**name      :   RadioFreqValue()    
**brief     :   change frequence value to a legal value according to  step value
**param     :   Freq: the freq to be changed    
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
static T_U32 RadioFreqValue(T_U32 Freq)
{
    T_U32 retFreq = 87500000;
    T_U32 BandWidth = 0;
    T_U32 tmp1,tmp2, tmp3;

    if(AK_NULL == pRadioParam)
        return 0;

    BandWidth = BAND_WIDTH;

    tmp1 = Freq;
    //caculat frequency according PLL
    if (tmp1 > (pRadioParam->FreqMin))
    {
        tmp2 = (tmp1 - pRadioParam->FreqMin) / BandWidth;

        if ((Freq - (pRadioParam->FreqMin + tmp2 * BandWidth))  < ((pRadioParam->FreqMin + (tmp2 + 1)*BandWidth) - Freq))
            tmp3 = pRadioParam->FreqMin + tmp2 * BandWidth;
        else
            tmp3 = pRadioParam->FreqMin + (tmp2 + 1) * BandWidth;
    }else
    {
        tmp3 = pRadioParam->FreqMin;
    }

    if (tmp3 > pRadioParam->FreqMax)
        tmp3 =pRadioParam->FreqMax;
    
    retFreq = tmp3;

    return retFreq;
}


/*******************************************************************************
**name      :   Radio_IsFindStation()     
**brief     :   get FM's state ,check wether find a station
**param     :   FreqRet: the pointer that station found will be saved   
**return    :   AK_TRUE = find a station  AK_FALSE = do not find a station   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_BOOL Radio_IsFindStation(T_U32 *FreqRet)
{
    T_RADIO_STATUS_EX Status;
    //T_RADIO_STATUS temp_Status;
    //T_RADIO_PARM Param;
    T_U16       sreadNum=0;
    T_U16       bstereo=0;
    T_BOOL      bstation = AK_FALSE;
    T_U16       temp = 0;

    if(AK_NULL == pRadioParam)
        return bstation;

    if (AK_TRUE == Fwl_RadioGetStatus(&Status, pRadioParam->FreqMin, pRadioParam->FreqMax)
        &&Status.ReadyFlag)
    {  
        /*AK_DEBUG_OUTPUT("\n\nread Flag: %d\n",Status.ReadyFlag);
        AK_DEBUG_OUTPUT("cur sys freq: %d\n",Fwl_FreqGet());
        AK_DEBUG_OUTPUT("freq: %d\n",RadioFreqValue(Status.CurFreq));
        AK_DEBUG_OUTPUT("flag: %d\n",Status.LimitFlag);
        AK_DEBUG_OUTPUT("Adc  level: %d\n\n\n",Status.AdcLevel);
*/
        sreadNum=0;
        for(temp =0; temp < 5; temp++)
        {
        
            if (AK_TRUE == Fwl_RadioGetStatus(&Status, pRadioParam->FreqMin, pRadioParam->FreqMax)
                &&Status.ReadyFlag
                &&!Status.LimitFlag
                )
            {
                if(Status.AdcLevel >= RADIO_ADC_LEVEL)
                    sreadNum++;

                if(Status.Stereo)
                    bstereo++;
            }
        }

        if((sreadNum >= 1 )&&(bstereo >= 4))
        {
            *FreqRet =  RadioFreqValue(Status.CurFreq);
            bstation = AK_TRUE;
        }

        /*
        if(sreadNum >= 3)
        {   //if found a station ,set as current frequency
            if(RADIO_JAPAN == pRadioParam->RadioArea)
                Param.BandLimit = AK_TRUE;
            else
                Param.BandLimit = AK_FALSE;

            Param.SearchDir = AK_FALSE;
            Param.Freq = RadioFreqValue(Status.CurFreq);
            Param.MonoStereo    = AK_FALSE;
            if(RadioSteCheck(&Param))
            {
                *FreqRet =  RadioFreqValue(Status.CurFreq);
                bstation = AK_TRUE;
            }
        }*/
    }else
    {
        //AK_DEBUG_OUTPUT("in f_Ready Flag: %d\n",Status.ReadyFlag);
        //AK_DEBUG_OUTPUT("in f_limit flag: %d\n",Status.LimitFlag);
    }

    return bstation;
}


/*******************************************************************************
**name      :   RadioAutoSearchStop()    
**brief     :   stop auto search
**param     :            
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_StopAutoSearch(T_VOID)
{
    T_U32   freq = 0;

    if(AK_NULL == pRadioParam)
        return;

    if (ERROR_TIMER != pRadioParam->Stimer)
    {   //stop timer
        Fwl_TimerStop(pRadioParam->Stimer);
        pRadioParam->Stimer = ERROR_TIMER;

        //clean watch dog long time
        Fwl_SetLongWatchdog(WATCHDOG_DISABLE_LONG_TIME);
    }

    if(RADIO_SEARCH_ALL == pRadioParam->RadioState)
    {
        freq = RadioGetChFromList(1);
        if(freq == INVALID)
            pRadioParam->CurFreq = pRadioParam->FreqMin;
        else
            pRadioParam->CurFreq = freq;
    }
    
    Radio_Switch(SWITCH_ON);
    pRadioParam->RadioState   = RADIO_PLAY; 
}


/*******************************************************************************
**name      :   RadioConserveData()    
**brief     :   conserve radio parameters current ,for next running
**param     :   NO  
**return    :   NO
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID  RadioConserveData(T_VOID)
{
    T_U32   i;
    T_RADIO_CFG radioparam;

    if(AK_NULL == pRadioParam)
        return;

    //get channel list
    for(i = 0; i < (RADIO_STATION_NUM*2); i++)
        radioparam.ChannelList[i] = pRadioParam->channelAll[i];

    //get frequency and area information
    radioparam.CurFreq      = pRadioParam->CurFreq;
    radioparam.RadioArea    = pRadioParam->RadioArea;
    radioparam.Volume       = pRadioParam->Volume;
    radioparam.SavedFreq    = pRadioParam->SavedFreq;
    radioparam.prePos       = pRadioParam->preChPos;

    Profile_WriteData(eCFG_RADIO, &radioparam);
}


/*******************************************************************************
**brief     :   free radioplayer
**param     :   NO
**return    :   NO
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_FreePlayer(T_VOID)
{
    if(AK_NULL == pRadioParam)
        return;

    RadioSetMute();
    RadioConserveData();
    
    //stop radioplayer
    if (ERROR_TIMER != pRadioParam->Stimer)
    {   //stop timer
        Fwl_TimerStop(pRadioParam->Stimer);
        pRadioParam->Stimer = ERROR_TIMER;
    }

    //release memory
    pRadioParam->channelAll = Fwl_Free(pRadioParam->channelAll);

    Fwl_RadioFree();
}


/*******************************************************************************
**brief     :   添加频段到列表中去，可通过limt指定添加的位置
**              否则就追加到当前列表
**param     :   freq:   要添加的频率； Limt: 限定位置 
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_BOOL Radio_AddChanToList(T_U32 freq, T_BOOL Limt)
{
    T_U8            i;
    T_U8            save_pos = 0;

    if(AK_NULL == pRadioParam)
        return AK_FALSE;


    pRadioParam->ch_pos = (T_U8)INVALID;

    
    if((Limt !=0) &&(Limt <= RADIO_STATION_NUM))
        save_pos = Limt; //在限定的长度范围那添加，否则直接添加到列表的最后

    if(save_pos == 0)
    {
        for (i=0; i<RADIO_STATION_NUM; i++)
        {
            if(freq == pRadioParam->CurList[i])
            {
                pRadioParam->ch_pos = i+1;
                break;
            }
            
            if (RADIO_FREQ_NULL == pRadioParam->CurList[i])
            {   //if have empty position , save it
                pRadioParam->CurList[i] = freq;         
                pRadioParam->ch_pos = i+1;  //set current frequency in the list
                break;
            }
        }
    }else
    {
        pRadioParam->CurList[save_pos-1] = freq;
        pRadioParam->ch_pos = save_pos;

        //删除后面重复的电台
        for(i=save_pos; i<RADIO_STATION_NUM; i++)
        {
            if(freq == pRadioParam->CurList[i])
            {
                for(; i<(RADIO_STATION_NUM-1);i++)
                    pRadioParam->CurList[i] = pRadioParam->CurList[i+1]; 

                pRadioParam->CurList[RADIO_STATION_NUM-1] = RADIO_FREQ_NULL;
            }
        }
    }

    if(pRadioParam->ch_pos > RADIO_STATION_NUM)
    {
        return AK_FALSE;
    }
    else
    {
        pRadioParam->preChPos= pRadioParam->ch_pos;
        return AK_TRUE;
    }
}


/*******************************************************************************
**brief     :   delete  the station in the station list
**param(in) :   DelType AK_TRUE : delete all station ,
**                      AK_FALSE :delete current frequency
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_BOOL Radio_DelChanFromList(T_BOOL DelType)
{
    T_U32  i;

    if(AK_NULL == pRadioParam)
        return AK_FALSE;

    if(DelType)
    {   //delete all
        for(i = 0; i < RADIO_STATION_NUM; i++)
            pRadioParam->CurList[i] = RADIO_FREQ_NULL;      
        pRadioParam->preChPos= 0;

    }else
    {
        if((0 == pRadioParam->ch_pos)
            ||pRadioParam->ch_pos > RADIO_STATION_NUM)
            return AK_TRUE;
        pRadioParam->preChPos= pRadioParam->ch_pos- 1;//modify by zhao_xiaowei
        //把删除后的列表重新排列        
        i = pRadioParam->ch_pos; 
        pRadioParam->ch_pos--;
        while(i < RADIO_STATION_NUM)
        {
            pRadioParam->CurList[pRadioParam->ch_pos] = pRadioParam->CurList[i];            
            i++;
            pRadioParam->ch_pos++;
        }
        pRadioParam->CurList[RADIO_STATION_NUM-1] = RADIO_FREQ_NULL;
    }
    
    //设置当前在列别的位置为失效状态；
    pRadioParam->ch_pos = (T_U8)-1;
    return AK_TRUE;
}


/*******************************************************************************
**brief     :   find current frequency position in the list
**param     :   pRadPar pointer of radio parameters, reference to Fwl_pfRadio.h
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_U8 RadioGetChPos(T_U32 freq,T_U8 PosLimt)
{
    T_U8    i;
    T_U8    limit_pos = RADIO_STATION_NUM;
    T_U8    pos = 0;

    if(AK_NULL == pRadioParam)
        return AK_FALSE;

    if((PosLimt>0) && (PosLimt <= RADIO_STATION_NUM))
        limit_pos = PosLimt;    //如果位置限制，就在限定的位置之前查找


    for(i = 0; i < limit_pos; i++)
    {
        if(limit_pos < RADIO_STATION_NUM)
        {
            if(abs(pRadioParam->CurList[i] - freq) < 2*BAND_WIDTH)
            {   //找到了对应的节点
                pos = i+1;
                break;
            }
        }
        else
        {
            if(pRadioParam->CurList[i] == freq)
            {   //找到了对应的节点
                pos = i+1;
                break;
            }
        }
    }

    if(pos >= 1 && pos <= RADIO_STATION_NUM)//if cur freq is station  change preChPos to pos
    {
        pRadioParam->preChPos= pos;
    }
    else 
    {
        //if ch_pos is station change preChPos to ch_pos
        if(pRadioParam->ch_pos >= 1 && pRadioParam->ch_pos <= RADIO_STATION_NUM)
        {
            pRadioParam->preChPos= pRadioParam->ch_pos;
        }
    }
    return pos;
}


/*******************************************************************************
**brief     :   find next channel  in the list , and turn on it
**param     :   NO
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_U32 RadioGetChFromList(T_U8 pos)
{
    T_U32   freq = INVALID;
    
    if(AK_NULL == pRadioParam)
        return freq;

    if(pos == 0 || pos > RADIO_STATION_NUM)
        return freq;

    if(pRadioParam->CurList[pos-1] != RADIO_FREQ_NULL)
        freq = pRadioParam->CurList[pos-1];
   
    return freq;
}


/*******************************************************************************
**brief     :   检查当前电台是否是立体声电台
**param     :   NO
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_BOOL RadioSteCheck(T_RADIO_PARM_EX * param)
{
    T_RADIO_STATUS_EX Status = {0};
    T_U8    stere_num=0;
    T_U16   checknum;

    if(AK_NULL == pRadioParam)
        return AK_FALSE;

    param->MuteFlag   = AK_FALSE;
    param->SearchFlag = AK_FALSE;
    param->StopLevel  = RADIO_STOP_LEVEL;
    param->Volume     = pRadioParam->Volume;
    Fwl_RadioSetParam(param);
    Fwl_DelayUs(10000);

    for (checknum=0; checknum<5; checknum++)
    {
        Fwl_RadioGetStatus(&Status,pRadioParam->FreqMin,pRadioParam->FreqMax);
        
        if(Status.Stereo)
            stere_num++;

        //if(Status.AdcLevel >= RADIO_ADC_LEVEL)
        //    adc_level++;
    }
    
    if((3 <= stere_num)/*&&(adc_level >= 3)*/)
    {
        return AK_TRUE;
    }
    else
        return AK_FALSE;
}


/*******************************************************************************
**brief     :   获得当前频率相对最小频率的宽度
**param     :
**return    :   bandwidth
**author    :   Yao Hongshi
**date      :   2008-06-02
*******************************************************************************/
T_U32 RadioGetBand(T_VOID)
{
    T_U32   ret;
    
    if(AK_NULL == pRadioParam)
        return 0;

    ret = (pRadioParam->CurFreq - pRadioParam->FreqMin)/BAND_WIDTH;

    return ret; 
}


/*******************************************************************************
**brief     :   set radio mute
**param     :   NO    
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_BOOL RadioSetMute(T_VOID)
{
    Fwl_RadioSetVolume(0);

    Fwl_DelayUs(100);
    return AK_TRUE;
}


/*******************************************************************************
**brief     :   set radio volume
**param     :   NO    
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_BOOL RadioSetVolume(T_VOID)
{
    Fwl_RadioSetVolume(pRadioParam->Volume);

    Fwl_DelayUs(100);
    return AK_TRUE;
}


/*******************************************************************************
**brief     :   get radio volume
**param     :   NO
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_U8 RadioGetVolume(T_VOID)
{
    return pRadioParam->Volume;
}


/***********************************************************************************
**brief     :   set radio volume add
**param     :   search direction    
**return    :   
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL RadioAddVolume(T_VOID)
{   
    if (pRadioParam->Volume < L2HP_MAX_VOLUME)
    {
       pRadioParam->Volume++;
       RadioSetVolume();
    }

    Fwl_DelayUs(100);
    return AK_TRUE;
}

/***********************************************************************************
**brief     :   set radio volume sub
**param     :   search direction    
**return    :   
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL RadioSubVolume(T_VOID)
{   
    if (pRadioParam->Volume > 0)
    {
        pRadioParam->Volume--;
        RadioSetVolume();
    }

    Fwl_DelayUs(100);
    return AK_TRUE;
}

#endif
#endif
//end of file

