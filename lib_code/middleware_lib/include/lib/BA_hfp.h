#ifndef _BA_HFP_H_
#define _BA_HFP_H_

#if defined __cplusplus
extern "C" {
#endif

#include "anyka_types.h"
#include "ba_lib_api.h"

//T_BOOL BA_HFP_VoiceRecognition(T_U8 activation);

T_VOID BA_HFP_Start(T_VOID);

T_S32 BA_HFP_Stop(T_VOID);

T_BOOL BA_HFP_Connect(T_BD_ADDR addr);

T_BOOL BA_HFP_Disconnect(T_VOID);

typedef T_VOID (*T_AT_CMD_CALLBACK)(T_VOID *para, T_U8 *result, T_U16 size);
T_BOOL BA_HFP_SendCustomATCommand(T_U8 *AT_command, T_AT_CMD_CALLBACK pCallBack, T_VOID *para, T_U16 size);

T_BOOL BA_HFP_AnswerCall(T_VOID);

T_BOOL BA_HFP_CancelCall(T_VOID);

T_BOOL BA_HFP_CallDial(T_pCDATA psz);

T_BOOL BA_HFP_UpdateSpeakerVolume(T_U8 vol);

T_BOOL BA_HFP_UpdateMicrophoneVolume(T_U8 vol);

T_BOOL BA_HFP_ConnectSCO(T_VOID);

T_BOOL BA_HFP_DisconnectSCO(T_VOID);

T_BOOL BA_HFP_SendDataSCO(T_U8 *sco_data, T_U16 size);

T_BOOL BA_HFP_UpdateIphoneBattery(T_U8 battery, T_U8 dock);

T_U8 BA_HFP_GetCurrentStatus(T_VOID);



#if defined __cplusplus
}
#endif

#endif // _BA_HFP_H_

