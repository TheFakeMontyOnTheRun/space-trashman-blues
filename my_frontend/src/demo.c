#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XRES 64
#define YRES 128
#define XRESMINUSONE XRES - 1
#define YRESMINUSONE YRES - 1


#define WALKSTEP 2
#define CAMERA_HEIGHT 2
#define VISIBILITY_LIMIT 32

#ifdef CPC_PLATFORM
#include <cpctelera.h>

extern uint16_t baseScreen;
extern const uint16_t lineStart[YRESMINUSONE];
#endif

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

enum GeometryType {
    CUBE = 0,
    RIGHT_NEAR = 4,
    LEFT_NEAR = 8,
    LEFT_WALL = 16,
    BACK_WALL = 32,
    CORNER = 64,
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))


void shutdownGraphics();

void clearGraphics();

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg);

void graphicsPut(uint8_t x, uint8_t y);

uint8_t getKey();

void init();

void graphicsFlush();

void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void hLine(uint8_t x0, uint8_t x1, uint8_t y);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1);

int8_t stencilHigh[XRES];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

int playerLocation = 0;

struct Projection {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


struct Pattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType;
    uint8_t block;
};

const struct Projection projections[40] =
        {
                {	0	,	63	,	-64	},	//	1
                {	0	,	63	,	-32	},	//	2
                {	9	,	63	,	-21	},	//	3
                {	15	,	63	,	-16	},	//	4
                {	18	,	63	,	-12	},	//	5
                {	20	,	63	,	-10	},	//	6
                {	21	,	63	,	-9	},	//	7
                {	23	,	63	,	-8	},	//	8
                {	23	,	63	,	-7	},	//	9
                {	24	,	63	,	-6	},	//	10
                {	25	,	63	,	-5	},	//	11
                {	25	,	63	,	-5	},	//	12
                {	26	,	63	,	-4	},	//	13
                {	26	,	63	,	-4	},	//	14
                {	26	,	63	,	-4	},	//	15
                {	27	,	63	,	-4	},	//	16
                {	27	,	63	,	-3	},	//	17
                {	27	,	63	,	-3	},	//	18
                {	27	,	63	,	-3	},	//	19
                {	27	,	63	,	-3	},	//	20
                {	27	,	63	,	-3	},	//	21
                {	28	,	63	,	-2	},	//	22
                {	28	,	63	,	-2	},	//	23
                {	28	,	63	,	-2	},	//	24
                {	28	,	63	,	-2	},	//	25
                {	28	,	63	,	-2	},	//	26
                {	28	,	63	,	-2	},	//	27
                {	28	,	63	,	-2	},	//	28
                {	28	,	63	,	-2	},	//	29
                {	28	,	63	,	-2	},	//	30
                {	28	,	63	,	-2	},	//	31
                {	29	,	63	,	-2	},	//	32
                {	29	,	63	,	-1	},	//	33
                {	29	,	63	,	-1	},	//	34
                {	29	,	63	,	-1	},	//	35
                {	29	,	63	,	-1	},	//	36
                {	29	,	63	,	-1	},	//	37
                {	29	,	63	,	-1	},	//	38
                {	29	,	63	,	-1	},	//	39
                {	29	,	63	,	-1	},	//	40
        };

