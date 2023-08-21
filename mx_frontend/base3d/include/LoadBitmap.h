#ifndef BITMAP_H
#define BITMAP_H

#define NATIVE_TEXTURE_SIZE 16

struct Bitmap {
    uint8_t *data;
    uint16_t width;
    uint16_t height;
};


struct Texture {
#ifndef AGS
    uint8_t rotations[4][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#else
    uint8_t rotations[1][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#endif
    uint8_t rowMajor[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
};

void clearTextures(void);

struct Texture *makeTextureFrom(const char *__restrict__ filename);

struct Bitmap *loadBitmap(const char *__restrict__ filename);

void releaseBitmap(struct Bitmap *ptr);

FixP_t lerpFix(const FixP_t v0, const FixP_t v1, const FixP_t t, const FixP_t total);

int lerpInt(const int v0, const int v1, const long t, const long total);

#endif
