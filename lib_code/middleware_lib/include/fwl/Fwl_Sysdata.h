#ifndef __FWL_SYSDATA_H__
#define __FWL_SYSDATA_H__

#include "anyka_types.h"

/******************************************************************************
* @NAME    Fwl_Sysdata_Open 
* @BRIEF   Open System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM   name: System File Name
*          bAlign: Page Alignment  
*          
* @RETURN  < 0 Is Failure; Others Is System File Handle
*   
*******************************************************************************/
T_S32 Fwl_Sysdata_Open(T_S8 *name, T_BOOL bAlign);


/******************************************************************************
* @NAME    Fwl_Sysdata_Close 
* @BRIEF   Close System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM   name: System File Handle
*          
* @RETURN  AK_FALSE Is Failure; AK_TRUE Is Success
*   
*******************************************************************************/
T_BOOL Fwl_Sysdata_Close(T_S32 handle);


/******************************************************************************
* @NAME    Fwl_Sysdata_Read 
* @BRIEF   Read System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM handle: System File Handle
*		offset:	Offset Related Start page
*		len:		Read Data Length	
*		pBuf:	Read Data From  Storage Medium
*          
* @RETURN  0 Is Failure; Others Is Read Data Length
*   
*******************************************************************************/
T_U32 Fwl_Sysdata_Read(T_S32 handle, T_U32 offset, T_U32 len, T_U8 *pBuf);

/******************************************************************************
* @NAME    Fwl_Sysdata_Write 
* @BRIEF   Write Data to System File
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-16
* @PARAM handle: System File Handle
*		offset: Position of the Data Will be Writed(Must be 0).
* 		buff:	the Data Will Be Writed 
*		size:	Data Length
*          
* @RETURN  0 Is Failure; Others Is Writed Data Length
*   
*******************************************************************************/
T_U16 Fwl_Sysdata_Write(T_S32 handle, T_U32 offset, T_pVOID buff, T_U16 size);

#endif 	// End of __FWL_SYSDATA_H__