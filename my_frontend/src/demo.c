#ifndef SMD

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#else
#include <genesis.h>
#endif

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "map.h"

#ifdef SUPPORTS_HACKING_MINIGAME
#include "HackingMinigame.h"
#endif

#ifndef EMBEDDED_DATA
#include "Common.h"
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
#define NEUTRAL_CELL '.'

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
	uint8_t py;
	int8_t dx;
};

const struct Projection projections[31] =
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
				{	0	,	127	,	-128	},	//	1
				{	0	,	127	,	-64	},	//	2
				{	21	,	106	,	-43	},	//	3
				{	32	,	95	,	-32	},	//	4
				{	38	,	89	,	-26	},	//	5
				{	43	,	84	,	-21	},	//	6
				{	46	,	81	,	-18	},	//	7
				{	48	,	79	,	-16	},	//	8
				{	50	,	77	,	-14	},	//	9
				{	51	,	76	,	-13	},	//	10
				{	52	,	75	,	-12	},	//	11
				{	53	,	74	,	-11	},	//	12
				{	54	,	73	,	-10	},	//	13
				{	55	,	72	,	-9	},	//	14
				{	55	,	72	,	-9	},	//	15
				{	56	,	71	,	-8	},	//	16
				{	56	,	71	,	-8	},	//	17
				{	57	,	70	,	-7	},	//	18
				{	57	,	70	,	-7	},	//	19
				{	58	,	69	,	-6	},	//	20
				{	58	,	69	,	-6	},	//	21
				{	58	,	69	,	-6	},	//	22
				{	58	,	69	,	-6	},	//	23
				{	59	,	68	,	-5	},	//	24
				{	59	,	68	,	-5	},	//	25
				{	59	,	68	,	-5	},	//	26
				{	59	,	68	,	-5	},	//	27
				{	59	,	68	,	-5	},	//	28
				{	60	,	67	,	-4	},	//	29
				{	60	,	67	,	-4	},	//	30
				{	60	,	67	,	-4	},	//	31
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


#ifndef SMD

int8_t max(int8_t x1, int8_t x2) {
	return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
	return x1 < x2 ? x1 : x2;
}

#endif

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

#ifndef USE_FILLED_POLYS
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
#else
#ifndef MSDOS
	uint8_t shouldStipple;

	if (type == LEFT_WALL) {
		shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 12 : 4;
	} else {
		shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 0 : 12;
	}
#else
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 0 : 5;
#endif
#endif

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


#ifndef USE_FILLED_POLYS
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

			while (x0 != x1) {

#ifndef USE_FILLED_POLYS
				if (shouldStipple) {
					stipple = !stipple;
				}
#endif

				if (IN_RANGE(0, XRESMINUSONE, x0)) {
#ifndef USE_FILLED_POLYS
					if (stipple && stencilHigh[x0] <= upperY0) {
						graphicsPut(x0, upperY0);
					}

#endif

					if (stencilHigh[x0] < lowerY0) {
#ifdef USE_FILLED_POLYS
						uint8_t top = max( upperY0, stencilHigh[x0]);
						vLine(x0, top, lowerY0, shouldStipple);
						graphicsPut(x0, top);
#endif
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

#ifdef USE_FILLED_POLYS
	if (elementMask & 2) {
			if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
				vLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]), 0);

			}
		}

		if (elementMask & 1) {
			if (IN_RANGE(0, XRESMINUSONE, px1z1) && py0z1 > stencilHigh[px1z1]) {
				vLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]), 0);
			}
		}
#endif

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


#ifndef USE_FILLED_POLYS
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
#else
#ifdef MSDOS
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 5 : 1;
#else
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 12 : 4;
#endif
#endif

	uint8_t stipple = 1;

	drawContour = (dY);
	{
		int16_t x;

#ifndef USE_FILLED_POLYS
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
#endif

		/* Draw the horizontal outlines of z0 and z1 */

		/* Ceiling is lower than camera */
		for (x = px0z0; x <= px1z0; ++x) {
			if (IN_RANGE(0, XRESMINUSONE, x)) {

#ifndef USE_FILLED_POLYS
				if (shouldStipple) {
					stipple = !stipple;
				}
#endif

#ifndef USE_FILLED_POLYS
				if (stencilHigh[x] <= py1z0) {
					if (drawContour && stipple) {
						graphicsPut(x, py1z0);
					}
					stencilHigh[x] = py1z0;
				}


				if (stencilHigh[x] <= py0z0) {
					stencilHigh[x] = py0z0;
				}
#else
				if (drawContour && stencilHigh[x] <= py0z0) {
					vLine(x, max( py1z0, stencilHigh[x]), py0z0, shouldStipple);
					stencilHigh[x] = py0z0;
				}
#endif
			}
		}

#ifdef USE_FILLED_POLYS
		if (drawContour) {
			if (elementMask & 2) {
				if ((elementMask != 255) && IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
					vLine(px0z0, py0z0, stencilHigh[px0z0] < py1z0 ? py1z0 : stencilHigh[px0z0], 0 );

				}

				if ((elementMask != 127) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
					vLine(px1z0, py0z0, stencilHigh[px1z0] < py1z0 ? py1z0 : stencilHigh[px1z0], 0 );
				}
			}
		}
#endif
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

	py0z0 = z0py + ((-CAMERA_HEIGHT) * z0dx);
	py0z1 = z1py + ((-CAMERA_HEIGHT) * z1dx);


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

#ifndef USE_FILLED_POLYS
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE);
#else
#ifndef MSDOS
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 9 : 1;
#else
	uint8_t shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 6 : 2;
