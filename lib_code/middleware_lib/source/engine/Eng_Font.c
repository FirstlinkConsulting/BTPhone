/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd. 
* All rights reserved.	
*  
* File Name：Fontdisp.c
* Function：offer the string disping interfence
*
* Author：xuping
* Date：2008-04-07
* Version：0.1.0
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
//#include "Gbl_Resource.h"
#include "Eng_Font.h"
#include "Eng_string.h"
#include "string.h"
#include "Gbl_Global.h"
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
//#include "Gbl_Global.h"
#include "Fwl_FreqMgr.h"
#if (USE_COLOR_LCD)
#include "log_aud_play.h"
#endif
#include "Fwl_osFS.h"

#if(NO_DISPLAY == 0)

#define DEFAULT_FONT            '?'
#define ALIGN(addr, align)			(((addr)+((align)-1))&~((align)-1))	
#define BUFFER_SIZE             (512)
#define SECTION_NUM		(1 + 0xffff/8/SECTION_SIZE)
#define SECTION_SIZE		(64)
#define TABLE_BUF_OFFSET    (512*3)
#define Get_Table_Buffer()            (FontLib_Get_CommBuf(FONTLIB_COMMBUF)+TABLE_BUF_OFFSET)        
#define LANGREION_IN_BUFFER   ((BUFFER_SIZE - gb_font.fontLibInfo.Format_LangRegion.LangRegion_Table)/4)
#define FONT_POS(ch)          ((ch)/CHAR_NUM_IN_512BYTE*512 + (ch)%CHAR_NUM_IN_512BYTE*(sizeof(T_FONT_ABC) + FONT_BYTE_COUNT))
#define FONT_OFFSET(ch)     ((ch)%CHAR_NUM_IN_512BYTE)* (sizeof(T_FONT_ABC) + FONT_BYTE_COUNT)

#define CHAR_NUM_IN_512BYTE                 (512/(sizeof(T_FONT_ABC) + FONT_BYTE_COUNT))
#define FontNumInPage(pageBit)             (CHAR_NUM_IN_512BYTE * (1 << ((pageBit) - 9)))
#define FontNumInBlock(blockBit)           (CHAR_NUM_IN_512BYTE * (1 << ((blockBit) - 9)))
#define GetBlock(ch, blockBit)             ((ch)/FontNumInBlock((blockBit)))   //第几个block
#define GetPage(ch, pageBit, blockBit)    ((ch)%FontNumInBlock((blockBit)))/FontNumInPage((pageBit)) //page num 
#define GetOffset(ch, pageBit, blockBit)  ((ch)%FontNumInBlock((blockBit)))%FontNumInPage((pageBit))/CHAR_NUM_IN_512BYTE //512 byte
#define STR_SEPARATOR           0x40000

#define HAVE_THAI_CONS                0x01
#define HAVE_THAI_THIRDCLASS_SPEC     0x02
#define THAI_FIRSTCLASS_EN            0x04
#define THAI_SECONDCLASS_EN           0x08
#define THAI_THIRDCLASS_EN            0x10

#define FONTLIB_FORMAT_NORMAL		    (0)		//字库格式类型，旧版本格式
#define FONTLIB_FORMAT_LANG_REGION    (1)		//字库格式类型，按语言域剪裁
#define FONTLIB_FORMAT_BITMAP		    (2)		//字库格式类型，按输入unicode码剪裁
#define INVALID_OFFSET				(0xffffffff)	//无效的偏移地址
#define MAX_LANGUAGE_REGION             146

typedef union
{  
	//字库头
    struct  
    {
        unsigned char    LibFormat:2;			//字库格式的类型
        unsigned char    FontHeight:6;          	//字体的大小    
    }FontLib_Header;

    //按语言域剪裁的格式
    struct  
    {
        unsigned char   LibFormat:2;			//字库格式的类型
        unsigned char   FontHeight:6;			//字体的大小    
        unsigned char   LangRegion_Table;     	//语言域查询表偏移
        unsigned short  unicode_min;			//支持的unicode码最小值
        unsigned short  unicode_max;          	//支持的unicode码最大值  
        unsigned short  FontLibOffset;			//首字模的偏移地址
    }Format_LangRegion;
    
    //按输入unicode码方式剪裁的格式
    struct  
    {
        unsigned char   LibFormat:2;                //字库格式的类型
        unsigned char   FontHeight:6;		    //字体的大小
        unsigned char   rev;				    //保留
        unsigned short  Sections_Table;         //分段表偏移
        unsigned short  BitMap_Table;           //位段查询表偏移
        unsigned short  FontLibOffset;		    //首字模的偏移地址
    }Format_BitMap;
}T_FONT_LIB_INFO;

//用于加快字模提取的数据结构
typedef struct
{
        T_U8	*pCommBuf;				        //字体显示时用到的commom buffer指针
        T_U32   bBufAva:1;                              //记录pCommBuf中的内容是否有效
        T_U32   record_LastRegion:8;    	        //记录上次使用的语言域
        T_U32   record_LastFontOffset:23;       //记录上次使用的语言域对应的字模偏移
        T_U8    commbuf_Manager;
}T_FONT_GET;

typedef enum{
    BG_NONE = 0,
        BG_COLOR,
        BG_PICTURE
}eBG_TYPE; //the background type

typedef struct
{
    eBG_TYPE    bgtype;  //background type
    T_U16       color;   // color of char
    T_U32       bgpara;  //background para
    T_U32       format;  //format for draw char
}FDRAW_PARA;

T_BOOL GetFontMatrix(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format);
T_U8 *GetBG(FDRAW_PARA *pDPara, T_POS offset, T_U16 width);
T_BOOL Tran_Thai_FONT(T_U16 ch, T_U16 *tempPos, T_U16 *orignalPos);
T_U16 GetResStringWidth(T_RES_STRING  ResID, T_U16 width);
T_BOOL Is_Thai_FirstClass(T_U16 ch);
T_BOOL Is_Thai_SecondClass(T_U16 ch);

static T_U16 GetFontMatrix_FromFontLib (T_U16 ch, T_U8* pBuf);
static T_BOOL FontLib_ReadFile(T_U32 pos, T_U8 *pBuf);
static T_VOID GetDefaultFontMatrix(T_U8* pBuf);


//字体显示的全局结构
typedef struct
{
    T_U8    pageBit;                //size of one page using bit(2^n)
    T_U8    blockBit;               //size of one block using bit(2^n)
    FONT_CALLBACK_GETMTRIX getMatrixFun;
    T_FONT_LIB_INFO fontLibInfo;   //字库信息 
    T_FONT_GET  fontGet;                //用于加快字模提取
    T_U16   font2block[BLOCK_USED];  //tabal of font information save in nandflash   
}T_GLOAL_FONT;

#pragma arm section zidata = "_bootbss1_"
static T_GLOAL_FONT gb_font;
#pragma arm section zidata

#pragma arm section rwdata = "_cachedata_"
static T_U8 TCh_Attr = THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN | THAI_THIRDCLASS_EN;
#pragma arm section rwdata

//extern T_NANDFLASH nf_info; //T_NANDFLASH:80B

#pragma arm section code = "_fontdisplay_"

