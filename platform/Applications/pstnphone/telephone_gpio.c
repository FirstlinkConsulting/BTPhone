/************************************************************************/
/* pstn telephone gpio command and notify ly2014                        */
/************************************************************************/

//78806->Main  notify
//Main->78806 command
//PICKUP 摘机   HANGUP  挂机

#include "telephone_gpio.h"
#include "Fwl_Gpio.h"
#include "Eng_Debug.h"

const char *const DTMFNUMBER = "D1234567890*#ABC";

//////////////////////////////////////////
//
//	解 DTMF 来电包
//
T_BOOL DecodeDTMFCallIDPackage(T_CHR *buf, CALLID_INFO *pcallid)
{
	int		i;

	for (i = 0; (*(buf + i) != TEL_DTMFEND) && (i < CALLID_NUM_LEN - 1); i ++)
	{
		pcallid->number[i] = DTMFNUMBER[*(buf + i) - DTMF_CHANNEL_STRAT];
	}
	pcallid->number[i] = '\0';
	pcallid->numberstatus = CALLID_OK;
	pcallid->type = CALLID_TYPE_DTMF;
	return AK_TRUE;
}


////////////////////////////////
//
//	解 FSK 来电显示包
//
T_BOOL DecodeFSKCallIDPackage(T_CHR *buf, CALLID_INFO *pcallid)
{
	int		pklen, datalen, i;
	T_CHR	*pparm;		// pparm: pointer_parameter
	T_CHR	*pd;		// pd:    pointer_data
	/*
	*	Assemble package
	*/
	for (pklen = 0; *(buf + pklen) != TEL_FSKEND; pklen ++){
		if (pklen % 2 == 0)
			*(buf + pklen / 2) = (*(buf + pklen) << 4) & 0xF0;
		else
			*(buf + pklen / 2) |= (*(buf + pklen) & 0x0F);
	}

	/*
	*	Package frame error
	*/
	//if (pklen % 2 != 0)
	//	return false;

	/*
	*	Locate pakage start flag error
	*/
	pklen /= 2;
	for (i = 0; i < pklen; i ++)
	{
		if ((*buf == CALLID_TYPE_FSK_SIMPLE)
		   ||(*buf == CALLID_TYPE_FSK_COMPLEX))
			break;
		else
			buf ++;
	}
	if (i == pklen)
		return AK_FALSE;
	else
		pklen -= i;

	/*
	*	Package check sum error
	*/
	{
		T_CHR sum = 0;
		for (i = 0; i < pklen - 1; i ++)
			sum += *(buf + i);
	}
	//if ((BYTE)(sum + *(buf + pklen - 1)) != 0)
	//	return false;

	/*
	*	Package length error
	*/
	datalen = *(buf + 1);
	//if (datalen + 3 != pklen)
	//	return false;

	/*
	*	Decode simple format
	*/
	if (*buf == CALLID_TYPE_FSK_SIMPLE){
		pd = buf + 2;
		DecodeFSKSimpleParam(pd, datalen, pcallid);
		pcallid->type = CALLID_TYPE_FSK_SIMPLE;
	}

	/*
	*	Decode complex format
	*/
	else if (*buf == CALLID_TYPE_FSK_COMPLEX){
		pparm = buf + 2;
		while (pparm < buf + 2 + datalen){
			DecodeFSKComplexParam(pparm, pcallid);		// Decode every parameter
			pparm += *(pparm + 1) + 2;
		}
		pcallid->type = CALLID_TYPE_FSK_COMPLEX;
	}
    
	return AK_TRUE;
}

////////////////////////////////
//
//	解 FSK 来电简单格式参数包
//
T_VOID DecodeFSKSimpleParam(T_CHR *pd, int datalen, CALLID_INFO *pcallid)
{
	/*
	*	Data length error
	*/
	//if (datalen < 8 + 1)
	//	return;

	T_CHR	*pnum = pd + 8;
	int		numlen = datalen - 8, i;

	/*
	*	Decode time
	*/
	pcallid->month = (*(pd + 0) - '0') * 10 + (*(pd + 1) - '0');
	pcallid->day = (*(pd + 2) - '0') * 10 + (*(pd + 3) - '0');
	pcallid->hour = (*(pd + 4) - '0') * 10 + (*(pd + 5) - '0');
	pcallid->minute = (*(pd + 6) - '0') * 10 + (*(pd + 7) - '0');
	pcallid->timestatus = CALLID_OK;

	/*
	*	Decode number
	*/

	// byw, 2002/8/27, 去掉奇偶校验信息
	for (i = 0; i < numlen; i ++)
	{
		*(pnum + i) &= 0x7F;
	}

	if (*pnum == 'O')
		pcallid->numberstatus = CALLID_NOTGET;
	else if (*pnum == 'P')
		pcallid->numberstatus = CALLID_FORBID;
	else
	{
        for (i = 0; i < numlen; i ++)
        {
                *(pnum + i) &= 0x0F;
        }
        for (i = 0; i < numlen; i ++)
        {
                *(pnum + i) |= 0x30;
        }

		for (i = 0; (i < numlen) && (i < CALLID_NUM_LEN -1); i ++)
			pcallid->number[i] = *(pnum + i);
		pcallid->number[i] = '\0';
		pcallid->numberstatus = CALLID_OK;
	}
}

