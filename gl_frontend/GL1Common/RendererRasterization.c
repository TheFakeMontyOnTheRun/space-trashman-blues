#include <stdlib.h>
#include <string.h>

#ifndef NDS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl.h>

#else
#include <GL/gl.h>
#endif
#else
#include <nds.h>
#include <malloc.h>
#include <stdio.h>
#include <nds/arm9/image.h>
#include <nds/arm9/trig_lut.h>
#endif

#ifdef N64
#include <libdragon.h>
#include <rdpq_text.h>
#endif

#include "Enums.h"
#include "Core.h"
#include "FixP.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "MapWithCharKey.h"
#include "Dungeon.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "LoadBitmap.h"
#include "Renderer.h"

#include "Engine.h"
#include "VisibilityStrategy.h"

uint8_t shouldDrawLights = TRUE;
int useDither = TRUE;

#ifndef N64
struct Bitmap *defaultFont;
#else
extern rdpq_font_t *fnt1;
#endif

#define NORMALIZE_ORTHO_X (1.0f / 100.0f)
#define NORMALIZE_ORTHO_Y (1.2f / 100.0f)
#define NORMALIZE_COLOUR (1.0f / 256.0f)

int submitBitmapToGPU(struct Bitmap *bitmap);

void drawRect(
        const int _x,
        const int _y,
        const size_t _dx,
        const size_t _dy,
        const FramebufferPixelFormat pixel) {

    float x = _x * NORMALIZE_ORTHO_X;
    float y = _y * NORMALIZE_ORTHO_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y;

    uint32_t fragment = pixel;//palette[pixel];

    if (fragment != TRANSPARENCY_COLOR) {
        float r, g, b;

        r = (fragment & 0xFF) * NORMALIZE_COLOUR;
        g = ((fragment & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
        b = ((fragment & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

        glColor3f(r,
                  g,
                  b);

#ifndef NDS
        glBegin(GL_LINE_LOOP);
        glVertex3f(x, y, -0.13);
        glVertex3f(x + dx, y, -0.13);
        glVertex3f(x + dx, y + dy, -0.13);
        glVertex3f(x, y + dy, -0.13);
        glEnd();
#else
        glBegin(GL_QUADS);

        glVertex3f(x, y, -0.13);
        glVertex3f(x + dx, y, -0.13);
        glVertex3f(x + dx, y, -0.13);
        glVertex3f(x, y, -0.13);

        glVertex3f(x + dx, y, -0.13);
        glVertex3f(x + dx, y + dy, -0.13);
        glVertex3f(x + dx, y + dy, -0.13);
        glVertex3f(x + dx, y, -0.13);

        glVertex3f(x, y + dy, -0.13);
        glVertex3f(x + dx, y + dy, -0.13);
        glVertex3f(x + dx, y + dy, -0.13);
        glVertex3f(x, y + dy, -0.13);

        glVertex3f(x, y, -0.13);
        glVertex3f(x, y + dy, -0.13);
        glVertex3f(x, y + dy, -0.13);
        glVertex3f(x, y, -0.13);

        glEnd();

#endif

        glColor3f(1, 1, 1);
    }
}

void fillTriangle(int *coords, FramebufferPixelFormat fragment) {
    float r, g, b;
    float x;
    float y;

    r = (fragment & 0xFF) * NORMALIZE_COLOUR;
    g = ((fragment & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    b = ((fragment & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

    glBegin(GL_TRIANGLES);

    glColor3f(r,
              g,
              b);

    x = coords[0] * NORMALIZE_ORTHO_X;
    y = coords[1] * NORMALIZE_ORTHO_Y;

    glVertex3f(x, y, -0.13);

    glColor3f(r,
              g,
              b);

    x = coords[2] * NORMALIZE_ORTHO_X;
    y = coords[3] * NORMALIZE_ORTHO_Y;

    glVertex3f(x, y, -0.13);

    glColor3f(r,
              g,
              b);

    x = coords[4] * NORMALIZE_ORTHO_X;
    y = coords[5] * NORMALIZE_ORTHO_Y;

    glVertex3f(x, y, -0.13);
    glEnd();
}

void drawTexturedTriangle(int *coords, UVCoord *uvCoords, struct Texture *texture, int z) {
}

void fillRect(
        const int _x,
        const int _y,
        const size_t _dx,
        const size_t _dy,
        const FramebufferPixelFormat pixel,
        const uint8_t stipple) {

    uint32_t fragment = pixel;//palette[pixel];

    float x = _x * NORMALIZE_ORTHO_X;
    float y = _y * NORMALIZE_ORTHO_Y;
    float dx = _dx * NORMALIZE_ORTHO_X;
    float dy = _dy * NORMALIZE_ORTHO_Y;

    if (fragment != TRANSPARENCY_COLOR) {

        float r, g, b;

        r = (fragment & 0xFF) * NORMALIZE_COLOUR;
        g = ((fragment & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
        b = ((fragment & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

        glColor3f(r,
                  g,
                  b);

        if (stipple) {
#ifdef NDS
            /* Temporarily disable the shadows on NDS, since the Z-ordering is wrong in it */
    return;
#endif

#ifndef N64
            float fontWidth = defaultFont->width;
            float fontHeight = 32.0f;//defaultFont->height;
            float blockWidth = 8.0f / fontWidth;
            float blockHeight = 8.0f / fontHeight;
            size_t repeatX;
            size_t repeatY;
            size_t c, d;
            uint32_t ascii;
            float line;
            float col;

            if (defaultFont->uploadId == -1) {
                defaultFont->uploadId = submitBitmapToGPU(defaultFont);
            }

            glBindTexture(GL_TEXTURE_2D, defaultFont->uploadId);

#ifndef NDS
            glAlphaFunc(GL_GREATER, 0.5f);
#endif
            glEnable(GL_ALPHA_TEST);
            glBegin(GL_QUADS);

            repeatX = (_dx / 8);
            repeatY = (_dy / 8);

            ascii = 0;
            line = (((ascii >> 5) + 1) * blockHeight);
            col = (((ascii & 31)) * blockWidth);

            for (c = 0; c < repeatY; ++c) {
                for (d = 0; d < repeatX; ++d) {

                    size_t dstX = _x + ((d * 8));
                    size_t dstY = _y + ((c * 8));

                    glTexCoord2f(col, line - blockHeight);
                    glVertex3f(dstX * NORMALIZE_ORTHO_X, dstY * NORMALIZE_ORTHO_Y, -0.1);
                    glTexCoord2f(col + blockWidth, line - blockHeight);
                    glVertex3f((dstX + 8) * NORMALIZE_ORTHO_X, dstY * NORMALIZE_ORTHO_Y, -0.1);
                    glTexCoord2f(col + blockWidth, line);
                    glVertex3f((dstX + 8) * NORMALIZE_ORTHO_X, (dstY + 8) * NORMALIZE_ORTHO_Y, -0.1);
                    glTexCoord2f(col, line);
                    glVertex3f(dstX * NORMALIZE_ORTHO_X, (dstY + 8) * NORMALIZE_ORTHO_Y, -0.1);
                }
            }

            glEnd();
            glDisable(GL_ALPHA_TEST);
#else
            glDisable(GL_TEXTURE_2D);
            glColor3f(0,
                      0,
                      0);

            glBegin(GL_QUADS);
            glVertex3f(x, y, -0.15);
            glVertex3f(x + dx, y, -0.15);
            glVertex3f(x + dx, y + dy, -0.15);
            glVertex3f(x, y + dy, -0.15);
            glEnd();
            glEnable(GL_TEXTURE_2D);
#endif
        } else {
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            glColor3f(r,
                      g,
                      b);

            glBegin(GL_QUADS);
            glVertex3f(x, y, -0.15);
            glVertex3f(x + dx, y, -0.15);
            glVertex3f(x + dx, y + dy, -0.15);
            glVertex3f(x, y + dy, -0.15);
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }
    }
    glColor3f(1, 1, 1);
}

void drawBitmap(const int _dx,
                const int _dy,
                struct Bitmap *bitmap,
                const uint8_t transparent) {

    float x;
    float y;
    float dx;
    float dy;

    if (bitmap->uploadId == -1) {
        bitmap->uploadId = submitBitmapToGPU(bitmap);
    }

    x = _dx * NORMALIZE_ORTHO_X;
    y = _dy * NORMALIZE_ORTHO_Y;
    dx = bitmap->width * NORMALIZE_ORTHO_X;
    dy = bitmap->height * NORMALIZE_ORTHO_Y;

    if (bitmap->uploadId != -1) {

        if (transparent) {
#ifndef NDS
            glAlphaFunc(GL_GREATER, 0.5f);
#endif
            glEnable(GL_ALPHA_TEST);
        }

        glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(x, y, -0.125);
        glTexCoord2f(1, 0);
        glVertex3f(x + dx, y, -0.125);
        glTexCoord2f(1, 1);
        glVertex3f(x + dx, y + dy, -0.125);
        glTexCoord2f(0, 1);
        glVertex3f(x, y + dy, -0.125);
        glEnd();

        if (transparent) {
            glDisable(GL_ALPHA_TEST);
        }
    }
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

void drawTextAt(const int x, const int y, const char *text, const FramebufferPixelFormat colour) {
    drawTextAtWithMargin(x, y, XRES_FRAMEBUFFER / 8,  text, colour);
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text,
                                       const uint8_t colour, char charToReplaceHifenWith) {

    size_t len = strlen(text);
    int32_t dstX = (x) * 8;
    int32_t dstY = (y) * 8;
    size_t c;
    uint32_t ascii;
    float line;
    float col;
    char shortStr[2];

#ifndef N64
    float r, g, b;

    float fontWidth = defaultFont->width;
    float fontHeight = 32.0f;//defaultFont->height;
    float blockWidth = 8.0f / fontWidth;
    float blockHeight = 8.0f / fontHeight;

    if (defaultFont->uploadId == -1) {
        defaultFont->uploadId = submitBitmapToGPU(defaultFont);
    }

    glBindTexture(GL_TEXTURE_2D, defaultFont->uploadId);

#ifndef NDS
    glAlphaFunc(GL_GREATER, 0.5f);
#endif
    glEnable(GL_ALPHA_TEST);
    glBegin(GL_QUADS);

    r = (colour & 0xFF) * NORMALIZE_COLOUR;
    g = ((colour & 0x00FF00) >> 8) * NORMALIZE_COLOUR;
    b = ((colour & 0xFF0000) >> 16) * NORMALIZE_COLOUR;

    glColor3f(r,
              g,
              b);

#else
    uint8_t r, g, b;
    
    shortStr[1] = 0;

    dstX = (x) * 8;
    dstY = (y + 2) * 9;

    r = (colour & 0xFF);
    g = ((colour & 0x00FF00) >> 8);
    b = ((colour & 0xFF0000) >> 16);

    rdpq_font_style(fnt1, 0, &(rdpq_fontstyle_t){
      .color = RGBA32(r, g, b, 0xFF),
    });
#endif
    for (c = 0; c < len; ++c) {
        char currentChar = text[c];

        if (currentChar == '-') {
            currentChar = charToReplaceHifenWith;
        }
        if (currentChar == '\n' || (dstX >= XRES_FRAMEBUFFER) || (dstX >= ((margin - 1) * 8)) ) {
            dstX = (x - 1) * 8;
            dstY += 8;
            continue;
        }

        if (currentChar == ' ' || currentChar == '\r') {
            dstX += 8;
            continue;
        }

#ifndef N64
        ascii = currentChar - ' ';
        line = (((ascii >> 5) + 1) * blockHeight);
        col = (((ascii & 31)) * blockWidth);

        glTexCoord2f(col, line - blockHeight);
        glVertex3f(dstX * NORMALIZE_ORTHO_X, dstY * NORMALIZE_ORTHO_Y, -0.1);
        glTexCoord2f(col + blockWidth, line - blockHeight);
        glVertex3f((dstX + 8) * NORMALIZE_ORTHO_X, dstY * NORMALIZE_ORTHO_Y, -0.1);
        glTexCoord2f(col + blockWidth, line);
        glVertex3f((dstX + 8) * NORMALIZE_ORTHO_X, (dstY + 8) * NORMALIZE_ORTHO_Y, -0.1);
        glTexCoord2f(col, line);
        glVertex3f(dstX * NORMALIZE_ORTHO_X, (dstY + 8) * NORMALIZE_ORTHO_Y, -0.1);
#else
        shortStr[0] = currentChar;
        rdpq_text_print(NULL, 1, dstX, dstY, &shortStr[0]);
#endif


        dstX += 8;
    }
#ifndef N64
    glEnd();
#else
#endif

    glColor3f(1, 1, 1);
    glDisable(GL_ALPHA_TEST);

}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {

}

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const FramebufferPixelFormat colour) {
    drawTextAtWithMarginWithFiltering(x, y, margin, text, colour, '-');
}

void renderPageFlip(OutputPixelFormat *stretchedBuffer, FramebufferPixelFormat *currentFrame,
                    FramebufferPixelFormat *prevFrame, int turnState, int turnTarget, uint8_t scale200To240) {
}
