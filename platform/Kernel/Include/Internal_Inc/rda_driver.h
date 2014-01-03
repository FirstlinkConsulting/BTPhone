#ifndef _RDA_DRIVER_H_
#define _RDA_DRIVER_H_
void rdabt_poweron_init(void);
int rdabt_poweroff(void);
int uart_check_error(void);
int RingStrm_GetReadLength(void);
int HCI_GetReceiveBuffer(void);
int BtMgr_SetLocalAddr(void);
int BtMgr_SetLocalName(void);
int BtMgr_EnableDiscoverable(void);
int BtMgr_LinkKeyRsp(void);
T_BOOL uart_close(T_eUART_ID id);
int rdabt_a2dp_connect(void);
int BtMgr_EnableConnectable(void);
int BtMgr_EnableSecurity(void);
int rdabt_a2dp_Active(void);
int rdabt_opp_Active(void);
int BtMgr_PinRsp(void);
#endif

