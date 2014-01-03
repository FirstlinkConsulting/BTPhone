
/*************************************************************************
*
* MODULE NAME:    rda5876_drv_init.c
* PROJECT CODE:   rdabt
* DESCRIPTION:    rda5876 bt drv.
* MAINTAINER:     
* CREATION DATE:  
*
* SOURCE CONTROL: $Id:  2011/10/18 16:22:38  Exp $
*
* LICENSE:
*     This source code is copyright (c) 2008-2009 rda.
*     All rights reserved.
*
* NOTES TO USERS:
*   None.
*		
*  version 
*
*************************************************************************/

#include "anyka_types.h"
#include "gpio_define.h"
#include "Gbl_Define.h"
#include "rdabt_uart.h"
#include "Arch_gpio.h"
#include "Fwl_Timer.h"

#if(5876 == BLUE_IC_TYPE)

typedef unsigned int        __u32;
typedef unsigned char       __u8;
typedef unsigned short      __u16;
#define RDA_HFP_LINK_MODE	0
/**
* dBm lvl select
*/
#define BT_POWER_DBM_LVL   9
/**
* !NOTE: adjust 0x24 and 0x28:
* 0x24:[9f~44] PSK
* 0x28:[4f~44] GPSK
*/
#if (5 == BT_POWER_DBM_LVL)
#define INIT_VAL_REG_0X24   (0x359A)
#define INIT_VAL_REG_0X28   (0x124F)
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

extern T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir );
extern T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level );


void rdabt_write_memory(__u32 addr,__u32 *data,__u8 len,__u8 memory_type)
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
	rdabt_adp_uart_tx(&(data_to_send[0]),num_to_send);
	
	// for (i =0; i < num_to_send; i++)
	//   { printf ("%02x ", data_to_send[i]);} 
}

void RDA_uart_write_simple(__u8* buf,__u16 len)
{
	//__u16 num_send; 
	//write(buf,len);
	rdabt_adp_uart_tx(buf,len);
	Fwl_DelayMs(10);//10ms?
}

void RDA_uart_write_array(__u32 buf[][2],__u16 len,__u8 type)
{
	__u32 i;
	for(i=0;i<len;i++)
	{
		rdabt_write_memory(buf[i][0],&buf[i][1],1,type);
	} 
	Fwl_DelayMs(12);//12ms?
}

const __u32 rdabt_rf_init_12[][2] = 
{   
	{0x3f,0x0000}, //;page 0
	{0x01,0x1FFF}, //;
	{0x06,0x07F7}, //;padrv_set,increase the power.
	{0x08,0x01E7}, //;
	{0x09,0x0520}, //;
	{0x0B,0x03DF}, //;filter_cap_tuning<3:0>1101
	{0x0C,0x85E8}, //;
	{0x0F,0x0DBC}, //; 0FH,16'h1D8C; 0FH,16'h1DBC;adc_clk_sel=1 20110314 ;adc_digi_pwr_reg<2:0>=011;                                                              
	{0x12,0x07F7}, //;padrv_set,increase the power.                                
	{0x13,0x0327}, //;agpio down pullen .                                          
	{0x14,0x0CCC}, //;h0CFE; bbdac_cm 00=vdd/2.                                                            
	{0x15,0x0526}, //;Pll_bypass_ontch:1,improve ACPR.
	{0x16,0x8918}, //;add div24 20101126
	{0x18,0x8800}, //;add div24 20101231
	{0x19,0x10C8}, //;pll_adcclk_en=1 20101126
	{0x1A,0x9128}, //;Mdll_adcclk_out_en=0
	{0x1B,0x80C0}, //;1BH,16'h80C2
	{0x1C,0x361f}, //;
	{0x1D,0x33fb}, //;Pll_cp_bit_tx<3:0>1110;13D3
	{0x1E,0x303f}, //;Pll_lpf_gain_tx<1:0> 00;304C
	//{0x23,0x3335}, //;txgain
	//{0x24,0x8AAF}, //;
	//{0x27,0x1112}, //;
	//{0x28,0x345F}, //;
	{0x23,0x2222}, //;txgain
	{0x24,INIT_VAL_REG_0X24},//;psk
	{0x27,0x0011}, //;
	{0x28,INIT_VAL_REG_0X28},//;gpsk
	
	{0x39,0xA5FC}, //;    
	{0x3f,0x0001}, //;page 1
	{0x00,0x043F}, //;agc
	{0x01,0x467F}, //;agc
	{0x02,0x28FF}, //;agc//20110323;82H,16'h68FF;agc
	{0x03,0x67FF}, //;agc
	{0x04,0x57FF}, //;agc
	{0x05,0x7BFF}, //;agc
	{0x06,0x3FFF}, //;agc
	{0x07,0x7FFF}, //;agc
	{0x18,0xf3f5}, //;
	{0x19,0xf3f5}, //;
	{0x1A,0xe7f3}, //;
	{0x1B,0xf1ff}, //;
	{0x1C,0xffff}, //;
	{0x1D,0xffff}, //;
	{0x1E,0xffff}, //;
	{0x1F,0xFFFF}, //;padrv_gain;9FH,16'hFFEC;padrv_gain20101103
	
	{0x23,0x4224}, //;ext32k
	{0x24,0x0110},
	{0x25,0x43E1}, //;ldo_vbit:110,2.04v
	{0x26,0x4bb5}, //;reg_ibit:100,reg_vbit:101,1.2v,reg_vbit_deepsleep:110,750mV
	{0x32,0x0079}, //;TM mod
	{0x3f,0x0000}, //;page 0
};

const __u32 RDA5876_ENABLE_SPI[][2] =
{
	{0x40240000,0x2004f39c},                               
};

const __u32 RDA5876_DISABLE_SPI[][2] = 
{
	{0x40240000,0x2000f29c},
};

