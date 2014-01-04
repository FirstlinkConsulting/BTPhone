/************************************************************************
 * Copyright (c) 2008, Anyka Co., Ltd.
 * All rights reserved.
 *
 * @FILENAME 
 * @BRIEF 
 * @Author£ºzhao_xiaowei
 * @Date£º
 * @Version£º
**************************************************************************/
#include "hal_radio.h"
#include "arch_timer.h"
#include "arch_gpio.h"
#include "arch_i2c.h"
#include "arch_init.h"
#include "drv_cfg.h"


#if (DRV_SUPPORT_RADIO > 0) && (RADIO_RDA5807P > 0)


#define RADIO_DEBUG             0


#define DEVICE_NAME             "RADIO:RDA5807"

#define RADIO_I2C_ADDR          0x20
#define LWORD(data)             (data& 0x00ff)
#define HWORD(data)             (data >> 8)
#define WRITE_U16_ARRAY(data)   WriteData(data, sizeof(data)/sizeof(T_U16))
#define MIN_JAP_FREQ            76000000
#define MIN_EU_FREQ             87000000
#define MAKEBOOL(value)         (value) ? AK_TRUE : AK_FALSE;
#define TRY_COUNT               3
#define HW_DELAY                20

#define FM_CLK_MODE             CLK_32K

typedef enum 
{
    CLK_32K= 0,  //32.768
    CLK_12M= 1,
    CLK_24M= 5,
    CLK_13M= 2,
    CLK_26M= 6,
    CLK_19M= 3,  //19.2
    CLK_38M= 7   //38.4
}T_FMClkMode;

typedef struct 
{
    T_U16 pll;
    T_BOOL stereo;
    T_BOOL stc;
    T_BOOL fm_ready;
    T_BOOL fm_true;
}T_RadioRead;

static T_BOOL m_band= AK_FALSE; 

static T_BOOL WriteData(T_U16 data[], T_U8 size)
{
    T_U8 byteSize = size* 2;
    T_U8 *pByte;
    T_U8 i;
    T_U8 j;
    T_BOOL bRet = AK_FALSE;
    T_U8 tryCount;
    
    if(byteSize == 0)
        return AK_FALSE;
    pByte = drv_malloc(byteSize* sizeof(T_U8));
    for(i=0, j=0; i<size; i++)
    {
        pByte[j++] = HWORD(data[i]);
        pByte[j++] = LWORD(data[i]);
    }

    for(tryCount=0; tryCount<TRY_COUNT; tryCount++)
    {
        bRet = i2c_write_data(RADIO_I2C_ADDR, pByte, byteSize);
        if(bRet)
        {
            break;
        }
        else
        {
            drv_print("Write Radio I2c Failed!", tryCount, AK_TRUE);
        }
        delay_ms(HW_DELAY);
    }

    pByte = drv_free(pByte);

    return bRet;
}

#if RADIO_DEBUG
static T_VOID PrintRead(T_RadioRead radioRead)
{
    AK_DEBUG_OUTPUT("stc: %d, stereo:%d,pll: %03x,fm_true: %d, fm_ready: %d\n",
                    radioRead.stc, radioRead.stereo, radioRead.pll,
                    radioRead.fm_true, radioRead.fm_ready);
}

static T_VOID Debug_Out(T_U8 data[], T_U8 size)
{
    T_U8 i;
    for(i=0; i<size; i++)
    {
        AK_DEBUG_OUTPUT("Byte%d: %x\n", i, data[i]);
    }   
}
#endif

static T_BOOL ReadData(T_RadioRead *pRadioRead)
{
    T_U8 data[4];
    T_U16 regA, regB;
    T_BOOL bRet;
    T_U8 tryCount;

    for(tryCount=0; tryCount<TRY_COUNT; tryCount++)
    {
        bRet = i2c_read_data(RADIO_I2C_ADDR, data, 4);

        if(bRet)
        {
            break;
        }
        else
        {
            drv_print("Read Fialed!", tryCount, AK_TRUE);
        }
        delay_ms(HW_DELAY);
    }
    
    if(bRet)
    {
        regA = (data[0]<<8) + data[1];
        regB = (data[2]<<8) + data[3];

    #if RADIO_DEBUG
        AK_DEBUG_OUTPUT("RegA: %04x,RegB: %04x\n", regA, regB);
    #endif

        pRadioRead->stc = MAKEBOOL(regA & 0x4000);
        pRadioRead->stereo = MAKEBOOL(regA & 0x0400);
        pRadioRead->pll = regA & 0x03ff;

        pRadioRead->fm_true = MAKEBOOL(regB & 0x0100);
        pRadioRead->fm_ready = MAKEBOOL(regB& 0x0080);
    #if RADIO_DEBUG
        PrintRead(*pRadioRead);
    #endif
    }

    return bRet; 
}

