/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP 摘机   HANGUP  挂机

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
	e_HANDHUNGON_VALUE = 0xc8,		//0xc8  手柄摘机
	e_FREEHUNGON_VALUE,   			//0xc9  免提摘机
	e_HANDHUNGOFF_VALUE,   			//0XCA  手柄挂机
	e_FREEHUNGOFF_VALUE,  			//0XCB  免提挂机
}TelephoneGpioMsg;


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


void ParseTelephoneGpioMsg(unsigned char const* const data, unsigned int const length);