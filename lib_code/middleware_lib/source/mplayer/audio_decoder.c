#include "audio_decoder.h"
#include <string.h>
#include "Fwl_osmalloc.h"
#include "eng_debug.h"
#include "sdcodec.h"
#include "Fwl_Timer.h"
//#include "utils.h"
#if (defined(SUPPORT_VOICE_TIP) || defined(SUPPORT_BLUETOOTH))

static T_eDEC_OUTPUT_STATUS AudDecoder_FilterFrame(T_DEC_FILTER_CTRL *fltCtrl, T_U8 *rawData, T_U32 len);
static T_BOOL AudDecoder_FilterValveCheck(T_U8 *rawData, T_U32 len, T_U32 valve);


T_VOID BtPlayer_CheckStart(T_U32 freelen);

/**
 * @BRIEF	clear decoder instance
 * @AUTHOR	liangjian
 * @DATE	2012-11-22
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_S32
 * @RETVAL	AUDDEC_SUCCESS: Succeccful
 *			else:  Failed.
 */
T_VOID AudDecoder_Clear(T_HANDLE Ctrl)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;
    _SD_Buffer_Clear(pCtrl->decoder);
}

/**
 * @BRIEF	 destory decoder instance
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_S32
 * @RETVAL	AUDDEC_SUCCESS: Succeccful
 *			else:  Failed.
 */
T_BOOL AudDecoder_Close(T_HANDLE Ctrl)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;

    if (AK_NULL == pCtrl)
    {
        return AK_FALSE;
    }
    
    if (AK_NULL != pCtrl->decoder)
    {
        _SD_Decode_Close(pCtrl->decoder);
    }

    Fwl_Free(pCtrl);
    
    return AK_TRUE;
}

#pragma arm section code = "_SYS_BLUE_A2DP_INIT_CODE_"

/**
 * @brief	 create an instance for audio decoder(mp3/sbc -> pcm)
 * @author	wangxi
 * @date	2012-03-08
 * @param	T_eAUD_DECODER_TYPE:  the source package type
 * @retval	0: create error
			else:  the handle for decoder
 **/
T_HANDLE AudDecoder_Open(T_AUDIO_TYPE srcType, T_BOOL A2DPFlag, T_U32 audVol)
{
    T_AUD_DECODER *pCtrl;
    
    pCtrl = Fwl_Malloc(sizeof(T_AUD_DECODER));
    if (AK_NULL == pCtrl)
    {
        return 0;
    }
    memset(pCtrl, 0, sizeof(T_AUD_DECODER));
    //pCtrl->vol = audVol;
    //======================== Input ===========================
    pCtrl->input.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)AUDDEC_MALLOC;
    pCtrl->input.cb_fun.Free   = (MEDIALIB_CALLBACK_FUN_FREE)AUDDEC_FREE;
    pCtrl->input.cb_fun.delay  = 0;
    pCtrl->input.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)AUDDEC_PRINT; 
    
    pCtrl->input.m_info.decVolEna = 1;
    pCtrl->input.m_info.decVolume = audVol;
    pCtrl->input.m_info.m_Type = srcType; 
    pCtrl->input.m_info.m_InbufLen =  A2DPFlag?BLUE_SBC_BUF_LEN:VOICE_TIP_SBC_BUF_LEN;
    

    //=======================================================
    pCtrl->decoder = _SD_Decode_Open(&(pCtrl->input), &(pCtrl->output));

    if (AK_NULL == pCtrl->decoder)
    {
        AudDecoder_Close((T_HANDLE)pCtrl);
        return 0;
    }
    pCtrl->filterCtrl.status = eDEC_OUTPUT_STATUS_NULL;	
    pCtrl->filterCtrl.monitorStartTime = 0;

    return (T_HANDLE)pCtrl;
}
#pragma arm section code

#pragma arm section code = "_SYS_BLUE_A2DP_CODE_"
/**
 * @brief	 set the volume for output pcm(vol*pcm/1024)
 * @author	wangxi
 * @date	2012-03-08
 * @param	T_HANDLE hdl: the decoder handle
 * @param	T_U32 vol: the volume val (if ge 1024,  the volume will raise up, else raise down )
 * @retval	0: set error
			else:  the current vol
 **/
T_S32 AudDecoder_SetVolume(T_HANDLE Ctrl, T_U32 A2dpVolume)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;

    if (AK_NULL != pCtrl && AK_NULL != pCtrl->decoder)
    {
		return _SD_Decode_SetDigVolume(pCtrl->decoder, A2dpVolume);
    }
	return 0;
}

/**
 * @BRIEF	get the latest decode output information 
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	AD_DA_INFO
 * @RETVAL	AK_NULL: decode fail.
 *			else:  decode pcm play info.
 */
