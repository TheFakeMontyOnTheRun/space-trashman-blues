#ifndef BITMAP_H
#define BITMAP_H

#include <kernel.h>
#include <malloc.h>
#include <tamtypes.h>
#include <math3d.h>
#include <packet.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>
#include <dma.h>
#include <graph.h>
#include <draw.h>
#include <draw3d.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>

typedef uint32_t TexturePixelFormat;

typedef uint32_t BitmapPixelFormat;

struct Bitmap {
	BitmapPixelFormat *data;
	uint16_t width;
	uint16_t height;
    int uploadId;
	texbuffer_t *nativeBuffer;
};

struct Texture {
	struct Bitmap *raw;
};

void clearTextures(void);

struct Texture *makeTextureFrom(const char *  filename);

struct Bitmap *loadBitmap(const char *  filename);


void bindTexture(struct Bitmap *ptr);

void releaseBitmap(struct Bitmap *ptr);

FixP_t lerpFix(const FixP_t v0, const FixP_t v1, const FixP_t t, const FixP_t total);

int lerpInt(const int v0, const int v1, const long t, const long total);

#endif
