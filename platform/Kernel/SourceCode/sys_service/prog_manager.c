/************************************************************************
* Copyright (c) 2011, Anyka Co., Ltd. 
* All rights reserved.  
*  
* File Name：prog_manager.c
* Function：offer the program bin file manage interfence
*
* Author：xuping
* Date：2011-03-02
* Version：0.1.0
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
#include "prog_manager.h"
#include "remap.h"
#include "Eng_Debug.h"
#include "asa.h"
#include "anyka_bsp.h"
#include "Fwl_NandFlash.h"
#include "arch_nand.h"
#include "mmu.h"
#include "arch_pmu.h"
#include "hal_mmu.h"
#include "utils.h"
#include "nandflash.h"
#include "arch_interrupt.h"
#include "arch_init.h"
#include "arch_timer.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Console.h"
#include "Fwl_System.h"
#include "boot.h"
#include "Fwl_nandflash.h"
#include <string.h>
#include "Gbl_global.h"
#include "Fwl_Mount.h"
#include "Fwl_residentstr.h"
#include "Fwl_Console.h"
#if (STORAGE_USED == SPI_FLASH)
#include "Fwl_spiflash.h"
#endif
#if (STORAGE_USED == SD_CARD)
#include "Fwl_SD.h"
#include "arch_mmc_sd.h"
#endif
#ifdef OS_ANYKA


#define MAX_SWAP_BLK_NUM    (320)    //40M

#define NAND_SWAP_RESV_BLOCK    4   //reserve blocks for replacement

#if(STORAGE_USED == SD_CARD)
#define SD_SECTOR_SIZE      512
#define SD_BOOT_SECTOR      3
#define PHPAGE_2VPAGE_NUM   8   //物理页对应虚拟页(4k)长度需要的数目
#define progmanage_erase_data(chip, blk_start_page)         (0)       
#define progmanage_read_data(rowAddr, data, spare, bBin)    !Fwl_SD_Read(sys_sd_handle,rowAddr,data,8)                               
#define progmanage_write_data                               
#define progmanage_get_bad_area     
#define progmanage_set_bad_area
#define check_bad_block(block)                              (0)
#elif(STORAGE_USED == NAND_FLASH)
#define progmanage_erase_data                               nand_eraseblock 
#define progmanage_read_data                                nand_remap_read   
#define progmanage_write_data                               nand_remap_write 
#define progmanage_get_bad_area                             asa_get_bad_block
#define progmanage_set_bad_area                             asa_set_bad_block
#define check_bad_block(block) \
    (prog_manage.bad_blk_bitmap[(block)>>3] & (1<<(7-((block)&0x7))))  
#elif(STORAGE_USED == SPI_FLASH)
#define PHPAGE_2VPAGE_NUM   16  //物理页对应虚拟页(4k)长度需要的数目
#define progmanage_erase_data(chip, blk_start_page)         (0)
#define progmanage_read_data                                !sflash_remap_read   
#define progmanage_write_data(rowAddr, data, spare, bBin)   (0)                         
#define progmanage_get_bad_area                         
#define progmanage_set_bad_area                         
#define check_bad_block(block)                              (AK_FALSE)
#else
#error "请定义STORAGE_USED为哪种类型的存储设备"
#endif

typedef struct
NandConfig{
    T_U8 MainVer;                   //Main version number
    T_U8 SubVer;                    //Subsidiary version number
    T_U8 Sub1Ver;                   //Subsidiary version number
    T_U8 Reserve;                   //Reserve
    T_U32 FileCount;
    T_U32 StartBlock;
    T_U32 BlockCount;
}T_NAND_CONFIG;

typedef struct
{
    T_U32 file_length;
    T_U32 ld_addr;
    T_U32 map_index;    
    T_U32 run_pos;
    T_U32 update_pos;
    T_U8 file_name[16];
}
T_FILE_CONFIG;


typedef struct {
    T_U32 start_blk;    /* the start block of swap area in nand mode, and the start serial space in sd mode*/
    T_U32 end_blk;      /* the end block of swap area in nand mode, and the start serial space in sd mode*/
    T_U32 block;        /* the current using block（serial space ） in swap area*/
    T_U32 page_offst;   /* the current using page（sector） in swap area*/
}T_SWAP_INFO;  

typedef struct
{
    T_U16 primary;
    T_U16 backup;
}T_DEEPSTDB_BLK;

typedef struct{
#if(STORAGE_USED == NAND_FLASH)
    T_U16            ram2store_tbl[REMAP_MEM_PSIZE/VPAGE_SIZE]; /* memory to storage mapping table in ram*/
#else
    T_U16            mem2store_phstart; //物理页起始索引
    T_U16            mem2store_vstart;  //虚拟页非常驻区的起始索引
    T_U32            mem2store_romlen;  //程序代码 Rom长度(虚拟页数目) 
#endif

    T_SWAP_INFO      swap;  /* swap info */
#if(STORAGE_USED == NAND_FLASH)
    T_U8             bad_blk_bitmap[(MAX_SWAP_BLK_NUM/8)]; /* bad block bitmap, */
#endif
    T_U8             nbit;  /* nandflash page size bitnum  or sdcard sector size bitnum*/
    T_U8             mbit;  /* nandflash page number bitnum or sdcard serial space sector number bitnum*/
    T_U8             M;     /* availiable block（serial space） number */
    T_U8             protect; /* protect nand read-only area or not */
#ifdef SWAP_WRITBAK_PRINT
    volatile T_U32   swap_cnt;
    volatile T_U32   writbak_cnt;
#endif
}T_PROG_MANAGE;

#ifdef CHECK_STARTUPTICK
extern T_U32 g_StartupTick;
extern T_U32 g_ResumeStartTick;
#endif

extern T_U32 g_nPageSize;
extern T_U32 g_nPagePerBlock_asa;
extern T_U32 g_nBlockPerChip;
extern T_U8  g_nChipCnt;
extern T_U32 Image$$bootbss$$ZI$$Limit;
extern T_U32 Image$$rodata$$Limit;
extern T_U32 Image$$bootbss1$$ZI$$Limit;
extern T_U32 Image$$data$$Base;
extern T_U32 Image$$init$$Limit;
extern T_U32 Image$$dynamic_bss$$Base;
extern T_U32 Image$$dynamic_bss$$ZI$$Limit;
extern T_U32 Image$$mmidata$$Base;
extern T_U32 Image$$ER_ZI$$ZI$$Limit;
extern T_VOID analog_init(T_VOID);

#pragma arm section zidata = "_bootbss_"
static T_PROG_MANAGE prog_manage;
#if 0
volatile T_BOOL checkirq; //request no check irq mode 
#endif
#if(STORAGE_USED == SD_CARD)
T_pCARD_HANDLE sys_sd_handle = AK_NULL;
T_MEM_DEV_ID bootSdIndex = MMC_SD_CARD;
#endif

//#pragma arm section rwdata = "_cachedata_"    //放在bootbss 前面的段
T_U32 m_ulDeepSP;               //record SP register
T_U32 s_ulDeepSum;              //check sum for fixed data
T_U32 s_ulBlkCount;             //count the swap block that has used

#ifdef DEEP_STANDBY
T_fPOWEROFF_CALLBACK power_off_callback = AK_NULL;
#if(STORAGE_USED == NAND_FLASH)
static T_DEEPSTDB_BLK stdb_blk_save;   //deep standby block position
#else
T_U32 tempIndex;
T_U8 *tempBuffer; 
#endif
#endif//#ifdef DEEP_STANDBY
#pragma arm section zidata

#ifdef DEEP_STANDBY
#pragma arm section rodata = "_deepstdb_"
//put the read/write character for printing
static const T_S8 s_cDeep_poweroff[]  = "power off!\n";
static const T_S8 s_cDeep_erase[]     = "erase:";
static const T_S8 s_cDeep_max[]       = ", max="; //the maximum counter
static const T_S8 s_cDeep_blk[]       = "block=";
static const T_S8 s_cDeep_cnt[]       = ", cnt=";
static const T_S8 s_cDeep_sum[]       = "sum=";
static const T_S8 s_cDeep_ret_line[]  = "retline=";
static const T_S8 s_cDeep_W8[]        = "W8: ";   //DMA alloc over:
static const T_S8 s_cDeep_W9[]        = "W9: ";   //deep standby block or counter verify failed.
static const T_S8 s_cDeep_W10[]       = "W10: ";  //sum verify failed: 
static const T_S8 s_cDeep_W11[]       = "W11: ";  //reserve pages over: 
static const T_S8 s_cDeep_W12[]       = "W12: ";  //phy2virTab pages over: 
static const T_S8 s_cDeep_W13[]       = "W13: ";  //maybe just one swap block is good: 
static const T_S8 s_cDeep_W14[]       = "W14: ";  //reserve block is: 
static const T_S8 s_cDeep_W15[]       = "W15: ";  //param is wrong when saving fixed data: 
static const T_S8 s_cDeep_W16[]       = "W16: ";  //flush failed.
static const T_S8 s_cDeep_W17[]       = "W17: ";  //fixed data too many: 
static const T_S8 s_cDeep_W18[]       = "W18: ";  //swap info is wrong! 
static const T_S8 s_cDeep_W19[]       = "W19: ";  //sys_reset for deepstdb_load error: 

#ifdef DEEP_DEBUG
static const T_S8 s_cDeep_flag[]      = " flag:"; //deep standby flag and cancel flag
static const T_S8 s_cDeep_resv[]      = "r=";
static const T_S8 s_cDeep_index[]     = "i=";
static const T_S8 s_cDeep_vaddr[]     = ", vaddr=";
static const T_S8 s_cDeep_size[]      = ", size=";
static const T_S8 s_cDeep_find_max[]  = " ****";  //look for the max counter
static const T_S8 s_cDeep_blk_start[] = "start:";
static const T_S8 s_cDeep_blk_end[]   = "end:";
#endif
#pragma arm section rodata
#endif //#ifdef DEEP_STANDBY

#pragma arm section rodata = "_initphase_rodata_"
static const T_S8 s_initphase[]       = "initphase:"; 
static const T_S8 s_initphase_end[]   = "pass initphase."; 
#pragma arm section rodata

#pragma arm section rodata = "_bootconst_"
static const T_CHR err_str[] = ERROR_PROG;
#pragma arm section rodata

//program string 
#define PROG_FILE_NAME        "PROG"


#define PROGSTR_INIT          s_initphase
#define PROGSTR_INIT_END      s_initphase_end

#define NAND_CONFIG_PHY_PAGENUM     ((1 << prog_manage.mbit)-2)    
#define FILE_CONFIG_PHY_PAGENUM     ((1 << prog_manage.mbit)-3) 

#define bootbss_end    ((T_U32)&Image$$bootbss$$ZI$$Limit)
#define rodata_end     (((T_U32)&Image$$rodata$$Limit)-1)
#define mmidata_start  ((T_U32)&Image$$mmidata$$Base)    
#define zibss_end        ((T_U32)&Image$$ER_ZI$$ZI$$Limit)


#ifdef DEEP_STANDBY
#define fixed_data_start    ((T_U32)&Image$$data$$Base)
#define fixed_data_end	    ((T_U32)&Image$$bootbss$$ZI$$Limit)     //same to bootbss_end
#define mmu_ptdata_start            (_MMUPT_PADDRESS)
#define mmu_ptdata_size             (_MMU_PT_SIZE)  

#if(STORAGE_USED == SPI_FLASH)
#define DEEP_FILE             "deep"
#define fixed_data_start_spi        ((T_U32)&Image$$init$$Limit)
#define DEEP_FIXDATA_PAGES_SPI      (((fixed_data_end-fixed_data_start_spi-1)>> prog_manage.nbit) + 1)
#define deep_save_flag              (0x01)
#define DEEPSTDB_BOLCK_START        (1)
#define SPIFLASH_PAGE_PER_VPAGE     (VPAGE_SIZE/SPIFLASH_PAGE_SIZE)
#define DEEP_UNFIXDATA_OFFSET       ((DEEP_FIXDATA_PAGES_SPI+2)*SPIFLASH_PAGE_PER_VPAGE)
#else
//#define fixed_data_start            ((T_U32)&Image$$data$$Base)
//#define fixed_data_end              ((T_U32)&Image$$bootbss$$ZI$$Limit)     //same to bootbss_end
#define heap_start                  ((T_U32)&Image$$dynamic_bss$$Base)
#define heap_end                    ((T_U32)&Image$$dynamic_bss$$ZI$$Limit)     //same to bootbss_end
//#define mmu_ptdata_start            (_MMUPT_PADDRESS)
//#define mmu_ptdata_size             (_MMU_PT_SIZE)  

#define DEEP_RESUM_CODE_VPAGE       (10)          //don't modify random
#define DEEP_MAX_U32                (0xffffffff)  //0xffffffff
#define DEEP_VIR2PHY_PAGES          ((T_U32)(VPAGE_SIZE>>prog_manage.nbit))//nand page number for vir page size(4KByte). 

#define DEEP_FIXDATA_PAGES          (((fixed_data_end-fixed_data_start-1) >> prog_manage.nbit) + 1) 
#define DEEP_ADDED_DATA_PAGES       (((mmu_ptdata_size) >> prog_manage.nbit) + 1) //nand pages for added data space.
#define DEEP_RESIDENT_SAVE_PAGES    (DEEP_FIXDATA_PAGES + DEEP_ADDED_DATA_PAGES)

#define DEEP_MAX_SWAP_NUM           (0x4000)    //set 0x4000 according to ((100*1024*1024/32/512+10)*2)=12820(0x3214)
#define DEEP_NAND_FLAG_PAGE         (g_nPagePerBlock_asa - 2) //Deep Standby flag position
#endif
#endif

extern T_U32 get_spsr(T_VOID);
extern T_U32 get_pc(void);
extern T_U32 MMU_Reg6(T_VOID);
extern T_VOID pmu_init(T_VOID);

extern T_VOID tick_start(T_VOID);
extern T_BOOL Fwl_ConsoleInit(T_VOID);

#ifdef DEEP_STANDBY
#define DEEP_FLAG_BLK_PRIMARY       (0x12345678)  //deep standby flag of primary block
#define DEEP_FLAG_BLK_BACKUP        (0x89abcdef)  //deep standby flag of backup block
#define DEEP_FLAG_CANCEL            (0x87654321)  //cancel flag of standby block


T_VOID deepstdb_handler(T_VOID);


#if(STORAGE_USED == NAND_FLASH)
static T_BOOL deepstdb_check(T_U32 *counter, T_DEEPSTDB_BLK *stdb_blk_find);
static T_BOOL deepstdb_load(T_U32 counter, T_DEEPSTDB_BLK stdb_blk_find);
static T_U16 deepstdb_front_valid_blk(T_U16 cur_blk, T_U32 num);
static T_BOOL deepstdb_compare_cnt(T_U32 biggish, T_U32 less);
#elif(STORAGE_USED == SPI_FLASH)
static T_BOOL deepstdb_load(T_VOID);
#endif
static T_U32 deepstdb_calc_sum(T_U32 *start_addr, T_U32 *end_addr);
static T_BOOL deepstdb_check(T_VOID );
extern T_VOID deepstdb_set_ABTStack(T_VOID);
extern T_VOID deepstdb_resume(T_VOID);
#endif

#if 0

#pragma arm section code = "_prog_check_"
/**
 * @brief when abort exception happens to check irq mode or not.
 * @param T_U32 ischeck, set to check irq or not.
 * @retval void.
 */
T_VOID progmanage_abort_checkirq(T_BOOL ischeck)
{
	akerror("aj test test checkirq", ischeck, 1);
    checkirq = ischeck;
}
/**
 * @brief if check irq mode or not.
 * @param void.
 * @retval T_BOOL true ischeck, flase is not.
 */
T_BOOL progmanage_is_checkirq(T_VOID)
{
    return checkirq;
}
#pragma arm section code
#endif


#pragma arm section code = "_frequentcode_"

#ifdef SWAP_WRITBAK_PRINT
/************************************************************************
 * @BRIEF get current swap count
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL current swap count
 **************************************************************************/
T_U32 get_the_swap_cnt(T_VOID)
{
    return prog_manage.swap_cnt;
}

/************************************************************************
 * @BRIEF get current write back count
 * @PARAM T_VOID
 * @RETURN T_U32
 * @RETVAL current write back count
 **************************************************************************/
T_U32 get_the_writbak_cnt(T_VOID)
{
    return prog_manage.writbak_cnt;
}

/************************************************************************
 * @BRIEF clear the swap count
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL T_VOID
 **************************************************************************/
T_VOID clear_the_swap_cnt(T_VOID)
{
    prog_manage.swap_cnt = 0;
}

/************************************************************************
 * @BRIEF clear the write back count
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL T_VOID
 **************************************************************************/
T_VOID clear_the_writbak_cnt(T_VOID)
{
    prog_manage.writbak_cnt = 0;
}
#endif //#ifdef SWAP_WRITBAK_PRINT
#pragma arm section code

