#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PackedFileReader.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "map.h"

#ifdef SUPPORTS_HACKING_MINIGAME
#include "HackingMinigame.h"
#endif

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

#define STIPPLE_DISTANCE 13

struct ObjectNode* focusedItem = NULL;
struct ObjectNode* roomItem = NULL;

extern int accessGrantedToSafe;

void performAction();

void startMusic();

void renderCameraNorth();

void renderCameraEast();

void renderCameraSouth();

void renderCameraWest();

int8_t stencilHigh[XRES];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

uint8_t enteredFrom = 0xFF;

extern int playerLocation;

struct Projection {
    uint8_t px;
    uint8_t py;
    int8_t dx;
};

const struct Projection projections[40] =
        {
#ifdef RES128X64
                {	0	,	64	,	-128	},	//	1
                {	0	,	63	,	-64	},	//	2
                {	20	,	52	,	-42	},	//	3
                {	31	,	47	,	-32	},	//	4
                {	37	,	43	,	-25	},	//	5
                {	41	,	41	,	-21	},	//	6
                {	44	,	40	,	-18	},	//	7
                {	47	,	39	,	-16	},	//	8
                {	48	,	38	,	-14	},	//	9
                {	50	,	37	,	-12	},	//	10
                {	51	,	36	,	-11	},	//	11
                {	52	,	36	,	-10	},	//	12
                {	53	,	35	,	-9	},	//	13
                {	53	,	35	,	-9	},	//	14
                {	54	,	35	,	-8	},	//	15
                {	55	,	35	,	-8	},	//	16
                {	55	,	34	,	-7	},	//	17
                {	55	,	34	,	-7	},	//	18
                {	56	,	34	,	-6	},	//	19
                {	56	,	34	,	-6	},	//	20
                {	56	,	34	,	-6	},	//	21
                {	57	,	33	,	-5	},	//	22
                {	57	,	33	,	-5	},	//	23
                {	57	,	33	,	-5	},	//	24
                {	57	,	33	,	-5	},	//	25
                {	58	,	33	,	-4	},	//	26
                {	58	,	33	,	-4	},	//	27
                {	58	,	33	,	-4	},	//	28
                {	58	,	33	,	-4	},	//	29
                {	58	,	33	,	-4	},	//	30
                {	58	,	33	,	-4	},	//	31
#else
#ifdef RES96x64
{	0	,	64	,	-96	},	//	1
{	0	,	63	,	-48	},	//	2
{	15	,	52	,	-32	},	//	3
{	23	,	47	,	-24	},	//	4
{	27	,	43	,	-19	},	//	5
{	31	,	41	,	-16	},	//	6
{	33	,	40	,	-13	},	//	7
{	35	,	39	,	-12	},	//	8
{	36	,	38	,	-10	},	//	9
{	37	,	37	,	-9	},	//	10
{	38	,	36	,	-8	},	//	11
{	39	,	36	,	-8	},	//	12
{	39	,	35	,	-7	},	//	13
{	40	,	35	,	-6	},	//	14
{	40	,	35	,	-6	},	//	15
{	41	,	35	,	-6	},	//	16
{	41	,	34	,	-5	},	//	17
{	41	,	34	,	-5	},	//	18
{	41	,	34	,	-5	},	//	19
{	42	,	34	,	-4	},	//	20
{	42	,	34	,	-4	},	//	21
{	42	,	33	,	-4	},	//	22
{	42	,	33	,	-4	},	//	23
{	43	,	33	,	-4	},	//	24
{	43	,	33	,	-3	},	//	25
{	43	,	33	,	-3	},	//	26
{	43	,	33	,	-3	},	//	27
{	43	,	33	,	-3	},	//	28
{	43	,	33	,	-3	},	//	29
{	43	,	33	,	-3	},	//	30
{	43	,	33	,	-3	},	//	31
#else
#ifdef RES64X128
            {	0	,	128	,	-64	},	//	1
            {	0	,	127	,	-32	},	//	2
            {	9	,	105	,	-21	},	//	3
            {	15	,	95	,	-16	},	//	4
            {	18	,	88	,	-12	},	//	5
            {	20	,	84	,	-10	},	//	6
            {	21	,	81	,	-9	},	//	7
            {	23	,	79	,	-8	},	//	8
            {	23	,	77	,	-7	},	//	9
            {	24	,	75	,	-6	},	//	10
            {	25	,	74	,	-5	},	//	11
            {	25	,	73	,	-5	},	//	12
            {	26	,	72	,	-4	},	//	13
            {	26	,	72	,	-4	},	//	14
            {	26	,	71	,	-4	},	//	15
            {	27	,	71	,	-4	},	//	16
            {	27	,	70	,	-3	},	//	17
            {	27	,	70	,	-3	},	//	18
            {	27	,	69	,	-3	},	//	19
            {	27	,	69	,	-3	},	//	20
            {	27	,	69	,	-3	},	//	21
            {	28	,	68	,	-2	},	//	22
            {	28	,	68	,	-2	},	//	23
            {	28	,	68	,	-2	},	//	24
            {	28	,	68	,	-2	},	//	25
            {	28	,	67	,	-2	},	//	26
            {	28	,	67	,	-2	},	//	27
            {	28	,	67	,	-2	},	//	28
            {	28	,	67	,	-2	},	//	29
            {	28	,	67	,	-2	},	//	30
            {	28	,	67	,	-2	},	//	31
#else
#ifdef RES128X128
                {	0	,	128	,	-128	},	//	1
                {	0	,	127	,	-64	},	//	2
                {	20	,	105	,	-42	},	//	3
                {	31	,	95	,	-32	},	//	4
                {	37	,	88	,	-25	},	//	5
                {	41	,	84	,	-21	},	//	6
                {	44	,	81	,	-18	},	//	7
                {	47	,	79	,	-16	},	//	8
                {	48	,	77	,	-14	},	//	9
                {	50	,	75	,	-12	},	//	10
                {	51	,	74	,	-11	},	//	11
                {	52	,	73	,	-10	},	//	12
                {	53	,	72	,	-9	},	//	13
                {	53	,	72	,	-9	},	//	14
                {	54	,	71	,	-8	},	//	15
                {	55	,	71	,	-8	},	//	16
                {	55	,	70	,	-7	},	//	17
                {	55	,	70	,	-7	},	//	18
                {	56	,	69	,	-6	},	//	19
                {	56	,	69	,	-6	},	//	20
                {	56	,	69	,	-6	},	//	21
                {	57	,	68	,	-5	},	//	22
                {	57	,	68	,	-5	},	//	23
                {	57	,	68	,	-5	},	//	24
                {	57	,	68	,	-5	},	//	25
                {	58	,	67	,	-4	},	//	26
                {	58	,	67	,	-4	},	//	27
                {	58	,	67	,	-4	},	//	28
                {	58	,	67	,	-4	},	//	29
                {	58	,	67	,	-4	},	//	30
                {	58	,	67	,	-4	},	//	31
#else

                {	0	,	64	,	-64	},	//	1
                {	0	,	63	,	-32	},	//	2
                {	9	,	52	,	-21	},	//	3
                {	15	,	47	,	-16	},	//	4
                {	18	,	43	,	-12	},	//	5
                {	20	,	41	,	-10	},	//	6
                {	21	,	40	,	-9	},	//	7
                {	23	,	39	,	-8	},	//	8
                {	23	,	38	,	-7	},	//	9
                {	24	,	37	,	-6	},	//	10
                {	25	,	36	,	-5	},	//	11
                {	25	,	36	,	-5	},	//	12
                {	26	,	35	,	-4	},	//	13
                {	26	,	35	,	-4	},	//	14
                {	26	,	35	,	-4	},	//	15
                {	27	,	35	,	-4	},	//	16
                {	27	,	34	,	-3	},	//	17
                {	27	,	34	,	-3	},	//	18
                {	27	,	34	,	-3	},	//	19
                {	27	,	34	,	-3	},	//	20
                {	27	,	34	,	-3	},	//	21
                {	28	,	33	,	-2	},	//	22
                {	28	,	33	,	-2	},	//	23
                {	28	,	33	,	-2	},	//	24
                {	28	,	33	,	-2	},	//	25
                {	28	,	33	,	-2	},	//	26
                {	28	,	33	,	-2	},	//	27
                {	28	,	33	,	-2	},	//	28
                {	28	,	33	,	-2	},	//	29
                {	28	,	33	,	-2	},	//	30
                {	28	,	33	,	-2	},	//	31
#endif
#endif
#endif
#endif
        };


