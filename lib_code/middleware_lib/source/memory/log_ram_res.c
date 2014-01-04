/******************************************************************************************
**FileName  	:      log_ram_res.c
**brief        	:      ram resource handle
**Copyright 	:      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author 		:	Hongshi  Yao
**date		: 	2008-11-05
**version 	:	1.0
*******************************************************************************************/
#include "Gbl_global.h"
//#include "Gbl_ImageRes.h"
//#include "Eng_Font.h"
//#include "Eng_ImageResDisp.h"
#include "log_ram_res.h"
#include "Fwl_osMalloc.h"
#include <stdlib.h>
#include <string.h>

#if(NO_DISPLAY == 0)

static T_RAM_RES * pRamRes = AK_NULL;
/******************************************************************************
 * @NAME    Ram_ResStructInit
 * @BRIEF   initialize ram resource data struct 
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *	
*******************************************************************************/
T_BOOL  Ram_ResStructInit(T_RAM_RES *dat)
{
    pRamRes = dat;
    pRamRes->bRamFontRes = AK_FALSE;
    pRamRes->bRamImageRes= AK_FALSE;
    pRamRes->pImg        = AK_NULL;
    pRamRes->pMat        = AK_NULL;

    return AK_TRUE;

}

/******************************************************************************
 * @NAME    Ram_GetFontMatrix
 * @BRIEF   Get FontMatrix  and abc of the unicode char from ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *		    char: unicode char
 *          FontMatrix: store the font bitmap
 *          fontAbc:store abc information          
 * @RETURN 
 *      T_BOOL :AK_TRUE,get success;AK_FLASE,get failed
 *	
*******************************************************************************/
T_BOOL Ram_GetFontMatrix(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format)
{
    T_MATRIX    *ptemp;
    T_BOOL      bOK = AK_FALSE;

    if(pRamRes == AK_NULL)
        return AK_FALSE;

    ptemp = pRamRes->pMat;
    while((ptemp!= AK_NULL)&&(ch != ptemp->un_code))
        ptemp = ptemp->pnext;

    if(ptemp != AK_NULL)
    {
        memcpy(FontMatrix,(T_U8 *)ptemp->mat, MAT_LEN);
        memcpy(fontABC,&(ptemp->abc), sizeof(T_FONT_ABC));
        bOK = AK_TRUE;
    }else
    {
        bOK = AK_FALSE;
    }
    
    return bOK;

}


/******************************************************************************
 * @NAME    Ram_StrResInit
 * @BRIEF   get string'Matrix to ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *		    StrStart:  start string ID 
 *             StrEnd:    the last string ID
 *          fontAbc:store abc information          
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *	
*******************************************************************************/
T_BOOL  Ram_StrResInit(T_RES_STRING StrStart, T_RES_STRING StrEnd)
{
    T_U16       strlen = 0;
    T_U16       i;
    T_U16       *pStr = AK_NULL;
    T_MATRIX    *pm =AK_NULL;
    T_MATRIX    *ptemp;
	T_U8		*pbuf = AK_NULL;

    //if ID error return
    if((StrEnd < StrStart)||(pRamRes == AK_NULL))
        return AK_FALSE;


    if(pRamRes->pMat == AK_NULL)
        pRamRes->bRamFontRes = AK_TRUE;

    //alloc ram for get string (unicode)
    pStr = (T_U16 *)Fwl_Malloc(STR_MAX_LEN);
	pbuf = (T_U8 *)Fwl_Malloc(512);
    
    for(i = 0; i < (StrEnd - StrStart + 1); i++)
    {
		memset(pStr,0, STR_MAX_LEN);
        strlen = GetResString((T_RES_STRING)(StrStart+i), pStr, STR_MAX_LEN);
        if(strlen == 0)
            continue;

        while(strlen--)
        {
            if(pRamRes->pMat != AK_NULL)
            {   //查找链表中是否存在当前字符的字模
				ptemp =  pRamRes->pMat;
                while((ptemp != AK_NULL)
                        &&(pStr[strlen] != ptemp->un_code))
                ptemp = ptemp->pnext;

                if((ptemp != AK_NULL)&&(pStr[strlen] == ptemp->un_code))
                    continue;
            }
            //new a node a char and initialize it
            Eng_FontRsgtFun(AK_NULL);
            ptemp = (T_MATRIX    *)Fwl_Malloc(sizeof(T_MATRIX));
            memset(ptemp, 0,sizeof(T_MATRIX));
            ptemp->un_code = pStr[strlen];
            GetFontMatrix(pStr[strlen], pbuf, &(ptemp->abc), 0);
			memcpy(ptemp->mat, pbuf,32);
            Eng_FontRsgtFun(Ram_GetFontMatrix);
            
            if(pRamRes->pMat == AK_NULL)
                pRamRes->pMat = ptemp;
            else
            {
                pm = pRamRes->pMat;
                while(pm->pnext != AK_NULL)
                    pm = pm->pnext;

                pm->pnext = ptemp;
            }
         }
  
	}

    Eng_FontRsgtFun(Ram_GetFontMatrix);
    pStr =(T_U16 *) Fwl_Free(pStr);
	pbuf = (T_U8 *) Fwl_Free(pbuf);
    return AK_TRUE;
}

