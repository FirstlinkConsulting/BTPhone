/**
 * @file    log_aud_play.c
 * @brief   audio play manager
 * @author  WuShanwei
 * @date    2008-04-03
 * @version 1.0
 */
#include "Gbl_Global.h"
#include "log_aud_play.h"
#include "audio_lib.h"
#include "Fwl_osMalloc.h"
#include <string.h>
//#include "m_state.h"
#include "log_record.h"
#include "Fwl_System.h"
#include "eng_string_uc.h"
#include "media_lib.h"
#include "Lib_avf_player.h"
#include "Fwl_Timer.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "Eng_Debug.h"
#include "Fwl_MicroTask.h"
#include "log_pcm_player.h"
#include "sdfilter.h"


#ifdef OS_WIN32
#include "w_audioplay.h"
#else
//#include "mmu.h"
#endif


#ifdef SUPPORT_MUSIC_PLAY

#define CBPTR_STACK_DEPTH   3 
#define DISCARD_WAVEOUTDATA_SIZE   (5*1024)

typedef enum
{
    AUD_CB_PRE = 0,
    AUD_CB_POST,
    AUD_CB_NUM
}T_AUD_CB_TYPE;

typedef struct
{
    T_U32 cbPtr[CBPTR_STACK_DEPTH];
    T_U8  pc;
}T_AUD_CBFUN_STACK;

#define SND_DATA_LEN 4096

extern volatile T_BOOL bSendZeroData;
static volatile T_U8   gb_AudioReadFlag = 0x00;

/******************************************************************************
 * @NAME    remap_lock_page
 * @BRIEF   lock virtual address in physical page
 * @AUTHOR  xuping
 * @DATE    2010-02-28;
 * @PARAM   vaddr:the virtual address which will be locked(enlocked)
 *          size: size of  be locked(unlocked)
 *          lock:AK_TRUE ,lock  AK_FALSE: unlock
 * @RETURN  AK_TRUE:success AK_FALSE:fail
*******************************************************************************/
extern T_BOOL remap_lock_page(T_U32 vaddr, T_U32 size, T_BOOL lock);

static volatile AUDPLAYER_CALLBACK_HANDLEPRE Aud_PlayerHandlePre = AK_NULL;
static volatile AUDPLAYER_CALLBACK_HANDLEPOST Aud_PlayerHandlePost = AK_NULL;
static T_AUD_CBFUN_STACK gAudCBStack[AUD_CB_NUM] = {0};
static T_U32 gAudMeidaCount = 0;//媒体库的初始化计数器,解决多个媒体句柄同时打开的情况

#pragma arm section zidata = "_bootbss_"
static volatile T_U8 Mutex_flag;    //解码互斥保护
#pragma arm section zidata 

/** volume index to value table */
static const T_U32 volIdx2ValTbl[AUD_PLAY_SETG_MAX_VOL_STEP] = {
    0,100,300,600,800,950,1024,1500,1800,2400};
    
static const T_U8 speedIdx2ValTbl[][AUD_PLAY_SETG_MAX_SPEED_STEP] = { 
{0/*0.5*/,1,2,3,4,5/*1.0*/,6,7,8,9/*1.4*/},
{0/*0.5*/,1,2,3,4,5/*1.0*/,6,7,8,9/*1.4*/}
};

#ifdef SUPPORT_BLUETOOTH
/* audio freq table */
static const T_U8 freqNeedTable[MAX_AUD_TYPE_ELE][MAX_AUD_BITSAMP_ELE][MAX_AUD_EQ_ELE][MAX_AUD_BL_ELE] = 
{   PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ3,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ4,PLAY_FREQ3,PLAY_FREQ4,PLAY_FREQ4, //mp3
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, //wma
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4, //wav
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, //ape
    PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, //ogg
};
#else
/* audio freq table */
static const T_U8 freqNeedTable[MAX_AUD_TYPE_ELE][MAX_AUD_BITSAMP_ELE][MAX_AUD_EQ_ELE][MAX_AUD_BL_ELE] = 
{   PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ4,PLAY_FREQ3,PLAY_FREQ4,PLAY_FREQ4, //mp3
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ3,PLAY_FREQ5,PLAY_FREQ5, //wma
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ3,PLAY_FREQ2,PLAY_FREQ4,PLAY_FREQ4, //wav
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, //ape
    PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4,PLAY_FREQ4, 
    PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5,PLAY_FREQ5, //ogg
};
#endif
#pragma arm section rodata = "_audioplayer_"
//客户要求
const T_U16 TabVolDig[MAX_VOLUME + 1] = {
    0, 7, 11, 16, 23, 32, 43, 56, 70, 86,
    104, 124, 146, 169, 189, 216, 245, 270, 300, 335,
    371, 409, 450, 491, 535, 580, 628, 677, 727, 804, 
    884, 950 };
#pragma arm section rodata 

#ifdef OS_ANYKA
#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)

extern T_U32        Image$$audioplayer_resident$$Base;
#define AUDIO_SECTION_BASE ((T_U32)&Image$$audioplayer_resident$$Base)
extern T_U32        Image$$audioplayer_resident$$Limit;
#define AUDIO_SECTION_LIMIT ((T_U32)&Image$$audioplayer_resident$$Limit)

#endif
#endif

T_BOOL TstClearDacInt(T_U8 DevID);
T_BOOL TstClearAdcInt(T_U8 DevID);
T_VOID aud_player_registertask(T_AUD_PLAYER* player, T_U16 readInterval, T_U16 decodeInterval);
T_VOID aud_player_unregistertask(T_AUD_PLAYER* player);
T_VOID aud_player_restarttask(T_AUD_PLAYER* player);
T_VOID aud_player_settask(T_AUD_PLAYER* player);
T_BOOL Aud_PlayerMediaLibInit(T_VOID);
T_BOOL Aud_PlayerMediaLibDestroy(T_VOID);
T_BOOL Aud_PlayerCfgComp(T_AUD_PLAYER* player);
T_BOOL Aud_PlayerSetDAInfo(T_AUD_PLAYER* player);
T_BOOL Aud_PlayerFirstDec(T_AUD_PLAYER* player);
T_S32  Aud_PlayerDecodeHandle(T_AUD_PLAYER* player);
T_VOID APlayer_PauseTask(T_AUD_PLAYER* player);
T_VOID aud_player_read(T_VOID);


#pragma arm section code = "_music_playing_"
/**
 * @brief   audio decode data
 * @author  mayeyu
 * @date          2012-7-4
 * @param   T_VOID
 * @return  T_VOID
 **/
T_VOID aud_player_decode(T_VOID)
{
    T_AUD_PLAYER* player = AK_NULL;     //播放信息结构
    T_S32 size           = 0;           //实际解码量

    if (AK_NULL == Aud_PlayerHandlePre)
    {
        return;
    }

    //在音频播放下，如果不在播放状态Aud_PlayerHandlePre返回AK_NULL
    player = Aud_PlayerHandlePre();
    if ((AK_NULL == player)
        || (FS_INVALID_HANDLE != player->hVideoFile))   //视频播放不跑此流程
    {
        return;
    }

    //防止大循环中音频数据的读取被耽搁，导致无数据解码
    if (gb_AudioReadFlag++ >= 0x09)
    {
        aud_player_read();
    }

    size = Aud_PlayerDecodeHandle(player);
    if (AK_NULL != Aud_PlayerHandlePost)
    {
        Aud_PlayerHandlePost(size, player);
    }
    if (size < 0)
    {
        APlayer_PauseTask(player);
    }
}
#pragma arm section code


#if(STORAGE_USED == NAND_FLASH|| STORAGE_USED == SD_CARD)
#pragma arm section code = "_audioplayer_resident_"
#else
#pragma arm section code = "_bootcode1_"
#endif
/**
 * @brief   read audio file data
 * @author  mayeyu
 * @date          2012-7-4
 * @param   T_pVOID:hMedia
 * @return  T_VOID
 **/