int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}

uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type) {

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
    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    if (z0 >= 32) {
        return 0;
    }

    z1 = z0 + dZ;

    if (z0 <= 2) {
        return 0;
    }

    if (z1 <= 2) {
        return 0;
    }

    if (z1 >= 32) {
        return 0;
    }


    if (type == RIGHT_NEAR) {
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

#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);

    fix_line( px0z1, py0z1, px1z1, py0z1, 4);

    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif

    {
        int16_t x0, x1;

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
        x0 = px0z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t upperY0 = py1z0;
            int16_t upperY1 = py1z1;
            int16_t upperDx = abs(x1 - x0);
            int16_t upperDy = -abs(upperY1 - upperY0);
            int16_t upperSy = upperY0 < upperY1 ? 1 : -1;
            int16_t upperErr = upperDx + upperDy;  /* error value e_xy */
            int16_t upperErr2;
            int16_t lowerY0 = py0z0;
            int16_t lowerY1 = py0z1;
            int16_t lowerDx = abs(x1 - x0);
            int16_t lowerSx = x0 < x1 ? 1 : -1;
            int16_t lowerDy = -abs(lowerY1 - lowerY0);
            int16_t lowerSy = lowerY0 < lowerY1 ? 1 : -1;
            int16_t lowerErr = lowerDx + lowerDy;  /* error value e_xy */
            int16_t lowerErr2 = 0;
            
            while (x0 != x1) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (IN_RANGE(0, XRESMINUSONE, x0)) {
                    if ( stipple && stencilHigh[x0] <= upperY0) {
                        graphicsPut(x0, upperY0);
                    }
                    
                    if (stencilHigh[x0] < lowerY0) {
                        stencilHigh[x0] = lowerY0;
                    }
                }
                
                /* loop */
                upperErr2 = upperErr * 2;
                
                if (upperErr2 >= upperDy || lowerErr2 >= lowerDy) {
                    upperErr += upperDy; /* e_xy+e_x > 0 */
                    lowerErr += lowerDy; /* e_xy+e_x > 0 */
                    x0 += lowerSx;
                }
                
                if (x0 >= XRES) {
                    return 0;
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
    }

    return 1;
}

uint8_t drawSquare(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, uint8_t elementMask) {
    
    uint8_t z0px;
    uint8_t z0py;
    int8_t z0dx;
    
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    
    uint8_t drawContour;
    
    if (z0 >= 32) {
        return 0;
    }
    
    z0px = (projections[z0].px);
    z0dx = ((projections[z0].dx));
    
    px0z0 = z0px - ((x0) * z0dx);
    
    px1z0 = px0z0 - (dX * z0dx);
    
    z0py = (projections[z0].py);
    
    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);
    
    if (px1z0 < 0 || px0z0 > XRESMINUSONE) {
        return 0;
    }

    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    drawContour = (dY);
    
#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);
    
    fix_line( px0z1, py0z1, px1z1, py0z1, 4);
    
    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif
    
    {
        int16_t x;
        
        if (drawContour) {
            if (elementMask & 2) {
                if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0] < py1z0 ? py1z0 : stencilHigh[px0z0], shouldStipple );
                }
                
                if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0] < py1z0 ? py1z0 : stencilHigh[px1z0], shouldStipple );
                }
            }
        }

        /* Draw the horizontal outlines of z0 and z1 */

        /* Ceiling is lower than camera */
        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x)) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stencilHigh[x] <= py1z0) {
                    if (drawContour && stipple) {
                        graphicsPut(x, py1z0);
                    }
                    stencilHigh[x] = py1z0;
                }


                if (stencilHigh[x] <= py0z0) {
                    stencilHigh[x] = py0z0;
                }
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

    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;


    if (z0 >= 32 || z0 <= 4) {
        return 0;
    }

    z1 = z0 + 1;

    if (z1 >= 32) {
        return 0;
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

    py0z0 = z0py + ((- CAMERA_HEIGHT) * z0dx);
    py0z1 = z1py + ((- CAMERA_HEIGHT) * z1dx);


    {
        int16_t x, x0, x1;

        /* Draw the horizontal outlines of z0 and z1 */

        for (x = px0z0; x <= px1z0; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x)) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple && stencilHigh[x] < py0z0) {
                    graphicsPut(x, py0z0);
                }
            }
        }

        for (x = px0z1; x <= px1z1; ++x) {
            if (IN_RANGE(0, XRESMINUSONE, x)) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple && stencilHigh[x] < py0z1) {
                    graphicsPut(x, py0z1);
                }
            }
        }



        /* The left segment */
        x0 = px0z0;
        x1 = px0z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, XRESMINUSONE, x0)) {

                    if (shouldStipple) {
                        stipple = !stipple;
                    }

                    if (stipple && stencilHigh[x0] < y0) {
                        graphicsPut(x0, y0);
                    }
                }

                /* loop */
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= XRES) {
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

        /* The right segment */
        x0 = px1z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple && IN_RANGE(0, XRESMINUSONE, x0) && stencilHigh[x0] < y0) {
                    graphicsPut(x0, y0);
                }

                /* loop */
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= XRES) {
                    return 1;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
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

    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
    uint8_t stipple = 1;

    uint8_t drawContour;

    if (z0 >= 32|| z0 <= 4) {
        return 0;
    }

    z1 = z0 + dZ;

    if (z1 >= 32) {
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

    z1py = (projections[z1].py);
    z0py = (projections[z0].py);

    py0z0 = z0py + ((y0) * z0dx);
    py0z1 = z1py + ((y0) * z1dx);

    drawContour = (dY);

#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);

    fix_line( px0z1, py0z1, px1z1, py0z1, 4);

    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif

    {
        int16_t x, x0, x1;

        if (drawContour) {
            if (elementMask & 2) {
                if ((elementMask != 255 ) &&IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0], shouldStipple);
                }

                if ( (elementMask != 127 ) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0], shouldStipple);
                }
            }

            if (elementMask & 1) {
                if ((elementMask != 255 ) &&IN_RANGE(0, XRESMINUSONE, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                    vLine(px0z1, py0z1, stencilHigh[px0z1], shouldStipple);
                }

                if ((elementMask != 127 ) && IN_RANGE(0, XRESMINUSONE, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                    vLine(px1z1, py0z1, stencilHigh[px1z1], shouldStipple);
                }
            }
        }

        /* Draw the horizontal outlines of z0 and z1 */

        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {

                    if (shouldStipple) {
                        stipple = !stipple;
                    }

                    if (drawContour && stipple) {
                        graphicsPut(x, stencilHigh[x]);
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else if (drawContour) {
            /* Ceiling is higher than the camera*/
            /* Let's just draw the nearer segment */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {

                    if (shouldStipple) {
                        stipple = !stipple;
                    }

                    if (stipple) {
                        graphicsPut(x, stencilHigh[x]);
                    }
                }
            }
        }


        /* The left segment */
        x0 = px0z0;
        x1 = px0z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, XRESMINUSONE, x0)) {
                    if (shouldStipple) {
                        stipple = !stipple;
                    }

                    if (stencilHigh[x0] < y0) {
                        if (drawContour && stipple) {
                            graphicsPut(x0, stencilHigh[x0]);
                        }
                        stencilHigh[x0] = y0;
                    }
                }

                /* loop */
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= XRES) {
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

        /* The right segment */
        x0 = px1z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, XRESMINUSONE, x0) && stencilHigh[x0] < y0) {

                    if (shouldStipple) {
                        stipple = !stipple;
                    }

                    if (drawContour && stipple) {
                        graphicsPut(x0, stencilHigh[x0]);
                    }
                    stencilHigh[x0] = y0;
                }

                /* loop */
                e2 = err << 2;

                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }

                if (x0 >= XRES) {
                    goto final_stroke;
                }

                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }

        final_stroke:
        if (py0z0 <= py0z1) {
            /* Ceiling is higher than the camera*/
            /* Draw the last segment */

            if (drawContour) {
                for (x = px0z1; x <= px1z1; ++x) {
                    if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z1) {
                        stencilHigh[x] = py0z1;
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
    }

    return 1;
}


uint8_t drawPattern(uint8_t _pattern, int8_t x0, int8_t x1, int8_t y) {
    int8_t diff;
    uint8_t pattern = (_pattern - 32) & 127;
    uint8_t type;

    /* 127 = 01111111 - the first bit is used for indicating the presence of an object.
     * And since there are only 127 patterns anyway...
     * */
#ifndef TRACE_OBJECTS_OVER_FLOOR
    if (_pattern & 128) {
        drawObjectAt(x0 - 1, y + 2);
        return 1;
    }
#endif

    diff = patterns[0].ceiling - patterns[pattern].ceiling;
    type = patterns[pattern].geometryType;

    uint8_t mask = patterns[pattern].elementsMask;

    if (x0 == 2) {
        mask = 255;
    }

    if (x1 == 2) {
        mask = 127;
    }

    if (type == CUBE) {
        return drawCubeAt(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2, x1 - x0,
                          diff, 1,  mask);
    } else if (type == RIGHT_NEAR || type == LEFT_NEAR  ){

        if ( cameraRotation == 1 || cameraRotation == 3 ) {

            if (type == RIGHT_NEAR ) {
                type = LEFT_NEAR;
            } else {
                type = RIGHT_NEAR;
            }
        }

        return drawWedge(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2, x1 - x0,
                         diff, 1, patterns[pattern].elementsMask, type);
        
    } else if (type == LEFT_WALL ){

        switch (cameraRotation) {
            case 0:
            case 2:
                return drawWedge(x0 - (cameraRotation == 0 ? 1 : 0), patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
                                 0, diff, 1, patterns[pattern].elementsMask, LEFT_NEAR);
            case 1:
            case 3:
                return drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT,
                        y + ( cameraRotation == 3 ? 1 : 0 ) + 2,
                                  x1 - x0, diff, mask);
        }
    } else if (type == BACK_WALL){


        switch (cameraRotation) {
            case 0:
            case 2:
                return drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT,
                        y + (cameraRotation == 0? 1 : 0) + 2,
                                  x1 - x0, diff, mask);
            case 1:
            case 3:
                return drawWedge(x0 - (cameraRotation == 1 ? 1 : 0 ),
                        patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
                                 0, diff, 1, patterns[pattern].elementsMask, LEFT_NEAR);


        }
    } else if (type == CORNER){
        int returnVal = 0;

        switch( cameraRotation) {

            case 3:
            case 0:
                returnVal = drawWedge(x0 - (cameraRotation == 3 ? 0 : 1),
                        patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
                                      0, diff, 1, patterns[pattern].elementsMask, LEFT_NEAR) ;

                returnVal = drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 1 + 2,
                                       x1 - x0, diff, patterns[pattern].elementsMask) || returnVal;
                break;

            case 1:
            case 2:
                returnVal = drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
                                       x1 - x0, diff, patterns[pattern].elementsMask);

                returnVal = drawWedge(x0  - (cameraRotation == 1 ? 1 : 0), patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
                                      0, diff, 1, patterns[pattern].elementsMask, LEFT_NEAR) || returnVal;

                break;
        }

        return returnVal;
    }
    
    return 0;
}

