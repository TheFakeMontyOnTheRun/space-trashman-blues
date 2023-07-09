#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef N64
#include <libdragon.h>
#endif

#ifndef LEAN_BUILD

#include <assert.h>

#else
#define assert(x) (x)
#endif

#include <errno.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Common.h"


#ifdef ANDROID
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

#ifndef LEAN_BUILD
#define kDataPath_MaxLength 256
#else
//8.3 + \0 = 8 + 5 = 13
#define kDataPath_MaxLength 13
#endif

#ifndef LEAN_BUILD
char mDataPath[kDataPath_MaxLength];

void initFileReader(const char * dataFilePath) {
	uint8_t len = strlen(dataFilePath);
    memCopyToFrom(&mDataPath[0], (void*)dataFilePath, len);
}

struct StaticBuffer loadBinaryFileFromPath(const char * path) {

#ifndef ANDROID
	FILE *mDataPack = fopen(mDataPath, "rb");
#else
	FILE *mDataPack = android_fopen(mDataPath);
#endif

	struct StaticBuffer toReturn;

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

	assert(FALSE);

found:

	if (offset == 0) {
		printf("failed to load %s\n", path);
		exit(-1);
	}

	fseek(mDataPack, offset, SEEK_SET);

	assert (fread(&size, 4, 1, mDataPack));
	size = toNativeEndianess(size);
	toReturn.size = size;
	toReturn.data = (uint8_t *) allocMem(size, GENERAL_MEMORY, 1);

	assert (fread(toReturn.data, sizeof(uint8_t), size, mDataPack));
	fclose(mDataPack);

	return toReturn;
}

#ifndef LEAN_BUILD

FILE *openBinaryFileFromPath(const char * path) {

#ifndef ANDROID
	FILE *mDataPack = fopen(mDataPath, "rb");
#else
	FILE *mDataPack = android_fopen(mDataPath);
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

#endif

#else

struct StaticBuffer loadBinaryFileFromPath(const uint8_t slot) {

	FILE *mDataPack = fopen("base.pfs", "rb");

	struct StaticBuffer toReturn;

	uint32_t offset = 0;
	uint16_t entries = 0;
	char buffer[85];
	uint16_t c;
	uint32_t size = 0;


	fread(&entries, 2, 1, mDataPack);

	for (c = 0; c < entries; ++c) {
		uint8_t stringSize = 0;

		fread(&offset, 4, 1, mDataPack);
		offset = toNativeEndianess(offset);
		fread(&stringSize, 1, 1, mDataPack);
		fread(&buffer, stringSize + 1, 1, mDataPack);

		if (c == slot) {
			goto found;
		}
	}

found:
	fseek(mDataPack, offset, SEEK_SET);

	fread(&size, 4, 1, mDataPack);
	size = toNativeEndianess(size);
	toReturn.size = size;
	toReturn.data = (uint8_t *) allocMem(size, GENERAL_MEMORY, 1);

	fread(toReturn.data, sizeof(uint8_t), size, mDataPack);
	fclose(mDataPack);

	return toReturn;
}

#endif

void disposeDiskBuffer(struct StaticBuffer buffer) {
	disposeMem(buffer.data);
}
