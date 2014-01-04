#ifndef _BA_AVRCP_H_
#define _BA_AVRCP_H_

#if defined __cplusplus
extern "C" {
#endif

#include "BA_lib_api.h"

typedef enum _BA_AVRCP_KEY
{
    OPID_SELECT             = (0x0),
    OPID_UP,
    OPID_DOWN,
    OPID_LEFT,
    OPID_RIGHT,
    OPID_RIGHT_UP,
    IPID_RIGHT_DOWN,
    OPID_LEFT_UP,
    OPID_LEFT_DOWN,
    OPID_ROOT_MENU,
    OPID_SETUP_MENU,
    OPID_CONTENTS_MENU,
    OPID_FAVOURITE_MENU,
    OPID_EXIT,
    /* 0x0e to 0x1f Reserved */
    OPID_0                  = (0x20),
    OPID_1,
    OPID_2,
    OPID_3,
    OPID_4,
    OPID_5,
    OPID_6,
    OPID_7,
    OPID_8,
    OPID_9,
    OPID_DOT,
    OPID_ENTER,
    OPID_CLEAR,
    /* 0x2d - 0x2f Reserved */
    OPID_CHANNEL_UP         = (0x30),
    OPID_CHANNEL_DOWN,
    OPID_SOUND_SELECT,
    OPID_INPUT_SELECT,
    OPID_DISPLAY_INFORMATION,
    OPID_HELP,
    OPID_PAGE_UP,
    OPID_PAGE_DOWN,
    /* 0x39 - 0x3f Reserved */
    OPID_POWER              = (0x40),
    OPID_VOLUME_UP,
    OPID_VOLUME_DOWN,
    OPID_MUTE,
    OPID_PLAY,
    OPID_STOP,
    OPID_PAUSE,
    OPID_RECORD,
    OPID_REWIND,
    OPID_FAST_FORWARD,
    OPID_EJECT,
    OPID_FORWARD,
    OPID_BACKWARD,
    /* 0x4d - 0x4f Reserved */
    OPID_ANGLE              = (0x50),
    OPID_SUBPICTURE,
    /* 0x52 - 0x70 Reserved */
    OPID_F1                 = (0x71),
    OPID_F2,
    OPID_F3,
    OPID_F4,
    OPID_F5,
    OPID_VENDOR_UNIQUE      = (0x7e)
    /* Ox7f Reserved */

}BA_AVRCP_KEY;

typedef enum _BA_AVRCP_KEY_FLAG
{
    OPID_FLAG_PUSHED        = (0x00),
    OPID_FLAG_RELEASED      = (0x80),
}BA_AVRCP_KEY_FLAG;

T_S32 BA_AVRCP_Start(T_VOID);
T_S32 BA_AVRCP_Stop(T_VOID);
T_S32 BA_AVRCP_Connect(T_BD_ADDR bd_addr);
T_S32 BA_AVRCP_Disconnect(T_VOID);
T_S32 BA_AVRCP_SendKey(BA_AVRCP_KEY nKey, T_U32 nFlags);
T_S32 BA_AVRCP_PressKey(BA_AVRCP_KEY nKey);


#if defined __cplusplus
}
#endif

#endif // _BA_AVRCP_H_