static T_BOOL UntilSetComplete()
{
    T_RadioRead radioRead;

    delay_ms(10);

#if RADIO_DEBUG
    AK_DEBUG_OUTPUT("After Write to Wait stc\n");
#endif
    do
    {
        if(!ReadData(&radioRead))
        {
            return AK_FALSE;
        }
    }while(!radioRead.stc);

    return AK_TRUE;
}

static T_U16 Freq2Pll_rda5807p(T_U32 Freq)
{
    T_U16 pll;

    if(m_band)
    {
        if(Freq < MIN_JAP_FREQ)
        {
            drv_print("Freq < Min_JAP_FREQ", 0, AK_TRUE);
            return 0;
        }
        pll = (T_U16)((Freq - 76000000) / BAND_WIDTH);
    }
    else
    {
        if(Freq < MIN_EU_FREQ)
        {
            drv_print("Freq < MIN_EU_FREQ", 0, AK_TRUE);
            return 0;
        }
        pll = (T_U16)((Freq - 87000000) / BAND_WIDTH);
    }

#if RADIO_DEBUG
    AK_DEBUG_OUTPUT("Frq2Pll, %d, %03x\n", Freq, pll);
#endif

    return pll;
}

static T_U32 Pll2Freq_rda5807p(T_U16 pll)
{
    T_U32 Freq;
    if(m_band)
    {
        Freq = BAND_WIDTH * pll + 76000000;
    }
    else
    {
        Freq = BAND_WIDTH * pll + 87000000;
    }

#if RADIO_DEBUG
    AK_DEBUG_OUTPUT("Pll2Freq, %x, %d\n", pll, Freq);
#endif
    return Freq;
}

T_BOOL radio_get_status(T_RADIO_STATUS *Status, T_U32 FreqMin, T_U32 FreqMax)
{
    T_RadioRead radioRead;

#if RADIO_DEBUG
    AK_DEBUG_OUTPUT("Before Get Status to Wait fm_ready\n");
#endif

    do
    {
        if(!ReadData(&radioRead))
        {
            return AK_FALSE;
        }
    }while(!radioRead.fm_ready);
    Status->CurFreq = Pll2Freq_rda5807p(radioRead.pll);
    if(Status->CurFreq == 0)
    {
        return AK_FALSE;
    }
    
    if(radioRead.fm_true)
    {
        Status->AdcLevel = RADIO_ADC_LEVEL;
    }
    else
    {
        Status->AdcLevel = 0;
    }

    Status->Stereo = radioRead.stereo;
    Status->ReadyFlag = radioRead.fm_true;
    Status->LimitFlag = AK_FALSE;
    Status->IFCounter = 0;

    return AK_TRUE;
}

T_BOOL radio_set_volume(T_U8 Volume)
{
#if 0
    T_U16 data[4] = {0x05,0x8b,0xa0};

    if (Volume)
    {
          data[2] += (Volume>>1);
    }

    return WRITE_U16_ARRAY(data);

#endif
    return AK_TRUE;
}


static T_VOID RadioSetClkMode(T_FMClkMode clkMode, T_U16* pData)
{
    #define RDA5807P_CLK            4   //[6:4]

    *pData &= ~(0x7 << RDA5807P_CLK);
    *pData |= (clkMode << RDA5807P_CLK);
}

T_BOOL radio_set_param(T_RADIO_PARM *Param)
{
    T_U16 data[] = {0xC2D1, 0x0010};
    T_U16 pll;

    RadioSetClkMode(FM_CLK_MODE,&data[0]);

    if(Param->BandLimit)
    {
        data[1] |= 0x0004;
    }
    if(Param->MuteFlag)
    {
        data[0] &= ~(0x4000);
    }
    m_band = Param->BandLimit;
    pll = Freq2Pll_rda5807p(Param->Freq);

    data[1] |= pll<<6;

#if RADIO_DEBUG
    AK_DEBUG_OUTPUT("\nBefore Write:%04x,%04x\n", data[0], data[1]);
#endif

    if(WRITE_U16_ARRAY(data))
    {
        UntilSetComplete();
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_BOOL radio_check(T_VOID)
{
    return AK_FALSE;
}

T_BOOL radio_init(T_VOID)
{
    T_U16 data[] = {0x82D3};

    drv_print(DEVICE_NAME, 0, AK_TRUE);

    RadioSetClkMode(FM_CLK_MODE, &data[0]);
    return WRITE_U16_ARRAY(data);
}

T_BOOL radio_exit(T_VOID)
{
    T_U16 data[] ={0x82D0};

    RadioSetClkMode(FM_CLK_MODE, &data[0]);
    return WRITE_U16_ARRAY(data);
}

T_VOID radio_line_in(T_BOOL enable)
{
}


#endif  //(DRV_SUPPORT_RADIO > 0) && (RADIO_RDA5807P > 0)

/* end of file */

