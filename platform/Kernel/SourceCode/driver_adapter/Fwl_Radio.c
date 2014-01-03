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
#include "Fwl_WaveOut.h"
#include "Fwl_FreqMgr.h"
#include "Gbl_define.h"
#include "Fwl_Detect.h"


#ifdef SUPPORT_RADIO
static T_U8 wavId = INVAL_WAVEOUT_ID;


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
#ifdef OS_ANYKA
    T_BOOL ret = AK_FALSE;

    ret = radio_init();
    if (ret)
    {
        if (INVAL_WAVEOUT_ID != wavId)
        {
            Fwl_WaveOutClose(wavId);
        }
        wavId = Fwl_WaveOutOpen(LINEIN_HP, AK_NULL);
        if (INVAL_WAVEOUT_ID == wavId)
        {
            ret = AK_FALSE;
        }
        Fwl_WaveOutStart(wavId,0,0,0);
        //Fwl_WaveOutSetGain(wavId, 10);  //设置芯片增益
    }
    return ret;
#endif

#ifdef OS_WIN32
    return AK_TRUE;
#endif
}


/*******************************************************************************
 * @brief   free radio
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  1: exit success; 0:exit failed
*******************************************************************************/
T_BOOL Fwl_RadioFree(T_VOID)
{
#ifdef OS_ANYKA
    T_BOOL ret = AK_FALSE;

    if (INVAL_WAVEOUT_ID != wavId)
    {
        //Fwl_WaveOutStop(wavId);
        Fwl_WaveOutClose(wavId);
        wavId = INVAL_WAVEOUT_ID;
    }
    ret = radio_exit();
    return ret;
#endif

#ifdef OS_WIN32
    return AK_TRUE;
#endif
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
#ifdef OS_ANYKA
    return radio_get_status(status, freqMin, freqMax);
#endif

#ifdef OS_WIN32
    status->ReadyFlag = AK_TRUE;
    return AK_TRUE;
#endif
}


/*******************************************************************************
 * @brief   write the param into radio register
 * @author  guohui
 * @date    2007-03-20
 * @param   [in]param: refer to radio.h
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioSetParam(T_RADIO_PARM_EX *param)
{
#ifdef OS_ANYKA
    return radio_set_param(param);
#endif

#ifdef OS_WIN32
    return AK_TRUE;
#endif            
}


/*******************************************************************************
 * @brief   set radio volume
 * @author  zhanggaoxin
 * @date    2013-03-19
 * @param   [in]volume: the volume to be set
 * @return  T_BOOL
 * @retval  1: write success; 0:write failed
*******************************************************************************/
T_BOOL Fwl_RadioSetVolume(T_U8 volume)
{
#ifdef OS_ANYKA
    /*if (INVAL_WAVEOUT_ID != wavId)
    {
        Fwl_WaveOutSetGain(wavId, volume);
    }*/
	if (!Fwl_DetectorGetStatus(DEVICE_HP))
	{
		Fwl_SpkConnectSet(AK_TRUE);
	}
    if(1 == volume)
    {
        return radio_set_volume(volume);
    }
    else
    {
        return radio_set_volume(volume >> 1);
    }
#endif

#ifdef OS_WIN32
    return AK_TRUE;
#endif
}

//从最低频率开始搜台
T_BOOL	Fwl_Radio_AutoSearch(T_U32 * pChanList)
{
	#ifdef OS_ANYKA
        return radio_search_freq(pChanList);
    #endif
    
    #ifdef OS_WIN32
        return AK_TRUE;
    #endif
}

#else

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
    return AK_FALSE;
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
T_BOOL Fwl_RadioGetStatus(T_RADIO_STATUS_EX *Status, T_U32 FreqMin, T_U32 FreqMax)
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
T_BOOL Fwl_RadioSetParam (T_RADIO_PARM_EX *Param)
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
T_BOOL Fwl_RadioSetVolume(T_U8 Volume)
{
    return AK_FALSE;
}

T_BOOL	Fwl_Radio_AutoSearch(T_U32 * pChanList)
{
    return AK_FALSE;
}

#endif
/* end of files */
