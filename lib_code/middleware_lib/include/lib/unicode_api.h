
#ifndef __UNICODE_API_H
#define __UNICODE_API_H

#include "code_page.h"

typedef unsigned short      WCHAR;

int cp_wcstombs(const unsigned char charsize,int flags,
                      const WCHAR *src, int srclen,
                      char *dst, int dstlen, const char *defchar, int *used );
int cp_mbstowcs(const unsigned char charsize,int flags,
                      const char *s, int srclen,
                      WCHAR *dst, int dstlen , const unsigned short *defchar,unsigned int *ConvertedNum);

int utf8_mbstowcs( int *ConvertedNum, const char *src, int srclen, WCHAR *dst, int dstlen );
int utf8_wcstombs( const WCHAR *src, int srclen, char *dst, int dstlen );


#endif

