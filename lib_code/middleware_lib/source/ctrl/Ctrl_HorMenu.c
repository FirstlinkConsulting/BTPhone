/******************************************************************************************
**FileName  	:      Ctrl_HorMenu.c
**brief        	:      a interface for menu between user and machine. it display menus on one line
**					 and the active title below the menu
**Copyright 	:      2004 Anyka (GuangZhou) Software Technology Co., Ltd.
**author 		:	Hongshi Yao
**date			: 	2008-06-07
**version 		:	1.0
*******************************************************************************************/
#include "Ctrl_HorMenu.h"
#include "Gbl_Global.h"

#if(NO_DISPLAY == 0)

// get resource information
#define RES_LOG_LEN(begID, endID)	GetResCount(begID, endID, CTRL_CUR_LANGUAGE)
#define RES_PHY_LOC(begID, offset)	GetNearbyValidStrResID(begID, CTRL_CUR_LANGUAGE, offset)

/***********************************************************************************
**brief: 		initialize radiopalyer default param
**param:		pHorMenu: �ؼ����ƽṹ��ָ��
**				menuBegID : ��ʼ�ַ����� ��menuEndID : ��ֹ�ַ�����pMenuIcon : ͼ����Դ����
**				CurID: ��ʼ��ʱָ����ǰ�����menu
***********************************************************************************/
T_BOOL	HorMenu_Init(CHorMenuCtrls *pHorMenu, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE *pMenuIcon,T_U16 CurID)
{
	T_U16	i;
	
	if((pHorMenu==AK_NULL)||(menuBegID == INVALID_STRINGRES_ID))
		return AK_FALSE;

	//�����û���������ݳ�ʼ���ؼ����ƽṹ��
	pHorMenu->begStrID = menuBegID;
	pHorMenu->MenuMax  = RES_LOG_LEN(menuBegID,menuEndID);

	for(i = 0; i < MENU_LINE_MAX; i++)
	{	//��ʼ��ICON ����
		if(i >= pHorMenu->MenuMax)
		{
			pHorMenu->MenuIcon[i][0] = INVALID_IMAGERES_ID;
			pHorMenu->MenuIcon[i][1] = INVALID_IMAGERES_ID;			
		}else
		{	pHorMenu->MenuIcon[i][0] = *(pMenuIcon + i*2);
			pHorMenu->MenuIcon[i][1] = *(pMenuIcon + i*2+1);
		}
	}

	//��ʼ�����λ��
	if((CurID <= menuEndID)&&(CurID >= menuBegID))
		pHorMenu->CurStrPos = (T_U16)(RES_LOG_LEN(menuBegID,CurID)-1);
	else
	{
		pHorMenu->CurStrPos 	= 0;
	}

	//��ʼ��һ֡����ʼ�˵�

	if(pHorMenu->CurStrPos%MENU_PER_LINE)
		pHorMenu->CrnStartPos  = (T_U16)(pHorMenu->CurStrPos - ((pHorMenu->CurStrPos)%MENU_PER_LINE));
	else
		pHorMenu->CrnStartPos	= pHorMenu->CurStrPos;	

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL; 

	return AK_TRUE;

}


/***********************************************************************************
**brief: 		��ʾˢ��
**param:		pHorMenu: �ؼ����ƽṹ��ָ��
***********************************************************************************/
T_VOID	HorMenu_Show(CHorMenuCtrls *pHorMenu)
{
	T_RES_IMAGE 	imageID;
	T_RES_STRING 	StrID;
	T_POS			left;
	T_POS			MenuTop;
	T_POS			StrTop;
	T_U16			i;
	T_U16			imageInter;

	if((pHorMenu == AK_NULL) 
		|| (pHorMenu->refresh == CTRL_REFRESH_HORMENU_NONE))
		return;

	//����menu��title��λ��
	imageInter = (MAIN_LCD_WIDTH - (MENU_MARGIN_WIDTH*2) - (MENU_PER_LINE * MENU_ICON_WIDTH) )/(MENU_PER_LINE-1);
	i = (MAIN_LCD_HEIGHT - MENU_ICON_HIGH - FONT_HEIGHT)/3;
	MenuTop = (T_POS)i;
	StrTop = (T_POS)(MENU_ICON_HIGH  + (2 * i));
	
	if(pHorMenu->refresh == CTRL_REFRESH_HORMENU_ALL)
	{	//
		#if (!USE_COLOR_LCD)
			Fwl_FillRect(CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT, CLR_BLACK);
		#else
			Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP, TITLE_BKG_IMAGEPTR, AK_FALSE);
			Eng_ImageResDisp(CTRL_WND_LEFT, CTRL_WND_TOP+CTRL_WND_LINEHIGH, MSGB0X_BK_IMAGEPTR, AK_FALSE);
		#endif	
	}

	for(i=0; i < MENU_PER_LINE; i++) 
	{	//��ʼˢ�¸����˵�
		if( ((pHorMenu->CrnStartPos + i) >= pHorMenu->MenuMax)
			  ||(pHorMenu->CrnStartPos + i) >=	MENU_LINE_MAX)
			  break;
		if((pHorMenu->refresh & (CTRL_REFRESH_HORMENU_ENTRY(i))))
		{	//need refresh again
			left = (T_POS)(MENU_MARGIN_WIDTH + i*( imageInter + MENU_ICON_WIDTH));
			if((pHorMenu->CurStrPos - pHorMenu->CrnStartPos) == i)
				imageID = pHorMenu->MenuIcon[pHorMenu->CrnStartPos + i][1];
			else
				imageID = pHorMenu->MenuIcon[pHorMenu->CrnStartPos + i][0];
			Eng_ImageResDisp((T_POS)left, (T_POS)MenuTop, imageID, 0x02);
		}

	}

	if(pHorMenu->refresh & CTRL_REFRESH_HORMENU_TITLE)
	{	//�����ַ���	
		StrID = RES_PHY_LOC(pHorMenu->begStrID,pHorMenu->CurStrPos);
		DispStringInWidth(CP_UNICODE, MENU_STRING_LEFT, StrTop, (T_U8*)StrID, MAIN_LCD_WIDTH,CTRL_WND_FONTCOLOR);
	}

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_NONE;
	
}



