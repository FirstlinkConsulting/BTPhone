#ifndef __JDRV_API_H__
#define __JDRV_API_H__

/**
* @FILENAME JDRV_api.h
* @BRIEF SpotLight11 JDRV library api  
* Copyright (C) 2011 Anyka (Guangzhou) Micro-electronics Technology Co., LTD
* @AUTHOR yan_chunxiang
* @DATE 2011-03-14
* @UPDATE 
* @VERSION 1.0
* @REF Snowbird2_micro_architecture.doc
*/

#ifdef __cplusplus
extern "C" {
#endif


#define JDRV_ENC_VERSION "Jdrv_enc_lib V1.1.03_UT"

/** 图像编码库使用说明：
>>  该图像编码库主要是JPEG编码，主要应用于拍照、录像及PC camera。
 >> 图像编码库初始化及必要设置
    在使用图像编码库的函数接口之前，必须要有以下设置：    
    (1)使用JDrv_EncInit()函数初始化图像编码库   
    示例代码：
    JDRV_ENC_CBFUNS cb_funs; 
    memset(&cb_funs, 0, sizeof(cb_funs));
    cb_funs.malloc = NewMalloc;	
    cb_funs.free   = NewFree;
	cb_funs.dmamalloc = NewDmaMalloc;
	cb_funs.dmafree = NewDmaFree;
	cb_funs.write = File_Write;
    cb_funs.printf = DebugOutput;
    JDrv_EncInit(&cb_funs);

	(2)使用JDrv_Set_Size()设置JPEG的尺寸
	示例代码：
	T_U16 jpg_dstwidth = 640;
	T_U16 jpg_dstheight = 480;
	JDrv_Set_Size(jpg_dstwidth,jpg_dstheight);

  >>开始JPEG编码，分拍照、录像及PC camera
    在启动编码前，如果需要改变图片质量，调用Set_Jpg_Quality()设置JPEG的编码质量。若无调用该接口，则使用缺省质量。
	启动编码JDrv_Start_EncJPG();注意由于JPEG的编码数据源来自Camera，JPEG的启动要在Camera的启动前设置。

	拍照调用示例：
	Set_Jpg_Quality(quality);
	JDrv_Start_EncJPG();
	// start camera
	if(JDrv_Capture_JPG(fileHandle)<0)
	{
		printf(" capture failed\n");
	}
	JDRV_Close_JPEG();

	录像调用示例：
	JDrv_Start_EncJPG();
	// start camera
	while(record)
	{
		if(Get_EncJpg_Lengh())
		{
			JDrv_Enc_WriteJPG(fileHandle);
		}
	}

	JDRV_Close_JPEG();

	PC camera调用示例：
   JDrv_Start_EncJPG();
   // start camera
	while(!end)
	{
		if(Get_EncJpg_Lengh())
		{
			JDrv_Enc_JPG2Buffer(fileHandle);
		}
	}
		
	JDRV_Close_JPEG();

>> JPEG完成时需调用JDrv_Jpg_Finished_Handler()来处理。
	该接口函数依赖于Camera的中断，在Camera中断调用该接口且需等待JPEG完成即JDrv_Jpg_Finished_Handler()返回1。
	
>> 退出图像编码库使用JDrv_EncDestroy()销毁，否则有内存泄露。

 *******/

typedef unsigned char			JDRV_T_U8;
typedef signed char				JDRV_T_S8;
typedef unsigned short			JDRV_T_U16;
typedef signed short			JDRV_T_S16;
typedef unsigned long			JDRV_T_U32;
typedef signed long				JDRV_T_S32;
typedef unsigned char			JDRV_T_BOOL;
typedef void					JDRV_T_VOID;

typedef void *					JDRV_T_pVOID;
typedef const char *		 	JDRV_T_pCSTR;		// const pointer of string
typedef JDRV_T_S32				JDRV_T_hFILE;    // file handle
typedef JDRV_T_U32				JDRV_T_hIMG;     // image handle

#define INVALID_FILE_HANDLE -1



////////////////////////////////////////////////////////////////////////////////
// Callback Function Definition
////////////////////////////////////////////////////////////////////////////////

typedef JDRV_T_pVOID (*JDrv_CBFUNC_MALLOC)(JDRV_T_U32 size);

typedef JDRV_T_pVOID (*JDrv_CBFUNC_FREE)(JDRV_T_pVOID var);


typedef JDRV_T_U32 (*JDRV_CBFUNC_FWRITE)(JDRV_T_hFILE hFile, JDRV_T_VOID *pBuffer, JDRV_T_U32 count);


typedef JDRV_T_VOID (*JDrv_CBFUNC_PRINTF)(JDRV_T_pCSTR format, ...);


typedef JDRV_T_pVOID (*JDrv_CBFUNC_DMAMALLOC)(JDRV_T_U32 size);

typedef JDRV_T_pVOID (*JDrv_CBFUNC_DMAFREE)(JDRV_T_pVOID var);


typedef struct
{
    // Memory Functions
    JDrv_CBFUNC_MALLOC       malloc;
    JDrv_CBFUNC_FREE         free;
    // File Access Functions   
	JDRV_CBFUNC_FWRITE		 write;

    // Other Functions
    JDrv_CBFUNC_PRINTF       m_printf;	
	JDrv_CBFUNC_DMAMALLOC    dmamalloc;
	JDrv_CBFUNC_DMAFREE      dmafree;
} JDRV_ENC_CBFUNS;


/** 
* @brief notify app when VBUF1 error occurred
* @author yan_chunxiang
* @date 2011-04-19
* @param[in] T_VOID 
* @return T_VOID
*/
typedef JDRV_T_VOID (*VBUF1ERRORCALLBACk)(JDRV_T_VOID);




////////////////////////////////////////////////////////////////////////////////
// Enumeration Definition
////////////////////////////////////////////////////////////////////////////////

typedef enum tagJDRVERROR
{
    JDrv_NO_ERROR,
	JDrv_PARAMETER_ERROR,
	JDrv_FILE_ERROR,
	JDrv_HEADER_ERROR,
	JDrv_STREAM_ERROR,
	JDrv_NOT_ENOUGH_MEMORY,
	JDrv_NOT_SUPPORT_FORMAT
} JDrv_T_ERROR;



typedef enum
{
	JDRV_ENCODE_MODE,
	JDRV_DECODE_MODE,
	JDRV_STN_MODE,
	JDRV_L2_MODE	
}VBUF_MODE;


typedef enum
{
	JDRV_RECORD_MODE,
	JDRV_CAPTURE_MODE,
	JDRV_PC_CAMERA_MODE	
}JDRV_MODE;



#define JDRV_INVALID_HANDLE  0



/******************************************************************************
 * Miscellaneous Function
 ******************************************************************************/
	
/** 设定回调函数
* @PARAM pCBFuns <in>函数指针结构体的指针
* @PARAM camera_dst_width <in>Camera输出宽度
* @PARAM camera_dst_height <in>Camera输出高度
*/
JDRV_T_hIMG JDrv_EncInit(const JDRV_ENC_CBFUNS *pCBFuns);

/**
* @BRIEF destroy image handle
* @PARAM hImg<in> image handle
*/
JDRV_T_VOID JDrv_EncDestroy(JDRV_T_hIMG hImg);




/** 
* @brief set notify callback function
* @author yan_chunxiang
* @date 2011-04-19
* @param[in] callback_func callback function
* @return T_VOID
*/
JDRV_T_VOID JDrv_SetVbuf1ErrorFunc(VBUF1ERRORCALLBACk func);



/** 获得最后一次出错的出错代码
* @RETURN 出错代码的枚举值(参见本文件中的JDrv_T_ERROR枚举类型定义)
*/
JDrv_T_ERROR JDrv_GetLastError(JDRV_T_VOID);

/**
* @BRIEF after change size, config vbuf1 register about encode mode 
* @PARAM JDrv_width: jpeg width
* @PARAM JDrv_height: jpeg height
* @RETVAL :parameter true return 1,or return 0
* @AUTHOR yan_chunxiang
* @DATE 2011-03-19
* @UPDATE 2011-03-21
* @NOTE:should be called after finished a frame 
*/
JDRV_T_S32 JDrv_Set_Size(JDRV_T_U32 JDrv_width,JDRV_T_U32 JDrv_height);


/**
* @BRIEF get encode data from VBuf2 and write to file
* @PARAM fHandle: the pointer of file handle
* @RETURN int: return the size of written file,return -1 if write file error 
*@return -2 if capture long time
* @AUTHOR yan_chunxiang
* @DATE 2011-03-22
* @UPDATE 2011-03-22
* @note: only be called in capture, JPEG stream data can be more than 44K
*/
JDRV_T_S32 JDrv_Capture_JPG (JDRV_T_hFILE fHandle);

/**
* @BRIEF set quantization table
* @PARAM quality: the quality of image,取值范围(0,200)
* @RETURN int
* @AUTHOR yan_chunxiang
* @DATE 2011-03-22
* @UPDATE 2011-03-22
*/
JDRV_T_U16	Set_Jpg_Quality(JDRV_T_U16 quality);



/**
* @BRIEF only get JPEG file length,include jpg file head length and stream length
* @PARAM none
* @RETURN int:the size of whole jpg file,if jpeg not finished , will return 0
* @AUTHOR yan_chunxiang
* @DATE 2011-03-22
* @UPDATE 2011-03-22
*/
JDRV_T_U32	Get_EncJpg_Lengh(JDRV_T_VOID);


/**
* @BRIEF get encode data from VBuf2 and write to file
* @PARAM fHandle: the pointer of file handle
* @RETURN int: return the size of written file
* @AUTHOR yan_chunxiang
* @DATE 2011-03-22
* @UPDATE 2011-03-22
* @note: only be called in recording, JPEG stream data is less than 44K
*/
JDRV_T_S32 JDrv_Enc_WriteJPG (JDRV_T_hFILE fHandle);



/**
* @BRIEF get encode data from VBuf2 and save to buffer
* @PARAM fHandle: the destination buffer
* @RETURN int: return the size of whole jpeg data
* @AUTHOR yan_chunxiang
* @DATE 2011-04-18
* @UPDATE 2011-04-18
* @note: only be called in recording, JPEG stream data is less than 44K
*/
JDRV_T_S32 JDrv_Enc_JPG2Buffer (JDRV_T_U8 *dstBuf);


/**
* @BRIEF set VBuf1 & VBuf2 mode
* @PARAM v_mode: enum of vbuf_mode
* @RETURN if parameter error return -1, else return 1
* @AUTHOR yan_chunxiang
* @DATE 2011-03-21
* @UPDATE 2011-03-21
*/
JDRV_T_S32 JDrv_Set_VBufMode(VBUF_MODE v_mode);



/**
* @BRIEF config the JPEG register and start encode codec 
* @PARA:jmode: capture, record of pc camera mode,
* @PARA:mem_size:malloc memsize acorrding j_mode
* @RETURN success:return malloc size,error return AK_FALSE
* @AUTHOR yan_chunxiang
* @DATE 2011-03-21
* @UPDATE 2011-03-21
* @NOTE:capture普通malloc，mem_size必为4K的倍数
*/
// JDRV_T_BOOL	JDrv_Start_EncJPG(JDRV_T_VOID);

JDRV_T_S32	JDrv_Start_EncJPG(JDRV_MODE j_mode,JDRV_T_U32 mem_size);


/**
* @BRIEF to receive VBUF2 jpg stream data, be called by timer interrupt of platform
* @PARAM none
* @RETURN  return read data from VBUF2
* @AUTHOR yan_chunxiang
* @DATE 2011-09-29
* @UPDATE 2011-09-29
*/
JDRV_T_S32  JDrv_ReceiveJpegData (JDRV_T_VOID);



/**
* @BRIEF restart encode codec 
* @PARA:none
* @RETURN always return true
* @AUTHOR yan_chunxiang
* @DATE 2011-03-21
* @UPDATE 2011-03-21
*/
JDRV_T_BOOL  JDrv_Restart_EncJPG(JDRV_T_VOID);


/**
* @BRIEF :丢帧
* @PARAM: framenum: 需要丢的帧数 
* @RETURN  return:T_U32 实际丢的帧数
* @AUTHOR yan_chunxiang
* @DATE 2012-01-09
*/
JDRV_T_U32  JDrv_Skipframe (JDRV_T_U32 framenum);

/**
* @BRIEF judge if codec has finished
* @PARAM none
* @RETURN int,if finished return 1,else return 0;
* @AUTHOR yan_chunxiang
* @DATE 2011-03-21
* @UPDATE 2011-03-21
*/
JDRV_T_S32 JDrv_Jpg_Finished_Handler(JDRV_T_VOID);



/**
* @BRIEF close jpeg codec
* @PARAM none
* @RETURN int
* @RETVAL always 0
* @AUTHOR yan_chunxiang
* @DATE 2011-03-19
* @UPDATE 2011-03-19
*/
JDRV_T_BOOL	JDRV_Close_JPEG(JDRV_T_VOID);



/**
* @BRIEF judge frame error or not
* @PARAM none
* @RETURN T_BOOL
* @RETVAL if error return 1,or return 0
* @AUTHOR yan_chunxiang
* @DATE 2011-03-29
* @UPDATE 2011-03-29
*/
JDRV_T_BOOL	JDRV_FrameError(JDRV_T_VOID);



/**
* @BRIEF modify writing pointer and clear frame error interrupt
* @PARAM none
* @RETURN int
* @AUTHOR yan_chunxiang
* @DATE 2011-03-21
* @UPDATE 2010-03-21
* @Note: used when frame error occurred
*/
JDRV_T_VOID JDrv_VBuf_ErrorHandler(JDRV_T_VOID);



/**
* @BRIEF reset VBuf1 module
* @PARAM none
* @RETURN int
* @AUTHOR yan_chunxiang
* @DATE 2010-10-21
* @UPDATE 2010-10-21
* @Note: used when frame error occurred
*/
JDRV_T_VOID JDrv_VBuf1_Reset(JDRV_T_VOID);



/**
* @BRIEF reset JPEG module
* @PARAM none
* @RETURN int
* @AUTHOR yan_chunxiang
* @DATE 2010-10-21
* @UPDATE 2010-10-21
* @Note: used when frame error occurred
*/
JDRV_T_VOID JDrv_JPEG_Reset(JDRV_T_VOID);



#ifdef __cplusplus
}
#endif

#endif    // #ifndef __JDRV_API_H__
