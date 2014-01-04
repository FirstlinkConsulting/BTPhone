
/*************************************************************************
 *
 * MODULE NAME:    rda5876_drv_init.c
 * PROJECT CODE:   rdabt
 * DESCRIPTION:    rda5876 bt drv.
 * MAINTAINER:     
 * CREATION DATE:  
 *
 * @Author：LHD
 * @Date：
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
#include "hal_radio.h"
#include "arch_gpio.h"
#include "i2c.h"

#include "drv_cfg.h"

#if (DRV_SUPPORT_RADIO > 0) && (RADIO_RDA5876 > 0)
//#ifdef SUPPORT_RADIO_PLAY

typedef unsigned int        uint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef struct 
{
    T_U16 pll;
    T_BOOL stereo;
    T_BOOL stc;
    T_BOOL fm_ready;
    T_BOOL fm_true;
}T_RadioRead5876;

#define READ    1
#define WRITE   0
#define RADIO_DEBUG          0

#define ADRW 0x20
#define ADRR 0x21
#define FM_SINGLE_REG_ADRW  0x22
#define FM_SINGLE_REG_ADRR  0x23
#define DELAY(DURATION)     delay_us(DURATION)   //{unsigned int i; for(i = 1; i <= DURATION; i++){}}
#define LWORD(data)     (data& 0x00ff)
#define HWORD(data)     (data >> 8)
#define MAKEBOOL(value)  (value)?AK_TRUE: AK_FALSE;

#define FM_CLK_5876          CLK_24M

#define RDA_RF_ADRW 0x2c
#define RDA_RF_ADRR 0x2d

#define RDA_CORE_ADRW 0x2a
#define RDA_CORE_ADRR 0x2b


typedef struct
{
    uint8   address;
    uint16  value;
}RDA_FM_REG_T; 

typedef enum 
{
    CLK_32M= 0,  //32.768
    CLK_12M= 1,
    CLK_24M= 5,
    CLK_13M= 2,
    CLK_26M= 6,
    CLK_19M= 3,  //19.2
    CLK_38M= 7    //38.4        
}T_FM5876ClkMode;

volatile uint8 Fm_is_ats = 0;

/************************************
*************************************/



#define DURATION_INIT_1 	30//600ns 
#define DURATION_INIT_2		30//600ns 
#define DURATION_INIT_3 	30//600ns 

#define DURATION_START_1	15//600ns 
#define DURATION_START_2	15//600ns 
#define DURATION_START_3	20//800ns 

#define DURATION_STOP_1		20//800ns 
#define DURATION_STOP_2		15//600ns 
#define DURATION_STOP_3		25//1300ns

#define DURATION_HIGH			8//900ns 
#define DURATION_LOW			20//1600ns

#define POWER_SETTLING		5*1000*80//80ms//50~100ms

#define MIN_JAP_FREQ      76000000
#define MIN_EU_FREQ       87000000

T_VOID GPIO_ModeSetup(uint16 pin, uint16 conf_dada);
T_VOID GPIO_InitIO(char direction, char port);
T_VOID GPIO_WriteIO(char data,char port);
char GPIO_ReadIO(char port);
//static T_BOOL WriteData5876(T_U16 data[], T_U8 size);
//static T_BOOL ReadData5876(T_RadioRead5876 *pRadioRead);

/************************************
*************************************/
 
//1.8V 32K clock support 
//only 5802(TSMC)/5802E
//32K--->0R--->5802/5802E
//#define LOW_AMPLITUDE_32K_SUPPORT

/***************************************************
Extern Definitions and Types
****************************************************/
/***************************************************
Serial communication interfaces 
****************************************************/
T_VOID SerialCommInit(void);
T_VOID SerialCommDeInit(void);
T_VOID SerialCommCryClkOn(void);
T_VOID SerialCommCryClkOff(void);
T_VOID SerialCommRxByte(uint8 *data, uint8 ack);
T_VOID SerialCommStop(void);
T_VOID SerialCommStart(void);

uint8 SerialCommTxByte(uint8 data);
uint8 OperationRDAFM_2w(uint8 operation, uint8 *data, uint8 numBytes);
uint16 RDAFM_GetChipID(void);
uint8   RDAFM_write_data(uint8 regaddr, uint16 *data, uint8 datalen);       ////only 5803/5820
uint8   RDAFM_read_data(uint8 regaddr, uint16 *data, uint8 datalen);        ////only 5803/5820
/***************************************************
Local Definitions and Types
****************************************************/
/***************************************************
RDA5802 interfaces
****************************************************/
void  RDA5802_PowerOnReset(void);
void  RDA5802_PowerOffProc(void);

T_VOID  RDA5802_ChipInit(void);
T_VOID  RDA5802_Mute(uint8 mute);
T_VOID  RDA5802_SetFreq( uint32 curf );
T_VOID  RDA5802_SetVolumeLevel(uint8 level);
T_VOID  RDABT_5876_fm_patch_on(void);

uint16  RDA5802_FreqToChan(uint16 frequency);
uint8 RDA5802_ValidStop(uint16 freq);
uint8 RDA5802_GetSigLvl( uint16 curf );
uint8 RDA5802_Get_stereo_mono(void);
uint8 RDA5802_Intialization(void);


