#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Vec.h"
#include "CActor.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "Core.h"
#include "Engine.h"
#include "FixP.h"
#include "VisibilityStrategy.h"

struct Texture *textures;
uint8_t usedTexture = 0;
extern struct Bitmap *mapTopLevel;

extern struct Bitmap *itemSprites[TOTAL_ITEMS];

extern int currentSelectedItem;


FixP_t lerpFix(const FixP_t v0, const FixP_t v1, const FixP_t dt, const FixP_t total) {
    FixP_t delta = (v1 - v0);
    FixP_t progress = Div(dt, total);
    FixP_t reach = Mul(delta, progress);

    return (v0 + reach);
}

int lerpInt(const int v0, const int v1, const long t, const long total) {
    return fixToInt(lerpFix(intToFix(v0), intToFix(v1), intToFix(t),
                            intToFix(total)));
}

void clearTextures(void) {
    int c;
    usedTexture = 0;
    for (c = 0; c < TOTAL_ITEMS; ++c) {
        if (itemSprites[c] != NULL) {
            releaseBitmap(itemSprites[c]);
            itemSprites[c] = NULL;
        }
    }

    if (mapTopLevel) {
        releaseBitmap(mapTopLevel);
        mapTopLevel = NULL;
    }

    /* The level textures don't need to be free'd */
}

struct Texture *makeTextureFrom(const char *__restrict__ filename) {
    struct Texture *toReturn;
    uint8_t pixel;
    uint8_t repetitions;
    size_t c;
    int d;
    int pixelIndex = 0;
    uint8_t buffer[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
    int x, y;


    struct StaticBuffer staticBuffer = loadBinaryFileFromPath(filename);
    const uint8_t *src = staticBuffer.data;
    size_t sizeInDisk = staticBuffer.size - 4;
    src += 4;

#ifndef CD32
    for (c = 0; c < sizeInDisk; c += 2) {
        pixel = src[c];
        repetitions = src[c + 1];

        for (d = 0; d < repetitions; ++d) {
            buffer[pixelIndex++] = pixel;
        }
    }
#else
    for ( c = 0; c < sizeInDisk; c += 2 ) {
      pixel = src[c];
      repetitions = src[c + 1];
      uint8_t r	  = ( pixel & 192 ) >> 6;
      uint8_t g	  = ( pixel & 56 ) >> 3;
      uint8_t b	  = ( pixel & 7 );

      for ( d = 0; d < repetitions; ++d ) {
          if ( pixel == TRANSPARENCY_COLOR ) {
            buffer[ pixelIndex++ ] = TRANSPARENCY_COLOR;
        } else {
            buffer[ pixelIndex++ ] = ( ( ( r >> 1 ) << 4 ) ) + ( ( g >> 1 ) << 2 ) + ( b >> 1 );
        }
      }
    }
#endif

    toReturn = &textures[usedTexture++];

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        uint8_t *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        uint8_t *dstLine = &toReturn->rotations[0][(y * NATIVE_TEXTURE_SIZE)];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine++;
        }
    }
#ifndef AGS
    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        uint8_t *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE) + (NATIVE_TEXTURE_SIZE - 1)];
        uint8_t *dstLine = &toReturn->rotations[1][y];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine--;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        uint8_t *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE)];
        uint8_t *dstLine = &toReturn->rotations[2][(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE) +
                                                   (NATIVE_TEXTURE_SIZE - 1)];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine--;
        }
    }

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        uint8_t *sourceLine = &buffer[(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE)];
        uint8_t *dstLine = &toReturn->rotations[3][y];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }
#endif

    disposeDiskBuffer(staticBuffer);

    return toReturn;
}

struct Bitmap *loadBitmap(const char *__restrict__ filename) {
    size_t c;
    size_t size;
    int d = 0;
    uint8_t pixel;
    uint8_t repetitions;

    struct Bitmap *toReturn =
            (struct Bitmap *) allocMem(sizeof(struct Bitmap), GENERAL_MEMORY, 1);

    struct StaticBuffer staticBuffer = loadBinaryFileFromPath(filename);
    const uint8_t *ptr = staticBuffer.data;
    size_t sizeInDisk = staticBuffer.size - 4;
    int pixelIndex = 0;
    uint16_t tmp;
    tmp = *ptr++;
    toReturn->width = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->width += tmp & 0xFF;

    tmp = *ptr++;
    toReturn->height = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->height += tmp & 0xFF;

    size = toReturn->width * toReturn->height;

    toReturn->data = (uint8_t *) allocMem(size, BITMAP_MEMORY, 1);

#ifndef CD32
    for (c = 0; c < sizeInDisk; c += 2) {
        pixel = *ptr++;
        repetitions = *ptr++;

        for (d = 0; d < repetitions; ++d) {
            toReturn->data[pixelIndex++] = pixel;
        }
    }
#else
    for ( c = 0; c < sizeInDisk; c += 2 ) {
      pixel = *ptr++;
      repetitions = *ptr++;
      uint8_t r	  = ( pixel & 192 ) >> 6;
      uint8_t g	  = ( pixel & 56 ) >> 3;
      uint8_t b	  = ( pixel & 7 );

      for ( d = 0; d < repetitions; ++d ) {
          if ( pixel == TRANSPARENCY_COLOR ) {
            toReturn->data[ pixelIndex++ ] = TRANSPARENCY_COLOR;
        } else {
            toReturn->data[ pixelIndex++ ] = ( ( ( r >> 1 ) << 4 ) ) + ( ( g >> 1 ) << 2 ) + ( b >> 1 );
        }
      }
    }
#endif

    disposeDiskBuffer(staticBuffer);

    return toReturn;
}

void releaseBitmap(struct Bitmap * ptr) {
    assert (ptr != NULL);

    disposeMem(ptr->data);
    disposeMem(ptr);
}
