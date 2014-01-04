# Microsoft Developer Studio Project File - Name="middleware" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=middleware - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "middleware_10C.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "middleware_10C.mak" CFG="middleware - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "middleware - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "middleware - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "middleware - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "middleware - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\include\\" /I "..\include\fwl\\" /I "..\include\lib\\" /I "..\interface\\" /I "..\source\\" /D "SUPORT_MUISE_EQ" /D "SUPPORT_VOICE_TIP" /D "SUPPORT_RADIO_RDA5876" /D "WIN32" /D "OS_WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D DEBUG=1 /D USE_CHIP_SERIES=3 /D NO_DISPLAY=1 /D STORAGE_USED=2 /D "SUPPORT_SDCARD" /D "SUPPORT_VOICE_PLAY" /D "SUPPORT_MUSIC_PLAY" /D "SUPPORT_AUDIO_RECORD" /D "AUD_SPEED_ARITHMETIC" /D "SUPPORT_RADIO" /D "SUPPORT_BLUETOOTH" /D "SUPPORT_A2DP_RESAMPLE_CONTROL_EN" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\..\platform\Middle\Library\mid_10C.lib"

!ENDIF 

# Begin Target

# Name "middleware - Win32 Release"
# Name "middleware - Win32 Debug"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Group "fwl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\fwl\file.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_blue.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Detect.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_FreqMgr.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Keypad.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_MicroTask.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Mount.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_osFS.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_osMalloc.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Radio.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Serial.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_System.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_Timer.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_usb_s_disk.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_usb_s_state.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_WaveIn.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\Fwl_WaveOut.h
# End Source File
# Begin Source File

SOURCE=..\include\fwl\mtdlib.h
# End Source File
# End Group
# Begin Group "lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\lib\aec_interface.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\audio_decoder.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_a2dp.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_avrcp.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_gap.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_hfp.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_lib_api.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\BA_SPP.h
# End Source File
# Begin Source File

SOURCE=..\interface\BA_user_msg.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\jdrv_decApi.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_audio_lib.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_avf_player.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_code_page.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_image_api.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_jdrv_api.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_media_lib.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\lib_media_recorder.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_medialib_global.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\Lib_medialib_struct.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\sdcodec.h
# End Source File
# Begin Source File

SOURCE=..\include\lib\unicode_api.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\include\anyka_types.h
# End Source File
# Begin Source File

SOURCE=..\include\Apl_Initialize.h
# End Source File
# Begin Source File

SOURCE=..\include\Eng_UStrPublic.h
# End Source File
# Begin Source File

SOURCE=..\include\Gbl_Global.h
# End Source File
# Begin Source File

SOURCE=..\include\Gbl_ImageRes.h
# End Source File
# Begin Source File

SOURCE=..\include\Gbl_Resource.h
# End Source File
# Begin Source File

SOURCE=..\include\log_image_view.h
# End Source File
# Begin Source File

SOURCE=..\include\vme.h
# End Source File
# Begin Source File

SOURCE=..\include\w_audioplay.h
# End Source File
# Begin Source File

SOURCE=..\include\w_winvme.h
# End Source File
# End Group
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\interface\Alarm_Common.h
# End Source File
# Begin Source File

SOURCE=..\interface\AlarmClock.h
# End Source File
# Begin Source File

SOURCE=..\interface\BtCtrl.h
# End Source File
# Begin Source File

SOURCE=..\interface\BtPhone.h
# End Source File
# Begin Source File

SOURCE=..\interface\BtPlayer.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_Button.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_Dialog.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_HorMenu.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_IconMenu.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_ListFile.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_ListMenu.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_MenuConfig.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_Progress.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_Public.h
# End Source File
# Begin Source File

SOURCE=..\interface\Ctrl_TopBar.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_CycBuf.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_DataConvert.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Debug.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Font.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Graph.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_ImageResDisp.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_loadMem.h
# End Source File
# Begin Source File

SOURCE=..\interface\eng_lunarcalendar.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Math.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Profile.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_String.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_String_UC.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Time.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_Tool.h
# End Source File
# Begin Source File

SOURCE=..\interface\Eng_USB.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_aud_play.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_file_com.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_pcm_player.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_pcm_record.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_radio_core.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_ram_res.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_Raw_RGB.h
# End Source File
# Begin Source File

SOURCE=..\interface\log_record.h
# End Source File
# Begin Source File

SOURCE=..\interface\Svc_MediaList.h
# End Source File
# End Group
# Begin Group "source"

# PROP Default_Filter ""
# Begin Group "alarm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\alarm\Alarm_Common.c
# End Source File
# Begin Source File

SOURCE=..\source\alarm\AlarmClock.c
# End Source File
# End Group
# Begin Group "ctrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_Button.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_Dialog.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_HorMenu.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_IconMenu.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_ListFile.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_ListMenu.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_MenuConfig.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_Progress.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\Ctrl_TopBar.c
# End Source File
# Begin Source File

SOURCE=..\source\ctrl\log_file_com.c
# End Source File
# End Group
# Begin Group "engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\engine\Eng_CycBuf.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Debug.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Font.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\eng_lunarcalendar.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Math.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Profile.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Time.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_Tool.c
# End Source File
# Begin Source File

SOURCE=..\source\engine\Eng_USB.c
# End Source File
# End Group
# Begin Group "FM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\FM\log_radio_core.c
# End Source File
# End Group
# Begin Group "graphic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\graphic\Eng_Graph.c
# End Source File
# Begin Source File

SOURCE=..\source\graphic\Eng_ImageResDisp.c
# End Source File
# Begin Source File

SOURCE=..\source\graphic\log_Raw_RGB.c
# End Source File
# End Group
# Begin Group "memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\memory\Eng_loadMem.c
# End Source File
# Begin Source File

SOURCE=..\source\memory\log_ram_res.c
# End Source File
# End Group
# Begin Group "mplayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\mplayer\audio_decoder.c
# End Source File
# Begin Source File

SOURCE=..\source\mplayer\log_aud_play.c
# End Source File
# Begin Source File

SOURCE=..\source\mplayer\log_pcm_player.c
# End Source File
# Begin Source File

SOURCE=..\source\mplayer\Svc_MediaList.c
# End Source File
# End Group
# Begin Group "record"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\record\log_pcm_record.c
# End Source File
# Begin Source File

SOURCE=..\source\record\log_record.c
# End Source File
# End Group
# Begin Group "string"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\string\Eng_DataConvert.c
# End Source File
# Begin Source File

SOURCE=..\source\string\Eng_String.c
# End Source File
# Begin Source File

SOURCE=..\source\string\Eng_String_UC.c
# End Source File
# End Group
# Begin Group "blue"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\blue\Eng_BtCtrl.c
# End Source File
# Begin Source File

SOURCE=..\source\blue\Eng_BtPhone.c
# End Source File
# Begin Source File

SOURCE=..\source\blue\Eng_BtPlayer.c
# End Source File
# End Group
# Begin Group "voice"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\voice\Eng_VoiceTip.c
# End Source File
# End Group
# End Group
# End Target
# End Project
