#include <stddef.h>

#ifdef AMIGA
#include "AmigaInt.h"
#endif

#ifdef ATARIST
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mint/osbind.h>
#include <mint/sysbind.h>
#endif

#ifndef SMD

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#else
#include <genesis.h>
int puts( const char* str );
#endif

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#ifdef SUPPORTS_HACKING_MINIGAME

#include "HackingMinigame.h"

#endif

#include "Common.h"
#include "PackedFileReader.h"
#include "MapWithCharKey.h"
#include "FixP.h"
#include "CTile3DProperties.h"
#include "map.h"

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

#define STIPPLE_DISTANCE 13
#define STIPPLE_COLOUR_THRESHOLD 16

struct ObjectNode *focusedItem = NULL;
struct ObjectNode *roomItem = NULL;
struct MapWithCharKey tileProperties;
extern int accessGrantedToSafe;
int cursorPosition = 0;

void performAction();

void startMusic();

void renderCameraNorth();

void renderCameraEast();

void renderCameraSouth();

void renderCameraWest();

int16_t cameraX = 33;
int16_t cameraZ = 22;
int16_t cameraRotation = 0;
uint16_t running = 1;
int16_t cameraHeight = 1;
uint16_t enteredFrom = 0xFF;

extern int playerLocation;

struct Projection {
    uint8_t px;
    uint8_t py;
    int8_t dx;
};

const struct Projection projections[31] =
        {
                {0,  128, -128},    //	1
                {0,  127, -64},    //	2
                {20, 105, -42},    //	3
                {31, 95,  -32},    //	4
                {37, 88,  -25},    //	5
                {41, 84,  -21},    //	6
                {44, 81,  -18},    //	7
                {47, 79,  -16},    //	8
                {48, 77,  -14},    //	9
                {50, 75,  -12},    //	10
                {51, 74,  -11},    //	11
                {52, 73,  -10},    //	12
                {53, 72,  -9},    //	13
                {53, 72,  -9},    //	14
                {54, 71,  -8},    //	15
                {55, 71,  -8},    //	16
                {55, 70,  -7},    //	17
                {55, 70,  -7},    //	18
                {56, 69,  -6},    //	19
                {56, 69,  -6},    //	20
                {56, 69,  -6},    //	21
                {57, 68,  -5},    //	22
                {57, 68,  -5},    //	23
                {57, 68,  -5},    //	24
                {57, 68,  -5},    //	25
                {58, 67,  -4},    //	26
                {58, 67,  -4},    //	27
                {58, 67,  -4},    //	28
                {58, 67,  -4},    //	29
                {58, 67,  -4},    //	30
                {58, 67,  -4},    //	31
        };

#ifdef LEAN_BUILD
int16_t max(int16_t x1, int16_t x2) {
    return x1 > x2 ? x1 : x2;
}

int16_t min(int16_t x1, int16_t x2) {
    return x1 < x2 ? x1 : x2;
}
#endif