#if(STORAGE_USED == NAND_FLASH)
 /******************************************************************************
* @NAME    progmanage_GetPhIdx
* @BRIEF   get page index of physical storage
* @AUTHOR  liangxiong
* @DATE    2012-5-23
* @PARAM   vaddr: the virtual address.(accord 4K page)
*          the index of ram page(except the resident memory)
* @RETURN  page index of physical storage
*   
*******************************************************************************/
T_U16 progmanage_GetPhIdx(T_U32 vaddr)
{
    T_S32 ramIdx;
    T_U32 nandIdx;

    nandIdx = MMU_GetPT2Paddr(vaddr);
    if (MMU_CheckMaping(nandIdx))
    {
        ramIdx = remap_get_vaddrindex(vaddr);
        if (0 > ramIdx)
        {
            AK_PRINTK(ERROR_PROG_GET_PHIDEX, vaddr, AK_TRUE);
            return 0;
        }
        return prog_manage.ram2store_tbl[ramIdx];
    }
    else
    {
        return ((T_U16)(nandIdx>>12)); 
    }
}

 /******************************************************************************
* @NAME    progmanage_SetPhIdx
* @BRIEF   set page index of physical storage
* @AUTHOR  liangxiong
* @DATE    2012-5-23
* @PARAM   vaddr: the virtual address.(accord 4K page)
*          the index of ram page(except the resident memory)
* @RETURN  T_VOID
*   
*******************************************************************************/
T_VOID progmanage_SetPhIdx(T_U32 vaddr, T_U16 index)
{
    T_S32 ramIdx;
    
    if (MMU_CheckMaping(MMU_GetPT2Paddr(vaddr)))
    {
        ramIdx = remap_get_vaddrindex(vaddr);
        if (0 > ramIdx)
        {
            AK_PRINTK(ERROR_PROG_SET_PHIDEX, vaddr, AK_TRUE);
        }
        else
        {
            prog_manage.ram2store_tbl[ramIdx] = index;
        }
    }
    else
    {
        MMU_MapPageEx(vaddr, vaddr+VPAGE_SIZE, (((T_U32)index)<<12), FB_SMALL); 
    }
}
#endif

 /******************************************************************************
* @NAME    progmanage_PreMapRam2Store
* @BRIEF    It prepares to map the ram to virtual address, after saving the page index 
           of physical storager to array of am2store_tbl.
* @AUTHOR  liangxiong
* @DATE    2012-5-23
* @PARAM   the index of ram page(except the resident memory)
*          vaddr: the virtual address.(accord 4K page)   
* @RETURN  T_VOID
*   
*******************************************************************************/
T_VOID progmanage_PreMapRam2Store(T_U8 ramIdx, T_U32 vaddr)
{
#if(STORAGE_USED == NAND_FLASH)
    prog_manage.ram2store_tbl[ramIdx] = progmanage_GetPhIdx(vaddr);
#endif
}

 /******************************************************************************
* @NAME    progmanage_PreUnmapRam2Store
* @BRIEF   It prepares to unmap the ram to virtual address, after getting the page index 
           of physical storager to array of am2store_tbl.
* @AUTHOR  liangxiong
* @DATE    2012-5-23
* @PARAM   the index of ram page(except the resident memory)
*          vaddr: the virtual address.(accord 4K page) 
*          
* @RETURN  the page index of physical storager in array of am2store_tbl.
*   
*******************************************************************************/
T_U32 progmanage_PreUnmapRam2Store(T_U8 ramIdx, T_U32 vaddr)
{
#if(STORAGE_USED == NAND_FLASH)
    return (T_U32)(prog_manage.ram2store_tbl[ramIdx]);
#else
    return 0;
#endif
}

 /******************************************************************************
* @NAME    progmanage_get_mapdata
* @BRIEF   get the bin file map info
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   strName:bin file name
*          pData:the buf to store the map info 
* @RETURN  bin file length
*   
*******************************************************************************/
static T_U32 progmanage_get_mapdata(T_S8 strName[], T_U8 *pData)
{
    T_U32 i;
    T_U32 file_count, file_len = 0;
    
#if (STORAGE_USED == NAND_FLASH)
    T_FILE_CONFIG *pFileCfg;
    T_NAND_CONFIG *pNandCfg;
#endif

#if (STORAGE_USED == SD_CARD)
    T_SD_STRUCT_INFO *sd_info;
    T_SD_FILE_INFO *sd_file_info;
#endif

#if (STORAGE_USED == SPI_FLASH)
    T_SPI_FILE_CONFIG *pSflashCfg;

    if (0 != progmanage_read_data(SFLASH_FILE_CONFIG_OFFSET,
        pData, 0, AK_FALSE))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        Fwl_SysReboot();
    }
    file_count = (T_U32)*pData;
    pSflashCfg = (T_SPI_FILE_CONFIG *)(pData+sizeof(T_U32));
    //find file
    for(i = 0; i < file_count; i++)
    {
        if(0 == strcmp((char *)strName, pSflashCfg->file_name))
        {
            break;
        }
        pSflashCfg++;
    }
    
    if (i == file_count)
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        return 0;
    }
    
    file_len = pSflashCfg->file_length;

    *((T_U32 *)pData) = pSflashCfg->start_page;

    
#elif (STORAGE_USED == NAND_FLASH)    
    //read nand config
    if(0 != progmanage_read_data(NAND_CONFIG_PHY_PAGENUM,  
                pData, &i, AK_FALSE))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        Fwl_SysReboot();
    }

    pNandCfg = (T_NAND_CONFIG *)pData;

    /* initialize nand swap space */
    prog_manage.swap.start_blk = (pNandCfg->StartBlock );
    prog_manage.swap.end_blk = ((pNandCfg->StartBlock + pNandCfg->BlockCount - 1) );


#ifdef DEBUG
    //akoutput("start_blk=", remap.swap.start_blk, AK_FALSE);
    //akoutput(", end_blk=", remap.swap.end_blk, AK_TRUE);
#endif

    //get file count
    file_count = pNandCfg->FileCount;

    //read map index
    if(0 != progmanage_read_data(FILE_CONFIG_PHY_PAGENUM,
                pData, &i, AK_FALSE))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        Fwl_SysReboot();
    }   

    //find file
    for(i = 0; i < file_count; i++)
    {
        pFileCfg = (T_FILE_CONFIG *)pData + i;
        if(0 == strcmp((char *)strName, (char *)(pFileCfg->file_name)))
        {
            break;
        }
    }
    if (i == file_count)
    {
       AK_PRINTK(err_str, __LINE__, AK_TRUE);
       return 0;
    }
    file_len = pFileCfg->file_length;


    //read map
    if(0 != progmanage_read_data(pFileCfg->map_index, 
                pData, &i, AK_FALSE))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        Fwl_SysReboot();
    }
#elif (STORAGE_USED == SD_CARD)
    if(Fwl_SD_Read(sys_sd_handle,SD_STRUCT_INFO_SECTOR,pData,1))
    {
        sd_info = (T_SD_STRUCT_INFO*)pData;
        file_count = sd_info->file_cnt;
        if(Fwl_SD_Read(sys_sd_handle,sd_info->bin_info_pos,pData+512,1))
        {
            sd_file_info = (T_SD_FILE_INFO*)(pData+512);
            for(i = 0; i < file_count; i++)
            {
                if(0 == strcmp((char *)strName,sd_file_info[i].file_name))
                {
                    break;
                }
            }
            if(i == file_count)
            {
                AK_PRINTK(err_str, __LINE__, AK_TRUE);
                while(1);
            }
            if(sd_file_info[i].start_sector > T_U16_MAX)
            {
                AK_PRINTK(err_str, __LINE__, AK_TRUE);
                while(1);
            }
            if((sd_file_info[i].start_sector + (sd_file_info[i].file_length/512))> T_U16_MAX)
            {
                AK_PRINTK(err_str, __LINE__, AK_TRUE);
                while(1);
            }
            *((T_U16 *)pData) = sd_file_info[i].start_sector;
            file_len = sd_file_info[i].file_length;
        }
    }
#endif

    return file_len;
}

 /******************************************************************************
* @NAME    progmanage_get_maplist 
* @BRIEF   get the bin file map info and check data is valid 
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   strName:bin file name
*          mapData:the buf to store the map info 
*          length: buf lenght
* @RETURN  bin file length
*   
*******************************************************************************/
T_U32 progmanage_get_maplist (T_S8 strName[], T_U16 *mapData, T_U32 Length)
{   
    T_U8 *pData;
    T_U32 file_len;
#if (STORAGE_USED != SPI_FLASH) 
    T_U16 i;
#endif
#if (STORAGE_USED == NAND_FLASH)    
    T_U8 bad_blk_info[64];
    T_U32 byte_loc, byte_offset;
    
#endif
    //get buffer
    pData = (T_U8 *)remap_alloc(VPAGE_SIZE);

    file_len = progmanage_get_mapdata(strName, pData);
    if(0 == file_len)
    {
        remap_free(pData);
        return 0;
    }

#if(STORAGE_USED == SPI_FLASH)
    if(Length < (file_len>>(prog_manage.nbit + prog_manage.mbit)))
    {
        AK_PRINTK(ERROR_PROG_GET_MAPLIST, Length, AK_TRUE);
        remap_free(pData);
        return 0;
    }
    else
    {
        Length = ((file_len -1)>>(prog_manage.nbit + prog_manage.mbit)) + 1;
    }
    
    mapData[0] = *(T_U32*)pData;

#elif (STORAGE_USED == NAND_FLASH)
    if(Length < (file_len>>(prog_manage.nbit + prog_manage.mbit)))
    {
        AK_PRINTK(ERROR_PROG_GET_MAPLIST, Length, AK_TRUE);
        remap_free(pData);
        return 0;
    }
    else
    {
        Length = ((file_len -1)>>(prog_manage.nbit + prog_manage.mbit)) + 1;
    }

    if(!progmanage_get_bad_area(0, bad_blk_info, 8*64))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        while(1);
    }
    
    //set data
    for(i = 0; i < Length; i++)
    {
        mapData[i] = *((T_U16 *)pData + i*2);
        
        //check if this is a valid block
        byte_loc = mapData[i] >> 3;
        byte_offset = 7 - (mapData[i] & 0x7);
        if(byte_loc < 64 && (bad_blk_info[byte_loc] & (0x1 << byte_offset)))
        {
            mapData[i] = *((T_U16 *)pData + i*2 + 1);
            
            //check if the backup block is a bad block
            byte_loc = mapData[i] >> 3;
            byte_offset = 7 - (mapData[i] & 0x7);
            if(byte_loc < 64 && (bad_blk_info[byte_loc] & (0x1 << byte_offset)))
            {
                AK_PRINTK(err_str, __LINE__, AK_TRUE);
                while(1);
            }
        }
    }
#elif (STORAGE_USED == SD_CARD)
    mapData[0] = *(T_U16*)pData;
    if(Length < (file_len>>(prog_manage.nbit + prog_manage.mbit)))
    {
        AK_PRINTK(ERROR_PROG_GET_MAPLIST, Length, AK_TRUE);
        remap_free(pData);
        return 0;
    }
    else
    {
        Length = ((file_len -1)>>(prog_manage.nbit + prog_manage.mbit)) + 1;
    }
    for(i = 0; i < Length; i++)
    {
        //mapData[i]记录每个block的起始扇区,一个block是128k
        mapData[i] = mapData[0] + (1<<(prog_manage.nbit + prog_manage.mbit - 9))*i;//一个元素记录连续的128K
    }
#endif 
    //free buffer
    remap_free(pData);
    
    return file_len;
}

   /******************************************************************************
 * @NAME    progmanage_recover_block 
 * @BRIEF   rcover the block from backup block
 * @AUTHOR  xuping
 * @DATE    2010-03-02
 * @PARAM   strName:bin name
 *          block: block index
            pBuf: buffer of operation. The size is must multiple of 4kB.
 * @RETURN  AK_TRUE: success   AK_FALSE:failure
 *   
 *******************************************************************************/
T_BOOL progmanage_recover_block (T_S8 strName[], T_U32 block, T_U8 * pBuf)
{
#if (STORAGE_USED == NAND_FLASH)
     T_U32 i;
     T_U8 *pData;
     T_U32 blk_backup = 0;
     T_U32 spare;
     T_U32 file_len;
     T_U32 count;
    
     //get buffer
     if (pBuf == AK_NULL)
     {
        pData = (T_U8 *)remap_alloc(VPAGE_SIZE);
     }
     else
     {
        pData = pBuf;
     }
     
     //get data
     file_len = progmanage_get_mapdata(strName, pData);
     if(0 == file_len)
     {
         if (pBuf == AK_NULL)
         {
            remap_free(pData);
         }
         return AK_FALSE; 
     }
     
     count = ((file_len - 1)>>(prog_manage.nbit + prog_manage.mbit)) + 1;
     
     for(i = 0; i < count; i++)
     {
         if(*((T_U16 *)pData + i*2) == block)
         {
             blk_backup = *((T_U16 *)pData + i*2 + 1);
             break;
         }
         
         if(*((T_U16 *)pData + i*2 + 1) == block)
         {
             AK_PRINTK(ERROR_PROG_RCV_BLOCK_BACKUP, i, AK_TRUE);
             while(1);
         }
     }
     
     if (blk_backup == 0)
     {
         AK_PRINTK(err_str, __LINE__, AK_TRUE);
         while(1);
     }
     
     if(i == count-1)//the last block
     {
         i = ((file_len - 1) >> prog_manage.nbit) + 1; //page count
         count = i - ((count - 1) << prog_manage.mbit); //剩余的page数量
        
     }
     else
     {
         count = (1 << prog_manage.mbit);
     }
     
     progmanage_set_protect(AK_FALSE);
     
     //recover
     for(i = 0; i < count; i++)
     {
         if(0 != progmanage_read_data((blk_backup << prog_manage.mbit) + i,
                     pData, &spare, AK_TRUE))
         {
             AK_PRINTK(ERROR_PROG_RCV_BLOCK_READ, blk_backup, AK_TRUE);
             while(1);
         }

        if (0 == i)//erase  block only when the backup block can be read
        {
            if(progmanage_erase_data(0, block << prog_manage.mbit) != 0)
            {
                goto ERROR_EXIT;
            }
        }
         
         if(0 != progmanage_write_data((block << prog_manage.mbit) + i,
                     pData, spare, AK_TRUE))
         {
             goto ERROR_EXIT;
         }
         
         //ensure
         if(0 != progmanage_read_data((block << prog_manage.mbit) + i,
                     pData, &spare, AK_TRUE))
         {
             goto ERROR_EXIT;
         }
     }
    
     //free buffer
     if (pBuf == AK_NULL)
     {
        remap_free(pData);
     }
     
     progmanage_set_protect(AK_TRUE);
     return AK_TRUE; 
 
ERROR_EXIT:
    if (pBuf == AK_NULL)
    {
       remap_free(pData);
    }
    progmanage_set_protect(AK_TRUE);        
    progmanage_set_bad_area(block);
#endif
     
     return AK_FALSE;     
 }

 /******************************************************************************
* @NAME    progmanage_build_ro_map
* @BRIEF   build prog bin file map
* @AUTHOR  xuping
* @DATE    2010-02-24
* @PARAM 
*           T_VOID
* @RETURN T_VOID
*   
*******************************************************************************/
static T_VOID progmanage_build_ro_map(T_VOID)
{
#if (STORAGE_USED == NAND_FLASH)
    T_U16 block;
    T_U16 i = 0;
    T_U16 phIdx = 0;
    T_U32 vaddr = 0;
#endif
    T_U16 va=0;
    T_U32 mbit;
    T_U16 pData[32];    //the block number of RO bin reserved
    T_U32 page_cnt;
    //T_U32 spare;    
    T_U32 file_len;
    
    mbit = prog_manage.mbit;

    file_len = progmanage_get_maplist((T_S8*)(PROG_FILE_NAME), pData, 32);
    if (file_len == 0)
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        while(1);
    }
    
    //page_cnt = ((file_len - 1) >> prog_manage.nbit) + 1; //物理页数目
    page_cnt = ((file_len - 1) >> prog_manage.nbit) + (VPAGE_SIZE>>prog_manage.nbit);

    va = (REMAP_VADDR_START - RAM_BASE_ADDR) >> VPAGE_BITS;

#if (STORAGE_USED == NAND_FLASH)

    vaddr = REMAP_VADDR_START;
    for(i = 0; i <= page_cnt; i+=(VPAGE_SIZE>>prog_manage.nbit), vaddr+=VPAGE_SIZE)
    {
        block = pData[i>>mbit];
        //prog_manage.mem2store_tbl[va] = (T_U16)((block<<mbit) + (i&((1 << mbit)-1)));
        phIdx = (T_U16)((block<<mbit) + (i&((1 << mbit)-1)));
        progmanage_SetPhIdx(vaddr, phIdx);
    }

#elif (STORAGE_USED == SPI_FLASH || STORAGE_USED == SD_CARD)

    prog_manage.mem2store_phstart = pData[0];
    prog_manage.mem2store_vstart = va;
    prog_manage.mem2store_romlen = page_cnt;

#endif
}

