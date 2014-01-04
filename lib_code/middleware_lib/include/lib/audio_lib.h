#ifndef __AUDIO_LIB_H__
#define __AUDIO_LIB_H__

#include "medialib_struct.h"

#ifdef _WIN32
T_S32 waveOutCheckBuffer(T_VOID);
T_S32 waveOutWriteBuffer(T_VOID *pbuf, T_U32 len);
#endif

/**
 * @brief Init
 *
 * @author Deng_Zhou
 * @param	medialib_init		[in]	pointer of T_MEDIALIB_INIT struct
 * @return T_S32
 * @retval	0			open failed
 * @retval	1			open ok
 */
T_S32 MediaLib_Init(T_MEDIALIB_INIT *medialib_init);

/**
 * @brief Destroy
 *
 * @author Deng_Zhou
 * @return T_S32
 * @retval	0			destroy failed
 * @retval	1			destroy ok
 */
T_S32 MediaLib_Destroy(T_VOID);

/**
 * @brief Open encoder
 *
 * @author Deng_Zhou
 * @param	open_input		[in]	pointer of T_MEDIALIB_ENC_OPEN_INPUT struct
 * @return T_pVOID
 * @retval	AK_NULL			open failed
 * @retval	other			open ok
 */
T_pVOID MediaLib_Encode_Open(T_MEDIALIB_ENC_OPEN_INPUT *open_input);

/**
 * @brief Encode
 *
 * @author Deng_Zhou
 * @param	pEncode		[in]	pointer which is returned by MediaLib_Encode_Open function
 * @param	enc_buf		[in]	pointer of T_MEDIALIB_ENC_BUF_STRC struct
 * @return T_S32
 * @retval	< 0		encode failed
 * @retval	other	encoded length
 */
T_S32 MediaLib_Encode(T_pVOID pEncode, T_MEDIALIB_ENC_BUF_STRC *enc_buf);

/**
 * @brief Close encoder
 *
 * @author Deng_Zhou
 * @param	pEncode		[in]	pointer which is returned by MediaLib_Encode_Open function
 * @return T_S32
 * @retval	0	close failed
 * @retval	1	close ok
 */
T_S32 MediaLib_Encode_Close(T_pVOID pEncode);
                   
/**
 * @brief End encode
 * @author Zhou Jiaqing
 * @param  pEncode  [in]  pointer which is returned by MediaLib_Encode_Open function
 * @return T_S32
 * @retval encode length
**/
T_S32 MediaLib_Encode_Last(T_pVOID pEncode,T_MEDIALIB_ENC_BUF_STRC *enc_buf);
                   
/**
 * @brief Get recently maximum left and right channel sample
 *
 * @author LiJun
 * @param	T_S16 *waveValueL		[in] maxisum left channel sample	
 * @param	T_S16 *waveValueR		[in] maxisum right channel sample	
 * @return T_VOID
 */                   
T_VOID MediaLib_GetWaveMaxValue(T_S16 *waveValueL, T_S16 *waveValueR);  

                 
/**
 * @brief Get audio data
 *
 * @author Deng_Zhou
 * @param	pbuf	[out]	pointer of data buffer
 * @param	len		[out]	data length
 * @return T_S32
 * @retval	0	close failed
 * @retval	1	close ok
 */
T_S32 MediaLib_GetData(T_VOID **pbuf, T_U32 *len);

/**
 * @brief Set audio seek status
 *
 * @author Deng_Zhou
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetAudioSeek(T_VOID);

/**
 * @brief Set audio DAC buff status
 *
 * @author Deng_Zhou
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetAudioDASta(T_eDA_STA_TYPE instate);

/**
 * @brief Get audio DAC buff status
 *
 * @author Deng_Zhou
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_GetAudioDASta(T_eDA_STA_TYPE instate);

/**
 * @brief Get current time
 *
 * @author Deng_Zhou
 * @param	hMedia	[in]	pointer which is returned by MediaLib_Open function
 * @param	time_ms	[out]	current time
 * @return T_S32
 * @retval	0	get failed
 * @retval	1	get ok
 */
