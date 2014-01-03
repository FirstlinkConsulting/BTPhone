/************************************************************************
 * @file    Fwl_Malloc.c
 * @brief   this header file is API for Memory Library
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * author   
 * date     2013-03-20
 * version  1.0
**************************************************************************/
#include <string.h>
#include <malloc.h>
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"



T_VOID Fwl_MallocInit(T_VOID)
{

}

T_pVOID Fwl_Malloc(unsigned long sz)
{  
    return malloc(sz);
}

T_pVOID Fwl_DMAMalloc(T_U32 size)
{
	return malloc(size);
}
T_U8 sys_rsv_buf[4096];
T_pVOID Fwl_SysRsvMalloc(T_VOID)
{
	return (T_pVOID)sys_rsv_buf;
}


T_pVOID Fwl_DMAFree(T_pVOID var)
{
    free(var);
	return 0;
}


T_pVOID Fwl_Free(T_pVOID var)
{
    free(var);
	return 0;
}