/***************************************************
Local variables
****************************************************/
static uint16 RDAFMChipID;
static uint8 FMshadowReg[90];

/***************************************************
MTK API
****************************************************/

/**
 * @brief   radio_init_rda5876(T_VOID)初始化RDA
 * @author  LHD
 * @date    2012-02-16
 * @param   T_VOID.
 * @return  T_BOOL
 */

T_BOOL radio_init(T_VOID)
{
    RDAFMChipID=0;  

//  delay_us(50000);

    RDAFMChipID=RDAFM_GetChipID();  
    AkDebugOutput("#FM->:RDAFMChipID:%x\n",RDAFMChipID);

    if(0x5802==RDAFMChipID)
    {
        RDA5802_PowerOnReset();
        return AK_TRUE;
    }
    else
        return AK_FALSE;
}

/**
 * @brief   radio_exit_rda5876(T_VOID)退出RDA模块
 * @author  LHD
 * @date    2012-02-16
 * @param   T_VOID
 * @return  T_BOOL
 */
T_BOOL radio_exit(T_VOID)
{
    if(0x5802==RDAFMChipID)
    {
        RDA5802_PowerOffProc();
    }

    SerialCommDeInit();
    
    return AK_TRUE;
}

/**
 * @brief   设置是否静音
 * @author  LHD
 * @date    2012-02-16
 * @param   T_U8 MUTE
 * @return  T_BOOL
 */
void  FMDrv_Mute(uint8 mute)
{
    if(0x5802==RDAFMChipID)
    {
        RDA5802_Mute(mute);
    }
}

/**
 * @brief   FMDrv_ValidStop初始化RDA
 * @author  LHD
 * @date    2012-02-16
 * @param   自动搜索时，作为判定条件，再从中选择信号最强的9个台
 * @return  T_BOOL
 */
uint8 FMDrv_ValidStop(uint16 freq, uint8 signalvl, T_BOOL is_step_up)  /* 自动搜索时，作为判定条件，再从中选择信号最强的9个台*/
{
    T_U8 result =0;
    if(0x5802==RDAFMChipID)
    {       
        result=RDA5802_ValidStop(freq);
    }
    
    return result;
}

uint8 FMDrv_GetSigLvl( uint16 curf )  /*当满足rssi 的条件时，将信号记录，再选最强的9个频段*/
{  
    uint8 result = 0;   
    if(0x5802==RDAFMChipID)
    {   
        result=RDA5802_GetSigLvl(curf);
    }

    return result;
}

T_BOOL radio_check(T_VOID)
{
    return AK_FALSE;
}

uint16 FMDrv_ReadByte(uint8 addr)
{
    return 1;
}

uint8 FMDrv_WriteByte(uint8 addr, uint16 data)
{
    return 1;
}

const unsigned short rdabt_587X_fm_init[][2] = 
{
{0x3f,0x0001},//page 1
{0x2D,0x002A},//page 1
{0x3f,0x0000},//page 1
};

const unsigned short rdabt_587X_fm_deinit[][2] = 
{
{0x3f,0x0001},//page 1
{0x2D,0x00AA},//page 1
{0x3f,0x0000},//page 1
};

/**
 * @brief   RDA_bt_SerialCommTxByte蓝牙I2C设置。
 * @author  LHD
 * @date    2012-02-16
 * @param   Data 要发送的数据
 * @return  uint8
 */
uint8 RDA_bt_SerialCommTxByte(uint8 data) /* return 0 --> ack */
{
    T_S32 i;
    uint8 temp_value = 0;
    for(i=7; (i>=0)&&(i<=7); i--){
       gpio_set_pin_level(GPIO_I2C_SCLK,0); /* low */
       DELAY(DURATION_LOW);
       if(i==7)gpio_set_pin_dir(GPIO_I2C_SDA,1);
       DELAY(DURATION_LOW/2);
       gpio_set_pin_level(GPIO_I2C_SDA,((data>>i)&0x01));
       DELAY(DURATION_LOW/2);
       gpio_set_pin_level(GPIO_I2C_SCLK,1); /* high */
       DELAY(DURATION_HIGH);
    }
    gpio_set_pin_level(GPIO_I2C_SCLK,0);/* low */  
    DELAY(DURATION_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA,0);
    DELAY(DURATION_LOW/2);
    gpio_set_pin_level(GPIO_I2C_SCLK,1);
    DELAY(DURATION_HIGH);    
    temp_value = gpio_get_pin_level(GPIO_I2C_SDA);
    gpio_set_pin_level(GPIO_I2C_SCLK,0);
    DELAY(DURATION_LOW);
    return temp_value;
}

void rdabt_iic_rf_write_data(unsigned char regaddr, unsigned const short *data, unsigned char datalen)
{
    unsigned char i=0;
    
    SerialCommStart();///start
    RDA_bt_SerialCommTxByte(RDA_RF_ADRW);//chip adress
    RDA_bt_SerialCommTxByte(regaddr);

    for(i=0;i<datalen;i++,data++)//data
    {
        RDA_bt_SerialCommTxByte(*data>>8);
        RDA_bt_SerialCommTxByte(*data);
    }
    SerialCommStop();
}

