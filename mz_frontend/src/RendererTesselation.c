#include <stddef.h>
#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#include "Common.h"
#include "Core.h"
#include "Renderer.h"
#include "map.h"
#include "MapWithCharKey.h"
#include "FixP.h"
#include "CTile3DProperties.h"
#include "PackedFileReader.h"

#include "map.h"

extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;
extern uint8_t playerLocation;
struct MapWithCharKey tileProperties;
struct MapWithCharKey customMeshes;
int16_t cameraHeight = 1;

void drawPattern(uint16_t _pattern, int16_t x0, int16_t x1, int16_t z) {
    int16_t diff;
    uint16_t pattern = (_pattern) & 127;
    uint16_t type;

    /* 127 = 01111111 - the first bit is used for indicating the presence of an object.
     * And since there are only 127 patterns anyway...
     * */
    if (_pattern & 128) {
        drawObjectAt(x0 - 1, z + 2);
    }

    struct CTile3DProperties *prop =
            (struct CTile3DProperties *) getFromMap(&tileProperties, pattern);

    int ceilingHeight = fixToInt(prop->mCeilingHeight);
    int floorHeight = fixToInt(prop->mFloorHeight);


    diff = ceilingHeight - floorHeight;

    type = prop->mGeometryType;

    if (prop->mCeilingRepeatedTextureIndex != 0xFF && prop->mCeilingRepetitions > 0) {
        drawCubeAt(x0 - 1, ceilingHeight - cameraHeight, z + 2, x1 - x0,
                   prop->mCeilingRepetitions, 1);
    }

    if (prop->mFloorRepeatedTextureIndex != 0xFF && prop->mFloorRepetitions > 0) {
        drawCubeAt(x0 - 1, floorHeight - prop->mFloorRepetitions - cameraHeight, z + 2, x1 - x0,
                   prop->mFloorRepetitions, 1);
    }


    if (prop->mCeilingTextureIndex != 0xFF) {
        drawFloorAt(x0 - 1, ceilingHeight - cameraHeight, z + 2, x1 - x0, 1);
    }

    if (prop->mFloorTextureIndex != 0xFF) {
        drawFloorAt(x0 - 1, floorHeight - cameraHeight, z + 2, x1 - x0, 1);
    }


    if (type == kCube) {
        drawCubeAt(x0 - 1, floorHeight - cameraHeight, z + 2, x1 - x0,
                   diff, 1);
    } else if (type == kRightNearWall || type == kLeftNearWall) {

        if (cameraRotation == 0 || cameraRotation == 2) {

            if (type == kRightNearWall) {
                type = kLeftNearWall;
            } else {
                type = kRightNearWall;
            }
        }

        drawWedge(x0 - 1, floorHeight - cameraHeight, z + 2, x1 - x0,
                  diff, 1, 0xFF, type);

    } else if (type == kWallWest) {

        switch (cameraRotation) {
            case 0:
            case 2:
                drawWedge(x0 - (cameraRotation == 0 ? 1 : 0), floorHeight - cameraHeight, z + 2,
                          0, diff, 1, 0xFF, kLeftNearWall);
                break;
            case 1:
            case 3:
            default:
                drawSquare(x0 - 1, floorHeight - cameraHeight,
                           z + (cameraRotation == 3 ? 1 : 0) + 2,
                           x1 - x0, diff, 0xFF);
        }
    } else if (type == kWallNorth) {


        switch (cameraRotation) {
            case 0:
            case 2:
                drawSquare(x0 - 1, floorHeight - cameraHeight,
                           z + (cameraRotation == 0 ? 1 : 0) + 2,
                           x1 - x0, diff, 0xFF);
                break;
            case 1:
            case 3:
            default:
                drawWedge(x0 - (cameraRotation == 1 ? 1 : 0),
                          floorHeight - cameraHeight, z + 2,
                          0, diff, 1, 0xFF, kLeftNearWall);
                break;
        }
    } else if (type == kWallCorner) {

        switch (cameraRotation) {

            case 3:
            case 0:
                drawWedge(x0 - (cameraRotation == 3 ? 0 : 1),
                          floorHeight - cameraHeight, z + 2,
                          0, diff, 1, 0xFF, kLeftNearWall);

                drawSquare(x0 - 1, floorHeight - cameraHeight, z + 1 + 2,
                           x1 - x0, diff, 0xFF);
                break;

            case 1:
            case 2:
            default:
                drawSquare(x0 - 1, floorHeight - cameraHeight, z + 2,
                           x1 - x0, diff, 0xFF);

                drawWedge(x0 - (cameraRotation == 1 ? 1 : 0), floorHeight - cameraHeight, z + 2,
                          0, diff, 1, 0xFF, kLeftNearWall);

                break;
        }
    }
}

void repaintMapItems(void) {
    struct ObjectNode *node;

    /* ignore header node */
    node = getRoom(playerLocation)->itemsPresent->next;

    switch (cameraRotation) {
        case 0:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(item->position.x - cameraX + 2 - 1, cameraZ - item->position.y + 2);
                node = node->next;
            }
            break;

        case 1:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt((item->position.y - cameraZ) + 1, (item->position.x - cameraX) + 2);
                node = node->next;
            }
            break;

        case 2:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(-(item->position.x - cameraX) + 1, (item->position.y - cameraZ) + 2);
                node = node->next;
            }
            break;

        case 3:
        default:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(-(item->position.y - cameraZ) + 1, (cameraX - item->position.x) + 2);
                node = node->next;
            }
            break;
    }
}

void updateMapItems(void) {
    struct ObjectNode *node;

    /* ignore header node */
    node = getRoom(playerLocation)->itemsPresent->next;

    while (node != NULL) {
        struct Item *item = getItem(node->item);
        uint16_t pattern = map[item->position.y][item->position.x];
        map[item->position.y][item->position.x] = pattern;
        node = node->next;
    }
}
