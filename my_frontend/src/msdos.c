#include <string.h>
#include <stdint.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

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

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

unsigned char imageBuffer[64 * 128];
unsigned char buffer[64 * 128];

void shutdownGraphics() {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t stipple = 1;

    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    x0 >>= 1;

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

    for (int y = y0; y < y1; ++y ) {

        if (shouldStipple) {
            stipple = !stipple;
        }

        if (!shouldStipple || stipple ) {
            imageBuffer[(64 * y) + x0] = colour;
        }
    }
}

void graphicsPut( uint8_t x, uint8_t y) {
    if (y > 127 ) {
        return;
    }

    x >>= 1;

    if (x > 63 ) {
        return;
    }

    imageBuffer[ (64 * y ) + x ] = 1;
}

void realPut( int x, int y, int value ) {
    int pixel = value;
    int px = x;
    int py = y;

    asm volatile ("movb $0x0C, %%ah\n\t"
                  "movb %0,    %%al\n\t"
                  "movb $0x0,  %%bh\n\t"
                  "movw %1,    %%cx\n\t"
                  "movw %2,    %%dx\n\t"
                  "int $0x10"
    :
    :"rm" (pixel), "rm" (px), "rm" (py)
    );
}

void clearGraphics() {
    memset(imageBuffer, 0, 64 * 128 );
}

void init() {
    asm("movb $0x0, %ah\n\t"
        "movb $0x4, %al\n\t"
        "int $0x10\n\t");
}

uint8_t getKey() {
    unsigned char toReturn = 255;


    asm volatile ("movb $0x00, %%ah\n\t"
                  "movb $0x00, %%al\n\t"
                  "int $0x16       \n\t"
                  "movb %%al, %0 "
    : "=rm"(toReturn)
    );

    asm volatile("movb $0x0C, %ah\n\t"
                 "movb $0x00, %al\n\t"
                 "int $0x21"
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
        "movb $0x02, %%ah\n"
        "movb    %0, %%dl\n"
        "movb    %1, %%dh\n"
        "movb  $0x0, %%bh\n"
        "int  $0x10\n"
        :
        : "rm" (x), "rm" (y)
        :
        );

        asm volatile (
        "movb $0x09, %%ah\n"
        "movb  %[c], %%al\n"
        "movw $0x01, %%cx\n"
        "movb  $0x0, %%bh\n"
        "movb $0x03, %%bl\n"
        "int $0x10\n"
        :
        :[c] "r"(cha)
        :
        );

        ++ptr;
    }
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void graphicsFlush() {
//    int origin = 0;
//    int lastOrigin = -1;
//    int value = -2;
//    int offset = 0;
//
//    for ( int y = 0; y < 128; ++y ) {
//
//
//        for ( int x = 0; x < 64; ++x ) {
//
//            origin = imageBuffer[ offset ];
//
//            if ( lastOrigin != origin ) {
//                value = origin;
//                lastOrigin = origin;
//            }
//
//
//            if ( buffer[ offset ] != value ) {
//                realPut( 16 + (2 * x), (y) + 36, value);
//                realPut( 16 + (2 * x) + 1, (y) + 36, value);
//            }
//
//            buffer[ offset ] = value;
//
//            ++offset;
//        }
//    }

    int offset = 0;
    for ( int y = 0; y < 100; ++y ) {
        for (int x = 0; x < 320; ++x) {

            uint8_t pixel = buffer[ (y) * 320 + x ];

            asm volatile (
                    "movw %0, %%ax\n"
                    "movw $0xB8000, %%ax\n"
                    "movb %1, 1(%%ax)\n"
            :
            :[c] "r"(offset), "r" (pixel)
            :
            );

            ++offset;
        }
    }


    memset( imageBuffer, 0, 64 * 128);
}

void showMessage(const char *message) {
    writeStr(1, 1, message, 2, 0);
}

void titleScreen() {

}

void HUD_initialPaint() {

    for ( int c = 15; c < (128 + 16 + 1); ++c ) {
        realPut( c, 35, 3);
        realPut( c, 36 + 128, 3);
    }

    for ( int c = 35; c < (128 + 36 + 1); ++c ) {
        realPut( 15, c, 3);
        realPut( 16 + 128, c, 3);
    }

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(21, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
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
            writeStr(22, 21, item->description, 2, 0);
        } else {
            writeStr(21, 21, item->description, 2, 0);
        }
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStr(1, 2, "*", 2, 0);
            writeStr(2, 2, item->description, 2, 0);
        } else {
            writeStr(1, 2, item->description, 2, 0);
        }
    }
}