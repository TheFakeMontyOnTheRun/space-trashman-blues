#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "map.h"

#ifdef MSDOS
#include "Common.h"
#endif

const struct CellPattern patterns[96] = {
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    32 */
        {0,                   3,  CUBE,       0}, /*    33 */
        {0,                   3,  RIGHT_NEAR, 0}, /*    34 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       1}, /*    35 #  */
        {3,                   3,  CUBE,       0}, /*    36 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    37 */
        {0,                   0,  LEFT_WALL,  0}, /*    38 */
        {0,                   3,  BACK_WALL,  0}, /*    39 */
        {6,                   3,  CUBE,       0}, /*    40 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    41 */
        {0,                   0,  CUBE,       0}, /*    42 */
        {0,                   3,  CUBE,       0}, /*    43 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    44 , */
        {0,                   15, BACK_WALL,  0}, /*    45 - */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    46 . */
        {0,                   3,  RIGHT_NEAR, 1}, /*    47 / */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    48 0 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    49 1 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    50 2 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    51 3 */
        {0,                   0,  CUBE,       0}, /*    52 4 */
        {0,                   3,  CUBE,       0}, /*    53 5 */
        {0,                   3,  RIGHT_NEAR, 0}, /*    54 6 */
        {0,                   3,  LEFT_NEAR,  0}, /*    55 7 */
        {3,                   3,  CUBE,       0}, /*    56 8 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    57 9 */
        {0,                   13, BACK_WALL,  0}, /*   58 : */
        {0,                   13, LEFT_WALL,  0}, /*    59 ; */
        {0,                   15, CORNER,     0}, /*    60 < */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    61 */
        {0,                   0,  CUBE,       0}, /*    62 */
        {0,                   3,  CUBE,       0}, /*    63 */
        {0,                   3,  RIGHT_NEAR, 0}, /*    64 */
        {0,                   3,  LEFT_NEAR,  0}, /*    65 */
        {3,                   3,  CUBE,       0}, /*    66 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    67 */
        {0,                   0,  CUBE,       0}, /*    68 D */
        {5,                   0,  CUBE,       0}, /*    69 E */
        {5,                   15, CUBE,       0}, /*    70 F */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    71 */
        {1,                   15, CUBE,       1}, /*    72 */
        {0,                   15, CUBE,       1}, /*    73 I */
        {0,                   3,  RIGHT_NEAR, 0}, /*    74 */
        {0,                   3,  LEFT_NEAR,  0}, /*    75 */
        {4,                   0,  CUBE,       0}, /*    76 L */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    77 */
        {0,                   0,  LEFT_WALL,  0}, /*    78 */
        {0,                   0,  CUBE,       0}, /*    79 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    80 */
        {0,                   3,  CUBE,       0}, /*    81 */
        {0,                   0,  CUBE,       0}, /*    82 */
        {0,                   3,  CUBE,       0}, /*    83 */
        {6,                   3,  CUBE,       0}, /*    84 */
        {0,                   3,  LEFT_NEAR,  0}, /*    85 */
        {3,                   3,  CUBE,       0}, /*    86 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    87 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    88 */
        {0,                   3,  BACK_WALL,  0}, /*    89 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    90 */
        {0,                   3,  LEFT_NEAR,  0}, /*    91 [ */
        {0,                   3,  LEFT_NEAR,  1}, /*    92 \ */
        {0,                   3,  RIGHT_NEAR, 0}, /*    93 ] */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    94 ^ */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       1}, /*    95 _ */
        {3,                   3,  CUBE,       0}, /*    96 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*    97 */
        {0,                   0,  LEFT_WALL,  0}, /*    98 */
        {0,                   3,  BACK_WALL,  0}, /*    99 */
        {6,                   3,  CUBE,       0}, /*   100 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   101 e */
        {0,                   0,  CUBE,       0}, /*   102 */
        {0,                   3,  CUBE,       0}, /*   103 */
        {0,                   3,  RIGHT_NEAR, 0}, /*   104 */
        {0,                   3,  LEFT_NEAR,  0}, /*   105 */
        {3,                   3,  CUBE,       0}, /*   106 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   107 */
        {0,                   0,  LEFT_WALL,  0}, /*   108 */
        {0,                   3,  BACK_WALL,  0}, /*   109 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   110 n */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   111 */
        {0,                   0,  CUBE,       0}, /*   112 */
        {0,                   3,  CUBE,       0}, /*   113 */
        {0,                   3,  RIGHT_NEAR, 0}, /*   114 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   115 s */
        {3,                   3,  CUBE,       0}, /*   116 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   117 */
        {0,                   0,  LEFT_WALL,  0}, /*   118 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   119 w */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   120 */
        {BASE_CEILING_HEIGHT, 3,  CUBE,       0}, /*   121 */
        {0,                   0,  CUBE,       0}, /*   122 */
        {0,                   3,  CUBE,       0}, /*   123 */
        {0,                   15, LEFT_WALL,  1}, /*   124 | */
        {0,                   3,  LEFT_NEAR,  0}, /*   125 */
        {3,                   3,  CUBE,       0}, /*   126 */
};

