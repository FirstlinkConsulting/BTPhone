#ifndef _BA_LIB_API_H_
#define _BA_LIB_API_H_

#if defined __cplusplus
extern "C" {
#endif


#include "anyka_types.h"
#include "BA_user_msg.h"

#define  BA_LIB_VERSION_STRING "BlueA_V1.0.06_svn727"


typedef T_pVOID   (*BA_CALLBACK_FUN_MALLOC)(T_U32 size, T_CHR * fun, T_U32 line); 
typedef T_VOID    (*BA_CALLBACK_FUN_FREE)(T_pVOID mem, T_CHR * fun, T_U32 line);
typedef T_pVOID   (*BA_CALLBACK_FUN_DMA_MALLOC)(T_U32 size, T_CHR * fun, T_U32 line); 
typedef T_VOID    (*BA_CALLBACK_FUN_DMA_FREE)(T_pVOID mem, T_CHR * fun, T_U32 line);
typedef T_S32     (*BA_CALLBACK_FUN_PRINTF)(T_pCSTR format, ...);

typedef T_S32     (*BA_CALLBACK_FUN_INIT_TRANSPORT)(T_VOID);
typedef T_S32     (*BA_CALLBACK_FUN_DEINIT_TRANSPORT)(T_VOID);
typedef T_S32     (*BA_CALLBACK_FUN_TRANSPORT_SEND)(T_pCVOID buffer, T_U32 count);

typedef T_S32     (*BA_CALLBACK_FUN_PLATFORM_INTERRUPT)(T_BOOL on);
typedef T_U32     (*BA_CALLBACK_FUN_GET_TICK)(T_VOID);

typedef T_U32     (*BA_CALLBACK_FUN_DUMP_HCI_PACKET)(T_U8 PacketType, T_BOOL DirectionIn, T_U8 *buffer, T_U16 count);


// call back functions that platform should provide
typedef struct BA_CB_FUNS
{
    BA_CALLBACK_FUN_MALLOC              Malloc; // malloc
    BA_CALLBACK_FUN_FREE                Free; // free
    BA_CALLBACK_FUN_DMA_MALLOC          DmaMalloc; // memory allocation for DMA, i.e., 4K block sized memory
    BA_CALLBACK_FUN_DMA_FREE            DmaFree; // free for DMA memory
    BA_CALLBACK_FUN_PRINTF              Printf; // printf
    BA_CALLBACK_FUN_INIT_TRANSPORT      InitTransport; // power on and do init stuff specific to transport hardware
    BA_CALLBACK_FUN_DEINIT_TRANSPORT    DeinitTransport; // power off transport hardware, which shall not working after this
    BA_CALLBACK_FUN_TRANSPORT_SEND      TransportSend; // send out data through transfort layer
    BA_CALLBACK_FUN_PLATFORM_INTERRUPT  PlatformInterrupt; // disable/enable OS interrupt, to pretect read/write of transport rx buffer
    BA_CALLBACK_FUN_GET_TICK            GetTick; // get system ticks (unit: T_BA_IN_INFO.msPerTick)
    BA_CALLBACK_FUN_USER_MSG            UserMsg; // BALib send message to user by this call back function
    BA_CALLBACK_FUN_DUMP_HCI_PACKET     DumpHciPacket; // can be NULL
}T_BA_CALLBACK_FUNS;

// NOTE: Platform shall also provide linkage of these standard C routines:
//  memcpy, memcmp, memset, strlen, strncpy, strncmp, strcmp, sprintf


// constants used to config BALib
typedef struct BA_IN_INFO
{
    T_pCSTR strVersion; // must be BA_LIB_VERSION_STRING
    T_U16   msPerTick; // # ms per system tick, used with GetTick call back function
    T_U32   TransportRxBufFreeSize; // transport rx buffer is a dynamically sized buffer, it would be at least this size (byte)
    T_U32   TransportRxBufMaxSize; // transport rx buffer would not exceed this size (byte)
    T_U16   msPageTimeout; // set bluetooth page timeout in ms. 1~40000ms.
    T_U16   msLinkSupervisionTimeout; // set link supervision timeout in ms. 1~40000ms, or 0 for no timeout.
}T_BA_IN_INFO;


typedef struct BA_INPUT
{
    T_BA_CALLBACK_FUNS  cb_fun;
    T_BA_IN_INFO    info;
}T_BA_INPUT;


// init BALib
// Shall be called before any other API
T_BOOL BA_Init(T_BA_INPUT *BA_Input);

// deinit BALib
// Shall be called when BALib is fully stopped.
T_BOOL BA_Deinit(T_VOID);

// force BALib to unload itself and clear all resources
// Only used in case of unrecoverable error or not responding.
//  Use it carefully for it left remote device not knowing the disconnection.
T_BOOL BA_ForceDeinit(T_VOID);

// BALib start running
T_VOID BA_Start(T_VOID);

// disconnect, stop all profile and core stack
// HOST_SHUTDOWN_CNF msg notifies user that BALib is fully stopped.
//  bStopBtStack
//      1: disconnect, stop all profile and core stack, with HOST_SHUTDOWN_CNF msg
//      0: disconnect, stop all profile, make undiscoverable, no HOST_SHUTDOWN_CNF msg
T_VOID BA_Stop(T_BOOL bStopBtStack);

// disconnect all connections with the remote device
// User should wait until no ACL connection is present.
T_VOID BA_Disconnect(T_BD_ADDR bd_addr);

// get connection status with the remote device
//  connections: bit-or of connections BA_CONNECTION_XXX, 1 means connection exist
#define BA_CONNECTION_A2DP     ((T_U32)(1<<0))
#define BA_CONNECTION_AVRCP    ((T_U32)(1<<1))
#define BA_CONNECTION_HFP      ((T_U32)(1<<2))
#define BA_CONNECTION_SCO      ((T_U32)(1<<14))
#define BA_CONNECTION_ACL      ((T_U32)(1<<15))
#define BA_CONNECTION_CREATING ((T_U32)(1<<16)) // just requested to create connection
T_BOOL BA_GetConnectionStatus(T_BD_ADDR bd_addr, T_U32 *connections);

// BALib working routine
// Shall be regularly called throughout from start running to fully stopped.
T_BOOL BA_Process(T_U32 ticks);

// let BA_Process return as fast as possible
// For example, when user receives certain user msg during calling BA_Process
//  and needs to handle it urgently, this is the place to call BA_ExitProcess.
T_VOID BA_ExitProcess(T_VOID);

// send received transport data to BALib
// BALib maintains an internal transport rx buffer. 
// User may pass on data without notice of its format.
T_S32  BA_TransportReceive(T_pCVOID buffer, T_U32 count);

// make sure the internal transport rx buffer has sufficient room
// User can call it regularly at short intervals in a separate thread to avoid
//  buffer overflow in case of data flood.
T_BOOL BA_TransportReceive_CheckBuf(T_VOID);

// stop to process SCO data.
// Overhead to parse SCO data packet can be saved, and user would not recieve 
//  SCO data msg after calling it.
T_VOID BA_BypassSCORxData(T_BOOL bypass);


#if defined __cplusplus
}
#endif

#endif // _BA_LIB_API_H_

