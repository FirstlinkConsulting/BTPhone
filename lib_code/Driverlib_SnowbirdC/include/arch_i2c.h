/**
 * @file arch_i2c.h
 * @brief I2C interface driver header file
 * This file provides I2C APIs: I2C initialization, write to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#ifndef __ARCH_I2C_H__
#define __ARCH_I2C_H__


#include "anyka_types.h"


/** @defgroup I2C_Interface I2C group
 *  @ingroup Drv_Lib
 */

/*@{*/
/**
 * @brief I2C interface initialization function
 *
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] pin_scl the pin assigned to SCL
 * @param[in] pin_sda the pin assigned to SDA
 * @return T_VOID
 */
T_VOID i2c_initial(T_U32 pin_scl, T_U32 pin_sda);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr and data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_write_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr is word width, data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_write_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr and data is word width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_write_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size);

/**
 * @brief write data to I2C device
 *
 * write size length data to daddr's raddr register, raddr is not required, data is byte width
 * @author guoshaofeng
 * @date 2008-03-10
 * @param[in] daddr I2C device address
 * @param[in] *data write data's pointer
 * @param[in] size write data's length
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_write_data4(T_U8 daddr, T_U8 *data, T_U32 size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr and data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_read_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr is word width, data is byte width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] *data read output data store address
 * @param[in] size read data size
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_read_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr and data is word width
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param[in] daddr I2C device address
 * @param[in] raddr register address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_read_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size);

/**
 * @brief read data from I2C device function
 *
 * read data from daddr's raddr register, raddr is not required, data is byte width
 * @author guoshaofeng
 * @date 2008-03-10
 * @param[in] daddr I2C device address
 * @param[out] data read output data store address
 * @param[in] size read data size
 * @return T_BOOL return operation successful or failed
 * @retval AK_FALSE operation failed
 * @retval AK_TRUE operation successful
 */
T_BOOL i2c_read_data4(T_U8 daddr, T_U8 *data, T_U32 size);
/*@}*/


#endif  /* __ARCH_I2C_H__ */

