/**
 * @file    log_aud_ui_cfg.h
 * @brief   audio UI config parameters
 * @author  WuShanwei
 * @date    2008-05-19
 * @version 1.0
 * @emend_author  lx
 * @emend_date    2012-11-14
 */
#ifndef _LOG_AUD_UI_CFG_H_
#define _LOG_AUD_UI_CFG_H_

#include "Eng_ImageResDisp.h"

#define AUD_LINE_HIGH               16
#define AUD_LINE1_POS_Y             0
#define AUD_LINE2_POS_Y             16
#define AUD_LINE3_POS_Y             32
#define AUD_LINE4_POS_Y             48


/** UI parameters */
#if (USE_COLOR_LCD)
#if (LCD_TYPE == 0)
    /** element need config */
    #define AUD_ELE_ENABLE_STAT 1
    #define AUD_ELE_ENABLE_IDX  1
    #define AUD_ELE_ENABLE_TIME 1
    #define AUD_ELE_ENABLE_TIME_ALL 1
    #define AUD_ELE_ENABLE_ICON 1
    #define AUD_ELE_ENABLE_TYPE 1
    #define AUD_ELE_ENABLE_CYC  1
    #define AUD_ELE_ENABLE_TONE 1
    #define AUD_ELE_ENABLE_SAMP 1
    #define AUD_ELE_ENABLE_AB   1
    #define AUD_ELE_ENABLE_JUMP 1

    #define AUD_ELE_ENABLE_BAT  1
    #define AUD_ELE_EMANLE_PRGRS 1
    #define AUD_ELE_EMABLE_VOL   1
    #define AUD_ELE_EMABLE_FLD   1

    /** Display parameters */
    #define AUD_DISP_INFO_LYCRUN_POS_X      110
    #define AUD_DISP_INFO_LYCRUN_POS_Y      82
    #define AUD_DISP_INFO_LYCRUN_POS_LEN    8

    #define AUD_DISP_INFO_AUDNAME_POS_X     26
    #define AUD_DISP_INFO_AUDNAME_POS_Y     126
    #define AUD_DISP_INFO_AUDNAME_POS_LEN   102

    #define AUD_DISP_INFO_ICON_LEN          26
    #define AUD_DISP_INFO_POS_X             0
    #define AUD_DISP_INFO_POS_Y             0
    #define AUD_DISP_INFO_LEN               16*8
    #define AUD_DISP_INFO_HIGH              16

    #define AUD_DISP_INFO_SAMPRATE_POS_X    60
    #define AUD_DISP_INFO_SAMPRATE_POS_Y    144
    #define AUD_DISP_INFO_SAMICON_POS_X     75
    #define AUD_DISP_INFO_SAMICON_POS_Y     143

    #define AUD_DISP_INFO_ICON_POS_X        94
    #define AUD_DISP_INFO_ICON_POS_Y        143
    #define AUD_DISP_INFO_ICON_POS_LEN      34

    #define AUD_DISP_INFO_TYPE_POS_X        2
    #define AUD_DISP_INFO_TYPE_POS_Y        127
    #define AUD_DISP_INFO_TYPE_POS_LEN      23

    #define AUD_DISP_INFO_SAMPLERATE_POS_X      (AUD_DISP_INFO_TYPE_POS_X+2)
    #define AUD_DISP_INFO_SAMPLERATE_POS_Y      (AUD_DISP_INFO_TYPE_POS_Y)
    #define AUD_DISP_INFO_SAMPLERATE_POS_LEN    20

    #define AUD_DISP_INFO_TONE_POS_X        30
    #define AUD_DISP_INFO_TONE_POS_Y        2
    #define AUD_DISP_INFO_TONE_POS_LEN      18


    #define AUD_DISP_INFO_CYC_POS_X         54  
    #define AUD_DISP_INFO_CYC_POS_Y         2
    #define AUD_DISP_INFO_CYC_POS_LEN       14

    #define AUD_DISP_INFO_AB_POS_X          2
    #define AUD_DISP_INFO_AB_POS_Y          2
    #define AUD_DISP_INFO_AB_POS_LEN        (T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDABMODEA)
    #define AUD_DISP_INFO_AB_POS_HIGH       (T_U8)Eng_GetResImageHeight(eRES_IMAGE_AUDABMODEA)


    #define AUD_DISP_INFO_VOLICON_POS_X     74
    #define AUD_DISP_INFO_VOLICON_POS_Y     1
    #define AUD_DISP_INFO_VOLICON_POS_LEN   16
    #define AUD_DISP_INFI_VOLICON_POS_HIGH  16
    #define AUD_DISP_INGO_VOLVAL_POS_X      92
    #define AUD_DISP_INGO_VOLVAL_POS_Y      6
    #define AUD_DISP_INGO_VOLVAL_POS_LEN    10

    #define AUD_DISP_INFO_FLD_POS_X         2
    #define AUD_DISP_INFO_FLD_POS_Y         113
    #define AUD_DISP_INFO_FLD_POS_LEN       14

    #define AUD_DISP_INFO_FLDNAME_POS_X     16
    #define AUD_DISP_INFO_FLDNAME_POS_Y     110
    #define AUD_DISP_INFO_FLDNAME_POS_LEN   112
    #define AUD_DISP_INFO_FLDNAME_CLO       RGB_COLOR(28, 163, 255)

    #define AUD_DISP_INFO_BAT_POS_X         (T_U8)(AUD_LCD_WIDTH- Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0) - 3)    
    #define AUD_DISP_INFO_BAT_POS_Y         2
    #define AUD_DISP_INFO_BAT_POS_LEN       21

    #define AUD_DISP_INFO_STAT_POS_X        1
    #define AUD_DISP_INFO_STAT_POS_Y        145
    #define AUD_DISP_INFO_STAT_POS_LEN      14

    #define AUD_DISP_INFO_IDX_CUR_POS_X     90
    #define AUD_DISP_INFO_IDX_CUR_POS_Y     42
    #define AUD_DISP_INFO_IDX_CUR_LEN       15

    #define AUD_DISP_INFO_IDX_SLD_POS_X     105
    #define AUD_DISP_INFO_IDX_SLD_POS_Y     42
    #define AUD_DISP_INFO_IDX_SLD_LEN       5

    #define AUD_DISP_INFO_IDX_ALL_POS_X     110
    #define AUD_DISP_INFO_IDX_ALL_POS_Y     42
    #define AUD_DISP_INFO_IDX_ALL_LEN       15

    #define AUD_DISP_INFO_TIME_CUR_POS_X    4
    #define AUD_DISP_INFO_TIME_CUR_POS_Y    30
    #define AUD_DISP_INFO_TIME_CUR_LEN      16*2

    #define AUD_DISP_INFO_TIME_ALL_POS_X    PRGRS_DISP_POS_X
    #define AUD_DISP_INFO_TIME_ALL_POS_Y    144
    #define AUD_DISP_INFO_TIME_ALL_LEN      AUD_DISP_INFO_TIME_CUR_LEN

    #define AUD_DISP_INFO_JUMP_POS_X        (T_U8)((AUD_LCD_WIDTH - Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_POS_Y        (T_U8)((AUD_LCD_HIGH - Eng_GetResImageHeight(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_LEN          (T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1)

    #define AUD_DISP_INFO_UPDATING_X        38
    #define AUD_DISP_INFO_UPDATING_Y        76
    #define AUD_DISP_INFO_UPDATING_LEN      30

    #define AUD_DISP_INFO_FERRICON_X        9
    #define AUD_DISP_INFO_FERR_ICON_Y       60
    #define AUD_DISP_INFO_FERR_COLOR        RGB_COLOR(255,166,0)
    #define AUD_DISP_INFO_FERROFT_H         23
    #define AUD_DISP_INFO_FERROFT_V         46
    #define AUD_DISP_INFO_FILEERROR_X       32
    #define AUD_DISP_INFO_FILEERROR_Y       106
    #define AUD_DISP_INFO_FILEERROR_LEN     30
    #define AUD_DISP_INFO_NOFILE_X          43
    #define AUD_DISP_INFO_NOFILE_Y          AUD_LINE3_POS_Y
    #define AUD_DISP_INFO_NOFILE_LEN        30

    #define AUD_DISP_INFO_NOMUICON_X        9
    #define AUD_DISP_INFO_NOMUICON_Y        46
    #define AUD_DISP_INFO_NOMUSTR_X         15
    #define AUD_DISP_INFO_NOMUSTR_Y         76
    #define AUD_DISP_INFO_NOMUOFT_H         6
    #define AUD_DISP_INFO_NOMUOFT_V         30
    #define AUD_DISP_INFO_NOMU_COLOR        RGB_COLOR(255, 166, 0)

    #define AUD_ALBUM_LEFT                  0
    #define AUD_ALBUM_RIGHT                 AUD_LCD_WIDTH
    #define AUD_ALBUM_TOP                   0
    #define AUD_ALBUM_BOTTOM                AUD_LCD_HIGH

    /** LYRIC display position  */
    #define LYRIC_DISPLAY_TITLEPOS_X        6
    #define LYRIC_DISPLAY_TITLEPOS_Y        4
    #define LYRIC_DISPLAY_TITLECOLOR        CLR_WHITE
    #define LYRIC_DISPLAY_POS_X             6
    #define LYRIC_DISPLAY_POS_Y             32
    #define LYRIC_DISPLAY_COLOR             RGB_COLOR(128, 128, 128)
    #define LYRIC_DISPLAY_CURPOS_X          6
    #define LYRIC_DISPLAY_CURPOS_Y          77
    #define LYRIC_DISPLAY_CURCOLOR          RGB_COLOR(80, 149, 234)
    #define LYRIC_DISPLAY_POSTPOS_X         6
    #define LYRIC_DISPLAY_POSTPOS_Y         122
    #define LYRIC_DISPLAY_LEN               (AUD_LCD_WIDTH - 6)
    #define LYRIC_DISPLAY_HIGH              (AUD_LCD_LINE_HIGH*2)
    #define LYRIC_DISPLAY_LINE_NUM          2
    #define LYRIC_DISPLAY_INFO_NUM          2
    #define LYRIC_DISPLAY_AVER_CHAR         32
    #define LYRIC_DISPLAY_SEG_NUM           3

    /** progress display position*/
    #define PRGRS_DISP_POS_X                18
    #define PRGRS_DISP_POS_Y                152
    #define IMAGE_AUD_SCHEDU2               eRES_IMAGE_SCHEDU2 
    #define IMAGE_AUD_SCHEDU1               eRES_IMAGE_SCHEDU1 
    #define PRGRS_DISP_LEN                  Eng_GetResImageWidth(IMAGE_AUD_SCHEDU1)
    #define PRGRS_DISP_HIGH                 Eng_GetResImageHeight(IMAGE_AUD_SCHEDU1)
