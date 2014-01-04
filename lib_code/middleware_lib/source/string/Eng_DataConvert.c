/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºEng_dataconvert.c
 * Function£ºThe file define some function of processing common data convert
 
 *
 * Author£ºxuping
 * Date£º2008-04-18
 * Version£º1.0		  
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Eng_DataConvert.h"
#include "Gbl_Global.h"
#include "Eng_String.h"
#include "string.h"
#include "eng_debug.h"

#include "unicode_api.h"

/*********************************************
* @brief convert hex string to integer
*
* @author @b Miaobaoli
*
* @author
* @date 2001-06-25
* @param T_pSTR str_Hex:   source hex string pointer.
* @return T_U16: the integer value.
* @retval
*******************************************************/
T_U16 ConverHexToi(T_pCSTR str_Hex)
{
    T_U16 i,c;
    T_S16            length,j;

    AK_FUNCTION_ENTER("ConverHexToi");
    AK_ASSERT_PTR(str_Hex, "ConverHexToi(): str_Hex", 0);

    length = Utl_StrLen((T_pSTR)str_Hex);
    i = 0;

    for( j=length-1; j>=0; j-- )
    {
        c = str_Hex[j];
        if('a' <= c && c <= 'f')
        {
            c -= 'a';
            c += 10;
        }
        else if('A' <= c && c <= 'F')
        {
            c -= 'A';
            c += 10;
        }
        else
            c -= '0';

        c <<= ((length - 1 - j) * 4);
        i |= c;
    }
    AK_FUNCTION_RET_INT("ConverHexToi",i);
    return i;
}

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
T_BOOL Eng_FirstIsGBKChn(const T_S8 *gbk, T_U32 gbkLen)
{
    T_U8 c1, c2;

    if(gbkLen >= 2)
    {
        c1 = gbk[0];
        c2 = gbk[1];
        if(c2>=0xa1 && c2<=0xfe)
        {
            if(c1>=0xa1 && c1<=0xa9)
            {
                return AK_TRUE;
            }
            else if(c1>=0xb0 && c1<=0xf7)
            {
                return AK_TRUE;
            }
        }
        if(c2>=0x40)
        {
            if(c2<=0xfe)
            {
                if(c1>=0x81 && c1<=0xa0)
                {
                    return AK_TRUE;
                }
            }
            if(c2<=0xa0)
            {
                if(c1>=0xa8 && c1<=0xfe)
                {
                    return AK_TRUE;
                }
            }
        }
    }
    return AK_FALSE;
}


static T_S32 Eng_Unic2Eng(const T_U16 *unicode, T_U32 ucLen, T_S8 *pDestBuf, T_U32 destLen)
{
    T_U32 i;

    if (AK_NULL != pDestBuf && destLen > 0)
    {
        for(i=0; (i<ucLen && i<(destLen-1) && 0 != unicode[0]); i++)
            pDestBuf[i] = (T_U8)(unicode[i]);


        pDestBuf[i] = 0;
        if (i == destLen-1 && i < ucLen)
        i = 0xffffffff;
    }
    else
    {
        i = ucLen << 1;
    }

    return (T_S32)i;
}
static T_S32 Eng_Eng2Unic(const T_S8 *src, T_U32 srcLen, T_U16 *ucBuf, T_U32 ucBufLen)
{
    T_U32 i;

    if (AK_NULL != ucBuf && ucBufLen > 0)
    {
        for(i=0; (i<srcLen && i<(ucBufLen-1) && 0 != src[i]); i++)
            ucBuf[i] = ((T_U16)(src[i])) & 0xff;

        ucBuf[i] = 0;

        if (i == ucBufLen-1 && i != srcLen)
            i = 0xffffffff;
    }
    else
    {
        i = srcLen;
    }

    return (T_S32)i;
}

#pragma arm section code = "_dataconvert_"

T_S32 Eng_WideCharToMultiByte_Ex(T_CODE_PAGE code_page, T_U32 dwFlags, const T_U16 *unicode, T_U32 ucLen, T_CHR *pDestBuf, T_U32 destLen, const T_CHR *defaultChr)
{

#ifndef SUPPORT_UNIC

    if (code_page >= CODE_PAGE_NUM)
    {
        return 0;
    }
    return Eng_Unic2Eng(unicode, ucLen, pDestBuf, destLen);

#else
    T_S32 ret = -1;
    int UsedDefCh = 0;
    int *pUsedDefCh = AK_NULL;
    
    if (CP_UTF8_CP == code_page)
    {
        ret = utf8_wcstombs( unicode, ucLen, pDestBuf, destLen - 1 );
    }
    else
    {  
        if (code_page >= CODE_PAGE_NUM)
        {
            return 0;
        }
        
        if (!CodePage_GetInitFlag())
        {
            return Eng_Unic2Eng(unicode, ucLen, pDestBuf, destLen);
        }
        pUsedDefCh = (defaultChr) ? (&UsedDefCh) : AK_NULL;
		if (destLen==0)
			return -1;
		
	    ret = cp_wcstombs(CodePage_GetCharSize(), 0,
	                          unicode, ucLen,
	                          pDestBuf, destLen - 1, defaultChr, pUsedDefCh );
    }
    
    if (destLen && pDestBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)destLen)
                pDestBuf[ret] = 0;
			else
			{
				//Should not come here
				AK_DEBUG_OUTPUT ("MultiByte Buffer Length is not enough!");
//				pDestBuf[destLen - 1] = 0;
			}
        }
        else
        {
            pDestBuf[0] = 0;
        }
    }

    return ret;
