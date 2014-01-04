/**
 * @file eng_lunarcalendar.c
 * @brief solar calendar convert lunar calendar source code.
 *
 * @author ANYKA 
 * @date 2009-04-29
 * @version 1.0 
 *
 * @reversion: 
 * @Author: 
 * @date: 
*/

#include "eng_lunarcalendar.h"
#include "Eng_String_UC.h"

#ifdef OS_WIN32
	#include <stdio.h>
#endif


#ifdef SUPPORT_CALENDAR

//gLunarMonthDay's offset and len in file
#define LUNAR_MONTH_DAY_OFFSET	0
#define LUNAR_MONTH_DAY_LEN		400

//gLunarMonth's offset and len in file
#define LUNAR_MONTH_OFFSET		400
#define LUNAR_MONTH_LEN			100

//gLunarHolDay's offset and len in file
#define LUNAR_HOLDAY_OFFSET		500
#define LUNAR_HOLDAY_LEN		2400

typedef enum
{
	TYPE_LUNAR_MONTH_DAY,
	TYPE_LUNAR_MONTH,
	TYPE_LUNAR_HOLDAY
}T_eCALEND_DATATYPE;



const T_U16 calendarFile[] =  {'A',':','/','c','a','l','e','n','d','a','r','.','d','a','t','\0'};

static T_hFILE   gCalendarFd = FS_INVALID_HANDLE;

#if 0
//The lunar year's month is 29 days or 30 days,use 12 or 13 bit to means a lunar year's all months.
//bit 0 means 29 days,bit 1 means 30 days.
const T_U16 gLunarMonthDay[]=
{
	//test date :1901.1.1 --2070.12.31
  0X4ae0, 0Xa570, 0X5268, 0Xd260, 0Xd950, 0X6aa8, 0X56a0, 0X9ad0, 0X4ae8, 0X4ae0,   //1910
  0Xa4d8, 0Xa4d0, 0Xd250, 0Xd548, 0Xb550, 0X56a0, 0X96d0, 0X95b0, 0X49b8, 0X49b0,   //1920
  0Xa4b0, 0Xb258, 0X6a50, 0X6d40, 0Xada8, 0X2b60, 0X9570, 0X4978, 0X4970, 0X64b0,   //1930
  0Xd4a0, 0Xea50, 0X6d48, 0X5ad0, 0X2b60, 0X9370, 0X92e0, 0Xc968, 0Xc950, 0Xd4a0,   //1940
  0Xda50, 0Xb550, 0X56a0, 0Xaad8, 0X25d0, 0X92d0, 0Xc958, 0Xa950, 0Xb4a8, 0X6ca0,   //1950
  0Xb550, 0X55a8, 0X4da0, 0Xa5b0, 0X52b8, 0X52b0, 0Xa950, 0Xe950, 0X6aa0, 0Xad50,   //1960
  0Xab50, 0X4b60, 0Xa570, 0Xa570, 0X5260, 0Xe930, 0Xd950, 0X5aa8, 0X56a0, 0X96d0,   //1970
  0X4ae8, 0X4ad0, 0Xa4d0, 0Xd268, 0Xd250, 0Xd528, 0Xb540, 0Xb6a0, 0X96d0, 0X95b0,   //1980
  0X49b0, 0Xa4b8, 0Xa4b0, 0Xb258, 0X6a50, 0X6d40, 0Xada0, 0Xab60, 0X9370, 0X4978,   //1990
  0X4970, 0X64b0, 0X6a50, 0Xea50, 0X6b28, 0X5ac0, 0Xab60, 0X9368, 0X92e0, 0Xc960,   //2000
  0Xd4a8, 0Xd4a0, 0Xda50, 0X5aa8, 0X56a0, 0Xaad8, 0X25d0, 0X92d0, 0Xc958, 0Xa950,   //2010
  0Xb4a0, 0Xb550, 0Xb550, 0X55a8, 0X4ba0, 0Xa5b0, 0X52b8, 0X52b0, 0Xa930, 0X74a8,   //2020
  0X6aa0, 0Xad50, 0X4da8, 0X4b60, 0X9570, 0Xa4e0, 0Xd260, 0Xe930, 0Xd530, 0X5aa0,   //2030
  0X6b50, 0X96d0, 0X4ae8, 0X4ad0, 0Xa4d0, 0Xd258, 0Xd250, 0Xd520, 0Xdaa0, 0Xb5a0,   //2040
  0X56d0, 0X4ad8, 0X49b0, 0Xa4b8, 0Xa4b0, 0Xaa50, 0Xb528, 0X6d20, 0Xada0, 0X55b0,   //2050
  0X9370, 0X4978, 0X4970, 0X64b0, 0X6a50, 0Xea50, 0X6b20, 0Xab60, 0Xaae0, 0X92e0,   //2060
  0Xc970, 0Xc960, 0Xd4a8, 0Xd4a0, 0Xda50, 0X5aa8, 0X56a0, 0Xa6d0, 0X52e8, 0X52d0,   //2070
  0Xa958, 0Xa950, 0Xb4a0, 0Xb550, 0Xad50, 0X55a0, 0Xa5d0, 0Xa5b0, 0X52b0, 0Xa938,   //2080
  0X6930, 0X7298, 0X6aa0, 0Xad50, 0X4da8, 0X4b60, 0Xa570, 0X5270, 0Xd260, 0Xe930,   //2090
  0Xd520, 0Xdaa0, 0X6b50, 0X56d0, 0X4ae0, 0Xa4e8, 0Xa4d0, 0Xd150, 0Xd928, 0Xd520,   //2100

};