void
drawWedge(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, int16_t dZ, uint16_t elementMask, uint16_t type) {

    int16_t z1;

    int16_t z0px;
    int16_t z1px;
    int16_t z0py;
    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;

    int16_t px0z0;
    int16_t py0z0;
    int16_t py0z1;
    int16_t py1z0;
    int16_t py1z1;
    int16_t px1z1;

    uint16_t shouldStippleFill = (z0 >= STIPPLE_DISTANCE) ? (11 + STIPPLE_COLOUR_THRESHOLD) : 11;
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? (3 + STIPPLE_COLOUR_THRESHOLD) : 3;

    if (z0 >= 32) {
        return;
    }

    z1 = z0 + dZ;

    if (z0 <= 2) {
        return;
    }

    if (z1 <= 2) {
        return;
    }

    if (z1 >= 32) {
        return;
    }


    if (type == kRightNearWall) {
        z0px = (projections[z0].px);
        z1px = (projections[z1].px);
        z0dx = ((projections[z0].dx));
        z1dx = ((projections[z1].dx));

        px0z0 = z0px - (x0 * z0dx);
        px1z1 = z1px - ((dX + x0) * z1dx);

        z1py = (projections[z1].py);
        z0py = (projections[z0].py);

        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);

        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);

    } else {
        z0px = (projections[z1].px);
        z1px = (projections[z0].px);
        z0dx = ((projections[z1].dx));
        z1dx = ((projections[z0].dx));

        px0z0 = z0px - ((x0) * z0dx);
        px1z1 = z1px - ((x0 + dX) * z1dx); //extra operations to avoid overflow

        z1py = (projections[z0].py);
        z0py = (projections[z1].py);

        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);

        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
    }


    int16_t lineX0, lineX1;

    if (py1z0 < 0) {
        py1z0 = 0;
    }

    if (py0z0 < 0) {
        py0z0 = 0;
    }

    if (py1z0 >= YRES) {
        py1z0 = YRESMINUSONE;
    }

    if (py0z0 >= YRES) {
        py0z0 = YRESMINUSONE;
    }


    if (py1z1 < 0) {
        py1z1 = 0;
    }

    if (py0z1 < 0) {
        py0z1 = 0;
    }

    if (py1z1 >= YRES) {
        py1z1 = YRESMINUSONE;
    }

    if (py0z1 >= YRES) {
        py0z1 = YRESMINUSONE;
    }

    /* The upper segment */
    lineX0 = px0z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t upperY0 = py1z0;
        int16_t upperY1 = py1z1;
        int16_t upperDx = abs(lineX1 - lineX0);
        int16_t upperDy = -abs(upperY1 - upperY0);
        int16_t upperSy = upperY0 < upperY1 ? 1 : -1;
        int16_t upperErr = upperDx + upperDy;  /* error value e_xy */
        int16_t upperErr2;
        int16_t lowerY0 = py0z0;
        int16_t lowerY1 = py0z1;
        int16_t lowerDx = abs(lineX1 - lineX0);
        int16_t lowerSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lowerDy = -abs(lowerY1 - lowerY0);
        int16_t lowerSy = lowerY0 < lowerY1 ? 1 : -1;
        int16_t lowerErr = lowerDx + lowerDy;  /* error value e_xy */
        int16_t lowerErr2 = 0;

        while (lineX0 != lineX1) {
            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
                vLine(lineX0, upperY0, lowerY0, shouldStippleFill);
                graphicsPut(lineX0, upperY0, shouldStippleBorder);
            }

            /* loop */
            upperErr2 = upperErr * 2;

            if (upperErr2 >= upperDy || lowerErr2 >= lowerDy) {
                upperErr += upperDy; /* e_xy+e_x > 0 */
                lowerErr += lowerDy; /* e_xy+e_x > 0 */
                lineX0 += lowerSx;
            }

            if (lineX0 >= XRES) {
                return;
            }

            if (upperErr2 <= upperDx) {
                /* e_xy+e_y < 0 */
                upperErr += upperDx;
                upperY0 += upperSy;
            }

            /* loop */
            lowerErr2 = lowerErr * 2;

            if (lowerErr2 <= lowerDx) {
                /* e_xy+e_y < 0 */
                lowerErr += lowerDx;
                lowerY0 += lowerSy;
            }
        }
    }


    if (elementMask & 2) {
        if (IN_RANGE(0, XRESMINUSONE, px0z0)) {
            vLine(px0z0, py0z0, py1z0, 0);
        }
    }

    if (elementMask & 1) {
        if (IN_RANGE(0, XRESMINUSONE, px1z1)) {
            vLine(px1z1, py0z1, py1z1, 0);
        }
    }
}

