/*******************************************************************************
 * @file    Fwl_Radio.c
 * @brief   this file will constraint the access to the bottom layer radio,
 *          avoid resource competition. Also, this file os for porting to
 *          different OS
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "Fwl_Radio.h"
#include "hal_radio.h"
#include "Eng_Debug.h"
#include "Fwl_FreqMgr.h"
#include "Gbl_define.h"



/*******************************************************************************
 * @brief   initialize radio
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioInit(T_VOID)
{
    return AK_FALSE;
}


/*******************************************************************************
 * @brief   free radio
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioFree(T_VOID)
{
    return AK_TRUE;
}


/*******************************************************************************
 * @brief   read the param into radio register
 * @author  guohui
 * @date    2007-03-20
 * @param   [out]status: refer to radio.h
 * @param   [in]freqMin: 
 * @param   [in]freqMax: 
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioGetStatus(T_RADIO_STATUS_EX *status, T_U32 freqMin, T_U32 freqMax)
{
    return AK_FALSE;
}


/*******************************************************************************
 * @brief   write the param into radio register
 * @author  guohui
 * @date    2007-03-20
 * @param   [in]param: refer to radio.h
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioSetParam (T_RADIO_PARM_EX *param)
{
    return AK_FALSE;           
}


/*******************************************************************************
 * @brief   set radio volume
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]volume: the volume to be set
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_RadioSetVolume(T_U8 volume)
{
    return AK_FALSE;           
}


/*******************************************************************************
 * @brief   auto search radio station
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]param: radio station array
 * @return  T_BOOL
 * @retval  1: initialize success; 0:initialize failed
*******************************************************************************/
T_BOOL Fwl_Radio_AutoSearch(T_U32 * pChanList)
{
    return AK_FALSE;
}

/* end of files */