const __u32 RDA5876_PSKEY_RF[][2] =
{
	{0x40240000,0x2004f39c}, //; SPI2_CLK_EN PCLK_SPI2_EN
	{0x800000C0,0x00000021}, //; CHIP_PS PSKEY: Total number -----------------
	{0x800000C4,0x003F0000},
	{0x800000C8,0x00414003},
	{0x800000CC,0x004225BD},
	{0x800000D0,0x004908E4},
	{0x800000D4,0x0043B074},
	{0x800000D8,0x0044D01A},
	{0x800000DC,0x004A0600},
	//    {0x800000DC,0x004A0800},
	{0x800000E0,0x0054A020},
	{0x800000E4,0x0055A020},
	{0x800000E8,0x0056A542},
	{0x800000EC,0x00574C18},
	{0x800000F0,0x003F0001},
	{0x800000F4,0x00410900},
	{0x800000F8,0x0046033F},
	{0x800000FC,0x004C0000},
	{0x80000100,0x004D0015},
	{0x80000104,0x004E002B},
	{0x80000108,0x004F0042},
	{0x8000010C,0x0050005A},
	{0x80000110,0x00510073},
	{0x80000114,0x0052008D},
	{0x80000118,0x005300A7},
	{0x8000011C,0x005400C4},
	{0x80000120,0x005500E3},
	{0x80000124,0x00560103},
	{0x80000128,0x00570127},
	{0x8000012C,0x0058014E},
	{0x80000130,0x00590178},
	{0x80000134,0x005A01A1},
	{0x80000138,0x005B01CE},
	{0x8000013C,0x005C01FF},
	{0x80000140,0x003F0000},
	{0x80000144,0x00000000}, //;         PSKEY: Page 0
	{0x80000040,0x10000000},
	//{0x40240000,0x0000f29c}, //; SPI2_CLK_EN PCLK_SPI2_EN 
	
};

const __u32 RDA5876_DCCAL[][2]=
{
	{0x00000030,0x00000129},
	{0x00000030,0x0000012b}
};

const __u32 RDA5876_PSKEY_MISC[][2] =
{
	{0x800004ec,0xfa8dFFFF},//disable 3m edr, enable edr	
	{0x800004f0,0x83793f98},//enable ssp
	//{0x800004f0,0x83713f98}, ///disable 3m esco ev4 ev5
	
#if (0 == RDA_HFP_LINK_MODE)
	{0x80000070,0x00012098},//short name ,屏蔽多连接//{0x80000070,0x00012008},//short name 
#else
    {0x80000070,0x00012018},//short name ,屏蔽多连接//{0x80000070,0x00012008},//short name 
#endif
    {0x80000074,0xaf025010},  //sleep
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
	//3200000
	//   {0x80000060,0x0030d400},
	//   {0x80000064,0x0030d400},
	//921600
	//{0x80000060,0x000e1000},
//	{0x80000064,0x000e1000},
	//3000000
	//{0x80000060,0x002dc6c0},
	//{0x80000064,0x002dc6c0},
    {0x80000060,RDA_UART_BAUD_RATE},
    {0x80000064,RDA_UART_BAUD_RATE},   
	
	{0x80000040,0x0702b300},	
	
};

const __u32 RDA5876_TRAP[][2] = 
{
	{0x40180100,0x000068b0},//inc power
	{0x40180120,0x000068f4},
	
	{0x40180104,0x000066b0},//dec power
	{0x40180124,0x000068f4},
	
	{0x40180108,0x0001544c},//esco w
	{0x40180128,0x0001568c},
	
	{0x80000100,0xe3a0704f}, ///2ev3 ev3 hv3
	{0x4018010c,0x0000bae8},//esco packet
	{0x4018012c,0x80000100},
	
	
	//{0x40180110,0x000247a0},//sleep patch
	//{0x40180130,0x0002475c},
	{0x40180114,0x0000f8c4},///all rxon
	{0x40180134,0x00026948},
	
	{0x40180118,0x000130b8},///qos PRH_CHN_QUALITY_MIN_NUM_PACKETS
	{0x40180138,0x0001cbb4},

	{0x40180014,0x00017354}, // auth after encry
	{0x40180034,0x00017368},
	{0x40180018,0x0001a338}, // auth after encry
	{0x40180038,0x00000014},
	{0x4018001c,0x0001936c},// pair after auth fail
	{0x4018003c,0x00001ca8},

	{0x40180000,0x00006f70},
	
};

const __u32 RDA5876_DUT[][2] = 
{
	
	{0x40180100,0x000068b0},
	{0x40180120,0x000068f4},
	{0x40180104,0x000066b0},
	{0x40180124,0x000068f4},
	{0x40180108,0x0000f8c4},//all rxon
	{0x40180128,0x00026948},
	{0x40180000,0x00000700},
};

const __u32 RDA5876_DUT_PSKEY[][2]=
{
	{0x800000a4,0x00000000},
	{0x80000074,0x05025010},
	{0x800000a8,0x0cbfbf30},
	{0x80000040,0x06002000},
};

const __u8 RDA_ENABLE_DUT[] = 
{
	0x01,0x03, 0x18, 0x00
};

const __u8  RDA_ENABLE_ALLSCAN[] = 
{
	0x01, 0x1a, 0x0c, 0x01, 0x03
};


const __u8 RDA_AUTOACCEPT_CONNECT[] = 
{
	0x01,0x05, 0x0c, 0x03, 0x02, 0x00, 0x02
};

void RDA5876_RfInit()
{
	RDA_uart_write_array(RDA5876_ENABLE_SPI,sizeof(RDA5876_ENABLE_SPI)/sizeof(RDA5876_ENABLE_SPI[0]),0);
	RDA_uart_write_array(rdabt_rf_init_12,sizeof(rdabt_rf_init_12)/sizeof(rdabt_rf_init_12[0]),1);
	Fwl_DelayMs(50);//50ms?
}

void RDA5876_Pskey_RfInit()
{
	RDA_uart_write_array(RDA5876_PSKEY_RF,sizeof(RDA5876_PSKEY_RF)/sizeof(RDA5876_PSKEY_RF[0]),0);
}

void RDA5876_Dccal()
{
	RDA_uart_write_array(RDA5876_DCCAL,sizeof(RDA5876_DCCAL)/sizeof(RDA5876_DCCAL[0]),1);
	RDA_uart_write_array(RDA5876_DISABLE_SPI,sizeof(RDA5876_DISABLE_SPI)/sizeof(RDA5876_DISABLE_SPI[0]),0);
}

void RDA5876_Pskey_Misc()
{
	RDA_uart_write_array(RDA5876_PSKEY_MISC,sizeof(RDA5876_PSKEY_MISC)/sizeof(RDA5876_PSKEY_MISC[0]),0);
}

