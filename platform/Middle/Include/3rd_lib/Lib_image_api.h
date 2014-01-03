#ifndef __IMAGE_API_H__
#define __IMAGE_API_H__

/**
* @FILENAME Lib_image_api.h
* @BRIEF SpotLight Image Library API
* Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR zhang_chengyan
* @DATE 2008-03-17
* @UPDATE 2009-01-21
* @VERSION 0.4.3
*/

/*******************************************************************************
    SpotLight Image Library API 说明

    图像库对图像格式的支持如下：
    
BMP图像：支持32Bit、24Bit、16Bit565、16Bit555、8Bit、4Bit、1Bit的非压缩BMP图像；
         支持高度为负数的Top-Bottom格式的BMP图像。
JPG图片：支持8位精度的Baseline或Progressive格式的JPEG图像。
GIF图片：支持GIF 87a和GIF 89a格式的GIF静态图像和GIF动画；支持非交织与交织格式的
         GIF图像。
JGIF图片：内部类似MJPEG格式，使用YUV420编码为JPEG

    由于微内存系统的限制，图像直接从文件中读入，并解码缩放显示到屏幕上（大于屏幕
尺寸的图像保持宽高比缩放到全屏，小于屏幕尺寸的图像则显示在屏幕中央），而不是保存
在内存中。所有解码过程都是纯软件实现，不使用任何硬件加速。


<1> 图像库初始化与销毁
    在调用图像库解码函数之前，必须调用Img_Init()函数对图像库进行初始化，主要工作
是申请内存资源用于保存一系列函数指针以及相关的一些信息，该函数返回一个IMG_T_hIMG
类型的图像句柄。
    与初始化相对应的是销毁过程，调用Img_Destroy()函数，传入有效的图像句柄即可，
注意调用此函数后应该设置图像句柄为无效（hImg = IMG_INVALID_HANDLE;）。

<2> 获取图像类型
    使用 Img_GetType() 函数，该函数有两个参数：
      hImg   - 图像初始化时获取得的图像句柄
      hFile  - 图像文件句柄
    该函数返回 IMG_T_TYPE 类型的枚举值，取值（具体的取值参见下面的枚举类型定义），
IMG_TYPE_UNKNOWN表示不支持的图像类型或发生其它出错。
    注意：不要在解码 GIF / JGIF 图像的各帧之间调用此函数，请在解码之前调用。

<3> 获取图像宽高信息
    使用 Img_GetInfo() 函数，该函数有四个参数：
      hImg   - 图像初始化时获取得的图像句柄
      hFile  - 图像文件句柄
      width  - (输出参数)图像宽度
      height - (输出参数)图像高度
    该函数返回值为AK_TRUE时表示成功，此时图像的宽高信息可在第三、四个参数中获取，
函数返回AK_FALSE时表示失败，此时第三、四个参数中的图像宽高信息无效。
    注意：不要在解码 GIF / JGIF 图像的各帧之间调用此函数，请在解码之前调用。

<4> 设置显示窗口及图像旋转
    在每次解码一个图像文件之前，调用 Img_SetDisplayWindow() 函数，可以设置显示窗
口的位置(参数：x, y)以及大小(参数：w, h)；
    调用 Img_EnableRotate() 函数，可以设置是否在窗口中自动旋转图片以获得较佳的显
示效果(图像面积较大)；
    调用 Img_SetRotateDirection() 函数，可以设置窗口中自动旋转图片的方向，可以是
顺时针(ROT_CLOCKWISE)或者逆时针(ROT_COUNTERCLOCKWISE)旋转。
    注意：这几个函数对JGIF图像不起效，JGIF图像不支持图像旋转，且其显示窗口在解码
函数调用 Img_DecJGIF() 时显式指定。

<5> 图像解码与显示
    I. 解码并显示 BMP/JPG 图像
    使用 Img_Decode() 函数，该函数有两个参数，第一个参数是图像初始化时获取得的图
像句柄，第二个参数是Unicode格式的图像文件名。该函数会自动识别 BMP/JPG 图像，并解
码显示，返回AK_TRUE表示正确解码; 返回AK_FALSE表示出错，可通过Img_GetLastError()
获取出错代码。
    使用 Img_DecBMP() 或 Img_DecJPG() 函数，可以分别解码 BMP 或 JPG 图像，这两个
函数的参数与返回值的含义同 Img_Decode() 函数。

    II. 解码并显示 GIF 图像
    对于每一幅 GIF 图像，首先需要调用Img_CreateGifHandle()函数创建一个IMG_T_hGIF
类型的GIF图像句柄，该函数有两个参数，第一个参数是图像初始化时获取得的图像句柄，
第二个参数是Unicode格式的图像文件名。若句柄创建失败则返回IMG_INVALID_HANDLE。
    获得了GIF图像句柄之后，可以使用Img_GetGifDelay()函数获取当前帧的延时，延时以
毫秒(ms)为单位（注意：应在调用Img_DecGIF()函数之前调用此函数获取延时）。该函数有
两个参数，第一个参数是图像初始化时获取得的图像句柄，第二个参数是GIF图像句柄。
    使用 Img_DecGIF() 函数，可以解码一帧GIF图像并显示， 该函数有两个参数，第一个
参数是图像初始化时获取得的图像句柄，第二个参数是GIF图像句柄。返回AK_TRUE表示正确
解码，且还有下一帧图像；若返回AK_FALSE，可通过Img_GetLastError()获取出错代码，
若出错代码为IMG_NO_ERROR，表示正确解码且所有帧已经解码完毕，否则表示出错。 如果
不断地调用 Img_DecGIF() 函数，则可以循环地解码帧图像。
    最后，需要调用 Img_FreeGifHandle() 函数释放创建GIF句柄时所申请的资源，该函数
有两个参数，第一个参数是图像初始化时获取得的图像句柄，第二个参数是GIF图像句柄，
注意调用此函数后应该设置图像句柄为无效（hGif = IMG_INVALID_HANDLE;）。

   III. 解码并显示JGIF图像
   访问接口与GIF基本相同，参考“II. 解码并显示 GIF 图像”。注意JGIF接口仅支持缩小
图像显示，不支持放大图像显示。

<6> 获取出错代码
    调用解码函数(Img_Decode(), Img_DecGIF())之后，如果返回值为AK_FALSE，可以通过
Img_GetLastError()函数获取出错代码， 该函数返回一个IMG_T_ERROR类型的枚举值（具体
的取值参见下面的枚举类型定义）。

<5> 示例伪代码

    IMG_T_CBFUNS cbFuns;
    IMG_T_hIMG   hImg;
    IMG_T_hGIF   hGif;
    T_U16   imgWidth, imgHeight;
    T_U32   delay;
    T_BOOL  bRet;

    // ImageLib Initialize
    cbFuns.malloc = (IMG_CBFUNC_MALLOC)New_Malloc;
    cbFuns.free   = (IMG_CBFUNC_FREE)New_Free;
    cbFuns.read  = (IMG_CBFUNC_FREAD)New_Read;
    cbFuns.seek  = (IMG_CBFUNC_FSEEK)New_Seek;
    cbFuns.tell  = (IMG_CBFUNC_FTELL)New_Tell;
    cbFuns.m_printf    = (IMG_CBFUNC_PRINTF)printf;
    cbFuns.refresh_lcd = (IMG_CBFUNC_REFRESH_LCD)RefreshLCD;
    hImg = Img_Init(&cbFuns);
    if (hImg == IMG_INVALID_HANDLE)
    {
        return;
    }
    Img_EnableRotate(hImg, AK_TRUE);

    // Decode BMP/JPG Image
    // Get hFile1 from strImageFile1
    Img_SetDisplayWindow(hImg, 0, 0, LCD_WIDTH, LCD_HEIGHT);
    bRet = Img_GetInfo(hImg, hFile, &imgWidth, &imgHeight);
    // ...
    bRet = Img_Decode(hImg, hFile1);
    if (!bRet)
    {
        printf("Error: %d, File: %s\n", (T_S32)Img_GetLastError(), strImageFile1);
    }

    // Decode GIF Image
    // Get hFile2 from strImageFile2
    Img_SetDisplayWindow(hImg, 9, 21, LCD_WIDTH * 2/3, LCD_HEIGHT * 2/3);
    bRet = Img_GetInfo(hImg, hFile, &imgWidth, &imgHeight);
    // ...
    hGif = Img_CreateGifHandle(hImg, hFile2);
    if (hGif == IMG_INVALID_HANDLE)
    {
        return;
    }
    do {
        delay = Img_GetGifDelay(hImg, hGif);
        // Set Timer        
        bRet = Img_DecGIF(hImg, hGif);
        if (!bRet && Img_GetLastError() != IMG_NO_ERROR))
        {
            printf("Error: %d, File: %s\n", (T_S32)Img_GetLastError(), strImageFile2);
        }
    } while (bRet);
    Img_FreeGifHandle(hImg, hGif);
    hGif = IMG_INVALID_HANDLE;

    // ImageLib Destory
    Img_Destroy(hImg);
    hImg = IMG_INVALID_HANDLE;

*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
// Type Definition
////////////////////////////////////////////////////////////////////////////////

typedef signed char         IMG_T_S8;
typedef signed short        IMG_T_S16;
typedef signed long         IMG_T_S32;
typedef unsigned char       IMG_T_U8;
typedef unsigned short      IMG_T_U16;
typedef unsigned long       IMG_T_U32;
typedef unsigned char       IMG_T_BOOL;
typedef void                IMG_T_VOID;
typedef void *              IMG_T_pVOID;

typedef const IMG_T_S8 *    IMG_T_pCSTR;    // const pointer of string
typedef IMG_T_S32           IMG_T_hFILE;    // file handle
typedef IMG_T_U32           IMG_T_hIMG;     // image handle
typedef IMG_T_U32           IMG_T_hGIF;     // GIF image handle
typedef IMG_T_U32           IMG_T_hJGIF;    // JGIF image handle

#define INVALID_FILE_HANDLE -1


////////////////////////////////////////////////////////////////////////////////
// Callback Function Definition
////////////////////////////////////////////////////////////////////////////////

typedef IMG_T_pVOID (*IMG_CBFUNC_MALLOC)
(IMG_T_U32 size);

typedef IMG_T_pVOID (*IMG_CBFUNC_FREE)
(IMG_T_pVOID var);

typedef IMG_T_U32  (*IMG_CBFUNC_FREAD)
(IMG_T_hFILE hFile, IMG_T_pVOID buffer, IMG_T_U32 count);

typedef IMG_T_S32  (*IMG_CBFUNC_FSEEK)
(IMG_T_hFILE hFile, IMG_T_S32 offset, IMG_T_U16 origin);

typedef IMG_T_U32  (*IMG_CBFUNC_FTELL)
(IMG_T_hFILE hFile);

typedef IMG_T_VOID (*IMG_CBFUNC_PRINTF)
(IMG_T_pCSTR format, ...);

typedef IMG_T_VOID (*IMG_CBFUNC_REFRESH_LCD)
(IMG_T_U16 left, IMG_T_U16 top, IMG_T_U16 width, IMG_T_U16 height,
 const IMG_T_U8 *content, IMG_T_BOOL reversed);


typedef struct
{
    // Memory Functions
    IMG_CBFUNC_MALLOC       malloc;
    IMG_CBFUNC_FREE         free;
    // File Access Functions
    IMG_CBFUNC_FREAD        read;
    IMG_CBFUNC_FSEEK        seek;
    IMG_CBFUNC_FTELL        tell;
    // Other Functions
    IMG_CBFUNC_PRINTF       m_printf;
    IMG_CBFUNC_REFRESH_LCD  refresh_lcd;
} IMG_T_CBFUNS;


////////////////////////////////////////////////////////////////////////////////
// Enumeration Definition
////////////////////////////////////////////////////////////////////////////////

typedef enum tagIMAGETYPE
{
    IMG_TYPE_UNKNOWN,
    IMG_TYPE_BMP,
    IMG_TYPE_JPG,
    IMG_TYPE_GIF,
    IMG_TYPE_JGIF
} IMG_T_TYPE;

typedef enum tagIMAGEERROR
{
    IMG_NO_ERROR,
    IMG_PARAMETER_ERROR,
    IMG_FILE_ERROR,
    IMG_HEADER_ERROR,
    IMG_STREAM_ERROR,
    IMG_NOT_ENOUGH_MEMORY,
    IMG_NOT_SUPPORT_FORMAT
} IMG_T_ERROR;

typedef enum tagIMAGEROTATE
{
    ROT_CLOCKWISE,
    ROT_COUNTERCLOCKWISE
} IMG_T_ROTATE;

#define IMG_INVALID_HANDLE  0


////////////////////////////////////////////////////////////////////////////////
// Image Library API
////////////////////////////////////////////////////////////////////////////////

IMG_T_hIMG Img_Init(const IMG_T_CBFUNS *pCBFuns);
IMG_T_VOID Img_Destroy(IMG_T_hIMG hImg);
IMG_T_TYPE Img_GetType(IMG_T_hIMG hImg, IMG_T_hFILE hFile);
IMG_T_BOOL Img_GetInfo(IMG_T_hIMG hImg, IMG_T_hFILE hFile,
                       IMG_T_U16 *width, IMG_T_U16 *height);
IMG_T_BOOL Img_SetDisplayWindow(IMG_T_hIMG hImg, IMG_T_U16 x, IMG_T_U16 y,
                                IMG_T_U16 w, IMG_T_U16 h);
IMG_T_BOOL Img_EnableRotate(IMG_T_hIMG hImg, IMG_T_BOOL enable);
IMG_T_BOOL Img_SetRotateDirection(IMG_T_hIMG hImg, IMG_T_ROTATE rotate);

IMG_T_ERROR Img_GetLastError(IMG_T_VOID);

// Decode BMP / JPG Image
IMG_T_BOOL Img_Decode(IMG_T_hIMG hImg, IMG_T_hFILE hFile);

// Decode BMP Image
IMG_T_BOOL Img_DecBMP(IMG_T_hIMG hImg, IMG_T_hFILE hFile);

// Decode JPEG Image
IMG_T_BOOL Img_DecJPG(IMG_T_hIMG hImg, IMG_T_hFILE hFile);

// Decode GIF Image
IMG_T_hGIF Img_CreateGifHandle(IMG_T_hIMG hImg, IMG_T_hFILE hFile);
IMG_T_VOID Img_FreeGifHandle(IMG_T_hIMG hImg, IMG_T_hGIF hGif);
IMG_T_U32  Img_GetGifDelay(IMG_T_hIMG hImg, IMG_T_hGIF hGif);
IMG_T_BOOL Img_DecGIF(IMG_T_hIMG hImg, IMG_T_hGIF hGif);

// Decode JGIF Image
IMG_T_hJGIF Img_CreateJGifHandle(IMG_T_hIMG hImg, IMG_T_hFILE hFile);
IMG_T_VOID  Img_FreeJGifHandle(IMG_T_hIMG hImg, IMG_T_hJGIF hJGif);
IMG_T_U32   Img_GetJGifDelay(IMG_T_hIMG hImg, IMG_T_hJGIF hJGif);
IMG_T_BOOL  Img_DecJGIF(IMG_T_hIMG hImg, IMG_T_hJGIF hJGif,
                        IMG_T_U16 xPos, IMG_T_U16 yPos,
                        IMG_T_U16 width, IMG_T_U16 height);

#ifdef __cplusplus
}
#endif

#endif // __IMAGE_API_H__