//save the 1901~2100 year's leap month,if the year is no a leaap year,set 0;
//a byte save two years.
const T_U8  gLunarMonth[]=
{
	0X00, 0X50, 0X04, 0X00, 0X20,   //1910
	0X60, 0X05, 0X00, 0X20, 0X70,   //1920
	0X05, 0X00, 0X40, 0X02, 0X06,   //1930
	0X00, 0X50, 0X03, 0X07, 0X00,   //1940
	0X60, 0X04, 0X00, 0X20, 0X70,   //1950
	0X05, 0X00, 0X30, 0X80, 0X06,   //1960
	0X00, 0X40, 0X03, 0X07, 0X00,   //1970
	0X50, 0X04, 0X08, 0X00, 0X60,   //1980
	0X04, 0X0a, 0X00, 0X60, 0X05,   //1990
	0X00, 0X30, 0X80, 0X05, 0X00,   //2000
	0X40, 0X02, 0X07, 0X00, 0X50,   //2010
	0X04, 0X09, 0X00, 0X60, 0X04,   //2020
	0X00, 0X20, 0X60, 0X05, 0X00,   //2030
	0X30, 0Xb0, 0X06, 0X00, 0X50,   //2040
	0X02, 0X07, 0X00, 0X50, 0X03,   //2050
	0X08, 0X00, 0X60, 0X04, 0X00,   //2060
	0X30, 0X70, 0X05, 0X00, 0X40,   //2070
    0X80, 0X06, 0X00, 0X40, 0X03,   //2080
	0X07, 0X00, 0X50, 0X04, 0X80,   //2090
	0X00, 0X60, 0X04, 0X00, 0X20    //2100
   
};

//gLanarHoliDay save the yearly twenty-four solar terms' lunar date.
//The twenty-four solar terms list:
//   1月          2月         3月         4月         5月         6月   
//小寒 大寒   立春  雨水   惊蛰 春分   清明 谷雨   立夏 小满   芒种 夏至
//   7月          8月         9月         10月       11月        12月  
//小暑 大暑   立秋  处暑   白露 秋分   寒露 霜降   立冬 小雪   大雪 冬至

//Date format:
//For example,1901 yearly solar terms:
//  1月     2月     3月   4月    5月   6月   7月    8月   9月    10月  11月     12月
// 6, 21, 4, 19,  6, 21, 5, 21, 6,22, 6,22, 8, 23, 8, 24, 8, 24, 8, 24, 8, 23, 8, 22
// 9, 6,  11,4,   9, 6,  10,6,  9,7,  9,7,  7, 8,  7, 9,  7,  9, 7,  9, 7,  8, 7, 15
//上面第一行数据为每月节气对应日期,15减去每月第一个节气,每月第二个节气减去15得第二行
// 这样每月两个节气对应数据都小于16,每月用一个字节存放,高位存放第一个节气数据,低位存放
//第二个节气的数据,可得下表