void RDA5876_Trap()
{
	RDA_uart_write_array(RDA5876_TRAP,sizeof(RDA5876_TRAP)/sizeof(RDA5876_TRAP[0]),0);
}
void RDA_pin_to_high(void)
{
	Fwl_UartSetStateRTS(AK_TRUE);
}

void RDA_pin_to_low(void)
{
	//set_rda_power(0);
	Fwl_UartSetStateRTS(AK_FALSE);
}

int rdabt_poweroff(void)
{
	//gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_UartSetStateRTS(AK_FALSE);

	return 1;
}

void rdabt_poweron_init(void)
{
	// gpio_set_pin_level(36,1);

	Fwl_UartSetStateRTS(AK_TRUE);
	Fwl_DelayMs(1);
	Fwl_UartSetStateRTS(AK_FALSE);
	
}


int rdabt_poweron_start(void)
{
    
	//RDA5876_RfInit();
	//RDA5876_Pskey_RfInit();
	rdabt_poweron_init();
	Fwl_DelayMs(50);
	RDA5876_RfInit();   
	RDA5876_Pskey_RfInit();
	
	RDA5876_Dccal(); 
	RDA5876_Trap();
	Fwl_DelayMs(50);
	RDA5876_Pskey_Misc();

	return 0;
}

#if (1==SUPPORT_BLUE_TEST)
void RDA5876_DUT_Test(void)
{
	RDA_pin_to_low();
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_DelayUs(100);
	RDA_pin_to_high();
	
	RDA5876_RfInit();
	RDA5876_Pskey_RfInit();
	
	RDA_pin_to_low();
	RDA_pin_to_high(); 
	Fwl_DelayUs(50000);
	
	RDA5876_RfInit();   
	RDA5876_Pskey_RfInit();
	
	RDA5876_Dccal(); 
	RDA_uart_write_array(RDA5876_DUT_PSKEY,sizeof(RDA5876_DUT_PSKEY)/sizeof(RDA5876_DUT_PSKEY[0]),0);
	RDA_uart_write_array(RDA5876_DUT,sizeof(RDA5876_DUT)/sizeof(RDA5876_DUT[0]),0);
	RDA_uart_write_simple(&(RDA_ENABLE_ALLSCAN[0]),sizeof(RDA_ENABLE_ALLSCAN)/sizeof(RDA_ENABLE_ALLSCAN[0]));
	RDA_uart_write_simple(&(RDA_AUTOACCEPT_CONNECT[0]),sizeof(RDA_AUTOACCEPT_CONNECT)/sizeof(RDA_AUTOACCEPT_CONNECT[0])); 
	RDA_uart_write_simple(&(RDA_ENABLE_DUT[0]),sizeof(RDA_ENABLE_DUT)/sizeof(RDA_ENABLE_DUT[0]));
	
}

#elif (2 == SUPPORT_BLUE_TEST)
const __u32 rdabt_BQB_rf_init_12[][2] = 
{   
	{0x0000003f,0x00000000}, //;page 0                                   
	{0x00000001,0x00001FFF}, //;                                    
	{0x00000006,0x000007F7}, //;padrv_set,increase the power.             
	{0x00000008,0x000001E7}, //;                                    
	{0x00000009,0x00000520}, //;                                    
	{0x0000000B,0x0000030F}, //;filter_cap_tuning<3:0>1101                
	{0x0000000C,0x000085E8}, //;                                    
	{0x0000000F,0x00000DBC}, //; 0FH,16'h1D8C; 0FH,16'h1DBC;adc_clk_sel=1 20110314 ;adc_digi_pwr_reg<2:0>=011;                                                              
	{0x00000012,0x000007F7}, //;padrv_set,increase the power.                                
	{0x00000013,0x00000327}, //;agpio down pullen .                                          
	{0x00000014,0x00000CCC}, //;h0CFE; bbdac_cm 00=vdd/2.                                                            
	{0x00000015,0x00000526}, //;Pll_bypass_ontch:1,improve ACPR.          
	{0x00000016,0x00008918}, //;add div24 20101126                        
	{0x00000018,0x00008800}, //;add div24 20101231                        
	{0x00000019,0x000010C8}, //;pll_adcclk_en=1 20101126      
	{0x0000001A,0x00009128}, //;Mdll_adcclk_out_en=0          
	{0x0000001B,0x000080C0}, //;1BH,16'h80C2                  
	{0x0000001C,0x0000b61f}, //;                           
	{0x0000001D,0x000033fb}, //;Pll_cp_bit_tx<3:0>1110;13D3   
	{0x0000001E,0x0000303f}, //;Pll_lpf_gain_tx<1:0> 00;304C  
	
	{0x00000023,0x00002222}, //;txgain                        
	{0x00000024,BQB_VAL_REG_0X24}, //;                           
	{0x00000027,0x00000011}, //;                           
	{0x00000028,BQB_VAL_REG_0X28}, //;                           
	
	{0x00000039,0x0000A5FC}, //;                              
	{0x0000003f,0x00000001}, //;page 1                        
	{0x00000000,0x0000043F}, //;agc                           
	{0x00000001,0x0000467F}, //;agc                           
	{0x00000002,0x000028FF}, //;agc//20110323;82H,16'h68FF;agc
	{0x00000003,0x000067FF}, //;agc
	{0x00000004,0x000057FF}, //;agc
	{0x00000005,0x00007BFF}, //;agc
	{0x00000006,0x00003FFF}, //;agc
	{0x00000007,0x00007FFF}, //;agc
	{0x00000018,0x0000f3f5}, //;
	{0x00000019,0x0000f3f5}, //;
	{0x0000001A,0x0000e7f3}, //;
	{0x0000001B,0x0000f1ff}, //;
	{0x0000001C,0x0000e2fe}, //;
	{0x0000001D,0x0000e2fe}, //;
	{0x0000001E,0x0000e3fe}, //;
	{0x0000001F,0x0000e7Fe}, //;padrv_gain;9FH,16'hFFEC;padrv_gain20101103
	{0x00000023,0x00004224}, //;ext32k
	{0x00000024,0x00000110},
	{0x00000025,0x000043E1}, //;ldo_vbit:110,2.04v
	{0x00000026,0x00004bb5}, //;reg_ibit:100,reg_vbit:101,1.2v,reg_vbit_deepsleep:110,750mV
	{0x00000032,0x00000079}, //;TM mod
	{0x0000003f,0x00000000}, //;page 0
	{0x00000030,0x00000129},
	{0x00000030,0x0000012b}
	
};

