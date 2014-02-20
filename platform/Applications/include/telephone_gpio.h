/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP 摘机   HANGUP  挂机

#include "anyka_types.h"

#define FASTDIAL_F1         0x0a //速度F1-F4
#define FASTDIAL_F2         0x0b 
#define FASTDIAL_F3         0x0c 
#define FASTDIAL_F4         0x0d 

#define CALLID_NUM_LEN		32
#define CALLID_NAME_LEN		32

typedef struct
{
	T_CHR type;			// CALLID_TYPE_SIMPLE / CALLID_TYPE_COMPLEX
	T_CHR timestatus;	// CALLID_OK / CALLID_NONE / CALLID_FORBID / CALLID_CANNOT_GET
	T_CHR month;			// 1 - 12
	T_CHR day;			// 0 - 31
	T_CHR hour;			// 0 - 23
	T_CHR minute;		// 0 - 59
	T_CHR Line;
	T_CHR numberstatus;				// CALLID_OK / CALLID_NONE / CALLID_FORBID / CALLID_CANNOT_GET
	T_CHR number[CALLID_NUM_LEN];	// string ending with '\0'
	T_CHR namestatus;				// CALLID_OK / CALLID_NONE / CALLID_FORBID / CALLID_CANNOT_GET
	T_CHR name[CALLID_NAME_LEN];		// string ending with '\0'
} CALLID_INFO;

typedef enum
{
	e_PICKUPFAIL_VALUE = 0x81,		//0x81 摘机失败(电话线检测无效)
	e_PICKUP_VALUE,         		//0x82 手柄或免提摘机消息	
	e_HANGUP_VALUE,         		//0x83 手柄或免提挂机消息，
	e_RING_VALUE,           		//0x84 振铃
	e_HANDLEUP_VALUE,       		//0x85 手柄提起
	e_HANDLEDOWN_VALUE,     		//0x86 手柄挂下
	e_PICKUP_MUTEOFF_VALUE,			//0x87 软件摘机 静默
	e_MESSAGEPICKUP_MUTEON_VALUE,   //0x88 留言摘机 静默
	e_HORN_MUTEON_VALUE,			//0x89 静默，免提时喇叭无声
	e_MESSAGE_HANGUP_MUTEON_VALUE,	//0x8A 留言软件挂机
	e_FORCE_HANGUP_VALUE,			//0x8B 强制挂机
	e_VERSIONINFO_END_VALUE,		//0x8C 版本信息结束
	e_HORN_MUTEOFF_VALUE,			//0x8D 解除静默，免提时喇叭发声
	e_DTMF_END_VALUE,			    //0x8E DTMF来电号码结束命令
	e_FSK_END_VALUE,			    //0x8F FSK来电号码结束命令
	
	e_HANDSFREE_VALUE = 0X90,		//0x90 免提按键
	
	e_SIPSTATUS_VALUE = 0xC4,		//0xC4  切换为SIP状态
	e_PSTNSTATUS_VALUE,				//0xC5  切换为PSTN状态
	e_FINDSTATUS_VALUE,				//0xC6  切换为SIP/PSTN状态查询
	e_HANDPICKUP_VALUE = 0xc8,		//0xc8  手柄摘机
	e_FREEPICKUP_VALUE,   			//0xc9  免提摘机
	e_HANDHANGUP_VALUE,   			//0XCA  手柄挂机
	e_FREEHANGUP_VALUE,  			//0XCB  免提挂机
}PSTNGpioData;


typedef enum 
{
	e_EXIT_VALUE = 0xA0,
	e_FIND_VALUE,		//0xA1
	e_RECORD_VALUE,		//0xA2
	e_PLAY_VALUE,		//0xA3
	e_DELETE_VALUE,		//0xA4
	e_UP_VALUE,			//0xA5
	e_DOWN_VALUE,		//0xA6
	e_RIGHT_VALUE,		//0xA7
	e_LEFT_VALUE,		//0xA8
	e_OK_VALUE,			//0xA9
	e_VCARD_VALUE,		//0xAA
	e_CLASS_VALUE,		//0xAB
}KeyBoard;

/**************************************************************************************************
物理层命令定义
**************************************************************************************************/
#define DTMF_CST                0x30 // DTMF 来电通道开始
#define DTMF_CEND               0x3F // DTMF 来电通道结束
#define FSK_CST                 0x40 // FSK 来电通道开始
#define FSK_CEND                0x4F // FSK 来电通道结束

#define DTMF_CHANNEL_STRAT      DTMF_CST  // DTMF 来电通道开始
#define DTMF_CHANNEL_END        DTMF_CEND // DTMF 来电通道结束
#define FSK_CHANNEL_STRAT       FSK_CST   // FSK 来电通道开始
#define FSK_CHANNEL_END         FSK_CEND  // FSK 来电通道结束

#define TEL_DTMFEND             0x8E // DTMF 来电结束
#define TEL_FSKEND              0x8F // FSK 来电结束

#define TEL_NONE                0 // 无效字符
#define TEL_BUSY                1 // busy info

//数据 buffer 大小定义
#define TEL_BUFFER_SIZE         4096 // 命令接收缓存大小
#define TEL_VERSION_LEN         128 // 来电号码长度
#define TEL_NUM_LEN             30 // 来电号码长度

//FSK 包类型
#define CALLID_TYPE_NONE        0
#define CALLID_TYPE_DTMF        1
#define CALLID_TYPE_FSK_SIMPLE  0x04 // YDN069-1997 标准规定
#define CALLID_TYPE_FSK_COMPLEX 0x80 // YDN069-1997 标准规定

//复合格式参数类型（用于解码）
#define CALLID_PARAM_TIME       0x01 // YDN069-1997 标准规定
#define CALLID_PARAM_NUMBER     0x02 // YDN069-1997 标准规定
#define CALLID_PARAM_NONUMBER   0x04 // YDN069-1997 标准规定
#define CALLID_PARAM_NAME       0x07 // YDN069-1997 标准规定
#define CALLID_PARAM_NONAME     0x08 // YDN069-1997 标准规定

//不能显示来电原因（用于应用）
#define CALLID_OK               0
#define CALLID_NONE             1
#define CALLID_FORBID           2
#define CALLID_NOTGET           3
#define CALLID_ERROR            4

T_BOOL DecodeDTMFCallIDPackage(T_CHR *buf, CALLID_INFO *pcallid);
T_BOOL DecodeFSKCallIDPackage(T_CHR *buf, CALLID_INFO *pcallid);
T_VOID DecodeFSKSimpleParam(T_CHR *pd, int datalen, CALLID_INFO *pcallid);
T_VOID DecodeFSKComplexParam(T_CHR *pparm, CALLID_INFO *pcallid);
T_VOID ParsePSTNGpioData(unsigned char const* const data, unsigned int const length);