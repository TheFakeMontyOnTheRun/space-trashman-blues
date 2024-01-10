#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "AtariInt.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <osbind.h>
#include <mint/sysbind.h>
#include <mint/osbind.h>

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

char *menuItems[] = {
        "8) Use/Toggle",
        "5) Use with...",
        "9) Use/pick...",
        "6) Drop",
        "7) Next item",
        "4) Next in room",
};

void graphicsFlush(void);

void nextItemInHand(void);

void useItemInHand(void);

void nextItemInRoom(void);

void interactWithItemInRoom(void);

void pickOrDrop(void);

void dropItem(void);

void pickItem(void);

void clearGraphics(void);

uint16_t *physBase;
uint16_t *logBase;

uint8_t *framebuffer;
uint8_t bufferInput = '.';

#define NORMALIZE(x) (((x * 8) / 256))

void framebuffer_set_palette_entry(int index, int red, int green, int blue) {
    *(uint16_t *) (0xffff8240 + (index * 2)) = blue | (green << 4) | (red << 8);
}

void init(void) {
    framebuffer = (uint8_t *) calloc(1, 128 * 128);

    physBase = Physbase();
    logBase = Logbase();
    memset(logBase, 0, 32000);
    memset(physBase, 0, 32000);
    Setscreen(-1, -1, 0);

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
void handleSystemEvents(void) {
    bufferInput = Cnecin();
}

uint8_t getKey(void) {
    handleSystemEvents();
    uint8_t toReturn = bufferInput;
    bufferInput = '.';
    return toReturn;
}

void clear(void) {}

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    framebuffer[(128 * y) + x] = colour;
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t pixel) {
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

void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0 || y >= 128) {
        return;
    }

    int16_t _x0 = x0;
    int16_t _x1 = x1;

    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }

    if (_x0 < 0) {
        _x0 = 0;
    }

    if (_x1 >= 128) {
        _x1 = 127;
    }

    uint8_t *ptr = &framebuffer[(128 * y) + _x0];
    for (int16_t x = _x0; x <= _x1; ++x) {
        *ptr++ = colour;
    }
}

void shutdownGraphics(void) {
}

void realPut(int x, int y, uint8_t value) {
}

void clearGraphics(void) {
    memset(framebuffer, 0, 128 * 128);
}

void clearScreen(void) {
}

void writeStrWithLimit(int _x, int y, const char *text, int limitX) {
}

void writeStr(int16_t _x, int16_t y, const char *text, uint16_t fg, uint16_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

void graphicsFlush(void) {
    memset(logBase, 0, 32000);
    uint8_t *index = &framebuffer[0];
    unsigned lineOffset = 0;
    uint16_t *words = (uint16_t *) logBase;

    for (uint16_t y = 127; y; y--) {
        for (uint16_t x = 0; x < 128; ++x) {

            unsigned value = *index++;

            if (value > 16) {
                if ((x + y) & 1) {
                    value = 0;
                } else {
                    value = value - 16;
                }
            }

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

void titleScreen(void) {
}

void HUD_initialPaint(void) {
}

void sleepForMS(uint32_t ms) {
}

void HUD_refresh(void) {
}
