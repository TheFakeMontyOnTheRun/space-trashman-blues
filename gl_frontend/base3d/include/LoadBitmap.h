#ifndef BITMAP_H
#define BITMAP_H

typedef uint32_t TexturePixelFormat;

typedef uint32_t BitmapPixelFormat;

struct Bitmap {
    BitmapPixelFormat *data;
    uint16_t width;
    uint16_t height;
    int uploadId;
};

struct Texture {
    struct Bitmap *raw;
};

void clearTextures(void);

struct Texture *makeTextureFrom(const char *filename);

struct Bitmap *loadBitmap(const char *filename);

void releaseBitmap(struct Bitmap *ptr);

#endif
