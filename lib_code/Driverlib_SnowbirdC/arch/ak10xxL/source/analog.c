/*******************************************************************************
 * @file    arch_analog.h
 * @brief   the interface for the control of mic/hp/linein and so on
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.11.19
 * @version 1.0
*******************************************************************************/
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "arch_analog.h"
#include "arch_timer.h"
#include "interrupt.h"
#include "drv_cfg.h"
#include "l2.h"




#ifndef BURN_TOOL

#define LIN_GAIN_MAX    0xF
#define MIC_GAIN_MAX    0xA
#define HP_GAIN_MAX     0x5


static T_U8 hp_open_flag = 0;
static T_U8 adc_open_flag = 0;
static T_U8 lineout_open_flag = 0;
static T_BOOL linein_fix = AK_FALSE;

#if !(HP_MODE_DC > 0)
static const T_U32 sina_data[] = 
{
   #include "sina_up.h"
};

static const T_U32 sina_down[] = 
{
    #include "sina_down.h"
};

static T_BOOL bsetinfo;


static void output_sina_on(const T_U32 *buf, T_U32 size, T_BOOL blast)
{
    T_U32       i, j, k;
    T_U32       *L2Buf;
    T_U32       len;
    T_U32       id;

    if(AK_FALSE == get_device_buf_info(ADDR_DAC, (T_U32 *)&L2Buf, &len, &id))
    {
        return ;
    }
    
    while(l2_get_status(id));

    REG32(REG_DACS_CFG) &= ~DACS_L2_MODE;
    REG32(REG_DACS_CFG) |= DACS_L2_MODE;    

    if(!bsetinfo)
    {
        dac_setinfo(16000, 2, 16);
        bsetinfo = AK_TRUE;
    } 

    len     = len >> 2;
    size    = size >> 2;

    j = 0;
    for(i = 0; i < size; ++i)
    {
        if(j >= len)
        {
            j = 0;
        }

        L2Buf[j++] = buf[i];

        if((j & 0xF) == 0)
        {
            while(l2_get_status(id) > 6)
            {
                //akerror("0x", REG32(REG_INT_ANALOG) & (ALG_STA_HP_OCP | INT_STA_HP_OCP), AK_TRUE);
                //akerror(".", 0, AK_FALSE);
            };
        }
    }

    if(blast)
    {        
        for(; (j & 0xF) != 0; ++j)
        {
            if(j >= len)
            {
                j = 0;
            }

            L2Buf[j++] = buf[size - 1];
        }
    }

    while(l2_get_status(id));
}
#endif

static void pnmos_open(void)
{
#if (HP_MODE_DC > 0)
    T_U32   reg_value;
    //power on NMOS and PMOS
    reg_value = REG32(REG_ANALOG_CTRL1);
    reg_value |= 0xF << PD_HP_CTRL;
    reg_value &= ~(PD_PMOS | PD_NMOS);
    REG32(REG_ANALOG_CTRL1) = reg_value;

#else

    //Open Nmos    
    REG32(REG_ANALOG_CTRL1) &= ~PD_NMOS;
    delay_ms(1);

    //Fade out HP driver  here

    //open Pmos
#if (CHIP_SEL_10C > 0)
    REG32(REG_ANALOG_CTRL1) |= ((0x07UL<<29)|(1<<18));
#endif    
    REG32(REG_ANALOG_CTRL1) &= ~(0xf << PD_HP_CTRL);
    delay_ms(1);
    REG32(REG_ANALOG_CTRL1) &= ~PD_PMOS;
    delay_ms(1);

    REG32(REG_ANALOG_CTRL1) |= (0x08 << PD_HP_CTRL);
    delay_ms(1);
    
    REG32(REG_ANALOG_CTRL1) |= (0x04 << PD_HP_CTRL);
    delay_ms(1);
    
    REG32(REG_ANALOG_CTRL1) |= (0x02 << PD_HP_CTRL);
    delay_ms(1);
    
    REG32(REG_ANALOG_CTRL1) |= (0x01 << PD_HP_CTRL);
    delay_ms(1);
#if (CHIP_SEL_10C > 0)
    REG32(REG_ANALOG_CTRL1) &= ~((0x07UL<<29));
    delay_ms(1);
    REG32(REG_ANALOG_CTRL1) &= ~(1<<18);
    delay_ms(1);
#endif    
#endif
}

typedef struct 
{
    T_U16 charge_sel;
    T_U16 delay_ms;
}T_VCM2_CHARGE;