T_VOID aud_player_read(T_VOID)
{
    T_AUD_PLAYER* player = AK_NULL;     //播放信息结构
    T_S32 len = 0;

    if (AK_NULL == Aud_PlayerHandlePre)
    {
        return;
    }

    //在音频播放下，如果不在播放状态Aud_PlayerHandlePre返回AK_NULL
    player = Aud_PlayerHandlePre();
    if ((AK_NULL == player)
        || (AK_NULL == player->hMedia)
        || (FS_INVALID_HANDLE != player->hVideoFile))   //视频播放不跑此流程
    {
        return;
    }

    len = MediaLib_GetAudio_DataLen(player->hMedia);
    if (len < 0)
    {
        akerror("read end!", len, 1);
    }
    else
    {
        if ((T_U32)len >= player->expReadLen)
        {
            return;
        }

        if ((T_U32)len <= player->emptyDecLen)
        {
            Fwl_ConsoleWriteChr('^');
        }

        gb_AudioReadFlag = 0;   //防止下面的函数在主循环里执行过程中
                                //被中断打断，然后在微任务中嵌套调用
        MediaLib_ReadAudio_Data(player->hMedia, (player->streamBufLen)-len);
    }
}
#pragma arm section code

/**
 * @brief     register audio player processing task
 * @author  mayeyu
 * @date    2012-7-6
 * @param  T_AUD_PLAYER*:player
 * @param  T_U16: decodeInterval
 * @param  T_U16: readInterval
 * @return  T_VOID
 * @retval  
 **/
T_VOID aud_player_registertask(T_AUD_PLAYER* player, T_U16 readInterval, T_U16 decodeInterval)
{
    player->decodetask_id = Fwl_MicroTaskRegister(aud_player_decode, decodeInterval);         //注册音频任务处理函数
    //player->readtask_id = Fwl_MicroTaskRegister(aud_player_read, readInterval);               //注册音频任务处理函数
}

/**
 * @brief         unregister audio player processing task
 * @author  mayeyu
 * @date          2012-7-6
 * @param   T_AUD_PLAYER*:player
 * @return  T_VOID
 * @retval  
 **/
T_VOID aud_player_unregistertask(T_AUD_PLAYER* player)
{
    //if (Fwl_MicroTaskUnRegister(player->readtask_id))              //销毁音频任务处理函数
    {
        //akerror("Fwl_MicroTaskUnRegister readtask success", 0, 1);
        player->readtask_id = 0xff;
    }

    if (Fwl_MicroTaskUnRegister(player->decodetask_id))              //销毁音频任务处理函数
    {
        akerror("Fwl_MicroTaskUnRegister decodetask success", 0, 1);
        player->decodetask_id = 0xff;
    }
}

/**
 * @brief         start audio player processing task
 * @author  mayeyu
 * @date          2012-7-6
 * @param   T_AUD_PLAYER*:player
 * @return  T_VOID
 * @retval  
 **/
T_VOID aud_player_restarttask(T_AUD_PLAYER* player)
{
    //if (Fwl_MicroTaskResume(player->readtask_id))
    {
        //akerror("start readtask success",0,1);
    }

    if (Fwl_MicroTaskResume(player->decodetask_id))
    {
        akerror("start decodetask success",0,1);
    }
}

/**
 * @brief         pause audio player processing task
 * @author  mayeyu
 * @date          2012-7-6
 * @param   T_AUD_PLAYER*:player
 * @return  T_VOID
 * @retval  
 **/
 T_VOID APlayer_PauseTask(T_AUD_PLAYER* player)
{
    //if (Fwl_MicroTaskPause(player->readtask_id))
    { 
        //akerror("p Fwl_MicroTaskPause", 0, 1);
    }

    if (Fwl_MicroTaskPause(player->decodetask_id))
    { 
        akerror("p Fwl_MicroTaskPause", 0, 1);
    }
}

/**
 * @brief        audio player set task
 * @author  mayeyu
 * @date          2012-7-6
 * @param   T_AUD_PLAYER*:player
 * @return  T_VOID
 * @retval  
 **/
T_VOID aud_player_settask(T_AUD_PLAYER* player)
{
    T_U16 readInterval = 10;            //读音频文件数据的时间间隔澹以10MS为单位
    T_U16 decodeInterval = 1;           //解码的时间间隔澹以10MS为单位


    aud_player_unregistertask(player);
    aud_player_registertask(player, readInterval, decodeInterval);
    AK_DEBUG_OUTPUT("\naudType=%d, readInterval=%dms, decodeInterval=%dms\n", \
        player->audType, readInterval*10, decodeInterval*10);
}


/**
 * @brief   calculate fit frequency for audio 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_U8
 * @retval  
 **/
T_U8 APlayer_CalcFreq(T_AUD_PLAYER* player,T_BOOL enableBL, T_U8 medType)
{
    T_U8 type, sample,eq;
    T_U32 sampleRate;

    if (2 == medType)
    {
        //vidio
        AK_DEBUG_OUTPUT("*Aud_PlayerFreqCal3,freq:%d\n",FREQ_APP_AUDIO_L5);
        return FREQ_APP_AUDIO_L5;
    }
    else
    {
        if(_SD_MEDIA_TYPE_MP3 == player->audType)
        {
            if(player->modeSpeed != AUD_PLAY_NORMAL_SPEED )//快放，慢放1，2，3倍时升频 
            {
                return FREQ_APP_AUDIO_L4;        
            } 
            
            type = 0;
        }
        else if(_SD_MEDIA_TYPE_WMA == player->audType)
        {
            type = 1;
        }
        else if(_SD_MEDIA_TYPE_PCM == player->audType ||
                    _SD_MEDIA_TYPE_ADPCM_IMA == player->audType ||
                    _SD_MEDIA_TYPE_ADPCM_MS == player->audType ||
                    _SD_MEDIA_TYPE_ADPCM_FLASH == player->audType)
        {
            if(player->modeSpeed != AUD_PLAY_NORMAL_SPEED )//快放，慢放1，2，3倍时升频 
            {
                return FREQ_APP_AUDIO_L4;        
            } 

            type = 2;
        }
        else if(_SD_MEDIA_TYPE_APE == player->audType)
        {
            type = 3;
        }
        else if (_SD_MEDIA_TYPE_FLAC == player->audType ||
                _SD_MEDIA_TYPE_OGG_FLAC == player->audType)
        {
            type = 4;
        }
        else    
        {
            type = 1;//110M
        }
    }

    sampleRate = (player->sampleRate > MAX_AUD_SAMPLE_RATE) ? (MAX_AUD_SAMPLE_RATE - 1):player->sampleRate;
    sample = (T_U8)(sampleRate*MAX_AUD_BITSAMP_ELE / MAX_AUD_SAMPLE_RATE);
    if (player->bitRate >= MAX_AUD_BIT_RATE)
    {
        sample = MAX_AUD_BITSAMP_ELE - 1;
    }

    eq = player->modeEQ > 0 ? 1 : 0;
    //AK_DEBUG_OUTPUT("**Aud_PlayerFreqCal,type:%d,sample:%d,eq:%d,freq:%d\n",type,sample,eq,freqNeedTable[type][sample][eq][enableBL]);

#ifdef OS_ANYKA
    if ((enableBL) && (eq == 0) && (player->modeSpeed == AUD_PLAY_NORMAL_SPEED))
    {
        //黑屏正常播放wma时降频播放,满足如下条件的不降频.
        if (_SD_MEDIA_TYPE_WMA == player->audType)
        {
            T_S32 wmaType = 0;

            wmaType = MediaLib_GetWMABitrateType(player->hMedia);
            AK_DEBUG_OUTPUT("*wma type=%d\n",wmaType);

            if (wmaType == 0)   //lpc
            {
                if ((player->bitRate >= 12000)
                    ||(player->sampleRate >= 32000))
                {
                    return FREQ_APP_AUDIO_L5;
                }
            }
            else if (wmaType == 1)  //mid
            {
                if ((player->bitRate >= 30000)
                    ||(player->sampleRate >= 44000))
                {
                    return FREQ_APP_AUDIO_L5;
                }
            }
            else    //high
            {
                return FREQ_APP_AUDIO_L5;
            }
        }
        else if(_SD_MEDIA_TYPE_MP3 == player->audType)
        {
            if ((player->bitRate > 145000)
                &&(player->sampleRate >= 44000))
            {
                return FREQ_APP_AUDIO_L3;
            }
        }
        else if(_SD_MEDIA_TYPE_AMR== player->audType)
        {
            enableBL = AK_FALSE;
        }
    }
#endif
    return freqNeedTable[type][sample][eq][enableBL];
}

