/*******************************************************************************
 * @file    LCD_r61580.c
 * @brief   the source code of r61580 lcd device
 * @        transmit LCD_r61580(8bit 320*240) LCD specific functions to lcd.c
 * @        Copyright (C) 2010 Anyka (GuangZhou) MicroElectronics Technology Co., Ltd.
 * @author  lhd
 * @date    2011-05-11
 * @version 1.0
 * @ref     AK11xx technical manual.
*******************************************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "lcd_device.h" 
#include "arch_init.h"
#include "drv_cfg.h"
#if (DRV_SUPPORT_LCD > 0) && (LCD_R61580 > 0)


#define DEVICE_NAME         "LCD:R61580"

//REG_LCD_CTRL
#define WR_LOW              3
#define WR_CYCLE            10

#define LCD_WIDTH           240
#define LCD_HIGHT           320

#define LCD_R61580_TYPE     0x61580
#define BUSWIDTH_8BIT 

#ifdef BUSWIDTH_8BIT
#define LCD_BUS_MODE   0             ///< data bus width(8,9,16,18 or 24)  
#else
#define LCD_BUS_MODE   0x01
#endif


static const T_U16 init_cmdset[][2] = 
{   
    //************* Start Initial Sequence **********//
    {0x0000, 0x0000},   
    {0x0000, 0x0000},   
    {0x0000, 0x0000},   
    {0x0000, 0x0000},       // set the back porch and front porch
    {0x00a4, 0x0001},       // FMARK function
    {DELAY_FLAG, 1000},

    {0x0060, 0xa700},
    {0x0008, 0x0808},                

    {0x0030, 0x0c00},
    {0x0031, 0x5b0a},
    {0x0032, 0x0804},
    {0x0033, 0x1017},
    {0x0034, 0x2300},
    {0x0035, 0x1700},
    {0x0036, 0x6309},//{0x0036, 0x6309},
    {0x0037, 0x0c0c},
    {0x0038, 0x100c},
    {0x0039, 0x2232},
    
    {0x0090, 0x001d},
    {0x0010, 0x0530},
    {0x0011, 0x0237},
    {0x0012, 0x01bc},                 // DC1[2:0], DC0[2:0], VC[2:0]
    {0x0013, 0x0f00},                 // VREG1OUT voltage

    {DELAY_FLAG, 50000},
    {DELAY_FLAG, 50000},
    
    {0x0001, 0x0100},                 // DC1[2:0], DC0[2:0], VC[2:0]
    {0x0002, 0x0200},                 // VREG1OUT voltage
    {0x0009, 0x0001},          //0x0004), //0x0005),                // VCM[4:0] for VCOMH
    {0x000a, 0x0008},            // Frame Rate and Color Control-----16M_EN, Dither, FR_SEL[1:0] 
    {0x000c, 0x0000},       // Horizontal GRAM Start Address-----HSA[7:0]
    {0x000d, 0xd000},       // Horizontal GRAM End Address-----HEA[7:0]
    {0x000e, 0x0030},       // Vertical GRAM Start Address-----VSA[8:0]
    {0x000f, 0x0000},       // Vertical GRAM Start Address-----VEA[8:0]
    
    {0x0020, 0x0000},                 // DC1[2:0], DC0[2:0], VC[2:0]
    {0x0021, 0x0000},                 // VREG1OUT voltage
    {0x0029, 0x0061},          //0x1300), //0x1800),                // VDV[4:0] for VCOM amplitude
    {0x0050, 0x0000},          //0x0004), //0x0005),                // VCM[4:0] for VCOMH
    {0x0051, 0xd0ef},            // Frame Rate and Color Control-----16M_EN, Dither, FR_SEL[1:0] 
    {0x0052, 0x0000},       // Horizontal GRAM Start Address-----HSA[7:0]
    {0x0053, 0x013f},       // Horizontal GRAM End Address-----HEA[7:0]
    {0x0061, 0x0001},       // Vertical GRAM Start Address-----VSA[8:0]
    {0x006a, 0x0000},
    
    {0x0080, 0x0000},       // GS, NL[5:0], SCN[5:0]
    {0x0081, 0x0000},       // NDL,VLE, REV
    {0x0082, 0x005f},       // VL[8:0]
    {0x0093, 0x0701},
    
    {0x0007, 0x0100},       // Display Control 1-----262K color and display ON

    {END_FLAG, END_FLAG}
};

static const T_U16 turnon_cmdset[][2] = 
{
    {0x0007, 0x0100},
    {END_FLAG, END_FLAG}
};

static const T_U16 turnoff_cmdset[][2] = 
{
    {0x0007, 0x0000}, 
    {END_FLAG, END_FLAG}
};

static T_VOID lcd_out(T_U16 reg_index, T_U16 reg_data)
{
    index_out(reg_index);
    data_out(reg_data);
}

