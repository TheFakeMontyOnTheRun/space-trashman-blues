#ifndef BITMAP_H
#define BITMAP_H

#define NATIVE_TEXTURE_SIZE 16

#ifdef RGBA32_FRAMEBUFFER
typedef uint32_t TexturePixelFormat;
typedef uint32_t BitmapPixelFormat;
#else
typedef uint8_t TexturePixelFormat;
typedef uint8_t BitmapPixelFormat;
#endif

struct Bitmap {
    BitmapPixelFormat *data;
    uint16_t width;
    uint16_t height;
};


struct Texture {
#ifndef FLOOR_TEXTURES_DONT_ROTATE
    TexturePixelFormat rotations[4][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#else
    TexturePixelFormat rotations[1][NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
#endif
    TexturePixelFormat rowMajor[NATIVE_TEXTURE_SIZE * NATIVE_TEXTURE_SIZE];
};

void clearTextures(void);

struct Texture *makeTextureFrom(const char *__restrict__ filename);

struct Bitmap *loadBitmap(const char *__restrict__ filename);

void releaseBitmap(struct Bitmap *ptr);

FixP_t lerpFix(const FixP_t v0, const FixP_t v1, const FixP_t t, const FixP_t total);

int lerpInt(const int v0, const int v1, const long t, const long total);

#endif
