/**
 * @file Apl_fStandby.h
 * @brief This file is for standby  function ( field strength, battery etc) prototype
 *
 */

#ifndef __ENG_PROFILE_H__
#define __ENG_PROFILE_H__

#include "anyka_types.h"
#include "gbl_global.h"

/******************************************************************************
* @NAME    Profile_CheckData 
* @BRIEF   Check User Profile Data
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the Bufffer Will Be Checked  
*          
* @RETURN  AK_TRUE Is OK; AK_FALSE Is ERROR and than Will Load Default Data
*   
*******************************************************************************/
T_BOOL Profile_CheckData(T_eCFG_ITEM item, T_pVOID buff, T_BOOL ldDefault);

/******************************************************************************
* @NAME    Profile_ReadData 
* @BRIEF   Read User Data From Profile file 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the buf Is Read From the Profile File 
*          
* @RETURN  AK_TRUE Is Success or AK_FALSE Is Failure
*   
*******************************************************************************/
T_BOOL Profile_ReadData(T_eCFG_ITEM item, T_pVOID buff);

/******************************************************************************
* @NAME    Profile_WriteData 
* @BRIEF   Write User Data to Profile file 
* @AUTHOR  xie_wenzhong
* @DATE    2013-05-15
* @PARAM   item: Enumeration ITEM Name
*          buff: the buf to Write the Profile File 
*          
* @RETURN  AK_TRUE Is Success or AK_FALSE Is Failure
*   
*******************************************************************************/
T_BOOL Profile_WriteData(T_eCFG_ITEM item, const T_pVOID buff);

#endif // Eng of __ENG_PROFILE_H__
