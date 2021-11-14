#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <conio.h>

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

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (int y = y0; y < y1; ++y ) {
        imageBuffer[(64 * y) + x0] = 2;
    }
}

void graphicsPut( uint8_t x, uint8_t y) {
    if (y > 127 ) {
        return;
    }

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


    if (toReturn >= '1' && toReturn <= '9') {
        HUD_refresh();
    }

    return toReturn;
}

void writeStrWithLimit(int x, int y, char *text, int limitX) {
    gotoxy(x, y);
    printf(text);
}

void writeStr(int _x, int y, char *text, int fg, int bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void graphicsFlush() {
    int origin = 0;
    int lastOrigin = -1;
    int value = -2;
    int offset = 0;

    for ( int y = 0; y < 128; ++y ) {


        for ( int x = 0; x < 64; ++x ) {

            origin = imageBuffer[ offset ];

            if ( lastOrigin != origin ) {
                value = origin;
                lastOrigin = origin;
            }


            if ( buffer[ offset ] != value ) {
                realPut( 16 + (2 * x), (y) + 36, value);
                realPut( 16 + (2 * x) + 1, (y) + 36, value);
            }

            buffer[ offset ] = value;

            ++offset;
        }
    }

    memset( imageBuffer, 0, 64 * 128);
}

void showMessage(const char *message) {
}

void titleScreen() {

}

void HUD_initialPaint() {
    for ( int y = 0; y < 200; ++y ) {
        realPut( 159, y, 3);
    }

    for ( int c = 15; c < (128 + 16 + 1); ++c ) {
        realPut( c, 35, 3);
        realPut( c, 36 + 128, 3);
    }

    for ( int c = 35; c < (128 + 36 + 1); ++c ) {
        realPut( 15, c, 3);
        realPut( 16 + 128, c, 3);
    }

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(22, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void HUD_refresh() {

    for (int c = 0; c < 20; ++c ) {
        writeStr(1 + c, 1, " ", 2, 0);
        writeStr(1 + c, 2, " ", 2, 0);
        writeStr(22 + c, 21, " ", 2, 0);
        writeStr(22 + c, 22, " ", 2, 0);
    }

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            writeStr(22, 21, "*", 2, 0);
        }

        writeStr(23, 21, item->description, 2, 0);
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStr(2, 2, "*", 2, 0);
        }

        writeStr(3, 2, item->description, 2, 0);
    }
}