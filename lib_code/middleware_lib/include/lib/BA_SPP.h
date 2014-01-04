#ifndef _BA_SPP_H_
#define _BA_SPP_H_

#if defined __cplusplus
extern "C" {
#endif

#include "anyka_types.h"
#include "ba_lib_api.h"

T_S32 BA_SPP_Start(T_S32 nOptions);
T_S32 BA_SPP_Stop(T_VOID);
T_S32 BA_SPP_Send(const void* pData, T_S32 nSize);

#if defined __cplusplus
}
#endif

#endif // _BA_SPP_H_

