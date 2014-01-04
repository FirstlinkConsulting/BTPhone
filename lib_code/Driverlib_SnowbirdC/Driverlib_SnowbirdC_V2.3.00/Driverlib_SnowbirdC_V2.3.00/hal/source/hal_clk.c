/*******************************************************************************
 * @FILENAME: hal_clk.c
 * @BRIEF hal_clk driver file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR zhanggaoxin
 * @DATE 2012-11-20
 * @VERSION 1.0
 * @REF
*******************************************************************************/
#include "anyka_types.h"
#include "clk.h"
#include "hal_clk.h"
#include "arch_init.h"


#pragma arm section zidata = "_drvbootbss_"
static T_U32 module_min_asic[MAX_ASIC_NUM];
static T_U32 max_of_module_min_asic;
#pragma arm section zidata


/*******************************************************************************
 * @brief   get cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @return  T_U32 the cpu frequency(Hz)
*******************************************************************************/
T_U32 clk_get_cpu(T_VOID)
{
    if (clk_get_cpu2x())
    {
        return clk_get_clk168m();
    }
    else
    {
        return clk_get_asic();
    }
}

/*******************************************************************************
 * @brief   set cpu frequency.
 * cpu frequency can the same as asic frequency or 2X of that
 * @author  zhanggaoxin
 * @date    2012-11-20
 * @param   [in]freq the cpu frequency to set
 * @return  T_U32 the cpu frequency(Hz) running
*******************************************************************************/
T_U32 clk_set_cpu(T_U32 freq)
{
    T_U32 cur_pll = clk_get_pll();

    if (0 == freq)
    {
        drv_print("The freq to be set can't equal to zero!", 0, AK_TRUE);
        return clk_get_cpu();
    }

    if (freq < max_of_module_min_asic)
    {
        drv_print("The asic that can be set can't be less than:0x", \
                    max_of_module_min_asic, AK_TRUE);
        freq = max_of_module_min_asic;
    }

    if (freq > cur_pll)
    {
        freq = cur_pll;
    }

    clk_adjust(freq, max_of_module_min_asic);

    return clk_get_cpu();
}

/*******************************************************************************
 * @brief   request minimal asic frequency.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @param   [in]asic_clk the asic frequency requested
 * @return  T_U8 the index of the asic frequency in array.
*******************************************************************************/
T_U8 clk_request_min_asic(T_U32 asic_clk)
{
    T_U8 index;

    if ((0 == asic_clk) || (asic_clk > MAX_ASIC_VAL))
    {
        return INVALID_ASIC_INDEX;
    }

    for (index=0; index<MAX_ASIC_NUM; index++)
    {
        if (module_min_asic[index] == 0)
        {
            module_min_asic[index] = asic_clk;
            break;
        }
    }
    if (index == MAX_ASIC_NUM)
    {
        drv_print("Array full! MAX:", MAX_ASIC_NUM, AK_TRUE);
        return INVALID_ASIC_INDEX;
    }

    if (max_of_module_min_asic < asic_clk)
    {
        max_of_module_min_asic = asic_clk;
        if (clk_get_asic() < asic_clk)
        {
            clk_set_cpu(asic_clk);
        }
    }

    return index;
}


#pragma arm section code = "_drvbootcode_"
/*******************************************************************************
 * @brief   free minimal asic frequency.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @param   [in]index
 * @return  T_BOOL
*******************************************************************************/
T_BOOL clk_cancel_min_asic(T_U8 asic_index)
{
    T_U32 asic_tmp;
    T_U8  index;

    if (asic_index >= MAX_ASIC_NUM)
    {
        //drv_print("Error index:0x", asic_index, 1);
        return AK_FALSE;
    }

    asic_tmp = module_min_asic[asic_index];
    module_min_asic[asic_index] = 0;

    if (asic_tmp == max_of_module_min_asic)
    {
        max_of_module_min_asic = module_min_asic[0];
        for (index=1; index<MAX_ASIC_NUM; index++)
        {
            if (module_min_asic[index] > max_of_module_min_asic)
            {
                max_of_module_min_asic = module_min_asic[index];
            }
        }
    }

    return AK_TRUE;
}
#pragma arm section code

