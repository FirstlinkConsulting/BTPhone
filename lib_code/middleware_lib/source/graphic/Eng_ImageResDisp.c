/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name：Eng_ImageResDisp.c
 * Function：offer the image res diap interfence
 *
 * Author：xuping
 * Date：2008-05-16
 * Version：0.1.0
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#include "Gbl_global.h"
#include "Eng_ImageResDisp.h"
#include "string.h"
#include "Fwl_FreqMgr.h"
#include "log_aud_play.h"
#include "Eng_Debug.h"

#if(NO_DISPLAY == 0)
//#define OLD_LOADIMAGE 
typedef struct
{
    T_U8                pageBit;                //size of one page in nandflash(2^n)
    T_U8                blockBit;               //size of one block in nandflash(2^n)
    IMAFERES_CALLBACK_GETDATA getDataFun;   
    T_U16               ImageRes2block[IMAGE_RES_BLOCK_USED];  //tabal of res information saved in nandflash   
    T_IMAGERES_HEADER*   ImageResHeader;  //image res header
}T_GLOAL_IMAGERES;

typedef enum{
	IMAGE_NOTHING,
	IMAGE_WHOLE,
	IMAGE_MORETHANHALF,
	IAMGE_LESSTHANHALF,
}E_IMAGEPART;

#define FLASH_LOAD_CELL             512 //读取数据的起始位置匹配单元(BYTE)
#define IMAGEBUF_ALLSIZE 		    5120//5 * 1024
#define IMAGEBUF_MIDDLESIZE 		2048//2 * 1024
#define IMAGEBUF_SMALLSIZE 		    1024//1 * 1024
extern T_BOOL IsInRadioMenu( T_VOID );

#define ImageRes_GetBlock(offset,blockBit)     (((offset) << 9) >> (blockBit))    
#define ImageRes_GetPage(offset,pageBit,blockBit)\
			((((offset) << 9) & ((1 << (blockBit)) - 1)) >> (pageBit))
#define Codepage_GetReadOffset(offset,pageBit)\
            ((((offset) << 10) & ((1 << (pageBit)) -1)) >> 10)
#define ImageRes_GetMod(var)  ((var) & (0x01ff))  //mode by 512
#define ImageRes_DIV(var)     ((var) >> 9)        //整除512
#define ImageRes_ALLIGN(var)  ((var) + 0x1ff) & ~0x1ff)  //512 对齐
#define ImageRes_MUL(var)     ((var) << 9)        //512倍数

#define Codepage_GetMaxOffsetInPage(pageBit)  (((1 << (pageBit)) -1) >> 9) 

#if (USE_COLOR_LCD)
#define ImageResDataLenPerHeight(width)         ((width)*2)
#define ImageResDataLenPerwidth(height)         ((height)*2)
#define ImageResHeightUnit                      1
#else
#define ImageResDataLenPerHeight(width)         ((width))
#define ImageResDataLenPerwidth(height)         ((height) >> 3)
#define ImageResHeightUnit                      8

#endif

#if (STORAGE_USED == SPI_FLASH)
#pragma arm section zidata = "_bootbss1_"
#else
#pragma arm section zidata = "_bootbss_"

#endif
T_GLOAL_IMAGERES gb_imageres;
#pragma arm section zidata

static T_U32 Eng_ImageResGetDataEx(T_RES_IMAGE imgResID, T_U16 Width, T_U16 Height, T_U16 hOffset, \
		T_U16 vOffset,T_U8 *data, T_U16 saveLen, T_U16 allLen, T_U16* dataStart, T_U16* dataHight);
/******************************************************************************
 * @NAME    ImageRes_LoadDatafromBinEx
 * @BRIEF   load data from bin with size
 * @AUTHOR  liangxiong
 * @DATE    2011-02-17
 * @PARAM   offset: data offset in bin file(allign by 512)
 *		    data: buf (must larger than size byte) 
 			size: the size of data by load (multiple 512B)
 * @RETURN  T_S8
 *	
*******************************************************************************/
static T_S8 ImageRes_LoadDatafromBinEx(T_U16 offset,T_U8 data[],T_U16 size)
{
    T_U16 index             = 0;
    T_U16 BlockId           = 0;
    T_U16 PageId            = 0;
    T_U16 Readoffset        = 0; 
	T_U16 saveNum = 0;
	T_U16 readNum = 0;
	T_U16 expectNum = 0;
	T_U16 pageMaxOff = 0;
	T_U8 *saveData = data;
	T_U16 readSize = size;

    //获取数据的位置，读取数据
    index = ImageRes_GetBlock(offset ,gb_imageres.blockBit);
    BlockId = gb_imageres.ImageRes2block[index];
    PageId  = ImageRes_GetPage(offset, gb_imageres.pageBit,gb_imageres.blockBit);
    Readoffset = Codepage_GetReadOffset(offset,gb_imageres.pageBit);

#if (OS_ANYKA)
	if ((0 == size) || (0 != ImageRes_GetMod(size)))
	{
		AK_DEBUG_OUTPUT("error:loading size is not multiple by 512B.\r\n");
		return -1;
	}
	expectNum = ImageRes_DIV(size);
	saveNum = 0;
	readNum = 1;
	pageMaxOff = Codepage_GetMaxOffsetInPage(gb_imageres.pageBit);

	do//进入循环加载
	{
	    if (expectNum>saveNum && (expectNum-saveNum)>1)//加载数目多个512B
	    {
			if (Readoffset < pageMaxOff)//页内号小于最大页内号。
			{
				readNum = pageMaxOff - Readoffset + 1;
				if ((expectNum-saveNum) < readNum)
				{
					readNum = expectNum - saveNum;
				}
				readSize = ImageRes_MUL(readNum);
			}
			else
			{
				readNum = 1;
				readSize = ImageRes_MUL(readNum);
			}
	    }
#endif

	    if ( 0 >= progmanage_LoadStorageMulti512Byte(BlockId, PageId, Readoffset, saveData, IMAGERES_PATH, readSize))
	    {
#if (OS_ANYKA)
	        //recover data from back block
	        if(progmanage_recover_block(IMAGERES_LIB_NAME, BlockId, AK_NULL))
	        {
	            if (0 < progmanage_LoadStorageMulti512Byte(BlockId, PageId, Readoffset, saveData, IMAGERES_PATH, readSize))
	            {
	                goto GET_SUCESS;
	            }
	        }
	        AK_DEBUG_OUTPUT("image res recover data error\n");
	        //load bin from back block
	        gb_imageres.ImageRes2block[index] = progmanage_get_backup_block(IMAGERES_LIB_NAME, index);
	        BlockId = gb_imageres.ImageRes2block[index];
	        if (0 < progmanage_LoadStorageMulti512Byte(BlockId, PageId, Readoffset, saveData, IMAGERES_PATH, readSize))
	        {            
	            goto GET_SUCESS;
	        }
	        AK_DEBUG_OUTPUT("image res get backup data error\n");
#endif
        return -1;
	    }
#if (OS_ANYKA)   
GET_SUCESS:
	    if (expectNum>saveNum && (expectNum-saveNum)>1)//加载数目多个512B
		{
			saveNum += readNum;	  //记录以保存数据
			saveData += ImageRes_MUL(readNum);//偏移指针
			//准备读取下一个数据块
			index = ImageRes_GetBlock((offset+saveNum),gb_imageres.blockBit);
			BlockId = gb_imageres.ImageRes2block[index];
			PageId	= ImageRes_GetPage((offset+saveNum), gb_imageres.pageBit,gb_imageres.blockBit);
			Readoffset = Codepage_GetReadOffset((offset+saveNum),gb_imageres.pageBit);
			readNum = 1;
			readSize = ImageRes_MUL(readNum);
		}
		else //当只加载一个512B数据
		{
			saveNum = expectNum;
		}

	}while (expectNum > saveNum);//判断是否加载完成
#endif  
    return 1;
}

