#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "FixP.h"
#include "Vec.h"
#include "Mesh.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "LoadBitmap.h"

void loadMesh(struct Mesh *mesh, char *filename) {
    struct StaticBuffer buffer = loadBinaryFileFromPath(filename);
    FixP_t val = 0;
    const uint8_t *bufferHead = buffer.data;
    int16_t trigCount = 0;
    int uvCoordsCount;
    int coordsCount;
    char *textureName;
    uint8_t *uvCoord;
    FixP_t *coord;
    uint8_t read;
    int c;

    read = (*(bufferHead++));
    trigCount += read;
    read = (*(bufferHead++)) << 8;
    trigCount += read;

    uvCoordsCount = trigCount * 6;
    coordsCount = trigCount * 9;

    mesh->triangleCount = trigCount;
    mesh->uvCoords = allocMem(uvCoordsCount, GENERAL_MEMORY, 1);
    mesh->geometry = allocMem(sizeof(FixP_t) * coordsCount, GENERAL_MEMORY, 1);

    uvCoord = mesh->uvCoords;
    coord = mesh->geometry;

    for (c = 0; c < uvCoordsCount; ++c) {
        uint8_t val = (*(bufferHead++));
        *(uvCoord++) = val / 2;
    }

    for (c = 0; c < coordsCount; ++c) {
        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        *(coord++) = val;
    }

    read = (*(bufferHead++));

    if (read == 0) {
        mesh->colour = *bufferHead;
        mesh->texture = NULL;
    } else {
        textureName = allocMem(read + 1, GENERAL_MEMORY, 1);
        memCopyToFrom(textureName, (void *) bufferHead, read);
        mesh->texture = makeTextureFrom(textureName);
        disposeMem(textureName);
    }

    disposeDiskBuffer(buffer);
}