void drawSquare(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, uint16_t elementMask) {

    int16_t z0px;
    int16_t z0py;
    int16_t z0dx;

    int16_t px0z0;
    int16_t py0z0;
    int16_t px1z0;
    int16_t py1z0;

    int16_t x;

    uint16_t drawContour;

    if (z0 >= 32) {
        return;
    }

    z0px = (projections[z0].px);
    z0dx = ((projections[z0].dx));

    px0z0 = z0px - ((x0) * z0dx);
    px1z0 = px0z0 - (dX * z0dx);

    z0py = (projections[z0].py);

    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);

    if (px1z0 < 0 || px0z0 > XRESMINUSONE) {
        return;
    }

    uint16_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? (4 + STIPPLE_COLOUR_THRESHOLD) : 4;

    drawContour = (dY);

    /* Draw the horizontal outlines of z0 and z1 */
    /* Ceiling is lower than camera */
    if (drawContour) {
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x)) {
                vLine(x, py0z0, py1z0, shouldStipple);
            }
        }

        if (elementMask & 2) {
            if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0)) {
                vLine(px0z0, py0z0, py1z0, 0);
            }

            if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0)) {
                vLine(px1z0, py0z0, py1z0, 0);
            }
        }
    }
}


void drawObjectAt(int16_t x0, int16_t z0) {

    int16_t z1;
    uint16_t z0px;
    uint16_t z0py;
    uint16_t z1px;
    uint16_t z1py;
    int16_t z0dx;
    int16_t z1dx;

    int16_t px0z0;
    int16_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int16_t py0z1;
    int16_t px1z1;
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? (5 + STIPPLE_COLOUR_THRESHOLD) : 5;

    if (z0 >= 32 || z0 <= 4) {
        return;
    }

    z1 = z0 + 1;

    if (z1 >= 32) {
        return;
    }


    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);

    px1z0 = px0z0 - (1 * z0dx);
    px1z1 = px0z1 - (1 * z1dx);

    z1py = (projections[z1].py);
    z0py = (projections[z0].py);

    py0z0 = z0py + ((-cameraHeight) * z0dx);
    py0z1 = z1py + ((-cameraHeight) * z1dx);


    int16_t lineX0, lineX1;

    /* Draw the horizontal outlines of z0 and z1 */
    for (lineX0 = px0z0; lineX0 <= px1z0; ++lineX0) {
        if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
            graphicsPut(lineX0, py0z0, shouldStippleBorder);
        }
    }

    for (lineX0 = px0z1; lineX0 <= px1z1; ++lineX0) {
        if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
            graphicsPut(lineX0, py0z1, shouldStippleBorder);
        }
    }

    /* The left segment */
    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int16_t y0 = py0z0;
        int16_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int16_t sx = lineX0 < lineX1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int16_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;  /* error value e_xy */
        int16_t e2;

        while ((lineX0 != lineX1 || y0 != y1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
                graphicsPut(lineX0, y0, shouldStippleBorder);
            }

            /* loop */
            e2 = err << 2;

            if (e2 >= dy) {
                err += dy; /* e_xy+e_x > 0 */
                lineX0 += sx;
            }

            if (lineX0 >= XRES) {
                goto right_stroke;
            }

            if (e2 <= dx) {
                /* e_xy+e_y < 0 */
                err += dx;
                y0 += sy;
            }
        }
    }

    right_stroke:

    lineX0 = px1z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t y0 = py0z0;
        int16_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int16_t sx = lineX0 < lineX1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int16_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;  /* error value e_xy */
        int16_t e2;

        while ((lineX0 != lineX1 || y0 != y1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
                graphicsPut(lineX0, y0, shouldStippleBorder);
            }

            e2 = err << 2;

            if (e2 >= dy) {
                err += dy; /* e_xy+e_x > 0 */
                lineX0 += sx;
            }

            if (lineX0 >= XRES) {
                return;
            }

            if (e2 <= dx) {
                /* e_xy+e_y < 0 */
                err += dx;
                y0 += sy;
            }
        }
    }
}

void drawCubeAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, int16_t dZ, uint16_t elementMask) {

    int16_t y1 = y0 + dY;
    int16_t z1;

    int16_t z0px;
    int16_t z0py;
    int16_t z1px;
    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;
    int16_t py1z0;
    int16_t px0z0;
    int16_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int16_t py0z1;
    int16_t px1z1;

    uint16_t shouldStippleFill = (z0 >= STIPPLE_DISTANCE) ? (9 + STIPPLE_COLOUR_THRESHOLD) : 9;
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? (1 + STIPPLE_COLOUR_THRESHOLD) : 1;

    uint16_t drawContour;

    if (z0 >= 32 || z0 <= 4) {
        return;
    }

    z1 = z0 + dZ;

    if (z1 >= 32) {
        return;
    }

    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    z1py = (projections[z1].py);
    z0py = (projections[z0].py);

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);
    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);
    py0z0 = z0py + ((y0) * z0dx);
    py0z1 = z1py + ((y0) * z1dx);
    py1z0 = z0py + ((y1) * z0dx);
    drawContour = (dY);


    int16_t x, lineX0, lineX1;

    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int16_t lineY0 = py0z0;
        int16_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int16_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lineDy = -abs(lineY1 - lineY0);
        int16_t lineSy = lineY0 < lineY1 ? 1 : -1;
        int16_t lineErr = lineDx + lineDy;
        int16_t lineE2;

        while ((lineX0 != lineX1 || lineY0 != lineY1)) {

            if (drawContour) {
                if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
                    vLine(lineX0, lineY0, py1z0, 6);
                    graphicsPut(lineX0, lineY0, shouldStippleBorder);
                }
            }

            lineE2 = lineErr << 2;

            if (lineE2 >= lineDy) {
                lineErr += lineDy;
                lineX0 += lineSx;
            }

            if (lineX0 >= XRES) {
                goto right_stroke;
            }

            if (lineE2 <= lineDx) {
                lineErr += lineDx;
                lineY0 += lineSy;
            }
        }
    }

    right_stroke:

    lineX0 = px1z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t lineY0 = py0z0;
        int16_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int16_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lineDy = -abs(lineY1 - lineY0);
        int16_t lineSy = lineY0 < lineY1 ? 1 : -1;
        int16_t lineErr = lineDx + lineDy;
        int16_t lineE2;

        while ((lineX0 != lineX1 || lineY0 != lineY1)) {

            if (drawContour) {
                if (IN_RANGE(0, XRESMINUSONE, lineX0)) {
                    vLine(lineX0, lineY0, py1z0, 6);
                    graphicsPut(lineX0, lineY0, shouldStippleBorder);
                }
            }

            lineE2 = lineErr << 2;

            if (lineE2 >= lineDy) {
                lineErr += lineDy;
                lineX0 += lineSx;
            }

            if (lineX0 >= XRES) {
                goto final_stroke;
            }

            if (lineE2 <= lineDx) {

                lineErr += lineDx;
                lineY0 += lineSy;
            }
        }
    }

    final_stroke:

    if (drawContour) {
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x)) {
                vLine(x, py0z0, py1z0, shouldStippleFill);
                graphicsPut(x, py0z0, shouldStippleBorder);
            }
        }
    }

    if (IN_RANGE(0, XRESMINUSONE, px0z0)) {
        vLine(px0z0, py0z0, py1z0, 1);
    }

    if (IN_RANGE(0, XRESMINUSONE, px1z0)) {
        vLine(px1z0, py0z0, py1z0, 1);
    }
}

void drawFloorAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dZ, uint16_t elementMask) {

    int16_t z1;

    int16_t z0px;
    int16_t z0py;
    int16_t z1px;
    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;
    int16_t px0z0;
    int16_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int16_t py0z1;
    int16_t px1z1;

    uint16_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? (2 + STIPPLE_COLOUR_THRESHOLD) : 2;

    if (z0 >= 32 || z0 < 1) {
        return;
    }

    z1 = z0 + dZ;

    if (z1 >= 32) {
        return;
    }

    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    z1py = (projections[z1].py);
    z0py = (projections[z0].py);

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);
    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);
    py0z0 = z0py + ((y0) * z0dx);
    py0z1 = z1py + ((y0) * z1dx);


    int16_t leftX0, leftX1, rightX0, rightX1;

    leftX0 = px0z0;
    leftX1 = px0z1;
    rightX0 = px1z0;
    rightX1 = px1z1;
    int16_t dy = -abs(py0z1 - py0z0);
    int16_t sy = py0z0 < py0z1 ? 1 : -1;

    int16_t currentY0 = py0z0;
    int16_t leftDx = abs(leftX1 - leftX0);
    int16_t leftSx = leftX0 < leftX1 ? 1 : -1;
    int16_t leftErr = leftDx + dy;
    int16_t leftE2;

    int16_t rightDx = abs(rightX1 - rightX0);
    int16_t rightSx = rightX0 < rightX1 ? 1 : -1;
    int16_t rightErr = rightDx + dy;
    int16_t rightE2;

    while (currentY0 != py0z1) {
        if (leftX0 >= XRES) {
            return;
        }

        if (IN_RANGE(0, XRESMINUSONE, leftX0) || IN_RANGE(0, XRESMINUSONE, rightX0)) {
            hLine(leftX0, rightX0, currentY0, shouldStipple);
        }

        leftE2 = leftErr << 2;

        if (leftE2 >= dy) {
            leftErr += dy;
            leftX0 += leftSx;
        }

        rightE2 = rightErr << 2;

        if (rightE2 >= dy) {
            rightErr += dy;
            rightX0 += rightSx;
        }

        if (rightE2 <= rightDx || leftE2 <= leftDx) {
            rightErr += rightDx;
            leftErr += leftDx;
            currentY0 += sy;
        }
    }
}

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
                   prop->mCeilingRepetitions, 1, 0xFF);
    }

    if (prop->mFloorRepeatedTextureIndex != 0xFF && prop->mFloorRepetitions > 0) {
        drawCubeAt(x0 - 1, floorHeight - prop->mFloorRepetitions - cameraHeight, z + 2, x1 - x0,
                   prop->mFloorRepetitions, 1, 0xFF);
    }


    if (prop->mCeilingTextureIndex != 0xFF) {
        drawFloorAt(x0 - 1, ceilingHeight - cameraHeight, z + 2, x1 - x0,
                    1, 0xFF);
    }

    if (prop->mFloorTextureIndex != 0xFF) {
        drawFloorAt(x0 - 1, floorHeight - cameraHeight, z + 2, x1 - x0,
                    1, 0xFF);
    }


    if (type == kCube) {
        drawCubeAt(x0 - 1, floorHeight - cameraHeight, z + 2, x1 - x0,
                   diff, 1, 0xFF);
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
            case 1:
            case 3:
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

    //        drawObjectAt(x0 - 1, y + 2);
    switch (cameraRotation) {
        case 0:
            //drawPattern(lastPattern, lastIndex - cameraX + 2, x - cameraX + 2, cameraZ - y);
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(item->position.x - cameraX + 2 - 1, cameraZ - item->position.y + 2);
                node = node->next;
            }
            break;

        case 1:
            //drawPattern(lastPattern, (lastIndex - cameraZ) + 2 , (y - cameraZ) + 2, x - cameraX);
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt((item->position.y - cameraZ) + 1, (item->position.x - cameraX) + 2);
                node = node->next;
            }
            break;

        case 2:
            //drawPattern(lastPattern, -(x - cameraX) + 2, -(lastIndex - cameraX) + 2, y - cameraZ);
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(-(item->position.x - cameraX) + 1, (item->position.y - cameraZ) + 2);
                node = node->next;
            }
            break;

        case 3:
            //        drawPattern(lastPattern, -(y - cameraZ) + 2, -(lastIndex - cameraZ)  + 2, cameraX - x);
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt(-(item->position.y - cameraZ) + 1, (cameraX - item->position.x) + 2);
                node = node->next;
            }
            break;
    }
}

/* all those refactors are due to a SDCC bug with very long functions */
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