/******************************************************************************
 * @NAME    ImageRes_LoadDatafromBin
 * @BRIEF   load data from bin
 * @AUTHOR  xuping
 * @DATE    2008-05-17
 * @PARAM   offset: data offset in bin file(allign by 512)
 *		    data: buf (must larger than 512 byte) 
 * @RETURN  T_S8
 *	
*******************************************************************************/
static T_S8 ImageRes_LoadDatafromBin(T_U16 offset,T_U8 data[])
{
    T_U16 index             = 0;
    T_U16 BlockId           = 0;
    T_U16 PageId            = 0;
    T_U16 Readoffset        = 0; 

    //获取数据的位置，读取数据
    index = ImageRes_GetBlock(offset ,gb_imageres.blockBit);
    BlockId = gb_imageres.ImageRes2block[index];
    PageId  = ImageRes_GetPage(offset, gb_imageres.pageBit,gb_imageres.blockBit);
    Readoffset = Codepage_GetReadOffset(offset,gb_imageres.pageBit);

    if ( 0 >= progmanage_LoadStorage512Byte(BlockId, PageId, Readoffset, data,IMAGERES_PATH))
    {
#if (OS_ANYKA)
        //recover data from back block
        if(progmanage_recover_block(IMAGERES_LIB_NAME, BlockId, AK_NULL))
        {
            if (0 < progmanage_LoadStorage512Byte(BlockId, PageId, Readoffset, data,IMAGERES_PATH))
            {
                goto GET_SUCESS;
            }
        }
        AK_DEBUG_OUTPUT("image res recover data error\n");
        //load bin from back block
        gb_imageres.ImageRes2block[index] = progmanage_get_backup_block(IMAGERES_LIB_NAME, index);
        BlockId = gb_imageres.ImageRes2block[index];
        if (0 < progmanage_LoadStorage512Byte(BlockId, PageId, Readoffset, data,IMAGERES_PATH))
        {            
            goto GET_SUCESS;
        }
        AK_DEBUG_OUTPUT("image res get backup data error\n");
#endif
        return -1;
    }
#if (OS_ANYKA)   
GET_SUCESS:
#endif  
    return 1;
}

#pragma arm section code = "_sysinit_"
/******************************************************************************
 * @NAME    Eng_ImageResInit
 * @BRIEF   init image resourse variable
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *		    T_VOID
 * @RETURN AK_TRUE:success
 *         AK_FALSE:fail
 *	
*******************************************************************************/
T_BOOL Eng_ImageResInit(T_U8* headbuf, T_U32 ImageResNum)
{
        T_U8 *tempBuf = AK_NULL;
        T_U16 NumReadheader = 0;
        T_U16 i = 0;
        T_U16 hdOffset  = 0;
#ifdef OS_WIN32        
        
        gb_imageres.blockBit = 18;//256k
        gb_imageres.pageBit  = 11;  //2k
        for (i = 0; i < IMAGE_RES_BLOCK_USED; i++)
        {
            gb_imageres.ImageRes2block[i] = i;
        }
#else

        gb_imageres.getDataFun = AK_NULL;
         
        progmanage_StorageGetInfo(&gb_imageres.pageBit, &gb_imageres.blockBit);
        
        if(!progmanage_get_maplist(IMAGERES_LIB_NAME, gb_imageres.ImageRes2block, IMAGE_RES_BLOCK_USED))
        {
            return AK_FALSE;
        }
#endif
    //初始化资源头信息   
    gb_imageres.ImageResHeader = (T_IMAGERES_HEADER *)headbuf;
    if (AK_NULL == gb_imageres.ImageResHeader)
    {
        return AK_FALSE;
    }
    //tempBuf  = Gbl_GetCommBuff(); 
    tempBuf  = LoadMem_GetRemainBuff(&i); 
    //读取头信息的偏移
    ImageRes_LoadDatafromBin(0,tempBuf);
    hdOffset = (T_U16)ImageRes_DIV( *((T_U32 *)tempBuf));
    
    NumReadheader = (T_U16)ImageRes_DIV(ImageResNum * sizeof(T_IMAGERES_HEADER));
    for (i = 0; i <= NumReadheader; i++)
    {
        //每次读取512byte头信息
        if (0 < ImageRes_LoadDatafromBin((T_U16)(i + hdOffset),tempBuf))
        {
            if (NumReadheader == i)
            {
                memcpy((T_U8 *)((T_U8*)gb_imageres.ImageResHeader + i * COMBUF_SIZE_USED),tempBuf, ImageRes_GetMod(ImageResNum * sizeof(T_IMAGERES_HEADER)));
            }
            else
            {
                memcpy((T_U8 *)((T_U8*)gb_imageres.ImageResHeader + i * COMBUF_SIZE_USED),tempBuf, COMBUF_SIZE_USED);
            }
        }
        else
        {
        	LoadMem_RemainBuffFree();
            AK_DEBUG_OUTPUT("get image resource header error\r\n");
            return AK_FALSE; 
        }
    }
	LoadMem_RemainBuffFree();
    return AK_TRUE;
}
#pragma arm section code

