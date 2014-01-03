#ifndef __FWL_SPIFLASH_H__
#define __FWL_SPIFLASH_H__

#ifdef OS_ANYKA

#define CALC_SPI_PAGES(pagesize, erasesize, res)  ((erasesize/pagesize)*(((res) -1)/(erasesize) + 1))

#define SPIFLASH_PAGE_SIZE          sflash_get_pagesize()//(256)
#define SPIFLASH_BOLCK_SIZE         sflash_get_erasesize()//(64*1024)
#define SPIFLASH_PAGE_PER_BLOCK     sflash_get_blockpgs()//(SPIFLASH_BOLCK_SIZE/SPIFLASH_PAGE_SIZE)
#define DEEPSTDB_PAGE_NUM           sflash_get_deepstandby_pages()//

typedef enum T_spi_port{
    Sflash_1 = 0,
    Sflash_2 = 0,
    Sflash_num
}SFlash_ID;

typedef enum
{
    SFLASH_BUS1 = 0,
    SFLASH_BUS2,
    SFLASH_BUS4,
    SFLASH_BUS_NUM
}T_eSFLASH_BUS;


typedef struct
{
    T_U32   id;                     ///< flash id
    T_U32   total_size;             ///< flash total size in bytes
    T_U32   page_size;              ///< total bytes per page
    T_U32   program_size;           ///< program size at 02h command
    T_U32   erase_size;             ///< erase size at d8h command 
    T_U32   clock;                  ///<spi clock, 0 means use default clock 

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
    T_U8    flag;                   ///< chip character bits
    T_U8    protect_mask;           ///< protect mask bits in status register:BIT2:BP0, BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
    T_U8    reserved1;
    T_U8    reserved2;
}SFlash_PARAM,*pSFlash_PARAM;

T_BOOL Fwl_spiflash_init(SFlash_ID sflash_id, T_eSFLASH_BUS bus_width);
T_VOID Fwl_spiflash_set(pSFlash_PARAM sflash_param);
T_U32 Fwl_spiflash_getid(T_VOID);
T_BOOL Fwl_spiflash_write(T_U32 page, T_U8 *buf, T_U32 page_cnt);
T_BOOL Fwl_spiflash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt);
T_BOOL Fwl_spiflash_erase(T_U32 sector);

T_U32 sflash_remap_read(T_U32 rowAddr,  T_U8 *data,  T_U32 *spare, T_BOOL  bBin);
T_BOOL sflash_remap_init(T_pVOID pParam);
T_U32 sflash_get_pagesize(T_VOID);
T_U32 sflash_get_deepstandby_pages(T_VOID);
T_U32 sflash_get_blockpgs(T_VOID);

T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage);

T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);

T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);

#endif

#endif