static T_VOID VCM2_charging(T_VOID)
{
#if (HP_MODE_DC > 0)
    static const T_VCM2_CHARGE value[] = {
#if 0
        {0x1E , 5},    //5uA
        {0x10 , 2},    //7uA
        {0x0F , 2},    //12uA
        {0x0E , 2},    //15uA
        {0x07 , 2},    //17uA
        {0x0C , 2},    //20uA
        {0x06 , 10},   //24uA
        {0x03 , 5},    //29uA
        {0x08 , 1},    //36uA
        {0x04 , 1},    //42uA
        {0x02 , 1},    //50uA
        {0x01 , 10},   //57uA
        {0x00 , 1},    //200uA
        {0xFF , 1},    //double
#endif
        {0x00 , 1},    //200uA
        {0xFF , 30},   //double
        {0x03 , 1},    //29uA
    };
    T_U32 reg_value;
    T_U32 i;

    drv_print("The DC HP is Used!", 0, 1);

    reg_value = REG32(REG_ANALOG_CTRL1);
    reg_value &= ~((PRE_EN2) | (0x7U << DISCHG_HP) 
                                | (0x1F << PTM_DCHG));
    REG32(REG_ANALOG_CTRL1) = reg_value;

    //power on HPMID
    reg_value &= ~(PD_HPMID);
    REG32(REG_ANALOG_CTRL1) = reg_value;

    //VCM2 charging current selection.
    reg_value = REG32(REG_ANALOG_CTRL3);
    reg_value &= ~(0x1F << PTM_CHG);

    REG32(REG_ANALOG_CTRL3) = reg_value | (value[0].charge_sel << PTM_CHG);
    REG32(REG_ANALOG_CTRL1) &= ~(PL_VCM2 | PD_VCM2);
    delay_ms(value[0].delay_ms);
    for(i=1; i < sizeof(value)/sizeof(value[0]); ++i)
    {
        if(0xFF == value[i].charge_sel)
        {
            REG32(REG_ANALOG_CTRL4) &= ~VCM2_BIAS_NOT_DB;
            delay_ms(value[i].delay_ms);
            REG32(REG_ANALOG_CTRL4) |= VCM2_BIAS_NOT_DB;
            continue;
        }
        REG32(REG_ANALOG_CTRL3) = reg_value | (value[i].charge_sel << PTM_CHG);
        delay_ms(value[i].delay_ms);
    }
#else
    static const T_VCM2_CHARGE value[] = 
    {
#if   0
//org
        {0x1E , 5},    //5uA

        {0x10 , 2},    //7uA
        {0x0F , 2},    //12uA
        {0x0E , 2},    //15uA
        {0x07 , 2},    //17uA
        {0x0C , 2},    //20uA
        {0x06 , 10},   //24uA
        {0x0C , 2},    //20uA
        {0x07 , 2},    //17uA
        {0x0E , 2},    //15uA
        {0x0F , 2},    //12uA
        {0x10 , 2},    //7uA
        {0x1E , 10},   //5uA
        {0x10 , 1},    //7uA

        {0xFF , 1},    //double
        {0x1E , 1},    //5uA
        {0x10 , 1},    //7uA
        {0x0F , 1},    //12uA
        {0x0E , 1},    //15uA
        {0x07 , 1},    //17uA
        {0x0C , 1},    //20uA
        {0x06 , 1},    //24uA
        {0x03 , 1},    //29uA
        {0x08 , 1},    //36uA
        {0x04 , 1},    //42uA
        {0x02 , 1},    //50uA
        {0x01 , 1},    //57uA

      //{0x00 , 0},    //200uA
#else
        {0x1E , 5},    //5uA
        {0x10 , 5},    //7uA
        {0x0F , 30},   //12uA
        {0x10 , 8},    //7uA
        {0x1E , 10},   //5uA
        {0x10 , 1},    //7uA
        {0x1E , 0},    //5uA
#if 0
        {0x10 , 2},    //7uA
        {0x0F , 2},    //12uA
        {0x0E , 2},    //15uA
        {0x07 , 2},    //17uA
      //{0x0C , 2},    //20uA
      //{0x06 , 9},    //24uA
      //{0x0C , 13},   //20uA
      //{0x0C , 2},    //20uA
        {0x07 , 20},   //17uA
        {0x0E , 2},    //15uA
        {0x0F , 2},    //12uA
        {0x10 , 1},    //7uA
        {0x1E , 10},   //5uA
#endif
        {0xFF , 1},    //double

        {0x10 , 1},    //7uA
        {0x0F , 1},    //12uA
        {0x0E , 1},    //15uA
        {0x07 , 1},    //17uA
        {0x0C , 1},    //20uA
        {0x06 , 1},    //24uA
        {0x03 , 1},    //29uA
        {0x08 , 1},    //36uA
        {0x04 , 1},    //42uA
        {0x02 , 1},    //50uA
        {0x01 , 1},    //57uA

      //{0x00 , 0},    //200uA
#endif

#if  0
        {0x0C , 2},    //20uA
        {0x06 , 2},    //24uA
        {0x03 , 2},    //29uA
        {0x08 , 2},    //36uA
        {0x04 , 180},  //42uA
      //{0x02 , 1},    //50uA
      //{0x01 , 1},    //57uA

      //{0x00 , 50},   //200uA

      //{0x01 , 5},    //57uA
      //{0x02 , 5},    //50uA
      //{0x04 , 5},    //42uA
        {0x08 , 5},    //36uA
        {0x03 , 5},    //29uA
        {0x06 , 5},    //24uA
        {0x0C , 5},    //20uA
        {0x07 , 5},    //17uA
        {0x0E , 6},    //15uA
        {0x0F , 7},    //12uA
      //{0x10 , 8},    //7uA
      //{0x1E , 10},   //5uA

      //{0x03 , 0}     //29uA , default value
#endif
    };

    T_U32  reg_value;
    T_U32  i,tmp;
    T_U32  buf[2];
    T_U32  hp;
    T_BOOL bdac_open;

    bsetinfo = AK_FALSE;

    drv_print("The AC HP is Used!", 0, 1);

    if(REG32(REG_CLOCK_RST_EN) & (1 << 5))
    {
        bdac_open = AK_FALSE;
    }
    else
    {
        bdac_open = AK_TRUE;
    }

    if(AK_FALSE == bdac_open)
    {
        dac_open();

        //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
        REG32(REG_ANALOG_CTRL1) &= ~(PD_CK | PD_OP);
        REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
        REG32(REG_ANALOG_CTRL1) &= ~DACS_RST_M_V;
    }

    //VCM2 disable to pull down,
    REG32(REG_ANALOG_CTRL1) &= ~((0x1FU << PTM_DCHG));
    //discharge the off-chip AC coupling capacitor.
    REG32(REG_ANALOG_CTRL1) |= (PRE_EN2) | (0x7U << DISCHG_HP);

    buf[0] = 0x80008000;
    output_sina_on(buf, 4, AK_TRUE);
    REG32(REG_ANALOG_CTRL4) |= ANTI_POP_EN;
    //delay_ms(1); 

    //VCM2 charging current selection.
    reg_value = REG32(REG_ANALOG_CTRL3);
    reg_value &= ~(0x1F << PTM_CHG);

    //5uA
    REG32(REG_ANALOG_CTRL3) = reg_value | (value[0].charge_sel << PTM_CHG);
    REG32(REG_ANALOG_CTRL1) &= ~(PL_VCM2);
    REG32(REG_ANALOG_CTRL1) &= ~(PD_VCM2);

    delay_ms(value[0].delay_ms);
    for(i=1; i < sizeof(value)/sizeof(value[0]); ++i)
    {
        if(0xFF == value[i].charge_sel)
        {
            REG32(REG_ANALOG_CTRL4) &= ~VCM2_BIAS_NOT_DB;
            delay_ms(value[i].delay_ms);
            REG32(REG_ANALOG_CTRL4) |= VCM2_BIAS_NOT_DB;
            continue;
        }
        
        REG32(REG_ANALOG_CTRL3) = reg_value | (value[i].charge_sel << PTM_CHG);

        delay_ms(value[i].delay_ms);
    }


    output_sina_on(sina_data, sizeof(sina_data), AK_FALSE);
    
    //VCM2 bias not double
    REG32(REG_ANALOG_CTRL4) |= VCM2_BIAS_NOT_DB;
    
    //set vcm2 charge 5uA
    reg_value = REG32(REG_ANALOG_CTRL3);
    reg_value &= ~(0x1F << PTM_CHG);
    REG32(REG_ANALOG_CTRL3) = reg_value | (0x1E << PTM_CHG);
    REG32(REG_ANALOG_CTRL4) &= ~ANTI_POP_EN;
    //restory the dac output level


    buf[0] = 0x00000000;
    output_sina_on(buf, 4, AK_TRUE);
    delay_ms(5);

    //pull down resistor of hp ,All disable
    REG32(REG_ANALOG_CTRL1) &= ~(PRE_EN2 | (0x7U << DISCHG_HP));

    if(AK_FALSE == bdac_open)
    {
        REG32(REG_ANALOG_CTRL1) |= PD_CK | PD_OP;
        dac_close();
    }
    else
    {
        REG32(REG_DACS_CFG) &= ~DACS_L2_MODE;
        REG32(REG_DACS_CFG) |= DACS_L2_MODE;
    }

    
    bsetinfo = AK_FALSE;
#endif
}


