/**
 * @file svc_medialist.h
 * @brief ANYKA software
 * 
 * @author 
 * @date  
 * @version 1,0 
 */


#ifndef _SVC_MEDIALIST_H_
#define _SVC_MEDIALIST_H_


#include "anyka_types.h"
#include "Fwl_osmalloc.h"
#include "Fwl_osfs.h"


#define MAX_MEDIA_NUM 			(1000)
#define MAX_FOLDER_NUM 			(300)

#define MEDIA_PATH_SIZE			(MAX_FILE_LEN<<1)
#define MEDIA_LISTNAME_SIZE		(20)


#define PLAYLIST_MAX_DEEP       (6)
#define LIST_LOAD_END_FLAG      (0X55AA)

#define LIST_FIND_DIR_NEXT		(1)
#define LIST_FIND_DIR_CUR		(0)
#define LIST_FIND_DIR_PRE		(-1)



/**
* @brief media list init
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_pCWSTR listname : listname 
* @param in T_pCWSTR searchpath : the highest path for search. 
* @param in T_BOOL bDividFolders : divide  folders or not. 
* @param in T_U16* fileFormat : file format for search. 
* @param in T_U16* skipFolder : folder name for skip. 
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Init(T_pCWSTR listname, T_pCWSTR searchpath, 
					T_BOOL bDividFolders, T_U16* fileFormat, T_U16* skipFolder);


/**
* @brief Destroy Media List 
*
* @author Songmengxing
* @date 2013-05-27
*
* @param T_VOID
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Destroy(T_VOID);


/**
* @brief have item or not
*
* @author Songmengxing
* @date 2013-05-27
*
* @param T_VOID
*
* @return T_BOOL 
* @retval 
*/
T_BOOL MList_IsHaveItem(T_VOID);




/**
* @brief Get item Path
*
* @author Songmengxing
* @date 2013-05-27
*
* @param out T_pWSTR path : Path 
* @param in T_S8 dir : -1, previous; 0, current; 1, next
* @param in T_BOOL bCyc : cycle or not
*
* @return T_BOOL 
* @retval 
*/
T_BOOL MList_GetItem(T_pWSTR path, T_S8 dir, T_BOOL bCyc);


/**
* @brief change folder
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_S8 dir : -1, previous; 0, current; 1, next
*
* @return T_BOOL
* @retval 
*/
T_BOOL MList_ChangeFolder(T_S8 dir);


/**
* @brief get load flag
*
* @author Songmengxing
* @date 2013-05-27
*
* @param T_VOID
*
* @return T_U16
* @retval load flag
*/
T_U16 MList_GetLoadFlag(T_VOID);


/**
* @brief delete media list file
*
* @author Songmengxing
* @date 2013-05-27
*
* @param in T_pCWSTR listname : list name
* @param in T_pCWSTR searchpath : the highest path. 
*
* @return T_BOOL
* @retval
*/
T_BOOL MList_DelList(T_pCWSTR listname, T_pCWSTR searchpath);



#endif	//_SVC_MEDIALIST_H_

