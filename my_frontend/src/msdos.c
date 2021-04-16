#ifdef MSDOS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

unsigned char imageBuffer[64 * 128];
unsigned char buffer[64 * 128];



void shutdownGraphics() {
    asm("movb $0x0, %ah\n\t"
        "movb $0x3, %al\n\t"
        "int $0x10\n\t");

    puts("Thanks for playing!");
    exit(0);
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *pStr, uint8_t fg, uint8_t bg){
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
}
gfdgdf

void fix_line (uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

}

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    for (int x = x0; x < x1; ++x ) {
        imageBuffer[(64 * y) + x] = 1;
    }
}

void vLine(int x0, int y0, int y1) {
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

    if (x > 64 ) {
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

    return toReturn;
}

void init() {
    asm("movb $0x0, %ah\n\t"
        "movb $0x4, %al\n\t"
        "int $0x10\n\t");
}

unsigned char getPaletteEntry( int origin ) {
  return origin;

  unsigned char shade = 0;

  if ( origin > 0 ) {
    return 3;
  } else {
    return 0;
  }
}

void graphicsFlush() {
    int origin = 0;
    int lastOrigin = -1;
    int value = -2;
    int offset = 0;

    for ( int y = 0; y < 128; ++y ) {


        for ( int x = 0; x < 64; ++x ) {


            offset = (y * 64) + x;
            origin = imageBuffer[ offset ];

            if ( lastOrigin != origin ) {
                value = getPaletteEntry( origin );
                lastOrigin = origin;
            }


            if ( buffer[ offset ] != value ) {
                realPut( (x), (y), value);
            }

            buffer[ offset ] = value;
        }
    }

    memset( imageBuffer, 0, 64 * 128);
}
#endif




#ifdef DONOTCOMPILE
/// VIANA

//
// Created by monty on 01-07-2017.
//
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>

#include "Renderer.h"

clock_t timeRendering = 0;
int desiredTimeSlice = 75;

clock_t t0;
clock_t t1;
clock_t ms;

uint8_t evenBuffer[320 * 100 / 4];
uint8_t oddBuffer[320 * 100 / 4];

EVideoType videoType = kCGA;

extern "C" struct ControlState getControlState() {
    ControlState toReturn;
    memset(&toReturn, 0, sizeof(struct ControlState));

    while (kbhit()) {
        auto getched = getch();
        switch (getched) {
            case 27:
                toReturn.escape = true;
                break;
            case 'z':
                toReturn.jump = true;
                break;
            case 'x':
                toReturn.fireArrow = true;
                break;
            case 'c':
            case ' ':
                toReturn.sword = true;
                break;
            case 13:
                toReturn.enter = true;
                break;
            case 224:
            case 0:
                auto arrow = getch();
                switch (arrow) {
                    case 75:
                        toReturn.moveLeft = true;
                        break;
                    case 72:
                        toReturn.moveUp = true;
                        break;
                    case 77:
                        toReturn.moveRight = true;
                        break;
                    case 80:
                        toReturn.moveDown = true;
                        break;
                }
                break;

        }
    }
    bdos(0xC, 0, 0);
    return toReturn;
}

extern "C" void beginFrame() {
    t0 = uclock();
}

extern "C" void doneWithFrame() {
    t1 = uclock();
    ms = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
    timeRendering += ms;

    if (ms < desiredTimeSlice) {
        usleep((desiredTimeSlice - ms) * 1000);
    } else {
        ++desiredTimeSlice;
    }
}

extern "C" void onQuit() {
    stopSounds();
    textmode(C80);
    clrscr();
    puts("Thanks for playing!\n\r\n");
}


extern "C" uint8_t getPaletteEntry(uint32_t origin) {
    uint8_t shade = 0;

    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}

void plot(int x, int y, int color) {
    int b, m; /* bits and mask */
    unsigned char c;
    /* address section differs depending on odd/even scanline */
    bool odd = (1 == (y & 0x1));

    /* divide by 2 (each address section is 100 pixels) */
    y >>= 1;

    /* start bit (b) and mask (m) for 2-bit pixels */
    switch (x & 0x3) {
        case 0:
            b = 6;
            m = 0xC0;
            break;
        case 1:
            b = 4;
            m = 0x30;
            break;
        case 2:
            b = 2;
            m = 0x0C;
            break;
        case 3:
            b = 0;
            m = 0x03;
            break;
    }

    /* divide X by 4 (2 bits for each pixel) */
    x >>= 2;

    unsigned int offset = ((80 * y) + x);

    /* read current pixel */
    if (odd) {
        c = oddBuffer[offset];
    } else {
        c = evenBuffer[offset];
    }

    /* remove bits at new position */
    c = c & ~m;

    /* set bits at new position */
    c = c | (color << b);

    if (odd) {
        oddBuffer[offset] = c;
    } else {
        evenBuffer[offset] = c;
    }
}

int frame = 0;

extern "C" void copyImageBufferToVideoMemory(uint8_t *imageBuffer) {
    if (videoType == kVGA) {
        uint8_t mFinalBuffer[320 * 200];
        uint8_t *currentImageBufferPos = imageBuffer;
        uint8_t *currentBufferPos = mFinalBuffer;

        memset(mFinalBuffer, 0, 320 * 200);

        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x < 320; ++x) {
                *currentBufferPos++ = *currentImageBufferPos++;
            }
        }
        dosmemput(&mFinalBuffer[0], 64000, 0xa0000);
    } else {
        int origin = 0;
        int value = 0;
        int last = 0;
        uint8_t *currentImageBufferPos = imageBuffer;

        for (int y = 0; y < 200; ++y) {
            for (int x = 0; x < 320; ++x) {

                origin = *currentImageBufferPos;

                if (last == origin) {
                    ++currentImageBufferPos;
                    continue;
                }

                value = origin;

                if (0 < origin && origin < 4) {
                    if (((x + y) % 2) == 0) {
                        value = 0;
                    } else {
                        value = origin;
                    }
                }

                if (4 <= origin && origin < 8) {
                    value = origin - 4;
                }

                if (origin >= 8) {
                    if (((x + y) % 2) == 0) {
                        value = 3;
                    } else {
                        value = origin - 8;
                    }
                }

                plot(x, y, value);

                ++currentImageBufferPos;
            }
        }

        dosmemput(evenBuffer, 320 * 100 / 4, 0xB800 * 16);
        dosmemput(oddBuffer, 320 * 100 / 4, (0xB800 * 16) + 0x2000);
    }
}

extern "C" void initVideoFor(EVideoType videoType) {

    if (videoType == kVGA) {
        __dpmi_regs reg;

        reg.x.ax = 0x13;
        __dpmi_int(0x10, &reg);

        outp(0x03c8, 0);

        for (int r = 0; r < 4; ++r) {
            for (int g = 0; g < 8; ++g) {
                for (int b = 0; b < 8; ++b) {
                    outp(0x03c9, (r * (21)));
                    outp(0x03c9, (g * (8)));
                    outp(0x03c9, (b * (8)));
                }
            }
        }
    } else {
        union REGS regs;

        regs.h.ah = 0x00;
        regs.h.al = 0x4;
        int86(0x10, &regs, &regs);
    }
}
#endif