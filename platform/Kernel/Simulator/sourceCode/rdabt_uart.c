#include "Anyka_types.h"
#include "arch_rdabt.h"
#include "Gbl_Define.h"
#include "Fwl_Serial.h"






/*******************************************************************************
T_BT_UART_COM_PARM  Ϊ�������ڽ������ݽṹ��
����: ���ջ�������СΪ RDABT_UART_BUFF_SIZE �ֽ�
��Ա����UARTComHead ����ǰ���λ���������Ч���ݵ���ʼλ�ã�
��Ա����UARTComTail ����ǰ���λ���������Ч���ݵĽ���λ�ã�
�����ڽ��յ����ݺ󣬴�UARTComTail ��ʼ��������������
��������������RDABT_UART_BUFF_SIZE ��1�ֽ����ݺ����µ����ݽ��ս���ʱ��
����������������ʶoverflag ��
��Ȼ���������ǲ����������ݶ�ʧ�ģ����Դ�ʱ��Ҫ���ĳ��򣬿��Կ���
�Ӵ󻺳���RDABT_UART_BUFF_SIZE�Ĵ�С�������Ż����򣬱�֤���ݵõ���ʱ����


********************************************************************************
������ͨ�����ڽ��յ������ݣ����԰�����ʽ�����ġ�
���ݰ���ʽ����:

��һ���ֽ������ͣ�=2�����ݣ�=3��������=4���¼���

ÿһ�����ݰ� ��[��ͷ] +[���ݳ���]+[����......] ��ɡ�

���ڲ�ͬ�����ݰ�����
��Ӧ�İ�ͷ������      {4,                       3,                        2};
��ͷ�������ݳ���ֵ{2�ֽ�      ��  1�ֽ�      ��1�ֽ�}
��֮�������ݣ����ݳ��������ݳ���ֵ���塣


********************************************************************************
AK10оƬ�����Ľ����ж�������: FULL / ThreshHold/ Timeout ;


�������յ���������һ�����������ݰ�ʱ����������ⷢ��
������Ϣ���������⼰ʱ�������ݣ����ͷŴ��ڻ�����ռ��
�����⻺���������
�������ȡ����ʱ����BT_UART_GetData()���������������ݣ���
��ռ�õĻ�������
�������ȡ����ʱ���Ӵ��ڻ�������UARTComHeadλ�ÿ�ʼ��

*/



/**
const static T_U8 rdabt_change_baudrate[] = 
#if 0
    {0x01,0x02,0xfd,0x0a,0x00,0x01,0x60,0x00,0x00,0x80,0x00,0x10,0x0e,0x00,
     0x01,0x02 ,0xfd ,0x0a ,0x00 ,0x01 ,0x40 ,0x00 ,0x00 ,0x80 ,0x00 ,0x01 ,0x00 ,0x00};
#else
    {0x01, 0x34, 0xfc, 0x04, 0x00, 0x10, 0x0e, 0x00};//{0x01, 0x34, 0xfc, 0x04, 0x00, 0x10, 0x0e, 0x00};//{0x01,0x77,0xfc,0x01,0x00};
#endif
*/
#pragma arm section code = "_ANYKA_HFP_CODE_"
extern int rdabt_poweron_start(void);
extern int rdabt_poweroff(void);

T_U16 rdabt_adp_uart_tx(T_U8 *buf, T_U16 num_to_send)
{
    return  Fwl_UartWriteData((T_U8*)buf, (T_U32)num_to_send);
}
#if 0

T_VOID rdabt_adp_uart_protect(T_VOID)
{
//  uart_rx_lock();
}

T_VOID rdabt_adp_uart_unprotect(T_VOID)
{
 // uart_rx_unlock();
}

T_U16 rdabt_uart_tx_sco_data(T_U8 *pdu, T_U16 handle, T_U16 length)
{
#if 1
    pdu[0] = 0x03;
    pdu[1] = handle&0xff;
    pdu[2] = (handle>>8)&0xff;
    pdu[3] = length;
    
    return rdabt_adp_uart_tx(pdu, length+4);
#else
    T_U8 pkt_head[4];

    pkt_head[0] = 0x03;
    pkt_head[1] = handle&0xff;
    pkt_head[2] = (handle>>8)&0xff;
    pkt_head[3] = length;

    rdabt_adp_uart_tx(pkt_head, 4);
    return rdabt_adp_uart_tx(pdu, length);
#endif
}


#endif

#pragma arm section code


T_VOID rdabt_adp_uart_start(T_VOID)
{
	AkDebugOutput("rdabt_adp_uart_start():%d\n", RDABT_DEFAULT_BR);
    Fwl_UartInit(RDABT_DEFAULT_BR);

    // must  poweron before uart start
    rdabt_poweron_start(); 
}

T_VOID rdabt_adp_uart_stop(T_VOID)
{
    AkDebugOutput("rdabt_adp_uart_stop()\n");
    Fwl_UartFree();
    rdabt_poweroff();
}


T_U32 rdabt_adp_uart_error(T_VOID)
{
    return 0;
}

T_VOID rdabt_adp_uart_configure(T_VOID (*rx_proc)(T_U8 *data, T_U32 len))
{
   	Fwl_UartSetCallback(rx_proc); 
	
	Fwl_UartSetBaudrate(RDA_UART_BAUD_RATE);
	AkDebugOutput("rdabt_adp_uart_configure():%d\n", RDA_UART_BAUD_RATE);
}



