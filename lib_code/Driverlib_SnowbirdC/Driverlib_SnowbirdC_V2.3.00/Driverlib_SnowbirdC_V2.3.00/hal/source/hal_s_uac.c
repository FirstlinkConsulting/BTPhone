/**
 * @filename hal_s_uvc.c
 * @brief: AK880x how to use slave uvc.
 *
 * This file describe frameworks of usb slave uvc driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-08-05
 * @version 1.0
 */
#include <stdio.h>
#include "usb_slave_drv.h"
#include "hal_usb_s_std.h"
#include "usb_common.h"
#include "hal_s_uac.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "arch_init.h"

#ifdef UAC_MIC_FUN
#define UAC_CFG_SIZE     300
#else
#define UAC_CFG_SIZE     150
#endif
static T_U8 uac_configdata[UAC_CFG_SIZE] = {0};

#pragma arm section zidata = "_usbbss_"
static volatile T_UAC_DEV s_UacDev ;
volatile  T_BOOL  bUacSend;
static volatile T_U32 uac_sample;
static volatile T_U32 uac_mic_sample;
volatile T_BOOL bUacMicSending;
static T_U8 *temp_buf;


#ifdef EXTERN_PCM_PLAYER
#else
static T_U8 *sound_buf = AK_NULL;
volatile T_BOOL  bUacSendPcm = AK_FALSE;
static volatile T_UAC_RESAMPLE s_ResampleCtl;


#endif

#pragma arm section zidata

#define     L2_DMA_SIZE                 4096
#define     LOBYTE(w)                   ((T_U8)(w))
#define     HIBYTE(w)                   ((T_U8)(((T_U16)(w) >> 8) & 0xFF))
#define     SAMPLE_DIF(x,y)             (((x)>(y))?((x)-(y)):((y)-(x)))
static T_VOID uac_dev_msg_proc(T_U32* pMsg,T_U32 len);
static T_BOOL uac_enable();
static T_BOOL uac_disable();
static T_VOID uac_reset(T_U32 mode);
static T_VOID uac_suspend();
static T_VOID uac_resume();
static T_VOID uac_configok();
static T_VOID uac_recv_finish();
static T_VOID uac_hid_tx_finish();
static T_VOID uac_tx_finish(T_VOID);
static T_VOID uac_receive_notify();
static T_SYNCH_STATE uac_buffer_synch(T_U8 *pcnt);
static T_BOOL uac_class_callback(T_CONTROL_TRANS *pTrans);

static T_U8 *uac_get_dev_qualifier_desc(T_U32 *count);
static T_U8 *uac_get_dev_desc(T_U32 *count);
static T_U8 *uac_get_cfg_desc(T_U32 *count);
static T_U8 *uac_get_hid_report_desc(T_U32 *count);

static T_U8 *uac_get_other_speed_cfg_desc(T_U32 *count);
static T_U8 *uac_get_str_desc(T_U8 index, T_U32 *count);

#define     UAC_RX_FRAME_PER            (uac_sample/100)
#define     UAC_RX_PER                  (UAC_RX_FRAME_PER<<2)
#define     UAC_RX_BUF_SIZE             (UAC_RX_PER*10)

#define     MAX_VOLUME                  31
#define     SET_DIG_PER                 0x421
#define     HID_REPORT_SIZE             4
#define     USB_CTRL_SIZE               4096
#define     UAC_RX_BUF_RESERVED         (2*1024)
#define     UAC_SEND_MAX                (s_UacDev.tTrans.len-UAC_RX_BUF_RESERVED)
#define     UAC_GET_MAX                 (16*1024)

#define     UAC_MICRO_FRAME_BUF_SIZE    UAC_RX_BUF_RESERVED


#define     UAC_MAX_READ_LEN            (4*1024+50)


const T_U16 UacTabVolDig[MAX_VOLUME+1] = {
   0, 7, 11, 16, 23, 32, 43, 56, 70, 86,
   104, 124, 146, 169, 189, 216, 245, 250, 260, 320,
   370, 400, 430, 450, 470, 480, 500, 520, 540, 560,
   580, 600 };






const static T_USB_DEVICE_DESCRIPTOR   s_UacDeviceDesc =
{
     0x12,                               // bLength (18)
     0x01,                               // bDescriptorType (DEVICE)
     0x0110,                             // bcdUSB (1.1)
     0x00,                               // bDeviceClass (none)
     0x00,                               // bDeviceSubClass (none)
     0x00,                               // bDeviceProtocol (none)
     64,                                 // bMaxPacketSize0 (64)
     0x0d8c,                             // idVendor
     0x103,                              // idProduct
     0x0010,                             // bcdDevice (1.0)
     0x01,                               // iManufacturer (index 1)
     0x02,                               // iProduct (index 2)
     0x00,                               // iSerialNumber (none)
     0x01                                // bNumConfigurations (1)
};

static T_USB_CONFIGURATION_DESCRIPTOR s_UacConfigDesc =
{
    0x09,                            // bLength (9)
    0x02,                            // bDescriptorType (CONFIGURATION)
    0x00,                             // wTotallength (145)
    NUM_INTERFACE,
    0x01,                            // bConfigurationValue (1)
    0x00,                            // iConfiguration (none)
    0x80,                            // bmAttributes (bus-powered)
    0xFA,                            // bMaxPower (100 mA)
};

const static T_USB_INTERFACE_DESCRIPTOR_AUDIO interface_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    0x00,                            // bInterfaceNumber (0)
    0x00,                            // bAlternateSetting (none)
    0x00,                            // bNumEndpoints (none)
    0x01,                            // bInterfaceClass (AUDIO)
    0x01,                            // bInterfaceSubClass (AUDIO_CONTROL)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)

};

const static T_USB_AUDIO_HEAD_DESC interface_head_descriptor =
{// Audio control interface
    INTERFACE_HEAD_LEN,              // bLength (9)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x01,                            // bDescriptorSubtype (HEADER)
    0x100,                           // bcdADC (1.0)
    #ifdef UAC_MIC_FUN
    (sizeof(T_USB_AUDIO_HEAD_DESC)+   // wTotalLength (43)
    sizeof(T_USB_AUDIO_IT_DESC)+
    sizeof(T_USB_AUDIO_FEATURE_DESC)+
    sizeof(T_USB_AUDIO_OT_DESC))*2,
    #else
    sizeof(T_USB_AUDIO_HEAD_DESC)+   // wTotalLength (43)
    sizeof(T_USB_AUDIO_IT_DESC)+
    sizeof(T_USB_AUDIO_FEATURE_DESC)+
    sizeof(T_USB_AUDIO_OT_DESC),
    #endif
    #ifdef UAC_MIC_FUN
    0x02,                            // bInCollection (1 streaming interface)
    0x02,                             // baInterfaceNr (interface 1 is stream)
    0x01                             // baInterfaceNr (interface 1 is stream)
    #else
    0x01,
    0x01
    #endif
};

#ifdef UAC_MIC_FUN
const static T_USB_AUDIO_IT_DESC input_mic_terminal_descriptor =
{// Audio control interface
    0x0C,                            // bLength (12)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x02,                            // bDescriptorSubtype (INPUT_TERMINAL)
    0x01,                            // bTerminalID (1)
    0x0201,                          // wTerminalType (radio SEND)
    0x00,                            // bAssocTerminal (none)
    0x02,                            // bNrChannels (2)
    0x0003,                          // wChannelConfig (left, right)
    0x00,                            // iChannelNames (none)
    0x00                             // iTerminal (none)
};

const static T_USB_AUDIO_FEATURE_DESC input_mic_feature_descriptor =
{// Audio control interface
    0x0D,                            // bLength (13)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x06,                            // bDescriptorSubtype (FEATURE_UNIT)
    0x02,                            // bUnitID (2)
    0x01,                            // bSourceID (input terminal 1)
    0x02,                            // bControlSize (2 bytes)
    0x0201,                          // Master controls
    0x0002,                          // Channel 0 controls
    0x0002,                          // Channel 1 controls
    0x00                             // iFeature (none)
};