#if(STORAGE_USED == NAND_FLASH)
 /******************************************************************************
* @NAME    mark_bad_block
* @BRIEF   set bad block
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   block index
* @RETURN  T_VOID
*   
*******************************************************************************/
static T_VOID mark_bad_block(T_U32 block)
{
    T_U32 bad_blk;

    bad_blk = block - prog_manage.swap.start_blk;    

    prog_manage.bad_blk_bitmap[bad_blk>>3] |= 1<<(7-(bad_blk&0x7));
    

    if (AK_FALSE == progmanage_set_bad_area(block))
    {
        AK_PRINTK(err_str, __LINE__, AK_TRUE);
        while(1);
    }
       
    AK_PRINTK(ERROR_PROG_MARK_BADBLOCK, block, AK_TRUE);
}

/******************************************************************************
 * @brief   get the block address of next 'num'th block
 * @author  yang yiming
 * @date     2013-03-25
 * @param   [in] cur_blk: the block in use
 * @param   [in] num: which block desired
 * @return  T_U32
 * @retval  the block address of  next 'num'th block
******************************************************************************/
static T_U32 progmanage_next_valid_blk(T_U32 cur_blk, T_U32 num)
{
    T_U32 ret = cur_blk;
    
    do
    {
        ret++;
        if (ret  > prog_manage.swap.end_blk)
            ret  = prog_manage.swap.start_blk;
        
        if (ret == cur_blk)//防止找不到需要的块而死循环
        {
           AK_PRINTK(err_str, __LINE__, AK_TRUE);
           while(1);
        }

        //get valid block
        if (!check_bad_block(ret - prog_manage.swap.start_blk))
        {
          num--;
        }

    } while(0 != num);
        
    return ret;
}

#endif 

#pragma arm section code ="_memcode_"

T_VOID progmanage_StorageGetInfo(T_U8* pagebit, T_U8* blockbit)
{
#if (STORAGE_USED == NAND_FLASH)
    Fwl_Nand_GetNandInfo(pagebit,blockbit,AK_TRUE);
#else
    *pagebit = 12;//4k页
    *blockbit = 17;//128k 一个块
#endif
}
/******************************************************************************
* @NAME    progmanage_free_mapinfo
* @BRIEF   free vaddr2store info
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   vaddr:start vaddr
*          size:free addr size
* @RETURN  bin file length
*   
*******************************************************************************/
T_U32 progmanage_free_mapinfo(T_U32 vaddr, T_U32 size)
{
#if (STORAGE_USED == NAND_FLASH)
    T_U32 start, end;
    
    start = vaddr & (~(VPAGE_SIZE-1));
    end   = (vaddr+size-1) & (~(VPAGE_SIZE-1));

    for (vaddr=start; vaddr<=end; vaddr+=VPAGE_SIZE)
    {
        /* update nand mapping table */
        //i = (vaddr - RAM_BASE_ADDR) >> VPAGE_BITS;
        //prog_manage.mem2store_tbl[i] = 0;     
        progmanage_SetPhIdx(vaddr, 0);
    }
#endif
    return 1;
}

/**
 * @brief get the location of a specificated block of the file reserved
 * @param strName, the given file name
 * @param index, the block index
 * @retval T_U32, the block location. if error, return 0
 */
T_U32 progmanage_get_backup_block(T_S8 strName[], T_U32 index)
{
#if (STORAGE_USED == NAND_FLASH)
    T_U8 *pData;
    
    //get buffer
    pData = (T_U8 *)remap_alloc(VPAGE_SIZE);
    
    //get data
    if(0 == progmanage_get_mapdata(strName, pData))
    {
        remap_free(pData);
        return 0;
    }

    index = *((T_U16 *)pData + index*2 + 1);
    
    remap_free(pData);

    return index;
#else
    return 0;
#endif
}


T_S8 progmanage_LoadStorage512Byte(T_U32 block,T_U32 sector, T_U32 offset,T_U8 data[],T_U8* filepath)
{
    //block起始块,sector即起始页,offset是4K页中具体哪一个扇区
#if ((defined(OS_WIN32))||(STORAGE_USED == NAND_FLASH))
    return Fwl_Nand_ReadSector_512Byte(block, sector, offset,data,filepath);
#endif
#if (STORAGE_USED == SD_CARD)
    return Fwl_SD_Read(sys_sd_handle,block+sector*8+offset,data,1);
#endif
    return 1;
}
T_S8 progmanage_LoadStorageMulti512Byte(T_U32 block,T_U32 sector, T_U32 offset,T_U8 data[],T_U8* filepath,T_U16 size)
{
    //block起始块,sector即起始页,offset是4K页中具体哪一个扇区
#if ((defined(OS_WIN32))||(STORAGE_USED == NAND_FLASH))
    return Fwl_Nand_ReadSector_AnyByte(block, sector, offset,data,filepath,size);
#endif
#if (STORAGE_USED == SD_CARD)
    return Fwl_SD_Read(sys_sd_handle,block+sector*8+offset,data,size/512);
#endif
    return 1;
}
#pragma arm section code

 /******************************************************************************
* @NAME    progmanage_load 
* @BRIEF   loag program bin file by virtual address
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   vaddr:virtual address
*          size: load bin size
* @RETURN  AK_TRUE:success AK_FALSE:failure
*   
*******************************************************************************/
T_BOOL progmanage_load(T_U32 vaddr, T_U32 size)
{
    T_U32 page_no, i, id;
#if (STORAGE_USED != SD_CARD)
    T_U32 spare;
#endif
    T_U8 *page;
    T_BOOL bbin = AK_FALSE;
    T_U32 virAddr; 
    T_U32 start, end;

    start = vaddr & (~(VPAGE_SIZE-1));
    end   = (start+size-1) & (~(VPAGE_SIZE-1));  

    for(virAddr = start;  virAddr <= end; virAddr+=VPAGE_SIZE)
    {

        /* load page */
        id = (virAddr - RAM_BASE_ADDR) >> VPAGE_BITS;
        
#if(STORAGE_USED == NAND_FLASH)
        page_no = progmanage_GetPhIdx(vaddr);

#elif (STORAGE_USED == SPI_FLASH || STORAGE_USED == SD_CARD)
        id = id - prog_manage.mem2store_vstart;
        if (id > prog_manage.mem2store_romlen) //超出代码长度虚拟地址页为数据页
        {
            page_no = 0;
        }
        else
        {
            page_no = prog_manage.mem2store_phstart + PHPAGE_2VPAGE_NUM *id;
        }
#endif
        
        page = (T_U8 *)virAddr;

        if (prog_manage.swap.end_blk < (page_no >> prog_manage.mbit)) //bin 文件存储在交换区后
        {
            bbin = AK_TRUE;
        }
        if (page_no)
        {        
            for (i=0; i<(T_U32)(VPAGE_SIZE>>prog_manage.nbit); i++, page_no++)
            {
                if (0 != progmanage_read_data(page_no,page, &spare, bbin))
                {
                    AK_PRINTK(ERROR_PROG_LOAD, virAddr, AK_TRUE);
                    #if(STORAGE_USED == NAND_FLASH)
                    if (virAddr <= rodata_end)
                    {
                        /* recover first, if failed, load from backup block */
                        if (!progmanage_recover_block((T_S8*)(PROG_FILE_NAME), (page_no>>prog_manage.mbit), (T_U8 *)virAddr))
                        {                    
                            /* recover failed, rebuild nand mapping */                    
                            progmanage_build_ro_map();
                        }      
                        
                        page_no = progmanage_GetPhIdx(vaddr);

                        page = (T_U8 *)virAddr;
                        //reload it
                        for (i=0; i<(T_U32)(VPAGE_SIZE>>prog_manage.nbit); i++, page_no++)
                        {
                            if (0 != progmanage_read_data(page_no,page, &spare, AK_TRUE))
                            {
                                AK_PRINTK(ERROR_PROG_RETRYLOAD, virAddr, AK_TRUE);
                                while(1);
                            }
                            page += (1<<prog_manage.nbit);
                        }
                        break;
                    }
                    else
                    {
                        #if(STORAGE_USED == NAND_FLASH)
                        mark_bad_block(page_no>>prog_manage.mbit);
                        #endif
                        Fwl_SysReboot();
                    }
                    #else
                    while(1);
                    #endif
                }
                
                page += (1<<prog_manage.nbit);
            }
        }
        else
        {
            akmemset((T_U32*)page, 0, VPAGE_SIZE>>2);    
        }
    }
    return AK_TRUE;
}

#if (STORAGE_USED == NAND_FLASH)
/******************************************************************************
* @NAME    progmanage_contain_valid_pages
* @BRIEF   check if there is valid pages in current block
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   block: block index
* @RETURN  AK_TRUE: contain valid pages
*   
*******************************************************************************/
static T_BOOL progmanage_contain_valid_pages(T_U32 block)
{
    T_U32 i;
    
    for (i=0; i<(RAM_SIZE-PRE_MAPSIZE)/VPAGE_SIZE; i++)
    {
        if ((progmanage_GetPhIdx((REMAP_VADDR_START+(i<<VPAGE_BITS)))>>prog_manage.mbit) == block)
        {
            /*has valid pages*/
            return AK_TRUE;
        }
    }
    return AK_FALSE;
}

/******************************************************************************
* @NAME    progmanage_move_valid_pages
* @BRIEF   move valid pages from current block
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   block: block index
* @RETURN  
*   
*******************************************************************************/
static T_S16 progmanage_move_valid_pages(T_U32 block)
{
    T_U32 pg;
    T_U32 vaddr;
    T_U8 *page;
    T_S16 ret=0;
    T_U16 phIdx;
    T_U8  j;

    page = (T_U8 *)remap_alloc(VPAGE_SIZE);
    vaddr = rodata_end & (~(VPAGE_SIZE-1));

    for (; vaddr<(RAM_BASE_ADDR+RAM_SIZE); vaddr+=VPAGE_SIZE)
    {
        phIdx = progmanage_GetPhIdx(vaddr);
        if ((phIdx>>prog_manage.mbit) == block)
        {
            T_U32 spare;
            
            /*got a valid page, read and write */
            pg = phIdx;
            for (j=0; j<(T_U32)(VPAGE_SIZE>>(prog_manage.nbit)); j++)
            {
                if (0 != progmanage_read_data(pg+j, page+(j<<prog_manage.nbit), &spare, AK_FALSE))
                    Fwl_SysReboot();
            }
                
            /*write 4k page to nand*/
#ifdef DEEP_STANDBY
            spare = s_ulBlkCount;
#else
            spare = prog_manage.swap.block;
#endif
            pg = (prog_manage.swap.block << prog_manage.mbit) + prog_manage.swap.page_offst; 
            for (j=0; j<(T_U32)(VPAGE_SIZE>>(prog_manage.nbit)); j++)
            {
                if (0 != progmanage_write_data(pg+j, page+(j<<prog_manage.nbit) , spare, AK_FALSE))
                {
                    ret = -1;
                    goto exit;
                }
                prog_manage.swap.page_offst++;
            }

            /* update nand mapping table */
            progmanage_SetPhIdx(vaddr, (T_U16)pg);
            ret++;
        }
    }
exit:
    /* remap and load the page */
    remap_free(page);

    return ret;
}

/******************************************************************************
* @NAME    progmanage_request_new_block 
* @BRIEF   get next valid block
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   T_VOID
* @RETURN  T_VOID
*   
*******************************************************************************/
static T_VOID progmanage_request_new_block(T_VOID)
{
#if (STORAGE_USED == NAND_FLASH)
    T_U32 block;

move_fail:        
     prog_manage.swap.block = progmanage_next_valid_blk(prog_manage.swap.block, 1);
#ifdef DEEP_STANDBY
     s_ulBlkCount++;
     /* counter shouldn't be 0x0 and 0xffffffff. and don't modify the 0x3 random to
        prevent writing 0 to counter in backup stdb block if counter equal 0x2. */
     if (s_ulBlkCount == 0xffffffff)
         s_ulBlkCount = 0x3;

#endif
    
     prog_manage.swap.page_offst = 0;

     /*check next block n+M empty or not
       if has valid pages, move them to current block.
       at last, erase the block n+M
      */
erase_fail:         
     block = prog_manage.swap.block;
     block = progmanage_next_valid_blk(block, NAND_SWAP_RESV_BLOCK-prog_manage.M);
     if (progmanage_contain_valid_pages(block))
     {   
         /*do move pages*/
         if (progmanage_move_valid_pages(block) < 0)
         {
             /* mark as bad block */
             #if(STORAGE_USED == NAND_FLASH)
             mark_bad_block(prog_manage.swap.block);
             #endif
             if (++prog_manage.M == NAND_SWAP_RESV_BLOCK)
             {
                AK_PRINTK(err_str, __LINE__, AK_TRUE);
                while(1);
             }
             goto move_fail;
         }
     }
     if (progmanage_erase_data(0, (block<<prog_manage.mbit)) != 0)
     {
         /* mark as bad block */
         #if(STORAGE_USED == NAND_FLASH)
         mark_bad_block(block);
         #endif
         goto erase_fail;
     }
#endif
}

 /******************************************************************************
* @NAME    progmanage_do_replacement 
* @BRIEF   move the the valid data from src to dst
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   src:source area
*          dst:dst area
*          offset: valid page offset
* @RETURN  AK_TRUE:success AK_FALSE:failure
*   
*******************************************************************************/
static T_BOOL progmanage_do_replacement(T_U32 src, T_U32 dst, T_U32 offset)
{
    T_U32 i, j;
    T_U32 pg;
    T_U8 *page;
    T_U32 vaddr;

    page = (T_U8 *)remap_alloc(VPAGE_SIZE);

    for (i=0; i<offset; i+=(VPAGE_SIZE>>(prog_manage.nbit)))
    {
        T_U32 spare;

        /* read 4k page */
        pg = (src<<prog_manage.mbit) + i;
        for (j=0; j<(T_U32)(VPAGE_SIZE>>(prog_manage.nbit)); j++)
        {
            if (0 != progmanage_read_data(pg+j, page+(j<<(prog_manage.nbit)), &spare, AK_FALSE))
                Fwl_SysReboot();
        }

        /* get corresponding virtual address index */
        for (vaddr=REMAP_VADDR_START; vaddr<(RAM_SIZE-PRE_MAPSIZE); vaddr+=VPAGE_SIZE)
        {
            if (progmanage_GetPhIdx(vaddr) == pg)
                break;
        }
        
        if (vaddr == RAM_SIZE)
        {
            return AK_FALSE;
        }

        /*write 4k page to nand*/
#ifdef DEEP_STANDBY
        spare = s_ulBlkCount;
#else
        spare = dst;
#endif
        pg = (dst << prog_manage.mbit) + i;

        for (j=0; j<(T_U32)(VPAGE_SIZE>>(prog_manage.nbit)); j++)
        {
            if (0 != progmanage_write_data(pg+j, page+(j<<(prog_manage.nbit)) , spare,AK_FALSE))
            {
                //TODO
                return AK_FALSE;
            }
        }

        /* update nand mapping table */
        progmanage_SetPhIdx(vaddr, (T_U16)pg);
    }

    
    remap_free(page);

    return AK_TRUE;
}
#endif
 /******************************************************************************
* @NAME    progmanage_writeback 
* @BRIEF   write back program bin to nand (or sd)by virtual address
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   vaddr:virtual address
*          size: flush bin size
* @RETURN  AK_TRUE:success AK_FALSE:failure
*   
*******************************************************************************/
T_BOOL progmanage_writeback (T_U32 vaddr, T_U32 size)
{
#if (STORAGE_USED == NAND_FLASH)

    T_U8 *page;
    T_U32 spare;
    T_U32 page_no, i;
    T_U32 block;
    T_U32 virtual_addr;
    
    virtual_addr = vaddr & (~(VPAGE_SIZE-1));
    size = ((size & (~(VPAGE_SIZE-1)))>>VPAGE_BITS); 

    for (; size>0; size--, virtual_addr+=VPAGE_SIZE)
    {
        /*end of the block, get next valid block and do page collection*/
        if (prog_manage.swap.page_offst == (1<<prog_manage.mbit))
        {
            progmanage_request_new_block();
        }

        /* nand read/write with cpu address */
        page = (T_U8*)MMU_Vaddr2Paddr(virtual_addr);
        if (page == 0)
        {
            AK_PRINTK(ERROR_PROG_WRITEBACK, virtual_addr, AK_TRUE);
            while(1);
        }
        //via cpu mode
        page = (T_U8*)virtual_addr;

        /*write 4k page to nand*/
        write_fail:    
#ifdef DEEP_STANDBY
        spare = s_ulBlkCount;
#else
        spare = prog_manage.swap.block;
#endif
        page_no = (prog_manage.swap.block<<prog_manage.mbit) + prog_manage.swap.page_offst; 
        for (i=0; i<(T_U32)(VPAGE_SIZE>>(prog_manage.nbit)); i++)
        {
            if (0 != progmanage_write_data(page_no+i, page, spare,AK_FALSE))
            {
                /* do replacement and mark as bad block */
                block = progmanage_next_valid_blk(prog_manage.swap.block, 1);
                if (prog_manage.swap.page_offst != 0)
                {
                    //move the valid page to next block
                    while(!progmanage_do_replacement(prog_manage.swap.block, 
                            block, 
                            prog_manage.swap.page_offst))
                    {
                        if (++prog_manage.M == NAND_SWAP_RESV_BLOCK)
                        {   
                            AK_PRINTK(err_str, __LINE__, AK_TRUE);
                            while(1);
                        }
                        /* mark as bad block */
                        #if(STORAGE_USED == NAND_FLASH)
                        mark_bad_block(block);
                        #endif
                        block = progmanage_next_valid_blk(block, 1);
                    }                
                }
                #if(STORAGE_USED == NAND_FLASH)
                mark_bad_block(prog_manage.swap.block);
                #endif
                prog_manage.swap.block = block;

                if (++prog_manage.M == NAND_SWAP_RESV_BLOCK)
                {    
                    AK_PRINTK(err_str, __LINE__, AK_TRUE);
                    while(1);
                }
                goto write_fail;
            }
            prog_manage.swap.page_offst++;
            page += (1<<(prog_manage.nbit));
        }

        /* update nand mapping table */
        //i = (virtual_addr - RAM_BASE_ADDR) >> VPAGE_BITS;
        //prog_manage.mem2store_tbl[i] = (T_U16)page_no;  
        progmanage_SetPhIdx(virtual_addr, (T_U16)page_no);
    }
    
    #ifdef SWAP_WRITBAK_PRINT
    prog_manage.writbak_cnt++;
    #endif

    return AK_TRUE;
#else
    return AK_FALSE;
#endif
}

