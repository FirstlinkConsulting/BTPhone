/**
 * @file Eng_Math.h
 * @brief This header file is for OS related function prototype
 * 
 */
#ifndef __FWL_MATH_H__
#define __FWL_MATH_H__



#include "anyka_types.h"

T_VOID  Fwl_RandSeed(T_VOID);
T_U32   Fwl_GetRand(T_U32 maxVal);
T_S8    U64cmpU32(T_U64_INT *size64, T_U32 size32);

#endif


