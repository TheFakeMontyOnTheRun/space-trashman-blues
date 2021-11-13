#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

int cursorPosition = 0;

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
    asm("movb $0x0, %ah\n\t"
        "movb $0x3, %al\n\t"
        "int $0x10\n\t");

    puts("Thanks for playing!");
    exit(0);
}

void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

}

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    for (int x = x0; x < x1; ++x ) {
        imageBuffer[(64 * y) + x] = 1;
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    for (int y = y0; y < y1; ++y ) {
        imageBuffer[(64 * y) + x0] = 1;
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

    asm volatile ("movb $0x01, %%ah\n\t"
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
        init();
        HUD_initialPaint();
        HUD_refresh();
    }

    return toReturn;
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *pStr, uint8_t fg, uint8_t bg){

    init();

    while(*pStr) {
        __asm__ __volatile__ (
        "movb 0x0e, %%ah\n"
        "movb %[c], %%al\n"
        "int $0x10\n"
        :
        : [c] "r" (*pStr)
        : "ax"
        );
        ++pStr;
    }

    HUD_initialPaint();
}

unsigned char getPaletteEntry( int origin ) {
  return origin;
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
                value = getPaletteEntry( origin );
                lastOrigin = origin;
            }


            if ( buffer[ offset ] != value ) {
                realPut( 32 + (2 * x), (y) + 36, value);
                realPut( 32 + (2 * x) + 1, (y) + 36, value);
            }

            buffer[ offset ] = value;

            ++offset;
        }
    }

    memset( imageBuffer, 0, 64 * 128);
}

uint8_t* graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {
    return NULL;
}

void showMessage(const char *message) {

}

void titleScreen() {

}

void HUD_initialPaint() {
    for ( int y = 0; y < 200; ++y ) {
        realPut( 159, y, 2);
    }
}

void HUD_refresh() {
    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(20, 14 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
    }

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            writeStr(20, 21, "*", 2, 0);
        }

        writeStr(21, 21, item->description);
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStr(0, 1, "*", 2, 0);
        }

        writeStr(1, 1, item->description);
    }
}