#endif
#endif
	uint8_t stipple = 1;

	uint8_t drawContour;

	if (z0 >= 32 || z0 <= 4) {
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

	{
		int16_t x, x0, x1;

#ifndef USE_FILLED_POLYS
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

#else
		for (x = px0z0; x <= px1z0; ++x) {
			if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
				if (drawContour) {
					vLine(x, stencilHigh[x], py0z0, shouldStipple);
				}
				stencilHigh[x] = py0z0;
			}
		}

#ifdef MSDOS
		shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 0 : 6;
#else
		shouldStipple = (z0 >= STIPPLE_DISTANCE) ? 0 : 9;
#endif

#endif
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
#ifndef USE_FILLED_POLYS
					if (shouldStipple) {
						stipple = !stipple;
					}
#endif

					if (stencilHigh[x0] < y0) {
#ifndef USE_FILLED_POLYS
						if (drawContour && stipple) {
							graphicsPut(x0, stencilHigh[x0]);
						}
#endif

#ifdef USE_FILLED_POLYS
						if (drawContour) {
							uint8_t top = stencilHigh[x0];
#ifdef MSDOS
							vLine(x0, top, y0, 6);
#else
							vLine(x0, top, y0, shouldStipple);
#endif
							graphicsPut(x0, top);
						}
#endif
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

#ifndef USE_FILLED_POLYS
					if (shouldStipple) {
						stipple = !stipple;
					}
#endif

#ifndef USE_FILLED_POLYS
					if (drawContour && stipple) {
						graphicsPut(x0, stencilHigh[x0]);
					}
#endif

#ifdef USE_FILLED_POLYS
					if (drawContour) {
						uint8_t top = stencilHigh[x0];
#ifdef MSDOS
						vLine(x0, top, y0, 6);
#else
						vLine(x0, top, y0, shouldStipple);
#endif
						graphicsPut(x0, top);
					}
#endif
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
#ifdef USE_FILLED_POLYS
		if (drawContour) {
			if (elementMask & 2) {
				if ((elementMask != 255 ) &&IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
					vLine(px0z0, py0z0, stencilHigh[px0z0], 0);
				}

				if ( (elementMask != 127 ) && IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
					vLine(px1z0, py0z0, stencilHigh[px1z0], 0);
				}
			}

			if (elementMask & 1) {
				if ((elementMask != 255 ) &&IN_RANGE(0, XRESMINUSONE, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
					vLine(px0z1, py0z1, stencilHigh[px0z1], 0);
				}

				if ((elementMask != 127 ) && IN_RANGE(0, XRESMINUSONE, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
					vLine(px1z1, py0z1, stencilHigh[px1z1], 0);
				}
			}
		}
#endif
		if (py0z0 <= py0z1) {
			/* Ceiling is higher than the camera*/
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
	}

	return 1;
}


uint8_t drawPattern(uint8_t _pattern, int8_t x0, int8_t x1, int8_t y) {
	int8_t diff;
	uint8_t pattern = (_pattern - RLE_THRESHOLD) & 127;
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
						  diff, 1, mask);
	} else if (type == RIGHT_NEAR || type == LEFT_NEAR) {

		if (cameraRotation == 1 || cameraRotation == 3) {

			if (type == RIGHT_NEAR) {
				type = LEFT_NEAR;
			} else {
				type = RIGHT_NEAR;
			}
		}

		return drawWedge(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2, x1 - x0,
						 diff, 1, patterns[pattern].elementsMask, type);

	} else if (type == LEFT_WALL) {

		switch (cameraRotation) {
			case 0:
			case 2:
				return drawWedge(x0 - (cameraRotation == 0 ? 1 : 0), patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
								 0, diff, 1, patterns[pattern].elementsMask, LEFT_WALL);
			case 1:
			case 3:
				return drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT,
								  y + (cameraRotation == 3 ? 1 : 0) + 2,
								  x1 - x0, diff, mask);
		}
	} else if (type == BACK_WALL) {
		switch (cameraRotation) {
			case 0:
			case 2:
				return drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT,
								  y + (cameraRotation == 0 ? 1 : 0) + 2,
								  x1 - x0, diff, mask);
			case 1:
			case 3:
				return drawWedge(x0 - (cameraRotation == 1 ? 1 : 0),
								 patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
								 0, diff, 1, patterns[pattern].elementsMask, LEFT_WALL);
		}
	} else if (type == CORNER) {
		uint8_t returnVal = 0;

		switch (cameraRotation) {

			case 3:
			case 0:
				returnVal = drawWedge(x0 - (cameraRotation == 3 ? 0 : 1),
									  patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
									  0, diff, 1, patterns[pattern].elementsMask, LEFT_WALL);

				returnVal = drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 1 + 2,
									   x1 - x0, diff, patterns[pattern].elementsMask) || returnVal;
				break;

			case 1:
			case 2:
				returnVal = drawSquare(x0 - 1, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
									   x1 - x0, diff, patterns[pattern].elementsMask);
				returnVal =
						drawWedge(x0 - (cameraRotation == 1 ? 1 : 0), patterns[pattern].ceiling - CAMERA_HEIGHT, y + 2,
								  0, diff, 1, patterns[pattern].elementsMask, LEFT_WALL) || returnVal;

				break;
		}

		return returnVal;
	}

	return 0;
}