static T_VOID VCM2_discharging(T_VOID)
{
#if (HP_MODE_DC > 0)
    static const T_VCM2_CHARGE value[] = 
    {
      //{0x01 , 1},    //2.5uA
        {0x03 , 5},    //7.5uA
      //{0x05 , 1},    //12.5uA
        {0x07 , 7},    //17.5uA
      //{0x09 , 1},    //22.5uA
      //{0x0B , 1},    //27.5uA
      //{0x0D , 1},    //32.5uA
        {0x0F , 8},    //37.5uA
      //{0x11 , 1},    //42.5uA
      //{0x13 , 1},    //47.5uA
      //{0x15 , 1},    //52.5uA
        {0x17 , 10},   //57.5uA
      //{0x19 , 1},    //62.5uA
      //{0x1B , 1},    //67.5uA
      //{0x1D , 1},    //72.5uA
        {0x1F , 65},   //77.5uA
    };
    T_U32 reg_value;
    T_U32 i;

    //VCM2 discharging current selection.
    reg_value = REG32(REG_ANALOG_CTRL1);
    reg_value &= ~(0x1F << PTM_DCHG);
    reg_value |= PD_VCM2;   //power down VCM2

    REG32(REG_ANALOG_CTRL1) = reg_value | (value[0].charge_sel << PTM_DCHG);        
    delay_ms(value[0].delay_ms);

    for (i=1; i<sizeof(value)/sizeof(value[0]); ++i)
    {
        REG32(REG_ANALOG_CTRL1) = reg_value | (value[i].charge_sel << PTM_DCHG);
        delay_ms(value[i].delay_ms);
    }
    REG32(REG_ANALOG_CTRL1) |= PL_VCM2;
    //power down HPMID
    reg_value |= (PD_HPMID);
    REG32(REG_ANALOG_CTRL1) = reg_value;
#else
    static const T_VCM2_CHARGE value[] = 
    {
        {0x01 , 1},    //2.5uA
        {0x03 , 1},    //7.5uA
        {0x05 , 1},    //12.5uA
        {0x07 , 1},    //17.5uA
        {0x09 , 1},    //22.5uA
        {0x0B , 1},    //27.5uA
        {0x0D , 1},    //32.5uA
        {0x0F , 1},    //37.5uA
        {0x11 , 1},    //42.5uA
        {0x13 , 1},    //47.5uA
        {0x15 , 1},    //52.5uA
        {0x17 , 1},    //57.5uA
        {0x19 , 1},    //62.5uA
        {0x1B , 1},    //67.5uA
        {0x1D , 1},    //72.5uA
        {0x1F , 1},    //77.5uA
    };

    T_U32  reg_value;
    T_U32  i;
    T_U32  buf[2];
    T_BOOL  bdac_open;


    bsetinfo = AK_FALSE;
    
    if(REG32(REG_CLOCK_RST_EN) & (1 << 5))
    {
        bdac_open = AK_FALSE;
    }
    else
    {
        bdac_open = AK_TRUE;
    }


    //VCM3 is generated from ref1.5
    REG32(REG_ANALOG_CTRL3) &= ~VCM3_SEL_VCM2;

    if(AK_FALSE == bdac_open)
    {
        dac_open();
        //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
        REG32(REG_ANALOG_CTRL1) &= ~(PD_CK | PD_OP);
        REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
        REG32(REG_ANALOG_CTRL1) &= ~DACS_RST_M_V;
    }


    buf[0] = sina_down[0];//0x027b027b;
    output_sina_on(buf, 4, AK_TRUE);
    delay_ms(5);
    //charge vcm2
    REG32(REG_ANALOG_CTRL4) |= ANTI_POP_EN;
    delay_ms(5);

    output_sina_on(sina_down, sizeof(sina_down), AK_TRUE);
    //delay_ms(1);
     
    //VCM2 discharging current selection.
    reg_value = REG32(REG_ANALOG_CTRL1);
    reg_value &= ~(0x1F << PTM_DCHG);
    reg_value |= PD_VCM2;   //power down VCM2

    REG32(REG_ANALOG_CTRL1) = reg_value | (value[0].charge_sel << PTM_DCHG);
    delay_ms(value[0].delay_ms);

    for (i=1; i<sizeof(value)/sizeof(value[0]); ++i)
    {
        REG32(REG_ANALOG_CTRL1) = reg_value | (value[i].charge_sel << PTM_DCHG);
        delay_ms(value[i].delay_ms);
    }
    
    REG32(REG_ANALOG_CTRL1) |= PL_VCM2;
    delay_ms(60);
    REG32(REG_ANALOG_CTRL4) &= ~ANTI_POP_EN;

    if(AK_FALSE == bdac_open)
    {
        REG32(REG_ANALOG_CTRL1) |= PD_CK | PD_OP;
        dac_close();
    }
#endif
}