/******************************************************************************
 * @NAME    Ram_StrResFree(T_VOID)
 * @BRIEF   release string matrix resource in ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM  MO
 * @RETURN 
 *      T_BOOL :AK_TRUE,free success;AK_FLASE,free failed
 *	
*******************************************************************************/
T_BOOL Ram_StrResFree(T_VOID)
{
    T_MATRIX    *ptemp = AK_NULL;
    T_MATRIX    *tail = AK_NULL;

    if((pRamRes == AK_NULL)||(pRamRes->pMat == AK_NULL))
        return AK_TRUE;

    
    Eng_FontRsgtFun(AK_NULL);
    pRamRes->bRamFontRes = AK_FALSE;

    ptemp = pRamRes->pMat;
    tail = pRamRes->pMat->pnext;

    while((ptemp != AK_NULL)||(tail != AK_NULL))
    {
        ptemp = (T_MATRIX *)Fwl_Free(ptemp);
        if(tail)
        {
            ptemp = tail;
            tail = tail->pnext;
        }
    }

    pRamRes->pMat = AK_NULL;
    return AK_TRUE;

}


/******************************************************************************
 * @NAME    Ram_ImageResGetData
 * @BRIEF   get image resource from ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *          imgeresID:res image id
 *          DestBuf: buf to store image
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
*******************************************************************************/
T_BOOL  Ram_ImageResGetData(T_RES_IMAGE imgeResID, T_U16 Width, T_U16 Height,T_U16 hOffset, T_U16 vOffset,T_U8 data[])
{   
    T_IMAGE  *ptemp = AK_NULL;
    T_U8     *srcdat =AK_NULL;
    T_BOOL   bOK =AK_FALSE;
    

    if((pRamRes == AK_NULL)||pRamRes->pImg == AK_NULL)
        return AK_FALSE;
    
    ptemp = pRamRes->pImg;
    while((ptemp != AK_NULL) && (imgeResID != ptemp->ID))
        ptemp = ptemp->pnext;

    if(ptemp != AK_NULL)
    {
        T_U32 i,len,Limtlen,offset;

        srcdat = ptemp->pImage;
        
        #if USE_COLOR_LCD
        len    = (T_U32)Width*Height*2;
        Limtlen = (T_U32)Width*2;
        #else
        len    = (T_U32)(Width*Height/8);
        Limtlen =(T_U32)Width/8;
        #endif

        for(i=0; i < Height; i++ )
        {
            #if USE_COLOR_LCD
            offset = ((vOffset+i) *  Eng_GetResImageWidth(imgeResID) + hOffset)*2;
            #else
            offset = ((vOffset+i) * Eng_GetResImageWidth(imgeResID) + hOffset)/8;   
            #endif    
        
            memcpy(data+((Limtlen > 512)?512:Limtlen)*i, \
                        srcdat+offset,\
                        (unsigned int)((Limtlen > 512)?512:Limtlen) );
        
            if(len <= ((Limtlen > 512)?512:Limtlen *(i+1)))
                break;
            
        }
        
        bOK = AK_TRUE;
    }else
    {
        bOK = AK_FALSE;
    }

    return bOK;
}




