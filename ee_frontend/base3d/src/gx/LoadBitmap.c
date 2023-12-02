#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Core.h"
#include "Enums.h"
#include "FixP.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Vec.h"
#include "CActor.h"
#include "Dungeon.h"
#include "PackedFileReader.h"
#include "MapWithCharKey.h"
#include "PackedFileReader.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

#include "Engine.h"
#include "FixP.h"
#include "VisibilityStrategy.h"

#include <gccore.h>
#include <gctypes.h>
#include <ogc/gx.h>

void bindTexture(struct Bitmap *bitmap) {

    GX_LoadTexObj(bitmap->nativeBuffer, GX_TEXMAP0);
}


int submitBitmapToGPU(struct Bitmap *bitmap) {
    bitmap->nativeBuffer = allocMem(sizeof(GXTexObj), TEXTURE_MEMORY, 1);
    DCFlushRange(bitmap->data, bitmap->width * bitmap->height * 4);
    GX_InitTexObj(bitmap->nativeBuffer, bitmap->data, bitmap->width, bitmap->height, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    bitmap->uploadId = 1;

    return 0;
}

/* Based on code from GRRLIB - https://github.com/GRRLIB/GRRLIB */
void  SetPixelToTexImg (const int x, const int y,
                               struct Bitmap *tex, const uint32_t color) {
    uint32_t  offs;
    uint8_t*  bp = (uint8_t*)tex->data;

    offs = (((y&(~3))<<2)*tex->width) + ((x&(~3))<<4) + ((((y&3)<<2) + (x&3)) <<1);

    *((uint16_t*)(bp+offs   )) = (uint16_t)((color <<8) | (color >>24));
    *((uint16_t*)(bp+offs+32)) = (uint16_t) (color >>8);
}

struct Bitmap *loadBitmap(const char *filename) {
    size_t c;
    uint8_t d;
    struct StaticBuffer src = loadBinaryFileFromPath(filename);

    struct Bitmap *toReturn =
            (struct Bitmap *) calloc(1, sizeof(struct Bitmap));

    size_t sizeInDisk = src.size - 4; //total size minus the header

    uint16_t tmp;
    const uint8_t *ptr = src.data;

    tmp = *ptr++;
    toReturn->width = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->width += tmp & 0xFF;

    tmp = *ptr++;
    toReturn->height = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->height += tmp & 0xFF;

    size_t size = toReturn->width * toReturn->height * sizeof(BitmapPixelFormat);

    uint8_t *buffer = (uint8_t *) calloc(1, sizeInDisk);

    memCopyToFrom(buffer, ptr, sizeInDisk);

    toReturn->data = (TexturePixelFormat *) memalign( 32, size);
    uint32_t *imgBuffer = allocMem(size, GENERAL_MEMORY, 0);

    uint8_t repetitions;
    int pixelIndex = 0;
    BitmapPixelFormat pixel;

    for (c = 0; c < sizeInDisk; c += 5) {
        pixel = 0;

        if (buffer[c + 3] < 255) {
            pixel = TRANSPARENCY_COLOR;
        } else {
            pixel += (0x80) << 24;
            pixel += buffer[c + 2] << 16;
            pixel += buffer[c + 1] << 8;
            pixel += buffer[c + 0] << 0;
        }
        repetitions = buffer[c + 4];

        pixel = toNativeEndianess(pixel);

        for (d = 0; d < repetitions; ++d) {
            imgBuffer[pixelIndex++] = pixel;
        }
    }

    for (int y = 0; y < toReturn->height; ++y ) {
        for (int x = 0; x < toReturn->width; ++x ) {
            SetPixelToTexImg(x, y, toReturn, imgBuffer[y * toReturn->width + x]);
        }
    }

    disposeMem(buffer);
    disposeDiskBuffer(src);

    toReturn->uploadId = -1;

    return toReturn;
}

void releaseBitmap(struct Bitmap *ptr) {
    assert(ptr != NULL);

    if (ptr->nativeBuffer != NULL) {
        disposeMem(ptr->nativeBuffer);
    }

    disposeMem(ptr->data);
    disposeMem(ptr);
}

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

