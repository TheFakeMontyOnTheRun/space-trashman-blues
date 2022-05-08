//
// Created by Daniel Monteiro on 2021-10-22.
//

#ifndef DERELICT8_ENGINE3D_H
#define DERELICT8_ENGINE3D_H

#ifdef RES128X64
#define XRES 128
#define YRES 64
#else
#ifdef RES96x64
#define XRES 96
#define YRES 64
#else
#ifdef RES64X128
#define XRES 64
#define YRES 128
#else
#ifdef RES128X128
#define XRES 128
#define YRES 128
#else
#define XRES 64
#define YRES 64
#endif
#endif
#endif
#endif

#define XRESMINUSONE XRES - 1
#define YRESMINUSONE YRES - 1


#define WALKSTEP 1
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

struct Pattern {
	uint8_t ceiling: 4;
	uint8_t elementsMask: 4;
	uint8_t geometryType : 7;
	uint8_t blockMovement : 1;
};

void HUD_initialPaint();

void HUD_refresh();

void tickRenderer();

void shutdownGraphics();

void clearGraphics();

void sleepForMS(uint32_t ms);

void clearScreen();

void writeStr(uint8_t nColumn, uint8_t nLine, const char *str, uint8_t fg, uint8_t bg);

uint8_t getKey();

void init();

void graphicsFlush();

void graphicsPut(uint8_t x, uint8_t y);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple);

void titleScreen();

void showMessage(const char *msg);

void printSituation();

void drawWindow(int tx, int ty, int tw, int th, const char *title);

#ifdef SMS
uint8_t* graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr);
#endif

#endif //DERELICT8_ENGINE3D_H
