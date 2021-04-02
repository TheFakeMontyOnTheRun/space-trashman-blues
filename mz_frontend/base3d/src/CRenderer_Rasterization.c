#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"

uint16_t clippingY1 = 200;
extern int8_t stencilHigh[128];
int16_t dirtyLineY0 = 0;
int16_t dirtyLineY1 = 200;

/*
    *         /|x1y0
    * x0y0   / |
    *       |  |
    *       |  |
    * x0y1  |  |
    *       \  |
    *        \ |
    *         \| x1y1
    */
void maskWall(
        FixP_t x0,
        FixP_t x1,
        FixP_t x0y0,
        FixP_t x0y1,
        FixP_t x1y0,
        FixP_t x1y1) {

    const FixP_t zero = 0;
    int32_t x;
    int32_t limit;
    FixP_t upperY0;
    FixP_t lowerY0;
    FixP_t upperY1;
    FixP_t lowerY1;
    FixP_t upperDy;
    FixP_t lowerDy;
    FixP_t y0;
    FixP_t y1;
    FixP_t dX;
    FixP_t upperDyDx;
    FixP_t lowerDyDx;
    uint8_t pixel = 0;
    int32_t ix;
    uint8_t *bufferData = &framebuffer[0];

    if (x0 > x1) {
        /* switch x0 with x1 */
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;

        /* switch x0y0 with x1y0 */
        x0y0 = x0y0 + x1y0;
        x1y0 = x0y0 - x1y0;
        x0y0 = x0y0 - x1y0;

        /* switch x0y1 with x1y1 */
        x0y1 = x0y1 + x1y1;
        x1y1 = x0y1 - x1y1;
        x0y1 = x0y1 - x1y1;
    }

    x = fixToInt(x0);
    limit = fixToInt(x1);

    if (x >= 256 || limit < 0 || x == limit) {
        return;
    }

    upperY0 = x0y0;
    lowerY0 = x0y1;
    upperY1 = x1y0;
    lowerY1 = x1y1;

    if (x0y0 > x0y1) {
        upperY0 = x0y1;
        lowerY0 = x0y0;
        upperY1 = x1y1;
        lowerY1 = x1y0;
    }

    upperDy = (upperY1 - upperY0);
    lowerDy = (lowerY1 - lowerY0);

    y0 = upperY0;
    y1 = lowerY0;

    dX = intToFix(limit - x);
    upperDyDx = Div(upperDy, dX);
    lowerDyDx = Div(lowerDy, dX);

    pixel = 0;
    /*
      0xFF here acts as a dirty value, indicating there is no last value.
      But even if we had textures this big, it would be only at the end of
      the run.
      we can use this statically, since the textures are already loaded.
      we don't need to fetch that data on every run.
  */
    ix = x;

    for (; ix < limit; ++ix) {
        if (ix >= 0 && ix < 256) {

            const FixP_t diffY = (y1 - y0);
            int32_t iY0 = fixToInt(y0);
            int32_t iY1 = fixToInt(y1);
            uint8_t *destinationLine = bufferData + (320 * iY0) + ix;
            int32_t iy;

            if (diffY == zero) {
                continue;
            }

            if (iY0 < 0) {
                iY0 = 0;
            }

            if (iY1 >= 128) {
                iY1 = 128;
            }

            for (iy = iY0; iy < iY1; ++iy) {
                *(destinationLine) = pixel;

                destinationLine += (320);
            }
        }
        y0 += upperDyDx;
        y1 += lowerDyDx;
    }
}

/*
    *         /|x1y0
    * x0y0   / |
    *       |  |
    *       |  |
    * x0y1  |  |
    *       \  |
    *        \ |
    *         \| x1y1
    */
