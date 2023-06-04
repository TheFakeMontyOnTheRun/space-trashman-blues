#ifndef COMMON_H
#define COMMON_H

#ifndef SMD
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#endif

#define MAP_SIZE 32

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

struct StaticBuffer {
    const uint8_t *data;
    size_t size;
};

int isBigEndian(void);

void initFileReader(const char *  dataFilePath);

uint32_t toNativeEndianess(const uint32_t val);

#ifndef SMD
#ifndef WIN32
#ifndef LEAN_BUILD
#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )
#endif
#endif
#endif

#endif
