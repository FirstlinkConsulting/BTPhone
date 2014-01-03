/**
 * @file    log_aud_abrec.c
 * @brief   manage AB play 
 * @author  WuShanwei
 * @date    2008-05-30
 * @version 1.0
 */

#ifndef __H_LOG_AUD_AB_
#define __H_LOG_AUD_AB_


#include "log_aud_abrec.h"


#ifdef SUPPORT_MUSIC_PLAY

#if  1

/*********************************************************************
  Function:         Aud_CtrlABInit
  Description:      ab player init.
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABInit(T_VOID);


/*********************************************************************
  Function:         Aud_CtrlABDestroy
  Description:      ab palyer destory.
  Input:            T_U16 *filepath:the file will be played after eixt the ab player;
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL  Aud_CtrlABDestroy(T_VOID);


/*********************************************************************
  Function:         WaveIn_SetMicGain
  Description:      abplayer open
  Input:            player: the pointer of the struct T_AUD_PLAYER
  Input:            filepath: the music file path
  Input:            timeA: the start time you want to setted
  Input:            timeA: the end time you want to setted
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABOpen(T_AUD_PLAYER* player, T_USTR_FILE filepath, T_U32 timeA, T_U32 timeB);


 /*********************************************************************
  Function:         Aud_CtrlABPlay
  Description:      abplayer start play
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABPlay(T_VOID);
 
 
 /*********************************************************************
  Function:         Aud_CtrlABStop
  Description:      abplayer stop
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABStop(T_VOID);
 
 
 /*********************************************************************
  Function:         Aud_CtrlABStartRec
  Description:      ab player start to record
  Input:            filepath: the record file path
  Input:            totaltime:the total you want to record
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABStartRec(T_USTR_FILE filepath, T_U32 totaltime);
 
 
 /*********************************************************************
  Function:         Aud_CtrlABStopRec
  Description:      ab player stop  record
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABStopRec(T_VOID);




#endif
#endif
#endif
