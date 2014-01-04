#include "Eng_CycBuf.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFs.h"
#include "Eng_debug.h"
#include <string.h>

#define MAX_CYCBUF_INSTANCE 5
#define ONE_BUFFER_SIZE     4096//0x1000
#define ONE_BUFSIZE_BIT     0xfff//4095
#define SHIFT_BIT           12
#define MAX_CYCBUF_COUNT    (MAX_CYCBUF_SIZE>>12)


//为了节省常驻空间，把指针数据改为偏移；
#ifdef OS_ANYKA				
#define INVAL_CYCBUF_INDEX  0xff                
extern T_U32        Image$$dynamic_bss$$Base;
#define DYNAMIC_SECTION_BASE ((T_U32)&Image$$dynamic_bss$$Base)
extern T_U32        Image$$dynamic_bss$$Limit;
#define DYNAMIC_SECTION_LIMIT ((T_U32)&Image$$dynamic_bss$$Limit)
#else
#define INVAL_CYCBUF_INDEX  0xff
#define DYNAMIC_SECTION_BASE 0
#define DYNAMIC_SECTION_LIMIT 0xffffffff
#endif

typedef enum
{
    CYCBUF_EMPTY = 0x1,
    CYCBUF_FULL  = 0x2,
    CYCBUF_NOMAL = 0x4,
}CYCBUF_STATE;

typedef struct 
{
    volatile T_U8   cycbuf_state;
    T_U8            bufcnt;
    volatile T_U16  writepos;
    volatile T_U16  readpos;
    T_U16           bufsize;
}T_CYCBUF_INFO;

#pragma arm section rwdata = "_cachedata_"
#ifdef OS_ANYKA
//为了节省常驻空间，把指针数据改为偏移；
T_U8 cycbuf[CYC_END_BUF_ID][MAX_CYCBUF_COUNT] = {INVAL_CYCBUF_INDEX};
#endif
#pragma arm section rwdata



#pragma arm section zidata = "_bootbss_"
T_CYCBUF_INFO cur_cycbuf_info[CYC_END_BUF_ID] = {0};

//指针数组的方式；
#ifdef OS_WIN32
T_U8 *cycbuf[CYC_END_BUF_ID][MAX_CYCBUF_COUNT] = {AK_NULL};
#endif
#pragma arm section zidata


T_U32 bufsize_arry[CYC_END_BUF_ID][MAX_CYCBUF_INSTANCE] = {0};
#pragma arm section code = "_audioplayer_open_code_"

static T_BOOL cycbuf_checksize(T_U32 CycBufID, T_U32 size, T_BOOL bclear)
{
    T_U8 i = 0;
    T_BOOL ret = AK_FALSE;

    for (i=0; i<MAX_CYCBUF_INSTANCE; i++)
    {
        if (bufsize_arry[CycBufID][i] == size)
        {
            if (bclear)
            {
                bufsize_arry[CycBufID][i] = 0;
            }
            ret = AK_TRUE;
            break;
        }
    }

    return ret;
}

static T_BOOL cycbuf_addsize(T_U32 CycBufID, T_U32 bufcnt)
{   
    T_U8 i; 
    T_BOOL ret = AK_FALSE;

    if (bufcnt > MAX_CYCBUF_COUNT)
    {
        return AK_FALSE;
    }
    
    for (i=0; i<MAX_CYCBUF_INSTANCE; i++)
    {
        if (bufsize_arry[CycBufID][i] == 0)
        {
            bufsize_arry[CycBufID][i] = bufcnt<<SHIFT_BIT;
            ret = AK_TRUE;
            break;
        }
    }
    
    return ret;
}