/******************************************************************************
 * @NAME    Ram_ImageResInit
 * @BRIEF   get image resource to ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *		    pImageID:  image ID pointer, 
 *               dat:   ram image data struct pointer
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *	
 *    note:  ID must end with eRES_IMAGE_NUM
*******************************************************************************/
T_BOOL Ram_ImageResInit(T_U32 *pImageID)
{
    T_IMAGE  *pImg = AK_NULL;
    T_IMAGE  *pm   = AK_NULL;
    T_U8*    pdata = AK_NULL;
    T_U32   ResLen = 0;
    T_U16    hight,width;
    T_U16   i=0;

	
    if((pImageID == AK_NULL)||(pRamRes == AK_NULL))
        return AK_FALSE;

	if(pRamRes->pImg == AK_NULL)
		pRamRes->bRamImageRes = AK_TRUE;
	
    for(i=0; pImageID[i] < eRES_IMAGE_NUM; i++)
    {
        if(pRamRes->pImg != AK_NULL)
        {   //查找链表中是否存在当前字符的字模
            pImg =  pRamRes->pImg ;
            while((pImg != AK_NULL)
                &&(pImageID[i] != pImg->ID))
            pImg = pImg->pnext;
        
            if((pImg!=AK_NULL)&&(pImageID[i] == pImg->ID))
                continue;
        }


        Eng_ImageRsgtFun(AK_NULL);
        //获取图片资源
        width = Eng_GetResImageWidth((T_RES_IMAGE)pImageID[i]);
        hight = Eng_GetResImageHeight((T_RES_IMAGE)pImageID[i]);
        #if USE_COLOR_LCD
        ResLen = (T_U32)width*hight*2;
        #else
        ResLen = (T_U32)(width*hight/8+1);
        #endif
        pdata = (T_U8*)Fwl_Malloc(ResLen);  
        memset(pdata,0,(unsigned int)ResLen);
        Eng_ImageResGetDataAll((T_RES_IMAGE)pImageID[i],pdata);
        Eng_ImageRsgtFun(Ram_ImageResGetData);

        //new a node a image ram data struct  and initialize it
        pImg = (T_IMAGE *)Fwl_Malloc(sizeof(T_IMAGE));
        memset(pImg, 0,sizeof(T_IMAGE));
        pImg->ID     = (T_U16)pImageID[i];
        pImg->pImage = pdata;
        pImg->pnext  = AK_NULL;

        if(pRamRes->pImg == AK_NULL)
            pRamRes->pImg = pImg;
        else
        {
            pm = pRamRes->pImg;
            while(pm->pnext != AK_NULL)
                pm = pm->pnext;
            
            pm->pnext = pImg;
        }
    }

    Eng_ImageRsgtFun(Ram_ImageResGetData);

    return AK_TRUE;
}



/******************************************************************************
 * @NAME    Ram_ImageResFree
 * @BRIEF   free image resource in ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *		    pImageID:  image ID pointer, 
 *               dat:   ram image data struct pointer
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *	
 *    note:  ID must end with eRES_IMAGE_NUM
*******************************************************************************/

T_BOOL Ram_ImageResFree(T_VOID)
{
    T_IMAGE *ptemp = AK_NULL;
    T_IMAGE *tail  = AK_NULL;

    if((pRamRes == AK_NULL)||(pRamRes->pImg == AK_NULL))
        return AK_TRUE;
    
    Eng_ImageRsgtFun(AK_NULL) ;
    pRamRes->bRamImageRes = AK_FALSE;


    ptemp = pRamRes->pImg;
    tail = pRamRes->pImg->pnext;

    while((ptemp != AK_NULL)||(tail != AK_NULL))
    {
        ptemp->pImage = (T_U8 *)Fwl_Free(ptemp->pImage);
        ptemp = (T_IMAGE *)Fwl_Free(ptemp);
        if(tail)
        {
            ptemp = tail;
            tail = tail->pnext;
        }
    }

    pRamRes->pImg = AK_NULL;
    return AK_TRUE;


}

#endif

/* end of files */


