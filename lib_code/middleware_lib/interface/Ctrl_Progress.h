#ifndef __CPROGRESS_CTRL_H__
#define __CPROGRESS_CTRL_H__

#include "Ctrl_Public.h"
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
#include "Ctrl_TopBar.h"
#endif

/***********************************************************************************
**brief:        ��ΪProgress_Handler�������ص�ֻ��������
                ������������Ǹ�������������Ҫת��Ϊ10��������������
                ͬ�������Ҳ��Ҫת��Ϊ0.1����ʵ��
***********************************************************************************/
#define CTRL_PROGRESS_SWITCHINT2FLOAT(value)    ((T_FLOAT)((value)*0.1)) //����ת������*0.1
#define CTRL_PROGRESS_SWITCHFLOAT2INT(value)    ((T_U16)((value)*10))    //������ת����*10
#define CTRL_PROGRESS_SWITCHINT2INT(value)      ((T_U16)((value)/10))    //����ת����/10

#define CTRL_PROGRESS_RESPONSE_CHANGED          0xFFFD


typedef struct tagCProgressCtrl
{
    T_S16   minValue;       //������������С��ֵ
    T_S16   maxValue;       //��������������ֵ
    T_BOOL  bFloat;         //�Ƿ񸡵���ֵ��ʾ
    T_U16   refresh;
    T_FLOAT stepIntvl;      //����ÿ����Ծ���
    T_FLOAT curValue;       //���������鵱ǰֵ
    T_RES_IMAGE iconPtr;    //�����ʾ��ICON
    T_RES_IMAGE riconPtr;   //�ұ���ʾ��ICON
    T_RES_IMAGE bkImage;    //background image
    T_U16   titleStringID;
    T_POS   xBefore;        //�����������ϴ����������� <for only display small region>

    T_U32   firstData;      //�����������2������
    T_U32   secondData;
    T_U32   firstIcon;      //�����������2�����ݣ��ֱ��в�ͬICON
    T_U32   secondIcon;
    T_BOOL  bScroll;        //��ǰ״̬�ܷ������ƶ�������

    T_U16   navigateID;     //navigate bar menu id
    T_U16   bkimgTitle;

    T_POS   left;
    T_POS   top;
    T_LEN   width;
    T_LEN   height;
#if ((LCD_TYPE == 3)&&(1 == LCD_HORIZONTAL))
    T_TOP_BAR_CTRL pro_topbar;
#endif

}CProgressCtrl;

T_BOOL  Progress_InitEx(CProgressCtrl *pProgress, T_S16 minValue, T_S16 maxValue, T_U16 titleStringID, T_RES_IMAGE iconPtr, T_U16 nIntvl, T_FLOAT curValue, T_BOOL bFloat, T_RES_IMAGE riconPtr, T_RES_IMAGE bkImg, T_POS left, T_POS top, T_LEN width, T_LEN height);
#define Progress_Init(pProgress, minValue, maxValue, titleStringID, iconPtr, nIntvl, curValue, bFloat, riconPtr, bkImg) Progress_InitEx((pProgress), (minValue), (maxValue), (titleStringID), (iconPtr), (nIntvl), (curValue), (bFloat), (riconPtr), (bkImg), CTRL_WND_LEFT, CTRL_WND_TOP, CTRL_WND_WIDTH, CTRL_WND_HEIGHT)

T_VOID  Progress_Show(CProgressCtrl *pProgress);
T_U16   Progress_Handler(CProgressCtrl *pProgress, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID  Progress_SetRefresh(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:        ��ȡ��ǰ�����ֵ
***********************************************************************************/
T_BOOL  Progress_SetPos(CProgressCtrl *pProgress, T_FLOAT curValue);

/***********************************************************************************
**brief:        ���û����
**parm:         curValue:�����ֵ
***********************************************************************************/
T_FLOAT Progress_GetPos(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:    ������������������ݣ�
            ����2���ַ����͸��Ե�ͼ��
***********************************************************************************/
T_BOOL  Progress_SetContent(CProgressCtrl *pProgress, T_U32 firstData, T_U16 firstIcon, T_U32 secondData, T_U16 secondIcon, T_BOOL bScroll);

T_VOID  Progress_Free(CProgressCtrl *pProgress);

/***********************************************************************************
**brief:        ���ò˵�������
                menuID: �����˵���ԴID
***********************************************************************************/
T_VOID  Progress_SetNavigate(CProgressCtrl *pProgress, T_U16 menuID);

/***********************************************************************************
**brief:        ���ò˵�������ͼƬ
                bkimgID: ����ͼƬ��ԴID
***********************************************************************************/
T_VOID  Progress_SetTitle(CProgressCtrl *pProgress, T_U16 bkimgID);


#endif

