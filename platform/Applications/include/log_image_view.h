#ifndef __LOG_IMAGE_VIEW_H__
#define __LOG_IMAGE_VIEW_H__
/************************************************************************
 * Copyright (c) 2008, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author��zhao_xiaowei
 * @Date��
 * @Version��
**************************************************************************/

#include "Fwl_Keypad.h"
#include "Fwl_Timer.h"
#include "Lib_image_api.h"
#include "Fwl_osFS.h"
#include "Fwl_osMalloc.h"

#define MIN_SLIDER_TIME 1
#define MAX_SLIDER_TIME 10
#define SLIDER_COUNT (MAX_SLIDER_TIME- MIN_SLIDER_TIME)

//��Ч���أ�������Ҫ��ЧʱΪ0������Ϊ1
#define USE_SPEC_EFFECT             0

#define SE_MODE_DIRECT              0
#define SE_MODE_SLIDE_FROM_TOP      1
#define SE_MODE_SLIDE_FROM_BOTTOM   2
#define SE_MODE_SLIDE_FROM_LEFT     3
#define SE_MODE_SLIDE_FROM_RIGHT    4
#define SE_MODE_ZOOM_IN             5
#define SE_MODE_ZOOM_OUT            6
#define SE_MODE_SHUTTER_VERTICAL    7
#define SE_MODE_SHUTTER_HORIZONTAL  8
#if(USE_COLOR_LCD)

typedef struct {
    //image global var
    T_U16     file[MAX_FILE_LEN];
    T_BOOL    isFileChanged;
    T_BOOL    isSlider;
    T_U8      sldTmIntvl;
    T_U8      CheckCode;
    T_U32     Dummy;
}T_ImageVSet, T_IMAGE_CFG;

typedef struct {
    IMG_T_hGIF handle;
    T_U32 timerId;
    T_U32 delay;
    T_U32 totalDelay;
    T_BOOL toPaintNext;
}T_GifInfo,PGifInfo;

typedef struct {
    IMG_T_hIMG imageSysHandle;
    IMG_T_CBFUNS imageFuctions;
    IMG_T_TYPE imageType;
    T_GifInfo gifInfo;

    T_hFILE fileHandle;
    T_U32 sliderTimerId; //Ϊ0��ʾֹͣ
    T_ImageVSet imageVSet;
    T_U16     xPos;
    T_U16     yPos;
    T_U16     imgWidth;
    T_U16     imgHeight;
}T_ImageView, *PImageView;

#if(USE_SPEC_EFFECT)
typedef struct {
    T_U8* imgBuf;
    T_U16 imgWidth;
    T_U16 imgHeight;    
}T_ImgEffect;
extern T_ImgEffect* g_pImgEffect;
#endif

extern const T_U16 g_imageFilter[];
extern T_ImageView* g_pImageView;
//extern T_TIMER g_imgShowTimeOutId;

//�Զ���ͼƬ�ļ�������
T_U32  Img_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count);
T_VOID InitImageSystem(PImageView pImageView);
T_VOID FreeGifResource(PImageView pImageView);
T_BOOL DisplayCurGifFrame(PImageView pImageView, T_BOOL isFirstFrame);
//JGIF�ͷ�JGIF�� ����λ�úʹ�С�����ǲ�һ����
//��JGIF��ָ������λ�úʹ�С
//JGIF�������е�λ�ü�����ʾ�Ŀ���
T_BOOL ShowImage(PImageView pImageView, T_pCWSTR fileName,IMG_T_U16 xPos, IMG_T_U16 yPos,
                        IMG_T_U16 width, IMG_T_U16 height);
T_VOID FreeCurImageResource(PImageView pImageView);
T_VOID FreeImageViewResource(PImageView pImageView);
T_VOID DestroyImageSystem(PImageView pImageView);
T_VOID StartNextAtViewTimer(PImageView pImageView);
T_VOID StartNextFrame(PImageView pImageView);
T_BOOL GetImageViewSet(T_ImageVSet* pImageViewSet);
T_BOOL SetImageViewSet(const T_ImageVSet* const pImageVSet);
T_VOID Img_LcdRefresh(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content, T_BOOL reversed);

/************************************************************************
 * COPYRIGHT (C) 2009, ANYKA CO., LTD.
 * ALL RIGHTS RESERVED.
 *
 * @brief ������ʾ��Ч�ĺ���
 * ��USE_SPEC_EFFECTΪ 1ʱ ����ʾ��Ч������
 * @author��Zhao_Xiaowei
 * @date��2009-2
 * @version��
**************************************************************************/
#if USE_SPEC_EFFECT
T_VOID ShowEffect(
    T_U16 mode,  //��Ч����
    T_U16 showSpan, // ��ʾһ������ÿһ����ʱ�䣬��΢�����
    T_U16 maxProgress,  //��ʾһ������Ҫ�����в���
    T_U16 shutterWidth);  //һ����Ҷ���Ŀ��
#endif


#endif

#endif
