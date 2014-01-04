/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved. 
 *  
 * File Name：Fontdisp.h
 * Function：This header file is API for font Library
 *
 * Author：xuping
 * Date：2008-04-07
 * Version：0.1.0
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#ifndef __ENG_FONT_H__
#define __ENG_FONT_H__

#include "anyka_types.h"
#include "Gbl_Global.h"
//#include "Fwl_nandflash.h"
#include "Eng_Debug.h"
#include "code_page.h"
#include "Gbl_resource.h"
#include "Eng_loadMem.h"

#define FONT_TYPE   (16) //选择字体大小，只支持12、16、24号字体

#ifndef USE_COLOR_LCD //黑白屏只支持16号字体
    #if(FONT_TYPE != 16)
        #undef FONT_TYPE
        #define FONT_TYPE   (16)
    #endif
#endif


//#define BLOCK_USED                  20               //the block used to save the font lib
#define MORE_RES_STRING_FLAG        0xFFFF

#if(FONT_TYPE == 12)
#define BLOCK_USED                  12               //the block used to save the font lib
#define FONT_WIDTH                  12               //字体的宽度
#define FONT_HEIGHT                 12               //字体的高度
#define FONT_PATH                   "W:/FontLib_12_Spot11.bin"
#define FONT_BYTE_COUNT             18               //字体点阵信息数据的大小（byte）
#endif
#if(FONT_TYPE == 16)
#define BLOCK_USED                  20               //the block used to save the font lib
#define FONT_WIDTH                  16               //字体的宽度
#define FONT_HEIGHT                 16               //字体的高度
#define FONT_PATH                   "W:/FontLib_16_Spot11.bin"
#define FONT_BYTE_COUNT             32               //字体点阵信息数据的大小（byte）
#endif
#if(FONT_TYPE == 24)
#define BLOCK_USED                  43               //the block used to save the font lib
#define FONT_WIDTH                  24              //字体的宽度
#define FONT_HEIGHT                 24               //字体的高度
#define FONT_PATH                   "W:/FontLib_24_Spot11.bin"
#define FONT_BYTE_COUNT        72                    //字体点阵信息数据的大小（byte）
#endif


//小号数字
#define SMALL_FONT_COUNT            95
#define SMALL_FONT_HEIGHT           8
#define SMALL_FONT_WIDTH            6
#define SMALL_FONT_BYTE_COUNT       6
#define SMALL_FONT_BEGIN            0x20
#define SMALL_FONT_END              0x7e

#define FORMAT_STRIKETHROUGH        (1 << 8)    //strikethrough
#define FORMAT_UNDERLINE            (1 << 9)    //underline
#define FORMAT_TOPLINE              (1 << 10)    //topline
#define FOTMAT_FIRSTCHAR            (1 << 11)    //first char of the string


#define PARA_COMPOUND(h,b,l)        (T_U32)(((((T_U16)(h)) << 16)) | (((T_U16)(b)) << 15) |((T_U16)(l & 0x7FFF)))
#define PARA_EXTRACTH(x)            (T_S16)((T_U32)(x) >> 16)
#define PARA_EXTRACTL(x)            (T_U16)((T_U32)(x) & 0x7FFF)
#define PARA_EXTRACTB(x)            (T_U16)(((T_U32)(x) >> 15) & 0x01)

#define FONT_LIB_NAME               "font_lib"
#define FONT_COMMONBUFFSIZE         2048

//字体显示窗口区域
#define FONT_WND_WIDTH              GRAPH_WIDTH
#define FONT_WND_HEIGHT             GRAPH_HEIGHT
#define CONVERT2STR(id)             (T_U8 *)(id)

//kind of the commom buffer is used for
#define FONTLIB_COMMBUF             (1<<0)
#define CODEPAGE_COMMBUF            (1<<1)

typedef struct
{
    T_S8 abcA;      //该字体位置向左的偏移
    T_U8 abcB;      //字体轮廓的宽度
    T_S8 abcC;      //该字体位置为右方字体预留的偏移
    T_U8 reserved;
} T_FONT_ABC;

typedef struct
{
    T_U16 resId;    //image resource id
    T_POS hOffset;  //horizontal offset
    T_POS vOffset;  //vertical offset
}T_BG_PIC;