#endif
#if (LCD_TYPE == 3) //for 240x320
#if (LCD_HORIZONTAL == 1)
    /** element need config */
    #define AUD_ELE_ENABLE_STAT 1
    #define AUD_ELE_ENABLE_IDX  1
    #define AUD_ELE_ENABLE_TIME 1
    #define AUD_ELE_ENABLE_TIME_ALL 1
    #define AUD_ELE_ENABLE_ICON 1
    #define AUD_ELE_ENABLE_TYPE 1
    #define AUD_ELE_ENABLE_CYC  1
    #define AUD_ELE_ENABLE_TONE 1
    #define AUD_ELE_ENABLE_SAMP 1
    #define AUD_ELE_ENABLE_AB   1
    #define AUD_ELE_ENABLE_JUMP 1

    #define AUD_ELE_ENABLE_BAT  1
    #define AUD_ELE_EMANLE_PRGRS 1
    #define AUD_ELE_EMABLE_VOL   1
    #define AUD_ELE_EMABLE_FLD   0

    /** Display parameters */
    #define AUD_DISP_INFO_MUSIC_NAME_POS_X      16
    #define AUD_DISP_INFO_MUSIC_NAME_POS_Y      29
    #define AUD_DISP_INFO_ALBUM_NAME_POS_X      16
    #define AUD_DISP_INFO_ALBUM_NAME_POS_Y      AUD_DISP_INFO_MUSIC_NAME_POS_Y + FONT_WIDTH
    #define AUD_DISP_INFO_SAMPLE_RATE_POS_X     16
    #define AUD_DISP_INFO_SAMPLE_RATE_POS_Y     AUD_DISP_INFO_ALBUM_NAME_POS_Y + FONT_WIDTH
    
    #define AUD_DISP_INFO_LYCRUN_POS_X          182
    #define AUD_DISP_INFO_LYCRUN_POS_Y          200
    #define AUD_DISP_INFO_LYCRUN_POS_LEN        68

    #define AUD_DISP_INFO_AUDNAME_POS_X         144
    #define AUD_DISP_INFO_AUDNAME_POS_Y         29
    #define AUD_DISP_INFO_AUDNAME_POS_LEN       ((T_U16)(AUD_LCD_WIDTH - AUD_DISP_INFO_AUDNAME_POS_X - 1))

    #define AUD_DISP_INFO_ICON_EQ_POS_X         283
    #define AUD_DISP_INFO_ICON_EQ_POS_Y         201

    #define AUD_DISP_INFO_ICON_PREV_POS_X       110
    #define AUD_DISP_INFO_ICON_PREV_POS_Y       201
    #define AUD_DISP_INFO_ICON_PREV_POS_X1      142
    #define AUD_DISP_INFO_ICON_PREV_POS_Y1      230

    #define AUD_DISP_INFO_ICON_NEXT_POS_X       189
    #define AUD_DISP_INFO_ICON_NEXT_POS_Y       201
    #define AUD_DISP_INFO_ICON_NEXT_POS_X1      221
    #define AUD_DISP_INFO_ICON_NEXT_POS_Y1      230

    #define AUD_DISP_INFO_ICON_LEN              26
    #define AUD_DISP_INFO_POS_X                 0
    #define AUD_DISP_INFO_POS_Y                 0
    #define AUD_DISP_INFO_LEN                   16*8
    #define AUD_DISP_INFO_HIGH                  16*2

    #define AUD_DISP_INFO_SAMPRATE_POS_X        (T_POS)(AUD_DISP_INFO_SAMPLE_RATE_POS_X + GetStringDispWidth(GetLangCodePage(gb.Lang), (T_U8*)eRES_STR_AUD_SAMPLE_RATE, 0))
    #define AUD_DISP_INFO_SAMPRATE_POS_Y        (T_POS)(AUD_DISP_INFO_SAMPLE_RATE_POS_Y)
    #define AUD_DISP_INFO_SAMICON_POS_X         225
    #define AUD_DISP_INFO_SAMICON_POS_Y         214

    #define AUD_DISP_INFO_ICON_POS_X            285
    #define AUD_DISP_INFO_ICON_POS_Y            215
    #define AUD_DISP_INFO_ICON_POS_LEN          34*2

    #define AUD_DISP_INFO_TYPE_POS_X            2
    #define AUD_DISP_INFO_TYPE_POS_Y            190
    #define AUD_DISP_INFO_TYPE_POS_LEN          23*2

    #define AUD_DISP_INFO_SAMPLERATE_POS_X      112//(AUD_DISP_INFO_TYPE_POS_X+2)
    #define AUD_DISP_INFO_SAMPLERATE_POS_Y      200//(AUD_DISP_INFO_TYPE_POS_Y)
    #define AUD_DISP_INFO_SAMPLERATE_POS_LEN    20

    #define AUD_DISP_INFO_TONE_POS_X        130
    #define AUD_DISP_INFO_TONE_POS_Y        4
    #define AUD_DISP_INFO_TONE_POS_LEN      18*2


    #define AUD_DISP_INFO_CYC_POS_X         250 
    #define AUD_DISP_INFO_CYC_POS_Y         201
    #define AUD_DISP_INFO_CYC_POS_LEN       14*2

    #define AUD_DISP_INFO_AB_POS_X          2
    #define AUD_DISP_INFO_AB_POS_Y          2
    #define AUD_DISP_INFO_AB_POS_LEN        (T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDABMODEA)
    #define AUD_DISP_INFO_AB_POS_HIGH       (T_U8)Eng_GetResImageHeight(eRES_IMAGE_AUDABMODEA)

    #define AUD_DISP_INFO_VOL_LEVEL         11

    #define AUD_DISP_INFO_VOLICON_POS_X     8
    #define AUD_DISP_INFO_VOLICON_POS_Y     201
    #define AUD_DISP_INFO_VOLICON_POS_LEN   16*2
    #define AUD_DISP_INFI_VOLICON_POS_HIGH  30
    #define AUD_DISP_INGO_VOLVAL_POS_X      50
    #define AUD_DISP_INGO_VOLVAL_POS_Y      205
    #define AUD_DISP_INGO_VOLVAL_POS_LEN    10*2

    #define AUD_DISP_INFO_FLD_POS_X         2
    #define AUD_DISP_INFO_FLD_POS_Y         83*2
    #define AUD_DISP_INFO_FLD_POS_LEN       14*2

    #define AUD_DISP_INFO_FLDNAME_POS_X     129
    #define AUD_DISP_INFO_FLDNAME_POS_Y     170
    #define AUD_DISP_INFO_FLDNAME_POS_LEN   180
    #define AUD_DISP_INFO_FLDNAME_CLO       RGB_COLOR(28, 163, 255)

    #define AUD_DISP_INFO_BAT_POS_X         275//(T_U8)(AUD_LCD_WIDTH- Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0) - 3)   
    #define AUD_DISP_INFO_BAT_POS_Y         2
    #define AUD_DISP_INFO_BAT_POS_LEN       45

    #define AUD_DISP_INFO_STAT_POS_X        142
    #define AUD_DISP_INFO_STAT_POS_Y        193
    #define AUD_DISP_INFO_STAT_POS_X1       186
    #define AUD_DISP_INFO_STAT_POS_Y1       239
    #define AUD_DISP_INFO_STAT_POS_LEN      46

    #define AUD_DISP_INFO_IDX_CUR_POS_X     168
    #define AUD_DISP_INFO_IDX_CUR_POS_Y     65
    #define AUD_DISP_INFO_IDX_CUR_LEN       15*2

    #define AUD_DISP_INFO_IDX_SLD_POS_X     198
    #define AUD_DISP_INFO_IDX_SLD_POS_Y     65
    #define AUD_DISP_INFO_IDX_SLD_LEN       22

    #define AUD_DISP_INFO_IDX_ALL_POS_X     209
    #define AUD_DISP_INFO_IDX_ALL_POS_Y     65
    #define AUD_DISP_INFO_IDX_ALL_LEN       15*2

    #define AUD_DISP_INFO_TIME_CUR_POS_X    17
    #define AUD_DISP_INFO_TIME_CUR_POS_Y    168
    #define AUD_DISP_INFO_TIME_CUR_LEN      16*2

    #define AUD_DISP_INFO_TIME_ALL_POS_X    256
    #define AUD_DISP_INFO_TIME_ALL_POS_Y    168
    #define AUD_DISP_INFO_TIME_ALL_LEN      50

    #define AUD_DISP_INFO_JUMP_POS_X        118//(T_U8)((AUD_LCD_WIDTH - Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_POS_Y        79//(T_U8)((AUD_LCD_HIGH - Eng_GetResImageHeight(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_LEN          185//(T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1)

    #define AUD_DISP_INFO_UPDATING_X        38
    #define AUD_DISP_INFO_UPDATING_Y        160
    #define AUD_DISP_INFO_UPDATING_LEN      30

    #define AUD_DISP_INFO_FERRICON_X        80
    #define AUD_DISP_INFO_FERR_ICON_Y       60
    #define AUD_DISP_INFO_FERR_COLOR        RGB_COLOR(255,166,0)
    #define AUD_DISP_INFO_FERROFT_H         23
    #define AUD_DISP_INFO_FERROFT_V         46
    #define AUD_DISP_INFO_FILEERROR_X       32
    #define AUD_DISP_INFO_FILEERROR_Y       106
    #define AUD_DISP_INFO_FILEERROR_LEN     30
    #define AUD_DISP_INFO_NOFILE_X          43
    #define AUD_DISP_INFO_NOFILE_Y          AUD_LINE3_POS_Y
    #define AUD_DISP_INFO_NOFILE_LEN        30

    #define AUD_DISP_INFO_NOMUICON_X        9
    #define AUD_DISP_INFO_NOMUICON_Y        46
    #define AUD_DISP_INFO_NOMUSTR_X         15
    #define AUD_DISP_INFO_NOMUSTR_Y         76
    #define AUD_DISP_INFO_NOMUOFT_H         6
    #define AUD_DISP_INFO_NOMUOFT_V         30
    #define AUD_DISP_INFO_NOMU_COLOR        RGB_COLOR(255, 166, 0)

    #define AUD_ALBUM_LEFT                  18
    #define AUD_ALBUM_RIGHT                 93
    #define AUD_ALBUM_TOP                   79
    #define AUD_ALBUM_BOTTOM                154

    #define AUD_VIDEO_PLAY_ICON_POS_X       145
    #define AUD_VIDEO_PLAY_ICON_POS_Y       105

    /** LYRIC display position  */
    #define LYRIC_DISPLAY_TITLEPOS_X        10
    #define LYRIC_DISPLAY_TITLEPOS_Y        4*2
    #define LYRIC_DISPLAY_TITLECOLOR        CLR_WHITE
    #define LYRIC_DISPLAY_POS_X             10
    #define LYRIC_DISPLAY_POS_Y             60
    #define LYRIC_DISPLAY_COLOR             RGB_COLOR(128, 128, 128)
    #define LYRIC_DISPLAY_CURPOS_X          10
    #define LYRIC_DISPLAY_CURPOS_Y          120
    #define LYRIC_DISPLAY_CURCOLOR          RGB_COLOR(80, 149, 234)
    #define LYRIC_DISPLAY_POSTPOS_X         10
    #define LYRIC_DISPLAY_POSTPOS_Y         180
    #define LYRIC_DISPLAY_LEN               (AUD_LCD_WIDTH - 10)
    #define LYRIC_DISPLAY_HIGH              (AUD_LCD_LINE_HIGH*2)
    #define LYRIC_DISPLAY_LINE_NUM          2
    #define LYRIC_DISPLAY_INFO_NUM          2
    #define LYRIC_DISPLAY_AVER_CHAR         32
    #define LYRIC_DISPLAY_SEG_NUM           3

    /** progress display position*/
    #define PRGRS_DISP_POS_X                18  
    #define PRGRS_DISP_POS_Y                165
    #define IMAGE_AUD_SCHEDU2               eRES_IMAGE_AUDPRGRSBACK 
    #define IMAGE_AUD_SCHEDU1               eRES_IMAGE_AUDPRGRS 
    #define PRGRS_DISP_LEN                  Eng_GetResImageWidth(IMAGE_AUD_SCHEDU1)
    #define PRGRS_DISP_HIGH                 Eng_GetResImageHeight(IMAGE_AUD_SCHEDU1)

