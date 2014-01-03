/*
 * @(#)Unicode.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef    _UNICODE_H_
#define    _UNICODE_H_

#include "anyka_types.h"

T_U32 Uni_AscSearch(const T_U8 *buf, T_U8 b, T_U32 count);
T_U32 Uni_UniSearch(const T_U16* str, T_U16 ch, T_U32 count);
T_U32 Uni_LongSearch(const T_U32 *buf, T_U32 l, T_U32 count);
T_U16 Uni_ToUpper(T_U16 uni);

#endif