void rdabt_iic_write_data(unsigned char regaddr, unsigned short *data, unsigned char datalen)
{
    unsigned char i=0;
    
    SerialCommStart();///start
    SerialCommTxByte(RDA_RF_ADRW);//chip adress
    SerialCommTxByte(regaddr);

    for(i=0;i<datalen;i++,data++)//data
    {
        SerialCommTxByte(*data>>8);
        SerialCommTxByte(*data);
    }
    SerialCommStop();
}


void RDABT_5876_fm_patch_on(void)
{
    unsigned char i=0;
    
    for(i =0;i<sizeof(rdabt_587X_fm_init)/sizeof(rdabt_587X_fm_init[0]); i++)
    {
        rdabt_iic_rf_write_data(rdabt_587X_fm_init[i][0],&rdabt_587X_fm_init[i][1],1);
    }
    DELAY(10000); 
}

/***************************************************
RDA5802
****************************************************/
const uint8 RDA5802_initialization_reg[]={
0xc4,0x01, //02H: 30pin input: LNA_port_sel<1:0>01 and antenna bit set to 1
0x1b,0x90,// 98MHz                                                                                                                                                                                                               
0x04,0x00,                                                                                                                  
0x86,0xAF, //05H: 0x86 0x68 阈值// 30pin input: LNA_port_sel<1:0>01  and antenna set to 1                                                                            
0x80,0x00,                                                                                                                  
0x5E,0xc6,//0x5F, 0x12,//xiaoyifeng softblend 0x47, 0x12,              
0x5e,0xc6,           
};

const uint8 RDA5802E_initialization_reg[]={
0xC4,0x01, //02H:
0x1b,0x90,
0x04,0x00,
0x86,0xad, //05H://86
0x80,0x00,
0x5F,0x1A, //07H  change the seek time;
0x5e,0xc6,
0x00,0x00,
0x40,0x6e, //0AH: 
0x2d,0x80,
0x58,0x03,
0x58,0x04,
0x58,0x04,
0x58,0x04,
0x00,0x47, //10H: 
0x90,0x00,
0xF5,0x87,
0x7F,0x0B, //13H: reset ADC's parameters
0x04,0xF1,
0x5E,0xc0, //15H: 0x42, 0xc0
0x41,0xe0,
0x50,0x6f,
0x55,0x92,
0x00,0x7d,
0x10,0xC0, //1AH
0x07,0x80,
0x41,0x1d, //1CH,
0x40,0x06,
0x1f,0x9B,
0x4c,0x2b, //1FH. 
0x81,0x10, //20H: 
0x45,0xa0, // 21H
0x19,0x3F, //22H: change AFC threshold
0xaf,0x40, 
0x06,0x81,
0x1b,0x2a, //25H
0x0D,0x04, //26H, shutdown Rssi_autodown function
0x80,0x9F, //0x80, 0x2F, 
0x17,0x8A,
0xD3,0x49,
0x11,0x42,
0xA0,0xC4, //2BH
0x3E,0xBB,
0x00,0x00,
0x58,0x04,
0x58,0x04, //2FH
0x58,0x04,
0x00,0x74,
0x3D,0x00,
0x03,0x0C,
0x2F,0x68,
0x38,0x77, //35H
};
const uint8 RDA5802H_initialization_reg[]={
0xc4,0x01, //02h:         
0x20,0xa0,
0x04,0x00,
0x86,0xad, //05h://86    
0x40,0x00,
0x56,0xc6,
0x00,0x00,
0x00,0x40,
0x00,0x00,  //0x0ah
0x00,0x00,
0x00,0x00,
0x00,0x00,
0x00,0x00,
0x00,0x00,
0x00,0x06,  //0x10h
0x00,0x19,// //0x00,0x09,//0830
0x2a,0x11,
0x00,0x2e,
0x2e,0x30,
0x88,0x3c,  //0x15h
0x90,0x00,
0x2a,0x91,
0x84,0x12,
0x00,0xa8,
0xc4,0x00,  //0x1ah
0xe0,0x00,
0x30,0x1d,//0x24,0x9d,1ch
0x81,0x6a,
0x46,0x08,
0x00,0x86,  //0x1fh
0x06,0x61,// 0x16,0x61, 20h
0x00,0x00,
0x10,0x9e,
0x24,0xc9,//   0x24,0x47,//0830//23h
0x04,0x08,//0830
0x06,0x08,  //0x06,0x08,//0830  //0x25h
0xe1,0x05,
0x3b,0x6c,
0x2b,0xec,
0x09,0x0f,
0x34,0x14,  //0x2ah
0x14,0x50,
0x09,0x6d,
0x2d,0x96,
0x01,0xda,
0x2a,0x7b,
0x08,0x21,   //0x30h
0x13,0xd5,
0x48,0x91,
0x00,0xbc,
0x08,0x96,//0830  0x34h
0x15,0x3c,
0x0b,0x80,
0x25,0xc7,
0x00,0x00,//38h
};
const uint8 RDA5876_5870p_initialization_reg[]={
0xc4,0x05,//02h
0x1a,0x00,
0x04,0x00,
#if 1 //def __RDA_CHIP_R12_5876__
0x8f,0x6d,//05h
#else
0x88,0xad,//05h
#endif
0x60,0x00,
0x42,0xc6,
0x00,0x00,
0x00,0x00,
0x44,0x68,
0x5d,0x8f,
0x00,0x00,
0x00,0x00,
0x00,0x00,
0x00,0x00,
0xf0,0x17,//10h
0x47,0x79,
0x2d,0x00,
0xb0,0x42,
0x22,0x11,
0xf8,0x31,//15h
0xc3,0xc4,
0x1f,0xce,
0x95,0x00,//0x94,0x12,//18h
0x00,0xa8,
0xc4,0x00,
0xe7,0xdf,
0x34,0xdc,
0x81,0x6a,
0x46,0x08,
0x00,0x86,
0x06,0x61,//20h
0x00,0x00,
0x10,0x9e,
0x23,0xc8,
0x04,0x06,
0x06,0x0b,//25h
0xe1,0x05,
0xbb,0x6c,
0x2b,0xec,
0x09,0x0f,
0x34,0x14,
0x14,0x50,
0x09,0x6d,
0x2d,0x96,
0x01,0xda,
0x2a,0x7b,
0x08,0x21,//30h
0x13,0xd5,
0x48,0xf1,
0x00,0xbc,
0x08,0x20,
0x0a,0x24,//35h
0x0b,0x82,
0x25,0xc7,
0x2b,0xe0,
0x3e,0xdd,
0x2c,0xb9,
0x0c,0x95,
0x08,0x8d,
0x04,0x85,
0x00,0x80,
0x00,0x00,
0x58,0x04,//40h
0x58,0x04,
0x38,0x35,
0x2f,0x2b,
0x27,0x22,
0x1b,0x15,
0x0f,0x08,
0x03,0xfd,
0xfd,0xfd,
0x17,0xaf,
0xff,0xff,
0xff,0x00,
0x2e,0x94,
0xc8,0x4e,
0x00,0xfd,
0xd4,0x98,
};
const uint8 RDA5870E_FM_PATCH_RFPLL[]={
0xc4,0x01, //02h:         bit10 ???? ????1.8v samsung flash
0x20,0xa0,
0x04,0x00,
0x86,0xad, //05h://86    
0x40,0x00,
0x56,0xc6,
0x00,0x00,
0x00,0x00,
};