const static T_USB_AUDIO_OT_DESC output_mic_terminal_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x03,                            // bDescriptorSubtype (OUTPUT_TERMINAL)
    0x03,                            // bTerminalID (3)
    0x0101,                          // wTerminalType (USB streaming)
    0x00,                            // bAssocTerminal (none)
    0x02,                            // bSourceID (feature unit 2)
    0x00                             // iTerminal (none)
};
#endif

const static T_USB_AUDIO_IT_DESC input_terminal_descriptor =
{// Audio control interface
    0x0C,                            // bLength (12)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x02,                            // bDescriptorSubtype (INPUT_TERMINAL)
    SPEAKER_IT_ID,                   // bTerminalID (1)
    0x0101,                          // wTerminalType (radio receiver)
    0x00,                            // bAssocTerminal (none)
    0x02,                            // bNrChannels (2)
    0x0003,                          // wChannelConfig (left, right)
    0x00,                            // iChannelNames (none)
    0x00                             // iTerminal (none)
};

const static T_USB_AUDIO_FEATURE_DESC input_feature_descriptor =
{// Audio control interface
    0x0D,                            // bLength (13)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x06,                            // bDescriptorSubtype (FEATURE_UNIT)
    SPEAKER_FU_ID,                   // bUnitID (2)
    SPEAKER_IT_ID,                   // bSourceID (input terminal 1)
    0x02,                            // bControlSize (2 bytes)
    0x0201,                          // Master controls
    0x0002,                          // Channel 0 controls
    0x0002,                          // Channel 1 controls
    0x00                             // iFeature (none)
};

const static T_USB_AUDIO_OT_DESC output_terminal_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x03,                            // bDescriptorSubtype (OUTPUT_TERMINAL)
    SPEAKER_OT_ID,                   // bTerminalID (3)
    0x0301,                          // wTerminalType (USB streaming)
    0x00,                            // bAssocTerminal (none)
    SPEAKER_FU_ID,                   // bSourceID (feature unit 2)
    0x00                             // iTerminal (none)
};
#ifdef UAC_MIC_FUN
const static T_USB_INTERFACE_DESCRIPTOR_AUDIO output_bandwidth_mic_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    0x01,                            // bInterfaceNumber (1)
    0x00,                            // bAlternateSetting (0)
    0x00,                            // bNumEndpoints (0)
    0x01,                            // bInterfaceClass (AUDIO)
    0x02,                            // bInterfaceSubClass (AUDIO_STREAMING)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)
};

const static T_USB_INTERFACE_DESCRIPTOR_AUDIO output_streaming_mic_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    0x01,                            // bInterfaceNumber (1)
    0x01,                            // bAlternateSetting (1)
    0x01,                            // bNumEndpoints (1)
    0x01,                            // bInterfaceClass (AUDIO)
    0x02,                            // bInterfaceSubClass (AUDIO_STREAMING)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)

};

const static T_USB_AUDIO_CLASS_SPEC_DESC class_specific_mic_descriptor =
{// Audio control interface
    0x07,                            // bLength (7)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x01,                            // bDescriptorSubtype (AS_GENERAL)
    0x03,                            // bTerminalLink (terminal 3)
    0x01,                            // bDelay (none)
    0x0001                           // wFormatTag (PCM format)
};
#endif

const static T_USB_INTERFACE_DESCRIPTOR_AUDIO output_bandwidth_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    SPEAKER_INTERFACE_ID,            // bInterfaceNumber (1)
    0x00,                            // bAlternateSetting (0)
    0x00,                            // bNumEndpoints (0)
    0x01,                            // bInterfaceClass (AUDIO)
    0x02,                            // bInterfaceSubClass (AUDIO_STREAMING)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)
};

const static T_USB_INTERFACE_DESCRIPTOR_AUDIO output_streaming_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    SPEAKER_INTERFACE_ID,            // bInterfaceNumber (1)
    0x01,                            // bAlternateSetting (1)
    0x01,                            // bNumEndpoints (1)
    0x01,                            // bInterfaceClass (AUDIO)
    0x02,                            // bInterfaceSubClass (AUDIO_STREAMING)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)

};

const static T_USB_AUDIO_CLASS_SPEC_DESC class_specific_descriptor =
{// Audio control interface
    0x07,                            // bLength (7)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x01,                            // bDescriptorSubtype (AS_GENERAL)
    SPEAKER_IT_ID,                   // bTerminalLink (terminal 3)
    0x01,                            // bDelay (none)
    0x0001                           // wFormatTag (PCM format)
};

const static T_USB_AUDIO_TYPE_DESC type_format_descriptor =
{// Audio control interface
    0x0E,                            // bLength (11)
    0x24,                            // bDescriptorType (CS_INTERFACE)
    0x02,                            // bDescriptorSubtype (FORMAT_TYPE)
    0x01,                            // bFormatType (TYPE_I)
    0x02,                            // bNrChannels (2)
    0x02,                            // bSubFrameSize (2)
    // The next field should be 10, but 16 works with more standard software
    0x10,                            // bBitResolution (16)
    0x02,                            // bSamFreqType (1 sampling frequency)
    0x44,                            // 48,000 Hz (byte 0)
    0xAC,                            // 48,000 Hz (byte 1)
    0x00,                             // 48,000 Hz (byte 2)
    0x80,                            // 48,000 Hz (byte 0)
    0xBB,                            // 48,000 Hz (byte 1)
    0x00                             // 48,000 Hz (byte 2)

};

#ifdef UAC_MIC_FUN
static T_USB_AUDIO_ENDPOINT_DESC iso_mic_endpoint_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x05,                            // bDescriptorType (ENDPOINT)
    0x82,                            // bEndpointAddress (EP3 in)
    0x05,                            // bmAttributes (asynchronous)
    0x00c0,                          // wMaxPacketSize (512)
    0x01,                            // bInterval (1 millisecond)
    0x00,                            // bRefresh (0)
    0x00                             // bSynchAddress (no synchronization)
};
#endif

static T_USB_AUDIO_ENDPOINT_DESC iso_endpoint_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x05,                            // bDescriptorType (ENDPOINT)
    0x03,                            // bEndpointAddress (EP3 in)
    0x09,                            // bmAttributes (asynchronous)
    0x00c0,                          // wMaxPacketSize (512)
    0x01,                            // bInterval (1 millisecond)
    0x00,                            // bRefresh (0)
    0x00                             // bSynchAddress (no synchronization)
};

const static T_USB_AUDIO_SUBTYPE_DESC iso_endpoint_subtype_descriptor =
{// Audio control interface
    0x07,                            // bLength (7)
    0x25,                            // bDescriptorType (CS_ENDPOINT)
    0x01,                            // bDescriptorSubtype (EP_GENERAL)
    0x01,                            // bmAttributes (none)
    0x00,                            // bLockDelayUnits (PCM samples)
    0x0000                           // wLockDelay (0)
};

const static T_USB_INTERFACE_DESCRIPTOR_AUDIO hid_interface_descriptor =
{// Audio control interface
    0x09,                            // bLength (9)
    0x04,                            // bDescriptorType (INTERFACE)
    #ifdef UAC_MIC_FUN
    0x03,                            // bInterfaceNumber (1)
    #else
    0x02,                            // bInterfaceNumber (1)
    #endif
    0x00,                            // bAlternateSetting (1)
    0x01,                            // bNumEndpoints (1)
    0x03,                            // bInterfaceClass (AUDIO)
    0x00,                            // bInterfaceSubClass (AUDIO_STREAMING)
    0x00,                            // bInterfaceProtocol (none)
    0x00                             // iInterface (none)

};
const static T_USB_HID_EPDESC hid_report_descriptor =
{
    0x09,                            // bLength (9)
    0x21,                            // bDescriptorType (HID_DESCRIPTOR)
#ifdef UAC_HID_EX
    0x0110,                          // bcdHID (1.11)
#else
    0x0100,
#endif
    0x00,                            // bCountryCode (none)
    0x01,                            // bNumDescriptors (1 class descriptor)
    0x22,                            // bClassDescriptorType (report descr.)
#ifdef UAC_HID_EX
    0x0043                            // wDescriptorLength (203)
#else
    0x003c
#endif

};

