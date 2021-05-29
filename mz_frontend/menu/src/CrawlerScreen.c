#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "FixP.h"
#include "LoadBitmap.h"
#include "Enums.h"
#include "Engine.h"
#include "FixP.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"
#include "UI.h"

extern size_t biggestOption;

#define XRES 128
#define YRES 128
#define XRESMINUSONE (XRES - 1)
#define YRESMINUSONE YRES - 1


#define WALKSTEP 2
#define CAMERA_HEIGHT 2
#define VISIBILITY_LIMIT 32

enum GeometryType {
    CUBE = 0,
    RIGHT_NEAR = 4,
    LEFT_NEAR = 8,
    LEFT_WALL = 16,
    BACK_WALL = 32,
    CORNER = 64,
};

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

const char *options[4] = {
        "Option 1",
        "Option 2",
        "Option 3",
        "Option 4",
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

int8_t stencilHigh[XRES];
uint8_t stencilLow[XRES];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
int8_t viewMenu;
int nextDirtyLineY1;

unsigned long lastTime = 0;
void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

struct Texture *montyTexture;

struct Projection {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


struct Pattern {
    int8_t floor;
    uint8_t ceiling;
    uint8_t elementsMask;
    enum GeometryType geometryType;
    uint8_t block;
    int8_t textureIndex;
};

const struct Projection projections[40] =
        {
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
                {	59	,	67	,	-4	},	//	32
                {	59	,	66	,	-3	},	//	33
                {	59	,	66	,	-3	},	//	34
                {	59	,	66	,	-3	},	//	35
                {	59	,	66	,	-3	},	//	36
                {	59	,	66	,	-3	},	//	37
                {	59	,	66	,	-3	},	//	38
                {	59	,	66	,	-3	},	//	39
                {	59	,	66	,	-3	},	//	40
        };

const struct Pattern patterns[16] = {
        {0, 7, 3, CUBE, 0, -1}, //0
        {0, 7, 3, CUBE, 1, -1}, // 1
        {0, 0, 0, CUBE, 0, -1}, // 2
        {0, 0, 3, CUBE, 0, -1}, //3
        {0, 0, 3, RIGHT_NEAR, 0, 0}, //4
        {0, 0, 3, LEFT_NEAR, 0, 0}, //5
        {0, 3, 3, CUBE, 0, -1}, //6
        {0, 7, 3, CUBE, 0, -1}, //7
        {0, 0, 0, LEFT_WALL, 0, -1}, // 8
        {0, 0, 3, BACK_WALL, 0, -1}, // 9
        {0, 6, 3, CUBE, 0, -1}, // a
        {2, 7, 3, CUBE, 0, -1}, // b
        {3, 7, 3, CUBE, 0, -1}, // c
        {4, 7, 3, CUBE, 0, -1}, // d
};

const int8_t map[32][32] = {
        {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1},
        {0x1, 0x2, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x2, 0x1, 0x1},
        {0x1, 0x8, 0x7, 0x7, 0x5, 0x2, 0x4, 0x7, 0x7, 0x7, 0x7, 0x7, 0x5, 0x3, 0x6, 0x6, 0x3, 0x4, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x5, 0x2, 0x1},
        {0x1, 0x8, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x7, 0x7, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x7, 0xC, 0xD, 0xC, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x7, 0x7, 0xB, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x9, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x4, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x3, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x6, 0x7, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x6, 0x7, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x3, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x5, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x7, 0x7, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x7, 0x8, 0x1},
        {0x1, 0x8, 0x5, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x4, 0x8, 0x1},
        {0x1, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x8, 0x1},
        {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1}
};

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}

void drawSquare(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t textureIndex, uint8_t elementMask) {

    uint8_t z0px;
    uint8_t z0py;
    int8_t z0dx;

    int16_t px0z0;
    int8_t py0z0;
    int8_t py1z0;
    int16_t px1z0;

    uint8_t drawContour;

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

#ifdef TEXTURES
    if (textureIndex != -1 ) {
      //drawFrontWall(intToFix(px0z0 * 2), intToFix(py0z0), intToFix(px1z0 * 2), intToFix(py1z0), &montyTexture->rotations[0][0], intToFix(2), z0, 0);
        drawFrontWall(intToFix(px0z0 * 2), intToFix(py0z0), intToFix(px1z0 * 2), intToFix(py1z0), &montyTexture->rotations[0][0], intToFix(2), z0, 0  );
        return ;
    }
#endif

    {
        int16_t x;

        if (drawContour) {
            if (elementMask & 2) {
                if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                    graphicsVerticalLine(px0z0, py0z0, stencilHigh[px0z0], 0);
                }

                if (IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                    graphicsVerticalLine(px1z0, py0z0, stencilHigh[px1z0], 0);
                }
            }
        }

        /* Draw the horizontal outlines of z0 and z1 */

        /* Ceiling is lower than camera */
        if (drawContour) {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
#ifndef FILLED_POLYS
                    graphicsPut(x, py0z0, 0);
                    graphicsPut(x, stencilHigh[x], 0);
#else
                    graphicsVerticalLine(x, py0z0, max(stencilHigh[x], py1z0), 9);
#endif
                    stencilHigh[x] = py0z0;
                }
            }
        } else {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    stencilHigh[x] = py0z0;
                }
            }
        }
    }
}

void drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, int8_t textureIndex, enum GeometryType type) {

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

    uint8_t drawContour;

    z1 = z0 + dZ;

    if (z0 <= 2) {
        return;
    }

    if (z1 <= 2) {
        return;
    }

    if (type != LEFT_NEAR) {
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

    if (px1z1 < 0 || px0z0 > XRESMINUSONE) {
        return;
    }

    drawContour = dY;
    
#ifdef TEXTURES
    if (textureIndex != -1 ) {
        drawWall(intToFix(px0z0 * 2), intToFix(px1z1 * 2), intToFix(py0z0), intToFix(py1z0), intToFix(py0z1), intToFix(py1z1), &montyTexture->rowMajor[0], intToFix(2), z0 );
        return;
    }
#endif


    {
        int16_t x0, x1;
#ifndef FILLED_POLYS
        if (drawContour) {

            if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                graphicsVerticalLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]), 0);
            }


            if (IN_RANGE(0, XRESMINUSONE, px1z1) && py0z1 > stencilHigh[px1z1]) {
                graphicsVerticalLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]), 0);
            }
        }
#endif


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

            while ((x0 != x1 && (upperY0 != upperY1 || lowerY0 != lowerY1))) {

                if (IN_RANGE(0, XRESMINUSONE, x0)) {
#ifndef FILLED_POLYS
                    if (stencilHigh[x0] <= upperY0) {
                        if (drawContour) {
                            graphicsPut(x0, upperY0, 0);
                        }

                    }
#endif

                    if (stencilHigh[x0] < lowerY0) {
                        if (drawContour) {
#ifdef FILLED_POLYS
                            graphicsVerticalLine(x0, max(upperY0, stencilHigh[x0]), lowerY0, 4);
#else
                            graphicsPut(x0, lowerY0 - 1, 0);
#endif
                        }
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
    }
}

void drawHighCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, int8_t textureIndex) {

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
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;

    uint8_t drawContour;

    z1 = z0 + dZ;

    if (dY) {
        if (z0 <= 5) {
            return;
        }

        if (z1 <= 5) {
            return;
        }
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
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);

    if (px1z0 < 0 || px0z0 > XRESMINUSONE) {
        return;
    }

#ifdef TEXTURES
    if (textureIndex != -1 ) {
        drawFrontWall(intToFix(px0z0 * 2), intToFix(py0z0), intToFix(px1z0 * 2), intToFix(py1z0), &montyTexture->rotations[0][0], intToFix(2), z0, 0);
        return;
    }
#endif

/*
    drawWall(intToFix(px0z0 * 2), intToFix(px1z1 * 2), intToFix(py0z0), intToFix(py1z0), intToFix(py0z1), intToFix(py1z1), &montyTexture->rowMajor[0], intToFix(1), 1 );

    drawWall(intToFix(px0z0 * 2), intToFix(px1z1 * 2), intToFix(py0z0), intToFix(py1z0), intToFix(py0z1), intToFix(py1z1), &montyTexture->rowMajor[0], intToFix(1), 1 );
*/

    drawContour = (dY);
    {
        int16_t x, x0, x1;
#ifndef FILLED_POLYS
        if (drawContour) {

            if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                graphicsVerticalLine(px0z0, py0z0, stencilHigh[px0z0], 0);
            }

            if (IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                graphicsVerticalLine(px1z0, py0z0, stencilHigh[px1z0], 0);
            }
            if (IN_RANGE(0, XRESMINUSONE, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                graphicsVerticalLine(px0z1, py0z1, stencilHigh[px0z1], 0);
            }

            if (IN_RANGE(0, XRESMINUSONE, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                graphicsVerticalLine(px1z1, py0z1, stencilHigh[px1z1], 0);
            }

        }
#endif
        /* Draw the horizontal outlines of z0 and z1 */

        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x, py0z0, stencilHigh[x], 9);
#else
                        graphicsPut(x, py0z0, 0);
                        graphicsPut(x, stencilHigh[x], 0);
#endif
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else if (drawContour) {
            /* Ceiling is higher than the camera*/
            /* Let's just draw the nearer segment */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
#ifdef FILLED_POLYS
                    graphicsVerticalLine(x, py0z0, stencilHigh[x], 9);
#else
                    graphicsPut(x, py0z0, 0);
                    graphicsPut(x, stencilHigh[x], 0);
#endif
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
                    if (stencilHigh[x0] < y0) {
                        if (drawContour) {
#ifdef FILLED_POLYS
                            graphicsVerticalLine(x0, y0, stencilHigh[x0], 3);
#else
                            graphicsPut(x0, y0, 0);
                            graphicsPut(x0, stencilHigh[x0], 0);
#endif
                        }
                        stencilHigh[x0] = y0;
                    }
                }

                /* loop */
                e2 = err * 2;

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

                if (IN_RANGE(0, 127, x0) && stencilHigh[x0] < y0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x0, y0, stencilHigh[x0], 3);
#else
                        graphicsPut(x0, y0, 0);
                        graphicsPut(x0, stencilHigh[x0], 0);
#endif
                    }
                    stencilHigh[x0] = y0;
                }

                /* loop */
                e2 = err * 2;

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

            for (x = px0z1; x <= px1z1; ++x) {
                if (IN_RANGE(0, 127, x) && stencilHigh[x] < py0z1) {
#ifndef FILLED_POLYS
                    if (drawContour) {
                        graphicsPut(x, py0z1, 0);
                    }
#endif
                    stencilHigh[x] = py0z1;
                }
            }
        }
    }
}