const T_U8 gLunarHolDay[]=
{
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1901
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X87, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1902
	0X96, 0XA5, 0X87, 0X96, 0X87, 0X87, 0X79, 0X69, 0X69, 0X69, 0X78, 0X78,   //1903
	0X86, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X78, 0X87,   //1904
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1905
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1906
	0X96, 0XA5, 0X87, 0X96, 0X87, 0X87, 0X79, 0X69, 0X69, 0X69, 0X78, 0X78,   //1907
	0X86, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1908
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1909
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1910
	0X96, 0XA5, 0X87, 0X96, 0X87, 0X87, 0X79, 0X69, 0X69, 0X69, 0X78, 0X78,   //1911
	0X86, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1912
	0X95, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1913
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1914
	0X96, 0XA5, 0X97, 0X96, 0X97, 0X87, 0X79, 0X79, 0X69, 0X69, 0X78, 0X78,   //1915
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1916
	0X95, 0XB4, 0X96, 0XA6, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X87,   //1917
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X77,   //1918
	0X96, 0XA5, 0X97, 0X96, 0X97, 0X87, 0X79, 0X79, 0X69, 0X69, 0X78, 0X78,   //1919
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1920
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X87,   //1921
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X77,   //1922
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X87, 0X79, 0X79, 0X69, 0X69, 0X78, 0X78,   //1923
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1924
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X87,   //1925
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1926
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X87, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1927
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1928
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1929
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1930
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X87, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1931
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1932
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1933
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1934
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1935
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1936
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1937
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1938
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1939
	0X96, 0XA5, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1940
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1941
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1942
	0X96, 0XA4, 0X96, 0X96, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1943
	0X96, 0XA5, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1944
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1945
	0X95, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X77,   //1946
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1947
	0X96, 0XA5, 0XA6, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //1948
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X79, 0X78, 0X79, 0X77, 0X87,   //1949
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X77,   //1950
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X79, 0X79, 0X79, 0X69, 0X78, 0X78,   //1951
	0X96, 0XA5, 0XA6, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //1952
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1953
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X78, 0X79, 0X78, 0X68, 0X78, 0X87,   //1954
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1955
	0X96, 0XA5, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //1956
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1957
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1958
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1959
	0X96, 0XA4, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1960
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1961
	0X96, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1962
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1963
	0X96, 0XA4, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1964
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1965
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1966
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1967
	0X96, 0XA4, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1968
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1969
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1970
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X79, 0X69, 0X78, 0X77,   //1971
	0X96, 0XA4, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1972
	0XA5, 0XB5, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1973
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1974
	0X96, 0XB4, 0X96, 0XA6, 0X97, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X77,   //1975
	0X96, 0XA4, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X89, 0X88, 0X78, 0X87, 0X87,   //1976
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //1977
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X78, 0X87,   //1978
	0X96, 0XB4, 0X96, 0XA6, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X77,   //1979
	0X96, 0XA4, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1980
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X77, 0X87,   //1981
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1982
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X78, 0X79, 0X78, 0X69, 0X78, 0X77,   //1983
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X87,   //1984
	0XA5, 0XB4, 0XA6, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //1985
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1986
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X79, 0X78, 0X69, 0X78, 0X87,   //1987
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //1988
	0XA5, 0XB4, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1989
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //1990
	0X95, 0XB4, 0X96, 0XA5, 0X86, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1991
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //1992
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1993
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1994
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X76, 0X78, 0X69, 0X78, 0X87,   //1995
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //1996
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //1997
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //1998
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //1999
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2000
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2001
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //2002
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //2003
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2004
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2005
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2006
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X69, 0X78, 0X87,   //2007
	0X96, 0XB4, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2008
	0XA5, 0XB3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2009
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2010
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X78, 0X87,   //2011
	0X96, 0XB4, 0XA5, 0XB5, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2012
	0XA5, 0XB3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X87,   //2013
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2014
	0X95, 0XB4, 0X96, 0XA5, 0X96, 0X97, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //2015
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2016
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X87,   //2017
	0XA5, 0XB4, 0XA6, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2018
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //2019
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X86,   //2020
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2021
	0XA5, 0XB4, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2022
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X79, 0X77, 0X87,   //2023
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2024
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2025
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2026
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //2027
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2028
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2029
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2030
	0XA5, 0XB4, 0X96, 0XA5, 0X96, 0X96, 0X88, 0X78, 0X78, 0X78, 0X87, 0X87,   //2031
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2032
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X86,   //2033
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X78, 0X88, 0X78, 0X87, 0X87,   //2034
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2035
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2036
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2037
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2038
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2039
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2040
	0XA5, 0XC3, 0XA5, 0XB5, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2041
	0XA5, 0XB3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2042
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2043
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X88, 0X87, 0X96,   //2044
	0XA5, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2045
	0XA5, 0XB3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X87,   //2046
	0XA5, 0XB4, 0X96, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2047
	0X95, 0XB4, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X88, 0X86, 0X96,   //2048
	0XA4, 0XC3, 0XA5, 0XA5, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X86,   //2049
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X78, 0X78, 0X87, 0X87,   //2050
    0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2051
    0XA5, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X78, 0X87, 0X88, 0X86, 0X96,   //2052
    0XA4, 0XC4, 0XA5, 0XB4, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X96,   //2053
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2054
    0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X78, 0X78, 0X87, 0X87,   //2055
    0XA5, 0XB4, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X96, 0X96,   //2056
    0XA4, 0XB3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2057
    0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2058
    0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0X96, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2059
    0XA5, 0XB4, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X87, 0X96, 0X96,   //2060
    0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2061
    0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2062
    0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0X96, 0X87, 0X87, 0X87, 0X78, 0X87, 0X87,   //2063
    0XA5, 0XB4, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X78, 0X96, 0X96,   //2064
    0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2065
    0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2066
    0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2067
    0XA5, 0XB4, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X78, 0X96, 0X96,   //2068
    0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2069
    0XA4, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2070
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2071
	0XA5, 0XB3, 0XA5, 0XB4, 0XB5, 0XA5, 0X97, 0X87, 0X78, 0X87, 0X96, 0X96,   //2072
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X78, 0X78, 0X88, 0X86, 0X96,   //2073
	0XA4, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X86,   //2074
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X86,   //2075
	0XA5, 0XB4, 0XB5, 0XB4, 0XB5, 0XA5, 0X97, 0X97, 0X87, 0X87, 0X96, 0X96,   //2076
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X78, 0X87, 0X88, 0X86, 0X96,   //2077
	0XA4, 0XC3, 0XA5, 0XB5, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X96,   //2078
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X86,   //2079
	0XA5, 0XB4, 0XB4, 0XB5, 0XB4, 0XA5, 0X97, 0X97, 0X87, 0X87, 0X96, 0X96,   //2080
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X88, 0X86, 0X96,   //2081
	0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X96,   //2082
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X86,   //2083
	0XA5, 0XB3, 0XB4, 0XB4, 0XB5, 0XA5, 0X97, 0X97, 0X87, 0X87, 0X96, 0X96,   //2084
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X88, 0X86, 0X96,   //2085
	0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X87, 0X88, 0X87, 0X78, 0X87, 0X96,   //2086
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2087
	0XA5, 0XB3, 0XB4, 0XB4, 0XB5, 0XA5, 0X97, 0X97, 0X87, 0X87, 0X96, 0X96,   //2088
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X87, 0X96, 0X96,   //2089
	0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2090
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2091
	0XA5, 0XB3, 0XB4, 0XB4, 0XB5, 0XA5, 0X97, 0X97, 0X97, 0X87, 0X96, 0X96,   //2092
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X87, 0X96, 0X96,   //2093
	0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2094
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2095
	0XA5, 0XB3, 0XB4, 0XB4, 0XB5, 0XB5, 0X97, 0X97, 0X97, 0X87, 0X96, 0X96,   //2096
	0XB4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA5, 0X97, 0X87, 0X87, 0X87, 0X96, 0X96,   //2097
	0XA4, 0XC3, 0XA5, 0XB4, 0XA5, 0XA6, 0X97, 0X87, 0X87, 0X78, 0X87, 0X96,   //2098
	0XA5, 0XC3, 0XA5, 0XB5, 0XA6, 0XA6, 0X87, 0X88, 0X88, 0X78, 0X87, 0X86,   //2099
	0XA5, 0XB3, 0XA5, 0XA5, 0XA6, 0XA6, 0X88, 0X88, 0X88, 0X78, 0X87, 0X87,   //2100
};
#endif
/*
const T_S16 gSolarTerm[24][5] = {
								_T("小寒"),
								_T("大寒"),
								_T("立春"),
								_T("雨水"),
								_T("惊蛰"),
								_T("春分"),
								_T("清明"),
								_T("谷雨"),
								_T("立夏"),
								_T("小满"),
								_T("芒种"),
								_T("夏至"),
								_T("小暑"),
								_T("大暑"),
								_T("立秋"),
								_T("处暑"),
								_T("白露"),
								_T("秋分"),
								_T("寒露"),
								_T("霜降"),
								_T("立冬"),
								_T("小雪"),
								_T("大雪"),
								_T("冬至")
							};
*/

