/*******************************************************************************
 * @file I2s.c
 * This file provides I2s APIs: 
 * Copyright (C) 2009 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Chen weiwen
 * @date 2009-09-04
 * @version 1.0
 * @ref AK1020 technical manual.
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
//#include "i2s.h"
//#include "i2s_device.h"
#include "clk.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_I2S > 0)


#define I2S_ADC_CLK     (1 << 27)
#define I2S_DAC_CLK     (1 << 28)

#pragma arm section zidata = "_drvbootbss_"
I2S_MODE  m_i2s_mode;
#pragma arm section zidata

#if (defined(EXTERNAL_ADC_CODEC) || defined(EXTERNAL_DAC_CODEC))
/*********************************************************************
  Function:    I2S
  Description: selete I2S clock
  Input:       type: selete I2S output clock type
  Return:      T_VOID
  Author:      Chenweiwen
  Data:        2009-09-04
**********************************************************************/
static T_VOID I2S_Clk_Sel (T_VOID)
{
    //clear bit 27 - 28
    REG32(REG_CLOCK_DIV2) &= ~(3 << 27);

    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_ADC:
            REG32(REG_CLOCK_DIV2) |= I2S_ADC_CLK;
            break;
        case USE_EXTERNAL_DAC:
            REG32(REG_CLOCK_DIV2) |= I2S_DAC_CLK;
            break;
        default:break;
    }
}

/*********************************************************************
  Function:    I2S
  Description: I2S interface initial
  Input:       T_VOID
  Return:      T_VOID
  Author:      Chenweiwen
  Data:        2009-11-02
**********************************************************************/
static T_VOID I2S_Init (T_VOID)
{
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_ADC:
            //set sharepin as I2S ADC interface
            REG32(REG_SHARE_PIN_CTRL) |= (1 << 13 ); 
            REG32(REG_SHARE_PIN_CTRL) &= ~(1 << 15);

            REG32(REG_ADCS_CTRL) &= ~(ADC_BITS_MAP);// Clear WORD len cfg
            REG32(REG_ADCS_CTRL) |= (ADC_WORD_LAN | L2_MODE_EN  //set word len and L2 mode
                                            | I2S_REC_EN | ADC_16bit_MODE); //16bit one channel

            REG32(REG_CLOCK_DIV2) &= ~(0xff << ADC2_DIV);
            REG32(REG_CLOCK_DIV2) |= (15 << ADC2_DIV);

            break;
        case USE_EXTERNAL_DAC:
            //set sharepin as I2S DAC interface
            REG32(REG_SHARE_PIN_CTRL) |= ((1 << 13 ) | (1 << 15));   

            REG32(REG_I2S_CFG) &= ~(0x1f);  //set 24 bits
            REG32(REG_I2S_CFG) |= (23 << 0);    

            //Normat format, disable MUTE
            REG32(DAC_CFG_REG) &= ~(DATA_FORMAT|CPU_WR_EN|DAC_MUTE_EN); 

            //enalbe dac controller, enable l2 mode, Normat format 
            REG32(DAC_CFG_REG) |= L2_MODE_EN;

            break;
        default:break;
    }
    I2S_Clk_Sel();
    I2S_Device_Init();
}
#endif

