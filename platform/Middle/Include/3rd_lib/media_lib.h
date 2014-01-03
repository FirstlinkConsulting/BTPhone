/**
 * @file media_player_spotlight.h
 * @brief This file provides AVI/mp3/wav/ape playing functions
 *
 * Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
 * @author Su Dan
 * @date 2008-5-20
 * @update date 2008-5-20
 * @version 0.1.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use playing APIs
   @code
T_VOID play_media(char* filaname);
T_VOID main(int argc, char* argv[])
{
	T_MEDIALIB_INIT init_input;

	init();	// initial file system, memory, lcd and etc.

	init_cb_func_init(&init_input);	//initial callback function pointer

	if (MediaLib_Init(&init_input) == AK_FALSE)
	{
		return;
	}
	//above only call one time when system start

	//play film or music
	play_media(argv[1]);

	//below only call one time when system close
	MediaLib_Destroy();
	return;
}

T_VOID play_media(char* filaname)
{
	T_S32 fid = 0;
	T_pVOID hMedia = AK_NULL;
	T_MEDIALIB_OPEN_INPUT open_input;
	T_MEDIALIB_MEDIA_INFO media_info;
	T_U32 begin_time = 0;
	T_S32 ret = 0;
	T_eMEDIALIB_STATUS player_status;

	fid = FileOpen("/test.avi");
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return;
	}

	open_input.m_hMediaSource = fid;
	open_cb_func_init(&(open_input.m_CBFunc));	//initial callback function pointer;
	
	open_input.m_FunVidioOut = video_out_func;
	open_input.m_vOutOffsetX = 0;
	open_input.m_vOutOffsetY = 0;

	hMedia = MediaLib_Open(&open_input);

	if (AK_NULL == hMedia)
	{
		FileClose(fid);
		return;
	}

	if (MediaLib_GetInfo(hMedia, &media_info) == AK_FALSE)
	{
		MediaLib_Close();
		FileClose(fid);
		return;
	}

	begin_time = MediaLib_SetPosition(hMedia, 5000);
	if (begin_time < 0)
	{
		MediaLib_Close(hMedia);
		FileClose(fid);
		return;
	}

	begin_time = MediaLib_Play(hMedia);
	if (begin_time < 0)
	{
		MediaLib_Close(hMedia);
		FileClose(fid);
		return;
	}

	while (1)
	{
		//return pts
		ret = MediaLib_Handle(hMedia);
		if (ret < 0)
		{
			player_status = MediaLib_GetStatus(hMedia);
			if (MEDIALIB_ERR == player_status)
			{
				printf("error\r\n");
				break;
			}
			else if (MEDIALIB_END == player_status)
			{
				printf("end\r\n");
				break;
			}
		}
	}

	MediaLib_Close(hMedia);

	FileClose(fid);
	return;
}
	@endcode

 ***************************************************/

#ifndef _MEDIA_PLAYER_LIB_H_
#define _MEDIA_PLAYER_LIB_H_

#include "medialib_struct.h"

#define MEDIA_LIB_VERSION	  "media_lib V1.9.00bchV1.0.13"


/**
 * @brief Get Player library version
 *
 * @author Su_Dan
 * @return const T_CHR *
 * @retval	version string
 */
const T_CHR *MediaLib_GetVersion(T_VOID);


/**
 * @brief Set CPU level
 *
 * @author Su_Dan
 * @param	bCPU2X		[in]	AK_TRUE: CPU 2X; AK_FALSE: CPU 1X
 * @return T_VOID
 */
T_VOID MediaLib_SetCPULevel(T_BOOL bCPU2X);


/**
 * @brief Open a resource
 *
 * @author Su_Dan
 * @param	open_input		[in]	pointer of T_MEDIALIB_OPEN_INPUT struct
 * @return T_pVOID
 * @retval	AK_NULL			open failed
 * @retval	other			open ok
 */
T_pVOID MediaLib_Open(T_MEDIALIB_OPEN_INPUT *open_input);

/**
 * @brief Close a resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL MediaLib_Close(T_pVOID hMedia);

/**
 * @brief Get information from an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	pInfo		[in]	pointer of T_MEDIALIB_MEDIA_INFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		get info ok
 * @retval	AK_FALSE	get info fail
 */
T_BOOL MediaLib_GetInfo(T_pVOID hMedia, T_MEDIALIB_MEDIA_INFO *pInfo);

/**
 * @brief Start playing an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	< 0		play fail
 * @retval	other	play time in millisecond
 */
T_S32 MediaLib_Play(T_pVOID hMedia);

/**
 * @brief Stop playing an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		stop ok
 * @retval	AK_FALSE	stop fail
 */
T_BOOL MediaLib_Stop(T_pVOID hMedia);

/**
 * @brief Pause playing resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		pause ok
 * @retval	AK_FALSE	pause fail
 */
T_BOOL MediaLib_Pause(T_pVOID hMedia);

