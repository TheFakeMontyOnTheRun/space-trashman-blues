#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define TEXTURE_BUFFER_SIZE 128

uint32_t textureBuffer[TEXTURE_BUFFER_SIZE * TEXTURE_BUFFER_SIZE];

#define isPowerOf2(X) (((X)&((X)-1))==0)

void scaleBitmap(FixP_t x0,
                   FixP_t y0,
                   FixP_t x1,
                   FixP_t y1,
                   struct Bitmap *bitmap) {
    int y;
    int limit;
    FixP_t dY;
    FramebufferPixelFormat pixel = 0;
    FixP_t v = 0;
    UVCoord lastU;
    UVCoord lastV = 0xFF;
    int iy;
    const TexturePixelFormat *data = bitmap->data;
    FixP_t dv;
    FixP_t diffX;
    int iX0;
    int iX1;
    FixP_t du;
    FixP_t bitmapWidth = intToFix(bitmap->width);
    FixP_t bitmapHeight = intToFix(bitmap->height);
    BitmapPixelFormat *bufferData = &textureBuffer[0];

    /* if we have a quad in which the base is smaller */
    if (y0 > y1) {
        /* switch y0 with y1 */
        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;
    }

    y = fixToInt(y0);
    limit = fixToInt(y1);

    if (y == limit) {
        /* degenerate */
        return;
    }

    /* what if the quad is flipped horizontally? */
    if (x0 > x1) {
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;
    }

    dY = (y1 - y0);

    /*
      0xFF here acts as a dirty value, indicating there is no last value.
      But even if we had textures this big, it would be only at the end of
      the run.
   */
    iy = (int32_t) (y);
    dv = Div(bitmapHeight, dY);
    diffX = (x1 - x0);
    iX0 = fixToInt(x0);
    iX1 = fixToInt(x1);

    if (iX0 == iX1) {
        /* degenerate case */
        return;
    }

    du = Div(bitmapWidth + intToFix(2), diffX);

    for (; iy < limit; ++iy) {

        if (iy < TEXTURE_BUFFER_SIZE && iy >= 0) {
            FixP_t u = 0;
            const UVCoord iv = fixToInt(v) % bitmap->height;
            const TexturePixelFormat *sourceLineStart = data + (iv * bitmap->width);
            FramebufferPixelFormat *destinationLine = bufferData + (TEXTURE_BUFFER_SIZE * iy) + iX0;
            int ix;
            lastU = 0;

            if (((iv == lastV)
                    && (iX1 < TEXTURE_BUFFER_SIZE && iX0 >= 0))) {
                int16_t start = (0 >= iX0) ? 0 : iX0;
                int16_t finish = ((TEXTURE_BUFFER_SIZE - 1) >= iX1) ? iX1 : (TEXTURE_BUFFER_SIZE - 1);
                v += dv;
                destinationLine = bufferData + (TEXTURE_BUFFER_SIZE * iy);
                sourceLineStart = destinationLine - TEXTURE_BUFFER_SIZE;
                memcpy(destinationLine + start, sourceLineStart + start,
                       finish - start);

                continue;
            }

            pixel = *(sourceLineStart);

            for (ix = iX0; ix < iX1; ++ix) {

                if (ix < TEXTURE_BUFFER_SIZE && ix >= 0) {

                    const UVCoord iu = fixToInt(u) % bitmap->width;
                    /*
                                  only fetch the next texel if we really changed the
                                  u, v coordinates (otherwise, would fetch the same
                                  thing anyway)
                                   */
                    if (iu != lastU) {

                        pixel = *(sourceLineStart);
                        sourceLineStart += (iu - lastU);
                        lastU = iu;
                        lastV = iv;
                    }



                        FramebufferPixelFormat color = pixel;

                        *(destinationLine) = color;

                }
                ++destinationLine;
                u += du;
            }
        }
        v += dv;
    }
}

struct Bitmap *loadBitmap(const char *filename) {
	size_t c;
	uint8_t d;
    struct StaticBuffer src = loadBinaryFileFromPath(filename);

	struct Bitmap *toReturn =
			(struct Bitmap *) calloc(1, sizeof(struct Bitmap));

    size_t sizeInDisk  = src.size - 4; //total size minus the header

    uint16_t tmp;
    uint8_t *ptr = src.data;

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

    memcpy( buffer, ptr, sizeInDisk);

    toReturn->data = (TexturePixelFormat *) calloc(1, size);

    uint8_t repetitions;
    int pixelIndex = 0;
    BitmapPixelFormat pixel;

    for (c = 0; c < sizeInDisk; c += 5) {
        pixel = 0;

        if (buffer[c + 3] < 255) {
            pixel = TRANSPARENCY_COLOR;
        } else {
            pixel += buffer[c + 3] << 24;
            pixel += buffer[c + 2] << 16;
            pixel += buffer[c + 1] << 8;
            pixel += buffer[c + 0] << 0;
        }
        repetitions = buffer[c + 4];

        for (d = 0; d < repetitions; ++d) {
            toReturn->data[pixelIndex++] = pixel;
        }
    }

    free(buffer);

    toReturn->uploadId = -1;

    return toReturn;
}

void releaseBitmap(struct Bitmap *ptr) {
	assert(ptr != NULL);
	free(ptr->data);
	free(ptr);
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

