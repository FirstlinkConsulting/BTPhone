#include "Fwl_blue.h"
#include "arch_rdabt.h"
#include "arch_uart.h"
#include "Fwl_Serial.h"
#include "gpio_define.h"
#include "Eng_debug.h"

/*********************************************************************
  Function:         Fwl_BlueInit
  Description:      initialize blue hardware.
  Input:             rx_proc  the callback of reading uart data
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueInit(T_VOID (*rx_proc)(T_U8 *data, T_U32 len))
{
    AkDebugOutput("rdabt_adp_uart_start(%d)\n",RDA_UART_BAUD_RATE);
    Fwl_UartInit(RDABT_DEFAULT_BR);

    // must  poweron before uart start
    rdabt_poweron_start(GPIO_BT_LDO_ON, SERIAL_UART); 

	Fwl_UartSetCallback(rx_proc);
	
    Fwl_UartSetBaudrate(RDA_UART_BAUD_RATE);

	return 1;
}

#pragma arm section code = "_SYS_BLUE_CODE_"

/*********************************************************************
  Function:         Fwl_BlueSendData
  Description:     send data to blue device.
  Input:            T_U8 *data: the data point
  			   T_U16 length :the data length
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueSendData(T_U8 *data,T_U16 length)
{
	Fwl_UartWriteData(data, length);
	return length;
}
#pragma arm section code


/*********************************************************************
  Function:         Fwl_BlueDeInit
  Description:     destroy the blue device and resource.
  Input:             T_VOID
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueDeInit(T_VOID)
{
    AkDebugOutput("rdabt_adp_uart_stop()\n");
    Fwl_UartFree();
    rdabt_poweroff();
	return 1;
}