/**
 * @brief Switch to fast forward status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		switch ok
 * @retval	AK_FALSE	switch fail
 */
T_BOOL MediaLib_FastForward(T_pVOID hMedia);

/**
 * @brief Switch to fast rewind status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		switch ok
 * @retval	AK_FALSE	switch fail
 */
T_BOOL MediaLib_FastRewind(T_pVOID hMedia);

/**
 * @brief Get current playing status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_eMEDIALIB_STATUS
 * @retval	MEDIALIB_END		end
 * @retval	MEDIALIB_PLAYING	playing
 * @retval	MEDIALIB_FF			fast forward
 * @retval	MEDIALIB_FR			fast rewind
 * @retval	MEDIALIB_PAUSE		pause
 * @retval	MEDIALIB_STOP		stop
 * @retval	MEDIALIB_ERR		error
 * @retval	MEDIALIB_SEEK		seek
 */
T_eMEDIALIB_STATUS MediaLib_GetStatus(T_pVOID hMedia);

/**
 * @brief Handle playing
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	< 0		decode fail or end of resource
 * @retval	other	play time in millisecond
 */
T_S32 MediaLib_Handle(T_pVOID hMedia);

/**
 * @brief Set media resource position
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	< 0		set fail
 * @retval	other	video time in millisecond
 */
T_S32 MediaLib_SetPosition(T_pVOID hMedia, T_S32 lMilliSec);

/**
 * @brief Release memory of information to save space
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		release ok
 * @retval	AK_FALSE	release fail
 */
T_BOOL MediaLib_ReleaseInfoMem(T_pVOID hMedia);

/**
 * @brief Release memory of information to save space
 *
 * @author Lin_Xiaoming
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		The Audio type supports seek
 * @retval	AK_FALSE	The Audio type don't supports seek
 */
T_BOOL MediaLib_AudioIsSeekable(T_pVOID hMedia);

/**
 * @brief Get picture data info from metainfo like ID3 
 *
 * @author Li Jun
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	**buf 		[out]  the buffer address that contain picture binary data
 * @param	*len      [out]  the pictrue length in the buf
 * @return T_BOOL
 * @retval	AK_TRUE		Get picture meta info succesfully
 * @retval	AK_FALSE	Get picture meta info failed
 */
T_BOOL MediaLib_GetPicMetaInfo(T_pVOID hMedia,T_S32 hAudioFile,T_U8 **buf, T_U32 *len);

/**
 * @brief Release memory that applied for picture metainfo.It is required to be called when 
 *  MediaLib_GetPicMetaInfo return AK_TRUE.
 * @author Li Jun
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	T_VOID
 */
T_VOID MediaLib_ReleasePicMetaInfo(T_pVOID hMedia);

/************************************************************************/
/**
 * @brief Set DA buffer length.
 *
 * @author Zhou_Jiaqing
 * @param	len [in]   want set DA buffer length
 * @return T_BOOL
 * @retval  AK_TRUE when set success
			AK_FALSE when set fail
 */
T_BOOL MediaLib_Set_DABuf_Len(T_S32 len);

/**
 * @brief   Get remain data length which in DA buffer
 * @author  Zhou_Jiaqing
 * @param   void
 * @return  T_BOOL
 * @return  T_S32
 * @retval  the data length in DA buffer 
 */
T_S32 MediaLib_Get_DABuf_Data_Len(T_VOID);

/************************************************************************/
/**
 * @brief   Get remain data length which in audio stream buffer 
 * @author  Zhou_Jiaqing
 * @param	hMedia [in], pointer which is returned by MediaLib_Open function
 * @return  T_S32
 * @retval	get data length
 */
T_S32  MediaLib_GetAudio_DataLen(T_pVOID hMedia);

/**
 * @brief   Get audio stream buffer length
 * @author  Zhou_Jiaqing
 * @param	hMedia [in], pointer which is returned by MediaLib_Open function
 * @return  T_S32
 * @retval	get buffer length
 */
T_S32  MediaLib_GetInput_BuffLen(T_pVOID hMedia);

/**
 * @brief   Get least data length witch is saved in audio stream buffer, for decoding frame.
 * @author  Zhou_Jiaqing
 * @param	hMedia [in], pointer which is returned by MediaLib_Open function
 * @return  T_S32
 * @retval	get buffer length
 */
T_S32 MediaLib_Get_MinBuf(T_pVOID hMedia);

/************************************************************************/
/**
 * @brief   read file data to audio stream buffer
 * @author  Zhou_Jiaqing
 * @param	hMedia [in], pointer which is returned by MediaLib_Open function
 * @param	len [in], data length (byte) wanted to read
 * @return  T_S32
 * @retval	>=0, read data length
 *          <0,  read fail
 */
T_S32 MediaLib_ReadAudio_Data(T_pVOID hMedia,T_S32 len);

#endif//_MEDIA_PLAYER_LIB_H_