#pragma arm section code = "_initphase_code_"
/************************************************************************
 * @BRIEF init and enable mmu
 * @PARAM T_VOID
 * @RETURN T_VOID
 **************************************************************************/
T_VOID progmanage_mmu_init(T_VOID)
{
    T_U32 ptr;

    //init mmu without enable abort exception.
    MMU_Init(RAM_SIZE, PRE_MAPSIZE, L2_FIFO_ADDR, _MMUTT_PADDRESS, _MMUPT_PADDRESS);

    //setup the mapped mmu flag for PHY_SVC_MODE_STACK
    for (ptr=(PHY_SVC_MODE_STACK-VPAGE_SIZE); \
         ptr>(PHY_SVC_MODE_STACK-VPAGE_SIZE-PHY_SVC_MODE_STACK_SIZE); ptr-=VPAGE_SIZE)
    {
        MMU_MapPageEx(ptr, ptr+VPAGE_SIZE, ptr, RW_SMALL);
    }
#ifdef CHECK_STARTUPTICK
    tick_start();
#endif
}

/******************************************************************************
* @NAME    progmanage_init 
* @BRIEF   init the program bin managent
* @AUTHOR  xuping
* @DATE    2010-03-02
* @PARAM   T_VOID
* @RETURN  T_VOID
*   
*******************************************************************************/
T_VOID progmanage_init(T_VOID)
{
#if (STORAGE_USED == NAND_FLASH)
    T_U32 vpagesize;
#elif (STORAGE_USED == SD_CARD)
    T_U8 *SD_InfoData = AK_NULL;
    T_BOOL FindBootInfoResult = AK_FALSE;
#endif
    T_U32 i;
    T_U32 vaddr;
    T_REMAP_CALLBACK remapcall;
#ifndef DEEP_STANDBY
    T_U16 block;
#endif

    Fwl_ConsoleInit();
    Fwl_SpkConnectSet(AK_FALSE);
    delay_ms(200);
    //AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
    //while(1);
    //delay_ms(5000);
    //AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
    pmu_init();
    analog_init();
    //delay_ms(5000);
    //AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
    pmu_vref15_sel(VREF_BGR);
    rtc_enable_in_pwd(AK_TRUE);
#if (STORAGE_USED != SPI_FLASH)
    //REG32(CLOCK_RST_CTRL_REG) |= (1<<8);//关闭SPI1接口时钟
#endif
    if (bootbss_end > ABORT_MODE_STACK_END)
    {
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        while(1);
    }

    /* initializing */
    prog_manage.M = 0;
    prog_manage.protect = 1; //default protect

#if (STORAGE_USED == NAND_FLASH)
    /*for (i=0; i<RAM_SIZE/VPAGE_SIZE; i++)
    {
        prog_manage.mem2store_tbl[i] = 0;
    }*/
    for (i=0; i<REMAP_MEM_PSIZE/VPAGE_SIZE; i++)
    {
        prog_manage.ram2store_tbl[i] = 0;
    }
    //清零
    vaddr = REMAP_VADDR_START;
    for(; vaddr<(RAM_BASE_ADDR+RAM_SIZE); vaddr+=VPAGE_SIZE)
    {
        MMU_MapPageEx(vaddr,vaddr+VPAGE_SIZE,0,FB_SMALL);
        //print_x(vaddr);
    }

#elif (STORAGE_USED == SPI_FLASH || STORAGE_USED == SD_CARD)
    prog_manage.mem2store_phstart = 0;
    prog_manage.mem2store_vstart = 0;
    prog_manage.mem2store_romlen = 0;
#endif
    
#if (STORAGE_USED == NAND_FLASH)
    /* establish nand_mem mapping table */
    Nand_Init_Remap(NAND_PARAM_ADDR);

    vpagesize =  g_nPageSize > VPAGE_SIZE ? VPAGE_SIZE : g_nPageSize;

    prog_manage.nbit = bitnum(vpagesize);
    prog_manage.mbit = bitnum(g_nPagePerBlock_asa);

    //init remap kernal
    remapcall.remapload  = progmanage_load;
    remapcall.remapflush = progmanage_writeback;
    
    remap_init(&remapcall, AK_TRUE, RAM_SIZE);    
   
#elif (STORAGE_USED == SPI_FLASH)
     //init remap kernal
    remapcall.remapload  = progmanage_load;
    remapcall.remapflush = progmanage_writeback;
    
    remap_init(&remapcall, AK_FALSE, RAM_SIZE);

    if(AK_FALSE == sflash_remap_init(SFLASH_PARAM_ADDR))
    {
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        while(1);
    }
    
    prog_manage.nbit = bitnum(VPAGE_SIZE);
    prog_manage.mbit = bitnum(128*1024/VPAGE_SIZE);
#elif(STORAGE_USED == SD_CARD)
    remapcall.remapload  = progmanage_load;
    remapcall.remapflush = progmanage_writeback;
    remap_init(&remapcall, AK_FALSE, RAM_SIZE);

    SD_InfoData = (T_U8 *)remap_alloc(VPAGE_SIZE);

    
    sys_sd_handle = Fwl_SD_Card_Init(SYSTEM_STORAGE); //init boot sdcard    
    if(sys_sd_handle)
    {
        if(Fwl_SD_Read(sys_sd_handle,SD_BOOT_SECTOR,SD_InfoData,1))
        {
            if(strcmp(SD_InfoData+4, "SUPERNNL") == 0)
            {
                FindBootInfoResult = AK_TRUE;
                bootSdIndex = i;
            }
        }
    }
    SD_InfoData = remap_free(SD_InfoData);
    if(!FindBootInfoResult)
    {
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        sys_sd_handle = AK_NULL;
        while(1);
    }
    prog_manage.nbit = bitnum(VPAGE_SIZE);
    prog_manage.mbit = bitnum(128*1024/VPAGE_SIZE);

#endif


#if (STORAGE_USED == NAND_FLASH)
    if (AK_FALSE == asa_initial(g_nPagePerBlock_asa, g_nPageSize, g_nBlockPerChip * g_nChipCnt))
    {   
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        while(1); 
    }
#endif

    progmanage_build_ro_map();
    
    prog_manage.swap.page_offst = 0;

    /* establish bad block bitmap */
    if ((prog_manage.swap.end_blk - prog_manage.swap.start_blk) > (MAX_SWAP_BLK_NUM-1))
    {
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        while(1);
    }

#if (STORAGE_USED == NAND_FLASH)
    if (AK_FALSE == progmanage_get_bad_area(prog_manage.swap.start_blk, prog_manage.bad_blk_bitmap, MAX_SWAP_BLK_NUM))
    {
        AK_PRINTK(PROGSTR_INIT, __LINE__, AK_TRUE);
        while(1);
    }
#endif

    /* get start valid block */
    for (i=0; i<MAX_SWAP_BLK_NUM; i++)
    {
        if (!check_bad_block(i))
        {
            prog_manage.swap.block = prog_manage.swap.start_blk + (T_U16)(i);         
            break;
        }
    }
    
#ifdef DEEP_STANDBY
    /* setup abort stack for the unfixed code */
    deepstdb_set_ABTStack();
#else
    /* erase reserved blocks of nand swap space at start-up */
    i = 0;
    block = prog_manage.swap.block;
#if (STORAGE_USED == NAND_FLASH)
    while(i++ < NAND_SWAP_RESV_BLOCK+1)
    {
        if (progmanage_erase_data(0, block << prog_manage.mbit) != 0)
        {
            /* erase failed, add to bad block table */
            mark_bad_block(block);
        }
        block = progmanage_next_valid_blk(block, 1);
    } 
#endif
#endif

    //clear mmu at end of initphase.
    for (vaddr=REMAP_PADDR_START; vaddr<(RAM_BASE_ADDR+NORMAL_PHYSICAL_SIZE); vaddr+=VPAGE_SIZE)
    {
        MMU_MapPageEx(vaddr, vaddr+VPAGE_SIZE, 0, FB_SMALL); 
    }

    remap_initphase_end(); 

    AK_PRINTK(PROGSTR_INIT_END, 0, AK_TRUE);
}
#pragma arm section code 

T_VOID progmanage_resume(T_VOID)
{
#ifdef DEEP_STANDBY
    /* check deep standby flag, load saved data, and resume cpu status.
       finally, go back to the place entering deep standby mode if all succeed. */
    deepstdb_handler();
#endif

    AK_PRINTK(SUCCESS_PROG, 0, AK_TRUE);
}

T_VOID progmanage_set_protect(T_BOOL protect)
{
    prog_manage.protect = protect;
}
/**
 * @brief check the page can write or not
 * @param T_U32 pageno, the page number to check
 * @retval T_BOOL, if writeable, return AK_TRUE, else dead
 */
T_BOOL progmanage_check_page_writeable(T_U32 chip, T_U32 pageno)
{
#if (STORAGE_USED == NAND_FLASH)
    if ((0==chip) && prog_manage.protect)
    {
        if ((pageno >> prog_manage.mbit) < prog_manage.swap.start_blk)
        {
            AK_PRINTK(ERROR_PROG_CHECK_WRITEAREA, pageno, AK_TRUE);
            while(1);
        }
    }
    return AK_TRUE;
#else
    return AK_FALSE;
#endif
}

#if (STORAGE_USED == SPI_FLASH)
#pragma arm section code = "_mmi_"

T_BOOL spiflash_file_read(T_S8 *name, T_U32 offset, T_U8 *pBuf, T_U32 len, T_BOOL bAlign)
{
    T_U16 startPage = 0;
    T_U32 pageCnt;
    T_U8 tempBuf[256];
    T_U32 misAlignBytes;
    T_U32 i;
    T_U8 *pOutBuf = pBuf;
    
    if(0 == progmanage_get_maplist(name, &startPage, 1))    //获取资源文件起始page
    {
    	// AK_DEBUG_OUTPUT("get startPage Error\n");
        return AK_FALSE;
    }

    if(bAlign)  //page是否需要以block对齐
    {
        startPage = ALIGN(startPage, SPIFLASH_PAGE_PER_BLOCK);
    }

    startPage += ALIGN_DOWN(offset, SPIFLASH_PAGE_SIZE) / SPIFLASH_PAGE_SIZE;    //偏移位置所在的page
    pageCnt = (ALIGN(offset+len, SPIFLASH_PAGE_SIZE) - ALIGN_DOWN(offset, SPIFLASH_PAGE_SIZE)) / SPIFLASH_PAGE_SIZE;    //需要读取的page数

    for(i = 0; i < pageCnt; i++)
    {      
        if(AK_FALSE == Fwl_spiflash_read(startPage+i, tempBuf, 1))
        {
        	// AK_DEBUG_OUTPUT("READ SPI Error\n");
            return AK_FALSE;
        }      
        
        if(0 == i)  //首页
        {
            //需要读取的偏移位置不对齐的字节数
            misAlignBytes = SPIFLASH_PAGE_SIZE - offset % SPIFLASH_PAGE_SIZE;    
            if(misAlignBytes > len)   //如果不对齐字节数大于要读取的长度
            {
                misAlignBytes = len;
            }   
            memcpy(pOutBuf, tempBuf + offset%SPIFLASH_PAGE_SIZE, misAlignBytes);
            pOutBuf += misAlignBytes;
        }
        else if((pageCnt - 1) == i) //末页
        {
            memcpy(pOutBuf, tempBuf, pBuf + len - pOutBuf);
            pOutBuf = pBuf + len;
        }
        else
        {   
            memcpy(pOutBuf, tempBuf, SPIFLASH_PAGE_SIZE);
            pOutBuf += SPIFLASH_PAGE_SIZE;
        }
    }

    return AK_TRUE;
}

#pragma arm section code
#endif
#if(STORAGE_USED == SD_CARD)
//bRead:AK_FALSE is Write,AK_FALSE is READ
#pragma arm section code = "_mmi_"
T_BOOL SDCARD_CFGFILE_RW(T_U32 start_sec, T_U32 FileOffset, T_U8 *pBuf,T_U32 len, T_BOOL bRead)
{
    T_U8 *FileBuf;
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    T_U32 ByteOffset = FileOffset&(SD_SECTOR_SIZE-1);
    //FileOffset&(SD_SECTOR_SIZE-1)+len 偏移对512字节取余后加长度后来获取最终要读的扇区数
    T_U32 SectorNum = ALIGN((ByteOffset+len),SD_SECTOR_SIZE)/SD_SECTOR_SIZE;
    T_U32 SectorOffset = ALIGN_DOWN(FileOffset, SD_SECTOR_SIZE)/SD_SECTOR_SIZE;
    FileBuf = (T_U8 *)Fwl_Malloc(SectorNum*SD_SECTOR_SIZE);
    //AK_DEBUG_OUTPUT("StartSec:%d,FileOffset:%d,ByteOffset:%d,len:%d\n",start_sec+SectorOffset,FileOffset,ByteOffset,len);
    if(FileBuf == AK_NULL)
    {
        return AK_FALSE;
    }
    if(!Fwl_SD_Read(sys_sd_handle,start_sec+SectorOffset,FileBuf,SectorNum))
    {
        goto FINISH;
    }
    if(bRead)
    {
        for(i=0;i<len;i++)
        {
            pBuf[i] = FileBuf[ByteOffset+i];
        }
    }
    else
    {
        for(i=0;i<len;i++)
        {
            FileBuf[ByteOffset+i]= pBuf[i];
        }
        if(!Fwl_SD_Write(sys_sd_handle,start_sec+SectorOffset,FileBuf,SectorNum))
        {
            goto FINISH;
        }
    }
    ret = AK_TRUE;
FINISH:
    Fwl_Free(FileBuf);
    return ret;
}
#pragma arm section code
#endif

void CorePageDbg(T_U32 pageNo, T_U32 addr)
{
#ifdef SWAP_WRITBAK_PRINT
    prog_manage.swap_cnt++;
#endif

#if 0
#ifdef OS_ANYKA
    {
        const T_CHR str[] = "->";

        akerror(AK_NULL, pageNo, 0);
        akerror(str, addr, AK_TRUE);
    }
#endif
#endif
}

/**
 * @brief prefecth and data abort exception handler
 * @param pc_lr, the lr value
 * @param type, 1: prefetch error, 2: data abort
 * @retval T_VOID
 */
T_VOID exception_handler(T_U32 pc_lr, T_U32 type, T_U32 sp, T_U32 cur_lr)
{   
    T_U32 fault_addr;
    T_U32 irq_flag = 0;
    
    /* exception is not come from irq mode */
     //if (type != DATA_ABORT_ERR) //(AK_TRUE == checkirq)
    {
        irq_flag = (get_spsr()&0x1F);
        if ((ANYKA_CPU_Mode_IRQ== irq_flag) || (ANYKA_CPU_Mode_FIQ == irq_flag))
        {
            AK_PRINTK(ERROR_ABORT_AT_IRQMODE, irq_flag, AK_TRUE);
        }
    }

    /* prefetch exception */
    if (type == PREFECTH_ERR)
    {
        fault_addr = pc_lr;
    }
    /* data abort exception */
    
    else if (type == DATA_ABORT_ERR)
    {
        fault_addr = MMU_Reg6();
        if (MMU_Vaddr2Paddr(fault_addr))
        {
            irq_flag = ANYKA_CPU_Mode_IRQ;
            AK_PRINTK(err_str, __LINE__, AK_TRUE);
        }
    }
 
    /*check fault address validity */
	//spi版本在IRQ和FIQ中可以产生DATA ABORT
    if (fault_addr < REMAP_VADDR_START
        ||  fault_addr >= (RAM_BASE_ADDR + RAM_SIZE) 
        || (
        ((ANYKA_CPU_Mode_IRQ == irq_flag) || (ANYKA_CPU_Mode_FIQ == irq_flag))))
    {
        AK_PRINTK(ERROR_ABORT_PREMODE_PC, pc_lr, AK_TRUE);
        AK_PRINTK(ERROR_ABORT_FAULTDATA_ADDR, fault_addr, AK_TRUE); 
        irq_flag = 6;
        while(irq_flag != 0)
        {
            AK_PRINTK(ERROR_ABORT_STACK,(T_U32)(*((T_U32*)(ABORT_MODE_STACK - (irq_flag<<2)))),AK_TRUE); 
            irq_flag--;
        }

        AK_PRINTK(ERROR_ABORT_PREMODE_LR, cur_lr, AK_TRUE);
        while(1);
    }
    
    /* do swap */
    do_swap(fault_addr, pc_lr, sp);

}

