/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP 摘机   HANGUP  挂机

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
		
		//todo 对c的解析和处理
		switch(c)
		{
		case e_RING_VALUE:			//振铃
			break;
		case e_HANDHUNGON_VALUE:	//手柄摘机
			break;
		case e_FREEHUNGON_VALUE:	//免提摘机
			break;
		}
	}
}
