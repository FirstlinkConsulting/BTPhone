/*******************************************************************************
 * @file hal_radio.h
 * @brief radio device header file
 * This file provides radio APIs: radio initialization, play radio, etc..
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2005-03-07
 * @version 1.0
 * @ref AK3210M technical manual.
*******************************************************************************/
#ifndef __HAL_RADIO_H__
#define __HAL_RADIO_H__


#include "anyka_types.h"


//radio define
#define RADIO_ADC_LEVEL             6
#define BAND_WIDTH                  100000


/*
typedef enum{
    RADIO_EUROPE = 0,
    RADIO_JAPAN,
    RADIO_AMERICA
}T_RADIO_AREA;

typedef struct{
    T_RADIO_AREA    Area;
    T_U8            Rssi;   
    T_U32           CurFreq;
    T_U32           FreqStep;
    T_U16           Volume;
}T_RADIO_INIT;
*/
typedef struct{
    T_BOOL      ReadyFlag;      //1:find a station or freq reach band limit  0:not find a station
    T_BOOL      LimitFlag;      //1:reach the band limit        0:not reach band limit
    T_U32       CurFreq;
    T_U8        IFCounter;
    T_U8        AdcLevel;       //adc level set:0~15
    T_BOOL      Stereo;         //stereo indicator   1:stereo  0: mono
}T_RADIO_STATUS;

typedef struct{
    T_BOOL      MuteFlag;       //1:mute  0:not mute
    T_BOOL      SearchFlag;     //1:in search mode  0:not search
    T_BOOL      SearchDir;      //1:search up  0:search down
    T_BOOL      MonoStereo;     //1:mono       0:stereo
    T_U32       Freq;           //
    T_U8        StopLevel;      //0~3 if not in search mode, 0 is not allowed
    T_BOOL      BandLimit;      //1: japan FM band    0:us/europe FM band
    T_U8        Volume;
}T_RADIO_PARM;


T_BOOL radio_check(T_VOID);


T_BOOL radio_init(T_VOID);


T_BOOL radio_get_status(T_RADIO_STATUS *Status, T_U32 FreqMin, T_U32 FreqMax);


T_BOOL radio_set_param(T_RADIO_PARM *Param);


T_BOOL radio_exit(T_VOID);


T_VOID radio_line_in(T_BOOL enable);


//T_VOID radio_read(T_U8 *DataTmp);


T_BOOL radio_set_volume(T_U8 DataTmp);

T_BOOL radio_search_freq(T_U32 *data);

#endif

/* end of __HAL_RADIO_H__ */