#if(FONT_TYPE == 12)
static const T_U8   Default_Font[FONT_BYTE_COUNT+sizeof(T_FONT_ABC)] = 
{
    0x00, 0x04, 0x01, 0xff, 0x08, 0x88, 0x48, 0x30, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

#if(FONT_TYPE == 16)
static const T_U8   Default_Font[FONT_BYTE_COUNT+sizeof(T_FONT_ABC)] = 
{
    0x00, 0x05, 0x01, 0xff, 0x20, 0x10, 0x10, 0x90, 
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
};
#endif

#if(FONT_TYPE == 24)
static const T_U8   Default_Font[FONT_BYTE_COUNT+sizeof(T_FONT_ABC)] = 
{
    0x01, 0x08, 0x00, 0xff, 0xc0, 0x60, 0x60, 0x60, 
    0x60, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0,
    0x10, 0x18, 0x0f, 0x07, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0c0c, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
};
#endif

static const T_U16 Check_LangRegion[MAX_LANGUAGE_REGION] = 
{
	0x0000, 0x0080, 0x0100, 0x0180, 0x0250, 0x02B0, 0x0300, 0x0370,
	0x0400, 0x0500, 0x0530, 0x0590, 0x0600, 0x0700, 0x0750, 0x0780,
	0x07C0, 0x0800, 0x0860, 0x0880, 0x0900, 0x0980, 0x0A00, 0x0A80, 
	0x0B00, 0x0B80, 0x0C00, 0x0C80, 0x0D00, 0x0D80, 0x0E00, 0x0E80,
	0x0F00, 0x1000, 0x10A0, 0x1100, 0x1200, 0x1380, 0x13A0, 0x1400,
	0x1680, 0x16A0, 0x1700, 0x1720, 0x1740, 0x1760, 0x1780, 0x1800, 
	0x18B0, 0x1900, 0x1950, 0x1980, 0x19E0, 0x1A00, 0x1A20, 0x1A80, 
	0x1B00, 0x1B80, 0x1BC0, 0x1C00, 0x1C50, 0x1C80, 0x1D00, 0x1D80, 
	0x1DC0, 0x1E00, 0x1F00, 0x2000, 0x2070, 0x20A0, 0x20D0, 0x2100, 
	0x2150, 0x2190, 0x2200, 0x2300, 0x2400, 0x2440, 0x2460, 0x2500, 
	0x2580, 0x25A0, 0x2600, 0x2700, 0x27C0, 0x27F0, 0x2800, 0x2900, 
	0x2980, 0x2A00, 0x2B00, 0x2C00, 0x2C60, 0x2C80, 0x2D00, 0x2D30, 
	0x2D80, 0x2E00, 0x2E80, 0x2F00, 0x2FF0, 0x3000, 0x3040, 0x30A0, 
	0x3100, 0x3130, 0x3190, 0x31A0, 0x31C0, 0x31F0, 0x3200, 0x3300, 
	0x3400, 0x4DC0, 0x4E00, 0xA000, 0xA490, 0xA500, 0xA660, 0xA700, 
	0xA720, 0xA800, 0xA840, 0xA880, 0xA900, 0xA980, 0xAA00, 0xAA40, 
	0xAA80, 0xAB00, 0xAB80, 0xAC00, 0xD800, 0xDC00, 0xE000, 0xF900, 
	0xFB00, 0xFB50, 0xFE00, 0xFE10, 0xFE20, 0xFE30, 0xFE50, 0xFE70, 
	0xFF00, 0xFFF0
};

#pragma arm section code 


#pragma arm section rodata = "_fontdisplay_"

static const T_U8 Small_Font[SMALL_FONT_COUNT][SMALL_FONT_BYTE_COUNT] = 
{
    {0x00,  0x00,  0x00,  0x00,  0x00,  0x00},    //  sp
    {0x00,  0x00,  0x5e,  0x5e,  0x00,  0x00},    //  !
    {0x00,  0x0e,  0x00,  0x0e,  0x00,  0x00},    //  "
    {0x20,  0x74,  0x2c,  0x76,  0x2c,  0x06},    //  #
    {0x44,  0x24,  0xca,  0x4b,  0x32,  0x00},    //  $
    {0x46,  0x29,  0x16,  0x68,  0x94,  0x62},    //  %
    {0x00,  0x36,  0x49,  0x36,  0x50,  0x08},    //  &
    {0x00,  0x0e,  0x0e,  0x00,  0x00,  0x00},    //  '
    {0x00,  0x7e,  0x81,  0x00,  0x00,  0x00},    //  (
    {0x00,  0x81,  0x7e,  0x00,  0x00,  0x00},    //  )
    {0x00,  0x14,  0x0e,  0x0e,  0x14,  0x00},    //  *
    {0x00,  0x08,  0x08,  0x3e,  0x08,  0x08},    //  +
    {0x00,  0x40,  0x20,  0x00,  0x00,  0x00},    //  ,  
    {0x00,  0x10,  0x10,  0x10,  0x10,  0x00},   //  -
    {0x00,  0x60,  0x60,  0x00,  0x00,  0x00},    //  .
    {0x00,  0x20,  0x10,  0x08,  0x04,  0x00},    //   /
    {0x00,  0x3c,  0x42,  0x42,  0x3c,  0x00},   //  0
    {0x00,  0x00,  0x04,  0x7e,  0x00,  0x00},   //  1
    {0x00,  0x44,  0x62,  0x52,  0x4e,  0x00},   //  2
    {0x00,  0x20,  0x4a,  0x4a,  0x34,  0x00},   //  3
    {0x00,  0x30,  0x28,  0x24,  0x7e,  0x20},   //  4
    {0x00,  0x2c,  0x4a,  0x4a,  0x32,  0x00},   //  5
    {0x00,  0x3c,  0x4a,  0x4a,  0x32,  0x00},   //  6
    {0x00,  0x02,  0x72,  0x0a,  0x06,  0x00},   //  7
    {0x00,  0x34,  0x4a,  0x4a,  0x34,  0x00},   //  8
    {0x00,  0x4c,  0x52,  0x52,  0x3c,  0x00},   //  9
    {0x00,  0x00,  0x24,  0x24,  0x00,  0x00},   //  :
    {0x00,  0x00,  0x40,  0x24,  0x00,  0x00},    //  ;
    {0x00,  0x08,  0x14,  0x14,  0x22,  0x00},    //  <
    {0x00,  0x14,  0x14,  0x14,  0x14,  0x00},    //  =
    {0x00,  0x22,  0x14,  0x14,  0x08,  0x00},    //  >
    {0x00,  0x02,  0xb1,  0x09,  0x06,  0x00},    //  ?
    {0x3c,  0x42,  0x99,  0xa5,  0x99,  0x9e},    //  @
    {0x60,  0x1c,  0x13,  0x13,  0x1c,  0x60},    //  A
    {0xff,  0x89,  0x89,  0x89,  0x76,  0x00},    //  B
    {0x3c,  0x42,  0x81,  0x81,  0x42,  0x00},    //  C
    {0xff,  0x81,  0x81,  0x42,  0x3c,  0x00},    //  D
    {0xff,  0x89,  0x89,  0x89,  0x89,  0x00},    //  E
    {0xff,  0x09,  0x09,  0x09,  0x01,  0x00},    //  F
    {0x3c,  0x42,  0x81,  0x91,  0x91,  0x76},    //  G
    {0xff,  0x08,  0x08,  0x08,  0xff,  0x00},    //  H
    {0x00,  0x00,  0xff,  0x00,  0x00,  0x00},    //  I
    {0x00,  0x40,  0x80,  0x80,  0x7f,  0x00},    //  J
    {0xff,  0x08,  0x14,  0x22,  0x41,  0x80},    //  K
    {0x00,  0xff,  0x80,  0x80,  0x80,  0x00},    //  L
    {0xff,  0x1e,  0xe0,  0xe0,  0x1e,  0xff},    //  M
    {0xff,  0x06,  0x18,  0x60,  0xff,  0x00},    //  N
    {0x3c,  0x42,  0x81,  0x81,  0x42,  0x3c},    //  O
    {0xff,  0x11,  0x11,  0x11,  0x0e,  0x00},    //  P
    {0x3c,  0x42,  0x81,  0xa1,  0x42,  0xbc},    //  Q
    {0xff,  0x11,  0x11,  0x31,  0x4e,  0x80},    //  R
    {0x46,  0x89,  0x89,  0x91,  0x62,  0x00},    //  S
    {0x00,  0x01,  0x01,  0xff,  0x01,  0x00},    //  T
    {0x7f,  0x80,  0x80,  0x80,  0x7f,  0x00},    //  U
    {0x07,  0x38,  0xc0,  0xc0,  0x38,  0x07},    //  V
    {0xff,  0xe0,  0x1f,  0x1f,  0xe0,  0xff},    //  W
    {0xc3,  0x24,  0x18,  0x18,  0x24,  0xc3},    //  X
    {0x03,  0x04,  0xf8,  0x04,  0x03,  0x00},    //  Y
    {0xc1,  0xa1,  0x91,  0x89,  0x85,  0x83},    //  Z
    {0x00,  0x00,  0xff,  0x81,  0x00,  0x00},    //  [
    {0x00,  0x07,  0x38,  0xc0,  0x00,  0x00},    //  '\'
    {0x00,  0x81,  0xff,  0x00,  0x00,  0x00},    //  ]
    {0x08,  0x04,  0x02,  0x04,  0x08,  0x00},    //  ^
    {0x80,  0x80,  0x80,  0x80,  0x80,  0x80},    //  _
    {0x00,  0x01,  0x03,  0x02,  0x00,  0x00},    //  `
    {0x00,  0x68,  0x94,  0x94,  0xfc,  0x00},    //  a
    {0x00,  0xff,  0x84,  0x84,  0x78,  0x00},    //  b
    {0x00,  0x78,  0x84,  0x84,  0x48,  0x00},    //  c
    {0x00,  0x78,  0x84,  0x84,  0xff,  0x00},    //  d
    {0x00,  0x78,  0x94,  0x94,  0x48,  0x00},    //  e
    {0x00,  0x00,  0x04,  0xfe,  0x05,  0x05},    //  f
    {0x00,  0x9e,  0xa1,  0xa1,  0x7e,  0x00},    //  g
    {0x00,  0xff,  0x04,  0x04,  0xf8,  0x00},    //  h
    {0x00,  0x00,  0xfd,  0x00,  0x00,  0x00},    //  i
    {0x00,  0x80,  0x7d,  0x00,  0x00,  0x00},    //  j
    {0x00,  0xff,  0x10,  0x28,  0xc4,  0x00},    //  k
    {0x00,  0x00,  0xff,  0x00,  0x00,  0x00},    //  l
    {0xf8,  0x04,  0xf8,  0x04,  0xf8,  0x00},    //  m
    {0x00,  0xf8,  0x04,  0x04,  0xf8,  0x00},    //  n
    {0x00,  0x78,  0x84,  0x84,  0x78,  0x00},    //  o
    {0x00,  0xfe,  0x21,  0x21,  0x1e,  0x00},    //  p
    {0x00,  0x1e,  0x21,  0x21,  0xfe,  0x00},    //  q
    {0x00,  0xf8,  0x04,  0x04,  0x00,  0x00},    //  r
    {0x00,  0x48,  0x94,  0xa4,  0x48,  0x00},    //  s
    {0x00,  0x04,  0x7f,  0x84,  0x00,  0x00},    //  t
    {0x00,  0x7c,  0x80,  0x80,  0x7c,  0x00},    //  u
    {0x00,  0x1c,  0xe0,  0x1c,  0x00,  0x00},    //  v
    {0x1c,  0xe0,  0x1c,  0xe0,  0x1c,  0x00},    //  w
    {0x00,  0x64,  0x18,  0x18,  0x64,  0x00},    //  x
    {0x00,  0x86,  0x48,  0x30,  0x0e,  0x00},    //  y
    {0x00,  0x88,  0xc8,  0xa8,  0x98,  0x00},    //  z
    {0x00,  0x08,  0x76,  0x81,  0x81,  0x00},    //  {
    {0x00,  0x00,  0xff,  0x00,  0x00,  0x00},    //  |
    {0x00,  0x81,  0x81,  0x76,  0x08,  0x00},    //  }
    {0x08,  0x04,  0x04,  0x08,  0x08,  0x04}     //  ~
};
#pragma arm section rodata
#if (USE_COLOR_LCD)
#define AUDDEC_CHAR_COUNT     10
#endif

/******************************************************************************
* @NAME    FontLib_Init
* @BRIEF   init fontlib global variable
* @AUTHOR  xuping
* @DATE    2008-04-07
* @PARAM 
*		    T_VOID
* @RETURN T_VOID
*	
*******************************************************************************/
#pragma arm section code = "_sysinit_"
T_BOOL FontLib_Init(T_VOID)
{      
#ifdef OS_WIN32
    T_U8 i = 0;
    
    gb_font.blockBit = 18;//256k
    gb_font.pageBit  = 11;  //2k
    for (i = 0; i < BLOCK_USED; i++)
    {
        gb_font.font2block[i] = i;
    }
#else
    
    gb_font.getMatrixFun = AK_NULL;

    progmanage_StorageGetInfo(&gb_font.pageBit, &gb_font.blockBit);


    if(!progmanage_get_maplist(FONT_LIB_NAME, gb_font.font2block, BLOCK_USED))
    {
        return AK_FALSE;
    }
#endif
    gb_font.fontGet.commbuf_Manager = 0;
    gb_font.fontGet.record_LastFontOffset = ~0;
    gb_font.fontGet.record_LastRegion = ~0;
    gb_font.fontGet.bBufAva = AK_FALSE;
    if(NULL == FontLib_CommBuf_Malloc(FONTLIB_COMMBUF))
    {
        return AK_FALSE;
    }
    if(AK_FALSE == FontLib_ReadFile(0, FontLib_Get_CommBuf(FONTLIB_COMMBUF)))
    {
        return AK_FALSE;
    }
    memcpy(&gb_font.fontLibInfo, FontLib_Get_CommBuf(FONTLIB_COMMBUF), sizeof(T_FONT_LIB_INFO));
    FontLib_CommBuf_Free(FONTLIB_COMMBUF);
    return AK_TRUE;
}
#pragma arm section code

/******************************************************************************
* @NAME    Eng_FontRsgtFun
* @BRIEF   register callback function
* @AUTHOR  xuping
* @DATE    2008-04-07
* @PARAM 
*		    callbackFun: callback fun to register
* @RETURN T_VOID
*	
*******************************************************************************/
T_VOID Eng_FontRsgtFun(FONT_CALLBACK_GETMTRIX callbackFun)
{
    gb_font.getMatrixFun = callbackFun;     
}

#pragma arm section code = "_fontdisplay_"
/******************************************************************************
* @NAME    GetFontMatrix
* @BRIEF   Get FontMatrix  and abc of the unicode char
* @AUTHOR  xuping
* @DATE    2008-04-07
* @PARAM 
*		    char: unicode char
*          FontMatrix: store the font bitmap
*          fontAbc:store abc information          
* @RETURN 
*      T_BOOL :AK_TRUE,get success;AK_FLASE,get failed
*	
*******************************************************************************/
T_BOOL GetFontMatrix(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format)
{
    T_U16 offset    = 0;
    T_U8 *pBuff;
    
    AK_ASSERT_PTR(FontMatrix, "GetFontMatrix", 0);
    AK_ASSERT_PTR(fontABC, "GetFontMatrix", 0);
   
    
    if(AK_NULL != gb_font.getMatrixFun)
    {
        if (gb_font.getMatrixFun(ch,FontMatrix,fontABC,format))
        {
            return AK_TRUE;
        }
    }

    offset = GetFontMatrix_FromFontLib(ch, FontMatrix);

    if((T_U16)INVALID_OFFSET == offset)
    {
        GetDefaultFontMatrix(FontMatrix);    
        offset = 0;
    }

    //offset = ((T_U16)ch%CHAR_NUM_IN_512BYTE)* (sizeof(T_FONT_ABC) + FONT_BYTE_COUNT);
    pBuff = FontMatrix + offset;
    
    *fontABC = *(T_FONT_ABC *)(pBuff);
    
    for (offset = 0; offset < FONT_BYTE_COUNT; offset++)
    {
        *(FontMatrix + offset) = *(pBuff + offset + sizeof(T_FONT_ABC));
#if(FONT_TYPE == 16)
        if (offset >= FONT_WIDTH && format)
        {
            //strikethrough
            if (format & FORMAT_STRIKETHROUGH)
                *(FontMatrix + offset) |= 1;
            //under line
            if (format & FORMAT_UNDERLINE)
                *(FontMatrix + offset) |= (1<<7);
        }
        //top line
        if (offset < FONT_WIDTH && (format & FORMAT_TOPLINE))
            *(FontMatrix + offset) |= 1;      
#endif
#if(FONT_TYPE == 12)
        if (offset >= FONT_WIDTH && format)
        {
            //under line
            if (format & FORMAT_UNDERLINE)
                *(FontMatrix + offset) |= ((1<<7) | (1<<3));
        }
        else
        {
            //strikethrough
            if (format & FORMAT_STRIKETHROUGH)
                *(FontMatrix + offset) |= (1<<6);
            //top line
            if (format & FORMAT_TOPLINE)
                *(FontMatrix + offset) |= 1;                 
        }
#endif
#if(FONT_TYPE == 24)
        if ((offset >= (FONT_BYTE_COUNT-FONT_WIDTH))  && format)
        {
            //strikethrough
            if (format & FORMAT_STRIKETHROUGH)
                *(FontMatrix + offset) |= 1;
            //under line
            if (format & FORMAT_UNDERLINE)
                *(FontMatrix + offset) |= (1<<7);
        }
        //top line
        if (offset < FONT_WIDTH && (format & FORMAT_TOPLINE))
            *(FontMatrix + offset) |= 1;    
#endif
        
    }
    
    return AK_TRUE;
}

/******************************************************************************
* @NAME    DrawChar
* @BRIEF   draw a char to lcd
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-15
* @PARAM 
*          pBG: background buffer, if NULL, mean no background
* @RETURN char width of A+BC+C
*******************************************************************************/
T_S16 DrawChar(T_POS x, T_POS y, T_U16 ch,T_U16 Width,FDRAW_PARA *pDPara,T_U16 dispedoffset,T_U16 *pscl, T_BOOL showEnd)
{
    T_U8  *DispBuf = AK_NULL;
	T_U16 i,  fontwidth;    
#if (USE_COLOR_LCD)
    T_U8 *pBG       = AK_NULL;
    T_U8 *MatrixBuf = AK_NULL;
    T_U8 data;
#else
    T_U16 data;
#endif
    T_POS    row,col;
    
    T_FONT_ABC  fontABC;
    T_U16    ColEnd = FONT_WIDTH;

    AK_ASSERT_PTR(FontLib_Get_CommBuf(FONTLIB_COMMBUF), "commbuf error while draw char!", -1)
    DispBuf = FontLib_Get_CommBuf(FONTLIB_COMMBUF);
#if (USE_COLOR_LCD)
    MatrixBuf = (T_U8 *)(DispBuf + 1024);//store font matrix
    DispBuf += 512;     //use the last half buffer    
#endif
    if (GetFontMatrix(ch, DispBuf, &fontABC, pDPara->format))//获得字模信息
    {
        //at limitation of lcd width or specified right       
        if ((pDPara->format & FOTMAT_FIRSTCHAR)  && (fontABC.abcA < 0) )
        {
            fontABC.abcA = 0;
        }
        ColEnd += fontABC.abcA;
        fontwidth = fontABC.abcA + fontABC.abcB + fontABC.abcC;
        //that wilder than LCD width
        if (showEnd)
        {
            T_S16 temp = ColEnd;
            if ((FONT_WND_WIDTH < x + fontABC.abcA + FONT_WIDTH) &&\
                (Width < dispedoffset + fontABC.abcA + FONT_WIDTH))
            {
                temp = (FONT_WND_WIDTH - x - fontABC.abcA) > (Width - dispedoffset - fontABC.abcA) ?\
                    (Width - dispedoffset - fontABC.abcA) : (FONT_WND_WIDTH - x - fontABC.abcA);
            }
            else if (FONT_WND_WIDTH < x + fontABC.abcA + FONT_WIDTH)
            {
                temp = FONT_WND_WIDTH - x - fontABC.abcA;
            }
            else if (Width < dispedoffset + fontABC.abcA + FONT_WIDTH)
            {
                temp = Width - dispedoffset - fontABC.abcA;
            }
            else
            {
                showEnd = AK_FALSE;
            }
            
            if (0 >= temp)
            {
				//LoadMem_CommBuffFree();
                return -1;
            }
            ColEnd = temp;
        }
        else if (((FONT_WND_WIDTH < x + fontABC.abcA + FONT_WIDTH) ||\
            (Width < dispedoffset + fontABC.abcA + FONT_WIDTH)))
        {
            if (((FONT_WND_WIDTH < x + fontwidth) ||\
                (Width < dispedoffset + fontwidth)))
            {
				//LoadMem_CommBuffFree();
                return -1;
            }
            else
            {
                ColEnd  = fontwidth;
            }
            
        }        
        
        if (*pscl >= fontwidth)
        {
            *pscl = *pscl - fontwidth;
			//LoadMem_CommBuffFree();
            return 0;
        }
        else if (*pscl > 0)
        {
            ColEnd -= fontABC.abcA;
            fontABC.abcA = 0;
        }
        //显示一个字符  
#if (USE_COLOR_LCD)
        memcpy(MatrixBuf,DispBuf,FONT_BYTE_COUNT);
        //刷新中间区域
        if (0 < fontABC.abcA)
        {
            col = ColEnd > fontABC.abcA ? fontABC.abcA : ColEnd;
            ColEnd -= col;
            pBG = GetBG(pDPara, dispedoffset, col);
            if (pBG)
            {
                Fwl_RefreshRect(x, y, col, FONT_HEIGHT, pBG, AK_FALSE);
            }
            if (0 == ColEnd)
            {
				//LoadMem_CommBuffFree();
                return -1;
            }
        }
        pBG = GetBG(pDPara, (T_U16)(dispedoffset+fontABC.abcA), ColEnd);
        if (pBG)
        {  
#if(FONT_TYPE == 16)      
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {
                for(col = *pscl; col< ColEnd; col++)                
                {
                    data = *(MatrixBuf+(row/8)*FONT_WIDTH+col);
                    for(i=0; i<8; i++)
                    {
                        if (data & (1<<i))
                        {
                            *((T_U16 *)(pBG+2*((row+i)*ColEnd+col - *pscl))) = pDPara->color;
                        }
                    }
                }
            }
#endif
#if(FONT_TYPE == 12)      
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {
                if (0 == row)
                {
                    for(col = *pscl; col< ColEnd; col++)                
                    {
                        data = *(MatrixBuf+col);
                        for(i=0; i<8; i++)
                        {
                            if (data & (1<<i))
                            {
                                *((T_U16 *)(pBG+2*((i)*ColEnd+col - *pscl))) = pDPara->color;
                            }
                        }
                    }
                }
                else
                {
                    for(col = *pscl; col< ColEnd; col++)                
                    {
                        data = *(MatrixBuf+(row/8)*FONT_WIDTH+col/2);
                        for(i=4*(col&0x1); i<4*((col&0x1) +1); i++)
                        {
                            if (data & (1<<i))
                            {
                                *((T_U16 *)(pBG+2*((row+i%4)*ColEnd+col - *pscl))) = pDPara->color;
                            }
                        }
                    }               
                }
            }
#endif
#if(FONT_TYPE == 24)      
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {
                for(col = *pscl; col< ColEnd; col++)                
                {
                    data = *(MatrixBuf+(row/8)*FONT_WIDTH+col);
                    for(i=0; i<8; i++)
                    {
                        if (data & (1<<i))
                        {
                            *((T_U16 *)(pBG+2*((row+i)*ColEnd+col - *pscl))) = pDPara->color;
                        }
                    }
                }
            }
#endif
            Fwl_RefreshRect((T_POS)(x+fontABC.abcA), y, ColEnd, FONT_HEIGHT, pBG, AK_FALSE);
        }
        else
        {
#if(FONT_TYPE == 16)         
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {          
                for(col=*pscl; col< ColEnd; col++)                
                {
                    data = *(DispBuf+(row/8)*FONT_WIDTH+col);
                    for(i=0; i<8; i++)
                    {
                        if (data & (1<<i))
                        {
                            Fwl_SetPixel((T_POS)(x+col+fontABC.abcA - *pscl), (T_POS)(y+row+i), pDPara->color);
                        }
                    }
                }
            }
#endif
#if(FONT_TYPE == 12)  
            for(row = 0; row < FONT_HEIGHT; row += 8)
            { 
                if (0 == row)
                {
                    for(col=*pscl; col< ColEnd; col++)                
                    {
                        data = *(DispBuf+col);
                        for(i=0; i<8; i++)
                        {
                            if (data & (1<<i))
                            {
                                Fwl_SetPixel((T_POS)(x+col+fontABC.abcA - *pscl), (T_POS)(y+i), pDPara->color);
                            }
                        }
                    }
                }
                else
                {
                    for(col=*pscl; col< ColEnd; col++)                
                    {
                        data = *(DispBuf+(row/8)*FONT_WIDTH+col/2);
                        for(i=4*(col&0x1); i<4*((col&0x1) +1); i++)
                        {
                            if (data & (1<<i))
                            {
                                Fwl_SetPixel((T_POS)(x+col+fontABC.abcA - *pscl), (T_POS)(y+row+i%4), pDPara->color);
                            }
                        }
                    }
                    
                }
            }  
#endif
#if(FONT_TYPE == 24)     
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {          
                for(col=*pscl; col< ColEnd; col++)                
                {
                    data = *(DispBuf+(row/8)*FONT_WIDTH+col);
                    for(i=0; i<8; i++)
                    {
                        if (data & (1<<i))
                        {
                            Fwl_SetPixel((T_POS)(x+col+fontABC.abcA - *pscl), (T_POS)(y+row+i), pDPara->color);
                        }
                    }
                }
            }
#endif
        }
#else
        //刷新字中间区域
        if (0 < fontABC.abcA)
        {
            col = ColEnd > fontABC.abcA ? fontABC.abcA : ColEnd;
            ColEnd -= col;
            for(i=0; i<col; i++)
            {
                data = 0;
                if (pDPara->format & FORMAT_STRIKETHROUGH)
                    data |= (1<<(FONT_HEIGHT/2));
                if (pDPara->format & FORMAT_UNDERLINE)
                    data |= (1<<(FONT_HEIGHT-1));
                if (pDPara->format & FORMAT_TOPLINE)
                    data |= 1;
                Fwl_RefreshRect((T_U16)(x+i), (T_U16)y, 1, FONT_HEIGHT, (T_U8 *)(&data), (T_U8)(!pDPara->color));
            }
            if (0 == ColEnd)
            {
				//LoadMem_CommBuffFree();
                return -1;
            }
        }		
        //显示字体
        if (*pscl > 0 && *pscl < fontwidth)//显示字体的一部分
        {
            T_U8 dispwidth = FONT_WIDTH - *pscl;
            T_U8 i = 0;
            T_U8 fontbuf[FONT_BYTE_COUNT] = {0};
            
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {
                for(col = *pscl; col < FONT_WIDTH ; col++)                
                {
                    fontbuf[i] = *(DispBuf +(row/8)*FONT_WIDTH+col);
                    i++;
                }
            } 
            Fwl_RefreshRect((T_POS)(x+fontABC.abcA), y, dispwidth, FONT_HEIGHT, fontbuf, (T_U8)(!pDPara->color));
        }
        else if (ColEnd < FONT_WIDTH)
        {
            T_U8 dispwidth = ColEnd - *pscl;
            T_U8 i = 0;
            T_U8 fontbuf[FONT_BYTE_COUNT] = {0};
            
            for(row = 0; row < FONT_HEIGHT; row += 8)
            {
                for(col = *pscl; col < ColEnd ; col++)                
                {
                    fontbuf[i] = *(DispBuf +(row/8)*FONT_WIDTH+col);
                    i++;
                }
            } 
            Fwl_RefreshRect((T_POS)(x+fontABC.abcA), y, dispwidth, FONT_HEIGHT, fontbuf, (T_U8)(!pDPara->color));		    
        }
        else
        {
            Fwl_RefreshRect((T_POS)(x+fontABC.abcA), y, FONT_WIDTH, FONT_HEIGHT, DispBuf, (T_U8)(!pDPara->color));
        }
#endif
        if (ColEnd < fontwidth && showEnd)//the end
        {
			//LoadMem_CommBuffFree();
            return -1;
        }
        if (*pscl > 0)
        {
            fontwidth -= *pscl;
            *pscl = 0;
        }
		//LoadMem_CommBuffFree();
        return fontwidth;
    }
	//LoadMem_CommBuffFree();
    return 0;
}

/******************************************************************************
* @NAME    GetBG
* @BRIEF   get the background buffer for a char
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-17
* @PARAM 
*		    bgtype: background type
*          bgpara: background parameter
* @RETURN the backgournd buffer
*	
*******************************************************************************/
T_U8 *GetBG(FDRAW_PARA *pDPara, T_POS offset, T_U16 width)
{
    T_U8* ret = AK_NULL;
#if (USE_COLOR_LCD)
    int i;
    T_U16 *pBuff;
    T_BG_PIC *bgPic = AK_NULL;
    
    switch(pDPara->bgtype)
    {
    case BG_COLOR:
        //ret = Gbl_GetCommBuff();  
		//ret = LoadMem_GetCommBuff();//不用释放
	 ret = FontLib_Get_CommBuf(FONTLIB_COMMBUF);
	AK_ASSERT_PTR(ret, "haven't malloc font lib commbuf1", NULL);
        pBuff = (T_U16 *)ret;
        for(i=0; i<width*FONT_HEIGHT; i++,pBuff++)
        {
            if (pDPara->format)
            {
                if (((pDPara->format & FORMAT_STRIKETHROUGH) && width*(FONT_HEIGHT/2) <= i && i<width*(FONT_HEIGHT/2+1))
                    || ((pDPara->format & FORMAT_UNDERLINE) && width*(FONT_HEIGHT-1) <= i)
                    || ((pDPara->format & FORMAT_TOPLINE) && i < width))
                {
                    *pBuff = pDPara->color;
                    continue;
                }
                
            }
            *pBuff = (T_U16)pDPara->bgpara;
        }
        break;
    case BG_PICTURE: 
        //ret = Gbl_GetCommBuff();  
	//	ret = LoadMem_GetCommBuff();
	 ret = FontLib_Get_CommBuf(FONTLIB_COMMBUF);
	 AK_ASSERT_PTR(ret, "haven't malloc font lib commbuf2", NULL);
        bgPic = (T_BG_PIC *)pDPara->bgpara;
        Eng_ImageResGetData(bgPic->resId,width, FONT_HEIGHT,(T_U16)(bgPic->hOffset + offset),bgPic->vOffset,ret);
        pBuff = (T_U16 *)ret;
        if (pDPara->format)
        {
            for(i=0; i<width*FONT_HEIGHT; i++,pBuff++)
            {     
                if (((pDPara->format & FORMAT_STRIKETHROUGH) && width*(FONT_HEIGHT/2) <= i && i<width*(FONT_HEIGHT/2+1))
                    || ((pDPara->format & FORMAT_UNDERLINE) && width*(FONT_HEIGHT-1) <= i)
                    || ((pDPara->format & FORMAT_TOPLINE) && i < width))
                {
                    *pBuff = pDPara->color;
                }
            }
        }
        break;
    default:
        break;
    }
#endif
    return ret;
}

/******************************************************************************
* @NAME    DispResString
* @BRIEF   disp resource string
* @AUTHOR  xuping
* @DATE    2008-04-07
* @PARAM 
*		    x:appoint the position
*          y:appoint the position
*          ResID:index of resource string
*          reverse:>0 reverse 
* @RETURN T_VOID
*	
*******************************************************************************/
T_POS DispResString(T_U32 x, T_U32 y, T_RES_STRING  ResID,T_U16 Width, FDRAW_PARA *pDPara)
{
    T_U16           offset;    
    T_U16			xPos = PARA_EXTRACTL(x);
    T_U16           yPos = PARA_EXTRACTL(y);
    T_S16			scl  = PARA_EXTRACTH(x);//高16位是显示位置相对于矩形的偏移
    T_S16			tempscl = scl;
    T_RES_STRING    dispResId = ResID;
    T_U16           ch;
    T_S16           chwidth;
    T_U16           *disp_string; 
    T_BOOL          bShowEnd = (T_BOOL)PARA_EXTRACTB(x);
    T_U16            tempPos[2] = {0};
    T_U16            orignalPos[2] = {0};
    FDRAW_PARA     temppara;
#if (USE_COLOR_LCD)
    T_U8        charCnt  = 0;
#endif 
    
    if (0 < scl) //
    {
        xPos += scl;
        scl  =  0;
    }
    else if (0 > scl)
    {
        scl = 0 - scl;
    }
    
    disp_string = GetResStringPtr(dispResId);
    
    if(NULL == FontLib_CommBuf_Malloc(FONTLIB_COMMBUF))
    {
        return 0;
    }
    
    for (offset = 0; *(disp_string+offset) > 0; )
    {
        
        ch = *(disp_string+offset);
        if (0x0D == ch || 0x0A == ch)//回车，换行符,超出范围
        {
            break;
        }  
        else if (MORE_RES_STRING_FLAG  == ch)
        {
            dispResId++;
            disp_string = GetResStringPtr(dispResId);
            offset = 0;
            continue;
        }
        
        if (xPos == (T_U16)(x & 0xff) )//first char to show 
        {
            pDPara->format |= FOTMAT_FIRSTCHAR;
        }
        if(ch <= 0x0e5b && ch >= 0x0e00)
        {
            memcpy(&temppara,pDPara,sizeof(FDRAW_PARA));
            orignalPos[0] = xPos;
            if(AK_FALSE == Tran_Thai_FONT(ch, tempPos, orignalPos)) 
            {
                offset++;
                continue; 
            }
            if (xPos  != orignalPos[0])
            {
                temppara.bgtype = BG_NONE;
            }
            xPos =  orignalPos[0]; 
            if(Is_Thai_FirstClass(ch) || Is_Thai_SecondClass(ch))
                scl = tempscl;
            else
                tempscl = scl;
            chwidth = DrawChar(xPos, yPos, ch,Width,&temppara, (T_U16)(xPos - PARA_EXTRACTL(x)),(T_U16 *)&scl,bShowEnd);
        }
        else
        chwidth = DrawChar(xPos, yPos, ch,Width,pDPara, (T_U16)(xPos - PARA_EXTRACTL(x)),(T_U16 *)&scl,bShowEnd);
        //decode audio
#if (USE_COLOR_LCD)
        if (charCnt >= AUDDEC_CHAR_COUNT)
        {
            charCnt = 0;
        }
        else
        {
            charCnt++;
        }
#endif
        
        if (chwidth < 0)
            break;
        
        xPos += chwidth;
        offset++;
    }  

    FontLib_CommBuf_Free(FONTLIB_COMMBUF);
    
    return 2*offset;
}

/******************************************************************************
* @NAME    DispString
* @BRIEF   display string in width
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-17
* @PARAM 
*          codepage:the codepage of the str 
*		    x:高16位为相对于矩形区域的位置，低8位为矩形相对于LCD的坐标，次八位为是否显示末尾部分像素
*          y:高16位为相对于矩形区域的未知，低16位为矩形相对于LCD的坐标
*          pStr:the pointer to the string
*          strLen :the length of the string for show
*          width: the width to show unicode string
*          color: the font color          
* @RETURN disp length
*******************************************************************************/
T_U16  DispString(T_CODE_PAGE codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color, T_U32 format, eBG_TYPE bgtype, T_U32 bgpara)
{
    T_U16       offset;
    T_S16        chwidth = 0;  
    T_U16        xPos = PARA_EXTRACTL(x);
    T_U16        yPos = PARA_EXTRACTL(y);
    T_S16       scl  = PARA_EXTRACTH(x);//高16位是显示位置相对于矩形的偏移
    T_S16       tempscl = scl;
    T_U16       ch[2] = {0};
    T_U32       ConvertedNum = 0, cp;
    T_U16       ret;
    FDRAW_PARA  dPara;
    T_U16            tempPos[2] = {0};
    T_U16            orignalPos[2] = {0};
    FDRAW_PARA     temppara;
#if (USE_COLOR_LCD)
    T_U8        charCnt  = 0;
#endif 
    T_BOOL      bShowEnd = (T_BOOL)PARA_EXTRACTB(x);
    
    if (0 < scl) //
    {
        xPos += scl;
        scl  =  0;
    }
    else if (0 > scl)
    {
        scl = 0 - scl;
    }
    
    
    Fwl_FreqPush(FREQ_APP_MAX);
    dPara.bgpara = bgpara;
    dPara.bgtype = bgtype;
    dPara.color  = color;
    dPara.format = format;

	if ((T_U32)pStr < STR_SEPARATOR)
    {
        ret = DispResString(x, y, (T_U32)pStr, Width,&dPara);
        Fwl_FreqPop();
        return ret;
    }
    cp = codepage;
    AK_ASSERT_PTR(pStr, "UDispStringInWidth", 0);

    if(NULL == FontLib_CommBuf_Malloc(FONTLIB_COMMBUF))
    {
        return 0;
    }
    for (offset = 0; AK_TRUE;  )
    {
        //process the special char
        if (cp == CP_UNICODE)
        {
            ch[0] = *((T_U16 *)(pStr + offset));
            if (ch[0] ==0 || ch[0] == 0x0D || ch[0] == 0x0A)
                break;
            
            ConvertedNum = 2;
        }
        else
        {
            chwidth = *(pStr + offset);
            if (chwidth ==0 || chwidth == 0x0D || chwidth == 0x0A)
                break;
            
            Eng_MultiByteToWideChar_Ex(cp, &ConvertedNum,pStr + offset, 
                Utl_StrLen((T_CHR*)(pStr + offset)), (T_U16 *)ch, 2,AK_NULL);
        }            
        
        if (0 == ConvertedNum)
        {
            AK_DEBUG_OUTPUT("converted string error\r\n");
            break;
        }
        
        if (0x09 == ch[0])//tab键显示为空格
        {
            ch[0] = 0x20;
        }
        
        if (xPos == PARA_EXTRACTL(x))//first char to show 
        {
            dPara.format |= FOTMAT_FIRSTCHAR;
        } 
        if(ch[0] <= 0x0e5b && ch[0] >= 0x0e00)
        {
            temppara = dPara;
            orignalPos[0] = xPos;
            if(AK_FALSE == Tran_Thai_FONT(ch[0], tempPos, orignalPos)) 
            {
                offset += (T_U16)ConvertedNum;
                continue; 
            } 
            if (xPos !=  orignalPos[0])
            {
                temppara.bgtype=BG_NONE; 
            }
            xPos =  orignalPos[0];
            if(Is_Thai_FirstClass(ch[0]) || Is_Thai_SecondClass(ch[0]))
                scl = tempscl;
            else
                tempscl = scl;
            chwidth = DrawChar(xPos, yPos, ch[0], Width,&temppara, (T_U16)(xPos - PARA_EXTRACTL(x)),(T_U16 *)&scl,bShowEnd);
        }
        else
        chwidth = DrawChar(xPos, yPos, ch[0], Width,&dPara, (T_U16)(xPos - PARA_EXTRACTL(x)),(T_U16 *)&scl,bShowEnd);
        //decode audio
#if (USE_COLOR_LCD)
        if (charCnt >= AUDDEC_CHAR_COUNT)
        {
            charCnt = 0;
        }
        else
        {
            charCnt++;
        }
#endif 
        
        
        if (chwidth >= 0)
        {
            xPos += chwidth;
            offset += (T_U16)ConvertedNum;
        }
        else
        {
            //AK_DEBUG_OUTPUT("can't disp all string\r\n");
            break;
        }
    }
    FontLib_CommBuf_Free(FONTLIB_COMMBUF);
    
    Fwl_FreqPop();
    return offset; 
}



/******************************************************************************
* @NAME    DispStringInWidth
* @BRIEF   disp string in width
* @AUTHOR  xuping
* @DATE    2008-05-16
* @PARAM 
*          codepage:the codepage of the str 
*		    x:appoint the position
*          y:appoint the position
*          pStr:the pointer to the string
*          strLen :the length of the string for show
*          width: the width to show unicode string
*          reverse:>0 reverse          
* @RETURN disp length
*******************************************************************************/
T_U16  DispStringInWidth(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color)
{
    return DispString((T_CODE_PAGE)(codepage & 0xFF), x, y, pStr, Width, color, (T_U32)(codepage & ~(0xFF)), BG_NONE, 0);
}

/******************************************************************************
* @NAME    DispStringInWidth
* @BRIEF   disp string in a specified width and a specified color background
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-17
* @PARAM 
*          codepage:the codepage of the str 
*		    x:appoint the position
*          y:appoint the position
*          pStr:the pointer to the string
*          strLen :the length of the string for show
*          width: the width to show unicode string
*          color: the font color 
*          BgColor: the background color
* @RETURN disp length
*******************************************************************************/
T_U16  DispStringOnColor(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color, T_U16 BgColor)
{
    return DispString((T_CODE_PAGE)(codepage & 0xFF), x, y, pStr, Width, color, (T_U32)(codepage & ~(0xFF)), BG_COLOR, (T_U32)BgColor);
}

#if (USE_COLOR_LCD)
T_U16  DispStringOnPic(T_U32 codepage, T_U32 x, T_U32 y, T_U8* pStr, T_U16 Width,T_U16 color, T_BG_PIC *pGg)
{
    return DispString((T_CODE_PAGE)(codepage & 0xFF), x, y, pStr, Width, color, (T_U32)(codepage & ~(0xFF)), BG_PICTURE, (T_U32)pGg);
}
#endif

/******************************************************************************
* @NAME    get resource string width
* @BRIEF   get the resource string width for display, 
*          if width is 0, then return a number of the chars dsiplayed in a specified width
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-20
* @PARAM 
*          ResID: resource string id
* @RETURN disp width
*******************************************************************************/
T_U16 GetResStringWidth(T_RES_STRING  ResID, T_U16 width)
{
    T_U16           offset;    
    T_U16           xPos = 0;
    T_RES_STRING    dispResId = ResID;
    T_U16           ch;
    T_U8            chwidth;
    T_U16           *disp_string;
    T_FONT_ABC  fontABC;
    T_U8* ret = AK_NULL;

    if(NULL == (ret = FontLib_CommBuf_Malloc(FONTLIB_COMMBUF)))
    {
        return 0;
    }
	//ret = LoadMem_CommBuffMalloc(FONT_COMMONBUFFSIZE);
	if (AK_NULL == ret)
	{
		AK_DEBUG_OUTPUT("malloc common buff is null.\n\r");
		return 0;
	}
    disp_string = (T_U16 *)GetResStringPtr(dispResId);
    
    for (offset = 0; *(disp_string+offset) > 0; )
    {
        ch = *(disp_string+offset);
        if (MORE_RES_STRING_FLAG  == ch)
        {
            dispResId++;
            disp_string = GetResStringPtr(dispResId);
            offset = 0;
            continue;
        }
        
        if(ch <= 0x0e5b && ch >= 0x0e00)
        {
            if(Is_Thai_FirstClass(ch) || Is_Thai_SecondClass(ch))
            {
           	    offset++;
           	    continue;
            }
        }        
        
        //if (GetFontMatrix(ch, Gbl_GetCommBuff(), &fontABC, 0))//获得字模信息
        if (GetFontMatrix(ch, ret, &fontABC, 0))
        {
            
            if (0 == offset && dispResId == ResID)//first time to show the res str
            {
                fontABC.abcA =  (fontABC.abcA > 0) ? fontABC.abcA : 0;
            }
            chwidth = fontABC.abcA + fontABC.abcB + fontABC.abcC;
        }
        else
        {
            break;
        }
        xPos += chwidth;
        offset++;
    }

    FontLib_CommBuf_Free(FONTLIB_COMMBUF);
	
    if (width)
        return 2*offset;
    else
        return xPos;
}

/******************************************************************************
* @NAME    GetStringDispWidth
* @BRIEF   get the resource string width for display, 
*          if width is 0, then return a number of the chars dsiplayed in a specified width
* @AUTHOR  Justin.Zhao
* @DATE    2008-05-20
* @PARAM
*          codepage:the codepage of the str 
*          pStr: string pointer
*          width: specified width for display
* @RETURN disp width
*******************************************************************************/
T_U16  GetStringDispWidth(T_CODE_PAGE codepage,T_U8 *pStr, T_U16 width)
{
    T_U8        chwidth;  
    T_U16       offset;    
    T_U16       xPos = 0;
    T_U16       ch[2] = {0};
    T_U32        ConvertedNum = 0;
    T_FONT_ABC  fontABC;
    T_U8* ret = AK_NULL;

    if(NULL == (ret = FontLib_CommBuf_Malloc(FONTLIB_COMMBUF)))
    {
        return 0;
    }
	//ret = LoadMem_CommBuffMalloc(FONT_COMMONBUFFSIZE);
	if (AK_NULL == ret)
	{
		AK_DEBUG_OUTPUT("malloc common buff is null.\n\r");
		return 0;
	}
    if ((T_U32)pStr < STR_SEPARATOR)
    {
   		FontLib_CommBuf_Free(FONTLIB_COMMBUF);
        offset = GetResStringWidth((T_U32)pStr, width);
		return offset;
    }
    
    AK_ASSERT_PTR(pStr, "UDispStringInWidth", 0);
    for (offset = 0; AK_TRUE;  )
    {
        
        //process the special char
        if (CP_UNICODE == codepage)
        {
            ch[0] = *((T_U16 *)(pStr + offset));
            if (ch[0] ==0 || ch[0] == 0x0D || ch[0] == 0x0A)
                break;
            
            ConvertedNum = 2;
            memcpy(ch, pStr+offset, 2);
        }
        else
        {
            chwidth = *(pStr + offset);
            if (chwidth ==0 || chwidth == 0x0D || chwidth == 0x0A)
                break;
            
            Eng_MultiByteToWideChar_Ex(codepage, &ConvertedNum,pStr + offset, 
                Utl_StrLen((T_CHR*)(pStr + offset)), (T_U16 *)ch, 2,AK_NULL);
        }            
        
        if (0 == ConvertedNum)
        {
            AK_DEBUG_OUTPUT("converted string error\r\n");
            break;
        }
        
        if (0x09 == ch[0])//tab键显示为空格
        {
            ch[0] = 0x20;
        }
        
        if(ch[0] <= 0x0e5b && ch[0] >= 0x0e00)
        {
            if(Is_Thai_FirstClass(ch[0]) || Is_Thai_SecondClass(ch[0]))
           	{
                offset += (T_U8)ConvertedNum;
                continue;
           	}
        }
        
		//if (GetFontMatrix(ch[0], Gbl_GetCommBuff(), &fontABC, 0))//获得字模信息
        if (GetFontMatrix(ch[0], ret, &fontABC, 0))
        {
            if (0 == offset)
            {
                
                fontABC.abcA =  (fontABC.abcA > 0) ? fontABC.abcA : 0;
            }
            chwidth = fontABC.abcA + fontABC.abcB + fontABC.abcC;
        }
        else
        {
            break;
        }
        if (width && (xPos + chwidth) >  width)
        {
            break;
        }
        xPos += chwidth;
        offset += (T_U8)ConvertedNum;
    } 
    
	FontLib_CommBuf_Free(FONTLIB_COMMBUF);
    if (width)
        return offset;
    else
        return xPos; 
}
#pragma arm section code

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
T_U16 GetResString(T_RES_STRING ID,T_U16 buf[], T_U16 bufLen)
{
    T_U8 offset      = 0;
    T_U16  ch;
    T_U16  Count     = 0;
    T_U16           *disp_string; 
    
    disp_string = (T_U16 *)GetResStringPtr(ID);
    for (offset = 0; *(disp_string+offset) > 0 && Count < bufLen; )
    {
        
        ch = *(disp_string+offset);
        if (MORE_RES_STRING_FLAG  == ch)
        {
            ID++;
            disp_string = (T_U16 *)GetResStringPtr(ID);
            offset = 0;
            continue;
        }
        buf[Count] = ch;
        Count++;
        offset++;
    }   
    
    return Count;
    
}


/******************************************************************************
* @NAME    GetResCount
* @BRIEF   get string resource ID between two ID.
* @AUTHOR  xuping
* @DATE    2008-04-14
* @PARAM 
*          BeginID: the begin id of str res
*          EndID:the end id of str res
*          language: the language of resourse to show
* @RETURN > 0 ResID Num
*******************************************************************************/
#pragma arm section code = "_audioplayer_menu_"
T_U16 GetResCount(T_RES_STRING BeginID,T_RES_STRING EndID,T_RES_LANGUAGE language)
{
    
    T_RES_STRING  i	= 0;
    T_U16  Count    = 0;
    
    
    AK_ASSERT_VAL((BeginID < GetResStrNum()), "GetResCount()", 0);
    AK_ASSERT_VAL((EndID < GetResStrNum()), "GetResCount()", 0);
    
    for (i = BeginID; i <= EndID; i++ )
    {
        if ( BeginID == i)
        {
            Count++;
        }
        else if((0x00 == GetResStringPtr(i - 1)[GetResStrMaxLen()])&&(0x00 != GetResStringPtr(i)[0]))
        {
            Count++;
        }
        
    }
    return Count;
    
}

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
T_RES_STRING  GetNearbyValidStrResID(T_RES_STRING StrResID,T_RES_LANGUAGE language,T_S16 offset)
{
    T_S8            i           = 0;
    T_U16           TempOffset  = 0;
    T_RES_STRING    tempID      = StrResID;
    
    AK_ASSERT_VAL((StrResID < GetResStrNum()), "GetResCount()", 0);
    
    TempOffset =  (offset >= 0) ? offset : (0 - offset);
    i          =  (offset >= 0) ? 1 : -1;    
    
    while(TempOffset--)
    {
        tempID += i;
        if (tempID >= GetResStrNum() )
        {
            break;
        }
        while(1)
        {
            if((0x00 == GetResStringPtr(tempID - i)[GetResStrMaxLen()])&&(0x00 != GetResStringPtr(tempID)[0]))
            {
                break;
            }
            else
            {
                tempID += i;
                if (tempID >= GetResStrNum() - 1 )
                {
                    break;
                }
            }
        }    
    } 
    return tempID;    
}
#pragma arm section code


#pragma arm section code = "_fontdisplay_"

/******************************************************************************
* @NAME    UDispStringInWidth_S
* @BRIEF   disp  string using small font within width
* @AUTHOR  xuping
* @DATE    2008-04-26
* @PARAM 
*		    x:appoint the position
*          y:appoint the position
*          pStr:the pointer to thestring
*          width :the width to show
*          color:
*          format:str disp format:FORMAT_STRIKETHROUGH ,FORMAT_UNDERLINE,FORMAT_TOPLINE
* @RETURN disp length
*	
*******************************************************************************/
T_U16 DispStringInWidth_S(T_POS x, T_POS y, T_U8* pStr, T_U16 Width,T_U16 color, T_U16 format)
{
    T_U8        offset;
    T_POS       xPos        = x;
    T_U8        ch;
    T_U8        col;
    T_U8        dispbuf[SMALL_FONT_BYTE_COUNT] = {0};
#if (USE_COLOR_LCD)
    T_U8  i;
#endif
    
    AK_ASSERT_PTR(pStr, "UDispString", 0);
    
    Fwl_FreqPush(FREQ_APP_MAX);
    for (offset = 0; *(pStr + offset)>0; offset++)
    {
        ch = *(pStr + offset);
        if (0x0D == ch || 0x0A == ch)//回车，换行符,超出范围
        {
            break;
        } 
        if ((SMALL_FONT_BEGIN> ch || SMALL_FONT_END< ch))
        {
            continue;
        }
        if ((xPos + SMALL_FONT_WIDTH ) >  FONT_WND_WIDTH )//larger than lcd width
        {
            AK_DEBUG_OUTPUT("set char out of lcd\r\n");
            break;
        } 
        
        if ((xPos + SMALL_FONT_WIDTH - x ) > Width)//larger than lcd width
        {
            break;
        } 
        //显示一个字符
        memcpy(dispbuf ,Small_Font[ch - SMALL_FONT_BEGIN],SMALL_FONT_BYTE_COUNT);
#if (USE_COLOR_LCD)        
        for(col=0; col<SMALL_FONT_WIDTH; col++)                
        {
            if (format & FORMAT_STRIKETHROUGH)
                dispbuf[col] |= (1<<(SMALL_FONT_HEIGHT/2));
            if (format & FORMAT_UNDERLINE)
                dispbuf[col] |= (1<<(SMALL_FONT_HEIGHT-1));
            if (format & FORMAT_TOPLINE)
                dispbuf[col] |= 1;
            
            for(i=0; i<8; i++)
            {
                if (dispbuf[col] & (1<<i))
                {
                    Fwl_SetPixel((T_POS)(xPos+col), (T_POS)(y+i), color);
                }
            }
        }
#else
        for(col=0; col<SMALL_FONT_WIDTH; col++) 
        {
            if (format & FORMAT_STRIKETHROUGH)
                dispbuf[col] |= (1<<(SMALL_FONT_HEIGHT/2));
            if (format & FORMAT_UNDERLINE)
                dispbuf[col] |= (1<<(SMALL_FONT_HEIGHT-1));
            if (format & FORMAT_TOPLINE)
                dispbuf[col] |= 1;            
        }
        Fwl_RefreshRect(xPos, y, SMALL_FONT_WIDTH, SMALL_FONT_HEIGHT, (T_U8 *)dispbuf, (T_U8)(!color));
#endif
        xPos += SMALL_FONT_WIDTH ;
    }
    Fwl_FreqPop();
    return offset;
}
#pragma arm section code 

//the end of files
#if 0
/*******************************/
/*资源字符串的动态加载*/
/*加载对应语言的资源字符串到内存中去*/
/* bufsize必须为一个语言的实际大小，destbuf大小必须>= BufSize*/

/************************************/
static const T_U16 StrResFileName[] = {'A',':','/','S','T','R','R','E','S','.','B','I','N','\0'};
T_BOOL Eng_StrResLoad(T_U32 lang, T_U8* DestBuf, T_U32 BufSize)
{
    T_hFILE fd;
    
    fd = Fwl_FileOpen(StrResFileName, _FMODE_READ, _FMODE_READ);
    if (fd > 0)
    {
        Fwl_FileSeek(fd , lang* BufSize, _FSEEK_SET);
        if (BufSize == Fwl_FileRead(fd,DestBuf,BufSize))
        {
            Fwl_FileClose(fd);
            return AK_TRUE;
        }          
    }
    Fwl_FileClose(fd);
    return AK_FALSE;
    
    
}
#endif
#pragma arm section code = "_fontdisplay_"
/***********************************************************************************
*                          泰文语法规则
*
*  1. 在子音后选择第一类后不能再选择一类符号，可选择二、三类符号 
*  2. 在子音后选择第二类后不能再选择一、二类符号，可选择三类符号 
*  3. 在子音后选择第三类后，不能再选择一、二类符号，但可以继续选择第三类符号 
*  4. 无子音情况下能输入第三类符号，以独立的字符存在，不能再和一、二、三类中的叠加组合成新符 
*  5. 第三类中“0x0e24”，“0x0e26”比较特殊，和子音操作一样，可在后面选择第一、二类字符
*  6. 其他字符为独立字符，不能与其他字符合成
***********************************************************************************/

T_BOOL Is_Thai_Font(T_U16 ch)      //判断是否泰文合成字符
{
    if((ch >= 0x0e00) && (ch <=0x0e4f))
        return AK_TRUE;
    
    TCh_Attr &= (~(HAVE_THAI_CONS | HAVE_THAI_THIRDCLASS_SPEC | THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN)) | THAI_THIRDCLASS_EN;
    //		HAVE_THAI_CONS = AK_FALSE;	
    //		HAVE_THAI_THIRDCLASS_SPEC	= AK_FALSE;			
    //		THAI_FIRSTCLASS_EN = AK_FALSE;	
    //		THAI_SECONDCLASS_EN = AK_FALSE;	
    //		THAI_THIRDCLASS_EN = AK_TRUE;		
    
    return AK_FALSE;				
}

T_BOOL Is_Thai_Cons(T_U16 ch)     //泰文子音
{
    if((ch >= 0x0e00) && (ch <=0x0e2e) && (ch != 0x0e24) && (ch != 0x0e26))
    { 
        TCh_Attr |= HAVE_THAI_CONS | THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN | THAI_THIRDCLASS_EN & (~HAVE_THAI_THIRDCLASS_SPEC);
        //			HAVE_THAI_CONS = AK_TRUE;
        //			HAVE_THAI_THIRDCLASS_SPEC	= AK_FALSE;
        //			THAI_FIRSTCLASS_EN = AK_TRUE;
        //			THAI_SECONDCLASS_EN = AK_TRUE;
        //			THAI_THIRDCLASS_EN = AK_TRUE;		
        return AK_TRUE;
    }
    return AK_FALSE;		
}

T_BOOL Is_Thai_FirstClass(T_U16 ch)  //一类符号
{
    if((ch == 0x0e31) || ((ch >= 0x0e34) && (ch <= 0x0e39)) || (ch == 0x0e47))
        return AK_TRUE;
    return AK_FALSE;	
}

T_BOOL Is_Thai_SecondClass(T_U16 ch)  //二类符号
{
    if((ch >= 0x0e48) && (ch <= 0x0e4c))
        return AK_TRUE;
    return AK_FALSE;		
}

T_BOOL Is_Thai_ThirdClass(T_U16 ch)  //三类符号
{
    if((ch == 0x0e2f) || (ch == 0x0e30) ||\
        (ch == 0x0e32) || (ch == 0x0e33) ||\
        ((ch >= 0x0e40) && (ch <= 0x0e45)) || \
        (ch == 0x0e3f) ||(ch == 0x0e46))
        return AK_TRUE;
    
    if((ch == 0x0e24) || (ch == 0x0e26))
    {
        TCh_Attr |= HAVE_THAI_THIRDCLASS_SPEC;
        //		 HAVE_THAI_THIRDCLASS_SPEC = AK_TRUE;
        return AK_TRUE;
    }		
    
    return AK_FALSE;		
}

T_BOOL Tran_Thai_FONT(T_U16 ch, T_U16 *tempPos, T_U16 *orignalPos)
{
    //check if Thai font   
    if(Is_Thai_Font(ch))
    {
        //        		Fwl_Freq_Add2Calc(58000000);
        if(Is_Thai_Cons(ch))
        {	
            tempPos[0] = orignalPos[0];
            TCh_Attr |= THAI_THIRDCLASS_EN;
            //        				THAI_THIRDCLASS_EN =AK_TRUE;
        }
        else
        {
            if(Is_Thai_ThirdClass(ch))
            {
                if(TCh_Attr & HAVE_THAI_CONS)
                {
                    tempPos[0] = orignalPos[0];
                    if(TCh_Attr & HAVE_THAI_THIRDCLASS_SPEC)
                    {
                        TCh_Attr |= THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN;
                        //        								THAI_FIRSTCLASS_EN = AK_TRUE;
                        //												THAI_SECONDCLASS_EN = AK_TRUE;
                    }
                    else
                    {
                        TCh_Attr &= ~(THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN);
                        //											THAI_FIRSTCLASS_EN = AK_FALSE;
                        //		 									THAI_SECONDCLASS_EN = AK_FALSE;												
                    }			
                    TCh_Attr |= THAI_THIRDCLASS_EN;
                    //        						THAI_THIRDCLASS_EN =AK_TRUE;
                }
                else
                {
                    if(TCh_Attr & THAI_THIRDCLASS_EN)
                    {
                        tempPos[0] = orignalPos[0];
                        if(TCh_Attr & HAVE_THAI_THIRDCLASS_SPEC)
                        {
                            TCh_Attr |= THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN;
                            //        								THAI_FIRSTCLASS_EN = AK_TRUE;
                            //												THAI_SECONDCLASS_EN = AK_TRUE;
                        }
                        else
                        {
                            TCh_Attr &= ~(THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN);
                            //											THAI_FIRSTCLASS_EN = AK_FALSE;
                            //		 									THAI_SECONDCLASS_EN = AK_FALSE;												
                        }
                        TCh_Attr &= ~THAI_THIRDCLASS_EN;
                        //        							THAI_THIRDCLASS_EN =AK_FALSE;
                    }
                    else
                        return AK_FALSE;
                }
                
            }
            else if(Is_Thai_SecondClass(ch))
            {
                if(TCh_Attr & THAI_SECONDCLASS_EN)
                {
                    if(TCh_Attr & HAVE_THAI_CONS)
                    {
                        orignalPos[0] = tempPos[0];
                        TCh_Attr &= ~(THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN);
                        //        								  THAI_FIRSTCLASS_EN = AK_FALSE;
                        //													THAI_SECONDCLASS_EN = AK_FALSE;
                    }
                    else
                    {
                        if(TCh_Attr & HAVE_THAI_THIRDCLASS_SPEC)
                        {
                            orignalPos[0] = tempPos[0];   
                            TCh_Attr &= ~(THAI_FIRSTCLASS_EN | THAI_SECONDCLASS_EN);
                            //        								 			 THAI_FIRSTCLASS_EN = AK_FALSE;
                            //															 THAI_SECONDCLASS_EN = AK_FALSE;   											
                        }
                        else
                            return AK_FALSE;
                    }
                }
                else
                    return AK_FALSE;
            }
            else if(Is_Thai_FirstClass(ch))
            {
                if(TCh_Attr & THAI_FIRSTCLASS_EN)
                {
                    if(TCh_Attr & HAVE_THAI_CONS)
                    {
                        orignalPos[0] = tempPos[0];
                        TCh_Attr &= ~THAI_FIRSTCLASS_EN;
                        //        								  THAI_FIRSTCLASS_EN = AK_FALSE;
                    }
                    else
                    {
                        if(TCh_Attr & HAVE_THAI_THIRDCLASS_SPEC)
                        {
                            orignalPos[0] = tempPos[0];  
                            TCh_Attr &= ~THAI_FIRSTCLASS_EN;
                            //        								 			 THAI_FIRSTCLASS_EN = AK_FALSE;											
                        }
                        else
                            return AK_FALSE;
                    }
                }
                else
                    return AK_FALSE;  					
            }
            else
                return AK_FALSE;
        }
        }
        else
        {
            //        		Fwl_Freq_Add2Calc(0);
            TCh_Attr &= ~HAVE_THAI_CONS;
            //        		HAVE_THAI_CONS =AK_FALSE;
        }
        
        return AK_TRUE;
}


/***********************************************************************/
//      阿拉伯文和希伯来文显示规则
//  1. 阿拉伯文和希伯来文右对齐显示,字符依次从右往左显示
//  2. 阿拉伯文UNICODE码范围0X0600----0X067F
//  3. 希伯来文UNICODE码范围0X0500----0X05FF
/************************************************************************/ 

#pragma arm section code 

/******************************************************************************
commom buffer公有2K
显示或者获取一串字符串长度时一直占有
此时其他模块不允许使用commom buffer

第一个512字节用于字体背景色显示
第二个512字节用于读取NAND数据，包括codepage使用的部分
第三个512字节用于存储字模
第四个512字节用于存储字库表格，加快速度
*******************************************************************************/
#pragma arm section code = "_fontdisplay_"


T_U8 *FontLib_Get_CommBuf(T_U8 kind) 
{
    if(0 != (gb_font.fontGet.commbuf_Manager & kind))
    {
        if(FONTLIB_COMMBUF == kind)
        {
            return gb_font.fontGet.pCommBuf;
        }
        else
        {
            return gb_font.fontGet.pCommBuf + 512;
        }
    }
    else
    {
        AK_DEBUG_OUTPUT("haven't malloc any font lib commbuf\n");
        return AK_NULL;
    }
}

T_U8 *FontLib_CommBuf_Malloc(T_U8 kind)
{
    if(0 != (gb_font.fontGet.commbuf_Manager & kind))    //重复申请，错误
    {
        AK_DEBUG_OUTPUT("font lib malloc commbuf duplicate\n");
    }
     
    if(0 == gb_font.fontGet.commbuf_Manager)   //还没有申请
    {
        AK_ASSERT_VAL(!gb_font.fontGet.pCommBuf, "font lib malloc error", NULL);
        gb_font.fontGet.pCommBuf = LoadMem_CommBuffMalloc(FONT_COMMONBUFFSIZE);      
    }

    if(AK_NULL == gb_font.fontGet.pCommBuf) 
    {
         AK_DEBUG_OUTPUT("font lib malloc false\n");
         return NULL;
    }   

    if(kind & FONTLIB_COMMBUF)
    {
        gb_font.fontGet.bBufAva = AK_FALSE;
    }
    
    gb_font.fontGet.commbuf_Manager |= kind;

    return FontLib_Get_CommBuf(kind);
}

T_U8 *FontLib_CommBuf_Free(T_U8 kind)
{   
    if(0 == (gb_font.fontGet.commbuf_Manager & kind))
    {
        AK_ASSERT_VAL(gb_font.fontGet.pCommBuf, "font buffer free error!!!", NULL);
    }

    gb_font.fontGet.commbuf_Manager &= ~kind;
    
    if(0 == gb_font.fontGet.commbuf_Manager)
    {
        LoadMem_CommBuffFree();
        gb_font.fontGet.pCommBuf = NULL;
        gb_font.fontGet.bBufAva = AK_FALSE;
    }
   
    return NULL;
}

/******************************************************************************
* @NAME    GetFontMatrix_Normal
* @BRIEF    get font matrix from the font libarary file (compatible for the old version)
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           ch: the unicode
*           pBuf:data buffer, at less of 512bytes 
* @RETURN    offset of the matrix in the buffer
                    INVALID_OFFSET:means do not support this unicode
*******************************************************************************/
static T_U16 GetFontMatrix_Normal(T_U16 ch, T_U8* pBuf)
{    
    T_U32 pos;

    pos = FONT_POS(ch);  //字模在字库中的位置
      
    if(AK_FALSE == FontLib_ReadFile(pos, pBuf))
    {
        return (T_U16)INVALID_OFFSET;
    }
    
    return (T_U16)FONT_OFFSET(ch);
}

/******************************************************************************
* @NAME    GetFontMatrix_LangRegion
* @BRIEF    get font matrix from the font libarary file which's type is FONTLIB_FORMAT_LANG_REGION
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           ch: the unicode
*           pBuf:data buffer, at less of 512bytes 
* @RETURN    offset of the matrix in the buffer
                    INVALID_OFFSET:means do not support this unicode
*******************************************************************************/
static T_U16 GetFontMatrix_LangRegion (T_U16 ch, T_U8* pBuf)
{
    T_U32 langRegion, langRegion_min = 0, langRegion_max = MAX_LANGUAGE_REGION-1;
    T_U32 pos;
    T_U32 offset;
    T_U32 num;
    T_U8 *pTableBuf = Get_Table_Buffer();

    if(ch > 255)    //判断是否在有效范围之内
    {
        if((ch < gb_font.fontLibInfo.Format_LangRegion.unicode_min)
            || (ch > gb_font.fontLibInfo.Format_LangRegion.unicode_max))
        {
            return (T_U16)INVALID_OFFSET;
        }
    }
    
    while(1)    //二分查找是否所属语言域
    {  
        langRegion = (langRegion_max + langRegion_min) / 2;
        if(Check_LangRegion[langRegion] > ch)
        {
            langRegion_max = langRegion;
        }
        else if(Check_LangRegion[langRegion+1] < ch)
        {
            langRegion_min = langRegion;
        }
        else
        {
            break;
        }
    }

    //如果和上次使用的语言域相同，则可以直接得到offset
    if(langRegion != gb_font.fontGet.record_LastRegion)
    {
        pos = langRegion * sizeof(T_U32) + gb_font.fontLibInfo.Format_LangRegion.LangRegion_Table;
        
        //如果缓存区以保存有合适的数据则不需要再读NAND
        if(langRegion/LANGREION_IN_BUFFER != gb_font.fontGet.record_LastRegion/LANGREION_IN_BUFFER
            || gb_font.fontGet.bBufAva == AK_FALSE)
        {
            if(AK_FALSE == FontLib_ReadFile(pos, pTableBuf)) //读取langRegion_Table
            {
                return (T_U16)INVALID_OFFSET;
            }
            gb_font.fontGet.bBufAva = AK_TRUE;
        }
        
        //第一个512字节只保存了前126个语言域的偏移
        offset = (pos<BUFFER_SIZE)? *(T_U32 *)(pTableBuf + gb_font.fontLibInfo.Format_LangRegion.LangRegion_Table + sizeof(T_U32)*langRegion) :
                        *(T_U32 *)(pTableBuf + sizeof(T_U32)*(langRegion -LANGREION_IN_BUFFER));

        gb_font.fontGet.record_LastRegion = langRegion;
        gb_font.fontGet.record_LastFontOffset = offset;
    }
    else
    {
        offset = gb_font.fontGet.record_LastFontOffset;
        if(offset > (1<<22))
        {
            return (T_U16)INVALID_OFFSET;
        }
    }
        
    if(INVALID_OFFSET == offset)    //不支持该语言域
    {
        return (T_U16)INVALID_OFFSET;
    }

    //计算该unicode离所属语言域开头有多少个unicode
    if(gb_font.fontLibInfo.Format_LangRegion.unicode_min > Check_LangRegion[langRegion]
            && ch > 255)
    {
        num = ch - gb_font.fontLibInfo.Format_LangRegion.unicode_min;
    }
    else
    {
        num = ch - Check_LangRegion[langRegion];
    }  
    
    //计算字模存储偏移
    pos = (offset&~(BUFFER_SIZE -1)) + FONT_POS((offset%BUFFER_SIZE)/(sizeof(T_FONT_ABC) + FONT_BYTE_COUNT) + num);
    offset = pos%BUFFER_SIZE;
    if(AK_FALSE == FontLib_ReadFile(pos, pBuf))
    {
        return (T_U16)INVALID_OFFSET;
    }
    
    return (T_U16)offset;
}


/******************************************************************************
* @NAME    bit_count
* @BRIEF    count bit 1 
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           x: the data which to be count
* @RETURN    number of bit 1
*******************************************************************************/
#define MASK1   0x55555555
#define MASK2   0x33333333
#define MASK4   0x0f0f0f0f
#define MASK8   0x00ff00ff
#define MASK16 0x0000ffff
T_U8 bit_count(T_U32 x)
{
    x = (x & MASK1) + ((x >> 1) & MASK1);
    x = (x & MASK2) + ((x >> 2) & MASK2);
    x = (x & MASK4) + ((x >> 4) & MASK4);
    x = (x & MASK8) + ((x >> 8) & MASK8);
    x = (x & MASK16) + ((x >> 16) & MASK16);
    
    return (T_U8)x;
}

/******************************************************************************
* @NAME    GetFontMatrix_BitMap
* @BRIEF    get font matrix from the font libarary file which's type is FONTLIB_FORMAT_BITMAP
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           ch: the unicode
*           pBuf:data buffer, at less of 512bytes 
* @RETURN    offset of the matrix in the buffer
                    INVALID_OFFSET:means do not support this unicode
*******************************************************************************/
static T_U16 GetFontMatrix_BitMap (T_U16 ch, T_U8* pBuf)
{
    T_U32 pos, section, offset, start, temp;
    T_S32 num;
    T_U8 *pTableBuf = Get_Table_Buffer();

    pos = gb_font.fontLibInfo.Format_BitMap.BitMap_Table + ch/8;    //unicode对应位段所处的位置
    if(AK_FALSE == FontLib_ReadFile(pos, pBuf))
    {
        return (T_U16)INVALID_OFFSET; 
    }
    if(0 == (pBuf[ch/8%BUFFER_SIZE] & (1<<(ch%8))))  //对应位段是否支持
    {
        return (T_U16)INVALID_OFFSET;
    } 
    
    section = ch/8/SECTION_SIZE;   //计算section_table所处的位置，并读取
    if(AK_FALSE == gb_font.fontGet.bBufAva)
    {
        pos = gb_font.fontLibInfo.Format_BitMap.Sections_Table + sizeof(T_U32)*section;
        if(AK_FALSE == FontLib_ReadFile(pos, pTableBuf))
        {
            return (T_U16)INVALID_OFFSET; 
        }
        gb_font.fontGet.bBufAva = AK_TRUE;
    }
    
    //每段有64字节，如果需要查找的数据在前32字节则从第0字节正序查找
    if(ch/8%SECTION_SIZE < SECTION_SIZE/2   //从段首开始统计位1数量
        || section == (SECTION_NUM-1))          //最后一段只能从段首开始
    {      
        offset = *(T_U32 *)(pTableBuf + section*sizeof(T_U32));   //该段首字模的偏移
        start = (ch/8) & ~(SECTION_SIZE-1);
        temp = (T_U32)((ch/8)&(~3));
        for(num = 0; start < temp; start += sizeof(T_U32))  //统计位1的数量
        {
            num += bit_count(*(T_U32 *)(pBuf+start%BUFFER_SIZE));
        }
        temp = *(T_U32 *)(pBuf+start%BUFFER_SIZE);
        temp &= (1<<(ch%32))-1;
        num += bit_count(temp);        
        pos = (offset&~(BUFFER_SIZE -1)) + FONT_POS((offset%BUFFER_SIZE)/(sizeof(T_FONT_ABC) + FONT_BYTE_COUNT) + num); //字模所在绝对地址
    }
    else        //从段尾开始统计位1数量
    {       
        offset = *(T_U32 *)(pTableBuf + (section+1)*sizeof(T_U32));   //该段首字模的偏移
        start = ALIGN(ch/8, SECTION_SIZE) - sizeof(T_U32);
        temp = ALIGN(ch, 8);
        temp = ALIGN(temp/8, sizeof(T_U32));
        for(num = 0; start >= temp; start -= sizeof(T_U32))  //统计位1的数量
        {
            num -= bit_count(*(T_U32 *)(pBuf+start%BUFFER_SIZE));
        }
        if(ch%32)
        {
            temp = *(T_U32 *)(pBuf+start%BUFFER_SIZE);     
            temp &= ~((1<<(ch%32))-1);
            num -= bit_count(temp);   
        }      
        temp = (num/(T_S32)CHAR_NUM_IN_512BYTE)*(-1) + 1;
        num = (T_S32)temp*CHAR_NUM_IN_512BYTE + num;
        offset -= (temp*512);
        pos = (offset&~(BUFFER_SIZE -1)) + FONT_POS((offset%BUFFER_SIZE)/(sizeof(T_FONT_ABC) + FONT_BYTE_COUNT) + num); //字模所在绝对地址
    }
    offset = pos%BUFFER_SIZE;
    if(AK_FALSE == FontLib_ReadFile(pos, pBuf))
    {
        return (T_U16)INVALID_OFFSET;
    }
    
    return (T_U16)offset;                   
}

/******************************************************************************
* @NAME    GetFontMatrix_FromFontLib
* @BRIEF    get font matrix from font library file
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           ch: the unicode
*           pBuf:data buffer, at less of 512bytes 
* @RETURN    offset of the matrix in the buffer
                    INVALID_OFFSET:means do not support this unicode
*******************************************************************************/
static T_U16 GetFontMatrix_FromFontLib (T_U16 ch, T_U8* pBuf)
{
    if(FONTLIB_FORMAT_NORMAL== gb_font.fontLibInfo.FontLib_Header.LibFormat)
    {
        return GetFontMatrix_Normal(ch, pBuf);
    }
    else if(FONTLIB_FORMAT_LANG_REGION == gb_font.fontLibInfo.FontLib_Header.LibFormat)
    {
        return GetFontMatrix_LangRegion(ch, pBuf);
    }
    else if(FONTLIB_FORMAT_BITMAP== gb_font.fontLibInfo.FontLib_Header.LibFormat)
    {
        return GetFontMatrix_BitMap(ch, pBuf);
    }
    
    return (T_U16)INVALID_OFFSET;
}

/******************************************************************************
* @NAME    GetDefaultFontMatrix
* @BRIEF    get default font matrix 
* @AUTHOR  luqizhou
* @DATE    2011-03-30
 * @PARAM  pBuf:data buffer, at less of 512bytes 
* @RETURN    
*******************************************************************************/
static T_VOID GetDefaultFontMatrix(T_U8* pBuf)
{
/*    //使用小号字体
    T_U8 const*pSrcFontMatrix = Small_Font[DEFAULT_FONT-SMALL_FONT_BEGIN];
    T_U8 *pDesFontMatrix = sizeof(T_FONT_ABC) + pBuf;
    T_FONT_ABC *pFontABC = (T_FONT_ABC*)pBuf;

    AK_DEBUG_OUTPUT("default font\n");
    
    pFontABC->abcA = 1;
    pFontABC->abcB = SMALL_FONT_WIDTH;
    pFontABC->abcC= FONT_TYPE - SMALL_FONT_WIDTH;
    pFontABC->reserved = 0xff;

    memcpy(pDesFontMatrix, pSrcFontMatrix, SMALL_FONT_BYTE_COUNT);
    memset(pDesFontMatrix+SMALL_FONT_BYTE_COUNT, 0, FONT_BYTE_COUNT - SMALL_FONT_BYTE_COUNT);*/

    //使用对应大小的字体
    memcpy(pBuf, Default_Font, sizeof(Default_Font));
}

/******************************************************************************
* @NAME    FontLib_ReadFile
* @BRIEF    read the font library file
* @AUTHOR  luqizhou
* @DATE    2011-03-22
* @PARAM 
*           pos: the position of file 
*           pBuf:data buffer, at less of 512bytes 
* @RETURN   AK_TRUE:success
                    AK_FALSE:false
*******************************************************************************/
static T_BOOL FontLib_ReadFile(T_U32 pos, T_U8 *pBuf)
{
    T_U32 index;
    T_U32 BlockId;
    T_U32 PageId;
    T_U32 offset;

    index = pos / (1<<gb_font.blockBit);
    BlockId = gb_font.font2block[index];
    PageId = (pos & ((1<<gb_font.blockBit) -1)) / (1<<gb_font.pageBit);
    offset = (pos & ((1<<gb_font.pageBit) -1)) / 512;
    
    if(0 >= progmanage_LoadStorage512Byte(BlockId, PageId, offset,pBuf,FONT_PATH))
    {
#if (OS_ANYKA)

        //recover data from back block        
        if(progmanage_recover_block(FONT_LIB_NAME, BlockId, AK_NULL))
        {
            if (0 < progmanage_LoadStorage512Byte(BlockId, PageId, offset,pBuf,FONT_PATH))
            {
                return AK_TRUE;
            }
        }
        
        //load bin from back block
        gb_font.font2block[index] = progmanage_get_backup_block(FONT_LIB_NAME, index);
        BlockId = gb_font.font2block[index];
        if (0 < progmanage_LoadStorage512Byte(BlockId, PageId, offset,pBuf,FONT_PATH))
        {   
            return AK_TRUE;
        }
        AK_DEBUG_OUTPUT("font get backup data error\n");
        
#endif

        return AK_FALSE; 
    }
    
    return AK_TRUE;
}
#pragma arm section code

#else
T_U8 *FontLib_Get_CommBuf(T_U8 kind) 
{
	return AK_NULL;
}
T_U8 *FontLib_CommBuf_Malloc(T_U8 kind)
{
	return AK_NULL;
}
T_U8 *FontLib_CommBuf_Free(T_U8 kind)
{
	return AK_NULL;
}

#endif

