/**
 * @file Eng_DataConvert.h
 * @brief This header file is for data convert function prototype
 * 
 */

#ifndef __ENG_DATA_CONVERT_H__
#define __ENG_DATA_CONVERT_H__


#include "Anyka_types.h"
#include "code_page.h"
#include "Gbl_Resource.h"
#include "Eng_String.h"
//#define T_RES_LANGUAGE T_U32


/**************************************
* @brief convert hex string to integer
*
* @author @b Miaobaoli
*
* @author
* @date 2001-06-25
* @param T_pSTR str_Hex:   source hex string pointer.
* @return T_U16: the integer value.
* @retval
******************************************/
T_U16   ConverHexToi(T_pCSTR str_Hex);

/*********************************************************
* @brief Judge first char is a chinese char in GBK buffer
*
* @author @b LiaoJianhua
*
* @date 2005-07-26
* @param gbk:[in] source GBK string
* @param gbkLen:[in] the length of source string, in bytes
* @return
* @retval AK_TRUE, first char is a chinese char
* @retval AK_FALSE, first char is not a chinese char
* @note
************************************************************/
T_BOOL Eng_FirstIsGBKChn(const T_S8 *gbk, T_U32 gbkLen);

/*******************************************************
* @brief get codepage from current language            *
*                                                      * 
* @author @b xuping                                    * 
*                                                      * 
* @author                                              * 
* @date 2008-04-19                                     *     
* @param lang :language                                * 
* @return codepage                                     * 
* @retval                                              * 
*******************************************************/
T_CODE_PAGE GetLangCodePage(T_RES_LANGUAGE lang );

/************************************************************************************************************************************
* @brief Convert UNICODE string to system string                                                                                    *
* @param[in]    unicode     source UNICODE string                                                                                   *    
* @param[in]    ucLen       the length of source string, in UNICODE char unit                                                       *   
* this parameter used to output converted UNICODE chars;this parameter can be NULL                                                  *
* @param[out]   pDestBuf        the output system string buffer                                                                     *   
* @param[in]    destLen indicate the output system string buffer size, in bytes                                                     *
* @param[in]    defaultChar Pointer to the characters used if a UNICODE character cannot be represented in the GBK code page,       *
* If this parameter is NULL, UNICODE character would be assigned to GBK character directly                                          *
* @return T_S32                                                                                                                     *    
* @retval if gbkBufLen is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string*
* @retval if gbkBufLen is not zero, the return value is the number of bytes written to the buffer pointed to by gbkBuf              *
*************************************************************************************************************************************/
T_S32 Eng_WideCharToMultiByte(const T_U16 *unicode, T_U32 ucLen,T_CHR *pDestBuf, T_U32 destLen, const T_CHR *defaultChr);

T_S32 Eng_WideCharToMultiByte_Ex(T_CODE_PAGE code_page, T_U32 dwFlags, const T_U16 *unicode, T_U32 ucLen, T_CHR *pDestBuf, T_U32 destLen, const T_CHR *defaultChr);

/**
 * @brief Convert system string to UNICODE string
 * @param[in] src       source system string
 * @param[in] srcLen    the length of source string, in bytes
 * this parameter used to output converted GBK bytes;this parameter can be NULL
 * @param[out] ucBuf    the output UNICODE string buffer
 * @param[in] ucBufLen  indicate the output UNICODE string buffer size, in UNICODE char
 * @param[in] defaultUChr   Pointer to the UNICODE character(one char) used if a GBK character is a invalid char in the GBK code page, 
 *                          If this parameter is NULL, a system default value is use
 * @return T_S32
 * @retval if ucBufLen is zero, the return value is the required size, in UNICODE char, for a buffer that can receive the translated string
 * @retval if ucBufLen is not zero, the return value is the number of UNICODE chars written to the buffer pointed to by ucBuf
 */
T_S32 Eng_MultiByteToWideChar(const T_CHR *src, T_U32 srcLen,T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);

T_S32 Eng_MultiByteToWideChar_Ex(T_CODE_PAGE code_page, T_U32 *ConvertedNum, const T_CHR *src, T_U32 srcLen, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);

#define ToWideChar(ucBuffer,src) Eng_MultiByteToWideChar(src, Utl_StrLen(src), ucBuffer, Utl_StrLen(src)+1, AK_NULL)


#endif
