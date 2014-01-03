/******************************************************************************************
**FileName      :      log_ram_res.h
**brief         :      NO
**Copyright     :      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author        :   Hongshi Yao
**date      :   2008-11-05
**version   :   1.0
*******************************************************************************************/
#ifndef _LOG_RAM_RES_H
#define _LOG_RAM_RES_H

#include "anyka_types.h"
#include "Eng_Font.h"
#include "Gbl_ImageRes.h"

#define MAT_LEN     32
#define STR_MAX_LEN  256        //UNICODE

typedef  struct matrix{
    struct matrix *pnext;
        T_U16      un_code;
        T_U8       mat[MAT_LEN];
        T_FONT_ABC abc;
}T_MATRIX;

typedef  struct image{
    struct image   *pnext;
        T_U16      ID; 
        T_U8       *pImage;
}T_IMAGE;


typedef  struct tag_ramres{
  T_MATRIX *pMat;
  T_IMAGE  *pImg; 

  T_BOOL   bRamImageRes;
  T_BOOL   bRamFontRes;

}T_RAM_RES;

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
T_BOOL  Ram_ResStructInit(T_RAM_RES *dat);


/******************************************************************************
 * @NAME    Ram_StrResInit
 * @BRIEF   get string'Matrix to ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *          StrStart:  start string ID 
 *             StrEnd:    the last string ID
 *          fontAbc:store abc information          
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *  
*******************************************************************************/
T_BOOL  Ram_StrResInit(T_RES_STRING StrStart, T_RES_STRING StrEnd);


/******************************************************************************
 * @NAME    Ram_GetFontMatrix
 * @BRIEF   Get FontMatrix  and abc of the unicode char from ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *          char: unicode char
 *          FontMatrix: store the font bitmap
 *          fontAbc:store abc information          
 * @RETURN 
 *      T_BOOL :AK_TRUE,get success;AK_FLASE,get failed
 *  
*******************************************************************************/
T_BOOL Ram_GetFontMatrix(T_U16 ch, T_U8* FontMatrix, T_FONT_ABC* fontABC, T_U32 format);


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
T_BOOL Ram_StrResFree(T_VOID);


/******************************************************************************
 * @NAME    Ram_ImageResInit
 * @BRIEF   get image resource to ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *          pImageID:  image ID pointer, 
 *               dat:   ram image data struct pointer
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *  
 *    note:  ID must end with eRES_IMAGE_NUM
*******************************************************************************/
T_BOOL Ram_ImageResInit(T_U32 *pImageID);


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
T_BOOL  Ram_ImageResGetData(T_RES_IMAGE imgeResID, T_U16 Width, T_U16 Height,T_U16 hOffset, T_U16 vOffset,T_U8 data[]);



/******************************************************************************
 * @NAME    Ram_ImageResFree
 * @BRIEF   free image resource in ram
 * @AUTHOR  Yao Hongshi
 * @DATE    2008-11-05
 * @PARAM 
 *          pImageID:  image ID pointer, 
 *               dat:   ram image data struct pointer
 * @RETURN 
 *      T_BOOL :AK_TRUE,init success;AK_FLASE, init failed
 *  
 *    note:  ID must end with eRES_IMAGE_NUM
*******************************************************************************/
T_BOOL Ram_ImageResFree(T_VOID);

#endif


//end  of file