////////////////////////////////
//
//	解 FSK 来电复杂格式参数包
//
T_VOID DecodeFSKComplexParam(T_CHR *pparm, CALLID_INFO *pcallid)
{
	// pparm: pointer_parameter, pd: pointer_data
	int		datalen, i;
	T_CHR	*pd;

	datalen = *(pparm + 1);
	pd = pparm + 2;

	/*
	*	odd-even check error
	*	just cast off check bit temporarily
	*/
	for (i = 0; i < datalen; i ++)
		*(pd + i) &= 0x7F;

	switch (*pparm){

	case CALLID_PARAM_TIME:

		/*
		*	Data time length error
		*/
		//if (datalen != 8)
		//	return;

		/*
		*	Decode date time
		*/
		pcallid->month = (*(pd + 0) - '0') * 10 + (*(pd + 1) - '0');
		pcallid->day = (*(pd + 2) - '0') * 10 + (*(pd + 3) - '0');
		pcallid->hour = (*(pd + 4) - '0') * 10 + (*(pd + 5) - '0');
		pcallid->minute = (*(pd + 6) - '0') * 10 + (*(pd + 7) - '0');
		pcallid->timestatus = CALLID_OK;
		break;

	case CALLID_PARAM_NUMBER:

        for (i = 0; i < datalen; i ++)
        {
                *(pd + i) &= 0x0F;
        }
        for (i = 0; i < datalen; i ++)
        {
                *(pd + i) |= 0x30;
        }

		for (i = 0; (i < datalen) && (i < CALLID_NUM_LEN -1); i ++)
			pcallid->number[i] = *(pd + i);
		pcallid->number[i] = '\0';
		pcallid->numberstatus = CALLID_OK;
		break;

	case CALLID_PARAM_NONUMBER:
		/*
		*	Set 'no number'
		*/
		if (*pd == 'O')
			pcallid->numberstatus = CALLID_NOTGET;
		else if (*pd == 'P')
                {
					pcallid->numberstatus = CALLID_FORBID;
                }
		else
			pcallid->numberstatus = CALLID_ERROR; //fjm
		break;

	case CALLID_PARAM_NAME:
		/*
		*	Decode name
		*/
		for (i = 0; (i < datalen) && (i < CALLID_NAME_LEN -1); i ++)
			pcallid->name[i] = *(pd + i);
		pcallid->name[i] = '\0';
		pcallid->namestatus = CALLID_OK;
		break;

	case CALLID_PARAM_NONAME:
		/*
		*	Set 'no name'
		*/
		if (*pd == 'O')
			pcallid->namestatus = CALLID_NOTGET;
		else if (*pd == 'P')
			pcallid->namestatus = CALLID_FORBID;
		else
			pcallid->namestatus = CALLID_FORBID;

		break;
	}
}

T_BOOL DecodeCallIDPackage(T_CHR *buf, CALLID_INFO *pcallid)
{
	T_BOOL	flag = AK_FALSE;
	
	if (*buf >= DTMF_CHANNEL_STRAT && *buf <= DTMF_CHANNEL_END)
	{
		flag = DecodeDTMFCallIDPackage(buf, pcallid);
	}
	
	else if (*buf >= FSK_CHANNEL_STRAT && *buf <= FSK_CHANNEL_END)
	{
		flag = DecodeFSKCallIDPackage(buf, pcallid);
	}
	
	if (flag)
	{
		// 过滤无效号码字符
		int		i, j, k;
		i = strlen((char*)(pcallid->number));
		
		for (j = 0, k = 0; k < i; k ++)
		{
			if ((pcallid->number[k] >= '0') && (pcallid->number[k] <= '9'))
			{
				pcallid->number[j] = pcallid->number[k];
				j ++;
			}
		}
		pcallid->number[j] = '\0';
		
		if (j == 0)
		{
			flag = AK_FALSE;
		}
	}
	
	else
	{
		pcallid->number[0] = '\0';
		pcallid->numberstatus = CALLID_ERROR;   //fjm
	}

	return flag;
}

