#include <genesis.h>
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern int accessGrantedToSafe;

uint8_t buffered = '.';
int cursorPosition = 0;


char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

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




void writeStrWithLimit(int _x, int y, char *str, int limitX) {
    BMP_clearText(1, _x, y);
    BMP_drawText(str, _x, y);
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 31);
}

void graphicsPut( uint8_t x, uint8_t y) {
    BMP_setPixel(x, y, 0x11);
}


void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    for (int x = x0; x < x1; ++x ) {
        graphicsPut( x, y);
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (int y = y0; y < y1; ++y ) {
        graphicsPut(x0, y);
    }
}

void showMessage(const char *message) {
    writeStr(1, 1, message, 2, 0);
}

void clearScreen() {

}

void clearGraphics() {
    BMP_waitWhileFlipRequestPending();
    BMP_clear();

}

void drawWindow(int tx, int ty, int tw, int th, const char* title ) {}

uint8_t getKey() {
    handleInput();
    return buffered;
}



void titleScreen() {
    int keepGoing = 1;
    clearGraphics();

    writeStr(1, 1, "Space Mare Imperium:", 2, 0);
    writeStr(1, 2, "     Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 6, "  Press B button ", 2, 0);
    writeStr(1, 7, "    to start", 2, 0);

//    while (keepGoing) {
//        if (getKey() != '.') {
//            keepGoing = 0;
//        }
//    }

    clearGraphics();
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

void HUD_initialPaint() {
    struct Room *room = getRoom(getPlayerRoom());


    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(16, 14 + i, i == cursorPosition ? ">" : " ", 2, 0);
        writeStr(17, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void HUD_refresh() {

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(16, 14 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
    }

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(16, 21, "*", 2, 0);
        }

        writeStrWithLimit(17, 21, item->name, 30);
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStr(0, 1, "*", 2, 0);
        }

        writeStrWithLimit(1, 1, item->name, 14);
    }
}