#ifdef TRACE_OBJECTS_OVER_FLOOR
void repaintMapItems() {
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
                drawObjectAt(-(item->position.x - cameraX) + 1, (item->position.y - cameraZ) +2);
                node = node->next;
            }
            break;

        case 3:
            //        drawPattern(lastPattern, -(y - cameraZ) + 2, -(lastIndex - cameraZ)  + 2, cameraX - x);
            while (node != NULL) {
                struct Item *item = getItem(node->item);
                drawObjectAt( -(item->position.y - cameraZ) + 1, (cameraX - item->position.x) + 2);
                node = node->next;
            }
            break;
    }
}
#endif

/* all those refactors are due to a SDCC bug with very long functions */
void renderScene() {
    uint8_t x;

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

    int8_t *stencilPtr = &stencilHigh[0];

    for (x = 0; x < XRESMINUSONE;) {
        uint8_t y, prevY;
        uint8_t *ptr;
next_cluster:
        //pixel 1
        y = *stencilPtr;
        ptr = graphicsPutAddr(x, y, NULL );

        if (x & 7) {
            ++x;
            ++stencilPtr;
            continue;
        }

        for (int c = 2; c < 8; ++c ) {
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
    int8_t *stencilPtr = &stencilHigh[0];
    uint8_t signal = 0;

    for (x = 0; x < XRES; ++x) {
        int8_t stencilY = (*stencilPtr);
#ifdef MSDOS
        signal = !signal;

        if (stencilY > 86) {
            vLine(x, stencilY, 128, 2);
        } else {
            vLine(x, stencilY + (signal), 86, 3);
            vLine(x, 86, 128, 2);
        }
#else
        graphicsPut(x, stencilY);
#endif
        ++stencilPtr;
    }
#endif

#ifdef TRACE_OBJECTS_OVER_FLOOR
    repaintMapItems();
#endif
}

void renderCameraWest() {
    int8_t x;
    uint8_t lastPattern, lastIndex;
    int8_t maxX = 0;
    int8_t y;
    uint8_t pattern;

    for (x = cameraX; x >= 0; --x) {

        int8_t minX = min(cameraZ + ((cameraX) - x), 31);
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
        drawPattern(lastPattern, -(y - cameraZ) + 2, -(lastIndex - cameraZ)  + 2, cameraX - x);


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

void
renderCameraSouth() {
    int8_t y;
    uint8_t lastPattern, lastIndex;
    int8_t const *mapXY;
    int8_t maxX = 0;
    int8_t x;
    uint8_t pattern;

    for (y = cameraZ; y < VISIBILITY_LIMIT; ++y) {

        int8_t const *mapY = &map[y][0];
        int8_t minX = min(cameraX + (y - cameraZ), 31);
        lastIndex = cameraX;
        lastPattern = *(mapY + lastIndex);
        mapXY = &map[y][lastIndex];

        for (x = lastIndex; x <= minX; ++x) {

            pattern = *mapXY;

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, -(x - cameraX) + 2, -(lastIndex - cameraX) + 2, y - cameraZ)) {
                    x = VISIBILITY_LIMIT;
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

void renderCameraEast() {
    int8_t x;
    uint8_t lastPattern, lastIndex;
    int8_t maxX = 0;
    int8_t y;
    uint8_t pattern;

    for (x = cameraX; x < VISIBILITY_LIMIT; ++x) {

        int8_t minY = min(cameraZ + (x - cameraX), 31);
        lastIndex = cameraZ;
        lastPattern = map[lastIndex][x];

        for (y = lastIndex; y <= minY; ++y) {

            pattern = map[y][x];

            if (pattern != lastPattern) {

                if (!drawPattern(lastPattern, (lastIndex - cameraZ) + 2, (y - cameraZ) + 2, x - cameraX)) {
                    y = VISIBILITY_LIMIT;
                }
                lastIndex = y;

                lastPattern = pattern;
            }
        }

        drawPattern(lastPattern, (lastIndex - cameraZ) + 2 , (y - cameraZ) + 2, x - cameraX);


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

void renderCameraNorth() {

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

void pickItem() {
    struct Room* room = getRoom(getPlayerRoom());

    if (roomItem && roomItem->item) {
        struct Item* itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL ) {

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

void dropItem() {

    struct Item *item = NULL;

    if (focusedItem != NULL) {
        item = getItem(focusedItem->item);
    }

    if (item != NULL) {
        uint8_t pattern;
        struct WorldPosition* pos = getPlayerPosition();

        dropObjectToRoom(getPlayerRoom(), item);

        focusedItem = getPlayerItems();

        roomItem = &objectNodes[item->index];

        switch(cameraRotation) {
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

void nextItemInRoom() {
    struct Room* room = getRoom(getPlayerRoom());

    if (roomItem == NULL ) {
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

void interactWithItemInRoom() {
    struct Item *item = NULL;
    struct Item* itemToPick = NULL;

    if (roomItem != NULL) {
        itemToPick = getItem(roomItem->item);
        if (itemToPick != NULL ) {
            if (focusedItem != NULL) {
                item = getItem(focusedItem->item);
                if (item != NULL) {
                    item->useWithCallback(item, itemToPick);
                }
            }
        }
    }
}

void useItemInHand() {
    useObjectNamed(getItem(focusedItem->item)->name);
}

void nextItemInHand() {
    focusedItem = focusedItem->next;

    if (!focusedItem) {
        focusedItem = getPlayerItems();
    }
}

void updateMapItems();

void initMap() {
    int x, y, c;
    const uint8_t *head;
    uint16_t offsetOnDataStrip = 0;
    int16_t repetitions = -1;
    uint8_t current = '.';

    /* first item in the list is always a dummy */
    roomItem = getRoom(playerLocation)->itemsPresent->next;


/* TODO: precalc absolute offsets */
    for (c = 0; c < playerLocation; ++c ) {
        offsetOnDataStrip += dataPositions[c];
    }

    head = &data[offsetOnDataStrip];

    for (y = 0; y < 32; ++y ) {
        for (x = 0; x < 32; ++x ) {

            if (repetitions < 1) {
                repetitions = *head;

                if (repetitions >= 32 ) {
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


            if ((current == 's' && enteredFrom == 0) ||
                (current == 'w' && enteredFrom == 1) ||
                (current == 'n' && enteredFrom == 2) ||
                (current == 'e' && enteredFrom == 3) ){

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

        }
    }
    updateMapItems();
    HUD_initialPaint();
}

void updateMapItems() {
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

void tickRenderer() {
    uint8_t prevX;
    uint8_t prevZ;
    struct WorldPosition *pos;
    int previousLocation = playerLocation;
    uint8_t newCell = 0;

#ifndef SMS
    clearGraphics();
#endif
    renderScene();

    graphicsFlush();
    memset(stencilHigh, 0, XRES);

    prevX = cameraX;
    prevZ = cameraZ;

    waitkey:
    switch (getKey()) {

#ifndef CPC_PLATFORM
#ifndef SMS
        case 'l':
            shutdownGraphics();
            exit(0);
#endif
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
            break;

        case '4':
            nextItemInRoom();
            break;

        case '8':
            useItemInHand();
            updateMapItems();
            break;

        case '5':
            interactWithItemInRoom();
            updateMapItems();
            break;

        case '9':
            pickItem();
            break;

        case '6':
            dropItem();
            break;

        case 'p':
#ifndef XCODE_BUILD
#if !defined(SDLSW) || !defined(AMIGA)
        default:
            goto waitkey;
#endif
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

    if (patterns[newCell - 32].blockMovement) {
        pos->x = cameraX = prevX;
        pos->y = cameraZ = prevZ;
        setPlayerPosition(pos);
    }

    /* unlike MX, we are signaling from the origin into the new room. MX allows for the movement and then searches where
     * did the player came from - hence the "opposite direction" there */

    if (newCell > ('0' - 1) && newCell < ('3' + 1) ) {
        enteredFrom = newCell - '0';
        moveBy(enteredFrom);
    }

    if (playerLocation != previousLocation) {
        initMap();
        cameraRotation = getPlayerDirection();
    } else {
        enteredFrom = 0xFF;
    }
    HUD_refresh();
}


void onError(const char* mesg) {
#ifndef SMS
    #ifdef CPC_PLATFORM
        writeStr(1,1, mesg, 1, 2);
    #else
        puts(mesg);
    #endif
#else
    showMessage(mesg);
#endif
}

void logDelegate(const char* mesg) {
    showMessage(mesg);
}

#ifdef XCODE_BUILD
int demoMain() {
    
    for ( int nLine = 0; nLine < 200; ++nLine) {
        printf("%d,\n", ((nLine & 248) * 10) + ((nLine & 7) << 11));
    }
#else

int main(
#ifndef SMS
        int argc, char **argv
#endif
        ) {
#endif


    {


        running = 1;
        enteredFrom = 0;
        cameraRotation = 0;
        init();
        initFileReader("base.pfs");
        initStation();
        
        titleScreen();

        focusedItem = getPlayerItems();
        setErrorHandlerCallback(onError);
        setLoggerDelegate(logDelegate);
        initMap();

        memset(stencilHigh, 0, XRES);



#ifndef XCODE_BUILD
        do {
            tickRenderer();
        } while (running);

        shutdownGraphics();
#endif
    }
    return 0;
}