const __u32 RDA5876_BQB_PSKEY_RF[][2] =
{
	{0x40240000,0x2004f39c}, //; SPI2_CLK_EN PCLK_SPI2_EN                                                                      
	{0x800000C0,0x00000021}, //; CHIP_PS PSKEY: Total number -----------------                                                 
	{0x800000C4,0x003F0000}, //;         PSKEY: Page 0                                                                         
	{0x800000C8,0x00414003}, //;         PSKEY: Swch_clk_ADC                                                                   
	{0x800000CC,0x004225BD}, //;         PSKEY: dig IF 625K IF  by lihua20101231                                               
	{0x800000D0,0x004908E4}, //;         PSKEY: freq_offset_for rateconv  by lihua20101231(replace 47H)                        
	{0x800000D4,0x0043B074}, //;         PSKEY: AM dac gain, 20100605                                                          
	{0x800000D8,0x0044D01A}, //;         PSKEY: gfsk dac gain, 20100605//22                                                    
	{0x800000DC,0x004A0800}, //;         PSKEY: 4AH=0800                                                                       
	{0x800000E0,0x0054A020}, //;         PSKEY: 54H=A020;agc_th_max=A0;agc_th_min=20                                           
	{0x800000E4,0x0055A020}, //;         PSKEY: 55H=A020;agc_th_max_lg=A0;agc_th_min_lg=20                                     
	{0x800000E8,0x0056A542}, //;         PSKEY: 56H=A542                                                                       
	{0x800000EC,0x00574C18}, //;         PSKEY: 57H=4C18                                                                       
	{0x800000F0,0x003F0001}, //;         PSKEY: Page 1                                                                         
	{0x800000F4,0x00410900}, //;         PSKEY: C1=0900;Phase Delay, 20101029                                                  
	{0x800000F8,0x0046033F}, //;         PSKEY: C6=033F;modulation Index;delta f2=160KHZ,delta f1=164khz                       
	{0x800000FC,0x004C0000}, //;         PSKEY: CC=0000;20101108                                                               
	{0x80000100,0x004D0015}, //;         PSKEY: CD=0015;                                                                       
	{0x80000104,0x004E002B}, //;         PSKEY: CE=002B;                                                                       
	{0x80000108,0x004F0042}, //;         PSKEY: CF=0042                                                                        
	{0x8000010C,0x0050005A}, //;         PSKEY: D0=005A                                                                        
	{0x80000110,0x00510073}, //;         PSKEY: D1=0073                                                                        
	{0x80000114,0x0052008D}, //;         PSKEY: D2=008D                                                                        
	{0x80000118,0x005300A7}, //;         PSKEY: D3=00A7                                                                        
	{0x8000011C,0x005400C4}, //;         PSKEY: D4=00C4                                                                        
	{0x80000120,0x005500E3}, //;         PSKEY: D5=00E3                                                                        
	{0x80000124,0x00560103}, //;         PSKEY: D6=0103                                                                        
	{0x80000128,0x00570127}, //;         PSKEY: D7=0127                                                                        
	{0x8000012C,0x0058014E}, //;         PSKEY: D8=014E                                                                        
	{0x80000130,0x00590178}, //;         PSKEY: D9=0178                                                                        
	{0x80000134,0x005A01A1}, //;         PSKEY: DA=01A1                                                                        
	{0x80000138,0x005B01CE}, //;         PSKEY: DB=01CE                                                                        
	{0x8000013C,0x005C01FF}, //;         PSKEY: DC=01FF                                                                        
	{0x80000140,0x003F0000}, //;         PSKEY: Page 0                                                                         
	{0x80000144,0x00000000}, //;         PSKEY: Page 0                                                                         
	{0x80000040,0x10000000}, //;         PSKEY: Flage                                                                          
};

void RDA5876_BQB_RfInit()
{
	RDA_uart_write_array(RDA5876_ENABLE_SPI,sizeof(RDA5876_ENABLE_SPI)/sizeof(RDA5876_ENABLE_SPI[0]),0);
	RDA_uart_write_array(rdabt_BQB_rf_init_12,sizeof(rdabt_BQB_rf_init_12)/sizeof(rdabt_BQB_rf_init_12[0]),1);
	Fwl_DelayUs(50000);//50ms?
}

void RDA5876_BQB_Pskey_RfInit()
{
	RDA_uart_write_array(RDA5876_BQB_PSKEY_RF,sizeof(RDA5876_BQB_PSKEY_RF)/sizeof(RDA5876_BQB_PSKEY_RF[0]),0);
}

