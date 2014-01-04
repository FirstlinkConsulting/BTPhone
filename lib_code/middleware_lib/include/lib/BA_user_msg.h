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
    //����BA_Stop�󷵻ص���Ϣ����ʾ���з����Ѿ���ȫֹͣ
    // return: 
    HOST_SHUTDOWN_CNF = 0x1000,

    //����BA_Deinit��BA_ForceDeinit�󷵻ص���Ϣ����ʾBlueA lib��ȫж����
    BALIB_UNLOADED,

    //BlueA lib��⵽�д���(��Ǳ�ڵĴ���)����
    //param1: BA_CONST err_type, BA_ERR_XXX
    //param2: T_U32 id, internal use only
    BALIB_ERROR_IND,

    
	/********** Core **********/
    //ACL�������󣬸�֪�û����������Ҫ�û���Ӧ��ϵͳĬ�Ͻ���ACL��������
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was sent connection 
    //param2: T_U32, Class of Device, 24 bits
    MGR_ACL_CONNECTION_REQUEST = 0x1101,

    //ACL���ӶϿ�����֪�û�
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was disconnected.
    //param2: T_U32 Reason, Reason for disconnection.
    MGR_ACL_DISCONNECT_COMPLETE,

    //ACL������ɣ���֪�û�
    //param1: T_BD_ADDR Addr, Bluetooth addr of connection which was connection complete.
    //param2: T_U32 Status, 0: Connection successfully completed. Other: Reason of failure.
    MGR_ACL_CONNECTION_COMPLETE,
	

	/********** GAP **********/
    // status: 0 ��ʾ�ɹ�������0 Ϊ�����ԭ�򣬾��������SPEC
    // ������return value

    //����BA_GAP_SetPinCodeType ���첽���ص���Ϣ
    // param1: status
	GAP_PIN_TYPE_CHANGE = 0x1201,

    //����BA_GAP_SetLocalBDAddr  ���첽���ص���Ϣ
    // param1: status
    GAP_LOCAL_BDADDR_CHANGE,

    //����BA_GAP_SetClassOfDevice ���첽���ص���Ϣ
    // param1: status
	GAP_DEVICE_CLASS_CHANGE,

    //����BA_GAP_SetLocalName���첽���ص���Ϣ
    // param1: status
	GAP_LOCAL_NAME_CHANGE,

    //����BA_GAP_SetInquirySpeed���첽���ص���Ϣ
    // param1: status
	GAP_INQUIRY_MODE_CHANGE,

    //����BA_GAP_SetPagedSpeed���첽���ص���Ϣ
    // param1: status
	GAP_PAGE_SPEED_CHANGE,
    
    //����BA_GAP_SetSSPDebugMode���첽���ص���Ϣ
    // param1: status
	GAP_SSP_DEBUG_MODE_CHANGE,

    //����BA_GAP_SSPSwitch���첽���ص���Ϣ
    // param1: status
	GAP_SSP_MODE_CHANGE,

    //����BA_GAP_GetRemoteName���첽���ص���Ϣ
    // param1: status
    // param2: ���statusΪ0���������ָ��Զ���豸��(null��β��utf-8�ַ���)
	GAP_REMOTE_NAME_CHANGE,

    //����BA_GAP_SetDeviceMode���첽���ص���Ϣ
    // param1: status
	GAP_DEVICE_MODE_CHANGE,

    //���������ӻ������������豸ʱ���������Ϣ��
    //��Ϣ��Я���豸��������ַ��
    //ƽ̨�����BA_GAP_SetLinKey_for �𸴣�Խ��Խ��
    //param1: T_BD_ADDR Addr
    GAP_LINK_KEY_REQUEST,
    
    //�����豸�����ɺ�����LinkKey,��ʱ���������Ϣ֪ͨƽ̨
    //ƽ̨��洢��Ϣ��Я����LinkKey,��Ϣ��Я���豸��ַ��LinkKey 
    //param1: T_BD_ADDR Addr
    //param2: T_LINK_KEY LineKey
    GAP_LINK_KEY_GENERATION,
    
    //�����ش���LinkKey,���Է�������LinkKey ʱ����ʱ����֤ʧ�ܣ�
    //��ʱ����׳�����Ϣ������ϢЯ���豸��ַ
    //param1: T_BD_ADDR Addr
    GAP_LINK_KEY_DELETE_REQ,

    //����BA_GAP_GetAroundDevice���첽���ص���Ϣ
    // param1: T_U32 number, result�ĸ���
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

	//����BA_GAP_GetAroundDevice���첽���ص���Ϣ
    // param1: status
	GAP_INQUIRY_COMPLETE_NOTIFY,
	
	//����һ���ֽڵ��з���������λΪdB
	// param1: T_S8 *pBuffer
	// param2: T_S32 nSize, pBuffer�ֽ���
    GAP_RSSI_RESULT_REPLY,

	/********** A2DP **********/

    //a2dp�յ���sbc���ݣ��û���Ҫ����ȡ������
    // param1: T_pVOID pBuffer, ����1-n�������
    // param2: T_S32 nSize��pBuffer�ֽ���
    // return: 
    A2DP_STREAM_DATA_IND = 0x1a01,

    //�����û�A2DP�Ĳ��Ź����Ѿ�׼��������
	//�û����ڴ�ʱ����������������˵������׼������
	//�˺��û����ų�������ڼ򵥵���ͣ״̬
    // return: 
    A2DP_STREAM_OPEN,

    //ָʾA2DP�������û�����SBC��
    //��ʱ�û����ų�������ͣתΪ����״̬
	//ע��,����Ϣ������SBC����������ϵط���
    // return: 
    A2DP_STREAM_START,

    //ָʾA2DP�Ѿ�ֹͣ���û�����SBC��
    //��ʱ�û����ų����ɲ���תΪֹͣ״̬
	//ע��,����Ϣ������SBC���Ѿ���ȫֹͣ����
    // return: 
    A2DP_STREAM_STOP,

    //�����û�A2DP�Ĳ��Ź����Ѿ�ֹͣ
	//�û����ڴ�ʱ�رս������������Ժ�رս�����
    // return: 
    A2DP_STREAM_CLOSE,
        
    //�κ�һ�������������󣬴�����Ϣ�����û���������
    // param1: BA_CONST reason, ��Ҫ���ӵ�ԭ��
	//		   BS_REQUEST: ���ڱ�������ʱ����Ӧ��ȷ�ϻ���
	//		   BS_FAIL: ����ʧ�ܣ�����A2DP�ж��ͨ��������Ϣ����ʱ���������Ѿ���
	//		   BS_OK: �Ѿ��ɹ��������ͨ�������ӣ�����Ϣ����ʱ���������Ѿ���
    // return: if reason is BS_REQUEST, return 1: ����blueA���ӣ�0: ȡ������;
    //         others: return value is not used.
    A2DP_CONNECT_IND,

    //�����û��Ͽ����ӵĽ����ԭ��
    // param1: BA_CONST reason, �Ͽ�ԭ�򣬵�ǰûʹ��
    // return: 
    A2DP_DISCONNECT_IND,

    //���˳�a2dp�������ʱ�������Ϣ˵��a2dp������ֹͣ��
    // return: 
    A2DP_PROFILE_STOP,

    
	/********** AVRCP **********/

    //�κ�һ�������������󣬴�����Ϣ�����û���������
    // param1: BA_CONST reason, ��Ҫ���ӵ�ԭ��
	//		   BS_REQUEST: ���ڱ�������ʱ����Ӧ��ȷ�ϻ���
	//		   BS_FAIL: ����ʧ��
	//		   BS_OK: ���ӳɹ����
    // return: if reason is BS_REQUEST, return 1: ����blueA���ӣ�0: ȡ������;
    //         others: return value is not used.
    AVRCP_CONNECT_IND = 0x1b01,

    //�����û��Ͽ����ӵĽ����ԭ��
    // param1: BA_CONST reason, �Ͽ�ԭ�򣬵�ǰûʹ��
    AVRCP_DISCONNECT_IND,

    //���˳�avrcp�������ʱ�������Ϣ˵��avrcp������ֹͣ��
    // return: 
    AVRCP_PROFILE_STOP,

    
	/********** HFP **********/
    
    // hfpÿ��״̬�ĸı䶼�ᷢһ�������Ϣ
    // �������ӳɹ������NULL���stanby�����绰�����stanby�ı��incoming        
    // param1: BA_HFP_PHONE_STATUS status
    // param2: BA_HFP_PHONE_DISCONNECTED_STATUS , if param1==BA_HFP_PHONE_DISCONNECTED;
    // return:
    HFP_CALL_STATUS_IND = 0x1c01,

    // hfpÿ��״̬�ĸı��п�����ΪһЩԭ��ʧ�ܣ�����Ϣ���ظ��û�ʧ�ܵ�ԭ��
    // Ҳ�п�����һЩ״̬�ı������ʧ���ˣ�Ҳ�ᷢ�������Ϣ
	// param1: BA_HFP_ERROR_IND_ERRORS error
    // return: 
    HFP_STATUS_ERROR_IND,

    // �����Ϣ˵�����绰�ˣ��û�ͨ������Ϣ��ȡ������룬
    // param1: T_U8 *phone_number, ascii array
    // param2: T_S32 size, phone_number size
    // return: 
    HFP_PHONE_NUMBER_IND,

    // ���ֻ����������ı�����ʱ������Ϣ�����û���ǰ���ֻ�������
    // param1: T_S32 speaker_gain, 0~15
    // return: 
    HFP_SPEAKER_GAIN_IND,

    // ���ֻ���mic�ı�����ʱ������Ϣ�����û���ǰ��mic������
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

    //�κ�һ�������������󣬴�����Ϣ�����û���������
    // param1: BA_CONST reason, ��Ҫ���ӵ�ԭ��
	//		   BS_REQUEST: ���ڱ�������ʱ����Ӧ��ȷ�ϻ���
	//		   BS_FAIL: ����ʧ��
	//		   BS_OK: ���ӳɹ����
    // return: if reason is BS_REQUEST, return 1: ����blueA���ӣ�0: ȡ������;
    //         others: return value is not used.
	// ע: Ŀǰ����֧����������
    SPP_CONNECT_IND = 0x1d01,

    //�����û��Ͽ����ӵĽ����ԭ��
    // param1: BA_CONST reason, �Ͽ�ԭ�򣬵�ǰûʹ��
	SPP_DISCONNECT_IND,

	// To indicate incoming spp data that should be receive
    // param1: const void *pBuffer
    // param2: T_U32 nSize
	// return: 
	SPP_DATA_RECEIVE,

	// ָʾ���ܷ��ͼ�������Ӧ�ó���ɾݴ˴����ٴη�������
	// ע�⣬�û��������ڱ���Ϣ���ù��̼��ӹ�����ִ�з��͹���
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

