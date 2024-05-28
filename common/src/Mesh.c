#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Common.h"
#include "FixP.h"
#include "Vec.h"
#include "Mesh.h"
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
    struct Vec3 *vecs;
    uint8_t read;
    uint16_t *indexPtr;
    int c;

    read = (*(bufferHead++));
    trigCount += read;
    read = (*(bufferHead++)) << 8;
    trigCount += read;

    uvCoordsCount = trigCount * 6;
    coordsCount = trigCount * 9;

    mesh->triangleCount = trigCount;
    mesh->uvCoords = (uint8_t*)allocMem(uvCoordsCount, GENERAL_MEMORY, 1);
    mesh->geometry = (FixP_t*)allocMem(sizeof(FixP_t) * coordsCount, GENERAL_MEMORY, 1);

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

    mesh->indexCount = (*(bufferHead++));
    mesh->indexCount += (*(bufferHead++)) << 8;

    mesh->nativeVertexBuffer = NULL;
    mesh->nativeTexCoordBuffer = NULL;
    mesh->nativeIndicesBuffer = NULL;
    mesh->nativeBuffer = NULL;

    mesh->vertices = (struct Vec3*)allocMem(3 * mesh->indexCount * sizeof(FixP_t), GENERAL_MEMORY, 1);
    vecs = mesh->vertices;

    for (c = 0; c < mesh->indexCount; ++c) {
        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        vecs->mX = val;

        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        vecs->mY = val;

        val = 0;
        val += (*(bufferHead++) << 0);
        val += (*(bufferHead++) << 8);
        val += (*(bufferHead++) << 16);
        val += (*(bufferHead++) << 24);
        vecs->mZ = val;
        vecs++;
    }

    mesh->indices = (uint16_t*)allocMem(3 * mesh->triangleCount * sizeof(uint16_t), GENERAL_MEMORY, 1);
    indexPtr = mesh->indices;
    for (c = 0; c < mesh->triangleCount * 3; ++c ) {
        uint16_t index;
        index = (*(bufferHead++));
        index += (*(bufferHead++)) << 8;;
        *(indexPtr++) = index;
    }

    read = (*(bufferHead++));

    if (read == 0) {
        mesh->colour = *bufferHead;
        mesh->texture = NULL;
    } else {
        textureName = (char*)allocMem(read + 1, GENERAL_MEMORY, 1);
        memCopyToFrom(textureName, (void *) bufferHead, read);
        mesh->texture = makeTextureFrom(textureName);
        disposeMem(textureName);
    }

    disposeDiskBuffer(buffer);
}
