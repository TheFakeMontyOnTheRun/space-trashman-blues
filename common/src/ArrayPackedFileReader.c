//
// Created by Daniel Monteiro on 10/02/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef SMD
#include <assert.h>
#else
#define assert(x) x
#define exit(x) /* x */
#endif

#include "Common.h"
#include "basepfs.h"

void initFileReader(const char * dataFilePath) {
}

void disposeDiskBuffer(struct StaticBuffer buffer) {}

struct StaticBuffer loadBinaryFileFromPath(const char * path) {

	const uint8_t *mDataPack = &basepfs[0];

	struct StaticBuffer toReturn;

	uint32_t offset = 0;
	uint16_t entries = 0;
	char buffer[85];
	int c;
	uint32_t size = 0;

	memcpy(&entries, mDataPack, 2);
	mDataPack+= 2;

	for (c = 0; c < entries; ++c) {
		uint8_t stringSize = 0;

		memcpy(&offset, mDataPack, 4);
		mDataPack+= 4;

		offset = toNativeEndianess(offset);

		memcpy(&stringSize, mDataPack, 1);
		mDataPack+= 1;

		memcpy(&buffer[0], mDataPack, stringSize + 1);
		mDataPack+= stringSize + 1;

		if (!strcmp(buffer, path)) {
			goto found;
		}
	}

	assert(FALSE);

found:

	if (offset == 0) {
		exit(-1);
	}

	mDataPack = &basepfs[offset];

	memcpy(&size, mDataPack, 4);
	mDataPack+= 4;

	size = toNativeEndianess(size);
	toReturn.size = size;
	toReturn.data = mDataPack;

	return toReturn;
}
