/******************************************************************************************
**FileName  	:      log__ebkcore.h
**brief        	:      NO
**Copyright 	:      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author 		:	Hongshi Yao
**date		: 	2008-04-21
**version 	:	1.0
*******************************************************************************************/
#ifndef __LOG_RADIO_CORE_H__
#define __LOG_RADIO_CORE_H__


#include "Fwl_Radio.h"


#ifdef SUPPORT_RADIO


#ifndef INVALID
#define INVALID             0xffffffff
#endif

typedef enum{
    SWITCH_OFF,
    SWITCH_ON
} SWITCH;

typedef struct {
    T_U32                   FreqMin;
    T_U32                   FreqMax;
    T_U32                   FreqIf;      //��Ƶ
    T_U32                   FreqRef;     //�ο�Ƶ��
    T_U32                   FreqStep;    //�����򲨿�
}T_RADIO_AREA_PARM;

#ifdef SUPPORT_RADIO_RDA5876

#define DEF_VOLUME          (5)

typedef struct {
		T_U32					CurFreq;
		T_U32					SavedFreq;
		T_U32					ChannelList[RADIO_VALID_FREQ_COUNT];
		T_U8					Volume;
		T_RADIO_AREA			RadioArea;
		T_U8 					prePos;
        T_U8                    CheckCode;    // Used to validate the config.
}T_RADIO_CFG;


/***********************************************************************************
**name:     	radioplayerinit()    
**brief: 	initialize radiopalyer default param and hardware
**param:	pRParam :radio struct  pointer (ref: Fwl_pfRadio.h)
**return:	AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
**author:	YaoHongshi
**date:		2008-04-09 ->(2008-08-21M)
***********************************************************************************/
T_BOOL	Radio_PlayerInit( T_RADIO_PLAYER_PARM * pRParam);


/***********************************************************************************
**name    : 		RadioSwitch()	  
**brief      :		start radioplayer ,on current frequency
**param   :		ONAndOff :		1  turn on radio
**				        			0	turn off radio	
**return   :		AK_TRUE ,initialize successfully , AK_FLASE  :	intialize failed
**author  :	      YaoHongshi
**date     : 	      2008-04-09->(2008-08-21)
***********************************************************************************/
T_BOOL	Radio_Switch(SWITCH switch_state);


/***********************************************************************************
**name: 		RadioConserveData()    
**brief:		conserve radio parameters current ,for next running
**param:		NO	
**return:		NO
**author:	YaoHongshi
**date: 	2008-04-09
***********************************************************************************/
T_VOID	Radio_ConserveData(T_VOID);


/*******************************************************************************
**brief     :   set radio mute
**param     :   NO    
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_VOID Radio_SetMute(T_VOID);


/***********************************************************************************
**brief:    free radioplayer
**param: NO		
**return: NO		
***********************************************************************************/
T_VOID Radio_PlayerFree(T_VOID);


/***********************************************************************************
**brief: 		set radio volume
**param:		search direction	
**return:		
**author:	       YaoHongshi
**date:		2008-04-09
***********************************************************************************/
T_BOOL Radio_SetVolume(T_VOID);


/*******************************************************************************
**brief     :   get radio volume
**param     :   NO
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_U8 Radio_GetVolume(T_VOID);


/***********************************************************************************
**brief     :   set radio volume add
**param     :   no    
**return    :       
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL Radio_AddVolume(T_VOID);


/***********************************************************************************
**brief     :   set radio volume sub
**param     :   no    
**return    :       
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL Radio_SubVolume(T_VOID);


/***********************************************************************************
**name: 		RadioAutoSearchStop()
**brief:		stop auto search
**param:			
**return:		
**author:	YaoHongshi
**date: 	2008-04-09
***********************************************************************************/
T_BOOL Radio_AutoSearch(T_VOID);


/***********************************************************************************
**brief     :   radio jump channel
**param     :   [in] :dir   
**return    :       
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL Radio_JumpChannel(T_U8 dir);


#else

#define DEF_VOLUME          (20)

typedef struct {
    T_U32                   CurFreq;
    T_U32                   SavedFreq;
    T_U32                   ChannelList[40];
    T_U8                    Volume;
    T_RADIO_AREA            RadioArea;
    T_U8                    prePos;
    T_U8                    CheckCode;    // Used to validate the config.
}T_RADIO_CFG;



/***********************************************************************************
**name      :   radioplayerinit()    
**brief     :   initialize radiopalyer default param and hardware
**param     :   pRParam :radio struct  pointer (ref: Fwl_pfRadio.h)
**return    :   AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
**author    :   YaoHongshi
**date      :   2008-04-09 ->(2008-08-21M)
***********************************************************************************/
T_BOOL Radio_InitPlayer( T_RADIO_PLAYER_PARM * pRParam);


/***********************************************************************************
**name      :   Radio_Switch()    
**brief     :   start radioplayer ,on current frequency
**param     :   ONAndOff :  1  turn on radio
**                          0  turn off radio  
**return    :   AK_TRUE ,initialize successfully , AK_FLASE  :  intialize failed
**author    :   YaoHongshi
**date      :   2008-04-09->(2008-08-21)
***********************************************************************************/
T_BOOL Radio_Switch(SWITCH switch_state);