/*
const T_S16 gSolarFestival[][40] = {
								_T("0101*元旦"),
								_T("0214 情人节"),
								_T("0308 妇女节"),
								_T("0312 植树节"),
								_T("0315 消费者权益日"),
								_T("0317 St. Patrick's"),
								_T("0401 愚人节"),
								_T("0501 劳动节"),
								_T("0504 青年节"),
								_T("0512 护士节"),
								_T("0512 茵生日"),
								_T("0601 儿童节"),
								_T("0614 Flag Day"),
								_T("0701 建党节 香港回归纪念"),
								_T("0703 炎黄在线诞辰"),
								_T("0718 托普诞辰"),
								_T("0801 建军节"),
								_T("0808 父亲节"),
								_T("0909 毛泽东逝世纪念"),
								_T("0910 教师节"),
								_T("0928 孔子诞辰"),
								_T("1001*国庆节"),
								_T("1006 老人节"),
								_T("1024 联合国日"),
								_T("1111 Veteran's / Remembrance Day"),
								_T("1112 孙中山诞辰纪念"),
								_T("1220 澳门回归纪念"),
								_T("1225 Christmas Day"),
								_T("1226 毛泽东诞辰纪念"),
								_T("")
								};
*/

/*
const T_S16 gLunarFestival[][40] = {
								_T("0100*除夕"),
								_T("0101*春节"),
								_T("0115 元宵节"),
								_T("0505 端午节"),
								_T("0707 七夕情人节"),
								_T("0715 中元节"),
								_T("0815 中秋节"),
								_T("0909 重阳节"),
								_T("1208 腊八节"),
								_T("1223 小年"),
								_T("")
};
*/