T_S32 MediaLib_GetCurTime(T_pVOID hMedia, T_U32 *time_ms);

/**
 * @brief Set volume
 *
 * @author Deng_Zhou
 * @param	volume	[in]	volume
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetVolume(T_U32 volume);

/**
 * @brief Set filter, will use eq's code. user can set freq center and gain freely
 *
 * @author Tang_Xuechai
 * @param	hMedia	[in]	pointer which is returned by MediaLib_Open function
 * @param	filters	[in]	filters containning freq center and gain information
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetEQFilters(T_pVOID hMedia, T_MEDIALIB_EQ_FILTERS *filters);

/**
 * @brief Set EQ mode
 *
 * @author Deng_Zhou
 * @param	hMedia	[in]	pointer which is returned by MediaLib_Open function
 * @param	eqmode	[in]	eq mode
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetEQMode(T_pVOID hMedia, T_U8 eqmode);

/**
 * @brief Set 3DSound
 *
 * @author Feng_Suiyu
 * @param	hMedia	[in]	pointer which is returned by MediaLib_Open function
 * @param	Mode	[in]	3DSound mode
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_Set3DSound(T_pVOID hMedia, T_S32 Mode);

/**
 * @brief Set WSOLA
 *
 * @author Deng_Zhou
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	wsola_tempo	[in]	wsola_tempo
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetWSOLA(T_pVOID hMedia, T_U8 wsola_tempo);

/**
 * @brief Select wsola arithmatic
 * @author Deng_Zhou
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	wsola_arith	[in]	wsola arithmetic
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetWSOLA_Arithmetic(T_pVOID hMedia, T_WSOLA_ARITHMATIC wsola_arith);

/**
* @brief Set talkingTom effect sound
* @author Tang_Xuechai
* @param	hMedia	[in]	pointer which is returned by MediaLib_Open function
* @param	mode	[in]	select talkingTom mode
* @return T_S32
* @retval	0	set failed
* @retval	1	set ok
*/
T_S32 MediaLib_SetTomSound(T_pVOID hMedia, T_S32 Mode);


//T_S32 MediaLib_DecPause(T_VOID);

//T_S32 MediaLib_DecResume(T_VOID);

//T_S32 MediaLib_DecStop(T_VOID);

//T_S32 MediaLib_DecSeek(T_U32 time_ms);

/**
 * @brief Set current time
 *
 * @author Deng_Zhou
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	time_ms		[in]	time in millisecond
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetCurTime(T_pVOID hMedia, T_U32 time_ms);

/**
 * @brief Get PCM info
 *
 * @author Deng_Zhou
 * @param	channels		[in]	channels
 * @param	bitsperSample	[in]	bitsperSample
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_SetPCMInfo(T_U16 channels, T_U16 bitsperSample);

/**
 * @brief check audio
 *
 * @author Deng_Zhou
 * @param	pbuf		[in]	audio data pointer
 * @param	len			[in]	data length
 * @param	valve		[in]	valve
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_CheckRecData(T_VOID *pbuf, T_U32 len, T_U32 valve);

/**
 * @brief get wma bitrate type which could be lpc/midrate/highrate
 *
 * @author LiJun
 * @param	T_pVOID hMedia		[in]	[in]	pointer which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	0	LPC 1:Midrate 2:Highrate
 */
T_S32 MediaLib_GetWMABitrateType(T_pVOID hMedia);

/**
 * @brief Set VolumeSmoothTime
 * @author Tang_Xuechai
 * @param	[in] time: smooth time for volume change
 * @return T_S32
 * @retval	0	set failed
 * @retval	1	set ok
 */
T_S32 MediaLib_Set_VolumeSmoothTime(T_U32 time);
#endif