#ifdef DRAW_LOW_GEOMETRY
void drawLowCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, int8_t textureIndex) {

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

    uint8_t drawContour;

    z1 = z0 + dZ;

    if (dY) {
        if (z1 <= 5) {
            return;
        }

        if (z0 <= 5) {
            return;
        }
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

    if (px1z0 < 0 || px0z0 > 127) {
        return;
    }

#ifdef TEXTURES
    if (textureIndex != -1 ) {
        drawFrontWall(intToFix(px0z0 * 2), intToFix(py0z0), intToFix(px1z0 * 2), intToFix(py1z0), &montyTexture->rotations[0][0], intToFix(2), z0, 0);
        return;
    }
#endif

/*
    drawWall(intToFix(px0z0 * 2), intToFix(px1z1 * 2), intToFix(py0z0), intToFix(py1z0), intToFix(py0z1), intToFix(py1z1), &montyTexture->rowMajor[0], intToFix(1), 1 );

    drawWall(intToFix(px0z0 * 2), intToFix(px1z1 * 2), intToFix(py0z0), intToFix(py1z0), intToFix(py0z1), intToFix(py1z1), &montyTexture->rowMajor[0], intToFix(1), 1 );
*/


    drawContour = (dY);

    {
        int16_t x, x0, x1;
#ifndef FILLED_POLYS
        if (drawContour) {

            if (IN_RANGE(0, 127, px0z0) && stencilLow[px0z0] > py0z0) {
                graphicsVerticalLine(px0z0, py0z0, stencilLow[px0z0], 0);
            }

            if (IN_RANGE(0, 127, px1z0) && stencilLow[px1z0] > py0z0) {
                graphicsVerticalLine(px1z0, py0z0, stencilLow[px1z0], 0);
            }
            if (IN_RANGE(0, 127, px0z1) && px0z1 < px0z0 && py0z1 < stencilLow[px0z1]) {
                graphicsVerticalLine(px0z1, py0z1, stencilLow[px0z1], 0);
            }

            if (IN_RANGE(0, 127, px1z1) && px1z1 > px1z0 && py0z1 < stencilLow[px1z1]) {
                graphicsVerticalLine(px1z1, py0z1, stencilLow[px1z1], 0);
            }
        }
#endif


        if (py0z0 < py0z1) {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 127, x) && stencilLow[x] > py0z0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x, py0z0, stencilLow[x], 9);
#else
                        graphicsPut(x, py0z0, 0);
                        graphicsPut(x, stencilLow[x], 0);
#endif
                    }
                    stencilLow[x] = py0z0;
                }
            }
        } else if (drawContour) {
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 127, x) && stencilLow[x] > py0z0) {
#ifdef FILLED_POLYS
                    graphicsVerticalLine(x, py0z0, stencilLow[x], 9);
#else
                    graphicsPut(x, py0z0, 50);
                    graphicsPut(x, stencilLow[x], 64);
#endif
                }
            }
        }


        /* The left segment */
        x0 = px0z1;
        x1 = px0z0;

        if (x0 != x1) {
            int16_t y0 = py0z1;
            int16_t y1 = py0z0;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;

            while ((x0 != x1 || y0 != y1)) {

                if (IN_RANGE(0, 127, x0)) {
                    if (stencilLow[x0] > y0) {
                        if (drawContour) {
#ifdef FILLED_POLYS
                            graphicsVerticalLine(x0, y0, stencilLow[x0], 3);
#else
                            graphicsPut(x0, y0, 0);
                            graphicsPut(x0, stencilLow[x0], 0);
#endif
                        }
                        stencilLow[x0] = y0;
                    }
                }

                /* loop */
                e2 = err * 2;

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

                if (IN_RANGE(0, 127, x0) && stencilLow[x0] > y0) {
                    if (drawContour) {
#ifdef FILLED_POLYS
                        graphicsVerticalLine(x0, y0, stencilLow[x0], 3);
#else
                        graphicsPut(x0, y0, 0);
                        graphicsPut(x0, stencilLow[x0], 0);
#endif
                    }
                    stencilLow[x0] = y0;
                }

                /* loop */
                e2 = err * 2;

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
        if (py0z0 >= py0z1) {
            /* Ceiling is higher than the camera*/
            /* Draw the last segment */

            for (x = px0z1; x <= px1z1; ++x) {
                if (IN_RANGE(0, 127, x) && stencilLow[x] > py0z1) {
#ifndef FILLED_POLYS
                    if (drawContour) {
                        graphicsPut(x, py0z1, 0);
                    }
#endif
                    stencilLow[x] = py0z1;
                }
            }
        }
    }
}
#endif

