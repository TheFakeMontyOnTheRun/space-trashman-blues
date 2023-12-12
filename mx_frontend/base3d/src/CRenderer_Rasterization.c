#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include <assert.h>
#include <stdio.h>

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


#define PAGE_FLIP_INCREMENT 32
#define HUD_WIDTH (XRES_FRAMEBUFFER - XRES)
#define FIXP_NATIVE_TEXTURE_SIZE  (intToFix(NATIVE_TEXTURE_SIZE))
#define FIXP_YRES intToFix(YRES)
char mTurnBuffer;

uint16_t clippingY1 = YRES_FRAMEBUFFER;

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
#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

void maskWall(
        FixP_t x0,
        FixP_t x1,
        FixP_t x0y0,
        FixP_t x0y1,
        FixP_t x1y0,
        FixP_t x1y1) {

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
    int32_t ix;
    uint8_t *bufferData = &framebuffer[0];

    if (x0 > x1) {
        FixP_t tmp = x0;
        x0 = x1;
        x1 = tmp;

        tmp = x0y0;
        x0y0 = x1y0;
        x1y0 = tmp;

        tmp = x0y1;
        x0y1 = x1y1;
        x1y1 = tmp;
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

    if (dX == 0) {
        return;
    } else {
        FixP_t oneOverDx;

        if (dX > 0 && dX < intToFix(320)) {
            oneOverDx = divLut[limit - x];
        } else if (dX < 0 && dX > -intToFix(320)) {
            oneOverDx = -divLut[-limit + x];
        } else {
            oneOverDx = Div(intToFix(1), dX);
        }

        upperDyDx = Mul(upperDy, oneOverDx);
        lowerDyDx = Mul(lowerDy, oneOverDx);
    }

    /*
      0xFF here acts as a dirty value, indicating there is no last value.
      But even if we had textures this big, it would be only at the end of
      the run.
      we can use this statically, since the textures are already loaded.
      we don't need to fetch that data on every run.
  */
    ix = x;

    if (ix < 0) {
        FixP_t diff = intToFix(-ix);
        y0 += Mul(diff, upperDyDx);
        y1 += Mul(diff, lowerDyDx);
        ix = 0;
    }

    if (limit > XRES) {
        limit = XRES;
    }

    for (; ix < limit; ++ix) {

        const FixP_t diffY = (y1 - y0);
        int32_t iY0;
        int32_t iY1 = fixToInt(y1);
        uint8_t *destinationLine;
        int32_t iy;

        if (diffY == 0) {
            continue;
        }

        if (y0 < 0) {
            iY0 = 0;
        } else {
            iY0 = fixToInt(y0);
        }

        if (iY1 >= YRES) {
            iY1 = YRES;
            continue;
        }

        destinationLine = bufferData + (XRES_FRAMEBUFFER * iY0) + ix;

        for (iy = iY0; iy < iY1; ++iy) {
            *(destinationLine) = 0;
            destinationLine += (XRES_FRAMEBUFFER);
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

#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

void drawWall(FixP_t x0,
              FixP_t x1,
              FixP_t x0y0,
              FixP_t x0y1,
              FixP_t x1y0,
              FixP_t x1y1,
              const TexturePixelFormat *texture,
              const FixP_t textureScaleY,
              const int z) {
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
    FixP_t lastDiffY = 0xFFFFFFFF;
    const uint8_t *data = texture;

    FixP_t du;
    int32_t ix;
    uint8_t *bufferData = &framebuffer[0];
    int farForStipple = (z >= distanceForPenumbra);

    if (x0 > x1) {
        FixP_t tmp = x0;
        x0 = x1;
        x1 = tmp;

        tmp = x0y0;
        x0y0 = x1y0;
        x1y0 = tmp;

        tmp = x0y1;
        x0y1 = x1y1;
        x1y1 = tmp;
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
    if (dX == 0) {
        return;
    } else {
        FixP_t oneOverDx;

        if (dX > 0 && dX < intToFix(320)) {
            oneOverDx = divLut[limit - x];
        } else if (dX < 0 && dX > -intToFix(320)) {
            oneOverDx = -divLut[-limit + x];
        } else {
            oneOverDx = Div(intToFix(1), dX);
        }

        upperDyDx = Mul(upperDy, oneOverDx);
        lowerDyDx = Mul(lowerDy, oneOverDx);
        du = Mul(FIXP_NATIVE_TEXTURE_SIZE, oneOverDx);
    }

    u = 0;

    ix = x;

    if (ix < 0) {
        FixP_t diff = intToFix((-ix));
        y0 += Mul(diff, upperDyDx);
        y1 += Mul(diff, lowerDyDx);
        u += Mul(diff, du);
        ix = 0;
    }

    if (limit > XRES) {
        limit = XRES;
    }

    for (; ix < limit; ++ix) {

        const FixP_t diffY = (y1 - y0);
        FixP_t v = 0;
        int32_t iu = fixToInt(u);
        int32_t iY0 = fixToInt(y0);
        int32_t iY1 = fixToInt(y1);
        const uint8_t *sourceLineStart = data + (iu * NATIVE_TEXTURE_SIZE);
        const uint8_t *lineOffset = sourceLineStart;
        uint8_t *destinationLine = bufferData + (XRES_FRAMEBUFFER * iY0) + ix;
        /*
            Yes, this would cause a div by zero...but the lastDiffY is set to be different from diffY, causing it be
            initialized with the Div between the texture size and the diffY
        */
        FixP_t dv = 0;
        int32_t iy;
        int stipple;

        if (diffY == 0) {
            continue;
        }

        if (diffY != lastDiffY) {
            dv = Div(Mul(FIXP_NATIVE_TEXTURE_SIZE, textureScaleY), diffY);
            lastDiffY = diffY;
        }

        lastV = 0;
        pixel = *(lineOffset);
        iy = iY0;

        if (iY1 >= YRES) {
            iY1 = YRES;
        }

        if (iy < 0) {
            FixP_t diff = intToFix((-iy));
            v += Mul(diff, dv);
            destinationLine += -iy * (XRES_FRAMEBUFFER);
            iy = 0;
        }

        stipple = (((ix + iy) & 1)) ? 0xFFFFFFFF : 0;

        for (; iy < iY1; ++iy) {

            const int32_t iv = fixToInt(v);

            stipple = ~stipple;

            if (iv != lastV && !(stipple && farForStipple)) {

                pixel = *(lineOffset);
                lineOffset = ((iv & (NATIVE_TEXTURE_SIZE - 1)) + sourceLineStart);
                lastV = iv;
            }

            if (pixel != TRANSPARENCY_COLOR) {
                *(destinationLine) = (farForStipple && stipple) ? 0 : pixel;
            }

            destinationLine += (XRES_FRAMEBUFFER);
            v += dv;
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

    fillRect(_x0, _y0, _x1 - _x0, _y1 - _y0, 0, FALSE);
}

#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

void drawFrontWall(FixP_t x0,
                   FixP_t y0,
                   FixP_t x1,
                   FixP_t y1,
                   const TexturePixelFormat *texture,
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
    FixP_t dv;
    FixP_t diffX;
    int iX0;
    int iX1;
    FixP_t du;
    uint8_t *bufferData = &framebuffer[0];
    int farEnoughForStipple = (z >= distanceForPenumbra);

    /* if we have a quad in which the base is smaller */
    if (y0 > y1) {
        FixP_t tmp = y0;
        y0 = y1;
        y1 = tmp;
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

    iy = (int32_t) (y);
    dv = Div(Mul(FIXP_NATIVE_TEXTURE_SIZE, textureScaleY) - intToFix(1), dY);
    diffX = (x1 - x0);
    iX0 = fixToInt(x0);
    iX1 = fixToInt(x1);

    if (iX0 == iX1) {
        /* degenerate case */
        return;
    }

    if (diffX > 0 && diffX < intToFix(320)) {
        du = Mul(FIXP_NATIVE_TEXTURE_SIZE, divLut[iX1 - iX0]);
    } else if (diffX < 0 && diffX > -intToFix(320)) {
        du = -Mul(FIXP_NATIVE_TEXTURE_SIZE, divLut[iX0 - iX1]);
    } else {
        du = Div(FIXP_NATIVE_TEXTURE_SIZE, diffX);
    }


    if (iy < 0) {
        FixP_t diff = intToFix(-iy);
        v += Mul(diff, dv);
        iy = 0;
    }

    if (limit > YRES) {
        limit = YRES;
    }

    for (; iy < limit; ++iy) {

        FixP_t u = 0;
        const uint8_t iv = fixToInt(v) & (NATIVE_TEXTURE_SIZE - 1);
        const uint8_t *sourceLineStart = data + (iv * NATIVE_TEXTURE_SIZE);
        uint8_t *destinationLine = bufferData + (XRES_FRAMEBUFFER * iy) + iX0;
        int ix;
        int stipple;
        lastU = 0;

        if (!farEnoughForStipple
            && ((!enableAlpha && iv == lastV)
                && (iX1 < XRES && iX0 >= 0))) {
            int start = (0 >= iX0) ? 0 : iX0;
            int finish = ((XRES - 1) >= iX1) ? iX1 : (XRES - 1);
            v += dv;
            destinationLine = bufferData + (XRES_FRAMEBUFFER * iy);
            sourceLineStart = destinationLine - XRES_FRAMEBUFFER;
            memCopyToFrom(destinationLine + start, (void *) (sourceLineStart + start),
                          finish - start);

            continue;
        }

        pixel = *(sourceLineStart);
        ix = iX0;

        if (ix < 0) {
            FixP_t diff = intToFix(-ix);
            destinationLine += -ix;
            u += Mul(diff, du);
            ix = 0;

        }

        if (iX1 >= XRES) {
            iX1 = XRES;
        }

        stipple = (((ix + iy) & 1)) ? 0xFFFFFFFF : 0;

        for (; ix < iX1; ++ix) {
            const uint8_t iu = fixToInt(u) & (NATIVE_TEXTURE_SIZE - 1);
            stipple = ~stipple;
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
                *(destinationLine) = (farEnoughForStipple && stipple) ? 0 : pixel;
            }
            ++destinationLine;
            u += du;
        }
        v += dv;
    }
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /              \
    *  x0y1 /________________\ x1y1
    */
#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

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
    FixP_t x0;
    FixP_t x1;
    uint8_t *bufferData = &framebuffer[0];
    int32_t iy;

    /* if we have a trapezoid in which the base is smaller */
    if (y0 > y1) {
        FixP_t tmp = y0;
        y0 = y1;
        y1 = tmp;

        tmp = x0y0;
        x0y0 = x0y1;
        x0y1 = tmp;

        tmp = x1y0;
        x1y0 = x1y1;
        x1y1 = tmp;
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

    if (dY == 0) {
        return;
    } else {
        FixP_t oneOverDy;
        if (dY > 0 && dY < intToFix(320)) {
            oneOverDy = divLut[limit - y];
        } else if (dY < 0 && dY > -intToFix(320)) {
            oneOverDy = -divLut[-limit + y];
        } else {
            oneOverDy = Div(intToFix(1), dY);
        }

        leftDxDy = Mul(leftDX, oneOverDy);
        rightDxDy = Mul(rightDX, oneOverDy);
    }

    x0 = upperX0;
    x1 = upperX1;
    iy = y;

    if (limit > YRES) {
        limit = YRES;
    }

    if (iy < 0) {
        FixP_t diff = intToFix(-iy);
        x0 += Mul(diff, leftDxDy);
        x1 += Mul(diff, rightDxDy);
        iy = 0;
    }


    for (; iy < limit; ++iy) {
        int32_t iX0;
        int32_t iX1;

        const FixP_t diffX = (x1 - x0);

        if (diffX == 0) {
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

        memFill(bufferData + (XRES_FRAMEBUFFER * iy) + iX0, pixel, iX1 - iX0);


        x0 += leftDxDy;
        x1 += rightDxDy;
    }
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /              \
    *  x0y1 /________________\ x1y1
    */
#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

void drawFloor(FixP_t y0,
               FixP_t y1,
               FixP_t x0y0,
               FixP_t x1y0,
               FixP_t x0y1,
               FixP_t x1y1,
               int z,
               const uint8_t *texture) {

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
    FixP_t x0;
    FixP_t x1;
    uint8_t pixel;
    FixP_t v = 0;
    int32_t iy;
    uint8_t *bufferData;
    FixP_t dv;
    FixP_t lastDiffX = 0xFFFFFFFF;
    const uint8_t *sourceLineStart;
    int farEnoughForStipple;

    if (y0 == y1) {
        return;
    }

    /* if we have a trapezoid in which the base is smaller */
    if (y0 > y1) {
        FixP_t tmp = y0;
        y0 = y1;
        y1 = tmp;

        tmp = x0y0;
        x0y0 = x0y1;
        x0y1 = tmp;

        tmp = x1y0;
        x1y0 = x1y1;
        x1y1 = tmp;
    }

    if (y1 < 0 || y0 >= FIXP_YRES) {
        return;
    }

    bufferData = &framebuffer[0];
    farEnoughForStipple = (z >= distanceForPenumbra);

    y = fixToInt(y0);
    limit = fixToInt(y1);

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

    if (dY == 0) {
        return;
    } else {
        FixP_t oneOverDy;
        if (dY > 0 && dY < intToFix(320)) {
            oneOverDy = divLut[limit - y];
        } else if (dY < 0 && dY > -intToFix(320)) {
            oneOverDy = -divLut[-limit + y];
        } else {
            oneOverDy = Div(intToFix(1), dY);
        }

        leftDxDy = Mul(leftDX, oneOverDy);
        rightDxDy = Mul(rightDX, oneOverDy);
        dv = Mul(FIXP_NATIVE_TEXTURE_SIZE, oneOverDy);
    }

    x0 = upperX0;
    x1 = upperX1;
    iy = y;

    /* Are we already withing the visible part of the screen? Why not jump straight to it?*/
    if (iy < 0) {
        FixP_t diff = intToFix((-iy));
        x0 += Mul(diff, leftDxDy);
        x1 += Mul(diff, rightDxDy);
        v += Mul(diff, dv);
        iy = 0;
    }

    if (limit > YRES) {
        limit = YRES;
    }

    for (; iy < limit; ++iy) {

        int32_t iX0;
        int32_t iX1;
        int32_t ix;
        FixP_t du;
        uint8_t *destinationLine;
        const FixP_t diffX = (x1 - x0);
        uint8_t lastU = 0;
        FixP_t u = 0;
        int stipple;

        if (diffX == 0) {
            continue;
        }

        ix = iX0 = fixToInt(x0);
        iX1 = fixToInt(x1);

        if (diffX != lastDiffX) {
            if (diffX > 0 && diffX < intToFix(320)) {
                du = Mul(FIXP_NATIVE_TEXTURE_SIZE, divLut[iX1 - iX0]);
            } else if (diffX < 0 && diffX > -intToFix(320)) {
                du = -Mul(FIXP_NATIVE_TEXTURE_SIZE, divLut[iX0 - iX1]);
            } else {
                du = Div(FIXP_NATIVE_TEXTURE_SIZE, diffX);
            }
            lastDiffX = diffX;
        }

        if (ix < 0) {
            FixP_t diff = intToFix((-ix));
            u += Mul(diff, du);
            ix = 0;
        }

        destinationLine = bufferData + (XRES_FRAMEBUFFER * iy) + ix;
        sourceLineStart = texture + (fixToInt(v) * NATIVE_TEXTURE_SIZE);
        pixel = *(sourceLineStart);
        stipple = ((iX0 + iy) & 1) ? 0xFFFFFFFF : 0;

        if (iX1 >= XRES) {
            iX1 = XRES;
        }

        if (!farEnoughForStipple) {
            for (; ix < iX1; ++ix) {
                const int32_t iu = fixToInt(u);
                /*
                  only fetch the next texel if we really changed the
                  u, v coordinates (otherwise, would fetch the same
                  thing anyway)
                */
                if (iu != lastU) {
                    pixel = *(sourceLineStart);
                    sourceLineStart += (iu - lastU);
                    lastU = iu;
                }

                *(destinationLine++) = pixel;
                u += du;
            }
        } else {
            for (; ix < iX1; ++ix) {
                const int32_t iu = fixToInt(u);
                stipple = ~stipple;
                /*ditto, but only if the stippling is not active for this fragment*/
                if (!stipple &&
                    iu != lastU) {

                    pixel = *(sourceLineStart);
                    sourceLineStart += (iu - lastU);
                    lastU = iu;
                }

                *(destinationLine++) = (stipple) ? 0 : pixel;
                u += du;
            }
        }


        x0 += leftDxDy;
        x1 += rightDxDy;
        v += dv;
    }
}

void drawRect(const int x,
              const int y,
              const size_t dx,
              const size_t dy,
              const FramebufferPixelFormat pixel) {

    uint8_t *destination = &framebuffer[0];
    uint8_t *destinationLineStart = destination + (XRES_FRAMEBUFFER * (y)) + x;
    uint16_t py;

    if (pixel == TRANSPARENCY_COLOR) {
        return;
    }

    memFill(destinationLineStart, pixel, dx);

    for (py = 0; py < (dy); ++py) {
        destinationLineStart = destination + (XRES_FRAMEBUFFER * (y + py)) + x;
        *destinationLineStart = pixel;
        destinationLineStart += dx;
        *destinationLineStart = pixel;
    }
    memFill(destination + (XRES_FRAMEBUFFER * (y + dy)) + x, pixel, dx);
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
        if (y >= YRES) {
            return;
        } else if (y >= 0) {
            int iFX1 = max(min((XRES - 1), fixToInt(fX1)), 0);
            int iFX0 = max(min((XRES - 1), fixToInt(fX0)), 0);
            uint8_t *destination = &framebuffer[(XRES_FRAMEBUFFER * y) + min(iFX0, iFX1)];
            memFill(destination, colour, abs(iFX1 - iFX0));
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
    fX0 = x0; /* p1 */
    fX1 = x0; /* p2 */

    for (; y >= yFinal; --y) {
        if (y < 0) {
            return;
        } else if (y < YRES) {
            int iFX1 = max(min((XRES - 1), fixToInt(fX1)), 0);
            int iFX0 = max(min((XRES - 1), fixToInt(fX0)), 0);
            uint8_t *destination = &framebuffer[(XRES_FRAMEBUFFER * y) + min(iFX0, iFX1)];
            memFill(destination, colour, abs(iFX1 - iFX0));
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


void drawTexturedBottomFlatTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture, int z) {
    int y = coords[1];
    int u, v;
    FixP_t fU1, fU2, fV1, fV2;
    FixP_t fDU1;
    FixP_t fDU2;
    FixP_t fDV1;
    FixP_t fDV2;
    int yFinal = coords[5]; /* not the lowest, neither the topmost */
    int stipple;
    int farEnoughForStipple = (z >= distanceForPenumbra);

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
    effectiveDelta = Div(intToFix(1), intToFix((coords[5]) - y));
    fDU1 = Mul((u2 - u0), effectiveDelta);
    fDV1 = Mul((v2 - v0), effectiveDelta);

    effectiveDelta = Div(intToFix(1), intToFix((coords[3]) - y));
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

        if (y >= YRES) {
            return;
        }

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
            if (limit > 0 && limit < 320) {
                oneOverLimit = divLut[limit];
            } else if (limit < 0 && limit > -320) {
                oneOverLimit = -divLut[-limit];
            } else {
                oneOverLimit = Div(intToFix(1), (fX1 - fX0));
            }


            destination = &framebuffer[(XRES_FRAMEBUFFER * y) + iFX0];

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
                stipple = ((xPos + y) & 1) ? 0xFFFFFFFF : 0;
                while (limit--) {
                    stipple = ~stipple;
                    u = abs(fixToInt(texelLineX)) % NATIVE_TEXTURE_SIZE;
                    v = abs(fixToInt(texelLineY)) % NATIVE_TEXTURE_SIZE;

                    if (xPos >= 0 && xPos <= XRES) {
                        if (stipple && farEnoughForStipple) {
                            *destination = 0;
                        } else {
                            *destination = *(&texture->rotations[1][0] + (NATIVE_TEXTURE_SIZE * v) + u);
                        }
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


void drawTexturedTopFlatTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture, int z) {
    int y = coords[1];
    int u, v;
    FixP_t fU1, fU2, fV1, fV2;
    FixP_t fDU1;
    FixP_t fDV1;
    FixP_t fDU2;
    FixP_t fDV2;
    int stipple;
    int yFinal = coords[3]; /* not the upper, not the lowest */
    int farEnoughForStipple = (z >= distanceForPenumbra);

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
    fX0 = x0; /* p1 */
    fX1 = x0; /* p2 */

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
    effectiveDelta = Div(intToFix(1), intToFix(y - (coords[3])));
    fDU1 = Mul((u1 - u0), effectiveDelta);
    fDV1 = Mul((v1 - v0), effectiveDelta);

    effectiveDelta = Div(intToFix(1), intToFix(y - (coords[5])));
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

        if (y <= 0) {
            return;
        }

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
            if (limit > 0 && limit < 320) {
                oneOverLimit = divLut[limit];
            } else if (limit < 0 && limit > -320) {
                oneOverLimit = -divLut[-limit];
            } else {
                oneOverLimit = Div(intToFix(1), (fX1 - fX0));
            }


            destination = &framebuffer[(XRES_FRAMEBUFFER * y) + iFX0];

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

                stipple = ((xPos + y) & 1) ? 0xFFFFFFFF : 0;
                while (limit--) {
                    stipple = ~stipple;
                    u = abs(fixToInt(texelLineX)) % NATIVE_TEXTURE_SIZE;
                    v = abs(fixToInt(texelLineY)) % NATIVE_TEXTURE_SIZE;

                    if (xPos >= 0 && xPos <= XRES) {
                        if (stipple && farEnoughForStipple) {
                            *destination = 0;
                        } else {
                            *destination = *(&texture->rotations[1][0] + (NATIVE_TEXTURE_SIZE * v) + u);
                        }
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

void drawTexturedTriangle(int *coords, uint8_t *uvCoords, struct Texture *texture, int z) {
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


    drawTexturedBottomFlatTriangle(&newCoors[0], &newUV[0], texture, z);

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


    drawTexturedTopFlatTriangle(&newCoors[0], &newUV[0], texture, z);
}

#ifdef AGS
__attribute__((target("arm"), section(".iwram"), noinline))
#endif

void fillRect(
        const int x, const int y,
        const size_t dx, const size_t dy,
        const FramebufferPixelFormat pixel, const uint8_t stipple) {

    uint8_t *destination = &framebuffer[0];
    unsigned int py;
    uint8_t *destinationLineStart;

    if (pixel == TRANSPARENCY_COLOR) {
        return;
    }

    destinationLineStart = destination + (XRES_FRAMEBUFFER * y) + x;

    if (!stipple) {
        for (py = 0; py < dy; ++py) {
            memFill(destinationLineStart, pixel, dx);
            destinationLineStart += XRES_FRAMEBUFFER;
        }
    } else {
        for (py = 0; py < dy; ++py) {
            unsigned int px;
            for (px = 0; px < dx; ++px) {
                destinationLineStart++;
                if ((px + py) & 1) {
                    *destinationLineStart = pixel;
                }
            }
            destinationLineStart += XRES_FRAMEBUFFER - dx;
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

    if ((dy + height) >= YRES_FRAMEBUFFER) {
        height = (YRES_FRAMEBUFFER - dy);
    }
    for (y = 0; y < height; ++y) {
        uint8_t *destinationLineStart = destination + (XRES_FRAMEBUFFER * (dy + y)) + dx;
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

void drawBitmap(const int x,
                const int y,
                struct Bitmap *tile,
                const uint8_t transparent) {

    drawBitmapRaw(x, y, tile->width, tile->height, tile->data, transparent);
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text,
                                       const uint8_t colour, char charToReplaceHifenWith) {

    size_t len = strlen(text);
    int32_t dstX = (x - 1) * 8;
    int32_t dstY = (y - 1) * 8;
    uint8_t *dstBuffer = &framebuffer[0];
    size_t fontWidth = defaultFont->width;
    uint8_t *fontPixelData = defaultFont->data;
    size_t c;
    int32_t srcX, srcY;

    for (c = 0; c < len; ++c) {
        uint8_t ascii;
        uint8_t line;
        uint8_t col;
        uint8_t *letter;

        char currentChar = text[c];

        if (currentChar == '-') {
            currentChar = charToReplaceHifenWith;
        }


        ascii = text[c] - ' ';
        line = ascii >> 5;
        col = ascii & 31;
        letter = fontPixelData + (col * 8) + (fontWidth * (line * 8));


        if (currentChar == '\n' || dstX >= margin) {
            dstX = (x - 1) * 8;
            dstY += 8;
            continue;
        }

        if (dstY >= YRES_FRAMEBUFFER) {
            return;
        }

        if (currentChar == ' ') {
            dstX += 8;
            continue;
        }

        for (srcY = 0; srcY < 8; ++srcY) {

            uint8_t *letterSrc = letter + (fontWidth * srcY);
            uint8_t *letterDst = dstBuffer + dstX + (XRES_FRAMEBUFFER * (dstY + srcY));

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

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const uint8_t colour) {
    drawTextAtWithMarginWithFiltering(x, y, margin, text, colour, '-');
}

void drawTextAt(const int x, const int y, const char *text, const uint8_t colour) {

    drawTextAtWithMargin(x, y, (XRES_FRAMEBUFFER - 1), text, colour);
}

void renderPageFlip(uint8_t *stretchedBuffer, uint8_t *currentFrame,
                    uint8_t *prevFrame, int turnState, int turnTarget, int scale200To240) {

    uint8_t index;
    uint8_t *src;
    uint8_t *dst;
    int x, y, chunky;

    if (abs(turnTarget - turnStep) < PAGE_FLIP_INCREMENT) {
        turnStep = turnTarget;
    }

#ifdef SCALE_200_TO_240
    int dstY = 0;
    int scaller = 0;
    int heightY;

    if (turnTarget == turnState || (mTurnBuffer != kCommandNone) ) {

        for (y = 0; y < YRES_FRAMEBUFFER; ++y) {

            if (scaller == 4) {
                heightY = 2;
            } else {
                heightY = 1;
            }

            for (chunky = 0; chunky < heightY; ++chunky) {

                dst = stretchedBuffer;
                src = &currentFrame[(XRES_FRAMEBUFFER * y)];
                dst += (XRES_FRAMEBUFFER * (dstY + chunky));

                for (x = 0; x < XRES_FRAMEBUFFER; ++x) {
                    index = *src;
                    *dst = index;
                    ++src;
                    ++dst;
                }
            }

            dstY++;
            scaller++;

            if (scaller == 5) {
                scaller = 0;
                dstY++;
            }
        }

        if (mTurnBuffer != kCommandNone) {
            mBufferedCommand = mTurnBuffer;
        }

        mTurnBuffer = kCommandNone;

        memCopyToFrom(prevFrame, currentFrame, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * sizeof(uint8_t));

    } else if (turnState < turnTarget) {

        for (y = 0; y < YRES_FRAMEBUFFER; ++y) {

            if (scaller == 4) {
                heightY = 2;
            } else {
                heightY = 1;
            }

            for (chunky = 0; chunky < heightY; ++chunky) {

                dst = stretchedBuffer;
                dst += (XRES_FRAMEBUFFER * (dstY + chunky));

                for (x = 0; x < XRES_FRAMEBUFFER; ++x) {
                    if (x < XRES && y >= 8) {
                        if (x >= turnStep ) {
                            index = prevFrame[(XRES_FRAMEBUFFER * y) + x - turnStep];
                        } else {
                            index = currentFrame[(XRES_FRAMEBUFFER * y) + x - ( XRES_FRAMEBUFFER - XRES)- turnStep];
                        }

                    } else {
                        index = currentFrame[(XRES_FRAMEBUFFER * y) + x];
                    }

                    *dst = index;
                    ++dst;
                }
            }

            dstY++;
            scaller++;

            if (scaller == 5) {
                scaller = 0;
                dstY++;
            }
        }

        turnStep += PAGE_FLIP_INCREMENT;
    } else {

        for (y = 0; y < YRES_FRAMEBUFFER; ++y) {

            if (scaller == 4) {
                heightY = 2;
            } else {
                heightY = 1;
            }

            for (chunky = 0; chunky < heightY; ++chunky) {

                dst = stretchedBuffer;
                dst += (XRES_FRAMEBUFFER * (dstY + chunky));

                for (x = 0; x < XRES_FRAMEBUFFER; ++x) {

                    if (x < XRES && y >= 8) {

                        if (x >= turnStep) {
                            index = currentFrame[(XRES_FRAMEBUFFER * y) + x - turnStep];
                        } else {
                            index = prevFrame[(XRES_FRAMEBUFFER * y) + x - (XRES_FRAMEBUFFER - XRES) - turnStep];
                        }

                    } else {
                        index = currentFrame[(XRES_FRAMEBUFFER * y) + x];
                    }

                    *dst = index;
                    ++dst;
                }
            }

            dstY++;
            scaller++;

            if (scaller == 5) {
                scaller = 0;
                dstY++;
            }
        }
        turnStep -= PAGE_FLIP_INCREMENT;
    }
#else
    if (turnTarget == turnStep || (mTurnBuffer != kCommandNone)) {
        if (mTurnBuffer != kCommandNone) {
            mBufferedCommand = mTurnBuffer;
        }

        mTurnBuffer = kCommandNone;

        memCopyToFrom(stretchedBuffer, currentFrame, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * sizeof(uint8_t));
        memCopyToFrom(prevFrame, currentFrame, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * sizeof(uint8_t));

    } else if (turnState < turnTarget) {
        for (y = 0; y < YRES_FRAMEBUFFER; ++y) {
            size_t lineOffset = (y * XRES_FRAMEBUFFER);
            memCopyToFrom(stretchedBuffer + lineOffset,
                          currentFrame + lineOffset + (XRES_FRAMEBUFFER - turnStep - HUD_WIDTH), turnStep);
            memCopyToFrom(stretchedBuffer + lineOffset + turnStep, prevFrame + lineOffset, (XRES - turnStep));
            memCopyToFrom(stretchedBuffer + lineOffset + XRES, currentFrame + lineOffset + XRES,
                          (XRES_FRAMEBUFFER - XRES));
        }

        turnStep += PAGE_FLIP_INCREMENT;
    } else {
        for (y = 0; y < YRES_FRAMEBUFFER; ++y) {
            size_t lineOffset = (y * XRES_FRAMEBUFFER);
            memCopyToFrom(stretchedBuffer + lineOffset,
                          prevFrame + lineOffset + (XRES_FRAMEBUFFER - turnStep - HUD_WIDTH), turnStep);
            memCopyToFrom(stretchedBuffer + lineOffset + turnStep, currentFrame + lineOffset, (XRES - turnStep));
            memCopyToFrom(stretchedBuffer + lineOffset + XRES, currentFrame + lineOffset + XRES,
                          (XRES_FRAMEBUFFER - XRES));
        }
        turnStep -= PAGE_FLIP_INCREMENT;
    }
#endif
}