static uint16 cChipID=0;

void  RDA5802_PowerOnReset(void)
{
    FMshadowReg[1]=0x01;

    delay_us(10000);
    SerialCommInit(); 
    RDA5802_Intialization(); 
}

void  RDA5802_PowerOffProc(void)
{
    FMshadowReg[1]=0x00;
    OperationRDAFM_2w(WRITE, &(FMshadowReg[0]), 2);
}

T_VOID radio_line_in(T_BOOL enable)
{
    
}

T_BOOL radio_set_volume(T_U8 Volume)
{
    //akerror("#FM->:rda5876 V:%d", Volume, 1);
    RDA5802_SetVolumeLevel(Volume&0xf);
    return AK_TRUE;
}


/**
 * @brief   设置频点是否合理
 * @author  LHD
 * @date    2012-02-16
 * @param   PARAM是上层传入的具体参数，autoseek是搜索类型
 * @return  T_BOOL
 */
T_BOOL radio_set_param(T_RADIO_PARM *Param)
{
    T_U32 i,save;
    T_U8 status;

    if ((875 > (Param->Freq/100000))||(1080 < (Param->Freq/100000)))//设置频点是否合理
    {
        akerror("#FM->:set freq error",0,1);
        return AK_FALSE;
    }

    //if (TUNE == autoseek)
    {
        RDA5802_SetFreq(Param->Freq/100000);
        AkDebugOutput("#FM->:cur frep:%d\n",Param->Freq/100000);
    }
    /*else        //进入自动搜台。
    {
        i = Param->Freq/100000;
        save = i;

        if (SEEK_UP == autoseek)
        {
            ++i;       
            AkDebugOutput("#FM->:search+:%d\n",i);
        }
        else
        {
            --i;
            AkDebugOutput("#FM->:search-:%d\n",i);
        }    
        
        while(1)
        {
            if (SEEK_UP == autoseek)
            {
                ++i;     
                if (1080 < i)
                    i = 875;//87.5Mhz
                AkDebugOutput("#FM->:search+:%d\n",i);
            }
            else
            {
                --i;
                if (i < 875)
                    i =1080;
                AkDebugOutput("#FM->:search-:%d\n",i);
            }         
            status = RDA5802_ValidStop(i);
            if (AK_TRUE == status)
            {
                if (AK_TRUE== RDA5802_ValidStop(i))
                {
                    akerror("#FM->:seek finish",0,1);
                    AkDebugOutput("#FM->:vaild frep:%d\n",i);
//                    pRadioParam->CurFreq = i*100000;
                    break;
                }
            }
            
            //if (1080 < i)
            //    i = 875;//87.5Mhz
            //else if (i < 875)
            //    i =1080;
            if (save == i)
            {
                akerror("#FM->:seek finish",0,1);
                break;
            }
        }
    }*/
    return AK_TRUE;
}

void RDA5802_Mute(uint8 mute)
{
    if(mute)
        FMshadowReg[0] &=  ~(1<<6);
    else
        FMshadowReg[0] |=   1<<6;

     OperationRDAFM_2w(WRITE, &(FMshadowReg[0]), 2);
}