/**
 * @brief   Get music infomation  
 * @author  WuShanwei
 * @date    2008-07-12
 * @param   player
            medInfo
 * @return  T_BOOL
 * @retval  
 **/
#pragma arm section code = "_audioplay_pre_" 
    
T_BOOL Aud_PlayerGetMedInfo(T_AUD_PLAYER* player,T_MEDIALIB_META_INFO* medInfo)
{
    T_U8 infoArr[MAX_AUD_MSC_INFO_LEN_ALL];
    T_U16 infoArrU[MAX_AUD_MSC_INFO_LEN_ALL];
    T_U16 arrSpac[3] = {UNICODE_SPACE,UNICODE_SPACE,0};
    T_U16 len;

    *player->strMscInfo = 0;
//  AK_DEBUG_OUTPUT("GetMedInfo,p:%d\n",medInfo);

    if(AK_NULL == medInfo)
        return AK_FALSE;

    //get title
    if(AK_NULL != medInfo->m_MetaContent.pTitle)
    {
        len = medInfo->m_MetaSizeInfo.uTitleLen < AUD_MSC_INFO_LEN_TITLE ? medInfo->m_MetaSizeInfo.uTitleLen : AUD_MSC_INFO_LEN_TITLE;
        AK_DEBUG_OUTPUT("title,len:%d,type:%d,endLen:%d,pCon:%d\n",
            medInfo->m_MetaSizeInfo.uTitleLen,medInfo->m_MetaTypeInfo.TitleType,
            len,medInfo->m_MetaContent.pTitle);

        if(0 != medInfo->m_MetaTypeInfo.TitleType)
        {//non-unicode
            Utl_StrCpyN(infoArr,(T_U8*)medInfo->m_MetaContent.pTitle,len);
            Eng_MultiByteToWideChar(infoArr, len, (T_U16*)infoArrU, 128, AK_NULL);
            Utl_UStrCpyN(player->strMscInfo,infoArrU,len);
        }
        else
        {//unicode
            Utl_UStrCpyN(player->strMscInfo,(T_U16*)medInfo->m_MetaContent.pTitle,len);
        }

        Utl_UStrCat(player->strMscInfo,arrSpac);
        infoArr[0] = 0;
        infoArrU[0] = 0;
    }
    
#if (!((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL)))   
    if(AK_NULL != medInfo->m_MetaContent.pArtist)
    {
        len = medInfo->m_MetaSizeInfo.uArtistLen < AUD_MSC_INFO_LEN_ARTIST ? medInfo->m_MetaSizeInfo.uArtistLen : AUD_MSC_INFO_LEN_TITLE;
        AK_DEBUG_OUTPUT("artist,len:%d,type:%d,endLen:%d,pCon:%d\n",
            medInfo->m_MetaSizeInfo.uArtistLen,medInfo->m_MetaTypeInfo.ArtistType,
            len,medInfo->m_MetaContent.pArtist);
        if(0 != medInfo->m_MetaTypeInfo.ArtistType)
        {//non-unicode
            Utl_StrCpyN(infoArr,(T_U8*)medInfo->m_MetaContent.pArtist,len);
            Eng_MultiByteToWideChar(infoArr, Utl_StrLen(infoArr), (T_U16*)infoArrU, 128, AK_NULL);          
        }
        else
        {//unicode
            Utl_UStrCpyN(infoArrU,medInfo->m_MetaContent.pArtist,len);
        }
        
        Utl_UStrCat(player->strMscInfo,infoArrU);
        Utl_UStrCat(player->strMscInfo,arrSpac);
        infoArr[0] = 0;
        infoArrU[0] = 0;
    }
#endif



    if(AK_NULL != medInfo->m_MetaContent.pAlbum)
    {
        len = medInfo->m_MetaSizeInfo.uAlbumLen < AUD_MSC_INFO_LEN_ARTIST ? medInfo->m_MetaSizeInfo.uAlbumLen : AUD_MSC_INFO_LEN_TITLE;
        AK_DEBUG_OUTPUT("album,len:%d,type:%d,endLen:%d,pCon:%d\n",
            medInfo->m_MetaSizeInfo.uAlbumLen,medInfo->m_MetaTypeInfo.AlbumType,
            len,medInfo->m_MetaContent.pAlbum);
        if(0 != medInfo->m_MetaTypeInfo.AlbumType)
        {//non-unicode
            Utl_StrCpyN(infoArr,(T_U8*)medInfo->m_MetaContent.pAlbum,len);
            Eng_MultiByteToWideChar(infoArr, Utl_StrLen(infoArr), (T_U16*)infoArrU, 128, AK_NULL);          
        }
        else
        {//unicode
            Utl_UStrCpyN(infoArrU,medInfo->m_MetaContent.pAlbum,len);
        }
        
        Utl_UStrCat(player->strMscInfo,infoArrU);
    }
    
    AK_DEBUG_OUTPUT("end,p:%d,len:%d\n",player->strMscInfo,Utl_UStrLen(player->strMscInfo));

    return AK_TRUE;
}

/**
 * @brief   media lib  Init
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_AUD_PLAYER*
 * @retval  
 **/
T_BOOL Aud_PlayerMediaLibInit(T_VOID)
{
    T_MEDIALIB_INIT mediaLibInit;

#ifdef DEBUG    
    mediaLibInit.m_CBFunc.m_FunPrintf = AK_DEBUG_OUTPUT;
#else
    #ifdef OS_ANYKA
    mediaLibInit.m_CBFunc.m_FunPrintf = AK_NULL;
    #else
    mediaLibInit.m_CBFunc.m_FunPrintf = AK_DEBUG_OUTPUT;
    #endif
#endif
    mediaLibInit.m_CBFunc.m_FunRead = Fwl_FileRead;
    mediaLibInit.m_CBFunc.m_FunWrite = Fwl_FileWrite;
    mediaLibInit.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)Fwl_FileSeek;
    mediaLibInit.m_CBFunc.m_FunTell = Fwl_FileTell;
    mediaLibInit.m_CBFunc.m_FunMalloc = Fwl_Malloc;
    mediaLibInit.m_CBFunc.m_FunFree = Fwl_Free;
    mediaLibInit.m_CBFunc.m_FunDMAFree = Fwl_DMAFree;
    mediaLibInit.m_CBFunc.m_FunDMAMalloc = Fwl_DMAMalloc;
    mediaLibInit.m_CBFunc.m_FunRtcDelay = Fwl_DelayUs;

    mediaLibInit.m_CBFunc.m_FunDMAMemcpy = (MEDIALIB_CALLBACK_FUN_DMA_MEMCPY)memcpy;
        
    if (0 == gAudMeidaCount)
    {
        if (0 == MediaLib_Init(&mediaLibInit))
        {
            AK_DEBUG_OUTPUT("Meidalib init fail\n");
            return AK_FALSE;
        }
    }
    gAudMeidaCount++;

    AK_DEBUG_OUTPUT("Meidalib end\n");
    return AK_TRUE;
}
/**
 * @brief   media lib  destroy
 * @auorth  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE-OK, AK_FALSE-fail
 *  媒体库的销毁必须与初始化成对使用
 **/