typedef T_BOOL (*FONT_CALLBACK_GETMTRIX)(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format);
#if(NO_DISPLAY == 0)

//#define T_RES_STRING    T_U32
//#define T_RES_LANGUAGE  T_U32
/******************************************************************************
 * @NAME    FontLib_Init
 * @BRIEF   init fontlib global variable
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          T_VOID
 * @RETURN T_VOID
 *  
*******************************************************************************/
T_BOOL FontLib_Init(T_VOID);

/******************************************************************************
 * @NAME    DispStringInWidth
 * @BRIEF   disp string in width
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          codepage:the codepage of the str 
 *          x:appoint the position
 *          y:appoint the position
 *          pStr:the pointer to the string
 *          strLen :the length of the string for show
 *          width: the width to show unicode string
 *          color: the font color          
 * @RETURN disp length
*******************************************************************************/
T_U16  DispStringInWidth(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color);
#if (USE_COLOR_LCD)
#define Fwl_DispString(cp, x, y, str, width, color) DispStringInWidth(cp, x, y, str, width, color)
#else
#define Fwl_DispString(cp, x, y, str, width, color) DispStringInWidth(cp, x, (T_POS)((y) & 0xF8), str, width, color)
#endif


/******************************************************************************
 * @NAME    DispStringInWidth
 * @BRIEF   disp string in a specified width and a specified picture background
 * @AUTHOR  Justin.Zhao
 * @DATE    2008-05-17
 * @PARAM 
 *          codepage:the codepage of the str 
 *          x:appoint the position
 *          y:appoint the position
 *          pStr:the pointer to the string
 *          strLen :the length of the string for show
 *          width: the width to show unicode string
 *          color: the font color 
 *          BgColor: the background color
 * @RETURN disp length
*******************************************************************************/
T_U16  DispStringOnColor(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color, T_U16 BgColor);

/******************************************************************************
 * @NAME    DispStringInWidth
 * @BRIEF   disp string in a specified width and a specified color background
 * @AUTHOR  Justin.Zhao
 * @DATE    2008-05-17
 * @PARAM 
 *          codepage:the codepage of the str 
 *          x:appoint the position
 *          y:appoint the position
 *          pStr:the pointer to the string
 *          strLen :the length of the string for show
 *          width: the width to show unicode string
 *          color: the font color 
 *          pGg: the background structure pointer
 * @RETURN disp length
*******************************************************************************/
#if (USE_COLOR_LCD)
T_U16  DispStringOnPic(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color, T_BG_PIC *pGg);
#endif

/******************************************************************************
 * @NAME    GetStringDispWidth
 * @BRIEF   get the string width for display, if width is 0, then return a number of the displayed char in a special width
 * @AUTHOR  Justin.Zhao
 * @DATE    2008-05-20
 * @PARAM
 *          codepage:the codepage of the str 
 *          pStr: string pointer
 *          width: specified width for display
 * @RETURN disp width
*******************************************************************************/
T_U16  GetStringDispWidth(T_CODE_PAGE codepage,T_U8 *pStr, T_U16 width);
/******************************************************************************
 * @NAME    GetResCount
 * @BRIEF   get string resource ID between two ID.
 * @AUTHOR  xuping
 * @DATE    2008-04-14
 * @PARAM 
 *          BeginID: the begin id of str res
 *          EndID:the end id of str res
 *          language: the language of resourse to show
 * @RETURN  ResID Num
*******************************************************************************/
T_U16 GetResCount(T_RES_STRING BeginID,T_RES_STRING EndID,T_RES_LANGUAGE language);

/******************************************************************************
 * @NAME    GetResString
 * @BRIEF   get res string
 * @AUTHOR  xuping
 * @DATE    2008-04-14
 * @PARAM 
 *          ID: id of str res
 *          buf: buf to store string
 *          buflen: buf len
 * @RETURN >= 0  the string len
*******************************************************************************/
T_U16 GetResString(T_RES_STRING ID,T_U16 buf[], T_U16 bufLen);

/******************************************************************************
 * @NAME    GetValidStrResIDNum
 * @BRIEF   get valid ResStrID number
 * @AUTHOR  xuping
 * @DATE    2008-04-14
 * @PARAM 
 *          StrResID: the string id
 *          language: the language of resourse to show
 *          offset :>0 next offset string id
 *                  =0 check the string ID is valid
 *                  <0 pre  offset string id
 * @RETURN  StrID
*******************************************************************************/
T_RES_STRING  GetNearbyValidStrResID(T_RES_STRING StrResID,T_RES_LANGUAGE language,T_S16 offset);