const static T_USB_AUDIO_EPDESC intr_endpoint_descriptor =
{// Audio control interface
    0x07,                            // bLength (7)
    0x05,                            // bDescriptorType (CS_ENDPOINT)
    0x81,                            // bDescriptorSubtype (EP_GENERAL)
    0x03,                            // bmAttributes (intr)
    0x0004,                          // bLockDelayUnits (PCM samples)
    0x02                             // wLockDelay (0)
};


static const T_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR s_UacOtherSpeedConfigDesc=
{
    0x9,                                        ///<Descriptor size is 9 bytes
    OTHER_SPEED_CONFIGURATION_DESC_TYPE,        ///<CONFIGURATION Descriptor Type
    0,                                          ///<The total length of data for this configuration is define when init. This includes the combined length of all the descriptors
    1,                                          ///<This configuration supports 2 interfaces
    0x01,                                       ///< The value 1 should be used to select this configuration
    0x00,                                       ///<The device doesn't have the string descriptor describing this configuration
    0x80,                                       ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0
    0xFA                                        ///<Maximum power consumption of the device in this configuration is 500 mA
};

const static T_USB_DEVICE_QUALIFIER_DESCRIPTOR s_UacDeviceQualifierDesc=
{
    sizeof(T_USB_DEVICE_QUALIFIER_DESCRIPTOR),      ///<Descriptor size is 10 bytes
    DEVICE_QUALIFIER_DESC_TYPE,                     ///<DEVICE_QUALIFIER Descriptor Type
    0x0110,                                         ///<  USB Specification version 2.00
    0x00,                                           ///<The device belongs to the Miscellaneous Device Class
    0x00,                                           ///<The device belongs to the Common Class Subclass
    0x00,                                           ///<The device uses the Interface Association Descriptor Protocol
    EP0_MAX_PAK_SIZE,                               ///<Maximum packet size for endpoint zero is 64
    0x01,                                           ///<The device has 1 possible other-speed configurations
    0                                               ///<Reserved for future use
};

/* language descriptor */
const static T_U8 s_UacString0[] =
{
    4,              ///<Descriptor size is 4 bytes
    3,              ///< Second Byte of this descriptor
    0x09, 0x04,     ///<Language Id: 1033
};

/*string descriptor*/
const static T_U8 s_UacString1[] =
{
    12,
    0x03,
    'A',0,
    'N',0,
    'Y',0,
    'K',0,
    'A',0
};

#define STR2LEN sizeof("USB Audio")*2
const static T_U8 s_UacString2[] =
{
    STR2LEN, 0x03,
    'U', 0,
    'S', 0,
    'B', 0,
    ' ', 0,
    'A', 0,
    'u', 0,
    'd', 0,
    'i', 0,
    'o', 0

};

const static T_U8 s_UacString3[] =
{
    12,
    0x03,
    '1',0,
    '2',0,
    '3',0,
    '4',0,
    '5',0
};


//报表的项目有Main、Global和Local三大类
const T_U8 HidReportDesc[] =
{
    //开一个集合，定义用法
#ifndef UAC_HID_EX
   0x05, 0x0c, 0x09,0x01, 0xa1, 0x01, 0x15, 0x00, // USAGE_PAGE (Vendor Defined Page 1)
   0x25, 0x01, 0x09,0xe9, 0x09, 0xea, 0x75, 0x01, // USAGE (Vendor Usage 1)
   0x95, 0x02, 0x81,0x02, 0x09, 0xe2, 0x09, 0x00, // COLLECTION (Application)
   0x81, 0x06, 0x05,0x0b, 0x09, 0x20, 0x95, 0x01, // END_COLLECTION
   0x81, 0x42, 0x05,0x0c, 0x09, 0x00, 0x95, 0x03,
   0x81, 0x02, 0x26,0xff, 0x00, 0x09, 0x00, 0x75,
   0x08, 0x95, 0x03,0x81, 0x02, 0x09, 0x00, 0x95,
   0x04, 0x91, 0x02,
   0xc0  //关闭集合
#else
    0x05,0x0C,// Usage Page (Consumer)
    0x09,0x01,// Usage(Consumer Control)
    0xA1,0x01,// Collection(Application )
    0x15,0x00,// Logical Minimum(0x0 )
    0x25,0x01,// Logical Maximum(0x1 )
    0x09,0xE2,// Usage(Mute)
    0x09,0xE9,// Usage(Volume Increment)
    0x09,0xEA,// Usage(Volume Decrement)
    0x75,0x01,// Report Size(0x1 )
    0x95,0x03,// Report Count(0x3 )
    0x81,0x02,// Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
    0x75,0x01,// Report Size(0x1 )
    0x95,0x05,// Report Count(0x5 )
    0x81,0x03,// Input(Constant, Variable, Absolute)
    0x09,0xB0,// Usage(Play)
    0x09,0xB7,// Usage(Stop)
    0x09,0xB1,// Usage(Pause)
    0x09,0xB5,// Usage(Scan Next Track)
    0x09,0xB6,// Usage(Scan Previous Track)
    0x09,0xB2,//
    0x09,0xB4,//
    0x09,0xB3,//
    0x75,0x01,// Report Size(0x1 )
    0x95,0x08,// Report Count(0x8 )
    0x81,0x02,// Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
    0x09,0x00,// Usage(Undefined)
    0x75,0x08,// Report Size(0x8 )
    0x95,0x06,// Report Count(0x6 )
    0x81,0x02,// Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
    0x09,0x00,// Usage(Undefined)
    0x75,0x08,// Report Size(0x8 )
    0x95,0x08,// Report Count(0x8 )
    0x91,0x00,
    0xC0
#endif
};

static T_U8 uac_ctrl_id_map(T_U8 param)
{
    T_U8 tmp;

    tmp = (param & 0x0f);

    return tmp;
}

/**
* @brief    USB Audio Class setup the advanced control features
*
* Implemented controls,such as volume,called after uac_init successful
* @author LiuHuadong
* @date 2012-05-13
* @param dwControl[in] The advanced feature  control selector
* @param ulMin[in] The min value
* @param ulMax[in] The max value
* @param ulDef[in] The  def value
* @param ulResf[in] The res value
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TRUE means successful
*/
T_BOOL uac_set_ctrl(T_eUAC_CONTROL dwControl, T_U32 ulMin, T_U32 ulMax, T_U32 ulDef,T_U32 ulRes)
{
    //  Check if the min and max are correct.
    if (ulMin >= ulMax)
    {
        return AK_FALSE;
    }
    //  The default value should between min and max.
    if (ulDef < ulMin || ulDef > ulMax)
    {
        return AK_FALSE;
    }
    switch (dwControl)
    {
        //uac device brightness control
        case VOLUME_CONTROL:
            s_UacDev.tCtrlSetting[dwControl].ulLen = 0x2;
            printf("dwControl:%x\n",dwControl);
            break;
        //uac device contrast control
        case MUTE_CONTROL:
            s_UacDev.tCtrlSetting[dwControl].ulLen = 0x1;
            printf("dwControl:%x\n",dwControl);
            break;
        //uac device saturation control
        case LOUDNESS_CONTROL:
            s_UacDev.tCtrlSetting[dwControl].ulLen = 0x1;
            printf("dwControl:%x\n",dwControl);
            break;
        default:
            drv_print("unsupported ctrl!\n", 0, AK_FALSE);
            return AK_FALSE;
    }
    s_UacDev.tCtrlSetting[dwControl].ulMin = ulMin;
    s_UacDev.tCtrlSetting[dwControl].ulMax = ulMax;
    s_UacDev.tCtrlSetting[dwControl].ulCur = ulDef;
    s_UacDev.tCtrlSetting[dwControl].ulRes = ulRes;

    return AK_TRUE;
}

