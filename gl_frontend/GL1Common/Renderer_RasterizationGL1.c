#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "Core.h"
#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "MapWithCharKey.h"
#include "Dungeon.h"

#include "CTile3DProperties.h"
#include "CRenderer.h"

#include "Engine.h"
#include "VisibilityStrategy.h"

uint8_t shouldDrawLights = TRUE;
int useDither = TRUE;
struct Bitmap *defaultFont;

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
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const FramebufferPixelFormat pixel) {
    
    uint32_t fragment = pixel;//palette[pixel];
    
    if (fragment != TRANSPARENCY_COLOR) {
        float r, g, b;
        
        r = (fragment & 0xFF) / 256.0f;
        g = ((fragment & 0x00FF00) >> 8) / 256.0f;
        b = ((fragment & 0xFF0000) >> 16)  / 256.0f;
        
        glColor3f(r,
                  g,
                  b);
       
        glBegin(GL_LINE_LOOP);
        glVertex3f(x, y, -2);
        glVertex3f(x + dx, y, -2);
        glVertex3f(x + dx, y + dy, -2);
        glVertex3f(x, y + dy, -2);
        glEnd();
        
        glColor3f(1, 1, 1);
    }
}

void fillTriangle(int *coords, FramebufferPixelFormat colour) {
}

void drawTexturedTriangle(int *coords, UVCoord *uvCoords, struct Texture *texture) {
}

void fill(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const FramebufferPixelFormat pixel,
		const uint8_t stipple) {
    
    uint32_t fragment = pixel;//palette[pixel];

    if (fragment != TRANSPARENCY_COLOR) {
        
        float r, g, b;
        
        r = (fragment & 0xFF) / 256.0f;
        g = ((fragment & 0x00FF00) >> 8) / 256.0f;
        b = ((fragment & 0xFF0000) >> 16)  / 256.0f;
        
        glColor3f(r,
                  g,
                  b);
        
        if (stipple) {
            float fontWidth = defaultFont->width;
            float fontHeight = defaultFont->height;
            float blockWidth = 8.0f / fontWidth;
            float blockHeight = 8.0f / fontHeight;
            
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

void drawBitmap(const int dx,
				const int dy,
                struct Bitmap *bitmap,
				const uint8_t transparent) {
    
    if (bitmap->uploadId == -1) {
        bitmap->uploadId = submitBitmapToGPU(bitmap);
    }
    
    if (bitmap->uploadId != -1) {
        
        if (transparent) {
            glEnable(GL_ALPHA_TEST);
        }

        glBindTexture(GL_TEXTURE_2D, bitmap->uploadId);
       
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f( dx, dy, -2);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f( dx + bitmap->width, dy, -2);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(dx + bitmap->width, dy + bitmap->height, -2);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(dx, dy + bitmap->height, -2);
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
		const struct Bitmap *tile) {
    
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
	float fontWidth = defaultFont->width;
    float fontHeight = 24.0f;//defaultFont->height;
    float blockWidth = 8.0f / fontWidth;
    float blockHeight = 8.0f / fontHeight;

	if (defaultFont->uploadId == -1) {
		defaultFont->uploadId = submitBitmapToGPU(defaultFont);
	}


	size_t c;
    
    uint32_t fragment = colour;// palette[colour];

    float r, g, b;

    r = (fragment & 0xFF) / 256.0f;
    g = ((fragment & 0x00FF00) >> 8) / 256.0f;
    b = ((fragment & 0xFF0000) >> 16)  / 256.0f;

    glColor3f(r,
              g,
              b);
    
    glEnable(GL_ALPHA_TEST);
    glBindTexture(GL_TEXTURE_2D, defaultFont->uploadId);
    glBegin(GL_QUADS);

    
	for (c = 0; c < len; ++c) {
		uint32_t ascii = text[c] - ' ';
		float line = (((ascii >> 5) + 1) * blockHeight);
		float col = (((ascii & 31)) * blockWidth);

		if (text[c] == '\n' || dstX >= XRES_FRAMEBUFFER) {
			dstX = (x - 1) * 8;
			dstY += 8;
			continue;
		}

		if (text[c] == ' ' || text[c] == '\r') {
			dstX += 8;
			continue;
		}
        
        glTexCoord2f(col, line - blockHeight);
        glVertex3f( dstX, dstY, -2);
        glTexCoord2f(col + blockWidth, line - blockHeight);
        glVertex3f( dstX + 8, dstY, -2);
        glTexCoord2f(col + blockWidth, line);
        glVertex3f( dstX + 8, dstY + 8, -2);
        glTexCoord2f(col, line);
        glVertex3f(dstX, dstY + 8, -2);
        
		dstX += 8;
	}
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnd();
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
