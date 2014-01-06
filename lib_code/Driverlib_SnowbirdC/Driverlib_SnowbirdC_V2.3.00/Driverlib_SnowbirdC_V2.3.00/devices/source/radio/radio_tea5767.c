/*******************************************************************************
 * @file radio.c
 * @brief radio driver, define radio APIs.
 * This file provides radio APIs: radio initialization,
 * play radio, etc..
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2005-03-07
 * @version 1.0
 * @ref AK3210M technical manual.
*******************************************************************************/
#include "hal_radio.h"
#include "arch_timer.h"
#include "arch_gpio.h"
#include "arch_i2c.h"
#include "arch_init.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_RADIO > 0) && (RADIO_TEA5767 > 0)


#define DEVICE_NAME         "RADIO:TEA5767"


#define RADIO_I2C_ADDR      0xc0
#define REF_FREQ            32768       //refrence frequence
#define IF_FREQ             225000      //fIF = 225kHz


static T_U32 Freq2Pll(T_U32 Freq)
{
    return (((Freq + IF_FREQ) << 2 ) >> 15); // qual to return (((Freq + IF_FREQ) << 2 )/REF_FREQ);
}

static T_U32 Pll2Freq(T_U32 Pll)
{
    if (((Pll << 15) >> 2) < IF_FREQ)
    {
        drv_print("Pll2Freq(): PLL is bad parm!!", 0, AK_TRUE);
        return 0;
    }
    else
    {
        /*High injection: fRF = (PLL*REF_FREQ)/4 - IF_FREQ */
        return (((Pll << 15) >> 2) - IF_FREQ);
    }
}

T_BOOL radio_get_status(T_RADIO_STATUS *Status, T_U32 FreqMin, T_U32 FreqMax)
{
    T_U8 DataTmp[5];
    T_U32 Pll;
    T_U8 i = 0;
    T_U32 curFreq = 0;
    T_BOOL ReadSuccess = AK_FALSE;
    T_RADIO_STATUS tmpStatus;
    T_BOOL ready = AK_FALSE;

    for (i=0; i<3; i++)
    {
        memset(&tmpStatus, 0, sizeof(tmpStatus));
        ReadSuccess = AK_FALSE;

        if (AK_FALSE == i2c_read_data(RADIO_I2C_ADDR, DataTmp, 5))
        {
            drv_print("radio_get_tmpStatus(): radio read fail!!", 0, AK_TRUE);
            continue;
        }

        ready = (T_BOOL)((DataTmp[0] & 0x80) >> 7);
        if (AK_FALSE == ready)              //???
            tmpStatus.ReadyFlag = AK_FALSE;
        else
            tmpStatus.ReadyFlag = ready;

        tmpStatus.LimitFlag = (T_BOOL)((DataTmp[0] & 0x40) >> 6);
        Pll = DataTmp[0] & 0x3f;
        Pll = (Pll << 8) + DataTmp[1];
        curFreq = Pll2Freq(Pll);

        if ((curFreq < FreqMin) || (curFreq > FreqMax))
        {
            drv_print("radio_get_tmpStatus(): PLL read error!!", 0, AK_TRUE);
            continue;
        }
        tmpStatus.CurFreq = curFreq;
        tmpStatus.IFCounter = DataTmp[2] & 0x7f;
        tmpStatus.Stereo = (DataTmp[2] & 0x80) >> 7;
        tmpStatus.AdcLevel = (DataTmp[3] &0xf0) >> 4;

        ReadSuccess = AK_TRUE;
        break;
    }

    if (ReadSuccess)
    {
        *Status = tmpStatus;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_BOOL radio_set_param(T_RADIO_PARM *Param)
{
    T_U32 Pll;
    T_U8 DataTmp[5] = {0x00, 0x00, 0x10, 0x11, 0x80};

    //AK_ASSERT_PTR(Param, "radio_set_param: Param", AK_FALSE);

    //AK_DEBUG_OUTPUT("debug token: radio_set_param(): freq = %d\r\n", Param->Freq);

    if (Param->MuteFlag)
        DataTmp[0] |= 0x80;

    if (Param->SearchFlag)
        DataTmp[0] |= 0x40;

    if (Param->SearchDir)
        DataTmp[2] |= 0x80;

    if (Param->MonoStereo)
        DataTmp[2] |= 0x08;

    if (Param->Freq)
    {
        Pll = Freq2Pll(Param->Freq);
        DataTmp[0] |= (T_U8)((Pll >> 8) & 0x3f);
        DataTmp[1] = (T_U8)(Pll & 0x00ff);
    }

    if (Param->StopLevel)
    {
       DataTmp[2] |= (Param->StopLevel << 5);
    }

    if (Param->BandLimit)
    {
        DataTmp[3] |= (T_U8)(1 << 5);
    }

    if (AK_FALSE == i2c_write_data(RADIO_I2C_ADDR, DataTmp, 5))
    {
        drv_print("Radio set param failed!!", 0, AK_TRUE);
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }
}

T_BOOL radio_set_volume(T_U8 Volume)
{
    T_U8 DataTmp[3] = {0x05, 0x88, 0x60};

    return AK_TRUE;

    if (Volume)
    {
        DataTmp[2] += (Volume>>1);
    }      

    if (AK_FALSE == i2c_write_data(RADIO_I2C_ADDR, DataTmp, 3))
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}

T_BOOL radio_check(T_VOID)
{
    T_U8 data[5] = {0};

    if (AK_FALSE == i2c_read_data(RADIO_I2C_ADDR, data, 5))
    {
        drv_print("Radio check failed!!", 0, AK_TRUE);
        return AK_FALSE;
    }

    return AK_TRUE;
}

T_BOOL radio_init(T_VOID)
{
    T_U8 DataTmp[5] = {0x80, 0x00, 0x00, 0x50, 0x00};

    drv_print(DEVICE_NAME, 0, AK_TRUE);

    if(i2c_write_data(RADIO_I2C_ADDR, DataTmp, 5))
    {
        drv_print("RADIO IS OPENED!!", 0, AK_TRUE);
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_BOOL radio_exit(T_VOID)
{
    T_U8 DataTmp[5] = {0x80, 0x00, 0x00, 0x5E, 0x00};

    if (i2c_write_data(RADIO_I2C_ADDR, DataTmp, 5))
    {
        drv_print("RADIO IS CLOSED!!", 0, AK_TRUE);
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_VOID radio_line_in(T_BOOL enable)
{
    //_AKSD_StartLineIn(enable);
}


#endif  //(DRV_SUPPORT_RADIO > 0) && (RADIO_RDA5807P > 0)

/* end of file */

