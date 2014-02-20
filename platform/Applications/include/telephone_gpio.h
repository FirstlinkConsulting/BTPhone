/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP ժ��   HANGUP  �һ�

#include "anyka_types.h"

#define FASTDIAL_F1         0x0a //�ٶ�F1-F4
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
	e_HANDPICKUP_VALUE = 0xc8,		//0xc8  �ֱ�ժ��
	e_FREEPICKUP_VALUE,   			//0xc9  ����ժ��
	e_HANDHANGUP_VALUE,   			//0XCA  �ֱ��һ�
	e_FREEHANGUP_VALUE,  			//0XCB  ����һ�
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
����������
**************************************************************************************************/
#define DTMF_CST                0x30 // DTMF ����ͨ����ʼ
#define DTMF_CEND               0x3F // DTMF ����ͨ������
#define FSK_CST                 0x40 // FSK ����ͨ����ʼ
#define FSK_CEND                0x4F // FSK ����ͨ������

#define DTMF_CHANNEL_STRAT      DTMF_CST  // DTMF ����ͨ����ʼ
#define DTMF_CHANNEL_END        DTMF_CEND // DTMF ����ͨ������
#define FSK_CHANNEL_STRAT       FSK_CST   // FSK ����ͨ����ʼ
#define FSK_CHANNEL_END         FSK_CEND  // FSK ����ͨ������

#define TEL_DTMFEND             0x8E // DTMF �������
#define TEL_FSKEND              0x8F // FSK �������

#define TEL_NONE                0 // ��Ч�ַ�
#define TEL_BUSY                1 // busy info

//���� buffer ��С����
#define TEL_BUFFER_SIZE         4096 // ������ջ����С
#define TEL_VERSION_LEN         128 // ������볤��
#define TEL_NUM_LEN             30 // ������볤��

//FSK ������
#define CALLID_TYPE_NONE        0
#define CALLID_TYPE_DTMF        1
#define CALLID_TYPE_FSK_SIMPLE  0x04 // YDN069-1997 ��׼�涨
#define CALLID_TYPE_FSK_COMPLEX 0x80 // YDN069-1997 ��׼�涨

//���ϸ�ʽ�������ͣ����ڽ��룩
#define CALLID_PARAM_TIME       0x01 // YDN069-1997 ��׼�涨
#define CALLID_PARAM_NUMBER     0x02 // YDN069-1997 ��׼�涨
#define CALLID_PARAM_NONUMBER   0x04 // YDN069-1997 ��׼�涨
#define CALLID_PARAM_NAME       0x07 // YDN069-1997 ��׼�涨
#define CALLID_PARAM_NONAME     0x08 // YDN069-1997 ��׼�涨

//������ʾ����ԭ������Ӧ�ã�
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