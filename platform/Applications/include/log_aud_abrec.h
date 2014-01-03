/**
 * @file    log_aud_abrec.h
 * @brief   manage record during AB play 
 * @author  WuShanwei
 * @date    2008-05-30
 * @version 1.0
 */

#ifndef __H_LOG_AUD_ABREC_
#define __H_LOG_AUD_ABREC_

#include "log_aud_play.h"


#ifdef SUPPORT_MUSIC_PLAY

//#define MSC_REC_AB_PLY_TYPE_STOPANDPLAY
#define MSC_REC_AB_SUPPORT_REAL_REC



/** AB record control */
typedef struct{
    T_USTR_FILE     fileNameRec;            /* pointer for path content */
    T_AUD_PLAYER*   pPlayerRec;             /* record player            */
    T_AUD_PLAYER*   pPlayerAud;             /* music player             */
    T_U32           recTimerCnt;            /* timer counter for record */
    T_U32           recTimerAll;            /* total time need to record*/
   T_WSOLA_ARITHMATIC           speedLibType;
} T_AUD_AB_REC;

/**
 * @brief   ab record init
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   T_VOID
 * @return  T_AUD_AB_REC*
 * @retval  
 **/
T_AUD_AB_REC* Aud_ABRecInit(T_VOID);

/**
 * @brief   ab record destroy
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecDestroy(T_AUD_AB_REC* pABRec);

/**
 * @brief   begin record
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
            recTotTime
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRBegin(T_AUD_AB_REC* pABRec,T_U32 recTotTime);

/**
 * @brief   delete old record file
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
            recTotTime
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRDelOldRec(T_AUD_AB_REC* pABRec);

/**
 * @brief   auto check record end
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecRCycCheckEnd(T_AUD_AB_REC* pABRec,T_U32 timeGapAdd);

/**
 * @brief   end record
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecREnd(T_AUD_AB_REC* pABRec);

/**
 * @brief   begin record play
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecPBegin(T_AUD_AB_REC* pABRec);

/**
 * @brief   begin record end
 * @author  WuShanwei
 * @date    2008-05-30
 * @param   pABRec
 * @return  T_BOOL
 * @retval  
 **/
T_BOOL  Aud_ABRecPEnd(T_AUD_AB_REC* pABRec);

#endif

#endif