T_PCM_INFO *AudDecoder_GetCurDaInfo(T_HANDLE Ctrl,T_PCM_INFO * info)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;
	
	if (AK_NULL != pCtrl && AK_NULL != pCtrl->decoder && AK_NULL != info)
    {
		info->bps = pCtrl->output.m_BitsPerSample;
		info->samplerate = pCtrl->output.m_SampleRate;
		info->channel = pCtrl->output.m_Channels;
		return info;
    }
	return AK_NULL;
}

/**
 * @BRIEF	get current sbc SampleRate
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_U32
 * @RETVAL	SampleRate
 */
T_U32 AudDecoder_GetCurSampleRate(T_HANDLE Ctrl)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;
	
	if (AK_NULL != pCtrl && AK_NULL != pCtrl->decoder)
    {
		return pCtrl->output.m_SampleRate;
    }
	return 0;
}

/**
 * @BRIEF	get inner buffer (for store src data) free length
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
			T_U32 *OnceBufLen
 * @RETURN	T_U8 *
 * @RETVAL	prewrite pos
 */
T_U8 *AudDecoder_GetFreeLen(T_HANDLE Ctrl, T_U32 *OnceBufLen)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;
	
	*OnceBufLen = 0;
	pCtrl->libBufStatus = _SD_Buffer_Check(pCtrl->decoder, &pCtrl->libBufInfo);
	if (_SD_BUFFER_WRITABLE == pCtrl->libBufStatus)
	{
		*OnceBufLen = pCtrl->libBufInfo.free_len;
	}
	else if(_SD_BUFFER_WRITABLE_TWICE == pCtrl->libBufStatus)
	{
		*OnceBufLen = pCtrl->libBufInfo.free_len + pCtrl->libBufInfo.start_len;
	}
	else if(_SD_BUFFER_ERROR == pCtrl->libBufStatus)
	{
		AkDebugOutput("Y:%d\n",pCtrl->libBufStatus);
	}
	#ifdef SUPPORT_BLUETOOTH
	BtPlayer_CheckStart(*OnceBufLen);
	#endif
	return pCtrl->libBufInfo.pwrite;
}

/**
 * @BRIEF	add src date to  inner buffer ,before AudDecoder_AddSrc,you must call AudDecoder_GetFreeLen
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
			 T_U8 *data
			 T_U16 len
 * @RETURN	T_U32
 * @RETVAL	0: add src failed.
 *			else:  update length.
 */
T_U32 AudDecoder_AddSrc(T_HANDLE Ctrl, T_U8 *data, T_U16 len)
{
	T_AUDIO_BUFFER_CONTROL *libBufInfo;
	T_U32 totalLen;
	T_U32 needLen;
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;
 
    if (AK_NULL == pCtrl)
    {
        return 0;
    }

    if (AK_NULL == pCtrl->decoder)
    {
        return 0;
    }
	
    //======================= Input ===========================
    totalLen = len;
    libBufInfo = &pCtrl->libBufInfo;
	
    if (totalLen > 0)
    {
        if (_SD_BUFFER_WRITABLE == pCtrl->libBufStatus)
        {
        	if (libBufInfo->free_len < totalLen)
            {
				AkDebugOutput("S1:%d\n",libBufInfo->free_len);
                return 0;
            }

            memcpy(libBufInfo->pwrite,data, totalLen);
            _SD_Buffer_Update(pCtrl->decoder, totalLen);
        }
        else if (_SD_BUFFER_WRITABLE_TWICE == pCtrl->libBufStatus)// if inner buffer is ring back
        {
            if ((libBufInfo->free_len + libBufInfo->start_len) < totalLen)
            {
				AkDebugOutput("S2:%d\n",libBufInfo->free_len + libBufInfo->start_len);
                return 0;
            }
			
            needLen = libBufInfo->free_len;
        	if (libBufInfo->free_len > totalLen)
            {
                needLen = totalLen;
            }
            memcpy(libBufInfo->pwrite,data, needLen);
            _SD_Buffer_Update(pCtrl->decoder, needLen);
            
            if (needLen != totalLen) 
            {    
            	totalLen -= needLen;            
                
                memcpy(libBufInfo->pstart,data + needLen, totalLen);
                _SD_Buffer_Update(pCtrl->decoder, totalLen);
            }
        }
    }

    return len;
}

/**
 * @BRIEF	get the output is valid 
 * @AUTHOR	wangxi
 * @DATE	2012-05-24
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @PARAM	T_BOOL isValid: check valid or invalid
 * @RETURN	T_BOOL
 * @RETVAL	AK_TRUE: is valid.
 *			else:  invalid.
 */