#else
    /** element need config */
    #define AUD_ELE_ENABLE_STAT 1
    #define AUD_ELE_ENABLE_IDX  1
    #define AUD_ELE_ENABLE_TIME 1
    #define AUD_ELE_ENABLE_TIME_ALL 1
    #define AUD_ELE_ENABLE_ICON 1
    #define AUD_ELE_ENABLE_TYPE 1
    #define AUD_ELE_ENABLE_CYC  1
    #define AUD_ELE_ENABLE_TONE 1
    #define AUD_ELE_ENABLE_SAMP 1
    #define AUD_ELE_ENABLE_AB   1
    #define AUD_ELE_ENABLE_JUMP 1

    #define AUD_ELE_ENABLE_BAT  1
    #define AUD_ELE_EMANLE_PRGRS 1
    #define AUD_ELE_EMABLE_VOL   1
    #define AUD_ELE_EMABLE_FLD   1

    /** Display parameters */
    #define AUD_DISP_INFO_LYCRUN_POS_X      182
    #define AUD_DISP_INFO_LYCRUN_POS_Y      282
    #define AUD_DISP_INFO_LYCRUN_POS_LEN    68

    #define AUD_DISP_INFO_AUDNAME_POS_X     50
    #define AUD_DISP_INFO_AUDNAME_POS_Y     252
    #define AUD_DISP_INFO_AUDNAME_POS_LEN   190

    #define AUD_DISP_INFO_ICON_LEN          26
    #define AUD_DISP_INFO_POS_X             0
    #define AUD_DISP_INFO_POS_Y             0
    #define AUD_DISP_INFO_LEN               16*8
    #define AUD_DISP_INFO_HIGH              16*2

    #define AUD_DISP_INFO_SAMPRATE_POS_X    110
    #define AUD_DISP_INFO_SAMPRATE_POS_Y    286
    #define AUD_DISP_INFO_SAMICON_POS_X     145
    #define AUD_DISP_INFO_SAMICON_POS_Y     285

    #define AUD_DISP_INFO_ICON_POS_X        200
    #define AUD_DISP_INFO_ICON_POS_Y        285
    #define AUD_DISP_INFO_ICON_POS_LEN      34*2

    #define AUD_DISP_INFO_TYPE_POS_X        2
    #define AUD_DISP_INFO_TYPE_POS_Y        252
    #define AUD_DISP_INFO_TYPE_POS_LEN      23*2

    #define AUD_DISP_INFO_SAMPLERATE_POS_X      112//(AUD_DISP_INFO_TYPE_POS_X+2)
    #define AUD_DISP_INFO_SAMPLERATE_POS_Y      284//(AUD_DISP_INFO_TYPE_POS_Y)
    #define AUD_DISP_INFO_SAMPLERATE_POS_LEN    20

    #define AUD_DISP_INFO_TONE_POS_X        53
    #define AUD_DISP_INFO_TONE_POS_Y        2
    #define AUD_DISP_INFO_TONE_POS_LEN      18*2


    #define AUD_DISP_INFO_CYC_POS_X         102 
    #define AUD_DISP_INFO_CYC_POS_Y         2
    #define AUD_DISP_INFO_CYC_POS_LEN       14*2

    #define AUD_DISP_INFO_AB_POS_X          2
    #define AUD_DISP_INFO_AB_POS_Y          2
    #define AUD_DISP_INFO_AB_POS_LEN        (T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDABMODEA)
    #define AUD_DISP_INFO_AB_POS_HIGH       (T_U8)Eng_GetResImageHeight(eRES_IMAGE_AUDABMODEA)


    #define AUD_DISP_INFO_VOLICON_POS_X     139
    #define AUD_DISP_INFO_VOLICON_POS_Y     1*2
    #define AUD_DISP_INFO_VOLICON_POS_LEN   16*2
    #define AUD_DISP_INFI_VOLICON_POS_HIGH  30
    #define AUD_DISP_INGO_VOLVAL_POS_X      173
    #define AUD_DISP_INGO_VOLVAL_POS_Y      6
    #define AUD_DISP_INGO_VOLVAL_POS_LEN    10*2

    #define AUD_DISP_INFO_FLD_POS_X         2
    #define AUD_DISP_INFO_FLD_POS_Y         113*2
    #define AUD_DISP_INFO_FLD_POS_LEN       14*2

    #define AUD_DISP_INFO_FLDNAME_POS_X     42
    #define AUD_DISP_INFO_FLDNAME_POS_Y     230
    #define AUD_DISP_INFO_FLDNAME_POS_LEN   180
    #define AUD_DISP_INFO_FLDNAME_CLO       RGB_COLOR(28, 163, 255)

    #define AUD_DISP_INFO_BAT_POS_X         194//(T_U8)(AUD_LCD_WIDTH- Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0) - 3)   
    #define AUD_DISP_INFO_BAT_POS_Y         2
    #define AUD_DISP_INFO_BAT_POS_LEN       45

    #define AUD_DISP_INFO_STAT_POS_X        2
    #define AUD_DISP_INFO_STAT_POS_Y        287
    #define AUD_DISP_INFO_STAT_POS_LEN      28

    #define AUD_DISP_INFO_IDX_CUR_POS_X     168
    #define AUD_DISP_INFO_IDX_CUR_POS_Y     81
    #define AUD_DISP_INFO_IDX_CUR_LEN       15*2

    #define AUD_DISP_INFO_IDX_SLD_POS_X     198
    #define AUD_DISP_INFO_IDX_SLD_POS_Y     81
    #define AUD_DISP_INFO_IDX_SLD_LEN       22

    #define AUD_DISP_INFO_IDX_ALL_POS_X     209
    #define AUD_DISP_INFO_IDX_ALL_POS_Y     81
    #define AUD_DISP_INFO_IDX_ALL_LEN       15*2

    #define AUD_DISP_INFO_TIME_CUR_POS_X    5
    #define AUD_DISP_INFO_TIME_CUR_POS_Y    53
    #define AUD_DISP_INFO_TIME_CUR_LEN      16*2

    #define AUD_DISP_INFO_TIME_ALL_POS_X    37
    #define AUD_DISP_INFO_TIME_ALL_POS_Y    286
    #define AUD_DISP_INFO_TIME_ALL_LEN      50

    #define AUD_DISP_INFO_JUMP_POS_X        30//(T_U8)((AUD_LCD_WIDTH - Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_POS_Y        110//(T_U8)((AUD_LCD_HIGH - Eng_GetResImageHeight(eRES_IMAGE_AUDUIJUMPEFF1))/2)
    #define AUD_DISP_INFO_JUMP_LEN          180//(T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDUIJUMPEFF1)

    #define AUD_DISP_INFO_UPDATING_X        38
    #define AUD_DISP_INFO_UPDATING_Y        160
    #define AUD_DISP_INFO_UPDATING_LEN      30

    #define AUD_DISP_INFO_FERRICON_X        65
    #define AUD_DISP_INFO_FERR_ICON_Y       130
    #define AUD_DISP_INFO_FERR_COLOR        RGB_COLOR(255,166,0)
    #define AUD_DISP_INFO_FERROFT_H         23
    #define AUD_DISP_INFO_FERROFT_V         46
    #define AUD_DISP_INFO_FILEERROR_X       32
    #define AUD_DISP_INFO_FILEERROR_Y       165
    #define AUD_DISP_INFO_FILEERROR_LEN     30
    #define AUD_DISP_INFO_NOFILE_X          43
    #define AUD_DISP_INFO_NOFILE_Y          AUD_LINE3_POS_Y
    #define AUD_DISP_INFO_NOFILE_LEN        30

    #define AUD_DISP_INFO_NOMUICON_X        9
    #define AUD_DISP_INFO_NOMUICON_Y        46
    #define AUD_DISP_INFO_NOMUSTR_X         15
    #define AUD_DISP_INFO_NOMUSTR_Y         76
    #define AUD_DISP_INFO_NOMUOFT_H         6
    #define AUD_DISP_INFO_NOMUOFT_V         30
    #define AUD_DISP_INFO_NOMU_COLOR        RGB_COLOR(255, 166, 0)

    #define AUD_ALBUM_LEFT                  0
    #define AUD_ALBUM_RIGHT                 AUD_LCD_WIDTH
    #define AUD_ALBUM_TOP                   0
    #define AUD_ALBUM_BOTTOM                AUD_LCD_HIGH

    /** LYRIC display position  */
    #define LYRIC_DISPLAY_TITLEPOS_X        6
    #define LYRIC_DISPLAY_TITLEPOS_Y        4*2
    #define LYRIC_DISPLAY_TITLECOLOR        CLR_WHITE
    #define LYRIC_DISPLAY_POS_X             6
    #define LYRIC_DISPLAY_POS_Y             35*2
    #define LYRIC_DISPLAY_COLOR             RGB_COLOR(128, 128, 128)
    #define LYRIC_DISPLAY_CURPOS_X          6
    #define LYRIC_DISPLAY_CURPOS_Y          160
    #define LYRIC_DISPLAY_CURCOLOR          RGB_COLOR(80, 149, 234)
    #define LYRIC_DISPLAY_POSTPOS_X         6
    #define LYRIC_DISPLAY_POSTPOS_Y         250
    #define LYRIC_DISPLAY_LEN               (AUD_LCD_WIDTH - 6)
    #define LYRIC_DISPLAY_HIGH              (AUD_LCD_LINE_HIGH*2)
    #define LYRIC_DISPLAY_LINE_NUM          2
    #define LYRIC_DISPLAY_INFO_NUM          2
    #define LYRIC_DISPLAY_AVER_CHAR         32
    #define LYRIC_DISPLAY_SEG_NUM           3

    /** progress display position*/
    #define PRGRS_DISP_POS_X                40
    #define PRGRS_DISP_POS_Y                298
    #define IMAGE_AUD_SCHEDU2               eRES_IMAGE_SCHEDU2 
    #define IMAGE_AUD_SCHEDU1               eRES_IMAGE_SCHEDU1
    #define PRGRS_DISP_LEN                  Eng_GetResImageWidth(IMAGE_AUD_SCHEDU1)
    #define PRGRS_DISP_HIGH                 Eng_GetResImageHeight(IMAGE_AUD_SCHEDU1)
