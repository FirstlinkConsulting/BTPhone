/**
 * @belief: transmit CS42L52 CODEC specific functions to i2s.c
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ChenWeiwen
 * @date 2009-11-02
 * @version 1.0
 * @NOTE
 * ADC/DAC Codec
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "arch_gpio.h"
//#include "i2s_device.h"
#include "arch_i2c.h"
#include "arch_adc.h"
#include "arch_dac.h"
#include "arch_timer.h"
#include "clk.h"
#include "arch_init.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_I2S > 0)&&(I2S_CS42L52 > 0)


#define DEVICE_NAME         "I2S:CS42L52"

#define CODECADDR_WR        (0X94)
#define CODECADDR_RD        (0X95)

#define READ_ID_ADDR        (0x01)

extern I2S_MODE m_i2s_mode;

static T_BOOL I2S_Write_Reg(T_U8 addr, T_U8 val)
{
    T_U8 buf[2];
    T_BOOL ret = AK_TRUE;
    
    //cs42l52 slave address
    buf[0] = addr;
    buf[1] = val;
    ret = i2c_write_data(CODECADDR_WR, buf, 2);
    return ret;
}

static T_U8 I2S_Read_Reg(T_U8 addr)
{
    T_U8 buf[2];
    T_BOOL ret = AK_TRUE;
    
    buf[0] = addr;
    
    ret = i2c_read_data_extend(CODECADDR_WR, buf, 1);
    
    return buf[1];
}

#if 0
static T_U8 I2S_Read_ID(T_VOID)
{
    return I2S_Read_Reg(0x01);
}
#endif

T_VOID I2S_Device_Init (T_VOID)
{
    drv_print(DEVICE_NAME, 0, AK_TRUE);

    //dac_clk = 12MHz
    REG32(REG_CLOCK_DIV2) &= ~(0xff << 13);
    REG32(REG_CLOCK_DIV2) |= (15 << 13);
    gpio_set_pin_level(GPIO_CODEC_RST, 0);
}

T_VOID I2S_Device_Open (T_VOID)
{
    T_U8 RegTmp;

    gpio_set_pin_level(GPIO_CODEC_RST, 1);
    delay_ms(10);
    
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_DAC:
            I2S_Write_Reg(0x06, 0xa4);      //configure as master, 24bit
            I2S_Write_Reg(0x07, 0x00);
            I2S_Write_Reg(0x0d, 0xa0);
            I2S_Write_Reg(0x0e, 0x06);
            
            I2S_Write_Reg(0x0f, 0x00);     
            
            I2S_Write_Reg(0x1a, 0x0);
            I2S_Write_Reg(0x1b, 0x0);
            I2S_Write_Reg(0x34, 0x5f);
            
            #if 0
            I2S_Write_Reg(0x1c, 0x7f);  //set beep
            I2S_Write_Reg(0x1d, 0x1e);
            I2S_Write_Reg(0x1e, 0x80);
            I2S_Write_Reg(0x22, 0x00);
            I2S_Write_Reg(0x23, 0x00);
            #endif

            //initialization setting
            I2S_Write_Reg(0x00, 0x99);
            I2S_Write_Reg(0x3e, 0xba);
            I2S_Write_Reg(0x47, 0x80);  
            RegTmp = I2S_Read_Reg(0x32);
            RegTmp |= (1 << 7);
            I2S_Write_Reg(0x32, RegTmp);
            RegTmp &= ~ (1 << 7);
            I2S_Write_Reg(0x32, RegTmp);
            I2S_Write_Reg(0x00, 0x00);
            

            I2S_Write_Reg(0x02, 0xfe);      //Power ctrl1
            break;
        case USE_EXTERNAL_ADC:
            //I2S_Write_Reg(0x03, 0x00);        //power on mic
            I2S_Write_Reg(0x04, 0xff);      //power off hp and sp
            I2S_Write_Reg(0x06, 0xa7);      //configure as master, 16bit
            I2S_Write_Reg(0x07, 0x00);

            //initialization setting
            I2S_Write_Reg(0x00, 0x99);
            I2S_Write_Reg(0x3e, 0xba);
            I2S_Write_Reg(0x47, 0x80);  
            RegTmp = I2S_Read_Reg(0x32);
            RegTmp |= (1 << 7);
            I2S_Write_Reg(0x32, RegTmp);
            RegTmp &= ~ (1 << 7);
            I2S_Write_Reg(0x32, RegTmp);
            I2S_Write_Reg(0x00, 0x00);
            

            I2S_Write_Reg(0x02, 0x00);      //Power on PGA and ADC
            break;
        default:break;
    }
}

T_VOID I2S_Device_Close (T_VOID)
{
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_DAC:
            I2S_Write_Reg(0x1b, 0x80);      //mute pcm
            I2S_Write_Reg(0x0f, 0xf0);      //mute hp and sp
            I2S_Write_Reg(0x02, 0xff);      //Power ctrl1
            gpio_set_pin_level(GPIO_CODEC_RST, 0);
            break;
        case USE_EXTERNAL_ADC:
            I2S_Write_Reg(0x0c, 0x03);      //mute adc
            I2S_Write_Reg(0x02, 0xff);      //Power ctrl1
            gpio_set_pin_level(GPIO_CODEC_RST, 0);
            break;
        default:break;
    }
}

T_VOID I2S_Device_SelectInput(AUDIO_AD_STATE ad_sta)
{
    switch(ad_sta)
    {
        case AD_LINE_IN_ON:
            I2S_Write_Reg(0x03, 0x07);      //power off mic
            I2S_Write_Reg(0x08, 0x81);      //select adc input: linein
            I2S_Write_Reg(0x09, 0x81);
            I2S_Write_Reg(0x10, 0x2e);      //use mic1 differential
            I2S_Write_Reg(0x11, 0x2e);
            break;
        case AD_MIC_IN_ON:
            I2S_Write_Reg(0x03, 0x00);      //power on mic
            I2S_Write_Reg(0x08, 0x90);      //select adc input: mic
            I2S_Write_Reg(0x09, 0x90);
            break;
        case AD_DAC_IN_ON:
            break;
        default:
            break;
    }
}


/*********************************************************************
  Function:     I2S
  Description:      
  Input:            
  Return:           
  Author:           Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_U32 I2S_Device_SetInfo(ADDA_INFO *info)
{
    T_U8 reg_val = 0;
    T_U32 ret_sp = info->nSampleRate;

    if(192 == sys_get_pll())
    {
        //dac_clk = 12MHz
        REG32(REG_CLOCK_DIV2) &= ~(0xff << 13);
        REG32(REG_CLOCK_DIV2) |= (15 << 13);
        
        //just for MCLK = 12MHz
        switch(info->nSampleRate)
        {
            case  8000:reg_val = 0x72;
                break;
            case 11025:reg_val = 0x66;
                ret_sp = 11029;
                break;
            case 12000:reg_val = 0x62;
                break;
            case 16000:reg_val = 0x52;
                break;
            case 22050:reg_val = 0x46;
                ret_sp = 22059;
                break;
            case 24000:reg_val = 0x42;
                break;
            case 32000:reg_val = 0x32;
                break;
            case 44100:reg_val = 0x26;
                ret_sp = 44118;
                break;
            case 48000:reg_val = 0x22;
                break;
            case 88200:reg_val = 0x06;
                ret_sp = 88235;
                break;
            case 96000:reg_val = 0x02;
                break;
            default:drv_print("codec not support spr: ", info->nSampleRate, 1);
                break;
        }
    }
    else
    {
        //dac_clk = 27.5MHz
        REG32(REG_CLOCK_DIV2) &= ~(0xff << 13);
        REG32(REG_CLOCK_DIV2) |= (7 << 13);

        switch(info->nSampleRate)
        {
            case  8000:reg_val = 0x7a;
                break;
            case 12000:reg_val = 0x6a;
                break;
            case 16000:reg_val = 0x5a;
                break;
            case 22050:reg_val = 0x4e;
                ret_sp = 22059;
                break;
            case 24000:reg_val = 0x4a;
                break;
            case 32000:reg_val = 0x3a;
                break;
            case 44100:reg_val = 0x2e;
                ret_sp = 44118;
                break;
            case 48000:reg_val = 0x2a;
                break;
            default:drv_print("codec not support spr: ", info->nSampleRate, 1);
                break;
        }
    }
    
    I2S_Write_Reg(0x05, reg_val);       //cking Ctrl
            
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_DAC:
            break;
        case USE_EXTERNAL_ADC:
            if(1 == info->nChannel)
            {
                I2S_Write_Reg(0x10, 0x2d);      //use mic1 differential
                I2S_Write_Reg(0x11, 0x2d);
            }
            else
            {
                I2S_Write_Reg(0x10, 0x2b);      //use mic1 differential
                I2S_Write_Reg(0x11, 0x2b);
            } 
            break;
        default:break;
    }
    return ret_sp;
}


/*********************************************************************
  Function:     I2S
  Description:      
  Input:            
  Return:           
  Author:           Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Device_SetHeadPhone(HP_STATE hp_sta)
{
    switch(hp_sta)
    {
    case HEADPHONE_ON:  //HEADPHONE_ON
        I2S_Write_Reg(0x04, 0xaa);      //power ctrl3
        break; 

    default:  // close headphone
        I2S_Write_Reg(0x04, 0x00);      //power ctrl3
        break;
    }
}

T_VOID I2S_Device_SetDACGain (HP_GAIN_LEVEL gain)
{   
    switch(gain)
    {
        case HPGAIN_LEVEL1:
            I2S_Write_Reg(0x0d,0x60);
            break;
            
        case HPGAIN_LEVEL2:
            I2S_Write_Reg(0x0d,0x00);
            break;
            
        default://HPGAIN_LEVEL0
            I2S_Write_Reg(0x0d,0xe0);
            break;
    }

}

/*********************************************************************
  Function:     I2S driver
  Description:  set the codec adc gain
  Input:        ad_sta: linein or mic
                gain: MIC or LINEIN GAIN LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Device_SetADCGain (AUDIO_AD_STATE ad_sta, T_U8 gain)
{
    T_U16 pga_gain = 0xc0;
    
    switch(ad_sta)
    {
        case AD_LINE_IN_ON:
            if(gain&0x0f < 7)
            {
                pga_gain |= (1 << 5);
                pga_gain |= (24 + gain&0x0f);
            }
            else
            {
                pga_gain |= (gain&0x0f - 7);
            }
            I2S_Write_Reg(0x12, pga_gain);      //set PGA gain
            I2S_Write_Reg(0x13, pga_gain);      //set PGA gain
            break;
        case AD_MIC_IN_ON:
            
            I2S_Write_Reg(0x12, 0xc6);      //set PGA gain
            I2S_Write_Reg(0x13, 0xc6);      //set PGA gain
            break;
        case AD_ALL_OFF:  
            break;
        default:
            break; 
    } 
}


#endif  //(DRV_SUPPORT_I2S > 0)&&(I2S_CS42L52 > 0)

