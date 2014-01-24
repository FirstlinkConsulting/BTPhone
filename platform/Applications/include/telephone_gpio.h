/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP ժ��   HANGUP  �һ�

typedef enum
{
	e_PICKUPFAIL_VALUE = 0x81,		//0x81 ժ��ʧ��(�绰�߼����Ч)
	e_PICKUP_VALUE,         		//0x82 �ֱ�������ժ����Ϣ	
	e_HANGUP_VALUE,         		//0x83 �ֱ�������һ���Ϣ��
	e_RING_VALUE,           		//0x84 ����
	e_HANDLEUP_VALUE,       		//0x85 �ֱ�����
	e_HANDLEDOWN_VALUE,     		//0x86 �ֱ�����
	e_PICKUP_MUTEOFF_VALUE,			//0x87 ���ժ�� ��Ĭ
	e_MESSAGEPICKUP_MUTEON_VALUE,   //0x88 ����ժ�� ��Ĭ
	e_HORN_MUTEON_VALUE,			//0x89 ��Ĭ������ʱ��������
	e_MESSAGE_HANGUP_MUTEON_VALUE,	//0x8A ��������һ�
	e_FORCE_HANGUP_VALUE,			//0x8B ǿ�ƹһ�
	e_VERSIONINFO_END_VALUE,		//0x8C �汾��Ϣ����
	e_HORN_MUTEOFF_VALUE,			//0x8D �����Ĭ������ʱ���ȷ���
	e_DTMF_END_VALUE,			    //0x8E DTMF��������������
	e_FSK_END_VALUE,			    //0x8F FSK��������������
	
	e_HANDSFREE_VALUE = 0X90,		//0x90 ���ᰴ��
	
	e_SIPSTATUS_VALUE = 0xC4,		//0xC4  �л�ΪSIP״̬
	e_PSTNSTATUS_VALUE,				//0xC5  �л�ΪPSTN״̬
	e_FINDSTATUS_VALUE,				//0xC6  �л�ΪSIP/PSTN״̬��ѯ
	e_HANDHUNGON_VALUE = 0xc8,		//0xc8  �ֱ�ժ��
	e_FREEHUNGON_VALUE,   			//0xc9  ����ժ��
	e_HANDHUNGOFF_VALUE,   			//0XCA  �ֱ��һ�
	e_FREEHUNGOFF_VALUE,  			//0XCB  ����һ�
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