#include "map.h"


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

    if (px1z1 < 0 || px0z0 > XRESMINUSONE) {
        return 0;
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


        if (elementMask & 2) {
            if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                vLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]));
            }
        }

        if (elementMask & 1) {
            if (IN_RANGE(0, XRESMINUSONE, px1z1) && py0z1 > stencilHigh[px1z1]) {
                vLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]));
            }
        }
        

        /* The upper segment */
        x0 = px0z0;
        x1 = px1z1;

        if (x0 != x1) {
            int16_t upperY0 = py1z0;
            int16_t upperY1 = py1z1;
            int16_t upperDx = abs(x1 - x0);
            int16_t upperSx = x0 < x1 ? 1 : -1;
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
                    if (stencilHigh[x0] <= upperY0) {
#ifdef CPC_PLATFORM
                        
                            
                        unsigned char *pS;
                        unsigned char nByte = 0;
                            
                        pS = (unsigned char *) baseScreen + lineStart[upperY0] + (x0 >> 1);
                        nByte = *pS;
                            
                        if (x0 & 1) {
                            nByte &= 170;
                            nByte |= 64;
                        } else {
                            nByte &= 85;
                            nByte |= 128;
                        }
                            
                        *pS = nByte;
#else
                        graphicsPut(x0, upperY0);
#endif
                        
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
                if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0]);
                }
                
                if (IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0]);
                }
            }
        }
        
        /* Draw the horizontal outlines of z0 and z1 */
        
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
#ifdef CPC_PLATFORM
                        unsigned char *pS;
                        unsigned char nByte = 0;
                        
                        pS = (unsigned char *) baseScreen + lineStart[stencilHigh[x]] + (x >> 1);
                        nByte = *pS;
                        
                        if (x & 1) {
                            nByte &= 170;
                            nByte |= 64;
                        } else {
                            nByte &= 85;
                            nByte |= 128;
                        }
                        
                        *pS = nByte;
#else
                        graphicsPut(x, stencilHigh[x]);
