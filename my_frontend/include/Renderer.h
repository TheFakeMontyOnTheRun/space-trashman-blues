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

struct CellPattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType: 7;
    uint8_t blockMovement: 1;
};

void clearGraphics(void);

void clearScreen(void);

void clearTextScreen(void);

void dropItem(void);

void initMap(void);

void shutdownGraphics(void);

enum ECommand getInput(void);

void graphicsFlush(void);

void graphicsPut(uint8_t x, uint8_t y);

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr);

void graphicsPutPointArray(uint8_t *y128Values);

void HUD_initialPaint(void);

void HUD_refresh(void);

void initHW(void);

void interactWithItemInRoom(void);

void nextItemInHand(void);

void nextItemInRoom(void);

void performAction(void);

void pickItem(void);

void pickOrDrop(void);

void printSituation(void);

void renderCameraNorth(void);

void renderCameraEast(void);

void renderCameraSouth(void);

void renderCameraWest(void);

void renderScene(void);

void showMessage(const char *msg);

void startMusic(void);

void shutdownGraphics(void);

void titleScreen(void);

void updateMapItems(void);

void useItemInHand(void);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple);

void writeStr(uint8_t column, uint8_t line, const char *str);

void flush3DBuffer(void);

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX, uint8_t fg, uint8_t bg);

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr);

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour);

void drawMap(void);

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour);

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title, uint8_t colour);

uint8_t drawCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask);

uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type);

uint8_t drawSquare(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, uint8_t elementMask);

uint8_t drawObjectAt(int8_t x0, int8_t z0);

void repaintMapItems(void);

uint8_t drawPattern(uint8_t _pattern, int8_t x0, int8_t x1, int8_t y);

extern uint8_t needs3dRefresh;

#endif /* DERELICT8_RENDERER_H */
