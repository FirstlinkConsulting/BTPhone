/**
 * @file    fwl_residentstr.h
 * @brief   define the string in resident code.
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  lx
 * @date    2013.4.9
 * @version 1.0
 */


#ifndef _FWL_RESIDENTSTR_H_
#define _FWL_RESIDENTSTR_H_


#ifndef __FILE__
    #define __FILE__    ""
#endif
#ifndef __LINE__
    #define __LINE__    0
#endif

//ERROR
#define ERROR_PROG                     "PG:"
#define ERROR_PROG_GET_PHIDEX          "PGI"
#define ERROR_PROG_SET_PHIDEX          "PSI"
#define ERROR_PROG_GET_MAPLIST         "PGT"
#define ERROR_PROG_RCV_BLOCK_BACKUP    "P1R"
#define ERROR_PROG_RCV_BLOCK_READ      "P2R"
#define ERROR_PROG_MARK_BADBLOCK       "PMK"
#define ERROR_PROG_LOAD                "P1L"
#define ERROR_PROG_RETRYLOAD           "P2L"
#define ERROR_PROG_WRITEBACK           "PWB"
#define ERROR_PROG_CHECK_WRITEAREA     "PWA"

#define ERROR_ABORT_AT_IRQMODE         "IR:"
#define ERROR_ABORT_PREMODE_PC         "PL:"
#define ERROR_ABORT_FAULTDATA_ADDR     "DA:"
#define ERROR_ABORT_STACK              "AP:"
#define ERROR_ABORT_PREMODE_LR         "UL:"

//SUCCESS
#define SUCCESS_PROG                   "Pgm OK"

#endif //_FWL_RESIDENTSTR_H_
