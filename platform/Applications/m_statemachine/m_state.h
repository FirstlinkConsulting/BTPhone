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
  eM_s_pstn_phone = 4,
  eM_s_pstn_ring = 5,
  eM_s_pstn_dial = 6,
  eM_s_pstn_connect = 7,
  eM_s_bt_phone = 8,
  eM_s_bt_player = 9,
  eM_s_pub_switch_off = 10,
  eM_s_pub_charger = 11,
  eM_s_pub_pullout_sd = 12,
  eM_s_pub_usb_detect = 13,
  eM_s_pub_usbdisk = 14,
  eM_s_pub_set_menu = 15,
  eM_s_pub_standby = 16,
  eM_preproc = 17,
  eM_postproc = 18,
  eM_s_audio_record = 19,
  eM_s_audio_tomplay = 20,
  eM_s_audio_main = 21,
  eM_s_clk_music_play = 22,
  eM_s_audio_abplay = 23,
  eM_s_linein_play = 24,
  eM_s_radio_player = 25
} M_STATES;
typedef enum 
{
  eM_handleaudio_abplay = 0,
  eM_handleaudio_main = 1,
  eM_handleaudio_record = 2,
  eM_handleaudio_tomplay = 3,
  eM_handlebt_phone = 4,
  eM_handlebt_player = 5,
  eM_handleclk_music_play = 6,
  eM_handleinit_power_on = 7,
  eM_handleinit_system = 8,
  eM_handlelinein_play = 9,
  eM_handlepstn_connect = 10,
  eM_handlepstn_dial = 11,
  eM_handlepstn_phone = 12,
  eM_handlepstn_ring = 13,
  eM_handlepub_charger = 14,
  eM_handlepub_pullout_sd = 15,
  eM_handlepub_set_menu = 16,
  eM_handlepub_standby = 17,
  eM_handlepub_switch_off = 18,
  eM_handlepub_usb_detect = 19,
  eM_handlepub_usbdisk = 20,
  eM_handleradio_player = 21,
  eM_handleset_firmware_updata = 22,
  eM_handlestdb_standby = 23
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
