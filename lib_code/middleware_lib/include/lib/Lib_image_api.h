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
    SpotLight Image Library API ˵��

    ͼ����ͼ���ʽ��֧�����£�
    
BMPͼ��֧��32Bit��24Bit��16Bit565��16Bit555��8Bit��4Bit��1Bit�ķ�ѹ��BMPͼ��
         ֧�ָ߶�Ϊ������Top-Bottom��ʽ��BMPͼ��
JPGͼƬ��֧��8λ���ȵ�Baseline��Progressive��ʽ��JPEGͼ��
GIFͼƬ��֧��GIF 87a��GIF 89a��ʽ��GIF��̬ͼ���GIF������֧�ַǽ�֯�뽻֯��ʽ��
         GIFͼ��
JGIFͼƬ���ڲ�����MJPEG��ʽ��ʹ��YUV420����ΪJPEG

    ����΢�ڴ�ϵͳ�����ƣ�ͼ��ֱ�Ӵ��ļ��ж��룬������������ʾ����Ļ�ϣ�������Ļ
�ߴ��ͼ�񱣳ֿ�߱����ŵ�ȫ����С����Ļ�ߴ��ͼ������ʾ����Ļ���룩�������Ǳ���
���ڴ��С����н�����̶��Ǵ����ʵ�֣���ʹ���κ�Ӳ�����١�


<1> ͼ����ʼ��������
    �ڵ���ͼ�����뺯��֮ǰ���������Img_Init()������ͼ�����г�ʼ������Ҫ����
�������ڴ���Դ���ڱ���һϵ�к���ָ���Լ���ص�һЩ��Ϣ���ú�������һ��IMG_T_hIMG
���͵�ͼ������
    ���ʼ�����Ӧ�������ٹ��̣�����Img_Destroy()������������Ч��ͼ�������ɣ�
ע����ô˺�����Ӧ������ͼ����Ϊ��Ч��hImg = IMG_INVALID_HANDLE;����

<2> ��ȡͼ������
    ʹ�� Img_GetType() �������ú���������������
      hImg   - ͼ���ʼ��ʱ��ȡ�õ�ͼ����
      hFile  - ͼ���ļ����
    �ú������� IMG_T_TYPE ���͵�ö��ֵ��ȡֵ�������ȡֵ�μ������ö�����Ͷ��壩��
IMG_TYPE_UNKNOWN��ʾ��֧�ֵ�ͼ�����ͻ�����������
    ע�⣺��Ҫ�ڽ��� GIF / JGIF ͼ��ĸ�֮֡����ô˺��������ڽ���֮ǰ���á�

<3> ��ȡͼ������Ϣ
    ʹ�� Img_GetInfo() �������ú������ĸ�������
      hImg   - ͼ���ʼ��ʱ��ȡ�õ�ͼ����
      hFile  - ͼ���ļ����
      width  - (�������)ͼ����
      height - (�������)ͼ��߶�
    �ú�������ֵΪAK_TRUEʱ��ʾ�ɹ�����ʱͼ��Ŀ����Ϣ���ڵ������ĸ������л�ȡ��
��������AK_FALSEʱ��ʾʧ�ܣ���ʱ�������ĸ������е�ͼ������Ϣ��Ч��
    ע�⣺��Ҫ�ڽ��� GIF / JGIF ͼ��ĸ�֮֡����ô˺��������ڽ���֮ǰ���á�

<4> ������ʾ���ڼ�ͼ����ת
    ��ÿ�ν���һ��ͼ���ļ�֮ǰ������ Img_SetDisplayWindow() ����������������ʾ��
�ڵ�λ��(������x, y)�Լ���С(������w, h)��
    ���� Img_EnableRotate() ���������������Ƿ��ڴ������Զ���תͼƬ�Ի�ýϼѵ���
ʾЧ��(ͼ������ϴ�)��
    ���� Img_SetRotateDirection() �������������ô������Զ���תͼƬ�ķ��򣬿�����
˳ʱ��(ROT_CLOCKWISE)������ʱ��(ROT_COUNTERCLOCKWISE)��ת��
    ע�⣺�⼸��������JGIFͼ����Ч��JGIFͼ��֧��ͼ����ת��������ʾ�����ڽ���
�������� Img_DecJGIF() ʱ��ʽָ����

<5> ͼ���������ʾ
    I. ���벢��ʾ BMP/JPG ͼ��
    ʹ�� Img_Decode() �������ú�����������������һ��������ͼ���ʼ��ʱ��ȡ�õ�ͼ
�������ڶ���������Unicode��ʽ��ͼ���ļ������ú������Զ�ʶ�� BMP/JPG ͼ�񣬲���
����ʾ������AK_TRUE��ʾ��ȷ����; ����AK_FALSE��ʾ������ͨ��Img_GetLastError()
��ȡ������롣
    ʹ�� Img_DecBMP() �� Img_DecJPG() ���������Էֱ���� BMP �� JPG ͼ��������
�����Ĳ����뷵��ֵ�ĺ���ͬ Img_Decode() ������

    II. ���벢��ʾ GIF ͼ��
    ����ÿһ�� GIF ͼ��������Ҫ����Img_CreateGifHandle()��������һ��IMG_T_hGIF
���͵�GIFͼ�������ú�����������������һ��������ͼ���ʼ��ʱ��ȡ�õ�ͼ������
�ڶ���������Unicode��ʽ��ͼ���ļ��������������ʧ���򷵻�IMG_INVALID_HANDLE��
    �����GIFͼ����֮�󣬿���ʹ��Img_GetGifDelay()������ȡ��ǰ֡����ʱ����ʱ��
����(ms)Ϊ��λ��ע�⣺Ӧ�ڵ���Img_DecGIF()����֮ǰ���ô˺�����ȡ��ʱ�����ú�����
������������һ��������ͼ���ʼ��ʱ��ȡ�õ�ͼ�������ڶ���������GIFͼ������
    ʹ�� Img_DecGIF() ���������Խ���һ֡GIFͼ����ʾ�� �ú�����������������һ��
������ͼ���ʼ��ʱ��ȡ�õ�ͼ�������ڶ���������GIFͼ����������AK_TRUE��ʾ��ȷ
���룬�һ�����һ֡ͼ��������AK_FALSE����ͨ��Img_GetLastError()��ȡ������룬
���������ΪIMG_NO_ERROR����ʾ��ȷ����������֡�Ѿ�������ϣ������ʾ���� ���
���ϵص��� Img_DecGIF() �����������ѭ���ؽ���֡ͼ��
    �����Ҫ���� Img_FreeGifHandle() �����ͷŴ���GIF���ʱ���������Դ���ú���
��������������һ��������ͼ���ʼ��ʱ��ȡ�õ�ͼ�������ڶ���������GIFͼ������
ע����ô˺�����Ӧ������ͼ����Ϊ��Ч��hGif = IMG_INVALID_HANDLE;����

   III. ���벢��ʾJGIFͼ��
   ���ʽӿ���GIF������ͬ���ο���II. ���벢��ʾ GIF ͼ�񡱡�ע��JGIF�ӿڽ�֧����С
ͼ����ʾ����֧�ַŴ�ͼ����ʾ��

<6> ��ȡ�������
    ���ý��뺯��(Img_Decode(), Img_DecGIF())֮���������ֵΪAK_FALSE������ͨ��
Img_GetLastError()������ȡ������룬 �ú�������һ��IMG_T_ERROR���͵�ö��ֵ������
��ȡֵ�μ������ö�����Ͷ��壩��

<5> ʾ��α����

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

