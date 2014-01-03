#ifndef        _UPDATE_SELF_H_
#define        _UPDATE_SELF_H_
#include "fwl_spiflash.h"

#define SPI_UPDATE_SELF_MAX_STACK  1024

/** define the file handler*/
#ifndef T_hFILE
#define	T_hFILE					T_S32		/* FILE * */
#endif

#ifndef FS_SEEK_SET
#define FS_SEEK_SET			0
#endif

#ifndef FS_SEEK_CUR	
#define FS_SEEK_CUR			1	
#endif

#ifndef FS_SEEK_END
#define FS_SEEK_END			2
#endif

#ifndef FS_INVALID_HANDLE
#define FS_INVALID_HANDLE		-1
#endif

typedef T_pVOID (*Prod_RamAlloc)(T_U32 size);
typedef T_pVOID (*Prod_RamReAlloc)(T_pVOID var, T_U32 size);
typedef T_pVOID (*Prod_RamFree)(T_pVOID var);
typedef T_pVOID (*Prod_MemSet)(T_pVOID pBuf, T_S32 value, T_U32 count);
typedef T_pVOID (*Prod_MemCpy)(T_pVOID dst, T_pVOID src, T_U32 count);
typedef T_S32   (*Prod_MemCmp)(T_pVOID pbuf1, T_pVOID pbuf2, T_U32 count);
typedef T_pVOID (*Prod_MemMov)(T_pVOID dst, T_pVOID src, T_U32 count);

typedef struct tag_ProdMemFunc
{
    Prod_RamAlloc         RamAlloc;
    Prod_RamFree          RamFree;
    Prod_RamReAlloc       RamReAlloc;
    Prod_MemSet           MemSet;
    Prod_MemCpy           MemCpy;
    Prod_MemCmp           MemCmp;
    Prod_MemMov           MemMov;
}T_PRODMEMFUNC, *T_PPRODMEMFUNC;

typedef T_VOID (*Prod_Printf)(T_pCSTR s, ...);
typedef T_BOOL (*Prod_EraseBlock)(T_U32 chip, T_U32 block);
typedef T_BOOL (*Prod_WritePage)(T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 spare[], T_U32 spareLen);
typedef T_BOOL (*Prod_ReadPage)(T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 spare[], T_U32 spareLen);
typedef T_BOOL (*Prod_WriteBootPage)(T_U32 page, T_U8 data[]);
typedef T_BOOL (*Prod_ReadBootPage)(T_U32 page, T_U8 data[]);
typedef T_BOOL (*Prod_WriteASAPage)(T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 spare[], T_U32 spareLen);
typedef T_BOOL (*Prod_ReadASAPage)(T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 spare[], T_U32 spareLen);
typedef T_BOOL (*Prod_ReadBytes)(T_U32 chip, T_U32 rowAddr, T_U32 colAddr, T_U8 data[], T_U32 len);

typedef struct tag_ProdDriverFunc
{
    Prod_Printf               Printf;
    Prod_EraseBlock       EraseBlock;
    Prod_WritePage        WritePage;
    Prod_ReadPage         ReadPage;
    Prod_WriteBootPage    WriteBootPage;
    Prod_ReadBootPage     ReadBootPage;
    Prod_WriteASAPage     WriteASAPage;
    Prod_ReadASAPage      ReadASAPage;
    Prod_ReadBytes        ReadBytes;
}T_PRODDRIVERFUNC, *T_PPRODDRIVERFUNC;

//typedef T_hFILE (*FS_FILE_Open)(T_pCSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
typedef T_BOOL  (*FS_FILE_Close)(T_hFILE hFile);
typedef T_U32   (*FS_FILE_Read)(T_hFILE hFile, T_pVOID buffer, T_U32 count);
typedef T_U32   (*FS_FILE_Write)(T_hFILE hFile, T_pVOID buffer, T_U32 count);
typedef T_S32   (*FS_FILE_Seek)(T_hFILE hFile, T_S32 offset, T_U16 origin);
typedef T_BOOL  (*FS_FILE_MkDir)(T_pCSTR path, T_U32 mode);
typedef T_S32   (*Asc_To_Unicode)(const T_CHR *src, T_U32 srcLen,T_U16 *ucBuf, T_U32 ucBufLen);
typedef T_S32   (*Unicode_To_Asc)(const T_U16 *src, T_U32 srcLen,T_CHR *ucBuf, T_U32 ucBufLen);

