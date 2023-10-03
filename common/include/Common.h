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

int isBigEndian(void);

void initFileReader(const char *  dataFilePath);

uint32_t toNativeEndianess(const uint32_t val);


extern const char *mainText;
extern char *textBuffer;
extern size_t biggestOption;

extern const char *focusItemName;
extern int currentSelectedItem;
extern int shouldContinue;
extern const char *thisMissionName;
extern int16_t thisMissionNameLen;

extern size_t biggestOption;

#define TEXT_BUFFER_SIZE (40 * 25)

#ifndef SMD
#ifndef WIN32
#ifndef LEAN_BUILD
#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )
#endif
#endif
#endif

#endif
