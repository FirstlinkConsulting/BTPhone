#ifndef _M_STATE_H
#define _M_STATE_H
#include "m_event.h"


#define MAX_STACK_DEPTH 11

typedef enum 
{
  eM_s_init_system = 0,
  eM_s_init_power_on = 1,
  eM_set_firmware_updata = 2,
  eM_s_stdb_standby = 3,
  eM_s_bt_phone = 4,
  eM_s_bt_player = 5,
  eM_s_pub_switch_off = 6,
  eM_s_pub_charger = 7,
  eM_s_pub_pullout_sd = 8,
  eM_s_pub_usb_detect = 9,
  eM_s_pub_usbdisk = 10,
  eM_s_pub_set_menu = 11,
  eM_s_pub_standby = 12,
  eM_preproc = 13,
  eM_postproc = 14,
  eM_s_camera_capture = 15,
  eM_s_audio_record = 16,
  eM_s_audio_tomplay = 17,
  eM_s_audio_main = 18,
  eM_s_clk_music_play = 19,
  eM_s_audio_abplay = 20,
  eM_s_linein_play = 21,
  eM_s_radio_player = 22
} M_STATES;
typedef enum 
{
  eM_handleaudio_abplay = 0,
  eM_handleaudio_main = 1,
  eM_handleaudio_record = 2,
  eM_handleaudio_tomplay = 3,
  eM_handlebt_phone = 4,
  eM_handlebt_player = 5,
  eM_handlecamera_capture = 6,
  eM_handleclk_music_play = 7,
  eM_handleinit_power_on = 8,
  eM_handleinit_system = 9,
  eM_handlelinein_play = 10,
  eM_handlepub_charger = 11,
  eM_handlepub_pullout_sd = 12,
  eM_handlepub_set_menu = 13,
  eM_handlepub_standby = 14,
  eM_handlepub_switch_off = 15,
  eM_handlepub_usb_detect = 16,
  eM_handlepub_usbdisk = 17,
  eM_handleradio_player = 18,
  eM_handleset_firmware_updata = 19,
  eM_handlestdb_standby = 20
} M_FUNCTIONS;
typedef enum 
{
  eM_ddaudctrlhandler = 0,
  eM_ddaudctrltimerhandler = 1,
  eM_ddlineinhandle = 2,
  eM_ddpubtimerhandler = 3,
  eM_ddsdcardhandler = 4,
  eM_ddusbdetecthandle = 5,
  eM_ddusbhostdiskhandler = 6,
  eM_ddusbouthandler = 7,
  eM_dduserkeyhandler = 8
} M_EFUNCTIONS;




#include "m_state_api.h"
#endif