#endif


#if(STORAGE_USED == NAND_FLASH)
#ifdef DEEP_STANDBY

#pragma arm section code ="_deepstdb_"

/******************************************************************************
 * @brief   test print phy2vir table for debug
 * @author  luojianhua
 * @date    2009-08-25
 * @param   NONE
 * @return  NONE
 ******************************************************************************/

#ifdef DEEP_DEBUG
static T_VOID deepstdb_print_phy2vir(T_VOID)
{
    T_U32 i;

    for (i=0; i<REMAP_MEM_PSIZE/VPAGE_SIZE; i++)
    {
        if (remap_get_phy2virTab(i))
        {
            akerror(s_cDeep_index, i, AK_FALSE);
            akerror(s_cDeep_resv, remap_page_is_resv(i), AK_FALSE);
            akerror(s_cDeep_vaddr, remap_get_phy2virTab(i), AK_TRUE);
        }
    }
}
#endif

/******************************************************************************
 * @brief   the callback function of deep standby.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_fPOWEROFF_CALLBACK callback_func:the address of the callback function.
 * @return  NONE
 ******************************************************************************/

T_VOID deepstdb_set_callback(T_fPOWEROFF_CALLBACK callback_func)
{
    power_off_callback = callback_func;
}

/******************************************************************************
 * @brief   save position and size of DMA memory
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [out] T_DMA_INFO dma_info[]: save position and size of DMA memory
            [in] T_U32 groups: max groups of DMA app
 * @return  T_U32: using DMA groups actually
 ******************************************************************************/
T_U32 deepstdb_DMA_save(T_DMA_INFO dma_info[], T_U32 groups)
{
    T_U32 grp=0;
    T_U32 index;
    T_U32 index_tmp=0;
    T_U32 vaddr;

    /* find DMA position and size */
    for (index=0; index<((REMAP_MEM_PSIZE>>VPAGE_BITS)-2); index++)
    {
        /* check if reserved */
        if (remap_page_is_resv(index))
        {
            vaddr = remap_get_phy2virTab(index);
            /* unreserve the memory */
            remap_clr_pg_resv(index);
                       
            /* check if heap memory */
            //if (vaddr >= heap_start && vaddr <= heap_end)
            if (vaddr >= mmidata_start && vaddr <= zibss_end)//包括了非常驻全局变量及堆变量。
            {
#ifdef DEEP_DEBUG
                akerror(s_cDeep_index, index, AK_FALSE);
                akerror(s_cDeep_vaddr, vaddr, AK_TRUE);
#endif
                /* groups don't over DMA_APP_MAX */
                if (grp > groups)
                {
                    akerror(s_cDeep_W8, grp, AK_TRUE);
                    break;
                }
                
                /* add size if vaddr(and paddr?) is sequential, otherwise save next group */
                if (grp != 0 && 
                    dma_info[grp-1].vaddr + dma_info[grp-1].size == vaddr && 
                    index_tmp+1 == index)
                {
                    //sequential pages is in the same group
                    dma_info[grp-1].size += VPAGE_SIZE;
                }
                else
                {
                    grp++;    //next group
                    dma_info[grp-1].vaddr = vaddr;
                    dma_info[grp-1].size = VPAGE_SIZE;
                }
                index_tmp = index;
            }
        }
    }
#ifdef DEEP_DEBUG
    /* test to print infor */
    for (index=0; index<grp; index++)
    {
        index_tmp = (MMU_Vaddr2Paddr(dma_info[grp-1].vaddr)-REMAP_PADDR_START) >> VPAGE_BITS;
        akerror(s_cDeep_index, index_tmp, AK_FALSE);
        akerror(s_cDeep_vaddr, dma_info[grp-1].vaddr, AK_FALSE);
        akerror(s_cDeep_size, dma_info[grp-1].size, AK_TRUE);
    }
#endif

    return grp;
}  

/******************************************************************************
 * @brief   resume application of DMA memory
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in] T_DMA_INFO dma_info[]: position and size of DMA memory
            [in] T_U32 groups: using groups of DMA app
 * @return  T_VOID
******************************************************************************/
T_VOID deepstdb_DMA_resume(T_DMA_INFO dma_info[], T_U32 groups)
{
    T_U32 grp;

#ifdef DEEP_DEBUG
    T_U32 i;
    deepstdb_print_phy2vir();
#endif

    for (grp = 0; grp < groups; grp++)
    {
        /* resume DMA application */
        remap_resv_page(dma_info[grp].vaddr, dma_info[grp].size, AK_TRUE);   //not get unused physical page!
#ifdef DEEP_DEBUG
        i = (MMU_Vaddr2Paddr(dma_info[grp].vaddr)-REMAP_PADDR_START)>>VPAGE_BITS;
        akerror(s_cDeep_index, i, AK_FALSE);
        akerror(s_cDeep_vaddr, dma_info[grp].vaddr, AK_FALSE);
        akerror(s_cDeep_size, dma_info[grp].size, AK_TRUE);
#endif
    }
}

/******************************************************************************
 * @brief   resume application of DMA memory
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in] T_U16 cur_blk: current block
            [in] T_U32 num: shift times
 * @return  T_U16: front valid block
******************************************************************************/
static T_U16 deepstdb_front_valid_blk(T_U16 cur_blk, T_U32 num)
{
    T_U32 i;

    for (i=0; i<num;)
    {
        cur_blk--;
        if (cur_blk < prog_manage.swap.start_blk)
            cur_blk = prog_manage.swap.end_blk;

        /* get valid block */
        if (check_bad_block(cur_blk - prog_manage.swap.start_blk))
            continue;
        i++;
    }    
    return cur_blk;
}

/******************************************************************************
 * @brief   compare counter, return AK_TRUE if "biggish" bigger then "less" truely 
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in] T_U32 biggish: one of the comparing counter
            [in] T_U32 less: other the comparing counter
 * @retval  T_BOOL: AK_TRUE if "biggish" is the bigger one, otherwise return AK_FALSE
 ******************************************************************************/
static T_BOOL deepstdb_compare_cnt(T_U32 biggish, T_U32 less)
{
    if (biggish > less)
    {
        /* "biggish" is the bigger one if 0xffffffff or difference is normal */
        if ((biggish == DEEP_MAX_U32) ||
            (less == 0x0) ||
            (biggish - less <= prog_manage.swap.end_blk - prog_manage.swap.start_blk))
            return AK_TRUE;
        else
            return AK_FALSE;
    }
    else
    {
        /* "less" is the bigger one if 0xffffffff or difference is normal */
        if ((less == DEEP_MAX_U32) ||
            (biggish == 0x0) ||
            (less - biggish <= prog_manage.swap.end_blk - prog_manage.swap.start_blk))
            return AK_FALSE;
        else
            return AK_TRUE;
    }
}

/******************************************************************************
 * @brief   compare counter in some page of all swap block to find max counter, 
            read counter in spare, just 512B every time.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [out] T_U32 *cnt_max: buffer for the max counter
            [out] T_U16 *last_blk: buffer for the block with max counter
 * @retval  T_BOOL: AK_TRUE if find max counter, otherwise return AK_FALSE
 ******************************************************************************/
static T_BOOL deepstdb_find_max_cnt(T_U32 *cnt_max, T_U16 *last_blk)
{
    T_U32 spare;
    T_U32 start_page;
    T_U32 cnt;          //n:中间块的counter值
    T_U32 cnt_start;    //n1:查找区间前限块的counter值
    T_U32 cnt_end;      //n2:查找区间后限块的counter值
    T_U16 blk;          //b:中间块
    T_U16 blk_tmp;      //b':中间块的备份
    T_U16 blk_start=prog_manage.swap.start_blk;   //b1:查找区间的前限block
    T_U16 blk_end=prog_manage.swap.end_blk;       //b2:查找区间的后限block
    T_U8 *addr;
    T_BOOL read_failed=AK_FALSE;
    T_BOOL find_max=AK_FALSE;


    /* initialize 0. */
    *cnt_max = 0;    //保留counter最大值
    *last_blk = 0;   //保留counter最大值所在的swap block

    /* alloc memory in svc stack region */
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

    /* 1. read spare from blk_start */
    blk = blk_start;
    if (check_bad_block(blk-prog_manage.swap.start_blk))
    {
        blk = progmanage_next_valid_blk(blk, 1);
    }
    start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
    while (0 !=  nand_remap_read(start_page, addr,&spare, AK_FALSE) && blk <= blk_end)
    {
        blk = progmanage_next_valid_blk(blk, 1);
        start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
    }
#ifdef DEEP_DEBUG
    akerror(s_cDeep_find_max, 0, AK_TRUE);
    akerror(s_cDeep_blk, blk, AK_FALSE);
    akerror(s_cDeep_cnt, spare, AK_TRUE);
#endif
    cnt_start = spare;
    blk_start = blk;
    
    /* 2. read spare from blk_end */
    blk = blk_end;
    if (check_bad_block(blk-prog_manage.swap.start_blk))
    {
        blk = deepstdb_front_valid_blk(blk, 1);
    }
    spare = 0;
    start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
    while ((blk > blk_start) && (0 != nand_remap_read(start_page, addr,&spare, AK_FALSE)))
    {
        blk = deepstdb_front_valid_blk(blk, 1);
        start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
    }
#ifdef DEEP_DEBUG
    akerror(s_cDeep_blk, blk, AK_FALSE);
    akerror(s_cDeep_cnt, spare, AK_TRUE);
#endif
    /* get cnt_end and blk_end according to spare */
    if (spare != 0)
    {
        cnt_end = spare;
        blk_end = blk;
    }
    else
    {
        akerror(s_cDeep_W13, blk, AK_TRUE);
        while(1);   //just one swap block is good!
    }

    /* 3. compare cnt_start and cnt_end and deal with */
    if (cnt_start == DEEP_MAX_U32)
    {
        //末块存在*cnt_max，结束查找!
        if (cnt_end != DEEP_MAX_U32)
        {
            *cnt_max = cnt_end;
            *last_blk = blk_end;
            find_max = AK_TRUE;    //finish looking for
        }
        //*cnt_max在swap区尾部m个块内或者系统未进入过Deep Standby，直接从中找出非0xffffffff的*cnt_max。
        else
        {
            T_U32 counter = 0;
            blk_start = deepstdb_front_valid_blk(blk_end, NAND_SWAP_RESV_BLOCK);

            /* find the max counter in the last four blocks */
            for (blk = deepstdb_front_valid_blk(blk_end, 1); blk >= blk_start; blk=deepstdb_front_valid_blk(blk, 1))
            {
                start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
                if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE))
                {
#ifdef DEEP_DEBUG
                    akerror(s_cDeep_blk, blk, AK_FALSE);
                    akerror(s_cDeep_cnt, spare, AK_TRUE);
#endif
                    /* break if find the max counter */
                    if (spare != DEEP_MAX_U32)
                    {
                        *cnt_max = spare;
                        *last_blk = blk;
                        find_max = AK_TRUE;
                        break;    //finish looking for
                    }
                }
                //刚好系统NAND_SWAP_RESV_BLOCK+1个交换区block，加入循环计数，防止死循环
                if (NAND_SWAP_RESV_BLOCK < counter)
                {
                    break;
                }
                counter++;
            }
            /* max counter 在swap区尾部m个块内, if (*cnt_max != 0),
               else: 系统未进入过Deep Standby, maybe have entered!? */
            
            /* quit here after free remap_alloc(or go on, but read once more) */
            /* free addr and return */
            remap_free(addr);
            return find_max;
        }
    }
    else
    {
        //记录*cnt_max，要取中值后才能判断
        if (cnt_end == DEEP_MAX_U32)
        {
            *cnt_max = cnt_start;
            *last_blk = blk_start;
        }
        else
        {
            //末块存在*cnt_max，结束查找!
            //if (cnt_start < cnt_end)
            if (deepstdb_compare_cnt(cnt_end, cnt_start))
            {
                *cnt_max = cnt_end;
                *last_blk = blk_end;
                find_max = AK_TRUE;    //finish looking for
            }
            //出现回环，记录*cnt_max，要取中值后才能判断
            else if (deepstdb_compare_cnt(cnt_start, cnt_end))
            {
                *cnt_max = cnt_start;
                *last_blk = blk_start;
            }
            /* else if equal: go on, but read once more(or quit here, but need remap_free) */
        }
    }
    
    /* just two cases: 1. start val is not DEEP_MAX_U32 and end val is DEEP_MAX_U32;
                       2. both are DEEP_MAX_U32 and start val is bigger then end val; */
    while(!find_max)
    {
#ifdef DEEP_DEBUG
        akerror(s_cDeep_blk_start, blk, AK_FALSE); // middle block
        akerror(s_cDeep_blk, blk_start, AK_FALSE); // middle block
        akerror(s_cDeep_cnt, cnt_start, AK_TRUE);  // middle count
        akerror(s_cDeep_blk_end, blk, AK_FALSE); // middle block
        akerror(s_cDeep_blk, blk_end, AK_FALSE); // middle block
        akerror(s_cDeep_cnt, cnt_end, AK_TRUE);  // middle count
#endif
        /* 4. compare blk_start and blk_end , then deal with it, 
        finish looking for if (blk_end - blk_start)<2 */
        /* get middle variable of blcok */    
        if (read_failed == AK_TRUE)
        {
            read_failed = AK_FALSE;
            blk = blk_end - blk_tmp;
        }
        else
        {
            blk = blk_end - blk_start;
        }
        /* get middle variable of blcok */    
        if (blk >= (2))
        {
            blk = blk>>1;
            blk += blk_start;   //取中值
        }
        else    //区间内没有元素，查找完毕！
        {
#ifdef DEEP_DEBUG
            akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
            find_max = AK_TRUE;
            break;    //finish looking for
        }
        
        /* 5. read from middle block and deal with */
        blk_tmp = blk;  //backup middle block
        if (check_bad_block(blk - prog_manage.swap.start_blk))
        {
            blk = deepstdb_front_valid_blk(blk, 1);
        }
        /* look forth for good block */
        start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
        while (blk > blk_start && 0 != nand_remap_read(start_page, addr, &spare, AK_FALSE))//avoid read blk_start again!
        {
            blk = deepstdb_front_valid_blk(blk, 1);
            start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
        }
        
        //往前读结束处理
        //if (blk==blk_start)
        if (blk<=blk_start) //blk_start与中间块之间全读错,再读中间块与blk_end之间的块，然后先判别处理
        {
            /* look back for good block if all is bad block in the middle of blk_start and blk */
            blk = progmanage_next_valid_blk(blk_tmp, 1);
            start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            while (blk < blk_end && 0 != nand_remap_read(start_page, addr, &spare, AK_FALSE))//avoid read blk_end again!
            {
                blk = progmanage_next_valid_blk(blk, 1);
                start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            }
            //往后读结束处理
            //if (blk==blk_end)
            if (blk>=blk_end)
            {
                //all interzone blocks is bad blocks!but fint max counter in blk_start just now.
                find_max = AK_TRUE;
#ifdef DEEP_DEBUG
                akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
                break;    //finish looking for
            }
            else
            {
                cnt = spare;
                /* the max counter must be in back-half */
                //if (cnt_start < cnt)
                if (deepstdb_compare_cnt(cnt, cnt_start))
                {
                    if (cnt != DEEP_MAX_U32)
                    {
                        *cnt_max = cnt;
                        *last_blk = blk;
                        blk_start = blk;
                        cnt_start = cnt;
                        continue;
                    }
                    /* the max counter must be in front-half, but all are bad blocks */
                    else
                    {
                        *cnt_max = cnt_start;
                        *last_blk = blk_start;
                        find_max = AK_TRUE;
#ifdef DEEP_DEBUG
                        akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
                        break;    //finish looking for
                    }
                }
                /* the max counter must be in front-half, but all are bad blocks */
                //else if (cnt_start > cnt)
                else if (deepstdb_compare_cnt(cnt_start, cnt))
                {
                    *cnt_max = cnt_start; //maybe saved in front
                    *last_blk = blk_start;
                    find_max = AK_TRUE;
#ifdef DEEP_DEBUG
                    akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
                    break;    //finish looking for
                }
                else    //quit while(!find_max) if equal
                {
#ifdef DEEP_DEBUG
                    akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
                    break;
                }
            }
        }
        else     //blk_start与中间块之间读到counter
        {
            cnt = spare;
            if (blk_tmp != blk) //说明曾经读错过
                read_failed = AK_TRUE;
        }
        
#ifdef DEEP_DEBUG
        akerror(s_cDeep_blk, blk, AK_FALSE); // middle block
        akerror(s_cDeep_cnt, cnt, AK_TRUE);  // middle count
#endif

        /* 6. compare cnt, cnt_start and cnt_end, then deal with */
        /* max counter in front-half: case I.III.IV.VI and VII. */
        if ((deepstdb_compare_cnt(cnt_end, cnt_start) && cnt_end == DEEP_MAX_U32 && cnt == DEEP_MAX_U32)
         || (deepstdb_compare_cnt(cnt_end, cnt_start) && cnt_end == DEEP_MAX_U32 && deepstdb_compare_cnt(cnt_start, cnt))
         || (deepstdb_compare_cnt(cnt_start, cnt_end) && cnt == DEEP_MAX_U32)
         || (deepstdb_compare_cnt(cnt_start, cnt_end) && deepstdb_compare_cnt(cnt_start, cnt)))
        {
            blk_end = blk;
            cnt_end = cnt;
            read_failed = AK_FALSE;
        }
        /* max counter in back-half: case II and V. */
        else if ((deepstdb_compare_cnt(cnt_end, cnt_start) && cnt_end == DEEP_MAX_U32 && deepstdb_compare_cnt(cnt, cnt_start))
              || (deepstdb_compare_cnt(cnt_start, cnt_end) && deepstdb_compare_cnt(cnt, cnt_start)))
        {
            blk_start = blk;
            cnt_start = cnt;
            *cnt_max = cnt_start;
            *last_blk = blk_start;
        }
        /* other case. */
        else    //quit while(!find_max) if equal
        {    
#ifdef DEEP_DEBUG
            akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
#endif
            break;
        }
    }   //end of while(!find_max)
    
