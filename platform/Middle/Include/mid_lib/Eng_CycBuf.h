/**
 * @file Eng_CycBuf.h
 * @brief This header file is for cycbuf related function prototype
 *
 */
#ifndef __CTRL_CYCBUF_H__
#define __CTRL_CYCBUF_H__

#include "Anyka_types.h"


#define MAX_CYCBUF_SIZE     (20<<12)

enum
{
	CYC_DA_BUF_ID,
	CYC_AD_BUF_ID,
	CYC_END_BUF_ID,
};

/*********************************************************************
  Function:         cycbuf_getdatasize
  Description:      get cycbuf data size
  Input:            T_U32 CycBufID: the cyc buffer id
  Return:           data size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/
T_U32 cycbuf_getdatasize(T_U32 CycBufID);


/*********************************************************************
  Function:         cycbuf_getblanksize
  Description:      get cycbuf blank size
  Input:            T_U32 CycBufID: the cyc buffer id
  Return:           blank size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getblanksize(T_U32 CycBufID);


/*********************************************************************
  Function:         cycbuf_create
  Description:      create cycbuf
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size you want to create
  Return:           create succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_BOOL cycbuf_create(T_U32 CycBufID, T_U32 size);

/*********************************************************************
  Function:         cycbuf_getwritebuf
  Description:      get a address which could be writed
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            buf: the data buf you want to write  
  Input:            T_U32 size: the size you want to write
  Return:           write size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getwritebuf(T_U32 CycBufID, T_U8** buf, T_U32 size);

/*********************************************************************
  Function:         cycbuf_getdatabuf
  Description:      get a address which could be read
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            buf: the data buf you want to read  
  Input:            T_U32 size: the size you want to read
  Return:           actul read size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getdatabuf(T_U32 CycBufID, T_U8** buf, T_U32 size);

/*********************************************************************
  Function:         cycbuf_write_updateflag
  Description:      update the cycbuf write flag 
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size had been writed in cycbuf
  Return:           T_VOID
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_VOID cycbuf_write_updateflag(T_U32 CycBufID, T_U32 size);

/*********************************************************************
  Function:         cycbuf_read_updateflag
  Description:      update the cycbuf read flag 
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size had been read in cycbuf
  Return:           T_VOID
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_VOID cycbuf_read_updateflag(T_U32 CycBufID, T_U32 size);


/*********************************************************************
  Function:         cycbuf_destory
  Description:      destory the cycbuf
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size you want to destory
  Return:           SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_BOOL cycbuf_destory(T_U32 CycBufID,T_U32 size);


/*********************************************************************
  Function:         cycbuf_clear
  Description:      clear the all data of the cycbuf
  Input:            T_U32 CycBufID: the cyc buffer id
  Return:           
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_VOID cycbuf_clear(T_U32 CycBufID);

#endif
