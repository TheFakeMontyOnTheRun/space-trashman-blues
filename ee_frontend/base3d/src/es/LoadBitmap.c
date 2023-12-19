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

    toReturn->data = (TexturePixelFormat *) calloc(1, size);

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

        for (d = 0; d < repetitions; ++d) {
            toReturn->data[pixelIndex++] = pixel;
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

