#include "Anyka_types.h"
#include "arch_rdabt.h"
#include "Gbl_Define.h"
#include "Fwl_Serial.h"






/*******************************************************************************
T_BT_UART_COM_PARM  为蓝牙串口接收数据结构体
其中: 接收缓冲区大小为 RDABT_UART_BUFF_SIZE 字节
成员变量UARTComHead 代表当前环形缓冲区中有效数据的起始位置；
成员变量UARTComTail 代表当前环形缓冲区中有效数据的结束位置；
当串口接收到数据后，从UARTComTail 开始拷贝到缓冲区；
当缓冲区保存了RDABT_UART_BUFF_SIZE －1字节数据后，有新的数据接收进来时，
会给出缓冲区溢出标识overflag 。
当然，蓝牙库是不允许有数据丢失的，所以此时需要更改程序，可以考虑
加大缓冲区RDABT_UART_BUFF_SIZE的大小，或者优化程序，保证数据得到及时处理。


********************************************************************************
蓝牙库通过串口接收到的数据，是以包的形式给出的。
数据包格式如下:

第一个字节是类型，=2是数据，=3是语音，=4是事件。

每一个数据包 由[包头] +[数据长度]+[数据......] 组成。

对于不同的数据包类型
对应的包头长度是      {4,                       3,                        2};
包头后是数据长度值{2字节      ，  1字节      ，1字节}
再之后是数据，数据长度由数据长度值定义。


********************************************************************************
AK10芯片正常的接收中断有三种: FULL / ThreshHold/ Timeout ;


当串口收到的数据是一个完整的数据包时，会给蓝牙库发送
数据消息，让蓝牙库及时处理数据，并释放串口缓冲区占用
，以免缓冲区溢出。
蓝牙库获取数据时调用BT_UART_GetData()函数，拷贝走数据，释
放占用的缓冲区。
蓝牙库获取数据时，从串口缓冲区中UARTComHead位置开始。

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



