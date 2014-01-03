#ifndef _BURNER_H_
#define _BURNER_H_
#include "updateself.h" 
typedef enum
{
    CHIP_3224, //AK_3224
    CHIP_322L, //AK_322L
    CHIP_36XX, //Sundance
    CHIP_780X, //Aspen
    CHIP_880X, //Aspen2
    CHIP_10X6, //Snowbird,snowbirdsA~D
    CHIP_3631, //Sundance2
    CHIP_3671, //Sundance2A
	CHIP_980X,	//aspen3s
	CHIP_3671A,	//sundance2a V6
	CHIP_1080A,	//snowbirdsE
	CHIP_37XX,  //sundance3
	CHIP_11XX,  //AK11
	CHIP_1080L,	//snowbirdL
	CHIP_10XXC,	//10c
	CHIP_39XX,  //AK39
    CHIP_RESERVER,
}E_CHIP_TYPE;

typedef struct NandBinParam
{
    T_U32   data_length;                //bin file length
    T_U32   ld_addr;                        //bin link address
    T_U8    file_name[16];                //bin file name in system
    T_BOOL  bUpdateself;                //whether support update self
    T_BOOL  bCompare;                        //whether compare
}T_BIN_PARAM;

typedef struct NandBinInfo
{
    T_U32   data_length;
    T_U32   ld_addr;
    T_BOOL  bBackup;
    T_U8    file_name[16];
}T_BIN_INFO;

typedef struct
{
    T_U32 reg_addr;                        //ram address
    T_U32 reg_value;                //ram address config value
}T_RAM_REG;

typedef struct
{
    E_CHIP_TYPE chip_type;  //chip type
    T_U32 nRamRegNum;                //ram num
    T_RAM_REG* pRamReg;                //ram config buffer
}T_BOOT_PARAM;

extern T_PRODCALLBACK  gpf;
T_U32 Burner_WriteSpiBinFile(T_hFILE hFile, T_BIN_PARAM*  binParam, E_CHIP_TYPE chip_type);
T_U32 Burner_WriteSpiBootFile(T_hFILE hFile, T_U32 dataLen, T_BOOL bCompare, T_BOOT_PARAM* pBootParam, E_CHIP_TYPE chip_type);

#endif
