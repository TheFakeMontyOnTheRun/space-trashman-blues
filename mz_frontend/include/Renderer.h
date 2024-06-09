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

#define STIPPLE_DISTANCE 13

#define STIPPLE_COLOUR_THRESHOLD 8


/*  not rendered and blocks visibility */
#define BLOCK_CELL '#'

struct CellPattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType: 7;
    uint8_t blockMovement: 1;
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

void graphicsPut(int16_t x, int16_t y, uint16_t colour);

void graphicsPutPointArray(uint8_t *y128Values);

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

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour);

void flush3DBuffer(void);

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const uint8_t colour);

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t colour,
                                       char charToReplaceHifenWith);

void realPut(int x, int y, uint8_t value);

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour);

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple);

void drawFloorAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dZ);

void drawCubeAt(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, int16_t dZ);

void
drawWedge(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, int16_t dZ, uint16_t elementMask, uint16_t type);

void drawSquare(int16_t x0, int16_t y0, int16_t z0, int16_t dX, int16_t dY, uint16_t elementMask);

void drawObjectAt(int16_t x0, int16_t z0);

void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour);

void repaintMapItems(void);

void drawPattern(uint16_t _pattern, int16_t x0, int16_t x1, int16_t z);

extern uint8_t needsToRedrawVisibleMeshes;

#endif /* DERELICT8_RENDERER_H */