T_BOOL  Aud_PlayerMediaLibDestroy(T_VOID)
{
    T_BOOL rst = AK_FALSE;
    
    AK_DEBUG_OUTPUT("MediaLib_Destroy :%d\n", gAudMeidaCount);  

	if (0 == gAudMeidaCount)
	{
		return rst;
	}
    
    if (1 == gAudMeidaCount)
    {
        rst = (T_BOOL)MediaLib_Destroy(); 
    }
    gAudMeidaCount--;   
    AK_DEBUG_OUTPUT("Destroy OK:%d\n",rst);
    return rst;
}


/**
 * @brief   player Init
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_AUD_PLAYER*
 * @retval  
 **/
T_AUD_PLAYER* APlayer_Init(T_U8 vol, T_U8 speed, T_U8 eq)
{
    T_AUD_PLAYER* player;

    player = (T_AUD_PLAYER*)Fwl_Malloc(sizeof(T_AUD_PLAYER));
    if(AK_NULL == player)
    {
        return AK_NULL; 
    }
    memset(player,0,sizeof(T_AUD_PLAYER));
    //player->flagHDPopen = AK_FALSE;
    player->hFile = FS_INVALID_HANDLE;
    player->hVideoFile= FS_INVALID_HANDLE;
    player->readtask_id = 0xff;
    player->decodetask_id = 0xff;
    #ifdef SUPPORT_BLUETOOTH
    player->bset_eq_flag = AK_FALSE;
    #endif
    player->pcm_id = Pcm_Open();//WaveOut_Open(DAC_HP, Aud_PlayerInterruptHandle);
    if(INVAL_PCMOPEN_ID == player->pcm_id)
    {
        AK_DEBUG_OUTPUT("APlayer_Init pcm open failed:%d\n", player->pcm_id);
        player = Fwl_Free(player);
        return AK_NULL;
    }

    Aud_PlayerMediaLibInit();

    AK_DEBUG_OUTPUT("APlayer_Init end, open id:%d\n", player->pcm_id);
#ifdef OS_ANYKA
#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)
    //AUDIO_SECTION_BASE对应的代码是本来是需要放在bootcode1中的,但由于内存不够的关系被裁出来,所以必须在用到的时候锁住对应的页
    remap_lock_page(AUDIO_SECTION_BASE,AUDIO_SECTION_LIMIT-AUDIO_SECTION_BASE,AK_TRUE);
#endif
#endif
    player->volume = vol;
    player->modeEQ = eq;
    player->modeSpeed = speed;

    return player;
}


static T_BOOL CBFun_Push(T_AUD_CB_TYPE cbType, T_U32 cbPtr)
{
    AK_DEBUG_OUTPUT("CBFun_Push:%d,ptr:%x\n",cbType,cbPtr);

    if (cbType >= AUD_CB_NUM || cbType < 0)
    {
        return AK_FALSE;
    }

    if (gAudCBStack[cbType].pc >= CBPTR_STACK_DEPTH)
    {
        AK_DEBUG_OUTPUT("aud cb stack over\n");
        return AK_FALSE;
    }

    gAudCBStack[cbType].cbPtr[gAudCBStack[cbType].pc] = cbPtr;
    gAudCBStack[cbType].pc++;

    return AK_TRUE; 
}

static T_pVOID CBFun_Pop(T_AUD_CB_TYPE cbType)
{
    AK_DEBUG_OUTPUT("CBFun_Pop:%d\n",cbType);

    if (cbType >= AUD_CB_NUM || cbType < 0)
    {
        return AK_NULL;
    }

    if (gAudCBStack[cbType].pc == 0)
    {
        AK_DEBUG_OUTPUT("aud cb stack empty\n");
        return AK_NULL;
    }

    gAudCBStack[cbType].pc--;

    if (gAudCBStack[cbType].pc == 0)
    {
        return AK_NULL;
    }
    else
    {
        return (T_pVOID)gAudCBStack[cbType].cbPtr[gAudCBStack[cbType].pc - 1];
    }
}


/**
 * @brief   Aud_PlayerRegitHDLPre
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  
 * @retval  
 **/
T_VOID APlayer_RegHDLPre(AUDPLAYER_CALLBACK_HANDLEPRE pCallBackFun)
{
    AK_DEBUG_OUTPUT("Aud_PlayerRegitHDLPre\n");
    Aud_PlayerHandlePre = pCallBackFun;
    CBFun_Push(AUD_CB_PRE, (T_U32)pCallBackFun);
}
/**
 * @brief   Aud_PlayerRegitHDLPOST
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_AUD_PLAYER*
 * @retval  
 **/
T_VOID APlayer_RegHDLPost(AUDPLAYER_CALLBACK_HANDLEPOST pCallBackFun)
{
    AK_DEBUG_OUTPUT("Aud_PlayerRegitHDLPost\n");
    Aud_PlayerHandlePost = pCallBackFun;
    CBFun_Push(AUD_CB_POST, (T_U32)pCallBackFun);
}

/**
 * @brief   player config comply
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  Aud_PlayerCfgComp(T_AUD_PLAYER* player)
{
    if (player->modeEQ != _SD_EQ_MODE_NORMAL)
    {
        APlayer_SetToneMode(player,player->modeEQ);
    }
    else if (player->modeSpeed != AUD_PLAY_NORMAL_SPEED)
    {
        APlayer_SetSpeed(player,player->modeSpeed);
    }
    APlayer_SetVolume(player->volume);
    AK_DEBUG_OUTPUT("Aud_PlayerCfgComp,vol:%d,eq:%d,speed:%d\n",player->volume,player->modeEQ,player->modeSpeed);
    return AK_TRUE;
}

/**
 * @brief   Aud_PlayerSetVolume
 * @author  liangxiong
 * @date    2012-11-22
 * @param   vol (0~1024)
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_SetVolume(T_U8 vol)
{
    if (vol > MAX_VOLUME)
        vol = MAX_VOLUME;
    return (T_BOOL)MediaLib_SetVolume(TabVolDig[vol]);
}


/**
 * @brief   player Exit
 * @auorth  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Destroy(T_AUD_PLAYER* player)
{  
	if(AK_NULL == player)
    {
        return AK_FALSE; 
    }
	
    aud_player_unregistertask(player);
    
    Aud_PlayerMediaLibDestroy();
    
    Pcm_Close(player->pcm_id);
    player->pcm_id = 0xff;
    
    player = Fwl_Free(player);
    APlayer_PrePostPop();
    
    AK_DEBUG_OUTPUT("Aud_PlayerHandlePre=%x,Aud_PlayerHandlePost=%x\n",
                    Aud_PlayerHandlePre, Aud_PlayerHandlePost);
#ifdef OS_ANYKA
#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)
    //AUDIO_SECTION_BASE对应的代码是在初始化时被锁住了,所以必须在退出状态机时解锁
        remap_lock_page(AUDIO_SECTION_BASE,AUDIO_SECTION_LIMIT- AUDIO_SECTION_BASE,AK_FALSE);//解锁1个页
#endif
#endif

    return AK_TRUE;
}


/**
 * @brief   player set DA info
 * @auorth  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  Aud_PlayerSetDAInfo(T_AUD_PLAYER* player)
{
    T_MEDIALIB_MEDIA_INFO mediaInfo;

    MediaLib_GetInfo(player->hMedia,&mediaInfo);


    MediaLib_SetPCMInfo(mediaInfo.nChannels,mediaInfo.wBitsPerSample);  
//  Fwl_DA_SetInfo(&pcmInfo);
    return AK_TRUE;
}

/**
 * @brief   get audio file type from file name
 * @auorth  xp
 * @date    2008-10-30
 * @param   T_U16* filename
 * @return  audio type
 * @retval  
 **/
 static T_U8 Aud_PlayerGetAudType(T_U16* filename)
 {
    T_U8 audType;
    T_U16 audFilter[7][5] = {
                        {'a', 'p', 'e', '\0'},
                        {'a', 's', 'f', '\0'},
                        {'o', 'g', 'g', '\0'},
                        {'f', 'l', 'a', 'c', '\0'},
                        {'a', 'm', 'r', '\0'},
                        {'w', 'm', 'a', '\0'},
                        {'w', 'a', 'v', '\0'}
                        };
    T_U32 i = Utl_UStrLen(filename);
    T_U16* pStr;

    while(i--)
    {
        if (filename[i] == UNICODE_DOT)
        {
            break;
        }
        
    }
    
    pStr = (T_U16*)(filename + i + 1);

    for( i = 0; i < 7; i++)
    {
        if (0 == Utl_UStrCmpC(pStr,audFilter[i]))
        {
            break;
        }
    }

    switch(i)
    {
        case 0:
            audType = _SD_MEDIA_TYPE_APE;
            break;
        case 1:
        case 5:
            audType = _SD_MEDIA_TYPE_WMA;
            break;           
        case 2:
            audType = _SD_MEDIA_TYPE_OGG_FLAC;
            break;     
        case 3:
            audType = _SD_MEDIA_TYPE_FLAC;
            break;
        case 4:
            audType = _SD_MEDIA_TYPE_AMR;
            break;    
        case 6:
            audType = _SD_MEDIA_TYPE_PCM;
            break;
        default:
            audType = _SD_MEDIA_TYPE_MP3;
            break;       
    }

    return audType;
}

