/*******************************************************************************
 * @file    Fwl_Radio.h
 * @brief   This header file is for radio function prototype
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @version 1.0
*******************************************************************************/
#ifndef __FWL_RADIO_H__
#define __FWL_RADIO_H__


#include "anyka_types.h"

//#ifdef SUPPORT_RADIO_RDA5876
#if 1
#define RADIO_VALID_FREQ_COUNT      20
#else
#define	RADIO_STATION_NUM			20	//saved station max number
#endif

#define RADIO_ADC_LEVEL             6


//radio define
#define RADIO_FREQ_NULL             0
#define VOLUME_MAX                  31
#define BAND_WIDTH                  100000   //

#define MAX_RADIO_AREA              3

typedef enum {
    RADIO_STOP = 0,
    RADIO_PLAY,
    RADIO_SEARCH_PREV,
    RADIO_SEARCH_NEXT,
    RADIO_SEARCH_ALL,
    RADIO_PAUSE
}T_RADIO_STATE;

typedef enum{
    RADIO_NOFM=0,
    RADIO_TEA5767,
    RADIO_RDA5807,
    RADIO_RDA5876,
    RADIO_MAXNUM
}T_RADIO_TYPE;

typedef enum {
    RADIO_EUROPE = 0,
    RADIO_JAPAN,
    RADIO_AMERICA
}T_RADIO_AREA;

typedef struct{
    T_BOOL          ReadyFlag;      //1:find a station or freq reach band limit  0:not find a station
    T_BOOL          LimitFlag;      //1:reach the band limit  0:not reach band limit
    T_U32           CurFreq;
    T_U8            IFCounter;
    T_U8            AdcLevel;       //adc level set:0~15
    T_BOOL          Stereo;         //stereo indicator  1:stereo  0: mono
}T_RADIO_STATUS_EX;

typedef struct{
    T_BOOL          MuteFlag;       //1:mute  0:not mute
    T_BOOL          SearchFlag;     //1:in search mode  0:not search
    T_BOOL          SearchDir;      //1:search up  0:search down
    T_BOOL          MonoStereo;     //1:mono  0:stereo
    T_U32           Freq;           //
    T_U8            StopLevel;      //0~3 if not in search mode, 0 is not allowed
    T_BOOL          BandLimit;      //1: japan FM band  0:us/europe FM band
    T_U8            Volume;
}T_RADIO_PARM_EX;

typedef struct{
    T_RADIO_AREA    Area;
    T_U8            Rssi;   
    T_U32           CurFreq;
    T_U32           FreqStep;
    T_U16           Volume;
}T_RADIO_INIT;

//#ifdef SUPPORT_RADIO_RDA5876
#if 1
typedef struct {
    T_RADIO_STATE   RadioState;
    T_RADIO_AREA    RadioArea;
	T_U8            Volume;	
	T_BOOL			bHWER;          //AK_TRUE == hardware error
	
//    T_U32           Stimer;         //search timer

    T_U32           CurFreq;
    T_U32           FreqMin;	    //当前区域电台最小电台频率
    T_U32           FreqMax;        //当前区域电台最大电台频率

    T_U32			*channelList;
	T_BOOL			bStereo;         //AK_TRUE == stereo
}T_RADIO_PLAYER_PARM;
#else
typedef struct {
    T_RADIO_STATE   RadioState;
    T_RADIO_AREA    RadioArea;
    T_U32           Stimer;         //search timer

    T_U32           CurFreq;
    T_U32           SavedFreq;
    T_U32           FreqMin;        //当前区域电台最小电台频率
    T_U32           FreqMax;        //当前区域电台最大电台频率
    T_U32           *CurList;       //channelAll中的电台分为两组，指向后20个未JAP，否则未EU&US
    T_U32           *channelAll;    //电台列表，分为两段，前20个为一个AREA，后20个为一个AREA

    T_U8            Volume; 
    T_U8            ch_pos;         //current frequency position in channel list
    T_U8            preChPos;       // add by Zhao_xiaowei 2009 

    T_BOOL          bHWER;          //AK_TRUE == hardware error
    T_BOOL          bStereo;        //AK_TRUE == stereo
}T_RADIO_PLAYER_PARM;

#endif

/*******************************************************************************
 * @brief   initialize radio
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioInit(T_VOID);


/*******************************************************************************
 * @brief   read the param into radio register
 * @author  guohui
 * @date    2007-03-20
 * @param   [out]status: refer to radio.h
 * @param   [in]freqMin: 
 * @param   [in]freqMax: 
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioGetStatus(T_RADIO_STATUS_EX *status, T_U32 freqMin, T_U32 freqMax);


/*******************************************************************************
 * @brief   write the param into radio register
 * @author  guohui
 * @date    2007-03-20
 * @param   [in]param: refer to radio.h
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioSetParam (T_RADIO_PARM_EX *param);


/*******************************************************************************
 * @brief   set radio volume
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]volume: the volume to be set
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioSetVolume(T_U8 volume);


/*******************************************************************************
 * @brief   free radio
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioFree(T_VOID);


/*******************************************************************************
 * @brief   auto search radio station
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]param: radio station array
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL	Fwl_Radio_AutoSearch(T_U32 * pChanList);


#endif //__FWL_RADIO_H__

