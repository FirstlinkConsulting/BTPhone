/**
 * @file    s_pub_set_menu.c
 * @brief   set menu for all applications
 * @copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Songmengxing
 * @date    2013-04-08
 * @version 1.0
 */
 
#include "akdefine.h"
#include "Fwl_osMalloc.h"
#include "fwl_keypad.h"
#include "M_event.h"
#include "Eng_debug.h"
#include "M_event_api.h"
#include "Ctrl_MenuConfig.h"
#include <string.h>

#define TIMEOUT_EXITCYCSTAT     5


typedef  struct{     
    T_U8	TimeCount;		//计时，5秒自动保存退出
    T_S8*	pStr;			//打印字串指针
} PUB_SET_MENU;

static PUB_SET_MENU *pPubSetMenu = AK_NULL;

void initpub_set_menu()
{
    pPubSetMenu = (PUB_SET_MENU*)Fwl_Malloc(sizeof(PUB_SET_MENU));
    AK_ASSERT_PTR_VOID(pPubSetMenu, "initpub_set_menu pPubSetMenu");
    memset(pPubSetMenu, 0, sizeof(PUB_SET_MENU));
    
    AK_DEBUG_OUTPUT("\nenter s_pub_set_menu !\n");
}

void exitpub_set_menu()
{
    pPubSetMenu = Fwl_Free(pPubSetMenu);
}

void paintpub_set_menu()
{

}

unsigned char handlepub_set_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	if (M_EVT_MENU == event)
	{
		pPubSetMenu->pStr = MenuCfg_GetStr();

		if (AK_NULL != pPubSetMenu->pStr)
		{
        	AK_DEBUG_OUTPUT("\n%s!\n", pPubSetMenu->pStr);
		}
	}
    else if (M_EVT_USER_KEY == event)
    {
        pPubSetMenu->TimeCount = 0;

        if (kbRIGHT == GET_KEY_ID(pEventParm)
            && PRESS_SHORT == GET_KEY_TYPE(pEventParm))//焦点到后一个
        {
            MenuCfg_MoveItem(MOVE_NEXT);
			pPubSetMenu->pStr = MenuCfg_GetStr();

			if (AK_NULL != pPubSetMenu->pStr)
			{
	        	AK_DEBUG_OUTPUT("\n%s!\n", pPubSetMenu->pStr);
			}	
        }
        else if (kbLEFT == GET_KEY_ID(pEventParm) 
            && PRESS_SHORT == GET_KEY_TYPE(pEventParm))//焦点到前一个
        {
            MenuCfg_MoveItem(MOVE_PREVIOUS);
			pPubSetMenu->pStr = MenuCfg_GetStr();

			if (AK_NULL != pPubSetMenu->pStr)
			{
	        	AK_DEBUG_OUTPUT("\n%s!\n", pPubSetMenu->pStr);
			}		
        }
        else if (kbMODE == GET_KEY_ID(pEventParm) 
            && PRESS_SHORT == GET_KEY_TYPE(pEventParm))//切换子模式
        {
            MenuCfg_MoveMenu(MOVE_NEXT);
			pPubSetMenu->pStr = MenuCfg_GetStr();

			if (AK_NULL != pPubSetMenu->pStr)
			{
	        	AK_DEBUG_OUTPUT("\n%s!\n", pPubSetMenu->pStr);
			}			
        }
        else if (kbOK == GET_KEY_ID(pEventParm) 
            && PRESS_SHORT == GET_KEY_TYPE(pEventParm))//短按ok键确认修改并退出
        {			
			MenuCfg_Confirm();

			pEventParm->s.Param1 = RETURN_FROM_SET_MENU;
			pEventParm->s.Param2 = AK_TRUE;
			
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            AK_DEBUG_OUTPUT("\nEXIT pub_set_menu!\n");
        }

    }
    else if (M_EVT_PUB_TIMER == event)
    {
        pPubSetMenu->TimeCount++;
        
        if (pPubSetMenu->TimeCount >= TIMEOUT_EXITCYCSTAT)//时间到自动退出
        {
			pPubSetMenu->TimeCount = 0;

			pEventParm->s.Param1 = RETURN_FROM_SET_MENU;
			pEventParm->s.Param2 = AK_FALSE;

            m_triggerEvent(M_EVT_EXIT, pEventParm);
            AK_DEBUG_OUTPUT("\nEXIT pub_set_menu!\n");
        }
        else
        {
            if (AK_NULL != pPubSetMenu->pStr)
			{
	        	AK_DEBUG_OUTPUT("\n%s!\n", pPubSetMenu->pStr);
			}	
        }
    }
    
    if (event >= M_EVT_Z00_POWEROFF)
    {   
        m_triggerEvent(M_EVT_EXIT, AK_NULL);
        m_triggerEvent(M_EVT_Z00_POWEROFF, pEventParm);
        return 0;
    }    
    else
    {
        return 0; 
    }
           
}

//file end