/**
 * @brief   player open 
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            file
            medType: media type, 0-audio, 1-record, 2-vedio
            bFade:  AK_TRUE - fade enable  AK_FALSE-fade disable
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
 
 T_BOOL  Aud_PlayerResourceFree(T_AUD_PLAYER* player)
 {
    #ifdef OS_ANYKA
     AK_PRINTK("Aud_PlayerResourceFree,h:",player->hMedia, 1);
    #endif
 
     if(AK_NULL == player)
         return AK_FALSE;
 
     if(FS_INVALID_HANDLE != player->hFile)
     {

        MediaLib_Close(player->hMedia);
#ifdef SUPORT_MUISE_EQ
		_SD_Filter_Close(player->pEQFilter);
#endif

 
#ifdef OS_WIN32    
         Audio_WINClose(); 
#endif
         
         player->hMedia = AK_NULL;
         Fwl_FileClose(player->hFile);
         player->hFile = FS_INVALID_HANDLE;
         Fwl_FileClose(player->hVideoFile);
         player->hVideoFile= FS_INVALID_HANDLE;

         akerror("  audio destory cycbuf :", player->daBufLen, 1);
         player->daBufLen = 0;

         return AK_TRUE;
     }
     
     AK_DEBUG_OUTPUT("invalid file handle\n");
     return AK_FALSE;
}

/**
 * @brief       check bitrate and samples is limiting
 * @author  mayeyu
 * @date    2011-04-01
 * @param   audio_type: audio type
                   audio_bitrate: audio bitrate
                   audio_samplesPerSec: audio samplesPerSec
        
 * @return  T_BOOL
 * @retVal  AK_TRUE-limit , AK_FALSE-no limit
 **/
static T_BOOL  Aud_PlayerBitrateAndSamplesLimit(T_AUD_PLAYER* player, T_U32 audio_type, T_U32 audio_bitrate, T_U32 audio_samplesPerSec)
{
    T_U32 limitBitrate = 300000;              ////bitrate limit value
    T_U32 samplesPerSec = 48000;    //support samplesPerSec max value
    T_AUDIO_TYPE audioType = _SD_MEDIA_TYPE_UNKNOWN; //audio type
    T_BOOL retVal = AK_FALSE;       //return value
    
    audioType = (T_AUDIO_TYPE)audio_type;

    switch(audioType) //different media type have different limitBitrate and samplesPerSec
    {

        case _SD_MEDIA_TYPE_MP3:
        {
            limitBitrate = 320000;
        }
        break;

        case _SD_MEDIA_TYPE_WMA:
        {
            limitBitrate = 320000;          
        }
        break;

        case _SD_MEDIA_TYPE_AMR:
        {
            samplesPerSec = 8000;
            limitBitrate = 12200;
        }
        break;

        case _SD_MEDIA_TYPE_OGG_VORBIS:
        {
           limitBitrate = 320000;
        }
        break;
        
        case _SD_MEDIA_TYPE_SPEEX:
        {
           limitBitrate = 0xffffffff;
        }        
        break;

		case _SD_MEDIA_TYPE_PCM:
		case _SD_MEDIA_TYPE_ADPCM_FLASH:
		case _SD_MEDIA_TYPE_ADPCM_IMA:
		case _SD_MEDIA_TYPE_ADPCM_MS:
		case _SD_MEDIA_TYPE_PCM_ALAW:
		case _SD_MEDIA_TYPE_PCM_ULAW:
        case _SD_MEDIA_TYPE_OGG_FLAC:
        {
            limitBitrate = 1600000;
        }
        break;

		case _SD_MEDIA_TYPE_FLAC:
		case _SD_MEDIA_TYPE_APE:
        {
            limitBitrate = 700000;
        }
        break;

        default:
        break;

    }

    if (MediaLib_Get_MinBuf(player->hMedia) > player->streamBufLen)
    {
        AK_DEBUG_OUTPUT("aud limit err, minbuf:%d, streamlen:%d\n", MediaLib_Get_MinBuf(player->hMedia), player->streamBufLen);
        retVal = AK_TRUE;
    }  

    if (((audio_bitrate > limitBitrate) && (limitBitrate != 0))
        ||(audio_samplesPerSec > samplesPerSec))
    {
        retVal = AK_TRUE;
        AK_DEBUG_OUTPUT("\nbitrate or samplesPerSec is limited!->limitBitrate:%d, limitsamplesPerSec:%d\n",limitBitrate, samplesPerSec);
    }

    AK_DEBUG_OUTPUT("\nAud_PlayerBitrateLimit:%d, medType:%d\n",retVal, audioType);
    AK_DEBUG_OUTPUT("audio_bitrate:%d, audio_samplesPerSec:%d\n",audio_bitrate, audio_samplesPerSec);
    return retVal;
}

static T_VOID  Aud_PlayerSetBufSize(T_U8 audType, T_AUD_PLAYER* player)
{
    T_U8 type = audType;
    
    player->emptyDecLen = 2<<10;

    switch(type)
    {
        case _SD_MEDIA_TYPE_APE:
        {
            player->streamBufLen = 16<<10;
            player->expReadLen = 16<<10;            
            player->daBufLen = 4<<10;
        }
        break;
        case _SD_MEDIA_TYPE_WMA:
        {
            player->streamBufLen = 6<<10;
            player->expReadLen = 6<<10;
            player->daBufLen = 12<<10;
        }
        break;           
        case _SD_MEDIA_TYPE_OGG_FLAC:
        {
            player->streamBufLen = 8<<10;
            player->expReadLen = 8<<10;
            player->daBufLen = 8<<10;
        }
        break;     
        case _SD_MEDIA_TYPE_AMR:
        {
            player->streamBufLen = 4<<10;
            player->expReadLen = 4<<10;
            player->daBufLen = 4<<10;
        }
        break;      
        case _SD_MEDIA_TYPE_MP3:
        {
            player->streamBufLen = 6<<10;
            player->expReadLen = 6<<10;
            player->daBufLen = 8<<10;
        }
        break;
        case _SD_MEDIA_TYPE_FLAC:
        {
            player->streamBufLen = 16<<10;
            player->expReadLen = 16<<10;
            player->daBufLen = 16<<10;
        }
        break;
        case _SD_MEDIA_TYPE_PCM:
        {
            player->streamBufLen = 16<<10;
            player->expReadLen = 16<<10;
            player->daBufLen = 4<<10;
        }
        break;
        default:
        {
            player->streamBufLen = 12<<10;
            player->expReadLen = 8<<10;
            player->daBufLen = 12<<10;
        }
        break;       
    }
    
#if (STORAGE_USED == SD_CARD)
    {
        player->streamBufLen = 6<<10;
        player->expReadLen = 4<<10;
        player->daBufLen = 12<<10;
    }
#endif

#ifdef OS_WIN32
		{
			player->streamBufLen = 20<<10;
			player->expReadLen = 20<<10;
			player->daBufLen = 20<<10;
		}
#endif


    AK_DEBUG_OUTPUT("\nAud_PlayerSetBufSize type=%d,streamBufSize=%d,expReadLen=%d,emptyDecLen=%d,daBufLen=%d.\n", \
        type, player->streamBufLen, player->expReadLen,player->emptyDecLen,player->daBufLen);
}
#ifdef SUPORT_MUISE_EQ
T_U32 APlayerEQHandle;
T_VOID EQDebugOutput(T_pCSTR s, ...)
{
}