static T_VOID input_power_manage(ANALOG_SIGNAL_SRC analog_src, 
                                 ANALOG_SIGNAL_DST analog_dst,
                                 ANALOG_SIGNAL_STATE state)
{
    T_U32 reg_value;
    T_U32 open_flag = (hp_open_flag | adc_open_flag | lineout_open_flag);


    if (SIGNAL_CONNECT == state)     //connect
    {
        switch (analog_src)
        {
        case SRC_DAC:
            if (0 == (open_flag & (1<<SRC_DAC)))
            {
                //do nothing, power on dac in dac_open()
                drv_print("DAC ON in dac_open", 0, AK_TRUE);
            }
            break;

        case SRC_LINEIN:
            if (0 == (open_flag & (1<<SRC_LINEIN)))
            {
                //power on the linein interface
                REG32(REG_ANALOG_CTRL3) &= ~(PD_R_LINEIN | PD_L_LINEIN);
                
                drv_print("LINEIN ON", 0, AK_TRUE);
            }
            break;

        case SRC_MIC:
            if (0 == (open_flag & (1<<SRC_MIC)))
            {
                REG32(REG_ANALOG_CTRL4) |= MIC_VCM2_CNCT;
                //power on mic interface
                reg_value = REG32(REG_ANALOG_CTRL3);
                reg_value &= ~(PD_R_MICP| PD_L_MICP);
                //connect VCM3 to AVDD_MIC
                reg_value |= CONNECT_VCM3;
                //when use voice wakeup, should Bypass AVCC to AVDD_MIC,
                //because VCM3 is close in standby mode
                //reg_value &= ~CONNECT_AVCC;
                REG32(REG_ANALOG_CTRL3) = reg_value;
                
                drv_print("MIC ON", 0, AK_TRUE);
            }
            break;

        default:
            break;
        }
    }
    else                          //disconnect
    {
        switch (analog_src)
        {
        case SRC_DAC:
            if (0 == (open_flag & (1<<SRC_DAC)))
            {
                //do nothing, power off dac in dac_close()
                drv_print("DAC OFF in dac_close", 0, AK_TRUE);
            }
            break;

        case SRC_LINEIN:
            if (0 == (open_flag & (1<<SRC_LINEIN)))
            {
                //power off the linein interface
                #if (CHIP_SEL_10C > 0)
                REG32(REG_ANALOG_CTRL3) |= (PD_R_LINEIN | PD_L_LINEIN); 
				#else
                //REG32(REG_ANALOG_CTRL3) |= (PD_R_LINEIN | PD_L_LINEIN); 
                #endif
                
                drv_print("LINEIN OFF", 0, AK_TRUE);
            }
            break;

        case SRC_MIC:
            if (0 == (open_flag & (1<<SRC_MIC)))
            {
                //power off mic interface
                reg_value = REG32(REG_ANALOG_CTRL3);
                reg_value |= (PD_R_MICP| PD_L_MICP);
                //disconnect VCM3 to AVDD_MIC
                reg_value &= ~CONNECT_VCM3;
                REG32(REG_ANALOG_CTRL3) = reg_value;
                REG32(REG_ANALOG_CTRL4) &= ~MIC_VCM2_CNCT;
                
                drv_print("MIC OFF", 0, AK_TRUE);
            }
            break;

        default:
            break;
        }
    }
}