#ifdef DEEP_DEBUG
    akerror(s_cDeep_blk, blk, AK_FALSE); // middle block
    akerror(s_cDeep_cnt, cnt, AK_TRUE);  // middle count
#endif

    /* free addr and return */
    remap_free(addr);
    return find_max;
}

/******************************************************************************
 * @brief   look for 80 times to find the max counter via the second way.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [out] T_U32 *cnt_max: buffer for the max counter
            [out] T_U16 *last_blk: buffer for the block with max counter
 * @return  T_VOID
 ******************************************************************************/
static T_VOID deepstdb_find_max_cnt_dg(T_U32 *cnt_max, T_U16 *last_blk)
{
    T_U32 spare;
    T_U32 start_page;
    T_U16 blk;
    T_U8 *addr;

    /* alloc memory in svc stack region */
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

    /* initialize 0. */
    *cnt_max = 0;
    *last_blk = 0;  //maybe not need
    /* find the max counter via the second way. */
    for (blk=prog_manage.swap.start_blk; blk<=prog_manage.swap.end_blk; blk++)
    {
        /* read spare if not bad block */
        if(!check_bad_block(blk-prog_manage.swap.start_blk))
        {
            /* read spare to save max counter */
            start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE))
            {
                /* save the bigger counter and its position */
                if (deepstdb_compare_cnt(spare, *cnt_max) && spare != DEEP_MAX_U32)
                {
                    *cnt_max = spare;
                    *last_blk = blk;
                }
            }
        }
    }

    /* free addr */
    remap_free(addr);
}

/******************************************************************************
 * @brief   judge deep standby mode via reading flag in block with max counter
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [out]T_U32 *counter: the max counter finding in swap space
            [out]T_DEEPSTDB_BLK *stdb_blk_find: the standby block finding in swap space
 * @retval  T_BOOL: AK_TRUE if deep standby mode, otherwise return AK_FALSE
 ******************************************************************************/
static T_BOOL deepstdb_check(T_U32 *counter, T_DEEPSTDB_BLK *stdb_blk_find)
{
    T_U32 i;
    T_U32 spare;
    T_U32 start_page;
    T_U32 cnt_max=0;    //保留counter最大值
    T_U32 temp;
    T_U16 blk;          //b:
    T_U16 last_blk=0;   //保留counter最大值所在的swap block
    T_U16 last_blk_tmp;
    T_U8 *addr;
    T_BOOL ret;
   
#ifdef DEEP_DEBUG
    /* alloc memory in svc stack region */
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

    /* test: print counter info of spare in swap block */
    for (blk=prog_manage.swap.start_blk; blk<=prog_manage.swap.end_blk; blk++)
    {
        /* read spare if not bad block */
        if(!check_bad_block(blk-prog_manage.swap.start_blk))
        {
            start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE))
            {
                akerror(s_cDeep_blk, blk, AK_FALSE);
                akerror(s_cDeep_cnt, spare, AK_TRUE);
            }
        }
    }

    /* free the addr applied by remap_alloc */
    remap_free(addr);
#endif

    /* compare counter in some page of all swap block to find max counter. */
    ret = deepstdb_find_max_cnt(&cnt_max, &last_blk);

    /* save block position of max counter */
    last_blk_tmp = last_blk;
    
    /* check deep standby flag and cancel flag if deepstdb_find_max_cnt return 1 */
    if (ret)
    {
        /* alloc memory in svc stack region */
        addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

        /* check flag in last two pages of the block to find the deep standby block.
           read spare data in front of the last page firstly, then read the last page. */
        for (i=0; i<2; i++)
        {
            /* read spare data in front of the last page firstly */
            start_page = (last_blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE;
            if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE))
            {
#ifdef DEEP_DEBUG
                akerror(s_cDeep_flag, spare, AK_FALSE);
#endif
                if ( spare == DEEP_FLAG_BLK_PRIMARY ||  
                     spare == DEEP_FLAG_BLK_BACKUP )
                {
                    /* backup the standby flag */
                    temp = spare;
                    /* read spare data in the last page */
                    start_page++;
                    if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE));
                    {
#ifdef DEEP_DEBUG
                        akerror(s_cDeep_flag, spare, AK_FALSE);
                        akerror(s_cDeep_flag, temp, AK_TRUE);
#endif
                        /* get deep standby position. */
                        if (spare == DEEP_MAX_U32)// && *((T_U32*)addr) == DEEP_MAX_U32)
                        {
                            if (temp == DEEP_FLAG_BLK_PRIMARY)
                            {
                                stdb_blk_find->primary = last_blk;
                                break;
                            }
                            else if (temp == DEEP_FLAG_BLK_BACKUP)
                                stdb_blk_find->backup = last_blk;
                        }
                    }
                }
            }
            last_blk = deepstdb_front_valid_blk(last_blk, 1);
        }   //end of for (i=0; i<2; i++)
        
        /* free the addr applied by remap */
          remap_free(addr);
    }

    akerror(s_cDeep_blk, last_blk_tmp, AK_FALSE);
    akerror(s_cDeep_max, cnt_max, AK_TRUE);
    
    /* transfer max counter */
    *counter = cnt_max + 1;

    /* judge according to *stdb_blk_find and return */
    if (stdb_blk_find->primary != 0 || stdb_blk_find->backup != 0)
    {
        akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
        return AK_TRUE;
    }
    else
    {
#if 1//test
        /* find the max counter via the second way. */
        deepstdb_find_max_cnt_dg(&cnt_max, &last_blk);

        /* print all counters in spare if not the block with the max counter */
        if (cnt_max != *counter - 1)    //just use *counter as backup of cnt_max here!
        {
            /* save block position of max counter */
            //last_blk_tmp = last_blk;  //避免重复查找，否则要重新check flag

            /* update max counter */
            *counter = cnt_max + 1;

            /* alloc memory in svc stack region */
            addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

            /* test: print counter info of spare in swap block */
            for (blk=prog_manage.swap.start_blk; blk<=prog_manage.swap.end_blk; blk++)
            {
                /* read spare if not bad block */
                if(!check_bad_block(blk-prog_manage.swap.start_blk))
                {
                    start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
                    if (0 == nand_remap_read(start_page, addr, &spare, AK_FALSE))
                    {
                        akerror(s_cDeep_blk, blk, AK_FALSE);
                        akerror(s_cDeep_cnt, spare, AK_TRUE);
                    }
                }
            }
            akerror(s_cDeep_blk, last_blk, AK_FALSE);
            akerror(s_cDeep_max, cnt_max, AK_TRUE);

            /* free the addr applied by remap_alloc */
           remap_free(addr);
        }
#endif
        /* transfer block position with max counter if not deep standby mode */
        stdb_blk_find->backup = last_blk_tmp;   //just use stdb_blk_find->backup to record last blk here!
        akerror(s_cDeep_ret_line, __LINE__, AK_TRUE);
        return AK_FALSE;
    }
}


/******************************************************************************
 * @brief check deep standby flag, load saved data, and resume cpu status.
          finally, go back to the original place.
 * @author  luojianhua
 * @date    2009-08-25
 * @param T_VOID
 * @retval T_VOID
******************************************************************************/

T_VOID deepstdb_handler(T_VOID)
{
    T_U32 i;
    T_U32 counter;
    T_DEEPSTDB_BLK blk_find;
    T_U16 block=0;  //must initialize to 0

    /* power on before deepstdb_handler if swap blk value is ok, or reset if not. */
    if (prog_manage.swap.start_blk<DEEP_MAX_SWAP_NUM && prog_manage.swap.end_blk<DEEP_MAX_SWAP_NUM)
    {
        //RTC_WatchDog_Output_level(1);
    }
    else
    {
        akerror(s_cDeep_W18, prog_manage.swap.start_blk, AK_FALSE);
        akerror(s_cDeep_blk, prog_manage.swap.end_blk, AK_TRUE);
        Fwl_SysReboot();
    }


    /* test start-up time */
    //gpio_set_pin_level(6, 0);

    /* check deep standby flag, load saved data if ok, then resume cpu status if all is ok*/
    counter = 0;      //initail 0, for transfer counter of using swap block
    blk_find.primary = 0;
    blk_find.backup = 0;
    if (deepstdb_check(&counter, &blk_find))
    {
        if (deepstdb_load(counter, blk_find))
        {
            /* resume cpu status and get back the pc to 
               jump to the place entering deep standby mode */       
            deepstdb_resume();  
        }
        else
        {
            akerror(s_cDeep_W19, 0, AK_TRUE);
            Fwl_SysReboot();  //sys reset
        }
    }

    /* get the first block for new swap */
    if (blk_find.backup  == 0)
    {
        //get primary block
        if (blk_find.primary != 0)
            block = progmanage_next_valid_blk(blk_find.primary, 2);
    }
    else    //get backup block(or block with max counter)
        block = progmanage_next_valid_blk(blk_find.backup, 1);

    /* insure block is in swap space */
    if (block < prog_manage.swap.start_blk || block > prog_manage.swap.end_blk)
        block = prog_manage.swap.start_blk;

    /* update s_ulBlkCount and remap.swap if deep standby or not. */
    s_ulBlkCount = counter;
    prog_manage.swap.block = block ;
    
    /* erase reserved blocks of nand swap space after block  */
    i = 0;
    while(i++ < NAND_SWAP_RESV_BLOCK+1)
    {
        akerror(s_cDeep_erase, block, AK_TRUE);
        if (nand_eraseblock(0, block << prog_manage.mbit) != 0)
        {
            /* erase failed, add to bad block table */
            mark_bad_block(block);
        }
        block = progmanage_next_valid_blk(block, 1);
    }

#ifdef DEEP_DEBUG
    deepstdb_print_phy2vir();
#endif
}


/******************************************************************************
 * @brief   load resident data, verify block and sum, then write cancel flag.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in]T_U32 counter: the max counter finding in swap space
            [in]T_DEEPSTDB_BLK stdb_blk_find: the standby block finding in swap space
 * @retval  T_BOOL: AK_TRUE if all is successful, otherwise return AK_FALSE
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform     
         
******************************************************************************/
static T_BOOL deepstdb_load(T_U32 counter, T_DEEPSTDB_BLK stdb_blk_find)
{
    T_U32 i;
    T_U32 temp; 
    T_U32 index;
    T_U32 grp_resv;
    T_U32 grp_p2v;
    T_U32 pagesize;
    T_U32 cnt = DEEP_RESIDENT_SAVE_PAGES;
    T_U32 spare;
    T_U32 start_page;
    T_U32 phIdx;
    T_U32 vaddr[DEEP_RESUM_CODE_VPAGE]={0x0};
    T_U32 store_tbl[DEEP_RESUM_CODE_VPAGE]={0x0};
    T_U32 store_index[DEEP_RESUM_CODE_VPAGE]={0x0};
    T_U32 id[DEEP_RESUM_CODE_VPAGE]={0x0};
    T_U16 blk;
    T_U32 virAddr;
    T_U8 *addr = AK_NULL;
    T_BOOL flag = AK_TRUE;

    /* alloc memory in svc stack region, remap alloc shoud be done before check_ptr
       because the special rule of remap_alloc */
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);
    if (addr == AK_NULL)
        return AK_FALSE;
        
    index = remap_get_vaddrindex(addr); // get tempbuffer ram index
    phIdx = MMU_GetPT2Paddr(addr);      // get tempbuffer mmu info

    //load next 4KB code to avoid swaping in the memcpy() 
    i = get_pc();
    check_ptr(i + VPAGE_SIZE);

    pagesize = 1 << prog_manage.nbit;   //2KB

    /* save reserve table and mapping memory position of this code. */
    //记录映射关系到恢复映射关系期间禁止倒换!!!!!
    for (i=0,grp_resv=0,grp_p2v=0; i<REMAP_MEM_PSIZE/VPAGE_SIZE; i++)
    {
        //store_tbl[i]=prog_manage.ram2store_tbl[i];
        /* save reserve table */
        if (remap_page_is_resv(i))
        {
#ifdef DEEP_DEBUG
            akerror(s_cDeep_index, i, AK_TRUE);
            akerror(s_cDeep_vaddr, remap_get_phy2virTab(i), AK_TRUE);
#endif
            /* break if over */
            if (grp_resv >= DEEP_RESUM_CODE_VPAGE)
            {
                akerror(s_cDeep_W11, grp_resv+1, AK_TRUE);
                flag = AK_FALSE;
                //while(1);
            }
            id[grp_resv] = i;
            grp_resv++;
        }
        
        /* save phy->vir mapping table */
        if (remap_get_phy2virTab(i))
        {
#ifdef DEEP_DEBUG
            akerror(s_cDeep_index, i, AK_FALSE);
            akerror(s_cDeep_vaddr, remap_get_phy2virTab(i), AK_TRUE);
#endif
            /* break if over */
            if (grp_p2v >= DEEP_RESUM_CODE_VPAGE)
            {
                akerror(s_cDeep_W12, grp_p2v+1, AK_TRUE);
                flag = AK_FALSE;
                break;
                //while(1);
            }
            vaddr[grp_p2v] = remap_get_phy2virTab(i);
            store_index[grp_p2v]=i;
            store_tbl[grp_p2v]=prog_manage.ram2store_tbl[i];
            grp_p2v++;
        }
    }
    
    
    if (stdb_blk_find.primary != 0)
    {
        blk = stdb_blk_find.primary;
    }
    else if (stdb_blk_find.backup != 0)
    {
        blk = stdb_blk_find.backup;
    }
    else    //stdb_blk_find是0的话其实不会执行此函数
        return AK_FALSE;
        
#ifdef CHECK_STARTUPTICK
    virAddr = get_tick_count_us();
#endif

    /* get data from the primary deep standby block firstly. */
    start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE - cnt + 1;
    for(i = 0;i < cnt; i++)
    {
        if (0 != nand_remap_read(start_page+i, addr, &spare, AK_FALSE))
        {
            /* get data from the backup deep standby block if load primary failed */
            if (stdb_blk_find.primary != 0 && stdb_blk_find.backup != 0)
            {
                blk = stdb_blk_find.backup;
                start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE - cnt + 1;
                if (0 != nand_remap_read(start_page+i, addr, &spare, AK_FALSE))
                {
                    flag = AK_FALSE;
                    break;
                }
            }
            else
            {
                flag = AK_FALSE;
                break;
            }
        }

        /* copy datas from nandflash to resident space */
        if (i < DEEP_FIXDATA_PAGES)
         {
             if ((DEEP_FIXDATA_PAGES-1) == i)
             {
                 temp = (mmu_ptdata_size - 1)%pagesize + 1;
             }
             else
             {
                 temp = pagesize;
             }
             memcpy((T_U8 *)(mmu_ptdata_start+(i<<prog_manage.nbit)), addr, temp);

             if (MMU_GetPT2Paddr(addr) != phIdx)
             {
                MMU_MapPageEx(addr, addr+VPAGE_SIZE, REMAP_PADDR_START+(index<<VPAGE_BITS), RW_SMALL);  
             }
         }
         else
         {
             if ((DEEP_RESIDENT_SAVE_PAGES-1) == i)
             {
                 temp = (fixed_data_end - fixed_data_start - 1)%pagesize + 1;
             }
             else
             {
                 temp = pagesize;
             }
             memcpy((T_U8 *)(fixed_data_start+((i-DEEP_ADDED_DATA_PAGES)<<prog_manage.nbit)), addr,  temp);
         }
    }
    /* 本函数和“堆内存”的物虚关系在load后被清掉，但MMU关系还在，不会影响其使用，
       新的物虚关系和预留表是从进入休眠前的状态中恢复的数据，必须清掉，为后面恢复作准备. */

