#ifndef _BA_OPP_H_
#define _BA_OPP_H_

#if defined __cplusplus
extern "C" {
#endif

#include "anyka_types.h"
#include "ba_lib_api.h"
typedef struct
{
	T_U16 *name;
	T_U16 name_length;
	T_U32 file_length;
	T_U16 body_length;
	T_U8 *body;
} BA_OPP_Initial_Packet; //First OPP Put packet, should contain the name and size of incoming file

T_VOID BA_OPP_Start(T_VOID);

T_VOID BA_OPP_Stop(T_VOID);

T_BOOL BA_OPP_RECEIVED_RESPONSE_Success(T_VOID);

T_BOOL BA_OPP_OPERATION_RESPONSE_Abort(T_VOID);

T_BOOL BA_OPP_OPERATION_RESPONSE_Disconnect(T_VOID);

T_BOOL BA_OPP_OPERATION_RESPONSE_Connect(T_VOID);

T_BOOL BA_OPP_OPERATION_RESPONSE_Continue(T_VOID);

T_BOOL BA_OPP_OPERATION_RESPONSE_Success(T_VOID);

T_BOOL BA_OPP_OPERATION_Disconnect(T_VOID);

T_BOOL BA_OPP_OPERATION_Abort(T_VOID);

T_BOOL BA_OPP_AcceptRFCOMMConnection(T_VOID);

T_BOOL BA_OPP_AcceptPUTObject(T_VOID);


#if defined __cplusplus
}
#endif

#endif // _BA_HFP_H_