uint16 RDA5802_FreqToChan(uint16 frequency) 
{
    uint8 channelSpacing=0;
    uint16 bottomOfBand=0;
    uint16 channel;

    if ((FMshadowReg[3] & 0x0c) == 0x00) 
        bottomOfBand = 870;
    else if ((FMshadowReg[3] & 0x0c) == 0x04)   
        bottomOfBand = 760;
    else if ((FMshadowReg[3] & 0x0c) == 0x08)   
        bottomOfBand = 760; 
    else if ((FMshadowReg[3] & 0x0c) == 0x0c)   
        bottomOfBand = 650;
    if ((FMshadowReg[3] & 0x03) == 0x00) 
        channelSpacing = 1;
    else if ((FMshadowReg[3] & 0x03) == 0x01) 
        channelSpacing = 2;

    channel = (frequency - bottomOfBand) / channelSpacing;
    return (channel);
}

void RDA5802_SetFreq(uint32 curFreq )
{   
    uint8 RDA5802_channel_start_tune[] ={0xC4,0x01,0x00,0x10};  //87.0MHz
    uint16 curChan;

    if(cChipID==0x5808)
    {
        if(curFreq<=950)
        {
            RDA_FM_REG_T RDA5802_REG ={0x15,0x8831}; 
            RDAFM_write_data(RDA5802_REG.address , &RDA5802_REG.value , 1); 
        }
        else
        {
            RDA_FM_REG_T RDA5802_REG ={0x15,0xf831}; 
            RDAFM_write_data(RDA5802_REG.address , &RDA5802_REG.value , 1); 
        }
    }

    
    curChan=RDA5802_FreqToChan(curFreq);
    
    //SetNoMute
    FMshadowReg[0] |=   1<<6;
    
    RDA5802_channel_start_tune[0]=FMshadowReg[0];
    RDA5802_channel_start_tune[1]=FMshadowReg[1];
    RDA5802_channel_start_tune[2]=curChan>>2;
    RDA5802_channel_start_tune[3]=(((curChan&0x0003)<<6)|0x10) | (FMshadowReg[3]&0x0f); //set tune bit
    
    OperationRDAFM_2w(WRITE, &(RDA5802_channel_start_tune[0]), 4);
    delay_us(50);
}

typedef T_VOID (*T_pRADIO_STOP_AUTOSEARCH_CALLBACK)(T_VOID);
typedef T_VOID (*T_pRADIO_PLAY_ONESECOND_CALLBACK)(T_U32 frq);

static T_pRADIO_STOP_AUTOSEARCH_CALLBACK s_radio_stop_auto_search_cb;
static T_pRADIO_PLAY_ONESECOND_CALLBACK s_radio_play_onesecond_cb;


T_VOID radio_auto_search_set_callback(T_pRADIO_STOP_AUTOSEARCH_CALLBACK radio_stop_autosearch_cb,
    T_pRADIO_PLAY_ONESECOND_CALLBACK radio_play_onesecond_cb)
{
    s_radio_stop_auto_search_cb = radio_stop_autosearch_cb;
    s_radio_play_onesecond_cb = radio_play_onesecond_cb;
}
T_BOOL radio_search_freq(T_U32 *data)
{
    T_U32 i=875;//从最低频率开始搜索有效电台。
    T_U8 j =0,status;
    T_BOOL ret = AK_FALSE;
    T_pRADIO_STOP_AUTOSEARCH_CALLBACK radio_stop_autosearch_cb;
    T_pRADIO_PLAY_ONESECOND_CALLBACK radio_play_onesecond_cb;
    
    if (AK_NULL == data)
        return ret;
    
    radio_stop_autosearch_cb = s_radio_stop_auto_search_cb;
    radio_play_onesecond_cb = s_radio_play_onesecond_cb;

    Fm_is_ats = 1;
    
    while(1)
    {
        radio_stop_autosearch_cb();
        
        status = RDA5802_ValidStop(i);
        if (AK_TRUE == status)
        {
        //    if (AK_TRUE== RDA5802_ValidStop(i))
            {
                data[j] = i*100000; //保存当前有效频率。
                j++;
                ret = AK_TRUE;
            }
        //电台有效，播放一秒  
        radio_play_onesecond_cb(i);
        }

        i++;
        if (i > 1080)
        {
            akerror("#FM->:seek over:",j,1);
            break;
        }

        if (j >= 20)
        {
            break;
        }
        if (!Fm_is_ats)
            break;
    }
    if (!Fm_is_ats)
    {
        keypad_enable_scan();
    }
    //RDA5802_Mute(AK_FALSE);
    akerror("#FM->:ats result total:", j, 1);
    return ret;
}

