/*
* @file Gbl_PowerVal.h
* @brief define the power value
* @author lhs
* @date 2013-06-20
* @version 1.0
*/

#ifndef _GBL_POWERVAL_H_
#define _GBL_POWERVAL_H_

#define BATTERY_VALUE_CHARGE        4100

#define BATTERY_VALUE_SDOWN         3450
#define BATTERY_VALUE_WARN          3550
#define BATTERY_VALUE_MIN           3450
#define BATTERY_VALUE_MAX           3920
#define BATTERY_VALUE_TEST          3700
#define BATTERY_VALUE_AVDD          3300
#define BATTERY_VALUE_NOR_L1        3650
#define BATTERY_VALUE_NOR_L2        3700
#define BATTERY_VALUE_NOR_L3        3800
#define BATTERY_VALUE_BASE_VAL      2500 //check battery is in of value

#define BATTERY_STAT_NULL           0
#define BATTERY_STAT_NOR_L0         1
#define BATTERY_STAT_NOR_L1         2
#define BATTERY_STAT_NOR_L2         3
#define BATTERY_STAT_NOR_L3         4           
#define BATTERY_STAT_NOR_FULL       5
#define BATTERY_STAT_LOW_WARN       6
#define BATTERY_STAT_LOW_SHUTDOWN   7
#define BATTERY_STAT_EXCEEDVOLT     8

#endif

//the end of files