static T_VOID output_power_manage(ANALOG_SIGNAL_SRC analog_src, 
                                  ANALOG_SIGNAL_DST analog_dst,
                                  ANALOG_SIGNAL_STATE state)
{
    T_U32 reg_value;


    if (SIGNAL_CONNECT == state) //connect
    {
        switch (analog_dst)
        {
        case DST_ADC:
            if (0 == adc_open_flag)
            {
                /*  disable VCM2 discharging
                    To power on VCM3
                    disable pull down VCM2 with a 3Kohm resistor to ground
                    disable pull down VCM3 with a 2Kohm resistor to ground
                    To power on VCM2
                    To power on the bias generator
                */
                reg_value = REG32(REG_ANALOG_CTRL1);
                reg_value &= ~((0x1F << PTM_DCHG) | PL_VCM3 | PD_VCM3 | 
                                PL_VCM2 | PD_VCM2 | PD_BIAS);
                REG32(REG_ANALOG_CTRL1) = reg_value;

                reg_value = REG32(REG_ANALOG_CTRL3);
                //VCM3 is generated from VCM2
                reg_value |= VCM3_SEL_VCM2;

                //To power on single-differential conversion in ADC2
                //do this in adc_setinfo
                //reg_value &= ~(PD_R_S2D | PD_L_S2D);
                //reg_value &= ~(PD_R_SDM | PD_L_SDM);
                //ADC2_LIM and ADC3_LIM enable limit
                //reg_value |= (LIMIT_R_EN | LIMIT_L_EN);

                //The charging current is 200uA on VCM2
                reg_value &= ~(0x1F << PTM_CHG);
                REG32(REG_ANALOG_CTRL3) = reg_value;

                drv_print("ADCS ON" , 0, AK_TRUE);
            }
            adc_open_flag |= (1<<analog_src);
            break;

        case DST_HP:
            if (0 == hp_open_flag)
            {
                //打开Hp的时候，先打开一下LineIn，否则会有LineIn串音
                if ((AK_FALSE == linein_fix) 
                    && (0 == ((1<<SRC_LINEIN) 
                            & ((1<<analog_src) | adc_open_flag 
                                | hp_open_flag | lineout_open_flag))))
                {
                    #if (CHIP_SEL_10C > 0)
                    //REG32(REG_ANALOG_CTRL3) &= ~(PD_L_LINEIN |PD_R_LINEIN);
                    #else
                    REG32(REG_ANALOG_CTRL3) &= ~(PD_L_LINEIN |PD_R_LINEIN);
                    #endif
                    linein_fix = AK_TRUE;
                }

                //VCM3 is generated from ref1.5
                REG32(REG_ANALOG_CTRL3) &= ~VCM3_SEL_VCM2;
                
                //power on bias
                //Disable the pull-down 2Kohm resistor to VCM3
                //Power on VCM3
                REG32(REG_ANALOG_CTRL1) &= ~(PD_BIAS | PL_VCM3 | PD_VCM3);

                //power on NMOS and PMOS
                pnmos_open();

                //power on VCM2 and charge.if in DC mode, will power on HPMID
                if (SRC_DAC == analog_src)
                {
                    //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
                    REG32(REG_ANALOG_CTRL1) &= ~(PD_CK | PD_OP);
                    REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
                    REG32(REG_ANALOG_CTRL1) &= ~DACS_RST_M_V;
                }

                VCM2_charging();

                //REG32(REG_ANALOG_CTRL3) |= VCM3_SEL_VCM2;

                drv_print("HP ON" , 0, AK_TRUE);
            }
            else
            {
                if (SRC_DAC == analog_src)
                {
                    //power on the integrator in DAC and DAC CLK, after VCM2 chareging fully
                    REG32(REG_ANALOG_CTRL1) &= ~(PD_CK | PD_OP);
                    REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
                    REG32(REG_ANALOG_CTRL1) &= ~DACS_RST_M_V;
                }
            }
            hp_open_flag |= (1<<analog_src);
            break;

        case DST_LINEOUT:
            //lineout_open_flag |= (1<<analog_src);
            break;

        default:
            break;
        }
    }
    else                        //disconnect
    {
        switch (analog_dst)
        {
        case DST_ADC:
            adc_open_flag &= ~(1<<analog_src);
            if (0 == adc_open_flag)
            {
                /*  
                    To discharge VCM2 with 77.5uA current
                    To pull down VCM3 with a 3Kohm resistor to ground
                    To power off VCM3
                    To pull down VCM2 with a 3Kohm resistor to ground
                    To power off VCM2
                    To power off the bias generator
                */
                //if hp if off, powerdown all the vcm3/vcm2, bias
                reg_value = REG32(REG_ANALOG_CTRL1);
                if ((PD_PMOS | PD_NMOS) == (reg_value & (PD_PMOS | PD_NMOS)))
                {
                    reg_value |= ((0x1F << PTM_DCHG) | PL_VCM3 | PD_VCM3 | 
                                    PL_VCM2 | PD_VCM2 | PD_BIAS);
                    REG32(REG_ANALOG_CTRL1) = reg_value;
                }
                
                
                reg_value = REG32(REG_ANALOG_CTRL3);
                //To power off ADC2
                reg_value |= (PD_R_SDM | PD_L_SDM);
                //To power off single-differential conversion in ADC2
                reg_value |= (PD_R_S2D | PD_L_S2D);
                //ADC2_LIM and ADC3_LIM disable limit
                reg_value &= ~(LIMIT_R_EN | LIMIT_L_EN);
                REG32(REG_ANALOG_CTRL3) = reg_value;

                drv_print("ADCS OFF" , 0, AK_TRUE);
            }
            break;

        case DST_HP:
            hp_open_flag &= ~(1<<analog_src);
            if (0 == hp_open_flag)
            {
                if ((AK_TRUE == linein_fix) 
                    && (0 == ((1<<SRC_LINEIN) & ((1<<analog_src) | adc_open_flag 
                                        | hp_open_flag | lineout_open_flag))))
                {
                    //REG32(REG_ANALOG_CTRL3) |= (PD_L_LINEIN |PD_R_LINEIN);
                    linein_fix = AK_FALSE;
                }

                //power off VCM2 and discharge it.After this, HP no need to discharge
                VCM2_discharging();

                //对于PD_CK 和 PD_OP的控制，需要放在合适的位置，再考虑；
                if (SRC_DAC == analog_src)
                {
                    //drv_print("HP should be power off before DAC close", 0, AK_TRUE);
                    
                    //power off the integrator in DAC and DAC CLK, just in case
                    REG32(REG_ANALOG_CTRL1) |= PD_CK | PD_OP;
                    REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
                }

                //power off PMOS & NMOS
                REG32(REG_ANALOG_CTRL1) |= (PD_PMOS | PD_NMOS);

                //Set HP at the discharging state
                REG32(REG_ANALOG_CTRL1) |= (PRE_EN2) | (0x7U << DISCHG_HP);

                //Power off VCM3
                REG32(REG_ANALOG_CTRL1) |= PD_VCM3;
                //Enable the pull-down 2Kohm resistor to VCM3
                REG32(REG_ANALOG_CTRL1) |= PL_VCM3;

                //power off bias
                REG32(REG_ANALOG_CTRL1) |= PD_BIAS;

                drv_print("HP OFF" , 0, AK_TRUE);
            }
            else
            {
                //对于PD_CK 和 PD_OP的控制，需要放在合适的位置，再考虑；
                if (SRC_DAC == analog_src)
                {
                    //drv_print("HP should be power off before DAC close", 0, AK_TRUE);
                    
                    //power off the integrator in DAC and DAC CLK, just in case
                    REG32(REG_ANALOG_CTRL1) |= PD_CK | PD_OP;
                    REG32(REG_ANALOG_CTRL1) |= DACS_RST_M_V;
                }
            }
            break;

        case DST_LINEOUT:
            //lineout_open_flag &= ~(1<<analog_src);
            break;

        default:
            break;
        }
    }
}



