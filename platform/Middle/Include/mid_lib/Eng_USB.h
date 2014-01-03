#ifndef __ENG_USB_H__
#define __ENG_USB_H__



#define USB_DISK_PARAM          0xFF



typedef enum _USB_Status{
    UDISK_CONNECT=0,
    UDISK_IDLE,
    UDISK_DATAIN,
    UDISK_DATAOUT,
    UDISK_MAX
}T_UDISK_STATUS;


T_VOID USB_ShowStatus(T_UDISK_STATUS status);
T_VOID USB_ShowBat(T_VOID);

T_BOOL usbdisk_main(T_VOID);
T_VOID usb_using_exit_handle(T_VOID);

#endif

