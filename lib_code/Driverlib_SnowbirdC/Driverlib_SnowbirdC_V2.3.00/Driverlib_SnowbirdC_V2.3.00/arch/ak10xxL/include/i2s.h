/**
 * @file i2s.h
 * @brief the bits define of I2S controller register
 * This file is the bits define of I2S controller register
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author ZGX
 * @date 2012-11-08
 * @version 1.0
 * @ref AK1180 programme manual.
 */
#ifndef __I2S_H__
#define __I2S_H__


#include "anyka_types.h"
#include "arch_adc.h"
#include "arch_dac.h"


//REG_MUL_FUNC_CTRL(0x04000,0058)
//Bits[26:25] selects the working modes of I2S interface.
#define FADEOUT_FINISH          (1 << 27) //1: The fade out process is finished.The software can insert
                                          //   software reset into DAC and change the OSR register.
#define FADEOUT_FATOR           19        //[23:19]: The slope of DAC output.Fade out = (1/2)^FADEOUT_FATOR
#define FADEOUT_EN              (1 << 18) //1: enable the fade out function of DAC. When this bit is set to 1, 
                                          //   it should't be cleared before the fade out process is finished.
#define I2SSR_MODE              (1 << 26) //1: enable. When external master ADC is used, this mode should be enabled.
#define I2SST_MODE              (1 << 25) //1: enable. When external master DAC is used, this mode should be enabled.
                                          //   When bit[24] is 1, this bit can't be set to 1.
#define I2SI_MODE               (1 << 24) //1: enable. When internal DAC is used, this mode should be enabled.
                                          //   When this bit is 0, bits[26:25] can be 00B, 01B, 10B, 11B.


//DAC_CFG_REG(0x0408,0000)
//The external DAC(s) can accept input from ARM or L2 memory.
//This register selects input of DAC(s), and enable the interrupt function.
#define DAC_DATA_FORMAT         (1 << 4)  //0: normal format, 1: memory saving format
#define CPU_WR_INT_EN           (1 << 3)  //0: disable CPU write interrupt, 1: enable CPU write interrupt
#define CPU_WR_EN               (1 << 3)  //0: disable CPU write interrupt, 1: enable CPU write interrupt
#define DAC_MUTE_EN             (1 << 2)  //0: provide DAC CLK for DAC, 1: inhibit DAC CLK for DAC
#define DAC_L2_MODE_EN          (1 << 1)  //0: disable L2 mode, 1: enable L2 mode
#define DAC_INTERFACE_EN        (1 << 0)  //0: disable DAC interface, 1: enable DAC interface

//DOUBLE_BUF_TRAMI_REG(0x0408,0008)
//This register holds the data written by CPU
//[31:0]: Data written by CPU

//REG_ADCS_CTRL(0x0409,0000)
//This register configures ADC2.
//Note:This register is applicable to both internal and external ADC.
#define ADC2_16BITS_MODE        (1 << 13) //
#define ADC2_WORD_LEN           8         //[12:8]:ADC2 word length = word_len_cfg + 1 (bit)
#define I2S_RECEIVE_EN          (1 << 4)  //0: use internal ADC and the data received is stroed 
                                          //   in ADC2 Receive Data Register 1(0x0409,0004)
                                          //1: use I2S receive(external ADC is used)and the data received
                                          //   is stored in ADC2 Receive Data Register 2(0x0409,0008)
#define ADC2_CHA_POL_SEL        (1 << 3)  //0: receivesent the left channel data when the LRCK is low
                                          //1: receivesent the left channel data when the LRCK is high
#define CPU_RD_INT_EN           (1 << 2)  //0: disable CPU read interrupt, 1: enable CPU read interrupt
#define ADC2_L2_MODE_EN         (1 << 1)  //0: disable L2 mode, 1: enable L2 mode
#define ADC2_EN                 (1 << 0)  //0: disable ADC2 controller, 1: enable ADC2 controller


//ADC2_REC_DATA_REG1(0x0409,0004)
//This register holds the data received from the internal ADC.
//[31:16]: Data receive from ADC2.
//[15:0] : These bits are reserved and should be read as 0s.


//ADC2_REC_DATA_REG2(0x0409,0008)
//This register holds the data received from the external ADC.
//[31:0] : Data receive from external ADC.


typedef enum
{
    ADC_CLOCK = 0,
    DAC_CLOCK,
    HIGHT_LEVEL
}I2S_CLOCK;

typedef enum
{
    USE_INTERNAL_DAC = 0,
    USE_EXTERNAL_DAC,
    USE_INTERNAL_ADC,
    USE_EXTERNAL_ADC
}I2S_MODE;

#define INTERNAL_DAC_MODE   (1 << 24)
#define EXTERNAL_DAC_MODE   (1 << 25)
#define EXTERNAL_ADC_MODE   (1 << 26)
#if DRV_SUPPORT_I2S > 0
/*********************************************************************
  Function:     I2S
  Description:  open I2S device
  Input:        T_VOID
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Open (T_VOID);


/*********************************************************************
  Function:     I2S
  Description:  close I2S device
  Input:        T_VOID
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_Close (T_VOID);


/*********************************************************************
  Function:     I2S
  Description:  Select line in or Mic in
  Input:        ad_sta: select linein or Micin
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SelectInput(AUDIO_AD_STATE ad_sta);


/*********************************************************************
  Function:     I2S
  Description:  Set AD/DA sample rate, channel, bitspersample
  Input:        *info: the buffer of some information
  Return:       T_U32: real sample rate
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_U32 I2S_SetInfo(ADDA_INFO *info);


/*********************************************************************
  Function:     I2S
  Description:  open or close the headphone
  Input:        hp_sta: open or close
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetHeadPhone(HP_STATE hp_sta);


/*********************************************************************
  Function:     I2S
  Description:  config I2S mode
  Input:        T_VOID
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-09-04
**********************************************************************/
T_VOID I2S_PCM_Mode_Cfg(T_VOID);


/*********************************************************************
  Function:     I2S
  Description:  Use external codec or not
  Input:        T_VOID
  Return:       I2S_MODE: return I2S mode 
  Author:       Chenweiwen
  Data:         2009-09-04
**********************************************************************/
I2S_MODE I2S_Is_Use_External_Codec(T_VOID);


/*********************************************************************
  Function:     I2S
  Description:  set the codec dac gain
  Input:        gain: HP_GAIN_LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetDACGain (HP_GAIN_LEVEL gain);


/*********************************************************************
  Function:     I2S
  Description:  set the codec adc gain
  Input:        ad_sta: linein or mic
                gain: HP_GAIN_LEVEL
  Return:       T_VOID
  Author:       Chenweiwen
  Data:         2009-11-02
**********************************************************************/
T_VOID I2S_SetADCGain (AUDIO_AD_STATE ad_sta, T_U8 gain);
#endif

#endif

