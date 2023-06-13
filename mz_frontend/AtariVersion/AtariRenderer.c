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

#define NORMALIZE(x) (((x * 8) / 256))

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


    framebuffer_set_palette_entry(0, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0x00));
    framebuffer_set_palette_entry(1, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(2, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0x00));
    framebuffer_set_palette_entry(3, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(4, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0x00));
    framebuffer_set_palette_entry(5, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(6, NORMALIZE(0xAA), NORMALIZE(0x55), NORMALIZE(0x00));
    framebuffer_set_palette_entry(7, NORMALIZE(0xAA), NORMALIZE(0xAA), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(8, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0x55));
    framebuffer_set_palette_entry(9, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(10, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0x55));
    framebuffer_set_palette_entry(11, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(12, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0x55));
    framebuffer_set_palette_entry(13, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(14, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0x55));
    framebuffer_set_palette_entry(15, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0xFF));
}


/*Same as above*/
void handleSystemEvents() {
    bufferInput = Cnecin();
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
    uint8_t *fragment = &framebuffer[0];
    unsigned lineOffset = 0;
    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {

            uint16_t *words = (uint16_t *) logBase;
            unsigned value = *fragment;
            unsigned offset = lineOffset + ((x >> 4) << 2);
            unsigned bitPattern = (1 << (15 - (x & 15)));

            if (value & 1) {
                words[offset] |= bitPattern;
            }

            offset++;

            if (value & 2) {
                words[offset] |= bitPattern;
            }

            offset++;

            if (value & 4) {
                words[offset] |= bitPattern;
            }

            offset++;

            if (value & 8) {
                words[offset] |= bitPattern;
            }

            ++fragment;
        }
        lineOffset += 80;
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