static T_BOOL cycbuf_bufcreate(T_U32 CycBufID, T_U32 bufcnt)
{
    T_U8 i;
    T_U8 *addr;

    if (bufcnt > MAX_CYCBUF_COUNT)
    {
        return AK_FALSE;
    }

    akerror("  cycbuf cnt:", bufcnt, 1);
    for (i=0; i<bufcnt; i++)
    {
#ifdef OS_ANYKA
//用偏移的方式  
        if (INVAL_CYCBUF_INDEX == cycbuf[CycBufID][i])
        {
            addr = (T_U8*)Fwl_DMAMalloc(ONE_BUFFER_SIZE);
            if (AK_NULL == addr)
            {
                akerror("  cycbuf create buf failed!", 0, 1);
                return AK_FALSE;
            }
            //申请出来的地址是4k对齐的，这里计算是第几个4kpage
            cycbuf[CycBufID][i] = (T_U8)(((T_U32)addr-DYNAMIC_SECTION_BASE)>>SHIFT_BIT);
        }
#else
//指针数组的方式    
        if (AK_NULL == cycbuf[CycBufID][i])
        {
            cycbuf[CycBufID][i] = (T_U8*)Fwl_DMAMalloc(ONE_BUFFER_SIZE);
            
            akerror("  cycbuf malloc:", cycbuf[CycBufID][i], 1);
            if (AK_NULL == cycbuf[CycBufID][i])
            {
                akerror("  cycbuf create buf failed!", 0, 1);
                return AK_FALSE;
            }
        }
#endif
    }


    return AK_TRUE;
}

T_VOID cycbuf_clear(T_U32 CycBufID)
{
    cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_EMPTY;
    cur_cycbuf_info[CycBufID].readpos = 0;
    cur_cycbuf_info[CycBufID].writepos = 0;
}

static T_BOOL cycbuf_bufdecrease(T_U32 CycBufID, T_U32 bufcnt)
{
    T_U8 i;

    if (bufcnt > (T_U32)cur_cycbuf_info[CycBufID].bufcnt)
    {
        return AK_FALSE;
    }

    for (i=MAX_CYCBUF_COUNT-1; i>=bufcnt; i--)
    {
#ifdef OS_ANYKA
//用偏移的方式  
        if (INVAL_CYCBUF_INDEX != cycbuf[CycBufID][i])
        {
            Fwl_DMAFree(DYNAMIC_SECTION_BASE+(cycbuf[CycBufID][i]<<SHIFT_BIT));
            cycbuf[CycBufID][i] = INVAL_CYCBUF_INDEX;
        }
        
#else
//指针数组的方式    
        if (AK_NULL != cycbuf[CycBufID][i])
        {
            akerror("  cycbuf free:", i, 1);
            Fwl_DMAFree(cycbuf[CycBufID][i]);
            cycbuf[CycBufID][i] = AK_NULL;
        }
#endif
        if (i == 0)
        {
            break;
        }
    }

    return AK_TRUE;
}

/*********************************************************************
  Function:         cycbuf_create
  Description:      create cycbuf
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size you want to create
  Return:           create succeed or not
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_BOOL cycbuf_create(T_U32 CycBufID, T_U32 size)
{
    T_U32 bufcnt;
    T_U32 i;

	if(size >= T_U16_MAX)
	{
		return AK_FALSE;
	}
	
    cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_EMPTY;
    cur_cycbuf_info[CycBufID].readpos = 0;
    cur_cycbuf_info[CycBufID].writepos = 0;

    bufcnt = (size+ONE_BUFFER_SIZE-1)>>SHIFT_BIT;

    if (cycbuf_checksize(CycBufID, bufcnt<<SHIFT_BIT, AK_FALSE))
    { 
        if (!cycbuf_addsize(CycBufID, bufcnt))
        {
            akerror("  cycbuf instance full 2!", 0, 1);
            return AK_FALSE;
        }   
        return AK_TRUE;
    }   
#ifdef OS_ANYKA
//初始化偏移地址buffer
    if (0 == cur_cycbuf_info[CycBufID].bufsize)
    {
        for(i=0; i<MAX_CYCBUF_COUNT; i++)
            cycbuf[CycBufID][i] = INVAL_CYCBUF_INDEX;
    }
#endif
/*
    if (cycbuf_checksize(size, AK_FALSE))
    {
        return (size&0xfff)?((size>>12)+1)<<12:size;
    }
    */
//  bufcnt = (size+ONE_BUFFER_SIZE-1)/ONE_BUFFER_SIZE;

    if (bufcnt > MAX_CYCBUF_COUNT)
    {
        akerror("   cycbuf bufcnt > maxbufcount!", 0, 1);
        return AK_FALSE;
    }

    if (!cycbuf_addsize(CycBufID, bufcnt))
    {
        akerror("  cycbuf instance full!", 0, 1);
        return AK_FALSE;
    }

    if (bufcnt > (T_U32)cur_cycbuf_info[CycBufID].bufcnt)
    {
        if (!cycbuf_bufcreate(CycBufID, bufcnt))
        {
            cycbuf_bufdecrease(CycBufID, (T_U32)cur_cycbuf_info[CycBufID].bufcnt);
            akerror("   cycbuf create failed:", bufcnt, 1);
            return AK_FALSE;
        }
        else
        {
            cur_cycbuf_info[CycBufID].bufcnt = (T_U16)bufcnt;
            cur_cycbuf_info[CycBufID].bufsize = bufcnt<<SHIFT_BIT;
        }
    }

    return AK_TRUE;
}
#pragma arm section code


