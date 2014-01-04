#ifndef _BA_USER_MSG_H_
#define _BA_USER_MSG_H_

#if defined __cplusplus
extern "C" {
#endif

#include "anyka_types.h"

typedef T_U8 T_BD_ADDR[6];
typedef T_U8 T_LINK_KEY[16];

enum BA_CONST
{
	// Common
	BS_FAIL			= 0,
	BS_OK,
	BS_PENDING,
	BS_BUSY,
	BS_REQUEST,
	BS_RESPONSE,
	BS_REJECT,
	BS_TIMEOUT,
	BS_INFINITE		= 0x7fffffff,

    // Lib Error
    BA_ERR_INVALID_PARAM = 0x00010000,
    BA_ERR_NOT_INIT,
    BA_ERR_MEMORY_LACK,
    BA_ERR_BUFFER_FULL,
    BA_ERR_TIMEOUT,
    BA_ERR_PACKET_DAMAGE,
};

typedef enum _BA_HFP_PHONE_STATUS
{
	BA_HFP_PHONE_NULL,			//This is the state the module enters once Stop() has been completed. No further operations can be performed except for Start().
	BA_HFP_PHONE_DISCONNECTED,	//This is the starting state of the module. The module enters this state if there are no active connections.
	BA_HFP_PHONE_CONNECTING,	//The module enters the CONNECTING state at the start of an HFP connection process and stays in this state for the duration of
								//of the connection process.
	BA_HFP_PHONE_DISCONNECTING, //The module enters the DISCONNECTING state when a disconnection process is initiated.
	BA_HFP_PHONE_STANDBY,		//The module enters the STANDBY state whenever it has an active connection and there are no outgoing, incoming,or ongoing calls.
	BA_HFP_PHONE_INCOMING_CALL,	//The module enters the INCOMING_CALL state when the module receives +CIEV callsetup = 1 urc from the remote device. 
								//This state indicates that there is a call incoming, but the call has not been answered.
	BA_HFP_PHONE_OUTGOING_CALL,	//The module enters the OUTGOING_CALL state if the module receives a +CIEV callsetup = 2 urc from the remote device. 
								//This state indicates that the remote device is trying to send an outgoing call and the call has not been answered.
	BA_HFP_PHONE_ONGOING_CALL,	//The module enters the ONGOING_CALL state if the module receives a +CIEV call = 1 urc. 
								//This state indicates that the OUTGOING or INCOMING call has been answered and there is currently an active call.
}BA_HFP_PHONE_STATUS;

typedef enum _BA_HFP_PHONE_DISCONNECTING_STATUS
{
	BA_HFP_PHONE_DISCONNECTING_CONNECTION_FAILED=1,
	BA_HFP_PHONE_DISCONNECTING_DISCONNECTION,

}BA_HFP_PHONE_DISCONNECTING_STATUS;

typedef enum _BA_HFP_ERROR_IND_ERRORS
{
	BA_HFP_ERROR_ANSWER_CALL,
	BA_HFP_ERROR_CALL_DIAL,
	BA_HFP_ERROR_REDIAL,
	BA_HFP_ERROR_CANCEL_CALL,
	BA_HFP_ERROR_IPHONE_BATTERY,
	BA_HFP_ERROR_MIC_VOL,
	BA_HFP_ERROR_SPEAKER_VOL,
	BA_HFP_ERROR_NO_RESPONSE_TIMEOUT,
	BA_HFP_ERROR_CMD_SEND_FAILED,

}BA_HFP_ERROR_IND_ERRORS;




// T_BA_USER_MSG is sent with BA_CALLBACK_FUN_USER_MSG callback function.
// param1, param2 and return value are specific to each user msg.
typedef enum _T_BA_USER_MSG
{
    //调用BA_Stop后返回的消息，表示所有服务已经完全停止
    // return: 
    HOST_SHUTDOWN_CNF = 0x1000,

    //调用BA_Deinit或BA_ForceDeinit后返回的消息，表示BlueA lib完全卸载了
    BALIB_UNLOADED,

    //BlueA lib检测到有错误(或潜在的错误)发生
    //param1: BA_CONST err_type, BA_ERR_XXX
    //param2: T_U32 id, internal use only
    BALIB_ERROR_IND,

    
	/********** Core **********/
    //ACL连接请求，告知用户，这个不需要用户回应，系统默认接受ACL连接请求
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was sent connection 
    //param2: T_U32, Class of Device, 24 bits
    MGR_ACL_CONNECTION_REQUEST = 0x1101,

    //ACL连接断开，告知用户
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was disconnected.
    //param2: T_U32 Reason, Reason for disconnection.
    MGR_ACL_DISCONNECT_COMPLETE,

    //ACL连接完成，告知用户
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was connection complete.
    //param2: T_U32 Status, 0: Connection successfully completed. Other: Reason of failure.
    MGR_ACL_CONNECTION_COMPLETE,
	

	/********** GAP **********/
    // status: 0 表示成功，大于0 为错误的原因，具体见蓝牙SPEC
    // 不关心return value

    //调用BA_GAP_SetPinCodeType 后异步返回的消息
    // param1: status
	GAP_PIN_TYPE_CHANGE = 0x1201,

    //调用BA_GAP_SetLocalBDAddr  后异步返回的消息
    // param1: status
    GAP_LOCAL_BDADDR_CHANGE,

    //调用BA_GAP_SetClassOfDevice 后异步返回的消息
    // param1: status
	GAP_DEVICE_CLASS_CHANGE,

    //调用BA_GAP_SetLocalName后异步返回的消息
    // param1: status
	GAP_LOCAL_NAME_CHANGE,

    //调用BA_GAP_SetInquirySpeed后异步返回的消息
    // param1: status
	GAP_INQUIRY_MODE_CHANGE,

    //调用BA_GAP_SetPagedSpeed后异步返回的消息
    // param1: status
	GAP_PAGE_SPEED_CHANGE,
    
    //调用BA_GAP_SetSSPDebugMode后异步返回的消息
    // param1: status
	GAP_SSP_DEBUG_MODE_CHANGE,

    //调用BA_GAP_SSPSwitch后异步返回的消息
    // param1: status
	GAP_SSP_MODE_CHANGE,

    //调用BA_GAP_GetRemoteName后异步返回的消息
    // param1: status
    // param2: 如果status为0，这个参数指向远程设备名(null结尾的utf-8字符串)
	GAP_REMOTE_NAME_CHANGE,

    //调用BA_GAP_SetDeviceMode后异步返回的消息
    // param1: status
	GAP_DEVICE_MODE_CHANGE,

    //当被动连接或者主动连接设备时会产生该消息，
    //消息中携带设备的蓝牙地址，
    //平台需调用BA_GAP_SetLinKey_for 答复，越快越好
    //param1: T_BD_ADDR Addr
    GAP_LINK_KEY_REQUEST,
    
    //当与设备配对完成后会产生LinkKey,此时会产生该消息通知平台
    //平台需存储消息所携带的LinkKey,消息中携带设备地址与LinkKey 
    //param1: T_BD_ADDR Addr
    //param2: T_LINK_KEY LineKey
    GAP_LINK_KEY_GENERATION,
    
    //当本地存在LinkKey,但对方不存在LinkKey 时，此时会认证失败，
    //这时库会抛出该消息，该消息携带设备地址
    //param1: T_BD_ADDR Addr
    GAP_LINK_KEY_DELETE_REQ,

    //调用BA_GAP_GetAroundDevice后异步返回的消息
    // param1: T_U32 number, result的个数
    // param2: T_INQUIRY_RESLUT *result
    /*
        typedef PACK_PARAM struct{
            T_BD_ADDR dev_addr;
    	    T_U8 PageScanRepeatMode;
    	    T_U8 Reserved[2];
    	    T_U8 ClassOfDev[3];
    	    T_U16 ClockOffset;
        }T_INQUIRY_RESLUT;
    */
	GAP_INQUIRY_RESULT_NOTIFY,

	//调用BA_GAP_GetAroundDevice后异步返回的消息
    // param1: status
	GAP_INQUIRY_COMPLETE_NOTIFY,
	
	//返回一个字节的有符号数，单位为dB
	// param1: T_S8 *pBuffer
	// param2: T_S32 nSize, pBuffer字节数
    GAP_RSSI_RESULT_REPLY,

	/********** A2DP **********/

    //a2dp收到的sbc数据，用户需要立即取走数据
    // param1: T_pVOID pBuffer, 包括1-n个缓冲包
    // param2: T_S32 nSize，pBuffer字节数
    // return: 
    A2DP_STREAM_DATA_IND = 0x1a01,

    //告诉用户A2DP的播放工作已经准备就绪。
	//用户宜在此时开启解码器，或者说解码器准备好了
	//此后用户播放程序仅处于简单的暂停状态
    // return: 
    A2DP_STREAM_OPEN,

    //指示A2DP即将向用户传送SBC包
    //此时用户播放程序由暂停转为播放状态
	//注意,此消息不代表SBC包会持续不断地发送
    // return: 
    A2DP_STREAM_START,

    //指示A2DP已经停止向用户传送SBC包
    //此时用户播放程序由播放转为停止状态
	//注意,此消息不代表SBC包已经完全停止发送
    // return: 
    A2DP_STREAM_STOP,

    //告诉用户A2DP的播放工作已经停止
	//用户宜在此时关闭解码器，或者稍后关闭解码器
    // return: 
    A2DP_STREAM_CLOSE,
        
    //任何一方发起连接请求，此条消息告诉用户即将连接
    // param1: BA_CONST reason, 需要连接的原因
	//		   BS_REQUEST: 仅在被动连接时请求应用确认或否决
	//		   BS_FAIL: 连接失败，由于A2DP有多个通道，该消息发布时，流可能已经打开
	//		   BS_OK: 已经成功完成所有通道的连接，此消息发布时，流可能已经打开
    // return: if reason is BS_REQUEST, return 1: 允许blueA连接，0: 取消连接;
    //         others: return value is not used.
    A2DP_CONNECT_IND,

    //告诉用户断开连接的结果和原因
    // param1: BA_CONST reason, 断开原因，当前没使用
    // return: 
    A2DP_DISCONNECT_IND,

    //当退出a2dp这个服务时，这个消息说明a2dp真正的停止了
    // return: 
    A2DP_PROFILE_STOP,

    
	/********** AVRCP **********/

    //任何一方发起连接请求，此条消息告诉用户即将连接
    // param1: BA_CONST reason, 需要连接的原因
	//		   BS_REQUEST: 仅在被动连接时请求应用确认或否决
	//		   BS_FAIL: 连接失败
	//		   BS_OK: 连接成功完成
    // return: if reason is BS_REQUEST, return 1: 允许blueA连接，0: 取消连接;
    //         others: return value is not used.
    AVRCP_CONNECT_IND = 0x1b01,

    //告诉用户断开连接的结果和原因
    // param1: BA_CONST reason, 断开原因，当前没使用
    AVRCP_DISCONNECT_IND,

    //当退出avrcp这个服务时，这个消息说明avrcp真正的停止了
    // return: 
    AVRCP_PROFILE_STOP,

    
	/********** HFP **********/
    
    // hfp每种状态的改变都会发一个这个消息
    // 例如连接成功将会从NULL变成stanby，来电话将会从stanby改变成incoming        
    // param1: BA_HFP_PHONE_STATUS status
    // param2: BA_HFP_PHONE_DISCONNECTED_STATUS , if param1==BA_HFP_PHONE_DISCONNECTED;
    // return:
    HFP_CALL_STATUS_IND = 0x1c01,

    // hfp每种状态的改变有可能因为一些原因失败，此消息返回给用户失败的原因
    // 也有可能是一些状态改变的命令失败了，也会发出这个消息
	// param1: BA_HFP_ERROR_IND_ERRORS error
    // return: 
    HFP_STATUS_ERROR_IND,

    // 这个消息说明来电话了，用户通过此消息获取来电号码，
    // param1: T_U8 *phone_number, ascii array
    // param2: T_S32 size, phone_number size
    // return: 
    HFP_PHONE_NUMBER_IND,

    // 当手机端扬声器改变音量时，此消息报告用户当前的手机音量。
    // param1: T_S32 speaker_gain, 0~15
    // return: 
    HFP_SPEAKER_GAIN_IND,

    // 当手机端mic改变音量时，此消息报告用户当前的mic音量。
    // param1: T_S32 mic_gain, 0~15
    // return: 
    HFP_MIC_GAIN_IND,

	//To indicate an SCO connection has been established and audio data
	//is ready to be output
	//return:
	HFP_WAVE_OPEN,
	
	//To indicate an SCO connection has been terminated and audio output
	//should be stopped
	//return:
	HFP_WAVE_CLOSE,

	//To indicate incoming SCO audio data that should be output
    // param1: T_pVOID pBuffer
    // param2: T_S32 nSize
	// return:
	HFP_WAVE_RECEIVE,

	//Indicates whether remote device supports Apple AT commands
	// param1: AK_TRUE = supported, AK_FALSE = unsupported
	HFP_IS_IPHONE_IND,

	//Contains the remote device's supported features as obtained from the BRSF response
	// param1: T_U16 AG_features
	HFP_SUPPORTED_FEATURES_IND,


	/********** SPP **********/

    //任何一方发起连接请求，此条消息告诉用户即将连接
    // param1: BA_CONST reason, 需要连接的原因
	//		   BS_REQUEST: 仅在被动连接时请求应用确认或否决
	//		   BS_FAIL: 连接失败
	//		   BS_OK: 连接成功完成
    // return: if reason is BS_REQUEST, return 1: 允许blueA连接，0: 取消连接;
    //         others: return value is not used.
	// 注: 目前并不支持主动连接
    SPP_CONNECT_IND = 0x1d01,

    //告诉用户断开连接的结果和原因
    // param1: BA_CONST reason, 断开原因，当前没使用
	SPP_DISCONNECT_IND,

	// To indicate incoming spp data that should be receive
    // param1: const void *pBuffer
    // param2: T_U32 nSize
	// return: 
	SPP_DATA_RECEIVE,

	// 指示又能发送几个包，应用程序可据此触发再次发包动作
	// 注意，用户程序不宜在本消息调用过程及子过程中执行发送过程
	SPP_SEND_CREDITS,

	// indicates remote device has initiated a rfcomm connection for OPP
	OPP_INCOMING_CONNECTION,

	// indicates remote device has initiated a PUT operation
	// param1: BA_OPP_Initial_Packet struct, containing name and size data of incoming object
	// param2: size of body in the event of multiple rfcomm packets per OPP packet. 0 if all data is contained in 1 rfcomm packet
	OPP_PUT_REQUEST,

	// indicates a portion of the OPP object has been received
	// param1: T_U8 * OPP data for a portion of the OPP object
	// param2: size of param1
	OPP_PUT_BODY,

	// indicates the last portion of the OPP object has been received
	// param1: T_U8 * OPP data for the last portion of the OPP object
	// param2: size of param1
	OPP_PUT_BODY_FINAL,

	// indicates that the abort operation has been completed
	OPP_ABORT_COMPLETE,

	//indicates that the disconnect operation has been completed
	OPP_DISCONNECT_COMPLETE,

}T_BA_USER_MSG;

typedef T_S32     (*BA_CALLBACK_FUN_USER_MSG)(T_BA_USER_MSG msg, T_S32 param1, T_S32 param2);


#if defined __cplusplus
}
#endif

#endif // _BA_USER_MSG_H_