void pickItem(void) {
    struct Room *room = getRoom(getPlayerRoom());

    if (roomItem && roomItem->item) {
        struct Item *itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL) {

            if (!strcmp(itemToPick->name, "digital-safe")) {

#ifdef SUPPORTS_HACKING_MINIGAME
                runHackingMinigame();
#else
                accessGrantedToSafe = TRUE;
#endif
                return;
            }

            if (itemToPick->pickable) {

                uint16_t pattern = map[itemToPick->position.y][itemToPick->position.x];
                map[itemToPick->position.y][itemToPick->position.x] = pattern;

                pickObject(itemToPick);
                focusedItem = roomItem;
                roomItem = room->itemsPresent->next;

            } else {
                useObjectNamed(itemToPick->name);
            }
        }
    }
}

void dropItem(void) {

    struct Item *item = NULL;

    if (focusedItem != NULL) {
        item = getItem(focusedItem->item);
    }

    if (item != NULL) {
        uint16_t pattern;
        struct WorldPosition *pos = getPlayerPosition();

        dropObjectToRoom(getPlayerRoom(), item);

        focusedItem = getPlayerItems();

        roomItem = &objectNodes[item->index];

        switch (cameraRotation) {
            case 0:
                item->position.x = pos->x;
                item->position.y = pos->y - 3;
                break;

            case 1:
                item->position.x = pos->x + 3;
                item->position.y = pos->y;
                break;

            case 2:
                item->position.x = pos->x;
                item->position.y = pos->y + 3;
                break;

            case 3:
                item->position.x = pos->x - 3;
                item->position.y = pos->y;
                break;
        }


        pattern = map[item->position.y][item->position.x];
        map[item->position.y][item->position.x] = pattern | 128;
    }
}

void nextItemInRoom(void) {
    struct Room *room = getRoom(getPlayerRoom());

    if (roomItem == NULL) {
        return;
    }

    roomItem = roomItem->next;

    if (!roomItem) {
        roomItem = room->itemsPresent;
    }

    if (roomItem->item == 0) {
        roomItem = roomItem->next;
    }
}

void interactWithItemInRoom(void) {
    struct Item *item = NULL;
    struct Item *itemToPick = NULL;

    if (roomItem != NULL) {
        itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL) {
            if (focusedItem != NULL) {
                item = getItem(focusedItem->item);
                if (item != NULL) {
                    item->useWithCallback(item, itemToPick);
                }
            }
        }
    }
}

void useItemInHand(void) {
    useObjectNamed(getItem(focusedItem->item)->name);
}

void nextItemInHand(void) {
    focusedItem = focusedItem->next;

    if (!focusedItem) {
        focusedItem = getPlayerItems();
    }
}

void updateMapItems();

void initMap(void) {
    int x, y;
    const uint8_t *head;
    char buffer[32];
    uint16_t current;

    roomItem = getRoom(getPlayerRoom())->itemsPresent->next;

    sprintf(&buffer[0], "map%d.txt", getPlayerRoom());

    struct StaticBuffer datafile = loadBinaryFileFromPath(&buffer[0]);
    head = datafile.data;

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {

            current = *head;


            if ((current == 's' && enteredFrom == 0) ||
                (current == 'w' && enteredFrom == 1) ||
                (current == 'n' && enteredFrom == 2) ||
                (current == 'e' && enteredFrom == 3)) {

                struct WorldPosition newPos;
                cameraX = x;
                cameraZ = y;
                newPos.x = x;
                newPos.y = y;
                setPlayerPosition(&newPos);
                enteredFrom = 0xFF;
                current = '.';
            }

            map[y][x] = current;
            ++head;
        }
        ++head; // line break
    }

    disposeDiskBuffer(datafile);

    sprintf(&buffer[0], "props%d.bin", getPlayerRoom());
    loadPropertyList(&buffer[0], &tileProperties);

    //updateMapItems();
    HUD_initialPaint();
}