#pragma arm section code = "_bootcode1_"
/*********************************************************************
  Function:         cycbuf_getblanksize
  Description:      get cycbuf blank size
  Input:            T_U32 CycBufID: the cyc buffer id
  Return:           blank size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getblanksize(T_U32 CycBufID)
{
    if (CYCBUF_EMPTY == cur_cycbuf_info[CycBufID].cycbuf_state)
    {
        return cur_cycbuf_info[CycBufID].bufsize;
    }
    
    if (cur_cycbuf_info[CycBufID].writepos>cur_cycbuf_info[CycBufID].readpos)
    {
        return (cur_cycbuf_info[CycBufID].bufsize + cur_cycbuf_info[CycBufID].readpos - cur_cycbuf_info[CycBufID].writepos);
    }
    else
    {
        return (cur_cycbuf_info[CycBufID].readpos - cur_cycbuf_info[CycBufID].writepos);
    }
}
/*********************************************************************
  Function:         cycbuf_getdatasize
  Description:      get cycbuf data size
  Input:            T_U32 CycBufID: the cyc buffer id
  Return:           data size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getdatasize(T_U32 CycBufID)
{
    return (cur_cycbuf_info[CycBufID].bufsize - cycbuf_getblanksize(CycBufID));
}

/*********************************************************************
  Function:         cycbuf_getwritebuf
  Description:      get a address which could be writed
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            buf: the data buf you want to write  
  Input:            T_U32 size: the size you want to write
  Return:           write size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

//0xfff = ONE_BUFFER_SIZE-1;
T_U32 cycbuf_getwritebuf(T_U32 CycBufID, T_U8** buf, T_U32 size)
{
    T_U32 size1, size2;

#if 0   
    if (cur_cycbuf_info.binit)
    {
    //偏移的方式
        *buf = DYNAMIC_SECTION_BASE+(cycbuf[0]<<12);

    /*
    //指针数组的方式
        *buf = cycbuf[0];
        */
        return (ONE_BUFFER_SIZE>size?size:ONE_BUFFER_SIZE);
    }
#endif

    size1 = ONE_BUFFER_SIZE - (cur_cycbuf_info[CycBufID].writepos&ONE_BUFSIZE_BIT);
    size2 = cycbuf_getblanksize(CycBufID);

    size1 = size1>size2?size2:size1;

#ifdef OS_ANYKA
    //偏移的方式
    *buf = DYNAMIC_SECTION_BASE+(cycbuf[CycBufID][cur_cycbuf_info[CycBufID].writepos>>SHIFT_BIT]<<SHIFT_BIT)+(cur_cycbuf_info[CycBufID].writepos&ONE_BUFSIZE_BIT);
#else

    //指针数组的方式
    *buf = cycbuf[CycBufID][cur_cycbuf_info[CycBufID].writepos>>12]+(cur_cycbuf_info[CycBufID].writepos&0xfff);
#endif


    return (size1>size?size:size1);
}

