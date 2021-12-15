#ifdef SPECTRUM
#include <lib3d.h>
#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint16_t heap;

void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    draw(x0, y0, x1, y1 );
}

void shutdownGraphics() {
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg){}

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    draw(x0, y, x1, y );
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    draw(x0, y0, x0, y1 );
}

void graphicsPut( uint8_t x, uint8_t y) {
    plot(x, y);
}

void clearGraphics() {
    clg();
}

uint8_t getKey() {
  return getk();
}

void init() {
    clg();
}

void graphicsFlush() {
}

uint8_t* graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {
    return NULL;
}

void showMessage(const char *message) {

}

void titleScreen() {

}

void HUD_initialPaint() {

}


void HUD_refresh() {

}

#endif
