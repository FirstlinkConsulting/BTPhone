/*******************************************************************************
 * @file    arch_efuse.h
 * @brief   read and burn Vendor ID and Serial ID
 * Copyright (C) 2013 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2013-08-09
 * @version 1.0
*******************************************************************************/ 
#ifndef __EFUSE_BURN_H__
#define __EFUSE_BURN_H__


#include <anyka_types.h>


#define EFUSE_LOCK_BIT          0xff

#define EFUSE_READ_VENDOR       0
#define EFUSE_READ_SERIAL       1

#define EFUSE_BURN_OK           0
#define EFUSE_BURN_ERR          1
#define EFUSE_BURN_LOCK         2
#define EFUSE_BURN_FAULT        3


/*******************************************************************************
 * @brief   read vendor id or serial id
 * @author  LiuHuadong
 * @date    2012-03-04
 * @param   [in]id: vendor id or serial id
 * @return  T_U32
 * @retval  the value of vendor id or serial id
*******************************************************************************/ 
T_U32 efuse_read(T_U8 id);


/*******************************************************************************
 * @brief   burn serial id
 * @author  LiuHuadong
 * @date    2012-03-04
 * @param   [in]bits: [0~31¡¢255]
 * @return  T_U8
 * @retval  EFUSE_BURN_OK:
 * @retval  EFUSE_BURN_ERR:
 * @retval  EFUSE_BURN_LOCK:
 * @retval  EFUSE_BURN_FAULT:
*******************************************************************************/ 
T_U8 efuse_burn(T_U8 bits);


#endif  //__EFUSE_BURN_H__