void drawWall(FixP_t x0,
              FixP_t x1,
              FixP_t x0y0,
              FixP_t x0y1,
              FixP_t x1y0,
              FixP_t x1y1,
              const uint8_t * __restrict__ texture,
              const FixP_t textureScaleY,
              const int z) {
    const FixP_t zero = 0;
    int32_t x;
    int32_t limit;
    FixP_t upperY0;
    FixP_t lowerY0;
    FixP_t upperY1;
    FixP_t lowerY1;
    FixP_t upperDy;
    FixP_t lowerDy;
    FixP_t y0;
    FixP_t y1;
    FixP_t dX;
    FixP_t upperDyDx;
    FixP_t lowerDyDx;
    uint8_t pixel = 3;
    FixP_t u = 0;
    uint8_t lastV;
    const uint8_t *data = texture;
    const int8_t textureWidth = 32;
    const FixP_t textureSize = intToFix(textureWidth);
    FixP_t du;
    int32_t ix;
    uint8_t *bufferData = &framebuffer[0];
    int farForStipple = (z >= distanceForPenumbra);

    if (x0 > x1) {
        /* switch x0 with x1 */
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;

        /* switch x0y0 with x1y0 */
        x0y0 = x0y0 + x1y0;
        x1y0 = x0y0 - x1y0;
        x0y0 = x0y0 - x1y0;

        /* switch x0y1 with x1y1 */
        x0y1 = x0y1 + x1y1;
        x1y1 = x0y1 - x1y1;
        x0y1 = x0y1 - x1y1;
    }

    x = fixToInt(x0);
    limit = fixToInt(x1);

    if (x >= 256 || limit < 0 || x == limit) {
        return;
    }

    upperY0 = x0y0;
    lowerY0 = x0y1;
    upperY1 = x1y0;
    lowerY1 = x1y1;

    if (x0y0 > x0y1) {
        upperY0 = x0y1;
        lowerY0 = x0y0;
        upperY1 = x1y1;
        lowerY1 = x1y0;
    }

    upperDy = (upperY1 - upperY0);
    lowerDy = (lowerY1 - lowerY0);

    y0 = upperY0;
    y1 = lowerY0;

    dX = intToFix(limit - x);
    upperDyDx = Div(upperDy, dX);
    lowerDyDx = Div(lowerDy, dX);
    du = Div(textureSize, dX);
    u = 0;
    ix = x;

    for (; ix < limit; ++ix) {
        if (ix >= 0 && ix < 256) {
            const FixP_t diffY = (y1 - y0);
            FixP_t v = 0;
            int32_t iu = fixToInt(u);
            int32_t iY0 = fixToInt(y0);
            int32_t iY1 = fixToInt(y1);
            int halfX = ix >> 1;
            const uint8_t *sourceLineStart = data + (iu * textureWidth);
            const uint8_t *lineOffset = sourceLineStart;
            uint8_t *destinationLine = bufferData + (320 * iY0) + ix;
            FixP_t dv;
            int32_t iy;

            if (diffY == zero) {
                continue;
            }
            dv = Div(Mul(textureSize, textureScaleY), diffY);
            lastV = 0;
            pixel = *(lineOffset);

            for (iy = iY0; iy < iY1; ++iy) {

                if (iy < 128 && iy >= 0 && iy > stencilHigh[halfX]) {
                    const int32_t iv = fixToInt(v);
                    int stipple = !((ix + iy) & 1);

                    if (iv != lastV && !( stipple && farForStipple)) {

                        pixel = *(lineOffset);
                        lineOffset = ((iv & (textureWidth - 1)) + sourceLineStart);
                        lastV = iv;
                    }

                    uint8_t color = pixel;

                    if ( farForStipple && stipple) {
                        color = 0;
                    }

                    *(destinationLine) = color;
                }
                destinationLine += (320);
                v += dv;
            }
            
            if (iy >= 0 && iy > stencilHigh[halfX] && (ix & 1)) {
                if (iy > 127 ) {
                    stencilHigh[halfX] = 127;
                } else {
                    stencilHigh[halfX] = iy;
                }
            }
        }
        
        y0 += upperDyDx;
        y1 += lowerDyDx;
        u += du;
    }
}