uint8 RDA5802_ValidStop(uint16 freq)  /* 自动搜索时，作为判定条件，再从中选择信号最强的9个台*/
{
    uint8 RDA5802_reg_data[4]={0};  
    uint8 falseStation = 0;
    uint8 i=0;
    uint16 curChan;
    
    if(cChipID==0x5808)
    {
        if(freq<=950)
        {
            RDA_FM_REG_T RDA5802_REG ={0x15,0x8831}; 
            RDAFM_write_data(RDA5802_REG.address , &RDA5802_REG.value , 1); 
        }
        else
        {
            RDA_FM_REG_T RDA5802_REG ={0x15,0xf831}; 
            RDAFM_write_data(RDA5802_REG.address , &RDA5802_REG.value , 1); 
        }
    }

    curChan=RDA5802_FreqToChan(freq);
    
    //SetNoMute
    FMshadowReg[0] |= 1<<6;
    
    RDA5802_reg_data[0]=FMshadowReg[0];
    RDA5802_reg_data[1]=FMshadowReg[1];
    RDA5802_reg_data[2]=curChan>>2;
    RDA5802_reg_data[3]=(((curChan&0x0003)<<6)|0x10) | (FMshadowReg[3]&0x0f);
    OperationRDAFM_2w(WRITE,&(RDA5802_reg_data[0]), 4); 
    delay_us(8000);     

    //waiting for FmReady
    do
    {
        i++;
        if(i>10) 
            return 0; 
        delay_us(1000);
        //read REG0A&0B 
        OperationRDAFM_2w(READ,&(RDA5802_reg_data[0]), 4);  
    }while((RDA5802_reg_data[3]&0x80)==0);
    
    //check FM_TURE
    if((RDA5802_reg_data[2] &0x01)==0) 
    {
        
        falseStation=1;
    }
    if(freq==960) 
        falseStation=1;

    if (falseStation==1)
        return 0;   
    else 
        return 1;
}

T_BOOL radio_get_status(T_RADIO_STATUS *Status, T_U32 FreqMin, T_U32 FreqMax)
{
    return AK_TRUE;
}

uint8 RDA5802_GetSigLvl(uint16 curf)  /*当满足rssi 的条件时，将信号记录，再选最强的9个频段*/
{
    uint8 RDA5802_reg_data[4]={0};
    if(875==curf)
        return(1);
    else
        OperationRDAFM_2w(READ,&(RDA5802_reg_data[0]), 4);
    return (RDA5802_reg_data[2]>>1);  /*返回rssi*/
}

void RDA5802_SetVolumeLevel(uint8 level)   /*一般不调用，即不用芯片来调节音量。*/
{
    FMshadowReg[7]=( FMshadowReg[7] & 0xf0 ) | level; 
    OperationRDAFM_2w(WRITE, &(FMshadowReg[0]), 8);
}

uint8 RDA5802_Get_stereo_mono(void)
{
    uint8 RDA5802_reg_data[4]={0};
    OperationRDAFM_2w(READ,&(RDA5802_reg_data[0]), 4);
    if((RDA5802_reg_data[0])&0x04==0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

T_BOOL RDA5802_Intialization(void)
{
    uint8 error_ind = 0;
    uint8 RDA5802_REG[]={0x00,0x02};
    uint8 RDAFM_reg_data[10]={0};
    uint8 RDA5802_reg_data[4]={0};  
    uint8 i=0;
    uint16 xbTemp;
    cChipID=0;
    RDA5802_REG[0]=0x00;
    RDA5802_REG[1]=0x02;    
    error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
    delay_us(50);
        
    OperationRDAFM_2w(READ,&(RDAFM_reg_data[0]), 10);
    
    cChipID=(RDAFM_reg_data[8]*0x100)+ RDAFM_reg_data[9];
    AkDebugOutput("#FM->:cChipID:%x\n",cChipID);
    if(0x5808==cChipID)
    {
            RDA5802_REG[0]=0xC4;
            RDA5802_REG[1]=0x01;
            error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
            delay_us(50);
            RDA5802_REG[0]=0x00;
            RDA5802_REG[1]=0x02;    
            error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
            delay_us(50);
            RDA5802_REG[0]=0xC4;
            RDA5802_REG[1]=0x01;
            error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
            delay_us(50);
            for(xbTemp=0;xbTemp<32;xbTemp++)
                FMshadowReg[xbTemp]=RDA5876_5870p_initialization_reg[xbTemp];
            error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5876_5870p_initialization_reg[0],sizeof(RDA5876_5870p_initialization_reg));
    }
    else if (0x5801==cChipID)
    {
        RDABT_5876_fm_patch_on();
        
        RDA5802_REG[0]=0xC4;
        RDA5802_REG[1]=0x01;
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);
        RDA5802_REG[0]=0x00;
        RDA5802_REG[1]=0x02;    
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);   
        RDA5802_REG[0]=0xC4;
        RDA5802_REG[1]=0x01;
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);
        for(xbTemp=0;xbTemp<32;xbTemp++)
            FMshadowReg[xbTemp]=RDA5802H_initialization_reg[xbTemp];        
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802H_initialization_reg[0],sizeof(RDA5802H_initialization_reg));

        delay_us(50);
        
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5870E_FM_PATCH_RFPLL[0],sizeof(RDA5870E_FM_PATCH_RFPLL));

        delay_us(50);
        //waiting for FmReady
        do
        {
            i++;
            if(i>10) return 0; 
            
            RDA5802_SetFreq(875);

            delay_us(50);       

            //read REG0A&0B 
            OperationRDAFM_2w(READ,&(RDA5802_reg_data[0]), 4);
            if((RDA5802_reg_data[3]&0x80)==0)
            {
                RDA5802_REG[0]=0xc4;
                RDA5802_REG[1]=0x00;    
                error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
                delay_us(50);
                RDA5802_REG[0]=0xc4;
                RDA5802_REG[1]=0x01;
                error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
                delay_us(50);
            }
        }while((RDA5802_reg_data[3]&0x80)==0);
    }
    else
    {
        cChipID=(RDAFM_reg_data[4]*0x100)+ RDAFM_reg_data[5];   
    }
    if( 0x5802==cChipID || 0x5803==cChipID )
    {   
        RDA5802_REG[0]=0xC4;
        RDA5802_REG[1]=0x01;
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);
        RDA5802_REG[0]=0x00;
        RDA5802_REG[1]=0x02;    
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);
        RDA5802_REG[0]=0xC4;
        RDA5802_REG[1]=0x01;
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_REG[0], 2);
        delay_us(50);
        for(xbTemp=0;xbTemp<32;xbTemp++)
            FMshadowReg[xbTemp]=RDA5802_initialization_reg[xbTemp];     
        error_ind = OperationRDAFM_2w(WRITE, (uint8 *)&RDA5802_initialization_reg[0],sizeof(RDA5802_initialization_reg));
    }
    else
    {
        error_ind = 1;
    }
    
    delay_us(50);   
    
    if (error_ind )
       return 0;
    else
       return 1;
}

