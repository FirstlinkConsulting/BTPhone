/**
 * @file i2c.c
 * @brief I2C interface driver, define I2C interface APIs.
 * This file provides I2C APIs: I2C initialization, write data to I2C & read data from I2C.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#include "drv_cfg.h"
#include "anyka_types.h"
#include "arch_gpio.h"
#include "interrupt.h"
#include "i2c.h"

#if DRV_SUPPORT_I2C > 0

#define  I2C_DELAY_UNIT            8

static T_VOID set_i2c_pin(T_U32 pin);
static T_VOID clr_i2c_pin(T_U32 pin);
static T_U8   get_i2c_pin(T_U32 pin);

static T_VOID i2c_init(T_VOID);
static T_VOID i2c_free(T_VOID);
static T_VOID i2c_delay(T_U32 time);
static T_VOID i2c_begin(T_VOID);
static T_VOID i2c_end(T_VOID);
static T_VOID i2c_write_ask(T_U8 flag);
static T_BOOL i2c_read_ack(T_VOID);
static T_BOOL i2c_write_byte(T_U8 data);
static T_U8   i2c_read_byte(T_VOID);

T_U32 i2c_sclk = 0xff;
T_U32 i2c_sda  = 0xff;


/**
 * @brief write data to I2C device
 * write size length data to dab's rab register
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U8 dab: I2C device address
 * @param T_U8 *data: write data's point
 * @param T_U8 size: write data's length
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_write_data(T_U8 dab, T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte(dab))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

/**
 * @brief read data from I2C device function
 * read data from dab's rab register
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U8 dab: I2C device address
 * @param T_U8 *data: read output data store address
 * @param T_U8 size: read data size
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
T_BOOL i2c_read_data(T_U8 dab, T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte((T_U8)(dab | 1)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

T_BOOL i2c_read_data_extend(T_U8 dab, T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte((T_U8)(dab)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }    

    if (!i2c_write_byte(data[0]))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    i2c_begin();
    if (!i2c_write_byte((T_U8)(dab | 1)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    for(i=0; i<size; i++)
    {
        data[i+1] = i2c_read_byte();
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

#if (DRV_SUPPORT_RTC > 0) && !(RTC_ANYKA > 0)
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
T_BOOL i2c_write_rtc_data(T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

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
T_BOOL i2c_read_rtc_data(T_U8 *data, T_U8 size)
{
    T_U8 i;

    i2c_init();

    i2c_begin();
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

static T_VOID i2c_write_rtc_byte(T_U8 data)
{
    T_U8 i;

    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        if (data & 0x01)
            set_i2c_pin(GPIO_RTC_DAT);
        else
            clr_i2c_pin(GPIO_RTC_DAT);
        data >>= 1;

        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_RTC_CLK);
        i2c_delay(I2C_DELAY_UNIT << 4);
        clr_i2c_pin(GPIO_RTC_CLK);
    }
}

static T_U8 i2c_read_rtc_byte(T_VOID)
{
    T_U8 i;
    T_U8 ret = 0;

    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_RTC_CLK);
        i2c_delay(I2C_DELAY_UNIT << 2);

        ret = ret>>1;
        if (get_i2c_pin(GPIO_RTC_DAT))
        {
            ret |= 0x80;
        }

        i2c_delay(I2C_DELAY_UNIT << 2);
        clr_i2c_pin(GPIO_RTC_CLK);
        i2c_delay(I2C_DELAY_UNIT << 1);
    }
    i2c_delay(I2C_DELAY_UNIT << 1);

    return ret;
}

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
T_BOOL i2c_write_rtc_data_ex(T_U8 *data, T_U8 size)
{
    T_U8 i;

    clr_i2c_pin(GPIO_RTC_CLK);
    i2c_delay(I2C_DELAY_UNIT << 4);

    for (i=0; i<size; i++)
    {
        i2c_write_rtc_byte(data[i]);
    }
    return AK_TRUE;
}

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
T_BOOL i2c_read_rtc_data_ex(T_U8 *data, T_U8 size)
{
    T_U8 i;

    gpio_set_pin_dir(GPIO_RTC_DAT, 0);
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_rtc_byte();
    }
    gpio_set_pin_dir(GPIO_RTC_DAT, 1);
    return AK_TRUE;
}
#endif

/**
 * @brief set  input function
 * set I2C input: 1
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID set_i2c_pin(T_U32 pin)
{
    gpio_set_pin_level(pin, 1);
}

/**
 * @brief clear I2C input function
 * set I2C input: 0
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_VOID
 * @retval
 */
static T_VOID clr_i2c_pin(T_U32 pin)
{
    gpio_set_pin_level(pin, 0);
}

/**
 * @brief get I2C output function
 * get I2C output data
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U32 pin: pin number
 * @return T_U8: get I2C output data
 * @retval
 */
