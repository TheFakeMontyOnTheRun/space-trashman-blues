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
#endif

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

#ifndef N64
struct Bitmap *defaultFont;
#else
extern rdpq_font_t *fnt1;
#endif

int submitBitmapToGPU(struct Bitmap* bitmap);

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
			  const TexturePixelFormat *texture,
			  const FixP_t textureScaleY,
			  const int z) {
}

void drawMask(
		const FixP_t x0,
		const FixP_t y0,
		const FixP_t x1,
		const FixP_t y1) {
}

/*
    *     x0y0 ____________ x1y0
    *         /            \
    *        /             \
    *  x0y1 /______________\ x1y1
    */
void maskFloor(
		FixP_t y0, FixP_t y1, FixP_t x0y0, FixP_t x1y0, FixP_t x0y1, FixP_t x1y1,         FramebufferPixelFormat pixel) {
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
			   const TexturePixelFormat *texture) {
}

void drawRect(
		const int _x,
		const int _y,
		const size_t _dx,
		const size_t _dy,
		const FramebufferPixelFormat pixel) {

	float x = _x / 100.0f;
	float y = _y / 100.0f;
	float dx = _dx / 100.0f;
	float dy = _dy / 100.0f;
    
    uint32_t fragment = pixel;//palette[pixel];
    
    if (fragment != TRANSPARENCY_COLOR) {
        float r, g, b;
        
        r = (fragment & 0xFF) / 256.0f;
        g = ((fragment & 0x00FF00) >> 8) / 256.0f;
        b = ((fragment & 0xFF0000) >> 16)  / 256.0f;
        
        glColor3f(r,
                  g,
                  b);

#ifndef NDS
        glBegin(GL_LINE_LOOP);
        glVertex3f(x, y, -2);
        glVertex3f(x + dx, y, -2);
        glVertex3f(x + dx, y + dy, -2);
        glVertex3f(x, y + dy, -2);
        glEnd();
#else
		glBegin(GL_QUADS);

        glVertex3f(x, y, -2);
		glVertex3f(x + dx, y, -2);
        glVertex3f(x + dx, y, -2);
		glVertex3f(x, y, -2);

        glVertex3f(x + dx, y, -2);
		glVertex3f(x + dx, y + dy, -2);
		glVertex3f(x + dx, y + dy, -2);
		glVertex3f(x + dx, y, -2);

        glVertex3f(x, y + dy, -2);
		glVertex3f(x + dx, y + dy, -2);
        glVertex3f(x + dx, y + dy, -2);
		glVertex3f(x, y + dy, -2);

        glVertex3f(x, y, -2);
		glVertex3f(x, y + dy, -2);
		glVertex3f(x, y + dy, -2);
		glVertex3f(x, y, -2);

        glEnd();

#endif
        
        glColor3f(1, 1, 1);
    }
}

void fillTriangle(int *coords, FramebufferPixelFormat colour) {
}

void drawTexturedTriangle(int *coords, UVCoord *uvCoords, struct Texture *texture) {
}