static T_VOID uac_volume_handle(T_U8 *data, T_U32 len)
{
    T_U32 i;
    T_S16 pdata = 0;
    T_U32 tmp;

    tmp =  (s_UacDev.tCtrlSetting[VOLUME_CONTROL].ulCur&0x7fff)/SET_DIG_PER;
    if (s_UacDev.tCtrlSetting[MUTE_CONTROL].ulCur&0x0f)
    {
        memset(data,0,len);
        return;
    }

    for(i = 0; i<len; i+=2)
    {
        pdata = (data[i+1]<<8)|data[i];
        pdata = (pdata * UacTabVolDig[tmp])/1024;
        data[i+1] = HIBYTE(pdata);
        data[i] = LOBYTE(pdata);
    }

}


static T_VOID uac_dev_msg_proc(T_U32* pMsg,T_U32 len)
{
    T_U32 tmp = 0;
    T_U32 cnt = 0;
    T_U32 tmp_len = 0;
    T_U32 size = 0;
    T_U32 tmp_data = 0;

    T_pUAC_DEV_MSG message = (T_pUAC_DEV_MSG)pMsg;

    switch(message->ucMsgId)
    {
        case MSG_PCM_SENT:
#ifdef EXTERN_PCM_PLAYER
        if (s_UacDev.fPCM_POST_CB)
            s_UacDev.fPCM_POST_CB(uac_sample);
#else
        uac_send_pcm_data();
#endif
        break;
    case MSG_AC_CTRL:

        if (HIBYTE(message->ulParam4))
        {
            switch(HIBYTE(message->ulParam4))
            {
            case SPEAKER_FU_ID:
            #ifdef UAC_MIC_FUN
            case MIC_FU_ID:
            #endif
                s_UacDev.tCtrlSetting[message->ucParam2].ulCur= message->ulParam3&0xffff;
                if (AK_NULL != s_UacDev.fAcCtrlCallBack)
                    s_UacDev.fAcCtrlCallBack(message->ucParam2,RECIPIENT_TYPE_ENTITY_FU_ID,message->ulParam3&0xffff);
                break;
            default:
                drv_print("Not Support CtrlID:",message->ucParam1,1);
                break;
            }
        }
        else if(SPEAKER_ENDPOINT_ID == LOBYTE(message->ulParam4))//judge whether endpoint
        {
            switch (message->ucParam2)
            {
            case SAMPLING_FREQ_CONTROL:
                message->ulParam3 = message->ulParam3&0xffffff;
                uac_sample = message->ulParam3;
                #ifdef EXTERN_PCM_PLAYER
                if (s_UacDev.fSR_SET_CB)
                    s_UacDev.fSR_SET_CB(uac_sample);
                #endif
                break;
            default:
                break;
            }

            if (AK_NULL != s_UacDev.fAcCtrlCallBack)
                s_UacDev.fAcCtrlCallBack(message->ucParam2,RECIPIENT_TYPE_ENDPOINT_ID,message->ulParam3);
        }
        #ifdef UAC_MIC_FUN
        else if(MIC_ENDPOINT_ID == LOBYTE(message->ulParam4))//judge whether endpoint
        {
            switch (message->ucParam2)
            {
            case SAMPLING_FREQ_CONTROL:
                message->ulParam3 = message->ulParam3&0xffffff;
                uac_mic_sample = message->ulParam3;
                drv_print("mic sample",uac_mic_sample,1);
                break;
            default:
                break;
            }

            if (AK_NULL != s_UacDev.fAcCtrlCallBack)
                s_UacDev.fAcCtrlCallBack(message->ucParam2,RECIPIENT_TYPE_ENDPOINT_ID,message->ulParam3);
        }
        #endif
        else
        {
            drv_print("Not Support RECIPIENT",LOBYTE(message->ulParam4),1);
        }
        break;
    }
}

/**
* @brief Initialize uac descriptor
* @author LiuHuadong
* @date 2012-05-10
* @param mode[in] usb2.0 or usb1.0
* @param format[in] The UAC data format
* @return T_BOOL
* @retval  AK_FALSE means failed
* @retval  AK_TRUE means successful
*/

T_BOOL uac_init(T_U32 mode,T_U32 size)
{
    T_U8 i;
    T_U8* pBuffer = AK_NULL;

    //init s_pUacDev member
    memset(&s_UacDev,0,sizeof(T_UAC_DEV));
    s_UacDev.ulMode = mode;

#ifdef EXTERN_PCM_PLAYER
    s_UacDev.mFrameBuf = (T_U8*)drv_malloc(UAC_MICRO_FRAME_BUF_SIZE);
    if (AK_NULL == s_UacDev.mFrameBuf)
    {
        drv_print("mFrameBuf alloc failed:", 0, AK_TRUE);
        return AK_FALSE;
    }
#else
    //advice size for 50*1024
    if (size < L2_DMA_SIZE)
    {
        drv_print("size:", size, AK_TRUE);
        return AK_FALSE;
    }
    memset(&s_ResampleCtl,0,sizeof(T_UAC_RESAMPLE));
    pBuffer = (T_U8 *)drv_malloc(size);
    s_ResampleCtl.buf = (T_U8 *)drv_malloc(UAC_GET_MAX);

    sound_buf = (T_U8 *)drv_malloc(L2_DMA_SIZE);
    temp_buf = (T_U8 *)drv_malloc(5*1024);//resample

    //check ram apply success
    if ((AK_NULL == pBuffer) || (AK_NULL == sound_buf) || (AK_NULL == s_ResampleCtl.buf)
        || (AK_NULL == temp_buf))
    {
        drv_print("pBuffer,alloc failed:", pBuffer, AK_TRUE);
        drv_print("pSoundbuf,alloc failed:", sound_buf, AK_TRUE);
        return AK_FALSE;
    }

    s_ResampleCtl.len   = UAC_GET_MAX;
    s_UacDev.tTrans.buf = pBuffer;
    s_UacDev.tTrans.len = size;
#endif

#if USB_VAR_MALLOC > 0
    DrvModule_Init();
#endif
    DrvModule_Map_Message(DRV_MODULE_UAC, UAC_DEV_MSG, uac_dev_msg_proc);

    return AK_TRUE;
}

/**
* @brief    Start UAC
* @author LiuHuadong
* @date 2012-05-04
* @return T_BOOL
*/
T_BOOL uac_start(T_VOID)
{
    //create task
    if (!DrvModule_Create_Task(DRV_MODULE_UAC))
        return AK_FALSE;
    if(!uac_enable())
    {
        DrvModule_Terminate_Task(DRV_MODULE_UAC);
        return AK_FALSE;
    }
    return AK_TRUE;
}

/**
* @brief stop UAC
* @author LiuHuadong
* @date 2012-05-4
* @return VOID
*/
T_VOID uac_stop(T_VOID)
{
    uac_disable();
    DrvModule_Terminate_Task(DRV_MODULE_UAC);
#ifdef EXTERN_PCM_PLAYER
    if (AK_NULL != s_UacDev.mFrameBuf)
    {
        drv_free(s_UacDev.mFrameBuf);
        s_UacDev.mFrameBuf = AK_NULL;
    }
#else
    if (AK_NULL != s_UacDev.tTrans.buf)
    {
        drv_free(s_UacDev.tTrans.buf);
        s_UacDev.tTrans.buf = AK_NULL;
    }
    if (AK_NULL != s_ResampleCtl.buf)
    {
        drv_free(s_ResampleCtl.buf);
        s_ResampleCtl.buf = AK_NULL;
    }
    if (AK_NULL != sound_buf)
    {
        drv_free(sound_buf);
        sound_buf = AK_NULL;
    }
    if (AK_NULL != temp_buf)
    {
        drv_free(temp_buf);
        temp_buf = AK_NULL;
    }
#endif

}