T_pVOID APlayer_InitEQ(T_PCM_INFO *info)
{ 
    T_AUDIO_FILTER_INPUT s_ininfo; 

    s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Fwl_Malloc;
    s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)Fwl_Free;
    s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)EQDebugOutput;     
    s_ininfo.m_info.m_BitsPerSample = info->bps;
    s_ininfo.m_info.m_Channels = info->channel;
    s_ininfo.m_info.m_SampleRate = info->samplerate; //歌曲本身采样率
    s_ininfo.m_info.m_Type = _SD_FILTER_EQ;
    s_ininfo.m_info.m_Private.m_eq.eqmode = _SD_EQ_MODE_NORMAL;
//    s_ininfo.m_info.m_Private.m_eq.bands = 0; 
//    s_ininfo.m_info.m_Private.m_eq.bandfreqs[0] = 0; 
//    s_ininfo.m_info.m_Private.m_eq.bandgains[0] = 0;

    APlayerEQHandle = _SD_Filter_Open(&s_ininfo);
	return APlayerEQHandle;
}
#pragma arm section code = "_audioplayer_resident_"
T_U32 APlayer_EQWork(T_U8 *data, T_U16 len)
{ 
	T_AUDIO_FILTER_BUF_STRC filter;

	filter.buf_in = data;
	filter.buf_out = data;
	filter.len_in = len;
	filter.len_out = len;
	filter.buf_in2 = AK_NULL;
	filter.len_in2 = 0;
    return _SD_Filter_Control(APlayerEQHandle,&filter);
}

T_S32 APlayer_PcmWrite(T_U8* buf, T_U32 size)
{
	size = APlayer_EQWork(buf, size);
	return Pcm_Write(buf, size);
}
#pragma arm section code
#endif
T_BOOL  APlayer_Open(T_AUD_PLAYER* player,T_USTR_FILE file,T_eMEDIALIB_MEDIA_TYPE medType, T_BOOL bFade,  T_U32 bFadeInTime, T_U32  bFadeOutTime, T_WSOLA_ARITHMATIC speedLibType)
{
    T_MEDIALIB_OPEN_INPUT mediaLibInput;
    T_MEDIALIB_MEDIA_INFO mediaInfo;
    T_PCM_INFO info;
	T_U8 MediaType;

    Aud_PlayerResourceFree(player);

    AK_DEBUG_OUTPUT("Aud_PlayerOpen begin\n");
    AK_DEBUG_OUTPUT("fileNameLen:%d\n",Utl_UStrLen(file));
    Printf_UC(file); 
    player->curTime = 0;
    player->totalTime = 0;
    player->pastTime = 0;
    player->pFileName = AK_NULL;
    player->audType   = _SD_MEDIA_TYPE_UNKNOWN;
    player->mediaType = medType;
    player->bitRate   = 0;
    player->bFade     = bFade;
    player->hFile = Fwl_FileOpen(file, _FMODE_READ, _FMODE_READ);
    player->hVideoFile= FS_INVALID_HANDLE;
    if(FS_INVALID_HANDLE == player->hFile)
    {
        AK_DEBUG_OUTPUT("open file failed,fH:%d\n",player->hFile);
        return AK_FALSE;
    }
	
    mediaLibInput.m_jdrv_himg = -1;

    AK_DEBUG_OUTPUT("open file OK,Aud fH:%x video fh:%x,type = %d\n",
                     player->hFile, player->hVideoFile, medType);
    mediaLibInput.m_hMediaSource = (T_S32)player->hFile;
#ifdef DEBUG
    mediaLibInput.m_CBFunc.m_FunPrintf = AK_DEBUG_OUTPUT;
#else
    #ifdef OS_ANYKA
    mediaLibInput.m_CBFunc.m_FunPrintf = AK_NULL;
    #else
    mediaLibInput.m_CBFunc.m_FunPrintf = AK_DEBUG_OUTPUT;
    #endif    
#endif
    mediaLibInput.m_CBFunc.m_FunRead = Fwl_FileRead;
    mediaLibInput.m_CBFunc.m_FunWrite = Fwl_FileWrite;
    mediaLibInput.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)Fwl_FileSeek;
    mediaLibInput.m_CBFunc.m_FunTell = Fwl_FileTell;
    mediaLibInput.m_CBFunc.m_FunMalloc = Fwl_Malloc;
    mediaLibInput.m_CBFunc.m_FunFree = Fwl_Free;
    mediaLibInput.m_CBFunc.m_FunDMAFree = Fwl_DMAFree;
    mediaLibInput.m_CBFunc.m_FunDMAMalloc = Fwl_DMAMalloc;
    mediaLibInput.m_CBFunc.m_FunRtcDelay = Fwl_DelayUs;
    mediaLibInput.m_CBFunc.m_FunDMAMemcpy = (MEDIALIB_CALLBACK_FUN_DMA_MEMCPY)memcpy;

    {
        //这是旧接口，这一版本的媒体库已不再调用，下一版本媒体库将去掉这些接口
        mediaLibInput.m_FunVidioOut = (AKV_CB_VIDEO_OUT)(1);    
        mediaLibInput.m_vOutOffsetX = 0;
        mediaLibInput.m_vOutOffsetY = 0;
    }
    
    mediaLibInput.m_bFadeEnable = bFade;
    mediaLibInput.m_MediaType= medType;
    mediaLibInput.m_bFadeInTime= bFadeInTime;
    mediaLibInput.m_bFadeOutTime= bFadeOutTime;
    player->speedLibType= speedLibType;

    AK_DEBUG_OUTPUT("bFade:%d,mType:%d,Fin:%d,Fout:%d\n", 
                    bFade, medType, bFadeInTime, bFadeOutTime);

    //这是用于音频streambuf的
    //如果将此变量置0，音频库会设置一个默认的大小
    //根据不同格式，设置的默认值不一样
    //mediaLibInput.m_buflen = AUD_PAYER_STREAMBUF_SIZE;
    MediaType = Aud_PlayerGetAudType(file);
	
APlayer_Open_SET_BUFFER_TABLE:
    Aud_PlayerSetBufSize(MediaType, player);
    mediaLibInput.m_buflen = player->streamBufLen;
#ifdef SUPORT_MUISE_EQ
    mediaLibInput.m_CBFunc.m_FunCpy = APlayer_PcmWrite;
#else
    mediaLibInput.m_CBFunc.m_FunCpy = Pcm_Write;
#endif

    AK_DEBUG_OUTPUT("Meidalib open start.\n");
    player->hMedia = MediaLib_Open(&mediaLibInput);
        
    AK_DEBUG_OUTPUT("MediaLib_Open,h:0x%x MedType:%d\n",player->hMedia, medType);
    if (AK_NULL == player->hMedia)
    {
        AK_DEBUG_OUTPUT("MediaLib_Open error,hd:0x%x\n",player->hFile);

        player->audType = Aud_PlayerGetAudType(file);
        MediaLib_Close(player->hMedia);

        Fwl_FileClose(player->hFile);
        player->hFile = FS_INVALID_HANDLE;
        if(FS_INVALID_HANDLE != player->hVideoFile)
        {
            Fwl_FileClose(player->hVideoFile);
            player->hVideoFile= FS_INVALID_HANDLE;
        }
        //AK_DEBUG_OUTPUT("close file end,hd:%d\n",player->hFile);
        
        return AK_FALSE;
    }

    //get media information 

    MediaLib_GetInfo(player->hMedia,&mediaInfo);
	if(MediaType != mediaInfo.audio_type)
	{
		AkDebugOutput("MediaLib_Open: the media is different with the file ext!\n");
        MediaLib_Close(player->hMedia);
		MediaType = mediaInfo.audio_type;
		goto APlayer_Open_SET_BUFFER_TABLE;
	}
    //Bitrate and SamplesPerSec Limit 
    if (AK_TRUE == Aud_PlayerBitrateAndSamplesLimit(player, mediaInfo.audio_type, mediaInfo.audio_bitrate, mediaInfo.nSamplesPerSec))
    {
        player->audType = Aud_PlayerGetAudType(file);
        MediaLib_Close(player->hMedia);
        Fwl_FileClose(player->hFile);
        player->hFile = FS_INVALID_HANDLE;
        
        return AK_FALSE;
    }

    info.samplerate = mediaInfo.nSamplesPerSec;
    info.channel = 2;
    info.bps = mediaInfo.wBitsPerSample;
    info.bufsize = DISCARD_WAVEOUTDATA_SIZE;
