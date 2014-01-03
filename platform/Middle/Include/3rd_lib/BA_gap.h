#ifndef _BA_GAP_H_
#define _BA_GAP_H_

#if defined __cplusplus
extern "C" {
#endif

typedef struct{
    T_U8 BD_ADDR[6];
    T_U8 LinkKey[16];
}T_PAIRING_INFO;

typedef enum{
	INQUIRY_SPEED_POWER_SAVE = 0,//节省功耗
	INQUIRY_SPEED_FOUND_SLOW,//被发现速度慢
	INQUIRY_SPEED_FOUND_FAST,//被发现速度快
}T_INQUIRY_SPEED;

typedef enum{
    //速度等级越高,被寻呼的速度越快，功耗越高
	PAGED_SPEED_LEVEL0,
	PAGED_SPEED_LEVEL1,
	PAGED_SPEED_LEVEL2,
	PAGED_SPEED_LEVEL3,
	PAGED_SPEED_LEVEL4,
	PAGED_SPEED_LEVEL5,
}T_PAGED_SPEED;

typedef enum{
	DEVICE_MODE_DISCOVERABLE = 0x01,
	DEVICE_MODE_CONNECTABLE=0x02,
	DEVICE_MODE_BONDABLE=0x04,
	DEVICE_MODE_BONDING=0x08,
	DEVICE_MODE_SSP_SUPPORT=0x10,
	DEVICE_MODE_ON_INQUIRY = 0x20,
	DEVICE_MODE_ENCRYPT_PENDING = 0x40,
}T_DEVICE_MODE;

T_BOOL BA_GAP_SetDeviceMode(T_U8 mode);
T_BOOL BA_GAP_SSPSwitch(T_BOOL SSP_Enable);
T_BOOL BA_GAP_SetPinCode(T_U8 *PinCode,T_U8 Length);
T_BOOL BA_GAP_SetPinCodeType(T_BOOL bFixPin);
T_BOOL BA_GAP_SetClassOfDevice(T_U32 CoD);
T_BOOL BA_GAP_SetLocalBDAddr(T_BD_ADDR address);
T_BOOL BA_GAP_SetLocalName(const T_U16 *NewName_Unicode);
T_BOOL BA_GAP_GetRemoteName(T_U8 *BD_ADDR);
T_BOOL BA_GAP_SetInquirySpeed(T_INQUIRY_SPEED Imode);
T_BOOL BA_GAP_SetPagedSpeed(T_PAGED_SPEED Pmode);
T_BOOL BA_GAP_SetLinKey_for(T_BD_ADDR DevAddr, T_LINK_KEY Linkey);
T_BOOL BA_GAP_SetSSPDebugMode(T_BOOL SSP_DebugEnable);
T_BOOL BA_GAP_GetAroundDevice(T_U8 time_limit);
T_BOOL BA_GAP_Cancel_Inquiry(T_VOID);
T_BOOL BA_GAP_CancelCreateConnect(T_BD_ADDR cancel_addr);
T_BOOL BA_GAP_ReadBtDev_RSSI(T_VOID);

#if defined __cplusplus
}
#endif

#endif // _BA_GAP_H_

