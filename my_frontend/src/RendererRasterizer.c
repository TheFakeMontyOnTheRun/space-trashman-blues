#include <stdlib.h>
#include <stdint.h>

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "Common.h"

#ifdef MSDOS
#include "Common.h"
#endif

int8_t stencilHigh[XRES];

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

#ifndef SPECCY
int8_t max(int8_t x1, int8_t x2);

int8_t min(int8_t x1, int8_t x2);
#endif

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
    int16_t px1z1;

    int8_t py0z0;
    int8_t py0z1;
    int8_t py1z0;
    int8_t py1z1;

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
        int8_t upperY0 = py1z0;
        int8_t upperY1 = py1z1;
        int8_t upperDx = abs(lineX1 - lineX0);
        int8_t upperDy = -abs(upperY1 - upperY0);
        int8_t upperSy = upperY0 < upperY1 ? 1 : -1;
        int16_t upperErr = upperDx + upperDy;
        int16_t upperErr2 = 0;
        int8_t lowerY0 = py0z0;
        int8_t lowerY1 = py0z1;
        int16_t lowerDx = abs(lineX1 - lineX0);
        int8_t lowerSx = lineX0 < lineX1 ? 1 : -1;
        int8_t lowerDy = -abs(lowerY1 - lowerY0);
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
    int16_t px1z0;

    int8_t py0z0;
    int8_t py1z0;
    uint8_t shouldStipple;
    uint8_t drawContour;

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

    if (px0z0 < 0) {
        px0z0 = 0;
    }

    if (px1z0 >= XRES) {
        px1z0 = XRESMINUSONE;
    }

    for (x = px0z0; x <= px1z0; ++x) {
        if (stencilHigh[x] <= py1z0) {
            if (drawContour && ((x & 1) || !shouldStipple)) {
                graphicsPut(x, py1z0);
            }
            stencilHigh[x] = py1z0;
        }

        if (stencilHigh[x] <= py0z0) {
            stencilHigh[x] = py0z0;
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
    int16_t px1z0;
    int16_t px0z1;
    int16_t px1z1;

    int8_t py0z0;
    int8_t py0z1;

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
    if (shouldStipple) {
        for (x = px0z0; x <= px1z0; x += 2) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                graphicsPut(x, py0z0);
            }
        }
        for (x = px0z1; x <= px1z1; x += 2) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z1) {
                graphicsPut(x, py0z1);
            }
        }
    } else {
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                graphicsPut(x, py0z0);
            }
        }
        for (x = px0z1; x <= px1z1; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z1) {
                graphicsPut(x, py0z1);
            }
        }
    }

    /* The left segment */
    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int8_t y0 = py0z0;
        int8_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int8_t sx = lineX0 < lineX1 ? 1 : -1;
        int8_t dy = -abs(y1 - y0);
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
        int8_t y0 = py0z0;
        int8_t y1 = py0z1;
        int16_t dx = abs(lineX1 - lineX0);
        int8_t sx = lineX0 < lineX1 ? 1 : -1;
        int8_t dy = -abs(y1 - y0);
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
    int16_t px1z0;
    int16_t px0z1;
    int16_t px1z1;

    int8_t py0z0;
    int8_t py0z1;

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
        if (shouldStipple) {
            for (x = px0z0; x <= px1z0; x += 2) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
                        graphicsPut(x, stencilHigh[x]);
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
                        graphicsPut(x, stencilHigh[x]);
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        }
    } else if (drawContour) {
        /* Ceiling is higher than the camera */
        /* Let's just draw the nearer segment */
        if (shouldStipple) {
            for (x = px0z0; x <= px1z0; x += 2) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    graphicsPut(x, stencilHigh[x]);
                }
            }
        } else {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    graphicsPut(x, stencilHigh[x]);
                }
            }
        }
    }

    /* The left segment */
    lineX0 = px0z0;
    lineX1 = px0z1;

    if (lineX0 != lineX1) {
        int8_t lineY0 = py0z0;
        int8_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int8_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int8_t lineDy = -abs(lineY1 - lineY0);
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
        int8_t lineY0 = py0z0;
        int8_t lineY1 = py0z1;
        int16_t lineDx = abs(lineX1 - lineX0);
        int8_t lineSx = lineX0 < lineX1 ? 1 : -1;
        int8_t lineDy = -abs(lineY1 - lineY0);
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
