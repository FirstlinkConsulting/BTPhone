/**
 * @file
 * @brief ANYKA software
 * mathematic library, include double operator
 *
 * @author ZouMai
 * @date    2002-11-02
 * @author 
 */

//#include "Gbl_Global.h"
#include "Eng_Math.h"
#include "Fwl_Timer.h"
#include "Eng_Debug.h"
#define  SEED_COUNT   30
//#if (USE_COLOR_LCD)

#ifdef OS_WIN32
    #include "stdlib.h"
#else
extern T_U32 fake(T_U32 *seed, T_U32 range);
#endif

T_U32 rand_seed=0;

extern T_U32 GetPubTimerCnt(T_VOID);
static T_BOOL s_isFirst= AK_TRUE;
T_VOID Fwl_RandSeed(T_VOID)
{
#ifdef OS_ANYKA
    rand_seed = Fwl_GetTickCountUs();
#else
    rand_seed= 0;
#endif
}

/**
 create a random number 
 author: wjw
 date: 2003.11.19
 **/
T_U32 Fwl_GetRand(T_U32 maxVal)
{
    T_U32 rand_value= 0;
     
    if (maxVal == 0)
        return 0;
#ifdef OS_ANYKA
    if(s_isFirst)
    {
        Fwl_RandSeed();
        s_isFirst= AK_FALSE;
    }
   rand_value = fake(&rand_seed, maxVal);
    
    return rand_value;
#endif
#ifdef OS_WIN32
    return (rand()%maxVal);
#endif
}

/**
    * @BRIEF    A 64bit unsigned integer to compare with a 32 bit unsigned integer
    * @AUTHOR   Liu weijun
    * @DATE     2008-01-23
    * @PARAM    size64   - a pointer to a 64 bit unsigned integer. 
    *           size32   - a 32 bit unsigned integer
    * @RETURN
    * @RETVAL   -1  size64 is less than size32
    * @RETVAL   0   size64 is equal to size32
    * @RETVAL   1   size64 is larger than size32
*/
#pragma arm section code = "_frequentcode_"
T_S8 U64cmpU32(T_U64_INT *size64, T_U32 size32)
{
    //AK_ASSERT_PTR(size64, "U64cmpU32: error! the ptr of T_U64_INT is null!", -127);
	if (AK_NULL == size64)
		return -1;
	
    if (size64->high > 0)
        return 1;

    if (size64->low > size32)
        return 1;

    if (size64->low == size32)
        return 0;

    return -1;
}
#pragma arm section code
//#endif
/* end of files */