void SerialCommStart(void) /* start or re-start */
{
   gpio_set_pin_dir(GPIO_I2C_SCLK,1);
   gpio_set_pin_dir(GPIO_I2C_SDA,1);
   gpio_set_pin_level(GPIO_I2C_SDA,1);
   DELAY(DURATION_START_1);
   gpio_set_pin_level(GPIO_I2C_SCLK,1);
   DELAY(DURATION_START_1);   
   gpio_set_pin_level(GPIO_I2C_SDA,0);
   DELAY(DURATION_START_2);
   gpio_set_pin_level(GPIO_I2C_SCLK,0);/* start condition */
   DELAY(DURATION_START_3);   
}

void SerialCommStop(void)
{
    gpio_set_pin_level(GPIO_I2C_SCLK,0);
    DELAY(DURATION_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA,1);
    gpio_set_pin_level(GPIO_I2C_SDA,0);
    DELAY(DURATION_STOP_1);
    gpio_set_pin_level(GPIO_I2C_SCLK,1);
    DELAY(DURATION_STOP_2);    
    gpio_set_pin_level(GPIO_I2C_SDA,1);/* stop condition */
    DELAY(DURATION_STOP_3);      
}

uint8 SerialCommTxByte(uint8 data) /* return 0 --> ack */
{
   T_S32 i;
   uint8 temp_value = 0;
   for(i=7; (i>=0)&&(i<=7); i--){
      gpio_set_pin_level(GPIO_I2C_SCLK,0); /* low */
      DELAY(DURATION_LOW);
      if(i==7)gpio_set_pin_dir(GPIO_I2C_SDA,1);
      DELAY(DURATION_LOW/2);
      gpio_set_pin_level(GPIO_I2C_SDA,((data>>i)&0x01));
      DELAY(DURATION_LOW/2);
      gpio_set_pin_level(GPIO_I2C_SCLK,1); /* high */
      DELAY(DURATION_HIGH);
   }
   gpio_set_pin_level(GPIO_I2C_SCLK,0);/* low */  
   DELAY(DURATION_LOW);
   gpio_set_pin_dir(GPIO_I2C_SDA,0);
   DELAY(DURATION_LOW/2);
   gpio_set_pin_level(GPIO_I2C_SCLK,1);
   DELAY(DURATION_HIGH);
   
   temp_value = gpio_get_pin_level(GPIO_I2C_SDA);
   DELAY(DURATION_LOW/2);
   gpio_set_pin_level(GPIO_I2C_SCLK,0);
   return temp_value;
}

void SerialCommRxByte(uint8 *data, uint8 ack)
{
   T_S32 i;
   uint32 dataCache;

   dataCache = 0;
   for(i=7; (i>=0)&&(i<=7); i--){
      gpio_set_pin_level(GPIO_I2C_SCLK,0);
      DELAY(DURATION_LOW);
       if(i==7)gpio_set_pin_dir(GPIO_I2C_SDA,0);
      DELAY(DURATION_LOW/2);
      gpio_set_pin_level(GPIO_I2C_SCLK,1);
      DELAY(DURATION_HIGH);
      dataCache |= (gpio_get_pin_level(GPIO_I2C_SDA)<<i);
      DELAY(DURATION_LOW/2);
   }
   
   gpio_set_pin_level(GPIO_I2C_SCLK,0);
   DELAY(DURATION_LOW);
   gpio_set_pin_dir(GPIO_I2C_SDA,1);
   DELAY(DURATION_LOW/2);
   gpio_set_pin_level(GPIO_I2C_SDA,ack);   
   DELAY(DURATION_LOW/2);   
   gpio_set_pin_level(GPIO_I2C_SCLK,1);
   DELAY(DURATION_HIGH);
   *data = (uint8)dataCache;
   gpio_set_pin_level(GPIO_I2C_SCLK,0);
}

void SerialCommInit(void)
{
    gpio_set_pin_dir(GPIO_I2C_SCLK,1);  
    gpio_set_pin_dir(GPIO_I2C_SDA,1);     
    gpio_set_pin_level(GPIO_I2C_SCLK,1);
    gpio_set_pin_level(GPIO_I2C_SDA,1);
}