static T_U16 ReadCalendarData(T_eCALEND_DATATYPE type,T_U32 offset)
{
	T_U16		ret = 0;
    T_S32       fileOffset;
    T_U8        buffer[4] = {0};

	switch(type)
	{
    case TYPE_LUNAR_MONTH_DAY:
        fileOffset = LUNAR_MONTH_DAY_OFFSET + offset * 2;
        Fwl_FileSeek(gCalendarFd, fileOffset, FS_SEEK_SET);
        Fwl_FileRead(gCalendarFd, buffer, 2);
        ret = (T_U16)(buffer[0]+ (buffer[1] << 8));
        break;
    case TYPE_LUNAR_MONTH:
        fileOffset = LUNAR_MONTH_OFFSET + offset;
        Fwl_FileSeek(gCalendarFd, fileOffset, FS_SEEK_SET);
        Fwl_FileRead(gCalendarFd, buffer, 1);
        ret = buffer[0];
        break;
    case TYPE_LUNAR_HOLDAY:
        fileOffset = LUNAR_HOLDAY_OFFSET + offset;
        Fwl_FileSeek(gCalendarFd, fileOffset, FS_SEEK_SET);
        Fwl_FileRead(gCalendarFd, buffer, 1);
        ret = buffer[0];
        break;
    default:
        break;
	}
    
    return ret;
}
	