/**
 * @brief   set up the signal device of source and destination or not. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]analog_src
 *              the source type of the signal.
 * @param   [in]analog_dst
 *              the destination type of the signal.
 * @param   [in]state
 *              connect or disconnect
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setsignal(ANALOG_SIGNAL_SRC analog_src, 
                        ANALOG_SIGNAL_DST analog_dst, 
                        ANALOG_SIGNAL_STATE state)
{
    if ((analog_src < SRC_DAC) || (SRC_NUL < analog_src) || 
        (analog_dst < DST_ADC) || (DST_NUL < analog_dst))
    {
        //drv_print("analog_src or analog_dst is error", 0, AK_TRUE);
        return AK_FALSE;
    }


    //打开时，必须按照先打开输入设备再打开输出设备的顺序执行；
    //关闭时，必须按照先关闭输出设备再关闭输入设备的顺序执行；
    //以上操作主要有两个原因，一是消PIPA音，二是变量XXX_open_flag
    if (SIGNAL_CONNECT == state)     //connect
    {
        input_power_manage(analog_src, analog_dst, state);
        //because linein interface occur a mistake state while vcm2 charging,
        //so do specially.
        if ((SRC_LINEIN == analog_src))
        {
            output_power_manage(analog_src, analog_dst, state);
            delay_ms(300);
            //这里仅打开设备，由上层控制何时去连接设备
            //analog_setconnect(analog_src, analog_dst, state);
        }
        else
        {
            output_power_manage(analog_src, analog_dst, state);
            //这里仅打开设备，由上层控制何时去连接设备
            //analog_setconnect(analog_src, analog_dst, state);
        }
    }
    else
    {
        //analog_setconnect(analog_src, analog_dst, state);
        output_power_manage(analog_src, analog_dst, state);
        input_power_manage(analog_src, analog_dst, state);
    }

    return AK_TRUE;
}



/**
 * @brief   connect or disconnect the signal between source and destination. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]analog_src
 *              the source type of the signal.
 * @param   [in]analog_dst
 *              the destinationtype of the signal.
 * @param   [in]state
 *              connect or disconnect
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setconnect(ANALOG_SIGNAL_SRC analog_src, 
                         ANALOG_SIGNAL_DST analog_dst, 
                         ANALOG_SIGNAL_STATE state)
{
    if ((analog_src < SRC_DAC) || (SRC_NUL <= analog_src) || 
        (analog_dst < DST_ADC) || (DST_NUL <= analog_dst))
    {
        //drv_print("analog_src or analog_dst is error", 0, AK_TRUE);
        return AK_FALSE;
    }

    if (SIGNAL_CONNECT == state)     //connect
    {
        switch (analog_dst)
        {
        case DST_ADC:
            REG32(REG_ANALOG_CTRL3) |= ((1<<analog_src) << ADCS_IN_SEL);
            break;

        case DST_HP:
            REG32(REG_ANALOG_CTRL1) |= ((1<<analog_src) << HP_IN);
            if ((AK_TRUE == linein_fix) 
                && (0 == ((1<<SRC_LINEIN) & (adc_open_flag 
                            | hp_open_flag | lineout_open_flag))))
            {
                //REG32(REG_ANALOG_CTRL3) |= (PD_L_LINEIN |PD_R_LINEIN);
                linein_fix = AK_FALSE;
            }
            break;

        case DST_LINEOUT:
            break;

        default:
            break;
        }
    }
    else                            //disconnect
    {
        switch (analog_dst)
        {
        case DST_ADC:
            REG32(REG_ANALOG_CTRL3) &= ~((1<<analog_src) << ADCS_IN_SEL);
            break;

        case DST_HP:
            REG32(REG_ANALOG_CTRL1) &= ~((1<<analog_src) << HP_IN);
            break;

        case DST_LINEOUT:
            break;

        default:
            break;
        }
    }

    return AK_TRUE;
}



/**
 * @brief   get the state of  the signal between source and destination. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]analog_src
 *              the source type of the signal.
 * @param   [in]analog_dst
 *              the destinationtype of the signal.
 * @param   [out]state
 *              connect or disconnect
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_getconnect(ANALOG_SIGNAL_SRC analog_src, 
                         ANALOG_SIGNAL_DST analog_dst, 
                         ANALOG_SIGNAL_STATE *state)
{
    T_U32 input = 0;

    if ((analog_src < SRC_DAC) || (SRC_NUL <= analog_src) || 
        (analog_dst < DST_ADC) || (DST_NUL <= analog_dst))
    {
        //drv_print("analog_src or analog_dstt is error", 0, AK_TRUE);
        return AK_FALSE;
    }


    *state = SIGNAL_DISCONNECT;

    switch (analog_dst)
    {
    case DST_ADC:
        input = (REG32(REG_ANALOG_CTRL3) >> ADCS_IN_SEL) & 0x07;
        break;

    case DST_HP:
        input = (REG32(REG_ANALOG_CTRL1) >> HP_IN) & 0x07;
        break;

    case DST_LINEOUT:
        break;

    default:
        break;
    }

    if (input & (1<<analog_src))
    {
        *state = SIGNAL_CONNECT;
    }

    return AK_TRUE;
}



/**
 * @brief   set the gain of the headphone. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]gain
 *              the gain of the headphone. the range of the gain is 0~5
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setgain_hp(T_U8 gain)
{
    T_U32       reg_value;
    const T_U8  hp_gain[HP_GAIN_MAX+1]={0x0, 0x18, 0x14, 0x12, 0x11, 0x10};


    if (gain > HP_GAIN_MAX)
    {
        //drv_print("set gain bigger than ", HP_GAIN_MAX, AK_TRUE);
        return AK_FALSE;
    }

    reg_value = REG32(REG_ANALOG_CTRL1);
    reg_value &= ~(0x1F << HP_GAIN);
    reg_value |= (hp_gain[gain] << HP_GAIN);
    REG32(REG_ANALOG_CTRL1) = reg_value;

    return AK_TRUE;
}



/**
 * @brief   set the gain of the linein. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]gain
 *              the gain of the linein. the range of the gain is 0~15
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */
T_BOOL analog_setgain_linein (T_U8 gain)
{
    T_U32 reg_value;


    if (gain > LIN_GAIN_MAX)
    {
        //drv_print("set gain bigger than ", LIN_GAIN_MAX, AK_TRUE);
        return AK_FALSE;
    }

    reg_value = REG32(REG_ANALOG_CTRL3);
    reg_value &= ~(0x0F << LINE_GAIN);
    reg_value |= (gain << LINE_GAIN);
    REG32(REG_ANALOG_CTRL3) = reg_value;

    return AK_TRUE;
}