T_VOID SerialCommDeInit(T_VOID)
{
    gpio_set_pin_dir(GPIO_I2C_SCLK,0);  
    gpio_set_pin_dir(GPIO_I2C_SDA,0);    
}
extern void BtRadio_SetupGPIO(char val, unsigned char pin);
#ifdef __CUST_NEW__
extern kal_uint8 BtRadio_GetLpoMode(uint8 numLpo);
#endif

uint8 OperationRDAFM_2w(uint8 operation, uint8 *data, uint8 numBytes)
{
    uint8 j;
    uint8 acknowledge=0;

    /***************************************************

    START: make sure here SDIO_DIR =OUT, RDABT_SCLK = 1, SDIO = 1

    ****************************************************/
    SerialCommStart();

    /***************************************************

    WRITE CONTROL DATA: make sure here: SLCK = 0; SDIO = 0

    ****************************************************/

    /***************************

    CHECK ACK for control word

    ***************************/

    if(operation == READ)
         acknowledge = SerialCommTxByte(ADRR);
    else 
         acknowledge = SerialCommTxByte(ADRW);
    
    //kal_prompt_trace(MOD_MED_V,"OperationRDAFM_2w   lrjaaaaaa acknowledge =%d ",acknowledge);  


    /***************************************

    WRITE or READ data

    ****************************************/   

    /******************************

    CHECK ACK or SEND ACK=0

    *******************************/

    for (j = 0; j < numBytes; j++, data++)
    {
        if (operation == READ)
        {
            if (j == (numBytes -1))
                SerialCommRxByte(data,1); 
            else
                SerialCommRxByte(data, 0); 
        }
        else
            acknowledge = SerialCommTxByte(*data);
        //kal_prompt_trace(MOD_MED_V,"OperationRDAFM_2w numBytes =%d acknowledge=%d,data=%x",numBytes,acknowledge,*data);  //lrj  add for test 20060522
    }
    /****************************

    STOP: make sure here: RDABT_SCLK = 0

    *****************************/
    SerialCommStop();
   
    return acknowledge;
}

uint8 RDAFM_write_data(uint8 regaddr, uint16 *data, uint8 datalen)
{
    uint8 i=0;
    uint8 acknowledge;
    
    SerialCommStart();///start
    acknowledge=SerialCommTxByte(FM_SINGLE_REG_ADRW);//chip adress
    acknowledge=SerialCommTxByte(regaddr);

    for(i=0;i<datalen;i++,data++)//data
    {
        acknowledge=SerialCommTxByte(*data>>8);
        acknowledge=SerialCommTxByte(*data);
    }
    SerialCommStop();
    return acknowledge;
}

//only 5803/5820
uint8 RDAFM_read_data(uint8 regaddr, uint16 *data, uint8 datalen)
{
    uint8 tempdata;
    uint8 i=0;
    uint8 acknowledge;
    
    SerialCommStart();///start
    acknowledge=SerialCommTxByte(FM_SINGLE_REG_ADRW);//chip adress
    acknowledge=SerialCommTxByte(regaddr);
    
    SerialCommStart();//start
    SerialCommTxByte(FM_SINGLE_REG_ADRR);//chip adress
    
    for( i=0;i<datalen-1;i++,data++)//data
    {
        SerialCommRxByte(&tempdata, 0);
        *data = (tempdata<<8);  
        SerialCommRxByte(&tempdata, 0);         
        *data |= tempdata;      
    }
    
    SerialCommRxByte(&tempdata, 0);
    *data = (tempdata<<8);  
    SerialCommRxByte(&tempdata, 1);         
    *data |= tempdata;  
    
    SerialCommStop();
    return acknowledge;
}

uint16 RDAFM_GetChipID(void)
{
    uint8 RDAFM_reg_data[6]={0};
    uint16 cChipID;
//  uint8 tryCount,bRet;

    SerialCommInit();
    
    gpio_set_pin_level(GPIO_I2C_SCLK,0);
    DELAY(DURATION_LOW);
    gpio_set_pin_level(GPIO_I2C_SCLK,1);
    DELAY(DURATION_LOW*30);
  
    gpio_set_pin_level(GPIO_I2C_SDA,1);
    gpio_set_pin_level(GPIO_I2C_SCLK,1);

    RDAFM_reg_data[0]=0x00;
    RDAFM_reg_data[1]=0x02;

    OperationRDAFM_2w(WRITE,&RDAFM_reg_data[0], 2);
    delay_us(30000);       
    OperationRDAFM_2w(READ,&RDAFM_reg_data[0], 6);
    cChipID=(RDAFM_reg_data[4]*0x100)+ RDAFM_reg_data[5];   
    
    if(0x5802==cChipID || 0x5803==cChipID )
    {
        return(0x5802);
    }
    else
    {
        RDABT_5876_fm_patch_on();
        delay_us(10000);
        OperationRDAFM_2w(WRITE,&(RDAFM_reg_data[0]), 2);
        delay_us(30000);
        OperationRDAFM_2w(READ,&(RDAFM_reg_data[0]), 6);
        cChipID=(RDAFM_reg_data[4]*0x100)+ RDAFM_reg_data[5];
        if(0x5802==cChipID || 0x5803==cChipID )
        {
            return(0x5802);
        }
    }
    return (0xFFFF);        
}
#endif