uint8_t drawPattern(uint8_t pattern, uint8_t x0, uint8_t x1, uint8_t y) {

    int8_t diffCeiling = patterns[0].ceiling     - patterns[pattern].ceiling;
    int8_t diffFloor   = patterns[pattern].floor - patterns[0].floor;
    uint8_t type = patterns[pattern].geometryType;

    if (patterns[pattern].block) {
        return 0;
    }

    if (type == CUBE) {
        drawHighCubeAt(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y, x1 - x0,
                       diffCeiling, 1, patterns[pattern].textureIndex);
#if DRAW_LOW_GEOMETRY
        drawLowCubeAt(x0, patterns[pattern].floor - CAMERA_HEIGHT, y, x1 - x0,
                      diffFloor, 1, patterns[pattern].textureIndex);
#endif

    } else if (type == RIGHT_NEAR || type == LEFT_NEAR  ){


        drawWedge(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y, x1 - x0,
                  diffCeiling, 1, patterns[pattern].textureIndex, type);

#if DRAW_LOW_GEOMETRY
        drawLowCubeAt(x0, patterns[pattern].floor - CAMERA_HEIGHT, y, x1 - x0,
                      diffFloor, 1, patterns[pattern].textureIndex);
#endif

    } else if (type == LEFT_WALL  ){


        drawWedge(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y,
                  0, diffCeiling, 1, patterns[pattern].textureIndex, LEFT_NEAR);

#if DRAW_LOW_GEOMETRY
        drawLowCubeAt(x0, patterns[pattern].floor - CAMERA_HEIGHT, y, x1 - x0,
                      diffFloor, 1, patterns[pattern].textureIndex);
#endif

    } else if (type == BACK_WALL  ){


        drawSquare(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 1,
                   x1 - x0, diffCeiling, patterns[pattern].textureIndex, patterns[pattern].elementsMask);

#if DRAW_LOW_GEOMETRY
        drawLowCubeAt(x0, patterns[pattern].floor - CAMERA_HEIGHT, y, x1 - x0,
                      diffFloor, 1, patterns[pattern].textureIndex);
#endif

    }

    return 0;
}

