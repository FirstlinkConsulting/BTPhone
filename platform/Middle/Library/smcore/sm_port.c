#include "m_state.h"
#include "sm_port.h"
#include "Gbl_Global.h"
#ifdef OS_ANYKA
#pragma arm section zidata = "_cachedata_"
#endif
static M_STATESTACK gb_StackBuffer[MAX_STACK_DEPTH];
static M_EVENTENTRY gb_EventQueueBuffer[MAX_EVENTQUEUE_ENTRIES];

#ifdef OS_ANYKA
#pragma arm section zidata
#endif

static M_STATE_CORE_STRUCT SmcoreStruct = {0};


static M_STATESTACK *SM_GetStackBuffer(void);
static M_EVENTENTRY *SM_GetEventQueueBuffer(void);
static vT_EvtCode SM_GetEvtReturn(void);
static M_STATES SM_GetPreProcID(void);
static M_STATES SM_GetPostProcID(void);
static int SM_GetStackSize(void);
static int SM_GetEventQueueSize(void);



const M_STATE_CB SmcoreCb = 
{
	&SmcoreStruct,
	SM_GetStackBuffer,
	SM_GetEventQueueBuffer,
	SM_GetEvtReturn,
	SM_GetPreProcID,
	SM_GetPostProcID,
	SM_GetStackSize,
	SM_GetEventQueueSize,
};

#define SM_PORT_initStateHandler_ROM_OFF	(0x5718)
#define SM_PORT_regSuspendFunc_ROM_OFF	(0x575C)
#define SM_PORT_regResumeFunc_ROM_OFF	(0x576C)
#define SM_PORT_triggerEvent_ROM_OFF	(0x577C)
#define SM_PORT_paint_ROM_OFF	(0x57F4)
#define SM_PORT_mainloop_ROM_OFF	(0x5ACC)
#define SM_PORT_GetCurrentSM_ROM_OFF	(0x5BE0)
#define SM_PORT_SetPaintStatus_ROM_OFF	(0x5BF0)
#define SM_PORT_GetVersion_ROM_OFF	(0x5BFC)


const T_U32 rom_add_arm[] = {SM_PORT_initStateHandler_ROM_OFF, //m_initStateHandler_Rom
							 SM_PORT_regSuspendFunc_ROM_OFF, //m_regSuspendFunc_Rom
							 SM_PORT_regResumeFunc_ROM_OFF, //m_regResumeFunc_Rom
							 SM_PORT_triggerEvent_ROM_OFF, //m_triggerEvent_Rom
							 SM_PORT_paint_ROM_OFF, //m_paint_Rom
							 SM_PORT_mainloop_ROM_OFF, //m_mainloop_Rom
							 SM_PORT_GetCurrentSM_ROM_OFF, //SM_GetCurrentSM_Rom
							 SM_PORT_SetPaintStatus_ROM_OFF, //SM_SetPaintStatus_Rom
							 SM_PORT_GetVersion_ROM_OFF	 //SM_GetVersion_Rom	
							 }; 


#if(STORAGE_USED == NAND_FLASH || STORAGE_USED == SD_CARD)
#pragma arm section code = "_sysinit_"
#else
#pragma arm section code = "_bootcode1_"
#endif
static M_STATESTACK *SM_GetStackBuffer(void)
{
    return gb_StackBuffer;
}

static M_EVENTENTRY *SM_GetEventQueueBuffer(void)
{
    return gb_EventQueueBuffer;
}
#pragma arm section code

#pragma arm section code = "_bootcode1_"
static vT_EvtCode SM_GetEvtReturn(void)
{
    return M_EVT_EXIT;
}

/*
vT_EvtCode SM_GetEvtPowerOff(void)
{
    return M_EVT_Z00_POWEROFF;
}*/

static M_STATES SM_GetPreProcID(void)
{
    return eM_preproc;
}

static M_STATES SM_GetPostProcID(void)
{
    return eM_postproc;
}

static int SM_GetStackSize(void)
{
    return MAX_STACK_DEPTH;
}
#pragma arm section code

#pragma arm section code = "_bootcode1_"

static int SM_GetEventQueueSize(void)
{
    return MAX_EVENTQUEUE_ENTRIES;
}
#pragma arm section code 



void m_initStateHandler(void)
{
	_finitStateHandler pFuncRom = (_finitStateHandler)rom_add_arm[0];//m_initStateHandler_Rom;
	SmcoreCb.pSmcoreStruct->m_statearray = SM_GetStateArray();
	SmcoreCb.pSmcoreStruct->m_funcarray = SM_GetfHande();
	SmcoreCb.pSmcoreStruct->m_efuncarray = SM_GeteHandle();
	pFuncRom(&SmcoreCb);
}


void m_regSuspendFunc(_fVoid pfSuspend)
{
	_fregSuspendFunc pFuncRom = (_fregSuspendFunc)rom_add_arm[1];//m_regSuspendFunc_Rom;

	pFuncRom(&SmcoreCb, pfSuspend);
}

void m_regResumeFunc(_fVoid pfResume)
{
	_fregResumeFunc pFuncRom = (_fregResumeFunc)rom_add_arm[2];//m_regResumeFunc_Rom;

	pFuncRom(&SmcoreCb, pfResume);
}

void m_triggerEvent(vT_EvtCode event, vT_EvtParam* pEventParm)
{
	_ftriggerEvent pFuncRom = (_ftriggerEvent)rom_add_arm[3];//m_triggerEvent_Rom;

	pFuncRom(&SmcoreCb, event, pEventParm);
}

void m_paint(void)
{
	_fpaint pFuncRom = (_fpaint)rom_add_arm[4];//m_paint_Rom;

	pFuncRom(&SmcoreCb);
}

void m_mainloop(vT_EvtCode Event, vT_EvtParam *pParam)
{
	_fmainloop pFuncRom = (_fmainloop)rom_add_arm[5];//m_mainloop_Rom;

	pFuncRom(&SmcoreCb, Event, pParam);
}

unsigned int SM_GetCurrentSM(void)
{
	_fGetCurrentSM pFuncRom = (_fGetCurrentSM)rom_add_arm[6];//SM_GetCurrentSM_Rom;

	return pFuncRom(&SmcoreCb);
}

void SM_SetPaintStatus(unsigned char paint)
{
	_fSetPaintStatus pFuncRom = (_fSetPaintStatus)rom_add_arm[7];//SM_SetPaintStatus_Rom;

	pFuncRom(&SmcoreCb, paint);
}

T_pSTR SM_GetVersion(T_VOID)
{
	_fGetVersion pFuncRom = (_fGetVersion)rom_add_arm[8];//SM_GetVersion_Rom;

	return pFuncRom();
}

