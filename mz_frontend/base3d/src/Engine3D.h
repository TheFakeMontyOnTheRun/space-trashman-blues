//
// Created by Daniel Monteiro on 2021-10-22.
//

#ifndef DERELICT8_ENGINE3D_H
#define DERELICT8_ENGINE3D_H

#define XRES 128
#define YRES 128

#define XRESMINUSONE (XRES - 1)
#define YRESMINUSONE (YRES - 1)

void HUD_initialPaint(void);

void HUD_refresh(void);

void tickRenderer(void);

void shutdownGraphics(void);

void clearGraphics(void);

void sleepForMS(uint32_t ms);

uint8_t getKey(void);

void graphicsFlush(void);

void graphicsPut(int16_t x, int16_t y, uint16_t colour);

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour);

void hLine(int16_t x0, int16_t x1, int16_t y0, uint16_t colour);

void showMessage(const char *msg);

#endif //DERELICT8_ENGINE3D_H
