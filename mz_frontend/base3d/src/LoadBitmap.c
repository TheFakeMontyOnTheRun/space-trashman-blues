#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Common.h"
#include "CPackedFileReader.h"
#include "CRenderer.h"
#include "Engine.h"
#include "FixP.h"

struct Texture textures[TOTAL_TEXTURES];
uint8_t usedTexture = 0;

void clearTextures() {
    usedTexture = 0;
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

#ifdef AGA5BPP


uint32_t expandToARGB(uint8_t origin) {
    uint32_t shade;

    if (origin == TRANSPARENCY_COLOR) {
        return 0;
    }

    shade = 0xFF000000;
    shade += (((origin >> 6) & 3) << 6) << 16; //G
    shade += (((origin >> 3) & 7) << 5) <<  8; //B
    shade += (((origin >> 0) & 7) << 5) <<  0; //?

    return shade;
}

#endif


struct Bitmap *loadBitmap(const char *__restrict__ filename) {
    int c, d = 0;
    uint8_t pixel;
    uint8_t repetitions;

    struct Bitmap *toReturn =
            (struct Bitmap *) calloc(1, sizeof(struct Bitmap));

    if (toReturn == NULL ) {
        puts("Not enough memory!");
        exit(0);
    }

    FILE *src = openBinaryFileFromPath(filename);
    size_t size;
    size_t sizeInDisk = sizeOfFile(filename) - 4;
    int pixelIndex = 0;
    uint8_t *buffer;
    uint8_t width = 0;
    uint8_t height = 0;

    toReturn->width = 0;
    assert (fread(&width, 1, 1, src));
    toReturn->width += (width & 0xFF) << 8;
    assert (fread(&width, 1, 1, src));
    toReturn->width += width & 0xFF;

    toReturn->height = 0;
    assert (fread(&height, 1, 1, src));
    toReturn->height += (height & 0xFF) << 8;
    assert (fread(&height, 1, 1, src));
    toReturn->height += height & 0xFF;

    size = toReturn->width * toReturn->height;
    buffer = (uint8_t *) calloc(1, sizeInDisk);

    assert (fread(&buffer[0], sizeInDisk, 1, src));

    toReturn->data = (uint8_t *) calloc(1, size);

    for (c = 0; c < sizeInDisk; c += 2) {
        pixel = buffer[c];
        repetitions = buffer[c + 1];

        for (d = 0; d < repetitions; ++d) {
#ifdef AGA5BPP
            toReturn->data[pixelIndex++] = getPaletteEntry(expandToARGB(pixel));
#else
            toReturn->data[pixelIndex++] = pixel;
#endif
        }
    }

    free(buffer);

    fclose(src);

    return toReturn;
}


struct Texture *makeTextureFrom(const char *__restrict__ filename) {
    struct Texture *toReturn;
    uint8_t *diskBuffer;
    uint8_t pixel;
    uint8_t repetitions;
    int c, d = 0;
    size_t sizeInDisk = sizeOfFile(filename) - 4;
    int pixelIndex = 0;
    uint8_t buffer[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
    uint8_t dummy;

    int x, y;
    FILE *src = openBinaryFileFromPath(filename);

    assert (fread(&dummy, 1, 1, src));
    assert (fread(&dummy, 1, 1, src));
    assert (fread(&dummy, 1, 1, src));
    assert (fread(&dummy, 1, 1, src));

#ifndef AGA5BPP
    diskBuffer = (uint8_t *) calloc(1, sizeInDisk);
    assert (fread(diskBuffer, sizeInDisk, 1, src));

    for ( c = 0; c < sizeInDisk; c += 2 ) {
        pixel = diskBuffer[ c ];
        repetitions = diskBuffer[ c + 1 ];

        for ( d = 0; d < repetitions; ++d ) {
            buffer[ pixelIndex++ ] = pixel;
        }
    }
    free(diskBuffer);
#else
    diskBuffer = (uint8_t *) calloc(1, sizeInDisk);
	assert (fread(diskBuffer, sizeInDisk, 1, src));

	for ( c = 0; c < sizeInDisk; c += 2 ) {
		pixel = diskBuffer[ c ];
		repetitions = diskBuffer[ c + 1 ];

		uint8_t r	  = ( pixel & 192 ) >> 6;
	  	uint8_t g	  = ( pixel & 56 ) >> 3;
	  	uint8_t b	  = ( pixel & 7 );

		for ( d = 0; d < repetitions; ++d ) {
		  if ( pixel == 199 ) {
				buffer[ pixelIndex++ ] = 199;
			  } else {
				buffer[ pixelIndex++ ] = getPaletteEntry(expandToARGB(pixel));
			  }
		}
	}
	free(diskBuffer);
#endif

    fclose(src);

    toReturn = &textures[usedTexture++];

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        uint8_t *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        uint8_t *dstLine = &toReturn->rotations[0][(y * NATIVE_TEXTURE_SIZE)];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine++;
        }
    }

    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        uint8_t *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE) + (NATIVE_TEXTURE_SIZE - 1)];
        uint8_t *dstLine = &toReturn->rotations[1][y];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine--;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    for (y = (NATIVE_TEXTURE_SIZE - 1); y >= 0; --y) {
        uint8_t *sourceLine = &buffer[(y * NATIVE_TEXTURE_SIZE)];
        uint8_t *dstLine = &toReturn->rotations[2][(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE) + (NATIVE_TEXTURE_SIZE - 1)];
        for (x = (NATIVE_TEXTURE_SIZE - 1); x >= 0; --x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine--;
        }
    }

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        uint8_t *sourceLine = &buffer[(((NATIVE_TEXTURE_SIZE - 1) - y) * NATIVE_TEXTURE_SIZE)];
        uint8_t *dstLine = &toReturn->rotations[3][y];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    for (y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
        uint8_t *sourceLine = &buffer[y * NATIVE_TEXTURE_SIZE];
        uint8_t *dstLine = &toReturn->rowMajor[y];
        for (x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
            *dstLine = *sourceLine;
            sourceLine++;
            dstLine += NATIVE_TEXTURE_SIZE;
        }
    }

    return toReturn;
}


void releaseBitmap(struct Bitmap *__restrict__ ptr) {
    assert (ptr != NULL);

    free(ptr->data);
    free(ptr);
}
