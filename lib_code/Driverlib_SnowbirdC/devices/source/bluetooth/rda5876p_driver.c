/*************************************************************************
 *
 * MODULE NAME:    bt_rda_chip_5876P.c
 * PROJECT CODE:   rda_bluetooth_chip
 * DESCRIPTION:    rda_bluetooth_chip 5876P file.
 * MAINTAINER:     lugongyu
 * CREATION DATE:  27/07/12
 * VERSION:        3.0.25
 * LICENSE:
 *     This source code is copyright (c) 2008-2012 rda.
 *     All rights reserved.
 *
 * NOTES TO USERS:
 *   None.
 *
 * MODIFICATION LOG
 *     
 *				 
 *************************************************************************/

#include "anyka_types.h"
#include "arch_rdabt.h"
#include "Arch_gpio.h"
#include "arch_uart.h"
#include "drv_cfg.h"
#include "arch_pwm.h"
#if (DRV_SUPPORT_BLUETOOTH > 0) && (BT_RDA5876p > 0)

typedef unsigned int        __u32;
typedef unsigned char       __u8;
typedef unsigned short      __u16;
typedef unsigned int        uint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;


#define RDA_5876P_VERB  1
#define RDA_5876P_VERC  3

/**
* !NOTE: adjust 0x24 and 0x28:
* 0x24:[9f~44] PSK
* 0x28:[4f~44] GPSK
*/
#if (5 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x359A)
#define INIT_VAL_REG_0X28   (0x124F)
#elif (4.5 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x359A)
#define INIT_VAL_REG_0X28   (0x1248)
#elif (3 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x359A)
#define INIT_VAL_REG_0X28   (0x1245)
#elif (1 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x3595)
#define INIT_VAL_REG_0X28   (0x1242)
#elif (-1 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x359A)
#define INIT_VAL_REG_0X28   (0x1244)
#elif (-3 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x3593)
#define INIT_VAL_REG_0X28   (0x1241)
#else //9 dbm
#define INIT_VAL_REG_0X24   (0x359f)
#define INIT_VAL_REG_0X28   (0x124f)
#endif

#define BQB_VAL_REG_0X24   (0x00001235)
#define BQB_VAL_REG_0X28   (0x00001234)



__u32 rda_chip_type;
__u8  rda_power_pin=INVALID_GPIO;
__u8  rda_uart_id=MAX_UART_NUM;
#define GPIO_BT_LDO_ON rda_power_pin
#define SERIAL_UART    rda_uart_id

extern T_VOID Fwl_DelayUs(T_U32 us);
extern T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir );
extern T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level );
extern void rdabt_iic_rf_write_data(unsigned char regaddr, unsigned short *data, unsigned char datalen);
extern void rdabt_iic_rf_read_data(uint8  regaddr, uint16*data, uint32 datalen);
extern void RDA_Bt_ReadData(uint32 regaddr, uint32 *data, uint32 datalen);
extern void RDA_rf_write(const uint16 pskey[][2],uint32 size);

/*******************************************************************************
 * @brief   rdabt_write_memory
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]addr  
 * @param   [in]data_addr
 * @param   [in]len
 * @param   [in]memory_type   
 * @return  T_void
*******************************************************************************/
void rdabt_write_memory(__u32 addr,const __u32 *data,__u8 len,__u8 memory_type)
{
	__u16 num_to_send; 
	__u16 i,j;
	__u8 data_to_send[256]={0};
	__u32 address_convert;
	
	data_to_send[0] = 0x01;
	data_to_send[1] = 0x02;
	data_to_send[2] = 0xfd;
	data_to_send[3] = (__u8)(len*4+6);
	data_to_send[4] = (memory_type+0x80);  // add the event display
	data_to_send[5] = len;
	if(memory_type == 0x01)
	{
		address_convert = addr*4+0x200;
		data_to_send[6] = (__u8)address_convert;
		data_to_send[7] = (__u8)(address_convert>>8);
		data_to_send[8] = (__u8)(address_convert>>16);
		data_to_send[9] = (__u8)(address_convert>>24);   
	}
	else
	{
		data_to_send[6] = (__u8)addr;
		data_to_send[7] = (__u8)(addr>>8);
		data_to_send[8] = (__u8)(addr>>16);
		data_to_send[9] = (__u8)(addr>>24);
	}
	for(i=0;i<len;i++,data++)
	{
		j=10+i*4;
		data_to_send[j] =  (__u8)(*data);
		data_to_send[j+1] = (__u8)((*data)>>8);
		data_to_send[j+2] = (__u8)((*data)>>16);
		data_to_send[j+3] = (__u8)((*data)>>24);
	}
	num_to_send = 4+data_to_send[3];
	
	//write(&(data_to_send[0]),num_to_send);
	//rdabt_adp_uart_tx(&(data_to_send[0]),num_to_send);
	uart_write_dat(SERIAL_UART, &(data_to_send[0]), num_to_send);
	// for (i =0; i < num_to_send; i++)
	//   { printf ("%02x ", data_to_send[i]);} 
}

