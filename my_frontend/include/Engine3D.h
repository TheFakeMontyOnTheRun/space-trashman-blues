/*
   Created by Daniel Monteiro on 2021-10-22.
*/

#ifndef DERELICT8_ENGINE3D_H
#define DERELICT8_ENGINE3D_H

#define XRES 127
#define YRES 127

#define XRESMINUSONE (XRES - 1)
#define YRESMINUSONE (YRES - 1)

#define CAMERA_HEIGHT 4

enum GeometryType {
    CUBE = 0,
    RIGHT_NEAR = 4,
    LEFT_NEAR = 8,
    LEFT_WALL = 16,
    BACK_WALL = 32,
    CORNER = 64
};

struct Pattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType: 7;
    uint8_t blockMovement: 1;
};

void backToGraphics(void);

void clearGraphics(void);

void clearScreen(void);

void clearTextScreen(void);

void dropItem(void);

void enterTextMode(void);

void exitTextMode(void);

uint8_t getKey(void);

void graphicsFlush(void);

void graphicsPut(uint8_t x, uint8_t y);

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr);

void graphicsPutPointArray(uint8_t* y128Values);

void HUD_initialPaint(void);

void HUD_refresh(void);

void init(void);

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

void tickRenderer(void);

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

void refreshJustGraphics(void);

#endif /* DERELICT8_ENGINE3D_H */
