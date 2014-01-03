/**
 * @file Eng_String.h
 * @author Junhua Zhao
 * @date 2005-08-10
 * @brief ANYKA software
 * this header file provide hal layer string function 
 */

#ifndef __UTL_UNICODE_STRING_H
#define __UTL_UNICODE_STRING_H

#include "anyka_types.h"
//#include "Gbl_Global.h"
#include "anyka_types.h"
#include "Eng_UStrPublic.h"

//#define MAX_USTRING_LEN       1024000

/* unicode string functions*/

/**************************************************************************
* @brief Set the Unicode String buffer to the char along the wSize
* 
* @author zhao_xiaowei
* @date 2008
* @param strDes[in/out]
* @param ch[in] the  char to set
* @param wSize the buffer size
* @return T_BOOL
* @retval AK_TURE Success
* @retval AK_FALSE fail
***************************************************************************/
T_BOOL UMemSet(T_U16* const strDes,T_U16 ch, T_U16 wSize);

/**************************************************************************
* @brief from the offsetEndCnt ofsset of the end string, then to the begin
* of the string to compare ,when find just return AK_TRUE
* @author zhao_xiaowei
* @date 2008
* @param strDes[in] the string to find
* @param  strSrc the string to be found
* @param offsetEndCnt the offset from the end of the source string
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/
T_S16 UStrRevFndR(const T_U16* const strDes, const T_U16*  const strSrc, T_U16 offsetEndCnt);

/**************************************************************************
* @brief copy the right with the size of length string to the buffer
* @author zhao_xiaowei
* @date 2008
* @param strDes[out] the destinattion string
* @param strSrc[in] the soucre string to copy
* @param length[in] the length of the string from the right to copy
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/
T_BOOL UStrCpyRN(T_U16* const strDes,const T_U16* const strSrc, T_U32 length);

/**************************************************************************
* @brief get the given strart pos and the given length of the souce string
* @author zhao_xiaowei
* @date 2008
* @param subStr[out] the destination string
* @param  strSrc[in] the source string
* @param srcStart[in] the start pos of the strSrc
* @param  desLen[in] the length to copy
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/
T_BOOL UStrSub(T_U16* const subStr, const T_U16* const strSrc, T_U16 srcStart, T_U16 desLen);

/**************************************************************************
* @brief change the acsii code to unicode
* @author zhao_xiaowei
* @date 2008
* @param ucBuffer[out] the desitnation unicode string
* @param src[in] the acsii code to change
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE fail
***************************************************************************/


/**************************************************************************
* @brief  from the offset of the left of strMain ,to zhe begin of the strMain
* compare string
* @author note zhao_xiaowei
* @date 2008
* @param stMain[in] the find source
* @param strSub[in] the find destination
@ @param offset[in] the finding length from left to right
* @return T_S16
* @retval < 0 fail 
* @retval >=0 success
***************************************************************************/
T_S16   Utl_UStrRevFnd(const T_U16* strMain, const T_U16* strSub, T_S16 offset);
T_U16*  Utl_UStrCatChr(T_U16* strDest, T_U16 chr, T_S16 count);
T_S8    Utl_UStrCmpN(const T_U16* str1, const T_U16* str2, T_U16 length);
T_S8    Utl_UStrCmpC(T_U16* str1, T_U16* str2);
T_S8    Utl_UStrCmpNC(T_U16* str1, T_U16* str2, T_U16 length);

T_U16   Utl_UStrLen(const T_U16* strMain);
T_U16*  Utl_UStrCpyN(T_U16* strDest, T_U16* strSour, T_U32 length);
T_U16*  Utl_UStrCpy(T_U16* strDest, const T_U16* strSrc);
T_U16*  Utl_UStrCat(T_U16* strDest, const T_U16* strSrc);
T_U16*  Utl_UStrCatN(T_U16 * strDest, const T_U16* strSrc, T_U32 length);
T_S8    Utl_UStrCmp(T_U16* str1, T_U16* str2);
T_VOID  Printf_UC(T_pWSTR ustr);
T_U16*  Utl_UFtoa(T_FLOAT fNum, T_U16* strDest, T_U8 cntDot);
T_U16*  Utl_UItoa(T_S32 intNum, T_U16* strDest, T_U8 flag);
T_S32   Utl_UAtoi(T_U16* strMain);
T_S16   Utl_UStrRevFndChr(const T_U16* strMain, const T_U16 chr, T_S16 offset);
T_S16   Utl_UStrFnd(T_U16*  strMain, const T_U16*  strSub, T_S16 offset);
T_S16   Utl_UStrFndChr(T_U16* strMain, T_U16 chr, T_S16 offset);

/**
* @brief convert system string to unicode string
*
* @author 
* @date 
*
* @param in T_S8 *src : system string buf
* @param out T_U16 *ucBuf : output unicode string buf
*
* @return T_S32
* @retval convert len
*/
T_S32   Utl_StrMbcs2Ucs(const T_S8 *src, T_U16 *ucBuf);

/**
* @brief convert unicode string to system string
*
* @author 
* @date 2013-07-03
*
* @param in T_U16 *ucSrc : unicode string buf
* @param out T_S8 *destBuf : output system string buf
*
* @return T_S32
* @retval convert len
*/
T_S32	Utl_StrUcs2Mbcs(const T_U16 *ucSrc, T_S8 *destBuf);

T_U16 Utl_CaclSolidas(const T_U16* str);
T_VOID Utl_USplitFilePath(T_pCWSTR file_path, T_pWSTR path, T_pWSTR name);


#endif // __UTL_UNICODE_STRING_H

