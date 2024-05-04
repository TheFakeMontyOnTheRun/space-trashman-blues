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

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Vec.h"
#include "CActor.h"
#include "PackedFileReader.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
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
    BitmapPixelFormat pixel;
    uint8_t repetitions;
    size_t c;
    int d;
    int pixelIndex = 0;
    BitmapPixelFormat buffer[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
    int x, y;

    struct Bitmap* bPtr = loadBitmap(filename);

    for (c = 0; c < NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE; ++c) {
        buffer[c] = bPtr->data[c];
    }

    releaseBitmap(bPtr);

    toReturn = &textures[usedTexture++];

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        BitmapPixelFormat *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        BitmapPixelFormat *dstLine = &toReturn->rotations[0][(y * NATIVE_TEXTURE_SIZE)];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine++;
        }
    }
#ifndef FLOOR_TEXTURES_DONT_ROTATE
    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        BitmapPixelFormat *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE) + (NATIVE_TEXTURE_SIZE - 1)];
        BitmapPixelFormat *dstLine = &toReturn->rotations[1][y];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine--;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        BitmapPixelFormat *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE)];
        BitmapPixelFormat *dstLine = &toReturn->rotations[2][(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE) +
                                                   (NATIVE_TEXTURE_SIZE - 1)];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine--;
        }
    }

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        BitmapPixelFormat *sourceLine = &buffer[(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE)];
        BitmapPixelFormat *dstLine = &toReturn->rotations[3][y];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }
#endif
    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        BitmapPixelFormat *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        BitmapPixelFormat *dstLine = &toReturn->rowMajor[y];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    return toReturn;
}

struct Bitmap *loadBitmap(const char *__restrict__ filename) {
    size_t c;
    size_t size;
    int d = 0;
    BitmapPixelFormat pixel;
    uint8_t repetitions;

    struct Bitmap *toReturn =
            (struct Bitmap *) allocMem(sizeof(struct Bitmap), GENERAL_MEMORY, 1);

#ifndef RGBA32_FRAMEBUFFER
    struct StaticBuffer staticBuffer = loadBinaryFileFromPath(filename);

    const uint8_t* ptr = staticBuffer.data;
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

    size = toReturn->width * toReturn->height * sizeof(BitmapPixelFormat);

    toReturn->data = (BitmapPixelFormat*)allocMem(size, BITMAP_MEMORY, 1);
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
#else
    struct StaticBuffer staticBuffer = loadBinaryFileFromPath(filename);

    size_t sizeInDisk = staticBuffer.size - 4; //total size minus the header

    uint16_t tmp;
    const uint8_t* ptr = staticBuffer.data;

    tmp = *ptr++;
    toReturn->width = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->width += tmp & 0xFF;

    tmp = *ptr++;
    toReturn->height = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->height += tmp & 0xFF;

    uint8_t* buffer = (uint8_t*)calloc(1, sizeInDisk);

    memCopyToFrom((void*)buffer, (void*)ptr, sizeInDisk);

    size = toReturn->width * toReturn->height * sizeof(BitmapPixelFormat);

    toReturn->data = (TexturePixelFormat*)calloc(1, size);

    int pixelIndex = 0;

    for (c = 0; c < sizeInDisk; c += 5) {
        pixel = 0;

        if (buffer[c + 3] < 255) {
            pixel = TRANSPARENCY_COLOR;
        }
        else {
            pixel += (0x80) << 24;
            pixel += buffer[c + 0] << 16;
            pixel += buffer[c + 1] << 8;
            pixel += buffer[c + 2] << 0;
        }
        repetitions = buffer[c + 4];

        for (d = 0; d < repetitions; ++d) {
            toReturn->data[pixelIndex++] = pixel;
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
