#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

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

unsigned char imageBuffer[128 * 32];

void shutdownGraphics() {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t stipple = 1;

    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    uint8_t colour;

    switch (shouldStipple) {
        case 0:
            shouldStipple = 0;
            colour = 2;
            break;

        case 1:
            shouldStipple = 1;
            colour = 2;
            break;

        case 2:
            shouldStipple = 0;
            colour = 3;
            break;

        case 3:
            shouldStipple = 1;
            colour = 3;
            break;
    }


    uint16_t offset = (32 * y0) + (x0 / 4);
    uint8_t dy = (y1 - y0);


    switch (x0 & 3) {
        case 3:
            while( dy--) {


                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple) {
                    uint8_t byteInVRAM = imageBuffer[offset];
                    byteInVRAM = (byteInVRAM & 0b11111100) | colour;
                    imageBuffer[offset] = byteInVRAM;
                }

                offset += 32;
            }
            break;
        case 2:
            colour = colour << 2;
            while( dy--) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple) {
                    uint8_t byteInVRAM = imageBuffer[offset];
                    byteInVRAM = (byteInVRAM & 0b11110011) | colour;
                    imageBuffer[offset] = byteInVRAM;
                }

                offset += 32;
            }
            break;
        case 1:
            colour = colour << 4;
            while( dy--) {
                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple) {
                    uint8_t byteInVRAM = imageBuffer[offset];
                    byteInVRAM = (byteInVRAM & 0b11001111) | colour;
                    imageBuffer[offset] = byteInVRAM;
                }

                offset += 32;
            }
            break;
        case 0:
            colour = colour << 6;
            while( dy--) {

                if (shouldStipple) {
                    stipple = !stipple;
                }

                if (stipple) {
                    uint8_t byteInVRAM = imageBuffer[offset];
                    byteInVRAM = (byteInVRAM & 0b00111111) | colour;
                    imageBuffer[offset] = byteInVRAM;
                }

                offset += 32;
            }
            break;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    if (y > 127) {
        return;
    }

    if (x > 127) {
        return;
    }

    uint8_t *ptrToByte = &imageBuffer[(32 * y) + (x / 4)];
    uint8_t byteInVRAM = *ptrToByte;

    switch (x & 3) {
        case 3:
            byteInVRAM = (byteInVRAM & 0b11111100) | 0b00000001;
            break;
        case 2:
            byteInVRAM = (byteInVRAM & 0b11110011) | 0b00000100;
            break;
        case 1:
            byteInVRAM = (byteInVRAM & 0b11001111) | 0b00010000;
            break;
        case 0:
            byteInVRAM = (byteInVRAM & 0b00111111) | 0b01000000;
            break;
    }
    *ptrToByte = byteInVRAM;
}

void realPut(int x, int y, int value) {

    int pixelRead = 0;
#ifndef __DJGPP__

    if (y & 1) {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %1, %%di  \n\t"
                     "xorw %%ax, %%ax\n\t"
                     "movb %%es:(%%di), %%al\n\t"
                     "movw %%ax, %0\n\t"
        : "=r"(pixelRead)
        : "r"( 0x2000 + ((x / 4) + ((y / 2) * 80)))
        : "ax", "es", "di"
        );
    } else {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %1, %%di  \n\t"
                     "xorw %%ax, %%ax\n\t"
                     "movb %%es:(%%di), %%al\n\t"
                     "movw %%ax, %0\n\t"
        : "=r"(pixelRead)
        : "r"((x / 4) + ((y / 2) * 80))
        : "ax", "es", "di"
        );
    }

    uint8_t pixel = pixelRead & 0xFFFF;

    switch (x & 3) {
        case 3:
            pixel = value | (pixel & 0b11111100);
            break;
        case 2:
            value = (value << 2);
            pixel = value | (pixel & 0b11110011);
            break;

        case 1:
            value = (value << 4);
            pixel = value | (pixel & 0b11001111);
            break;

        case 0:
            value = (value << 6);
            pixel = value | (pixel & 0b00111111);
            break;
    }

    value = pixel;

    if (y & 1) {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %0, %%di  \n\t"
                     "movb %1, %%es:(%%di)\n\t"
        :
        : "r"( 0x2000 + ((x / 4) + ((y / 2) * 80))), "r" (value)
        : "ax", "es", "di"
        );
    } else {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %0, %%di  \n\t"
                     "movb %1, %%es:(%%di)\n\t"
        :
        : "r"(((x / 4) + ((y / 2) * 80))), "r" (value)
        : "ax", "es", "di"
        );
    }
