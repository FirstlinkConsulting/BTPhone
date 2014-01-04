#ifndef _FWL_BLUE_H_
#define _FWL_BLUE_H_
#include "Anyka_types.h"

/*********************************************************************
  Function:         Fwl_BlueInit
  Description:      initialize blue hardware.
  Input:             rx_proc  the callback of reading uart data
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueInit(T_VOID (*rx_proc)(T_U8 *data, T_U32 len));

/*********************************************************************
  Function:         Fwl_BlueSendData
  Description:     send data to blue device.
  Input:            T_U8 *data: the data point
  			   T_U16 length :the data length
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueSendData(T_U8 *data, T_U16 length);

/*********************************************************************
  Function:         Fwl_BlueDeInit
  Description:     destroy the blue device and resource.
  Input:             T_VOID
  Return:           T_VOID
  Author:           
  Data:             2013-5-16
**********************************************************************/

T_S32 Fwl_BlueDeInit(T_VOID);


#endif