static T_U16 GetLeapMonth(T_U16 iLunarYear)
{
	T_U8 flag;
	
//	flag = gLunarMonth[(iLunarYear - START_YEAR)/2];
    flag = (T_U8)ReadCalendarData(TYPE_LUNAR_MONTH, (iLunarYear - START_YEAR)/2);
	
 	return (T_U16)((iLunarYear - START_YEAR)%2 ? flag&0x0f : flag>>4);
}

static T_U32 LunarMonthDays(T_U16 iLunarYear, T_U16 iLunarMonth)
{
	T_U32 height =0 ,low =29;
	T_U32 iBit = 16 - iLunarMonth;
    T_U16 lunarMonthDay;

	if(iLunarYear < START_YEAR) 
	{
		return 30;
	}

  	if(iLunarMonth > GetLeapMonth(iLunarYear) && GetLeapMonth(iLunarYear))
  	{
		   iBit --;
  	}

    lunarMonthDay = ReadCalendarData(TYPE_LUNAR_MONTH_DAY, iLunarYear - START_YEAR);

	//if(gLunarMonthDay[iLunarYear - START_YEAR] & (1<<iBit))
    if(lunarMonthDay & (1<<iBit))
	{
	        low ++;
	}
	    
	if(iLunarMonth == GetLeapMonth(iLunarYear))
	{
	//	if(gLunarMonthDay[iLunarYear - START_YEAR] & (1<< (iBit -1)))
        if(lunarMonthDay & (1<< (iBit -1)))
		{
			height =30;
		}
		else
		{
			 height =29;
		}
	}

	return ( low | (height<<16) );
}

static T_U16 LunarYearDays(T_U16 iLunarYear)
{
	T_U16 days =0;
	T_U16 i;
	T_U32 tmp;
	
	for( i=1; i<=12; i++)
	{ 
        tmp = LunarMonthDays(iLunarYear ,i); 
		days = (T_U16)(((tmp >>16) & 0xffff) + days);
		days = (T_U16)(((tmp) & 0xffff) + days);
	}
   	return days;
}

static T_U32 CalcDateDiff(T_U16 iEndYear, T_U16 iEndMonth, T_U16 iEndDay,
		                    T_U16  iStartYear, T_U16 iStartMonth, T_U16 iStartDay)
{
	T_U16 monthday[]={0, 31, 59 ,90, 120, 151, 181, 212, 243, 273, 304, 334}; 
	T_U32 iDiffDays =(iEndYear - iStartYear)*365;	

	iDiffDays += (iEndYear-1)/4 - (iStartYear-1)/4;
	iDiffDays -= ((iEndYear-1)/100 - (iStartYear-1)/100);
	iDiffDays += (iEndYear-1)/400 - (iStartYear-1)/400;

   	iDiffDays += monthday[iEndMonth-1] + (IsLeapYear(iEndYear)&&iEndMonth>2? 1: 0);
    iDiffDays += iEndDay;

	iDiffDays -= (monthday[iStartMonth-1] + (IsLeapYear(iStartYear)&&iStartMonth>2 ? 1: 0));
    iDiffDays -= iStartDay;	
	
	return iDiffDays;
}

static T_U8  CalcLunarDate(T_U16 *iYear, T_U16 *iMonth ,T_U16 *iDay, T_U32 iSpanDays)
{
	T_U8 rcode =0;
	T_U32 tmp;
	
   	if(iSpanDays <49)
	{
		*iYear  = START_YEAR-1;
		if(iSpanDays <19)
		{ 
		  *iMonth = 11;  
		  *iDay   = (T_U16)(11+(T_U16)(iSpanDays));
		}
		else
		{
			*iMonth = 12;
			*iDay   =  (T_U16)((T_U16)(iSpanDays) -18);
		}
		return  rcode;
	}

	iSpanDays -=49;
	*iYear  = START_YEAR;
	*iMonth = 1;
	*iDay   = 1;

	tmp = LunarYearDays(*iYear); 

	while(iSpanDays >= tmp)
	{
		iSpanDays -= tmp;
		tmp = LunarYearDays(++*iYear);
	}
 
	tmp = LunarMonthDays(*iYear, *iMonth);
	tmp &= 0xffff;
	
	while(iSpanDays >= tmp)
	{
		iSpanDays -= tmp;
	    	if(*iMonth == GetLeapMonth(*iYear))
		{
			tmp  = (LunarMonthDays(*iYear, *iMonth))>>16;
			tmp &= 0xffff;
			
			if(iSpanDays < tmp)	
			{
			   rcode = 1;
			   break;
			}
			iSpanDays -= tmp;
		}
		tmp = LunarMonthDays(*iYear, ++*iMonth);
		tmp &= 0xffff;
	}

	*iDay = (T_U16)((iSpanDays & 0xffff) + (*iDay));
	
	return rcode;
}


