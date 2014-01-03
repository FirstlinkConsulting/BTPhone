#ifndef _BA_A2DP_H_
#define _BA_A2DP_H_

#if defined __cplusplus
extern "C" {
#endif

#include "BA_lib_api.h"


T_S32 BA_A2DP_Start(T_VOID);
T_S32 BA_A2DP_Stop(T_VOID);

T_S32 BA_A2DP_Connect(T_BD_ADDR bd_addr);
T_S32 BA_A2DP_Disconnect(T_VOID);


#if defined __cplusplus
}
#endif

#endif // _BA_A2DP_H_