#endif
#endif
#else  //BW LCD
/** UI parameters */

    /** element need config */
    #define AUD_ELE_ENABLE_STAT 0
    #define AUD_ELE_ENABLE_IDX  1
    #define AUD_ELE_ENABLE_TIME 1
    #define AUD_ELE_ENABLE_TIME_ALL 1
    #define AUD_ELE_ENABLE_JUMP 1
    #define AUD_ELE_ENABLE_ICON 1
    #define AUD_ELE_ENABLE_TYPE 1
    #define AUD_ELE_ENABLE_CYC  1
    #define AUD_ELE_ENABLE_TONE 1
    #define AUD_ELE_ENABLE_SAMP 1
    #define AUD_ELE_ENABLE_AB   1
    #define AUD_ELE_ENABLE_BAT  1

    #define AUD_ELE_EMANLE_PRGRS 0
    #define AUD_ELE_EMABLE_VOL   0
    #define AUD_ELE_EMABLE_FLD   0

    /** Display parameters */
    #define AUD_DISP_INFO_ICON_LEN          26
    #define AUD_DISP_INFO_POS_X             0
    #define AUD_DISP_INFO_POS_Y             0
    #define AUD_DISP_INFO_LEN               16*8
    #define AUD_DISP_INFO_HIGH              16

    #define AUD_DISP_INFO_LYCRUN_POS_X      100
    #define AUD_DISP_INFO_LYCRUN_POS_Y      AUD_LINE1_POS_Y
    #define AUD_DISP_INFO_LYCRUN_POS_LEN    16

    #define AUD_DISP_INFO_AUDNAME_POS_X     0
    #define AUD_DISP_INFO_AUDNAME_POS_Y     AUD_LINE3_POS_Y
    #define AUD_DISP_INFO_AUDNAME_POS_LEN   AUD_LCD_WIDTH

    #define AUD_DISP_INFO_ICON_POS_X        0
    #define AUD_DISP_INFO_ICON_POS_Y        AUD_LINE4_POS_Y
    #define AUD_DISP_INFO_ICON_POS_LEN      14

    #define AUD_DISP_INFO_TYPE_POS_X        (AUD_DISP_INFO_ICON_POS_X+AUD_DISP_INFO_ICON_POS_LEN+2) 
    #define AUD_DISP_INFO_TYPE_POS_Y        AUD_LINE4_POS_Y
    #define AUD_DISP_INFO_TYPE_POS_LEN      21

    #define AUD_DISP_INFO_SAMPRATE_POS_X    (AUD_DISP_INFO_TYPE_POS_X+2)
    #define AUD_DISP_INFO_SAMPRATE_POS_Y    (AUD_LINE4_POS_Y)
    #define AUD_DISP_INFO_SAMPRATE_POS_LEN  20

    #define AUD_DISP_INFO_TONE_POS_X        (AUD_DISP_INFO_TYPE_POS_X+AUD_DISP_INFO_TYPE_POS_LEN+2)
    #define AUD_DISP_INFO_TONE_POS_Y        AUD_LINE4_POS_Y
    #define AUD_DISP_INFO_TONE_POS_LEN      18

    #define AUD_DISP_INFO_CYC_POS_X         (AUD_DISP_INFO_TONE_POS_X+AUD_DISP_INFO_TONE_POS_LEN+2) 
    #define AUD_DISP_INFO_CYC_POS_Y         AUD_LINE4_POS_Y
    #define AUD_DISP_INFO_CYC_POS_LEN       14

    #define AUD_DISP_INFO_JUMP_POS_X        (AUD_DISP_INFO_CYC_POS_X+AUD_DISP_INFO_CYC_POS_LEN)
    #define AUD_DISP_INFO_JUMP_POS_Y        AUD_LINE4_POS_Y
    #define AUD_DISP_INFO_JUMP_LEN          (AUD_LCD_WIDTH-AUD_DISP_INFO_JUMP_POS_X)

    #define AUD_DISP_INFO_STAT_POS_X        0
    #define AUD_DISP_INFO_STAT_POS_Y        0   
    #define AUD_DISP_INFO_STAT_POS_LEN      0

    #define AUD_DISP_INFO_AB_POS_X          90
    #define AUD_DISP_INFO_AB_POS_Y          AUD_LINE2_POS_Y
    #define AUD_DISP_INFO_AB_POS_LEN        (T_U8)Eng_GetResImageWidth(eRES_IMAGE_AUDABMODEA)
    #define AUD_DISP_INFO_AB_POS_HIGH       (T_U8)Eng_GetResImageHeight(eRES_IMAGE_AUDABMODEA)


    #define AUD_DISP_INFO_IDX_CUR_POS_X     0
    #define AUD_DISP_INFO_IDX_CUR_POS_Y     AUD_LINE1_POS_Y
    #define AUD_DISP_INFO_IDX_CUR_LEN       24

    #define AUD_DISP_INFO_IDX_ALL_POS_X     0
    #define AUD_DISP_INFO_IDX_ALL_POS_Y     AUD_LINE2_POS_Y
    #define AUD_DISP_INFO_IDX_ALL_LEN       AUD_DISP_INFO_IDX_CUR_LEN

    #define AUD_DISP_INFO_TIME_CUR_POS_X    (AUD_DISP_INFO_IDX_ALL_POS_X+AUD_DISP_INFO_IDX_CUR_LEN+4)
    #define AUD_DISP_INFO_TIME_CUR_POS_Y    AUD_LINE1_POS_Y
    #define AUD_DISP_INFO_TIME_CUR_LEN      16*4

    #define AUD_DISP_INFO_TIME_ALL_POS_X    AUD_DISP_INFO_TIME_CUR_POS_X
    #define AUD_DISP_INFO_TIME_ALL_POS_Y    AUD_LINE2_POS_Y
    #define AUD_DISP_INFO_TIME_ALL_LEN      AUD_DISP_INFO_TIME_CUR_LEN

    #define AUD_DISP_INFO_BAT_POS_X         (T_U8)(AUD_LCD_WIDTH- Eng_GetResImageWidth(eRES_IMAGE_AUDBATL0))
    #define AUD_DISP_INFO_BAT_POS_Y         AUD_LINE1_POS_Y
    #define AUD_DISP_INFO_BAT_POS_LEN       26

    #define AUD_DISP_INFO_UPDATING_X        30
    #define AUD_DISP_INFO_UPDATING_Y        AUD_LINE2_POS_Y+8
    #define AUD_DISP_INFO_UPDATING_LEN      30
    #define AUD_DISP_INFO_FILEERROR_X       36
    #define AUD_DISP_INFO_FILEERROR_Y       AUD_LINE2_POS_Y+8
    #define AUD_DISP_INFO_FILEERROR_LEN     30
    #define AUD_DISP_INFO_NOFILE_X          36
    #define AUD_DISP_INFO_NOFILE_Y          AUD_LINE3_POS_Y
    #define AUD_DISP_INFO_NOFILE_LEN        30

    /** LYRIC display position  */
    #define LYRIC_DISPLAY_POS_X             0
    #define LYRIC_DISPLAY_POS_Y             16
    #define LYRIC_DISPLAY_COLOR             CLR_WHITE
    #define LYRIC_DISPLAY_LEN               AUD_LCD_WIDTH
    #define LYRIC_DISPLAY_HIGH              AUD_LCD_LINE_HIGH*2
    #define LYRIC_DISPLAY_LINE_NUM          2
    #define LYRIC_DISPLAY_INFO_NUM          2
    #define LYRIC_DISPLAY_AVER_CHAR         32
    #define LYRIC_DISPLAY_SEG_NUM           1

#endif  //USE_COLOR_LCD

#endif //#define _S_AUDIO_UI_CFG_H_