/***********************************************************************************
**name      :   Radio_JumpBandWidth()    
**brief     :   jump to next station with adding or  
**param     :   ONAndOff :  1  forward
**                          0  back 
**return    :   NO
**author    :   YaoHongshi
**date      :   2008-04-09
***********************************************************************************/
T_VOID Radio_JumpBandWidth(T_S16 dir);

    
/***********************************************************************************
**name      :   Radio_SetSearchFreq()   
**brief     :   set search frequency
**param     :   search direction    
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
//change by lsk 2011-5-26
***********************************************************************************/
T_BOOL Radio_SetSearchFreq(T_BOOL bSerchfirst);

    
/***********************************************************************************
**name      :   Radio_StartAutoSearch()   
**brief     :   start auto rearch stations
**param     :   dir : radio auto search direction   
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
//change by lsk 2011-5-26
***********************************************************************************/
T_VOID Radio_StartAutoSearch(T_RADIO_STATE dir, T_BOOL bSerchFirst);


/***********************************************************************************
**name      :   Radio_IsFindStation()     
**brief     :   get FM's state ,check wether find a station
**param     :   FreqRet: the pointer that station found will be saved   
**return    :   AK_TRUE = find a station  AK_FALSE = do not find a station   
**author    :   YaoHongshi
**date      :   2008-04-09
***********************************************************************************/
T_BOOL Radio_IsFindStation(T_U32 *FreqRet);


/***********************************************************************************
**name      :   RadioAutoSearchStop()    
**brief     :   stop auto search
**param     :            
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
***********************************************************************************/
T_VOID Radio_StopAutoSearch(T_VOID);


/***********************************************************************************
**name      :   RadioConserveData()    
**brief     :   conserve radio parameters current ,for next running
**param     :   NO  
**return    :   NO
**author    :   YaoHongshi
**date      :   2008-04-09
***********************************************************************************/
T_VOID Radio_ConserveData(T_VOID);


/***********************************************************************************
**brief     :   free radioplayer
**param     :   NO     
**return    :   NO        
***********************************************************************************/
T_VOID Radio_FreePlayer(T_VOID);


/**********************************************************
**brief     :   ���Ƶ�ε��б���ȥ����ͨ��limtָ����ӵ�λ��
**              �����׷�ӵ���ǰ�б�
**param     :   freq:   Ҫ��ӵ�Ƶ�ʣ� Limt: �޶�λ�� 
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_BOOL Radio_AddChanToList(T_U32 freq, T_BOOL Limt);


/**********************************************************
**brief     :   delete the station in the station list
**param(in) :   DelType AK_TRUE : delete all station ,else AK_FALSE :delete current frequency
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_BOOL Radio_DelChanFromList(T_BOOL DelType);


/**********************************************************
**brief     :   ���ݵ�ǰƵ�ʲ��������б��е�λ�ã������浽
**              T_RADIO_PLAYER_PARM�е�ch_pos��
**param     :   freq:Ҫ���ҵ�Ƶ�ʣ� PosLimt: ���ҵ����Χ����
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_U8 RadioGetChPos(T_U32 freq, T_U8 PosLimt);


/**********************************************************
**brief     :   �ӵ�̨�б��еĹ̶�λ�û�ñ���ĵ�̨Ƶ��      
**param     :   pos:ָ��ѡ��ĵ�̨���б��е�λ��
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_U32 RadioGetChFromList(T_U8 pos);


/**********************************************************
**brief     :   ��鵱ǰ��̨�Ƿ�����������̨
**param     :   param:  ��Ҫ���ĵ�̨����������μ�T_RADIO_PARM�ṹ��
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_BOOL RadioSteCheck(T_RADIO_PARM_EX *param);


/**********************************************************
**brief     :   ��õ�ǰƵ�������СƵ�ʵĿ��
**param     :
**return    :   bandwidth
**author    :   Yao Hongshi
**date      :   2008-06-02
*************************************************************/
T_U32 RadioGetBand(T_VOID);


/*******************************************************************************
**brief     :   set radio mute
**param     :   NO    
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_BOOL RadioSetMute(T_VOID);


/***********************************************************************************
**brief     :   set radio volume
**param     :   no    
**return    :       
**author    :   YaoHongshi
**date      :   2008-04-09
***********************************************************************************/
T_BOOL RadioSetVolume(T_VOID);


/*******************************************************************************
**brief     :   get radio volume
**param     :   NO
**return    :   
**author    :   YaoHongshi
**date      :   2008-04-09
*******************************************************************************/
T_U8 RadioGetVolume(T_VOID);


/***********************************************************************************
**brief     :   set radio volume add
**param     :   no    
**return    :       
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL RadioAddVolume(T_VOID);


/***********************************************************************************
**brief     :   set radio volume sub
**param     :   no    
**return    :       
**author    :   hyl
**date      :   2013-03-26
***********************************************************************************/
T_BOOL RadioSubVolume(T_VOID);


T_VOID RadioSearchFunc(T_EVT_PARAM *pEventParm);

#endif
#else

#define RadioSearchFunc(param)
#define RadioGetVolume()            (0)
#define RadioAddVolume()            (AK_FALSE)
#define RadioSubVolume()            (AK_FALSE)


#endif

#endif