#ifdef CHECK_STARTUPTICK
    g_StartupTick = virAddr;
    g_ResumeStartTick = get_tick_count_us();
#endif

    temp = (mmidata_start & (~(VPAGE_SIZE-1))) - VPAGE_SIZE; //mmidata段前一个代码页

    // 清除掉phy2virTab表，ram2store_tbl表,同时phIdx保存到mmu中.
    for(i = 0;i < (REMAP_MEM_PSIZE/VPAGE_SIZE); i++)
    {
        remap_clr_pg_resv(i);
        virAddr = remap_get_phy2virTab(i);
        if(virAddr)
        {
            remap_set_phy2virTab(i,0);
            if (virAddr < temp)// 更新非坏块代码页
            {
                phIdx = progmanage_PreUnmapRam2Store(i, virAddr);
                MMU_MapPageEx(virAddr, virAddr+VPAGE_SIZE, (phIdx<<12), FB_SMALL);
            }
            remap_set_page_cleanone(i);
         }
         prog_manage.ram2store_tbl[i]=0;   
     }

    /* resume reserve table */
    // 设置remap的resver表
    for(i = 0;i < grp_resv; i++)
    {
       remap_set_pg_resv(id[i]);  
    }
    
    /* resume phy->vir mapping table before loading resident data */
    // 根据RAM中保存的内存页，设置phy2virTab表，ram2store_tbl表及mmu的映射.
    for(i = 0;i < grp_p2v; i++)
    {  
        index = store_index[i]; 
        if ((vaddr[i] < temp) || (vaddr[i] == (T_U32)addr))
        {
            remap_set_phy2virTab(index,vaddr[i]);
            prog_manage.ram2store_tbl[index]=store_tbl[i];
            MMU_MapPageEx(vaddr[i], vaddr[i]+VPAGE_SIZE, REMAP_PADDR_START+(index<<VPAGE_BITS), RW_SMALL);   
        }
        remap_set_page_cleanone(index);
     }

#ifdef DEEP_DEBUG
     akerror(s_cDeep_blk, stdb_blk_find.primary, AK_FALSE);
     akerror(s_cDeep_blk, stdb_blk_save.primary, AK_FALSE);
     akerror(s_cDeep_blk, stdb_blk_find.backup, AK_FALSE);
     akerror(s_cDeep_blk, stdb_blk_save.backup, AK_FALSE);
     akerror(s_cDeep_cnt, counter, AK_FALSE);
     akerror(s_cDeep_cnt, s_ulBlkCount, AK_TRUE);
#endif

    /* verify just after loading resident data successful */
    if (flag)
    {
        
        if ( (stdb_blk_find.primary != stdb_blk_save.primary && stdb_blk_find.backup != stdb_blk_save.backup)
          || (stdb_blk_find.backup == stdb_blk_save.backup && counter != s_ulBlkCount) )
        {
            akerror(s_cDeep_W9, 0, AK_TRUE);
            flag = AK_FALSE;
        }

        /* check sum just after succeed to load and verify */
        if (flag)
        {
            /* check sum of all fixed data, write false flag if wrong */
            temp = s_ulDeepSum;
            s_ulDeepSum = 0;
            s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start, (T_U32 *)fixed_data_end);

            if (s_ulDeepSum != temp)
            {
                akerror(s_cDeep_W10, s_ulDeepSum, AK_TRUE);
                flag = AK_FALSE;
            }
            /* 修正remap.swap.block: 当从主块恢复数据时，考虑备份块重新申请过的情况 */
            else
            {
                if (blk == stdb_blk_find.primary)
                    prog_manage.swap.block = progmanage_next_valid_blk(stdb_blk_find.primary, 2);
            }
        }
    }
    /* Invalidate TLB*/
    MMU_InvalidateTLB();
    
    /* Invalidate IDCache*/
    MMU_InvalidateIDCache();

      /* write cancel flag to the deep standby block whether valid */
    progmanage_set_protect(AK_FALSE);
    

    spare = DEEP_FLAG_CANCEL;
    temp = 0;
    blk = stdb_blk_find.primary;
    //check_stack();
    for (i = 0; i < 2; i++)
    {
        /* insure the block in swap space. */
        if (blk >= prog_manage.swap.start_blk && blk <= prog_manage.swap.end_blk)
        {
            start_page = (blk << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE + 1;
            if (0 != nand_remap_write(start_page, addr, spare, AK_FALSE))
            {
                /* mark as bad block */
                mark_bad_block(blk);
                temp ++;
                /* false if writing cancel to stdb_blk_find is failed */
                /* must both write failed when it's two standby blocks! */
                if (temp == 2 || stdb_blk_find.primary == 0 || stdb_blk_find.backup == 0)
                    flag = AK_FALSE;
            }
        }
        blk = stdb_blk_find.backup;
    }
    progmanage_set_protect(AK_TRUE);

    /* free addr and return */
    remap_free(addr);

    return flag;
}


/******************************************************************************
 * @brief   transfer data to next block and mark bad block
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in]T_U32 src: the source block
            [in]T_U32 dst: the targer block
            [in]T_U32 offset: page offset of datas in the source block
 * @retval  T_VOID
 ******************************************************************************/
static T_VOID deepstdb_transfer_data(T_U32 src, T_U32 dst, T_U32 offset)
{
    /* do replacement, mark as bad block, if wrong */
    while(!progmanage_do_replacement(src, dst, offset))
    {
        if (++prog_manage.M == NAND_SWAP_RESV_BLOCK)
        {
            akerror(s_cDeep_W14, NAND_SWAP_RESV_BLOCK-prog_manage.M, AK_TRUE);
            while(1);   //maybe not best solution?
        }
        /* mark as bad block */
        mark_bad_block(dst);      
        /* update dst blk */
        dst = progmanage_next_valid_blk(dst, 1);
    }
    
    /* mark it as bad block */
    mark_bad_block(src);
}

/******************************************************************************
 * @brief   save the fixed data to the deep standby block in nandflash
 * @author  luojianhua
 * @date    2009-08-25
 * @param   [in]T_U16 stdb_block: the deep standby block saving the fixed data 
            [in]T_U32 cnt: physical pages of the fixed data 
            [in]T_U16 page_offst: page offset of data in the (primary) deep standby block
 * @retval  T_VOID
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform,add two page of fixdata:
            the MMU and the address of (0x08009000~0x0800A000) fixdata.
 ******************************************************************************/
static T_VOID deepstdb_save_fixdata(T_U16 stdb_block, T_U32 cnt, T_U16 page_offst)
{
    T_U32 i, temp;
    T_U32 pagesize;
    T_U32 spare = 0;
    T_U32 start_page;
    T_U8 *addr;

    /* suspend if parameter is wrong */
    if (stdb_block < prog_manage.swap.start_blk ||
        stdb_block > prog_manage.swap.end_blk ||
        page_offst > g_nPagePerBlock_asa- cnt - 1)
    {
        akerror(s_cDeep_W15, page_offst, AK_FALSE);
        akerror(s_cDeep_blk, stdb_block, AK_TRUE);
        while(1);
    }
    /* alloc memory in svc stack region */
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);

    pagesize = 1 << prog_manage.nbit;   //2KB
    progmanage_set_protect(AK_FALSE);

write_failed:
    start_page = (stdb_block << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE - cnt + 1;
    for(i = 0;i < cnt; i++)
    {
        /* copy datas from fixed space to nandflash */
        if (i < DEEP_ADDED_DATA_PAGES)
        {
            if ((DEEP_ADDED_DATA_PAGES-1) == i)
            {
                temp = (mmu_ptdata_size - 1)%pagesize + 1;
            }
            else
            {
                temp = pagesize;
            }
            memcpy(addr, (T_U8 *)(mmu_ptdata_start+(i<<prog_manage.nbit)), temp);
            if (stdb_block == stdb_blk_save.primary)
                spare = s_ulBlkCount - 2;
            else if (stdb_block == stdb_blk_save.backup)
                spare = s_ulBlkCount - 1;   

        }
        else
        {
            if ((DEEP_RESIDENT_SAVE_PAGES-1) == i)
            {
                temp = (fixed_data_end - fixed_data_start - 1)%pagesize + 1;
            }
            else
            {
                temp = pagesize;
            }
            memcpy(addr, (T_U8 *)(fixed_data_start+((i-DEEP_ADDED_DATA_PAGES)<<prog_manage.nbit)), temp);
            if (stdb_block == stdb_blk_save.primary)
                spare = DEEP_FLAG_BLK_PRIMARY;
            else if (stdb_block == stdb_blk_save.backup)
                spare = DEEP_FLAG_BLK_BACKUP;
        }
        /* stdb_blk have changed, so need to go back to write_failed. */
        if (0 != nand_remap_write(start_page + i, addr, spare, AK_FALSE))
        {
            if (stdb_block == stdb_blk_save.primary)
            {
                /* resume page_offst, do replacement and mark as bad block */
                if (page_offst != 0)
                  deepstdb_transfer_data(stdb_blk_save.primary, stdb_blk_save.backup, page_offst);  
                
                /* update the deep standby block */
                stdb_blk_save.primary = progmanage_next_valid_blk(stdb_blk_save.primary, 1);
                stdb_blk_save.backup = progmanage_next_valid_blk(stdb_blk_save.primary, 1);
                prog_manage.swap.block = stdb_blk_save.backup;
                stdb_block = stdb_blk_save.primary;
            }
            else if (stdb_block == stdb_blk_save.backup)
            {
                /* mark as bad block */
                mark_bad_block(stdb_blk_save.backup);
                
                /* update the deep standby block */
                stdb_blk_save.backup = prog_manage.swap.block;
                stdb_block = stdb_blk_save.backup;
            }

             progmanage_request_new_block();
            /* calculate check-sum of all fixed data over again */
            s_ulDeepSum = 0;
            s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start, (T_U32 *)fixed_data_end);
            goto write_failed;
        }
    }

    progmanage_set_protect(AK_TRUE);
    remap_free(addr);
}
 
 /******************************************************************************
  * @brief   the calculate sum of the fixed data. 
  * @author  luojianhua
  * @date    2009-08-25
  * @param   T_U32 *start_addr:the start address of the data.
             T_U32 *end_addr:the end address of the data
  * @retval  the calculate sum of the data.
  * @MENDER        lisichun
  * @AMEND DATE    2012-09-12
  * @BRIEF:  add the address of fixdata.     
  ******************************************************************************/

__inline T_U32 deepstdb_calc_sum(T_U32 *start_addr, T_U32 *end_addr)
{
    T_U32 sum=0;
    T_U32 *pi;
    
    /* the fixed data add together, but just add some datas, not all */
    //for (pi=start_addr; pi<=end_addr; pi++)
    for (pi = start_addr;pi < (T_U32 *)end_addr; pi += 15)
    {
        /* do not add data in phy2virTab and resv_pg_tbl */
        if (remap_data_notin_remap(pi)) //不包括remap结构体变量
            continue;    
            
        else if (pi < (T_U32 *)prog_manage.ram2store_tbl || pi > (T_U32 *)(&prog_manage.protect))
        {
            sum ^= *pi;
        }
         
    }
    //sum = DEEP_MAX_U32 - sum + 1;
    sum = ~sum + 1;
#ifdef DEEP_DEBUG
    akerror(s_cDeep_sum, sum, AK_TRUE);
#endif
    return sum;
}

/******************************************************************************
 * @brief   释放非常驻236K的映射关系，将dirty的数据回写到nandflash上
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_VOID
 * @retval  T_VOID
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform.     
 ******************************************************************************/
T_VOID deepstdb_unmap(T_VOID)
{
    T_U32 index;
    T_U32 virAddr;
    T_U32 phIdx;
    
    for (index = 0; index < (REMAP_MEM_PSIZE/VPAGE_SIZE); index++)
    {
        virAddr = remap_get_phy2virTab(index);
        if (0 != virAddr)
        {
            if(remap_page_is_dirtyone(index))
            {
                if (!(progmanage_writeback(virAddr, VPAGE_SIZE)))
                {                
                    akerror(s_cDeep_W16, 0, AK_TRUE);
                    while(1);            
                }
            }
            /* after flush back, set the physical page clean */
            remap_set_phy2virTab(index,0);
            phIdx = progmanage_PreUnmapRam2Store(index, virAddr);
            MMU_MapPage(virAddr, virAddr+VPAGE_SIZE, (phIdx<<VPAGE_BITS), FB_SMALL);
            //prog_manage.ram2store_tbl[index]=0;
            remap_set_page_cleanone(index);  
            /* invalidate page */   
        }
    }
    
}

/******************************************************************************
 * @brief   select standby block, save resident data after calculating sum, then shut down GPIO
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_VOID
 * @retval  T_VOID
 * @MENDER        lisichun
 * @AMEND DATE    2012-09-12
 * @BRIEF:  modify the function accord with 11 view platform     
 ******************************************************************************/
T_VOID deepstdb_hang(T_VOID)
{
    T_U32 cnt = DEEP_RESIDENT_SAVE_PAGES;
    T_U32 spare = 0;
    T_U32 start_page;
    T_U32 block;
    T_U16 page;
    T_U16 page_offst;
    T_U8 *addr;

    /* suspend if parameter is wrong */
    if (cnt > (g_nPagePerBlock_asa- 1))
    {
        akerror(s_cDeep_W17, cnt, AK_TRUE);
        while(1);
    }
    addr = (T_U8 *)remap_alloc(VPAGE_SIZE);
    page = DEEP_NAND_FLAG_PAGE - cnt + 1; //第一个保存数据页
    progmanage_set_protect(AK_FALSE);

    /* select two standby blocks until successful. */
    stdb_blk_save.primary = 0;    //clean var saving in the used standby block
    stdb_blk_save.backup = 0;
    while (stdb_blk_save.backup == 0)
    {
        /* 处理倒数第三页未写序号值的特殊情况：
            1.当前块空余页数不足保存数据，且倒数第三页未写序号值；
            2.当前块空余页数足够保存数据但数据未超过1页，且倒数第三页未写序号值。*/
        if ((prog_manage.swap.page_offst > page && prog_manage.swap.page_offst < DEEP_NAND_FLAG_PAGE)
         || (prog_manage.swap.page_offst < page && cnt == 1))
        {
            /* write counter to the spare. */
            start_page = (prog_manage.swap.block << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            spare = s_ulBlkCount;
            while (0 != nand_remap_write(start_page, addr, spare, AK_FALSE))
            {
                /* do replacement and mark as bad block */
                block = progmanage_next_valid_blk(prog_manage.swap.block, 1);
                deepstdb_transfer_data(prog_manage.swap.block, block, prog_manage.swap.page_offst);  
               
                prog_manage.swap.block = progmanage_next_valid_blk(prog_manage.swap.block, 1);

                if (++prog_manage.M == NAND_SWAP_RESV_BLOCK)
                {
                    akerror(s_cDeep_W14, NAND_SWAP_RESV_BLOCK-prog_manage.M, AK_TRUE);
                    while(1);   //maybe not best solution?
                }

                start_page = (prog_manage.swap.block << prog_manage.mbit) + DEEP_NAND_FLAG_PAGE-1;
            }
            prog_manage.swap.page_offst = DEEP_NAND_FLAG_PAGE;
        }
        
        /* get standby block when empty pages is enough*/
        if (prog_manage.swap.page_offst <= page)
        {
            /* get primary standby block */
            if (stdb_blk_save.primary == 0)
            {
                page_offst = prog_manage.swap.page_offst;
                stdb_blk_save.primary = prog_manage.swap.block;
            }
            /* get backup standby block */
            else if (stdb_blk_save.backup == 0)
            {
                stdb_blk_save.backup = prog_manage.swap.block;
            }
        }
        
         progmanage_request_new_block();
    }

    progmanage_set_protect(AK_TRUE);
    remap_free(addr);

    /* calculate check-sum of all fixed data */
    s_ulDeepSum = 0;
    s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start, (T_U32 *)fixed_data_end);
    
    /* save datas to primary standby block. 
       mark_bad_block if wrong, and modify standby block */
    deepstdb_save_fixdata(stdb_blk_save.primary, cnt, page_offst);

    /* save data to backup standby block, request again if write wrong. */
    deepstdb_save_fixdata(stdb_blk_save.backup, cnt, page_offst);
    
    akerror(s_cDeep_sum, s_ulDeepSum, AK_TRUE);
    akerror(s_cDeep_blk, stdb_blk_save.backup, AK_FALSE);
    akerror(s_cDeep_max, s_ulBlkCount-1, AK_TRUE);
    akerror(s_cDeep_poweroff, 0, AK_TRUE);
    
    if(power_off_callback != AK_NULL)
    {
        power_off_callback();
    }
    /* power off after shut down one GPIO */
}
#pragma arm section code
#endif  //DEEP_STANDBY
#endif