/**
 * @brief   set the gain of the mic. 
 * @author  wangguotian
 * @date    2012.11.19
 * @param   [in]gain
 *              the gain of the mic. the range of the gain is 0~10
 * @return  T_BOOL
 * @retval  If the function succeeds, the return value is AK_TRUE;
 *          If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL analog_setgain_mic(T_U8 gain)
{
    T_U32 reg_value;


    if (gain > MIC_GAIN_MAX)
    {
        //drv_print("set gain bigger than ", MIC_GAIN_MAX, AK_TRUE);
        return AK_FALSE;
    }

    if (gain >= 8)
    {
        gain = gain + 5;
    }

    reg_value = REG32(REG_ANALOG_CTRL3);
    reg_value &= ~(0x07 << MIC_GAIN);
    reg_value |= ((gain & 0x07) << MIC_GAIN);
    if (gain >= 8)
    {
        reg_value |= MIC_GAIN_2X;
    }
    else
    {
        reg_value &= ~MIC_GAIN_2X;
    }
    REG32(REG_ANALOG_CTRL3) = reg_value;
    
    return AK_TRUE;
}


/******************************************************************************/


#pragma arm section code = "_drvbootcode_"
static T_VOID adc1_open(T_VOID)
{
    //power on adc1 and enable the akc1 clk
    REG32(REG_CLOCK_DIV2) |= ADC1_CLK_EN;
    REG32(REG_ADC1_CFG) &= ~ADC1_PD;
}


static T_VOID adc1_close(T_VOID)
{
    //power down adc1 and disable the adc1 clk
    REG32(REG_ADC1_CFG) |= ADC1_PD;
    REG32(REG_CLOCK_DIV2) &= ~ADC1_CLK_EN;
}


static T_BOOL check_cov_finish(T_VOID)
{
    T_U32 reg_value;
    T_U32 count = 0;

    while (ADC1_COV_STATE != (REG32(REG_ADC1_STA)&ADC1_COV_STATE))
    {
        if (count++ > 0xFFFF)
        {
            //drv_print(__FILE__, __LINE__, AK_TRUE);
            return AK_FALSE;
        }
    }

    //chip bug, the value that write to 0x00400070 will also write to 0x00400078
    reg_value = REG32(REG_ADC1_STA);
    reg_value &= ~ADC1_COV_STATE;
    REG32(REG_ADC1_STA) = reg_value;

    return AK_TRUE;
}

/*******************************************************************************
 * @brief   get the voltage of the battery.
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the voltage of the battery, unit: mv
*******************************************************************************/
T_U32 analog_getvoltage_bat(T_VOID)
{
    T_U32 data = 0;


    adc1_open();

    INT_DISABLE(INT_EN_SYSCTL);
    //select bat channel and set adc1 vref 1.5V
    REG32(REG_ADC1_CFG) |= (BAT_CHANNEL | ADC1_VREF_BGR | BAT_DIV_EN);

    //enable adc1 converter
    REG32(REG_ANALOG_CTRL2) |= ADC1_COV_EN;

    //wait for convert finish
    check_cov_finish();
    data += ((REG32(REG_ADC1_STA) >> BAT_VALUE) & 0x3ff);

    //disable bat channel and set adc1 vref 3.3V
    REG32(REG_ADC1_CFG) &= ~(BAT_CHANNEL | ADC1_VREF_BGR | BAT_DIV_EN);
    INT_ENABLE(INT_EN_SYSCTL);

    adc1_close();

    data = (data*4500)>>10;  //data = data*1.5*1000*3/1024(mv)

    return data;
}