extern int8_t cameraX;
extern int8_t cameraZ;
extern int8_t cameraRotation;
extern uint8_t playerLocation;

uint8_t drawPattern(uint8_t _pattern, int8_t x0, int8_t x1, int8_t y) {
    int8_t diff;
    uint8_t pattern = (_pattern - RLE_THRESHOLD) & 127;
    uint8_t type;
    uint8_t mask;
    /* 127 = 01111111 - the first bit is used for indicating the presence of an object.
     * And since there are only 127 patterns anyway...
     * */

#ifdef OPTIMIZATION_BLOCK_CELL
    if (_pattern == BLOCK_CELL) {
        return 0;
    }
#endif

    if (_pattern == NEUTRAL_CELL) {
        return 1;
    }

    diff = patterns[0].ceiling - patterns[pattern].ceiling;
    type = patterns[pattern].geometryType;
    mask = patterns[pattern].elementsMask;

    if (x0 == 2) {
        mask = 255;
    }

    if (x1 == 2) {
        mask = 127;
    }

    if (type == CUBE) {
        return drawCubeAt(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT,
                          RENDER_SCALE_Z * (y + 2),
                          RENDER_SCALE_X * (x1 - x0), diff, RENDER_SCALE_Z,
                          mask);
    } else if (type == RIGHT_NEAR || type == LEFT_NEAR) {

        if (cameraRotation == 1 || cameraRotation == 3) {

            if (type == RIGHT_NEAR) {
                type = LEFT_NEAR;
            } else {
                type = RIGHT_NEAR;
            }
        }

        return drawWedge(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT, RENDER_SCALE_Z * (y + 2),
                         RENDER_SCALE_X * (x1 - x0), diff, RENDER_SCALE_Z,
                         patterns[pattern].elementsMask, type);

    } else if (type == LEFT_WALL) {

        switch (cameraRotation) {
            case 0:
            case 2:
                return drawWedge(RENDER_SCALE_X * (x0 - (cameraRotation == 0 ? 1 : 0)),
                                 patterns[pattern].ceiling - CAMERA_HEIGHT, RENDER_SCALE_Z * (y + 2),
                                 0, diff, RENDER_SCALE_Z,
                                 patterns[pattern].elementsMask, LEFT_WALL);
            case 1:
            case 3:
                return drawSquare(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT,
                                  RENDER_SCALE_Z * (y + (cameraRotation == 3 ? 1 : 0) + 2),
                                  RENDER_SCALE_X * (x1 - x0), diff, mask);
        }
    } else if (type == BACK_WALL) {
        switch (cameraRotation) {
            case 0:
            case 2:
                return drawSquare(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT,
                                  RENDER_SCALE_Z * (y + (cameraRotation == 0 ? 1 : 0) + 2),
                                  RENDER_SCALE_X * (x1 - x0), diff, mask);
            case 1:
            case 3:
                return drawWedge(RENDER_SCALE_X * (x0 - (cameraRotation == 1 ? 1 : 0)),
                                 patterns[pattern].ceiling - CAMERA_HEIGHT, RENDER_SCALE_Z * (y + 2),
                                 0, diff, RENDER_SCALE_Z, patterns[pattern].elementsMask, LEFT_WALL);
        }
    } else if (type == CORNER) {
        uint8_t returnVal = 0;

        switch (cameraRotation) {

            case 3:
            case 0:
                returnVal = drawWedge(RENDER_SCALE_X * (x0 - (cameraRotation == 3 ? 0 : 1)),
                                      patterns[pattern].ceiling - CAMERA_HEIGHT, RENDER_SCALE_Z * (y + 2),
                                      0, diff, RENDER_SCALE_Z, patterns[pattern].elementsMask, LEFT_WALL);

                returnVal = drawSquare(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT,
                                       RENDER_SCALE_Z * (y + 1 + 2),
                                       RENDER_SCALE_X * (x1 - x0), diff, patterns[pattern].elementsMask) || returnVal;
                break;

            case 1:
            case 2:
                returnVal = drawSquare(RENDER_SCALE_X * (x0 - 1), patterns[pattern].ceiling - CAMERA_HEIGHT,
                                       RENDER_SCALE_Z * (y + 2),
                                       RENDER_SCALE_X * (x1 - x0), diff, patterns[pattern].elementsMask);
                returnVal =
                        drawWedge(RENDER_SCALE_X * (x0 - (cameraRotation == 1 ? 1 : 0)),
                                  patterns[pattern].ceiling - CAMERA_HEIGHT, RENDER_SCALE_Z * (y + 2),
                                  0, diff, RENDER_SCALE_Z, patterns[pattern].elementsMask, LEFT_WALL) || returnVal;

                break;
        }

        return returnVal;
    }

    return 0;
}

