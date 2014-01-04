/**
 * @file i2c.h
 * @brief I2C interface driver header file
 * This file provides I2C APIs: I2C initialization, write to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#ifndef __I2C_H__
#define __I2C_H__


#include "anyka_types.h"

extern T_U32 i2c_sclk;
extern T_U32 i2c_sda;

#define  GPIO_I2C_SCLK              i2c_sclk
#define  GPIO_I2C_SDA               i2c_sda


/**
 * @brief write data to I2C device
 * write size length data to dab's rab register
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U8 dab: I2C device address
 * @param T_U8 rab: register address
 * @param T_U8 *data: write data's point
 * @param T_U8 size: write data's length
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_write_data(T_U8 dab, T_U8 *data, T_U8 size);

/**
 * @brief read data from I2C device function
 * read data from dab's rab register
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U8 dab: I2C device address
 * @param T_U8 rab: register address
 * @param T_U8 *data: read output data store address
 * @param T_U8 size: read data size
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_read_data(T_U8 dab, T_U8 *data, T_U8 size);

//other way 
T_BOOL i2c_read_data_extend(T_U8 dab, T_U8 *data, T_U8 size);

/**
 * @brief write data to I2C device which has no address
 * @author ChenWeiwen
 * @date 2008-07-01
 * @param T_U8 *data: write data's point
 * @param T_U8 size: write data's length
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_write_rtc_data(T_U8 *data, T_U8 size);


/**
 * @brief read data from I2C device which has no address
 * read data from dab's rab register
 * @author ChenWeiwen
 * @date 2008-07-01
 * @param T_U8 *data: read output data store address
 * @param T_U8 size: read data size
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_read_rtc_data(T_U8 *data, T_U8 size);


/**
 * @brief write data to RTC device HT1381
 * @author ChenWeiwen
 * @date 2008-07-01
 * @param T_U8 *data: write data's point
 * @param T_U8 size: write data's length
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_write_rtc_data_ex(T_U8 *data, T_U8 size);


/**
 * @brief read data from RTC device HT1381
 * read data from dab's rab register
 * @author ChenWeiwen
 * @date 2008-07-01
 * @param T_U8 *data: read output data store address
 * @param T_U8 size: read data size
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_read_rtc_data_ex(T_U8 *data, T_U8 size);


#endif  /* __I2C_H__ */
