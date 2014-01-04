
/**
 * @file Eng_UStrPublic.h
 * @author Songmengxing
 * @date 2012-12-31
 * @brief ANYKA software
 * this header file provide hal layer string function 
 */

#ifndef __ENG_USTRPUBLIC_H
#define __ENG_USTRPUBLIC_H


#define MAX_USTRING_LEN     0x7fff
#define MAX_USTR_LINE       1000

#define UNICODE_R           0x000d      // '\R'
#define UNICODE_N           0x000a      // '\N'
#define UNICODE_SPACE       0x0020      // ' '

#define UNICODE_A           0x0041      // 'A'
#define UNICODE_B           0x0042      // 'B'
#define UNICODE_C           0x0043      // 'C'
#define UNICODE_D           0x0044      // 'D'
#define UNICODE_Z           0x005a      // 'Z'

#define UNICODE_a           0x0061      // 'a'  
#define UNICODE_b           0x0062      // 'b'
#define UNICODE_c           0x0063      // 'c'
#define UNICODE_d           0x0064      // 'd'
#define UNICODE_e           0x0065      // 'e'
#define UNICODE_f           0x0066      // 'f'
#define UNICODE_i           0x0069      // 'i'
#define UNICODE_k           0x006b      // 'k'
#define UNICODE_l           0x006c      // 'l'
#define UNICODE_o           0x006f      // 'o'
#define UNICODE_r           0x0072      // 'r'
#define UNICODE_s           0x0073      // 's'
#define UNICODE_t           0x0074      // 't'
#define UNICODE_y           0x0079      // 'y'

#define UNICODE_0           0x0030      // '0'
#define UNICODE_1           0x0031      // '1'
#define UNICODE_2           0x0032      // '2'
#define UNICODE_3           0x0033      // '3'
#define UNICODE_4           0x0034      // '4'
#define UNICODE_5           0x0035      // '5'
#define UNICODE_6           0x0036      // '6'
#define UNICODE_7           0x0037      // '7'
#define UNICODE_8           0x0038      // '8'
#define UNICODE_9           0x0039      // '9'

#define UNICODE_END         0x0000      // '\0'
#define UNICODE_COLON       0x003A      // ':'
#define UNICODE_SEPARATOR   0x003B      //';'
#define UNICODE_DOT         0x002E      // '.'
#define UNICODE_SOLIDUS     0x002F      // '/'
#define UNICODE_RES_SOLIDUS 0x005C      // '\'
#define UNICODE_STAR        0x002A      // '*'
#define UNICODE_QUESTION    0x003F      // '?'
#define UNICODE_LBRACKET    0x005b      // '['
#define UNICODE_RBRACKET    0x005d      // ']'
#define UNICODE_LITTLE		0x003c      // '<'
#define UNICODE_LARGE	    0x003e      // '>'

#define  UStrCpyN(strDes,  strSrc, length)                Utl_UStrCpyN(strDes, strSrc, length)
#define  UStrCpy(strDes, strSrc)                          Utl_UStrCpy(strDes, strSrc)
#define  UStrLen(strSrc)                                  Utl_UStrLen(strSrc)
#define  UStrCmp(strDes, strSrc)                          Utl_UStrCmp(strDes, strSrc)
#define  UStrCmpN(strDes,  strSrc, length)                Utl_UStrCmpN(strDes, strSrc, length)
#define  UStrCat(strDes, strSrc)                          Utl_UStrCat(strDes, strSrc)
#define  UStrFnd(strFndDes,strFndSrc,offsetStart)         Utl_UStrFnd(strFndSrc, strFndDes, offsetStart)

#endif