/******************************************************************************
 * @NAME    Eng_ImageResGetData
 * @BRIEF   load data of rectangular area  in image resource   
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *		    imgeResID: image resource ID square
 *          Width : the width of the rectangular area
 *          Height: the height of the rectangular area
 *          hOffset: the horizonal offset of the rectangular area
 *          voffset: the vertical offset of the rectangular area
 *          data:    the buf of the data (must more then 1kB)                   
 * @RETURN AK_TRUE:success
 *         AK_FALSE:fail
 *	
*******************************************************************************/

T_BOOL Eng_ImageResGetData(T_RES_IMAGE imgResID, T_U16 Width, T_U16 Height,T_U16 hOffset, T_U16 vOffset,T_U8 data[])
{
#ifdef OLD_LOADIMAGE
    T_U16  filledHeight      = 0;    //已填充的高度
    T_U16  filledWidth       = 0;    //已填充的宽度
    T_U16 filledoffset      = 0;    //数据在被填充区域的偏移
    T_U16 ImageResoffset    = 0;    //图片资源在文件中的偏移
    T_U16 tempoffset        = 0;    //数据在读取的buf中的偏移
    T_U32 currentoffset     = 0;    //数据在整个图片中的偏移
    T_U8  *tempbuf          = AK_NULL;//读取数据的临时buf

    AK_ASSERT_VAL( (Width > 0 && Width  <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData width error", AK_FALSE);
    AK_ASSERT_VAL( (Height > 0 && Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData height error", AK_FALSE);
    AK_ASSERT_VAL( (hOffset + Width <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData hoffset error", AK_FALSE);
    AK_ASSERT_VAL( (vOffset + Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData voffset error", AK_FALSE);

    if (AK_NULL != gb_imageres.getDataFun)
    {
        if(gb_imageres.getDataFun(imgResID,Width ,Height, hOffset, vOffset, data))
        {
            return AK_TRUE;
        }
    }

    tempbuf = (T_U8*)(Gbl_GetCommBuff() + COMBUF_SIZE_USED);
    ImageResoffset = gb_imageres.ImageResHeader[imgResID].offset;

       //定位要读取数据的位置
    currentoffset = ImageResDataLenPerwidth(vOffset)*gb_imageres.ImageResHeader[imgResID].width + hOffset * ImageResDataLenPerHeight(1);
    do
    { 
        //获取数据
        if(0 >= ImageRes_LoadDatafromBin((T_U16)(ImageResoffset + ImageRes_DIV(currentoffset)), tempbuf))
        {
            AK_DEBUG_OUTPUT("Eng_ImageResGetData error\r\n");
            return AK_FALSE;
        }
        //填充数据
        tempoffset = (T_U16)ImageRes_GetMod(currentoffset);
        //尾部分散数据
        if (0 != filledWidth)
        {
            T_U16 tempLen = ImageResDataLenPerHeight(Width - filledWidth);

            if (tempLen > COMBUF_SIZE_USED- tempoffset)//读取的数据不够填充矩形区域一行的剩余区域
            {
                memcpy(data + filledoffset, tempbuf + tempoffset, COMBUF_SIZE_USED- tempoffset);
                filledoffset    += COMBUF_SIZE_USED - tempoffset;
                filledWidth     += (COMBUF_SIZE_USED - tempoffset)/ImageResDataLenPerHeight(1);
                currentoffset   += COMBUF_SIZE_USED - tempoffset;
                continue;
            }
            else 
            {
                //填充一行尾部的数据
                memcpy(data + filledoffset, tempbuf + tempoffset, tempLen);
                filledoffset    += tempLen;
                tempoffset      = tempoffset + tempLen + ImageResDataLenPerHeight(gb_imageres.ImageResHeader[imgResID].width - Width);
                currentoffset   = currentoffset + tempLen + ImageResDataLenPerHeight(gb_imageres.ImageResHeader[imgResID].width - Width);
                filledHeight    += ImageResHeightUnit;
                filledWidth     = 0;
                if (filledHeight >= Height)//填充完毕
                {
                     goto exit;
                }                
            }
        }
        //中间整行数据
        while( COMBUF_SIZE_USED > tempoffset + ImageResDataLenPerHeight(Width) )
        {
              
            memcpy(data + filledoffset, tempbuf + tempoffset, ImageResDataLenPerHeight(Width));
            filledoffset  += ImageResDataLenPerHeight(Width);
            tempoffset    += ImageResDataLenPerHeight(gb_imageres.ImageResHeader[imgResID].width);
            currentoffset += ImageResDataLenPerHeight(gb_imageres.ImageResHeader[imgResID].width);
            filledHeight += ImageResHeightUnit;
            filledWidth  = 0;
            if (filledHeight >= Height)//填充完毕
            {
              goto exit;
            } 
        }
        //头部分散数据
        if(tempoffset < COMBUF_SIZE_USED)
        {
            memcpy(data + filledoffset, tempbuf + tempoffset, COMBUF_SIZE_USED - tempoffset);
            filledoffset  += COMBUF_SIZE_USED - tempoffset;
            currentoffset += COMBUF_SIZE_USED - tempoffset;
            filledWidth   += (COMBUF_SIZE_USED - tempoffset)/ImageResDataLenPerHeight(1);;            
        }
        
    }while(1);

exit:    
    return AK_TRUE;
#else
	//截图获取情况
	T_U16    dataStart		 	 = 0;
	T_U16	 lenSaved			 = 0;
	T_U16	 lenLoaded			 = 0;
    T_U16    HeightRequire       = 0;
	T_U16    HeightOffset        = 0;
    T_U16    HeightLoaded        = 0;
	T_U16    HeightSaved         = 0;
    T_U8    *tempbuf             = AK_NULL;
	T_U16    bufSize             = 0;
	
    AK_ASSERT_VAL( (Width > 0 && Width  <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData width error", AK_FALSE);
    AK_ASSERT_VAL( (Height > 0 && Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData height error", AK_FALSE);
    AK_ASSERT_VAL( (hOffset + Width <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData hoffset error", AK_FALSE);
    AK_ASSERT_VAL( (vOffset + Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData voffset error", AK_FALSE);

    if (AK_NULL != gb_imageres.getDataFun)
    {
        if(gb_imageres.getDataFun(imgResID,Width ,Height, hOffset, vOffset, data))
        {
            return AK_TRUE;
        }
    }

	tempbuf = LoadMem_GetRemainBuff(&bufSize);
	if (AK_NULL == tempbuf || bufSize < IMAGEBUF_MIDDLESIZE)//不小于2KB
	{
		AK_DEBUG_OUTPUT("error: no enough memory to load.");
		return AK_FALSE;
	}
	Fwl_FreqPush(FREQ_APP_MAX);

	while (HeightSaved < Height)
	{
        HeightRequire = Height - HeightSaved;
		HeightOffset = vOffset + HeightSaved;

  	    if (0 == Eng_ImageResGetDataEx(imgResID,	Width, HeightRequire, hOffset, HeightOffset,\
  	    		tempbuf, FLASH_LOAD_CELL, bufSize, &dataStart, &HeightLoaded))
  	    {
			Fwl_FreqPop();
			LoadMem_RemainBuffFree();
			return AK_FALSE;
		}
		lenLoaded = HeightLoaded*ImageResDataLenPerwidth(Width);
		memcpy(data+lenSaved, tempbuf+dataStart, lenLoaded);
		HeightSaved += HeightLoaded;
		lenSaved += lenLoaded;
	}
    Fwl_FreqPop();
	LoadMem_RemainBuffFree();

	return AK_TRUE;
#endif
}
/******************************************************************************
 * @NAME    GetResImagedataEx
 * @BRIEF   get resource data of image in many lines
 * @AUTHOR  liangxiong
 * @DATE    2011-02-17
 * @PARAM 
 *		    imgResID: image resource ID square
 *          Width :    the width of the rectangular area
 *          Height:    the height of the rectangular area
 *          hOffset:   the horizonal offset of the rectangular area
 *          voffset:   the vertical offset of the rectangular area
 *          data:      the buf of the data 
 *          saveLen:   the size of the buf to save  
 *          allLen:    the size of the buf (best more then saveLen) 
 *          dataStart: the data start of buf
  *         dataHight: the data hight of buf
 * @RETURN the data length
 * 
*******************************************************************************/
static T_U32 Eng_ImageResGetDataEx(T_RES_IMAGE imgResID, T_U16 Width, T_U16 Height, T_U16 hOffset, \
		T_U16 vOffset,T_U8 *data, T_U16 saveLen, T_U16 allLen, T_U16* dataStart, T_U16* dataHight)
{
	T_U16    ImageResoffset		 = 0;	//图片资源在文件中的偏移
	T_U16    headOffset		 	 = 0;	//首行偏移长度
	T_U16    cellOffset		 	 = 0;	//首行所在存储块偏移数
    T_U16    imageWidthLen       = 0;
    T_U16    cutWidthLen         = 0;
    T_U16    loadLen             = 0;
    T_U16    dataLen			 = 0;
    T_U16    HeightFeatReq       = 0;	//适合缓存容量的需求行数
	T_U16    HeightFilled        = 0;   //已填充缓存中的行数
    T_U16    HeightLoaded        = 0;   //
	T_U16    i                   = 0;
	T_U32    temp                = 0;	
    T_U8    *tempbuf             = AK_NULL;
    E_IMAGEPART  imagePart       = IMAGE_NOTHING;
	
    AK_ASSERT_VAL( (Width > 0 && Width  <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData width error", AK_FALSE);
    AK_ASSERT_VAL( (Height > 0 && Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData height error", AK_FALSE);
    AK_ASSERT_VAL( (hOffset + Width <= gb_imageres.ImageResHeader[imgResID].width), "Eng_ImageResGetData hoffset error", AK_FALSE);
    AK_ASSERT_VAL( (vOffset + Height <= gb_imageres.ImageResHeader[imgResID].height), "Eng_ImageResGetData voffset error", AK_FALSE);


	ImageResoffset = gb_imageres.ImageResHeader[imgResID].offset;
	imageWidthLen = ImageResDataLenPerwidth(gb_imageres.ImageResHeader[imgResID].width);
	//imageHight = gb_imageres.ImageResHeader[imgResID].height;

	//判断图片的截图部分
	if (gb_imageres.ImageResHeader[imgResID].width == Width)
	{
		imagePart = IMAGE_WHOLE;
	}
	/*else if ((gb_imageres.ImageResHeader[imgResID].width/2) < Width)
	{
		imagePart = IMAGE_MORETHANHALF;
	}
	else
	{
		imagePart = IAMGE_LESSTHANHALF;
	}*/
	else
		imagePart = IMAGE_MORETHANHALF;

	//获取图片数据
	if (imagePart == IMAGE_WHOLE)
	{
		headOffset = ImageRes_GetMod(imageWidthLen * vOffset);
		HeightLoaded = (allLen - headOffset)/imageWidthLen;
		HeightLoaded = (Height>HeightLoaded) ? HeightLoaded : Height;
		dataLen = headOffset + HeightLoaded * imageWidthLen;
		loadLen = dataLen;
		i = ImageRes_GetMod(loadLen);
		//修正512B块
		while (0 != i)
		{
			if ((allLen-loadLen) >= (FLASH_LOAD_CELL-i))
			{
				loadLen += (FLASH_LOAD_CELL-i);
			}
			else if (HeightLoaded > 1)
			{
				HeightLoaded--;
				dataLen = headOffset + HeightLoaded * imageWidthLen;
				loadLen = dataLen;
			}
			else
			{
				AK_DEBUG_OUTPUT("no enough space to load.\r\n");
				return 0;
			}
			i = ImageRes_GetMod(loadLen);
		}
		
		cellOffset = ImageResoffset + ImageRes_DIV(imageWidthLen * vOffset);
#if 0
		//用多次512B获取数据
		num = ImageRes_DIV(loadLen);
		i = num;
		while (i > 0)
		{
			//tempbuf = data+(num-i)*FLASH_LOAD_CELL;
			tempbuf = data+ImageRes_MUL(num-i);
			temp = cellOffset+(num-i);
			if(0 >= ImageRes_LoadDatafromBin((T_U16)temp, tempbuf, FLASH_LOAD_CELL))
			{
				AK_DEBUG_OUTPUT("Eng_ImageResGetData error\r\n");
				return 0;
			}
			i--;
		}
#else
		//可获取整页数据
		if(0 >= ImageRes_LoadDatafromBinEx(cellOffset,data, loadLen))
        {
            AK_DEBUG_OUTPUT("ImageResGetDataEx IMAGE_WHOLE error! imgResID:%d\r\n",imgResID);
            return 0;
        }
#endif
		*dataStart = headOffset;
		*dataHight = HeightLoaded;
		return dataLen;
	}
	else if ((imagePart == IMAGE_MORETHANHALF)||(imagePart == IAMGE_LESSTHANHALF))
	{
		cutWidthLen = ImageResDataLenPerwidth(Width);
		HeightFeatReq = saveLen / cutWidthLen;
		HeightFeatReq = (Height>HeightFeatReq) ? HeightFeatReq : Height;
		
		while (HeightFilled < HeightFeatReq)
		{
			headOffset = ImageRes_GetMod(imageWidthLen*(vOffset+HeightFilled)+ImageResDataLenPerwidth(hOffset));

			if ((allLen-saveLen) >= (headOffset+cutWidthLen))
			{
				if (imagePart == IMAGE_MORETHANHALF)
				{
					temp = allLen - saveLen - headOffset - cutWidthLen;
					HeightLoaded = (T_U16)(temp / imageWidthLen);

					if (HeightLoaded > 0)
					{
						HeightLoaded = ((HeightFeatReq - HeightFilled)>HeightLoaded) ? \
							HeightLoaded : (HeightFeatReq - HeightFilled);
						loadLen = headOffset + cutWidthLen + HeightLoaded * imageWidthLen;
					}
					
					i = ImageRes_GetMod(loadLen);
					//修正512B块
					while (0 != i)
					{
						if ((allLen-loadLen) >= (FLASH_LOAD_CELL-i))
						{
							loadLen += (FLASH_LOAD_CELL-i);
						}
						else if (HeightLoaded >= 1)
						{
							HeightLoaded--;
							dataLen = headOffset + HeightLoaded * imageWidthLen;
							loadLen = dataLen;
						}
						else
						{
							AK_DEBUG_OUTPUT("no enough space to load.\r\n");
							return 0;
						}
						i = ImageRes_GetMod(loadLen);
					}

				}
				/*else
				{
					dataLen = headOffset + cutWidthLen;
					loadLen = dataLen;
					i = loadLen % FLASH_LOAD_CELL;
					if ((allLen-loadLen) >= (FLASH_LOAD_CELL-i))
					{
						loadLen += (FLASH_LOAD_CELL-i);
					}
					else
					{
						AK_DEBUG_OUTPUT("no enough space to load.\r\n");
						return 0;
					}
				}*/
			}
			else
			{
				AK_DEBUG_OUTPUT("no enough space to load.\r\n");
				return 0;
			}
	
			temp = ImageResDataLenPerwidth(hOffset) + imageWidthLen * (vOffset + HeightFilled);
			cellOffset = ImageResoffset + (T_U16)ImageRes_DIV(temp);
#if 0
			//用多次512B获取数据
			num = ImageRes_DIV(loadLen);
			i = num;
			while (i > 0)
			{
				tempbuf = data+saveLen+ImageRes_MUL(num-i);
				temp = cellOffset+(num-i);
				if(0 >= ImageRes_LoadDatafromBin((T_U16)temp, tempbuf, FLASH_LOAD_CELL))
				{
					AK_DEBUG_OUTPUT("Eng_ImageResGetData error\r\n");
					return 0;
				}
				i--;
			}
#else
			//可获取整页数据
			if(0 >= ImageRes_LoadDatafromBinEx(cellOffset, data+saveLen, loadLen))
	        {
				AK_DEBUG_OUTPUT("ImageResGetDataEx part error! imgResID:%d\r\n",imgResID);
	            return 0;
	        }
#endif
			tempbuf = data + saveLen + headOffset;
			memcpy(data + HeightFilled*cutWidthLen, tempbuf, cutWidthLen);
			HeightFilled++;
			if (imagePart == IMAGE_MORETHANHALF)
			{
				for (i=0; i<(HeightLoaded-1) && HeightLoaded>0; i++)
				{
					memcpy(data + HeightFilled*cutWidthLen, \
						tempbuf + ((i+1)*imageWidthLen), cutWidthLen);
					HeightFilled++;
				}
			}

		}
		*dataStart = 0;
		*dataHight = HeightFilled;	
		return HeightFilled*cutWidthLen;
	}
	return 0;
}

/******************************************************************************
 * @NAME    Eng_DrawResImage
 * @BRIEF   draw  image resource 
 * @AUTHOR  xuping
 * @DATE    2008-05-22
 * @PARAM 
 *		    x:appoint the position
 *          y:appoint the position
 *          DispWidth : the width of the rectangular area
 *          DispHeight: the height of the rectangular area
 *          data  : res image data
 *          format: 0 bit : reverse
 *                  1 bit :transparent
 * @RETURN AK_VOID
 *	
*******************************************************************************/

T_VOID Eng_DrawResImage(T_POS x, T_POS y, T_U16 DispWidth,T_U16 DispHeight,T_U8* data,T_U8 format)
{
    #if (USE_COLOR_LCD)
    if (format & IMG_TRANSPARENT)
    {
        //透明色的图片显示
        T_U16 i ,j;
        T_U16 *imgdata = (T_U16 *)data;

        for (i = 0; i < DispHeight; i++)
        {
            for (j = 0; j < DispWidth; j++)
            {
                if (IMG_TRNSPRNT_COLOR != imgdata[i*DispWidth + j])
                {
                    Fwl_SetPixel((T_POS)(x+j), (T_POS)(y+i), imgdata[i*DispWidth + j]);
                }
            }
        }
        
    }
    else
    #endif
    {
        Fwl_RefreshRect(x, y, DispWidth,DispHeight,data,(T_U8)(format&IMG_INVERTED));
    }
}

#ifdef OLD_LOADIMAGE
#if (USE_COLOR_LCD)
static T_VOID Eng_ImageResLineDisp(T_POS x, T_POS y, T_RES_IMAGE imgeresID, T_U16 DispWidth,T_U16 DispHeight, T_U16  hOffset,T_U16 vOffset,T_U8 format)
{
    T_U16	height	= 0;   
	T_U16	maxLine	=0;	
	T_U16 	offset      = 0;    //数据的偏移
	T_U32	firstOffset = 0;
	T_U16 	ImageResoffset    = 0;    //图片资源在文件中的偏移
	T_U32 	ReadDataoffset     = 0;    //已读取的数据块
    T_U8    *tempbuf            = AK_NULL;

    if (!((hOffset + DispWidth > 0) && (hOffset + DispWidth <= IMAGERES_WND_WIDTH)))
    {
        AK_DEBUG_OUTPUT("DispWidth error");
        return;
    }
    AK_ASSERT_VAL_VOID( (vOffset + DispHeight > 0 && vOffset + DispHeight <= IMAGERES_WND_HEIGHT), " DispHeight error");

    Fwl_FreqPush(FREQ_APP_MAX);


 	tempbuf = Gbl_GetCommBuff(); 
    ImageResoffset = gb_imageres.ImageResHeader[imgeresID].offset;

	if (vOffset != 0)
	{
	    //定位要读取数据的位置
	    firstOffset = vOffset*gb_imageres.ImageResHeader[imgeresID].width*2;
		ImageResoffset = (T_U16)(firstOffset/512 + ImageResoffset);
		firstOffset = firstOffset%512;
	}
		
    //定位要读取数据的位置
    do
    { 
       	//获取数据
       	if(0 >= ImageRes_LoadDatafromBin((T_U16)(ImageResoffset + ReadDataoffset), tempbuf + offset))
       	{
           	AK_DEBUG_OUTPUT("Eng_ImageResGetData error\r\n");
			break;
        }
        ReadDataoffset++;
		offset += 512;

		if (firstOffset != 0)
		{
			offset = (T_U16)(512 - firstOffset);
			memcpy(tempbuf, tempbuf + firstOffset, offset);
			firstOffset = 0;
		}
		
        maxLine = offset/(DispWidth*2);

        if (maxLine == 0)
        {
            continue;
        }

		if ((DispHeight <= maxLine)
			||((height+maxLine) > DispHeight))
		{
			Eng_DrawResImage(x, (T_POS)(y + height ) , DispWidth, \
				(T_U16)(DispHeight - height),tempbuf,format);	
			break;
		}

        //over disp area
		if (((x + DispWidth) > IMAGERES_WND_WIDTH)  ||\
		        ((y + height + maxLine) > IMAGERES_WND_HEIGHT))
		{
		    AK_DEBUG_OUTPUT("disp image res out of lcd:%d\r\n", imgeresID);
		    break;
		}
       	Eng_DrawResImage(x, (T_POS)(y + height ) , DispWidth, maxLine,tempbuf,format);

        
        height += maxLine;

		if (height >= DispHeight )
		{
			break;
		}

        offset = offset - DispWidth*maxLine*2;
        
        if (offset > 0)
        {             
		    memcpy(tempbuf, tempbuf + DispWidth*maxLine*2,offset);
        }
        
    }while(1);
	
    Fwl_FreqPop();
}
#endif

#endif
/******************************************************************************
 * @NAME    Eng_ImageResDispEx
 * @BRIEF   disp resourse image 
 * @AUTHOR  xuping
 * @DATE    2008-05-16
 * @PARAM 
 *		    x:appoint the position
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
T_VOID Eng_ImageResDispEx(T_POS x, T_POS y, T_RES_IMAGE imgResID, T_U16 DispWidth,T_U16 DispHeight, T_U16  hOffset,T_U16 vOffset,T_U8 format)
{
#ifdef OLD_LOADIMAGE
    T_U16    i                   = 0;
    T_U16    HeightPerUnit       = 0;
	T_U16    HeightDisped        = 0;
    T_U8    *tempbuf            = AK_NULL;

//    AK_ASSERT_VAL_VOID( (hOffset + DispWidth > 0 && hOffset + DispWidth <= IMAGERES_WND_WIDTH), "Eng_ImageResDispEx DispWidth error");
    if (!((hOffset + DispWidth > 0) && (hOffset + DispWidth <= IMAGERES_WND_WIDTH)))
    {
        AK_DEBUG_OUTPUT("Eng_ImageResDispEx DispWidth error:id:%d,dispwidth = %d,hOffset=%d",imgResID,DispWidth,hOffset);
        return;
    }
    AK_ASSERT_VAL_VOID( (vOffset + DispHeight > 0 && vOffset + DispHeight <= IMAGERES_WND_HEIGHT), "Eng_ImageResDispEx DispHeight error");
    //AK_ASSERT_VAL_VOID( (imgeresID < eRES_IMAGE_NUM), "Eng_ImageResDispEx imgeresID error");

	#if (USE_COLOR_LCD)
	if ((Gbl_GetLcdType() == 3 )
		&& (AK_NULL == gb_imageres.getDataFun))	//for QVGA
	{
		if ((hOffset == 0) 
			&& (DispWidth <= 240)
			&& (gb_imageres.ImageResHeader[imgResID].width == DispWidth))
		{
			Eng_ImageResLineDisp(x, y, imgResID, DispWidth, DispHeight,\
                       0, vOffset, format);
			return;
		}
	}
	#endif
    Fwl_FreqPush(FREQ_APP_MAX);
    
    tempbuf =  Gbl_GetCommBuff(); 

    HeightPerUnit = (COMBUF_SIZE_USED/(ImageResDataLenPerHeight(DispWidth)) * ImageResHeightUnit) > 0xFF ?\
                        0xFF : (COMBUF_SIZE_USED/(ImageResDataLenPerHeight(DispWidth)) * ImageResHeightUnit);

  	if (0 != HeightPerUnit)//512buf可以满足填充一行以上
    {
        HeightDisped = DispHeight/HeightPerUnit;// 最大限度的填充512buf
        for(; i < HeightDisped; i++)
        {
            if (x + DispWidth > IMAGERES_WND_WIDTH || (y + (i + 1) * HeightPerUnit) > IMAGERES_WND_HEIGHT)
            {
                AK_DEBUG_OUTPUT("disp image res out of lcd:%d\r\n", imgResID);
                Fwl_FreqPop();
                return ;
            }

            Eng_ImageResGetData(imgResID, DispWidth, HeightPerUnit,hOffset, (T_U16)(vOffset + i * HeightPerUnit),tempbuf);
            Eng_DrawResImage(x, (T_U16)(y + i * HeightPerUnit) , DispWidth, HeightPerUnit,tempbuf,format);
        }

        HeightDisped = DispHeight%HeightPerUnit;//剩余部分
        if (HeightDisped)
        {
            if (x + DispWidth > IMAGERES_WND_WIDTH || (y + i  * HeightPerUnit + HeightDisped) > IMAGERES_WND_HEIGHT)
            {
                AK_DEBUG_OUTPUT("diap image res out of lcd: %d\r\n", imgResID);
                Fwl_FreqPop();
                return ;
            }
        	Eng_ImageResGetData(imgResID, DispWidth, HeightDisped,hOffset, (T_U16)(vOffset + i * HeightPerUnit),tempbuf); 
            Eng_DrawResImage(x, (T_U16)(y + i * HeightPerUnit), DispWidth, HeightDisped,tempbuf,format);
        }
    }
    else//不足以填充一行
    {
        T_U16 tempDispWidth;
        tempDispWidth = COMBUF_SIZE_USED/ImageResDataLenPerHeight(1);
        
        Eng_ImageResDispEx(x, y, imgResID, tempDispWidth, DispHeight, hOffset,vOffset,format);
        Eng_ImageResDispEx((T_POS)(x + tempDispWidth), y , imgResID, (T_U16)(DispWidth - tempDispWidth), DispHeight, (T_U16)(hOffset + tempDispWidth),vOffset,format);
        
    }
    Fwl_FreqPop();
#else

	//截图显示情况
	T_U16    dataStart		 	 = 0;
    T_U16    HeightRequire       = 0;
	T_U16    HeightOffset        = 0;
    T_U16    HeightLoaded        = 0;
	T_U16    HeightDisped        = 0;
    T_U8    *tempbuf            = AK_NULL;
	T_U16    bufSize             = 0;
	T_U16    saveSize            = 0;

	tempbuf = LoadMem_GetRemainBuff(&bufSize);
	if (AK_NULL == tempbuf || bufSize < IMAGEBUF_MIDDLESIZE)//不小于2KB
	{
		AK_DEBUG_OUTPUT("error: no enough memory to load.");
		return ;
	}
	Fwl_FreqPush(FREQ_APP_MAX);
	saveSize = bufSize<(IMAGEBUF_MIDDLESIZE*2) ? IMAGEBUF_SMALLSIZE : IMAGEBUF_MIDDLESIZE;


	while (HeightDisped < DispHeight)
	{
        HeightRequire = DispHeight - HeightDisped;
		HeightOffset = vOffset + HeightDisped;

  	     if (0 == Eng_ImageResGetDataEx(imgResID,	DispWidth, HeightRequire, hOffset, HeightOffset,\
  	    	tempbuf, saveSize, bufSize, &dataStart, &HeightLoaded))
  	     {
			break;
		 }
		Eng_DrawResImage(x, (T_U16)(y + HeightDisped), DispWidth,\
			HeightLoaded, tempbuf+dataStart, format);
		
		HeightDisped += HeightLoaded;
	}
    Fwl_FreqPop();
    LoadMem_RemainBuffFree();

#endif
}

/******************************************************************************
 * @NAME    Eng_ImageResDisp
 * @BRIEF   disp rectangular area of the image resource 
 * @AUTHOR  xuping
 * @DATE    2008-05-19
 * @PARAM 
 *		    x:appoint the position
 *          y:appoint the position
 *          imgeresID: image resource ID
 *          format: 0 bit : reverse
 *                  1 bit :transparent
 * @RETURN AK_VOID
 *	
*******************************************************************************/

T_VOID Eng_ImageResDisp(T_POS x, T_POS y, T_RES_IMAGE imgResID,T_U8 format)
{   
#ifdef OLD_LOADIMAGE
	Eng_ImageResDispEx(x, y, imgResID,\
                       gb_imageres.ImageResHeader[imgResID].width,\
                       gb_imageres.ImageResHeader[imgResID].height,\
                       0,\
                       0,\
                       format);
#else
	
    //整张图片显示情况
	T_U16    dataStart		 	 = 0;	
	T_U16    imageHight       	 = 0;
	T_U16    imageWidth          = 0;
    T_U16    HeightRequire        = 0;
    T_U16    HeightLoaded        = 0;
	T_U16    HeightDisped        = 0;
    T_U8    *tempbuf            = AK_NULL;
	T_U16    bufSize             = 0;
	T_U16    saveSize            = 0;

	tempbuf = LoadMem_GetRemainBuff(&bufSize);
	if (AK_NULL == tempbuf || bufSize < IMAGEBUF_MIDDLESIZE)//不小于2KB
	{
		AK_DEBUG_OUTPUT("error: no enough memory to load.");
		return ;
	}
	Fwl_FreqPush(FREQ_APP_MAX);
	saveSize = bufSize<(IMAGEBUF_MIDDLESIZE*2) ? IMAGEBUF_SMALLSIZE : IMAGEBUF_MIDDLESIZE;


	imageWidth = gb_imageres.ImageResHeader[imgResID].width;
	imageHight = gb_imageres.ImageResHeader[imgResID].height;
    if ((x + imageWidth > IMAGERES_WND_WIDTH) || (y + imageHight > IMAGERES_WND_HEIGHT))
    {
        Fwl_FreqPop();
	    LoadMem_RemainBuffFree();
        AK_DEBUG_OUTPUT("Eng_ImageResDispEx DispWidth error:id:%d,x:%d,y:%d,w,%d,h%d",imgResID,x,y,imageWidth,imageHight);
        return;
    }

	while (HeightDisped < imageHight)
	{
        HeightRequire = imageHight - HeightDisped;

  	    if (0 == Eng_ImageResGetDataEx(imgResID,	imageWidth, HeightRequire, 0, HeightDisped,\
  	    	tempbuf, saveSize, bufSize, &dataStart, &HeightLoaded))
  	    {
			break;
		}
		Eng_DrawResImage(x, (T_U16)(y + HeightDisped), imageWidth,\
			HeightLoaded, tempbuf+dataStart, format);
		HeightDisped += HeightLoaded;
	}
    Fwl_FreqPop();
	LoadMem_RemainBuffFree();

#endif
}
/******************************************************************************
 * @NAME    Eng_GetResImageWidth
 * @BRIEF   get resource image width
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *		    ImageResbuf:the point to the image data buf
 * @RETURN the width of image
 *	
*******************************************************************************/
T_U16 Eng_GetResImageWidth(T_RES_IMAGE imgeresID)
{
    //AK_ASSERT_VAL( (imgeresID < eRES_IMAGE_NUM), "Eng_GetResImageWidth", 0);

    return gb_imageres.ImageResHeader[imgeresID].width; 
}

/******************************************************************************
 * @NAME    GetResImageheight
 * @BRIEF   get resource image height
 * @AUTHOR  xuping
 * @DATE    2008-04-07
 * @PARAM 
 *		    ImageResbuf:the point to the image data buf
 * @RETURN the height of image
 *	
*******************************************************************************/
T_U16 Eng_GetResImageHeight(T_RES_IMAGE imgeresID)
{
    //AK_ASSERT_VAL( (imgeresID < eRES_IMAGE_NUM), "Eng_GetResImageHeight", 0);

    return gb_imageres.ImageResHeader[imgeresID].height;     
}

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
 T_U32 Eng_ImageResGetDataAll(T_RES_IMAGE imgResID, T_U8* DestBuf)
{
#ifdef OLD_LOADIMAGE
    T_U16    i                   = 0;
    T_U16    HeightPerUnit       = 0;
    T_U16    HeightDisped        = 0;
    T_U8    *tempbuf            = AK_NULL;
    T_U16   width, height;
    T_U32   offset  = 0;
 

    //AK_ASSERT_VAL( (imgeresID < eRES_IMAGE_NUM), "Eng_ImageResDispEx imgeresID error",0);
    
    tempbuf =  Gbl_GetCommBuff(); 
    width  = Eng_GetResImageWidth(imgResID);
    height = Eng_GetResImageHeight(imgResID);
    
    HeightPerUnit = (T_U16)((COMBUF_SIZE_USED/(ImageResDataLenPerHeight(width)) * ImageResHeightUnit) > 0xFF ?\
                        0xFF : (COMBUF_SIZE_USED/(ImageResDataLenPerHeight(width)) * ImageResHeightUnit));
 
    if (0 != HeightPerUnit)
    {
    HeightDisped = (T_U16)height/HeightPerUnit;// 最大限度的填充512buf
    for(; i < HeightDisped; i++)
    {
	        Eng_ImageResGetData(imgResID, width, HeightPerUnit,0, (T_U16)(i * HeightPerUnit),tempbuf);
        memcpy(DestBuf + offset,tempbuf, width*ImageResDataLenPerwidth(HeightPerUnit));
        offset += width*ImageResDataLenPerwidth(HeightPerUnit);
    }
 
    HeightDisped = (T_U16)height%HeightPerUnit;//剩余部分
    if (HeightDisped)
    {
        Eng_ImageResGetData(imgResID, width, HeightDisped,0, (T_U16)(i * HeightPerUnit),tempbuf);
        memcpy(DestBuf + offset,tempbuf, width*ImageResDataLenPerwidth(HeightDisped));
        offset += width*ImageResDataLenPerwidth(HeightDisped);  
        }
    }
    else//不足以填充一行
    {
        T_U16 temp;
        temp = (T_U16)(COMBUF_SIZE_USED/(ImageResDataLenPerHeight(width/2)) * ImageResHeightUnit);
        if (0 == temp)
        {
            AK_DEBUG_OUTPUT("image too arge,get image data error\r\n");
        }
        else
        {
            for(i = 0; i < (height/ImageResHeightUnit); i++)
            {
                Eng_ImageResGetData(imgResID, (T_U16)(width/2), ImageResHeightUnit,0, (T_U16)(i * ImageResHeightUnit),tempbuf);
                memcpy(DestBuf + offset,tempbuf, (width/2)*ImageResDataLenPerwidth(ImageResHeightUnit));
                offset += width/2*ImageResDataLenPerwidth(ImageResHeightUnit);  
                Eng_ImageResGetData(imgResID, (T_U16)(width - width/2), ImageResHeightUnit,(T_U16)(width/2), (T_U16)(i * ImageResHeightUnit),tempbuf);
                memcpy(DestBuf + offset,tempbuf, (width- width/2)*ImageResDataLenPerwidth(ImageResHeightUnit));
                offset += (width- width/2)*ImageResDataLenPerwidth(ImageResHeightUnit);  
            }
        }
        
        
    }
 
    return offset;
#else
	//整张图片获取情况
	T_U16	 dataStart			= 0;	
	T_U16	 imageHight 		= 0;
	T_U16	 imageWidth 		= 0;
	T_U16	 HeightRequire		= 0;
	T_U16	 HeightLoaded		= 0;
	T_U16	 HeightSaved		= 0;
	T_U8	*tempbuf			= AK_NULL;
	T_U32    lenLoaded          = 0;
	T_U32    length             = 0;
	T_U16    bufSize             = 0;
	T_U16    saveSize            = 0;

	tempbuf = LoadMem_GetRemainBuff(&bufSize);
	if (AK_NULL == tempbuf || bufSize < IMAGEBUF_MIDDLESIZE)//不小于2KB
	{
		AK_DEBUG_OUTPUT("error: no enough memory to load.");
		return AK_FALSE;
	}
	Fwl_FreqPush(FREQ_APP_MAX);
	saveSize = bufSize<(IMAGEBUF_MIDDLESIZE*2) ? IMAGEBUF_SMALLSIZE : IMAGEBUF_MIDDLESIZE;
	
	imageWidth = gb_imageres.ImageResHeader[imgResID].width;
	imageHight = gb_imageres.ImageResHeader[imgResID].height;
	
	while (HeightSaved < imageHight)
	{
		HeightRequire = imageHight - HeightSaved;

		if (0 == Eng_ImageResGetDataEx(imgResID, imageWidth, HeightRequire, 0, HeightSaved,\
			   tempbuf, saveSize, bufSize, &dataStart, &HeightLoaded))
		{
			length = 0;
			break;
		}
		lenLoaded = HeightLoaded * ImageResDataLenPerwidth(imageWidth);
		memcpy(DestBuf + length, tempbuf+dataStart, lenLoaded);
		HeightSaved += HeightLoaded;
		length += lenLoaded;
	}
	Fwl_FreqPop();
    LoadMem_RemainBuffFree();
	
	return length;
#endif
}


T_VOID Eng_ImageRsgtFun(IMAFERES_CALLBACK_GETDATA callbackFun)
{
    gb_imageres.getDataFun = callbackFun;     
}

#endif


