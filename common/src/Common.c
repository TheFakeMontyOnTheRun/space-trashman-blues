#ifdef AMIGA
#include "AmigaInt.h"
#endif

#ifdef ATARIST
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mint/osbind.h>
#include <mint/sysbind.h>
#endif

#ifdef N64
#include <libdragon.h>
#endif

#include <stddef.h>

#ifndef SMD
#include <stdlib.h>
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#endif
#else
#include <genesis.h>
#endif

#include "Common.h"


#ifndef USE_CUSTOM_MEMORY_HANDLER
void *allocMem(size_t sizeInBytes, enum MemoryType type, int clearAfterAlloc) {
    /*
    For the general allocator, we're not worried about the type of memory. It all comes from the
    same place.
     */
    void *ptr;

#ifndef N64
    ptr = malloc(sizeInBytes);
#else
    ptr = malloc_uncached(sizeInBytes);
#endif

    if (clearAfterAlloc) {
        memset(ptr, 0, sizeInBytes);
    }

    return ptr;
}

void disposeMem(void* ptr) {
#ifndef N64
    free(ptr);
#else
    free_uncached(ptr);
#endif

}

void memCopyToFrom(void* dst, void* src, size_t sizeInBytes) {
    memcpy(dst, src, sizeInBytes);
}

void memFill(void* dst, uint8_t val, size_t sizeInBytes) {
    memset(dst, val, sizeInBytes);
}
#endif

int isBigEndian() {
	union {
		uint32_t i;
		char c[4];
	} e = {0x01000000};

	return e.c[0];
}

uint32_t toNativeEndianess(const uint32_t val) {
	uint32_t val2 = val;

	if (isBigEndian()) {
		uint32_t b0, b1, b2, b3;

		b0 = (val & 0x000000ff) << 24u;
		b1 = (val & 0x0000ff00) << 8u;
		b2 = (val & 0x00ff0000) >> 8u;
		b3 = (val & 0xff000000) >> 24u;

		val2 = b0 | b1 | b2 | b3;
	}

	return val2;
}