/*********************************************************************
  Function:     I2S
  Description:  open I2S device
  Input:        T_VOID
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Open (T_VOID)
{
    I2S_Device_Open();
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_DAC:
            REG32(DAC_CFG_REG) |= DAC_INTERFACE_EN;
            REG32(REG_CLOCK_DIV2) |= DAC_CLK_EN;           //enable DAC clk
            REG32(REG_ANALOG_CTRL2) |= DAC_EN;
            break;
        case USE_EXTERNAL_ADC:
            REG32(REG_CLOCK_DIV2) |= (ADC2_CLK_EN | ADC2_RST);
            REG32(REG_ADCS_CTRL) |= ADC2_EN; //enable ADC2 interface
            break;
        default:break;
    }
}

/*********************************************************************
  Function:     I2S
  Description:  close I2S device
  Input:        T_VOID
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Close (T_VOID)
{
    I2S_Device_Close();
    switch(m_i2s_mode)
    {
        case USE_EXTERNAL_DAC:
            REG32(DAC_CFG_REG) &= ~DAC_INTERFACE_EN;   //disable DAC interface
            REG32(REG_CLOCK_DIV2) &= ~DAC_CLK_EN;    
            break;
        case USE_EXTERNAL_ADC:
            REG32(REG_CLOCK_DIV2) &= ~ADC2_CLK_EN;        //disable ADC2 clk
            REG32(REG_ADCS_CTRL) &= ~ADC2_EN;   //disable ADC2 interface
            break;
        default:break;
    }
}

/*********************************************************************
  Function:     I2S
  Description:  Select line in or Mic in
  Input:        ad_sta: select linein or Micin
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SelectInput(AUDIO_AD_STATE ad_sta)
{
    I2S_Device_SelectInput(ad_sta);
}

/*********************************************************************
  Function:     I2S
  Description:  Set AD/DA sample rate, channel, bitspersample
  Input:        *info: the buffer of some information
  Return:       T_U32: real sample rate
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_U32 I2S_SetInfo(ADDA_INFO *info)
{
    return I2S_Device_SetInfo(info);
}

/*********************************************************************
  Function:     I2S
  Description:  open or close the headphone
  Input:        hp_sta: open or close
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetHeadPhone(HP_STATE hp_sta)
{
    I2S_Device_SetHeadPhone(hp_sta);
}

/*********************************************************************
  Function:     I2S
  Description:  set the codec dac gain
  Input:        gain: HP_GAIN_LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetDACGain (HP_GAIN_LEVEL gain)
{
    I2S_Device_SetDACGain(gain);
}

/*********************************************************************
  Function:     I2S
  Description:  set the codec adc gain
  Input:        ad_sta: linein or mic
                gain: HP_GAIN_LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetADCGain (AUDIO_AD_STATE ad_sta, T_U8 gain)
{
    I2S_Device_SetADCGain(ad_sta, gain);
}

/*********************************************************************
  Function:     I2S
  Description:  config I2S mode
  Input:        mode: config I2S use internal or external device
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-09-04
**********************************************************************/
T_VOID I2S_PCM_Mode_Cfg(T_VOID)
{
#ifdef EXTERNAL_DAC_CODEC
    REG32(REG_MUL_FUNC_CTRL) &= ~INTERNAL_DAC_MODE;
    REG32(REG_MUL_FUNC_CTRL) |= EXTERNAL_DAC_MODE;
    m_i2s_mode = USE_EXTERNAL_DAC;
    I2S_Init();
#else
    #ifdef EXTERNAL_ADC_CODEC
        REG32(REG_MUL_FUNC_CTRL) |= EXTERNAL_ADC_MODE;
        REG32(REG_MUL_FUNC_CTRL) |= INTERNAL_DAC_MODE;
        m_i2s_mode = USE_EXTERNAL_ADC;
        I2S_Init();
    #else
        REG32(REG_MUL_FUNC_CTRL) &= ~EXTERNAL_DAC_MODE;
        REG32(REG_MUL_FUNC_CTRL) |= INTERNAL_DAC_MODE;
        REG32(REG_MUL_FUNC_CTRL) &= ~EXTERNAL_ADC_MODE;
        m_i2s_mode = USE_INTERNAL_DAC;
    #endif
#endif
}

/*********************************************************************
  Function:     I2S
  Description:  Use external codec or not
  Input:        T_VOID
  Return:       I2S_MODE: return I2S mode 
  Author:       Chenweiwen
  Data:         2009-09-04
**********************************************************************/
#pragma arm section code = "_audioplay_pre_" 
I2S_MODE I2S_Is_Use_External_Codec(T_VOID)
{
    return m_i2s_mode;
}
#pragma arm section code

#endif  //(DRV_SUPPORT_I2S > 0)
