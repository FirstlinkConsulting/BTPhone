/**
 * @file Eng_String.h
 * @brief ANYKA software
 * this header file provide hal layer string function 
 */

#ifndef __UTLSTRING_H
#define __UTLSTRING_H

#include "anyka_types.h"
//#include "Gbl_Global.h"
#include "eng_string_uc.h"

/* string functions*/
T_U16   Utl_StrLen(T_pCSTR strMain);
T_S8    Utl_StrCmp(T_pCSTR str1, T_pCSTR str2);
T_S8    Utl_StrCmpC(T_pCSTR str1, T_pCSTR str2);
T_S8	Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_pSTR  Utl_StrCpy(T_pSTR strDest, T_pCSTR strSrc);
T_pSTR  Utl_StrCpyN(T_pSTR strDest, T_pCSTR strSour, T_S32 length);
T_pSTR  Utl_StrCat(T_pSTR strDest, T_pCSTR strSrc);
T_pSTR  Utl_StrStr(T_pCSTR strMain, T_pCSTR strSub);

/* string VS digital */
T_pSTR  Utl_Itoa(T_S32 intNum, T_pSTR strDest);
T_S32   Utl_Atoi(T_pCSTR strMain);


/* memory function */
T_pVOID Utl_MemCpy(T_pDATA strDest, T_pCDATA strSour, T_S32 count);
T_BOOL  Utl_MemSet(T_pDATA strDest, T_U8 chr, T_U32 count);
T_S8    Utl_MemCmp(T_pCDATA data1, T_pCDATA data2, T_S32 count);
T_pVOID Utl_MemMove(T_pDATA strDest, T_pDATA strSour, T_S32 count);



#endif

