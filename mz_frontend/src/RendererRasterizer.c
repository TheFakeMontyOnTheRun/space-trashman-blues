#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#include <stdlib.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "FixP.h"
#include "CTile3DProperties.h"

#ifdef MSDOS
#include "Common.h"
#endif

extern int16_t cameraHeight;

struct Projection {
    uint8_t px;
    uint8_t py;
    int8_t dx;
};

const struct Projection projections[32] =
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

#ifdef USE_OWN_MIN_MAX
int8_t max(int8_t x1, int8_t x2);

int8_t min(int8_t x1, int8_t x2);
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

    uint16_t shouldStippleFill = (z0 >= STIPPLE_DISTANCE) ? 6 : (6 + STIPPLE_COLOUR_THRESHOLD);
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? 0 : (0 + STIPPLE_COLOUR_THRESHOLD);

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
            vLine(px0z0, py0z0, py1z0, shouldStippleBorder);
        }
    }

    if (elementMask & 1) {
        if (IN_RANGE(0, XRESMINUSONE, px1z1)) {
            vLine(px1z1, py0z1, py1z1, shouldStippleBorder);
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

    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? 0 : (0 + STIPPLE_COLOUR_THRESHOLD);
    uint16_t shouldStippleFill = (z0 >= STIPPLE_DISTANCE) ? 4: (4 + STIPPLE_COLOUR_THRESHOLD);

    drawContour = (dY);

    /* Draw the horizontal outlines of z0 and z1 */
    /* Ceiling is lower than camera */
    if (drawContour) {
        if (elementMask & 2) {
            if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0)) {
                vLine(px0z0, py0z0, py1z0, shouldStippleBorder);
            }

            if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0)) {
                vLine(px1z0, py0z0, py1z0, shouldStippleBorder);
            }
        }

        for (x = px0z0; x < px1z0; ++x) {
            if ( x < 0 ) {
                continue;
            }

            if (x >= XRES ) {
                return;
            }

            vLine(x, py0z0, py1z0, shouldStippleFill);
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
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? 0 : (0 + STIPPLE_COLOUR_THRESHOLD);

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



void drawCubeAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, int16_t dZ) {

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

    uint16_t shouldStippleFill = (z0 >= STIPPLE_DISTANCE) ? 7 : (7 + STIPPLE_COLOUR_THRESHOLD);
    uint16_t shouldStippleBorder = (z0 >= STIPPLE_DISTANCE) ? 0 : (0 + STIPPLE_COLOUR_THRESHOLD);

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
                    vLine(lineX0, lineY0, py1z0, shouldStippleFill);
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
                    vLine(lineX0, lineY0, py1z0, shouldStippleFill);
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
        vLine(px0z0, py0z0, py1z0, shouldStippleBorder);
    }

    if (IN_RANGE(0, XRESMINUSONE, px1z0)) {
        vLine(px1z0, py0z0, py1z0, shouldStippleBorder);
    }
}



void drawFloorAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dZ) {

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

    uint16_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 2 : (2 + STIPPLE_COLOUR_THRESHOLD);

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

        if (rightX1 < 0) {
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

