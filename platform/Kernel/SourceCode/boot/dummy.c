#include "anyka_types.h"
#include "prog_manager.h"
    
#pragma arm section rodata = "_separator_data_"
const unsigned char separator[0x1000] = {0x0};
#pragma arm section

#pragma arm section rodata = "_separator_data2_"
const unsigned char separator2[0x1000] = {0x0};
#pragma arm section

/*spi版本为了优化内存使用，已将整个ER_ZI段放到常驻，
但为了不影响交换核需保留ER_ZI，所以定义一个数据占空间*/
#if (STORAGE_USED == SPI_FLASH)     
int zi_reserved;
#endif 

//the end of file
