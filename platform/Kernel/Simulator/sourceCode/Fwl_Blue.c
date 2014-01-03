#include "Fwl_blue.h"
#include "arch_rdabt.h"
#include "Fwl_Serial.h"

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
    rdabt_adp_uart_start(); 
    rdabt_adp_uart_configure(rx_proc);

	return 1;
}

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
    rdabt_adp_uart_stop();
	return 1;
}