/*********************************************************************
  Function:         cycbuf_getdatabuf
  Description:      get a address which could be read
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            buf: the data buf you want to read  
  Input:            T_U32 size: the size you want to read
  Return:           actul read size
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_U32 cycbuf_getdatabuf(T_U32 CycBufID, T_U8** buf, T_U32 size)
{
    T_U32 size1, size2;
    
    size1 = ONE_BUFFER_SIZE - (cur_cycbuf_info[CycBufID].readpos&ONE_BUFSIZE_BIT);
    size2 = cycbuf_getdatasize(CycBufID);
    
    size1 = size1>size2?size2:size1;
#ifdef OS_ANYKA
    //偏移的方式
    *buf = DYNAMIC_SECTION_BASE+(cycbuf[CycBufID][cur_cycbuf_info[CycBufID].readpos>>SHIFT_BIT]<<SHIFT_BIT)+(cur_cycbuf_info[CycBufID].readpos&ONE_BUFSIZE_BIT);
    
#else
    //指针数组的方式    
    *buf = cycbuf[CycBufID][cur_cycbuf_info[CycBufID].readpos>>12]+(cur_cycbuf_info[CycBufID].readpos&0xfff);
#endif

    return (size1>size?size:size1);
}

/*********************************************************************
  Function:         cycbuf_write_updateflag
  Description:      update the cycbuf write flag 
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size had been writed in cycbuf
  Return:           T_VOID
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_VOID cycbuf_write_updateflag(T_U32 CycBufID, T_U32 size)
{
//  cur_cycbuf_info.binit = AK_FALSE;
    if(!size)
    {
        return;
    }
    
    cur_cycbuf_info[CycBufID].writepos += size;
    
    if (cur_cycbuf_info[CycBufID].writepos >= cur_cycbuf_info[CycBufID].bufsize)
    {
        cur_cycbuf_info[CycBufID].writepos -= cur_cycbuf_info[CycBufID].bufsize;
    }
    
    if(cur_cycbuf_info[CycBufID].writepos == cur_cycbuf_info[CycBufID].readpos)
    {
        cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_FULL;
    }
    else
    {
        cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_NOMAL;
    }
}

/*********************************************************************
  Function:         cycbuf_read_updateflag
  Description:      update the cycbuf read flag 
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size had been read in cycbuf
  Return:           T_VOID
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_VOID cycbuf_read_updateflag(T_U32 CycBufID, T_U32 size)
{
    if(!size)
    {
        return;
    }

    cur_cycbuf_info[CycBufID].readpos += size;
    
    if (cur_cycbuf_info[CycBufID].readpos >= cur_cycbuf_info[CycBufID].bufsize)
    {
        cur_cycbuf_info[CycBufID].readpos -= cur_cycbuf_info[CycBufID].bufsize;
    }

    if(cur_cycbuf_info[CycBufID].writepos == cur_cycbuf_info[CycBufID].readpos)
    {
        cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_EMPTY;
    }
    else
    {
        cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_NOMAL;
    }
}
#pragma arm section code


static T_U32 cycbuf_getmaxsize(T_U32 CycBufID)
{
    T_U8 i;
    T_U32 size = 0;

    for (i=0; i<MAX_CYCBUF_INSTANCE; i++)
    {
        if (bufsize_arry[CycBufID][i] > size)
        {
            size = bufsize_arry[CycBufID][i];
        }
    }

    return size;
}

/*********************************************************************
  Function:         cycbuf_destory
  Description:      destory the cycbuf
  Input:            T_U32 CycBufID: the cyc buffer id
  Input:            T_U32 size: the size you want to destory
  Return:           SUCCEED:AK_TRUE   FAILED:AK_FALSE
  Author:           lishengkai
  Data:             2013-3-6
**********************************************************************/

T_BOOL cycbuf_destory(T_U32 CycBufID, T_U32 size)
{
    T_U32 maxsize, tmpsize;

    tmpsize = ((size+ONE_BUFFER_SIZE-1)>>SHIFT_BIT)<<SHIFT_BIT;
//  tmpsize = size&0xfff?((size>>12+1)<<12):size;
    
    if (!cycbuf_checksize(CycBufID, tmpsize, AK_TRUE))
    {
        return AK_FALSE;
    }
	
    cur_cycbuf_info[CycBufID].readpos = 0;
	cur_cycbuf_info[CycBufID].writepos= 0;
	cur_cycbuf_info[CycBufID].cycbuf_state = CYCBUF_EMPTY;
	
    if (cur_cycbuf_info[CycBufID].bufsize != tmpsize)
    {
        return AK_TRUE;
    }

    maxsize = cycbuf_getmaxsize(CycBufID);

    cycbuf_bufdecrease(CycBufID, (maxsize+ONE_BUFFER_SIZE-1)>>SHIFT_BIT);

    cur_cycbuf_info[CycBufID].bufcnt = (T_U16)((maxsize+ONE_BUFFER_SIZE-1)>>SHIFT_BIT);
    cur_cycbuf_info[CycBufID].bufsize = ((T_U32)cur_cycbuf_info[CycBufID].bufcnt)<<SHIFT_BIT;
    
    return AK_TRUE;
}


