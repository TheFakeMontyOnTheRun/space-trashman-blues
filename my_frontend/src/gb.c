#ifdef GB


#include <gb/gb.h>
#include <gb/drawing.h>



void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
}

void shutdownGraphics() {
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


    line(x0, y, x1, y );
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

    line(x0, y0, x0, y1 );
}

void graphicsPut( uint8_t x, uint8_t y) {
    if (y > 127 ) {
        return;
    }

    if (x > 127 ) {
        return;
    }


    plot(x, y, BLACK, SOLID);
}

void clearGraphics() {
    box( 0, 0, 127, 127, M_FILL);
}

uint8_t getKey() {
    wait_vbl_done();
    int key = joypad();
    if(key & (J_UP|J_DOWN|J_LEFT|J_RIGHT)) {

        if(key & J_UP) {
            return 'w';
        }

        if(key & J_DOWN) {
            return 's';
        }

        if(key & J_LEFT) {
            return 'a';
        }

        if(key & J_RIGHT) {
            return 'd';
        }
    }
  return '.';
}

void init() {
    OBP1_REG = 0xE0U;
}

void graphicsFlush() {
    wait_vbl_done();

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
#endif
