/**
 * @file    log_aud_ab.c
 * @brief   manage AB play 
 * @author  WuShanwei
 * @date    2008-05-30
 * @version 1.0
 */
#include "Gbl_Global.h"
#include "akdefine.h"
#include "Eng_debug.h"
#include "Fwl_osFS.h"
#include "Eng_DataConvert.h"
#include "eng_string_uc.h"
#include "log_aud_play.h"
#include "log_aud_ab.h"
#include "Fwl_Timer.h"
#include "Fwl_osMalloc.h"
#include "media_lib.h"
#include "Fwl_System.h"
//#include "l2.h"
#include "Eng_Profile.h"
#include "Fwl_MicroTask.h"
#include "log_pcm_player.h"


typedef struct
{
    T_U32           timeA;
    T_U32           timeB;
    T_AUD_PLAYER*   abmuc_player;
    T_AUD_AB_REC*   abrec_handle;  
}T_ABCTRL_INFO;


T_ABCTRL_INFO abctrl;

//#ifdef SUPPORT_AUDIO_AB

#ifdef SUPPORT_MUSIC_PLAY
#ifdef SUPPORT_AUDIO_AB

static T_AUD_PLAYER* Aud_CtrlABPre(T_VOID)
{
    return abctrl.abmuc_player;
}

static T_VOID Aud_CtrlABPost(T_S32 size, T_AUD_PLAYER* pAudPlayer)
{
    T_eMEDIALIB_STATUS decStat = MEDIALIB_END;

    if (size < 0)
    {//decode failed 
        decStat = MediaLib_GetStatus(abctrl.abmuc_player->hMedia);
    
        if(MEDIALIB_PAUSE == decStat)
        {
            if (!Pcm_CheckPause())
            {   
                Pcm_Pause(abctrl.abmuc_player->pcm_id);
            }
        }
        else
        {//other end including file end or decode error
            if (!Pcm_CheckPause())
            {           
                Pcm_Pause(abctrl.abmuc_player->pcm_id);
                //视频播放中快进快退中到首尾不播放其它歌曲
                if ((MEDIALIB_END == decStat)) 
                {  
                    VME_EvtQueuePut(M_EVT_ABPLAY_FILEEND, AK_NULL);
                }
                else
                {
                    VME_EvtQueuePut(M_EVT_ABPLAY_DECERR, AK_NULL);
                }
    
            }
        }
    }
}



/*********************************************************************
  Function:         Aud_CtrlABInit
  Description:      ab player init.
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/

T_BOOL Aud_CtrlABInit(T_VOID)
{
    abctrl.timeA = 0;
    abctrl.timeB = 0;
    abctrl.abmuc_player = AK_NULL;

    APlayer_RegHDLPre(Aud_CtrlABPre);
    APlayer_RegHDLPost(Aud_CtrlABPost);
    return AK_TRUE;
}

/*********************************************************************
  Function:         Aud_CtrlABDestroy
  Description:      ab palyer destory.
  Input:            T_U16 *filepath:the file will be played after eixt the ab player;
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/

T_BOOL  Aud_CtrlABDestroy(T_VOID)
{
    abctrl.abmuc_player = AK_NULL;

    return AK_TRUE;
}

/*********************************************************************
  Function:         Aud_CtrlABOpen
  Description:      abplayer open
  Input:            player: the pointer of the struct T_AUD_PLAYER
  Input:            filepath: the music file path
  Input:            timeA: the start time you want to setted
  Input:            timeA: the end time you want to setted
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/

T_BOOL Aud_CtrlABOpen(T_AUD_PLAYER* player, T_USTR_FILE filepath, T_U32 timeA, T_U32 timeB)
{
    if (AK_NULL == player)
    {
        AK_DEBUG_OUTPUT("abplayer player is null\n");
        return AK_FALSE;
    }
    
    AK_DEBUG_OUTPUT("abpalyer open timeA:%d, timeB:%d, path:", timeA, timeB);
    Printf_UC(filepath);

    abctrl.timeA = timeA;
    abctrl.timeB = timeB;
    
    abctrl.abmuc_player = player;

    if(!APlayer_Open(player,filepath,MEDIALIB_MEDIA_UNKNOWN,AK_FALSE,0, 0, AUD_SPEED_ARITHMETIC))
    {
        AK_DEBUG_OUTPUT("abpalyer open failed!\n");
        return AK_FALSE;
    }
        
    return AK_TRUE;
}

 /*********************************************************************
  Function:         Aud_CtrlABPlay
  Description:      abplayer start play
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/ 
T_BOOL Aud_CtrlABPlay(T_VOID)
{
    
    AK_DEBUG_OUTPUT("abplayer play!\n");

    if (0 != abctrl.timeA)  
    {
        AK_DEBUG_OUTPUT("abplayer set timeA: %d", abctrl.timeA);
        APlayer_Seek(abctrl.abmuc_player, abctrl.timeA);
    }   
        
    return APlayer_Resume(abctrl.abmuc_player);   
}

  /*********************************************************************
  Function:         Aud_CtrlABStop
  Description:      abplayer stop
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/
T_BOOL Aud_CtrlABStop(T_VOID)
{
    APlayer_Pause(abctrl.abmuc_player);
    APlayer_Stop(abctrl.abmuc_player);
    AK_DEBUG_OUTPUT("abplayer stop!\n");
    return AK_TRUE;
}

 /*********************************************************************
  Function:         Aud_CtrlABStartRec
  Description:      ab player start to record
  Input:            filepath: the record file path
  Input:            totaltime:the total you want to record
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/ 
T_BOOL Aud_CtrlABStartRec(T_USTR_FILE filepath, T_U32 totaltime)
{
    abctrl.abrec_handle = Aud_ABRecInit();

    if (AK_NULL == abctrl.abrec_handle)
        return AK_FALSE;

    UStrCpy(abctrl.abrec_handle->fileNameRec, filepath);
    return Aud_ABRecRBegin(abctrl.abrec_handle, totaltime);

}


 /*********************************************************************
  Function:         Aud_CtrlABStopRec
  Description:      ab player stop  record
  Input:            T_VOID
  Return:           succeed:ak_true    failed:ak_false
  Author:           lishengkai
  Data:             2013-3-11
**********************************************************************/ 
T_BOOL Aud_CtrlABStopRec(T_VOID)
{
    return Aud_ABRecREnd(abctrl.abrec_handle);
}

#endif
#endif
