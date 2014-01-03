/*****************************************************************************
 * Copyright (C) Siemens AG 2003. All Rights reserved.
 *
 * Transmittal, reproduction and/or dissemination of this document as well
 * as utilization of its contents and communication thereof to others without
 * express authorization are prohibited. Offenders will be held liable for
 * payment of damages. All rights created by patent grant or registration of
 * a utility model or design patent are reserved.
 *****************************************************************************
 *    VME Version: 0.9$
 *****************************************************************************
 * $Workfile: vme.h $
 *     $Date: 2003/10/29 15:42:54 $
 *****************************************************************************
 * Requirements:
 * Target:  C16x Tasking C/ST10 6.0r6 or higher
 *****************************************************************************
*/
#ifndef _VME_H
#define _VME_H

#include "vme.h"

/** 
\defgroup scheduler Interface vme.h
This interface contains the basic VME definitions. This is the the only header file which must
included in all userware programs.  Other header files are included as necessary.
*/
/*@{ */
#ifndef VME_DOXYGEN_SHOULD_SKIP_THIS   
/* DO NOT USE !! WILL BE REMOVED !! */
#define MAX_PAGE_SIZE_C166  (16384)
#endif
/**
@defgroup btypes Basic Data Types
@ingroup scheduler

These are the basic VME data types which are used by all VME functions.
It is strongly recommended that these data types be used in every VME based userware
application.
*/


typedef struct tagM_EVENTENTRY
{
  vT_EvtCode event;
  vT_EvtParam* pEventParm;
  struct tagM_EVENTENTRY* pNext;
} M_EVENTENTRY;


//ak_identify.h
/************************AK Chip Indentity*************************
@Macros:
    @IS_AK_CHIP:    judge if the chip is ANYKA chip
    @AK_CHIPID:        AK Chip ID register addr
*******************************************************************/

#ifdef OS_WIN32
#define IS_AK_CHIP() (1)
#pragma warning(disable:4068)

#else

#define AK_CHIPID_ADDR 0x4000000
#define AK_CHIPID (*((volatile int *)(AK_CHIPID_ADDR)))

//open the chip identity
#ifdef AK_CHIP_INDENTITY
#define IS_AK_CHIP() (0x20101001 == AK_CHIPID)    //in AK3223, has no this register
//#define IS_AK_CHIP() (1==AK_CHIPID || 0==AK_CHIPID)
#else
//close the chip identity
#define IS_AK_CHIP() (1)
#endif

#endif  //OS_WIN32

#endif
