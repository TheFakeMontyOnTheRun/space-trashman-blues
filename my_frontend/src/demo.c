#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
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

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

#define STIPPLE_DISTANCE 13
#define RLE_THRESHOLD 32
#define MAP_SIZE_X 32
#define MAP_SIZE_Y 32
#define VISIBILITY_LIMIT_X (MAP_SIZE_X - 1)
#define VISIBILITY_LIMIT_Y (MAP_SIZE_Y - 1)
#define FAR_PLANE_Z 32
#define NEAR_PLANE_Z 6
#define RENDER_SCALE_X 1
#define RENDER_SCALE_Z 1

/*  Not rendered, but won't block visibility */
#define NEUTRAL_CELL '.'

/*  not rendered and blocks visibility */
#define BLOCK_CELL '#'

/*  used to mark the edge between the neutral cells and the walls.
    doesn't need to be used, but I'm leaving this here for posterity. */
#define BORDER_CELL '_'

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

struct ObjectNode *focusedItem = NULL;
struct ObjectNode *roomItem = NULL;

extern uint8_t accessGrantedToSafe;

void performAction(void);

void startMusic(void);

void renderCameraNorth(void);

void renderCameraEast(void);

void renderCameraSouth(void);

void renderCameraWest(void);

int8_t stencilHigh[XRES];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

uint8_t enteredFrom = 0xFF;

extern uint8_t playerLocation;

struct Projection {
    uint8_t px;
    int8_t dx;
};

const struct Projection projections[32] = {
        {0,  -127},    /*	1	*/
        {0,  -64},    /*	2	*/
        {21, -43},    /*	3	*/
        {32, -32},    /*	4	*/
        {38, -26},    /*	5	*/
        {42, -22},    /*	6	*/
        {45, -19},    /*	7	*/
        {48, -16},    /*	8	*/
        {49, -15},    /*	9	*/
        {51, -13},    /*	10	*/
        {52, -12},    /*	11	*/
        {53, -11},    /*	12	*/
        {54, -10},    /*	13	*/
        {54, -10},    /*	14	*/
        {55, -9},    /*	15	*/
        {56, -8},    /*	16	*/
        {56, -8},    /*	17	*/
        {56, -8},    /*	18	*/
        {57, -7},    /*	19	*/
        {57, -7},    /*	20	*/
        {57, -7},    /*	21	*/
        {58, -6},    /*	22	*/
        {58, -6},    /*	23	*/
        {58, -6},    /*	24	*/
        {58, -6},    /*	25	*/
        {59, -5},    /*	26	*/
        {59, -5},    /*	27	*/
        {59, -5},    /*	28	*/
        {59, -5},    /*	29	*/
        {59, -5},    /*	30	*/
        {59, -5},    /*	31	*/
        {60, -4},    /*	32	*/
};

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}

uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type) {

    int8_t z1;
    uint8_t z0px;
    uint8_t z1px;
    uint8_t z0py;

    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    int16_t lineX0;
    int16_t lineX1;

    int16_t px0z0;
    int16_t py0z0;
    int16_t py0z1;

    int16_t py1z0;
    int16_t py1z1;

    int16_t px1z1;

    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    if (z0 >= FAR_PLANE_Z) {
        return 0;
    }

    if (z0 < NEAR_PLANE_Z) {
        return 0;
    }

    z1 = z0 + dZ;

    if (z1 < NEAR_PLANE_Z) {
        return 0;
    }

    if (z1 >= FAR_PLANE_Z) {
        return 0;
    }


    if (type == RIGHT_NEAR) {
        z0px = (projections[z0].px);
        z1px = (projections[z1].px);
        z0dx = ((projections[z0].dx));
        z1dx = ((projections[z1].dx));

        px0z0 = z0px - (x0 * z0dx);
        px1z1 = z1px - ((dX + x0) * z1dx);

        z1py = (projections[z1].px);
        z0py = (projections[z0].px);

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
        px1z1 = z1px - ((x0 + dX) * z1dx); /* extra operations to avoid overflow */

        z1py = (projections[z0].px);
        z0py = (projections[z1].px);

        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);
        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
    }

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

    if (elementMask & 2) {
        if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
            vLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]), shouldStipple);
        }
    }

    if (elementMask & 1) {
        if (IN_RANGE(0, XRESMINUSONE, px1z1) && py0z1 > stencilHigh[px1z1]) {
            vLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]), shouldStipple);
        }
    }

    /* The upper segment */
    lineX0 = px0z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t upperY0 = py1z0;
        int16_t upperY1 = py1z1;
        int16_t upperDx = abs(lineX1 - lineX0);
        int16_t upperDy = -abs(upperY1 - upperY0);
        int8_t upperSy = upperY0 < upperY1 ? 1 : -1;
        int16_t upperErr = upperDx + upperDy;
        int16_t upperErr2;
        int16_t lowerY0 = py0z0;
        int16_t lowerY1 = py0z1;
        int16_t lowerDx = abs(lineX1 - lineX0);
        int8_t lowerSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lowerDy = -abs(lowerY1 - lowerY0);
        int8_t lowerSy = lowerY0 < lowerY1 ? 1 : -1;
        int16_t lowerErr = lowerDx + lowerDy;
        int16_t lowerErr2 = 0;

        while (lineX0 != lineX1) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {

                stipple = ~stipple;

                if ((stipple || !shouldStipple) && stencilHigh[lineX0] <= upperY0) {
                    graphicsPut(lineX0, upperY0);
                }

                if (stencilHigh[lineX0] < lowerY0) {
                    stencilHigh[lineX0] = lowerY0;
                }
            }

            upperErr2 = upperErr * 2;

            if (upperErr2 >= upperDy || lowerErr2 >= lowerDy) {
                upperErr += upperDy;
                lowerErr += lowerDy;
                lineX0 += lowerSx;
            }

            if (lineX0 >= XRES) {
                return 0;
            }

            if (upperErr2 <= upperDx) {
                upperErr += upperDx;
                upperY0 += upperSy;
            }

            lowerErr2 = lowerErr * 2;

            if (lowerErr2 <= lowerDx) {
                lowerErr += lowerDx;
                lowerY0 += lowerSy;
            }
        }
    }

    return 1;
}

uint8_t drawSquare(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, uint8_t elementMask) {

    uint8_t z0px;
    uint8_t z0py;
    int8_t z0dx;
    int16_t x;
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    uint8_t shouldStipple;
    uint8_t drawContour;
    uint8_t stipple;

    if (z0 >= FAR_PLANE_Z) {
        return 0;
    }

    z0px = (projections[z0].px);
    z0dx = ((projections[z0].dx));

    px0z0 = z0px - ((x0) * z0dx);

    px1z0 = px0z0 - (dX * z0dx);

    z0py = (projections[z0].px);

    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);

    if (px1z0 < 0 || px0z0 > XRESMINUSONE) {
        return 0;
    }

    shouldStipple = (z0 >= STIPPLE_DISTANCE);
    stipple = 1;

    drawContour = (dY);

    if (drawContour) {
        if (elementMask & 2) {
            if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                vLine(px0z0, py0z0, stencilHigh[px0z0] < py1z0 ? py1z0 : stencilHigh[px0z0], shouldStipple);
            }

            if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                vLine(px1z0, py0z0, stencilHigh[px1z0] < py1z0 ? py1z0 : stencilHigh[px1z0], shouldStipple);
            }
        }
    }

    /* Draw the horizontal outlines of z0 and z1 */

    /* Ceiling is lower than camera */
    for (x = px0z0; x <= px1z0; ++x) {
        if (IN_RANGE(0, XRESMINUSONE, x)) {

            stipple = ~stipple;

            if (stencilHigh[x] <= py1z0) {
                if (drawContour && (stipple || !shouldStipple)) {
                    graphicsPut(x, py1z0);
                }
                stencilHigh[x] = py1z0;
            }


            if (stencilHigh[x] <= py0z0) {
                stencilHigh[x] = py0z0;
            }
        }
    }


    return 1;
}


