#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef N64
#include <libdragon.h>
#endif

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

					*(destinationLine) = pixel;
				}
				++destinationLine;
				u += du;
			}
		}
		v += dv;
	}
}

int submitBitmapToGPU(struct Bitmap *bitmap) {
	uint32_t newId;
	int c;
	int width, height;
    uint16_t *ptr;
    uint16_t *expanded;
	BitmapPixelFormat *dataSource = bitmap->data;
	width = bitmap->width;
	height = bitmap->height;

	if (!isPowerOf2(bitmap->width) || !isPowerOf2(bitmap->height)) {

		scaleBitmap(intToFix(0), intToFix(0), intToFix(TEXTURE_BUFFER_SIZE), intToFix(TEXTURE_BUFFER_SIZE), bitmap);

		width = height = TEXTURE_BUFFER_SIZE;
		dataSource = &textureBuffer[0];
	}

	glGenTextures(1, (uint32_t *) &newId);
	glBindTexture(GL_TEXTURE_2D, newId);

#ifndef N64
	expanded = (uint16_t *) malloc(width * height * sizeof(uint16_t));
#else
	expanded = (uint16_t*)malloc_uncached(width * height * sizeof(uint16_t));
#endif

	ptr = expanded;

	for (c = 0; c < width * height; ++c) {
		BitmapPixelFormat index = dataSource[c];

		uint32_t texel = index;//palette[index];
#ifndef NDS
		uint16_t finalFragment = 0;
#else
		uint16_t finalFragment = ARGB16(0, 0, 0, 0);
#endif

		if (index & 0xFF000000) {
			uint8_t r = ((((texel & 0x0000FF)) * 32) / 256);
			uint8_t g = ((((texel & 0x00FF00) >> 8) * 32) / 256);
			uint8_t b = ((((texel & 0xFF0000) >> 16) * 32) / 256);
#ifndef NDS
			finalFragment = 1 + (r << 11) + (g << 6) + (b << 1); // alpha;
#else
			finalFragment = ARGB16(1, r, g, b);
#endif
		}

		*ptr = finalFragment;
		++ptr;
	}

#ifndef N64
#ifndef NDS
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA2, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, expanded);
#else

	// at this point, we need to flush back the changed `expanded` array before submitting to the GPU, otherwise, it might
	// get incomplete or corrupted texture data - our processed texels still live in the data cache
	DC_FlushAll();

	if (width == 16) {
		// most in game objects
		glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_16 , TEXTURE_SIZE_16, 0, TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, expanded);
	} else if (width == 256) {
		// special case - the 256 x 32 font
		glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_256 , TEXTURE_SIZE_32, 0, TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, expanded);
	} else {
		// most 2D assets - UI stuff.
		glTexImage2D(0, 0, GL_RGBA, TEXTURE_SIZE_32 , TEXTURE_SIZE_32, 0, TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, expanded);
	}
#endif
	free(expanded);
	free(bitmap->data);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA2, width, height, 0,  GL_RGBA,   GL_UNSIGNED_SHORT_5_5_5_1_EXT, expanded);
	free_uncached( expanded );
	free_uncached(bitmap->data);
#endif
	bitmap->data = NULL;

#ifndef NDS
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
#endif


	return newId;
}

struct Bitmap *loadBitmap(const char *filename) {
	size_t c;
	uint8_t d;
    size_t size;
    uint8_t *buffer;
    uint8_t repetitions;
	int pixelIndex;
	BitmapPixelFormat pixel;
	struct StaticBuffer src = loadBinaryFileFromPath(filename);

	struct Bitmap *toReturn =
#ifndef N64
			(struct Bitmap *) calloc(1, sizeof(struct Bitmap));
#else
	(struct Bitmap *) malloc_uncached(sizeof(struct Bitmap));
	memset(toReturn, 0, sizeof(struct Bitmap));
#endif

	size_t sizeInDisk = src.size - 4; //total size minus the header

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

	size = toReturn->width * toReturn->height * sizeof(BitmapPixelFormat);
#ifndef N64
	buffer = (uint8_t *) calloc(1, sizeInDisk);
#else
	buffer = (uint8_t *) malloc_uncached(sizeInDisk);
	memset(buffer, 0, sizeInDisk);
#endif

	memcpy(buffer, ptr, sizeInDisk);

#ifndef N64
	toReturn->data = (TexturePixelFormat *) calloc(1, size);
#else
	toReturn->data = (TexturePixelFormat *) malloc_uncached(size);
	memset(toReturn->data, 0, size);
#endif

	pixelIndex = 0;

#ifdef PALETTE_COLOURS_FRAMEBUFFER
	for (size_t c = 0; c < sizeInDisk; c += 2) {
		pixel = buffer[c];
		repetitions = buffer[c + 1];

		for (uint8_t d = 0; d < repetitions; ++d) {
			toReturn->data[pixelIndex++] = pixel;
		}
	}
#else
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
#endif

#ifndef N64
	free(buffer);
	free(src.data);
#else
	free_uncached(buffer);
	free_uncached(src.data);
#endif

	toReturn->uploadId = -1;

	return toReturn;
}

void releaseBitmap(struct Bitmap *ptr) {
	assert(ptr != NULL);

#ifndef N64
	if (ptr->uploadId != -1) {
		glDeleteTextures(1, (unsigned int *) &ptr->uploadId);
	}
	free(ptr->data);
	free(ptr);
#else
	free_uncached(ptr->data);
	free_uncached(ptr);
#endif
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