#ifdef TRACE_OBJECTS_OVER_FLOOR

void repaintMapItems(void) {
	struct ObjectNode *node;

	/* ignore header node */
	node = getRoom(playerLocation)->itemsPresent->next;

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

#endif

/* all those refactors are due to a SDCC bug with very long functions */
void renderScene(void) {
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
	  uint8_t y, prevY, c;
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
	int8_t *stencilPtr = &stencilHigh[0];

	for (x = 0; x < XRES; ++x) {
		int8_t stencilY = (*stencilPtr);
#ifdef USE_FILLED_POLYS
#ifdef MSDOS
		if (stencilY > 86) {
			vLine(x, stencilY, 128, 3);
		} else {
			vLine(x, stencilY, 86, 7);
			vLine(x, 86, 128, 3);
		}
#else
		if (stencilY > 86) {
			vLine(x, stencilY, 128, 2);
		} else {
			vLine(x, stencilY, 86, 10);
			vLine(x, 86, 128, 2);
		}
#endif
#else
		graphicsPut(x, stencilY);
#endif
		++stencilPtr;
	}
#endif

#ifdef TRACE_OBJECTS_OVER_FLOOR
	repaintMapItems();
#endif
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

void updateMapItems(void);

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

	//the last location
	if (dataPositions[playerLocation + 1] == 0 ) {
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

	memset(map, NEUTRAL_CELL, MAP_SIZE_X * MAP_SIZE_Y);

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
		++head; // line break
#endif
	}

	done_loading:
	updateMapItems();
	HUD_initialPaint();
}

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
void startRoomTransitionAnimation(void) {
  uint8_t x,y;

  for (y = MAP_SIZE_Y; y >= 2; --y ) {
		vLine(y, y, 95 + (MAP_SIZE_Y - y), 1);
		vLine(95 + (MAP_SIZE_Y - y), y, 95 + (MAP_SIZE_Y - y), 1);

		for (x = y; x < (95 + (MAP_SIZE_Y - y)); ++x) {
			graphicsPut(x, y);
			graphicsPut(x, 95 + (MAP_SIZE_Y - y));
			//door opening

#ifdef MSDOS
			vLine(x, y, 95 - 3 * (MAP_SIZE_Y - y), 7);
#else
#ifndef USE_FILLED_POLYS
			graphicsPut(x, 95 - 3 * (MAP_SIZE_Y - y));
#else
			if (y > STIPPLE_DISTANCE) {
				vLine(x, y, 95 - 3 * (MAP_SIZE_Y - y), 12);
			} else {
				vLine(x, y, 95 - 3 * (MAP_SIZE_Y - y), 4);
			}

			vLine(x, 95 - 3 * (MAP_SIZE_Y - y), 95, 10);
			vLine(x, 95, 95 + (MAP_SIZE_Y - y), 2);
#endif
#endif
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
#ifndef SMS
#ifdef SMD
	writeStr(1,1, mesg, 1, 2);
#else
	puts(mesg);
#endif
#else
	showMessage(mesg);
#endif
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

	titleScreen();

	focusedItem = getPlayerItems();
	setErrorHandlerCallback(onError);
	setLoggerDelegate(logDelegate);
	initMap();

	memset(stencilHigh, 0, XRES);

	do {
		tickRenderer();
	} while (running);

	shutdownGraphics();

	return 0;
}
