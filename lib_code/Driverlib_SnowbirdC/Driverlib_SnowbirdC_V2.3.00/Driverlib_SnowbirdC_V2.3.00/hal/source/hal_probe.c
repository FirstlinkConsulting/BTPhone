/*******************************************************************************
 * @file hal_probe.c
 * @brief device probe framework
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author guoshaofeng 
 * @date 2010-12-07
 * @version 1.0
 * @ref
*******************************************************************************/
#include "anyka_types.h"
#include "hal_probe.h"
#include "drv_cfg.h"
#include "arch_init.h"


#if DRV_SUPPORT_LCD > 0
static T_LCD_INFO LCD_INFO_TABLE[LCD_MAX_SUPPORT] = {0};
#endif

#if DRV_SUPPORT_CAMERA > 0
static T_CAMERA_INFO CAMERA_INFO_TABLE[CAMERA_MAX_SUPPORT] = {0};
#endif


#if DRV_SUPPORT_LCD > 0
/*******************************************************************************
 * @brief   lcd probe pointer
 * @author  guoshaofeng
 * @date    2007-12-24
 * @param   T_VOID
 * @return  T_LCD_FUNCTION_HANDLER
 * @retval  lcd device pointer
*******************************************************************************/
T_LCD_FUNCTION_HANDLER *lcd_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i = 0; i < LCD_MAX_SUPPORT; i++)
    {
        if (LCD_INFO_TABLE[i].handler != AK_NULL)
        {
            id = LCD_INFO_TABLE[i].handler->lcd_read_type_func();

            if (id == LCD_INFO_TABLE[i].DeviceID)
            {
                drv_print("match lcd, id = 0x%x\n", id, AK_TRUE);
                return LCD_INFO_TABLE[i].handler;
            }
        }
    }
    return AK_NULL;
}

T_BOOL lcd_reg_dev(T_U32 id, T_LCD_FUNCTION_HANDLER *handler)
{
    T_S32 i;
    T_BOOL ret = AK_FALSE;

    for (i = 0; i < LCD_MAX_SUPPORT; i++)
    {
        // check device register or not
        if (LCD_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it
        if (LCD_INFO_TABLE[i].DeviceID == 0 &&
            LCD_INFO_TABLE[i].handler == AK_NULL)
        {
            drv_print("lcd register id = 0x%x\n", id, AK_TRUE);
            LCD_INFO_TABLE[i].DeviceID = id;
            LCD_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}
#endif


#if DRV_SUPPORT_CAMERA > 0
/*******************************************************************************
 * @brief   camera probe pointer
 * @author  xia_wenting
 * @date    2010-12-07
 * @param   T_VOID
 * @return  T_CAMERA_FUNCTION_HANDLER
 * @retval  camera device pointer
*******************************************************************************/
T_CAMERA_FUNCTION_HANDLER *cam_probe(T_VOID)
{
    T_U32 i, id;
    
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        if (CAMERA_INFO_TABLE[i].handler != AK_NULL)
        {
            CAMERA_INFO_TABLE[i].handler->cam_open_func();
            id = CAMERA_INFO_TABLE[i].handler->cam_read_id_func();
            
            drv_print("camera probe id = ", id, AK_FALSE);
            drv_print(", device id = ", CAMERA_INFO_TABLE[i].DeviceID, AK_TRUE);

            if (id == CAMERA_INFO_TABLE[i].DeviceID)
            {
                drv_print("match camera, id = ", id, AK_TRUE);
                return CAMERA_INFO_TABLE[i].handler;
            }
            else
            {
                CAMERA_INFO_TABLE[i].handler->cam_close_func();
            }
        }
    }
    return AK_NULL;
}

T_BOOL camera_reg_dev(T_U32 id, T_CAMERA_FUNCTION_HANDLER *handler)
{
    T_U32 i;
    T_BOOL ret = AK_FALSE;
    
    for (i = 0; i < CAMERA_MAX_SUPPORT; i++)
    {
        // check device register or not
        if (CAMERA_INFO_TABLE[i].DeviceID == id)
            break;
        // got an empty place for it
        if (CAMERA_INFO_TABLE[i].DeviceID == 0 &&
            CAMERA_INFO_TABLE[i].handler == AK_NULL)
        {
            //akprintf(C3, M_DRVSYS, "camera register id = 0x%x, cnt = %d\n", id, i);
            CAMERA_INFO_TABLE[i].DeviceID = id;
            CAMERA_INFO_TABLE[i].handler = handler;
            ret = AK_TRUE;
            break;
        }
    }
    return ret;
}
#endif

