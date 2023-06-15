#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

extern qword_t *_q;

void bindTexture(struct Bitmap *bitmap) {
	qword_t *q = _q;

	// Using a texture involves setting up a lot of information.
	clutbuffer_t clut;

	lod_t lod;

	lod.calculation = LOD_USE_K;
	lod.max_level = 0;
	lod.mag_filter = LOD_MAG_LINEAR;
	lod.min_filter = LOD_MIN_LINEAR;
	lod.l = 0;
	lod.k = 0;

	bitmap->nativeBuffer->info.width = draw_log2(bitmap->width);
	bitmap->nativeBuffer->info.height = draw_log2(bitmap->height);
	bitmap->nativeBuffer->info.components = TEXTURE_COMPONENTS_RGBA;
	bitmap->nativeBuffer->info.function = TEXTURE_FUNCTION_MODULATE;

	clut.storage_mode = CLUT_STORAGE_MODE1;
	clut.start = 0;
	clut.psm = 0;
	clut.load_method = CLUT_NO_LOAD;
	clut.address = 0;

	q = draw_texture_sampling(q,0,&lod);
	q = draw_texturebuffer(q,0,bitmap->nativeBuffer,&clut);
	_q = q;
}



int submitBitmapToGPU(struct Bitmap *bitmap) {


	bitmap->nativeBuffer = (texbuffer_t*)calloc(1, sizeof(texbuffer_t));
	bitmap->nativeBuffer->width = bitmap->width;
	bitmap->nativeBuffer->psm = GS_PSM_32;
	bitmap->nativeBuffer->address = graph_vram_allocate(bitmap->width, bitmap->height, GS_PSM_32,GRAPH_ALIGN_BLOCK);

	packet_t *packet = packet_init(50,PACKET_NORMAL);

	qword_t *q;

	q = packet->data;

	q = draw_texture_transfer(q, bitmap->data, bitmap->width, bitmap->height,GS_PSM_32,bitmap->nativeBuffer->address,bitmap->nativeBuffer->width);
	q = draw_texture_flush(q);
    FlushCache(0);
	dma_channel_send_chain(DMA_CHANNEL_GIF,packet->data, q - packet->data, 0,0);
	dma_wait_fast();

	packet_free(packet);

    disposeMem(bitmap->data);
	bitmap->data = NULL;
	bitmap->uploadId = 1;

	return 0;
}

struct Bitmap *loadBitmap(const char *filename) {
	size_t c;
	uint8_t d;
    struct StaticBuffer src = loadBinaryFileFromPath(filename);

	struct Bitmap *toReturn =
			(struct Bitmap *) calloc(1, sizeof(struct Bitmap));

    size_t sizeInDisk  = src.size - 4; //total size minus the header

    uint16_t tmp;
    const uint8_t *ptr = src.data;

    tmp = *ptr++;
    toReturn->width = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->width += tmp & 0xFF;

    tmp = *ptr++;
    toReturn->height = (tmp & 0xFF) << 8;
    tmp = *ptr++;
    toReturn->height += tmp & 0xFF;

    size_t size = toReturn->width * toReturn->height * sizeof(BitmapPixelFormat);

    uint8_t *buffer = (uint8_t *) calloc(1, sizeInDisk);

    memCopyToFrom( buffer, ptr, sizeInDisk);

    toReturn->data = (TexturePixelFormat *) calloc(1, size);

    uint8_t repetitions;
    int pixelIndex = 0;
    BitmapPixelFormat pixel;

    for (c = 0; c < sizeInDisk; c += 5) {
        pixel = 0;

        if (buffer[c + 3] < 255) {
            pixel = TRANSPARENCY_COLOR;
        } else {
            pixel += (0x80) << 24;
            pixel += buffer[c + 2] << 16;
            pixel += buffer[c + 1] << 8;
            pixel += buffer[c + 0] << 0;
        }
        repetitions = buffer[c + 4];

        for (d = 0; d < repetitions; ++d) {
            toReturn->data[pixelIndex++] = pixel;
        }
    }

    disposeMem(buffer);
    disposeDiskBuffer(src);

    toReturn->uploadId = -1;

    return toReturn;
}

void releaseBitmap(struct Bitmap *ptr) {
	assert(ptr != NULL);
	graph_vram_free(ptr->nativeBuffer->address);
    disposeMem(ptr->nativeBuffer);
    disposeMem(ptr->data);
    disposeMem(ptr);
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

