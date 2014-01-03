#ifndef __UTILS_H
#define __UTILS_H

#include "anyka_types.h"
#include "Fwl_Serial.h"

int strcmp(const char *s1, const char *s2);
void akmemset(unsigned long *s, unsigned long c, int n);
T_BOOL akmemcmp(T_U32 *p, T_U32 len);

/*@brief calculate the bit number of a integer
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 *@author Justin Zhao
 *@date 2008.1.25
 *@output: T_U8 return the bit number
 *@version 1.0
*/
T_U8 bitnum(T_U32 i);

T_BOOL lock_valid_addr(T_U32 data, T_U32 nBufLen, T_BOOL bLock, T_BOOL bEverLocked);

#endif /* __UTILS_H */