/**
 * @brief   uac enable
 *
 * called by uac_start() when start uac task successful
 * @author LiuHuadong
 * @date 2012-05-04
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL uac_enable(T_VOID)
{
    T_U8* ptmpbuf = AK_NULL;

    Usb_Slave_Standard.usb_get_device_descriptor = uac_get_dev_desc;
    Usb_Slave_Standard.usb_get_config_descriptor = uac_get_cfg_desc;
    Usb_Slave_Standard.usb_get_hid_report_descriptor = uac_get_hid_report_desc;
    Usb_Slave_Standard.usb_get_string_descriptor = uac_get_str_desc;
    Usb_Slave_Standard.usb_get_device_qualifier_descriptor = uac_get_dev_qualifier_desc;
    Usb_Slave_Standard.usb_get_other_speed_config_descriptor = uac_get_other_speed_cfg_desc;
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;

    ptmpbuf = (T_U8 *)drv_malloc(USB_CTRL_SIZE);
    if (AK_NULL == ptmpbuf)
    {
        drv_print("ptmpbuf,alloc failed:", (T_U32)ptmpbuf, AK_TRUE);
        return AK_FALSE;
    }

    Usb_Slave_Standard.Buffer  =            ptmpbuf;
    Usb_Slave_Standard.buf_len =            USB_CTRL_SIZE;

    //usb slave init,alloc L2 buffer,register irq
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    //usb std init,set ctrl callback
    usb_slave_std_init();
    //set class req callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, uac_class_callback);
    usb_slave_set_callback(uac_reset,uac_suspend,uac_resume, uac_configok);
    #ifdef EXTERN_PCM_PLAYER
    usb_slave_set_rx_callback(USB_ISO_OUT_INDEX, uac_receive_notify, AK_NULL);
    #else
    usb_slave_set_rx_callback(USB_ISO_OUT_INDEX, uac_receive_notify, uac_recv_finish);
    #endif
    usb_slave_set_tx_callback(USB_INTR_IN_INDEX, uac_hid_tx_finish);
    #ifdef UAC_MIC_FUN
    usb_slave_set_tx_callback(USB_ISO_IN_INDEX, uac_tx_finish);
    #endif
    usb_slave_device_enable(s_UacDev.ulMode);

    drv_print("uac enable ok\n", 0, AK_FALSE);
    return AK_TRUE;
}

T_BOOL uac_proc(T_VOID)
{
    return DrvModule_Retrieve_Message(DRV_MODULE_UAC, UAC_DEV_MSG);
}

#ifdef UAC_MIC_FUN
static T_VOID uac_tx_finish(T_VOID)
{
    bUacMicSending = AK_FALSE;
}

T_BOOL uac_send(T_U8 *data_buf, T_U32 length)
{
    T_U32 ret = 0;

    if (AK_NULL == data_buf)
    {
        return AK_FALSE;
    }
    if (bUacMicSending)
    {
        return AK_FALSE;
    }

    bUacMicSending = AK_TRUE;
    ret = usb_slave_data_in(USB_ISO_IN_INDEX, data_buf, length);

    if(ret == 0)
        return AK_FALSE;

    return AK_TRUE;

}
#endif

/**
 * @brief   uac disable
 *
 * called by uac_stop() when terminate uac task successful
 * @author LiuHuadong
 * @date 2012-05-04
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL uac_disable(T_VOID)
{
    usb_slave_device_disable();

    drv_free(Usb_Slave_Standard.Buffer);
    memset(&Usb_Slave_Standard,0,sizeof(Usb_Slave_Standard));
    usb_slave_free();

    //clear  callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, AK_NULL);
    usb_slave_set_callback(AK_NULL,AK_NULL,AK_NULL, AK_NULL);
    usb_slave_set_tx_callback(USB_ISO_OUT_INDEX, AK_NULL);
    return AK_TRUE;
}


/**
 * @brief  reset callback
 *
 * called  when usb reset
 * @author LiuHuadong
 * @date 2012-05-04
 * @param mode[in] usb1.1 or usb2.0
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_VOID uac_reset(T_U32 mode)
{
    T_U16 cnt = 0;

    //clr stall status to avoid die when uac task is waiting clr stall to send csw
    usb_slave_set_ep_status(USB_ISO_OUT_INDEX,0);
    cnt = s_UacConfigDesc.wTotalLength;
    if(mode == USB_MODE_20)
    {
        iso_endpoint_descriptor.wMaxPacketSize = 0xc0;
    }
    else
    {
        iso_endpoint_descriptor.wMaxPacketSize = 0xc0;
    }

    usb_slave_set_ep(USB_ISO_OUT_INDEX,EP_ATTRIBUTE_SYNCH,USB_EP_OUT_TYPE,512);
#ifdef UAC_MIC_FUN
    usb_slave_set_ep(USB_ISO_IN_INDEX,EP_ATTRIBUTE_SYNCH,USB_EP_IN_TYPE,512);
#endif
    usb_slave_set_ep(USB_INTR_IN_INDEX,EP_ATTRIBUTE_INTR,USB_EP_IN_TYPE,64);
    //reinit s_pUacDev member,usb1.1 or usb2.0
    s_UacDev.ulMode = mode;
    drv_print("reset ok",0,1);


}

/**
 * @brief  suspend callback
 *
 * called  when usb suspend
 * @author LiuHuadong
 * @date 2012-05-04
 * @return  T_VOID
 */
static T_VOID uac_suspend(T_VOID)
{
    drv_print("uac suspend \n", 0, AK_FALSE);

}

/**
 * @brief  resume callback
 *
 * called  when usb resume
 * @author LiuHuadong
 * @date 2012-05-04
 * @return  T_VOID
 */
static T_VOID uac_resume(T_VOID)
{
    drv_print("uac resume \n", 0, AK_FALSE);
}

/**
 * @brief config ok callback
 *
 * called  when enum successful
 * @author Huang Xin
 * @date 2012-05-04
 * @return  T_VOID
 */
static T_VOID uac_configok(T_VOID)
{
    drv_print("uac ok!\n", 0, AK_FALSE);
}


static T_VOID uac_receive_notify(T_VOID)
{
#ifdef EXTERN_PCM_PLAYER
    T_U32 rec_len;
    static T_U32 cnt=0;

    rec_len = usb_slave_data_out(USB_ISO_OUT_INDEX, s_UacDev.mFrameBuf+cnt*200, 0);
    if (AK_NULL != s_UacDev.fPCM_SETBUF_CB)
    {
        T_UAC_DEV_MSG msg;
        //master volume control
        uac_volume_handle(s_UacDev.mFrameBuf, rec_len);
        //保存PCM数据。
        s_UacDev.fPCM_SETBUF_CB((T_U32 *)s_UacDev.mFrameBuf, rec_len);

        //由外面应用判断是否有足够数据。
        #if 0//每个数据包发消息太过频繁，平台应用处理不及时容易丢消息。
        msg.ucMsgId = MSG_PCM_SENT;
        if (!DrvModule_Send_Message(DRV_MODULE_UAC, UAC_DEV_MSG, (T_U32 *)&msg))
        {
            drv_print("send uac msg failed\n",0,1);
        }
        #endif
    }
#else
    usb_slave_data_out(USB_ISO_OUT_INDEX, s_UacDev.tTrans.buf+s_UacDev.tTrans.wr, UAC_RX_PER);
#endif

}
static T_BOOL uac_judge_data_ov(T_VOID);

