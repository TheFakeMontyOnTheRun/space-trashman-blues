#ifndef COMMON_H
#define COMMON_H

#ifdef SWAN
#include <stddef.h>
#define DATA_ADDRESS_SPACE __far
#else
#define DATA_ADDRESS_SPACE
#endif

#ifdef __MWERKS__
/* WHAT A HORRIBLE NIGHT TO HAVE A KLUDGE! */
#define XRES_FRAMEBUFFER 320
#define YRES_FRAMEBUFFER 200
#define SDLSW
#define CLI_BUILD
#define PAGE_FLIP_ANIMATION
#define INCLUDE_ITEM_DESCRIPTIONS
#define ENDIANESS_AWARE
#define SUPPORTS_HACKING_MINIGAME
#endif

#ifndef SMD
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#ifndef GB
#include <unistd.h>
#else
#include <stddef.h>
#endif
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

#ifndef USE_CUSTOM_MEMORY_HANDLER
#ifndef NO_DYNAMIC_MEMORY_ALLOC
void *allocMem(size_t sizeInBytes, enum MemoryType type, uint8_t clearAfterAlloc);

void disposeMem(void* ptr);
#endif

void memCopyToFrom(void* dst, void* src, size_t sizeInBytes);

void memFill(void* dst, uint8_t val, size_t sizeInBytes);
#endif

#ifdef ENDIANESS_AWARE
uint8_t isBigEndian(void);

uint32_t toNativeEndianess(const uint32_t val);
#endif

int countLines(const char* text);

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


#ifndef USE_OWN_MIN_MAX

#ifndef SMD
#ifndef WIN32
#ifndef LEAN_BUILD
#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )
#endif
#endif
#endif

#endif

#endif
