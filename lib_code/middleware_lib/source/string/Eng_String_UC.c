#include <string.h>
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
//#include "Gbl_Global.h"
#include "Eng_String_UC.h"
#include "Fwl_osFS.h"
#include "Eng_Debug.h"

T_S16 Utl_UStrFndL(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 length)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndL(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFndL(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);      /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);      /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = Utl_UStrLen(strSub);
    if (offset + subStrLen > length)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);        /* length can't exceed MAX_USTRING_LEN */

        for (i = 0; i < subStrLen; i++)
        {
            if (*(pMain + i) != *(strSub + i))
                break;
        }
        if (i == subStrLen)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
    }

    return -1;
}

T_U16*  Utl_UStrInit(T_U16* strMain,const T_pCSTR pStr)
{
    int i;
    for(i=0;pStr[i]!='\0';i++)
    {
        strMain[i] = pStr[i];
        strMain[i] &= 0xFF;
    }
    return strMain;
}


T_U16 * Utl_UStrIns(T_U16 * strDest, const T_U16* strSub, T_S16 offset)
{
    T_U16   lenDest, lenSub;
    T_S16   i;
    T_S16   curOffset = offset;

    AK_ASSERT_PTR(strDest, "Utl_UStrIns()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_UStrIns()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrIns()", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = Utl_UStrLen(strDest);
    lenSub  = Utl_UStrLen(strSub);

    if (lenSub == 0)
        return strDest;
    if ((T_U16)curOffset > lenDest)
        curOffset = lenDest;
    strDest[lenDest + lenSub] = 0;
    for (i = lenDest + lenSub - 1; i >= (T_S16)(curOffset + lenSub); i--)
        strDest[i] = strDest[i-lenSub];
    for (i = curOffset + lenSub - 1; i >= curOffset; i--)
        strDest[i] = strSub[i - curOffset];
  
    return strDest;
}// end Utl_UStrIns(T_U16 * strDest, T_U16 * strSub, T_S16 offset)

#pragma arm section code = "_commoncode_"

T_U16 Utl_UStrLen(const T_U16* strMain)
{
    T_U16 len = 0;

    if (strMain == 0)
        return 0;

    while(*(strMain+len) != 0x00)
        len++;
    return len;
}

T_S8 Utl_UStrCmpN(const T_U16* str1, const T_U16* str2, T_U16 length)
{
    T_U16   i;
    const T_U16*  pStr1 = str1;
    const T_U16*  pStr2 = str2;

    AK_ASSERT_PTR(str1, "Utl_UStrCmpN()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmpN()", 0);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrCmpN()", 0);       /* length can't exceed MAX_USTRING_LEN */
#endif

    for( i=0; i<length; i++ )
    {
        if( pStr1[i] > pStr2[i] )
        {
            return 1;
        }
        else if( pStr1[i] < pStr2[i] )
        {
            return -1;
        }
        if( pStr1[i] == 0 )
        {
            return 0;
        }
    }
    return 0;
}

// ***____ ofsetEnd the first _ to *, 
T_S16 UStrRevFndR(const T_U16* const strFndDes, const T_U16* const strFndSrc, T_U16 offsetEnd)
{
	T_U16 desLen, srcLen;
	T_U16 startPos;
	T_S16 i;

    AK_ASSERT_PTR(strFndDes, "Utl_UStrCmpN()", -1);
    AK_ASSERT_PTR(strFndSrc, "Utl_UStrCmpN()", -1);

	desLen= Utl_UStrLen(strFndDes);
	srcLen= Utl_UStrLen(strFndSrc);

	if(desLen == 0 || srcLen == 0)
	{
		return -1;
	}

	if(desLen+offsetEnd > srcLen)
	{
		return -1; //not found
	}
	
	if((offsetEnd+1) < desLen)
	{
		startPos= srcLen- offsetEnd- 1;
	}
	else
	{
		startPos= srcLen- desLen; 
	}
	for(i= (srcLen- offsetEnd-1); i>= 0; i--)
	{
		if(Utl_UStrCmpN((T_U16* )strFndDes, (T_U16* )(strFndSrc+ i), desLen) == 0)
		{
			return i; //if find,then it will return from this
		}
	}
	return -1;
}

T_BOOL UStrSub(T_U16* const subStr, const T_U16* const strSrc, T_U16 srcStart, T_U16 desLen)
{
	T_U16 srcLen;

	//ASSERT_PTR_FLAG(subStr)
	//ASSERT_PTR_FLAG(strSrc)

	srcLen= Utl_UStrLen(strSrc);
	if(srcStart+ desLen > srcLen)
	{
		return AK_FALSE;
	}
	Utl_UStrCpyN((T_U16 *)subStr, (T_U16* )(strSrc+ srcStart), desLen);
	return AK_TRUE;
}

T_BOOL UStrCpyRN(T_U16* const strDes,const T_U16* const strSrc, T_U32 length)
{
	T_U32 srcLen= 0;
	const T_U16* tmpSrc= AK_NULL;
	AK_ASSERT_PTR(strDes, AK_NULL, AK_FALSE);
	AK_ASSERT_PTR(strSrc, AK_NULL, AK_FALSE);

	srcLen= UStrLen(strSrc);
	if(srcLen< length)//if less then strSrc all
	{
		UStrCpy(strDes, strSrc);
		return AK_FALSE;
	}
	else
	{ 
		tmpSrc= strSrc+ srcLen- length+ 1;
		
	     UStrCpy(strDes, tmpSrc);
		 return AK_TRUE;
	}
}

T_BOOL UMemSet(T_U16* const strDes,T_U16 ch, T_U16 wSize)
{
	T_U16* tmpDes;

	AK_ASSERT_PTR(strDes, AK_NULL, AK_FALSE);

	tmpDes= strDes;
	while(tmpDes != (strDes+ wSize))
	{
		*tmpDes++ = ch;
	}

	return AK_TRUE;
}

T_U16 * Utl_UStrCpyN(T_U16 * strDest, T_U16* strSour, T_U32 length)
{
    T_U32   i = 0;
    T_U16 *     d;
    T_U16*  s;

    d = strDest;
    s = strSour;
    
    while ((*s) && (i < length))
    {
        *d++ = *s++;
        i++;
    }
    *d = 0;

    return strDest;
}

T_U16 * Utl_UStrCpy(T_U16 * strDest, const T_U16* strSour)
{
    T_S16   i = 0;
    T_U16   *d;
    T_U16   *s;

    AK_ASSERT_PTR(strDest, "Utl_UStrCpy(): strDest", AK_NULL);
    AK_ASSERT_PTR(strSour, "Utl_UStrCpy(): strSour", AK_NULL);
    
    d = strDest;
    s = (T_U16 *)strSour;

    while (*s)
    {
        *d++ = *s++;
        i++;
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCpy(): i", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
    }
    *d = 0;
    
    return strDest;
}

T_U16 * Utl_UStrCat(T_U16 * strDest, const T_U16* strSub)
{
    T_S16   i = 0;
    T_S16   len;

    AK_ASSERT_PTR(strDest, "Utl_UStrCat()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_UStrCat()", AK_NULL);

    len = Utl_UStrLen(strDest);

    while (*(strSub + i++) != 0)
    {
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCat()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */

        strDest[len + i - 1] = strSub[i - 1];
    }
    strDest[len + i - 1] = 0;
  
    return strDest;
}

T_U16 * Utl_UStrCatN(T_U16 * strDest, const T_U16* strSrc, T_U32 length)
{
    T_U32  i = 0;
    T_S16   len;

    AK_ASSERT_PTR(strDest, "Utl_UStrCat()", AK_NULL);
    AK_ASSERT_PTR(strSrc, "Utl_UStrCat()", AK_NULL);

    len = Utl_UStrLen(strDest);

    while ((i < length) && *(strSrc + i++) != 0)
    {
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCat()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */

        strDest[len + i - 1] = strSrc[i - 1];
    }
    //strDest[len + i ] = 0;
    strDest[len + i - 1] = 0;
  
    return strDest;
}



T_S8 Utl_UStrCmp(T_U16* str1, T_U16* str2)
{
    T_S16   len = 0;
    T_U16*  pStr1 = str1;
    T_U16*  pStr2 = str2;

    AK_ASSERT_PTR(str1, "Utl_UStrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmp()", 0);

    while (((*(pStr1)) != 0) || ((*(pStr2)) != 0))
    {
        if ((*(pStr1)) > (*(pStr2)))
            return 1;

        if ((*(pStr1)) < (*(pStr2)))
            return -1;

        pStr1++;
        pStr2++;
        len++;
        //AK_ASSERT_VAL(len < MAX_USTRING_LEN, "Utl_UStrCmp()", -1);        /* length can't exceed MAX_USTRING_LEN */
    }

    return 0;
}
#pragma arm section code 


T_VOID  Printf_UC(T_pWSTR ustr)
{
#ifdef DEBUG
    T_U8 ch = ' '; //space
    T_S8 strtmp[(MAX_FILE_LEN+40)*2];

    Eng_WideCharToMultiByte(ustr, Utl_UStrLen(ustr),strtmp, (MAX_FILE_LEN+40)*2, &ch);
    AK_DEBUG_OUTPUT("$$$$Print_UC:  %s\n", strtmp);
#endif
}

T_U16*  Utl_UFtoa(T_FLOAT fNum, T_U16* strDest, T_U8 cntDot)
{
	T_U8  i;
	float tmp1 = 1.0;
	T_S32 tmp = 1;
	
	for(i=0; i<cntDot; i++)
		tmp1 *= 10;
	tmp = (int)(fNum*tmp1);

	Utl_UItoa(tmp, strDest, 10);
	tmp = Utl_UStrLen(strDest);

	if(0 == Utl_UStrCmp(strDest, (T_U16*)"0"))
	{
		for(i=0; i<cntDot; i++)
			strDest[tmp+i] = '0';
		strDest[tmp+i] = 0;
	}
	tmp = Utl_UStrLen(strDest);

	for(i=tmp-1; i>=tmp-cntDot; i--)
		strDest[i+1] = strDest[i];
	
	strDest[tmp-cntDot] = '.';
	strDest[tmp+cntDot] = 0;

	return strDest;
}

#pragma arm section code = "_audioplayer_para_"
T_U16 * Utl_UItoa(T_S32 intNum, T_U16 * strDest, T_U8 flag)
{
    T_S16       i = 0;
    T_S32       datanew;
    T_S16       index;
    T_U16       strTemp[100];
    T_BOOL      negv = AK_FALSE;

    AK_ASSERT_PTR(strDest, "Utl_UItoa()", AK_NULL);

    strDest[0] = '\0';
    if (intNum < 0)
    {
        intNum *= (-1);
        negv = AK_TRUE;
    }

    if (flag == 16)
    {
        if (intNum < 16)
        {
            if(intNum >= 10)
                strDest[0] = intNum + 55;
            else 
                strDest[0] = intNum + 48;
            strDest[1] = '\0';
        }
        else
        {
            while (intNum >= 16)
            {
                datanew = intNum;
                intNum = intNum/16;
                if((datanew - intNum*16) >= 10)
                    strTemp[i] = datanew - intNum * 16 + 55;
                else 
                    strTemp[i] = datanew - intNum * 16 + 48;
                i ++ ;
                if (intNum < 16)
                {
                    if(intNum >= 10)
                        strTemp[i] = intNum + 55;
                    else if(intNum != 0)
                        strTemp[i] = intNum + 48;
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }
    else
    {
        if (intNum < 10)
        {
            strDest[0] = intNum + 48;
            strDest[1] = '\0';
        }
        else
        {
            while(intNum >= 10)
            {
                datanew = intNum;
                intNum = intNum/10;
                strTemp[i] = datanew - intNum * 10 + 48;
                i ++ ;
                if(intNum < 10)
                {   
                    strTemp[i] = intNum + 48;
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }

    if (negv)
        //Utl_UStrIns(strDest, Utl_UStrInit(strTemp,"-"), 0);
	{
		index = Utl_UStrLen(strDest);
		for(i=index-1; i>=0; i--)
			strDest[i+1] = strDest[i];

		strDest[0] = '-';
		strDest[index+1] = 0;
	}

    return strDest;
}
#pragma arm section code

#pragma arm section code = "_commoncode_"

T_S32 Utl_UAtoi(T_U16* strMain)
{
    T_U16*  pMain = AK_NULL;
    T_S32       sum;
    T_BOOL      negv = AK_FALSE;
    T_S16       i = 0;

    AK_ASSERT_PTR(strMain, "Utl_UAtoi()", 0);

    pMain = strMain;
    sum = 0;
    if ((*pMain) == '-')
    {
        negv = AK_TRUE;
        pMain++;
    }

    while (*pMain)
    {
        if ('0' <= (*pMain) && (*pMain) <= '9')
            sum = sum * 10 + (*pMain - '0');
        else
            break;;
        pMain++;
        i++;
        #if MAX_USTRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCarve()", 0);       /* length can't exceed MAX_USTRING_LEN */
        #endif
    }

    if (negv)
        sum *= (-1);

    return sum;
}
#pragma arm section code

T_S16 Utl_UStrRevFndChr(const T_U16* strMain, const T_U16 chr, T_S16 offset)
{
    T_S16 i;

    AK_ASSERT_PTR(strMain, "Utl_UStrRevFndChr(): strMain", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrRevFndChr()", -1);     /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if(offset >= Utl_UStrLen(strMain)) 
        offset = Utl_UStrLen(strMain) -1;
	else
		offset = Utl_UStrLen(strMain)-1-offset;

    for (i = offset; i >= 0; i--)
    {
        if (strMain[i] == chr) 
            return i;
    }
    
    return -1;
}

T_S16 Utl_UStrFnd(T_U16* strMain, const T_U16* strSub, T_S16 offset)
{
    AK_ASSERT_PTR(strMain, "Utl_UStrFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFnd(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFnd()", -1);       /* length can't exceed MAX_USTRING_LEN */
#endif

    return Utl_UStrFndL(strMain, strSub, offset, Utl_UStrLen(strMain));
}

T_S16 Utl_UStrFndChr(T_U16* strMain, T_U16 chr, T_S16 offset)
{
    T_U16   curLoc = offset;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndChr(): strMain", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndChr()", -1);        /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if (offset >= Utl_UStrLen(strMain))
        return -1;

    pMain = strMain + offset;

    while (*pMain != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndChr(): curLoc", -1);      /* length can't exceed MAX_USTRING_LEN */

        if (*pMain == chr)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
    }

    return -1;
}

T_S16   Utl_UStrRevFnd(const T_U16* strMain, const T_U16* strSub, T_S16 offset)
{
    T_U16 i, j, len1, len2, flag;

    AK_ASSERT_PTR(strMain, "Utl_UStrRevFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrRevFnd(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrRevFnd()", -1);        /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    len1 = Utl_UStrLen(strMain);
    len2 = Utl_UStrLen(strSub);
    
    offset = ( offset > len1 - 1 ) ? ( len1 - 1 ) : offset;
    if( offset + 1 < len2 )
        return -1;

    for( i = offset; i >= len2 - 1; i--)
    {
        flag = 1;
        for( j = 0; j < len2; j++)
        {
            if( strMain[i - j] != strSub[len2 - 1 - j] )
            {
                flag = 0;
                break;
            }
        }

        if( flag == 1 )
            return i - len2 + 1;
    }

    return -1;
}

T_U16 * Utl_UStrCatChr(T_U16 * strDest, T_U16 chr, T_S16 count)
{
    T_U16   i;
    T_U16   len;
   
    AK_ASSERT_PTR(strDest, "Utl_UStrCatChr()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrCatChr()", AK_NULL);        /* length can't exceed MAX_USTRING_LEN */
#endif

    len = Utl_UStrLen(strDest);
    if (count <= 0)
        return strDest;

    for (i = 0; i < count; i++)
        strDest[len + i] = chr;
    strDest[len + i] = 0;
  
    return strDest;
}

T_S8 Utl_UStrCmpC(T_U16* str1, T_U16* str2)
{
    T_U16   c1, c2;
    T_S16   i=0;
    T_U16   *pStr1 = str1;
    T_U16   *pStr2 = str2;

    for( i=0; ; i++ )
    {
        c1 = pStr1[i];
        if( c1 >= 'A' && c1 <= 'Z' )
        {
            c1 += 0x20;
        }

        c2 = pStr2[i];
        if( c2 >= 'A' && c2 <= 'Z' )
        {
            c2 += 0x20;
        }

        if( c1 > c2 )
        {
            return 1;
        }
        else if( c1 < c2 )
        {
            return -1;
        }
        else
        {
            if( c1 == 0 )
            {
                return 0;
            }
        }
    }
}

T_S8 Utl_UStrCmpNC(T_U16* str1, T_U16* str2, T_U16 length)
{
    T_U16   c1,c2;
    T_U16  i;
    T_U16*  pStr1 = str1;
    T_U16*  pStr2 = str2;
    const T_U16 char_A = 'A';
    const T_U16 char_Z = 'Z';

    AK_ASSERT_PTR(str1, "Utl_UStrCmpNC()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmpNC()", 0);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrCmpNC()", 0);      /* length can't exceed MAX_USTRING_LEN */
#endif

    for( i=0; i<length; i++, pStr1++, pStr2++ )
    {
        c1 = *pStr1;
        if( c1 >= char_A && c1 <= char_Z )
            c1 += 0x20;

        c2 = *pStr2;
        if( c2 >= char_A && c2 <= char_Z )
            c2 += 0x20;

        if(c1 > c2) 
        {
            return 1;
        }
        if(c1 < c2)
        {
            return -1;
        }
    }
    return 0;
}


#pragma arm section code = "_commoncode_"


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
T_S32 Utl_StrMbcs2Ucs(const T_S8 *src, T_U16 *ucBuf)
{
	T_S32 len= 0;
	T_U16 ch = UNICODE_SPACE; //space

    if (AK_NULL == src || AK_NULL == ucBuf)
    {
        return _FOPEN_FAIL;  
    }

   len = Eng_MultiByteToWideChar(src, strlen(src), ucBuf, MAX_FILE_LEN, &ch);
   ucBuf[len] = 0;
   return len;
}
#pragma arm section code


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
T_S32 Utl_StrUcs2Mbcs(const T_U16 *ucSrc, T_S8 *destBuf)
{
	T_S32 len= 0;
	T_U8 ch = ' '; //space

    if (AK_NULL == ucSrc || AK_NULL == destBuf)
    {
        return _FOPEN_FAIL;
    }

   len = Eng_WideCharToMultiByte(ucSrc, Utl_UStrLen(ucSrc), destBuf, MAX_FILE_LEN, &ch);
   destBuf[len] = 0;
   return len;
}


T_U16 Utl_CaclSolidas(const T_U16* str)
{
	T_U16 cnt = 0;
	T_U32 len = Utl_UStrLen(str);	
	
	while (len)
	{
		if (UNICODE_SOLIDUS == str[len--])
			cnt++;
	}
	
	return cnt;
}

#pragma arm section code = "_commoncode_"

T_VOID Utl_USplitFilePath(T_pCWSTR file_path, T_pWSTR path, T_pWSTR name)
{
    T_S32 i, len;

    if ((file_path == AK_NULL) || ((T_U16)Utl_UStrLen(file_path) == 0))
    {
        if (path != AK_NULL)
        {
            path[0] = 0;
        }
        if (name != AK_NULL)
        {
            name[0] = 0;
        }
        return;
    }

    len = (T_U16)Utl_UStrLen(file_path);
    for (i=len-1; i>=0; i--)
    {
        if ((file_path[i] == UNICODE_SOLIDUS) || (file_path[i] == UNICODE_RES_SOLIDUS))
            break;
    }

    if (path != AK_NULL)
    {
        Utl_UStrCpyN(path, (T_pWSTR)file_path, i+1);
        path[i+1] = 0;
    }
    if (name != AK_NULL)
    {
        Utl_UStrCpyN(name, &file_path[i+1], (T_U32)(MAX_FILE_LEN - 1));
        name[len-(i+1)] = 0;
    }
}
#pragma arm section code