void repaintMapItems(void) {
    struct ObjectNode *node;

    /* ignore header node */
    node = getRoom(playerLocation)->itemsPresent->next;

    switch (cameraRotation) {
        case 0:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(RENDER_SCALE_X * (item->position.x - cameraX + 2 - 1), cameraZ - item->position.y + 2);
                node = node->next;
            }
            break;

        case 1:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(RENDER_SCALE_X * ((item->position.y - cameraZ) + 1), (item->position.x - cameraX) + 2);
                node = node->next;
            }
            break;

        case 2:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(RENDER_SCALE_X * (-(item->position.x - cameraX) + 1), (item->position.y - cameraZ) + 2);
                node = node->next;
            }
            break;

        case 3:
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(RENDER_SCALE_X * (-(item->position.y - cameraZ) + 1), (cameraX - item->position.x) + 2);
                node = node->next;
            }
            break;
    }
}

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION

void startRoomTransitionAnimation(void) {
    uint8_t x, y;

    for (y = MAP_SIZE_Y; y >= 2; --y) {
        vLine(y, y, 95 + (MAP_SIZE_Y - y), 1);
        vLine(95 + (MAP_SIZE_Y - y), y, 95 + (MAP_SIZE_Y - y), 1);

        for (x = y; x < (95 + (MAP_SIZE_Y - y)); ++x) {
            graphicsPut(x, y);
            graphicsPut(x, 95 + (MAP_SIZE_Y - y));

            /* door opening */
            graphicsPut(x, 95 - 3 * (MAP_SIZE_Y - y));
        }
#ifdef SDLW
        /* Just to keep the OS happy */
        getInput();
#endif
        graphicsFlush();
    }
    HUD_initialPaint();
}

#endif

void updateMapItems(void) {
    struct ObjectNode *node;

    /* ignore header node */
    node = getRoom(playerLocation)->itemsPresent->next;

    while (node != NULL) {
        struct Item *item = getItem(node->item);
        uint8_t pattern = map[item->position.y][item->position.x];
        map[item->position.y][item->position.x] = pattern | 128;
        node = node->next;
    }

}