void renderScene() {
    uint8_t lastPattern, lastIndex;

    switch (cameraRotation) {
        case DIRECTION_N: {

            int8_t limit = max(cameraZ - 31, 0);
            for (int8_t y = min(cameraZ - 3, 31); y >= limit; --y) {
                int8_t x;
                lastIndex = cameraX;
                lastPattern = map[y][lastIndex];

                for (x = lastIndex; x < min(cameraX + 5 + ((cameraZ - 3) - y), 31); ++x) {
                    uint8_t pattern;

                    pattern = map[y][x];

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    if (patterns[pattern].block) {
                        break;
                    }

                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                }

                lastIndex = max(cameraX - 1, 0);
                lastPattern = map[y][lastIndex];

                for (x = lastIndex; x >= max(cameraX - 3 - ((cameraZ - 3) - y), 0); --x) {
                    uint8_t pattern;
                    pattern = map[y][x];

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    if (patterns[pattern].block) {
                        break;
                    }

                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                }
            }
        }
            break;

        case DIRECTION_E: {

            for (int8_t x = min(cameraX - 3, 31); x <= min(cameraX + 13, 31); ++x) {
                int8_t y;

                for (y = cameraZ; y <= min(cameraZ + (x - cameraX), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ - (x - cameraX), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

            }
        }
            break;

        case DIRECTION_S: {

            for (int8_t y = min(cameraZ + 3, 31); y <= min(cameraZ + 19, 31); ++y) {
                int8_t x;
                for (x = cameraX; x <= min(cameraX + (y - (cameraZ + 3)), 31); ++x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }

                for (x = max(cameraX - 1, 0); x >= max(cameraX - (y - (cameraZ + 3)), 0); --x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
            }
        }
            break;

        case DIRECTION_W: {

            for (int8_t x = max(cameraX, 0); x >= max(cameraX - 16, 0); --x) {
                int8_t y;
                for (y = cameraZ; y <= min(cameraZ - (x - (cameraX)), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ + (x - (cameraX)), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
            }
        }
            break;
    }

    for (int x = 0; x < XRES; x++) {
        graphicsVerticalLine(x, stencilHigh[x] + 1, YRESMINUSONE, 6);
    }
}


int32_t Interrogation_initStateCallback(int32_t tag, void *data) {

    currentBackgroundBitmap = loadBitmap("pattern.img");
    montyTexture = makeTextureFrom("montytex.img");

    timeUntilNextState = 10000 - 1;

    currentPresentationState = kWaitingForInput;

    cameraX = 5;
    cameraZ = 15;
    cameraRotation = 0;
    viewMenu = -1;

    memset(&stencilHigh[0], 0, XRES);
    memset(&stencilLow[0], 127, XRES);

    return 0;
}

void Interrogation_initialPaintCallback() {
    drawRepeatBitmap(0, 0, 320, 200, currentBackgroundBitmap);

    fill(8, 144, 120, 8, 0, FALSE);
    fill(142, 144, 80, 8, 0, FALSE);
    fill(236, 144, 64, 8, 0, FALSE);

    drawTextAt(3, 19, "Interrogation", 4);
    drawTextAt(20, 19, "Emotions", 4);
    drawTextAt(32, 19, "Stress", 4);


    fill(8 + 8, 144 + 8, 120, 48, 0, TRUE);
    fill(142 + 8, 144 + 8, 80, 48, 0, TRUE);
    fill(236 + 8, 144 + 8, 64, 48, 0, TRUE);

    fill(8, 152, 120, 40, 7, FALSE);
    fill(142, 152, 80, 40, 7, FALSE);
    fill(236, 152, 64, 40, 7, FALSE);

    drawRect(8, 152, 120, 40, 0);
    drawRect(142, 152, 80, 40, 0);

    drawRect(236, 152, 64, 40, 0);

    dirtyLineY0 = 0;
    dirtyLineY1 = 200;
    fill(7, 128, 256 - 8, 8, 0, TRUE);
}

void Interrogation_repaintCallback() {

    memset(&stencilHigh[0], 0, XRES);
    memset(&stencilLow[0], 127, XRES);

    fill(255, 8, 8, 128, 0, TRUE);


    fill(0, 0, 256, 64,
#ifndef FILLED_POLYS
         7
#else
            5
#endif
            , FALSE);

    fill(0, 64, 256, 64, 7, FALSE);

    drawRect(0, 0, 256, 128, 0);

#ifndef AGA5BPP
    startup();
#endif
    
    renderScene();

#ifndef AGA5BPP
    lastTime = getMilliseconds();
#endif

    int farthest = 0;


    for (int x = 0; x < XRES; x += 1) {
        if (farthest < stencilHigh[x]) {
            farthest = stencilHigh[x];
        }
    }

    if (stateTick > 10) {
        dirtyLineY0 = 0;
        dirtyLineY1 = nextDirtyLineY1;
        nextDirtyLineY1 = farthest + 1;
    } else {
        nextDirtyLineY1 = 127;
    }
    
    
    char buffer[256];
    sprintf(&buffer[0], "%zu", lastTime);
    drawTextAt(1, 1, &buffer[0], 5);
    
    if (viewMenu != -1) {
        int level = 0;
        while (level <= viewMenu) {

            int c;
            uint8_t optionsHeight = 8 * 4;
            biggestOption = 10;

            drawWindow(1 + (4 * level), 1, biggestOption, 5, "Action");

            for (c = 0; c < 4; ++c) {

                int isCursor = (level == viewMenu) && (cursorPosition == c)
                               && ((currentPresentationState == kConfirmInputBlink1)
                                   || (currentPresentationState == kConfirmInputBlink3)
                                   || (currentPresentationState == kConfirmInputBlink5)
                                   || (currentPresentationState == kWaitingForInput));

                if (isCursor) {
                    fill(1 + (8 * 4 * level), ((c + 1) * 8), (biggestOption * 8), 8, 0, FALSE);

                    drawTextAt(2 + (4 * level),
                               2 + c,
                               &options[c][0], 2);

                } else {
                    drawTextAt(2 + (4 * level),
                               2 + c,
                               &options[c][0], 4);

                }

            }

            ++level;
        }
    }
}

int32_t Interrogation_tickCallback(int32_t tag, void *data) {

    long delta = *((long *) data);

    timeUntilNextState -= delta;
    uint8_t prevX;
    uint8_t prevZ;

    if (currentPresentationState == kWaitingForInput) {

        prevX = cameraX;
        prevZ = cameraZ;

        if (viewMenu == -1) {
            switch (tag) {
                case kCommandLeft:
                    cameraRotation--;
                    if (cameraRotation < 0) {
                        cameraRotation = 3;
                    }
                    break;

                case kCommandRight:
                    cameraRotation = (cameraRotation + 1) & 3;
                    break;

                case kCommandBack:
                    return kMainMenu;

                case kCommandStrafeLeft:
                    cameraX-=2;
                    break;
                case kCommandStrafeRight:
                    cameraX+=2;
                    break;
                case kCommandDown:
                    cameraZ+=2;
                    break;
                case kCommandUp:
                    cameraZ-=2;
                    break;
                case kCommandFire1:
                    ++viewMenu;
                    break;
            }
        } else {
            switch (tag) {
                case kCommandDown:
                    cursorPosition++;
                    break;

                case kCommandBack:
                    viewMenu = -1;
                    break;

                case kCommandFire2:
                    --viewMenu;
                    break;

                case kCommandFire1:
                    ++viewMenu;
                    break;

                case kCommandUp:
                    cursorPosition--;
                    break;
            }
        }

        if (cameraZ >= 32) {
            cameraZ = 31;
        }

        if (cameraX >= 32) {
            cameraX = 31;
        }

        if (cameraZ < 0) {
            cameraZ = 0;
        }

        if (cameraX < 0) {
            cameraX = 0;
        }

        if (patterns[map[cameraZ - 2][cameraX]].ceiling < 2) {
            cameraX = prevX;
            cameraZ = prevZ;
        }
    }

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
                currentPresentationState = kWaitingForInput;
                break;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                timeUntilNextState = MENU_ITEM_TIME_TO_BLINK_MS;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }


    return -1;
}

void Interrogation_unloadStateCallback() {
    releaseBitmap(currentBackgroundBitmap);
}
