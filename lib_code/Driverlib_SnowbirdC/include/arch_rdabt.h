
#ifndef __RDABT_UART_H__
#define __RDABT_UART_H__

#ifdef __cplusplus
extern "C"{
#endif

/**
* dBm lvl select
*/
#define BT_POWER_DBM_LVL        9

#define RDABT_DEFAULT_BR        (115200)
#define RDA_UART_BAUD_RATE      (921600)
#define RDA_HFP_LINK_MODE	0

/*******************************************************************************
 * @brief   rdabt_poweron_start
 * @author  liuhuadong
 * @date    2013.02.25
 * @param   T_U8 power_pin
 * @param   T_U8 uart_id
 * @return  int
*******************************************************************************/
int rdabt_poweron_start(T_U8 power_pin, T_U8 uart_id);

/*******************************************************************************
 * @brief   rdabt_poweroff
 * @author  liuhuadong
 * @date    2013.02.25
 * @param   T_U8 power_pin
 * @param   T_U8 uart_id
 * @return  int
*******************************************************************************/
int rdabt_poweroff(void);

#ifdef __cplusplus
}
#endif

#endif


