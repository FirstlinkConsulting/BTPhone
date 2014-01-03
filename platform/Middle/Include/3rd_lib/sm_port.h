#ifndef _SM_PORT_H_
#define _SM_PORT_H_

#include "sm_core.h"

const _fHandle* SM_GetfHande(void);

const _feHandle* SM_GeteHandle(void);

const M_STATESTRUCT** SM_GetStateArray(void);



void m_initStateHandler(void);

void m_regSuspendFunc(_fVoid pfSuspend);

void m_regResumeFunc(_fVoid pfResume);

void m_triggerEvent(vT_EvtCode event, vT_EvtParam* pEventParm);

void m_paint(void);

void m_mainloop(vT_EvtCode Event, vT_EvtParam *pParam);

unsigned int SM_GetCurrentSM(void);

void SM_SetPaintStatus(unsigned char paint);

T_pSTR SM_GetVersion(T_VOID);


#endif