#endif
}




T_S32 Eng_MultiByteToWideChar_Ex(T_CODE_PAGE code_page, T_U32 *ConvertedNum, const T_CHR *src, T_U32 srcLen, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr)
{
   
#ifndef SUPPORT_UNIC

    T_U32 Len = 0;
    // before load convert table to ram, just convert english to unicode
    Len = Eng_Eng2Unic(src, srcLen, ucBuf, ucBufLen);
    if (AK_NULL != ConvertedNum)
    {
        *ConvertedNum = Len << 1;
    }
    return Len;

#else
    T_S32 ret;

    if (NULL != ConvertedNum)
    {
       *ConvertedNum  = 0;
    }

    if (CP_UTF8_CP == code_page)
    {
        ret = utf8_mbstowcs( ConvertedNum, src, srcLen, ucBuf, ucBufLen - 1);
    }
    else
    {
        if (code_page >= CODE_PAGE_NUM)
        {
            return 0;
        }
        
        if (!CodePage_GetInitFlag())
        {
            T_U32 Len = 0;
            // before load convert table to ram, just convert english to unicode
            Len = Eng_Eng2Unic(src, srcLen, ucBuf, ucBufLen);
            if (AK_NULL != ConvertedNum)
            {
                *ConvertedNum = Len << 1;
            }
            return Len;
        }

		if (ucBufLen==0)
			return -1;
        ret = cp_mbstowcs( CodePage_GetCharSize(), 0,
                          src, srcLen,
                          ucBuf, ucBufLen-1, defaultUChr ,ConvertedNum);
    }

    if (ucBufLen && ucBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)ucBufLen)
                ucBuf[ret] = 0;
			else
			{
				//Should not come here
				AK_DEBUG_OUTPUT ("WideChar Buffer Length is not enough!");
			}
        }
        else if (AK_NULL == ConvertedNum)//try to convert some ch ,Don't retyrn empty.
        {
            ucBuf[0] = 0;
        }
    }
    
    return ret;
#endif
}


/************************************************************************************************************************************
* @brief Convert UNICODE string to system string                                                                                    *
* @param[in]	unicode		source UNICODE string                                                                                   *    
* @param[in]	ucLen		the length of source string, in UNICODE char unit                                                       *   
* this parameter used to output converted UNICODE chars;this parameter can be NULL                                                  *
* @param[out]	pDestBuf		the output system string buffer                                                                     *   
* @param[in]	destLen	indicate the output system string buffer size, in bytes                                                     *
* @param[in]	defaultChar	Pointer to the characters used if a UNICODE character cannot be represented in the GBK code page,       *
* If this parameter is NULL, UNICODE character would be assigned to GBK character directly                                          *
* @return T_S32                                                                                                                     *    
* @retval if gbkBufLen is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string*
* @retval if gbkBufLen is not zero, the return value is the number of bytes written to the buffer pointed to by gbkBuf              *
*************************************************************************************************************************************/
T_S32 Eng_WideCharToMultiByte(const T_U16 *unicode, T_U32 ucLen,T_CHR *pDestBuf, T_U32 destLen, const T_CHR *defaultChr)
{
    T_CODE_PAGE code_page;
    
#if(NO_DISPLAY == 0)
    code_page = GetCurLangCodePage();
#else
    code_page = CP_936;
#endif

    return Eng_WideCharToMultiByte_Ex(code_page, 0, unicode, ucLen, pDestBuf, destLen, defaultChr);
}

/**
 * @brief Convert system string to UNICODE string
 * @param[in] src		source system string
 * @param[in] srcLen	the length of source string, in bytes
 * this parameter used to output converted GBK bytes;this parameter can be NULL
 * @param[out] ucBuf	the output UNICODE string buffer
 * @param[in] ucBufLen	indicate the output UNICODE string buffer size, in UNICODE char
 * @param[in] defaultUChr	Pointer to the UNICODE character(one char) used if a GBK character is a invalid char in the GBK code page, 
 *							If this parameter is NULL, a system default value is use
 * @return T_S32
 * @retval if ucBufLen is zero, the return value is the required size, in UNICODE char, for a buffer that can receive the translated string
 * @retval if ucBufLen is not zero, the return value is the number of UNICODE chars written to the buffer pointed to by ucBuf
 */
T_S32 Eng_MultiByteToWideChar(const T_CHR *src, T_U32 srcLen,T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr)
{
    T_CODE_PAGE code_page;

#if(NO_DISPLAY == 0)
    code_page = GetCurLangCodePage();
#else
    code_page = CP_936;
#endif

    return Eng_MultiByteToWideChar_Ex(code_page, AK_NULL, src, srcLen, ucBuf, ucBufLen, defaultUChr);
}
#pragma arm section code



