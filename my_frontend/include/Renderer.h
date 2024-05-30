/*
   Created by Daniel Monteiro on 2021-10-22.
*/

#ifndef DERELICT8_RENDERER_H
#define DERELICT8_RENDERER_H

#define XRES 127
#define YRES 127

#define XRESMINUSONE (XRES - 1)
#define YRESMINUSONE (YRES - 1)

#define CAMERA_HEIGHT 4

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))

#define STIPPLE_DISTANCE 12
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

enum GeometryType {
    CUBE = 0,
    RIGHT_NEAR = 4,
    LEFT_NEAR = 8,
    LEFT_WALL = 16,
    BACK_WALL = 32,
    CORNER = 64
};

#ifndef NES
#define BITFIELD(n) : n
#else
#define BITFIELD(n) /* : n */
#endif

struct CellPattern {
    uint8_t ceiling BITFIELD(4);
    uint8_t elementsMask  BITFIELD(4);
    uint8_t geometryType BITFIELD(7);
    uint8_t blockMovement BITFIELD(1);
};

#define getPaletteEntry(c) (( (c) - 0xFF000000 ) % 3)

void clearGraphics(void);

void clearTextScreen(void);

void dropItem(void);

void initMap(void);

#ifdef EMIT_QUIT_OPTION
void shutdownGraphics(void);
#endif

enum ECommand getInput(void);

void endFrame(void);

void startFrame(int x, int y, int width, int height);

void graphicsPut(uint8_t x, uint8_t y);

void graphicsPutPointArray(int8_t *y128Values);

void HUD_initialPaint(void);

void HUD_refresh(void);

void initHW(int, char **pString);

void interactWithItemInRoom(void);

void nextItemInHand(void);

void nextItemInRoom(void);

void pickItem(void);

void renderCameraNorth(void);

void renderCameraEast(void);

void renderCameraSouth(void);

void renderCameraWest(void);

void renderScene(void);

void updateMapItems(void);

void useItemInHand(void);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple);

void flush3DBuffer(void);

void writeStrWithLimit(uint8_t _x, uint8_t y, const char *text, uint8_t limitX, uint8_t fg, uint8_t bg);

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr);

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour);

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple);

uint8_t drawCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask);

uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type);

uint8_t drawSquare(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, uint8_t elementMask);

uint8_t drawObjectAt(int8_t x0, int8_t z0);

void repaintMapItems(void);

uint8_t drawPattern(uint8_t _pattern, int8_t x0, int8_t x1, int8_t y);

extern uint8_t needsToRedrawVisibleMeshes;

#endif /* DERELICT8_RENDERER_H */