#else
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
#endif
}

void clearGraphics() {
    memset(imageBuffer, 0, 128 * 32);
}

void init() {

    asm volatile("movb $0x0, %%ah\n\t"
                 "movb $0x4, %%al\n\t"
                 "int $0x10\n\t"
    :
    :
    : "ax"
    );
}

void clearScreen() {
#ifndef __DJGPP__
    init();
#else
    uint8_t *mirrorVRAM = (uint8_t*)alloca( 320 * 100);
    memset(mirrorVRAM, 0, 320 * 100);
    dosmemput(mirrorVRAM, 320 * 100, (0xB800 * 16));
    dosmemput(mirrorVRAM, 320 * 100, (0xB800 * 16) + 0x2000);
#endif
}

uint8_t getKey() {
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

void writeStrWithLimit(int _x, int y, char *text, int limitX) {

    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t chary = 0;
    uint8_t x = _x;

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

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void drawWindow(int tx, int ty, int tw, int th, const char* title ) {}

void graphicsFlush() {
    uint8_t origin = 0;
    uint16_t value;
    int diOffset;
    uint8_t *bufferPtr = &imageBuffer[0];
    int index = 0;
    for (int y = 0; y < 128; ++y) {
        diOffset = ((y & 1) ? 0x2000 : 0x0) + (((y + 36) >> 1) * 80) + 4;
#ifndef __DJGPP__
        asm volatile(
        //save old values
        "pushw %%si\n\t"
        "pushw %%ds\n\t"

        //mimicking GCC move here.
        //making DS the same as SS
        "pushw %%ss\n\t"
        "popw %%ds\n\t"

        //set ES to point to VRAM
        "movw $0xb800, %%ax\n\t"
        "movw %%ax, %%es\n\t"

        //point to the correct offset inside VRAM
        "movw %0, %%di\n\t"

        //we will copy 32-bytes
        "movw $16, %%cx\n\t"

        //point SI to imageBuffer
        "movw %1, %%ax\n\t"
        "addw $imageBuffer, %%ax\n\t"
        "movw %%ax, %%si\n\t"

        //clear direction flag
        "cld\n\t"

        //copy the damn thing
        //DS:[SI] to ES:[DI], CX times
        "rep movsw\n\t"

        //restore previous values
        "popw %%ds\n\t"
        "popw %%si\n\t"

        :
        : "r"( diOffset ), "r"(index)
        : "ax", "cx", "es", "di"
        );
#else
        dosmemput(bufferPtr + index, 32, (0xB800 * 16) + diOffset);
#endif
        index += 32;
    }
}

void showMessage(const char *message) {
    writeStr(1, 1, message, 2, 0);
}

void titleScreen() {
    int keepGoing = 1;
    clearGraphics();

    writeStr(1, 1, "Space Mare Imperium:", 2, 0);
    writeStr(1, 2, "     Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 6, "  Press B button ", 2, 0);
    writeStr(1, 7, "    to start", 2, 0);

    while (keepGoing) {
        if (getKey() != '.') {
            keepGoing = 0;
        }
    }

    clearScreen();
}

void HUD_initialPaint() {

    for (int c = 15; c < (128 + 16 + 1); ++c) {
        realPut(c, 35, 3);
        realPut(c, 36 + 128, 3);
    }

    for (int c = 35; c < (128 + 36 + 1); ++c) {
        realPut(15, c, 3);
        realPut(16 + 128, c, 3);
    }

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(21, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void sleepForMS(uint32_t ms) {
    usleep(ms);
}

void HUD_refresh() {
    writeStr(21, 21, "                    ", 2, 0);
    writeStr(21, 22, "                    ", 2, 0);
    writeStr(1, 2, "                    ", 2, 0);
    writeStr(1, 3, "                    ", 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            writeStr(21, 21, "*", 2, 0);
            writeStr(22, 21, item->name, 2, 0);
        } else {
            writeStr(21, 21, item->name, 2, 0);
        }
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStr(1, 2, "*", 2, 0);
            writeStr(2, 2, item->name, 2, 0);
        } else {
            writeStr(1, 2, item->name, 2, 0);
        }
    }
}