#ifdef SUPORT_MUISE_EQ
    player->pEQFilter = APlayer_InitEQ(&info);
#endif
    Pcm_Start(player->pcm_id, &info);
    player->discard_size = DISCARD_WAVEOUTDATA_SIZE;

    if( (MEDIALIB_MEDIA_UNKNOWN== medType) && 
        ( (_SD_MEDIA_TYPE_MP3 == mediaInfo.audio_type) || (_SD_MEDIA_TYPE_WMA == mediaInfo.audio_type) ) )
    {
        Aud_PlayerGetMedInfo(player,mediaInfo.m_pMetaInfo);
    }

   
    player->curTime = 0;
    player->totalTime = mediaInfo.total_time_ms;
    player->sampleRate = mediaInfo.nSamplesPerSec;
    player->bitRate = mediaInfo.audio_bitrate;
    player->audType = (T_U8)mediaInfo.audio_type;
    
    AK_DEBUG_OUTPUT("Aud_PlayerOpen OK,tolT:%d,splPSec:%d,bitR:%d,bitPSec:%d,audType:%d,ch:%d\n",
                    mediaInfo.total_time_ms,mediaInfo.nSamplesPerSec,
                    mediaInfo.audio_bitrate,mediaInfo.wBitsPerSample,
                    mediaInfo.audio_type,mediaInfo.nChannels);

    if(MEDIALIB_MEDIA_AV != medType)
    {
        MediaLib_ReleaseInfoMem(player->hMedia);
    }

    player->pFileName = file;
    AK_DEBUG_OUTPUT("Aud_PlayerOpen,end,hMedia:h%x\n",player->hMedia);
    Printf_UC(player->strMscInfo);



    //在这里也可以不设置DA BUF的大小，DA BUF使用音频库中默认的大小24K
  /*  
    if (!MediaLib_Set_DABuf_Len(player->daBufLen))              //set DA Buf size
    {
        AK_DEBUG_OUTPUT("Meidalib Set_DABuf fail\n");               
        return AK_FALSE;
    }
*/
    aud_player_settask(player);
#ifdef OS_WIN32
    Audio_WINOpen(player->sampleRate, 2, 5120, 16);
#endif

    return AK_TRUE;
}


/**
 * @brief   decode first time for normal music play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_S32   Aud_PlayerFirstDecode(T_AUD_PLAYER* player)
{
    T_S32   nSize = -1;
    
    //Fwl_ConsoleWriteChr('@'); 
    if (Mutex_flag)
    {
        return 0;
    }
    else
    {
        Mutex_flag = 1;
    }
    
    if (AK_NULL != player->hMedia)
    {
        nSize = MediaLib_Handle(player->hMedia);
    }
        
    if (nSize < 0)
    {
        Mutex_flag = 0;
        return nSize;       
    }
    Mutex_flag = 0;
    return nSize;
}

/**
 * @brief   player decode first
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            file
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  Aud_PlayerFirstDec(T_AUD_PLAYER* player)
{
    T_MEDIALIB_MEDIA_INFO mediaInfo;    
    T_PCM_INFO info;
    
    APlayer_SetVolume(player->volume);

    MediaLib_GetInfo(player->hMedia,&mediaInfo);
    
    Pcm_Pause(player->pcm_id);

    if (FS_INVALID_HANDLE == player->hVideoFile)
    {
        akerror("Aud_PlayerFirstDec", 0, 1);
        aud_player_read();      //read audio file data
    }
        
    if(Aud_PlayerFirstDecode(player) < 0)
    {
        //AK_DEBUG_OUTPUT("failed\n");
        return AK_FALSE;
    }
    else
    {
        if (AK_TRUE == player->bFade)
        {
		    AK_DEBUG_OUTPUT("discard wave size: %d.\n", Pcm_DiscardWaveData(player->pcm_id, player->discard_size));
            player->discard_size = 0;
        }

        info.samplerate = mediaInfo.nSamplesPerSec;
        info.channel = 2;
        info.bps = mediaInfo.wBitsPerSample;
        if (AUD_PLAY_NORMAL_SPEED != player->modeSpeed)
        {
            info.bufsize = player->daBufLen + 4096;
        }
        else
        {
            info.bufsize = player->daBufLen;
        }
        
        Pcm_ReStart(player->pcm_id, &info);    

        aud_player_restarttask(player);
        return AK_TRUE;
    }
}


/**
 * @brief   player play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            file
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Play(T_AUD_PLAYER* player)
{
    AK_DEBUG_OUTPUT("Aud_PlayerPlay,h:%d\n",player->hMedia);
    Aud_PlayerSetDAInfo(player);
    Aud_PlayerCfgComp(player);
    return (APlayer_Resume(player));
}


/**
 * @brief   player pause
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Pause(T_AUD_PLAYER* player)
{
    AK_DEBUG_OUTPUT("Aud_PlayerPause\n");

    return MediaLib_Pause(player->hMedia);
}

/**
 * @brief   resume play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_AUD_PLAYER*
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Resume(T_AUD_PLAYER* player)
{
    T_BOOL bRet = AK_FALSE;
    T_S32 ret;
    
    if (AK_NULL != player->hMedia)
    {
        //Aud_PlayerOpenHP(player);

        ret = MediaLib_Play(player->hMedia);

        if (ret < 0)
        {
            AK_DEBUG_OUTPUT("AVFPlayer_Play or MediaLib_Play failed: %d\n", ret);
        }
        else
        {
            bRet = Aud_PlayerFirstDec(player);
        }
    }
    
    return bRet;
}
#pragma arm section code

/**
 * @brief   player stop
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_Stop(T_AUD_PLAYER* player)
{
    #ifdef OS_ANYKA
    AK_PRINTK("Aud_PlayerStop,h:",player->hMedia, 1);
    #endif

    if(AK_NULL == player)
        return AK_FALSE;

    player->curTime = 0;
    *player->strMscInfo = 0;
    player->pastTime = 0;
    //player->totalTime = 0;

    if(FS_INVALID_HANDLE != player->hFile)
    {
        //要先把任务关闭，才能做媒体库的关闭操作
        //否则可能会出现解码或者读的时候调用已释放的媒体库句柄，造成死机
        aud_player_unregistertask(player);
        if (0xff != player->pcm_id)
        {
            Pcm_Stop(player->pcm_id);
            player->daBufLen = 0;
        }
        
        MediaLib_Close(player->hMedia);
#ifdef SUPORT_MUISE_EQ
		_SD_Filter_Close(player->pEQFilter);
#endif

#ifdef OS_WIN32    
        Audio_WINClose(); 
#endif
        
        player->hMedia = AK_NULL;
        Fwl_FileClose(player->hFile);
        player->hFile = FS_INVALID_HANDLE;
        Fwl_FileClose(player->hVideoFile);
        player->hVideoFile= FS_INVALID_HANDLE;
        /*if (player->flagHDPopen)
        {
            Aud_OpenDACSignal(AK_FALSE);
        }*/
        return AK_TRUE;
    }


    AK_DEBUG_OUTPUT("invalid file handle\n");
    return AK_FALSE;
}


