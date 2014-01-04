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
**param:		pHorMenu: 控件控制结构体指针
**				menuBegID : 起始字符串号 ，menuEndID : 终止字符串号pMenuIcon : 图标资源数组
**				CurID: 初始化时指定当前激活的menu
***********************************************************************************/
T_BOOL	HorMenu_Init(CHorMenuCtrls *pHorMenu, T_U16 menuBegID, T_U16 menuEndID, T_RES_IMAGE *pMenuIcon,T_U16 CurID)
{
	T_U16	i;
	
	if((pHorMenu==AK_NULL)||(menuBegID == INVALID_STRINGRES_ID))
		return AK_FALSE;

	//根据用户传入的数据初始化控件控制结构体
	pHorMenu->begStrID = menuBegID;
	pHorMenu->MenuMax  = RES_LOG_LEN(menuBegID,menuEndID);

	for(i = 0; i < MENU_LINE_MAX; i++)
	{	//初始化ICON 数组
		if(i >= pHorMenu->MenuMax)
		{
			pHorMenu->MenuIcon[i][0] = INVALID_IMAGERES_ID;
			pHorMenu->MenuIcon[i][1] = INVALID_IMAGERES_ID;			
		}else
		{	pHorMenu->MenuIcon[i][0] = *(pMenuIcon + i*2);
			pHorMenu->MenuIcon[i][1] = *(pMenuIcon + i*2+1);
		}
	}

	//初始化光标位置
	if((CurID <= menuEndID)&&(CurID >= menuBegID))
		pHorMenu->CurStrPos = (T_U16)(RES_LOG_LEN(menuBegID,CurID)-1);
	else
	{
		pHorMenu->CurStrPos 	= 0;
	}

	//初始化一帧的起始菜单

	if(pHorMenu->CurStrPos%MENU_PER_LINE)
		pHorMenu->CrnStartPos  = (T_U16)(pHorMenu->CurStrPos - ((pHorMenu->CurStrPos)%MENU_PER_LINE));
	else
		pHorMenu->CrnStartPos	= pHorMenu->CurStrPos;	

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL; 

	return AK_TRUE;

}


/***********************************************************************************
**brief: 		显示刷新
**param:		pHorMenu: 控件控制结构体指针
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

	//计算menu和title的位置
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
	{	//开始刷新各个菜单
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
	{	//书信字符串	
		StrID = RES_PHY_LOC(pHorMenu->begStrID,pHorMenu->CurStrPos);
		DispStringInWidth(CP_UNICODE, MENU_STRING_LEFT, StrTop, (T_U8*)StrID, MAIN_LCD_WIDTH,CTRL_WND_FONTCOLOR);
	}

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_NONE;
	
}



T_VOID	HorMenu_SetRefresh(CHorMenuCtrls *pHorMenu)
{
	if(pHorMenu)
	{	//设置刷新标志
		pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL;	
	}
}


/***********************************************************************************
**brief: 		光标上下移动的处理
**param:		pHorMenu: 控件控制结构体指针
**				dir : AK_TRUE 光标向下移动， AK_FALSE 光标向上移动
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

	if(dir)		//移动光标
		CurPos++;		//向下移动
	else
	{ 			//向上移动
		if((CurPos == 0)||(CurPos < BegPos))
			CurPos = MENU_INVALID;
		else
			CurPos--;			
	}

	//根据移动光标的结果调整光标
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

	//根据光标的位置调整一帧的起始位置

	if(pHorMenu->CurStrPos%MENU_PER_LINE)
		pHorMenu->CrnStartPos  = (T_U16)(pHorMenu->CurStrPos - ((pHorMenu->CurStrPos)%MENU_PER_LINE));
	else
		pHorMenu->CrnStartPos	= pHorMenu->CurStrPos;	

	pHorMenu->refresh = CTRL_REFRESH_HORMENU_ALL;
}


/***********************************************************************************
**brief: 		主处理函数，负责对按键的处理
**param:		pHorMenu: 控件控制结构体指针
***********************************************************************************/
T_U16  HorMenu_Handler(CHorMenuCtrls *pHorMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
	if(!pHorMenu || M_EVT_USER_KEY!=Event || !pParam || pParam->c.Param2 == PRESS_UP)
		return CTRL_RESPONSE_NONE;

	switch(pParam->c.Param1)
	{
	case CTRL_EVT_KEY_UP:	//光标向上移动
		HorMenu_MoveFocus(pHorMenu, AK_FALSE);
		break;
	
	case CTRL_EVT_KEY_DOWN:	//光标向下移动
		HorMenu_MoveFocus(pHorMenu, AK_TRUE);
		break;
		
	case CTRL_EVT_KEY_OK:	//选择当前菜单
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

