#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include <assert.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"

uint16_t clippingY1 = 200;

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

    if (x >= XRES || limit < 0 || x == limit) {
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
        if (ix >= 0 && ix < XRES) {

            const FixP_t diffY = (y1 - y0);
            int32_t iY0;
            int32_t iY1 = fixToInt(y1);
            uint8_t *destinationLine;
            int32_t iy;

            if (diffY == zero) {
                continue;
            }

            if (y0 < 0) {
                iY0 = 0;
            } else {
                iY0 = fixToInt(y0);
            }

            if (iY1 >= YRES) {
                iY1 = YRES;
            }

            destinationLine = bufferData + (320 * iY0) + ix;

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
              const uint8_t *__restrict__ texture,
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
    uint8_t pixel = 0;
    FixP_t u = 0;
    uint8_t lastV;
    const uint8_t *data = texture;
    const FixP_t textureSize = intToFix(NATIVE_TEXTURE_SIZE);
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

    if (x >= XRES || limit < 0 || x == limit) {
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

    u = 0;
    /*
       0xFF here acts as a dirty value, indicating there is no last value.
       But even if we had textures this big, it would be only at the end of
       the run.
      we can use this statically, since the textures are already loaded.
      we don't need to fetch that data on every run.
   */

    du = Div(textureSize, dX);
    ix = x;

    for (; ix < limit; ++ix) {
        if (ix >= 0 && ix < XRES) {

            const FixP_t diffY = (y1 - y0);
            FixP_t v = 0;
            int32_t iu = fixToInt(u);
            int32_t iY0 = fixToInt(y0);
            int32_t iY1 = fixToInt(y1);
            const uint8_t *sourceLineStart = data + (iu * NATIVE_TEXTURE_SIZE);
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

                if (iy < YRES && iy >= 0) {
                    const int32_t iv = fixToInt(v);
                    int stipple = !((ix + iy) & 1);

                    if (iv != lastV && !(stipple && farForStipple)) {

                        pixel = *(lineOffset);
                        lineOffset = ((iv & (NATIVE_TEXTURE_SIZE - 1)) + sourceLineStart);
                        lastV = iv;
                    }

                    if (pixel != TRANSPARENCY_COLOR) {
                        uint8_t color = pixel;

                        if (farForStipple && stipple) {
                            color = 0;
                        }

                        *(destinationLine) = color;
                    }
                }
                destinationLine += (320);
                v += dv;
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

    if (_x0 >= XRES) {
        _x0 = XRES - 1;
    }

    if (_x1 >= XRES) {
        _x1 = XRES - 1;
    }

    if (_y0 >= YRES) {
        _y0 = YRES - 1;
    }

    if (_y1 >= YRES) {
        _y1 = YRES - 1;
    }

    fill(_x0, _y0, _x1 - _x0, _y1 - _y0, 0, FALSE);
}

void drawFrontWall(FixP_t x0,
                   FixP_t y0,
                   FixP_t x1,
                   FixP_t y1,
                   const uint8_t *__restrict__ texture,
                   const FixP_t textureScaleY,
                   const int z,
                   const int enableAlpha,
                   const int size) {
    int16_t y;
    int limit;
    FixP_t dY;
    uint8_t pixel = 0;
    FixP_t v = 0;
    uint8_t lastU;
    uint8_t lastV = 0xFF;
    int32_t iy;
    const uint8_t *data = texture;
    const FixP_t textureSize = intToFix(NATIVE_TEXTURE_SIZE);
    FixP_t dv;
    FixP_t diffX;
    int iX0;
    int iX1;
    FixP_t du;
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

    du = Div(textureSize, diffX);

    for (; iy < limit; ++iy) {

        if (iy < YRES && iy >= 0) {
            FixP_t u = 0;
            const uint8_t iv = fixToInt(v) & (NATIVE_TEXTURE_SIZE - 1);
            const uint8_t *sourceLineStart = data + (iv * NATIVE_TEXTURE_SIZE);
            uint8_t *destinationLine = bufferData + (320 * iy) + iX0;
            int ix;
            lastU = 0;

            if (!farEnoughForStipple
                && ((!enableAlpha && iv == lastV)
                    && (iX1 < XRES && iX0 >= 0))) {
                int start = (0 >= iX0) ? 0 : iX0;
                int finish = ((256 - 1) >= iX1) ? iX1 : (256 - 1);
                v += dv;
                destinationLine = bufferData + (320 * iy);
                sourceLineStart = destinationLine - 320;
                memcpy (destinationLine + start, sourceLineStart + start,
                        finish - start);

                continue;
            }

            pixel = *(sourceLineStart);

            for (ix = iX0; ix < iX1; ++ix) {

                if (ix < XRES && ix >= 0) {
                    int stipple = ((ix + iy) & 1);
                    const uint8_t iu = fixToInt(u) & (NATIVE_TEXTURE_SIZE - 1);
                    /*
                                  only fetch the next texel if we really changed the
                                  u, v coordinates (otherwise, would fetch the same
                                  thing anyway)
                                   */
                    if (iu != lastU
                        && !(stipple && farEnoughForStipple)) {

                        pixel = *(sourceLineStart);
                        sourceLineStart += (iu - lastU);
                        lastU = iu;
                        lastV = iv;
                    }

                    if (pixel != TRANSPARENCY_COLOR) {

                        uint8_t color = pixel;

                        if (farEnoughForStipple && stipple) {
                            color = 0;
                        }

                        *(destinationLine) = color;
                    }
                }
                ++destinationLine;
                u += du;
            }
        }
        v += dv;
    }
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /             \
    *  x0y1 /______________\ x1y1
    */
void maskFloor(FixP_t y0, FixP_t y1, FixP_t x0y0, FixP_t x1y0, FixP_t x0y1, FixP_t x1y1, uint8_t pixel) {

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

    if (y == limit || limit < 0 || y >= YRES) {
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
        if (iy < YRES && iy >= 0) {

            int32_t iX0;
            int32_t iX1;
            const FixP_t diffX = (x1 - x0);

            if (diffX == zero) {
                continue;
            }

            if (x0 < 0) {
                iX0 = 0;
            } else {
                iX0 = fixToInt(x0);
            }

            if (x1 < 0) {
                iX1 = 0;
            } else {
                iX1 = fixToInt(x1);
            }

            if (iX1 >= XRES) {
                iX1 = XRES - 1;
            }

            if (iX0 >= XRES) {
                iX0 = XRES - 1;
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
               const uint8_t *__restrict__ texture) {

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
    uint8_t pixel = *texture;
    FixP_t v = 0;
    uint8_t lastU;
    int16_t iy;
    uint8_t *bufferData = &framebuffer[0];
    const uint8_t *data = texture;
    const FixP_t textureSize = intToFix(NATIVE_TEXTURE_SIZE);
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

    if (y == limit || limit < 0 || y >= YRES) {
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

        if (iy < YRES && iy >= 0) {

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
            sourceLineStart = data + (iv * NATIVE_TEXTURE_SIZE);
            pixel = *(sourceLineStart);

            for (ix = iX0; ix < iX1; ++ix) {

                if (ix >= 0 && ix < XRES) {
                    const int32_t iu = fixToInt(u);
                    int stipple = ((ix + iy) & 1) == 0;
                    /*
                                  only fetch the next texel if we really changed the
                                  u, v coordinates (otherwise, would fetch the same
                                  thing anyway)
                                  */
                    if (iu != lastU
                        && !(stipple && farEnoughForStipple)) {

                        pixel = *(sourceLineStart);
                        sourceLineStart += (iu - lastU);
                        lastU = iu;
                    }

                    if (pixel != TRANSPARENCY_COLOR) {
                        uint8_t color = pixel;

                        if (farEnoughForStipple && stipple) {
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


/*
 *     x0y0 ____________ x1y0
 *         /            \
 *        /             \
 *  x0y1 /______________\ x1y1
 */
void drawSlantedFloor(
        FixP_t p0x,
        FixP_t p0y,
        FixP_t p1x,
        FixP_t p1y,
        FixP_t p2x,
        FixP_t p2y,
        FixP_t p3x,
        FixP_t p3y,
        int z,
        const uint8_t *__restrict__ texture) {

    const FixP_t one = intToFix(1);
    uint8_t pixel = 0;
    int farEnoughForStipple = (z >= distanceForPenumbra);
    FixP_t X;
    FixP_t Y;
    FixP_t d01X = one;
    FixP_t d02Y = one;
    FixP_t d03XdY;
    FixP_t currDy0 = 0;
    FixP_t currX0;
    FixP_t fragmentSizeFactor;
    FixP_t currX1;
    int cachedVi;
    FixP_t textureSizeFP = intToFix(NATIVE_TEXTURE_SIZE - 1);


    d03XdY = Div(p0x - p3x, p0y - p3y);
    currX0 = p0x;
    currX1 = p1x;

    fragmentSizeFactor = Div(p2y - p1y, p3y - p0y);
    
    if (farEnoughForStipple) {
        int stipple = FALSE;
        for (Y = p0y; Y < p3y; Y += d02Y) {
            FixP_t percentile = Div((Y - p0y), (p3y - p0y));
            FixP_t targetY = Mul((p2y - p1y), percentile) + p1y;
            FixP_t dydx = Div((targetY - Y), currX1 - currX0);
            currX0 += d03XdY;
            currX1 = Mul((p2x - p1x), percentile) + p1x;
            currDy0 = 0;
            
            cachedVi = (fixToInt(Mul(percentile, textureSizeFP)) * NATIVE_TEXTURE_SIZE);
            
            for (X = currX0; X <= currX1; X += d01X) {
                
                FixP_t percentileX = Div((X - currX0), (currX1 - currX0));
                FixP_t sizeY = Mul(percentileX, fragmentSizeFactor);
                
                pixel = texture[cachedVi + (fixToInt(Mul(percentileX, textureSizeFP)))];
                
                if (sizeY < one) {
                    framebuffer[(320 * (fixToInt(Y + currDy0))) + fixToInt(X)] = pixel;
                } else {
                    int i = 0;
                    FixP_t frag;
                    for (frag = 0; frag <= (sizeY); frag += one) {
                        framebuffer[(320 * (fixToInt(Y + currDy0) + i++)) + fixToInt(X)] = stipple ? pixel : 0;
                        stipple = !stipple;
                    }
                }
                
                currDy0 += dydx;
            }
        }
    } else {
        for (Y = p0y; Y < p3y; Y += d02Y) {
            FixP_t percentile = Div((Y - p0y), (p3y - p0y));
            FixP_t targetY = Mul((p2y - p1y), percentile) + p1y;
            FixP_t dydx = Div((targetY - Y), currX1 - currX0);
            currX0 += d03XdY;
            currX1 = Mul((p2x - p1x), percentile) + p1x;
            currDy0 = 0;
            
            cachedVi = (fixToInt(Mul(percentile, textureSizeFP)) * NATIVE_TEXTURE_SIZE);
            
            for (X = currX0; X <= currX1; X += d01X) {
                
                FixP_t percentileX = Div((X - currX0), (currX1 - currX0));
                FixP_t sizeY = Mul(percentileX, fragmentSizeFactor);
                
                pixel = texture[cachedVi + (fixToInt(Mul(percentileX, textureSizeFP)))];
                
                if (sizeY < one) {
                    framebuffer[(320 * (fixToInt(Y + currDy0))) + fixToInt(X)] = pixel;
                } else {
                    int i = 0;
                    FixP_t frag;
                    for (frag = 0; frag <= (sizeY); frag += one) {
                        framebuffer[(320 * (fixToInt(Y + currDy0) + i++)) + fixToInt(X)] = pixel;
                    }
                }
                
                currDy0 += dydx;
            }
        }
    }
}

void drawRect(
        const int x,
        const int y,
        const unsigned int dx,
        const unsigned int dy,
        const uint8_t pixel) {

    uint8_t *destination = &framebuffer[0];
    uint8_t *destinationLineStart = destination + (320 * (y)) + x;
    uint16_t py;

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

void fillBottomFlat(const int *coords, uint8_t colour) {
    int y = coords[1];
    FixP_t dXDy2;
    FixP_t dXDy1;
    FixP_t fX0;
    FixP_t fX1;
    FixP_t dX1X0;
    FixP_t dX0X2;
    FixP_t dY1Y0;
    FixP_t dY2Y0;
    int yFinal = min(coords[3], coords[5]);

    FixP_t x0 = intToFix(coords[0]);
    FixP_t y0 = intToFix(coords[1]);
    FixP_t x1 = intToFix(coords[2]);
    FixP_t y1 = intToFix(coords[3]);
    FixP_t x2 = intToFix(coords[4]);
    FixP_t y2 = intToFix(coords[5]);


    dX1X0 = (x1 - x0);
    dX0X2 = (x0 - x2);
    dY1Y0 = (y1 - y0);
    dY2Y0 = (y2 - y0);

    if (dY2Y0 == 0 || dY1Y0 == 0) {
        return;
    }

    dXDy2 = Div(dX0X2, dY2Y0);
    dXDy1 = Div(dX1X0, dY1Y0);
    fX0 = x0;
    fX1 = x0;

    for (; y < yFinal; ++y) {
        if (y >= 200) {
            return;
        } else if (y >= 0) {
            int iFX1 = max(min(255, fixToInt(fX1)), 0);
            int iFX0 = max(min(255, fixToInt(fX0)), 0);
            uint8_t *destination = &framebuffer[(320 * y) + min(iFX0, iFX1)];
            memset(destination, colour, abs(iFX1 - iFX0));
        }
        fX0 -= dXDy2;
        fX1 += dXDy1;


    }
}


void fillTopFlat(int *coords, uint8_t colour) {
    int y = coords[1];
    int yFinal = max(coords[3], coords[5]);

    FixP_t x0 = intToFix(coords[0]);
    FixP_t y0 = intToFix(coords[1]);
    FixP_t x1 = intToFix(coords[2]);
    FixP_t y1 = intToFix(coords[3]);
    FixP_t x2 = intToFix(coords[4]);
    FixP_t y2 = intToFix(coords[5]);
    
    FixP_t dXDy1;
    FixP_t dXDy2;
    FixP_t fX0;
    FixP_t fX1;


    FixP_t dX1X0 = (x1 - x0);
    FixP_t dX2X0 = (x2 - x0);
    FixP_t dY0Y1 = (y0 - y1);
    FixP_t dY0Y2 = (y0 - y2);

    if (dY0Y1 == 0 || dY0Y2 == 0) {
        return;
    }

    dXDy1 = Div(dX1X0, dY0Y1);
    dXDy2 = Div(dX2X0, dY0Y2);
    fX0 = x0; //p1
    fX1 = x0; //p2

    for (; y >= yFinal; --y) {
        if (y < 0) {
            return;
        } else if (y < 200) {
            int iFX1 = max(min(255, fixToInt(fX1)), 0);
            int iFX0 = max(min(255, fixToInt(fX0)), 0);
            uint8_t *destination = &framebuffer[(320 * y) + min(iFX0, iFX1)];
            memset(destination, colour, abs(iFX1 - iFX0));
        }

        fX0 += dXDy1;
        fX1 += dXDy2;
    }
}


void fillTriangle(int *coords, uint8_t colour) {
    int newCoors[6];
    int newCoors2[6];

    int upper = -1;
    int lower = -1;
    int other = 0;
	int c;
	
    for (c = 0; c < 3; ++c) {
        if (upper == -1 || coords[(2 * c) + 1] < coords[(2 * upper) + 1]) {
            upper = c;
        }

        if (lower == -1 || coords[(2 * c) + 1] > coords[(2 * lower) + 1]) {
            lower = c;
        }
    }

    if (lower == 0 || upper == 0) {
        other = 1;
    }

    if ((lower == 1 || upper == 1) && (other == 1)) {
        other = 2;
    }

    if ((lower == 2 || upper == 2) && (other == 2)) {
        other = 0;
    }

    newCoors[0] = coords[2 * upper];
    newCoors[1] = coords[(2 * upper) + 1];
    newCoors[2] = coords[2 * lower];
    newCoors[3] = coords[(2 * lower) + 1];
    newCoors[4] = coords[2 * other];
    newCoors[5] = coords[(2 * other) + 1];

    newCoors2[0] = coords[2 * lower];
    newCoors2[1] = coords[(2 * lower) + 1];
    newCoors2[2] = coords[2 * other];
    newCoors2[3] = coords[(2 * other) + 1];
    newCoors2[4] = coords[2 * upper];
    newCoors2[5] = coords[(2 * upper) + 1];


    fillBottomFlat(&newCoors[0], colour);
    fillTopFlat(&newCoors2[0], colour);
}


void drawTexturedBottomFlatTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture) {
    int y = coords[1];
    int u, v;
    FixP_t fU1, fU2, fV1, fV2;
    FixP_t fDU1;
    FixP_t fDU2;
    FixP_t fDV1;
    FixP_t fDV2;
    FixP_t one = intToFix(1);

    int yFinal = coords[5]; //not the lowest, neither the topmost

    FixP_t x0 = intToFix(coords[0]);
    FixP_t y0 = intToFix(coords[1]);
    FixP_t x1 = intToFix(coords[2]);
    FixP_t y1 = intToFix(coords[3]);
    FixP_t x2 = intToFix(coords[4]);
    FixP_t y2 = intToFix(coords[5]);

    FixP_t u0 = intToFix(uvCoords[0]);
    FixP_t v0 = intToFix(uvCoords[1]);
    FixP_t u1 = intToFix(uvCoords[2]);
    FixP_t v1 = intToFix(uvCoords[3]);
    FixP_t u2 = intToFix(uvCoords[4]);
    FixP_t v2 = intToFix(uvCoords[5]);
    FixP_t effectiveDelta;

    FixP_t dX1X0 = (x1 - x0);
    FixP_t dX0X2 = (x0 - x2);
    FixP_t dY1Y0 = (y1 - y0);
    FixP_t dY2Y0 = (y2 - y0);
    
    FixP_t dXDy2;
    FixP_t dXDy1;
    FixP_t fX0;
    FixP_t fX1;

    if (dY2Y0 == 0 || dY1Y0 == 0) {
        return;
    }

    dXDy2 = Div(dX0X2, dY2Y0);
    dXDy1 = Div(dX1X0, dY1Y0);
    fX0 = x0;
    fX1 = x0;

    fV1 = fV2 = v0;
    fU1 = fU2 = u0;

#ifndef FASTER_TEXTURE_MAP
    effectiveDelta = intToFix((coords[5]) - y);
    fDU1 = Div((u2 - u0), effectiveDelta);
    fDV1 = Div((v2 - v0), effectiveDelta);

    effectiveDelta = (intToFix((coords[3]) - y));
    fDU2 = Div((u1 - u0), effectiveDelta);
    fDV2 = Div((v1 - v0), effectiveDelta);
#else
    effectiveDelta = Div(one, intToFix((coords[5]) - y));
    fDU1 = Mul((u2 - u0), effectiveDelta);
    fDV1 = Mul((v2 - v0), effectiveDelta);

    effectiveDelta = Div(one, intToFix((coords[3]) - y));
    fDU2 = Mul((u1 - u0), effectiveDelta);
    fDV2 = Mul((v1 - v0), effectiveDelta);
#endif


    for (; y < yFinal; ++y) {

        int iFX1;
        int iFX0;
        int flipped;
        FixP_t texelLineX;
        FixP_t texelLineY;
        FixP_t texelLineDX;
        FixP_t texelLineDY;
        FixP_t oneOverLimit;
        int limit;
        
        fU1 += fDU1;
        fV1 += fDV1;
        fU2 += fDU2;
        fV2 += fDV2;

        flipped = (fX0 > fX1);

        if (flipped) {
            iFX1 = fixToInt(fX0);
            iFX0 = fixToInt(fX1);
        } else {
            iFX1 = fixToInt(fX1);
            iFX0 = fixToInt(fX0);
        }

		limit = iFX1 - iFX0;

        if (limit) {
	        uint8_t *destination;
            oneOverLimit = Div(one, intToFix(limit));

            destination = &framebuffer[(320 * y) + iFX0];

            if (flipped) {
                texelLineDX = Mul((fU1 - fU2), oneOverLimit);
                texelLineDY = Mul((fV1 - fV2), oneOverLimit);
                texelLineX = fU2;
                texelLineY = fV2;
            } else {
                texelLineDX = Mul((fU2 - fU1), oneOverLimit);
                texelLineDY = Mul((fV2 - fV1), oneOverLimit);
                texelLineX = fU1;
                texelLineY = fV1;
            }

            if (y >= 0 && y <= YRES) {

                int xPos = iFX0;

                while (limit--) {
                    u = abs(fixToInt(texelLineX)) % NATIVE_TEXTURE_SIZE;
                    v = abs(fixToInt(texelLineY)) % NATIVE_TEXTURE_SIZE;

                    if (xPos >= 0 && xPos <= XRES) {
                        *destination = *(&texture->rowMajor[0] + (NATIVE_TEXTURE_SIZE * v) + u);
                    }
                    ++xPos;
                    ++destination;
                    texelLineX += texelLineDX;
                    texelLineY += texelLineDY;
                }
            }
        }
        fX0 -= dXDy2;
        fX1 += dXDy1;
    }
}


void drawTexturedTopFlatTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture) {
    int y = coords[1];
    int u, v;
    FixP_t fU1, fU2, fV1, fV2;
    FixP_t fDU1;
    FixP_t fDV1;
    FixP_t fDU2;
    FixP_t fDV2;

    int yFinal = coords[3]; //not the upper, not the lowest
    FixP_t one = intToFix(1);
    FixP_t x0 = intToFix(coords[0]);
    FixP_t y0 = intToFix(coords[1]);
    FixP_t x1 = intToFix(coords[2]);
    FixP_t y1 = intToFix(coords[3]);
    FixP_t x2 = intToFix(coords[4]);
    FixP_t y2 = intToFix(coords[5]);

    FixP_t u0 = intToFix(uvCoords[0]);
    FixP_t v0 = intToFix(uvCoords[1]);
    FixP_t u1 = intToFix(uvCoords[2]);
    FixP_t v1 = intToFix(uvCoords[3]);
    FixP_t u2 = intToFix(uvCoords[4]);
    FixP_t v2 = intToFix(uvCoords[5]);


    FixP_t dX1X0 = (x1 - x0);
    FixP_t dX2X0 = (x2 - x0);
    FixP_t dY0Y1 = (y0 - y1);
    FixP_t dY0Y2 = (y0 - y2);
    
    FixP_t dXDy1;
    FixP_t dXDy2;
    FixP_t fX0;
    FixP_t fX1;
    FixP_t effectiveDelta;
    
    if (dY0Y1 == 0 || dY0Y2 == 0) {
        return;
    }

    dXDy1 = Div(dX1X0, dY0Y1);
    dXDy2 = Div(dX2X0, dY0Y2);
    fX0 = x0; //p1
    fX1 = x0; //p2

    fV1 = fV2 = v0;
    fU1 = fU2 = u0;

#ifndef FASTER_TEXTURE_MAP
    effectiveDelta = (intToFix(y - (coords[3])));
    fDU1 = Div((u1 - u0), effectiveDelta);
    fDV1 = Div((v1 - v0), effectiveDelta);

    effectiveDelta = (intToFix(y - (coords[5])));
    fDU2 = Div((u2 - u0), effectiveDelta);
    fDV2 = Div((v2 - v0), effectiveDelta);
#else
    effectiveDelta = Div(one, intToFix(y - (coords[3])));
    fDU1 = Mul((u1 - u0), effectiveDelta);
    fDV1 = Mul((v1 - v0), effectiveDelta);

    effectiveDelta = Div(one, intToFix(y - (coords[5])));
    fDU2 = Mul((u2 - u0), effectiveDelta);
    fDV2 = Mul((v2 - v0), effectiveDelta);
#endif


    for (; y >= yFinal; --y) {
        int iFX1;
        int iFX0;
		int flipped;
        FixP_t texelLineX;
        FixP_t texelLineY;
        FixP_t texelLineDX;
        FixP_t texelLineDY;
        FixP_t oneOverLimit;
		int limit;
		
        fU1 += fDU1;
        fV1 += fDV1;
        fU2 += fDU2;
        fV2 += fDV2;

        flipped = (fX0 > fX1);

        if (flipped) {
            iFX1 = fixToInt(fX0);
            iFX0 = fixToInt(fX1);
        } else {
            iFX1 = fixToInt(fX1);
            iFX0 = fixToInt(fX0);
        }

        limit = iFX1 - iFX0;

        if (limit) {
	        uint8_t *destination;
            oneOverLimit = Div(one, intToFix(limit));

            destination = &framebuffer[(320 * y) + iFX0];

            if (flipped) {
                texelLineDX = Mul((fU1 - fU2), oneOverLimit);
                texelLineDY = Mul((fV1 - fV2), oneOverLimit);
                texelLineX = fU2;
                texelLineY = fV2;
            } else {
                texelLineDX = Mul((fU2 - fU1), oneOverLimit);
                texelLineDY = Mul((fV2 - fV1), oneOverLimit);
                texelLineX = fU1;
                texelLineY = fV1;
            }

            if (y >= 0 && y <= YRES) {

                int xPos = iFX0;


                while (limit--) {
                    u = abs(fixToInt(texelLineX)) % NATIVE_TEXTURE_SIZE;
                    v = abs(fixToInt(texelLineY)) % NATIVE_TEXTURE_SIZE;

                    if (xPos >= 0 && xPos <= XRES) {
                        *destination = *(&texture->rowMajor[0] + (NATIVE_TEXTURE_SIZE * v) + u);
                    }
                    ++xPos;
                    ++destination;
                    texelLineX += texelLineDX;
                    texelLineY += texelLineDY;
                }
            }
        }

        fX0 += dXDy1;
        fX1 += dXDy2;
    }
}

void drawTexturedTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture) {
    int newCoors[6];
    uint8_t newUV[6];
	int c;
    int upper = -1;
    int lower = -1;
    int other = 0;

    for (c = 0; c < 3; ++c) {
        if (upper == -1 || coords[(2 * c) + 1] < coords[(2 * upper) + 1]) {
            upper = c;
        }

        if (lower == -1 || coords[(2 * c) + 1] > coords[(2 * lower) + 1]) {
            lower = c;
        }
    }

    if (lower == 0 || upper == 0) {
        other = 1;
    }

    if ((lower == 1 || upper == 1) && (other == 1)) {
        other = 2;
    }

    if ((lower == 2 || upper == 2) && (other == 2)) {
        other = 0;
    }

    newCoors[0] = coords[2 * upper];
    newCoors[1] = coords[(2 * upper) + 1];
    newCoors[2] = coords[2 * lower];
    newCoors[3] = coords[(2 * lower) + 1];
    newCoors[4] = coords[2 * other];
    newCoors[5] = coords[(2 * other) + 1];

    newUV[0] = 32 - uvCoords[(2 * upper) + 1];
    newUV[1] = uvCoords[2 * upper];

    newUV[2] = 32 - uvCoords[(2 * lower) + 1];
    newUV[3] = uvCoords[2 * lower];

    newUV[4] = 32 - uvCoords[(2 * other) + 1];
    newUV[5] = uvCoords[2 * other];


    drawTexturedBottomFlatTriangle(&newCoors[0], &newUV[0], texture);

    newCoors[0] = coords[2 * lower];
    newCoors[1] = coords[(2 * lower) + 1];
    newCoors[2] = coords[2 * other];
    newCoors[3] = coords[(2 * other) + 1];
    newCoors[4] = coords[2 * upper];
    newCoors[5] = coords[(2 * upper) + 1];

    newUV[0] = 32 - uvCoords[(2 * lower) + 1];
    newUV[1] = uvCoords[2 * lower];

    newUV[2] = 32 - uvCoords[(2 * other) + 1];
    newUV[3] = uvCoords[2 * other];

    newUV[4] = 32 - uvCoords[(2 * upper) + 1];
    newUV[5] = uvCoords[2 * upper];


    drawTexturedTopFlatTriangle(&newCoors[0], &newUV[0], texture);
}

void fill(
        const int x,
        const int y,
        const unsigned int dx,
        const unsigned int dy,
        const uint8_t pixel,
        const int stipple) {

    uint8_t *destination = &framebuffer[0];
    unsigned int py;
	uint8_t *destinationLineStart;

    if (pixel == TRANSPARENCY_COLOR) {
        return;
    }

	destinationLineStart = destination + (320 * y) + x;
	
    for (py = 0; py < dy; ++py) {
        if (!stipple) {
            memset (destinationLineStart, pixel, dx);
            destinationLineStart += 320;
        } else {
            unsigned int px;
            for (px = 0; px < dx; ++px) {
                destinationLineStart++;
                if ((px + py) & 1) {
                    *destinationLineStart = pixel;
                }
            }
            destinationLineStart += 320 - dx;
        }
    }
}

void drawBitmapRaw(const int dx,
                const int dy,
                int width,
                int height,
                uint8_t *bitmapData,
                const int transparent) {

    uint8_t *destination = &framebuffer[0];
    uint8_t *sourceLine = bitmapData;

    int y;

    if ((dy + height) >= 200) {
        height = (200 - dy);
    }
    for (y = 0; y < height; ++y) {
        uint8_t *destinationLineStart = destination + (320 * (dy + y)) + dx;
        uint8_t *sourceLineStart = sourceLine + (width * y);
        int x;

        if ((dy + y) >= clippingY1) {
            return;
        }

        for (x = 0; x < width; ++x) {
            uint8_t pixel = *sourceLineStart;

            if (!transparent || (pixel != TRANSPARENCY_COLOR)) {
                *destinationLineStart = pixel;
            }

            ++sourceLineStart;
            ++destinationLineStart;
        }
    }
}

void drawBitmap(const int dx,
                const int dy,
                const struct Bitmap *__restrict__ tile,
                const int transparent) {

    drawBitmapRaw(dx, dy, tile->width, tile->height, tile->data, transparent);
}

void drawRepeatBitmap(
        const int x,
        const int y,
        const unsigned int dx,
        const unsigned int dy,
        const struct Bitmap *__restrict__ tile) {

    int repeatX = (dx / tile->width) + 1;
    int repeatY = (dy / tile->height) + 1;
    int c, d;
    for (c = 0; c < repeatY; ++c) {
        for (d = 0; d < repeatX; ++d) {

            int px = d * tile->width;
            int py = c * tile->height;

            if (px < 320 && py < 200) {
                drawBitmap(x + px, y + py, tile, FALSE);
            }
        }
    }
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char *__restrict__ text, const uint8_t colour) {

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

        if (text[c] == '\n' || dstX >= margin) {
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

void drawTextAt(const int x, const int y, const char *__restrict__ text, const uint8_t colour) {

    drawTextAtWithMargin( x, y, 319, text, colour);
}