T_U16  GetLunarHolDay(T_U16 iYear, T_U16 iMonth, T_U16 iDay)
{
	T_U8 flag;// = gLunarHolDay[(iYear - START_YEAR)*12+iMonth -1];
	T_U16 day;

    flag = (T_U8)ReadCalendarData(TYPE_LUNAR_HOLDAY, (iYear - START_YEAR)*12+iMonth -1);

	if(iDay <15)
	{
		 day= (T_U16)(15 - ((flag>>4)&0x0f));
	}
	else
	{
		day = (T_U16)(((flag)&0x0f)+15);
	}

	if(iDay == day)
	{
		return (T_U16)((iMonth-1) *2 + (iDay>15? 1: 0) +1); 
	}

	return 0;
}

T_BOOL Eng_LunarCalendarCovert(const T_SYSTIME systime, T_pLUNARCALENDAR lunar)
{
	T_U16 iYear,iMonth,iDay;


  	memset((T_U8*)lunar , 0,  sizeof(T_LUNARCALENDAR));
	
	if( (systime.year > END_YEAR) ||( systime.year < START_YEAR ) ||(	systime.month> 12) ||
		( systime.month < 1 ) ||(systime.day > 31) ||( systime.day < 1) ||( lunar == AK_NULL ) )
	{
		return AK_FALSE;
	}

	iYear = systime.year;
	iMonth = systime.month;
	iDay = systime.day;

	lunar->weekDay = WeekDay( iYear, iMonth, iDay);
	lunar->isLeap = IsLeapYear( iYear);

	gCalendarFd = Fwl_FileOpen(calendarFile, _FMODE_READ, _FMODE_READ);
    if(gCalendarFd == _FOPEN_FAIL)
    {
        AK_DEBUG_OUTPUT("**ReadCalendarData:open file fail\n");
        return AK_FALSE;		
    }
	
	CalcLunarDate(&lunar->cYear, &lunar->cMonth, &lunar->cDay, CalcDateDiff(iYear, iMonth, iDay,1901,1,1));
    Fwl_FileClose(gCalendarFd);
    gCalendarFd = FS_INVALID_HANDLE;

	return AK_TRUE;
}

#endif

T_BOOL IsLeapYear(T_U16 iYear)
{
	return (T_BOOL)(!(iYear%4)&&(iYear%100) || !(iYear%400));
}

T_U16 WeekDay(T_U16 iYear, T_U16 iMonth, T_U16 iDay)
{
   	
	T_U16 monthday[]={0,3,3,6,1,4,6,2,5,0,3,5};
	T_U16 iDays = (T_U16) ((iYear-1)%7 + (iYear-1)/4 - (iYear-1)/100 +(iYear-1)/400);
	
	iDays = (T_U16)(iDays + (monthday[iMonth-1] +iDay));

 	if(IsLeapYear(iYear) && iMonth>2)
 	{
		iDays++;
 	}

	return (T_U16)(iDays%7);
}

T_U16 MonthDays(T_U16 iYear, T_U16 iMonth)
{
	switch(iMonth)
	{
	case 1:case 3:case 5:case 7:case 8:case 10:case 12:
		return 31;
		break;
		
	case 4:case 6:case 9:case 11:
		return 30;
		break;

	case 2:
		if(IsLeapYear(iYear))
		{
			return 29;
		}
		else
		{
			return 28;
		}
		break;
	}
	return 0;
}


