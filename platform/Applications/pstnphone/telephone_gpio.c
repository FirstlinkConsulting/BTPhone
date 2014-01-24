/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP ժ��   HANGUP  �һ�

#include "telephone_gpio.h"




void ParseTelephoneGpioMsg(unsigned char const* const data, unsigned int const length)
{
	int i,c;
	//static CALLID_INFO	CallID;
	//static UINT8   CallIDbuff[128];
	static int     CallIDLen = 0;
	
	//extern CTestDlg *m_pTestDlg0;
	//extern BOOL _test_call;
	//Sleep(10);
	
	for (i=0; i<length; i++)
	{
		if(CallIDLen >= 128)
			CallIDLen = 0;
		c = data[i];
		AK_DEBUG_OUTPUT("%x\n", data[i]);
		
		//todo ��c�Ľ����ʹ���
		switch(c)
		{
		case e_RING_VALUE:			//����
			break;
		case e_HANDHUNGON_VALUE:	//�ֱ�ժ��
			break;
		case e_FREEHUNGON_VALUE:	//����ժ��
			break;
		}
	}
}