static T_VOID send_cmd(const T_U16 pCmdSet[][2])
{
    int i = 0;

    for(i=0; AK_TRUE; i++)
    {
        if ((END_FLAG == pCmdSet[i][0]) && (END_FLAG == pCmdSet[i][1]))
        {
            break;
        }
        else if (DELAY_FLAG == pCmdSet[i][0])
        {
            delay_us(pCmdSet[i][1]);
        }
        else
        {
            lcd_out(pCmdSet[i][0], pCmdSet[i][1]);
        }
    }
}

T_VOID lcd_device_initial(T_VOID)
{
    drv_print(DEVICE_NAME, 0, AK_TRUE);

    send_cmd(init_cmdset);
}

T_VOID lcd_device_turn_on(T_VOID)
{
    send_cmd(turnon_cmdset);
}

T_VOID lcd_device_turn_off(T_VOID)
{
    send_cmd(turnoff_cmdset);
}

T_BOOL lcd_device_enable_set_contrast(T_VOID)
{
    return AK_FALSE;
}

T_VOID lcd_device_set_mode(T_eLCD_DEGREE avimode)
{
    switch (avimode)
    {
        case DEGREE_0:
            lcd_out(0x0003, 0x1030);
            break;

        case DEGREE_90:
            lcd_out(0x0003, 0x1018);
            break;

        case DEGREE_180:
            lcd_out(0x0003, 0x1000);
            break;

        case DEGREE_270:
            lcd_out(0x0003, 0x1028);
            break;
        
        default:
            break;
    }

    lcd_avi_mode = avimode;    
}


static T_VOID lcd_device_set_disp_address(T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2)
{

    T_U32 HardwareX1, HardwareX2, HardwareY1, HardwareY2;
    T_U32 lcd_hardware_w, lcd_hardware_h;
   
    lcd_hardware_w = LCD_WIDTH;
    lcd_hardware_h = LCD_HIGHT;    
    
    switch(lcd_avi_mode)
    {
        case DEGREE_0:

            lcd_out(0x0020, x1);
            lcd_out(0x0021, y1); 
            HardwareX1 = x1;
            HardwareX2 = x2;
            HardwareY1 = y1;
            HardwareY2 = y2;      
            break;
            
        case DEGREE_90:
            lcd_out(0x0020, y1);
            lcd_out(0x0021, lcd_hardware_h - x1 - 1);
            HardwareX1 = y1;
            HardwareX2 = y2;
            HardwareY1 = lcd_hardware_h - x1 - 1;
            HardwareY2 = lcd_hardware_h - x2 - 1;            
            break;
            
        case DEGREE_180:
            lcd_out(0x0020, lcd_hardware_w - x1 -1);
            lcd_out(0x0021, lcd_hardware_h - y1 -1);
            HardwareX1 = lcd_hardware_w - x1 -1;
            HardwareX2 = lcd_hardware_w - x2 -1;
            HardwareY1 = lcd_hardware_h - y1 -1;
            HardwareY2 = lcd_hardware_h - y2 -1;                                 
            break;        
            
        case DEGREE_270:
            lcd_out(0x0020, lcd_hardware_w - y1 -1);
            lcd_out(0x0021, x1);                      
            HardwareX1 = lcd_hardware_w - y1 -1;
            HardwareX2= lcd_hardware_w - y2 -1;
            HardwareY1= x1;
            HardwareY2 = x2;                                                                                
            break;
            
        default :
            drv_print("The rotate is wrong !", 0, 1);
            while(1);
    }
    
    if(HardwareX2 > HardwareX1)                              //set x range
    {   
        lcd_out(0x0050, HardwareX1);                         //start of x
        lcd_out(0x0051, HardwareX2);
    }
    else
    {
        lcd_out(0x0050, HardwareX2);                         //start of x
        lcd_out(0x0051, HardwareX1);    
    }
    
    if(HardwareY2 > HardwareY1)                              //set y range
    {       
        lcd_out(0x0052, HardwareY1);                         //start of y
        lcd_out(0x0053, HardwareY2);
    }
    else
    {
        lcd_out(0x0052, HardwareY2);                         //start of y
        lcd_out(0x0053, HardwareY1);
    }
    index_out(0x22);   
}

static T_U32 lcd_device_read_type(T_VOID)
{

    return LCD_R61580_TYPE;
}

static T_VOID lcd_device_start(T_VOID)
{
    index_out(0x0007);
    data_out(0x0100);
}


T_VOID lcd_device_set_contrast(T_U8 contrast)
{

}

static T_LCD_FUNCTION_HANDLER r61580_function_handler =
{
    WR_LOW,                 ///< Clock cycle, Unit Hz
    WR_CYCLE,
    LCD_BUS_MODE,
    0,
    AK_NULL,                ///< refer to T_RGBLCD_INFO define
    lcd_device_read_type,
    lcd_device_initial,     ///< init for lcd driver 
    lcd_device_turn_on,        
    lcd_device_turn_off,            
    lcd_device_set_disp_address,
    lcd_device_set_mode,
    lcd_device_start    
};

static int lcd_device_reg(void)
{
    lcd_reg_dev(LCD_R61580_TYPE, &r61580_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_device_reg)
#ifdef __CC_ARM
#pragma arm section
#endif


#endif  //(DRV_SUPPORT_LCD > 0) && (LCD_R61580 > 0)