static T_VOID uac_recv_finish()
{
#ifdef EXTERN_PCM_PLAYER
    {
        T_UAC_DEV_MSG msg;

        msg.ucMsgId = MSG_PCM_SENT;
        if (!DrvModule_Send_Message(DRV_MODULE_UAC, UAC_DEV_MSG, (T_U32 *)&msg))
        {
            drv_print("send uac msg failed\n",0,1);
        }
    }
#else

    if (AK_FALSE == uac_judge_data_ov())
    {
        drv_print("", 'e', AK_TRUE);
    }

    uac_volume_handle(s_UacDev.tTrans.buf+s_UacDev.tTrans.wr,UAC_RX_PER);

    s_UacDev.tTrans.wr += UAC_RX_PER;

    if (s_UacDev.tTrans.wr >= UAC_SEND_MAX)
    {

        s_UacDev.tTrans.wr = s_UacDev.tTrans.wr-UAC_SEND_MAX;
        memcpy(s_UacDev.tTrans.buf,s_UacDev.tTrans.buf+UAC_SEND_MAX,s_UacDev.tTrans.wr);
    }
#endif
}


T_BOOL uac_volume_set_func(T_eUAC_VOLCTRL type)
{

    T_U8 data[4]={0};

    if (AK_TRUE == bUacSend)
    {
        return AK_FALSE;
    }

    if (UAC_VOLUME_CTRL_DEC == type)
    {
        data[0]=DEC_BUTTON;
    }
    else if (UAC_VOLUME_CTRL_INC == type)
    {
        data[0]=INC_BUTTON;
    }
    #ifdef UAC_HID_EX
    else if (UAC_VOLUME_CTRL_MUTE == type)
    {
        data[0]=MUTE_BUTTON;
    }
    else if (UAC_VOLUME_CTRL_PRE == type)
    {
        data[1]=PRE_BUTTON;
    }
    else if (UAC_VOLUME_CTRL_NEXT == type)
    {
        data[1]=NEXT_BUTTON;
    }
    //和播放器有关系的。目前在windows media play测试正常。
    else if (UAC_VOLUME_CTRL_PLAY == type)
    {
        data[1]=PLAY_BUTTON;
    }
    //和播放器有关系的。目前在windows media play测试正常。
    else if (UAC_VOLUME_CTRL_PAUSE == type)
    {
        data[1]=PAUSE_BUTTON;
    }
    #endif
    else if (UAC_VOLUME_CTRL_RELEASE == type)
    {
        data[0]=RELEASE_BUTTON;
    }
    else
    {
        return AK_FALSE;
    }

    bUacSend = AK_TRUE;
    usb_slave_start_send(USB_INTR_IN_INDEX);
    usb_slave_data_in(USB_INTR_IN_INDEX, data, HID_REPORT_SIZE);

    return AK_TRUE;

}

static T_VOID uac_hid_tx_finish(T_VOID)
{
    bUacSend = AK_FALSE;
}
#ifdef EXTERN_PCM_PLAYER
T_VOID uac_pcm_set_callback(T_pUAC_PCM_SETBUF_CB fGetBuf_cb, T_pUAC_PCM_POST_CB fPost_cb)
{
    s_UacDev.fPCM_SETBUF_CB = fGetBuf_cb;
    s_UacDev.fPCM_POST_CB= fPost_cb;
}


T_VOID uac_sr_set_callback(T_pUAC_SET_SR_CB fSR_set_cb)
{
    s_UacDev.fSR_SET_CB = fSR_set_cb;
}

#else

T_BOOL uac_get_pcm_data(T_pVOID pfilter)
{
    T_U32 tmp = 0;
    T_U32 cnt = 0;
    T_U32 tmp_len = s_UacDev.tTrans.wr;
    T_U32 size = 0;
    T_S32 processlen;
    T_UAC_DEV_MSG msg;
    T_U32 tmp_data = 0;
    static T_U32 bufcnt=0;
    T_AUDIO_FILTER_BUF_STRC fbuf_strc;

    if (AK_NULL == pfilter)
    {
        drv_print("pfilter is null",0,1);
        return AK_FALSE;
    }

    if (s_ResampleCtl.wr < s_ResampleCtl.rd)
    {
        if (s_ResampleCtl.wr + UAC_MAX_READ_LEN >= s_ResampleCtl.rd)
        {

            return AK_FALSE;
        }
    }
    else if ((s_ResampleCtl.rd+(s_ResampleCtl.len-s_ResampleCtl.wr)<= UAC_MAX_READ_LEN))
    {

        return AK_FALSE;
    }

    if ((tmp_len >= s_UacDev.tTrans.rd))
    {

        tmp = tmp_len - s_UacDev.tTrans.rd;
        cnt = tmp/512;
        if (0 == cnt)
        {
            return AK_FALSE;
        }
        if (cnt>8)
            cnt=8;

        size = cnt*512;
        memcpy(temp_buf ,s_UacDev.tTrans.buf + s_UacDev.tTrans.rd,size);
        s_UacDev.tTrans.rd += size;

    }
    else if((tmp_len < s_UacDev.tTrans.rd))
    {
        tmp = UAC_SEND_MAX - s_UacDev.tTrans.rd + tmp_len;
        cnt = tmp / 512;
        if (0 == cnt)
        {
            return AK_FALSE;
        }

        if (cnt > 8)
            cnt = 8;

        size = cnt*512;
        tmp_data = UAC_SEND_MAX - s_UacDev.tTrans.rd;
        if (tmp_data >= size)
        {
            memcpy(temp_buf,s_UacDev.tTrans.buf + s_UacDev.tTrans.rd,size);

            s_UacDev.tTrans.rd += size;

        }
        else
        {
            memcpy(temp_buf,s_UacDev.tTrans.buf + s_UacDev.tTrans.rd,UAC_SEND_MAX - s_UacDev.tTrans.rd);
            memcpy(temp_buf+(UAC_SEND_MAX - s_UacDev.tTrans.rd),s_UacDev.tTrans.buf,size-(UAC_SEND_MAX - s_UacDev.tTrans.rd));
            s_UacDev.tTrans.rd = size-(UAC_SEND_MAX - s_UacDev.tTrans.rd);

        }
    }

    fbuf_strc.buf_in  = temp_buf;
    fbuf_strc.buf_out = temp_buf;
    fbuf_strc.len_in  = size;
    fbuf_strc.len_out = UAC_MAX_READ_LEN;

    processlen = _SD_Filter_Control(pfilter, &fbuf_strc);


    if ((s_ResampleCtl.len - s_ResampleCtl.wr) > processlen)
    {
        memcpy(s_ResampleCtl.buf+s_ResampleCtl.wr, temp_buf, processlen);
        s_ResampleCtl.wr += processlen;
    }
    else
    {
        memcpy(s_ResampleCtl.buf+s_ResampleCtl.wr, temp_buf, s_ResampleCtl.len-s_ResampleCtl.wr);
        memcpy(s_ResampleCtl.buf, temp_buf+s_ResampleCtl.len-s_ResampleCtl.wr, processlen -(s_ResampleCtl.len-s_ResampleCtl.wr));
        s_ResampleCtl.wr = processlen -(s_ResampleCtl.len-s_ResampleCtl.wr);
    }

    bufcnt += processlen;
    if ((AK_FALSE == CHK_SENDFLAG(bUacSendPcm, DAC_SIGNAL)) && (bufcnt>=L2_DMA_SIZE))
    {
        bufcnt = 0;
        SET_SENDFLAG(bUacSendPcm, DAC_SIGNAL, AK_TRUE);

        msg.ucMsgId = MSG_PCM_SENT;
        if (!DrvModule_Send_Message(DRV_MODULE_UAC, UAC_DEV_MSG, (T_U32 *)&msg))
        {
            drv_print("send uac msg failed\n",0,1);
        }
    }

    return AK_TRUE;
}


