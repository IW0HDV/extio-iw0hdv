/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */
 
/** 
* @file util.h
* @brief Header files for utility functions
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#if !defined	__UTIL_H__
#define			__UTIL_H__

//
// http://sourceforge.net/p/mingw/mailman/mingw-users/thread/Pine.LNX.4.44.0310310459430.7328-100000@boutell.com/
//
#if defined __MINGW32__
#define SHARED __attribute__((section(".shr"), shared))
#define UNSHARED
#else
#define SHARED
#endif


#if defined _MSC_VER
//#define  strdup _strdup
//#define  wcstombs wcstombs_s
//#define free(x)   
#endif
#if defined __MINGW32__
	#include <string.h>
	#if defined strncpy_s
	#message "strncpy_s is defined"
	#else
	#define  strcpy_s(a,b,c) strcpy(a,c)
	#endif
#endif

#include <stdio.h>

// generic macro for array size computation
#define ARRAY_SIZE(x) ((sizeof(x))/(sizeof(x[0])))
#define MAX(a,b) (((a)>(b))?(a):(b))

//
// snprintf is not present in VC++ 2010, here there a possible remediation
// See http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
//
#ifdef _MSC_VER

#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

#define snprintf c99_snprintf

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}


inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(str, size, format, ap);
	va_end(ap);

	return count;
}

#endif // _MSC_VER

char *xstrdup(const char *p);
void  xstrdel(const char *p, int line = 0);

/*
 * Binary constant generator macro
 * By Tom Torfs - donated to the public domain
 *
 */

/* All macro's evaluate to compile-time constants */

/*
 *
 *
 * helper macros
 *
 *
 */

/* turn a numeric literal into a hex constant
 * (avoids problems with leading zeroes)
 * 8-bit constants max value 0x11111111, always fits in unsigned long
 */
#define HEX__(n) 0x##n##LU

// 8-bit conversion function
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)



/* 
 *
 *
 * user macros
 *
 *
 */

// for upto 8-bit binary constants 
#define B8(d) ((unsigned char)B8__(HEX__(d)))

// for upto 16-bit binary constants, MSB first
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) \
+ B8(dlsb))

// for upto 32-bit binary constants, MSB first
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<<8) \
+ B8(dlsb))

/*
 * Sample usage:
 *
 * B8(01010101) = 85
 * B16(10101010,01010101) = 43605
 * B32(10000000,11111111,10101010,01010101) = 2164238933
 *
 */


/* 
 * set/clear bit 
 * http://stackoverflow.com/questions/2978408/macros-to-set-and-clear-bits
 */
#define SET_BIT(p,n) ((p) |= (1 << (n)))
#define CLR_BIT(p,n) ((p) &= ~((1) << (n)))

#if _WIN32
void ErrorLog(const char* lpszFunction);
#endif

#endif	/* _UTIL_H */