/*******************************************************************************
 * @brief   RDA_uart_write_simple
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]*buf  
 * @param   [in]len
 * @return  T_void
*******************************************************************************/
void RDA_uart_write_simple(__u8* buf,__u16 len)
{
	//__u16 num_send; 
	//write(buf,len);
	uart_write_dat(SERIAL_UART, buf, len);
	//rdabt_adp_uart_tx(buf,len);
	delay_us(10000);//10ms?
}

/*******************************************************************************
 * @brief   RDA_uart_write_array
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]*buf  
 * @param   [in]type
 * @param   [in]len
 * @return  T_void
*******************************************************************************/
void RDA_uart_write_array(const __u32 buf[][2],__u16 len,__u8 type)
{
	__u32 i;
	for(i=0;i<len;i++)
	{
		rdabt_write_memory(buf[i][0],&buf[i][1],1,type);
	} 
	delay_us(12000);//12ms?
}


/*******************************************************************************
 * @brief   RDA_pin_to_high
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA_pin_to_high(void)
{
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	gpio_set_pin_level(GPIO_BT_LDO_ON,1);
}

/*******************************************************************************
 * @brief   RDA_pin_to_low
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA_pin_to_low(void)
{
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);	
	//set_rda_power(0);
	gpio_set_pin_level(GPIO_BT_LDO_ON,0);
}




__u8 RDA5876P_IS_FM_ON=0;
__u8 RDA5876P_IS_BT_ON=0;
/*******************************************************************************
 * @brief   const __u32 RDA5876P_VERB_5400_PSKEY[][2]
 * @author  liuhuadong
 * @rda5876 init para
*******************************************************************************/
const __u32 RDA5876P_VERB_5400_PSKEY[][2] = 
{   
	{0x3f,0x0000},
	{0x01,0x1FFF},
	{0x06,0x161C},
	{0x07,0x040D},
	{0x08,0x8326},
	{0x09,0x04B5},
	{0x0B,0x238F},
	{0x0C,0x85E8},
	{0x0E,0x0920},
	{0x0F,0x8DB3},
	{0x10,0x1402},
	{0x12,0x5604},
	{0x14,0x4ECC},
	{0x15,0x5124},
	{0x18,0x0812},
	{0x19,0x10C8},
	{0x1E,0x3024},
	{0x23,0x9991},
	{0x24,0x2369},
	{0x27,0x8881},
	{0x28,0x2358},
	{0x32,0x0E00},
	{0x3f,0x0001},
	{0x0A,0x001f},
	{0x00,0x020f},
	{0x01,0xf9cf},
	{0x02,0xfc2f},
	{0x03,0xf92f},
	{0x04,0xfa2f},
	{0x05,0xfc2f},
	{0x06,0xfb3f},
	{0x07,0x7fff},
	{0x18,0xFFFF},
	{0x19,0xFFFF},
	{0x1A,0xFFFF},
	{0x1B,0xFFFF},
	{0x1C,0xFFFF},
	{0x1D,0xFFFF},
	{0x1E,0xFFFF},
	{0x1F,0xFFFF},
	{0x3f,0x0000},
};
/*******************************************************************************
 * @brief   const __u32 RDA5876P_VERC_5400_PSKEY[][2]
 * @author  liuhuadong
 * @rda5876 init para
*******************************************************************************/
const __u32 RDA5876P_VERC_5400_PSKEY[][2] = 
{
	{0x3f,0x0000},
	{0x01,0x1FFF},
	{0x06,0x161C},
	{0x07,0x040D},
	{0x08,0x8326},
	{0x09,0x04B5},
	{0x0B,0x238F},
	{0x0C,0x85E8},
	{0x0E,0x0920},
	{0x0F,0x8DB3},
	{0x10,0x1400},
	{0x12,0x5604},
	{0x14,0x4ECC},
	{0x15,0x5124},
	{0x18,0x0812},
	{0x19,0x10C8},
	{0x1E,0x3024},
	{0x23,0x1111},
	{0x24,0x2468},
	{0x27,0x1111},
	{0x28,0x2358},
	{0x32,0x0E00},
	{0x3f,0x0001},
	{0x0A,0x001f},
	{0x00,0x020f},
	{0x01,0xf9cf},
	{0x02,0xfc2f},
	{0x03,0xf92f},
	{0x04,0xfa2f},
	{0x05,0xfc2f},
	{0x06,0xfb3f},
	{0x07,0x7fff},
	{0x17,0xE7F4},
	{0x18,0xF8F8},
	{0x19,0xFDFD},
	{0x1A,0xF2F3},
	{0x1B,0xFEFF},
	{0x1C,0xFEFF},
	{0x1D,0xFFFF},
	{0x1E,0xFFFF},
	{0x1F,0xFFFF},
	{0x3f,0x0000},      
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_VERB_PSKEY_RF[][2]
 * @author  liuhuadong
 * @rda5876 init para
*******************************************************************************/
const __u32 RDA5876P_VERB_PSKEY_RF[][2] =
{
	{0x40240000,0x2004f39c},
	{0x800000C0,0x0000000E},
	{0x800000C4,0x003F0000},
	{0x800000C8,0x0041000B},
	{0x800000CC,0x004232D0},
	{0x800000D0,0x0043BFC6},
	{0x800000D4,0x0044040F},
	{0x800000D8,0x004908E4},
	{0x800000DC,0x00694075},
	{0x800000E0,0x006B10C0},
	{0x800000E4,0x003F0001},
	{0x800000E8,0x00453000},
	{0x800000EC,0x0047D132},
	{0x800000F0,0x00490008},
	{0x800000F4,0x003F0000},
	{0x80000040,0x10000000},
	//{0x40240000,0x0000f29c},   //; SPI2_CLK_EN PCLK_SPI2_EN
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_VERC_PSKEY_RF[][2]
 * @author  liuhuadong
 * @rda5876 init para
*******************************************************************************/
const __u32 RDA5876P_VERC_PSKEY_RF[][2] = 
{
	{0x40240000,0x2004f39c},
	{0x800000C0,0x0000000F},
	{0x800000C4,0x003F0000},
	{0x800000C8,0x0041000B},
	{0x800000CC,0x004225BD},
	{0x800000D0,0x0043BFC6},
	{0x800000D4,0x0044040F},
	{0x800000D8,0x004908E4},
	{0x800000DC,0x00694075},
	{0x800000E0,0x006B10C0},
	{0x800000E4,0x003F0001},
	{0x800000E8,0x00402000},
	{0x800000EC,0x00453000},
	{0x800000F0,0x0047D132},
	{0x800000F4,0x00490008},
	{0x800000F8,0x003F0000},
	{0x80000040,0x10000000},
	//{0x40240000,0x0000f29c},   //; SPI2_CLK_EN PCLK_SPI2_EN

};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_ENABLE_SPI[][2]
 * @author  liuhuadong
 * @rda5876 spi para
*******************************************************************************/
const __u32 RDA5876P_ENABLE_SPI[][2] =
{
    	{0x40240000,0x2004f39c},                               
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_DISABLE_SPI[][2]
 * @author  liuhuadong
 * @rda5876 spi para
*******************************************************************************/
const __u32 RDA5876P_DISABLE_SPI[][2] = 
{
    	{0x40240000,0x2000f29c},
};



const __u32 RDA5876P_DCCAL[][2]=
{
	{0x30,0x0129},
	{0x30,0x012b}
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_TRAP[][2]
 * @author  liuhuadong
 * @rda5876 RDA5876P_TRAP
*******************************************************************************/
const __u32 RDA5876P_TRAP[][2] = 
{
	{0x80000098,0x92610020},
	{0x80000040,0x00400000},
	
	{0x40180004,0x0001b578},
	{0x40180024,0x0001dc68},
	{0x40180008,0x00003034},
	{0x40180028,0x00000014},
	{0x4018000c,0x0001514c},
	{0x4018002c,0x00000014},
	
	{0x40180010,0x0000d1a8},//inquiry patch 
	{0x40180030,0x00000014},

	{0x40180014,0x0002b9b8},//connection patch
	{0x40180034,0x00000014},

	{0x40180018,0x000193cc},//encrpyt   
	{0x40180038,0x00004000},   
	
	{0x40180000,0x0000003f}, 
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_PSKEY_MISC[][2]
 * @author  liuhuadong
 * @rda5876 RDA5876P_pskey
*******************************************************************************/
const __u32 RDA5876P_PSKEY_MISC[][2] =
{
	{0x800004f0,0xfa8dFFFF},//disable 3m edr, enable edr	
	{0x800004f4,0x83793f98},//enable ssp
	//{0x800004f4,0x83713f98}, ///disable 3m esco ev4 ev5
	
#if (0 == RDA_HFP_LINK_MODE)
	{0x80000070,0x00012098},//short name//{0x80000070,0x00012008},//short name 
#else
	{0x80000070,0x00012018},//short name//{0x80000070,0x00012008},//short name 
#endif
	{0x80000074,0x0f025010},  //sleep
 	{0x80000078,0x0a004000},
	{0x8000007c,0xb530b530},
#if  (1 == RDA_HFP_LINK_MODE)
	{0x80000084,0x9090C007},
#elif (2 == RDA_HFP_LINK_MODE)
	{0x80000084,0x101E0007},
#else
	{0x80000084,0x9098C007},
#endif
	{0x800000a0,0x00000000},//g_host_wake_event1 g_host_wake_event2=0
	{0x800000a4,0x00000000},
	{0x800000a8,0x0Bbaba30},//min power level

    {0x80000060,RDA_UART_BAUD_RATE},
    {0x80000064,RDA_UART_BAUD_RATE},   
	
	{0x80000040,0x0702f300},
};

/*******************************************************************************
 * @brief   const __u32 RDA5876P_RfInit[][2]
 * @author  liuhuadong
 * @rda5876 RDA5876P_rfinit
*******************************************************************************/
void RDA5876P_RfInit(void)
{
	RDA_uart_write_array(RDA5876P_ENABLE_SPI,sizeof(RDA5876P_ENABLE_SPI)/sizeof(RDA5876P_ENABLE_SPI[0]),0);
	if(rda_chip_type==RDA_5876P_VERB)
	{
		RDA_uart_write_array(RDA5876P_VERB_5400_PSKEY,sizeof(RDA5876P_VERB_5400_PSKEY)/sizeof(RDA5876P_VERB_5400_PSKEY[0]),1);
	}
	else if(rda_chip_type==RDA_5876P_VERC)
	{
		RDA_uart_write_array(RDA5876P_VERC_5400_PSKEY,sizeof(RDA5876P_VERC_5400_PSKEY)/sizeof(RDA5876P_VERC_5400_PSKEY[0]),1);
	}
	delay_us(10);
}

/*******************************************************************************
 * @brief   RDA5876P_Pskey_RfInit
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_Pskey_RfInit(void)
{
	if(rda_chip_type==RDA_5876P_VERB)
	{
		RDA_uart_write_array(RDA5876P_VERB_PSKEY_RF,sizeof(RDA5876P_VERB_PSKEY_RF)/sizeof(RDA5876P_VERB_PSKEY_RF[0]),0);
	}
	else if(rda_chip_type==RDA_5876P_VERC)
	{
		RDA_uart_write_array(RDA5876P_VERC_PSKEY_RF,sizeof(RDA5876P_VERC_PSKEY_RF)/sizeof(RDA5876P_VERC_PSKEY_RF[0]),0);
	}
}
/*******************************************************************************
 * @brief   RDA5876P_Dccal
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_Dccal(void)
{
	RDA_uart_write_array(RDA5876P_DCCAL,sizeof(RDA5876P_DCCAL)/sizeof(RDA5876P_DCCAL[0]),1);
	RDA_uart_write_array(RDA5876P_DISABLE_SPI,sizeof(RDA5876P_DISABLE_SPI)/sizeof(RDA5876P_DISABLE_SPI[0]),0);
}

/*******************************************************************************
 * @brief   RDA5876P_BTON_FMOFF_INIT
 * @author  liuhuadong
 * @date    2013.07.19
*******************************************************************************/
const __u16 RDA5876P_BTON_FMOFF_INIT[][2] = 
{
    {0x003f,0x0001},//page 1
    {0x0025,0x03e1},//
    {0x0026,0x43a5},//set voltage=1.2v
    {0x0028,0x6800},//
    {0x002d,0x00aa},//FM_OFF
    {0x002f,0x1100},//
#ifdef RDA_CHANGE_BT_ADDR   
    {0x0032,0x88f8},//TM=000;DN=1111
#else
    {0x0032,0x88f9},//TM=001;DN=1111
#endif
    {0x0022,0x0e93},//
    {0x003f,0x0000} //page 0
};

/*******************************************************************************
 * @brief   RDA5876P_BTON_FMON_INIT
 * @author  liuhuadong
 * @date    2013.07.19
*******************************************************************************/
const __u16 RDA5876P_BTON_FMON_INIT[][2] = 
{
    {0x003f,0x0001},//page 1
    {0x0025,0x07ff},//
    {0x0026,0x23b3},//set voltage=1.2v
    {0x0028,0x6800},//
    {0x002d,0x006a},//FM_OFF
    {0x002f,0x1100},//
#ifdef RDA_CHANGE_BT_ADDR   
    {0x0032,0x88f8},//TM=000;DN=1111
#else
    {0x0032,0x88f9},//TM=001;DN=1111
#endif
    {0x0022,0x0e93},//
    {0x003f,0x0000} //page 0
};

/*******************************************************************************
 * @brief   RDA5876P_BTOFF_FMON_INIT
 * @author  liuhuadong
 * @date    2013.07.19
*******************************************************************************/
const __u16 RDA5876P_BTOFF_FMON_INIT[][2] = 
{
    {0x003f,0x0001},//page 1
    {0x0025,0x67e5},//
    {0x0026,0x23a3},//set voltage=1.2v
    {0x0028,0x2800},//
    {0x002d,0x006a},//FM_OFF
    {0x002f,0x2100},//
#ifdef RDA_CHANGE_BT_ADDR   
    {0x0032,0x88f8},//TM=000;DN=1111
#else
    {0x0032,0x88f9},//TM=001;DN=1111
#endif
    {0x0022,0x0ea3},//
    {0x003f,0x0000} //page 0
};
const __u16 RDA5876P_BTOFF_FMOFF_INIT[][2] = 
{
    {0x003f,0x0001},//page 1
    {0x003f,0x0000} //page 0
};

/*******************************************************************************
 * @brief   RDABT_5876p_bton_fmoff_patch
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDABT_5876p_bton_fmoff_patch(void)
{
	RDA_rf_write(RDA5876P_BTON_FMOFF_INIT,sizeof(RDA5876P_BTON_FMOFF_INIT)/sizeof(RDA5876P_BTON_FMOFF_INIT[0])); 																  
}

/*******************************************************************************
 * @brief   RDABT_5876p_bton_fmon_patch
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDABT_5876p_bton_fmon_patch(void)
{
	RDA_rf_write(RDA5876P_BTON_FMON_INIT,sizeof(RDA5876P_BTON_FMON_INIT)/sizeof(RDA5876P_BTON_FMON_INIT[0]));																  		
}

/*******************************************************************************
 * @brief   RDABT_5876p_btoff_fmon_patch
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDABT_5876p_btoff_fmon_patch(void)
{
	RDA_rf_write(RDA5876P_BTOFF_FMON_INIT,sizeof(RDA5876P_BTOFF_FMON_INIT)/sizeof(RDA5876P_BTOFF_FMON_INIT[0]));																  		
}

/*******************************************************************************
 * @brief   RDABT_5876p_btoff_fmoff_patch
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDABT_5876p_btoff_fmoff_patch(void)
{
	RDA_rf_write(RDA5876P_BTOFF_FMOFF_INIT,sizeof(RDA5876P_BTOFF_FMOFF_INIT)/sizeof(RDA5876P_BTOFF_FMOFF_INIT[0]));																  		
}

void rdabt_poweron_init(void)
{

}

/*******************************************************************************
 * @brief   RDA5876P_PMU_RfInit
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_PMU_RfInit(void)
{
	if(RDA5876P_IS_FM_ON==0)
	{ //FM off          
		RDABT_5876p_bton_fmoff_patch();		
	}
	else
	{ //FM on, not need LDO on again
		RDABT_5876p_bton_fmon_patch();		
	}
	RDA5876P_IS_BT_ON = 1;
}



/*******************************************************************************
 * @brief   RDA_bt_get_chipVer
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]t_u32
 * @return  T_void
*******************************************************************************/
__u32 RDA_bt_get_chipVer(void)
{
	uint32 i;
    uint8 pageaddr = 0x3f;
    uint8 chipidaddr = 0x20;
    uint8 revidaddr =0x21;
    uint16 pagevalue = 0x0001;
	uint16 chipid=0xffff;
    uint16 revid=0;
    rdabt_iic_rf_write_data(pageaddr,&pagevalue,1);
    rdabt_iic_rf_read_data(chipidaddr,&chipid,1);
    rdabt_iic_rf_read_data(revidaddr,&revid,1);
	RDA_Bt_ReadData(0x11111111,&i,1);//just to ensure the I2C return to normal
	
    //SCI_TRACE_LOW("The chip id is 0x%08x",chipid);
    if(chipid == 0x587f)
	{
		if(revid==1)
		{
			return RDA_5876P_VERB;
		}
		else if (revid==3)
		{
			return RDA_5876P_VERC;
		}
    }
    else
        return 0;
}

/*******************************************************************************
 * @brief   rdabt_poweroff
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]int
 * @return  T_void
*******************************************************************************/
int rdabt_poweroff(void)
{
	if(0 == RDA5876P_IS_FM_ON)
		RDA_pin_to_low();
	else
		RDABT_5876p_btoff_fmon_patch();
	RDA5876P_IS_BT_ON = 0;
    rda_power_pin=INVALID_GPIO;
    rda_uart_id=MAX_UART_NUM;
	
	return 1;
}

/*******************************************************************************
 * @brief   rdabt_poweron_start
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]t_u8 power_pin
 * @param   [in]t_u8 uart_id 
 * @return  int
*******************************************************************************/
int rdabt_poweron_start(T_U8 power_pin, T_U8 uart_id)
{
    rda_power_pin = power_pin;
    rda_uart_id = uart_id;
    
    
	RDA_pin_to_high();
	
	akerror("rdafm id=", RDAFM_GetChipID(), 1);
    rda_chip_type = RDA_bt_get_chipVer();
	akerror("rdabt id=", rda_chip_type, 1);

	RDA_pin_to_high();
	delay_us(50000); 
	RDA5876P_PMU_RfInit();
	RDA5876P_RfInit();
	RDA5876P_Pskey_RfInit();	
	//RDA_pin_to_low();
	RDA_pin_to_high();    
	delay_us(50); //wait for digi running 

	RDA5876P_RfInit(); 	
	RDA5876P_Pskey_RfInit();	
	RDA5876P_Dccal();	

	RDA_uart_write_array(RDA5876P_TRAP,sizeof(RDA5876P_TRAP)/sizeof(RDA5876P_TRAP[0]),0);
	RDA_uart_write_array(RDA5876P_PSKEY_MISC,sizeof(RDA5876P_PSKEY_MISC)/sizeof(RDA5876P_PSKEY_MISC[0]),0);
	
	return 0;
}


/*******************************************************************************
 * @brief   RDA5876P_DUT_Test
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  void
*******************************************************************************/
#if (1==SUPPORT_BLUE_TEST)
void RDA5876P_DUT_Test(void)
{

	RDA_pin_to_high();
	
    rda_chip_type = RDA_bt_get_chipVer();
	//akerror("rdabt id=", rda_chip_type, 1);

	RDA_pin_to_high();
	Fwl_DelayUs(50000); 

	RDA5876P_RfInit();
	RDA5876P_Pskey_RfInit();
	
	//RDA_pin_to_low();
	RDA_pin_to_high(); 
	Fwl_DelayUs(50000);
	
	RDA5876P_RfInit();   
	RDA5876P_Pskey_RfInit();
	
	RDA5876P_Dccal(); 
	RDA_uart_write_array(RDA5876P_DUT_PSKEY,sizeof(RDA5876P_DUT_PSKEY)/sizeof(RDA5876P_DUT_PSKEY[0]),0);
	RDA_uart_write_array(RDA5876P_DUT,sizeof(RDA5876P_DUT)/sizeof(RDA5876P_DUT[0]),0);
	RDA_uart_write_simple(&(RDA_ENABLE_ALLSCAN[0]),sizeof(RDA_ENABLE_ALLSCAN)/sizeof(RDA_ENABLE_ALLSCAN[0]));
	RDA_uart_write_simple(&(RDA_AUTOACCEPT_CONNECT[0]),sizeof(RDA_AUTOACCEPT_CONNECT)/sizeof(RDA_AUTOACCEPT_CONNECT[0])); 
	RDA_uart_write_simple(&(RDA_ENABLE_DUT[0]),sizeof(RDA_ENABLE_DUT)/sizeof(RDA_ENABLE_DUT[0]));
	
}

/*******************************************************************************
 * @brief   RDA5876P_BQB_VERB_5400_PSKEY
 * @author  liuhuadong
*******************************************************************************/
#elif (2 == SUPPORT_BLUE_TEST)
const __u32 RDA5876P_BQB_VERB_5400_PSKEY[][2] = 
{   
	{0x3f,0x0000},
	{0x01,0x1FFF},
	{0x06,0x161C},
	{0x07,0x040D},
	{0x08,0x8326},
	{0x09,0x04B5},
	{0x0B,0x238F},
	{0x0C,0x85E8},
	{0x0E,0x0920},
	{0x0F,0x8DB3},
	{0x10,0x1402},
	{0x12,0x5604},
	{0x14,0x4ECC},
	{0x15,0x5124},
	{0x18,0x0812},
	{0x19,0x10C8},
	{0x1E,0x3024},
	{0x23,0x9991},
	{0x24,BQB_VAL_REG_0X24},//
	{0x27,0x8881},
	{0x28,BQB_VAL_REG_0X28},//
	{0x32,0x0E00},
	{0x3f,0x0001},
	{0x0A,0x001f},
	{0x00,0x020f},
	{0x01,0xf9cf},
	{0x02,0xfc2f},
	{0x03,0xf92f},
	{0x04,0xfa2f},
	{0x05,0xfc2f},
	{0x06,0xfb3f},
	{0x07,0x7fff},
	{0x18,0xFFFF},
	{0x19,0xFFFF},
	{0x1A,0xFFFF},
	{0x1B,0xFFFF},
	{0x1C,0xFFFF},
	{0x1D,0xFFFF},
	{0x1E,0xFFFF},
	{0x1F,0xFFFF},
	{0x3f,0x0000},
};

/*******************************************************************************
 * @brief   RDA5876P_BQB_VERC_5400_PSKEY
 * @author  liuhuadong
*******************************************************************************/
const __u32 RDA5876P_BQB_VERC_5400_PSKEY[][2] = 
{
	{0x3f,0x0000},
	{0x01,0x1FFF},
	{0x06,0x161C},
	{0x07,0x040D},
	{0x08,0x8326},
	{0x09,0x04B5},
	{0x0B,0x238F},
	{0x0C,0x85E8},
	{0x0E,0x0920},
	{0x0F,0x8DB3},
	{0x10,0x1400},
	{0x12,0x5604},
	{0x14,0x4ECC},
	{0x15,0x5124},
	{0x18,0x0812},
	{0x19,0x10C8},
	{0x1E,0x3024},
	{0x23,0x1111},
	{0x24,BQB_VAL_REG_0X24},//
	{0x27,0x1111},
	{0x28,BQB_VAL_REG_0X28},//
	{0x32,0x0E00},
	{0x3f,0x0001},
	{0x0A,0x001f},
	{0x00,0x020f},
	{0x01,0xf9cf},
	{0x02,0xfc2f},
	{0x03,0xf92f},
	{0x04,0xfa2f},
	{0x05,0xfc2f},
	{0x06,0xfb3f},
	{0x07,0x7fff},
	{0x17,0xE7F4},
	{0x18,0xF8F8},
	{0x19,0xFDFD},
	{0x1A,0xF2F3},
	{0x1B,0xFEFF},
	{0x1C,0xFEFF},
	{0x1D,0xFFFF},
	{0x1E,0xFFFF},
	{0x1F,0xFFFF},
	{0x3f,0x0000},      
};

/*******************************************************************************
 * @brief   RDA5876P_BQB_VERB_PSKEY_RF
 * @author  liuhuadong
*******************************************************************************/
const __u32 RDA5876P_BQB_VERB_PSKEY_RF[][2] =
{
	{0x40240000,0x0004f39c},
	{0x800000C0,0x0000000E},
	{0x800000C4,0x003F0000},
	{0x800000C8,0x0041000B},
	{0x800000CC,0x004232D0},
	{0x800000D0,0x0043BFC6},
	{0x800000D4,0x0044040F},
	{0x800000D8,0x004908E4},
	{0x800000DC,0x00694075},
	{0x800000E0,0x006B10C0},
	{0x800000E4,0x003F0001},
	{0x800000E8,0x00453000},
	{0x800000EC,0x0047D132},
	{0x800000F0,0x00490008},
	{0x800000F4,0x003F0000},
	{0x80000040,0x10000000},
};

/*******************************************************************************
 * @brief   RDA5876P_BQB_VERC_PSKEY_RF
 * @author  liuhuadong
*******************************************************************************/
const __u32 RDA5876P_BQB_VERC_PSKEY_RF[][2] = 
{
	{0x40240000,0x0004f39c},
	{0x800000C0,0x0000000F},
	{0x800000C4,0x003F0000},
	{0x800000C8,0x0041000B},
	{0x800000CC,0x004225BD},
	{0x800000D0,0x0043BFC6},
	{0x800000D4,0x0044040F},
	{0x800000D8,0x004908E4},
	{0x800000DC,0x00694075},
	{0x800000E0,0x006B10C0},
	{0x800000E4,0x003F0001},
	{0x800000E8,0x00402000},
	{0x800000EC,0x00453000},
	{0x800000F0,0x0047D132},
	{0x800000F4,0x00490008},
	{0x800000F8,0x003F0000},
	{0x80000040,0x10000000},
};

/*******************************************************************************
 * @brief   RDA5876P_BQB_RfInit
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_BQB_RfInit()
{
	RDA_uart_write_array(RDA5876P_ENABLE_SPI,sizeof(RDA5876P_ENABLE_SPI)/sizeof(RDA5876P_ENABLE_SPI[0]),0);
#if(5876P_VERB == rda_chip_type)
	{
		RDA_uart_write_array(RDA5876P_BQB_VERB_5400_PSKEY,sizeof(RDA5876P_BQB_VERB_5400_PSKEY)/sizeof(RDA5876P_BQB_VERB_5400_PSKEY[0]),1);
	}
#elif(5876P_VERC == rda_chip_type)
	{
		RDA_uart_write_array(RDA5876P_BQB_VERC_5400_PSKEY,sizeof(RDA5876P_BQB_VERC_5400_PSKEY)/sizeof(RDA5876P_BQB_VERC_5400_PSKEY[0]),1);
	}
#endif
	delay_us(50000);//50ms?
}

/*******************************************************************************
 * @brief   RDA5876P_BQB_Pskey_RfInit
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_BQB_Pskey_RfInit()
{
	#if(5876P_VERB == rda_chip_type)
	{
		RDA_uart_write_array(RDA5876P_BQB_VERB_PSKEY_RF,sizeof(RDA5876P_BQB_VERB_PSKEY_RF)/sizeof(RDA5876P_BQB_VERB_PSKEY_RF[0]),0);
	}
	#elif(5876P_VERC == rda_chip_type)
	{
		RDA_uart_write_array(RDA5876P_BQB_VERC_PSKEY_RF,sizeof(RDA5876P_BQB_VERC_PSKEY_RF)/sizeof(RDA5876P_BQB_VERC_PSKEY_RF[0]),0);
	}
	#endif
}

/*******************************************************************************
 * @brief   RDA5876P_BQB_Test
 * @author  liuhuadong
 * @date    2013.07.19
 * @param   [in]void
 * @return  T_void
*******************************************************************************/
void RDA5876P_BQB_Test()
{
/*
	RDA_pin_to_low();
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_DelayUs(100);
	RDA_pin_to_high();
*/
	RDA_pin_to_high();
	
    rda_chip_type = RDA_bt_get_chipVer();
	//akerror("rdabt id=", rda_chip_type, 1);

	RDA_pin_to_high();
	delay_us(50000); 
	
	RDA5876P_BQB_RfInit();
	RDA5876P_BQB_Pskey_RfInit();
	
	//RDA_pin_to_low();
	RDA_pin_to_high(); 
	delay_us(50000);
	
	RDA5876P_BQB_RfInit();   
	RDA5876P_BQB_Pskey_RfInit();
	
	RDA5876P_Dccal();  
	
	RDA_uart_write_array(RDA5876P_DUT_PSKEY,sizeof(RDA5876P_DUT_PSKEY)/sizeof(RDA5876P_DUT_PSKEY[0]),0);
	
	RDA_uart_write_array(RDA5876P_DUT,sizeof(RDA5876P_DUT)/sizeof(RDA5876P_DUT[0]),0);
	
	RDA_uart_write_simple(&(RDA_ENABLE_ALLSCAN[0]),sizeof(RDA_ENABLE_ALLSCAN)/sizeof(RDA_ENABLE_ALLSCAN[0]));
	
	RDA_uart_write_simple(&(RDA_AUTOACCEPT_CONNECT[0]),sizeof(RDA_AUTOACCEPT_CONNECT)/sizeof(RDA_AUTOACCEPT_CONNECT[0])); 
	
	RDA_uart_write_simple(&(RDA_ENABLE_DUT[0]),sizeof(RDA_ENABLE_DUT)/sizeof(RDA_ENABLE_DUT[0]));
}
#endif


#endif