void drawMask(
        const FixP_t x0,
        const FixP_t y0,
        const FixP_t x1,
        const FixP_t y1) {

    int32_t _x0 = fixToInt(x0);
    int32_t _y0 = fixToInt(y0);
    int32_t _x1 = fixToInt(x1);
    int32_t _y1 = fixToInt(y1);

    if (_x0 < 0) {
        _x0 = 0;
    }

    if (_x1 < 0) {
        _x1 = 0;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 < 0) {
        _y1 = 0;
    }

    if (_x0 >= 256) {
        _x0 = 256 - 1;
    }

    if (_x1 >= 256) {
        _x1 = 256 - 1;
    }

    if (_y0 >= 128) {
        _y0 = 128 - 1;
    }

    if (_y1 >= 128) {
        _y1 = 128 - 1;
    }

    fill(_x0, _y0, _x1 - _x0, _y1 - _y0, 0, FALSE);
}

void drawFrontWall(FixP_t x0,
                   FixP_t y0,
                   FixP_t x1,
                   FixP_t y1,
                   const uint8_t *  __restrict__  texture,
                   const FixP_t textureScaleY,
                   const int z,
                   const int enableAlpha) {
    int16_t y;
    int limit;
    FixP_t dY;
    uint8_t pixel = 4;
    int32_t iy;
    FixP_t v = 0;
    FixP_t du;
    uint8_t lastU;
    uint8_t lastV = 0xFF;
    const uint8_t *data = texture;
    const FixP_t textureSize = intToFix(32);
    const FixP_t texWidth = intToFix(32);
    FixP_t dv;
    FixP_t diffX;
    int iX0;
    int iX1;
    uint8_t *bufferData = &framebuffer[0];
    int farEnoughForStipple = (z >= distanceForPenumbra);

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
    dv = Div(Mul(textureSize, textureScaleY) - intToFix(1), dY);
    diffX = (x1 - x0);
    iX0 = fixToInt(x0);
    iX1 = fixToInt(x1);

    if (iX0 == iX1) {
        /* degenerate case */
        return;
    }

    du = Div(texWidth + intToFix(2), diffX);

    for (; iy < limit; ++iy) {
        if (iy < 128 && iy >= 0 ) {
            FixP_t u = 0;
            const uint8_t iv = fixToInt(v) & 31;
            const uint8_t *sourceLineStart = data + (iv * 32);
            uint8_t *destinationLine = bufferData + (320 * iy) + iX0;
            int ix;
            lastU = 0;

            if (!farEnoughForStipple
                && ((!enableAlpha && iv == lastV)
                    && (iX1 < 256 && iX0 >= 0))) {
                int16_t start = (0 >= iX0) ? 0 : iX0;
                int16_t finish = ((256 - 1) >= iX1) ? iX1 : (256 - 1);
                v += dv;
                destinationLine = bufferData + (320 * iy);
                sourceLineStart = destinationLine - 320;
                memcpy (destinationLine + start, sourceLineStart + start,
                        finish - start);
                continue;
            }

            pixel = *(sourceLineStart);

            for (ix = iX0; ix < iX1; ++ix) {

                if (ix < 256 && ix >= 0 && iy > stencilHigh[ix >> 1] ) {
                    int stipple = ((ix + iy) & 1);
                    const uint8_t iu = fixToInt(u) & 31;

                    if (iu != lastU
                        && !( stipple && farEnoughForStipple)) {

                        pixel = *(sourceLineStart);
                        sourceLineStart += (iu - lastU);
                        lastU = iu;
                        lastV = iv;
                    }
                    
                    uint8_t color = pixel;

                    if ( farEnoughForStipple && stipple) {
                        color = 0;
                    }

                    *(destinationLine) = color;
                }
                ++destinationLine;
                u += du;
            }
        }
        v += dv;
    }
    
    
    if (iX0 < 0 ) {
        iX0 = 0;
    }
    
    if (iX0 >= 256 ) {
        iX0 = 255;
    }

    if (iX1 < 0 ) {
        iX1 = 0;
    }
    
    if (iX1 >= 256 ) {
        iX1 = 255;
    }
    
    iX0 = iX0 / 2;
    iX1 = iX1 / 2;

    for (int ix = iX0; ix < iX1; ++ix) {
        if (iy > stencilHigh[ix] ) {
            stencilHigh[ix] = iy;
        }
    }
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /             \
    *  x0y1 /______________\ x1y1
    */
void maskFloor(
        FixP_t y0, FixP_t y1, FixP_t x0y0, FixP_t x1y0, FixP_t x0y1, FixP_t x1y1
#ifdef FLAT_FLOOR_CEILING
        , uint8_t pixel
#endif
) {

    int32_t y;
    int32_t limit;
    FixP_t upperX0;
    FixP_t upperX1;
    FixP_t lowerX0;
    FixP_t lowerX1;
    FixP_t leftDX;
    FixP_t rightDX;
    FixP_t dY;
    FixP_t leftDxDy;
    FixP_t rightDxDy;
    const FixP_t zero = 0;
    FixP_t x0;
    FixP_t x1;
#ifndef FLAT_FLOOR_CEILING
    uint8_t pixel = 0;
#endif
    uint8_t *bufferData = &framebuffer[0];
    int16_t iy;
    /*
      0xFF here acts as a dirty value, indicating there is no last value.
      But even if we had textures this big, it would be only at the end of
      the run.
   */

    /* if we have a trapezoid in which the base is smaller */
    if (y0 > y1) {
        /* switch y0 with y1 */
        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;

        /* switch x0y0 with x0y1 */
        x0y0 = x0y0 + x0y1;
        x0y1 = x0y0 - x0y1;
        x0y0 = x0y0 - x0y1;

        /* switch x1y0 with x1y1 */
        x1y0 = x1y0 + x1y1;
        x1y1 = x1y0 - x1y1;
        x1y0 = x1y0 - x1y1;
    }

    y = fixToInt(y0);
    limit = fixToInt(y1);

    if (y == limit || limit < 0 || y >= 128) {
        return;
    }

    upperX0 = x0y0;
    upperX1 = x1y0;
    lowerX0 = x0y1;
    lowerX1 = x1y1;

    /* what if the trapezoid is flipped horizontally? */
    if (x0y0 > x1y0) {
        upperX0 = x1y0;
        upperX1 = x0y0;
        lowerX0 = x1y1;
        lowerX1 = x0y1;
    }

    leftDX = (lowerX0 - upperX0);
    rightDX = (lowerX1 - upperX1);
    dY = (y1 - y0);
    leftDxDy = Div(leftDX, dY);
    rightDxDy = Div(rightDX, dY);
    x0 = upperX0;
    x1 = upperX1;
    iy = y;

    for (; iy < limit; ++iy) {

        if (iy < 128 && iy >= 0) {

            const FixP_t diffX = (x1 - x0);
            int32_t iX0 = fixToInt(x0);
            int32_t iX1 = fixToInt(x1);

            if (diffX == zero) {
                continue;
            }

            if (iX0 < 0) {
                iX0 = 0;
            }

            if (iX1 >= 256) {
                iX1 = 256 - 1;
            }

            if (iX1 < 0) {
                iX1 = 0;
            }

            if (iX0 >= 256) {
                iX0 = 256 - 1;
            }

            memset (bufferData + (320 * iy) + iX0, pixel, iX1 - iX0);
        }

        x0 += leftDxDy;
        x1 += rightDxDy;
    }
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /             \
    *  x0y1 /______________\ x1y1
    */
void drawFloor(FixP_t y0,
               FixP_t y1,
               FixP_t x0y0,
               FixP_t x1y0,
               FixP_t x0y1,
               FixP_t x1y1,
               int z,
               const uint8_t * __restrict__ texture) {

    int32_t y;
    int32_t limit;
    FixP_t upperX0;
    FixP_t upperX1;
    FixP_t lowerX0;
    FixP_t lowerX1;
    FixP_t leftDX;
    FixP_t rightDX;
    FixP_t dY;
    FixP_t leftDxDy;
    FixP_t rightDxDy;
    const FixP_t zero = 0;
    FixP_t x0;
    FixP_t x1;
    uint8_t pixel = 0;
    FixP_t v = 0;
    uint8_t lastU;
    int16_t iy;
    uint8_t *bufferData = &framebuffer[0];
    const uint8_t *data = texture;
    const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
    const FixP_t textureSize = intToFix(textureWidth);
    FixP_t dv;
    const uint8_t *sourceLineStart;
    int farEnoughForStipple = (z >= distanceForPenumbra);

    /* if we have a trapezoid in which the base is smaller */
    if (y0 > y1) {
        /* switch y0 with y1 */
        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;

        /* switch x0y0 with x0y1 */
        x0y0 = x0y0 + x0y1;
        x0y1 = x0y0 - x0y1;
        x0y0 = x0y0 - x0y1;

        /* switch x1y0 with x1y1 */
        x1y0 = x1y0 + x1y1;
        x1y1 = x1y0 - x1y1;
        x1y0 = x1y0 - x1y1;
    }

    y = fixToInt(y0);
    limit = fixToInt(y1);

    if (y == limit || limit < 0 || y >= 128) {
        return;
    }

    upperX0 = x0y0;
    upperX1 = x1y0;
    lowerX0 = x0y1;
    lowerX1 = x1y1;

    /* what if the trapezoid is flipped horizontally? */
    if (x0y0 > x1y0) {
        upperX0 = x1y0;
        upperX1 = x0y0;
        lowerX0 = x1y1;
        lowerX1 = x0y1;
    }

    leftDX = (lowerX0 - upperX0);
    rightDX = (lowerX1 - upperX1);
    dY = (y1 - y0);
    leftDxDy = Div(leftDX, dY);
    rightDxDy = Div(rightDX, dY);
    x0 = upperX0;
    x1 = upperX1;
    iy = y;
    dv = Div(textureSize, dY);

    for (; iy < limit; ++iy) {

        if (iy < 128 && iy >= 0) {

            const FixP_t diffX = (x1 - x0);
            int32_t iX0 = fixToInt(x0);
            int32_t iX1 = fixToInt(x1);
            int32_t ix;
            FixP_t u = 0;
            FixP_t du;
            int32_t iv;
            uint8_t *destinationLine;
            lastU = 0;

            if (diffX == zero) {
                continue;
            }

            du = Div(textureSize, diffX);
            iv = fixToInt(v);
            destinationLine = bufferData + (320 * iy) + iX0;
            sourceLineStart = data + (iv * textureWidth);
            pixel = *(sourceLineStart);

            for (ix = iX0; ix < iX1; ++ix) {

                if (ix >= 0 && ix < 256) {
                    const int32_t iu = fixToInt(u);
                    int stipple = ((ix + iy) & 1) == 0;
                    /*
                                  only fetch the next texel if we really changed the
                                  u, v coordinates (otherwise, would fetch the same
                                  thing anyway)
                                  */
                    if (iu != lastU
                        && !( stipple && farEnoughForStipple)) {

                        pixel = *(sourceLineStart);
                        sourceLineStart += (iu - lastU);
                        lastU = iu;
                    }

                    if (pixel != TRANSPARENCY_COLOR) {
                        uint8_t color = pixel;

                        if ( farEnoughForStipple && stipple) {
                            color = 0;
                        }

                        *(destinationLine) = color;
                    }
                }
                ++destinationLine;
                u += du;
            }
        }

        x0 += leftDxDy;
        x1 += rightDxDy;
        v += dv;
    }
}

void drawRect(
        const int16_t x,
        const int16_t y,
        const uint16_t dx,
        const uint16_t dy,
        const uint8_t pixel) {

    uint8_t *destination = &framebuffer[0];
    uint8_t *destinationLineStart = destination + (320 * (y)) + x;
    int16_t py;

    if (pixel == TRANSPARENCY_COLOR) {
        return;
    }

    memset (destinationLineStart, pixel, dx);

    for (py = 0; py < (dy); ++py) {
        destinationLineStart = destination + (320 * (y + py)) + x;
        *destinationLineStart = pixel;
        destinationLineStart += dx;
        *destinationLineStart = pixel;
    }
    memset (destination + (320 * (y + dy)) + x, pixel, dx);
}

void fill(
        const int16_t x,
        const int16_t y,
        const int16_t dx,
        const int16_t dy,
        const uint8_t pixel,
        const int stipple) {

    uint8_t *destination = &framebuffer[0];
    int16_t py;

    if (pixel == TRANSPARENCY_COLOR) {
        return;
    }

    for (py = 0; py < dy; ++py) {
        uint8_t *destinationLineStart = destination + (320 * (y + py)) + x;

        if (!stipple) {
            memset (destinationLineStart, pixel, dx);
        } else {
            int16_t px;
            for (px = 0; px < dx; ++px) {
                destinationLineStart++;
                if ((px + py) & 1) {
                    *destinationLineStart = pixel;
                }
            }
        }
    }
}

void drawBitmap(const int16_t dx,
                const int16_t dy,
                const struct Bitmap *__restrict__ tile,
                const int transparent) {

    uint8_t *destination = &framebuffer[0];
    uint8_t *sourceLine = tile->data;
    size_t height = tile->height;
    size_t width = tile->width;
    size_t y;

    if ((dy + height) >= 200) {
        height = (200 - dy);
    }


    if (transparent) {
        for (y = 0; y < height; ++y) {
            uint8_t *destinationLineStart = destination + (320 * (dy + y)) + dx;
            uint8_t *sourceLineStart = sourceLine + (width * y);
            size_t x;

            for (x = 0; x < width; ++x) {
                uint8_t pixel = *sourceLineStart;

                if (!transparent || (pixel != TRANSPARENCY_COLOR)) {
                    *destinationLineStart = pixel;
                }

                ++sourceLineStart;
                ++destinationLineStart;
            }
        }
    } else {
        for (y = 0; y < height; ++y) {
            uint8_t *destinationLineStart = destination + (320 * (dy + y)) + dx;
            uint8_t *sourceLineStart = sourceLine + (width * y);
            size_t x;

            for (x = 0; x < width; ++x) {
                *destinationLineStart = *sourceLineStart;
                ++sourceLineStart;
                ++destinationLineStart;
            }
        }
    }

}

void drawRepeatBitmap(
        const int16_t x,
        const int16_t y,
        const int16_t dx,
        const int16_t dy,
        const struct Bitmap *__restrict__ tile) {

	int16_t repeatX;
	int16_t repeatY;
    uint16_t px, py;

	py = y;
    repeatY = dy;
    while (repeatY > 0) {
    	repeatX = dx;
		px = x;
    	while (repeatX > 0 ) {
			repeatX -= tile->width;

			if (px < 320 && py < 200) {
				drawBitmap(px, py, tile, FALSE);
			}

			px += tile->width;
    	}
		py += tile->height;
		repeatY -= tile->height;
    }
}

void drawTextAt(const uint16_t x, const uint16_t y, const char *__restrict__ text, const uint8_t colour) {

    size_t len = strlen(text);
    int32_t dstX = (x - 1) * 8;
    int32_t dstY = (y - 1) * 8;
    uint8_t *dstBuffer = &framebuffer[0];
    size_t fontWidth = defaultFont->width;
    uint8_t *fontPixelData = defaultFont->data;
    size_t c;
    int32_t srcX, srcY;

    for (c = 0; c < len; ++c) {
        uint8_t ascii = text[c] - ' ';
        uint8_t line = ascii >> 5;
        uint8_t col = ascii & 31;
        uint8_t *letter =
                fontPixelData + (col * 8) + (fontWidth * (line * 8));

        if (text[c] == '\n' || dstX >= 320) {
            dstX = (x - 1) * 8;
            dstY += 8;
            continue;
        }

        if (text[c] == ' ') {
            dstX += 8;
            continue;
        }

        for (srcY = 0; srcY < 8; ++srcY) {

            uint8_t *letterSrc = letter + (fontWidth * srcY);
            uint8_t *letterDst = dstBuffer + dstX + (320 * (dstY + srcY));

            for (srcX = 0; srcX < 8; ++srcX) {

                if ((*letterSrc) != TRANSPARENCY_COLOR) {
                    *letterDst = colour;
                }

                ++letterSrc;
                ++letterDst;
            }
        }
        dstX += 8;
    }
}
