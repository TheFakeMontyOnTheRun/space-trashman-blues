//
// Created by Daniel Monteiro on 10/02/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "Common.h"
#include "basepfs.h"

void initFileReader(const char * dataFilePath) {
}

struct StaticBuffer loadBinaryFileFromPath(const char * path) {

	uint8_t *mDataPack = &basepfs[0];

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
		printf("failed to load %s\n", path);
		exit(-1);
	}

	mDataPack = &basepfs[offset];

	memcpy(&size, mDataPack, 4);
	mDataPack+= 4;

	size = toNativeEndianess(size);
	toReturn.size = size;
	toReturn.data = (uint8_t *) malloc(size);

	memcpy(toReturn.data, mDataPack, sizeof(uint8_t) * size);

	return toReturn;
}