void RDA5876_BQB_Test()
{
	RDA_pin_to_low();
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_DelayUs(100);
	RDA_pin_to_high();
	
	
	RDA5876_BQB_RfInit();
	RDA5876_BQB_Pskey_RfInit();
	
	RDA_pin_to_low();
	RDA_pin_to_high(); 
	Fwl_DelayUs(50000);
	
	RDA5876_BQB_RfInit();   
	RDA5876_BQB_Pskey_RfInit();
	
	RDA5876_Dccal();  
	
	RDA_uart_write_array(RDA5876_DUT_PSKEY,sizeof(RDA5876_DUT_PSKEY)/sizeof(RDA5876_DUT_PSKEY[0]),0);
	
	RDA_uart_write_array(RDA5876_DUT,sizeof(RDA5876_DUT)/sizeof(RDA5876_DUT[0]),0);
	
	RDA_uart_write_simple(&(RDA_ENABLE_ALLSCAN[0]),sizeof(RDA_ENABLE_ALLSCAN)/sizeof(RDA_ENABLE_ALLSCAN[0]));
	
	RDA_uart_write_simple(&(RDA_AUTOACCEPT_CONNECT[0]),sizeof(RDA_AUTOACCEPT_CONNECT)/sizeof(RDA_AUTOACCEPT_CONNECT[0])); 
	
	RDA_uart_write_simple(&(RDA_ENABLE_DUT[0]),sizeof(RDA_ENABLE_DUT)/sizeof(RDA_ENABLE_DUT[0]));
}
#elif (4 == SUPPORT_BLUE_TEST)
const __u32 RDA5876_FCC_PATCH[][2]=
{
0x80000008,0xea00248c,
0x4018000c,0x0002247c,
0x4018002c,0x00032d1c,
0x8000000c,0xea00004f,
0x80000150,0xe59f0014,
0x80000154,0xe5900000,
0x80000158,0xe3500000,
0x8000015c,0x092d40f8,
0x80000160,0x03a00b7f,
0x80000164,0x0280fe21,
0x80000168,0xea00250c,
0x8000016c,0x80009768,
0x40180010,0x0001fe0c,
0x40180030,0x00032d20,
0x80000010,0xea000056,
0x80000170,0xe59f0014,
0x80000174,0xe5900000,
0x80000178,0xe3500000,
0x8000017c,0x092d4070,
0x80000180,0x03a00b7f,
0x80000184,0x0280f090,
0x80000188,0xea0024b5,
0x8000018c,0x80009768,
0x40180010,0x0001fc8c,
0x40180030,0x00032d24,
0x80009200,0xe51ff004,
0x80009204,0x00003458,
0x80009208,0xe51ff004,
0x8000920c,0x0002c88c,
0x80009210,0xe51ff004,
0x80009214,0x00000c90,
0x80009218,0xe51ff004,
0x8000921c,0x0001beb8,
0x80009220,0xe51ff004,
0x80009224,0x0001bd00,
0x80009228,0xe51ff004,
0x8000922c,0x00003284,
0x80009230,0xe51ff004,
0x80009234,0x0002ec88,
0x80009238,0xe51ff004,
0x8000923c,0x00004c5c,
0x80009240,0xe92d43fe,
0x80009244,0xe5d05000,
0x80009248,0xe5d06001,
0x8000924c,0xe1a04000,
0x80009250,0xe2800002,
0x80009254,0xebffffe9,
0x80009258,0xe1a09000,
0x8000925c,0xe5d47006,
0x80009260,0xe5d48007,
0x80009264,0xe2840008,
0x80009268,0xebffffe4,
0x8000926c,0xe3550000,
0x80009270,0x0a00002d,
0x80009274,0xe3570000,
0x80009278,0x03c11480,
0x8000927c,0xe59f20b0,
0x80009280,0x13811480,
0x80009284,0xe5821014,
0x80009288,0xe59f10a8,
0x8000928c,0xe3a02001,
0x80009290,0xe5810004,
0x80009294,0xe3a00000,
0x80009298,0xe5810008,
0x8000929c,0xe5812000,
0x800092a0,0xe5818018,
0x800092a4,0xe581000c,
0x800092a8,0xe5810010,
0x800092ac,0xe5810014,
0x800092b0,0xe2860cc0,
0x800092b4,0xe2800dc6,
0x800092b8,0xe3800b80,
0x800092bc,0xebffffd1,
0x800092c0,0xe28d2004,
0x800092c4,0xe28d1008,
0x800092c8,0xe1a00009,
0x800092cc,0xebffffcf,
0x800092d0,0xe28d0004,
0x800092d4,0xebffffcf,
0x800092d8,0xeb00004e,
0x800092dc,0xe59f0058,
0x800092e0,0xe5d01001,
0x800092e4,0xe5d02008,
0x800092e8,0xe26110dd,
0x800092ec,0xe2622f9b,
0x800092f0,0xe3822c80,
0x800092f4,0xe2811e40,
0x800092f8,0xe1812802,
0x800092fc,0xe3a01470,
0x80009300,0xe5812140,
0x80009304,0xe5d02002,
0x80009308,0xe5d00009,
0x8000930c,0xe2622015,
0x80009310,0xe26000be,
0x80009314,0xe2800e40,
0x80009318,0xe2822f40,
0x8000931c,0xe1820800,
0x80009320,0xe3800c80,
0x80009324,0xe5810158,
0x80009328,0xe8bd83fe,
0x8000932c,0xeb000003,
0x80009330,0xeafffffc,
0x80009334,0x800018a0,
0x80009338,0x80009768,
0x8000933c,0x80002e34,
0x80009340,0xe92d403e,
0x80009344,0xe59f40bc,
0x80009348,0xe3a00017,
0x8000934c,0xe98d0011,
0x80009350,0xe59f50b4,
0x80009354,0xe3a00000,
0x80009358,0xe5850000,
0x8000935c,0xe59f00ac,
0x80009360,0xe3a01000,
0x80009364,0xe5902000,
0x80009368,0xe3c22003,
0x8000936c,0xe1811002,
0x80009370,0xe5801000,
0x80009374,0xe3a00000,
0x80009378,0xe3a01470,
0x8000937c,0xe5810170,
0x80009380,0xe59f008c,
0x80009384,0xe5d02001,
0x80009388,0xe5d03008,
0x8000938c,0xe26220dd,
0x80009390,0xe2822e40,
0x80009394,0xe2633f9b,
0x80009398,0xe1822803,
0x8000939c,0xe5812140,
0x800093a0,0xe5d02002,
0x800093a4,0xe5d00009,
0x800093a8,0xe2622f90,
0x800093ac,0xe26000be,
0x800093b0,0xe2800e40,
0x800093b4,0xe3800c80,
0x800093b8,0xe1820800,
0x800093bc,0xe3800c80,
0x800093c0,0xe5810158,
0x800093c4,0xe2840007,
0x800093c8,0xe5951010,
0x800093cc,0xebffff93,
0x800093d0,0xe284000b,
0x800093d4,0xe5951008,
0x800093d8,0xebffff90,
0x800093dc,0xe284000f,
0x800093e0,0xe5951014,
0x800093e4,0xebffff8d,
0x800093e8,0xe2840013,
0x800093ec,0xe595100c,
0x800093f0,0xebffff8a,
0x800093f4,0xe3a02000,
0x800093f8,0xe28d1004,
0x800093fc,0xe28d0008,
0x80009400,0xebffff88,
0x80009404,0xe8bd803e,
0x80009408,0x80009788,
0x8000940c,0x80009768,
0x80009410,0x70000048,
0x80009414,0x80002e34,
0x80009418,0xe92d4010,
0x8000941c,0xe3a01470,
0x80009420,0xe5910020,
0x80009424,0xe3c0001f,
0x80009428,0xe3c004ff,
0x8000942c,0xe59f402c,
0x80009430,0xe3800041,
0x80009434,0xe5840008,
0x80009438,0xe5910024,
0x8000943c,0xe3c004ff,
0x80009440,0xe3c008bf,
0x80009444,0xe584000c,
0x80009448,0xe3a00003,
0x8000944c,0xe5c40020,
0x80009450,0xebffff76,
0x80009454,0xe5d40020,
0x80009458,0xe8bd4010,
0x8000945c,0xeaffff75,
0x80009460,0x800018a0,
0x80009464,0xe92d41f0,
0x80009468,0xe3a07470,
0x8000946c,0xe5970050,
0x80009470,0xe59f6118,
0x80009474,0xe20050ff,
0x80009478,0xe596000c,
0x8000947c,0xe0800185,
0x80009480,0xe586000c,
0x80009484,0xe5960008,
0x80009488,0xe2800001,
0x8000948c,0xe5860008,
0x80009490,0xe5970050,
0x80009494,0xe1a00280,
0x80009498,0xe1b00fa0,
0x8000949c,0x0a000012,
0x800094a0,0xe5960010,
0x800094a4,0xe3a04000,
0x800094a8,0xe2800001,
0x800094ac,0xe5860010,
0x800094b0,0xe59f80dc,
0x800094b4,0xea00000a,
0x800094b8,0xe59f00d8,
0x800094bc,0xe5961018,
0x800094c0,0xe7d00004,
0x800094c4,0xe7d81001,
0x800094c8,0xe1510000,
0x800094cc,0x0a000003,
0x800094d0,0xeb000069,
0x800094d4,0xe5961014,
0x800094d8,0xe0800001,
0x800094dc,0xe5860014,
0x800094e0,0xe2844001,
0x800094e4,0xe1540005,
0x800094e8,0x3afffff2,
0x800094ec,0xe5b61004,
0x800094f0,0xe5960004,
0x800094f4,0xe1500001,
0x800094f8,0x08bd41f0,
0x800094fc,0x0affff8f,
0x80009500,0xe59f4094,
0x80009504,0xe5d40001,
0x80009508,0xe5d41008,
0x8000950c,0xe26000dd,
0x80009510,0xe2800e40,
0x80009514,0xe2611f9b,
0x80009518,0xe1800801,
0x8000951c,0xe5870140,
0x80009520,0xe5d40002,
0x80009524,0xe5d41009,
0x80009528,0xe2600015,
0x8000952c,0xe26110be,
0x80009530,0xe2811e40,
0x80009534,0xe2800f40,
0x80009538,0xe1800801,
0x8000953c,0xe3800c80,
0x80009540,0xe5870158,
0x80009544,0xebffffb3,
0x80009548,0xe5d40001,
0x8000954c,0xe5d41008,
0x80009550,0xe26000dd,
0x80009554,0xe2611f9b,
0x80009558,0xe3811c80,
0x8000955c,0xe2800e40,
0x80009560,0xe1800801,
0x80009564,0xe5870140,
0x80009568,0xe5d40002,
0x8000956c,0xe5d41009,
0x80009570,0xe2600015,
0x80009574,0xe26110be,
0x80009578,0xe2811e40,
0x8000957c,0xe2800f40,
0x80009580,0xe1800801,
0x80009584,0xe3800c80,
0x80009588,0xe5870158,
0x8000958c,0xe8bd81f0,
0x80009590,0x80009768,
0x80009594,0x80009784,
0x80009598,0x70000600,
0x8000959c,0x80002e34,
0x800095a0,0xe92d4038,
0x800095a4,0xe3a05470,
0x800095a8,0xe5950050,
0x800095ac,0xe1a00300,
0x800095b0,0xe1b00fa0,
0x800095b4,0x0a00002d,
0x800095b8,0xe59f00b4,
0x800095bc,0xe5901008,
0x800095c0,0xe2811001,
0x800095c4,0xe5801008,
0x800095c8,0xe5902010,
0x800095cc,0xe2822001,
0x800095d0,0xe5802010,
0x800095d4,0xe5900004,
0x800095d8,0xe1510000,
0x800095dc,0x08bd4038,
0x800095e0,0x0affff56,
0x800095e4,0xe59f408c,
0x800095e8,0xe5d40001,
0x800095ec,0xe5d41008,
0x800095f0,0xe26000dd,
0x800095f4,0xe2800e40,
0x800095f8,0xe2611f9b,
0x800095fc,0xe1800801,
0x80009600,0xe5850140,
0x80009604,0xe5d40002,
0x80009608,0xe5d41009,
0x8000960c,0xe2600015,
0x80009610,0xe26110be,
0x80009614,0xe2811e40,
0x80009618,0xe2800f40,
0x8000961c,0xe1800801,
0x80009620,0xe3800c80,
0x80009624,0xe5850158,
0x80009628,0xebffff7a,
0x8000962c,0xe5d40001,
0x80009630,0xe5d41008,
0x80009634,0xe26000dd,
0x80009638,0xe2611f9b,
0x8000963c,0xe3811c80,
0x80009640,0xe2800e40,
0x80009644,0xe1800801,
0x80009648,0xe5850140,
0x8000964c,0xe5d40002,
0x80009650,0xe5d41009,
0x80009654,0xe2600015,
0x80009658,0xe26110be,
0x8000965c,0xe2811e40,
0x80009660,0xe2800f40,
0x80009664,0xe1800801,
0x80009668,0xe3800c80,
0x8000966c,0xe5850158,
0x80009670,0xe8bd8038,
0x80009674,0x80009768,
0x80009678,0x80002e34,
0x8000967c,0xe59f20e0,
0x80009680,0xe1a01000,
0x80009684,0xe5922018,
0x80009688,0xe3a00000,
0x8000968c,0xe3a0c001,
0x80009690,0xe3520000,
0x80009694,0x0a000009,
0x80009698,0xe3520001,
0x8000969c,0x03a02000,
0x800096a0,0x0a00000e,
0x800096a4,0xe3520002,
0x800096a8,0x03a02000,
0x800096ac,0x0a000018,
0x800096b0,0xe3520003,
0x800096b4,0x11a0f00e,
0x800096b8,0xe3a02000,
0x800096bc,0xea000020,
0x800096c0,0xe111021c,
0x800096c4,0x12800001,
0x800096c8,0xe2822001,
0x800096cc,0xe20220ff,
0x800096d0,0x120000ff,
0x800096d4,0xe3520008,
0x800096d8,0x3afffff8,
0x800096dc,0xe1a0f00e,
0x800096e0,0xe1a03082,
0x800096e4,0xe111031c,
0x800096e8,0xe08c3082,
0x800096ec,0x12800001,
0x800096f0,0x120000ff,
0x800096f4,0xe111031c,
0x800096f8,0x02800001,
0x800096fc,0xe2822001,
0x80009700,0xe20220ff,
0x80009704,0x020000ff,
0x80009708,0xe3520004,
0x8000970c,0x3afffff3,
0x80009710,0xe1a0f00e,
0x80009714,0xe111021c,
0x80009718,0xe2823004,
0x8000971c,0x12800001,
0x80009720,0x120000ff,
0x80009724,0xe111031c,
0x80009728,0x02800001,
0x8000972c,0xe2822001,
0x80009730,0xe20220ff,
0x80009734,0x020000ff,
0x80009738,0xe3520004,
0x8000973c,0x3afffff4,
0x80009740,0xe1a0f00e,
0x80009744,0xe111021c,
0x80009748,0x02800001,
0x8000974c,0xe2822001,
0x80009750,0xe20220ff,
0x80009754,0x020000ff,
0x80009758,0xe3520008,
0x8000975c,0x3afffff8,
0x80009760,0xe1a0f00e,
0x80009764,0x80009768,
0x80009768,0x00000000,
0x8000976c,0x00000000,
0x80009770,0x00000000,
0x80009774,0x00000000,
0x80009778,0x00000000,
0x8000977c,0x00000000,
0x80009780,0x00000000,
0x80009784,0xfff0aa00,
0x80009788,0x01140e04,
0x8000978c,0x0000fd77,
0x80009790,0x00000000,
0x80009794,0x00000000,
0x80009798,0x00000000,
0x8000979c,0x00000000,
0x40180000,0x0000001f
};
void RDA5876_FCC_Test()
{
	RDA_pin_to_low();
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_DelayUs(100);
	RDA_pin_to_high();
	
	
	
	RDA5876_Pskey_RfInit();
	RDA_pin_to_low();
	RDA_pin_to_high(); 
	Fwl_DelayUs(50000);
	RDA5876_RfInit();	
	RDA5876_Pskey_RfInit();
	RDA5876_Dccal(); 

	RDA_uart_write_array(RDA5876_FCC_PATCH,sizeof(RDA5876_FCC_PATCH)/sizeof(RDA5876_FCC_PATCH[0]),0);
}	
#elif (5 == SUPPORT_BLUE_TEST)
const __u32 RDA5876_FCC_PATCH[][2]=
{
0x70000008,0x331a3ae2,
0x7000000c,0x4e7a2cce,
0x80000000,0xea00238A,
0x40180004,0x0001c26c,
0x40180024,0x00032d14,
0x80000004,0xea00003f,
0x80000100,0xe51ff004,
0x80000104,0x800090dc,
0x80000108,0xebfffffc,
0x8000010c,0xe8bd8038,
0x40180008,0x0001fbdc,
0x40180028,0x00032d18,
0x80008e00,0xe51ff004,
0x80008e04,0x00000434,
0x80008e08,0xe51ff004,
0x80008e0c,0x0000529c,
0x80008e10,0xe51ff004,
0x80008e14,0x0001c6e4,
0x80008e18,0xe51ff004,
0x80008e1c,0x0002c88c,
0x80008e20,0xe51ff004,
0x80008e24,0x00004ce8,
0x80008e28,0xe51ff004,
0x80008e2c,0x000052c8,
0x80008e30,0xe92d4038,
0x80008e34,0xe59f5064,
0x80008e38,0xe1a04000,
0x80008e3c,0xe3a00000,
0x80008e40,0xe5850000,
0x80008e44,0xe5d40000,
0x80008e48,0xe350004f,
0x80008e4c,0x11a00004,
0x80008e50,0x18bd4038,
0x80008e54,0x1a000012,
0x80008e58,0xe5950000,
0x80008e5c,0xe3500000,
0x80008e60,0x1a00000d,
0x80008e64,0xe5d41000,
0x80008e68,0xe3a0004f,
0x80008e6c,0xebffffe3,
0x80008e70,0xe5c41000,
0x80008e74,0xe1a00004,
0x80008e78,0xeb000009,
0x80008e7c,0xe3a01000,
0x80008e80,0xe3a00064,
0x80008e84,0xebffffdf,
0x80008e88,0xebffffe0,
0x80008e8c,0xe5d40000,
0x80008e90,0xe2800001,
0x80008e94,0xe5c40000,
0x80008e98,0xeaffffee,
0x80008e9c,0xe8bd8038,
0x80008ea0,0x80009158,
0x80008ea4,0xe5d01002,
0x80008ea8,0xe3510000,
0x80008eac,0x0a000002,
0x80008eb0,0xe3510001,
0x80008eb4,0x0a00004d,
0x80008eb8,0xe1a0f00e,
0x80008ebc,0xe92d4038,
0x80008ec0,0xe3a05470,
0x80008ec4,0xe1a04000,
0x80008ec8,0xe5950020,
0x80008ecc,0xe5d41014,
0x80008ed0,0xe3c00007,
0x80008ed4,0xe2011007,
0x80008ed8,0xe1810000,
0x80008edc,0xe5850020,
0x80008ee0,0xe5950024,
0x80008ee4,0xe3c00480,
0x80008ee8,0xe5850024,
0x80008eec,0xe5d40014,
0x80008ef0,0xe59f10f0,
0x80008ef4,0xe5c10000,
0x80008ef8,0xe5d40006,
0x80008efc,0xe3a01003,
0x80008f00,0xe0810280,
0x80008f04,0xe5d41000,
0x80008f08,0xe1a00b80,
0x80008f0c,0xe0810820,
0x80008f10,0xe3800b80,
0x80008f14,0xebffffbf,
0x80008f18,0xe5950024,
0x80008f1c,0xe3800e80,
0x80008f20,0xe5850024,
0x80008f24,0xe5d40001,
0x80008f28,0xe5d41002,
0x80008f2c,0xe1800081,
0x80008f30,0xe5d41003,
0x80008f34,0xe1800181,
0x80008f38,0xe1d411b0,
0x80008f3c,0xe1800381,
0x80008f40,0xe1d411b2,
0x80008f44,0xe1800801,
0x80008f48,0xe5d41005,
0x80008f4c,0xe3510002,
0x80008f50,0x03800004,
0x80008f54,0xe58500f4,
0x80008f58,0xe5950030,
0x80008f5c,0xe5d41005,
0x80008f60,0xe1a00720,
0x80008f64,0xe1a00700,
0x80008f68,0xe3a02000,
0x80008f6c,0xe3510000,
0x80008f70,0x05852034,
0x80008f74,0xe3800b80,
0x80008f78,0x13a01480,
0x80008f7c,0x15851034,
0x80008f80,0xe1d411b6,
0x80008f84,0xe0810000,
0x80008f88,0xe5d41004,
0x80008f8c,0xe0800501,
0x80008f90,0xe5850030,
0x80008f94,0xe59f0050,
0x80008f98,0xe5d01001,
0x80008f9c,0xe5d03008,
0x80008fa0,0xe26110dd,
0x80008fa4,0xe2811e40,
0x80008fa8,0xe2633f9b,
0x80008fac,0xe1811803,
0x80008fb0,0xe5851140,
0x80008fb4,0xe5d01002,
0x80008fb8,0xe5d00009,
0x80008fbc,0xe2611f90,
0x80008fc0,0xe26000be,
0x80008fc4,0xe2800e40,
0x80008fc8,0xe3800c80,
0x80008fcc,0xe1810800,
0x80008fd0,0xe5850158,
0x80008fd4,0xe5852170,
0x80008fd8,0xe5950028,
0x80008fdc,0xe3c00040,
0x80008fe0,0xe5850028,
0x80008fe4,0xe8bd8038,
0x80008fe8,0x80003c78,
0x80008fec,0x80002e34,
0x80008ff0,0xe92d4010,
0x80008ff4,0xe1a04000,
0x80008ff8,0xe5d00006,
0x80008ffc,0xe3a01003,
0x80009000,0xe0810280,
0x80009004,0xe5d41000,
0x80009008,0xe1a00b80,
0x8000900c,0xe0810820,
0x80009010,0xe3800b80,
0x80009014,0xebffff7f,
0x80009018,0xe3a02470,
0x8000901c,0xe5920024,
0x80009020,0xe3800e80,
0x80009024,0xe5820024,
0x80009028,0xe5d40001,
0x8000902c,0xe5d41002,
0x80009030,0xe1800081,
0x80009034,0xe5d41003,
0x80009038,0xe1800181,
0x8000903c,0xe1d411b0,
0x80009040,0xe1800381,
0x80009044,0xe1d411b2,
0x80009048,0xe5d43005,
0x8000904c,0xe1801801,
0x80009050,0xe3a00000,
0x80009054,0xe3530000,
0x80009058,0x05820034,
0x8000905c,0x13a03480,
0x80009060,0x15823034,
0x80009064,0xe5d43005,
0x80009068,0xe1a04002,
0x8000906c,0xe3530002,
0x80009070,0x03811004,
0x80009074,0xe58210f4,
0x80009078,0xe59f1058,
0x8000907c,0xe5d12001,
0x80009080,0xe5d13008,
0x80009084,0xe26220dd,
0x80009088,0xe2822e40,
0x8000908c,0xe2633f9b,
0x80009090,0xe1822803,
0x80009094,0xe3822c80,
0x80009098,0xe5842140,
0x8000909c,0xe5d12002,
0x800090a0,0xe5d11009,
0x800090a4,0xe2622f90,
0x800090a8,0xe26110be,
0x800090ac,0xe2811e40,
0x800090b0,0xe3811c80,
0x800090b4,0xe1821801,
0x800090b8,0xe5841158,
0x800090bc,0xe5840170,
0x800090c0,0xe3a00003,
0x800090c4,0xebffff55,
0x800090c8,0xe5940028,
0x800090cc,0xe3800040,
0x800090d0,0xe5840028,
0x800090d4,0xe8bd8010,
0x800090d8,0x80002e34,
0x800090dc,0xe59f006c,
0x800090e0,0xe5d00000,
0x800090e4,0xe35000ff,
0x800090e8,0x01a0f00e,
0x800090ec,0xe92d4008,
0x800090f0,0xe3a00014,
0x800090f4,0xebffff4b,
0x800090f8,0xe3a00001,
0x800090fc,0xebffff47,
0x80009100,0xe59f004c,
0x80009104,0xe5d01001,
0x80009108,0xe5d02008,
0x8000910c,0xe26110dd,
0x80009110,0xe2811e40,
0x80009114,0xe2622f9b,
0x80009118,0xe1812802,
0x8000911c,0xe3a01470,
0x80009120,0xe5812140,
0x80009124,0xe5d02002,
0x80009128,0xe5d00009,
0x8000912c,0xe2622f90,
0x80009130,0xe26000be,
0x80009134,0xe2800e40,
0x80009138,0xe3800c80,
0x8000913c,0xe1820800,
0x80009140,0xe5810158,
0x80009144,0xe3a00000,
0x80009148,0xe5810170,
0x8000914c,0xe8bd8008,
0x80009150,0x80003c78,
0x80009154,0x80002e34,
0x80009158,0x00000000,
0x40180000,0x00000003
};
void RDA5876_FCC_Test()
{
	RDA_pin_to_low();
	gpio_set_pin_dir(GPIO_BT_LDO_ON,1);
	Fwl_DelayUs(100);
	RDA_pin_to_high();
	
	
	
	RDA5876_Pskey_RfInit();
	RDA_pin_to_low();
	RDA_pin_to_high(); 
	Fwl_DelayUs(50000);
	RDA5876_RfInit();	
	RDA5876_Pskey_RfInit();
	RDA5876_Dccal(); 

	RDA_uart_write_array(RDA5876_FCC_PATCH,sizeof(RDA5876_FCC_PATCH)/sizeof(RDA5876_FCC_PATCH[0]),0);
}
#endif
#endif