typedef struct tag_ProdFileFunc
{
//    FS_FILE_Open        FileOpen;
//    FS_FILE_Close       FileClose;
    FS_FILE_Read        FileRead;
//    FS_FILE_Write       FileWrite;
    FS_FILE_Seek        FileSeek;
//    FS_FILE_MkDir       FsMkDir;
//    Asc_To_Unicode      A2U;
//    Unicode_To_Asc      U2A;
 }T_PRODFILEFUNC, *T_PPRODFILEFUNC;

typedef struct tag_ProdCallback
{
    T_PRODMEMFUNC       fMem;
    T_PRODDRIVERFUNC    fDriver;
    T_PRODFILEFUNC      fFs;  
	T_U8                *pComBuf;//在初始化的时候一次性申请 
}T_PRODCALLBACK;

typedef struct tag_SPIFunc
{
    Prod_RamAlloc         RamAlloc;
    Prod_RamFree          RamFree;
    Prod_MemSet           MemSet;
    Prod_MemCpy           MemCpy;
    Prod_MemCmp           MemCmp;
    Prod_Printf           Printf;
    FS_FILE_Read          fSeek;
    FS_FILE_Read          fRead;  
}T_SPIBURNFUNC, *T_PSPIBURNFUNC;
typedef struct tagSFLASH_PARAM
{
    T_U32   id;                     ///< flash id
    T_U32   total_size;             ///< flash total size in bytes
    T_U32   page_size;              ///< bytes per page
    T_U32   program_size;           ///< program size at 02h command
    T_U32   erase_size;             ///< erase size at d8h command 
    T_U32   clock;                  ///< spi clock, 0 means use default clock 

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
    //bit 2: AAI flag, the serial flash support auto address increment word programming 
    T_U8    flag;                   ///< chip character bits
    T_U8    protect_mask;           ///< protect mask bits in status register:BIT2:BP0, BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
    T_U8    reserved1;
    T_U8    reserved2;
}T_SFLASH_PARAM;


T_BOOL UnpacketSpiFileInit(T_SPIBURNFUNC* fSpi, T_hFILE hFile, T_SFLASH_PARAM *spi_info);
T_BOOL UnpacketSpiBootFile(T_hFILE hFile, T_BOOL bCompare);
T_BOOL UnpacketSpiBinFile(T_hFILE hFile);

typedef struct tagUpdateFileHead
{
    T_U8    head_info[8];                       //文件头信息，固定为"anyka106"
    T_U32   format_count;                       //format信息个数
    T_U32   format_offset;                      //format信息偏移量
    T_U32   nand_count;                         //nand信息个数
    T_U32   nand_offset;                        //nand信息偏移量
    T_U32   producer_offset;                    //producer偏移量
    T_U32   producer_size;                      //producer文件大小
    T_U32   bios_offset;                        //bios偏移量
    T_U32   bios_size;                          //bios文件大小
    T_U32   udisk_info_count;                   //udisk目录信息个数
    T_U32   udisk_info_offset;                  //udisk目录信息偏移量
    T_U32   udisk_file_count;                   //udisk文件个数
    T_U32   udisk_file_info_offset;             //udisk文件信息偏移量
    T_U32   file_count;                         //nand文件个数
    T_U32   file_info_offset;                   //nand信息偏移量
    T_U32   config_tool[8][2];                  //配置工具文件信息使用,config_tool[0]--ImageRse,config_tool[1]--PROG

    //烧录配置信息使用: 
    //Other_config[0]:Usb mode
    //Other_config[1]:NonFS reserve size
    //Other_config[2]:fs reserve size
    //Other_config[3]:volumelable data offset
    //Other_config[4]:bUpdateself
    T_U32   Other_config[8];
    T_U32   check_sum;                                
}
T_UPDATE_FILE_HEAD;

typedef struct tagFileInfo
{
    T_U32   bCompare;                           //是否比较
    T_U32   ld_addr;                            //链接地址
    T_U32   file_length;                        //文件长度
    T_U32   file_offset;                        //文件偏移量
    T_U8    file_name[16];                      //文件名（存储在设备上文件名）
    T_U32   check_sum;
}
T_FILE_INFO;

#endif //_UPDATE_SELF_H_


