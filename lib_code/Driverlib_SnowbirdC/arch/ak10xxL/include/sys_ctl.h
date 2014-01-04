/*******************************************************************************
 * @file    sys_ctl.h
 * @brief   system control register bits define file
 * Copyright (C) 2012 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author  zhanggaoxin
 * @date    2012-11-22
 * @version 1.0
 * @ref     AK1180 technical manual.
*******************************************************************************/
#ifndef __SYS_CTL_H__
#define __SYS_CTL_H__


typedef struct
{
    T_U32 m_in1;
    T_U32 m_in2;
    T_U32 m_out1;
    T_U32 m_out2;
    T_U32 m_dir1;
    T_U32 m_dir2;
    T_U32 m_intp1;
    T_U32 m_intp2;
    T_U32 m_inte1;
    T_U32 m_inte2;
    T_U32 m_pud1;
    T_U32 m_pud2;
    T_U32 m_sysctl;
    T_U32 m_share_pin;
    T_U32 m_timer2_cnt;
} T_REG_BACKUP;


#endif //#ifndef __SYS_CTL_H__

