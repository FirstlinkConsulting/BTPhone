/************************************************************************
 * Copyright (c) 2011, Anyka Co., Ltd. 
 * All rights reserved. 
 *  
 * File Name£ºEng_loadMem.h
 * Function£ºThis header file is API for load Memory 
 * 
 * Author£º liangxiong
 * Date£º
 * Version£º0.0.1
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#ifndef __ENG_LOADMEM_H__
#define __ENG_LOADMEM_H__

#include "anyka_types.h"

/*********************************************************************
  Function:     LoadMem_Init
  Description:  initialize the load memory
  Input:        T_VOID
  Return:       T_VOID
  Author:       liangxiong
  Data:         2011-02-28
**********************************************************************/
T_VOID LoadMem_Init(T_VOID);

/*********************************************************************
  Function:     LoadMem_VBuffEnable
  Description:  The load memory enable using the Video buffer 
                during closing the Video function.
  Input:        true: the load memory can using vbuf. 
                false: can't using vbuf buring video working;
  Return:       true: seccess; false: error.
  Author:       liangxiong
  Data:         2011-4
**********************************************************************/
T_BOOL LoadMem_VBuffEnable(T_BOOL enable);

/*********************************************************************
  Function:     LoadMem_GetAllSize
  Description:  get the size of load memory 
  Input:        T_VOID
  Return:       T_U16 size of buffer
  Author:       liangxiong
  Data:         2011-02-28
**********************************************************************/
T_U16 LoadMem_GetAllSize(T_VOID);

/*********************************************************************
  Function:     LoadMem_GetBuffSize
  Description:  get the common size of the load memory
  Return:       T_U16   common size
  Author:       liangxiong
  Data:         2011-02-28
**********************************************************************/
T_U16 LoadMem_GetCommSize(T_VOID);

/*********************************************************************
  Function:     LoadMem_CommBuffMalloc
  Description:  Get the common buffer by the size. 
                At the same time,there is only one user. 
                Other wants to use the buffer, after using the LoadMem_CommBuffFree().
  Input:        T_U16   common size  (must less than 2KB,equal to multiple 512B)
  Return:       T_U8 *   common buffer 
  Author:       liangxiong
  Data:         2011-02-28
**********************************************************************/
T_U8 *LoadMem_CommBuffMalloc(T_U16 size);

T_U8 *LoadMem_GetCommBuff(T_VOID);

T_VOID LoadMem_CommBuffFree(T_VOID);

/*********************************************************************
  Function:     LoadMem_GetRemainBuff
  Description:  Get the remain buffer by the size. 
                If the buffer is null, it will malloc new buffer.
                At the same time,there is only one user. 
                Other wants to use the buffer, after using the LoadMem_RemainBuffFree().
  Output:       T_U16 * remain size
  Return:       T_U8 * remain buffer
  Author:       liangxiong
  Data:         2011-02-28
**********************************************************************/
T_U8 *LoadMem_GetRemainBuff(T_U16 * size);

T_VOID LoadMem_RemainBuffFree(T_VOID );

#endif //__ENG_LOADMEM_H__

