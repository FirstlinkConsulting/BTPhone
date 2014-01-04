#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__
#include "anyka_types.h"
#include "sdcodec.h"
#include "log_pcm_player.h"
#include "medialib_struct.h"

#ifdef __cplusplus
extern "C"{
#endif


#define AUDDEC_SUCCESS             (0)
#define AUDDEC_FAILED              (1)
#define AUDDEC_INVALIDPARAM        (2)


#define BLUE_SBC_BUF_LEN	(16*1024)
#define VOICE_TIP_SBC_BUF_LEN	(2*1024)

/**
 * check mute hold time(ms)
*/
#define  OUTPUT_MUTE_CHECK_ENABLE        (1000)

/**
 * check mute pcm threshold val
*/
#define  OUTPUT_MUTE_THRESHOLD_MIN     (0x2)

/**
 * check mute  default valve val(for ipod)
*/
#define  OUTPUT_MUTE_THRESHOLD_DEFAULT (6)

/**
 * check mute pcm threshold method
*/
#define  OUTPUT_MUTE_CHECK_METHOD      1



#define AUDDEC_DEBUG(s,n)      akerror(s,n,1)
#define AUDDEC_PUTS(s)         akerror(s,0,1)
#define AUDDEC_PUTCH(c)        putch(c)
#define AUDDEC_PRINT           AK_DEBUG_OUTPUT

#define AUDDEC_MALLOC   Fwl_Malloc
#define AUDDEC_FREE     Fwl_Free




/**
 * decode output status
*/
typedef enum {
    eDEC_OUTPUT_STATUS_NULL,
    eDEC_OUTPUT_STATUS_MUTE,
    eDEC_OUTPUT_STATUS_NORMAL,
} T_eDEC_OUTPUT_STATUS;

/**
 * output filter control
*/
typedef struct {
    T_U32                timeThreshold;
    T_U32                monitorStartTime;
    T_eDEC_OUTPUT_STATUS status;
} T_DEC_FILTER_CTRL;

/**
 * decode control
*/
typedef struct {
	//T_U32 vol;
	T_VOID *decoder;//decode pcm handle
	T_AUDIO_DECODE_INPUT input;// input configure
	T_AUDIO_DECODE_OUT	 output;// out infomation
	T_DEC_FILTER_CTRL filterCtrl;//用于音频库对于mute控制
	T_AUDIO_BUFFER_CONTROL libBufInfo;
	T_AUDIO_BUF_STATE      libBufStatus;
} T_AUD_DECODER;





/**
 * @brief	 create an instance for audio decoder(mp3/sbc -> pcm)
 * @author	wangxi
 * @date	2012-03-08
 * @param	T_eAUD_DECODER_TYPE:  the source package type
 * @retval	0: create error
			else:  the handle for decoder
 **/
T_HANDLE AudDecoder_Open(T_AUDIO_TYPE srcType, T_BOOL A2DPFlag, T_U32 audVol);

/**
 * @BRIEF	clear decoder instance
 * @AUTHOR	liangjian
 * @DATE	2012-11-22
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_S32
 * @RETVAL	AUDDEC_SUCCESS: Succeccful
 *			else:  Failed.
 */
T_VOID AudDecoder_Clear(T_HANDLE hdl);

/**
 * @BRIEF	 destory decoder instance
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_S32
 * @RETVAL	AUDDEC_SUCCESS: Succeccful
 *			else:  Failed.
 */
T_BOOL AudDecoder_Close(T_HANDLE hdl);

/**
 * @BRIEF	get inner buffer (for store src data) free length
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
			T_U32 *OnceBufLen
 * @RETURN	T_U8 *
 * @RETVAL	prewrite pos
 */
T_U8 *AudDecoder_GetFreeLen(T_HANDLE hdl, T_U32 *OnceBufLen);

/**
 * @BRIEF	add src date to  inner buffer 
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_U32
 * @RETVAL	0: add src failed.
 *			else:  update length.
 */
T_U32 AudDecoder_AddSrc(T_HANDLE hdl, T_U8 *data, T_U16 len);

/**
 * @BRIEF	decode the src from inner buffer(one frame be decode at once callle) 
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_U32
 * @RETVAL	0: decode fail.
 *			else:  decode pcm length.
 */
T_U32 AudDecoder_DecFrame(T_HANDLE hdl, T_U8 *pcmBuf, T_U32 bufLen);

/**
 * @brief	 set the volume for output pcm(vol*pcm/1024)
 * @author	wangxi
 * @date	2012-03-08
 * @param	T_HANDLE hdl: the decoder handle
 * @param	T_U32 vol: the volume val (if ge 1024,  the volume will raise up, else raise down )
 * @retval	0: set error
			else:  the current vol
 **/
T_S32 AudDecoder_SetVolume(T_HANDLE Ctrl, T_U32 A2dpVolume);

/**
 * @BRIEF	get the latest decode output information 
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	AD_DA_INFO
 * @RETVAL	AK_NULL: decode fail.
 *			else:  decode pcm play info.
 */
T_PCM_INFO *AudDecoder_GetCurDaInfo(T_HANDLE Ctrl,T_PCM_INFO * info);

/**
 * @BRIEF	get current sbc SampleRate
 * @AUTHOR	wangxi
 * @DATE	2012-05-21
 * @PARAM	T_HANDLE hdl: decoder Handle
 * @RETURN	T_U32
 * @RETVAL	SampleRate
 */
T_U32 AudDecoder_GetCurSampleRate(T_HANDLE hdl);

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
T_BOOL AudDecoder_CheckOutput(T_HANDLE hdl, T_BOOL isValid);



#ifdef __cplusplus
}
#endif


#endif