static T_U8 get_i2c_pin(T_U32 pin)
{
    return gpio_get_pin_level(pin);
}

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
T_VOID i2c_initial(T_U32 gpio_i2c_sclk, T_U32 gpio_i2c_sda)
{
    i2c_sclk = gpio_i2c_sclk;
    i2c_sda  = gpio_i2c_sda;
}

/**
 * @brief I2C interface initialize function
 * setup I2C interface
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_init(T_VOID)
{
    //SET_I2C_SHAREPIN();
    gpio_set_pin_as_gpio(GPIO_I2C_SCLK);
    gpio_set_pin_as_gpio(GPIO_I2C_SDA);
	
    gpio_int_enable(GPIO_I2C_SCLK, 0);
    gpio_int_enable(GPIO_I2C_SDA, 0);
    gpio_set_pullup_pulldown(GPIO_I2C_SCLK, 0);
    gpio_set_pullup_pulldown(GPIO_I2C_SDA, 0);
    gpio_set_pin_dir(GPIO_I2C_SCLK, 1);
    gpio_set_pin_dir(GPIO_I2C_SDA, 1);
    gpio_set_pin_level(GPIO_I2C_SCLK, 1);
    gpio_set_pin_level(GPIO_I2C_SDA, 1);
}

static T_VOID i2c_free(T_VOID)
{
    //退出时拉高引脚,以节省功耗
    gpio_set_pin_level(GPIO_I2C_SCLK, 1);
    gpio_set_pin_level(GPIO_I2C_SDA, 1);
    return;
}

/**
 * @brief delay function
 * delay the time
 * @author Guanghua Zhang
 * @date 2004-09-21
 * @param T_U32 time: delay time
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_delay(T_U32 time)
{
    while(time--)
    {
        ;
    }
}

/**
 * @brief I2C interface start function
 * start I2C transmit
 * @author Junhua Zhao
 * @date 2004-04-05
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_begin(T_VOID)
{
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 4);
}

/**
 * @brief I2C interface stop function
 * stop I2C transmit
 * @author Junhua Zhao
 * @date 2004-05-04
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_end(T_VOID)
{
    i2c_delay(I2C_DELAY_UNIT << 2);
    clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 4);
}

/**
 * @brief I2C interface send asknowlege function
 * send a asknowlege to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_U8
 *       0:send bit 0
 *   not 0:send bit 1
 * @return T_VOID
 * @retval
 */
static T_VOID i2c_write_ask(T_U8 flag)
{
    if(flag)
        set_i2c_pin(GPIO_I2C_SDA);
    else
        clr_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 3);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);
    set_i2c_pin(GPIO_I2C_SDA);
    i2c_delay(I2C_DELAY_UNIT << 2);
}

/**
 * @brief I2C receive anknowlege
 * receive anknowlege from i2c bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_VOID
 * @return T_BOOL: return received anknowlege bit
 * @retval AK_FALSE: 0
 * @retval AK_TRUE: 1
 */
static T_BOOL i2c_read_ack(T_VOID)
{
    T_BOOL ret;
    set_i2c_pin(GPIO_I2C_SDA);
    gpio_set_pin_dir(GPIO_I2C_SDA, 0);
    i2c_delay(I2C_DELAY_UNIT << 3);
    set_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);

    if (!get_i2c_pin(GPIO_I2C_SDA))
    {
        ret = AK_TRUE;
    }
    else
    {
        ret = AK_FALSE;
    }

    i2c_delay(I2C_DELAY_UNIT << 2);
    clr_i2c_pin(GPIO_I2C_SCLK);
    i2c_delay(I2C_DELAY_UNIT << 2);
    gpio_set_pin_dir(GPIO_I2C_SDA, 1);
    i2c_delay(I2C_DELAY_UNIT << 2);
    return ret;
}

/**
 * @brief write one byte to I2C interface function
 * write one byte data to I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_U8 data: send the data
 * @return T_BOOL: return write success or failed
 * @retval AK_FALSE: operate failed
 * @retval AK_TRUE: operate success
 */
static T_BOOL i2c_write_byte(T_U8 data)
{
    T_U8 i;

    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        if (data & 0x80)
            set_i2c_pin(GPIO_I2C_SDA);
        else
            clr_i2c_pin(GPIO_I2C_SDA);
        data <<= 1;

        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 3);
        clr_i2c_pin(GPIO_I2C_SCLK);
    }
    return i2c_read_ack();
}

/**
 * @brief receive one byte from I2C interface function
 * receive one byte data from I2C bus
 * @author Junhua Zhao
 * @date 2005-04-05
 * @param T_VOID
 * @return T_U8: received the data
 * @retval
 */
