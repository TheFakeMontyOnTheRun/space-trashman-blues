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

enum MemoryType {
    GENERAL_MEMORY,
    BITMAP_MEMORY,
    DISKBUFFER_MEMORY,
    SOUND_MEMORY,
    TEXTURE_MEMORY,
    FONT_MEMORY
};

struct StaticBuffer {
    const uint8_t *data;
    size_t size;
};

void *allocMem(size_t sizeInBytes, enum MemoryType type, int clearAfterAlloc);

void disposeMem(void* ptr);

void memCopyToFrom(void* dst, void* src, size_t sizeInBytes);

void memFill(void* dst, uint8_t val, size_t sizeInBytes);

#ifndef LEAN_BUILD
int isBigEndian(void);

uint32_t toNativeEndianess(const uint32_t val);
#endif


#ifndef SMD
#ifndef WIN32
#ifndef LEAN_BUILD
#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )
#endif
#endif
#endif

#endif