/******************************************************************************
 * @NAME    UDispStringInWidth_S
 * @BRIEF   disp  string using small font within width
 * @AUTHOR  xuping
 * @DATE    2008-04-26
 * @PARAM 
 *          x:appoint the position
 *          y:appoint the position
 *          pStr:the pointer to thestring
 *          width :the width to show
 *          color:
 *          format:str disp format:FORMAT_STRIKETHROUGH ,FORMAT_UNDERLINE,FORMAT_TOPLINE
 * @RETURN disp length
 *  
*******************************************************************************/
T_U16 DispStringInWidth_S(T_POS x, T_POS y, T_U8* pStr, T_U16 Width,T_U16 color, T_U16 format);


/******************************************************************************
 * @NAME    GetFontMatrix
 * @BRIEF   Get FontMatrix  and abc of the unicode char
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          char: unicode char
 *          FontMatrix: store the font bitmap
 *          fontAbc:store abc information          
 * @RETURN 
 *      T_BOOL :AK_TRUE,get success;AK_FLASE,get failed
 *  
*******************************************************************************/
T_BOOL GetFontMatrix(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format);


/******************************************************************************
 * @NAME    Eng_FontRsgtFun
 * @BRIEF   register callback function
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *          callbackFun: callback fun to register
 * @RETURN T_VOID
 *  
*******************************************************************************/
T_VOID Eng_FontRsgtFun(FONT_CALLBACK_GETMTRIX callbackFun);

/********************************/
//资源字符串的动态加载
//加载对应语言的资源字符串到内存中去
//bufsize必须为一个语言的实际大小，destbuf大小必须>= BufSize
/************************************/

/******************************************************************************
 * @NAME    FontLib_Get_CommBuf
 * @BRIEF   get commom buffer pointer
 * @AUTHOR  luqizhou
 * @DATE    2011-03-21
 * @PARAM   kind: kind of the buffer is used for
 * @RETURN T_U8 * :pointer of the buffer
*******************************************************************************/
T_U8 *FontLib_Get_CommBuf(T_U8 kind) ;

/******************************************************************************
 * @NAME    FontLib_CommBuf_Malloc
 * @BRIEF   申请2K的公共缓存
                    第一个512字节用于字体背景色显示
                    第二个512字节用于读取NAND数据
                    第三个512字节用于存储字模
                    第四个512字节用于存储字库表格，加快速度
 * @AUTHOR  luqizhou
 * @DATE    2011-03-21
 * @PARAM   kind: kind of the buffer is used for
 * @RETURN T_U8 * :pointer of the buffer
*******************************************************************************/
T_U8 *FontLib_CommBuf_Malloc(T_U8 kind);

/******************************************************************************
 * @NAME    FontLib_CommBuf_Free
 * @BRIEF    free the commom buffer
 * @AUTHOR  luqizhou
 * @DATE    2011-03-21
 * @PARAM   kind: kind of the buffer is used for
 * @RETURN T_U8 * :NULL
*******************************************************************************/
T_U8 *FontLib_CommBuf_Free(T_U8 kind);

#else   // #if(NO_DISPLAY == 0)

#define FontLib_Init()  AK_TRUE

#define GetResString(ID, buf, bufLen)   0
#define DispStringInWidth(codepage, x, y, pStr, Width, color)   0
#define Fwl_DispString  DispStringInWidth
#define DispStringInWidth_S(x, y, pStr, Width, color, format)   0
#define DispStringOnColor(codepage, x, y, pStr,  Width, color, BgColor) 0
#define GetStringDispWidth(codepage, pStr, width)   0
#define GetResCount(BeginID, EndID, language)   0
#define GetNearbyValidStrResID(StrResID, language, offset)  0
T_U8 *FontLib_Get_CommBuf(T_U8 kind);
T_U8 *FontLib_CommBuf_Malloc(T_U8 kind);
T_U8 *FontLib_CommBuf_Free(T_U8 kind);

#endif

//T_BOOL Eng_StrResLoad(T_U32 lang, T_U8* DestBuf, T_U32 BufSize);
#endif
