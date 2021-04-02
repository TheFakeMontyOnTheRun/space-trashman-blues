#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#ifndef ANDROID

#include "Common.h"

#else
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

extern int isInstantApp;
extern AAssetManager *defaultAssetManager;

int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *) cookie, buf, size);
}

int android_write(void *cookie, const char *buf, int size) {
    return EACCES;
}

fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *) cookie, offset, whence);
}

int android_close(void *cookie) {
    AAsset_close((AAsset *) cookie);
    return 0;
}


FILE *android_fopen(const char* filename) {

    AAsset *asset = AAssetManager_open(defaultAssetManager, isInstantApp ? "demo.pfs" : "base.pfs", 0);
    if (!asset) {
        return NULL;
    }

    return funopen(asset, android_read, android_write, android_seek, android_close);

}
#endif


#define kDataPath_MaxLength 256

char mDataPath[kDataPath_MaxLength];


void initFileReader(const char *__restrict__ dataFilePath) {
    sprintf (mDataPath, "%s", dataFilePath);
}

size_t sizeOfFile(const char *__restrict__ path) {

#ifndef ANDROID
    FILE *mDataPack = fopen(mDataPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mDataPath[0]);
#endif

    char buffer[85];
    int c;
    uint32_t size = 0;
    uint32_t offset = 0;
    uint16_t entries = 0;
    assert (fread(&entries, 2, 1, mDataPack));

    for (c = 0; c < entries; ++c) {
        uint8_t stringSize = 0;

        assert (fread(&offset, 4, 1, mDataPack));
        offset = toNativeEndianess(offset);
        assert (fread(&stringSize, 1, 1, mDataPack));
        assert (fread(&buffer, stringSize + 1, 1, mDataPack));

        if (!strcmp(buffer, path)) {
            goto found;
        }
    }

    found:
    if (offset == 0) {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    fseek(mDataPack, offset, SEEK_SET);
    assert (fread(&size, 4, 1, mDataPack));
    size = toNativeEndianess(size);
    fclose(mDataPack);

    return size;
}

uint8_t *loadBinaryFileFromPath(const char *__restrict__ path) {

#ifndef ANDROID
    FILE *mDataPack = fopen(mDataPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mDataPath[0]);
#endif

    uint32_t offset = 0;
    uint16_t entries = 0;
    char buffer[85];
    int c;
    uint32_t size = 0;
    uint8_t *toReturn;

    assert (fread(&entries, 2, 1, mDataPack));

    for (c = 0; c < entries; ++c) {
        uint8_t stringSize = 0;

        assert (fread(&offset, 4, 1, mDataPack));
        offset = toNativeEndianess(offset);
        assert (fread(&stringSize, 1, 1, mDataPack));
        assert (fread(&buffer, stringSize + 1, 1, mDataPack));

        if (!strcmp(buffer, path)) {
            goto found;
        }
    }

    found:

    if (offset == 0) {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    fseek(mDataPack, offset, SEEK_SET);

    assert (fread(&size, 4, 1, mDataPack));
    size = toNativeEndianess(size);
    toReturn = (uint8_t *) malloc(size);

    assert (fread(toReturn, sizeof(uint8_t), size, mDataPack));
    fclose(mDataPack);

    return toReturn;
}

FILE *openBinaryFileFromPath(const char *__restrict__ path) {

#ifndef ANDROID
    FILE *mDataPack = fopen(mDataPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mDataPath[0]);
#endif

    uint32_t offset = 0;
    uint16_t entries = 0;
    char buffer[85];
    int c;
    uint32_t size = 0;

    assert (fread(&entries, 2, 1, mDataPack));

    for (c = 0; c < entries; ++c) {
        uint8_t stringSize = 0;

        assert (fread(&offset, 4, 1, mDataPack));
        offset = toNativeEndianess(offset);

        assert (fread(&stringSize, 1, 1, mDataPack));
        assert (fread(&buffer, stringSize + 1, 1, mDataPack));

        if (!strcmp(buffer, path)) {
            goto found;
        }
    }

    return NULL;

    found:

    if (offset == 0) {
        printf("failed to load %s\n", path);
        exit(-1);
    }

    fseek(mDataPack, offset, SEEK_SET);
    assert (fread(&size, 4, 1, mDataPack));
    size = toNativeEndianess(size);

    return mDataPack;
}