/*******************************************************************************
 * @brief   get the voltage of the analog keypad.
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the voltage of the analog keypad, unit: mv
*******************************************************************************/
T_U32 analog_getvoltage_keypad(T_VOID)
{
    T_U32 data = 0;


    adc1_open();

    //enable AIN0 channel
    REG32(REG_ADC1_CFG) |= AIN0_CHANNEL;
    //enable adc1 converter
    REG32(REG_ANALOG_CTRL2) |= ADC1_COV_EN;

    //wait for convert finish
    check_cov_finish();
    data = (REG32(REG_ADC1_STA) >> AIN0_VALUE) & 0x3ff;

    //disable AIN0 channel
    REG32(REG_ADC1_CFG) &= ~AIN0_CHANNEL;

    adc1_close();

    data = (data*3300)>>10;   //data = data*3.3*1000/1024(mv)

    return data;
}

#if (DETECT_MODE_ADC > 0)
/*******************************************************************************
 * @brief   get the voltage of detection.
 * @author  wangguotian
 * @date    2012.11.19
 * @param   T_VOID
 * @return  T_U32
 * @retval  the voltage of the detection
*******************************************************************************/
T_U32 analog_getvoltage_detection(T_VOID)
{
    T_U32 data = 0;


    adc1_open();

    //enable AIN1 channel
    REG32(REG_ADC1_CFG) |= AIN1_CHANNEL;
    //enable adc1 converter
    REG32(REG_ANALOG_CTRL2) |= ADC1_COV_EN;

    //wait for convert finish
    check_cov_finish();
    data = (REG32(REG_ADC1_STA) >> AIN1_VALUE) & 0x3ff;

    //disable AIN1 channel
    REG32(REG_ADC1_CFG) &= ~AIN1_CHANNEL;

    adc1_close();

    data = (data*3300)>>10;   //data = data*3.3*1000/1024(mv)

    return data;
}
#endif
#pragma arm section code



#define ADC1_WORK_CLK       1000000     //Recommended ADC1 CLK is 1MHz.

#if CHIP_SEL_10C > 0
#define ADC1_DIV            (26000000/ADC1_WORK_CLK - 1)
#else
#define ADC1_DIV            (12000000/ADC1_WORK_CLK - 1)
#endif

#pragma arm section code = "_drvbootinit_"
/*******************************************************************************
 * @brief   init analog. Mainly to configurate some parameters of the analog
 * @author  zhanggaoxin
 * @date    2013-01-24
 * @param   T_VOID
 * @return  T_VOID
*******************************************************************************/
T_VOID analog_init(T_VOID)
{
    T_U32 reg_value;


    reg_value = REG32(REG_CLOCK_DIV2);
    //set adc1 working clk
    reg_value &= ~((7 << ADC1_DIV_L) | (3UL << ADC1_DIV_H));
    reg_value |= (((ADC1_DIV & 7) << ADC1_DIV_L) | 
                 (((ADC1_DIV>>3) & 0x3ul)<< ADC1_DIV_H) | ADC1_RESET_DIS);
    REG32(REG_CLOCK_DIV2) = reg_value;

    adc1_open();
    reg_value = REG32(REG_ADC1_CFG);
    //reset adc1
    reg_value |= ADC1_RESET;
    //enable ain0 and ain1 wakeup signal input
	reg_value |= (AIN0_WAKEUP_EN);//AIN1_WAKEUP_EN 这个位在10C 芯片上属于保留位
    //set bat division ratio = 1/3
    reg_value &= ~BAT_DIV_2;
    //set adc1 vref 3.3V
    reg_value &= ~ADC1_VREF_BGR;
    //set vref pin source select internal BGR
    //reg_value |= VREF1V5_BGR;
    //set vref pin output high impedance
    reg_value |= VREF_PIN_SEL;
    REG32(REG_ADC1_CFG) = reg_value;

    //clear adc1 reset
    reg_value &= ~ADC1_RESET;
    REG32(REG_ADC1_CFG) = reg_value;
    adc1_close();

    REG32(REG_ANALOG_CTRL1) &= ~(0x6U << DISCHG_HP);
    REG32(REG_ANALOG_CTRL1) &= ~PD_BIAS;
    delay_ms(100);

    reg_value = REG32(REG_ANALOG_CTRL4);
    //close voice wakeup
    reg_value |= PD_VW;
    reg_value |= DISCHG_VP;
#if (HP_MODE_DC > 0)
    //if used HP_MODE_DC, enable HP over current protect
    //reg_value &= ~HP_OCP_DIS;
#endif
    //disable HP plugin detect
    reg_value &= ~HP_PLUGIN_DET_EN;
    reg_value &= ~MIC_VCM2_CNCT;
    REG32(REG_ANALOG_CTRL4) = reg_value;

    REG32(REG_ANALOG_CTRL1) |= PD_BIAS;
    //set HP at the discharging state
    REG32(REG_ANALOG_CTRL1) |= (PRE_EN2 | (0x7U << DISCHG_HP));

    //set MIC_VDD low level
    REG32(REG_ANALOG_CTRL3) &= ~(CONNECT_VCM3 | CONNECT_AVCC);

    reg_value = REG32(REG_ADC1_STA);
    reg_value &= ~ADC1_COV_STATE;
    REG32(REG_ADC1_STA) = reg_value;
}
#pragma arm section code

#endif

