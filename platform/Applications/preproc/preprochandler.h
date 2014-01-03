
#ifndef _PREPROCHANDLER_H_
#define _PREPROCHANDLER_H_

#include "vme.h"

unsigned char ddsysstarthandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm);
unsigned char dduserkeyhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm);
unsigned char ddpubtimerhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm);
unsigned char ddsdcardhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm);


#endif // endif _PREPROCHANDLER_H_

