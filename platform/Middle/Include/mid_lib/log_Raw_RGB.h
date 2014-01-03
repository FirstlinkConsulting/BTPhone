/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME log_Raw_RGB.h
 * @BRIEF the file provide the api for raw_rgb application
 * @Author：li_shengkai
 * @Date：2011-11
 * @Version：
**************************************************************************/

#ifndef  __LOG_RAW_RGB_H__
#define  __LOG_RAW_RGB_H__

#include "Anyka_types.h"
#include "Fwl_osFS.h"
#include "Vme.h"


#define  RAW_BUF_A  0
#define  RAW_BUF_B  1       //判断rawBuf是A还是B 

typedef enum
{
    RAW_RGB_START = 1,
    RAW_RGB_COPYING,
    RAW_RGB_COPYED,
    RAW_RGB_ERROR,
    RAW_RGB_QUIT,
    RAW_RGB_INVALID
}T_RAW_RGB_STATE;

typedef enum
{
    RAW_RGB_HANLDE_OPERATE_NONE = 10,
    RAW_RGB_HANLDE_QUIT,
    RAW_RGB_HANLDE_SAVE,
    RAW_RGB_HANLDE_STARTED,
    RAW_RGB_HANLDE_INIT_ERR,
    RAW_RGB_HANLDE_INVALID
}T_RAW_RGB_HANDLE_STATE;

typedef struct
{
    T_BOOL                      bclose;
    T_U8*                       pbufa;          //乒乓buf的指针
    T_U8*                       pbufb;          //乒乓buf的指针
    T_hFILE                     raw_rgb_file;   //文件句柄
    T_U32                       cap_width;      //setsize的宽
    T_U32                       cap_height;     //setsize的高
    T_U16                       path[MAX_FILE_LEN];
}T_RAW_RGB_INFO_EX, *P_RAW_RGB_INFO_EX;


typedef  struct
{
    //volatile T_RAW_RGB_STATE      raw_rgb_state;
    //volatile T_BOOL               bkey_out;
    volatile T_U8               raw_rgb_state:4;
    volatile T_U8               bkey_out:1;
    volatile T_U8               rawBuf:2;
    T_U8                        binit_ok:1;
    //T_U8*                         dmabufa;            //乒乓buf的物理
    //T_U8*                         dmabufb;            //乒乓buf的物理
    //T_U8*                         dmabuf_cur;         //当前buf指针
    T_U32                       dmabuf[2];          //当前buf指针
    P_RAW_RGB_INFO_EX           P_RAW_INFO;
}T_RAW_RGB_INFO,  *P_RAW_RGB_INFO;  


T_BOOL  Raw_RGB_Init(T_U16 *path, T_U32 width, T_U32 height);
//T_BOOL  Raw_RGB_Set_Szie(T_U32 width, T_U32 height);
T_BOOL  Raw_RGB_Open(T_VOID);
T_RAW_RGB_HANDLE_STATE  Raw_RGB_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
T_BOOL  Raw_RGB_Close(T_VOID);

T_BOOL  Raw_RGB_Save(T_VOID);
T_BOOL Raw_RGB_Start_Next_file(T_VOID);

T_BOOL  Raw_RGB_Destroy(T_VOID);
T_RAW_RGB_STATE  Raw_RGB_Get_state(T_VOID);

T_BOOL  Raw_RGB_Get_Close_state(T_VOID);

T_BOOL Raw_RGB_Get_init_state(T_VOID);

#endif
