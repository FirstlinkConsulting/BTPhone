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
    SpotLight11 Image Library API ˵�� --�����ڲ���ʹ��

    ͼ����ͼ���ʽ��֧�����£�   

JPGͼƬ��֧��8λ���ȵ�Baseline��JPEGͼ��

    ����΢�ڴ�ϵͳ�����ƣ�ͼ��ֱ�Ӵ��ļ��ж��룬������������ʾ����Ļ�ϣ������Ǳ���
���ڴ��С����н��뼰���Ź��̶��Ǵ�Ӳ��ʵ�֡�


<1> ͼ����ʼ��������
    �ڵ���ͼ�����뺯��֮ǰ���������Jdrv_Init()������ͼ�����г�ʼ������Ҫ����
�������ڴ���Դ���ڱ���һϵ�к���ָ���Լ���ص�һЩ��Ϣ���ú�������һ��IMG_T_hJdrv
���͵�ͼ������
    ���ʼ�����Ӧ�������ٹ��̣�����Jdrv_Destroy()������������Ч��ͼ�������ɣ�
ע����ô˺�����Ӧ������ͼ����Ϊ��Ч��hJdrv = Jdrv_INVALID_HANDLE;����

  ����LCD��С
	ͼ����ʼ����Ҫ������LCD��С����JPEGӲ����������ֱ�ӵ�LCD��ʾʱҪ���ø�Scaler2������
�ź��ͼƬĿ�Ĵ�С��������ݾ���L2�����Բ����øú���������������õ���Jdrv_SetLCDSize()��
����LCD�Ĵ�С����Jdrv_SetDisplayWindow()����w,h������ͬ��w,h��ʾ�Ĵ���ʾ�Ĵ��ڴ�С��
	 hJdrv   - ͼ���ʼ��ʱ��ȡ�õ�ͼ����	 
	 lcd_width  - LCD���
	 lcd_height - LCD�߶�


<2> ͼ���������ʾ
    I. ���벢��ʾjpg��LCD
    ʹ�� Jdrv_VideoJpeg2Lcd() �������ú�����������������һ��������ͼ���ʼ��ʱ��ȡ�õ�ͼ
�������ڶ���������ͼ���ļ�������������������ļ���С���ú������Զ�ʶ�� JPG ͼ�񣬲���
����ʾ������AK_TRUE��ʾ��ȷ����; ����AK_FALSE��ʾ������ͨ��Jdrv_GetLastError()
��ȡ������롣
    
   
<3> ��ȡ�������
    ����ͨ��Jdrv_GetLastError()������ȡ������룬 �ú�������һ��Jdrv_T_ERROR���͵�ö��ֵ������
��ȡֵ�μ������ö�����Ͷ��壩��

<4> ʾ��α����

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