T_VOID ParsePSTNGpioData(unsigned char const* const data, unsigned int const length)
{
	int i,c;
	//static CALLID_INFO	CallID;
	//static UINT8   CallIDbuff[128];
	static int CallIDLen = 0;
	static T_BOOL isHANDSET = AK_FALSE;
	static T_BOOL isHANDFREE = AK_FALSE;
	static unsigned char CallIDbuff[128];
	
	//extern CTestDlg *m_pTestDlg0;
	//extern BOOL _test_call;
	//Sleep(10);
	
	for (i=0; i<length; i++)
	{
		if(CallIDLen >= 128)
			CallIDLen = 0;
		c = data[i];
		AK_DEBUG_OUTPUT("%x\n", data[i]);


		/*CMultimediaPhoneDlg* main = (CMultimediaPhoneDlg*)theApp.m_pMainWnd;
		int rr = main->phone_->DetectTestStatus(c);
		if(rr == 0)
		{
			PostMessage(theApp.m_pMainWnd->m_hWnd, WM_CLEARPWD, 0, 0);
		}
		else if(rr == 1)
		{
			PostMessage(theApp.m_pMainWnd->m_hWnd, WM_FORMATDATA, 0, 0);
		}*/


		//85+C8=手柄提起+手柄摘机  b0+C9=免提按键+免提摘机
		isHANDSET = (c == e_HANDLEUP_VALUE)?AK_TRUE:AK_FALSE;
		isHANDFREE = (c == e_HANDSFREE_VALUE)?AK_TRUE:AK_FALSE;
		
		//todo 对c的解析和处理
		switch(c)
		{
		case e_RING_VALUE:			//振铃
			{
				//keybd_event(VK_F9, 0, 0, 0);	//这句不能用,如果不等于VK_F9，背光恢复到设置状态

				//窗口发送消息
				//m_triggerEvent(M_EVT_RING, AK_NULL);
			}
			break;
		case e_HANDPICKUP_VALUE:	//手柄摘机
			if (isHANDSET)
			{
				isHANDSET = AK_FALSE;
				//m_triggerEvent(M_EVT_PICKUP, AK_NULL);
			}
			break;
		case e_FREEPICKUP_VALUE:	//免提摘机
			if (isHANDFREE)
			{
				isHANDFREE = AK_FALSE;
				//m_triggerEvent(M_EVT_PICKUP, AK_NULL);
			}
			break;
		case e_HANGUP_VALUE:	//手柄或免提挂机消息
			{	
				//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_TEL_HUNGOFF, 0, 0);
				CallIDLen = 0;
			}
			break;
		case e_FSK_END_VALUE:
			
			//CallIDbuff[CallIDLen++] = c;
			//memset(&CallID, 0, sizeof(CALLID_INFO));
			//DecodeCallIDPackage(CallIDbuff, &CallID);
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_TEL_CALLIDEND, (WPARAM)&CallID, 0);
			CallIDLen = 0;
			break;
		case e_DTMF_END_VALUE:
			
			//CallIDbuff[CallIDLen++] = c;
			//memset(&CallID, 0, sizeof(CALLID_INFO));
			//DecodeCallIDPackage(CallIDbuff, &CallID);
			//if(strlen(CallID.number)>2)
				//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_TEL_CALLIDEND, (WPARAM)&CallID, 0);
			CallIDLen = 0;
			break;
		case e_DOWN_VALUE:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_KEYDOWN, 'U', 0);
			break;
		case e_LEFT_VALUE:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_KEYDOWN, 'Z' ,0);
			break;
		case e_RIGHT_VALUE:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_KEYDOWN, 'R', 0);
			break;
		case e_OK_VALUE:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_KEYDOWN, 'O', 0);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			//isRingTelCode = TRUE;
			CallIDbuff[CallIDLen++] = c;
			break;
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			//isRingTelCode = TRUE;
			CallIDbuff[CallIDLen++] = c;
			break;
		case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_TEL_KEYCODE, g_tel_code[c-0xB0], 0);
			{
				//keybd_event(g_tel_code[c-0xB0],   0,   0,   0);  
				//keybd_event(g_tel_code[c-0xB0],   0,   KEYEVENTF_KEYUP,   0);
			}
			break;
		case FASTDIAL_F1://速拨F1-F4
		case FASTDIAL_F2:
		case FASTDIAL_F3:
		case FASTDIAL_F4:
			//PostMessage(theApp.m_pMainWnd->m_hWnd, WM_FAST_KEY, c, 0);
			break;
		}
	}
}
