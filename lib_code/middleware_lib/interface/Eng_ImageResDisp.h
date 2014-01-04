/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved. 
 *  
 * File Name��Eng_ImageReseDisp.h
 * Function��This header file of res image disp
 *
 * Author��xuping
 * Date��2008-04-07
 * Version��0.1.0
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#ifndef __IMAGERESOURCE_H__
#define __IMAGERESOURCE_H__

#include "anyka_types.h"
#include "Gbl_ImageRes.h"
#include "Eng_Graph.h"

#if(USE_COLOR_LCD)
#define IMAGE_RES_BLOCK_USED             160  //ͼƬ��Դռ�õ�block��С���ݶ�ÿ��block128k
#else
#define IMAGE_RES_BLOCK_USED             2  //ͼƬ��Դռ�õ�block��С���ݶ�ÿ��block128k
#endif
#define COMBUF_SIZE_USED                 512 //ÿ�ζ�ȡ���ݵĴ�С(BYTE)

#define INVALID_IMAGERES_ID              eRES_IMAGE_NUM
#define INVALID_STRINGRES_ID             (T_U16)GetResStrNum()

typedef struct
{   
    T_U16   width;      //ͼƬ�Ŀ�
    T_U16   height;     //ͼƬ�ĸ�
    T_U16  offset;      //ͼƬ���ļ��е�ƫ�ƣ�ÿ512byte��
} T_IMAGERES_HEADER;

#if(NO_DISPLAY == 1)

#define  Eng_ImageResInit(headbuf, ImageResNum) AK_TRUE
#define  Eng_ImageResDisp(x, y, imgeresID, format)  

#else

#define IMAGERES_LIB_NAME       "ImageRes"
#define IMAGERES_PATH           "W:/ImageRes.bin"

#define IMG_INVERTED            1
#define IMG_TRANSPARENT         2
#define IMG_TRNSPRNT_COLOR      RGB_COLOR(255,0,255)

//ͼƬ��ʾ��������
#define IMAGERES_WND_WIDTH      GRAPH_WIDTH
#define IMAGERES_WND_HEIGHT     GRAPH_HEIGHT

typedef T_BOOL (*IMAFERES_CALLBACK_GETDATA)(T_RES_IMAGE imgeResID, T_U16 Width, T_U16 Height,T_U16 hOffset, T_U16 vOffset,T_U8 data[]);

/******************************************************************************
 * @NAME    Eng_GetResImageWidth
 * @BRIEF   get resource image width
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          ImageResbuf:the point to the image data buf
 * @RETURN the width of image
 *  
*******************************************************************************/
T_U16 Eng_GetResImageWidth(T_RES_IMAGE imgeresID);


/******************************************************************************
 * @NAME    Eng_GetResImageHeight
 * @BRIEF   get resource image height
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          ImageResbuf:the point to the image data buf
 * @RETURN the height of image
 *  
*******************************************************************************/
T_U16 Eng_GetResImageHeight(T_RES_IMAGE imgeresID);

/******************************************************************************
 * @NAME    Eng_ImageResInit
 * @BRIEF   init image resourse variable
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *          T_VOID
 * @RETURN AK_TRUE:success
 *         AK_FALSE:fail
 *  
*******************************************************************************/
T_BOOL Eng_ImageResInit(T_U8* headbuf, T_U32 ImageResNum);

/******************************************************************************
 * @NAME    Eng_ImageResGetData
 * @BRIEF   load data of rectangular area  in image resource   
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *          imgeResID: image resource ID square
 *          Width : the width of the rectangular area
 *          Height: the height of the rectangular area
 *          hOffset: the horizonal offset of the rectangular area
 *          voffset: the vertical offset of the rectangular area
 *          data:    the buf of the data (must more then 1kB)                   
 * @RETURN AK_TRUE:success
 *         AK_FALSE:fail
 *  
*******************************************************************************/

T_BOOL Eng_ImageResGetData(T_RES_IMAGE imgeResID, T_U16 Width, T_U16 Height,T_U16 hOffset, T_U16 vOffset,T_U8 data[]);

/******************************************************************************
 * @NAME    Eng_ImageResDisp
 * @BRIEF   disp rectangular area of the image resource 
 * @AUTHOR  xuping
 * @DATE    2008-05-19
 * @PARAM 
 *          x:appoint the position
 *          y:appoint the position
 *          imgeresID: image resource ID
 *          format: 0 bit : reverse
 *                   bit :transparent
 * @RETURN AK_VOID
 *  
*******************************************************************************/

T_VOID Eng_ImageResDisp(T_POS x, T_POS y, T_RES_IMAGE imgeresID,T_U8 format);

/******************************************************************************
 * @NAME    Eng_ImageResDispEx
 * @BRIEF   disp resourse image 
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *          x:appoint the position
 *          y:appoint the position
 *          imgeresID: image resource ID
 *          DispWidth : the width of the rectangular area
 *          DispHeight: the height of the rectangular area
 *          hOffset: the horizonal offset of the rectangular area
 *          voffset: the vertical offset of the rectangular area  
 *          format: 0 bit : reverse
 *                  1 bit :transparent
 * @RETURN AK_VOID
 *  
*******************************************************************************/
#if (USE_COLOR_LCD)
T_VOID Eng_ImageResDispEx(T_POS x, T_POS y, T_RES_IMAGE imgeresID, T_U16 DispWidth,T_U16 DispHeight, T_U16  hOffset,T_U16 vOffset,T_U8 format);
#endif


/******************************************************************************
 * @NAME    GetResImagedata
 * @BRIEF   get resource image data
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *      imgeresID:res image id
 *          DestBuf: buf to store image
 * @RETURN the data length
 * 
*******************************************************************************/
T_U32 Eng_ImageResGetDataAll(T_RES_IMAGE imgeresID, T_U8* DestBuf);


T_VOID Eng_ImageRsgtFun(IMAFERES_CALLBACK_GETDATA callbackFun);

#endif

#endif