T_BOOL AudDecoder_CheckOutput(T_HANDLE Ctrl, T_BOOL isValid)
{
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;

    if (AK_NULL == pCtrl)
    {
        return AK_FALSE;
    }
    
    if (AK_NULL == pCtrl->decoder)
    {
        return AK_FALSE;
    }

    #if OUTPUT_MUTE_CHECK_METHOD
    if (isValid)
    {
        return (eDEC_OUTPUT_STATUS_NORMAL == pCtrl->filterCtrl.status);
    }
    else
    {
        return (eDEC_OUTPUT_STATUS_MUTE == pCtrl->filterCtrl.status);
    }
	#else
	return AK_FALSE;
	#endif
    
}

/**
 * @BRIEF	decode the src from inner buffer(one frame be decode at once callle) 
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_U32
 * @RETVAL	0: decode fail.
 *			else:  decode pcm length.
 */
T_U32 AudDecoder_DecFrame(T_HANDLE Ctrl, T_U8 *pcmBuf, T_U32 bufLen)
{
    T_S32 decodeLen;
	T_AUD_DECODER *pCtrl = (T_AUD_DECODER *)Ctrl;

    if (AK_NULL == pCtrl)
    {
        return 0;
    }
    
    if ((AK_NULL == pCtrl->decoder) ||(AK_NULL == pcmBuf))
    {
        return 0;
    }
    //======================= Output ===========================
    pCtrl->output.m_pBuffer = pcmBuf;//DAC buffer
    pCtrl->output.m_ulSize  = bufLen;//Size
    decodeLen = _SD_Decode(pCtrl->decoder, &(pCtrl->output));


	if (decodeLen > 0)// ok
    {
        AudDecoder_FilterFrame(&pCtrl->filterCtrl, 
            pCtrl->output.m_pBuffer, pCtrl->output.m_ulSize);
        return decodeLen;
    }
    //else if (0 == decodeLen)// error code, need continue
    //{
    //    pcmBuf->len = 0;
   //     return -1;
   // }
    else // no data for decoding, need add src
    {
        return 0;
    }
}

/**
 * @BRIEF	 filter the stream
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_DEC_FILTER_CTRL *fltCtrl: filter control
 * @PARAM	T_U8 *rawData: raw data for filter
 * @PARAM	T_U32 len: raw data length
 * @RETURN	T_eDEC_OUTPUT_STATUS
 */
static T_eDEC_OUTPUT_STATUS AudDecoder_FilterFrame(T_DEC_FILTER_CTRL *fltCtrl, T_U8 *rawData, T_U32 len)
{
    if (!fltCtrl)
    {
        return eDEC_OUTPUT_STATUS_NULL;
    }
    
    if (AudDecoder_FilterValveCheck(rawData, len, OUTPUT_MUTE_THRESHOLD_DEFAULT))
    {
        fltCtrl->status = eDEC_OUTPUT_STATUS_NORMAL;
        if (fltCtrl->monitorStartTime)
        {
            fltCtrl->monitorStartTime = 0;
        }
    }
    else 
    {
        if (eDEC_OUTPUT_STATUS_MUTE != fltCtrl->status)
        {
            if (0 == fltCtrl->monitorStartTime)
            {
                fltCtrl->monitorStartTime = Fwl_GetTickCountMs();
            }
            else
            {
                T_U32 expiredTime;
        
                expiredTime = Fwl_GetTickCountMs();
        
                if (expiredTime < fltCtrl->monitorStartTime)
                {
                    fltCtrl->monitorStartTime = expiredTime;
                }
                else if (expiredTime > fltCtrl->monitorStartTime)
                {
                    expiredTime -= fltCtrl->monitorStartTime;
					if (expiredTime >= OUTPUT_MUTE_CHECK_ENABLE)
                    {
                        fltCtrl->status = eDEC_OUTPUT_STATUS_MUTE;
                    }
                }
            }
        }
    }
    return fltCtrl->status;
}

/**
 * @BRIEF	 check the threshold of output
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_U8 *rawData: data
 * @PARAM	T_U32 len: raw data length
 * @RETURN	T_BOOL whether in threshold 
 */
static T_BOOL AudDecoder_FilterValveCheck(T_U8 *rawData, T_U32 len, T_U32 valve)
{
#if OUTPUT_MUTE_CHECK_METHOD
    if(len)
    {
        T_S16 *pdata;
        T_U32 i;
        
        i = len >> 1;
        pdata = (T_S16 *)rawData;
        
        do
        {
            i--;
            if(pdata[i])
            {
				return AK_TRUE;
            }
        } while(i);
		return AK_FALSE;
    }
    return AK_TRUE;
#else
    extern T_S32 MediaLib_CheckRecData(T_VOID *pbuf, T_U32 len, T_U32 valve);

    return !!MediaLib_CheckRecData(rawData, len, valve);
#endif
}


#pragma arm section code

#endif