/**
 * @brief   player seek
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   player
            time
 * @return  T_U32
 * @retval  
 **/
T_S32   APlayer_Seek(T_AUD_PLAYER* player,T_U32 time)
{
    player->curTime = time;
    AK_DEBUG_OUTPUT("Aud_PlayerSeek,h:%d,time:%d\n",player,time);
    
    return MediaLib_SetPosition(player->hMedia, (T_S32) time);
}

/**
 * @brief   Set play speed
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   speed - 15 step
         _SD_WSOLA_0_5 ,
         _SD_WSOLA_0_6 ,
         _SD_WSOLA_0_7 ,
         _SD_WSOLA_0_8 ,
         _SD_WSOLA_0_9 ,
         _SD_WSOLA_1_0 ,
         _SD_WSOLA_1_1 ,
         _SD_WSOLA_1_2 ,
         _SD_WSOLA_1_3 ,
         _SD_WSOLA_1_4 ,
         _SD_WSOLA_1_5 ,
         _SD_WSOLA_1_6 ,
         _SD_WSOLA_1_7 ,
         _SD_WSOLA_1_8 ,
         _SD_WSOLA_1_9 ,
         _SD_WSOLA_2_0 
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
#pragma arm section code = "_audioplay_pre_"
T_BOOL  APlayer_SetSpeed(T_AUD_PLAYER* player,T_U32 speed)
{
    T_U8 valIdx,valVal= 0; 
    T_S32 rst =0;
    T_U8 speedIdx= 0;
    valIdx = (T_U8)(((speed > (AUD_PLAY_SETG_MAX_SPEED_STEP-1)) ? (AUD_PLAY_SETG_MAX_SPEED_STEP-1):speed));

    player->modeSpeed = (T_U8)valIdx;

    if(MEDIALIB_MEDIA_AV != player->mediaType)
    {
        if (_SD_MEDIA_TYPE_APE == player->audType)
        {
            return AK_FALSE;
        }

        AK_DEBUG_OUTPUT("Speed:%d,",valIdx);
        #ifdef OS_ANYKA
        if(player->speedLibType == _SD_WSOLA_ARITHMATIC_1)
        {
            speedIdx= 1;
            MediaLib_SetWSOLA_Arithmetic(player->hMedia, _SD_WSOLA_ARITHMATIC_1);
            AK_DEBUG_OUTPUT("new  wosa\n");
        }
        else
        {
            speedIdx= 0;
            MediaLib_SetWSOLA_Arithmetic(player->hMedia, _SD_WSOLA_ARITHMATIC_0);
            AK_DEBUG_OUTPUT("old wosa\n");
        }
        #endif

        valVal= speedIdx2ValTbl[speedIdx][valIdx];
        #ifdef OS_ANYKA
        rst = MediaLib_SetWSOLA(player->hMedia,(T_U8)valVal);
    //  AK_DEBUG_OUTPUT("MediaLib_SetWSOLA,set speed:%d %d\n",rst, player->speedLibType);
        #endif
    }

    AK_DEBUG_OUTPUT("MediaLib_SetWSOLA,valIdx:%d,speed:%d,valVal:%d\n",speed,valIdx,valVal);
    
    return AK_TRUE;
}
#pragma arm section code
#pragma arm section code = "_audioplayer_menu_"
/**
 * @brief   Set EQ mode
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   toneMode
            _SD_EQ_MODE_NORMAL - 普通
            _SD_EQ_MODE_CLASSIC - 古典
            _SD_EQ_MODE_JAZZ - 爵士
            _SD_EQ_MODE_POP - 流行
            _SD_EQ_MODE_ROCK - 摇滚
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
T_BOOL  APlayer_SetToneMode(T_AUD_PLAYER* player,T_U8 toneMode)
{
    if (MEDIALIB_MEDIA_AV != player->mediaType)
    {
        if (_SD_EQ_USER_DEFINE == toneMode)
        {
        #ifdef OS_ANYKA
            MediaLib_Set3DSound(player->hMedia,5);
        #endif
        }
        else
        {
            MediaLib_SetEQMode(player->hMedia,toneMode);
        }

        AK_DEBUG_OUTPUT("EQ:%d\n",toneMode);
    }
    player->modeEQ = toneMode;
    return AK_TRUE;
}
#pragma arm section code


/**
 * @brief   Set Volume
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_U32
 * @return  T_BOOL
 * @retval  AK_TRUE-usable, AK_FALSE-unusable
 **/
#if(STORAGE_USED == SPI_FLASH)
#pragma arm section code = "audioplayer"
#else
#pragma arm section code = "_frequentcode_"
#endif
T_BOOL  APlayer_GetDuration(T_AUD_PLAYER* player,T_U32 *duration)
{
    MediaLib_GetCurTime(player->hMedia,duration);

    return AK_FALSE;
}

#pragma arm section code

#if(STORAGE_USED == NAND_FLASH|| STORAGE_USED == SD_CARD)
#pragma arm section code = "_audioplayer_resident_"
#else
#pragma arm section code = "_bootcode1_"
#endif

/**
 * @brief   handle normal music play
 * @author  WuShanwei
 * @date    2008-04-01
 * @param   T_VOID
 * @return  T_VOID
 * @retval  
 **/
T_S32   Aud_PlayerDecodeHandle(T_AUD_PLAYER* player)
{
    T_S32   nSize = -1;
    
    //Fwl_ConsoleWriteChr('@'); 
    if (Mutex_flag)
    {
        return 0;
    }
    else
    {
        Mutex_flag = 1;
    }
    
#ifdef SUPPORT_BLUETOOTH
    if(AK_TRUE == player->bset_eq_flag)
    {
        player->bset_eq_flag = AK_FALSE;
        APlayer_SetToneMode(player,player->modeEQ);    
    }
#endif

    if (AK_NULL != player->hMedia)
    {
        nSize = MediaLib_Handle(player->hMedia);
    }
        
    if (nSize < 0)
    {
        Mutex_flag = 0;
        return nSize;       
    }
#ifdef OS_ANYKA

    if (Pcm_Playing(player->pcm_id))
    {
        Fwl_ConsoleWriteChr('>'); 
    }

#endif
    Mutex_flag = 0;
    return nSize;
}
#pragma arm section code


#pragma arm section code = "_frequentcode_" 

T_BOOL  APlayer_LevDepStdby(T_AUD_PLAYER* player)
{
    T_MEDIALIB_MEDIA_INFO mediaInfo;
    T_PCM_INFO info;

    if (Fwl_WaveOutDACIsOpen())
        return AK_FALSE;
    
    player->pcm_id = Pcm_Open();//WaveOut_Open(DAC_HP, Aud_PlayerInterruptHandle);
    if(INVAL_PCMOPEN_ID == player->pcm_id)
    {
        AK_DEBUG_OUTPUT("Aud_PlayerOpenWave pcm open failed:%d\n", player->pcm_id);
        return AK_FALSE;
    }
    MediaLib_GetInfo(player->hMedia,&mediaInfo);
    info.bufsize = player->daBufLen;
    info.samplerate = mediaInfo.nSamplesPerSec;
    info.channel = 2;
    info.bps = mediaInfo.wBitsPerSample;
    
    Pcm_Start(player->pcm_id, &info);    
    aud_player_settask(player);
    return AK_TRUE;
}
#pragma arm section code

T_BOOL  APlayer_EntDepStdby(T_AUD_PLAYER* player)
{
    aud_player_unregistertask(player);
    Pcm_Stop(player->pcm_id);
    Pcm_Close(player->pcm_id);

    return AK_TRUE;
}
#endif


T_VOID APlayer_PrePostPop(T_VOID)
{
    Aud_PlayerHandlePre = (AUDPLAYER_CALLBACK_HANDLEPRE)CBFun_Pop(AUD_CB_PRE);
    Aud_PlayerHandlePost = (AUDPLAYER_CALLBACK_HANDLEPOST)CBFun_Pop(AUD_CB_POST);
}


