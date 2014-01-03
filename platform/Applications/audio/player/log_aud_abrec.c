/**
 * @file    log_aud_abrec.c
 * @brief   manage record during AB play 
 * @author  WuShanwei
 * @date    2008-05-30
 * @version 1.0
 */
#include "Gbl_Global.h"
#include "akdefine.h"
#include "Eng_debug.h"
#include "Fwl_osFS.h"
#include "Fwl_osMalloc.h"
//#include "file.h"
#include "Eng_DataConvert.h"
#include "eng_string_uc.h"
#include "log_aud_play.h"
#include "log_aud_abrec.h"
#include "audio_lib.h"
#include "log_aud_play.h"
#include "log_record.h"
#include "log_aud_control.h"
#include "Eng_String.h"

#ifdef SUPPORT_MUSIC_PLAY
#ifdef SUPPORT_AUDIO_AB

/**
 * @brief   ab record init
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   T_VOID
 * @return  T_AUD_AB_REC*
 * @retval  
 **/
T_AUD_AB_REC* Aud_ABRecInit(T_VOID)
{
    T_AUD_AB_REC* pABRec;
    pABRec = (T_AUD_AB_REC*)Fwl_Malloc(sizeof(T_AUD_AB_REC));
    if(AK_NULL == pABRec)
    {
        return AK_NULL; 
    }
    memset(pABRec,0,sizeof(T_AUD_AB_REC));

    pABRec->pPlayerRec = (T_AUD_PLAYER*)Fwl_Malloc(sizeof(T_AUD_PLAYER));
    if(AK_NULL == pABRec->pPlayerRec)
    {
        return AK_FALSE;
    }
    memset((void*)(pABRec->pPlayerRec),0,sizeof(T_AUD_PLAYER));
    //pABRec->pPlayerRec->flagHDPopen = AK_TRUE;
    return pABRec;
}

/**
 * @brief   ab record destroy
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecDestroy(T_AUD_AB_REC* pABRec)
{
    if(AK_NULL == pABRec)
        return AK_FALSE;

    Aud_ABRecRDelOldRec(pABRec);
    
    pABRec->pPlayerRec = Fwl_Free(pABRec->pPlayerRec);
    pABRec = Fwl_Free(pABRec);
    pABRec = AK_NULL;
    return AK_TRUE;
}

/**
 * @brief   begin record
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
            recTotTime
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRBegin(T_AUD_AB_REC* pABRec,T_U32 recTotTime)
{
#ifdef MSC_REC_AB_SUPPORT_REAL_REC
    T_U32           RecSec = 0;

    //创建默认目录
//    Eng_MultiByteToWideChar(AUD_AB_REC_FILE_PATH, Utl_StrLen(AUD_AB_REC_FILE_PATH), foldpath, MAX_FILE_LEN, AK_NULL);
//  Fwl_FsMkDir((const T_U16*)foldpath);

    AudioRecord_Init();
//  Eng_MultiByteToWideChar(AUD_AB_REC_FILE, Utl_StrLen(AUD_AB_REC_FILE), pABRec->fileNameRec, MAX_FILE_LEN, AK_NULL);
    AudioRecord_SetDataSource(eSTAT_RCV_MIC);

    RecSec = AudioRecord_GetTotalTime((pABRec->fileNameRec)[0], eREC_MODE_ADPCM16K);
    //录音空间不足
    if((RecSec < 3) || (1000*RecSec < recTotTime))
    {
        AK_DEBUG_OUTPUT("Aud_ABRecRBegin,err1\n");
        AudioRecord_Destroy();
//          Aud_AudUISetErr(AUD_ERR_TYPE_MEMORYFULL);
        return AK_FALSE;
    }

    if (!AudioRecord_Start(pABRec->fileNameRec, eREC_MODE_ADPCM16K, 1024, AK_FALSE))
    {
        AK_DEBUG_OUTPUT("Aud_ABRecStart,err2\n");
        AudioRecord_Destroy();
//          Aud_AudUISetErr(AUD_ERR_TYPE_MEMORYFULL);
        return AK_FALSE;
    }
    //Printf_UC(pABRec->fileNameRec);
    pABRec->recTimerCnt = 0;    
    pABRec->recTimerAll = recTotTime;
    AK_DEBUG_OUTPUT("Aud_ABRecRBegin,recTotTime:%d\n",recTotTime);
#endif
    return AK_TRUE;
}

/**
 * @brief   delect old record file
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
            recTotTime
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRDelOldRec(T_AUD_AB_REC* pABRec)
{
    //AK_DEBUG_OUTPUT("Aud_ABRecRDelOldRec,p:%d\n",*pABRec->fileNameRec);
    if((*pABRec->fileNameRec) != 0)
    {
        AK_DEBUG_OUTPUT("delete rec file");
        Fwl_FileDelete(pABRec->fileNameRec);
        *pABRec->fileNameRec = 0;
        return AK_TRUE;
    }

    return AK_FALSE;
}

/**
 * @brief   auto check record end
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRCycCheckEnd(T_AUD_AB_REC* pABRec,T_U32 timeGapAdd)
{
    //pABRec->recTimerCnt += timeGapAdd;
#ifdef OS_WIN32
    pABRec->recTimerCnt += timeGapAdd;
#else
    pABRec->recTimerCnt = AudioRecord_GetCurrentTime();
#endif
    if(pABRec->recTimerCnt >= pABRec->recTimerAll)
    {
        AK_DEBUG_OUTPUT("Aud_ABRecRCycCheckEnd,end,timeAll:%d\n",pABRec->recTimerAll);
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

/**
 * @brief   end record
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecREnd(T_AUD_AB_REC* pABRec)
{
#ifdef MSC_REC_AB_SUPPORT_REAL_REC
    AudioRecord_Stop();
    AudioRecord_Destroy();
    AK_DEBUG_OUTPUT("Aud_ABRecREnd end\n");
#endif
    return AK_TRUE;
}

#endif
#endif
