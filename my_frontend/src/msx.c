#ifdef MSX
#include <lib3d.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <msx/gfx.h>

uint8_t *heap;

uint8_t getch();


unsigned char* sbuffer;
surface_t surf;

void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    surface_draw(&surf, x0, y0, x1, y1 );
}

void shutdownGraphics() {
  // go back to text mode
  set_mode(mode_0);
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg){}

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {

    if (y > 127 ) {
        return;
    }

    if (x1 > 127 ) {
        x1 = 127;
    }

    if (x0 > 127 ) {
        x0 = 127;
    }


    surface_draw(&surf, x0, y, x1, y );
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {

    if (y1 > 127 ) {
        y1 = 127;
    }

    if (y0 > 127 ) {
        y0 = 127;
    }

    if (x0 > 127 ) {
        return;
    }

    surface_draw(&surf, x0, y0, x0, y1 );
}

void graphicsPut( uint8_t x, uint8_t y) {
    if (y > 127 ) {
        return;
    }

    if (x > 127 ) {
        return;
    }


    surface_draw(&surf, x, y, x, y );
}

void clearGraphics() {
   memset(sbuffer, 0, MODE2_MAX);
}

uint8_t getKey() {
  return getch();
}

void init() {
    sbuffer = (unsigned char*)malloc(MODE2_MAX);
    surf.data.ram = sbuffer;
    set_color(15, 1, 1);
    set_mode(mode_2);
    fill(MODE2_ATTR, 0xF1, MODE2_MAX);
}

void graphicsFlush() {
    msx_vwrite_direct(sbuffer, 0, MODE2_MAX);
}

void printSituation() {}

void showMessage(char *txt) {}

void titleScreen(){}

void HUD_initialPaint() {}

void HUD_refresh() {}
#endif