T_VOID	HorMenu_SetRefresh(CHorMenuCtrls *pHorMenu)
{
	if(pHorMenu)
	{	//����ˢ�±�־
		pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL;	
	}
}


/***********************************************************************************
**brief: 		��������ƶ��Ĵ���
**param:		pHorMenu: �ؼ����ƽṹ��ָ��
**				dir : AK_TRUE ��������ƶ��� AK_FALSE ��������ƶ�
***********************************************************************************/
static T_VOID HorMenu_MoveFocus(CHorMenuCtrls *pHorMenu, T_BOOL dir)
{
	T_U16 CurPos;
	T_U16 BegPos;
	T_U16 MaxPos;

	if(pHorMenu == AK_NULL)
		return;

	CurPos = pHorMenu->CurStrPos;
	BegPos = pHorMenu->CrnStartPos;
	MaxPos = pHorMenu->MenuMax;

	if(dir)		//�ƶ����
		CurPos++;		//�����ƶ�
	else
	{ 			//�����ƶ�
		if((CurPos == 0)||(CurPos < BegPos))
			CurPos = MENU_INVALID;
		else
			CurPos--;			
	}

	//�����ƶ����Ľ���������
	if(CurPos == MENU_INVALID)
	{
		pHorMenu->CurStrPos  = (T_U16)(MaxPos - 1);
		CurPos = pHorMenu->CurStrPos;
	}
	else if(CurPos >= MaxPos)
	{
		pHorMenu->CurStrPos		= 0;
		pHorMenu->CrnStartPos	= 0;
	}else
	{
		pHorMenu->CurStrPos     = CurPos;
	}

	//���ݹ���λ�õ���һ֡����ʼλ��

	if(pHorMenu->CurStrPos%MENU_PER_LINE)
		pHorMenu->CrnStartPos  = (T_U16)(pHorMenu->CurStrPos - ((pHorMenu->CurStrPos)%MENU_PER_LINE));
	else
		pHorMenu->CrnStartPos	= pHorMenu->CurStrPos;	

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL;
}


/***********************************************************************************
**brief: 		��������������԰����Ĵ���
**param:		pHorMenu: �ؼ����ƽṹ��ָ��
***********************************************************************************/
T_U16  HorMenu_Handler(CHorMenuCtrls *pHorMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
	if(!pHorMenu || M_EVT_USER_KEY!=Event || !pParam || pParam->c.Param2 == PRESS_UP)
		return CTRL_RESPONSE_NONE;

	switch(pParam->c.Param1)
	{
	case CTRL_EVT_KEY_UP:	//��������ƶ�
		HorMenu_MoveFocus(pHorMenu, AK_FALSE);
		break;
	
	case CTRL_EVT_KEY_DOWN:	//��������ƶ�
		HorMenu_MoveFocus(pHorMenu, AK_TRUE);
		break;
		
	case CTRL_EVT_KEY_OK:	//ѡ��ǰ�˵�
        if (pParam->c.Param2 == PRESS_SHORT)
        	return (T_U16)(RES_PHY_LOC(pHorMenu->begStrID,pHorMenu->CurStrPos));
        else
            return CTRL_RESPONSE_QUIT;
	default:
		break;
	}

	return CTRL_RESPONSE_NONE;
}

#endif
//end  of file