uint8_t drawObjectAt(int8_t x0, int8_t z0) {

    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    int16_t x;
    int16_t lineX0;
    int16_t lineX1;
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    z0 = z0 * RENDER_SCALE_Z;

    if (z0 >= FAR_PLANE_Z || z0 < NEAR_PLANE_Z) {
        return 0;
    }

    z1 = z0 + RENDER_SCALE_Z;

    if (z1 >= FAR_PLANE_Z || z1 < NEAR_PLANE_Z) {
        return 0;
    }

    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);

    px1z0 = px0z0 - (RENDER_SCALE_X * z0dx);
    px1z1 = px0z1 - (RENDER_SCALE_X * z1dx);

    z1py = (projections[z1].px);
    z0py = (projections[z0].px);

    py0z0 = z0py + ((-CAMERA_HEIGHT) * z0dx);
    py0z1 = z1py + ((-CAMERA_HEIGHT) * z1dx);

    /* Draw the horizontal outlines of z0 and z1 */

    for (x = px0z0; x <= px1z0; ++x) {
        if (IN_RANGE(0, XRESMINUSONE, x)) {

            stipple = !stipple;

            if ((stipple || !shouldStipple) && stencilHigh[x] < py0z0) {
                graphicsPut(x, py0z0);
            }
        }
    }

    for (x = px0z1; x <= px1z1; ++x) {
        if (IN_RANGE(0, XRESMINUSONE, x)) {

            stipple = ~stipple;

            if ((stipple || !shouldStipple) && stencilHigh[x] < py0z1) {
                graphicsPut(x, py0z1);
            }
        }
    }

    /* The left segment */
    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int16_t y0 = py0z0;
        int16_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int8_t sx = lineX0 < lineX1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;
        int16_t e2 = 0;

        while ((lineX0 != lineX1 || y0 != y1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {

                stipple = ~stipple;

                if ((stipple || !shouldStipple) && stencilHigh[lineX0] < y0) {
                    graphicsPut(lineX0, y0);
                }
            }

            e2 = err << 2;

            if (e2 >= dy) {
                err += dy;
                lineX0 += sx;
            }

            if (lineX0 >= XRES) {
                goto right_stroke;
            }

            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    right_stroke:

    /* The right segment */
    lineX0 = px1z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t y0 = py0z0;
        int16_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int8_t sx = lineX0 < lineX1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;
        int16_t e2 = 0;

        while ((lineX0 != lineX1 || y0 != y1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0) && stencilHigh[lineX0] < y0) {
                stipple = ~stipple;

                if ((stipple || !shouldStipple)) {
                    graphicsPut(lineX0, y0);
                }
            }

            e2 = err << 2;

            if (e2 >= dy) {
                err += dy;
                lineX0 += sx;
            }

            if (lineX0 >= XRES) {
                return 1;
            }

            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }


    return 1;
}

