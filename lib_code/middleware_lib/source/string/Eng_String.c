#include <string.h>
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
//#include "Gbl_Global.h"
#include "eng_string_uc.h"
#include "Eng_Debug.h"

/*
	@ Created by ZMJ
*/
#define UTL_ALIGNBASE	(sizeof(T_U32))
#define UTL_ALIGNBIGK	(sizeof(T_U32)<<2)
#define UTL_ALIGNED(x)	((T_U32)(x) & (sizeof(T_U32)-1))
#define UTL_ISNULL(x)	(((x)-0x01010101) & ~(x) & 0x80808080)


//////////////////////////////////////////////////////////////////////////
T_U16 Utl_StrLen(T_pCSTR strMain)
{
	T_pCSTR ptr0 = strMain;

	AK_ASSERT_VAL(strMain, "Utl_StrLen(): strMain", 0);

	if(!UTL_ALIGNED(strMain))
	{
		T_U32	*align = (T_U32*)strMain;
		while(!UTL_ISNULL(*align))
			align ++;
		strMain = (T_pSTR)align;
	}

	while(*strMain)
		strMain ++;

	return strMain-ptr0;
}

T_S8    Utl_StrCmp(T_pCSTR str1, T_pCSTR str2)
{
	AK_ASSERT_VAL((str1 && str2), "Utl_StrCmp() : str1 or str2", 0);

	if(!UTL_ALIGNED(str1) && !UTL_ALIGNED(str2))
	{
		T_U32	*align1 = (T_U32*)str1;
		T_U32	*align2 = (T_U32*)str2;

		while(*align1 == *align2)
		{
			if(UTL_ISNULL(*align1))
				return 0;
			align1 ++;
			align2 ++;
		}

		str1 = (T_pSTR)align1;
		str2 = (T_pSTR)align2;
	}

	while(*str1 && *str1==*str2)
	{
		str1 ++;
		str2 ++;
	}

	return *(T_U8*)str1-*(T_U8*)str2;
}

/*
	@该函数准确的快速算法是 [查表法], 此处微系统不采用
	@ZMJ Hint, need 256Byte RAM
*/
T_S8    Utl_StrCmpC(T_pCSTR str1, T_pCSTR str2)
{
	AK_ASSERT_VAL((str1 && str2), "Utl_StrCmpC() : str1 or str2", 0);

	for(; ; str1++, str2++)
	{
		if(*str1 != *str2)
		{
			if(((*(T_U8*)str1) & 0xDF) != ((*(T_U8*)str2) & 0xDF))
				return (T_S8)(*(T_U8*)str1-*(T_U8*)str2);
		}

		if(!*str1)
			return 0;
	}

	return 0;
}

/*
	@该函数准确的快速算法是 [查表法], 此处微系统不采用
	@ZMJ Hint, need 256Byte RAM
*/
T_S8	Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length)
{
	AK_ASSERT_VAL((str1 && str2), "Utl_StrCmpC() : str1 or str2", 0);

	for(; length; length--, str1++, str2++)
	{
		if(*str1 != *str2)
		{
			if(((*(T_U8*)str1) & 0xDF) != ((*(T_U8*)str2) & 0xDF))
				return (T_S8)(*(T_U8*)str1-*(T_U8*)str2);
		}

		if(!*str1)
			return 0;
	}

	return 0;
}

T_pSTR  Utl_StrCpy(T_pSTR strDest, T_pCSTR strSrc)
{
	T_pSTR ptr0 = strDest;

	AK_ASSERT_VAL((strDest && strSrc), "Utl_StrCpy() : str1 or str2", 0);

	if(!UTL_ALIGNED(strDest) && !UTL_ALIGNED(strSrc))
	{
		T_U32	*align1 = (T_U32*)strDest;
		T_U32	*align2 = (T_U32*)strSrc;

		while(!UTL_ISNULL(*align2))
			*align1++ = *align2++;

		strDest = (T_pSTR)align1;
		strSrc  = (T_pSTR)align2;
	}

	while((T_BOOL)(*strDest++ = *strSrc++));

	return ptr0;
}

T_pSTR  Utl_StrCpyN(T_pSTR strDest, T_pCSTR strSrc, T_S32 length)
{
	T_pSTR ptr0 = strDest;

	AK_ASSERT_VAL((strDest && strSrc), "Utl_StrCpyN() : str1 or str2", 0);

	if((length>=UTL_ALIGNBASE) && !UTL_ALIGNED(strDest) && !UTL_ALIGNED(strSrc))
	{
		T_U32	*align1 = (T_U32*)strDest;
		T_U32	*align2 = (T_U32*)strSrc;

		while(!UTL_ISNULL(*align2) && length>=UTL_ALIGNBASE)
		{
			*align1++ = *align2++;	
			length -= UTL_ALIGNBASE;
		}

		strDest = (T_pSTR)align1;
		strSrc  = (T_pSTR)align2;		
	}

	while(length-- > 0)
	{
		if(!(*strDest++ = *strSrc++))
			break;
	}

	*strDest++ = 0;

	return ptr0;
}

T_pSTR  Utl_StrCat(T_pSTR strDest, T_pCSTR strSrc)
{
	T_pSTR ptr0 = strDest;

	AK_ASSERT_VAL((strDest && strSrc), "Utl_StrCat() : str1 or str2", 0);

	strDest += Utl_StrLen(strDest);
	Utl_StrCpy(strDest, strSrc);

	return ptr0;
}

