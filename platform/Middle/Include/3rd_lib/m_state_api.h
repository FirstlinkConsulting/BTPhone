#ifndef __M_STATE_API_H__
#define __M_STATE_API_H__

#include "anyka_types.h"
#include "sm_core.h"

#define SMCORELIBVERSION "V1.1.7"

#define M_STATES    T_U8

typedef void (*_fVoid)(void);

/** definition of transmit type of state machine*/
typedef enum
{
	/** @{@name all function call types
     */    
	/** funcall*/
    eM_TYPE_SCALL,
	/** efuncall*/
    eM_TYPE_ECALL,
	/** none*/
    eM_TYPE_NONE,
	/** @} */

	/** @{@name all transistion types
     */  
	/** popped transition*/
    eM_TYPE_EXIT,
	/** pushed transition*/
    eM_TYPE_IRPT,
	/** @} */

	/** @{@name all return types
     */  
	/** return top state transition*/
    eM_TYPE_RETURN_ROOT,
	/** return n level transition*/
    eM_TYPE_RETURN
	/** @} */
} M_TRANSTYPE;

//vT_EvtCode must defined as unsigned int outside lib
//sizeof(vT_EvtParam) must <= sizeof(unsigned long[4]) outside lib


/**
 * @brief process function of state machine events
 * @param[in] event   incoming event value 
 * @param[in] pEventParm   address pointer of incoming event parameter
 * @return 1: transmit this event to next state;
 *         0: this event has been processed
 */
typedef unsigned char (*_fHandle)(vT_EvtCode event, vT_EvtParam* pEventParm);
/**
 * @brief process efunction of state machine events
 * @param[in] event   incoming event value 
 * @param[in] pEventParm   address pointer of incoming event parameter
 * @return 1: transmit this event to next state;
 *         0: this event has been processed
 */
typedef unsigned char (*_feHandle)(vT_EvtCode* event, vT_EvtParam** pEventParm);
/**
 * @brief query function for state machine transfer
 * @param[in] event    event value 
 * @param[in] pTrans   transfer type
 * @return ID for event processing or destination state machine 
 */
typedef int (*_fGetNextState)(vT_EvtCode event, M_TRANSTYPE *pTrans);

/** structure type definition for state*/
typedef struct 
{
	/** address pointer for init function*/
    _fVoid pInit;
	/** address pointer for exit function*/
    _fVoid pExit;
	/** address pointer for paint function*/
    _fVoid pPaint;
	/** address pointer for next state machine*/
    _fGetNextState pNext;
} M_STATESTRUCT;


typedef struct {
    M_STATES state;
    _fVoid pfSuspend;
    _fVoid pfResume;    
} M_STATESTACK;


typedef M_STATESTACK* (*_fGetStackBuffer)(void);
typedef M_EVENTENTRY* (*_fGetEventQueueBuffer)(void);
typedef vT_EvtCode (*_fGetEvtReturn)(void);
typedef M_STATES (*_fGetPreProcID)(void);
typedef M_STATES (*_fGetPostProcID)(void);
typedef int (*_fGetStackSize)(void);
typedef int (*_fGetEventQueueSize)(void);

typedef struct {

	M_EVENTENTRY			*m_pFree;
	M_EVENTENTRY			*m_pFirst;
	M_EVENTENTRY			*m_pLast;

	M_STATESTACK			*m_pstateStack;
	unsigned short			m_stateDepth;

	M_STATESTRUCT			**m_statearray;
	_fHandle				*m_funcarray;
	_feHandle				*m_efuncarray;

	M_STATES				m_actualState;
	unsigned char volatile	m_triggerflag;
	unsigned char			m_paint_status;
} M_STATE_CORE_STRUCT;

typedef struct {

	M_STATE_CORE_STRUCT 	*pSmcoreStruct;
	_fGetStackBuffer 		GetStackBuffer;
	_fGetEventQueueBuffer	GetEventQueueBuffer;
	_fGetEvtReturn			GetEvtReturn;
	_fGetPreProcID			GetPreProcID;
	_fGetPostProcID			GetPostProcID;
	_fGetStackSize			GetStackSize;
	_fGetEventQueueSize		GetEventQueueSize;
} M_STATE_CB;

typedef void (*_finitStateHandler)(const M_STATE_CB* pCb);
typedef void (*_fmainloop)(const M_STATE_CB* pCb, vT_EvtCode Event, vT_EvtParam *pParam);
typedef void (*_fregSuspendFunc)(const M_STATE_CB* pCb, _fVoid pfSuspend);
typedef void (*_fregResumeFunc)(const M_STATE_CB* pCb, _fVoid pfResume);
typedef unsigned int (*_fGetCurrentSM)(const M_STATE_CB* pCb);
typedef void (*_fSetPaintStatus)(const M_STATE_CB* pCb, unsigned char paint);
typedef T_pSTR (*_fGetVersion)(void);
typedef void (*_fpaint)(const M_STATE_CB* pCb);
typedef void (*_ftriggerEvent)(const M_STATE_CB* pCb, vT_EvtCode event, vT_EvtParam* pEventParm);



/** @defgroup State_API State interface 
 *	@ingroup SMCORE
 */
/*@{*/
/**
 * @brief init SM Core
 * @param[in] void
 * @return void
 */
void m_initStateHandler_Rom(const M_STATE_CB* pCb);

/**
 * @brief main loop for management of stae machine
 * @param[in] Event   triggered event 
 * @param[in] pParam   address pointer of triggered event parameter
 * @return void
 */
void m_mainloop_Rom(const M_STATE_CB* pCb, vT_EvtCode Event, vT_EvtParam *pParam);

/**
 * @brief register suspend function
 * @param[in] pfSuspend   address pointer for suspend function
 * @return void
 */
void  m_regSuspendFunc_Rom(const M_STATE_CB* pCb, _fVoid pfSuspend);

/**
 * @brief register resume function
 * @param[in] pfResume   address pointer for resume function
 * @return void
 */
void  m_regResumeFunc_Rom(const M_STATE_CB* pCb, _fVoid pfResume);



/**
 * @brief get current ID of state machine on toe of stack
 * @param[in] void
 * @return ID of state machine on toe of stack
 */
unsigned int SM_GetCurrentSM_Rom(const M_STATE_CB* pCb);



/**
 * @brief call this function to enable or disable m_paint_Rom function
 * @param[in] paint status
 * @return void
 */
void SM_SetPaintStatus_Rom(const M_STATE_CB* pCb, unsigned char paint);

/**
 * @brief call this function to get version str
 * @param[in] void
 * @return T_pSTR
 */
T_pSTR SM_GetVersion_Rom(T_VOID);

void m_triggerEvent_Rom(const M_STATE_CB* pCb, vT_EvtCode event, vT_EvtParam* pEventParm);

extern void m_paint_Rom(const M_STATE_CB* pCb);

/*@}*/
#endif
