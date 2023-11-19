#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Core.h"
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "MapWithCharKey.h"
#include "Dungeon.h"

#include "CTile3DProperties.h"
#include "LoadBitmap.h"
#include "CRenderer.h"

#include "Engine.h"
#include "VisibilityStrategy.h"

uint8_t shouldDrawLights = TRUE;
int useDither = TRUE;

struct Bitmap *defaultFont;


#define NORMALIZE_ORTHO_X (1.0f / 320.0f)
#define NORMALIZE_ORTHO_Y (1.0f / 200.0f)
#define ADJUST_RESOLUTION_Y (((200.0f/256.0f) * 200.0f) / 240.0f )
#define OFFSET_X (-0.5f)
#define OFFSET_Y (-0.375f)
#define NORMALIZE_COLOUR (2.0f / 256.0f)

#define GEOMETRY_SCALE_X 2.0f
#define GEOMETRY_SCALE_Y 1.0f
#define GEOMETRY_SCALE_Z 2.0f
#define GEOMETRY_TEXTURE_SCALE_X 1.0f
#define GEOMETRY_TEXTURE_SCALE_Y 1.0f

#define BIAS (intToFix(8))
#define REVERSE_BIAS (1.0f/8.0f)
#define FOG_MAX_DISTANCE 32.0f

int submitBitmapToGPU(struct Bitmap *bitmap);

void drawRect(
        const int x,
        const int y,
        const size_t dx,
        const size_t dy,
        const FramebufferPixelFormat pixel) {

    fillRect(x, y, 1, dy - 1, pixel, 0);

    fillRect(x, y, dx - 1, 1, pixel, 0);

    fillRect(x + dx - 1, y, 1, dy - 1, pixel, 0);

    fillRect(x, y + dy - 1, dx, 1, pixel, 0);
}

void fillRect(
        const int _x,
        const int _y,
        const size_t _dx,
        const size_t _dy,
        const FramebufferPixelFormat pixel,
        const uint8_t stipple) {


}

void drawBitmapRegion(const int _x,
                      const int _y,
                      const int _dx,
                      const int _dy,
                      BitmapPixelFormat tint,
                      struct Bitmap *bitmap,
                      const uint8_t transparent,
                      float u0, float u1, float v0, float v1) {

}

void drawBitmap(const int _x,
                const int _y,
                struct Bitmap *bitmap,
                const uint8_t transparent) {
    drawBitmapRegion(_x, _y, bitmap->width, bitmap->height, getPaletteEntry(0xFFFFFFFF), bitmap, transparent, 0.0f,
                     1.0f, 0.0f, 1.0f);
}

void drawRepeatBitmap(
        const int x,
        const int y,
        const size_t dx,
        const size_t dy,
        struct Bitmap *tile) {

    size_t repeatX = (dx / tile->width) + 1;
    size_t repeatY = (dy / tile->height) + 1;
    size_t c, d;
    for (c = 0; c < repeatY; ++c) {
        for (d = 0; d < repeatX; ++d) {

            size_t px = x + d * tile->width;
            size_t py = y + c * tile->height;

            if (px < XRES_FRAMEBUFFER && py < YRES_FRAMEBUFFER) {
                drawBitmap(px, py, tile, FALSE);
            }
        }
    }
}

void drawTextAt(const int _x, const int _y, const char *text, const FramebufferPixelFormat colour) {

    if (defaultFont == NULL) {
        defaultFont = loadBitmap("font.img");
    }

    if (defaultFont->uploadId == -1) {
        submitBitmapToGPU(defaultFont);
    }

    size_t len = strlen(text);
    int32_t dstX = (_x - 1) * 8;
    int32_t dstY = (_y - 1) * 8;
    size_t c;
    uint32_t ascii;
    float line;
    float col;

    float fontWidth = defaultFont->width;
    float fontHeight = defaultFont->height;
    float blockWidth = (8.0f / fontWidth) * 0.999f;
    float blockHeight = (8.0f / fontHeight) * 0.999f;

    for (c = 0; c < len; ++c) {
        if (text[c] == '\n' || dstX >= XRES_FRAMEBUFFER) {
            dstX = (_x - 1) * 8;
            dstY += 8;
            continue;
        }

        if (text[c] == ' ' || text[c] == '\r') {
            dstX += 8;
            continue;
        }

        ascii = text[c] - ' ';

        line = (((float) ((ascii >> 5))) * blockHeight);
        col = (((ascii & 31)) * blockWidth);

        drawBitmapRegion(dstX, dstY, 8, 8, colour, defaultFont, 1, col, col + blockWidth, line, line + blockHeight);

        dstX += 8;
    }
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *__restrict__ text,
                                       const uint8_t colour, char charToReplaceHifenWith) {
    drawTextAt(x, y, text, colour);
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const FramebufferPixelFormat colour) {
    drawTextAt(x, y, text, colour);
}