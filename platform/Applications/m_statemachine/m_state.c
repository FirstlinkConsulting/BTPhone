#include "m_event.h"
#include "m_state.h"
#include "sm_port.h"

    
#ifdef WIN32
#include <stdio.h>
#include <string.h>
#endif

/* ---------------------------------------------------------------------------------------*/
/*                     state functions: init, exit, paint                                 */
/* ---------------------------------------------------------------------------------------*/
extern void exitaudio_abplay(void);
extern void exitaudio_main(void);
extern void exitaudio_record(void);
extern void exitaudio_tomplay(void);
extern void exitbt_phone(void);
extern void exitbt_player(void);
extern void exitclk_music_play(void);
extern void exitinit_power_on(void);
extern void exitinit_system(void);
extern void exitlinein_play(void);
extern void exitpstn_connect(void);
extern void exitpstn_dial(void);
extern void exitpstn_phone(void);
extern void exitpstn_ring(void);
extern void exitpub_charger(void);
extern void exitpub_pullout_sd(void);
extern void exitpub_set_menu(void);
extern void exitpub_standby(void);
extern void exitpub_switch_off(void);
extern void exitpub_usb_detect(void);
extern void exitpub_usbdisk(void);
extern void exitradio_player(void);
extern void exitset_firmware_updata(void);
extern void exitstdb_standby(void);
extern void initaudio_abplay(void);
extern void initaudio_main(void);
extern void initaudio_record(void);
extern void initaudio_tomplay(void);
extern void initbt_phone(void);
extern void initbt_player(void);
extern void initclk_music_play(void);
extern void initinit_power_on(void);
extern void initinit_system(void);
extern void initlinein_play(void);
extern void initpstn_connect(void);
extern void initpstn_dial(void);
extern void initpstn_phone(void);
extern void initpstn_ring(void);
extern void initpub_charger(void);
extern void initpub_pullout_sd(void);
extern void initpub_set_menu(void);
extern void initpub_standby(void);
extern void initpub_switch_off(void);
extern void initpub_usb_detect(void);
extern void initpub_usbdisk(void);
extern void initradio_player(void);
extern void initset_firmware_updata(void);
extern void initstdb_standby(void);
extern void paintaudio_abplay(void);
extern void paintaudio_main(void);
extern void paintaudio_record(void);
extern void paintaudio_tomplay(void);
extern void paintbt_phone(void);
extern void paintbt_player(void);
extern void paintclk_music_play(void);
extern void paintinit_power_on(void);
extern void paintinit_system(void);
extern void paintlinein_play(void);
extern void paintpstn_connect(void);
extern void paintpstn_dial(void);
extern void paintpstn_phone(void);
extern void paintpstn_ring(void);
extern void paintpub_charger(void);
extern void paintpub_pullout_sd(void);
extern void paintpub_set_menu(void);
extern void paintpub_standby(void);
extern void paintpub_switch_off(void);
extern void paintpub_usb_detect(void);
extern void paintpub_usbdisk(void);
extern void paintradio_player(void);
extern void paintset_firmware_updata(void);
extern void paintstdb_standby(void);
/* ---------------------------------------------------------------------------------------*/
/*                      functions                                                        */
/* ---------------------------------------------------------------------------------------*/
extern unsigned char handleaudio_abplay(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleaudio_main(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleaudio_record(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleaudio_tomplay(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlebt_phone(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlebt_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleclk_music_play(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleinit_power_on(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleinit_system(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlelinein_play(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepstn_connect(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepstn_dial(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepstn_phone(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepstn_ring(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_charger(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_pullout_sd(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_set_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_standby(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_switch_off(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_usb_detect(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlepub_usbdisk(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleradio_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handleset_firmware_updata(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
extern unsigned char handlestdb_standby(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
/* ---------------------------------------------------------------------------------------*/
/*                     event functions                                                   */
/* ---------------------------------------------------------------------------------------*/
extern unsigned char ddaudctrlhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddaudctrltimerhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddlineinhandle(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddpubtimerhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddsdcardhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddusbdetecthandle(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddusbhostdiskhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char ddusbouthandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
extern unsigned char dduserkeyhandler(T_EVT_CODE* event, T_EVT_PARAM** pEventParm);
/* ---------------------------------------------------------------------------------------*/
/*                     state functions:    getNext                                        */
/* ---------------------------------------------------------------------------------------*/
static int m_postprocGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_preprocGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_audio_abplayGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_audio_mainGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_audio_recordGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_audio_tomplayGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_bt_phoneGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_bt_playerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_clk_music_playGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_init_power_onGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_init_systemGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_linein_playGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pstn_connectGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pstn_dialGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pstn_phoneGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pstn_ringGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_chargerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_pullout_sdGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_set_menuGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_standbyGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_switch_offGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_usb_detectGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_pub_usbdiskGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_radio_playerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_s_stdb_standbyGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
static int m_set_firmware_updataGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans);
/* ---------------------------------------------------------------------------------------*/
/*                     state structs                                                      */
/* ---------------------------------------------------------------------------------------*/
static const M_STATESTRUCT m_postproc =
{
  (void *)0,
  (void *)0,
  (void *)0,
  m_postprocGetNext
};
#pragma arm section rodata = "_bootcode1_"
static const M_STATESTRUCT m_preproc =
{
  (void *)0,
  (void *)0,
  (void *)0,
  m_preprocGetNext
};
#pragma arm section rodata
static const M_STATESTRUCT m_s_audio_abplay =
{
  initaudio_abplay,
  exitaudio_abplay,
  paintaudio_abplay,
  m_s_audio_abplayGetNext
};
#pragma arm section rodata = "_audioplayer_"
static const M_STATESTRUCT m_s_audio_main =
{
  initaudio_main,
  exitaudio_main,
  paintaudio_main,
  m_s_audio_mainGetNext
};
#pragma arm section rodata
static const M_STATESTRUCT m_s_audio_record =
{
  initaudio_record,
  exitaudio_record,
  paintaudio_record,
  m_s_audio_recordGetNext
};
static const M_STATESTRUCT m_s_audio_tomplay =
{
  initaudio_tomplay,
  exitaudio_tomplay,
  paintaudio_tomplay,
  m_s_audio_tomplayGetNext
};
static const M_STATESTRUCT m_s_bt_phone =
{
  initbt_phone,
  exitbt_phone,
  paintbt_phone,
  m_s_bt_phoneGetNext
};
static const M_STATESTRUCT m_s_bt_player =
{
  initbt_player,
  exitbt_player,
  paintbt_player,
  m_s_bt_playerGetNext
};
static const M_STATESTRUCT m_s_clk_music_play =
{
  initclk_music_play,
  exitclk_music_play,
  paintclk_music_play,
  m_s_clk_music_playGetNext
};
static const M_STATESTRUCT m_s_init_power_on =
{
  initinit_power_on,
  exitinit_power_on,
  paintinit_power_on,
  m_s_init_power_onGetNext
};
static const M_STATESTRUCT m_s_init_system =
{
  initinit_system,
  exitinit_system,
  paintinit_system,
  m_s_init_systemGetNext
};
static const M_STATESTRUCT m_s_linein_play =
{
  initlinein_play,
  exitlinein_play,
  paintlinein_play,
  m_s_linein_playGetNext
};
static const M_STATESTRUCT m_s_pstn_connect =
{
  initpstn_connect,
  exitpstn_connect,
  paintpstn_connect,
  m_s_pstn_connectGetNext
};
static const M_STATESTRUCT m_s_pstn_dial =
{
  initpstn_dial,
  exitpstn_dial,
  paintpstn_dial,
  m_s_pstn_dialGetNext
};
static const M_STATESTRUCT m_s_pstn_phone =
{
  initpstn_phone,
  exitpstn_phone,
  paintpstn_phone,
  m_s_pstn_phoneGetNext
};
static const M_STATESTRUCT m_s_pstn_ring =
{
  initpstn_ring,
  exitpstn_ring,
  paintpstn_ring,
  m_s_pstn_ringGetNext
};
static const M_STATESTRUCT m_s_pub_charger =
{
  initpub_charger,
  exitpub_charger,
  paintpub_charger,
  m_s_pub_chargerGetNext
};
static const M_STATESTRUCT m_s_pub_pullout_sd =
{
  initpub_pullout_sd,
  exitpub_pullout_sd,
  paintpub_pullout_sd,
  m_s_pub_pullout_sdGetNext
};
static const M_STATESTRUCT m_s_pub_set_menu =
{
  initpub_set_menu,
  exitpub_set_menu,
  paintpub_set_menu,
  m_s_pub_set_menuGetNext
};
static const M_STATESTRUCT m_s_pub_standby =
{
  initpub_standby,
  exitpub_standby,
  paintpub_standby,
  m_s_pub_standbyGetNext
};
static const M_STATESTRUCT m_s_pub_switch_off =
{
  initpub_switch_off,
  exitpub_switch_off,
  paintpub_switch_off,
  m_s_pub_switch_offGetNext
};
static const M_STATESTRUCT m_s_pub_usb_detect =
{
  initpub_usb_detect,
  exitpub_usb_detect,
  paintpub_usb_detect,
  m_s_pub_usb_detectGetNext
};
static const M_STATESTRUCT m_s_pub_usbdisk =
{
  initpub_usbdisk,
  exitpub_usbdisk,
  paintpub_usbdisk,
  m_s_pub_usbdiskGetNext
};
static const M_STATESTRUCT m_s_radio_player =
{
  initradio_player,
  exitradio_player,
  paintradio_player,
  m_s_radio_playerGetNext
};
static const M_STATESTRUCT m_s_stdb_standby =
{
  initstdb_standby,
  exitstdb_standby,
  paintstdb_standby,
  m_s_stdb_standbyGetNext
};
static const M_STATESTRUCT m_set_firmware_updata =
{
  initset_firmware_updata,
  exitset_firmware_updata,
  paintset_firmware_updata,
  m_set_firmware_updataGetNext
};
/* ---------------------------------------------------------------------------------------*/
/*                     next function implementations                                      */
/* ---------------------------------------------------------------------------------------*/
static int m_postprocGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_Z00_POWEROFF:
      nextstate = eM_s_pub_switch_off;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_Z01_MUSIC_PLAY:
      nextstate = eM_s_clk_music_play;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_Z02_STANDBY:
      nextstate = eM_s_pub_standby;
      *pTrans = eM_TYPE_IRPT;
      break;
    default:
      break;
  }
  return(nextstate);
}
#pragma arm section code = "_bootcode1_"
static int m_preprocGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_AUDIO_CTRL:
      nextstate = eM_ddaudctrlhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_AUDIO_CTRL_TIMER:
      nextstate = eM_ddaudctrltimerhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_LINE_IN:
      nextstate = eM_ddlineinhandle;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_PUB_TIMER:
      nextstate = eM_ddpubtimerhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_SDCARD:
      nextstate = eM_ddsdcardhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_USBHOSTDISK:
      nextstate = eM_ddusbhostdiskhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_USB_IN:
      nextstate = eM_ddusbdetecthandle;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_USB_OUT:
      nextstate = eM_ddusbouthandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    case M_EVT_USER_KEY:
      nextstate = eM_dduserkeyhandler;
      *pTrans = eM_TYPE_ECALL;
      break;
    default:
      break;
  }
  return(nextstate);
}
#pragma arm section code
static int m_s_audio_abplayGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleaudio_abplay;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
#pragma arm section code = "_audioplayer_"
static int m_s_audio_mainGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_ABPLAY:
      nextstate = eM_s_audio_abplay;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_MENU:
      nextstate = eM_s_pub_set_menu;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RETURN2:
      *pTrans = eM_TYPE_RETURN + 1;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleaudio_main;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
#pragma arm section code
static int m_s_audio_recordGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_MENU:
      nextstate = eM_s_pub_set_menu;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleaudio_record;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_audio_tomplayGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_MENU:
      nextstate = eM_s_pub_set_menu;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleaudio_tomplay;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_bt_phoneGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_EXIT_MODE:
      *pTrans = eM_TYPE_RETURN + 2;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlebt_phone;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_bt_playerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_BTPHONE:
      nextstate = eM_s_bt_phone;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_MENU:
      nextstate = eM_s_pub_set_menu;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RADIO_REC:
      nextstate = eM_s_audio_record;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RETURN:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlebt_player;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_clk_music_playGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    case M_EVT_TIMEOUT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handleclk_music_play;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_init_power_onGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_NEXT:
      nextstate = eM_s_stdb_standby;
      *pTrans = eM_TYPE_EXIT;
      break;
    case M_EVT_RESTART:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleinit_power_on;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_init_systemGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_1:
      nextstate = eM_s_init_power_on;
      *pTrans = eM_TYPE_EXIT;
      break;
    case M_EVT_UPDATA:
      nextstate = eM_set_firmware_updata;
      *pTrans = eM_TYPE_IRPT;
      break;
    default:
      nextstate = eM_handleinit_system;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_linein_playGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlelinein_play;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pstn_connectGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlepstn_connect;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pstn_dialGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlepstn_dial;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pstn_phoneGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_EXIT_MODE:
      *pTrans = eM_TYPE_RETURN + 2;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlepstn_phone;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pstn_ringGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handlepstn_ring;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_chargerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_CHARGER_OUT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handlepub_charger;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_pullout_sdGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handlepub_pullout_sd;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_set_menuGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    case M_EVT_TIMEOUT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handlepub_set_menu;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_standbyGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handlepub_standby;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_switch_offGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_ALARM_PLAY:
      nextstate = eM_s_init_power_on;
      *pTrans = eM_TYPE_EXIT;
      break;
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RESTART:
      nextstate = eM_s_init_power_on;
      *pTrans = eM_TYPE_EXIT;
      break;
    default:
      nextstate = eM_handlepub_switch_off;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_usb_detectGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_CHARGER:
      nextstate = eM_s_pub_charger;
      *pTrans = eM_TYPE_EXIT;
      break;
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_USBDISK:
      nextstate = eM_s_pub_usbdisk;
      *pTrans = eM_TYPE_EXIT;
      break;
    default:
      nextstate = eM_handlepub_usb_detect;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_pub_usbdiskGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handlepub_usbdisk;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_radio_playerGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RADIO_REC:
      nextstate = eM_s_audio_record;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RETURN:
      *pTrans = eM_TYPE_RETURN;
      break;
    case M_EVT_RETURN_ROOT:
      *pTrans = eM_TYPE_RETURN_ROOT;
      break;
    default:
      nextstate = eM_handleradio_player;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_s_stdb_standbyGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_AUDIO_VOICE:
      nextstate = eM_s_audio_main;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_BTPLAYER:
      nextstate = eM_s_bt_player;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_UPDATA:
      nextstate = eM_set_firmware_updata;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_USBDISK:
      nextstate = eM_s_pub_usbdisk;
      *pTrans = eM_TYPE_EXIT;
      break;
    case M_EVT_USB_AUDIO:
      nextstate = eM_s_audio_main;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_USB_DETECT:
      nextstate = eM_s_pub_usb_detect;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_LINEIN_PLAY:
      nextstate = eM_s_linein_play;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_PULLOUT_SD:
      nextstate = eM_s_pub_pullout_sd;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RADIO:
      nextstate = eM_s_radio_player;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_RECORD:
      nextstate = eM_s_audio_record;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_SD_AUDIO:
      nextstate = eM_s_audio_main;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_SD_RECORD:
      nextstate = eM_s_audio_record;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_SD_VOICE:
      nextstate = eM_s_audio_main;
      *pTrans = eM_TYPE_IRPT;
      break;
    case M_EVT_TOMPLAY:
      nextstate = eM_s_audio_tomplay;
      *pTrans = eM_TYPE_IRPT;
      break;
    default:
      nextstate = eM_handlestdb_standby;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
static int m_set_firmware_updataGetNext(T_EVT_CODE event, M_TRANSTYPE *pTrans)
{
  M_STATES nextstate = SM_GetCurrentSM();
  switch(event)
  {
    case M_EVT_EXIT:
      *pTrans = eM_TYPE_RETURN;
      break;
    default:
      nextstate = eM_handleset_firmware_updata;
      *pTrans = eM_TYPE_SCALL;
      break;
  }
  return(nextstate);
}
/* ---------------------------------------------------------------------------------------*/
/*                    state array                                                         */
/* ---------------------------------------------------------------------------------------*/
#pragma arm section rodata = "_frequentcode_"
static M_STATESTRUCT* const m_statearray[] = 
{
 (M_STATESTRUCT* const)&m_s_init_system,
 (M_STATESTRUCT* const)&m_s_init_power_on,
 (M_STATESTRUCT* const)&m_set_firmware_updata,
 (M_STATESTRUCT* const)&m_s_stdb_standby,
 (M_STATESTRUCT* const)&m_s_pstn_phone,
 (M_STATESTRUCT* const)&m_s_pstn_ring,
 (M_STATESTRUCT* const)&m_s_pstn_dial,
 (M_STATESTRUCT* const)&m_s_pstn_connect,
 (M_STATESTRUCT* const)&m_s_bt_phone,
 (M_STATESTRUCT* const)&m_s_bt_player,
 (M_STATESTRUCT* const)&m_s_pub_switch_off,
 (M_STATESTRUCT* const)&m_s_pub_charger,
 (M_STATESTRUCT* const)&m_s_pub_pullout_sd,
 (M_STATESTRUCT* const)&m_s_pub_usb_detect,
 (M_STATESTRUCT* const)&m_s_pub_usbdisk,
 (M_STATESTRUCT* const)&m_s_pub_set_menu,
 (M_STATESTRUCT* const)&m_s_pub_standby,
 (M_STATESTRUCT* const)&m_preproc,
 (M_STATESTRUCT* const)&m_postproc,
 (M_STATESTRUCT* const)&m_s_audio_record,
 (M_STATESTRUCT* const)&m_s_audio_tomplay,
 (M_STATESTRUCT* const)&m_s_audio_main,
 (M_STATESTRUCT* const)&m_s_clk_music_play,
 (M_STATESTRUCT* const)&m_s_audio_abplay,
 (M_STATESTRUCT* const)&m_s_linein_play,
  (M_STATESTRUCT* const)&m_s_radio_player
};
#pragma arm section rodata
/* ---------------------------------------------------------------------------------------*/
/*                    efunction array                                                      */
/* ---------------------------------------------------------------------------------------*/
#pragma arm section rodata = "_bootcode1_"
static const _feHandle m_efuncarray[] = 
{
  ddaudctrlhandler,
  ddaudctrltimerhandler,
  ddlineinhandle,
  ddpubtimerhandler,
  ddsdcardhandler,
  ddusbdetecthandle,
  ddusbhostdiskhandler,
  ddusbouthandler,
  dduserkeyhandler
};
#pragma arm section rodata
/* ---------------------------------------------------------------------------------------*/
/*                    function array                                                      */
/* ---------------------------------------------------------------------------------------*/
#pragma arm section rodata = "_frequentcode_"
static _fHandle const m_funcarray[] = 
{
  handleaudio_abplay,
  handleaudio_main,
  handleaudio_record,
  handleaudio_tomplay,
  handlebt_phone,
  handlebt_player,
  handleclk_music_play,
  handleinit_power_on,
  handleinit_system,
  handlelinein_play,
  handlepstn_connect,
  handlepstn_dial,
  handlepstn_phone,
  handlepstn_ring,
  handlepub_charger,
  handlepub_pullout_sd,
  handlepub_set_menu,
  handlepub_standby,
  handlepub_switch_off,
  handlepub_usb_detect,
  handlepub_usbdisk,
  handleradio_player,
  handleset_firmware_updata,
  handlestdb_standby
};
#pragma arm section rodata
/* ---------------------------------------------------------------------------------------*/
/*                    porting lib functions                                                   */
/* ---------------------------------------------------------------------------------------*/
const _fHandle* SM_GetfHande(void)
{
	return m_funcarray;
}

const _feHandle* SM_GeteHandle(void)
{
	return m_efuncarray;
}

const M_STATESTRUCT** SM_GetStateArray(void)
{
	return (const M_STATESTRUCT**)m_statearray;
}