#if (STORAGE_USED == SPI_FLASH)
#ifdef DEEP_STANDBY
#pragma arm section code ="_deepstdb_"
extern T_U32 Fwl_spiflash_blocknum(T_VOID);

/******************************************************************************
 * @brief   save the fixed data to the deep standby block in nandflash
 * @author  lisichun
 * @date    2012-09-12
 * @param   [in]T_U32 start_page: the start page of SPI flash to save the fixed data.             
 * @retval  T_VOID
 ******************************************************************************/
T_BOOL deepstdb_check_blocks(T_VOID)
{
	T_U32 temp_block;	
    T_U32 temp_page;
	T_U32 len;
	T_U32 total_block;
	
	len = progmanage_get_mapdata(DEEP_FILE, tempBuffer);
	if (0 == len)
	{	
		AK_PRINTK("can not find backup file", 0, AK_TRUE);
		return AK_FALSE;
	}

	temp_page = *(T_U16 *)tempBuffer;	
	temp_block = (temp_page - 1) /SPIFLASH_PAGE_PER_BLOCK +1;
	
	total_block = Fwl_spiflash_blocknum();
    if( temp_block < (total_block -3))
		return AK_TRUE;
	else
		return AK_FALSE;

}

T_VOID deepstdb_save_fixdata(T_U32 start_page)
{
  T_U32 index;
  T_U32 temp_page;
  

  for (index = 0; index < DEEP_FIXDATA_PAGES_SPI; index++)
    { 
       memcpy(tempBuffer, (T_U8 *)(fixed_data_start_spi+(index<< prog_manage.nbit)), VPAGE_SIZE);	   
       temp_page = start_page+16*index; 
       if(AK_FALSE == Fwl_spiflash_write(temp_page, tempBuffer, VPAGE_SIZE/SPIFLASH_PAGE_SIZE))
       {  
          AK_DEBUG_OUTPUT("spi write false:%d", VPAGE_SIZE/SPIFLASH_PAGE_SIZE);
          while(1);
       }
    }
}

/******************************************************************************
 * @brief   释放非常驻236K的映射关系，将dirty和resv的数据回写到SPI flash上
 * @author  lisichun
 * @date    2012-09-12
 * @param   T_VOID
 * @retval  T_VOID
 ******************************************************************************/

T_VOID deepstdb_unmap(T_VOID)
{
    T_U32 index;
    T_U32 virAddr;
    T_U32 temp_block;
	T_U32 total_block;
    T_U32 temp_page;
    T_U32 flag;
	
    tempBuffer = (T_U8 *)remap_alloc(VPAGE_SIZE);

	if (AK_NULL == tempBuffer)
	{
		akerror("m f", 0, AK_TRUE);
	}

	
	total_block= Fwl_spiflash_blocknum();
	
	if(!deepstdb_check_blocks())
	{
	  remap_free(tempBuffer);
	  akerror("NO spare to deepstandby", 0,AK_TRUE);	
	  return;
	}
    //erase all of blocks
    for(temp_block = total_block - 3; temp_block < total_block; temp_block++)
    {
       if(AK_FALSE == Fwl_spiflash_erase(temp_block))          
         {  
            akerror("spi erase false=", temp_block,AK_TRUE);
             while(1);
          }
     } 
	
    //write back dirty and resv pages to flash
    for (index = 0; index < (REMAP_MEM_PSIZE/VPAGE_SIZE); index++)
    { 
       virAddr = remap_get_phy2virTab(index);
       if (0 != virAddr)
       { 
           if ((T_U32)tempBuffer == virAddr)
           {
                tempIndex = index;
                continue;
           }
         
           if( remap_page_is_dirtyone(index) || remap_page_is_resv(index) )
           { 
             #ifdef DEEP_DEBUG
             akerror(s_cDeep_index, index, AK_FALSE);
             akerror(s_cDeep_vaddr, virAddr, AK_TRUE);
             #endif
             remap_set_phy2virTab(index,(virAddr+(T_U32)deep_save_flag));
             memcpy(tempBuffer,(T_U8 *)virAddr, VPAGE_SIZE);
             temp_page = (total_block-3)*SPIFLASH_PAGE_PER_BLOCK + 16*index;
			 if(AK_FALSE == Fwl_spiflash_write(temp_page, tempBuffer, VPAGE_SIZE/SPIFLASH_PAGE_SIZE))
              {  
                 akerror("spi write false=", VPAGE_SIZE/SPIFLASH_PAGE_SIZE,AK_TRUE);
                 while(1);
              }
               continue;
            }
         
            remap_set_phy2virTab(index,0);  
           //MMU_MapPage(virAddr, virAddr+VPAGE_SIZE, 0, FB_SMALL);
            remap_set_page_cleanone(index); 
        } 
    }

   //calc sum
   s_ulDeepSum = 0;
  
   //s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start, (T_U32 *)fixed_data_end); 
   s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start_spi, (T_U32 *)fixed_data_end);
   
   // save the fixdata
   temp_page = total_block*SPIFLASH_PAGE_PER_BLOCK-(DEEP_FIXDATA_PAGES_SPI+1)*16-2;

   deepstdb_save_fixdata(temp_page);
  
  //write back deep standby flag
   temp_page = total_block*SPIFLASH_PAGE_PER_BLOCK-1;;
   flag = (T_U32)DEEP_FLAG_BLK_PRIMARY;
   memcpy(tempBuffer, ((T_U8*)(&flag)), sizeof(T_U32));
  
   if(AK_FALSE == Fwl_spiflash_write(temp_page, tempBuffer,1))
    {  
       AK_DEBUG_OUTPUT("spi write false:%d", VPAGE_SIZE/SPIFLASH_PAGE_SIZE);
       while(1);
    } 
}

/******************************************************************************
 * @brief   select standby block, save fixed data after calculating sum, then shut down GPIO
 * @author  lisichun
 * @date    2012-09-12
 * @param   T_VOID
 * @retval  T_VOID
 ******************************************************************************/

T_VOID deepstdb_hang(T_VOID)
{   
    akerror(s_cDeep_poweroff, 0, AK_TRUE);
    if(power_off_callback != AK_NULL)
    {
        power_off_callback();
    }
}

/******************************************************************************
 * @brief   the callback function of deep standby.
 * @author  luojianhua
 * @date    2009-08-25
 * @param   T_fPOWEROFF_CALLBACK callback_func:the address of the callback function.
 * @return  NONE
 ******************************************************************************/

T_VOID deepstdb_set_callback(T_fPOWEROFF_CALLBACK callback_func)
{
    power_off_callback = callback_func;
}

/******************************************************************************
 * @brief   judge deep standby mode via reading flag in the flash.
 * @author  lisichun
 * @date    2012-09-12
 * @param   NONE
 * @retval  T_BOOL: AK_TRUE if deep standby mode, otherwise return AK_FALSE
 ******************************************************************************/

static T_BOOL deepstdb_check(T_VOID )
{
  T_U32 flag;
  T_U32 total_block;
  T_U32 temp_page;
  T_U8 * pData;
  
  pData = (T_U8 *)remap_alloc(VPAGE_SIZE); 
  if (AK_NULL == pData)
  {
	akerror("pData= N", 0, AK_TRUE);
  }
  total_block= Fwl_spiflash_blocknum();
  temp_page = total_block*SPIFLASH_PAGE_PER_BLOCK-1;;

   if(AK_FALSE == Fwl_spiflash_read(temp_page, pData, 1))
   {  
   //     AK_DEBUG_OUTPUT("spi read false:%d", VPAGE_SIZE/SPIFLASH_PAGE_SIZE);
        while(1);
   }
   memcpy((T_U8 *)(&flag), pData, sizeof(T_U32));
   akerror("Df=", flag, AK_TRUE);

   if( DEEP_FLAG_BLK_PRIMARY == flag )//避免deepstdby失败时，无法正常启动。
   {  	    
   		flag = 0;             //清除标志位
    	memcpy(pData, (T_U8 *)&flag, sizeof(T_U32)); 
    	Fwl_spiflash_write(temp_page, pData, 1);
		remap_free(pData);
    	return AK_TRUE;
   }
   else
   {
   	  remap_free(pData);
      return AK_FALSE;
   }
}

/******************************************************************************
 * @brief check deep standby flag, load saved data, and resume cpu status.
          finally, go back to the original place.
 * @author  lisichun
 * @date    2012-09-12
 * @param T_VOID
 * @retval T_VOID
******************************************************************************/
T_VOID deepstdb_handler(T_VOID)
{
  T_U32 total_block;
  T_U32 i;
  T_U32 virAddr;
  T_U32 flag;
  T_BOOL ret = AK_TRUE;
  T_U8 *pData = AK_NULL;
 // RTC_WatchDog_Output_level(1); 

  if (deepstdb_check())    
    {
        // load fixed data
        if (deepstdb_load())  
         {
			total_block = Fwl_spiflash_blocknum();
			
            if(AK_FALSE == Fwl_spiflash_erase(total_block-1))   //2s?           
            {
              ret = AK_FALSE;
             // akerror("spi erase false=", ret,AK_TRUE);
              while(1);
             }
            //write back DEEP_FLAG_CANCEL
            
            if (ret)         
            {
                pData = (T_U8 *)remap_alloc(VPAGE_SIZE);
                flag = (T_U32)DEEP_FLAG_CANCEL;
                memcpy(pData,((T_U8*)(&flag)), sizeof(T_U32));
                Fwl_spiflash_write((T_U32)total_block*SPIFLASH_PAGE_PER_BLOCK-1, pData,1);
                remap_free(pData);	
                deepstdb_resume();
            }
         }
        //loading fail sys reset
        if(AK_FALSE == Fwl_spiflash_erase(total_block-1))            
          {
             ret = AK_FALSE;
          //   akerror("spi erase false=", ret,AK_TRUE);
             while(1);
           }
        Fwl_SysReboot();
    }

   /* clear table of phy2virTab and resv_pg_tbl */
   for (i = 0; i < REMAP_MEM_PSIZE/VPAGE_SIZE; i++)
    {
         virAddr = remap_get_phy2virTab(i);
         if (virAddr != 0)
          {
             remap_set_phy2virTab(i,0);  
             MMU_MapPage(virAddr, virAddr+VPAGE_SIZE, (REMAP_PADDR_START+(i<<VPAGE_BITS)), FB_SMALL);
             remap_set_page_cleanone(i);
          }     
    }
}

#pragma arm section code

#pragma arm section code = "_bootcode3_"
/******************************************************************************
  * @brief   the calculate sum of the fixed data. 
  * @author  luojianhua
  * @date    2009-08-25
  * @param   T_U32 *start_addr:the start address of the fixed data.
             T_U32 *end_addr:the end address of the fixed data
  * @retval  the calculate sum of the fixed data.
  * @MENDER        lisichun
  * @AMEND DATE    2012-09-12
  * @BRIEF:  add the address of (0x08009000~0x0800A000) fixdata.     
  ******************************************************************************/

static T_U32 deepstdb_calc_sum(T_U32 *start_addr, T_U32 *end_addr)
{
    T_U32 sum=0;
    T_U32 *pi;
    
  //  for (pi = start_addr; pi < (T_U32 *)resident_data_end; pi += 16)
  for (pi = start_addr; pi < end_addr; pi += 16)
    {
        
        /*if(pi > end_addr && (pi-16)<=end_addr)
            pi = (T_U32 *) MMU_PT_end;*/
            
        if (remap_data_notin_remap(pi))
            {
              continue;  
            }
        else if (pi < (T_U32 *)(&prog_manage.mem2store_phstart) || pi > (T_U32 *)(&prog_manage.protect))
         {
          
           sum ^= *pi;
         // sum += *pi;
         }
    }
    sum = ~sum + 1;
#ifdef DEEP_DEBUG
    akerror(ERROR_PROG, sum, AK_TRUE);
#endif

    return sum;
}
//#pragma arm section code

//#pragma arm section code = "_deepstdb_"

/******************************************************************************
 * @brief   load fixed data, verify block and sum, then write cancel flag.
 * @author  lisichun
 * @date    2012-09-12
 * @param   NONE
 * @retval  T_BOOL: AK_TRUE if all is successful, otherwise return AK_FALSE  
 * @BRIEF:  里面绝对不充许倒换，包括打印信息。
******************************************************************************/

static T_BOOL deepstdb_load(T_VOID )
{
    T_U32 temp;
    T_U32 index;
    T_U32 virAddr; 
    T_U8 *pData;
    T_U32 idxPData;
	T_U32 block;
    T_BOOL ret = AK_TRUE;
//	temp = get_pc();
 //  check_ptr(temp + VPAGE_SIZE);

	
    block = Fwl_spiflash_blocknum();
    pData = (T_U8 *)remap_alloc(VPAGE_SIZE);
   // akerror("pD:",pData,1);

    ////=================禁止倒换==========================
    //  循环清除全部phy2virTab已影射的mmu，但不包含pData影射的mmu。
    for (index = 0; index < (REMAP_MEM_PSIZE/VPAGE_SIZE); index++)
    {
        virAddr = remap_get_phy2virTab(index);
        if ((T_U32)pData == virAddr)
        {
            idxPData = index;
            continue;
        }
        if (0 != virAddr)
        {
           remap_set_phy2virTab(index,0);  
           MMU_MapPage(virAddr, virAddr+VPAGE_SIZE, 0, FB_SMALL);
           remap_set_page_cleanone(index);
        }
    }
	
	m_ulDeepSP = 0;

    //  循环加载zidata
    for (index = 0;index < DEEP_FIXDATA_PAGES_SPI; index++)
    {
         temp = block*SPIFLASH_PAGE_PER_BLOCK - (DEEP_FIXDATA_PAGES_SPI+1-index)*16 -2;
        
		 if(AK_FALSE == Fwl_spiflash_read(temp, pData, VPAGE_SIZE/SPIFLASH_PAGE_SIZE))
         {  
             //akerror("SF1", VPAGE_SIZE/SPIFLASH_PAGE_SIZE,AK_TRUE);
             while(1);
         }
		 
		 if( index == DEEP_FIXDATA_PAGES_SPI -1 )
		 {      
		   	 memcpy((T_U8 *)(fixed_data_start_spi+(index << prog_manage.nbit)), pData, (fixed_data_end - fixed_data_start_spi)%VPAGE_SIZE);
		 }
     	 else 
		 {
        	 memcpy((T_U8 *)(fixed_data_start_spi+(index << prog_manage.nbit)), pData, VPAGE_SIZE);
		 }
		 //   remap_set_page_dirty(index);
    }

    temp = s_ulDeepSum; 
    s_ulDeepSum = 0;
	
	s_ulDeepSum = deepstdb_calc_sum((T_U32 *)fixed_data_start_spi, (T_U32 *)fixed_data_end);

	if ((T_U32)s_ulDeepSum != temp)
    {
        ret = AK_FALSE;
        return AK_FALSE;
    }
    
   //  清除pData buffer ,设置tempBuffer 的 mmu
    if ((tempBuffer != pData) || (tempIndex != idxPData))
    {
        MMU_MapPage(pData, pData+VPAGE_SIZE, 0, FB_SMALL);
        MMU_MapPage(tempBuffer, tempBuffer+VPAGE_SIZE, REMAP_PADDR_START+(tempIndex<<VPAGE_BITS), RW_SMALL);
        pData = tempBuffer;
		
    }

    //  根据load的phy2virTab,循环更新mmu
    for (index = 0; index < (REMAP_MEM_PSIZE/VPAGE_SIZE); index++)
    {
        virAddr = remap_get_phy2virTab(index);
      
        if ((T_U32)tempBuffer == virAddr)
        {
            continue;
        }
        if (virAddr & (T_U32)(deep_save_flag))
        {
            #ifdef DEEP_DEBUG
            akerror(s_cDeep_index, index, AK_FALSE);
            akerror(s_cDeep_vaddr, virAddr, AK_TRUE);
            #endif
            virAddr = virAddr - (T_U32)deep_save_flag;
            remap_set_phy2virTab(index,virAddr);
            MMU_MapPage(virAddr, virAddr+VPAGE_SIZE, REMAP_PADDR_START+(index<<VPAGE_BITS), RW_SMALL);

			temp= (block-3)*SPIFLASH_PAGE_PER_BLOCK+16*index;
			
            if(AK_FALSE == Fwl_spiflash_read(temp, pData, VPAGE_SIZE/SPIFLASH_PAGE_SIZE))
            {  
              //  akerror("SF2", VPAGE_SIZE/SPIFLASH_PAGE_SIZE,AK_TRUE);
                while(1);
            }
            memcpy((T_U8 *)virAddr, pData, VPAGE_SIZE);
			// remap_set_page_dirty(index);
        }
        else if(virAddr)
        {
          remap_set_phy2virTab(index,0);
        }
    }
   
    remap_free(pData);
    if (ret)
    {
        return AK_TRUE;
    }
    else
        return AK_FALSE;
}

#pragma arm section code
#endif  //DEEP_STANDBY
#endif

