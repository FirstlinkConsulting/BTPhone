#include "anyka_types.h"
#include "prog_manager.h"
    
#pragma arm section rodata = "_separator_data_"
const unsigned char separator[0x1000] = {0x0};
#pragma arm section

#pragma arm section rodata = "_separator_data2_"
const unsigned char separator2[0x1000] = {0x0};
#pragma arm section

/*spi�汾Ϊ���Ż��ڴ�ʹ�ã��ѽ�����ER_ZI�ηŵ���פ��
��Ϊ�˲�Ӱ�콻�����豣��ER_ZI�����Զ���һ������ռ�ռ�*/
#if (STORAGE_USED == SPI_FLASH)     
int zi_reserved;
#endif 

//the end of file
