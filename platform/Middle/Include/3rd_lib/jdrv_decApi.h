#ifndef __JDRV_DECAPI_H__
#define __JDRV_DECAPI_H__

/**
* @FILENAME image_api.h
* @BRIEF SpotLight Image Library API
* Copyright (C) 2011 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR yan_chunxiang
* @DATE 2011-04-19
* @UPDATE 2011-04-19
* @VERSION 0.0.1
*/

/*******************************************************************************
    SpotLight11 Image Library API 说明 --仅限于播放使用

    图像库对图像格式的支持如下：   

JPG图片：支持8位精度的Baseline的JPEG图像。

    由于微内存系统的限制，图像直接从文件中读入，并解码缩放显示到屏幕上，而不是保存
在内存中。所有解码及缩放过程都是纯硬件实现。


<1> 图像库初始化与销毁
    在调用图像库解码函数之前，必须调用Jdrv_Init()函数对图像库进行初始化，主要工作
是申请内存资源用于保存一系列函数指针以及相关的一些信息，该函数返回一个IMG_T_hJdrv
类型的图像句柄。
    与初始化相对应的是销毁过程，调用Jdrv_Destroy()函数，传入有效的图像句柄即可，
注意调用此函数后应该设置图像句柄为无效（hJdrv = Jdrv_INVALID_HANDLE;）。

  设置LCD大小
	图像库初始化后要先设置LCD大小，当JPEG硬件解码数据直接到LCD显示时要设置给Scaler2经过缩
放后的图片目的大小，如果数据经过L2，可以不设置该函数，否则必须设置调用Jdrv_SetLCDSize()，
设置LCD的大小。和Jdrv_SetDisplayWindow()参数w,h有所不同，w,h表示的待显示的窗口大小。
	 hJdrv   - 图像初始化时获取得的图像句柄	 
	 lcd_width  - LCD宽度
	 lcd_height - LCD高度


<2> 图像解码与显示
    I. 解码并显示jpg到LCD
    使用 Jdrv_VideoJpeg2Lcd() 函数，该函数有两个参数，第一个参数是图像初始化时获取得的图
像句柄，第二个参数是图像文件句柄，第三个参数是文件大小。该函数会自动识别 JPG 图像，并解
码显示，返回AK_TRUE表示正确解码; 返回AK_FALSE表示出错，可通过Jdrv_GetLastError()
获取出错代码。
    
   
<3> 获取出错代码
    可以通过Jdrv_GetLastError()函数获取出错代码， 该函数返回一个Jdrv_T_ERROR类型的枚举值（具体
的取值参见下面的枚举类型定义）。

<4> 示例伪代码

    Jdrv_T_CBFUNS cbFuns;
    Jdrv_T_hJdrv   hJdrv;   
    T_U16   JdrvWidth, JdrvHeight;
  
    T_BOOL  bRet;

    // ImageLib Initialize
    cbFuns.malloc = (Jdrv_CBFUNC_MALLOC)New_Malloc;
    cbFuns.free   = (Jdrv_CBFUNC_FREE)New_Free;
    cbFuns.read  = (Jdrv_CBFUNC_FREAD)New_Read;
    cbFuns.seek  = (Jdrv_CBFUNC_FSEEK)New_Seek;
    cbFuns.tell  = (Jdrv_CBFUNC_FTELL)New_Tell;
    cbFuns.m_printf    = (Jdrv_CBFUNC_PRINTF)printf;
	
    hJdrv = Jdrv_Init(&cbFuns);
    if (hJdrv == Jdrv_INVALID_HANDLE)
    {
        return;
    }
	Jdrv_SetLCDSize(hJdrv, lcd_width, lcd_height);   

    bRet = Jdrv_VideoJpeg2Lcd(hJdrv, hFile1);
    if (!bRet)
    {
        printf("Error: %d, File: %s\n", (T_S32)Jdrv_GetLastError(), strImageFile1);
    }
    Jdrv_Destroy(hJdrv);
    hJdrv = JDRV_INVALID_HANDLE;

*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
// Type Definition
////////////////////////////////////////////////////////////////////////////////

typedef signed char         JDRV_T_S8;
typedef signed short        JDRV_T_S16;
typedef signed long         JDRV_T_S32;
typedef unsigned char       JDRV_T_U8;
typedef unsigned short      JDRV_T_U16;
typedef unsigned long       JDRV_T_U32;
typedef unsigned char       JDRV_T_BOOL;
typedef void                JDRV_T_VOID;
typedef void *              JDRV_T_pVOID;

typedef const char *		JDRV_T_pCSTR;    // const pointer of string
typedef JDRV_T_S32          JDRV_T_hFILE;    // file handle
typedef JDRV_T_U32          JDRV_T_hImg;     // image handle


#define INVALID_FILE_HANDLE -1


////////////////////////////////////////////////////////////////////////////////
// Callback Function Definition
////////////////////////////////////////////////////////////////////////////////

typedef JDRV_T_pVOID (*JDRV_CBFUNC_MALLOC)
(JDRV_T_U32 size);

typedef void* (*CALLBACK_FUN_REMALLOC)(void* mem, JDRV_T_U32 size); 

typedef JDRV_T_pVOID (*JDRV_CBFUNC_FREE)
(JDRV_T_pVOID var);

typedef JDRV_T_U32  (*JDRV_CBFUNC_FREAD)
(JDRV_T_hFILE hFile, JDRV_T_pVOID buffer, JDRV_T_U32 count);

typedef JDRV_T_S32  (*JDRV_CBFUNC_FSEEK)
(JDRV_T_hFILE hFile, JDRV_T_S32 offset, JDRV_T_U16 origin);

typedef JDRV_T_U32  (*JDRV_CBFUNC_FTELL)
(JDRV_T_hFILE hFile);

typedef JDRV_T_VOID (*JDRV_CBFUNC_PRINTF)
(JDRV_T_pCSTR format, ...);




typedef struct
{
    // Memory Functions
    JDRV_CBFUNC_MALLOC       malloc;
    JDRV_CBFUNC_FREE         free;
    // File Access Functions
    JDRV_CBFUNC_FREAD        read; 
    // Other Functions
    JDRV_CBFUNC_PRINTF       m_printf;
} JDRV_T_CBFUNS;


////////////////////////////////////////////////////////////////////////////////
// Enumeration Definition
////////////////////////////////////////////////////////////////////////////////

typedef enum tagJDRVEERROR
{
    JDRV_NO_ERROR,
    JDRV_PARAMETER_ERROR,
    JDRV_FILE_ERROR,
    JDRV_HEADER_ERROR,
    JDRV_STREAM_ERROR,
    JDRV_NOT_ENOUGH_MEMORY,
    JDRV_NOT_SUPPORT_FORMAT
} JDRV_T_ERROR;



#define JDRV_INVALID_HANDLE  0


////////////////////////////////////////////////////////////////////////////////
// Image Library API
////////////////////////////////////////////////////////////////////////////////

JDRV_T_hImg Jdrv_DecInit(const JDRV_T_CBFUNS *pCBFuns);
JDRV_T_VOID Jdrv_Destroy(JDRV_T_hImg hJdrv);



JDRV_T_BOOL Jdrv_SetLCDSize(JDRV_T_hImg hImg, JDRV_T_U16 lcd_width, JDRV_T_U16 lcd_height);


JDRV_T_ERROR Jdrv_GetLastError(JDRV_T_VOID);


// Decode JPEG to LCD by hardware wholly, used for video play
JDRV_T_BOOL Jdrv_VideoJpeg2Lcd(JDRV_T_hImg hImg,JDRV_T_hFILE fHandle,T_U32 filesize);


#define AK_DEBUG 1


#ifdef __cplusplus
}
#endif

#endif // __IMAGE_API_H__