void fill(
		const int _x,
		const int _y,
		const size_t _dx,
		const size_t _dy,
		const FramebufferPixelFormat pixel,
		const uint8_t stipple) {
    
    uint32_t fragment = pixel;//palette[pixel];

	float x = _x / 100.0f;
	float y = _y / 100.0f;
	float dx = _dx / 100.0f;
	float dy = _dy / 100.0f;

    if (fragment != TRANSPARENCY_COLOR) {
        
        float r, g, b;
        
        r = (fragment & 0xFF) / 256.0f;
        g = ((fragment & 0x00FF00) >> 8) / 256.0f;
        b = ((fragment & 0xFF0000) >> 16)  / 256.0f;
        
        glColor3f(r,
                  g,
                  b);
        
        if (stipple) {
#ifndef N64
            float fontWidth = defaultFont->width;
            float fontHeight = defaultFont->height;
            float blockWidth = 8.0f / fontWidth;
            float blockHeight = 8.0f / fontHeight;

			if (defaultFont->uploadId == -1) {
				defaultFont->uploadId = submitBitmapToGPU(defaultFont);
			}

            glEnable(GL_ALPHA_TEST);

            size_t repeatX = (dx / 4);
            size_t repeatY = (dy / 4);

            size_t c, d;
            
            glBindTexture(GL_TEXTURE_2D, defaultFont->uploadId);
            glBegin(GL_QUADS);
            
            uint32_t ascii = 0;
            float line = (((ascii >> 5) + 1) * blockHeight);
            float col = (((ascii & 31)) * blockWidth);

            for (c = 0; c < repeatY; ++c) {
                for (d = 0; d < repeatX; ++d) {

                    size_t dstX = x + d * 4;
                    size_t dstY = y + c * 4;

                    glTexCoord2f(col, line - blockHeight);
                    glVertex3f( dstX, dstY, -2);
                    glTexCoord2f(col + blockWidth, line - blockHeight);
                    glVertex3f( dstX + 4, dstY, -2);
                    glTexCoord2f(col + blockWidth, line);
                    glVertex3f( dstX + 4, dstY + 4, -2);
                    glTexCoord2f(col, line);
                    glVertex3f(dstX, dstY + 4, -2);
                }
            }
            
            glEnd();
            glDisable(GL_ALPHA_TEST);
			glBindTexture(GL_TEXTURE_2D, 0);
#else
			glDisable(GL_TEXTURE_2D);
            glColor3f(0,
                      0,
                      0);

            glBegin(GL_QUADS);
            glVertex3f(x, y, -2);
            glVertex3f(x + dx, y, -2);
            glVertex3f(x + dx, y + dy, -2);
            glVertex3f(x, y + dy, -2);
            glEnd();
            glEnable(GL_TEXTURE_2D);
#endif
        } else {
            glDisable(GL_TEXTURE_2D);
            glColor3f(r,
                      g,
                      b);
            
            glBegin(GL_QUADS);
            glVertex3f(x, y, -2);
            glVertex3f(x + dx, y, -2);
            glVertex3f(x + dx, y + dy, -2);
            glVertex3f(x, y + dy, -2);
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
    
    if (bitmap->uploadId == -1) {
        bitmap->uploadId = submitBitmapToGPU(bitmap);
    }

	float x = _dx / 100.0f;
	float y = _dy / 100.0f;
	float dx = bitmap->width / 100.0f;
	float dy = bitmap->height / 100.0f;
    
    if (bitmap->uploadId != -1) {
        
        if (transparent) {
            glEnable(GL_ALPHA_TEST);
        }

        glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);
       
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( x, y, -2);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( x + dx, y, -2);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(x + dx, y + dy, -2);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(x, y + dy, -2);
        glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
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

	size_t len = strlen(text);
	int32_t dstX = (x - 1) * 8;
	int32_t dstY = (y - 1) * 8;
	size_t c;

	uint32_t fragment = colour;// palette[colour];

#ifndef N64
	float fontWidth = defaultFont->width;
	float fontHeight = 32.0f;//defaultFont->height;
	float blockWidth = 8.0f / fontWidth;
	float blockHeight = 8.0f / fontHeight;

	if (defaultFont->uploadId == -1) {
		defaultFont->uploadId = submitBitmapToGPU(defaultFont);
	}

	glBindTexture(GL_TEXTURE_2D, defaultFont->uploadId);

	glEnable(GL_ALPHA_TEST);
	glBegin(GL_QUADS);

	float r, g, b;

	r = (fragment & 0xFF) / 256.0f;
	g = ((fragment & 0x00FF00) >> 8) / 256.0f;
	b = ((fragment & 0xFF0000) >> 16)  / 256.0f;

	glColor3f(r,
			  g,
			  b);

#else
	char shortStr[2];
	shortStr[1] = 0;

	dstX = (x - 1) * 8;
	dstY = (y + 1) * 9;

	uint8_t r, g, b;

		r = (colour & 0xFF);
		g = ((colour & 0x00FF00) >> 8);
		b = ((colour & 0xFF0000) >> 16);

	rdpq_mode_end();
	rdpq_font_begin(RGBA32(r, g, b, 0xFF));
#endif
	for (c = 0; c < len; ++c) {
		if (text[c] == '\n' || dstX >= XRES_FRAMEBUFFER) {
			dstX = (x - 1) * 8;
			dstY += 8;
			continue;
		}

		if (text[c] == ' ' || text[c] == '\r') {
			dstX += 8;
			continue;
		}

#ifndef N64
		uint32_t ascii = text[c] - ' ';
		float line = (((ascii >> 5) + 1) * blockHeight);
		float col = (((ascii & 31)) * blockWidth);

        glTexCoord2f(col, line - blockHeight);
        glVertex3f( dstX / 100.0f, dstY / 100.0f, -2);
        glTexCoord2f(col + blockWidth, line - blockHeight);
        glVertex3f( (dstX + 8) / 100.0f, dstY / 100.0f, -2);
        glTexCoord2f(col + blockWidth, line);
        glVertex3f( (dstX + 8) / 100.0f, (dstY + 8) / 100.0f, -2);
        glTexCoord2f(col, line);
        glVertex3f(dstX / 100.0f, (dstY + 8) / 100.0f, -2);
#else
		shortStr[0] = text[c];
		rdpq_font_position(dstX, dstY);
		rdpq_font_print(fnt1, &shortStr[0]);
#endif


		dstX += 8;
	}
#ifndef N64
	glEnd();
#else
	rdpq_font_end();
	rdpq_mode_begin();
#endif



    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *__restrict__ text, const uint8_t colour, char charToReplaceHifenWith) {
    drawTextAt( x, y, text, colour);
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char * text, const FramebufferPixelFormat colour) {
    drawTextAt( x, y, text, colour);
}

void renderPageFlip(OutputPixelFormat *stretchedBuffer, FramebufferPixelFormat *currentFrame,
					FramebufferPixelFormat *prevFrame, int turnState, int turnTarget, uint8_t scale200To240) {
}
