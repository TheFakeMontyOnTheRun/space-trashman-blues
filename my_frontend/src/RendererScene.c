#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "map.h"

#ifdef SUPPORTS_HACKING_MINIGAME

#include "HackingMinigame.h"

#endif

#ifdef MSDOS
#include "Common.h"
#endif

#ifndef EMBEDDED_DATA
#include "PackedFileReader.h"
#endif

extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;

extern int8_t stencilHigh[XRES];

extern uint8_t playerLocation;

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}

/* all those refactors are due to a SDCC bug with very long functions */
void renderScene(void) {

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
            renderCameraWest();
            break;
    }

    graphicsPutPointArray(&stencilHigh[0]);
    repaintMapItems();

    memset(stencilHigh, 0, XRES);
}

void renderCameraWest(void) {
    int8_t x;
    uint8_t lastPattern, lastIndex;
    int8_t maxX = 0;
    int8_t y;
    uint8_t pattern;

    for (x = cameraX; x >= 0; --x) {

        int8_t minX = min(cameraZ + ((cameraX) - x), (MAP_SIZE_Y - 1));
        lastIndex = cameraZ;
        lastPattern = map[lastIndex][x];

        for (y = lastIndex; y < minX; ++y) {

            pattern = map[y][x];

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, -(y - cameraZ) + 2, -(lastIndex - cameraZ) + 2, cameraX - x)) {
                    y = minX - 1;
                }
                lastIndex = y;
                lastPattern = pattern;
            }
        }
        drawPattern(lastPattern, -(y - cameraZ) + 2, -(lastIndex - cameraZ) + 2, cameraX - x);


        lastIndex = cameraZ - 1;
        lastPattern = map[lastIndex][x];

        maxX = max(cameraZ - ((cameraX) - x), 0);

        for (y = lastIndex; y >= maxX; --y) {
            pattern = map[y][x];

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, -(lastIndex + 1 - cameraZ) + 2, -(y + 1 - cameraZ) + 2, cameraX - x)) {
                    y = maxX + 1;
                }

                lastIndex = y;
                lastPattern = pattern;
            }
        }
        drawPattern(lastPattern, -(lastIndex + 1 - cameraZ) + 2, -(y + 1 - cameraZ) + 2, cameraX - x);
    }
}

void renderCameraSouth(void) {
    int8_t y;
    uint8_t lastPattern, lastIndex;
    int8_t const *mapXY;
    int8_t maxX = 0;
    int8_t x;
    uint8_t pattern;

    for (y = cameraZ; y < VISIBILITY_LIMIT_Y; ++y) {

        int8_t const *mapY = &map[y][0];
        int8_t minX = min(cameraX + (y - cameraZ), (MAP_SIZE_X - 1));
        lastIndex = cameraX;
        lastPattern = *(mapY + lastIndex);
        mapXY = &map[y][lastIndex];

        for (x = lastIndex; x <= minX; ++x) {

            pattern = *mapXY;

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, -(x - cameraX) + 2, -(lastIndex - cameraX) + 2, y - cameraZ)) {
                    x = VISIBILITY_LIMIT_X;
                }
                lastIndex = x;
                lastPattern = pattern;
            }

            ++mapXY;
        }

        drawPattern(lastPattern, -(x - cameraX) + 2, -(lastIndex - cameraX) + 2, y - cameraZ);

        lastIndex = cameraX - 1;
        lastPattern = *(mapY + lastIndex);

        mapXY = &map[y][lastIndex];
        maxX = max(cameraX - (y - cameraZ), 0);

        for (x = lastIndex; x >= maxX; --x) {
            pattern = *mapXY;

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, cameraX - lastIndex + 1, cameraX - x + 1, y - cameraZ)) {
                    x = maxX + 1;
                }

                lastIndex = x;
                lastPattern = pattern;
            }
            --mapXY;
        }
        drawPattern(lastPattern, cameraX - lastIndex + 1, cameraX - x + 1, y - cameraZ);
    }
}

void renderCameraEast(void) {
    int8_t x;
    uint8_t lastPattern, lastIndex;
    int8_t maxX = 0;
    int8_t y;
    uint8_t pattern;

    for (x = cameraX; x < VISIBILITY_LIMIT_X; ++x) {

        int8_t minY = min(cameraZ + (x - cameraX), 31);
        lastIndex = cameraZ;
        lastPattern = map[lastIndex][x];

        for (y = lastIndex; y <= minY; ++y) {

            pattern = map[y][x];

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, (lastIndex - cameraZ) + 2, (y - cameraZ) + 2, x - cameraX)) {
                    y = VISIBILITY_LIMIT_Y;
                }
                lastIndex = y;
                lastPattern = pattern;
            }
        }

        drawPattern(lastPattern, (lastIndex - cameraZ) + 2, (y - cameraZ) + 2, x - cameraX);


        lastIndex = cameraZ - 1;
        lastPattern = map[lastIndex][x];

        maxX = max(cameraZ - (x - cameraX), 0);

        for (y = lastIndex; y >= maxX; --y) {
            pattern = map[y][x];

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, -(cameraZ - y) + 3, -(cameraZ - lastIndex) + 3, x - cameraX)) {
                    y = maxX + 1;
                }

                lastIndex = y;
                lastPattern = pattern;
            }
        }

        drawPattern(lastPattern, -(cameraZ - y) + 3, -(cameraZ - lastIndex) + 3, x - cameraX);
    }
}

void renderCameraNorth(void) {

    uint8_t lastPattern, lastIndex;
    int8_t const *mapXY;
    int8_t maxX = 0;
    int8_t y;
    int8_t x;
    uint8_t pattern;

    for (y = cameraZ; y >= 0; --y) {

        int8_t const *mapY = &map[y][0];
        int8_t minX = min(cameraX + ((cameraZ) - y), 31);
        lastIndex = cameraX;
        lastPattern = *(mapY + lastIndex);
        mapXY = &map[y][lastIndex];

        for (x = lastIndex; x < minX; ++x) {

            pattern = *mapXY;

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, lastIndex - cameraX + 2, x - cameraX + 2, cameraZ - y)) {
                    x = minX - 1;
                }
                lastIndex = x;
                lastPattern = pattern;
            }

            ++mapXY;
        }
        drawPattern(lastPattern, lastIndex - cameraX + 2, x - cameraX + 2, cameraZ - y);

        lastIndex = cameraX - 1;
        lastPattern = *(mapY + lastIndex);

        mapXY = &map[y][lastIndex];
        maxX = max(cameraX - ((cameraZ) - y), 0);

        for (x = lastIndex; x >= maxX; --x) {
            pattern = *mapXY;

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, x + 1 - cameraX + 2, lastIndex + 1 - cameraX + 2, cameraZ - y)) {
                    x = maxX + 1;
                }

                lastIndex = x;
                lastPattern = pattern;
            }
            --mapXY;
        }
        drawPattern(lastPattern, x + 1 - cameraX + 2, lastIndex + 1 - cameraX + 2, cameraZ - y);
    }
}