T_BOOL uac_send_pcm_data(T_VOID)
{
    T_U32 tmp;
    T_U32 tmp_len = s_ResampleCtl.wr;
    T_U32 cnt ;
    T_U32 size,tmp_data;


    if ((tmp_len >= s_ResampleCtl.rd))
    {
        tmp = tmp_len - s_ResampleCtl.rd;
        cnt = tmp/512;
        if (0 == cnt)
        {
            SET_SENDFLAG(bUacSendPcm, DAC_SIGNAL, AK_FALSE);
            return AK_FALSE;
        }
        if (cnt>8)
            cnt=8;

        size = cnt*512;
        memcpy(sound_buf,s_ResampleCtl.buf+ s_ResampleCtl.rd,size);
        s_ResampleCtl.rd += size;

    }
    else if((tmp_len < s_ResampleCtl.rd))
    {
        tmp = s_ResampleCtl.len - s_ResampleCtl.rd + tmp_len;
        cnt = tmp / 512;
        if (0 == cnt)
        {
            SET_SENDFLAG(bUacSendPcm, DAC_SIGNAL, AK_FALSE);
            return AK_FALSE;
        }

        if (cnt > 8)
            cnt = 8;

        size = cnt*512;
        tmp_data = s_ResampleCtl.len - s_ResampleCtl.rd;

        if (tmp_data >= size)
        {
            memcpy(sound_buf,s_ResampleCtl.buf+ s_ResampleCtl.rd,size);
            s_ResampleCtl.rd += size;
        }
        else
        {
            memcpy(sound_buf,s_ResampleCtl.buf+ s_ResampleCtl.rd,tmp_data);
            memcpy(sound_buf+tmp_data,s_ResampleCtl.buf,size-tmp_data);
            s_ResampleCtl.rd = size-tmp_data;
        }
    }
    else
    {
        SET_SENDFLAG(bUacSendPcm, DAC_SIGNAL, AK_FALSE);
        return AK_FALSE;
    }



    if (AK_NULL != s_UacDev.fPCMSentCallBack)
    {
        s_UacDev.fPCMSentCallBack(sound_buf,size);
    }

    return AK_TRUE;

}

static T_BOOL uac_judge_data_ov(T_VOID)
{
    T_U32 tmp;

    if (s_UacDev.tTrans.wr < s_UacDev.tTrans.rd)
    {
        if (s_UacDev.tTrans.wr + UAC_RX_PER > s_UacDev.tTrans.rd)
        {
            return AK_FALSE;
        }
    }
    else if ((s_UacDev.tTrans.wr + UAC_RX_PER) > UAC_SEND_MAX)
    {
        tmp = s_UacDev.tTrans.wr + UAC_RX_PER - UAC_SEND_MAX;
        if (tmp > s_UacDev.tTrans.rd)
        {
            return AK_FALSE;
        }
    }
    return AK_TRUE;
}



#endif


T_VOID uac_set_callback(T_pUAC_AC_CTRL_CALLBACK ac_ctrl_callback,
                            T_pUAC_PCM_SENT_CALLBACK pcm_sent_callback)
{
    s_UacDev.fAcCtrlCallBack = ac_ctrl_callback;
#ifndef EXTERN_PCM_PLAYER
    s_UacDev.fPCMSentCallBack = pcm_sent_callback;
#endif
}


/**
 * @brief   set udisk class req callback
 *
 * called by usb drv  when msc class req is received successful
 * @author LiuHuadong
 * @date 2012-05-04
 * @param pTrans[in] ctrl trans struct
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TRUE means successful
 */
static T_BOOL uac_class_callback(T_CONTROL_TRANS *pTrans)
{
    T_U8 req_type;
    T_BOOL ret = AK_TRUE;
    T_U8 ctrl_id=0;
    T_U32 i;
    T_UAC_DEV_MSG msg;

    req_type = (pTrans->dev_req.bmRequestType >> 5) & 0x3;
    if (req_type != REQUEST_CLASS)
        return AK_FALSE;

    switch (pTrans->stage)
    {
        case CTRL_STAGE_SETUP:
        {
        switch (pTrans->dev_req.bRequest)
        {
            case 0x01:
                pTrans->data_len = 0;
            break;
            case GET_CUR:
                drv_print("GET_CUR",0,1);
                switch (HIBYTE(pTrans->dev_req.wIndex))
                {
                    case SPEAKER_FU_ID:
                    #ifdef UAC_MIC_FUN
                    case MIC_FU_ID:
                    #endif
                        ctrl_id = uac_ctrl_id_map(HIBYTE(pTrans->dev_req.wValue));

                        memcpy(pTrans->buffer,(T_U8 *)&s_UacDev.tCtrlSetting[ctrl_id].ulCur,s_UacDev.tCtrlSetting[ctrl_id].ulLen);
                        pTrans->data_len = s_UacDev.tCtrlSetting[ctrl_id].ulLen;

                    break;
                    default:
                        drv_print("Unsupport GET_CUR:Unit",0,1);
                        //ep0 will stall
                        ret = AK_FALSE;
                    break;
                }
                break;
            case GET_MIN:
                drv_print("GET_MIN",0,1);
                switch (HIBYTE(pTrans->dev_req.wIndex))
                {
                    case SPEAKER_FU_ID:
                    #ifdef UAC_MIC_FUN
                    case MIC_FU_ID:
                    #endif
                        ctrl_id = uac_ctrl_id_map(HIBYTE(pTrans->dev_req.wValue));

                        memcpy(pTrans->buffer,(T_U8 *)&s_UacDev.tCtrlSetting[ctrl_id].ulMin,s_UacDev.tCtrlSetting[ctrl_id].ulLen);
                        pTrans->data_len = s_UacDev.tCtrlSetting[ctrl_id].ulLen;
                    break;
                    default:
                        ret = AK_FALSE;
                    break;
                }
                break;
            case GET_MAX:
                drv_print("GET_MAX",0,1);
                switch (HIBYTE((pTrans->dev_req.wIndex)))
                {
                    case SPEAKER_FU_ID:
                    #ifdef UAC_MIC_FUN
                    case MIC_FU_ID:
                    #endif
                        ctrl_id = uac_ctrl_id_map(HIBYTE(pTrans->dev_req.wValue));

                        memcpy(pTrans->buffer,(T_U8 *)&s_UacDev.tCtrlSetting[ctrl_id].ulMax,s_UacDev.tCtrlSetting[ctrl_id].ulLen);
                        pTrans->data_len = s_UacDev.tCtrlSetting[ctrl_id].ulLen;
                    break;

                    default:
                        drv_print("Unsupport GET_MAX:Unit",0,1);
                        //ep0 will stall
                        ret = AK_FALSE;
                    break;
                }
                break;
            case GET_RES:
                drv_print("GET_RES",0,1);
                switch (HIBYTE(pTrans->dev_req.wIndex))
                {
                    case SPEAKER_FU_ID:
                    #ifdef UAC_MIC_FUN
                    case MIC_FU_ID:
                    #endif
                        ctrl_id = uac_ctrl_id_map(HIBYTE(pTrans->dev_req.wValue));

                        memcpy(pTrans->buffer,(T_U8 *)&s_UacDev.tCtrlSetting[ctrl_id].ulRes,s_UacDev.tCtrlSetting[ctrl_id].ulLen);
                        pTrans->data_len = s_UacDev.tCtrlSetting[ctrl_id].ulLen;
                    break;
                    default:
                        drv_print("Unsupport GET_RES:Unit",0,1);
                        //ep0 will stall
                        ret = AK_FALSE;
                    break;
                }
                break;
            case RESERVED:
                //这里需加此处理！
                pTrans->data_len = 0;
                break;
            default:
                ret = AK_FALSE;
                break;
        }

        }break;
        case CTRL_STAGE_DATA_OUT:

            if (SET_CUR == pTrans->dev_req.bRequest)
            {

                msg.ucMsgId = MSG_AC_CTRL;
                msg.ulParam4 = pTrans->dev_req.wIndex;
                msg.ucParam2 = HIBYTE(pTrans->dev_req.wValue);
                msg.ulParam3 = *((T_U32 *)(pTrans->buffer));

                DrvModule_Send_Message(DRV_MODULE_UAC, UAC_DEV_MSG, (T_U32*)&msg);
            }
            break;
        case CTRL_STAGE_DATA_IN:
        case CTRL_STAGE_STATUS:
            break;
        default:
            break;
    }
    return ret;

}

