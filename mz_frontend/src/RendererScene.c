#ifndef SMD
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif

#include "Common.h"
#include "Renderer.h"
#include "map.h"
#include "MapWithCharKey.h"
#include "FixP.h"
#include "CTile3DProperties.h"
#include "PackedFileReader.h"

extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;
extern int16_t cameraHeight;
extern struct MapWithCharKey tileProperties;
extern struct MapWithCharKey customMeshes;

extern uint8_t playerLocation;

#ifdef USE_OWN_MIN_MAX
int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}
#endif

void renderScene(void) {

    uint16_t pattern = map[cameraZ][cameraX];

    struct CTile3DProperties *prop =
            (struct CTile3DProperties *) getFromMap(&tileProperties, pattern);

    cameraHeight = fixToInt(prop->mFloorHeight) + 1;


    switch (cameraRotation) {
        case DIRECTION_N:
            renderCameraNorth();
            break;

        case DIRECTION_E:
            renderCameraEast();
            break;

        case DIRECTION_S:
            renderCameraSouth();
            break;

        case DIRECTION_W:
        default:
            renderCameraWest();
            break;
    }
    return;
    repaintMapItems();
}

void renderCameraWest(void) {
    int16_t x;
    int16_t y;

    for (x = 0; x < cameraX; ++x) {

        int16_t minZ = max(cameraZ - ((cameraX) - x) - 2, 0);
        int16_t maxZ = min(cameraZ + ((cameraX) - x) + 1, 31);

        for (y = minZ; y < cameraZ; ++y) {
            drawPattern(map[y][x], -(y - cameraZ) + 1, -(y - cameraZ) + 2, cameraX - x);
        }

        for (y = maxZ; y >= cameraZ; --y) {
            drawPattern(map[y][x], -(y - cameraZ) + 1, -(y - cameraZ) + 2, cameraX - x);
        }
    }
}

void
renderCameraSouth(void) {
    int16_t y;
    int16_t x;

    for (y = 31; y > cameraZ; --y) {

        int16_t maxX = min(cameraX + (y - cameraZ) + 1, 31);
        int16_t minX = max(cameraX - (y - cameraZ) - 3, 0);

        for (x = minX; x < cameraX - 1; ++x) {
            drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
        }


        for (x = maxX - 1; x >= cameraX - 1; --x) {
            drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
        }

    }
}

void renderCameraEast(void) {
    int16_t x;

    int16_t y;


    for (x = 31; x > cameraX; --x) {

        int16_t maxZ = min(cameraZ + (x - cameraX) + 2, 31);
        int16_t minZ = max(cameraZ - (x - cameraX) - 2, 0);

        for (y = minZ; y < cameraZ; ++y) {
            drawPattern(map[y][x], (y - cameraZ) + 2, (y - cameraZ) + 3, x - cameraX);
        }

        for (y = maxZ - 1; y >= cameraZ; --y) {
            drawPattern(map[y][x], (y - cameraZ) + 2, (y - cameraZ) + 3, x - cameraX);
        }
    }
}

void loadMesh(struct Mesh *mesh, char *filename) {
    /* Dummy, just to satisfy the linker */
}

void renderCameraNorth(void) {

    int16_t y;
    int16_t x;

    for (y = 0; y < cameraZ; ++y) {
        int16_t maxX = min(cameraX + ((cameraZ) - y + 3), 31);
        int16_t minX = max(cameraX - ((cameraZ) - y), 0);

        for (x = maxX; x > cameraX - 1; --x) {
            drawPattern(map[y][x], x - cameraX, x - cameraX + 1, cameraZ - y);
        }

        for (x = minX; x <= cameraX + 1; ++x) {
            drawPattern(map[y][x], x - cameraX, x - cameraX + 1, cameraZ - y);
        }
    }
}