static T_U8 i2c_read_byte(T_VOID)
{
    T_U8 i;
    T_U8 ret;

    ret = 0;
    gpio_set_pin_dir(GPIO_I2C_SDA, 0);
    for (i=0; i<8; i++)
    {
        i2c_delay(I2C_DELAY_UNIT << 2);
        set_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 2);

        ret = ret<<1;
        if (get_i2c_pin(GPIO_I2C_SDA))
        {
            ret |= 1;
        }

        i2c_delay(I2C_DELAY_UNIT << 2);
        clr_i2c_pin(GPIO_I2C_SCLK);
        i2c_delay(I2C_DELAY_UNIT << 1);

        if (i==7)
        {
            gpio_set_pin_dir(GPIO_I2C_SDA, 1);
        }
        i2c_delay(I2C_DELAY_UNIT << 1);
    }

    return ret;
}

T_BOOL i2c_write_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{
    T_U32 i;

    i2c_init();

    // start transmite
    i2c_begin();
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }

    // transmite data
    for (i=0; i<size; i++)
    {
        if (!i2c_write_byte(data[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }
    // stop transmited
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

T_BOOL i2c_read_bytes(T_U8 *addr, T_U32 addrlen, T_U8 *data, T_U32 size)
{

    T_U32 i;

    i2c_init();

    // start transmite
    i2c_begin();  
    // write address to I2C device, first is device address, second is the register address
    for (i=0; i<addrlen; i++)
    {
        if (!i2c_write_byte(addr[i]))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
    }
    i2c_end();

    // restart transmite
    i2c_begin();   
    // send message to I2C device to transmite data
    if (!i2c_write_byte((T_U8)(addr[0] | 1)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    // transmite data
    for(i=0; i<size; i++)
    {
        data[i] = i2c_read_byte();
        (i<size-1) ? i2c_write_ask(0) : i2c_write_ask(1);
    }
    // stop transmite
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

T_BOOL i2c_write_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];

    addr[0] = daddr;
    addr[1] = raddr;

    return i2c_write_bytes(addr, 2, data, size);
}

T_BOOL i2c_write_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[3];

    addr[0] = daddr;
    addr[1] = (T_U8)(raddr >> 8);   //hight 8bit
    addr[2] = (T_U8)(raddr);        //low 8bit

    return i2c_write_bytes(addr, 3, data, size);
}

T_BOOL i2c_write_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
    T_U32 i;
    T_U8 high_8bit,low_8bit;

    high_8bit = (T_U8)(raddr >> 8);   //hight 8bit
    low_8bit = (T_U8)(raddr);         //low 8bit

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    for(i=0; i<size; i++)
    {
        low_8bit = (T_U8)(*data);
        high_8bit = (T_U8)((*data) >> 8);

        if (!i2c_write_byte(high_8bit))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }
        if (!i2c_write_byte(low_8bit))
        {
            i2c_end();

            i2c_free();
            return AK_FALSE;
        }       
        data++;
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

T_BOOL i2c_write_data4(T_U8 daddr, T_U8 *data, T_U32 size)
{
    return i2c_write_bytes(&daddr, 1, data, size);
}

T_BOOL i2c_read_data1(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[2];

    addr[0] = daddr;
    addr[1] = raddr;   

    return i2c_read_bytes(addr, 2, data, size);
}

T_BOOL i2c_read_data2(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
    T_U8 addr[3];

    addr[0] = daddr;
    addr[1] = (T_U8)(raddr >> 8);   //hight 8bit
    addr[2] = (T_U8)(raddr);        //low 8bit

    return i2c_read_bytes(addr, 3, data, size);
}

T_BOOL i2c_read_data3(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
    T_U32 i;
    T_U8 high_8bit,low_8bit;

    high_8bit = (T_U8)(raddr >> 8);   //hight 8bit
    low_8bit = (T_U8)(raddr);         //low 8bit

    i2c_init();

    i2c_begin();
    if (!i2c_write_byte(daddr))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    if (!i2c_write_byte(high_8bit))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }
    if (!i2c_write_byte(low_8bit))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    i2c_begin();
    if (!i2c_write_byte((T_U8)(daddr | 1)))
    {
        i2c_end();

        i2c_free();
        return AK_FALSE;
    }

    for(i=0; i<size; i++)
    {
        high_8bit = i2c_read_byte();
        i2c_write_ask(0);
        low_8bit = i2c_read_byte();
        (i<size-1)?i2c_write_ask(0):i2c_write_ask(1);

        *data = (T_U16)(high_8bit) << 8 | low_8bit;
        data++;
    }
    i2c_end();

    i2c_free();
    return AK_TRUE;
}

T_BOOL i2c_read_data4(T_U8 daddr, T_U8 *data, T_U32 size)
{  
    return i2c_read_bytes(&daddr, 1, data, size);
}

#endif
