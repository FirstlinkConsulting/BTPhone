
#ifndef __RDABT_UART_H__
#define __RDABT_UART_H__
#include "eng_debug.h"

#ifdef __cplusplus
extern "C"{
#endif


#define RDABT_DEFAULT_BR        (115200)
#define RDA_UART_BAUD_RATE      (921600)

T_VOID rdabt_adp_uart_stop(T_VOID);
T_VOID rdabt_adp_uart_start(T_VOID);
T_U16 rdabt_adp_uart_tx(T_U8 *buf, T_U16 num_to_send);
T_U32 rdabt_adp_uart_error(T_VOID);
T_VOID rdabt_adp_uart_configure(T_VOID (*rx_proc)(T_U8 *data, T_U32 len));
T_VOID rdabt_adp_uart_protect(T_VOID);
T_VOID rdabt_adp_uart_unprotect(T_VOID);
T_U16 rdabt_uart_tx_sco_data(T_U8 *pdu, T_U16 handle, T_U16 length);

#ifdef __cplusplus
}
#endif

#endif


