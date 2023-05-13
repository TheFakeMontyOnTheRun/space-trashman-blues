#ifndef WIN32INT_H
#define WIN32INT_H

#include <windows.h>
#include <wingdi.h>
#include <winuser.h>

#define int32_t int
#define uint32_t unsigned int

#define int64_t __int64
#define uint64_t unsigned __int64

#define int16_t short
#define uint16_t unsigned short

#define uint8_t unsigned char
#define int8_t signed char
#define __restrict__  
#endif
