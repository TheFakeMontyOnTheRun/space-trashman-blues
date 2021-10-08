#ifdef SMD
#include <genesis.h>

uint8_t buffered = '.';

static void handleInput()
{
    u16 value;
    buffered = '.';

    // need to call it manually as we don't use SYS_doVBlankProcess() here
    JOY_update();

    value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_A)
    {
        if (value & BUTTON_LEFT)
        {

        }
        if (value & BUTTON_RIGHT)
        {

        }
    } else {
        if (value & BUTTON_UP)
        {
            buffered = 'w';
        }
        if (value & BUTTON_DOWN)
        {
            buffered = 's';
        }
        if (value & BUTTON_LEFT)
        {
            buffered = 'q';
        }
        if (value & BUTTON_RIGHT)
        {
            buffered = 'e';
        }
    }
}


static void joyEvent(u16 joy, u16 changed, u16 state)
{
    // START button state changed
    if (changed & BUTTON_START)
    {
        // START button pressed ?
        if (state & BUTTON_START)
        {
        }
    }

    if (changed & state & BUTTON_A)
    {

    }
    if (changed & state & BUTTON_B)
    {

    }

    // C button state changed
    if (changed & BUTTON_C)
    {
        // C button pressed ?
        if (state & BUTTON_C)
        {

        }
    }
}

void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
}

void shutdownGraphics() {
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg){
    BMP_clearText(1, nColumn, nLine);
    BMP_drawText(str, nColumn, nLine);
}

void graphicsPut( uint8_t x, uint8_t y) {
    BMP_setPixel(x, y, 0x11);
}


void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    for (int x = x0; x < x1; ++x ) {
        graphicsPut( x, y);
    }
}

void vLine(int x0, int y0, int y1) {
    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (int y = y0; y < y1; ++y ) {
        graphicsPut(x0, y);
    }
}

void clearGraphics() {
    BMP_waitWhileFlipRequestPending();
    BMP_clear();

}

uint8_t getKey() {
    handleInput();
    return buffered;
}

void puts(char *unused) {

}

void assert( int unused) {

}

void init() {
    char col;

    JOY_setEventHandler(joyEvent);
    VDP_setScreenWidth256();
    VDP_setHInterrupt(0);
    VDP_setHilightShadow(0);

    // reduce DMA buffer size to avoid running out of memory (we don't need it)
    DMA_setBufferSize(2048);

    // init Bitmap engine (require a ton shit of memory)
    BMP_init(TRUE, BG_A, PAL0, FALSE);


    col = 0xFF;
}

void graphicsFlush() {
    BMP_flip(1);
}
#endif
