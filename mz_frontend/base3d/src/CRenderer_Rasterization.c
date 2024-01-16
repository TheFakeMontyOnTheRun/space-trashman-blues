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


void drawMask(
        const FixP_t x0,
        const FixP_t y0,
        const FixP_t x1,
        const FixP_t y1,
        uint8_t pixel) {

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

    fillRect(_x0, _y0, _x1 - _x0, _y1 - _y0, pixel, FALSE);
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

    if (abs(turnTarget - turnStep) < PAGE_FLIP_INCREMENT) {
        turnStep = turnTarget;
    }

        if (mTurnBuffer != kCommandNone) {
            mBufferedCommand = mTurnBuffer;
        }

        mTurnBuffer = kCommandNone;

        memCopyToFrom(stretchedBuffer, currentFrame, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * sizeof(uint8_t));
        memCopyToFrom(prevFrame, currentFrame, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER * sizeof(uint8_t));

}