/**
 * @brief get dev qualifier descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author LiuHuadong
 * @date 2010-08-04
 * @param count[out] len of dev qualifier descriptor
 * @return  T_U8 *
 * @retval  addr of dev qualifier descriptor
 */
static T_U8 *uac_get_dev_qualifier_desc(T_U32 *count)
{
    *count = sizeof(s_UacDeviceQualifierDesc);
    return (T_U8 *)&s_UacDeviceQualifierDesc;
}

/**
 * @brief get dev descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author LiuHuadong
 * @date 2012-05-04
 * @param count[out] len of dev descriptor
 * @return  T_U8 *
 * @retval  addr of dev descriptor
 */
static T_U8 *uac_get_dev_desc(T_U32 *count)
{
    *count = sizeof(s_UacDeviceDesc);
    return (T_U8 *)&s_UacDeviceDesc;
}

/**
 * @brief get all config descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author LiuHuadong
 * @date 2012-05-04
 * @param count[out] len of all config descriptor
 * @return  T_U8 *
 * @retval  addr of all config descriptor
 */
static T_U8 *uac_get_cfg_desc(T_U32 *count)
{
    T_U32 cnt = 0;

    memcpy(uac_configdata, (T_U8 *)&s_UacConfigDesc, *(T_U8 *)&s_UacConfigDesc);
    cnt += *(T_U8 *)&s_UacConfigDesc;
    memcpy(uac_configdata + cnt, (T_U8 *)&interface_descriptor, *(T_U8 *)&interface_descriptor);
    cnt += *(T_U8 *)&interface_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&interface_head_descriptor, *(T_U8 *)&interface_head_descriptor);
    cnt += *(T_U8 *)&interface_head_descriptor;

    #ifdef UAC_MIC_FUN
    memcpy(uac_configdata + cnt, (T_U8 *)&input_mic_terminal_descriptor, *(T_U8 *)&input_terminal_descriptor);
    cnt += *(T_U8 *)&input_terminal_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&input_mic_feature_descriptor, *(T_U8 *)&input_feature_descriptor);
    cnt += *(T_U8 *)&input_feature_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&output_mic_terminal_descriptor, *(T_U8 *)&output_terminal_descriptor);
    cnt += *(T_U8 *)&output_terminal_descriptor;
    #endif
    memcpy(uac_configdata + cnt, (T_U8 *)&input_terminal_descriptor, *(T_U8 *)&input_terminal_descriptor);
    cnt += *(T_U8 *)&input_terminal_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&input_feature_descriptor, *(T_U8 *)&input_feature_descriptor);
    cnt += *(T_U8 *)&input_feature_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&output_terminal_descriptor, *(T_U8 *)&output_terminal_descriptor);
    cnt += *(T_U8 *)&output_terminal_descriptor;

    #ifdef UAC_MIC_FUN
    memcpy(uac_configdata + cnt, (T_U8 *)&output_bandwidth_mic_descriptor, *(T_U8 *)&output_bandwidth_descriptor);
    cnt += *(T_U8 *)&output_bandwidth_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&output_streaming_mic_descriptor, *(T_U8 *)&output_streaming_descriptor);
    cnt += *(T_U8 *)&output_streaming_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&class_specific_mic_descriptor, *(T_U8 *)&class_specific_descriptor);
    cnt += *(T_U8 *)&class_specific_descriptor;

    memcpy(uac_configdata + cnt, (T_U8 *)&type_format_descriptor, *(T_U8 *)&type_format_descriptor);
    cnt += *(T_U8 *)&type_format_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&iso_mic_endpoint_descriptor, *(T_U8 *)&iso_endpoint_descriptor);
    cnt += *(T_U8 *)&iso_endpoint_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&iso_endpoint_subtype_descriptor, *(T_U8 *)&iso_endpoint_subtype_descriptor);
    cnt += *(T_U8 *)&iso_endpoint_subtype_descriptor;
    #endif
    memcpy(uac_configdata + cnt, (T_U8 *)&output_bandwidth_descriptor, *(T_U8 *)&output_bandwidth_descriptor);
    cnt += *(T_U8 *)&output_bandwidth_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&output_streaming_descriptor, *(T_U8 *)&output_streaming_descriptor);
    cnt += *(T_U8 *)&output_streaming_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&class_specific_descriptor, *(T_U8 *)&class_specific_descriptor);
    cnt += *(T_U8 *)&class_specific_descriptor;

    memcpy(uac_configdata + cnt, (T_U8 *)&type_format_descriptor, *(T_U8 *)&type_format_descriptor);
    cnt += *(T_U8 *)&type_format_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&iso_endpoint_descriptor, *(T_U8 *)&iso_endpoint_descriptor);
    cnt += *(T_U8 *)&iso_endpoint_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&iso_endpoint_subtype_descriptor, *(T_U8 *)&iso_endpoint_subtype_descriptor);
    cnt += *(T_U8 *)&iso_endpoint_subtype_descriptor;

    memcpy(uac_configdata + cnt, (T_U8 *)&hid_interface_descriptor, *(T_U8 *)&hid_interface_descriptor);
    cnt += *(T_U8 *)&hid_interface_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&hid_report_descriptor, *(T_U8 *)&hid_report_descriptor);
    cnt += *(T_U8 *)&hid_report_descriptor;
    memcpy(uac_configdata + cnt, (T_U8 *)&intr_endpoint_descriptor, *(T_U8 *)&intr_endpoint_descriptor);
    cnt += *(T_U8 *)&intr_endpoint_descriptor;
    s_UacConfigDesc.wTotalLength=cnt;

    *count = cnt;

    return uac_configdata;
}

static T_U8 *uac_get_hid_report_desc(T_U32 *count)
{
    *count = sizeof(HidReportDesc);
    return (T_U8 *)&HidReportDesc;
}


/**
 * @brief get other speed config descriptor callback
 *
 * this callback is set at uac_enable()
 * @author LiuHuadong
 * @date 2012-05-04
 * @param count[out] len of other speed config descriptor
 * @return  T_U8 *
 * @retval  addr of other speed config descriptor
 */
static T_U8 * uac_get_other_speed_cfg_desc(T_U32 *count)
{
    T_U16 cnt = 0;

    memcpy(uac_configdata, (T_U8 *)&s_UacOtherSpeedConfigDesc, *(T_U8 *)&s_UacOtherSpeedConfigDesc);
    cnt = s_UacOtherSpeedConfigDesc.wTotalLength;
    //other speed is full speed,ep2 maxpktsize is 64
    if (s_UacDev.ulMode == USB_MODE_20)
    {
        uac_configdata[cnt-3]=0x40;
        uac_configdata[cnt-2]=0x00;
    }
    //other speed is high speed,ep2 maxpktsize is 512
    else
    {
        uac_configdata[cnt-3]=0x00;
        uac_configdata[cnt-2]=0x02;
    }
    *count = cnt;
    return uac_configdata;
}

/**
 * @brief get string descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author LiuHuadong
 * @date 2012-05-04
 * @param index[in] index of string descriptor
 * @param count[out] len of stirng descriptor
 * @return  T_U8 *
 * @retval  addr of string descriptor
 */
static T_U8 *uac_get_str_desc(T_U8 index, T_U32 *count)
{
    if(index == 0)
    {
        *count = sizeof(s_UacString0);
        return (T_U8 *)s_UacString0;
    }
    else if(index == 1)
    {
        *count = sizeof(s_UacString1);
        return (T_U8 *)s_UacString1;
    }
    else if(index == 2)
    {
        *count = sizeof(s_UacString2);
        return (T_U8 *)s_UacString2;
    }
      else if(index == 3)
    {
        *count = sizeof(s_UacString3);
        return (T_U8 *)s_UacString3;
    }
    return AK_NULL;
}


