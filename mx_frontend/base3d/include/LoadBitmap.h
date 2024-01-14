#ifndef BITMAP_H
#define BITMAP_H

#define NATIVE_TEXTURE_SIZE 16

typedef uint8_t TexturePixelFormat;

typedef uint8_t BitmapPixelFormat;

struct Bitmap {
    uint8_t *data;
    uint16_t width;
    uint16_t height;
};


struct Texture {
#ifndef AGS
    TexturePixelFormat rotations[4][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#else
    uint8_t rotations[1][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#endif
    TexturePixelFormat rowMajor[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
};

void clearTextures(void);

struct Texture *makeTextureFrom(const char *__restrict__ filename);

struct Bitmap *loadBitmap(const char *__restrict__ filename);

void releaseBitmap(struct Bitmap *ptr);

#endif
