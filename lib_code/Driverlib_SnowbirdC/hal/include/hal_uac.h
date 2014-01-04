/**
* @FILENAME uac_audio.h
* @BRIEF USB audio Class type definition & data structure
* Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR liuhuadong
* @DATE 2012-06-10
* @UPDATE 2012-06-11
* @VERSION 0.0.4
* @REF USB audio Class 1.0 Spec
*/

#ifndef _UAC_H_
#define _UAC_H_

#include "anyka_types.h"


#ifdef __CC_ARM
__packed
#endif
struct S_USB_INTERFACE_DESCRIPTOR_AUDIO
{
   T_U8 bLength;                       // Size of this descriptor (=0x09)
   T_U8 bDescriptorType;               // Descriptor type (=0x04)
   T_U8 bInterfaceNumber;              // Number of *this* interface (0..)
   T_U8 bAlternateSetting;             // Alternative for this interface
   T_U8 bNumEndpoints;                 // No of EPs used by this interface
   T_U8 bInterfaceClass;               // Interface class code
   T_U8 bInterfaceSubClass;            // Interface subclass code
   T_U8 bInterfaceProtocol;            // Interface protocol code
   T_U8 iInterface;                    // Index of interface string
} 
#ifdef __GNUC__
    __attribute__((packed))
#endif
;

typedef struct S_USB_INTERFACE_DESCRIPTOR_AUDIO T_USB_INTERFACE_DESCRIPTOR_AUDIO;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Header Descriptor Type Definition
struct  S_USB_AUDIO_HEAD_DESC
{
   T_U8 bLength;                       // Size of this descriptor (=0x09)
   T_U8 bDescriptorType;               // Descriptor type (=0x24, CS_INTERFACE)
   T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x01)
   T_U16 bcdADC;                        // Revision of class specification
   T_U16 wTotalLength;                  // Total size of audio descriptors
   T_U8 bInCollection;                 // Number of streaming interfaces
   T_U8 baInterfaceNr;                 // Interface number of stream interface
   T_U8 baInterfaceNr1;                 // Interface number of stream interface
}
#ifdef __GNUC__
    __attribute__((packed))
#endif
;

typedef struct S_USB_AUDIO_HEAD_DESC T_USB_AUDIO_HEAD_DESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Header Descriptor Type Definition
struct  S_USB_AUDIO_IT_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x0C)
    T_U8 bDescriptorType;               // Descriptor type (=0x24, CS_INTERFACE)
    T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x02)
    T_U8 bTerminalID;                   // Unique terminal identification
    T_U16 wTerminalType;                 // Audio terminal type
    T_U8 bAssocTerminal;                // TerminalID of associated terminal
    T_U8 bNrChannels;                   // Number of logical channels
    T_U16 wChannelConfig;                // Channel configuration
    T_U8 iChannelNames;                 // Index of channel string
    T_U8 iTerminal;                     // Index of terminal string

}
#ifdef __GNUC__
    __attribute__((packed))
#endif
;

typedef struct S_USB_AUDIO_IT_DESC T_USB_AUDIO_IT_DESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_FEATURE_DESC
{
   T_U8 bLength;                       // Size of this descriptor (=0x0D)
   T_U8 bDescriptorType;               // Descriptor type (=0x24, CS_INTERFACE)
   T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x06)
   T_U8 bUnitID;                       // Unique unit identification
   T_U8 bSourceID;                     // ID of connected terminal or unit
   T_U8 bControlSize;                  // Size of bmaControl element
   T_U16 bmaControlMaster;              // Master control bitmask
   T_U16 bmaControlLogical1;            // Logical channel 1 control bitmask
   T_U16 bmaControlLogical2;            // Logical channel 2 control bitmask
   T_U8 iFeature;                      // Index of feature unit string
}              // End of feature_unit_descriptor

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_FEATURE_DESC T_USB_AUDIO_FEATURE_DESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_OT_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x09)
    T_U8 bDescriptorType;               // Descriptor type (=0x24, CS_INTERFACE)
    T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x03)
    T_U8 bTerminalID;                   // Unique terminal identification
    T_U16 wTerminalType;                 // Audio terminal type
    T_U8 bAssocTerminal;                // TerminalID of associated terminal
    T_U8 bSourceID;                     // Terminal ID of source
    T_U8 iTerminal;                     // Index of terminal string

}              // End of feature_unit_descriptor

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_OT_DESC T_USB_AUDIO_OT_DESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_CLASS_SPEC_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x07)
    T_U8 bDescriptorType;               // Descriptor Type (=0x24, CS_INTERFACE)
    T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x01)
    T_U8 bTerminalLink;                 // ID of associated terminal
    T_U8 bDelay;                        // Interface delay
    T_U16 wFormatTag;                    // Stream format
}              // End of feature_unit_descriptor

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_CLASS_SPEC_DESC T_USB_AUDIO_CLASS_SPEC_DESC;
typedef unsigned char BYTE3[3];

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_TYPE_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x0B)
    T_U8 bDescriptorType;               // Descriptor Type (=0x24, CS_INTERFACE)
    T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x02)
    T_U8 bFormatType;                   // Stream format type (=0x01, TYPE_I)
    T_U8 bNrChannels;                   // Number of logical channels
    T_U8 bSubFrameSize;                 // Number of bytes/audio subframe
    T_U8 bBitResolution;                // Number of bits/audio subframe
    T_U8 bSamFreqType;                  // Number of sampling frequencies
    BYTE3 bSamFreq;                     // Sampling frequency
    BYTE3 bSamFreq2;                     // Sampling frequency
}              // End of feature_unit_descriptor

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_TYPE_DESC T_USB_AUDIO_TYPE_DESC;


