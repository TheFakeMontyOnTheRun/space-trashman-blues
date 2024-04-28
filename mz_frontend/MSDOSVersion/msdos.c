#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

unsigned char imageBuffer[128 * 128];

void shutdownGraphics(void) {
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {

    if (x0 < 0) {
        return;
    }

    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    }

    for (y = _y0; y <= _y1; ++y) {
        imageBuffer[(128 * y) + x0] = colour;
    }
}


void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0) {
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

    for (int x = _x0; x <= _x1; ++x) {
        imageBuffer[(128 * y) + x] = colour;
    }
}

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    if (x < 0) {
        x = 0;
    }

    if (x >= 128) {
        x = 127;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= 128) {
        y = 127;
    }

    imageBuffer[(128 * y) + x] = colour;
}

void realPut(int x, int y, uint16_t value) {

    int pixel = value;
    int px = x;
    int py = y;

    asm volatile ("movb $0x0C, %%ah\n\t"
                  "movb %0,    %%al\n\t"
                  "movb $0x0,  %%bh\n\t"
                  "movw %1,    %%cx\n\t"
                  "movw %2,    %%dx\n\t"
                  "int $0x10\n\t"
            :
            :"rm" (pixel), "rm" (px), "rm" (py)
            : "ax", "bx", "cx", "dx"
            );
}

void clearGraphics(void) {
    memset(imageBuffer, 0, 128 * 128);
}

void init(void) {

    asm volatile("movb $0x0, %%ah\n\t"
                 "movb $0x0D, %%al\n\t"
                 "int $0x10\n\t"
            :
            :
            : "ax"
            );
}

void clearScreen(void) {
    init();
}

uint8_t getKey(void) {
    unsigned char toReturn = 255;


    asm volatile ("movb $0x00, %%ah\n\t"
                  "movb $0x00, %%al\n\t"
                  "int $0x16       \n\t"
                  "movb %%al, %0\n\t"
            : "=rm"(toReturn)
            :
            : "ax"
            );

    asm volatile("movb $0x0C, %%ah\n\t"
                 "movb $0x00, %%al\n\t"
                 "int $0x21\n\t"
            :
            :
            : "ax"
            );

    return toReturn;
}

void writeStrWithLimit(int _x, int y, const char *text, int limitX) {

    uint16_t len = strlen(text);
    const char *ptr = text;
    uint16_t c = 0;
    uint16_t chary = 0;
    uint16_t x = _x;

    for (; c < len && y < 25; ++c) {

        char cha = *ptr;

        if (x == limitX) {
            ++y;
            x = _x;
        } else if (cha == '\n') {
            ++y;
            x = _x;
            ++ptr;
            continue;
        } else {
            ++x;
        }

        asm volatile (
                "movb $0x02, %%ah\n\t"
                "movb    %0, %%dl\n\t"
                "movb    %1, %%dh\n\t"
                "movb  $0x0, %%bh\n\t"
                "int  $0x10\n\t"
                "movb $0x09, %%ah\n"
                "movb %2,    %%al\n"
                "movw $0x01, %%cx\n"
                "movb $0x0,  %%bh\n"
                "movb $0x03, %%bl\n"
                "int  $0x10\n\t"
                :
                : "rm" (x), "rm" (y), "rm"(cha)
                : "ax", "bx", "cx", "dx"
                );

        ++ptr;
    }
}

void writeStr(int16_t _x, int16_t y, const char *text, uint16_t fg, uint16_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

void graphicsFlush(void) {
    uint8_t *bufferPtr = &imageBuffer[0];

    for (int y = 0; y < 128; ++y) {
        for (int x = 0; x < 128; ++x) {
            uint16_t index = *bufferPtr;

            realPut(16 + x, 36 + y, index);
            bufferPtr++;
        }
    }
}

void showMessage(const char *message) {
    writeStr(1, 1, message, 2, 0);
}

void titleScreen(void) {
}

void HUD_initialPaint(void) {
}

void sleepForMS(uint32_t ms) {
    usleep(ms);
}

void HUD_refresh(void) {
}
