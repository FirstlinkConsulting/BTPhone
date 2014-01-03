#ifndef __SOUND_BIN_MAP_H__
#define __SOUND_BIN_MAP_H__
#include "Anyka_types.h"
typedef struct {
	T_U32 VoiceID;
	T_U32 OffsetFile;
	T_U32 SzFile;
	T_CHR *PathFile;
}T_BIN_MAP;
#pragma arm section rodata = "_VOICE_TIP_DATA_"
#define SOUND_BIN_EXIST	(1)
const T_BIN_MAP bin_map[]={
{eBTPLY_SOUND_TYPE_CHARGEOK ,0,	13986, "chargeok_cn.mp3"},
{eBTPLY_SOUND_TYPE_CHARGING ,14080,	11618, "charging_cn.mp3"},
{eBTPLY_SOUND_TYPE_CHILD ,25856,	7622, "child_cn.mp3"},
{eBTPLY_SOUND_TYPE_CLASSICAL ,33536,	7622, "classical_cn.mp3"},
{eBTPLY_SOUND_TYPE_CONNECTED ,41216,	11988, "connected_cn.mp3"},
{eBTPLY_SOUND_TYPE_DANCE ,53248,	7622, "dance_cn.mp3"},
{eBTPLY_SOUND_TYPE_DI ,60928,	1628, "di_cn.mp3"},
{eBTPLY_SOUND_TYPE_DU ,62720,	15984, "du_cn.mp3"},
{eBTPLY_SOUND_TYPE_ECHO ,78848,	7992, "echo_cn.mp3"},
{eBTPLY_SOUND_TYPE_EIGHT ,87040,	3996, "eight_cn.mp3"},
{eBTPLY_SOUND_TYPE_FIVE ,91136,	3996, "five_cn.mp3"},
{eBTPLY_SOUND_TYPE_FM ,95232,	9990, "FM_cn.mp3"},
{eBTPLY_SOUND_TYPE_FOUR ,105472,	4662, "four_cn.mp3"},
{eBTPLY_SOUND_TYPE_LINEIN ,110336,	12950, "linein_cn.mp3"},
{eBTPLY_SOUND_TYPE_LOWESTPOWER ,123392,	10656, "lowestpower_cn.mp3"},
{eBTPLY_SOUND_TYPE_LOWPOWER ,134144,	9620, "lowpower_cn.mp3"},
{eBTPLY_SOUND_TYPE_MICREC ,143872,	12654, "micrec_cn.mp3"},
{eBTPLY_SOUND_TYPE_NINE ,156672,	5328, "nine_cn.mp3"},
{eBTPLY_SOUND_TYPE_NOFREE ,162048,	11618, "nofree_cn.mp3"},
{eBTPLY_SOUND_TYPE_NOMUSIC ,173824,	11322, "nomusic_cn.mp3"},
{eBTPLY_SOUND_TYPE_NORMAL ,185344,	6660, "normal_cn.mp3"},
{eBTPLY_SOUND_TYPE_ONE ,192256,	3626, "one_cn.mp3"},
{eBTPLY_SOUND_TYPE_PAIRING ,196096,	11988, "pairing_cn.mp3"},
{eBTPLY_SOUND_TYPE_PERSONAL ,208128,	9990, "personal_cn.mp3"},
{eBTPLY_SOUND_TYPE_POINT ,218368,	4662, "point_cn.mp3"},
{eBTPLY_SOUND_TYPE_POP ,223232,	7622, "pop_cn.mp3"},
{eBTPLY_SOUND_TYPE_RECPLAY ,230912,	10656, "recplay_cn.mp3"},
{eBTPLY_SOUND_TYPE_ROBOT ,241664,	9324, "robot_cn.mp3"},
{eBTPLY_SOUND_TYPE_ROCK ,251136,	7622, "rock_cn.mp3"},
{eBTPLY_SOUND_TYPE_SEARCHING ,258816,	11618, "searching_cn.mp3"},
{eBTPLY_SOUND_TYPE_SEVEN ,270592,	4662, "seven_cn.mp3"},
{eBTPLY_SOUND_TYPE_SIX ,275456,	3626, "six_cn.mp3"},
{eBTPLY_SOUND_TYPE_TCARD ,279296,	9990, "tcard_cn.mp3"},
{eBTPLY_SOUND_TYPE_THREE ,289536,	4662, "three_cn.mp3"},
{eBTPLY_SOUND_TYPE_TOMCAT ,294400,	9324, "tomcat_cn.mp3"},
{eBTPLY_SOUND_TYPE_TWO ,303872,	3626, "two_cn.mp3"},
{eBTPLY_SOUND_TYPE_UAC ,307712,	14282, "uac_cn.mp3"},
{eBTPLY_SOUND_TYPE_UDISKPLAY ,322048,	10656, "udiskplay_cn.mp3"},
{eBTPLY_SOUND_TYPE_UDISK ,332800,	15318, "udisk_cn.mp3"},
{eBTPLY_SOUND_TYPE_UPDATE ,348160,	12284, "update_cn.mp3"},
{eBTPLY_SOUND_TYPE_VOCAL ,360448,	9620, "vocal_cn.mp3"},
{eBTPLY_SOUND_TYPE_XBASS ,370176,	11618, "xbass_cn.mp3"},
{eBTPLY_SOUND_TYPE_ZERO ,381952,	3626, "zero_cn.mp3"}};
#pragma arm section rodata
#endif