void startRoomTransitionAnimation(void) {
    for (uint16_t y = 32; y >= 2; --y) {
        clearGraphics();

        vLine(y, y, 95 + (32 - y), 1);
        vLine(95 + (32 - y), y, 95 + (32 - y), 1);

        for (uint16_t x = y; x < (95 + (32 - y)); ++x) {
            graphicsPut(x, y, 7);
            graphicsPut(x, 95 + (32 - y), 7);

            //door opening
            vLine(x, y, 95 - 3 * (32 - y), 7);
        }


        graphicsFlush();
        sleepForMS(20000);
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

void tickRenderer(void) {
    uint16_t prevX;
    uint16_t prevZ;
    struct WorldPosition *pos;
    int previousLocation = playerLocation;
    uint16_t newCell = 0;

    renderScene();

    graphicsFlush();

    prevX = cameraX;
    prevZ = cameraZ;

    waitkey:
    switch (getKey()) {
#ifndef SMD
        case 'l':
            shutdownGraphics();
            exit(0);
#endif
        case 'q':
            turnLeft();
            break;

        case 'e':
            turnRight();
            break;

        case 'a':
            walkBy(3);
            break;
        case 'd':
            walkBy(1);
            break;
        case 's':
            walkBy(2);
            break;
        case 'w':
            walkBy(0);
            break;

        case '7':
            nextItemInHand();
            HUD_refresh();
            break;

        case '4':
            nextItemInRoom();
            HUD_refresh();
            break;

        case '8':
            useItemInHand();
            updateMapItems();
            HUD_refresh();
            break;

        case '5':
            interactWithItemInRoom();
            updateMapItems();
            HUD_refresh();
            break;

        case '9':
            pickItem();
            HUD_refresh();
            break;

        case '6':
            dropItem();
            HUD_refresh();
            break;

#if !defined(SDLSW)
            case 'p':
            default:
                goto waitkey;
#endif
    }

    cameraRotation = getPlayerDirection();
    pos = getPlayerPosition();

    cameraX = pos->x;
    cameraZ = pos->y;

    switch (cameraRotation) {
        case 0:
            newCell = map[cameraZ - 2][cameraX];
            break;
        case 1:
            newCell = map[cameraZ][cameraX + 2];
            break;
        case 2:
            newCell = map[cameraZ + 2][cameraX];
            break;
        case 3:
            newCell = map[cameraZ][cameraX - 2];
            break;
    }

    struct CTile3DProperties *prop =
            (struct CTile3DProperties *) getFromMap(&tileProperties, newCell);


    if (prop->mBlockMovement) {
        pos->x = cameraX = prevX;
        pos->y = cameraZ = prevZ;
        setPlayerPosition(pos);
    }

    /* unlike MX, we are signaling from the origin into the new room. MX allows for the movement and then searches where
     * did the player came from - hence the "opposite direction" there */

    if (newCell > ('0' - 1) && newCell < ('3' + 1)) {
        enteredFrom = newCell - '0';
        moveBy(enteredFrom);
    }

    if (playerLocation != previousLocation) {
        initMap();

        if (newCell == '.') {
            newCell = '0';
        } else {
            startRoomTransitionAnimation();
        }

        setPlayerDirection(cameraRotation = (newCell - '0'));
    } else {
        enteredFrom = 0xFF;
    }
}

void onError(const char *mesg) {
    puts(mesg);
}

void logDelegate(const char *mesg) {
    showMessage(mesg);
}

#ifdef ATARIST
int doMain(void);

int main(int argc, char **argv) {
    Supexec(&doMain);
    return 0;
}

int doMain(void) {
#else

int main(int argc, char **argv) {
#endif

    for (int c = 0; c < 32; ++c) {
        map[c] = (uint8_t *) allocMem(32, GENERAL_MEMORY, 1);
    }

    running = 1;
    enteredFrom = 0;
    cameraRotation = 0;
    initFileReader("base.pfs");
    init();
    initStation();
    titleScreen();

    focusedItem = getPlayerItems();
    setErrorHandlerCallback(onError);
    setLoggerDelegate(logDelegate);
    initMap();

    do {
        tickRenderer();
    } while (running);

    shutdownGraphics();

    return 0;
}
