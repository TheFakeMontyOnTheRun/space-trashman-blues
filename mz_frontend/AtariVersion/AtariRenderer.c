#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <osbind.h>
#include <mint/sysbind.h>

#include "AtariInt.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mint/osbind.h>
#include <mint/sysbind.h>

uint16_t *physBase;
uint16_t *logBase;

uint8_t *framebuffer;
uint8_t bufferInput = '.';



void putStr(int x, int y, const char *str, int fg, int bg) {}

void drawTitleBox() {}

void querySoundDriver() {
}

void framebuffer_set_palette_entry(int index, int red, int green, int blue) {
    *(uint16_t *) (0xffff8240 + (index * 2)) = blue | (green << 4) | (red << 8);
}

void init() {
    framebuffer = (uint8_t *) calloc(1, 128 * 128);

    physBase = Physbase();
    logBase = Logbase();
    memset(logBase, 0, 32000);
    memset(physBase, 0, 32000);
    Setscreen(-1, -1, 0);
    querySoundDriver();
}


/*Same as above*/
void handleSystemEvents() {
    bufferInput = getchar();
}

uint8_t getKey() {
    handleSystemEvents();
    uint8_t toReturn = bufferInput;
    bufferInput = '.';
    return toReturn;
}

void clear() {}


void graphicsPut(int16_t x, int16_t y) {
    framebuffer[(128 * y) + x] = 2;
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel) {
    uint8_t *ptr;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (x0 < 0 || x0 >= 128) {
        return;
    }

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 >= 128 || _y1 < 0) {
        return;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    }

    ptr = &framebuffer[(128 * _y0) + (x0)];

    for (int16_t y = _y0; y <= _y1; ++y) {
        *ptr = pixel;
        ptr += 128;
    }
}

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
int cursorPosition = 0;

char *menuItems[] = {
        "8) Use/Toggle",
        "5) Use with...",
        "9) Use/pick...",
        "6) Drop",
        "7) Next item",
        "4) Next in room",
};

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

void shutdownGraphics() {
}

void realPut(int x, int y, uint8_t value) {
    int posX = x / 16;

    uint16_t *words = (uint16_t*)logBase;

    uint16_t word = words[(y * 80) + posX + (posX * 4) ];

    word = word | (1 << (15 - ( x & 15)));

    words[(y * 80) + posX + (posX * 4) ] = word;
}

void clearGraphics() {
    memset(framebuffer, 0, 128 * 128);
}

void clearScreen() {
}


void writeStrWithLimit(int _x, int y, const char *text, int limitX) {
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

void graphicsFlush() {
    memset(logBase, 0, 32000);
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            if (framebuffer[128 * y + x]) {
                realPut(x, y, 1);
            }
        }
    }

    uint16_t *tmp;
    tmp = physBase;
    physBase = logBase;
    logBase = tmp;
    Setscreen(logBase, physBase, -1);
}


void showMessage(const char *message) {

}

void titleScreen() {
}

void HUD_initialPaint() {
}

void sleepForMS(uint32_t ms) {
}

void HUD_refresh() {
}