#endif
                    }
                    stencilHigh[x] = py0z0;
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
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;

    uint8_t drawContour;

    if (z0 >= 32) {
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
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);

    if (px1z0 < 0 || px0z0 > XRESMINUSONE) {
        return 0;
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

    {
        int16_t x, x0, x1;

        if (drawContour) {
            if (elementMask & 2) {
                if (IN_RANGE(0, XRESMINUSONE, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0]);
                }

                if (IN_RANGE(0, XRESMINUSONE, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0]);
                }
            }

            if (elementMask & 1) {
                if (IN_RANGE(0, XRESMINUSONE, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                    vLine(px0z1, py0z1, stencilHigh[px0z1]);
                }

                if (IN_RANGE(0, XRESMINUSONE, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                    vLine(px1z1, py0z1, stencilHigh[px1z1]);
                }
            }
        }

        /* Draw the horizontal outlines of z0 and z1 */

        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, XRESMINUSONE, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
#ifdef CPC_PLATFORM
                        unsigned char *pS;
                        unsigned char nByte = 0;
                        
                        pS = (unsigned char *) baseScreen + lineStart[stencilHigh[x]] + (x >> 1);
                        nByte = *pS;
                        
                        if (x & 1) {
                            nByte &= 170;
                            nByte |= 64;
                        } else {
                            nByte &= 85;
                            nByte |= 128;
                        }
                        
                        *pS = nByte;
#else
                        graphicsPut(x, stencilHigh[x]);
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
#ifdef CPC_PLATFORM
                    unsigned char *pS;
                    unsigned char nByte = 0;
                    
                    pS = (unsigned char *) baseScreen + lineStart[stencilHigh[x]] + (x >> 1);
                    nByte = *pS;
                    
                    if (x & 1) {
                        nByte &= 170;
                        nByte |= 64;
                    } else {
                        nByte &= 85;
                        nByte |= 128;
                    }
                    
                    *pS = nByte;
#else
                    graphicsPut(x, stencilHigh[x]);
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
#ifdef CPC_PLATFORM
                            unsigned char *pS;
                            unsigned char nByte = 0;
                            
                            pS = (unsigned char *) baseScreen + lineStart[stencilHigh[x0]] + (x0 >> 1);
                            nByte = *pS;
                            
                            if (x0 & 1) {
                                nByte &= 170;
                                nByte |= 64;
                            } else {
                                nByte &= 85;
                                nByte |= 128;
                            }
                            
                            *pS = nByte;
#else
                            graphicsPut(x0, stencilHigh[x0]);
#endif
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
                    if (drawContour) {
#ifdef CPC_PLATFORM
                        unsigned char *pS;
                        unsigned char nByte = 0;
                        
                        pS = (unsigned char *) baseScreen + lineStart[stencilHigh[x0]] + (x0 >> 1);
                        nByte = *pS;
                        
                        if (x0 & 1) {
                            nByte &= 170;
                            nByte |= 64;
                        } else {
                            nByte &= 85;
                            nByte |= 128;
                        }
                        
                        *pS = nByte;
#else
                        graphicsPut(x0, stencilHigh[x0]);
#endif
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


uint8_t drawPattern(uint8_t pattern, uint8_t x0, uint8_t x1, uint8_t y) {

    int8_t diff = patterns[0].ceiling - patterns[pattern].ceiling;
    uint8_t type = patterns[pattern].geometryType;

    if (patterns[pattern].block) {
        return 0;
    }

    if (type == CUBE) {
        return drawCubeAt(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y, x1 - x0,
                          diff, 1, patterns[pattern].elementsMask);

    } else if (type == RIGHT_NEAR || type == LEFT_NEAR  ){


        return drawWedge(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y, x1 - x0,
                         diff, 1, patterns[pattern].elementsMask, type);
        
    } else if (type == LEFT_WALL  ){
        
        
        return drawWedge(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y,
                         0, diff, 1, patterns[pattern].elementsMask, LEFT_NEAR);
        
    } else if (type == BACK_WALL  ){
        
        
        return drawSquare(x0, patterns[pattern].ceiling - CAMERA_HEIGHT, y + 1,
                         x1 - x0, diff, patterns[pattern].elementsMask);
        
    }
    
    return 0;
}

void renderScene() {
#ifdef CPC_PLATFORM
    int8_t *stencilPtr;
    unsigned char *pS = NULL;
    unsigned char *lastPS = NULL;
    unsigned char nByte;
    uint8_t y;
    uint8_t lastY;
#endif
    
    uint8_t lastPattern, lastIndex;

    switch (cameraRotation) {
        case DIRECTION_N: {
            int8_t y;
            int8_t limit = max(cameraZ - VISIBILITY_LIMIT, 0);
            for (y = min(cameraZ - 3, 31); y >= limit; --y) {
                int8_t x;
                int8_t const *mapY = &maps[playerLocation][y][0];
                int8_t const *mapXY;
                int8_t minX = cameraX + 5 + ((cameraZ - 3) - y);
                int8_t maxX = 0;

                if (minX > 31) {
                    minX = 31;
                }

                lastIndex = cameraX;
                lastPattern = *(mapY + lastIndex);

                mapXY = &maps[playerLocation][y][lastIndex];
                for (x = lastIndex; x < minX - 1; ++x) {
                    uint8_t pattern;

                    pattern = *mapXY;

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            if (!drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y)) {
                                x = minX - 1;
                            }
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    ++mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                }

                lastIndex = cameraX - 1;
                
                if (!cameraX) {
                    lastIndex = 0;
                }

                lastPattern = *(mapY + lastIndex);

                mapXY = &maps[playerLocation][y][lastIndex];

                maxX = max(cameraX - 3 - ((cameraZ - 3) - y), 0);

                for (x = lastIndex; x >= maxX + 1; --x) {
                    uint8_t pattern;
                    pattern = *mapXY;

                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {

                            if (!drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y)) {
                                x = maxX + 1;
                            }

                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }

                    --mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                }
            }
        }
            break;

        case DIRECTION_E: {
            int8_t x;
            for (x = min(cameraX - 3, 31); x <= min(cameraX + 13, 31); ++x) {
                int8_t y;

                for (y = cameraZ; y <= min(cameraZ + (x - cameraX), 31); ++y) {
                    drawPattern(maps[playerLocation][y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ - (x - cameraX), 0); --y) {
                    drawPattern(maps[playerLocation][y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }

            }
        }
            break;

        case DIRECTION_S: {
            int8_t y;
            for (y = min(cameraZ + 3, 31); y <= min(cameraZ + 19, 31); ++y) {
                int8_t x;
                for (x = cameraX; x <= min(cameraX + (y - (cameraZ + 3)), 31); ++x) {
                    drawPattern(maps[playerLocation][y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }

                for (x = max(cameraX - 1, 0); x >= max(cameraX - (y - (cameraZ + 3)), 0); --x) {
                    drawPattern(maps[playerLocation][y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
            }
        }
            break;

        case DIRECTION_W: {
            int8_t x;
            for (x = max(cameraX, 0); x >= max(cameraX - 16, 0); --x) {
                int8_t y;
                for (y = cameraZ; y <= min(cameraZ - (x - (cameraX)), 31); ++y) {
                    drawPattern(maps[playerLocation][y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }

                for (y = max(cameraZ - 1, 0); y >= max(cameraZ + (x - (cameraX)), 0); --y) {
                    drawPattern(maps[playerLocation][y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
            }
        }
            break;
    }
    
#ifdef CPC_PLATFORM

    stencilPtr = &stencilHigh[0];
    lastY = 0xFF;
    
    for (uint8_t x = 0; x < ( (XRES / 2) - 1); ++x) {

        y = *stencilPtr;
        
        if (y != lastY) {
            lastPS = (unsigned char *) baseScreen + lineStart[y];
        }
        
        pS = lastPS + x;
        nByte = *pS;
        
        nByte &= 85;
        nByte |= 128;
        
        lastY = *(++stencilPtr);

        //if the line is the same, there is no need to write, recompute the same address and load the same byte
        if (y != lastY) {
            *pS = nByte;
            lastPS = (unsigned char *) baseScreen + lineStart[lastY];
            pS = lastPS + x;
            nByte = *pS;
        }
        
        nByte &= 170;
        nByte |= 64;
        
        *pS = nByte;
        ++stencilPtr;
    }
#else
    for (uint8_t x = 0; x < XRESMINUSONE; ++x) {
        graphicsPut(x, stencilHigh[x]);
    }
#endif
}

void tickRenderer() {
    uint8_t prevX;
    uint8_t prevZ;

#ifndef CPC_PLATFORM
    clearGraphics();
#endif
    vLine(XRES, 0, YRES);
    vLine(0, 0, YRES);
    hLine(0, XRES, 0);
    hLine(0, XRES, YRES);
    
    renderScene();

    graphicsFlush();
    memset(stencilHigh, 0, XRES);
#ifdef CPC_PLATFORM
    clearGraphics();
#endif
    prevX = cameraX;
    prevZ = cameraZ;

    waitkey:
    switch (getKey()) {
        case 'k':
            playerLocation = 0;
            break;

        case 'l':
            playerLocation = 1;
            break;

        case 'q':
            cameraRotation--;
            if (cameraRotation < 0) {
                cameraRotation = 3;
            }
            break;
        case 'e':
            cameraRotation = (cameraRotation + 1) & 3;
            break;
            
        case 'a':
            cameraX -= WALKSTEP;
            break;
        case 'd':
            cameraX += WALKSTEP;
            break;


        case 's':
            switch (cameraRotation) {
                case 0:
                    cameraZ += WALKSTEP;
                    break;
                case 1:
                    cameraX -= WALKSTEP;
                    break;
                case 2:
                    cameraZ -= WALKSTEP;
                    break;
                case 3:
                    cameraX += WALKSTEP;
                    break;
            }


            break;
        case 'w':
            switch (cameraRotation) {
                case 0:
                    cameraZ -= WALKSTEP;
                    break;
                case 1:
                    cameraX += WALKSTEP;
                    break;
                case 2:
                    cameraZ += WALKSTEP;
                    break;
                case 3:
                    cameraX -= WALKSTEP;
                    break;
            }
            break;

#ifndef XCODE_BUILD
#if !defined(SDLSW) || !defined(AMIGA)
        default:
            goto waitkey;
#endif
#endif
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

    if (patterns[maps[playerLocation][cameraZ - 2][cameraX]].ceiling < 2) {
        cameraX = prevX;
        cameraZ = prevZ;
    }
}


#ifdef XCODE_BUILD
int demoMain() {
    
    for ( int nLine = 0; nLine < 200; ++nLine) {
        printf("%d,\n", ((nLine & 248) * 10) + ((nLine & 7) << 11));
    }
#else

int main(int argc, char **argv) {
#endif


#ifdef CPC_PLATFORM
    cpct_setStackLocation((uint8_t*)0x8000);
#endif
    {
        running = 1;
        cameraX = 15;
        cameraZ = 6;
        cameraRotation = 0;
        init();

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
