
#ifndef __CODE_PAGE_H
#define __CODE_PAGE_H
#include "anyka_types.h"


#define CODEPAGE_DEFCHAR             CodePage_GetDefChar()
#define CODEPAGE_DEFUNICHAR          CodePage_GetDefUniChar()
#define CODEPAGE_CP2UNI(var)         CodePage_GetCp2UniMember((var))
#define CODEPAGE_UNI2CPLOW(var)      CodePage_GetUni2CpLowMember((var))
#define CODEPAGE_UNI2CPHIGH(var)     CodePage_GetUni2CpHighMember((var))
#define CODEPAGE_CP2UNILEADBYTE(var) CodePage_GetCp2UniLeadbytesMember((var))
#define CODEPAGE_UNICOMPOSE(var)     CodePage_GetUniComposeMember((var))
#define CODEPAGE_UNIDECOMPOSE(var)   CodePage_GetUniDecomposeMember((var))
#define CODEPAGE_COMPOSESIZE         CodePage_GetUniComposeTableSize()

#define CODEPAGE_BLOCK_USED                     8     //nlock used 
#define CODEPAGE_NAME                           "codepage" 
#define CODEPAGE_PATH                           "W:/codepage.bin"

#define CODEPAGE_COMMONBUFFSIZE                     1024
typedef enum _CODEPAGE_ {
    CP_037 = 0,
    CP_424,
    CP_437,
    CP_500,
    CP_737,
    CP_775,
    CP_850,
    CP_852,
    CP_855,
    CP_856,
    CP_857,
    CP_860,
    CP_861,
    CP_862,
    CP_863,
    CP_864,
    CP_865,
    CP_866,
    CP_869,
    CP_874,
    CP_875,
    CP_878,
    CP_932,
    CP_936,
    CP_949,
    CP_950,
    CP_1006,
    CP_1026,
    CP_1250,
    CP_1251,
    CP_1252,
    CP_1253,
    CP_1254,
    CP_1255,
    CP_1256,
    CP_1257,
    CP_1258,
    CP_10000,
    CP_10006,
    CP_10007,
    CP_10029,
    CP_10079,
    CP_10081,
    CP_20127,
    CP_20866,
    CP_20932,
    CP_21866,
    CP_28591,
    CP_28592,
    CP_28593,
    CP_28594,
    CP_28595,
    CP_28596,
    CP_28597,
    CP_28598,
    CP_28599,
    CP_28600,
    CP_28603,
    CP_28604,
    CP_28605,
    CP_28606,
    CP_UTF8_CP,
    CP_UNICODE,
    CODE_PAGE_NUM
} T_CODE_PAGE;

/******************************************************************************
 * @NAME    CodePage init
 * @BRIEF   init Codepage global variable
 * @AUTHOR  xuping
 * @DATE    2008-04-28
 * @PARAM 
 *		    T_VOID
 * @RETURN T_VOID
 *	
*******************************************************************************/
T_BOOL CodePage_init(T_CODE_PAGE codepage);

/******************************************************************************
 * @NAME    CodePage_Set
 * @BRIEF   set codepage 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   code_page:codepage to set
 *		    T_VOID
 * @RETURN AK_TRUE:set success
 *	netos: 
*******************************************************************************/
T_BOOL CodePage_Set(T_CODE_PAGE code_page);

/******************************************************************************
 * @NAME    CodePage_GetDefChar
 * @BRIEF   Get default char of the codepage
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetDefChar(T_VOID);
/******************************************************************************
 * @NAME    CodePage_GetDefUniChar
 * @BRIEF   Get default unicode char of the codepage
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetDefUniChar(T_VOID);

/******************************************************************************
 * @NAME    CodePage_GetCp2UniMember
 * @BRIEF   get cp2unicode array member 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetCp2UniMember(T_U32 offset);

/******************************************************************************
 * @NAME    CodePage_GetCp2UniLeadbytesMember
 * @BRIEF   get cp2uni_leadbytes array member
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U8 CodePage_GetCp2UniLeadbytesMember(T_U32 offset);

/******************************************************************************
 * @NAME    CodePage_GetUni2CpLowMember
 * @BRIEF   get uni2cp_low array member 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetUni2CpLowMember(T_U32 offset);

/******************************************************************************
 * @NAME   CodePage_GetUni2CpHighMember
 * @BRIEF   get uni2cp_High array member 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetUni2CpHighMember(T_U32 offset);

/******************************************************************************
 * @NAME    CodePage_GetUniComposeMember
 * @BRIEF   get compose array member 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetUniComposeMember(T_U32 offset);   

/******************************************************************************
 * @NAME    CodePage_GetUniDecomposeMember
 * @BRIEF   get decompose array member 
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U16 CodePage_GetUniDecomposeMember(T_U32 offset);

/******************************************************************************
 * @NAME    CodePage_GetUniComposeTableSize
 * @BRIEF   get compose table size
 * @AUTHOR  xuping
 * @DATE    2008-04-29
 * @PARAM   arroy offset
 * @RETURN  array member
 *	netos: 
*******************************************************************************/
T_U8 CodePage_GetUniComposeTableSize(T_VOID);

/******************************************************************************
 * @NAME    CodePage_GetInitFlag
 * @BRIEF   get the codepage init flag
 * @AUTHOR  xuping
 * @DATE    2008-04-30
 * @PARAM   T_VOID
 * @RETURN  init flag
*******************************************************************************/
T_BOOL CodePage_GetInitFlag(T_VOID);

/******************************************************************************
 * @NAME    CodePage_GetCharSize
 * @BRIEF   get the charsize of the current language
 * @AUTHOR  xuping
 * @DATE    2008-04-18
 * @PARAM   T_VOID
 * @RETURN  char size
*******************************************************************************/
T_U8 CodePage_GetCharSize(T_VOID);

/******************************************************************************
 * @NAME    CodePage_GetVersion
 * @BRIEF   get the unicode lib version
 * @AUTHOR  
 * @DATE    2013-07-25
 * @PARAM   T_VOID
 * @RETURN  char *
*******************************************************************************/
T_pSTR CodePage_GetVersion(T_VOID);

#endif

