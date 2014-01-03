#include "utils.h"
#include "anyka_types.h"
#include "arch_timer.h"
#include "mmu.h"
#include "hal_mmu.h"
#include "remap.h"
#include "arch_interrupt.h"


/*@brief calculate the bit number of a integer
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 *@author Justin Zhao
 *@date 2008.1.25
 *@output: T_U8 return the bit number
 *@version 1.0
*/
#pragma arm section code = "_bootcode1_"
T_U8 bitnum(T_U32 i)
{
    T_U8 ret = 0;
    for(; (T_U32)(1<<ret) < i; ret++);

    return ret;
}

void akmemset(unsigned long *s, unsigned long c, int n)
{
    int i;
    for( i = 0 ; i < n ; i ++)
    {
        s[i] = c;
    }
}

T_BOOL akmemcmp(T_U32 *p, T_U32 len)
{
    T_U32 i;
    for (i=0; i<len; i++)
    {
        if (p[i] != 0xffffffff)
            return AK_FALSE;
    }
    return AK_TRUE;
}

int strcmp(const char *s1, const char *s2)
{
    int i;

    for(i=0; s1[i] && s2[i]; i++)
    {
        if (s1[i] != s2[i])
        {
            return -1;
        }
    }

    if (s1[i] != s2[i])
        return -1;
    else
        return 0;
}


T_BOOL lock_valid_addr(T_U32 data, T_U32 nBufLen, T_BOOL bLock, T_BOOL bEverLocked)
{
    T_S32 index;
    T_U8 ret = 0;

    if (bLock)
    {
        check_stack();
        
        if (0 == data)
        {
            return AK_FALSE;
        }
        
        store_all_int();
        //aligned with 4k
        index = remap_get_vaddrindex(data & ~(VPAGE_SIZE - 1));
        if (-1 != index)
        {
            ret = remap_page_is_resv(index);
        }
        
        restore_all_int();
        if (0 == ret)
        {
            remap_lock_page(data, nBufLen, AK_TRUE);
            return AK_TRUE;
        }
    }
    else
    {
        if (bEverLocked)
        {
            remap_lock_page(data, nBufLen, AK_FALSE);
        }
    }
    return AK_FALSE;
}

#pragma arm section



