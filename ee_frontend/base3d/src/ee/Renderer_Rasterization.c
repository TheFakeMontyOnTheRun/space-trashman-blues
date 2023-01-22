#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
}

void drawBitmap(const int dx,
				const int dy,
                struct Bitmap *bitmap,
				const uint8_t transparent) {
}

void drawRepeatBitmap(
		const int x,
		const int y,
		const size_t dx,
		const size_t dy,
		const struct Bitmap *tile) {
}

void drawTextAt(const int x, const int y, const char *text, const FramebufferPixelFormat colour) {

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