uint8_t drawCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask) {

    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    int16_t x;
    int16_t lineX0;
    int16_t lineX1;

    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;

    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    uint8_t drawContour;

    if (z0 >= FAR_PLANE_Z || z0 < NEAR_PLANE_Z) {
        return 0;
    }

    z1 = z0 + dZ;

    if (z1 >= FAR_PLANE_Z) {
        return 0;
    }


    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));

    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);

    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);

    z1py = (projections[z1].px);
    z0py = (projections[z0].px);

    py0z0 = z0py + ((y0) * z0dx);
    py0z1 = z1py + ((y0) * z1dx);

    drawContour = (dY);

    if (drawContour) {
        if (elementMask & 2) {
            if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                vLine(px0z0, py0z0, stencilHigh[px0z0], shouldStipple);
            }

            if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                vLine(px1z0, py0z0, stencilHigh[px1z0], shouldStipple);
            }
        }

        if (elementMask & 1) {
            if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z1) && px0z1 < px0z0 &&
                py0z1 > stencilHigh[px0z1]) {
                vLine(px0z1, py0z1, stencilHigh[px0z1], shouldStipple);
            }

            if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z1) && px1z1 > px1z0 &&
                py0z1 > stencilHigh[px1z1]) {
                vLine(px1z1, py0z1, stencilHigh[px1z1], shouldStipple);
            }
        }
    }

    /* Draw the horizontal outlines of z0 and z1 */

    if (py0z0 > py0z1) {
        /* Ceiling is lower than camera */
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {

                stipple = ~stipple;

                if (drawContour && (stipple || !shouldStipple)) {
                    graphicsPut(x, stencilHigh[x]);
                }
                stencilHigh[x] = py0z0;
            }
        }
    } else if (drawContour) {
        /* Ceiling is higher than the camera */
        /* Let's just draw the nearer segment */
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {

                stipple = ~stipple;

                if ((stipple || !shouldStipple)) {
                    graphicsPut(x, stencilHigh[x]);
                }
            }
        }
    }

    /* The left segment */
    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int16_t lineY0 = py0z0;
        int16_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int8_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lineDy = -abs(lineY1 - lineY0);
        int8_t lineSy = lineY0 < lineY1 ? 1 : -1;
        int16_t lineErr = lineDx + lineDy;
        int16_t lineE2 = 0;

        while ((lineX0 != lineX1 || lineY0 != lineY1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0)) {

                stipple = ~stipple;

                if (stencilHigh[lineX0] < lineY0) {
                    if (drawContour && (stipple || !shouldStipple)) {
                        graphicsPut(lineX0, stencilHigh[lineX0]);
                    }
                    stencilHigh[lineX0] = lineY0;
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
    /* The right segment */
    lineX0 = px1z0;
    lineX1 = px1z1;

    if (lineX0 != lineX1) {
        int16_t lineY0 = py0z0;
        int16_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int8_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int16_t lineDy = -abs(lineY1 - lineY0);
        int8_t lineSy = lineY0 < lineY1 ? 1 : -1;
        int16_t lineErr = lineDx + lineDy;
        int16_t lineE2 = 0;

        while ((lineX0 != lineX1 || lineY0 != lineY1)) {

            if (IN_RANGE(0, XRESMINUSONE, lineX0) && stencilHigh[lineX0] < lineY0) {

                stipple = ~stipple;

                if (drawContour && (stipple || !shouldStipple)) {
                    graphicsPut(lineX0, stencilHigh[lineX0]);
                }
                stencilHigh[lineX0] = lineY0;
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

    if (py0z0 <= py0z1) {
        /* Ceiling is higher than the camera */
        /* Draw the last segment */
        if (drawContour) {
            for (x = px0z1; x <= px1z1; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x)) {

                    int8_t stencilY = stencilHigh[x];

                    if (stencilY < py0z0) {
                        graphicsPut(x, py0z0);
                    }

                    if (stencilY < py0z1) {
                        stencilHigh[x] = py0z1;
                    }
                }
            }
        } else {
            for (x = px0z1; x <= px1z1; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z1) {
                    stencilHigh[x] = py0z1;
                }
            }
        }
    }


    return 1;
}


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

/* all those refactors are due to a SDCC bug with very long functions */
void renderScene(void) {
    uint8_t x;
    int8_t *stencilPtr;
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

#ifdef SMS
    stencilPtr = &stencilHigh[0];

    for (x = 0; x < XRESMINUSONE;) {
      uint8_t y, prevY, c;
        uint8_t *ptr;
next_cluster:
        /* pixel 1 */
        y = *stencilPtr;
        ptr = graphicsPutAddr(x, y, NULL );

        if (x & 7) {
            ++x;
            ++stencilPtr;
            continue;
        }

        for (c = 2; c < 8; ++c ) {
            ++x;
            ++stencilPtr;
            prevY = y;
            y = *stencilPtr;
            if ( y != prevY ) {
                goto next_cluster;
            }
            ptr = graphicsPutAddr(x, y, ptr );
        }

        ++x;
        ++stencilPtr;
    }
#else
#ifdef CPC
    stencilPtr = &stencilHigh[0];

    for (x = 0; x < XRESMINUSONE;) {
      uint8_t y, prevY, c;
        uint8_t *ptr;
next_cluster:
        /* pixel 1 */
        y = *stencilPtr;
        ptr = graphicsPutAddr(x, y, NULL );

        if (x & 3) {
            ++x;
            ++stencilPtr;
            continue;
        }

        for (c = 2; c < 4; ++c ) {
            ++x;
            ++stencilPtr;
            prevY = y;
            y = *stencilPtr;
            if ( y != prevY ) {
                goto next_cluster;
            }
            ptr = graphicsPutAddr(x, y, ptr );
        }

        ++x;
        ++stencilPtr;
    }
#else
#ifdef MSDOS
    stencilPtr = &stencilHigh[0];

    for (x = 0; x < XRESMINUSONE;) {
      uint8_t y, prevY, c;
        uint8_t *ptr;
next_cluster:
        /* pixel 1 */
        y = *stencilPtr;
        ptr = graphicsPutAddr(x, y, NULL );

        if (x & 3) {
            ++x;
            ++stencilPtr;
            continue;
        }

        for (c = 2; c < 4; ++c ) {
            ++x;
            ++stencilPtr;
            prevY = y;
            y = *stencilPtr;
            if ( y != prevY ) {
                goto next_cluster;
            }
            ptr = graphicsPutAddr(x, y, ptr );
        }

        ++x;
        ++stencilPtr;
    }
#else
    stencilPtr = &stencilHigh[0];

    for (x = 0; x < XRES; ++x) {
        int8_t stencilY = (*stencilPtr);
        graphicsPut(x, stencilY);
        ++stencilPtr;
    }
#endif
#endif
#endif

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

                uint8_t pattern = map[itemToPick->position.y][itemToPick->position.x];
                map[itemToPick->position.y][itemToPick->position.x] = pattern & 127;

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
        uint8_t pattern;
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

void initMap(void) {
    uint8_t x, y, c;
    const uint8_t *head;
    const uint8_t *headEnd;
    uint8_t current = NEUTRAL_CELL;

    uint16_t offsetOnDataStrip = 0;
    int16_t repetitions = -1;

#ifdef EMBEDDED_DATA
    for (c = 0; c < playerLocation; ++c) {
        offsetOnDataStrip += dataPositions[c];
    }

    head = &data[offsetOnDataStrip];

    /* the last location */
    if (dataPositions[playerLocation + 1] == 0) {
        size_t extra = sizeof(data) - 1;
        headEnd = &data[0] + extra;
    } else {
        headEnd = head + (dataPositions[playerLocation]);
    }
#else
    struct StaticBuffer datafile = loadBinaryFileFromPath(playerLocation);
    head = datafile.data;
#endif
    /* first item in the list is always a dummy */
    roomItem = getRoom(playerLocation)->itemsPresent->next;

#ifdef OPTIMIZATION_BLOCK_CELL
    memset(map, BLOCK_CELL, MAP_SIZE_X * MAP_SIZE_Y);
#else
    memset(map, NEUTRAL_CELL, MAP_SIZE_X * MAP_SIZE_Y);
#endif
    for (y = 0; y < MAP_SIZE_Y; ++y) {
        for (x = 0; x < MAP_SIZE_X; ++x) {

#ifdef RLE_COMPRESSED_MAPS
            if (head == headEnd) {
                goto done_loading;
            }

            if (repetitions < 1) {
                repetitions = *head;

                if (repetitions >= RLE_THRESHOLD) {
                    ++head;
                    current = repetitions;
                    repetitions = 0;
                } else {
                    ++head;
                    current = *head;
                    ++head;
                    repetitions--;
                }
            } else {
                repetitions--;
            }
#else
            current = *head;
#endif

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
                current = NEUTRAL_CELL;
            }

            map[y][x] = current;
#ifndef EMBEDDED_DATA
            ++head;
#endif
        }
#ifndef EMBEDDED_DATA
        ++head; /*  line break */
#endif
    }

    done_loading:
#ifndef EMBEDDED_DATA
    disposeDiskBuffer(datafile);
#endif

    updateMapItems();
    HUD_initialPaint();
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
        graphicsFlush();
        sleepForMS(20000);
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

void tickRenderer(void) {
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    uint8_t previousLocation = playerLocation;
    uint8_t newCell = 0;
    renderScene();
    graphicsFlush();

    prevX = cameraX;
    prevZ = cameraZ;

    waitkey:
    switch (getKey()) {

#ifndef GAMEPAD
        case 'l':
            shutdownGraphics();
            exit(0);
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

    newCell = newCell & 127;

    if (patterns[newCell - RLE_THRESHOLD].blockMovement) {
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

        if (newCell == NEUTRAL_CELL) {
            newCell = '0';
#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
        } else {
            startRoomTransitionAnimation();
#endif
        }
        setPlayerDirection(cameraRotation = (newCell - '0'));
    } else {
        enteredFrom = 0xFF;
    }
}


void onError(const char *mesg) {
    showMessage(mesg);
}

void logDelegate(const char *mesg) {
    showMessage(mesg);
}


int main(int argc, char **argv) {

    running = 1;
    enteredFrom = 0;
    cameraRotation = 0;
    init();
    initStation();
    focusedItem = getPlayerItems();
    setErrorHandlerCallback(onError);
    setLoggerDelegate(logDelegate);

    memset(stencilHigh, 0, XRES);
    titleScreen();

    initMap();

    do {
        tickRenderer();
    } while (running);

    shutdownGraphics();

    return 0;
}
