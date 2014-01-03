#ifndef _AVF_PLAYER_LIB_H_
#define _AVF_PLAYER_LIB_H_

#include "medialib_struct.h"

#define AVFPLAYER_VERSION	"V0.1.0"

T_BOOL AVFPlayer_ChkAVFType(T_U16 *filename,T_U32 len);

T_pVOID AVFPlayer_Open(T_MEDIALIB_OPEN_INPUT *open_input);

T_BOOL AVFPlayer_Close(T_pVOID hAVF);

T_BOOL AVFPlayer_GetInfo(T_pVOID hAVF, T_MEDIALIB_MEDIA_INFO *pInfo);

T_S32 AVFPlayer_Play(T_pVOID hAVF);

T_BOOL AVFPlayer_Stop(T_pVOID hAVF);

T_BOOL AVFPlayer_Pause(T_pVOID hAVF);

T_BOOL AVFPlayer_FastForward(T_pVOID hAVF);

T_BOOL AVFPlayer_FastRewind(T_pVOID hAVF);

T_eMEDIALIB_STATUS AVFPlayer_GetStatus(T_pVOID hAVF);

T_S32 AVFPlayer_Handle(T_pVOID hAVF);

T_S32 AVFPlayer_SetPosition(T_pVOID hAVF, T_S32 lMilliSec);

T_S32 AVFPlayer_SetCurTime(T_pVOID hAVF, T_U32 time_ms);

T_S32 AVFPlayer_GetCurTime(T_pVOID hAVF, T_U32 *time_ms);
#endif//_AVF_PLAYER_LIB_H_
