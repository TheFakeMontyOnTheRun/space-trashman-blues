#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint16_t heap;

void shutdownGraphics() {
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg){}


void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {

    uint8_t _y0 = y0;
    uint8_t _y1 = y1;


    if (x0 >= 128) return;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    };


    if (_y0 >= 128) {
        _y0 = 127;
    };

    for (uint8_t y = _y0; y <= _y1; ++y) {
        plot(x0, y);
    }
}

void graphicsPut( uint8_t x, uint8_t y) {
    plot(x, y);
}

void clearGraphics() {
    clg();
}

uint8_t getch();

uint8_t getKey() {
  return getch();
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