#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_ENDPOINT_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x09)
    T_U8 bDescriptorType;               // Descriptor Type (=0x05)
    T_U8 bEndpointAddress;              // Endpoint address (number + direction)
    T_U8 bmAttributes;                  // Endpoint attributes (transfer type)
    T_U16 wMaxPacketSize;                // Maximum endpoint packet size
    T_U8 bInterval;                     // Polling interval milliseconds
    T_U8 bRefresh;                      // Reset to 0
    T_U8 bSynchAddress;                 // Synchronization endpoint number

}              // End of feature_unit_descriptor

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_ENDPOINT_DESC T_USB_AUDIO_ENDPOINT_DESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_SUBTYPE_DESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x07)
    T_U8 bDescriptorType;               // Descriptor type (=0x25, CS_ENDPOINT)
    T_U8 bDescriptorSubtype;            // Descriptor subtype (=0x01)
    T_U8 bmAttributes;                  // Endpoint attributes
    T_U8 bLockDelayUnits;               // Units used for wLockDelay
    T_U16 wLockDelay;                    // Time for clock recovery lock
}              // End of feature_unit_descriptor
#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_SUBTYPE_DESC T_USB_AUDIO_SUBTYPE_DESC;


#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_HIDDESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x09)
    T_U8 bDescriptorType;               // Descriptor type (=0x21)
    T_U16 bcdHID;                        // HID spec release number in BCD
    T_U8 bCountryCode;                  // Country code of localized hardware
    T_U8 bNumDescriptors;               // Number of class descriptors
    T_U8 bClassDescriptorType;          // Type of class descriptor
    T_U16 wDescriptorLength;             // Total size of report descriptor

}              // End of feature_unit_descriptor
#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_HIDDESC T_USB_AUDIO_HIDDESC;

#ifdef __CC_ARM
__packed
#endif
// Audio Class-Specific Feature Unit Descriptor Type Definition
struct S_USB_AUDIO_EPDESC
{
    T_U8 bLength;                       // Size of this descriptor (=0x07)
    T_U8 bDescriptorType;               // Descriptor type (=0x05)
    T_U8 bEndpointAddress;              // Endpoint address (number + direction)
    T_U8 bmAttributes;                  // Endpoint attributes (transfer type)
    T_U16 wMaxPacketSize;                // Maximum endpoint packet size
    T_U8 bInterval;                     // Polling interval milliseconds
}              // End of feature_unit_descriptor
#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_AUDIO_EPDESC T_USB_AUDIO_EPDESC;

#ifdef __CC_ARM
__packed
#endif

// HID Class-Specific Descriptor Type Definition
struct S_USB_HID_EPDESC
{
   T_U8  bLength;                       // Size of this descriptor (=0x09)
   T_U8  bDescriptorType;               // Descriptor type (=0x21)
   T_U16 bcdHID;                        // HID spec release number in BCD
   T_U8  bCountryCode;                  // Country code of localized hardware
   T_U8  bNumDescriptors;               // Number of class descriptors
   T_U8  bClassDescriptorType;          // Type of class descriptor
   T_U16 wDescriptorLength;             // Total size of report descriptor
} 

#ifdef __GNUC__
    __attribute__((packed))
#endif
;
typedef struct S_USB_HID_EPDESC T_USB_HID_EPDESC;


#endif  //  _UVC_H_
