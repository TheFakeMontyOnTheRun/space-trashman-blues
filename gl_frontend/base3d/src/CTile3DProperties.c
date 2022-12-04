#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "Common.h"
#include "FixP.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "PackedFileReader.h"
#include "MapWithCharKey.h"

void loadPropertyList(const char *__restrict__ propertyFile, struct MapWithCharKey *__restrict__ map) {

    struct StaticBuffer buffer = loadBinaryFileFromPath(propertyFile);
    uint8_t *limit = buffer.data + buffer.size;
    uint8_t *bufferHead = buffer.data;

    clearMap(map);

    while (bufferHead != limit) {
        FixP_t val = 0;
        uint8_t key = *(bufferHead++);
        struct CTile3DProperties *prop = (struct CTile3DProperties *) calloc(
                1, sizeof(struct CTile3DProperties));

        prop->mNeedsAlphaTest = *(bufferHead++);
        prop->mBlockVisibility = *(bufferHead++);
        prop->mBlockMovement = *(bufferHead++);
        prop->mBlockEnemySight = *(bufferHead++);
        prop->mRepeatMainTexture = *(bufferHead++);

        prop->mCeilingTextureIndex = *(bufferHead++);
        prop->mFloorTextureIndex = *(bufferHead++);
        prop->mMainWallTextureIndex = *(bufferHead++);

        prop->mGeometryType = (enum GeometryType) (*(bufferHead++));

        prop->mCeilingRepeatedTextureIndex = *(bufferHead++);
        prop->mFloorRepeatedTextureIndex = *(bufferHead++);
        prop->mCeilingRepetitions = *(bufferHead++);
        prop->mFloorRepetitions = *(bufferHead++);

        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        prop->mCeilingHeight = val;

        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        prop->mFloorHeight = val;

        setInMap(map, key, prop);
    }

    free(buffer.data);
}