/*
	@该函数准确的快速算法是 [Boyee-Moore法], 此处微系统不采用
	@ZMJ Hint need 256Byte RAM
*/
T_pSTR   Utl_StrStr(T_pCSTR strMain, T_pCSTR strSub)
{
	AK_ASSERT_VAL((strMain && strSub), "Utl_StrStr() : str1 or str2", 0);

	while(*strMain)
	{
		T_U32 i = 0;

		while(1)
		{
			if(!strSub[i])
				return (T_pSTR)strMain;

			if(strSub[i] != strMain[i])
				break;

			i ++;
		}

		strMain ++;
	}

	return AK_NULL;
}

//////////////////////////////////////////////////////////////////////////////////////
T_pSTR Utl_Itoa(T_S32 intNum, T_pSTR strDest)
{
	T_S8	str[16];
	T_S8	*ptr = str+15;
	AK_ASSERT_VAL(strDest, "Utl_Itoa() : strDest", 0);

	str[15] = 0;
	str[0]  = 0;
	if(intNum < 0)
	{
		str[0] = '-';
		intNum = 0-intNum;
	}

	do 
	{
		*--ptr = '0'+(intNum%10);
		intNum /= 10;
	} while(intNum);

	if(str[0] == '-')
		*--ptr = '-';

	Utl_StrCpy(strDest, ptr);
	return strDest;
}

T_S32 Utl_Atoi(T_pCSTR strMain)
{
    T_S32       sum;
    T_pCDATA    pMain = AK_NULL;
    T_BOOL      negv = AK_FALSE;

    AK_ASSERT_PTR(strMain, "Utl_Atoi()", 0);

    pMain = (T_pCDATA)strMain;
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
            break;
        pMain++;
    }

    if (negv)
        sum = 0-sum;

    return sum;
}

#if 0 //use c library instead
/////////////////////////////////////////////////////////////////////////////////////
T_pVOID Utl_MemCpy(T_pDATA strDest, T_pCDATA strSrc, T_S32 length)
{
	T_pVOID ptr0 = strDest;

	AK_ASSERT_VAL((strDest && strSrc), "Utl_MemCpy() : str1 or str2", 0);

	if((length>=UTL_ALIGNBIGK) && !UTL_ALIGNED(strDest) && !UTL_ALIGNED(strSrc))
	{
		T_U32	*align1 = (T_U32*)strDest;
		T_U32	*align2 = (T_U32*)strSrc;

		while(length >= UTL_ALIGNBIGK)
		{
			*align1++ = *align2++;	
			*align1++ = *align2++;
			*align1++ = *align2++;
			*align1++ = *align2++;
			
			length -= UTL_ALIGNBIGK;
		}

		while(length >= UTL_ALIGNBASE)
		{
			*align1++ = *align2++;
			length -= UTL_ALIGNBASE;
		}

		strDest = (T_pSTR)align1;
		strSrc  = (T_pSTR)align2;		
	}

	while(length--)
		*strDest++ = *strSrc++;

	return ptr0;
}

T_BOOL Utl_MemSet(T_pDATA strDest, T_U8 chr, T_U32 length)
{
	AK_ASSERT_VAL(strDest, "Utl_MemSet() : strdEST", 0);

	if((length>=UTL_ALIGNBIGK) && !UTL_ALIGNED(strDest))
	{
		T_U32   val = chr;
		T_U32	*align1 = (T_U32*)strDest;

		val = (val<<8) | val;
		val |= (val << 16);

		while(length >= UTL_ALIGNBIGK)
		{
			*align1++ = val;	
			*align1++ = val;
			*align1++ = val;
			*align1++ = val;
			
			length -= UTL_ALIGNBIGK;
		}

		while(length >= UTL_ALIGNBASE)
		{
			*align1++ = val;
			length -= UTL_ALIGNBASE;
		}

		strDest = (T_pSTR)align1;		
	}

	while(length--)
		*strDest++ = chr;

	return AK_TRUE;
}

T_S8 Utl_MemCmp(T_pCDATA str1, T_pCDATA str2, T_S32 length)
{
	AK_ASSERT_VAL((str1 && str2), "Utl_MemCmp() : str1 or str2", 0);

	if((length>=UTL_ALIGNBASE) && !UTL_ALIGNED(str1) && !UTL_ALIGNED(str2))
	{
		T_U32	*align1 = (T_U32*)str1;
		T_U32	*align2 = (T_U32*)str2;

		while(*align1++==*align2++ && length>=UTL_ALIGNBASE)
			length -= UTL_ALIGNBASE;

		str1 = (T_pCDATA)align1;
		str2 = (T_pCDATA)align2;
	}

	while(length--)
	{
		if(*str1 != *str2)
			return (T_S8)(*str1-*str2);

		str1 ++;
		str2 ++;
	}

	return 0;
}

/*
	@该函数后向重叠区域拷贝快速算法是 [临界双缓冲法], 此处微系统不采用
	@ZMJ Hint need 2*(str1-str2)Byte RAM
*/
T_pVOID Utl_MemMove(T_pDATA str1, T_pDATA str2, T_S32 length)
{
	T_pVOID ptr0 = str1;

	AK_ASSERT_VAL((str1 && str2), "Utl_MemCmp() : str1 or str2", 0);

	if(str2<str1 && str1<str2+length)
	{
		str1 += length;
		str2 += length;

		while(length --)
			*--str1 = *--str2;
	}
	else
	{
		return Utl_MemCpy(str1, str2, length);
	}

	return ptr0;
}
#endif // if 0
////////////////////////////////////////////////////////////////////////////////
