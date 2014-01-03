#ifdef OS_WIN32
#ifndef _W_AUDIOPLAY_H
#define _W_AUDIOPLAY_H

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <mmsystem.h>
#include "anyka_types.h"

#define WAVEHDR_NUM 4

int Audio_WINOpen(int SamplingRate, int nChannels, int nSamples, int nBuffers);
int Audio_WINWrite(short *SampleBuf, int nSamples);
int Audio_WINClose(void);

#endif
#endif

