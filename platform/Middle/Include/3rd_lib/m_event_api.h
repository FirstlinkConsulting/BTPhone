#ifndef __M_EVENT_API_H__
#define __M_EVENT_API_H__

void  m_triggerEvent(vT_EvtCode event, vT_EvtParam* pEventParm);
unsigned int SM_GetEventMaxEntries(void);
unsigned int SM_CalcEventBufferByMaxEntries(unsigned int maxEntries);

#endif
